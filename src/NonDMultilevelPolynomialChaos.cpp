/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDMultilevelPolynomialChaos
//- Description: Implementation code for NonDMultilevelPolynomialChaos class
//- Owner:       Mike Eldred, Sandia National Laboratories

#include "NonDMultilevelPolynomialChaos.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSparseGrid.hpp"
#include "DataFitSurrModel.hpp"
#include "SharedPecosApproxData.hpp"
#include "PecosApproximation.hpp"
#include "dakota_data_io.hpp"
#include "dakota_system_defs.hpp"


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDMultilevelPolynomialChaos::
NonDMultilevelPolynomialChaos(ProblemDescDB& problem_db, Model& model):
  NonDPolynomialChaos(BaseConstructor(), problem_db, model),
  expOrderSeqSpec(probDescDB.get_usa("method.nond.expansion_order")),
  collocPtsSeqSpec(probDescDB.get_sza("method.nond.collocation_points")),
  expSamplesSeqSpec(probDescDB.get_sza("method.nond.expansion_samples")),
  quadOrderSeqSpec(probDescDB.get_usa("method.nond.quadrature_order")),
  ssgLevelSeqSpec(probDescDB.get_usa("method.nond.sparse_grid_level")),
  sequenceIndex(0),
  kappaEstimatorRate(
    probDescDB.get_real("method.nond.multilevel_estimator_rate")),
  gammaEstimatorScale(1.),
  pilotSamples(probDescDB.get_sza("method.nond.pilot_samples"))
  //resizedFlag(false), callResize(false)
{
  assign_hierarchical_response_mode();

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  initialize_random(uSpaceType);

  // --------------------
  // Data import settings
  // --------------------
  String pt_reuse = probDescDB.get_string("method.nond.point_reuse");
  if (!importBuildPointsFile.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------

  // pull DB settings
  unsigned short sample_type = probDescDB.get_ushort("method.sample_type");
  const String& rng = probDescDB.get_string("method.random_number_generator");
  short regress_type = probDescDB.get_short("method.nond.regression_type"),
    ls_regress_type
      = probDescDB.get_short("method.nond.least_squares_regression_type");
  Real colloc_ratio_terms_order
    = probDescDB.get_real("method.nond.collocation_ratio_terms_order");
  const UShortArray& tensor_grid_order
    = probDescDB.get_usa("method.nond.tensor_grid_order");

  // extract sequences and invoke shared helper fn with a scalar...
  unsigned short exp_order = USHRT_MAX, quad_order = USHRT_MAX,
    ssg_level = USHRT_MAX, cub_int = USHRT_MAX;
  size_t SZ_MAX = std::numeric_limits<size_t>::max(),
    colloc_pts = SZ_MAX, exp_samples = SZ_MAX;
  if (!expOrderSeqSpec.empty())
    exp_order = (sequenceIndex < expOrderSeqSpec.size()) ?
      expOrderSeqSpec[sequenceIndex] : expOrderSeqSpec.back();
  if (!collocPtsSeqSpec.empty())
    colloc_pts = (sequenceIndex < collocPtsSeqSpec.size()) ?
      collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();
  if (!expSamplesSeqSpec.empty())
    exp_samples = (sequenceIndex < expSamplesSeqSpec.size()) ?
      expSamplesSeqSpec[sequenceIndex] : expSamplesSeqSpec.back();
  if (!quadOrderSeqSpec.empty())
    quad_order = (sequenceIndex < quadOrderSeqSpec.size()) ?
      quadOrderSeqSpec[sequenceIndex] : quadOrderSeqSpec.back();
  if (!ssgLevelSeqSpec.empty())
    ssg_level = (sequenceIndex < ssgLevelSeqSpec.size()) ?
      ssgLevelSeqSpec[sequenceIndex] : ssgLevelSeqSpec.back();

  Iterator u_space_sampler;
  String approx_type;
  UShortArray exp_orders; // defined for expansion_samples/regression
  config_expansion_orders(exp_order, dimPrefSpec, exp_orders);

  if (!config_integration(quad_order, ssg_level, cubIntSpec,
			  u_space_sampler, g_u_model, approx_type) &&
      !config_expectation(exp_samples, sample_type, rng, u_space_sampler,
			  g_u_model, approx_type) &&
      !config_regression(exp_orders, colloc_pts, colloc_ratio_terms_order,
			 regress_type, ls_regress_type, tensor_grid_order,
			 sample_type, rng, pt_reuse, u_space_sampler,
			 g_u_model, approx_type)) {
    Cerr << "Error: incomplete configuration in NonDMultilevelPolynomialChaos "
	 << "constructor." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  //const Variables& g_u_vars = g_u_model.current_variables();
  ActiveSet pce_set = g_u_model.current_response().active_set(); // copy
  pce_set.request_values(3); // stand-alone mode: surrogate grad evals at most
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, importBuildPointsFile, importBuildFormat,
    importBuildActiveOnly,
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format")), false);
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler(importApproxPointsFile, importApproxFormat, 
			      importApproxActiveOnly);

  if (parallelLib.command_line_check())
    Cout << "\nPolynomial_chaos construction completed: initial grid size of "
	 << numSamplesOnModel << " evaluations to be performed." << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ numerical integration (quadrature, sparse grid, cubature). */
NonDMultilevelPolynomialChaos::
NonDMultilevelPolynomialChaos(/*unsigned short method_name,*/ Model& model,
			      short exp_coeffs_approach,
			      const UShortArray& num_int_seq,
			      const RealVector& dim_pref, short u_space_type,
			      bool piecewise_basis, bool use_derivs):
  NonDPolynomialChaos(/*method_name*/MULTIFIDELITY_POLYNOMIAL_CHAOS, model,
		      exp_coeffs_approach, dim_pref, u_space_type,
		      piecewise_basis, use_derivs), 
  sequenceIndex(0), kappaEstimatorRate(2.), gammaEstimatorScale(1.)
{
  assign_hierarchical_response_mode();

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  initialize_random(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  unsigned short quad_order = USHRT_MAX, ssg_level = USHRT_MAX,
                 cub_int    = USHRT_MAX;
  unsigned short num_int = (sequenceIndex < num_int_seq.size()) ?
    num_int_seq[sequenceIndex] : num_int_seq.back();
  switch (exp_coeffs_approach) {
  case Pecos::QUADRATURE:
    quadOrderSeqSpec = num_int_seq; quad_order = num_int; break;
  case Pecos::COMBINED_SPARSE_GRID:
    ssgLevelSeqSpec  = num_int_seq; ssg_level  = num_int; break;
  case Pecos::CUBATURE:             cubIntSpec = num_int; break;
  default:
    Cerr << "Error: Unsupported expansion coefficients approach." << std::endl;
    abort_handler(METHOD_ERROR); break;
  }
  Iterator u_space_sampler; String approx_type;
  config_integration(quad_order, ssg_level, cubIntSpec,
		     u_space_sampler, g_u_model, approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  UShortArray exp_orders; String pt_reuse; // empty for integration approaches
  short corr_order = -1, corr_type = NO_CORRECTION;
  //const Variables& g_u_vars = g_u_model.current_variables();
  ActiveSet pce_set = g_u_model.current_response().active_set(); // copy
  pce_set.request_values(7); // helper mode: support surrogate Hessian evals
                             // TO DO: consider passing in data_mode
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
    outputLevel, pt_reuse), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ regression (least squares, CS, OLI). */
NonDMultilevelPolynomialChaos::
NonDMultilevelPolynomialChaos(unsigned short method_name, Model& model,
			      short exp_coeffs_approach,
			      const UShortArray& exp_order_seq,
			      const RealVector& dim_pref,
			      const SizetArray& colloc_pts_seq,
			      Real colloc_ratio, const SizetArray& pilot,
			      int seed, short u_space_type,
			      bool piecewise_basis, bool use_derivs,
			      bool cv_flag, const String& import_build_pts_file,
			      unsigned short import_build_format,
			      bool import_build_active_only):
  NonDPolynomialChaos(method_name, model, exp_coeffs_approach, dim_pref,
		      u_space_type, piecewise_basis, use_derivs, colloc_ratio,
		      seed, cv_flag), 
  expOrderSeqSpec(exp_order_seq), collocPtsSeqSpec(colloc_pts_seq),
  sequenceIndex(0), kappaEstimatorRate(2.), gammaEstimatorScale(1.),
  pilotSamples(pilot)
{
  assign_hierarchical_response_mode();

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  initialize_random(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  unsigned short exp_order_spec = USHRT_MAX;
  size_t colloc_pts = std::numeric_limits<size_t>::max();
  UShortArray exp_orders; // defined for expansion_samples/regression
  if (!expOrderSeqSpec.empty()) {
    exp_order_spec = (sequenceIndex  < expOrderSeqSpec.size()) ?
      expOrderSeqSpec[sequenceIndex] : expOrderSeqSpec.back();
    config_expansion_orders(exp_order_spec, dimPrefSpec, exp_orders);
  }
  if (!collocPtsSeqSpec.empty())
    colloc_pts =      (sequenceIndex  < collocPtsSeqSpec.size()) ?
      collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();

  Iterator u_space_sampler;
  UShortArray tensor_grid_order; // for OLI + tensorRegression (not supported)
  String approx_type, rng("mt19937"), pt_reuse;
  config_regression(exp_orders, colloc_pts, 1, exp_coeffs_approach,
		    Pecos::DEFAULT_LEAST_SQ_REGRESSION, tensor_grid_order,
		    SUBMETHOD_LHS, rng, pt_reuse, u_space_sampler,
		    g_u_model, approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  //const Variables& g_u_vars = g_u_model.current_variables();
  if (!import_build_pts_file.empty()) pt_reuse = "all";
  ActiveSet pce_set = g_u_model.current_response().active_set(); // copy
  pce_set.request_values(7); // helper mode: support surrogate Hessian evals
                             // TO DO: consider passing in data_mode
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, import_build_pts_file, import_build_format,
    import_build_active_only), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDMultilevelPolynomialChaos::~NonDMultilevelPolynomialChaos()
{ }


/*
bool NonDMultilevelPolynomialChaos::resize()
{
  //resizedFlag = true;

  bool parent_reinit_comms = NonDExpansion::resize();
  
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  initialize(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  UShortArray exp_order; // empty for numerical integration approaches
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    construct_quadrature(u_space_sampler,  g_u_model,
                         quadOrderSeqSpec[sequenceIndex], dimPrefSpec);
    break;
  case Pecos::COMBINED_SPARSE_GRID:
    construct_sparse_grid(u_space_sampler, g_u_model,
                          ssgLevelSeqSpec[sequenceIndex], dimPrefSpec);
    break;
  case Pecos::CUBATURE:
    construct_cubature(u_space_sampler, g_u_model, cubIntSpec);
    break;
  default:
    if (expansionCoeffsApproach == Pecos::ORTHOG_LEAST_INTERPOLATION ||
	expOrderSeqSpec.empty()) {
      // extract number of collocation points
      numSamplesOnModel = (sequenceIndex < collocPtsSeqSpec.size()) ?
	collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();
      // Construct u_space_sampler
      String rng("mt19937");
      construct_lhs(u_space_sampler, g_u_model, SUBMETHOD_LHS,
		    numSamplesOnModel, randomSeed, rng, false, ACTIVE);
    }
    else { // expansion_order-based
      // resolve expansionBasisType, exp_terms, numSamplesOnModel
      if (!expansionBasisType)
	expansionBasisType = (tensorRegression && numContinuousVars <= 5) ?
	  Pecos::TENSOR_PRODUCT_BASIS : Pecos::TOTAL_ORDER_BASIS;
      unsigned short scalar = (sequenceIndex < expOrderSeqSpec.size()) ?
	expOrderSeqSpec[sequenceIndex] : expOrderSeqSpec.back();
      NonDIntegration::dimension_preference_to_anisotropic_order(scalar,
	dimPrefSpec, numContinuousVars, exp_order);

      size_t exp_terms;
      switch (expansionBasisType) {
      case Pecos::TOTAL_ORDER_BASIS:
      case Pecos::ADAPTED_BASIS_GENERALIZED:
      case Pecos::ADAPTED_BASIS_EXPANDING_FRONT:
	exp_terms = Pecos::SharedPolyApproxData::total_order_terms(exp_order);
	break;
      case Pecos::TENSOR_PRODUCT_BASIS:
	exp_terms
	  = Pecos::SharedPolyApproxData::tensor_product_terms(exp_order);
	break;
      }
    
      if (!collocPtsSeqSpec.empty()) // define collocRatio from colloc pts
	collocRatio = terms_samples_to_ratio(exp_terms, numSamplesOnModel);
      else if (collocRatio > 0.)     // define colloc pts from collocRatio
	numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);

      // Construct u_space_sampler
      if (tensorRegression) // tensor sub-sampling
	// define nominal quadrature order as exp_order + 1
	// (m > p avoids most of the 0's in the Psi measurement matrix)
	construct_quadrature(u_space_sampler, g_u_model, scalar+1, dimPrefSpec);
      else {
	String rng("mt19937");
	construct_lhs(u_space_sampler, g_u_model, SUBMETHOD_LHS,
		      numSamplesOnModel, randomSeed, rng, false, ACTIVE);
      }
    }
    break;
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  ActiveSet pce_set = g_u_model.current_response().active_set(); // copy
  pce_set.request_values(7);
  if(expansionCoeffsApproach == Pecos::QUADRATURE ||
      expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
      expansionCoeffsApproach == Pecos::CUBATURE) {
    String pt_reuse, approx_type = "global_projection_orthogonal_polynomial";
    uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
      pce_set, approx_type, exp_order, corr_type, corr_order, data_order,
      outputLevel, pt_reuse), false);
  }
  else {
    String pt_reuse, approx_type = "global_regression_orthogonal_polynomial";

    if (!importBuildPointsFile.empty())
      pt_reuse = "all";

    uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
      pce_set, approx_type, exp_order, corr_type, corr_order, data_order,
      outputLevel, pt_reuse, importBuildPointsFile, importBuildFormat,
      importBuildActiveOnly), false);
  }

  initialize_u_space_model();

  // -------------------------------------
  // (Re)Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler(importApproxPointsFile, importApproxFormat,
			      importApproxActiveOnly);

  return true; // Always need to re-initialize communicators
}
*/


void NonDMultilevelPolynomialChaos::core_run()
{
  initialize_expansion();

  // Options for import of discrepancy data.
  // > There is no good way for import to segregate the desired Q^l sample sets
  //   from among paired sets for Q^0, Q^1 - Q^0, Q^2 - Q^1, ..., Q^L - Q^Lm1.
  //   Would have to develop special processing to match up sample pairs (e.g.,
  //   top down starting from Q^L or bottom up starting from Q^0).
  // > Could import discrepancy data for lev > 0.  But violates use of low level
  //   data within restart (the transfer mechanism), and lookup can't use 2
  //   solution control levels -- would have to rely on, e.g., higher level and
  //   propagate this level from HF model vars to HierarchSurr vars to DataFit
  //   vars to capture imported data in build_global()).  Less clean, more
  //   hack-ish, although may still need better solution level propagation
  //   across models for import consistency() checks to work properly.
  // > Migrate to recursive approximation, where each level only imports one
  //   set of sample data (no solution control hacks or sampling pairing reqd).
  //   Diverges from MLMC, but is more consistent with HierarchInterpPolyApprox.
  //   Other advantages: sample set freedom across levels, reduced cost from
  //   not requiring additional Q^lm1 observations, telescoping consistency of
  //   level surrogates, local error estimates relative to previous surrogate.
  //   >> supporting recursive and distinct emulation cases would lead to
  //      uneven support for import.  Best to implement, verify, migrate?
  //   >> want to support import for MF PCE as well, including future
  //      adaptive MF PCE.

  bool multifid_uq = false;
  switch (methodName) {
  case MULTIFIDELITY_POLYNOMIAL_CHAOS:
    multifid_uq = true;
    multifidelity_expansion(); // from NonDExpansion, includes sanity checks
    break;
  case MULTILEVEL_POLYNOMIAL_CHAOS: {
    if (expansionCoeffsApproach < Pecos::DEFAULT_REGRESSION) {
      Cerr << "Error: unsupported solver configuration within "
	   << "NonDMultilevelPolynomialChaos::core_run()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    multilevel_regression();
    break;
  }
  default:
    Cerr << "Error: bad configuration in NonDMultilevelPolynomialChaos::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
  
  // generate final results
  Cout << "\n----------------------------------------------------\n";
  if (multifid_uq) Cout << "Multifidelity UQ: ";
  else             Cout <<    "Multilevel UQ: ";
  Cout << "approximated high fidelity results"
       << "\n----------------------------------------------------\n\n";
  annotated_results(); // full set of statistics and debug traces (default)
  if (!summaryOutputFlag) // post_run() output is suppressed, leading to
    print_results(Cout);  // intermediate output wth no final output

  // clean up for re-entrancy of ML PCE
  uSpaceModel.clear_stored();

  ++numUncertainQuant;
}


void NonDMultilevelPolynomialChaos::increment_specification_sequence()
{
  bool update_exp = false, update_sampler = false, update_from_ratio = false;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex+1 < quadOrderSeqSpec.size()) {
      ++sequenceIndex;      // advance order sequence if sufficient entries
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    }
    else if (refineControl)
      nond_quad->reset();   // reset driver to pre-refinement state
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::HIERARCHICAL_SPARSE_GRID: {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex+1 < ssgLevelSeqSpec.size()) {
      ++sequenceIndex;      // advance level sequence if sufficient entries
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    }
    else if (refineControl)
      nond_sparse->reset(); // reset driver to pre-refinement state
    break;
  }
  case Pecos::CUBATURE:
    Cerr << "Error: cubature sequences not supported in NonDMultilevel"
	 << "PolynomialChaos::increment_specification_sequence()" << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  case Pecos::SAMPLING: {
    // advance expansionOrder and/or expansionSamples, as admissible
    size_t next_i = sequenceIndex + 1;
    if (next_i <   expOrderSeqSpec.size()) update_exp = true;
    if (next_i < expSamplesSeqSpec.size())
      { numSamplesOnModel = expSamplesSeqSpec[next_i]; update_sampler = true; }
    if (update_exp || update_sampler) ++sequenceIndex;
    break;
  }
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    // advance collocationPoints
    if (sequenceIndex+1 < collocPtsSeqSpec.size()) {
      update_sampler = true; ++sequenceIndex;
      numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
    }
    break;
  default: { // regression
    // advance expansionOrder and/or collocationPoints, as admissible
    size_t next_i = sequenceIndex + 1;
    if (next_i <  expOrderSeqSpec.size()) update_exp = true;
    if (next_i < collocPtsSeqSpec.size())
      { numSamplesOnModel = collocPtsSeqSpec[next_i]; update_sampler = true; }
    if (update_exp || update_sampler) ++sequenceIndex;
    if (update_exp && collocPtsSeqSpec.empty()) // (fixed) collocation ratio
      update_from_ratio = update_sampler = true;
    break;
  }
  }

  UShortArray exp_order;
  if (update_exp) {
    // update expansion order within Pecos::SharedOrthogPolyApproxData
    NonDIntegration::dimension_preference_to_anisotropic_order(
      expOrderSeqSpec[sequenceIndex], dimPrefSpec, numContinuousVars,
      exp_order);
    SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
      uSpaceModel.shared_approximation().data_rep();
    shared_data_rep->expansion_order(exp_order);
    if (update_from_ratio) { // update numSamplesOnModel from collocRatio
      size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
	Pecos::SharedPolyApproxData::tensor_product_terms(exp_order) :
	Pecos::SharedPolyApproxData::total_order_terms(exp_order);
      numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);
    }
  }
  else if (update_sampler && tensorRegression) {
    // extract unchanged expansion order from Pecos::SharedOrthogPolyApproxData
    SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
      uSpaceModel.shared_approximation().data_rep();
    exp_order = shared_data_rep->expansion_order();
  }

  // udpate sampler settings (NonDQuadrature or NonDSampling)
  if (update_sampler) {
    if (tensorRegression) {
      NonDQuadrature* nond_quad
	= (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
      nond_quad->samples(numSamplesOnModel);
      if (nond_quad->mode() == RANDOM_TENSOR) { // sub-sampling i/o filtering
	UShortArray dim_quad_order(numContinuousVars);
	for (size_t i=0; i<numContinuousVars; ++i)
	  dim_quad_order[i] = exp_order[i] + 1;
        nond_quad->quadrature_order(dim_quad_order);
      }
      nond_quad->update(); // sanity check on sizes, likely a no-op
    }
    else { // enforce increment through sampling_reset()
      // no lower bound on samples in the subiterator
      uSpaceModel.subordinate_iterator().sampling_reference(0);
      DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
      dfs_model->total_points(numSamplesOnModel);
    }
  }
}


void NonDMultilevelPolynomialChaos::
increment_sample_sequence(size_t new_samp, size_t total_samp, size_t lev)
{
  numSamplesOnModel = new_samp;

  bool update_exp = false, update_sampler = false, update_from_ratio = false,
    err_flag = false;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: case Pecos::COMBINED_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: case Pecos::CUBATURE:
    err_flag = true; break;
  case Pecos::SAMPLING:
    //if () update_exp = true;
    update_sampler = true; break;
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    update_sampler = true; break;
  default: // regression
    update_exp = update_sampler = true;
    if (collocPtsSeqSpec.empty()) // (fixed) collocation ratio
      update_from_ratio = true;
    break;
  }

  if (update_exp) {
    if (update_from_ratio) {
      //increment_order_from_grid(); // need total samples
      unsigned short exp_order = (lev < expOrderSeqSpec.size()) ?
	expOrderSeqSpec[lev] : expOrderSeqSpec.back();
      // reset lower bound for each level
      UShortArray exp_orders;
      config_expansion_orders(exp_order, dimPrefSpec, exp_orders);
      ratio_samples_to_order(collocRatio, total_samp, exp_orders, false);
      SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
	uSpaceModel.shared_approximation().data_rep();
      shared_data_rep->expansion_order(exp_orders);
    }
    else
      err_flag = true;
  }

  // udpate sampler settings (NonDQuadrature or NonDSampling)
  if (update_sampler) {
    Iterator* sub_iter_rep = uSpaceModel.subordinate_iterator().iterator_rep();
    if (tensorRegression) {
      NonDQuadrature* nond_quad = (NonDQuadrature*)sub_iter_rep;
      nond_quad->samples(numSamplesOnModel);
      if (nond_quad->mode() == RANDOM_TENSOR) { // sub-sampling i/o filtering
	SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
	  uSpaceModel.shared_approximation().data_rep();
	const UShortArray& exp_orders = shared_data_rep->expansion_order();
	UShortArray dim_quad_order(numContinuousVars);
	for (size_t i=0; i<numContinuousVars; ++i)
	  dim_quad_order[i] = exp_orders[i] + 1;
	nond_quad->quadrature_order(dim_quad_order);
      }
      nond_quad->update(); // sanity check on sizes, likely a no-op
    }
    // test for valid sampler for case of build data import (unstructured grid)
    else if (sub_iter_rep != NULL) { // enforce increment using sampling_reset()
      sub_iter_rep->sampling_reference(0);// no lower bnd on samples in sub-iter
      DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
      // total including reuse from DB/file (does not include previous ML iter)
      dfs_model->total_points(numSamplesOnModel);
    }
  }

  if (err_flag) {
    Cerr << "Error: option not yet supported in NonDMultilevelPolynomialChaos::"
	 << "increment_sample_sequence." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void NonDMultilevelPolynomialChaos::multilevel_regression()
{
  // Allow either model forms or discretization levels, but not both
  // (discretization levels take precedence)
  ModelList& ordered_models = iteratedModel.subordinate_models(false);
  size_t form, num_mf = ordered_models.size();
  ModelLIter   m_iter = ordered_models.begin();
  std::advance(m_iter, num_mf - 1); // HF model
  size_t i, index, lev, num_lev, num_hf_lev = m_iter->solution_levels();
  bool multilev, recursive = (multilevDiscrepEmulation == RECURSIVE_EMULATION);
  RealVector cost;
  if (num_hf_lev > 1) {
    multilev = true; num_lev = num_hf_lev; form = num_mf - 1;
    cost = m_iter->solution_level_costs();
    if (num_mf > 1)
      Cerr << "Warning: multiple model forms will be ignored in "
	   << "NonDMultilevelPolynomialChaos::multilevel_regression().\n";
  }
  else if (num_mf > 1) {
    multilev = false; num_lev = num_mf;
    cost.sizeUninitialized(num_mf);
    for (i=0, m_iter=ordered_models.begin(); i<num_mf; ++i, ++m_iter)
      cost[i] = m_iter->solution_level_cost(); // cost for active soln index
  }
  else {
    Cerr << "Error: no model hierarchy evident in NonDMultilevel"
	 << "PolynomialChaos::multilevel_regression()." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Multilevel variance aggregation requires independent sample sets
  Iterator* u_sub_iter = uSpaceModel.subordinate_iterator().iterator_rep();
  if (u_sub_iter != NULL)
    ((Analyzer*)u_sub_iter)->vary_pattern(true);

  size_t iter = 0, new_N_l, last_active = 0;
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real eps_sq_div_2, sum_root_var_cost, estimator_var0 = 0., lev_cost, var_l; 
  // retrieve cost estimates across soln levels for a particular model form
  RealVector agg_var(num_lev);
  // factors for relationship between variance of mean estimator and NLev
  // (hard coded for right now; TO DO: fit params)
  Real inv_k = 1./kappaEstimatorRate, inv_kp1 = 1./(kappaEstimatorRate+1.);
  
  // Build point import is active only for the pilot sample and we overlay an
  // additional pilot_sample spec, but we do not augment with samples from a
  // collocation pts/ratio enforcement (pts/ratio controls take over on
  // subsequent iterations).
  bool import_pilot = (!importBuildPointsFile.empty());// && recursive);
  if (import_pilot) {
    if (recursive)
      Cout << "\nPilot sample to include imported build points.\n";
    else {
      Cerr << "Error: build data import only supported for recursive emulation "
	   << "in NonDMultilevelPolynomialChaos::multilevel_regression()."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  // Load the pilot sample from user specification
  SizetArray delta_N_l(num_lev);
  load_pilot_sample(pilotSamples, delta_N_l);

  // now converge on sample counts per level (NLev)
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  NLev.assign(num_lev, 0);
  while ( iter <= max_iter &&
	  ( Pecos::l1_norm(delta_N_l) || (iter == 0 && import_pilot) ) ) {

    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);

    sum_root_var_cost = 0.;
    for (lev=0; lev<num_lev; ++lev) {

      lev_cost = cost[lev];
      if (multilev) iteratedModel.truth_model_indices(form, lev);
      else          iteratedModel.truth_model_indices(lev);
      if (lev && !recursive) {
	if (lev == 1) iteratedModel.surrogate_response_mode(MODEL_DISCREPANCY);
	if (multilev) iteratedModel.surrogate_model_indices(form, lev-1);
	else          iteratedModel.surrogate_model_indices(lev-1);
	lev_cost += cost[lev-1]; // discrepancies incur 2 level costs
      }
      index = (recursive) ? lev : _NPOS;

      if (iter == 0) { // initial expansion build
	// Update solution control variable in uSpaceModel to support
	// DataFitSurrModel::consistent() logic
	if (import_pilot)
	  uSpaceModel.update_from_subordinate_model(); // max depth

	NLev[lev] += delta_N_l[lev]; // update total samples for this level
	increment_sample_sequence(delta_N_l[lev], NLev[lev], lev);
	if (lev == 0) compute_expansion(index); // init + build; not recursive
	else           update_expansion(index); //   just build; not recursive

	if (import_pilot) { // update counts to include imported data
	  NLev[lev] = delta_N_l[lev]
	    = uSpaceModel.approximation_data(0).points();
	  Cout << "Pilot count including import = " << delta_N_l[lev] << "\n\n";
	  // Trap zero samples as it will cause FPE downstream
	  if (NLev[lev] == 0) { // no pilot spec, no import match
	    Cerr << "Error: insufficient sample recovery for level " << lev
		 << " in multilevel_regression()." << std::endl;
	    abort_handler(METHOD_ERROR);
	  }
	}
      }
      else if (delta_N_l[lev]) {
	// retrieve prev expansion for this level & append new samples
	uSpaceModel.restore_approximation(lev);
	NLev[lev] += delta_N_l[lev]; // update total samples for this level
	increment_sample_sequence(delta_N_l[lev], NLev[lev], lev);
	// Note: import build data is not re-processed by append_expansion()
	append_expansion(index); // not recursive
      }

      // aggregate variances across QoI for estimating NLev (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_l = agg_var[lev]; // carried over from prev iter if no samp
      if (delta_N_l[lev]) {
        // compute and accumulate variance of mean estimator from the set of
	// fold results within the selected settings from cross-validation:
	agg_var_l = 0.;
	for (i=0; i<numFunctions; ++i) {
	  PecosApproximation* poly_approx_q
	    = (PecosApproximation*)poly_approxs[i].approx_rep();
	  var_l = poly_approx_q->variance();
	  agg_var_l += var_l;
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "Variance (lev " << lev << ", qoi " << i
		 << ", iter " << iter << ") = " << var_l << '\n';
	  // We assume a functional dependence of estimator variance on NLev
	  // for minimizing aggregate cost subject to an MSE error balance:
	  //   Var(Q-hat) = sigma_Q^2 / (gamma NLev^kappa)
	  // where Monte Carlo has gamma = kappa = 1.  To fit these parameters,
	  // one approach is to numerically estimate the variance in the mean
	  // estimator (alpha_0) from two sources:
	  // > from variation across k folds for the selected CV settings
	  // > from var decrease as NLev increases across iters
          //Real cv_var_i = poly_approx_rep->
	  //  cross_validation_solver().cv_metrics(MEAN_ESTIMATOR_VARIANCE);
	  //  (need to make MultipleSolutionLinearModelCrossValidationIterator
	  //   cv_iterator class scope)
	  // To validate this approach, the actual estimator variance can be
	  // computed and compared with the CV variance approximation (as for
	  // traditional CV error plots, but predicting estimator variance
	  // instead of L2 fit error).
	}
        // store all approximation levels, whenever recomputed.
	uSpaceModel.store_approximation(lev);
	// The active approximation upon completion of the refinement loop may
	// be any level -> store the last approximation index for use within
	// combine_approximation().
	last_active = lev;
      }

      sum_root_var_cost	+= std::pow(agg_var_l * 
	std::pow(lev_cost, kappaEstimatorRate), inv_kp1);
      // MSE reference is MC applied to HF:
      if (iter == 0) estimator_var0 += agg_var_l / NLev[lev];
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / NLev).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (iter == 0) { // eps^2 / 2 = var * relative factor
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // update targets based on variance estimates
    Real fact
      = std::pow(sum_root_var_cost / eps_sq_div_2 / gammaEstimatorScale, inv_k);
    for (lev=0; lev<num_lev; ++lev) {
      lev_cost = cost[lev];
      if (lev && !recursive) lev_cost += cost[lev-1];
      new_N_l = std::pow(agg_var[lev] / lev_cost, inv_kp1) * fact;
      delta_N_l[lev] = (new_N_l > NLev[lev]) ? new_N_l - NLev[lev] : 0;
    }
    ++iter;
    Cout << "\nML PCE iteration " << iter << " sample increments:\n"
	 << delta_N_l << std::endl;
  }

  // remove redundancy between current active and stored, prior to combining
  uSpaceModel.remove_stored_approximation(last_active);
  // compute aggregate expansion and generate its statistics
  uSpaceModel.combine_approximation();

  // compute the equivalent number of HF evaluations
  equivHFEvals = NLev[0] * cost[0]; // first level is single eval
  for (lev=1; lev<num_lev; ++lev)  // subsequent levels incur 2 model costs
    equivHFEvals += (recursive) ? NLev[lev] * cost[lev] :
      NLev[lev] * (cost[lev] + cost[lev-1]);
  equivHFEvals /= cost[num_lev-1]; // normalize into equivalent HF evals
}


void NonDMultilevelPolynomialChaos::
print_results(std::ostream& s, short results_state)
{
  if (outputLevel >= NORMAL_OUTPUT)
    print_coefficients(s);
  if (!expansionExportFile.empty())
    export_coefficients();

  if (!NLev.empty()) {
    s << "<<<<< Samples per solution level:\n";
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << std::endl;
  }

  // skip over NonDPolynomialChaos::print_results()
  NonDExpansion::print_results(s, results_state);
}

} // namespace Dakota
