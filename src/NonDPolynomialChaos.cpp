/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPolynomialChaos
//- Description: Implementation code for NonDPolynomialChaos class
//- Owner:       Mike Eldred, Sandia National Laboratories

#include "dakota_system_defs.hpp"
#include "NonDPolynomialChaos.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSampling.hpp"
#include "PecosApproximation.hpp"
#include "SparseGridDriver.hpp"
#include "TensorProductDriver.hpp"
#include "CubatureDriver.hpp"
#include "dakota_data_io.hpp"


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDPolynomialChaos::NonDPolynomialChaos(Model& model): NonDExpansion(model),
  expansionImportFile(
    probDescDB.get_string("method.nond.expansion_import_file")),
  collocRatio(probDescDB.get_real("method.nond.collocation_ratio")),
  tensorRegression(probDescDB.get_bool("method.nond.tensor_grid")),
  crossValidation(probDescDB.get_bool("method.nond.cross_validation")),
  noiseTols(probDescDB.get_rv("method.nond.regression_noise_tolerance")),
  l2Penalty(probDescDB.get_real("method.nond.regression_penalty")),
  expOrderSeqSpec(probDescDB.get_usa("method.nond.expansion_order")),
  dimPrefSpec(probDescDB.get_rv("method.nond.dimension_preference")),
  collocPtsSeqSpec(probDescDB.get_sza("method.nond.collocation_points")),
  expSamplesSeqSpec(probDescDB.get_sza("method.nond.expansion_samples")),
  sequenceIndex(0)
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order,
      u_space_type = probDescDB.get_short("method.nond.expansion_type");
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // --------------------
  // Data import settings
  // --------------------
  String pt_reuse = probDescDB.get_string("method.nond.point_reuse"),
    import_pts_file = probDescDB.get_string("method.import_points_file");
  if (!import_pts_file.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  // For data import, global bounds needed for DataFitSurrModel::inside()
  bool global_bnds
    = ( numContDesVars || numContEpistUncVars || numContStateVars ||
	( !import_pts_file.empty() && pt_reuse == "region" ) );
  transform_model(iteratedModel, g_u_model, global_bnds);

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  String approx_type;
  bool import_annotated = false, regression_flag = false;
  // expansion_order defined for expansion_samples/regression
  UShortArray exp_order;
  if (!expOrderSeqSpec.empty()) {
    unsigned short scalar = (sequenceIndex < expOrderSeqSpec.size()) ?
      expOrderSeqSpec[sequenceIndex] : expOrderSeqSpec.back();
    NonDIntegration::dimension_preference_to_anisotropic_order(scalar,
      dimPrefSpec, numContinuousVars, exp_order);
  }
  if (expansionImportFile.empty()) {
    const UShortArray& quad_order_seq_spec
      = probDescDB.get_usa("method.nond.quadrature_order");
    const UShortArray& ssg_level_seq_spec
      = probDescDB.get_usa("method.nond.sparse_grid_level");
    unsigned short cub_int_spec
      = probDescDB.get_ushort("method.nond.cubature_integrand");
    if (!quad_order_seq_spec.empty()) {
      expansionCoeffsApproach = Pecos::QUADRATURE;
      construct_quadrature(u_space_sampler, g_u_model, quad_order_seq_spec,
	dimPrefSpec);
    }
    else if (!ssg_level_seq_spec.empty()) {
      expansionCoeffsApproach = Pecos::COMBINED_SPARSE_GRID;
      construct_sparse_grid(u_space_sampler, g_u_model, ssg_level_seq_spec,
	dimPrefSpec);
    }
    else if (cub_int_spec != USHRT_MAX) {
      expansionCoeffsApproach = Pecos::CUBATURE;
      construct_cubature(u_space_sampler, g_u_model, cub_int_spec);
    }
    else { // expansion_samples or collocation_{points,ratio}
      // default pattern is static for consistency in any outer loop,
      // but gets overridden below for unstructured grid refinement.
      bool vary_pattern = false;
      if (!expSamplesSeqSpec.empty()) { // expectation
	if (refineType) { // no obvious logic for sample refinement
	  Cerr << "Error: uniform/adaptive refinement of expansion_samples not "
	       << "supported." << std::endl;
	  abort_handler(-1);
	}
	numSamplesOnModel = (sequenceIndex < expSamplesSeqSpec.size()) ?
	  expSamplesSeqSpec[sequenceIndex] : expSamplesSeqSpec.back();
	expansionCoeffsApproach = Pecos::SAMPLING;

	// reuse type/seed/rng settings intended for the expansion_sampler.
	// Unlike expansion_sampler, allow sampling pattern to vary under
	// unstructured grid refinement/replacement/augmentation.  Also unlike
	// expansion_sampler, we use an ACTIVE sampler mode for estimating the
	// coefficients over all active variables.
	if (numSamplesOnModel)
	  construct_lhs(u_space_sampler, g_u_model,
	    probDescDB.get_string("method.sample_type"), numSamplesOnModel,
	    probDescDB.get_int("method.random_seed"),
	    probDescDB.get_string("method.random_number_generator"),
	    vary_pattern, ACTIVE);

	if (!import_pts_file.empty())
	  import_annotated
	    = probDescDB.get_bool("method.import_points_file_annotated");
      }
      else { // regression
	if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
	  // adaptive precluded since grid anisotropy not readily supported
	  // for synchronization with expansion anisotropy.
	  Cerr << "Error: only uniform refinement is supported for PCE "
	       << "regression." << std::endl;
	  abort_handler(-1);
	}

	regression_flag = true;
	expansionCoeffsApproach
	  = probDescDB.get_short("method.nond.regression_type");
	if (expansionCoeffsApproach == Pecos::DEFAULT_LEAST_SQ_REGRESSION) {
	  switch(probDescDB.get_short(
		 "method.nond.least_squares_regression_type")) {
	  case SVD_LS:
	    expansionCoeffsApproach = Pecos::SVD_LEAST_SQ_REGRESSION;    break;
	  case EQ_CON_LS:
	    expansionCoeffsApproach = Pecos::EQ_CON_LEAST_SQ_REGRESSION; break;
	  // else leave as DEFAULT_LEAST_SQ_REGRESSION
	  }
	}

	if (!collocPtsSeqSpec.empty())
	  numSamplesOnModel = (sequenceIndex < collocPtsSeqSpec.size()) ?
	    collocPtsSeqSpec[sequenceIndex] : collocPtsSeqSpec.back();
	if (expansionCoeffsApproach != Pecos::ORTHOG_LEAST_INTERPOLATION ) {
	  size_t exp_terms
	    = Pecos::PolynomialApproximation::total_order_terms(exp_order);
	  termsOrder
	    = probDescDB.get_real("method.nond.collocation_ratio_terms_order");
	  if (!collocPtsSeqSpec.empty()) // define collocRatio from colloc pts
	    collocRatio = terms_samples_to_ratio(exp_terms, numSamplesOnModel,
						 termsOrder);
	  else if (collocRatio > 0.)     // define colloc pts from collocRatio
	    numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio,
						       termsOrder);
	}

	if (tensorRegression) { // structured grid: uniform sub-sampling of TPQ
	  UShortArray dim_quad_order;
	  if ( expansionCoeffsApproach == Pecos::ORTHOG_LEAST_INTERPOLATION ) {
	    dim_quad_order
	      = probDescDB.get_usa("method.nond.tensor_grid_order");
	    Pecos::inflate_scalar(dim_quad_order, numContinuousVars);
	  }
	  else {
	    // define nominal quadrature order as exp_order + 1
	    // (m > p avoids most of the 0's in the Psi measurement matrix)
	    dim_quad_order.resize(numContinuousVars);
	    for (size_t i=0; i<numContinuousVars; ++i)//misses nested increment
	      dim_quad_order[i] = exp_order[i] + 1;
	  }
	  
	  // define order sequence for input to NonDQuadrature
	  UShortArray quad_order_seq(1); // one level of refinement
	  // convert aniso vector to scalar + dim_pref.  If isotropic, dim_pref
	  // is empty; if aniso, it differs from exp_order aniso due to offset.
	  RealVector dim_pref;
	  NonDIntegration::anisotropic_order_to_dimension_preference(
	    dim_quad_order, quad_order_seq[0], dim_pref);
	  // use alternate NonDQuad ctor to filter or sample TPQ points
	  // (NonDExpansion invokes uSpaceModel.build_approximation()
	  // which invokes daceIterator.run_iterator()).  The quad order inputs
	  // are updated within NonDQuadrature as needed to satisfy min order
	  // constraints (but not nested constraints: nestedRules is false).
	  construct_quadrature(u_space_sampler, g_u_model, numSamplesOnModel,
			       probDescDB.get_int("method.random_seed"),
			       quad_order_seq, dim_pref);
	  // don't allow data import (currently permissible in input spec)
	  pt_reuse.clear(); import_pts_file.clear();
	}
	else { // unstructured grid: LHS samples
	  // if reusing samples within a refinement strategy, ensure different
	  // random numbers are generated for points within the grid (even if
	  // the number of samples differs)
	  vary_pattern = (refineType && !pt_reuse.empty());
	  // reuse type/seed/rng settings intended for the expansion_sampler.
	  // Unlike expansion_sampler, allow sampling pattern to vary under
	  // unstructured grid refinement/replacement/augmentation.  Also
	  // unlike expansion_sampler, we use an ACTIVE sampler mode for
	  // forming the PCE over all active variables.
	  if (numSamplesOnModel)
	    construct_lhs(u_space_sampler, g_u_model,
	      probDescDB.get_string("method.sample_type"), numSamplesOnModel,
	      probDescDB.get_int("method.random_seed"),
	      probDescDB.get_string("method.random_number_generator"),
	      vary_pattern, ACTIVE);

	  if (!import_pts_file.empty())
	    import_annotated
	      = probDescDB.get_bool("method.import_points_file_annotated");
	}
	// TO DO:
	//if (probDescDB.get_string("method.nond.expansion_sample_type")
	//    == "incremental_lhs"))
	//  construct_incremental_lhs();
      }

      // maxConcurrency updated here for expansion samples and regression
      // and in initialize_u_space_model() for sparse/quad/cub
      if (numSamplesOnModel) // optional with default = 0
	maxConcurrency *= numSamplesOnModel;
    }

    if (regression_flag)
      approx_type =
	//(piecewiseBasis) ? "piecewise_regression_orthogonal_polynomial" :
	"global_regression_orthogonal_polynomial";
    else
      approx_type =
	//(piecewiseBasis) ? "piecewise_projection_orthogonal_polynomial" :
	"global_projection_orthogonal_polynomial";
  }
  else // PCE import: regression/projection facilities not needed
    approx_type = //(piecewiseBasis) ? "piecewise_orthogonal_polynomial" :
      "global_orthogonal_polynomial";

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, exp_order, corr_type, corr_order, data_order, outputLevel,
    pt_reuse, probDescDB.get_string("method.export_points_file"),
    probDescDB.get_bool("method.export_points_file_annotated"),
    import_pts_file, import_annotated), false);
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler();

  // uSpaceModel concurrency is defined by the number of samples used
  // in evaluating the PC expansion
  uSpaceModel.init_communicators(
    numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


/** This constructor is used for helper iterator instantiation on the fly. */
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		    unsigned short num_int_level, short u_space_type,
		    bool piecewise_basis, bool use_derivs):
  NonDExpansion(model, exp_coeffs_approach, u_space_type,
		piecewise_basis, use_derivs), 
  crossValidation(false), l2Penalty(0.)
{
  // ----------------------------------------------
  // Resolve settings and initialize natafTransform
  // ----------------------------------------------
  short data_order;
  resolve_inputs(u_space_type, data_order);
  initialize(u_space_type);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  bool global_bnds
    = (numContDesVars || numContEpistUncVars || numContStateVars);
  transform_model(iteratedModel, g_u_model, global_bnds);

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  if (expansionCoeffsApproach == Pecos::QUADRATURE) {
    RealVector  dim_pref;                     // empty -> isotropic
    UShortArray quad_order(1, num_int_level); // single sequence
    construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref);
  }
  else if (expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID) {
    RealVector  dim_pref;                    // empty -> isotropic
    UShortArray ssg_level(1, num_int_level); // single sequence
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level, dim_pref);
  }
  else if (expansionCoeffsApproach == Pecos::CUBATURE)
    construct_cubature(u_space_sampler, g_u_model, num_int_level);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short  corr_order = -1, corr_type = NO_CORRECTION;
  String pt_reuse, approx_type =
    //(piecewiseBasis) ? "piecewise_projection_orthogonal_polynomial" :
    "global_projection_orthogonal_polynomial";
  UShortArray exp_order; // empty for numerical integration approaches
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    approx_type, exp_order, corr_type, corr_order, data_order, outputLevel,
    pt_reuse), false);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDPolynomialChaos::~NonDPolynomialChaos()
{
  if (numSamplesOnExpansion)
    uSpaceModel.free_communicators(
      numSamplesOnExpansion*uSpaceModel.derivative_concurrency());
}


