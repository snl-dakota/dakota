/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDPolynomialChaos
//- Description: Implementation code for NonDPolynomialChaos class
//- Owner:       Mike Eldred, Sandia National Laboratories

#include "dakota_system_defs.hpp"
#include "NonDPolynomialChaos.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "NonDQuadrature.hpp"
#include "NonDSampling.hpp"
#include "NonDAdaptImpSampling.hpp"
#include "SharedPecosApproxData.hpp"
#include "PecosApproximation.hpp"
#include "SparseGridDriver.hpp"
#include "TensorProductDriver.hpp"
#include "CubatureDriver.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "pecos_math_util.hpp"
#include "nested_sampling.hpp"
#include "math_tools.hpp"

namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator
    instantiation using the ProblemDescDB. */
NonDPolynomialChaos::
NonDPolynomialChaos(ProblemDescDB& problem_db, Model& model):
  NonDExpansion(problem_db, model),
  crossValidation(problem_db.get_bool("method.nond.cross_validation")),
  crossValidNoiseOnly(
    problem_db.get_bool("method.nond.cross_validation.noise_only")),
  maxCVOrderCandidates(
    problem_db.get_ushort("method.nond.cross_validation.max_order_candidates")),
  respScaling(problem_db.get_bool("method.nond.response_scaling")),
  noiseTols(problem_db.get_rv("method.nond.regression_noise_tolerance")),
  l2Penalty(problem_db.get_real("method.nond.regression_penalty")),
