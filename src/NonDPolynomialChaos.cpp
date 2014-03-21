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
#include "SharedPecosApproxData.hpp"
#include "PecosApproximation.hpp"
#include "SparseGridDriver.hpp"
#include "TensorProductDriver.hpp"
#include "CubatureDriver.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDPolynomialChaos::
NonDPolynomialChaos(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model),
  expansionExportFile(
    probDescDB.get_string("method.nond.export_expansion_file")),
  expansionImportFile(
    probDescDB.get_string("method.nond.import_expansion_file")),
  collocRatio(probDescDB.get_real("method.nond.collocation_ratio")),
  tensorRegression(probDescDB.get_bool("method.nond.tensor_grid")),
  crossValidation(probDescDB.get_bool("method.nond.cross_validation")),
  noiseTols(probDescDB.get_rv("method.nond.regression_noise_tolerance")),
  l2Penalty(probDescDB.get_real("method.nond.regression_penalty")),
  expOrderSeqSpec(probDescDB.get_usa("method.nond.expansion_order")),
  dimPrefSpec(probDescDB.get_rv("method.nond.dimension_preference")),
  collocPtsSeqSpec(probDescDB.get_sza("method.nond.collocation_points")),
  expSamplesSeqSpec(probDescDB.get_sza("method.nond.expansion_samples")),
  sequenceIndex(0),
  normalizedCoeffOutput(probDescDB.get_bool("method.nond.normalized"))
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
  bool regression_flag = false;
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
      if (!expSamplesSeqSpec.empty()) { // expectation
	if (refineType) { // no obvious logic for sample refinement
	  Cerr << "Error: uniform/adaptive refinement of expansion_samples not "
	       << "supported." << std::endl;
	  abort_handler(-1);
	}
	numSamplesOnModel = (sequenceIndex < expSamplesSeqSpec.size()) ?
	  expSamplesSeqSpec[sequenceIndex] : expSamplesSeqSpec.back();
	expansionCoeffsApproach = Pecos::SAMPLING;
	// assign a default expansionBasisType, if unspecified
	if (!expansionBasisType) expansionBasisType = Pecos::TOTAL_ORDER_BASIS;

	// reuse type/seed/rng settings intended for the expansion_sampler.
	// Unlike expansion_sampler, allow sampling pattern to vary under
	// unstructured grid refinement/replacement/augmentation.  Also unlike
	// expansion_sampler, we use an ACTIVE sampler mode for estimating the
	// coefficients over all active variables.
	if (numSamplesOnModel) {
	  // default pattern is fixed for consistency in any outer loop,
	  // but gets overridden in cases of unstructured grid refinement.
	  bool vary_pattern = false;
	  construct_lhs(u_space_sampler, g_u_model,
	    probDescDB.get_ushort("method.sample_type"), numSamplesOnModel,
	    probDescDB.get_int("method.random_seed"),
	    probDescDB.get_string("method.random_number_generator"),
	    vary_pattern, ACTIVE);
	}
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
	  // for sub-sampled tensor grid, seems desirable to use tensor exp
	  // TO DO: only for CS candidate? or true basis for Least sq as well?
	  // TO DO: consider a dimensionality limit for TP expansion
	  if (!expansionBasisType)
	    expansionBasisType = (tensorRegression) ?
	      Pecos::TENSOR_PRODUCT_BASIS : Pecos::TOTAL_ORDER_BASIS;
	  size_t exp_terms;
	  switch (expansionBasisType) {
	  case Pecos::TOTAL_ORDER_BASIS: case Pecos::ADAPTED_BASIS:
	    exp_terms =
	      Pecos::SharedPolyApproxData::total_order_terms(exp_order);
	    break;
	  case Pecos::TENSOR_PRODUCT_BASIS:
	    exp_terms =
	      Pecos::SharedPolyApproxData::tensor_product_terms(exp_order);
	    break;
	  }
	  termsOrder
	    = probDescDB.get_real("method.nond.collocation_ratio_terms_order");
	  if (!collocPtsSeqSpec.empty()) // define collocRatio from colloc pts
	    collocRatio = terms_samples_to_ratio(exp_terms, numSamplesOnModel,
						 termsOrder);
	  else if (collocRatio > 0.)     // define colloc pts from collocRatio
	    numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio,
						       termsOrder);
	}

	if (numSamplesOnModel) {
	  if (tensorRegression) {// structured grid: uniform sub-sampling of TPQ
	    UShortArray dim_quad_order;
	    if (expansionCoeffsApproach == Pecos::ORTHOG_LEAST_INTERPOLATION) {
	      dim_quad_order
		= probDescDB.get_usa("method.nond.tensor_grid_order");
	      Pecos::inflate_scalar(dim_quad_order, numContinuousVars);
	    }
	    else {
	      // define nominal quadrature order as exp_order + 1
	      // (m > p avoids most of the 0's in the Psi measurement matrix)
	      // Note: misses nested exp order increment (increment_order())
	      dim_quad_order.resize(numContinuousVars);
	      for (size_t i=0; i<numContinuousVars; ++i)
		dim_quad_order[i] = exp_order[i] + 1;
	    }
	  
	    // define order sequence for input to NonDQuadrature
	    UShortArray quad_order_seq(1); // one level of refinement
	    // convert aniso vector to scalar + dim_pref.  If iso, dim_pref is
	    // empty; if aniso, it differs from exp_order aniso due to offset.
	    RealVector dim_pref;
	    NonDIntegration::anisotropic_order_to_dimension_preference(
	      dim_quad_order, quad_order_seq[0], dim_pref);
	    // use alternate NonDQuad ctor to filter (deprecated) or sub-sample
	    // quadrature points (uSpaceModel.build_approximation() invokes
	    // daceIterator.run()).  The quad order inputs are updated within
	    // NonDQuadrature as needed to satisfy min order constraints (but
	    // not nested constraints: nestedRules is false to retain m >= p+1).
	    construct_quadrature(u_space_sampler, g_u_model, numSamplesOnModel,
				 probDescDB.get_int("method.random_seed"),
				 quad_order_seq, dim_pref);
	  }
	  else { // unstructured grid: LHS samples
	    // if reusing samples within a refinement strategy, ensure different
	    // random numbers are generated for points within the grid (even if
	    // the number of samples differs)
	    bool vary_pattern = (refineType && !pt_reuse.empty());
	    // reuse type/seed/rng settings intended for the expansion_sampler.
	    // Unlike expansion_sampler, allow sampling pattern to vary under
	    // unstructured grid refinement/replacement/augmentation.  Also
	    // unlike expansion_sampler, we use an ACTIVE sampler mode for
	    // forming the PCE over all active variables.
	    construct_lhs(u_space_sampler, g_u_model,
	      probDescDB.get_ushort("method.sample_type"), numSamplesOnModel,
	      probDescDB.get_int("method.random_seed"),
	      probDescDB.get_string("method.random_number_generator"),
	      vary_pattern, ACTIVE);
	  }
	  // TO DO:
	  //if (probDescDB.get_ushort("method.nond.expansion_sample_type") ==
	  //    SUBMETHOD_INCREMENTAL_LHS))
	  //  construct_incremental_lhs();
	}
      }

      // maxEvalConcurrency updated here for expansion samples and regression
      // and in initialize_u_space_model() for sparse/quad/cub
      if (numSamplesOnModel) // optional with default = 0
	maxEvalConcurrency *= numSamplesOnModel;
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
  short corr_order = -1, corr_type = NO_CORRECTION;
  //const Variables& g_u_vars = g_u_model.current_variables();
  uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
    //g_u_vars.view(), g_u_vars.variables_components(),
    //g_u_model.current_response().active_set(),
    approx_type, exp_order, corr_type, corr_order, data_order, outputLevel,
    pt_reuse, probDescDB.get_string("method.export_points_file"),
    probDescDB.get_bool("method.export_points_file_annotated"), import_pts_file,
    probDescDB.get_bool("method.import_points_file_annotated")), false);
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler();

  // uSpaceModel concurrency is defined by the number of samples used
  // in evaluating the PC expansion
  uSpaceModel.init_communicators(
    numSamplesOnExpansion*uSpaceModel.derivative_concurrency());

  if (probDescDB.parallel_library().command_line_check())
    Cout << "\nPolynomial_chaos construction completed: initial grid size of "
	 << numSamplesOnModel << " evaluations to be performed." << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly. */
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		    unsigned short num_int_level, short u_space_type,
		    bool piecewise_basis, bool use_derivs):
  NonDExpansion(POLYNOMIAL_CHAOS, model, exp_coeffs_approach, u_space_type,
		piecewise_basis, use_derivs), 
  crossValidation(false), l2Penalty(0.), normalizedCoeffOutput(false)
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
    if (iteratedModel.gradient_type()  != "none") data_order |= 2;
    //if (iteratedModel.hessian_type() != "none") data_order |= 4; // not yet
    if (data_order == 1)
      Cerr << "\nWarning: use_derivatives option in polynomial_chaos "
	   << "requires a response\n         gradient specification.  "
	   << "Option will be ignored.\n" << std::endl;
  }
  useDerivs = (data_order > 1); // override input specification
}