void NonDPolynomialChaos::
resolve_inputs(short& u_space_type, short& data_order)
{
  NonDExpansion::resolve_inputs(u_space_type, data_order);

  // There are two derivative cases of interest: (1) derivatives used as
  // additional data for forming the approximation (derivatives w.r.t. the
  // expansion variables), and (2) derivatives that will be approximated 
  // separately (derivatives w.r.t. auxilliary variables).  The data_order
  // passed through the DataFitSurrModel defines Approximation::buildDataOrder,
  // which is restricted to managing the former case.  If we need to manage the
  // latter case in the future, we do not have a good way to detect this state
  // at construct time, as neither the finalStats ASV/DVV nor subIteratorFlag
  // have yet been defined.  One indicator that is defined is the presence of
  // inactive continuous vars, since the subModel inactive view is updated
  // within the NestedModel ctor prior to subIterator instantiation.
  data_order = 1;
  if (useDerivs) { // input specification
    if (gradientType  != "none") data_order |= 2;
    //if (hessianType != "none") data_order |= 4; // not yet supported
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in polynomial_chaos "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
  }
  useDerivs = (data_order > 1); // override input specification
}


void NonDPolynomialChaos::initialize_u_space_model()
{
  // The expansion_order specification is passed through the DataFitSurrModel
  // ctor.  Any additional specification data needed by OrthogPolyApproximation
  // must be passed through by diving through the hierarchy.
  bool num_int = (expansionCoeffsApproach == Pecos::QUADRATURE ||
		  expansionCoeffsApproach == Pecos::CUBATURE   ||
		  expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const Pecos::ShortArray& u_types = natafTransform.u_types();

  PecosApproximation *poly_approx_rep, *poly_approx_rep0;
  NonDIntegration* u_space_sampler_rep = (num_int || tensorRegression) ?
    (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep() : NULL;

  // Rather than automatically constructing the orthogonal polynomial
  // basis within each OrthogPolyApproximation ctor invocation, manage
  // it here so that we may reuse bases among the response function
  // set and an integration driver (if present).  This efficiency
  // becomes more important for numerically-generated polynomials.
  bool first = true;
  Pecos::BasisConfigOptions bc_options(nestedRules, false, true, useDerivs);
  for (size_t i=0; i<numFunctions; i++) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep) { // may be NULL based on approxFnIndices
      if (first) {
	poly_approx_rep->construct_basis(u_types,
	  iteratedModel.aleatory_distribution_parameters(), bc_options);
	if (num_int || tensorRegression)
	  u_space_sampler_rep->
	    initialize_grid(poly_approx_rep->polynomial_basis());
	poly_approx_rep0 = poly_approx_rep;
	first = false;
      }
      else {           // reuse rep0 basis for all other reps
        poly_approx_rep->basis_types(poly_approx_rep0->basis_types());
	poly_approx_rep->polynomial_basis(poly_approx_rep0->polynomial_basis());
      }
      // NumerGenOrthogPolynomial instances need to compute polyCoeffs and
      // orthogPolyNormsSq in addition to gaussPoints and gaussWeights
      poly_approx_rep->coefficients_norms_flag(true);
      // Transfer regression data: cross validation, noise tol, and L2 penalty.
      // Note: regression solver type is transferred via expansionCoeffsApproach
      //       in NonDExpansion::initialize_u_space_model()
      if (expansionCoeffsApproach >= Pecos::DEFAULT_REGRESSION) {
	poly_approx_rep->cross_validation(crossValidation);
	if (!noiseTols.empty())
	  poly_approx_rep->noise_tolerance(noiseTols);
	if (expansionCoeffsApproach == Pecos::LASSO_REGRESSION)
	  poly_approx_rep->l2_penalty(l2Penalty);
      }
    }
  }

  // perform last due to numSamplesOnModel update
  NonDExpansion::initialize_u_space_model();
}