//initSGLevel(problem_db.get_ushort("method.nond.adapted_basis.initial_level")),
  numAdvance(problem_db.get_ushort("method.nond.adapted_basis.advancements")),
  expOrderSpec(problem_db.get_ushort("method.nond.expansion_order")),
  collocPtsSpec(problem_db.get_sizet("method.nond.collocation_points")),
  expSamplesSpec(problem_db.get_sizet("method.nond.expansion_samples")),
  normalizedCoeffOutput(problem_db.get_bool("method.nond.normalized")),
  uSpaceType(problem_db.get_short("method.nond.expansion_type")),
  quadOrderSpec(problem_db.get_ushort("method.nond.quadrature_order")),
  ssgLevelSpec(problem_db.get_ushort("method.nond.sparse_grid_level")),
  cubIntSpec(problem_db.get_ushort("method.nond.cubature_integrand")),
  importBuildPointsFile(
    problem_db.get_string("method.import_build_points_file")),
  expansionImportFile(
    problem_db.get_string("method.nond.import_expansion_file")),
  expansionExportFile(
    problem_db.get_string("method.nond.export_expansion_file"))
  //resizedFlag(false), callResize(false)
{
  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // --------------------
  // Data import settings
  // --------------------
  String pt_reuse = problem_db.get_string("method.nond.point_reuse");
  if (!importBuildPointsFile.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType, iteratedModel.current_variables().view()));
    // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  Iterator u_space_sampler;
  String approx_type;
  unsigned short sample_type = problem_db.get_ushort("method.sample_type");
  const String& rng = problem_db.get_string("method.random_number_generator");

  UShortArray exp_orders; // defined for expansion_samples/regression
  configure_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);

  if (!expansionImportFile.empty()) // PCE import: no regression/projection
    approx_type = //(piecewiseBasis) ? "piecewise_orthogonal_polynomial" :
      "global_orthogonal_polynomial";
  else if (!config_integration(quadOrderSpec, ssgLevelSpec, cubIntSpec,
	     u_space_sampler, g_u_model, approx_type) &&
	   !config_expectation(expSamplesSpec, sample_type, randomSeed, rng,
	     u_space_sampler, g_u_model, approx_type) &&
	   !config_regression(exp_orders, collocPtsSpec,
	     problem_db.get_real("method.nond.collocation_ratio_terms_order"),
	     problem_db.get_short("method.nond.regression_type"),
	     problem_db.get_short("method.nond.least_squares_regression_type"),
	     problem_db.get_usa("method.nond.tensor_grid_order"), sample_type,
	     randomSeed, rng, pt_reuse, u_space_sampler, g_u_model,
	     approx_type)) {
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
  // DFSModel consumes QoI aggregations; supports surrogate grad evals at most
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  ShortArray pce_asv(g_u_model.qoi(), 3); // for stand alone mode
  ActiveSet  pce_set(pce_asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model.current_variables().view();
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse, importBuildPointsFile,
    problem_db.get_ushort("method.import_build_format"),
    problem_db.get_bool("method.import_build_active_only"),
    problem_db.get_string("method.export_approx_points_file"),
    problem_db.get_ushort("method.export_approx_format")));
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler(problem_db.get_ushort("method.sample_type"),
    problem_db.get_string("method.random_number_generator"),
    problem_db.get_ushort("method.nond.integration_refinement"),
    problem_db.get_iv("method.nond.refinement_samples"),
    problem_db.get_string("method.import_approx_points_file"),
    problem_db.get_ushort("method.import_approx_format"), 
    problem_db.get_bool("method.import_approx_active_only"));

  if (parallelLib.command_line_check())
    Cout << "\nPolynomial_chaos construction completed: initial grid size of "
	 << numSamplesOnModel << " evaluations to be performed." << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ numerical integration (quadrature, sparse grid, cubature). */
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		    unsigned short num_int, const RealVector& dim_pref,
		    short u_space_type, short refine_type, short refine_control,
		    short covar_control, short rule_nest, short rule_growth,
		    bool piecewise_basis, bool use_derivs,
                    String exp_expansion_file ):
  NonDExpansion(POLYNOMIAL_CHAOS, model, model.current_variables().view(),
		exp_coeffs_approach, dim_pref, 0, refine_type, refine_control,
		covar_control, 0., rule_nest, rule_growth, piecewise_basis,
		use_derivs), 
  // Note: non-zero seed would be needed for expansionSampler, if defined
  crossValidation(false), crossValidNoiseOnly(false),
  maxCVOrderCandidates(USHRT_MAX), respScaling(false), l2Penalty(0.),
  numAdvance(3), normalizedCoeffOutput(false), uSpaceType(u_space_type),
  expansionExportFile(exp_expansion_file)
  //resizedFlag(false), callResize(false), initSGLevel(0)
{
  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType, iteratedModel.current_variables().view()));
    // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  Iterator u_space_sampler; String approx_type;
  switch (exp_coeffs_approach) {
  case Pecos::QUADRATURE:
    config_integration(num_int, USHRT_MAX, USHRT_MAX, u_space_sampler,
		       g_u_model, approx_type); break;
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
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
  // DFSModel consumes QoI aggregations. Helper mode: support surrogate Hessians
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  ShortArray pce_asv(g_u_model.qoi(), 7);// TO DO: consider passing in data_mode
  ActiveSet  pce_set(pce_asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model.current_variables().view();
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse));
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ regression (least squares, CS, OLI). */
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, short exp_coeffs_approach,
		    unsigned short exp_order, const RealVector& dim_pref,
		    size_t colloc_pts, Real colloc_ratio,
		    int seed, short u_space_type, short refine_type,
		    short refine_control, short covar_control,
		    //short rule_nest, short rule_growth,
		    bool piecewise_basis, bool use_derivs, bool cv_flag,
		    const String& import_build_pts_file,
		    unsigned short import_build_format,
		    bool import_build_active_only,
                    String exp_expansion_file ):
  NonDExpansion(POLYNOMIAL_CHAOS, model, model.current_variables().view(),
		exp_coeffs_approach, dim_pref, seed, refine_type,
		refine_control, covar_control, colloc_ratio,
		Pecos::NO_NESTING_OVERRIDE, Pecos::NO_GROWTH_OVERRIDE,
		piecewise_basis, use_derivs), 
  crossValidation(cv_flag), crossValidNoiseOnly(false),
  maxCVOrderCandidates(USHRT_MAX), respScaling(false),
  importBuildPointsFile(import_build_pts_file), l2Penalty(0.),
  numAdvance(3), expOrderSpec(exp_order), collocPtsSpec(colloc_pts),
  normalizedCoeffOutput(false), uSpaceType(u_space_type),
  expansionExportFile(exp_expansion_file)
  //resizedFlag(false), callResize(false)
{
  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType, iteratedModel.current_variables().view()));
    // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  UShortArray exp_orders; // defined for expansion_samples/regression
  configure_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);

  Iterator u_space_sampler;
  UShortArray tensor_grid_order; // for OLI + tensorRegression (not supported)
  String approx_type, rng("mt19937"), pt_reuse;
  config_regression(exp_orders, collocPtsSpec, 1, exp_coeffs_approach,
		    Pecos::DEFAULT_LEAST_SQ_REGRESSION, tensor_grid_order,
		    SUBMETHOD_LHS, randomSeed, rng, pt_reuse, u_space_sampler,
		    g_u_model, approx_type);

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  if (!importBuildPointsFile.empty()) pt_reuse = "all";
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  ShortArray pce_asv(g_u_model.qoi(), 7);// TO DO: consider passing in data_mode
  ActiveSet  pce_set(pce_asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model.current_variables().view();
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse, importBuildPointsFile,
    import_build_format, import_build_active_only));
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


