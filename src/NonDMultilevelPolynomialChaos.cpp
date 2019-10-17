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
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSparseGrid.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
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
  mlmfAllocControl(
    probDescDB.get_short("method.nond.multilevel_allocation_control")),
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
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  //initialize_random(uSpaceType);

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
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    uSpaceType), false); // retain dist bounds

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

  // mlmfAllocControl config and specification checks:
  if (methodName == MULTILEVEL_POLYNOMIAL_CHAOS) {
    if (expansionCoeffsApproach < Pecos::DEFAULT_REGRESSION) {
      Cerr << "Error: unsupported solver configuration within "
	   << "NonDMultilevelPolynomialChaos::core_run()." << std::endl;
      abort_handler(METHOD_ERROR);
    }

    switch (mlmfAllocControl) {
    case RIP_SAMPLING:
      // use OMP for robustness (over or under-determined)
      if (expansionCoeffsApproach == Pecos::DEFAULT_REGRESSION)
	expansionCoeffsApproach    = Pecos::ORTHOG_MATCH_PURSUIT;
      // Require CV, else OMP will compute #terms = #samples.
      // Use noise only to avoid interaction with any order progression.
      //if (!crossValidation)     Cerr << "Warning: \n";
      //if (!crossValidNoiseOnly) Cerr << "Warning: \n";
      crossValidation = crossValidNoiseOnly = true;
      // Main accuracy control is shared expansion order / dictionary size
      break;
    default: //case ESTIMATOR_VARIANCE: case GREEDY_REFINEMENT:
      break;
    }
  }

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel consumes QoI aggregations; supports surrogate grad evals at most
  ShortArray asv(g_u_model.qoi(), 3); // for stand alone mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
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
			      short refine_type, short refine_control,
			      short covar_control, short ml_alloc_control,
			      short ml_discrep, short rule_nest,
			      short rule_growth, bool piecewise_basis,
			      bool use_derivs):
  NonDPolynomialChaos(/*method_name*/MULTIFIDELITY_POLYNOMIAL_CHAOS, model,
		      exp_coeffs_approach, dim_pref, u_space_type, refine_type,
		      refine_control, covar_control, ml_discrep, rule_nest,
		      rule_growth, piecewise_basis, use_derivs), 
  mlmfAllocControl(ml_alloc_control), sequenceIndex(0), kappaEstimatorRate(2.),
  gammaEstimatorScale(1.)
{
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  //initialize_random(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    uSpaceType), false); // retain dist bounds

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
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
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
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  ShortArray asv(g_u_model.qoi(), 7); // TO DO: consider passing in data_mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
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
			      short refine_type, short refine_control,
			      short covar_control, short ml_alloc_control,
			      short ml_discrep,
			      //short rule_nest, short rule_growth,
			      bool piecewise_basis, bool use_derivs,
			      bool cv_flag, const String& import_build_pts_file,
			      unsigned short import_build_format,
			      bool import_build_active_only):
  NonDPolynomialChaos(method_name, model, exp_coeffs_approach, dim_pref,
		      u_space_type, refine_type, refine_control, covar_control,
		      ml_discrep, //rule_nest, rule_growth,
		      piecewise_basis, use_derivs, colloc_ratio, seed, cv_flag),
  mlmfAllocControl(ml_alloc_control), expOrderSeqSpec(exp_order_seq),
  collocPtsSeqSpec(colloc_pts_seq), sequenceIndex(0), kappaEstimatorRate(2.),
  gammaEstimatorScale(1.), pilotSamples(pilot)
{
  assign_discrepancy_mode();
  assign_hierarchical_response_mode();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  //initialize_random(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    uSpaceType), false); // retain dist bounds

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
  if (!import_build_pts_file.empty()) pt_reuse = "all";
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  ShortArray asv(g_u_model.qoi(), 7); // TO DO: consider passing in data_mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
    outputLevel, pt_reuse, import_build_pts_file, import_build_format,
    import_build_active_only), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDMultilevelPolynomialChaos::~NonDMultilevelPolynomialChaos()
{ }


void NonDMultilevelPolynomialChaos::assign_discrepancy_mode()
{
  // assign alternate defaults for correction and discrepancy emulation types
  switch (iteratedModel.correction_type()) {
  //case ADDITIVE_CORRECTION:
  //case MULTIPLICATIVE_CORRECTION:
  case NO_CORRECTION: // assign method-specific default
    iteratedModel.correction_type(ADDITIVE_CORRECTION); break;
  }

  switch (multilevDiscrepEmulation) {
  //case DISTINCT_EMULATION:
  //case RECURSIVE_EMULATION:
  case DEFAULT_EMULATION: // assign method-specific default
    multilevDiscrepEmulation = DISTINCT_EMULATION;      break;
  }
}