void NonDPolynomialChaos::compute_expansion()
{
  if (expansionImportFile.empty())
    // ------------------------------
    // Calculate the PCE coefficients
    // ------------------------------
    NonDExpansion::compute_expansion(); // default implementation
  else {
    // ---------------------------
    // Import the PCE coefficients
    // ---------------------------
    std::ifstream import_stream(expansionImportFile.c_str());
    if (!import_stream) {
      Cerr << "\nError: cannot open polynomial chaos expansion import file "
	   << expansionImportFile << std::endl;
      abort_handler(-1);
    }
    if (subIteratorFlag || !finalStatistics.function_gradients().empty()) {
      Cerr << "\nError: PCE coefficient import not supported in advanced modes"
	   << std::endl;
      abort_handler(-1);
    }
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    RealVectorArray chaos_coeffs(numFunctions);
    PecosApproximation* poly_approx_rep;
    for (size_t i=0; i<numFunctions; i++) {
      poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
      if (poly_approx_rep) { // may be NULL based on approxFnIndices
	poly_approx_rep->allocate_arrays();
	chaos_coeffs[i].sizeUninitialized(poly_approx_rep->expansion_terms());
      }
    }
    read_data(import_stream, chaos_coeffs);
    uSpaceModel.approximation_coefficients(chaos_coeffs);
  }
}


