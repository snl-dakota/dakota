/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
#include "nested_sampling.hpp"


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
  randomSeed(probDescDB.get_int("method.random_seed")),
  tensorRegression(probDescDB.get_bool("method.nond.tensor_grid")),
  crossValidation(probDescDB.get_bool("method.nond.cross_validation")),
  crossValidNoiseOnly(
    probDescDB.get_bool("method.nond.cross_validation.noise_only")),
  noiseTols(probDescDB.get_rv("method.nond.regression_noise_tolerance")),
  l2Penalty(probDescDB.get_real("method.nond.regression_penalty")),
//initSGLevel(probDescDB.get_ushort("method.nond.adapted_basis.initial_level")),
  numAdvance(probDescDB.get_ushort("method.nond.adapted_basis.advancements")),
  expOrderSpec(probDescDB.get_ushort("method.nond.expansion_order")),
  dimPrefSpec(probDescDB.get_rv("method.nond.dimension_preference")),
  collocPtsSpec(probDescDB.get_sizet("method.nond.collocation_points")),
  expSamplesSpec(probDescDB.get_sizet("method.nond.expansion_samples")),
  normalizedCoeffOutput(probDescDB.get_bool("method.nond.normalized")),
  uSpaceType(probDescDB.get_short("method.nond.expansion_type")),
  quadOrderSpec(probDescDB.get_ushort("method.nond.quadrature_order")),
  ssgLevelSpec(probDescDB.get_ushort("method.nond.sparse_grid_level")),
  cubIntSpec(probDescDB.get_ushort("method.nond.cubature_integrand")),
  importBuildPointsFile(
    probDescDB.get_string("method.import_build_points_file")),
  importBuildFormat(probDescDB.get_ushort("method.import_build_format")),
  importBuildActiveOnly(probDescDB.get_bool("method.import_build_active_only")),
  importApproxPointsFile(
    probDescDB.get_string("method.import_approx_points_file")),
  importApproxFormat(probDescDB.get_ushort("method.import_approx_format")),
  importApproxActiveOnly(
    probDescDB.get_bool("method.import_approx_active_only"))
  //resizedFlag(false), callResize(false)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

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
  unsigned short sample_type = probDescDB.get_ushort("method.sample_type");
  const String& rng = probDescDB.get_string("method.random_number_generator");
  short regress_type = probDescDB.get_short("method.nond.regression_type"),
    ls_regress_type
      = probDescDB.get_short("method.nond.least_squares_regression_type");
  Real colloc_ratio_order
    = probDescDB.get_real("method.nond.collocation_ratio_terms_order");
  const UShortArray& tensor_grid_order
    = probDescDB.get_usa("method.nond.tensor_grid_order");
  Iterator u_space_sampler;
  String approx_type;

  UShortArray exp_orders; // defined for expansion_samples/regression
  config_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);

  if (!expansionImportFile.empty()) // PCE import: no regression/projection
    approx_type = //(piecewiseBasis) ? "piecewise_orthogonal_polynomial" :
      "global_orthogonal_polynomial";
  else if (!config_integration(quadOrderSpec, ssgLevelSpec, cubIntSpec,
			       u_space_sampler, g_u_model, approx_type) &&
	   !config_expectation(expSamplesSpec, sample_type, rng,
			       u_space_sampler, g_u_model, approx_type) &&
	   !config_regression(exp_orders, collocPtsSpec, colloc_ratio_order,
			      regress_type, ls_regress_type, tensor_grid_order,
			      sample_type, rng, pt_reuse, u_space_sampler,
			      g_u_model, approx_type)) {
    Cerr << "Error: incomplete configuration in NonDPolynomialChaos "
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
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		    unsigned short num_int, const RealVector& dim_pref,
		    short u_space_type, bool piecewise_basis, bool use_derivs):
  NonDExpansion(POLYNOMIAL_CHAOS, model, exp_coeffs_approach, piecewise_basis,
		use_derivs), 
  randomSeed(0), crossValidation(false), crossValidNoiseOnly(false),
  l2Penalty(0.), numAdvance(3), dimPrefSpec(dim_pref),
  normalizedCoeffOutput(false), uSpaceType(u_space_type)
  //resizedFlag(false), callResize(false), initSGLevel(0)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

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
  Iterator u_space_sampler; String approx_type;
  switch (exp_coeffs_approach) {
  case Pecos::QUADRATURE:
    config_integration(num_int, USHRT_MAX, USHRT_MAX, u_space_sampler,
		       g_u_model, approx_type); break;
  case Pecos::COMBINED_SPARSE_GRID:
    config_integration(USHRT_MAX, num_int, USHRT_MAX, u_space_sampler,
		       g_u_model, approx_type); break;
  case Pecos::CUBATURE:
    config_integration(USHRT_MAX, USHRT_MAX, num_int, u_space_sampler,
		       g_u_model, approx_type); break;
  default:
    Cerr << "Error: Unsupported PCE coefficient estimation approach in "
	 << "NonDPolynomialChaos constructor."
	 << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

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
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		    unsigned short exp_order, const RealVector& dim_pref,
		    size_t colloc_pts, Real colloc_ratio,
		    int seed, short u_space_type, bool piecewise_basis,
		    bool use_derivs, bool cv_flag,
		    const String& import_build_pts_file,
		    unsigned short import_build_format,
		    bool import_build_active_only):
  NonDExpansion(POLYNOMIAL_CHAOS, model, exp_coeffs_approach, piecewise_basis,
		use_derivs), 
  collocRatio(colloc_ratio), termsOrder(1.), randomSeed(seed),
  tensorRegression(false), crossValidation(cv_flag), crossValidNoiseOnly(false),
  l2Penalty(0.), numAdvance(3), expOrderSpec(exp_order), dimPrefSpec(dim_pref),
  collocPtsSpec(colloc_pts), normalizedCoeffOutput(false),
  uSpaceType(u_space_type) //resizedFlag(false), callResize(false)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

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
  UShortArray exp_orders; // defined for expansion_samples/regression
  config_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);

  Iterator u_space_sampler;
  UShortArray tensor_grid_order; // for OLI + tensorRegression (not supported)
  String approx_type, rng("mt19937"), pt_reuse;
  config_regression(exp_orders, collocPtsSpec, 1, exp_coeffs_approach,
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


/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDPolynomialChaos::
NonDPolynomialChaos(BaseConstructor, ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model),
  expansionExportFile(
    probDescDB.get_string("method.nond.export_expansion_file")),
  expansionImportFile(
    probDescDB.get_string("method.nond.import_expansion_file")),
  collocRatio(probDescDB.get_real("method.nond.collocation_ratio")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  tensorRegression(probDescDB.get_bool("method.nond.tensor_grid")),
  crossValidation(probDescDB.get_bool("method.nond.cross_validation")),
  crossValidNoiseOnly(
    probDescDB.get_bool("method.nond.cross_validation.noise_only")),
  noiseTols(probDescDB.get_rv("method.nond.regression_noise_tolerance")),
  l2Penalty(probDescDB.get_real("method.nond.regression_penalty")),
//initSGLevel(probDescDB.get_ushort("method.nond.adapted_basis.initial_level")),
  numAdvance(probDescDB.get_ushort("method.nond.adapted_basis.advancements")),
  dimPrefSpec(probDescDB.get_rv("method.nond.dimension_preference")),
  normalizedCoeffOutput(probDescDB.get_bool("method.nond.normalized")),
  uSpaceType(probDescDB.get_short("method.nond.expansion_type")),
  cubIntSpec(probDescDB.get_ushort("method.nond.cubature_integrand")),
  importBuildPointsFile(
    probDescDB.get_string("method.import_build_points_file")),
  importBuildFormat(probDescDB.get_ushort("method.import_build_format")),
  importBuildActiveOnly(probDescDB.get_bool("method.import_build_active_only")),
  importApproxPointsFile(
    probDescDB.get_string("method.import_approx_points_file")),
  importApproxFormat(probDescDB.get_ushort("method.import_approx_format")),
  importApproxActiveOnly(
    probDescDB.get_bool("method.import_approx_active_only"))
  //resizedFlag(false), callResize(false)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

  // Rest is in derived class...
}


NonDPolynomialChaos::
NonDPolynomialChaos(unsigned short method_name, Model& model,
		    short exp_coeffs_approach, const RealVector& dim_pref,
		    short u_space_type, bool piecewise_basis, bool use_derivs):
  NonDExpansion(method_name, model, exp_coeffs_approach, piecewise_basis,
		use_derivs), 
  randomSeed(0), crossValidation(false), crossValidNoiseOnly(false),
  l2Penalty(0.), numAdvance(3), dimPrefSpec(dim_pref),
  normalizedCoeffOutput(false), uSpaceType(u_space_type)
  //resizedFlag(false), callResize(false), initSGLevel(0)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

  // Rest is in derived class...
}