/** This constructor is used for helper iterator instantiation on the fly
    that import the PCE coefficients rather than compute them. */
NonDPolynomialChaos::
NonDPolynomialChaos(Model& model, const String& exp_import_file,
		    short u_space_type, const ShortShortPair& approx_view):
  NonDExpansion(POLYNOMIAL_CHAOS, model, approx_view, -1, RealVector(), 0,
		Pecos::NO_REFINEMENT, Pecos::NO_CONTROL, DEFAULT_COVARIANCE,
		0., Pecos::NO_NESTING_OVERRIDE, Pecos::NO_GROWTH_OVERRIDE,
		false, false),
  //expOrderSpec(problem_db.get_ushort("method.nond.expansion_order")),
  normalizedCoeffOutput(false), // TO DO: need to detect this in file
  uSpaceType(u_space_type), expansionImportFile(exp_import_file)
  //expansionExportFile(
  //  problem_db.get_string("method.nond.export_expansion_file"))
{
  if (expansionImportFile.empty()) {
    Cerr << "Error: coefficient import ctor requires a file name." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  // Alternate view for PCE is injected into iteratedModel upstream in the
  // Analyzer ctor, since we want everything upstream in the consistent (ALL)
  // view --> numContinuousVars, allVars, etc., resulting in use of functions
  // like mean(x), etc., when appropriate.  This view is retained for g_u_model
  // and uSpaceModel w/o inducing any additional mappings.
  // > Note: including alternate view w/i Recast also allows prob transform
  //   to operate on inactive state prior to emulation (see ProbTransformModel::
  //   initialize_distribution_types(): "inactive vars are not transformed"
  //   (u_types[i] = x_types[i])
  // > default param list value retains distribution bounds
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType, approx_view));

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  UShortArray exp_orders; // not necessary to pre-define this
  //configure_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);
  Iterator u_space_sampler;
  String pt_reuse, approx_type("global_orthogonal_polynomial"), rng("mt19937");
  short corr_order = -1, corr_type = NO_CORRECTION;
  // DFSModel consumes QoI aggregations; supports up to Hessian eval for full
  // Newton MAP pre-solve
  const ActiveSet& approx_set = g_u_model.current_response().active_set();
  ShortArray pce_asv(iteratedModel.qoi(), 7); // for stand alone mode
  ActiveSet  pce_set(pce_asv, approx_set.derivative_vector());
  uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, approx_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse));
  //uSpaceModel.active_view(pce_view, false); // too far downstream...
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


/** This constructor is called by derived class constructors that
    customize the object construction. */