void NonDPolynomialChaos::increment_specification_sequence()
{
  bool update_exp = false, update_sampler = false, update_ratio = false;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: case Pecos::COMBINED_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: case Pecos::CUBATURE:
    // update grid order/level, if multiple values were provided
    NonDExpansion::increment_specification_sequence(); break;
  case Pecos::SAMPLING:
    // advance expansionOrder and/or expansionSamples, as admissible
    if (sequenceIndex+1 <   expOrderSeqSpec.size()) update_exp     = true;
    if (sequenceIndex+1 < expSamplesSeqSpec.size()) update_sampler = true;
    if (update_exp || update_sampler) ++sequenceIndex;
    if (update_sampler) numSamplesOnModel = expSamplesSeqSpec[sequenceIndex];
    break;
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    // advance collocationPoints
    if (sequenceIndex+1 < collocPtsSeqSpec.size()) {
      update_sampler = true; ++sequenceIndex;
      numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
    }
    break;
  default: // regression
    // advance expansionOrder and/or collocationPoints, as admissible
    if (sequenceIndex+1 <  expOrderSeqSpec.size())   update_exp     = true;
    if (sequenceIndex+1 < collocPtsSeqSpec.size())   update_sampler = true;
    else if (collocPtsSeqSpec.empty() && update_exp) update_ratio   = true;
    if (update_exp || update_sampler) ++sequenceIndex;
    if (update_sampler) numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
    if (collocPtsSeqSpec.empty() && update_exp) // (fixed) collocRatio case
      update_sampler = true;
    break;
  }

  // define exp_order for local use
  UShortArray exp_order;
  if (update_exp)
    NonDIntegration::dimension_preference_to_anisotropic_order(
      expOrderSeqSpec[sequenceIndex], dimPrefSpec, numContinuousVars,
      exp_order);
  else if (update_sampler && tensorRegression) {
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    PecosApproximation* poly_approx_rep;
    for (size_t i=0; i<numFunctions; i++) {
      poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
      if (poly_approx_rep) // may be NULL based on approxFnIndices
	{ exp_order = poly_approx_rep->expansion_order(); break; }
    }
  }

  // update expansion order within Pecos::OrthogPolyApproximation
  if (update_exp) {
    std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
    PecosApproximation* poly_approx_rep;
    for (size_t i=0; i<numFunctions; i++) {
      poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
      if (poly_approx_rep) { // may be NULL based on approxFnIndices
	poly_approx_rep->update_order(exp_order);
	// could have OrthogPolyApproximation::update_order() update
	// numExpansionTerms and then add logic like this:
	//if (update_ratio) exp_terms = poly_approx_rep->expansion_terms();
      }
    }
  }

  // update numSamplesOnModel from collocRatio
  if (update_ratio) {
    numSamplesOnModel = terms_ratio_to_samples(
      Pecos::PolynomialApproximation::total_order_terms(exp_order),
      collocRatio, termsOrder);
    update_sampler = true;
  }

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
      NonDSampling* nond_sampling
	= (NonDSampling*)uSpaceModel.subordinate_iterator().iterator_rep();
      nond_sampling->sampling_reference(0); // no lower bound
      DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
      dfs_model->total_points(numSamplesOnModel);
    }
  }
}