NonDPolynomialChaos::
NonDPolynomialChaos(unsigned short method_name, Model& model,
		    short exp_coeffs_approach, const RealVector& dim_pref,
		    short u_space_type, bool piecewise_basis, bool use_derivs,
		    Real colloc_ratio, int seed, bool cv_flag):
  NonDExpansion(method_name, model, exp_coeffs_approach, piecewise_basis,
		use_derivs), 
  collocRatio(colloc_ratio), termsOrder(1.), randomSeed(seed),
  tensorRegression(false), crossValidation(cv_flag), crossValidNoiseOnly(false),
  l2Penalty(0.), numAdvance(3), dimPrefSpec(dim_pref),
  normalizedCoeffOutput(false), uSpaceType(u_space_type)
  //resizedFlag(false), callResize(false)
{
  // -------------------
  // input sanity checks
  // -------------------
  check_dimension_preference(dimPrefSpec);

  // Rest is in derived class...
}


NonDPolynomialChaos::~NonDPolynomialChaos()
{ }


void NonDPolynomialChaos::
config_expansion_orders(unsigned short exp_order, const RealVector& dim_pref,
			UShortArray& exp_orders)
{
  // expansion_order defined for expansion_samples/regression
  if (exp_order != USHRT_MAX)
    NonDIntegration::dimension_preference_to_anisotropic_order(exp_order,
      dim_pref, numContinuousVars, exp_orders);
}