NonDPolynomialChaos::
NonDPolynomialChaos(unsigned short method_name, ProblemDescDB& problem_db,
		    Model& model):
  NonDExpansion(problem_db, model),
  crossValidation(problem_db.get_bool("method.nond.cross_validation")),
  crossValidNoiseOnly(
    problem_db.get_bool("method.nond.cross_validation.noise_only")),
  maxCVOrderCandidates(
    problem_db.get_ushort("method.nond.cross_validation.max_order_candidates")),
  respScaling(problem_db.get_bool("method.nond.response_scaling")),
  noiseTols(problem_db.get_rv("method.nond.regression_noise_tolerance")),
  l2Penalty(problem_db.get_real("method.nond.regression_penalty")),
//initSGLevel(problem_db.get_ushort("method.nond.adapted_basis.initial_level")),
  numAdvance(problem_db.get_ushort("method.nond.adapted_basis.advancements")),
  normalizedCoeffOutput(problem_db.get_bool("method.nond.normalized")),
  uSpaceType(problem_db.get_short("method.nond.expansion_type")),
  cubIntSpec(problem_db.get_ushort("method.nond.cubature_integrand")),
  importBuildPointsFile(
    problem_db.get_string("method.import_build_points_file")),
  expansionImportFile(
    problem_db.get_string("method.nond.import_expansion_file")),
  expansionExportFile(
    problem_db.get_string("method.nond.export_expansion_file"))
  //resizedFlag(false), callResize(false)
{
  // Rest is in derived class...
}


/** This constructor is called by derived class constructors for lightweight
    instantiations that employ numerical integration (quadrature, sparse grid,
    cubature). */
NonDPolynomialChaos::
NonDPolynomialChaos(unsigned short method_name, Model& model,
		    short exp_coeffs_approach, const RealVector& dim_pref,
		    short u_space_type, short refine_type, short refine_control,
		    short covar_control, short ml_alloc_control,
		    short ml_discrep, short rule_nest, short rule_growth,
		    bool piecewise_basis, bool use_derivs):
  NonDExpansion(method_name, model, model.current_variables().view(),
		exp_coeffs_approach, dim_pref, 0, refine_type, refine_control,
		covar_control, 0., rule_nest, rule_growth, piecewise_basis,
		use_derivs), 
  crossValidation(false), crossValidNoiseOnly(false),
  maxCVOrderCandidates(USHRT_MAX), respScaling(false), l2Penalty(0.),
  numAdvance(3), normalizedCoeffOutput(false), uSpaceType(u_space_type)
  //resizedFlag(false), callResize(false), initSGLevel(0)
{
  multilevAllocControl     = ml_alloc_control;
  multilevDiscrepEmulation = ml_discrep;

  // Rest is in derived class...
}


/** This constructor is called by derived class constructors for lightweight
    instantiations that employ regression (least squares, CS, OLI). */
NonDPolynomialChaos::
NonDPolynomialChaos(unsigned short method_name, Model& model,
		    short exp_coeffs_approach, const RealVector& dim_pref,
		    short u_space_type, short refine_type, short refine_control,
		    short covar_control, const SizetArray& colloc_pts_seq,
		    Real colloc_ratio, short ml_alloc_control, short ml_discrep,
		    //short rule_nest, short rule_growth,
		    bool piecewise_basis, bool use_derivs, bool cv_flag):
  NonDExpansion(method_name, model, model.current_variables().view(),
		exp_coeffs_approach, dim_pref, 0, refine_type, refine_control,
		covar_control, colloc_ratio, Pecos::NO_NESTING_OVERRIDE,
		Pecos::NO_GROWTH_OVERRIDE, piecewise_basis, use_derivs),
  crossValidation(cv_flag), crossValidNoiseOnly(false),
  maxCVOrderCandidates(USHRT_MAX), respScaling(false), l2Penalty(0.),
  numAdvance(3), normalizedCoeffOutput(false), uSpaceType(u_space_type)
  //resizedFlag(false), callResize(false)
{
  multilevAllocControl     = ml_alloc_control;
  multilevDiscrepEmulation = ml_discrep;
  collocPtsSeqSpec         = colloc_pts_seq;

  // Rest is in derived class...
}