/** Used for uniform refinement of regression-based PCE. */
void NonDPolynomialChaos::increment_order()
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep; size_t exp_terms = 0;
  for (size_t i=0; i<numFunctions; i++) {
    poly_approx_rep = (PecosApproximation*)poly_approxs[i].approx_rep();
    if (poly_approx_rep) { // may be NULL based on approxFnIndices
      poly_approx_rep->increment_order();
      exp_terms = std::max(poly_approx_rep->expansion_terms(), exp_terms);
    }
  }

  // update numSamplesOnModel based on existing collocation ratio and
  // updated number of expansion terms
  numSamplesOnModel
    = terms_ratio_to_samples(exp_terms, collocRatio, termsOrder);

  // update u-space sampler to use new sample count
  if (tensorRegression) {
    NonDQuadrature* nond_quad
      = (NonDQuadrature*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_quad->samples(numSamplesOnModel);
    if (nond_quad->mode() == RANDOM_TENSOR)
      nond_quad->increment_grid(); // increment dimension quad order
    nond_quad->update();
  }
  else { // enforce increment through sampling_reset()
    NonDSampling* nond_sampling
      = (NonDSampling*)uSpaceModel.subordinate_iterator().iterator_rep();
    nond_sampling->sampling_reference(0); // no lower bound
    DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
    dfs_model->total_points(numSamplesOnModel);
  }
}