bool NonDPolynomialChaos::
config_integration(unsigned short quad_order, unsigned short ssg_level,
		   unsigned short cub_int, Iterator& u_space_sampler,
		   Model& g_u_model, String& approx_type)
{
  bool num_int = true;
  if (quad_order != USHRT_MAX) {
    expansionCoeffsApproach = Pecos::QUADRATURE;
    construct_quadrature(u_space_sampler, g_u_model, quad_order, dimPrefSpec);
  }
  else if (ssg_level != USHRT_MAX) {
    expansionCoeffsApproach = Pecos::COMBINED_SPARSE_GRID;
    construct_sparse_grid(u_space_sampler, g_u_model, ssg_level, dimPrefSpec);
  }
  else if (cub_int != USHRT_MAX) {
    expansionCoeffsApproach = Pecos::CUBATURE;
    construct_cubature(u_space_sampler, g_u_model, cub_int);
  }
  else num_int = false;

  if (num_int)
    approx_type =
      //(piecewiseBasis) ? "piecewise_projection_orthogonal_polynomial" :
      "global_projection_orthogonal_polynomial";
  return num_int;
}


bool NonDPolynomialChaos::
config_expectation(size_t exp_samples, unsigned short sample_type,
		   const String& rng, Iterator& u_space_sampler,
		   Model& g_u_model,  String& approx_type)
{
  if (exp_samples == std::numeric_limits<size_t>::max())
    return false;

  // expectation of PCE coefficients based on random sampling

  if (refineType) { // no obvious logic for sample refinement
    Cerr << "Error: uniform/adaptive refinement of expansion_samples not "
	 << "supported." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  numSamplesOnModel = exp_samples;
  expansionCoeffsApproach = Pecos::SAMPLING;
  // assign a default expansionBasisType, if unspecified
  if (!expansionBasisType)
    expansionBasisType = Pecos::TOTAL_ORDER_BASIS;

  // reuse type/seed/rng settings intended for the expansion_sampler.
  // Unlike expansion_sampler, allow sampling pattern to vary under
  // unstructured grid refinement/replacement/augmentation.  Also unlike
  // expansion_sampler, we use an ACTIVE sampler mode for estimating the
  // coefficients over all active variables.
  if (numSamplesOnModel) {
    // default pattern is fixed for consistency in any outer loop,
    // but gets overridden in cases of unstructured grid refinement.
    bool vary_pattern = false;
    construct_lhs(u_space_sampler, g_u_model, sample_type, numSamplesOnModel,
		  randomSeed, rng, vary_pattern, ACTIVE);

    // maxEvalConcurrency updated here for expansion samples and regression
    // and in initialize_u_space_model() for sparse/quad/cub
    maxEvalConcurrency *= numSamplesOnModel;
  }

  approx_type =
    //(piecewiseBasis) ? "piecewise_projection_orthogonal_polynomial" :
    "global_projection_orthogonal_polynomial";
  return true;
}


bool NonDPolynomialChaos::
config_regression(const UShortArray& exp_orders, size_t colloc_pts,
		  Real colloc_ratio_terms_order, short regress_type,
		  short ls_regress_type, const UShortArray& tensor_grid_order,
		  unsigned short sample_type, const String& rng,
		  const String& pt_reuse, Iterator& u_space_sampler,
		  Model& g_u_model, String& approx_type)
{
  if (refineType && refineControl > Pecos::UNIFORM_CONTROL) {
    // adaptive precluded since grid anisotropy not readily supported
    // for synchronization with expansion anisotropy.
    Cerr << "Error: only uniform refinement is supported for PCE "
	 << "regression." << std::endl;
    abort_handler(METHOD_ERROR);
    return false;
  }
  if (exp_orders.empty() && regress_type != Pecos::ORTHOG_LEAST_INTERPOLATION) {
    Cerr << "Warning: unsupported regression configuration in "
	 << "NonDPolynomialChaos::config_regression()." << std::endl;
    return false;
  }

  if (regress_type == Pecos::DEFAULT_LEAST_SQ_REGRESSION)
    switch (ls_regress_type) {
    case SVD_LS:
      expansionCoeffsApproach = Pecos::SVD_LEAST_SQ_REGRESSION;    break;
    case EQ_CON_LS:
      expansionCoeffsApproach = Pecos::EQ_CON_LEAST_SQ_REGRESSION; break;
    default: // else leave as DEFAULT_LEAST_SQ_REGRESSION
      expansionCoeffsApproach = regress_type; break;
    }
  else
    expansionCoeffsApproach = regress_type;

  size_t SZ_MAX = std::numeric_limits<size_t>::max();
  switch (expansionCoeffsApproach) {
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    if (colloc_pts == SZ_MAX) {
      Cerr << "Error: OLI requires collocation_points specification."
	   << std::endl;
      abort_handler(METHOD_ERROR);
      return false;
    }
    else
      numSamplesOnModel = colloc_pts;
    break;
  case Pecos::DEFAULT_REGRESSION:      case Pecos::DEFAULT_LEAST_SQ_REGRESSION:
  case Pecos::SVD_LEAST_SQ_REGRESSION: case Pecos::EQ_CON_LEAST_SQ_REGRESSION:
  case Pecos::BASIS_PURSUIT:           case Pecos::BASIS_PURSUIT_DENOISING:
  case Pecos::ORTHOG_MATCH_PURSUIT:    case Pecos::LASSO_REGRESSION:
  case Pecos::LEAST_ANGLE_REGRESSION: {
    // for sub-sampled tensor grid, seems desirable to use tensor exp,
    // but enforce an arbitrary dimensionality limit of 5.
    // TO DO: only for CS candidate? or true basis for Least sq as well?
    if (!expansionBasisType)
      expansionBasisType = (tensorRegression && numContinuousVars <= 5) ?
	Pecos::TENSOR_PRODUCT_BASIS : Pecos::TOTAL_ORDER_BASIS;
    size_t exp_terms;
    switch (expansionBasisType) {
    case Pecos::TOTAL_ORDER_BASIS: case Pecos::ADAPTED_BASIS_GENERALIZED:
    case Pecos::ADAPTED_BASIS_EXPANDING_FRONT:
      exp_terms = Pecos::SharedPolyApproxData::total_order_terms(exp_orders);
      break;
    case Pecos::TENSOR_PRODUCT_BASIS:
      exp_terms
	= Pecos::SharedPolyApproxData::tensor_product_terms(exp_orders);
      break;
    }
    termsOrder = colloc_ratio_terms_order;
    if (colloc_pts != SZ_MAX) { // define collocRatio from colloc pts
      numSamplesOnModel = colloc_pts;
      collocRatio = terms_samples_to_ratio(exp_terms, numSamplesOnModel);
    }
    else if (collocRatio > 0.)     // define colloc pts from collocRatio
      numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);
    break;
  }
  default:
    Cerr << "Warning: unsupported regression type in NonDPolynomialChaos::"
	 << "config_regression()." << std::endl;
    return false;
  }

  if (numSamplesOnModel) {
    if (tensorRegression) {// structured grid: uniform sub-sampling of TPQ
      UShortArray dim_quad_order;
      if (expansionCoeffsApproach == Pecos::ORTHOG_LEAST_INTERPOLATION) {
	dim_quad_order = tensor_grid_order;
	Pecos::inflate_scalar(dim_quad_order, numContinuousVars);
      }
      else {
	// define nominal quadrature order as exp_order + 1
	// (m > p avoids most of the 0's in the Psi measurement matrix)
	// Note 1: eo+1 neglects nested quad order increment but this is
	//   enforced by initialize_u_space_model() ->
	//   NonDQuadrature::initialize_grid(),reset()
	// Note 2: nominal order provides lower bound that gets updated
	//   if insufficient sample size in initialize_u_space_model() ->
	//   NonDQuadrature::initialize_grid(),update(),sampling_reset()
	dim_quad_order.resize(numContinuousVars);
	for (size_t i=0; i<numContinuousVars; ++i)
	  dim_quad_order[i] = exp_orders[i] + 1;
      }

      // convert aniso vector to scalar + dim_pref.  If iso, dim_pref is
      // empty; if aniso, it differs from exp_order aniso due to offset.
      unsigned short quad_order; RealVector dim_pref;
      NonDIntegration::anisotropic_order_to_dimension_preference(dim_quad_order,
	quad_order, dim_pref);
      // use alternate NonDQuad ctor to filter (deprecated) or sub-sample
      // quadrature points (uSpaceModel.build_approximation() invokes
      // daceIterator.run()).  The quad order inputs are updated within
      // NonDQuadrature as needed to satisfy min order constraints (but
      // not nested constraints: nestedRules is false to retain m >= p+1).
      construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref,
			   numSamplesOnModel, randomSeed);
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
      construct_lhs(u_space_sampler, g_u_model, sample_type, numSamplesOnModel,
		    randomSeed, rng, vary_pattern, ACTIVE);
    }
    // TO DO:
    // BMA NOTE: If this code is activated, need to instead use LHS, with
    // refinement samples
    //if (expansion_sample_type == SUBMETHOD_INCREMENTAL_LHS))
    //  construct_incremental_lhs();

    // maxEvalConcurrency updated here for expansion samples and regression
    // and in initialize_u_space_model() for sparse/quad/cub
    maxEvalConcurrency *= numSamplesOnModel;
  }

  approx_type =
    //(piecewiseBasis) ? "piecewise_regression_orthogonal_polynomial" :
    "global_regression_orthogonal_polynomial";

  return true;
}