NonDPolynomialChaos::~NonDPolynomialChaos()
{ }


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
    expansionCoeffsApproach = (refineControl) ?
      Pecos::INCREMENTAL_SPARSE_GRID : Pecos::COMBINED_SPARSE_GRID;
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
		   int seed, const String& rng, Iterator& u_space_sampler,
		   Model& g_u_model, String& approx_type)
{
  if (exp_samples == SZ_MAX)
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
    if (refineType && fixedSeed)
      Cerr << "Warning: combining sample refinement with fixed_seed is more "
	   << "likely to cause sample redundancy." << std::endl;

    construct_lhs(u_space_sampler, g_u_model, sample_type, numSamplesOnModel,
		  seed, rng, !fixedSeed, ACTIVE);

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
		  unsigned short sample_type, int seed, const String& rng,
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
      exp_terms = Pecos::SharedPolyApproxData::tensor_product_terms(exp_orders);
      break;
    }
    termsOrder = colloc_ratio_terms_order;
    if (colloc_pts != SZ_MAX) { // define collocRatio from colloc pts
      numSamplesOnModel = colloc_pts;
      if (collocRatio == 0.) // if default (no user spec), then infer
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
      Pecos::anisotropic_order_to_dimension_preference(dim_quad_order,
	quad_order, dim_pref);
      // use alternate NonDQuad ctor to filter (deprecated) or sub-sample
      // quadrature points (uSpaceModel.build_approximation() invokes
      // daceIterator.run()).  The quad order inputs are updated within
      // NonDQuadrature as needed to satisfy min order constraints (but
      // not nested constraints: nestedRules is false to retain m >= p+1).
      construct_quadrature(u_space_sampler, g_u_model, quad_order, dim_pref,
			   numSamplesOnModel, seed);
    }
    else { // unstructured grid: LHS samples
      // if reusing samples within a refinement strategy, we prefer generating
      // different random numbers for new points within the grid (even if the
      // number of samples differs)
      // Note: uniform refinement uses DFSModel::rebuild_approximation()
      // which directly computes sample increment
      // *** TO DO: would be good to disntinguish top-level seed fixing for OUU
      //            from lower-level seed fixing across levels or refine iters.
      if (refineType && fixedSeed)
	Cerr << "Warning: combining sample refinement with fixed_seed is more "
	     << "likely to cause sample redundancy." << std::endl;
      // reuse type/seed/rng settings intended for the expansion_sampler.
      // Unlike expansion_sampler, allow sampling pattern to vary under
      // unstructured grid refinement/replacement/augmentation.  Also
      // unlike expansion_sampler, we use an ACTIVE sampler mode for
      // forming the PCE over all active variables.
      construct_lhs(u_space_sampler, g_u_model, sample_type, numSamplesOnModel,
		    seed, rng, !fixedSeed, ACTIVE);
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

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  Model g_u_model;
  g_u_model.assign_rep(std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType, iteratedModel.current_variables().view()));
    // retain dist bounds

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
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
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
      configure_expansion_orders(expOrderSpec, dimPrefSpec, exp_orders);
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
    
      if (collocPtsSpec != SZ_MAX) // ratio from pts
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
  // DFSModel: consume any QoI aggregation. Resize: support approx Hessians
  const ActiveSet& recast_set = g_u_model.current_response().active_set();
  ShortArray pce_asv(g_u_model.qoi(), 7);// TO DO: consider passing in data_mode
  ActiveSet  pce_set(pce_asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model.current_variables().view();
  if (expansionCoeffsApproach == Pecos::QUADRATURE ||
      expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
      expansionCoeffsApproach == Pecos::INCREMENTAL_SPARSE_GRID ||
      expansionCoeffsApproach == Pecos::CUBATURE) {
    approx_type = "global_projection_orthogonal_polynomial";
    uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
      g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
      corr_order, data_order, outputLevel, pt_reuse));
  }
  else {
    approx_type = "global_regression_orthogonal_polynomial";
    if (!importBuildPointsFile.empty())
      pt_reuse = "all";
    //DataFitSurrModel* orig_dfs_model
    //  = (DataFitSurrModel*)uSpaceModel.model_rep();
    //unsigned short import_format = orig_dfs_model->import_build_format();
    //bool      import_active_only = orig_dfs_model->import_build_active_only();
    uSpaceModel.assign_rep(std::make_shared<DataFitSurrModel>(u_space_sampler,
      g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
      corr_order, data_order, outputLevel, pt_reuse)); // no import after resize
    //, importBuildPointsFile, import_format, import_active_only), false);
  }

  initialize_u_space_model();

  // -----------------------------------------
  // (Re)Construct expansionSampler, if needed
  // -----------------------------------------
  // Rather than caching these settings in the class, just preserve them
  // from the previously constructed expansionSampler:
  std::shared_ptr<NonDSampling> exp_sampler_rep =
    std::static_pointer_cast<NonDSampling>(expansionSampler.iterator_rep());
  unsigned short sample_type(SUBMETHOD_DEFAULT); String rng;
  if (exp_sampler_rep) {
    sample_type = exp_sampler_rep->sampling_scheme();
    rng         = exp_sampler_rep->random_number_generator();
  }
  std::shared_ptr<NonDAdaptImpSampling> imp_sampler_rep =
    std::static_pointer_cast<NonDAdaptImpSampling>
    (importanceSampler.iterator_rep());
  unsigned short int_refine(NO_INT_REFINE); IntVector refine_samples;
  if (imp_sampler_rep) {
    int_refine = imp_sampler_rep->sampling_scheme();
    refine_samples.sizeUninitialized(1);
    refine_samples[0] = imp_sampler_rep->refinement_samples();
  }
  construct_expansion_sampler(sample_type, rng, int_refine, refine_samples);
  // no import after resize (data would be in original space)

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
  NonDExpansion::initialize_u_space_model();
  configure_pecos_options(); // pulled out of base because C3 does not use it

  SharedApproxData& shared_data = uSpaceModel.shared_approximation();
  std::shared_ptr<SharedPecosApproxData> shared_data_rep
    = std::static_pointer_cast<SharedPecosApproxData>(shared_data.data_rep());
  // Transfer regression data: cross validation, noise tol, and L2 penalty.
  // Note: regression solver type is transferred via expansionCoeffsApproach
  //       in NonDExpansion::initialize_u_space_model()
  if (expansionCoeffsApproach >= Pecos::DEFAULT_REGRESSION) {
    // TO DO: consider adding support for machine-generated seeds (when no
    // user spec) as well as seed progressions for varyPattern.  Coordinate
    // with JDJ on whether Dakota or CV should own these features.
    Pecos::RegressionConfigOptions
      rc_options(crossValidation, crossValidNoiseOnly, maxCVOrderCandidates,
		 respScaling, random_seed(), noiseTols, l2Penalty, false,
		 0/*initSGLevel*/, 2, numAdvance);
    shared_data_rep->configuration_options(rc_options);

    // updates for automatic order adaptation
    //...
  }

  // DataFitSurrModel copies u-space mvDist from ProbabilityTransformModel
  const Pecos::MultivariateDistribution& u_mvd
    = uSpaceModel.multivariate_distribution();
  // construct the polynomial basis (shared by integration drivers)
  shared_data.construct_basis(u_mvd);
  // mainly a run-time requirement, but also needed at construct time
  // (e.g., to initialize NumericGenOrthogPolynomial::distributionType)
  //shared_data_rep->update_basis_distribution_parameters(u_mvd);
  // NumerGenOrthogPolynomial instances need to compute polyCoeffs and
  // orthogPolyNormsSq in addition to gaussPoints and gaussWeights
  shared_data_rep->coefficients_norms_flag(true);

  // if numerical integration, manage u_space_sampler updates
  bool num_int = (expansionCoeffsApproach == Pecos::QUADRATURE           ||
		  expansionCoeffsApproach == Pecos::CUBATURE             ||
		  expansionCoeffsApproach == Pecos::COMBINED_SPARSE_GRID ||
		  expansionCoeffsApproach == Pecos::INCREMENTAL_SPARSE_GRID);
  if ( num_int || ( tensorRegression && numSamplesOnModel ) ) {
    shared_data.integration_iterator(uSpaceModel.subordinate_iterator());
    initialize_u_space_grid(); // propagates dist param updates
  }
  else // propagate dist param updates in case without IntegrationDriver
    shared_data.update_basis_distribution_parameters(u_mvd);
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
      abort_handler(METHOD_ERROR);
    }

    // Import the PCE coefficients for all QoI and a shared multi-index:
    // > Annotation provides questionable value in this context & is off for now
    // > see view management in ctor supporting exp_import_file; alternate views
    //   are injected at the g_u_model level.
    RealVectorArray coeffs_array(numFunctions); UShort2DArray multi_index;
    String context("polynomial chaos expansion import file");
    unsigned short tabular_format = TABULAR_NONE;
    TabularIO::read_data_tabular(expansionImportFile, context,
				 coeffs_array, multi_index, tabular_format,
				 numContinuousVars, numFunctions);

    // post the shared data
    std::shared_ptr<SharedPecosApproxData> data_rep =
      std::static_pointer_cast<SharedPecosApproxData>
      (uSpaceModel.shared_approximation().data_rep());
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
  std::shared_ptr<SharedPecosApproxData> shared_data_rep
    = std::static_pointer_cast<SharedPecosApproxData>(shared_approx.data_rep());
  const Pecos::SDVArray& sdv_array
    = poly_approxs[0].surrogate_data().variables_data();
  int num_surr_data_pts = sdv_array.size();
  RealMatrix current_samples( numContinuousVars, num_surr_data_pts, false );
  for (int j=0; j<num_surr_data_pts; ++j) {
    const RealVector& c_vars = sdv_array[j].continuous_variables();
    for (int i=0; i<numContinuousVars; ++i) 
      current_samples(i,j) = c_vars[i];
  }

  LejaSampler sampler;
  sampler.set_seed(random_seed());
  sampler.set_precondition(true);
  //sampler.set_precondition(false);
  std::vector<Pecos::BasisPolynomial>& poly_basis
    = shared_data_rep->polynomial_basis();
  sampler.set_polynomial_basis( poly_basis );
  sampler.set_total_degree_basis_from_num_samples(numContinuousVars, new_size);
  RealMatrix candidate_samples_matrix;
  Pecos::util::convert( candidate_samples, candidate_samples_matrix );

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
  Pecos::util::extract_submatrix_from_column_indices(unique_candidate_samples,
						     selected_candidate_indices,
						     best_samples );
  
  if (outputLevel >= DEBUG_OUTPUT) {
    // write samples to output
    Cout << "Select refinement pts: best_samples =\n" << best_samples;

    // write samples to file
    std::ofstream export_file_stream;
    std::string filename = "bayesian-adaptive-emulator-samples-";
    filename += std::to_string(best_samples.numCols() + num_surr_data_pts);
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
  std::shared_ptr<PecosApproximation> poly_approx_rep =
    std::static_pointer_cast<PecosApproximation>(poly_approxs[0].approx_rep());

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
  Pecos::util::truncated_pivoted_lu_factorization(A, L_factor, U_factor, pivots,
						  new_size, numSamplesOnModel);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Select refinement pts: pivots =\n" << pivots;

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
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Select refinement pts: best_samples =\n" << best_samples;
}