void NonDPolynomialChaos::print_coefficients(std::ostream& s)
{
  s << std::scientific << std::setprecision(write_precision);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();

  size_t i, width = write_precision+7,
    num_cuv = numContAleatUncVars+numContEpistUncVars;
  char tag[10];
  int j; // for sprintf %i

  s << "-------------------------------------------------------------------\n";
  for (i=0; i<numFunctions; i++) {
    s << "Polynomial Chaos coefficients for " << fn_labels[i] << ":\n";
    // header
    s << "  " << std::setw(width) << "coefficient";
    for (j=0; j<numContDesVars; j++)
      { std::sprintf(tag, "d%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<num_cuv; j++)
      { std::sprintf(tag, "u%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<numContStateVars; j++)
      { std::sprintf(tag, "s%i", j+1); s << std::setw(5) << tag; }
    s << "\n  " << std::setw(width) << "-----------";
    for (j=0; j<numContinuousVars; j++)
      s << " ----";
    poly_approxs[i].print_coefficients(s);
  }
}


// BMA: demonstrate crude output of coefficients to get user feedback.
void NonDPolynomialChaos::archive_coefficients()
{
  if (!resultsDB.active())
    return;

  // TODO: variable labels
  MetaDataType md;
  md["Array Spans"] = make_metadatavalue("Response Functions");
  resultsDB.array_allocate<RealVector>
    (run_identifier(), resultsNames.pce_coeffs, numFunctions, md);
  resultsDB.array_allocate<std::vector<std::string> >
    (run_identifier(), resultsNames.pce_coeff_labels, numFunctions, md);

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();

  for (size_t i=0; i<numFunctions; i++) {
 
    const RealVector& coeffs = poly_approxs[i].approximation_coefficients();
    resultsDB.array_insert<RealVector>
      (run_identifier(), resultsNames.pce_coeffs, i, coeffs);

    std::vector<std::string> coeff_labels;
    poly_approxs[i].coefficient_labels(coeff_labels);
    resultsDB.array_insert<std::vector<std::string> >
      (run_identifier(), resultsNames.pce_coeff_labels, i, coeff_labels);

  }
}


} // namespace Dakota