bool NonDPolynomialChaos::resize()
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
  initialize_random(uSpaceType);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  transform_model(iteratedModel, g_u_model); // retain distribution bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  Iterator u_space_sampler;
  UShortArray exp_orders; // empty for numerical integration approaches
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    construct_quadrature(u_space_sampler, g_u_model, quadOrderSpec,
			 dimPrefSpec);
    break;
  case Pecos::COMBINED_SPARSE_GRID:
    construct_sparse_grid(u_space_sampler, g_u_model, ssgLevelSpec,
			  dimPrefSpec);
    break;
  case Pecos::CUBATURE:
    construct_cubature(u_space_sampler, g_u_model, cubIntSpec);
    break;
  default:
    if (expansionCoeffsApproach == Pecos::ORTHOG_LEAST_INTERPOLATION ||
	expOrderSpec == USHRT_MAX) {
      // extract number of collocation points
      numSamplesOnModel = collocPtsSpec;
      // Construct u_space_sampler
      String rng("mt19937");
      construct_lhs(u_space_sampler, g_u_model, SUBMETHOD_LHS,
		    numSamplesOnModel, randomSeed, rng, false, ACTIVE);
    }
    else { // expansion_order-based
      config_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);
      // resolve expansionBasisType, exp_terms, numSamplesOnModel
      if (!expansionBasisType)
	expansionBasisType = (tensorRegression && numContinuousVars <= 5) ?
	  Pecos::TENSOR_PRODUCT_BASIS : Pecos::TOTAL_ORDER_BASIS;

      size_t exp_terms;
      switch (expansionBasisType) {
      case Pecos::TOTAL_ORDER_BASIS:
      case Pecos::ADAPTED_BASIS_GENERALIZED:
      case Pecos::ADAPTED_BASIS_EXPANDING_FRONT:
	exp_terms = Pecos::SharedPolyApproxData::total_order_terms(exp_orders);
	break;
      case Pecos::TENSOR_PRODUCT_BASIS:
	exp_terms
	  = Pecos::SharedPolyApproxData::tensor_product_terms(exp_orders);
	break;
      }
    
      if (collocPtsSpec != std::numeric_limits<size_t>::max()) // ratio from pts
	collocRatio = terms_samples_to_ratio(exp_terms, numSamplesOnModel);
      else if (collocRatio > 0.)     // define colloc pts from collocRatio
	numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);

      // Construct u_space_sampler
      if (tensorRegression) // tensor sub-sampling
	// define nominal quadrature order as exp_order + 1
	// (m > p avoids most of the 0's in the Psi measurement matrix)
	construct_quadrature(u_space_sampler, g_u_model, expOrderSpec+1,
			     dimPrefSpec);
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
  String pt_reuse, approx_type;
  ActiveSet pce_set = g_u_model.current_response().active_set(); // copy
  pce_set.request_values(7);
  if (expansionCoeffsApproach == Pecos::QUADRATURE ||
      expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
      expansionCoeffsApproach == Pecos::CUBATURE) {
    approx_type = "global_projection_orthogonal_polynomial";
    uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
      pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
      outputLevel, pt_reuse), false);
  }
  else {
    approx_type = "global_regression_orthogonal_polynomial";
    if (!importBuildPointsFile.empty())
      pt_reuse = "all";
    uSpaceModel.assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
      pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
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


void NonDPolynomialChaos::derived_init_communicators(ParLevLIter pl_iter)
{
  // this is redundant with Model recursions except for PCE coeff import case
  if (!expansionImportFile.empty())
    iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  NonDExpansion::derived_init_communicators(pl_iter);
}


void NonDPolynomialChaos::derived_set_communicators(ParLevLIter pl_iter)
{
  // this is redundant with Model recursions except for PCE coeff import case
  if (!expansionImportFile.empty())
    iteratedModel.set_communicators(pl_iter, maxEvalConcurrency);

  NonDExpansion::derived_set_communicators(pl_iter);
}


void NonDPolynomialChaos::derived_free_communicators(ParLevLIter pl_iter)
{
  NonDExpansion::derived_free_communicators(pl_iter);

  // this is redundant with Model recursions except for PCE coeff import case
  if (!expansionImportFile.empty())
    iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
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

  // If the model is not yet fully initialized, skip grid initialization.
  if ( expansionCoeffsApproach == Pecos::QUADRATURE ||
       expansionCoeffsApproach == Pecos::CUBATURE   ||
       expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
       ( tensorRegression && numSamplesOnModel ) ) {
    if (iteratedModel.resize_pending()) // defer grid initialization
      { /* callResize = true; */ }
    else {
      NonDIntegration* u_space_sampler_rep = 
        (NonDIntegration*)uSpaceModel.subordinate_iterator().iterator_rep();
      u_space_sampler_rep->initialize_grid(shared_data_rep->polynomial_basis());
    }
  }

  // NumerGenOrthogPolynomial instances need to compute polyCoeffs and
  // orthogPolyNormsSq in addition to gaussPoints and gaussWeights
  shared_data_rep->coefficients_norms_flag(true);

  // Transfer regression data: cross validation, noise tol, and L2 penalty.
  // Note: regression solver type is transferred via expansionCoeffsApproach
  //       in NonDExpansion::initialize_u_space_model()
  if (expansionCoeffsApproach >= Pecos::DEFAULT_REGRESSION) {
    // TO DO: consider adding support for machine-generated seeds (when no
    // user spec) as well as seed progressions for varyPattern.  Coordinate
    // with JDJ on whether Dakota or CV should own these features.
    Pecos::RegressionConfigOptions
      rc_options(crossValidation, crossValidNoiseOnly, randomSeed, noiseTols,
		 l2Penalty, false, 0/*initSGLevel*/, 2, numAdvance);
    shared_data_rep->configuration_options(rc_options);

    // updates for automatic order adaptation
    //...
  }

  // perform last due to numSamplesOnModel update
  NonDExpansion::initialize_u_space_model();
}


void NonDPolynomialChaos::compute_expansion(size_t index)
{
  if (expansionImportFile.empty())
    // ------------------------------
    // Calculate the PCE coefficients
    // ------------------------------
    NonDExpansion::compute_expansion(index); // default implementation
  else {
    // ---------------------------
    // Import the PCE coefficients
    // ---------------------------
    if (subIteratorFlag || !finalStatistics.function_gradients().empty()) {
      // fatal for import, but warning for export
      Cerr << "\nError: PCE coefficient import not supported in advanced modes"
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }

    // import the PCE coefficients for all QoI and a shared multi-index.
    // Annotation provides questionable value in this context & is off for now.
    RealVectorArray coeffs_array(numFunctions); UShort2DArray multi_index;
    String context("polynomial chaos expansion import file");
    unsigned short tabular_format = TABULAR_NONE;
    TabularIO::read_data_tabular(expansionImportFile, context, coeffs_array,
				 multi_index, tabular_format, numContinuousVars,
				 numFunctions);

    // post the shared data
    SharedPecosApproxData* data_rep
      = (SharedPecosApproxData*)uSpaceModel.shared_approximation().data_rep();
    data_rep->allocate(multi_index); // defines multiIndex, sobolIndexMap

    // post coefficients to the OrthogPolyApproximation instances (also calls
    // OrthogPolyApproximation::allocate_arrays())
    uSpaceModel.approximation_coefficients(coeffs_array, normalizedCoeffOutput);
  }
}


void NonDPolynomialChaos::
select_refinement_points(const RealVectorArray& candidate_samples,
			 unsigned short batch_size, 
			 RealMatrix& best_samples)
{
  // from initial candidate_samples, select the best batch_size points in terms
  // of information content, as determined by pivoted LU factorization
  int new_size = numSamplesOnModel + batch_size;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Select refinement points: new_size = " << new_size << "\n";

  // This computation does not utilize any QoI information, so aggregation
  // across the QoI vector is not necessary.
  // TO DO: utilize static fn instead of 0th poly_approx; this would also
  // facilitate usage from other surrogate types (especially GP).
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  SharedApproxData&          shared_approx = uSpaceModel.shared_approximation();
  PecosApproximation* poly_approx_rep
    = (PecosApproximation*)poly_approxs[0].approx_rep();
  SharedPecosApproxData* shared_data_rep
    = (SharedPecosApproxData*)shared_approx.data_rep();

  const Pecos::SurrogateData& surr_data = poly_approx_rep->surrogate_data();
  int num_surr_data_pts = surr_data.points();
  RealMatrix current_samples( numContinuousVars, num_surr_data_pts, false );
  for (int j=0; j<num_surr_data_pts; ++j) 
    for (int i=0; i<numContinuousVars; ++i) 
      current_samples(i,j)=surr_data.continuous_variables(j)[i];

  LejaSampler sampler;
  sampler.set_seed(randomSeed);
  sampler.set_precondition(true);
  //sampler.set_precondition(false);
  std::vector<Pecos::BasisPolynomial>& poly_basis
    = shared_data_rep->polynomial_basis();
  sampler.set_polynomial_basis( poly_basis );
  sampler.set_total_degree_basis_from_num_samples(numContinuousVars, new_size);
  RealMatrix candidate_samples_matrix;
  Pecos::convert( candidate_samples, candidate_samples_matrix );

  // Remove any candidate samples already in the initial sample set
  RealMatrix unique_candidate_samples;
  sampler.get_unique_samples( current_samples, (int)batch_size, 
			      candidate_samples_matrix,
			      unique_candidate_samples );
  IntVector selected_candidate_indices;
  sampler.get_enriched_sample_indices( (int)numContinuousVars, 
				       current_samples, (int)batch_size, 
				       unique_candidate_samples, 
				       selected_candidate_indices );
  best_samples.shapeUninitialized( (int)numContinuousVars, (int)batch_size );
  Pecos::extract_submatrix_from_column_indices( unique_candidate_samples,
						selected_candidate_indices,
						best_samples );
  
  if (outputLevel >= DEBUG_OUTPUT) {
    // write samples to output
    Cout << "Select refinement pts: best_samples =\n";
    write_data(Cout, best_samples);

    // write samples to file
    std::ofstream export_file_stream;
    std::string filename = "bayesian-adaptive-emulator-samples-";
    filename+=static_cast<std::ostringstream*>(&(std::ostringstream()<<
						 best_samples.numCols()+
						 num_surr_data_pts))->str();
    filename += ".txt";
    TabularIO::open_file(export_file_stream, filename,
			 "adaptive emulator samples");
    bool brackets = false, row_rtn = true, final_rtn = true;
    Dakota::write_data(export_file_stream, best_samples,
		       brackets, row_rtn, final_rtn);
    export_file_stream.close();
  }
}


void NonDPolynomialChaos::
select_refinement_points_deprecated(const RealVectorArray& candidate_samples,
				    unsigned short batch_size,
				    RealMatrix& best_samples)
{
  // from initial candidate_samples, select the best batch_size points in terms
  // of information content, as determined by pivoted LU factorization

  // define a total-order basis of sufficient size P >= current pts + batch_size
  // (not current + chain size) and build A using basis at each of the total pts
  int new_size = numSamplesOnModel + batch_size;
  UShortArray exp_order(numContinuousVars, 0); UShort2DArray multi_index;
  ratio_samples_to_order(1./*collocRatio*/, new_size, exp_order, false);
  Pecos::SharedPolyApproxData::total_order_multi_index(exp_order, multi_index);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Select refinement points: new_size = " << new_size
	 << " exp_order =\n" << exp_order;//<< "multi_index =\n" << multi_index;

  // candidate MCMC points aggregated across all restart cycles
  // > one option is to pre-filter the full batch and use pivoted cholesky on
  //   a smaller set of highest post prob's
  // > to start, throw the whole aggregated set at it
  RealMatrix A, L_factor, U_factor;

  // This computation does not utilize any QoI information, so aggregation
  // across the QoI vector is not necessary.
  // TO DO: utilize static fn instead of 0th poly_approx; this would also
  // facilitate usage from other surrogate types (especially GP).
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  PecosApproximation* poly_approx_rep
    = (PecosApproximation*)poly_approxs[0].approx_rep();

  // reference A built from surrData and reference multiIndex
  poly_approx_rep->build_linear_system(A, multi_index);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Select refinement pts: reference A = " << A.numRows() << " by "
	 << A.numCols() << ".\n";
  // add MCMC chain (all_samples): A size = num current+num chain by P,
  // with current pts as 1st rows 
  poly_approx_rep->augment_linear_system(candidate_samples, A, multi_index);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Select refinement pts: augmented A = " << A.numRows() << " by "
	 << A.numCols() << ".\n";

  IntVector pivots;
  Pecos::truncated_pivoted_lu_factorization( A, L_factor, U_factor, pivots,
					     new_size, numSamplesOnModel);
  if (outputLevel >= DEBUG_OUTPUT)
    { Cout << "Select refinement pts: pivots =\n"; write_data(Cout, pivots); }

  // On return, pivots is size new_size and contains indices of rows of A.
  // Entries i=numSamplesOnModel to i<new_size identify points to select to
  // refine emulator.
  //pivots_to_all_samples();
  best_samples.shapeUninitialized(numContinuousVars, batch_size);
  int b, t, j; Real *b_col;
  for (b=0, t=numSamplesOnModel; b<batch_size; ++b, ++t) {
    b_col = best_samples[b]; 
    const RealVector& selected_rv
      = candidate_samples[pivots[t] - numSamplesOnModel];
    for (j=0; j<numContinuousVars; ++j)
      b_col[j] = selected_rv[j];
  }
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Select refinement pts: best_samples =\n";
    write_data(Cout, best_samples);
  }
}