void NonDMultilevelPolynomialChaos::assign_hierarchical_response_mode()
{
  // override default SurrogateModel::responseMode for purposes of setting
  // comms for the ordered Models within HierarchSurrModel::set_communicators(),
  // which precedes mode updates in {multifidelity,multilevel}_expansion().

  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: multilevel/multifidelity expansions require a "
	 << "hierarchical model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // ML-MF PCE is based on model discrepancies, but multi-index PCE may evolve
  // towards BYPASS_SURROGATE as sparse grids in model space will manage QoI
  // differences.  (See also hierarchical multilevel SC.)
  if (multilevDiscrepEmulation == RECURSIVE_EMULATION)
    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);
  else
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);//MODEL_DISCREPANCY
}


void NonDMultilevelPolynomialChaos::initialize_u_space_model()
{
  // For greedy ML, activate combined stats now for propagation to Pecos
  // > don't call statistics_type() as ExpansionConfigOptions not initialized
  //if (mlmfAllocControl == GREEDY_REFINEMENT)
  //  statsType = Pecos::COMBINED_EXPANSION_STATS;

  // initializes ExpansionConfigOptions, among other things
  NonDPolynomialChaos::initialize_u_space_model();

  // Bind more than one SurrogateData instance via DataFitSurrModel ->
  // PecosApproximation
  uSpaceModel.link_multilevel_approximation_data();
}


