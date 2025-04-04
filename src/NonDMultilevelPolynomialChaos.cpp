/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
NonDMultilevelPolynomialChaos(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDPolynomialChaos(DEFAULT_METHOD, problem_db, model), // bypass PCE ctor
  expOrderSeqSpec(problem_db.get_usa("method.nond.expansion_order")),
  expSamplesSeqSpec(problem_db.get_sza("method.nond.expansion_samples")),
  quadOrderSeqSpec(problem_db.get_usa("method.nond.quadrature_order")),
  ssgLevelSeqSpec(problem_db.get_usa("method.nond.sparse_grid_level")),
  sequenceIndex(0) //resizedFlag(false), callResize(false)
{
  randomSeedSeqSpec = problem_db.get_sza("method.random_seed_sequence");

  assign_modes();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // --------------------
  // Data import settings
  // --------------------
  String pt_reuse = probDescDB.get_string("method.nond.point_reuse");
  if (!importBuildPointsFile.empty() && pt_reuse.empty())
    pt_reuse = "all"; // reassign default if data import

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  auto g_u_model = std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  std::shared_ptr<Iterator> u_space_sampler;
  String approx_type;
  unsigned short sample_type = probDescDB.get_ushort("method.sample_type");
  const String& rng = probDescDB.get_string("method.random_number_generator");

  UShortArray exp_orders; // defined for expansion_samples/regression
  configure_expansion_orders(expansion_order(), dimPrefSpec, exp_orders);

  if (!config_integration(quadrature_order(), sparse_grid_level(), cubIntSpec,
	u_space_sampler, g_u_model, approx_type) &&
      !config_expectation(expansion_samples(), sample_type, random_seed(), rng,
	u_space_sampler, g_u_model, approx_type) &&
      !config_regression(exp_orders, collocation_points(),
	probDescDB.get_real("method.nond.collocation_ratio_terms_order"),
	probDescDB.get_short("method.nond.regression_type"),
	probDescDB.get_short("method.nond.least_squares_regression_type"),
	probDescDB.get_usa("method.nond.tensor_grid_order"), sample_type,
	random_seed(), rng, pt_reuse, u_space_sampler, g_u_model, approx_type)){
    Cerr << "Error: incomplete configuration in NonDMultilevelPolynomialChaos "
	 << "constructor." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Configure settings for ML allocation (follows solver type config)
  assign_allocation_control();

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  const ActiveSet& recast_set = g_u_model->current_response().active_set();
  // DFSModel consumes QoI aggregations; supports surrogate grad evals at most
  ShortArray pce_asv(g_u_model->qoi(), 3); // for stand alone mode
  ActiveSet  pce_set(pce_asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model->current_variables().view();
  uSpaceModel = std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse, importBuildPointsFile,
    probDescDB.get_ushort("method.import_build_format"),
    probDescDB.get_bool("method.import_build_active_only"),
    probDescDB.get_string("method.export_approx_points_file"),
    probDescDB.get_ushort("method.export_approx_format"));
  initialize_u_space_model();

  // -------------------------------------
  // Construct expansionSampler, if needed
  // -------------------------------------
  construct_expansion_sampler(problem_db.get_ushort("method.sample_type"),
    problem_db.get_string("method.random_number_generator"),
    problem_db.get_ushort("method.nond.integration_refinement"),
    problem_db.get_iv("method.nond.refinement_samples"),
    probDescDB.get_string("method.import_approx_points_file"),
    probDescDB.get_ushort("method.import_approx_format"),
    probDescDB.get_bool("method.import_approx_active_only"));

  if (parallelLib.command_line_check())
    Cout << "\nPolynomial_chaos construction completed: initial grid size of "
	 << numSamplesOnModel << " evaluations to be performed." << std::endl;
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ numerical integration (quadrature, sparse grid, cubature). */
NonDMultilevelPolynomialChaos::
NonDMultilevelPolynomialChaos(/*unsigned short method_name,*/ std::shared_ptr<Model> model,
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
		      refine_control, covar_control, ml_alloc_control,
		      ml_discrep, rule_nest, rule_growth, piecewise_basis,
		      use_derivs),
  sequenceIndex(0)
{
  assign_modes();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  auto g_u_model = std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType); // retain dist bounds

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
  std::shared_ptr<Iterator> u_space_sampler;
  String approx_type;
  config_integration(quad_order, ssg_level, cubIntSpec, u_space_sampler,
		     g_u_model, approx_type);

  // Configure settings for ML allocation (follows solver type config)
  assign_allocation_control();

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  UShortArray exp_orders; String pt_reuse; // empty for integration approaches
  short corr_order = -1, corr_type = NO_CORRECTION;
  const ActiveSet& recast_set = g_u_model->current_response().active_set();
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  ShortArray asv(g_u_model->qoi(), 7); // TO DO: consider passing in data_mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model->current_variables().view();
  uSpaceModel = std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


/** This constructor is used for helper iterator instantiation on the fly
    that employ regression (least squares, CS, OLI). */
NonDMultilevelPolynomialChaos::
NonDMultilevelPolynomialChaos(unsigned short method_name, std::shared_ptr<Model> model,
			      short exp_coeffs_approach,
			      const UShortArray& exp_order_seq,
			      const RealVector& dim_pref,
			      const SizetArray& colloc_pts_seq,
			      Real colloc_ratio, const SizetArray& seed_seq,
			      short u_space_type, short refine_type,
			      short refine_control, short covar_control,
			      short ml_alloc_control, short ml_discrep,
			      //short rule_nest, short rule_growth,
			      bool piecewise_basis, bool use_derivs,
			      bool cv_flag, const String& import_build_pts_file,
			      unsigned short import_build_format,
			      bool import_build_active_only):
  NonDPolynomialChaos(method_name, model, exp_coeffs_approach, dim_pref,
		      u_space_type, refine_type, refine_control, covar_control,
		      colloc_pts_seq, colloc_ratio, ml_alloc_control,
		      ml_discrep, //rule_nest, rule_growth,
		      piecewise_basis, use_derivs, cv_flag),
  expOrderSeqSpec(exp_order_seq), sequenceIndex(0)
{
  randomSeedSeqSpec = seed_seq;

  assign_modes();

  // ----------------
  // Resolve settings
  // ----------------
  short data_order;
  resolve_inputs(uSpaceType, data_order);

  // -------------------
  // Recast g(x) to G(u)
  // -------------------
  auto g_u_model = std::make_shared<ProbabilityTransformModel>(
    iteratedModel, uSpaceType); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  size_t colloc_pts = collocation_points();
  UShortArray exp_orders; // defined for expansion_samples/regression
  configure_expansion_orders(expansion_order(), dimPrefSpec, exp_orders);

  std::shared_ptr<Iterator> u_space_sampler;
  UShortArray tensor_grid_order; // for OLI + tensorRegression (not supported)
  String approx_type, rng("mt19937"), pt_reuse;
  config_regression(exp_orders, colloc_pts, 1, exp_coeffs_approach,
		    Pecos::DEFAULT_LEAST_SQ_REGRESSION, tensor_grid_order,
		    SUBMETHOD_LHS, random_seed(), rng, pt_reuse,
		    u_space_sampler, g_u_model, approx_type);

  // Configure settings for ML allocation (follows solver type config)
  assign_allocation_control();

  // --------------------------------
  // Construct G-hat(u) = uSpaceModel
  // --------------------------------
  // G-hat(u) uses an orthogonal polynomial approximation over the
  // active/uncertain variables (using same view as iteratedModel/g_u_model:
  // not the typical All view for DACE).  No correction is employed.
  // *** Note: for PCBDO with polynomials over {u}+{d}, change view to All.
  short corr_order = -1, corr_type = NO_CORRECTION;
  if (!import_build_pts_file.empty()) pt_reuse = "all";
  const ActiveSet& recast_set = g_u_model->current_response().active_set();
  // DFSModel: consume any QoI aggregation. Helper mode: support approx Hessians
  ShortArray asv(g_u_model->qoi(), 7); // TO DO: consider passing in data_mode
  ActiveSet pce_set(asv, recast_set.derivative_vector());
  const ShortShortPair& pce_view = g_u_model->current_variables().view();
  uSpaceModel = std::make_shared<DataFitSurrModel>(u_space_sampler,
    g_u_model, pce_set, pce_view, approx_type, exp_orders, corr_type,
    corr_order, data_order, outputLevel, pt_reuse, import_build_pts_file,
    import_build_format, import_build_active_only);
  initialize_u_space_model();

  // no expansionSampler, no numSamplesOnExpansion
}


NonDMultilevelPolynomialChaos::~NonDMultilevelPolynomialChaos()
{ }


void NonDMultilevelPolynomialChaos::initialize_u_space_model()
{
  // For greedy ML, activate combined stats now for propagation to Pecos
  // > don't call statistics_type() as ExpansionConfigOptions not initialized
  //if (multilevAllocControl == GREEDY_REFINEMENT)
  //  statsMetricType = Pecos::COMBINED_EXPANSION_STATS;

  // initializes ExpansionConfigOptions, among other things
  NonDPolynomialChaos::initialize_u_space_model();

  // emulation mode needed for ApproximationInterface::qoi_set_to_key_index()
  uSpaceModel->discrepancy_emulation_mode(multilevDiscrepEmulation);

  // Bind more than one SurrogateData instance via DataFitSurrModel ->
  // PecosApproximation
  //uSpaceModel->link_multilevel_approximation_data();
}


/*
bool NonDMultilevelPolynomialChaos::resize()
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
  g_u_model.assign_rep(new ProbabilityTransformModel(iteratedModel,
    uSpaceType), false); // retain dist bounds

  // -------------------------
  // Construct u_space_sampler
  // -------------------------
  // LHS/Incremental LHS/Quadrature/SparseGrid samples in u-space
  // generated using active sampling view:
  Iterator u_space_sampler;
  UShortArray exp_orders; // empty for numerical integration approaches
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE:
    construct_quadrature(u_space_sampler,  g_u_model,
                         quadrature_order(), dimPrefSpec);
    break;
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
    construct_sparse_grid(u_space_sampler, g_u_model,
                          sparse_grid_level(), dimPrefSpec);
    break;
  case Pecos::CUBATURE:
    construct_cubature(u_space_sampler, g_u_model, cubIntSpec);
    break;
  default:
    numSamplesOnModel = collocation_points();
    if (expansionCoeffsApproach == Pecos::ORTHOG_LEAST_INTERPOLATION ||
	expOrderSeqSpec.empty()) {
      // extract number of collocation points
      // Construct u_space_sampler
      String rng("mt19937");
      construct_lhs(u_space_sampler, g_u_model, SUBMETHOD_LHS,
		    numSamplesOnModel, random_seed(), rng, false, ACTIVE);
    }
    else { // expansion_order-based
      // resolve expansionBasisType, exp_terms, numSamplesOnModel
      if (!expansionBasisType)
	expansionBasisType = (tensorRegression && numContinuousVars <= 5) ?
	  Pecos::TENSOR_PRODUCT_BASIS : Pecos::TOTAL_ORDER_BASIS;
      unsigned short exp_order = expansion_order();
      configure_expansion_orders(exp_order, dimPrefSpec, exp_orders);

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
    
      if (!collocPtsSeqSpec.empty()) // define collocRatio from colloc pts
	collocRatio = terms_samples_to_ratio(exp_terms, numSamplesOnModel);
      else if (collocRatio > 0.)     // define colloc pts from collocRatio
	numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);

      // Construct u_space_sampler
      if (tensorRegression) // tensor sub-sampling
	// define nominal quadrature order as exp_order + 1
	// (m > p avoids most of the 0's in the Psi measurement matrix)
	construct_quadrature(u_space_sampler, g_u_model, exp_order+1,
			     dimPrefSpec);
      else {
	String rng("mt19937");
	construct_lhs(u_space_sampler, g_u_model, SUBMETHOD_LHS,
		      numSamplesOnModel, random_seed(), rng, false, ACTIVE);
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
    uSpaceModel->assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
      pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
      outputLevel, pt_reuse), false);
  }
  else {
    String pt_reuse, approx_type = "global_regression_orthogonal_polynomial";

    if (!importBuildPointsFile.empty())
      pt_reuse = "all";

    //DataFitSurrModel* orig_dfs_model
    //  = (DataFitSurrModel*)uSpaceModel->model_rep();
    //unsigned short import_format = orig_dfs_model->import_build_format();
    //bool      import_active_only = orig_dfs_model->import_build_active_only();

    uSpaceModel->assign_rep(new DataFitSurrModel(u_space_sampler, g_u_model,
      pce_set, approx_type, exp_orders, corr_type, corr_order, data_order,
      outputLevel, pt_reuse), false); // no import after resize
    //, importBuildPointsFile, import_format, import_active_only), false);
  }

  initialize_u_space_model();

  // -----------------------------------------
  // (Re)Construct expansionSampler, if needed
  // -----------------------------------------
  // Rather than caching these settings in the class, just preserve them
  // from the previously constructed expansionSampler:
  std::shared_ptr<NonDSampling> exp_sampler_rep =
    std::static_pointer_cast<NonDSampling>(expansionSampler);
  unsigned short sample_type(SUBMETHOD_DEFAULT); String rng;
  if (exp_sampler_rep) {
    sample_type = exp_sampler_rep->sampling_scheme();
    rng         = exp_sampler_rep->random_number_generator();
  }
  std::shared_ptr<NonDAdaptImpSampling> imp_sampler_rep =
    std::static_pointer_cast<NonDAdaptImpSampling>
    (importanceSampler);
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
    multifidelity_expansion();    break;
  case MULTILEVEL_POLYNOMIAL_CHAOS:
    multilevel_regression();      break;
  default:
    Cerr << "Error: bad configuration in NonDMultilevelPolynomialChaos::"
	 << "core_run()" << std::endl;
    abort_handler(METHOD_ERROR);  break;
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
  uSpaceModel->clear_inactive();

  finalize_expansion();
}


void NonDMultilevelPolynomialChaos::assign_allocation_control()
{
  // multilevAllocControl config and specification checks:
  switch (methodName) {
  case MULTILEVEL_POLYNOMIAL_CHAOS:
    if (expansionCoeffsApproach < Pecos::DEFAULT_REGRESSION) {
      Cerr << "Error: unsupported solver configuration within "
	   << "NonDMultilevelPolynomialChaos constructor." << std::endl;
      abort_handler(METHOD_ERROR);
    }

    switch (multilevAllocControl) {
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
    case DEFAULT_MLMF_CONTROL: // define MLPCE-specific default
      multilevAllocControl = ESTIMATOR_VARIANCE; break;
    case ESTIMATOR_VARIANCE:
      break;
    default:
      Cerr << "Error: unsupported multilevAllocControl in "
	   << "NonDMultilevelPolynomialChaos constructor." << std::endl;
      abort_handler(METHOD_ERROR);           break;
    }
    break;
  case MULTIFIDELITY_POLYNOMIAL_CHAOS:
    break; // GREEDY remains off by default
  }
}


void NonDMultilevelPolynomialChaos::assign_specification_sequence()
{
  bool update_exp = false, update_sampler = false, update_from_ratio = false;
  switch (expansionCoeffsApproach) {
  case Pecos::QUADRATURE: {
    std::shared_ptr<NonDQuadrature> nond_quad =
      std::static_pointer_cast<NonDQuadrature>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex < quadOrderSeqSpec.size())
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    else //if (refineControl)
      nond_quad->reset();   // reset refinement, capture dist param updates
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex < ssgLevelSeqSpec.size())
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    else //if (refineControl)
      nond_sparse->reset(); // reset refinement, capture dist param updates
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
    if (sequenceIndex < randomSeedSeqSpec.size())
      update_sampler = true;
    break;
  }
  case Pecos::ORTHOG_LEAST_INTERPOLATION:
    // assign collocationPoints
    if (sequenceIndex < collocPtsSeqSpec.size()) {
      numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
      update_sampler = true;
    }
    if (sequenceIndex < randomSeedSeqSpec.size())
      update_sampler = true;
    break;
  default: { // regression
    // assign expansionOrder and/or collocationPoints, as admissible
    if (sequenceIndex <  expOrderSeqSpec.size()) update_exp = true;
    if (sequenceIndex < collocPtsSeqSpec.size()) {
      numSamplesOnModel = collocPtsSeqSpec[sequenceIndex];
      update_sampler = true;
    }
    if (sequenceIndex < randomSeedSeqSpec.size())
      update_sampler = true;
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
    std::shared_ptr<NonDQuadrature> nond_quad =
      std::static_pointer_cast<NonDQuadrature>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex+1 < quadOrderSeqSpec.size()) {
      ++sequenceIndex;      // advance order sequence if sufficient entries
      nond_quad->quadrature_order(quadOrderSeqSpec[sequenceIndex]);
    }
    else //if (refineControl)
      nond_quad->reset();   // reset refinement, capture dist param updates
    break;
  }
  case Pecos::COMBINED_SPARSE_GRID: case Pecos::INCREMENTAL_SPARSE_GRID:
  case Pecos::HIERARCHICAL_SPARSE_GRID: {
    std::shared_ptr<NonDSparseGrid> nond_sparse =
      std::static_pointer_cast<NonDSparseGrid>
      (uSpaceModel->subordinate_iterator());
    if (sequenceIndex+1 < ssgLevelSeqSpec.size()) {
      ++sequenceIndex;      // advance level sequence if sufficient entries
      nond_sparse->sparse_grid_level(ssgLevelSeqSpec[sequenceIndex]);
    }
    else //if (refineControl)
      nond_sparse->reset(); // reset refinement, capture dist param updates
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
    if (next_i < randomSeedSeqSpec.size())
      update_sampler = true;
    if (update_exp || update_sampler) ++sequenceIndex;
    break;
  }
  case Pecos::ORTHOG_LEAST_INTERPOLATION: {
    // advance collocationPoints
    size_t next_i = sequenceIndex + 1;
    if (next_i < collocPtsSeqSpec.size()) {
      numSamplesOnModel = collocPtsSeqSpec[next_i];
      update_sampler = true; 
    }
    if (next_i < randomSeedSeqSpec.size())
      update_sampler = true;
    if (update_sampler) ++sequenceIndex;
    break;
  }
  default: { // regression
    // advance expansionOrder and/or collocationPoints, as admissible
    size_t next_i = sequenceIndex + 1;
    if (next_i <  expOrderSeqSpec.size()) update_exp = true;
    if (next_i < collocPtsSeqSpec.size())
      { numSamplesOnModel = collocPtsSeqSpec[next_i]; update_sampler = true; }
    if (next_i < randomSeedSeqSpec.size())
      update_sampler = true;
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
  std::shared_ptr<SharedPecosApproxData> shared_data_rep =
    std::static_pointer_cast<SharedPecosApproxData>
    (uSpaceModel->shared_approximation().data_rep());
  if (update_exp) {
    // update expansion order within Pecos::SharedOrthogPolyApproxData
    UShortArray exp_orders;
    configure_expansion_orders(expansion_order(), dimPrefSpec, exp_orders);
    shared_data_rep->expansion_order(exp_orders);
    if (update_from_ratio) { // update numSamplesOnModel from collocRatio
      size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
	Pecos::SharedPolyApproxData::tensor_product_terms(exp_orders) :
	Pecos::SharedPolyApproxData::total_order_terms(exp_orders);
      numSamplesOnModel = terms_ratio_to_samples(exp_terms, collocRatio);
    }
  }

  // udpate sampler settings (NonDQuadrature or NonDSampling)
  if (update_sampler)
    update_u_space_sampler(sequenceIndex, shared_data_rep->expansion_order());
}


void NonDMultilevelPolynomialChaos::
infer_pilot_sample(/*Real ratio, */size_t num_steps, SizetArray& pilot)
{
  pilot.resize(num_steps);
  UShortArray exp_orders;
  for (size_t i=0; i<num_steps; ++i) {
    configure_expansion_orders(expansion_order(i), dimPrefSpec, exp_orders);
    size_t exp_terms = (expansionBasisType == Pecos::TENSOR_PRODUCT_BASIS) ?
      Pecos::SharedPolyApproxData::tensor_product_terms(exp_orders) :
      Pecos::SharedPolyApproxData::total_order_terms(exp_orders);
    pilot[i] = terms_ratio_to_samples(exp_terms, collocRatio);
  }
}


void NonDMultilevelPolynomialChaos::
increment_sample_sequence(size_t new_samp, size_t total_samp, size_t step)
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
    if (multilevAllocControl != RIP_SAMPLING) { // ESTIMATOR_VARIANCE
      if (collocRatio > 0.) // (fixed) collocation ratio
	update_from_ratio = true;
      else // collocPtsSeqSpec is used in load_pilot_sample(), not here
	err_flag = true;
    }
    break;
  }
  if (err_flag) {
    Cerr << "Error: option not yet supported in NonDMultilevelPolynomialChaos::"
	 << "increment_sample_sequence." << std::endl;
    abort_handler(METHOD_ERROR);
  }


  std::shared_ptr<SharedPecosApproxData> shared_data_rep =
    std::static_pointer_cast<SharedPecosApproxData>
    (uSpaceModel->shared_approximation().data_rep());
  if (update_exp) {
    //increment_order_from_grid(total_samp); // not sufficient in this context

    // reset lower bound for each level:
    UShortArray exp_orders;
    configure_expansion_orders(expansion_order(step), dimPrefSpec, exp_orders);

    if (update_from_ratio) // update the exp_orders based on total_samp
      ratio_samples_to_order(collocRatio, total_samp, exp_orders, false);

    shared_data_rep->expansion_order(exp_orders);
  }

  // udpate sampler settings (NonDQuadrature or NonDSampling)
  if (update_sampler)
    update_u_space_sampler(step, shared_data_rep->expansion_order());
}