/** Used for uniform refinement of regression-based PCE. */
void NonDPolynomialChaos::increment_order_and_grid()
{
  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  shared_data_rep->increment_order();
  increment_grid_from_order();
}


/** Used for uniform refinement of regression-based PCE. */
void NonDPolynomialChaos::increment_grid_from_order()
{
  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();
  const UShortArray& exp_order = shared_data_rep->expansion_order();
  size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
    Pecos::SharedPolyApproxData::tensor_product_terms(exp_order) :
    Pecos::SharedPolyApproxData::total_order_terms(exp_order);

  // update numSamplesOnModel based on existing collocation ratio and
  // updated number of expansion terms
  numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);

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
    // no lower bound on samples in the subiterator
    uSpaceModel.subordinate_iterator().sampling_reference(0);
    DataFitSurrModel* dfs_model = (DataFitSurrModel*)uSpaceModel.model_rep();
    dfs_model->total_points(numSamplesOnModel);
  }
}


/** Used for uniform refinement of regression-based PCE. */
void NonDPolynomialChaos::increment_order_from_grid()
{
  SharedPecosApproxData* shared_data_rep = (SharedPecosApproxData*)
    uSpaceModel.shared_approximation().data_rep();

  // update expansion order based on existing collocation ratio and
  // updated number of truth model samples

  // copy
  UShortArray exp_order = shared_data_rep->expansion_order();
  // increment
  ratio_samples_to_order(collocRatio, numSamplesOnModel, exp_order, true);
  // restore
  shared_data_rep->expansion_order(exp_order);
}