/*
bool NonDMultilevelPolynomialChaos::resize()
{
  //resizedFlag = true;

  bool parent_reinit_comms = NonDExpansion::resize();
  
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);
  //initialize_random(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    uSpaceType), false); // retain dist bounds

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
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
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
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  // DFSModel: consume any QoI aggregation. Resize: support approx Hessians.
  ShortArray asv(g_u_model.qoi(), 7);
  ActiveSet pce_set(asv, recast_set.derivative_vector());
  if (expansionCoeffsApproach == Pecos::QUADRATURE ||
      expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
      expansionCoeffsApproach == Pecos::INCREMENTAL_SPARSE_GRID ||
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
  sequenceIndex = 0;

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
    // general-purpose algorithms inherited from NonDExpansion:
    switch (mlmfAllocControl) {
    case GREEDY_REFINEMENT:    greedy_multifidelity_expansion();    break;
    default:                   multifidelity_expansion(refineType); break;
    }
    break;
  case MULTILEVEL_POLYNOMIAL_CHAOS:
    switch (mlmfAllocControl) {
    case GREEDY_REFINEMENT:    greedy_multifidelity_expansion();  break;
    // Projection-based approaches would require multifidelity_expansion(),
    // but these options are currently precluded by the ML PCE specification.
    default:                   multilevel_regression();           break;
    }
    break;
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
  compute_statistics(FINAL_RESULTS);
  // Override summaryOutputFlag control (see Analyzer::post_run()) for ML case
  // to avoid intermediate output with no final output
  if (!summaryOutputFlag) print_results(Cout, FINAL_RESULTS);

  // clean up for re-entrancy of ML PCE
  uSpaceModel.clear_inactive();

  finalize_expansion();
}


void NonDMultilevelPolynomialChaos::assign_specification_sequence()
{
  bool update_exp = false, update_sampler = false, update_from_ratio = false;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex < quadOrderSeqSpec.size())
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    else if (refineControl)
      nond_quad->reset();   // reset driver to pre-refinement state
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    NonDSparseGrid* nond_sparse
      = (NonDSparseGrid*)uSpaceModel.subordinate_iterator().iterator_rep();
    if (sequenceIndex < ssgLevelSeqSpec.size())
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
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
    // assign expansionOrder and/or expansionSamples, as admissible
    if (sequenceIndex <   expOrderSeqSpec.size()) update_exp = true;
    if (sequenceIndex < expSamplesSeqSpec.size()) {
      numSamplesOnModel = expSamplesSeqSpec[sequenceIndex];
      update_sampler = true;
    }
    break;
  }
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    // assign collocationPoints
    if (sequenceIndex < collocPtsSeqSpec.size()) {
      numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
      update_sampler = true;
    }
    break;
  default: { // regression
    // assign expansionOrder and/or collocationPoints, as admissible
    if (sequenceIndex <  expOrderSeqSpec.size()) update_exp = true;
    if (sequenceIndex < collocPtsSeqSpec.size()) {
      numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
      update_sampler = true;
    }
    if (update_exp && collocPtsSeqSpec.empty()) // (fixed) collocation ratio
      update_from_ratio = update_sampler = true;
    break;
  }
  }

  update_from_specification(update_exp, update_sampler, update_from_ratio);
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
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
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

  update_from_specification(update_exp, update_sampler, update_from_ratio);
}


void NonDMultilevelPolynomialChaos::
update_from_specification(bool update_exp, bool update_sampler,
			  bool update_from_ratio)
{
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
  case Pecos::QUADRATURE:            case Pecos::CUBATURE:
  case Pecos::COMBINED_SPARSE_GRID:  case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID:
    err_flag = true; break;
  case Pecos::SAMPLING:
    //if () update_exp = true;
    update_sampler = true; break;
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    update_sampler = true; break;
  default: // regression
    update_exp = update_sampler = true;
    // fix the basis cardinality in the case of RIP_SAMPLING
    if (mlmfAllocControl != RIP_SAMPLING) {
      if (collocPtsSeqSpec.empty()) // (fixed) collocation ratio
	update_from_ratio = true;
      else // config via collocation pts sequence not supported
	err_flag = true;
    }
    break;
  }

  if (update_exp) {
    //increment_order_from_grid(); // need total samples
    unsigned short exp_order = (lev < expOrderSeqSpec.size()) ?
      expOrderSeqSpec[lev] : expOrderSeqSpec.back();
    // reset lower bound for each level
    UShortArray exp_orders;
    config_expansion_orders(exp_order, dimPrefSpec, exp_orders);

    if (update_from_ratio) // update the exp_orders based on total_samp
      ratio_samples_to_order(collocRatio, total_samp, exp_orders, false);

    SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
      uSpaceModel.shared_approximation().data_rep();
    shared_data_rep->expansion_order(exp_orders);
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
  // remove default key (empty activeKey) since this interferes with
  // combine_approximation().  Also useful for ML/MF re-entrancy.
  uSpaceModel.clear_model_keys();
  // all stats are level stats
  statistics_type(Pecos::ACTIVE_EXPANSION_STATS);

  // Allow either model forms or discretization levels, but not both
  // (discretization levels take precedence)
  unsigned short lev, form;
  size_t num_lev, iter = 0, max_iter = (maxIterations < 0) ? 25 : maxIterations;
  Real eps_sq_div_2, sum_root_var_cost, estimator_var0 = 0.; 
  RealVector cost;
  bool multilev, optional_cost = (mlmfAllocControl == RIP_SAMPLING),
    recursive = (multilevDiscrepEmulation == RECURSIVE_EMULATION);
  configure_levels(num_lev, form, multilev, false);
  configure_cost(num_lev, multilev, cost);
  SizetArray cardinality;  RealVector level_metric(num_lev);
  if (mlmfAllocControl == RIP_SAMPLING)
    cardinality.resize(num_lev);

  // Multilevel variance aggregation requires independent sample sets
  Iterator* u_sub_iter = uSpaceModel.subordinate_iterator().iterator_rep();
  if (u_sub_iter != NULL)
    ((Analyzer*)u_sub_iter)->vary_pattern(true);

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
  NLev.assign(num_lev, 0);
  while ( iter <= max_iter &&
	  ( Pecos::l1_norm(delta_N_l) || (iter == 0 && import_pilot) ) ) {

    sum_root_var_cost = 0.;
    for (lev=0; lev<num_lev; ++lev) {

      configure_indices(lev, form, multilev);

      if (iter == 0) { // initial expansion build
	// Update solution control variable in uSpaceModel to support
	// DataFitSurrModel::consistent() logic
	if (import_pilot)
	  uSpaceModel.update_from_subordinate_model(); // max depth

	NLev[lev] += delta_N_l[lev]; // update total samples for this level
	increment_sample_sequence(delta_N_l[lev], NLev[lev], lev);
	if (lev == 0 || import_pilot)
	  compute_expansion(); // init + import + build; not recursive
	else
	  update_expansion();  // just build; not recursive

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
	NLev[lev] += delta_N_l[lev]; // update total samples for this level
	increment_sample_sequence(delta_N_l[lev], NLev[lev], lev);
	// Note: import build data is not re-processed by append_expansion()
	append_expansion(); // not recursive
      }

      switch (mlmfAllocControl) {
      case RIP_SAMPLING: // use RMS of sparsity across QoI
	if (delta_N_l[lev] > 0)
	  sparsity_metrics(cardinality[lev], level_metric[lev], 2.);
	break;
      default: { //case ESTIMATOR_VARIANCE:
	Real& agg_var_l = level_metric[lev];
	if (delta_N_l[lev] > 0) aggregate_variance(agg_var_l);
	sum_root_var_cost += std::pow(agg_var_l *
	  std::pow(level_cost(lev, cost), kappaEstimatorRate),
	  1./(kappaEstimatorRate+1.));
        // MSE reference is ML MC aggregation for pilot(+import) sample:
	if (iter == 0) estimator_var0 += agg_var_l / NLev[lev];
	break;
      }
      }
    }

    switch (mlmfAllocControl) {
    case RIP_SAMPLING:
      compute_sample_increment(cardinality, 2., level_metric, NLev, delta_N_l);
      break;
    default: //case ESTIMATOR_VARIANCE:
      if (iter == 0) { // eps^2 / 2 = var * relative factor
	eps_sq_div_2 = estimator_var0 * convergenceTol;
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Epsilon squared target = " << eps_sq_div_2 << '\n';
      }
      compute_sample_increment(level_metric, cost, sum_root_var_cost,
			       eps_sq_div_2, NLev, delta_N_l);
      break;
    }
    ++iter;
    Cout << "\nML PCE iteration " << iter << " sample increments:\n"
	 << delta_N_l << std::endl;
  }
  compute_equivalent_cost(NLev, cost); // compute equivalent # of HF evals

  combined_to_active(); // combine PCE terms and promote to active expansion
  // Final annotated results are computed / printed in core_run()
}


void NonDMultilevelPolynomialChaos::aggregate_variance(Real& agg_var_l)
{
  // case ESTIMATOR_VARIANCE:
  // statsType remains as Pecos::ACTIVE_EXPANSION_STATS

  // control ML using aggregated variance across the vector of QoI
  // (alternate approach: target QoI with largest variance)
  agg_var_l = 0.;
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    PecosApproximation* poly_approx_q
      = (PecosApproximation*)poly_approxs[qoi].approx_rep();
    Real var_l = poly_approx_q->variance(); // for active level
    agg_var_l += var_l;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Variance(" /*"lev " << lev << ", "*/ << "qoi " << qoi
	/* << ", iter " << iter */ << ") = " << var_l << '\n';
  }
}