void NonDMultilevelPolynomialChaos::
initialize_ml_regression(size_t num_lev, bool& import_pilot)
{
  NonDExpansion::initialize_ml_regression(num_lev, import_pilot);

  // Build point import is active only for the pilot sample and we overlay an
  // additional pilot_sample spec, but we do not augment with samples from a
  // collocation pts/ratio enforcement (pts/ratio controls take over on
  // subsequent iterations).
  if (!importBuildPointsFile.empty()) {
    if (multilevDiscrepEmulation == RECURSIVE_EMULATION) {
      Cout << "\nPilot sample to include imported build points.\n";
      import_pilot = true;
    }
    else
      Cerr << "Warning: build data import only supported for recursive "
	   << "emulation in multilevel_regression()." << std::endl;
  }
}


void NonDMultilevelPolynomialChaos::
compute_sample_increment(const RealVector& sparsity, const SizetArray& N_l,
			 SizetArray& delta_N_l)
{
  // case RIP_SAMPLING in NonDExpansion::multilevel_regression():

  // update targets based on sparsity estimates
  std::shared_ptr<SharedPecosApproxData> data_rep =
    std::static_pointer_cast<SharedPecosApproxData>
    (uSpaceModel->shared_approximation().data_rep());
  const std::map<Pecos::ActiveKey, UShort2DArray>& mi_map
    = data_rep->multi_index_map();
  std::map<Pecos::ActiveKey, UShort2DArray>::const_iterator cit;
  Real s_l;  size_t card_l, lev, num_lev = N_l.size();
  RealVector new_N_l(num_lev, false);
  for (lev=0, cit=mi_map.begin(); lev<num_lev; ++lev, ++cit) {
    s_l = sparsity[lev];  card_l = cit->second.size();
    // Shape of RIP sample profile ~= s_l log^3(s_l) log(card_l) where we
    // are omitting a mutual coherence term (actual sample requirements
    // are conservative upper bounds and relative shape is most important)
    new_N_l[lev] = s_l * std::pow(std::log(s_l), 3.) * std::log((Real)card_l);
  }

  // Sparsity estimates tend to grow for compressible QoI as increased samples
  // drive increased accuracy in less important terms --> CV scores improve for
  // dense solutions.  To control this effect, we retain the shape of the
  // profile but enforce an upper bound on one of the levels.
  // > ML  FT factor: collocRatio multiplier is applied to regression size
  // > ML PCE factor: provides an upper bound on all samples within the profile
  //                  to control feedback, so not used as a sample target

  Real curr_factor, max_curr_factor = 0., factor_ratio,
    factor_bound = 2.; // hard-wired (bound is separate from collocRatio)
  for (lev=0, cit=mi_map.begin(); lev<num_lev && cit!=mi_map.end();
       ++lev, ++cit) {
    curr_factor = new_N_l[lev] / cit->second.size();// samples/cardinality ratio
    if (curr_factor > max_curr_factor)
      max_curr_factor = curr_factor;
  }
  factor_ratio = factor_bound / max_curr_factor;
  if (factor_ratio < 1.) // exceeds upper bound -> scale back
    for (lev=0; lev<num_lev; ++lev)
      new_N_l[lev] *= factor_ratio;

  // Now compute the one-sided sample increments
  for (lev=0; lev<num_lev; ++lev)
    delta_N_l[lev] = one_sided_delta(N_l[lev], new_N_l[lev]);
}


void NonDMultilevelPolynomialChaos::
print_results(std::ostream& s, short results_state)
{
  switch (results_state) {
  case REFINEMENT_RESULTS:  case INTERMEDIATE_RESULTS:
    if (outputLevel >= NORMAL_OUTPUT)  print_coefficients(s);
    break;
  case FINAL_RESULTS:
    if (outputLevel >= NORMAL_OUTPUT)  print_coefficients(s);
    if (!expansionExportFile.empty())  export_coefficients();
    if (!NLev.empty()) {
      s << "<<<<< Samples per solution level:\n";
      print_multilevel_evaluation_summary(s, NLev);
      if (equivHFEvals > 0.) {
	s << "<<<<< Equivalent number of high fidelity evaluations: "
	  << equivHFEvals << std::endl;
	archive_equiv_hf_evals(equivHFEvals);
      }
    }
    break;
  }

  // skip over NonDPolynomialChaos::print_results()
  NonDExpansion::print_results(s, results_state);
}

} // namespace Dakota