void NonDPolynomialChaos::
ratio_samples_to_order(Real colloc_ratio, int num_samples,
		       UShortArray& exp_order, bool less_than_or_equal)
{
  if (exp_order.empty()) // ramp from order 0; else ramp from starting point
    exp_order.assign(numContinuousVars, 0);

  // ramp expansion order to synchronize with num_samples and colloc_ratio

  size_t i, incr = 0, data_size = (useDerivs) ?
    num_samples * (numContinuousVars + 1) : num_samples;
  size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
    Pecos::SharedPolyApproxData::tensor_product_terms(exp_order) :
    Pecos::SharedPolyApproxData::total_order_terms(exp_order);
  // data_reqd = colloc_ratio * exp_terms^termsOrder
  size_t data_reqd = (size_t)std::floor(std::pow((Real)exp_terms, termsOrder) *
					colloc_ratio + .5);
  while (data_reqd < data_size) {
    // uniform order increment
    for (i=0; i<numContinuousVars; ++i)
      ++exp_order[i];
    // terms in total order expansion
    exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
      Pecos::SharedPolyApproxData::tensor_product_terms(exp_order) :
      Pecos::SharedPolyApproxData::total_order_terms(exp_order);
    data_reqd = (size_t)std::floor(std::pow((Real)exp_terms, termsOrder) *
				   colloc_ratio + .5);
    ++incr;
  }
  if (less_than_or_equal && incr && data_reqd > data_size) // 1 too many
    for (i=0; i<numContinuousVars; ++i)
      --exp_order[i];
}