/** Used for uniform refinement of regression-based PCE. */
void NonDPolynomialChaos::increment_order_from_grid()
{
  std::shared_ptr<SharedPecosApproxData> shared_data_rep =
    std::static_pointer_cast<SharedPecosApproxData>
    (uSpaceModel.shared_approximation().data_rep());

  // update expansion order based on existing collocation ratio and
  // updated number of truth model samples

  // copy
  UShortArray exp_order = shared_data_rep->expansion_order();
  // increment
  ratio_samples_to_order(collocRatio, numSamplesOnModel, exp_order, true);
  // restore
  shared_data_rep->expansion_order(exp_order);
}


void NonDPolynomialChaos::update_samples_from_order_increment()
{
  std::shared_ptr<SharedPecosApproxData> shared_data_rep =
    std::static_pointer_cast<SharedPecosApproxData>
    (uSpaceModel.shared_approximation().data_rep());
  const UShortArray& exp_order = shared_data_rep->expansion_order();
  size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
    Pecos::SharedPolyApproxData::tensor_product_terms(exp_order) :
    Pecos::SharedPolyApproxData::total_order_terms(exp_order);

  // update numSamplesOnModel based on existing collocation ratio and
  // updated number of expansion terms
  numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);
}
// Note: update_samples_from_order_decrement() defaults to
// update_samples_from_order_increment(), so no redefinition reqd for PCE


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