void NonDPolynomialChaos::initialize_u_space_model()
{
  // Commonly used approx settings (e.g., order, outputLevel, useDerivs) are
  // passed through the DataFitSurrModel ctor chain.  Additional data needed
  // by OrthogPolyApproximation are passed using Pecos::BasisConfigOptions.
  // Note: passing useDerivs again is redundant with the DataFitSurrModel ctor.
  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  Pecos::BasisConfigOptions bc_options(nestedRules, false, true, useDerivs);
  shared_data_rep->configuration_options(bc_options);

  // For PCE, the approximation and integration bases are the same.  We (always)
  // construct it for the former and (conditionally) pass it in to the latter.
  shared_data_rep->construct_basis(natafTransform.u_types(),
    iteratedModel.aleatory_distribution_parameters());
  if ( expansionCoeffsApproach == Pecos::QUADRATURE ||
       expansionCoeffsApproach == Pecos::CUBATURE   ||
       expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
       ( tensorRegression && numSamplesOnModel ) ) {
    NonDIntegration* u_space_sampler_rep = 
      (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
    u_space_sampler_rep->initialize_grid(shared_data_rep->polynomial_basis());
  }

  // NumerGenOrthogPolynomial instances need to compute polyCoeffs and
  // orthogPolyNormsSq in addition to gaussPoints and gaussWeights
  shared_data_rep->coefficients_norms_flag(true);

  // Transfer regression data: cross validation, noise tol, and L2 penalty.
  // Note: regression solver type is transferred via expansionCoeffsApproach
  //       in NonDExpansion::initialize_u_space_model()
  if (expansionCoeffsApproach >= Pecos::DEFAULT_REGRESSION) {
    shared_data_rep->cross_validation(crossValidation);
    if (!noiseTols.empty())
      shared_data_rep->noise_tolerance(noiseTols);
    if (expansionCoeffsApproach == Pecos::LASSO_REGRESSION)
      shared_data_rep->l2_penalty(l2Penalty);
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
    if (subIteratorFlag || !finalStatistics.function_gradients().empty()) {
      // fatal for import, but warning for export
      Cerr << "\nError: PCE coefficient import not supported in advanced modes"
	   << std::endl;
      abort_handler(-1);
    }

    // import the PCE coefficients for all QoI and a shared multi-index.
    // Annotation provides questionable value in this context & is off for now.
    RealVectorArray coeffs_array(numFunctions); UShort2DArray multi_index;
    String context("polynomial chaos expansion import file");
    TabularIO::read_data_tabular(expansionImportFile, context, coeffs_array,
				 multi_index, false, numContinuousVars,
				 numFunctions);

    // post the shared data
    SharedPecosApproxData* data_rep
      = (SharedPecosApproxData*)uSpaceModel.shared_approximation().data_rep();
    data_rep->allocate(multi_index); // defines multiIndex, sobolIndexMap

    // post coefficients to the OrthogPolyApproximation instances (also calls
    // OrthogPolyApproximation::allocate_arrays())
    uSpaceModel.approximation_coefficients(coeffs_array);
  }
}


void NonDPolynomialChaos::increment_specification_sequence()
{
  bool update_exp = false, update_sampler = false, update_from_ratio = false;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: case Pecos::COMBINED_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: case Pecos::CUBATURE:
    // update grid order/level, if multiple values were provided
    NonDExpansion::increment_specification_sequence(); break;
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
      numSamplesOnModel
	= terms_ratio_to_samples(exp_terms, collocRatio, termsOrder);
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
  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  shared_data_rep->increment_order();
  const UShortArray& exp_order = shared_data_rep->expansion_order();
  size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
    Pecos::SharedPolyApproxData::tensor_product_terms(exp_order) :
    Pecos::SharedPolyApproxData::total_order_terms(exp_order);

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
  bool export_pce = false;
  RealVectorArray coeffs_array;
  if (!expansionExportFile.empty()) {
    if (subIteratorFlag || !finalStatistics.function_gradients().empty())
      Cerr << "\nWarning: PCE coefficient export not supported in advanced "
	   << "modes" << std::endl;
    else
      { export_pce = true; coeffs_array.resize(numFunctions); }
  }

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();

  size_t i, width = write_precision+7,
    num_cuv = numContAleatUncVars+numContEpistUncVars;
  char tag[10];
  int j; // for sprintf %i

  s << std::scientific << std::setprecision(write_precision) << "-----------"
    << "------------------------------------------------------------------\n";
  for (i=0; i<numFunctions; i++) {
    if (normalizedCoeffOutput) s << "Normalized coefficients of ";
    else                       s << "Coefficients of ";
    s << "Polynomial Chaos Expansion for " << fn_labels[i] << ":\n";
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
    poly_approxs[i].print_coefficients(s, normalizedCoeffOutput);
    if (export_pce) {
      // poly_approxs[i].approximation_coefficients() may be sparse; need
      // dense coeffs consistent with shared multi-index for tabular export
      PecosApproximation* approx_rep
	= (PecosApproximation*)poly_approxs[i].approx_rep();
      // default returns a vector view; sparse returns a copy
      coeffs_array[i] = approx_rep->dense_coefficients();
    }
  }

  if (export_pce) {
    // export the PCE coefficients for all QoI and a shared multi-index.
    // Annotation provides questionable value in this context & is off for now.
    SharedPecosApproxData* data_rep
      = (SharedPecosApproxData*)uSpaceModel.shared_approximation().data_rep();
    String context("polynomial chaos expansion export file");
    TabularIO::write_data_tabular(expansionExportFile, context, coeffs_array,
				  data_rep->multi_index());//, false);
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