void NonDPolynomialChaos::print_results(std::ostream& s, short results_state)
{
  if (outputLevel >= NORMAL_OUTPUT)
    print_coefficients(s);
  if (!expansionExportFile.empty() && results_state == FINAL_RESULTS)
    export_coefficients();

  NonDExpansion::print_results(s, results_state);
}


void NonDPolynomialChaos::print_coefficients(std::ostream& s)
{
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
  }
}


void NonDPolynomialChaos::export_coefficients()
{
  if (subIteratorFlag || !finalStatistics.function_gradients().empty()) {
    Cerr << "\nWarning: PCE coefficient export not supported in advanced "
	 << "modes" << std::endl;
    return;
  }

  RealVectorArray coeffs_array(numFunctions);
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  for (size_t i=0; i<numFunctions; i++)
    coeffs_array[i] // default returns a vector view; sparse returns a copy
      = poly_approxs[i].approximation_coefficients(normalizedCoeffOutput);

  // export the PCE coefficients for all QoI and a shared multi-index.
  // Annotation provides questionable value in this context & is off for now.
  SharedPecosApproxData* data_rep
  = (SharedPecosApproxData*)uSpaceModel.shared_approximation().data_rep();
  String context("polynomial chaos expansion export file");
  TabularIO::write_data_tabular(expansionExportFile, context, coeffs_array,
				data_rep->multi_index());
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
 
    // default returns a vector view; sparse returns a copy
    RealVector coeffs
      = poly_approxs[i].approximation_coefficients(normalizedCoeffOutput);

    resultsDB.array_insert<RealVector>
      (run_identifier(), resultsNames.pce_coeffs, i, coeffs);

    std::vector<std::string> coeff_labels;
    poly_approxs[i].coefficient_labels(coeff_labels);
    resultsDB.array_insert<std::vector<std::string> >
      (run_identifier(), resultsNames.pce_coeff_labels, i, coeff_labels);

  }
}

} // namespace Dakota