/* Compute power mean of sparsity (common power values: 1 = average value,
   2 = root mean square, DBL_MAX = max value). */
void NonDPolynomialChaos::
sample_allocation_metric(Real& sparsity_metric, Real power)
{
  // case RIP_SAMPLING in NonDExpansion::multilevel_regression():

  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  bool pow_1   = (power == 1.), // simple average
       pow_inf = (power == std::numeric_limits<Real>::max());
  Real sum = 0., max = 0.;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    std::shared_ptr<PecosApproximation> poly_approx_q =
      std::static_pointer_cast<PecosApproximation>
      (poly_approxs[qoi].approx_rep());
    size_t sparsity_q = poly_approx_q->sparsity();
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Sparsity(" /*lev " << lev << ", "*/ << "qoi " << qoi
	/* << ", iter " << iter */ << ") = " << sparsity_q << '\n';

    if (pow_inf) {
      if (sparsity_q > max)
	max = sparsity_q;
    }
    else
      sum += (pow_1) ? sparsity_q : std::pow((Real)sparsity_q, power);
  }
  if (pow_inf)
    sparsity_metric = max;
  else {
    sum /= numFunctions;
    sparsity_metric = (pow_1) ? sum : std::pow(sum, 1. / power);
  }
}


void NonDPolynomialChaos::post_run(std::ostream& s)
{
  Analyzer::post_run(s);
  // allow quiet helper methods to still export their expansion:
  if (!summaryOutputFlag && !expansionExportFile.empty())
    export_coefficients();
  // else post_run will export during print_results
}