/* Retrieve basis cardinality and compute power mean of sparsity
   (common power values: 1 = average, 2 = root mean square). */
void NonDMultilevelPolynomialChaos::
sparsity_metrics(size_t& cardinality_l, Real& sparsity_metric_l, Real power)
{
  // case RIP_SAMPLING:

  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  cardinality_l = shared_data_rep->expansion_terms();// shared multiIndex.size()

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  Real sum = 0.; bool pow1 = (power == 1.); // simple average
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    PecosApproximation* poly_approx_q
      = (PecosApproximation*)poly_approxs[qoi].approx_rep();
    size_t sparsity_l = poly_approx_q->sparsity();
    sum += (pow1) ? sparsity_l : std::pow((Real)sparsity_l, power);
    //if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Sparsity(" /*lev " << lev << ", "*/ << "qoi " << qoi
	/* << ", iter " << iter */ << ") = " << sparsity_l << '\n';
  }
  sum /= numFunctions;
  sparsity_metric_l = (pow1) ? sum : std::pow(sum, 1. / power);
}


void NonDMultilevelPolynomialChaos::
compute_sample_increment(const RealVector& agg_var, const RealVector& cost,
			 Real sum_root_var_cost, Real eps_sq_div_2,
			 const SizetArray& N_l, SizetArray& delta_N_l)
{
  // case ESTIMATOR_VARIANCE:

  // eps^2 / 2 target computed based on relative tolerance: total MSE = eps^2
  // which is equally apportioned (eps^2 / 2) among discretization MSE and
  // estimator variance (\Sum var_Y_l / NLev).  Since we do not know the
  // discretization error, we compute an initial estimator variance and then
  // seek to reduce it by a relative_factor <= 1.

  // We assume a functional dependence of estimator variance on NLev
  // for minimizing aggregate cost subject to an MSE error balance:
  //   Var(Q-hat) = sigma_Q^2 / (gamma NLev^kappa)
  // where Monte Carlo has gamma = kappa = 1.  To fit these parameters,
  // one approach is to numerically estimate the variance in the mean
  // estimator (alpha_0) from two sources:
  // > from variation across k folds for the selected CV settings
  // > from var decrease as NLev increases across iters

  // compute and accumulate variance of mean estimator from the set of
  // k-fold results within the selected settings from cross-validation:
  //Real cv_var_i = poly_approx_rep->
  //  cross_validation_solver().cv_metrics(MEAN_ESTIMATOR_VARIANCE);
  //  (need to make MultipleSolutionLinearModelCrossValidationIterator
  //   cv_iterator class scope)
  // To validate this approach, the actual estimator variance can be
  // computed and compared with the CV variance approximation (as for
  // traditional CV error plots, but predicting estimator variance
  // instead of L2 fit error).

  // update targets based on variance estimates
  Real new_N_l; size_t lev, num_lev = N_l.size();
  Real fact = std::pow(sum_root_var_cost / eps_sq_div_2 / gammaEstimatorScale,
		       1. / kappaEstimatorRate);
  for (lev=0; lev<num_lev; ++lev) {
    new_N_l = std::pow(agg_var[lev] / level_cost(lev, cost),
		       1. / (kappaEstimatorRate+1.)) * fact;
    delta_N_l[lev] = one_sided_delta(N_l[lev], new_N_l);
  }
}