void NonDPolynomialChaos::print_results(std::ostream& s, short results_state)
{
  switch (results_state) {
  case REFINEMENT_RESULTS:  case INTERMEDIATE_RESULTS:
    if (outputLevel == DEBUG_OUTPUT)   print_coefficients(s);
    break;
  case FINAL_RESULTS:
    if (outputLevel >= NORMAL_OUTPUT)  print_coefficients(s);
    if (!expansionExportFile.empty())  export_coefficients();
    break;
  }

  NonDExpansion::print_results(s, results_state);
}


void NonDPolynomialChaos::print_coefficients(std::ostream& s)
{
  std::vector<Approximation>& poly_approxs = uSpaceModel.approximations();
  const StringArray& fn_labels = iteratedModel.response_labels();
  const Variables&   vars      = iteratedModel.current_variables();
  const SizetArray&  ac_totals = vars.shared_data().active_components_totals();

  size_t i, width = write_precision+7, num_ceuv = ac_totals[TOTAL_CEUV],
    num_csv = ac_totals[TOTAL_CSV];
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
    for (j=0; j<startCAUV; ++j)
      { std::sprintf(tag, "d%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<numCAUV; ++j)
      { std::sprintf(tag, "u%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<num_ceuv; ++j)
      { std::sprintf(tag, "e%i", j+1); s << std::setw(5) << tag; }
    for (j=0; j<num_csv; ++j)
      { std::sprintf(tag, "s%i", j+1); s << std::setw(5) << tag; }
    s << "\n  " << std::setw(width) << "-----------";
    for (j=0; j<numContinuousVars; ++j)
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
  std::shared_ptr<SharedPecosApproxData> data_rep =
    std::static_pointer_cast<SharedPecosApproxData>
    (uSpaceModel.shared_approximation().data_rep());
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