void NonDMultilevelPolynomialChaos::
compute_sample_increment(const SizetArray& cardinality, Real factor,
			 const RealVector& sparsity, const SizetArray& N_l,
			 SizetArray& delta_N_l)
{
  // case RIP_SAMPLING:

  // update targets based on sparsity estimates
  Real s/*, card*/; size_t lev, num_lev = N_l.size();
  RealVector new_N_l(num_lev, false);
  for (lev=0; lev<num_lev; ++lev) {
    s = sparsity[lev]; //card = (Real)cardinality[lev];
    // RIP samples ~= s log^3(s) log(C), but we are more interested in the shape
    // of the profile, since the actual values are conservative upper bounds
    // --> can omit constant terms that don't affect shape, e.g. log(C)
    new_N_l[lev] = s * std::pow(std::log(s), 3.); //* std::log(card);
  }

  // Sparsity estimates tend to grow for compressible QoI as increased samples
  // drive increased accuracy in less important terms --> CV scores improve for
  // dense solutions.  To control this effect, we retain the shape of the
  // profile but enforce an upper bound on one of the levels.
  scale_profile(cardinality, factor, new_N_l); // bound = cardinality * factor

  for (lev=0; lev<num_lev; ++lev)
    delta_N_l[lev] = one_sided_delta(N_l[lev], new_N_l[lev]);
}


void NonDMultilevelPolynomialChaos::
scale_profile(const SizetArray& cardinality, Real factor, RealVector& new_N_l)
{
  size_t lev, num_lev = cardinality.size();
  Real curr_factor, max_curr_factor = 0., factor_ratio;
  for (lev=0; lev<num_lev; ++lev) {
    curr_factor = new_N_l[lev] / cardinality[lev];
    if (curr_factor > max_curr_factor)
      max_curr_factor = curr_factor;
  }
  factor_ratio = factor / max_curr_factor;
  if (factor_ratio < 1.) // exceeds upper bound -> scale back
    for (lev=0; lev<num_lev; ++lev)
      new_N_l[lev] *= factor_ratio;
}


void NonDMultilevelPolynomialChaos::
print_results(std::ostream& s, short results_state)
{
  switch (results_state) {
  case REFINEMENT_RESULTS:  case INTERMEDIATE_RESULTS:
    if (outputLevel == DEBUG_OUTPUT)   print_coefficients(s);
    break;
  case FINAL_RESULTS:
    if (outputLevel >= NORMAL_OUTPUT)  print_coefficients(s);
    if (!expansionExportFile.empty())  export_coefficients();
    if (!NLev.empty()) {
      s << "<<<<< Samples per solution level:\n";
      print_multilevel_evaluation_summary(s, NLev);
      s << "<<<<< Equivalent number of high fidelity evaluations: "
	<< equivHFEvals << std::endl;
    }
    break;
  }

  // skip over NonDPolynomialChaos::print_results()
  NonDExpansion::print_results(s, results_state);
}

} // namespace Dakota
