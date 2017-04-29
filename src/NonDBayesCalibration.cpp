/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDBayesCalibration
//- Description: Base class for generic Bayesian inference
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "NonDBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DataFitSurrModel.hpp"
#include "RecastModel.hpp"
#include "DataTransformModel.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDLHSSampling.hpp"
#include "NPSOLOptimizer.hpp"
#include "SNLLOptimizer.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

#include "LHSDriver.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random.hpp"
#include "boost/generator_iterator.hpp"
#include "boost/math/special_functions/digamma.hpp"
// BMA: May need to better manage DLL export / import from ANN in the future
// #ifdef DLL_EXPORTS
// #undef DLL_EXPORTS
// #endif
#include "ANN/ANN.h"
//#include "ANN/ANNperf.h"
//#include "ANN/ANNx.h"
#include "dakota_data_util.hpp"
//#include "dakota_tabular_io.hpp"
#include "DiscrepancyCorrection.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

// initialization of statics
NonDBayesCalibration* NonDBayesCalibration::nonDBayesInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::
NonDBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDCalibration(problem_db, model),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  mcmcModelHasSurrogate(false),
  mapOptAlgOverride(probDescDB.get_ushort("method.nond.pre_solve_method")),
  chainSamples(0), chainCycles(1),
  randomSeed(probDescDB.get_int("method.random_seed")),
  mcmcDerivOrder(1),
  adaptExpDesign(probDescDB.get_bool("method.nond.adapt_exp_design")),
  initHifiSamples (probDescDB.get_int("method.samples")),
  scalarDataFilename(probDescDB.get_string("responses.scalar_data_filename")),
  importCandPtsFile(
    probDescDB.get_string("method.import_candidate_points_file")),
  importCandFormat(
    probDescDB.get_ushort("method.import_candidate_format")),
  numCandidates(probDescDB.get_sizet("method.num_candidates")),
  maxHifiEvals(probDescDB.get_int("method.max_hifi_evaluations")),
  calModelDiscrepancy(probDescDB.get_bool("method.nond.model_discrepancy")),
  discrepancyType(probDescDB.get_string("method.nond.discrepancy_type")),
  numPredConfigs(probDescDB.get_sizet("method.num_prediction_configs")),
  predictionConfigList(probDescDB.get_rv("method.nond.prediction_configs")),
  importPredConfigs(probDescDB.get_string("method.import_prediction_configs")),
  importPredConfigFormat(
    probDescDB.get_ushort("method.import_prediction_configs_format")),
  exportCorrModelFile(
    probDescDB.get_string("method.nond.export_corrected_model_file")),
  exportCorrModelFormat(
    probDescDB.get_ushort("method.nond.export_corrected_model_format")),
  exportDiscrepFile(
    probDescDB.get_string("method.nond.export_discrepancy_file")),
  exportDiscrepFormat(
    probDescDB.get_ushort("method.nond.export_discrep_format")),
  exportCorrVarFile(
    probDescDB.get_string("method.nond.export_corrected_variance_file")),
  exportCorrVarFormat(
    probDescDB.get_ushort("method.nond.export_corrected_variance_format")),
  approxCorrectionOrder(probDescDB.get_short("method.nond.correction_order")),
  configLowerBnds(probDescDB.get_rv("variables.continuous_state.lower_bounds")),
  configUpperBnds(probDescDB.get_rv("variables.continuous_state.upper_bounds")),
  obsErrorMultiplierMode(
    probDescDB.get_ushort("method.nond.calibrate_error_mode")),
  numHyperparams(0),
  invGammaAlphas(probDescDB.get_rv("method.nond.hyperprior_alphas")),
  invGammaBetas(probDescDB.get_rv("method.nond.hyperprior_betas")),
  adaptPosteriorRefine(
    probDescDB.get_bool("method.nond.adaptive_posterior_refinement")),
  proposalCovarType(
    probDescDB.get_string("method.nond.proposal_covariance_type")),
  proposalCovarData(probDescDB.get_rv("method.nond.proposal_covariance_data")),
  proposalCovarFilename(
    probDescDB.get_string("method.nond.proposal_covariance_filename")),
  proposalCovarInputType(
    probDescDB.get_string("method.nond.proposal_covariance_input_type")),
  burnInSamples(probDescDB.get_int("method.burn_in_samples")),
  posteriorStatsKL(probDescDB.get_bool("method.posterior_stats.kl_divergence")),
  posteriorStatsMutual(
    probDescDB.get_bool("method.posterior_stats.mutual_info")),
  subSamplingPeriod(probDescDB.get_int("method.sub_sampling_period")),
  exportMCMCFilename(
    probDescDB.get_string("method.nond.export_mcmc_points_file")),
  exportMCMCFormat(probDescDB.get_ushort("method.nond.export_samples_format"))
{
  // BMA ERROR! Random seed should be set before using it!

  if (adaptExpDesign) {
    // TODO: instead of pulling these models out, change modes on the
    // iteratedModel
    if (iteratedModel.model_type() != "surrogate") {
      Cerr << "\nError: Adaptive Bayesian experiment design requires " 
	   << "hierarchical surrogate\n       model.\n";
      abort_handler(PARSE_ERROR);
    }
    hifiModel = iteratedModel.truth_model();

    int num_exp = expData.num_experiments();
    int num_lhs_samples = std::max(initHifiSamples - num_exp, 0);
    // construct a hi-fi LHS sampler only if needed
    if (num_lhs_samples > 0) {
      unsigned short sample_type = SUBMETHOD_LHS;
      bool vary_pattern = true;
      String rng("mt19937");
      NonDLHSSampling* lhs_sampler_rep;
      lhs_sampler_rep =
        new NonDLHSSampling(hifiModel, sample_type, num_lhs_samples, randomSeed,
                            rng, vary_pattern, ACTIVE_UNIFORM);
      hifiSampler.assign_rep(lhs_sampler_rep, false);
    }
  }

  // assign default proposalCovarType
  if (proposalCovarType.empty()) {
    if (emulatorType) proposalCovarType = "derivatives"; // misfit Hessian
    else              proposalCovarType = "prior";       // prior covariance
  }

  // manage sample partitions and defaults
  int samples_spec = probDescDB.get_int("method.nond.chain_samples");
  if (proposalCovarType == "derivatives") {
    int pc_update_spec
      = probDescDB.get_int("method.nond.proposal_covariance_updates");
    if (pc_update_spec < 1) { // default partition: update every 100 samples
      // if the user specified less than 100 samples, use that,
      // resulting in chainCycles = 1
      chainSamples = std::min(samples_spec, 100);
      chainCycles  = (int)floor((Real)samples_spec / (Real)chainSamples + .5);
    }
    else { // partition as specified
      if (samples_spec < pc_update_spec) {
	// hard error since the user explicitly gave both controls
	Cerr << "\nError: chain_samples must be >= proposal_updates.\n";
	abort_handler(-1);
      }
      chainSamples = (int)floor((Real)samples_spec / (Real)pc_update_spec + .5);
      chainCycles  = pc_update_spec;
    }
  }
  else { 
    chainSamples = samples_spec; 
    chainCycles = 1; 
  }

  if (randomSeed != 0)
    Cout << " NonDBayes Seed (user-specified) = " << randomSeed << std::endl;
  else {
    // Use NonD convenience function for system seed
    randomSeed = generate_system_seed();
    Cout << " NonDBayes Seed (system-generated) = " << randomSeed << std::endl;
  }

  // assign default maxIterations (DataMethod default is -1)
  if (adaptPosteriorRefine && maxIterations < 0)
    maxIterations = 25;

  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    standardizedSpace = true; break; // natafTransform defined w/i NonDExpansion
  default:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");

    // define local natafTransform, whether standardized space or not,
    // since we utilize x-space bounds, moments, density routines
    initialize_random_variable_transformation();
    initialize_random_variable_types(ASKEY_U); // need ranVarTypesX below
    // initialize_random_variable_parameters() is performed at run time
    initialize_random_variable_correlations();
    //initialize_final_statistics(); // statistics set is not default

    // only needed if Nataf transform will actually be performed
    if (standardizedSpace)
      verify_correlation_support(ASKEY_U);
    break;
  }

  // should be independent of data resizes
  construct_mcmc_model();

  init_hyper_parameters();

  // Now the underlying simulation model mcmcModel is setup; wrap it
  // in a data transformation, making sure to allocate gradient/Hessian space
  if (calibrationData) {
    residualModel.assign_rep
      (new DataTransformModel(mcmcModel, expData, numHyperparams, 
                              obsErrorMultiplierMode, mcmcDerivOrder), false);
    // update bounds for hyper-parameters
    Real dbl_inf = std::numeric_limits<Real>::infinity();
    for (size_t i=0; i<numHyperparams; ++i) {
      residualModel.continuous_lower_bound(0.0,     numContinuousVars + i);
      residualModel.continuous_upper_bound(dbl_inf, numContinuousVars + i);
    }
  }
  else
    residualModel = mcmcModel;  // shallow copy

  // TODO: will need to be resized when data changes
  construct_map_optimizer();

  int mcmc_concurrency = 1; // prior to concurrent chains
  maxEvalConcurrency *= mcmc_concurrency;
}


void NonDBayesCalibration::construct_mcmc_model()
{
  // for adaptive experiment design, the surrogate model is the low-fi
  // model which should be calibrated
  // TODO: could avoid this lightweight copy entirely, but less clean
  Model inbound_model = 
    adaptExpDesign ? iteratedModel.surrogate_model() : iteratedModel;

  switch (emulatorType) {

  case PCE_EMULATOR: case SC_EMULATOR: {
    mcmcModelHasSurrogate = true;
    const UShortArray& level_seq
      = probDescDB.get_usa("method.nond.sparse_grid_level");
    const RealVector& dim_pref
      = probDescDB.get_rv("method.nond.dimension_preference");    // not exposed
    bool derivs = probDescDB.get_bool("method.derivative_usage"); // not exposed
    NonDExpansion* se_rep;
    if (emulatorType == SC_EMULATOR) { // SC sparse grid interpolation
      se_rep = new NonDStochCollocation(inbound_model,
	Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	false, derivs);
      mcmcDerivOrder = 3; // Hessian computations not yet implemented for SC
    }
    else {
      if (!level_seq.empty()) // PCE with spectral projection via sparse grid
	se_rep = new NonDPolynomialChaos(inbound_model,
	  Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	  false, false);
      else { 
        // regression PCE: LeastSq/CS (exp_order,colloc_ratio), OLI (colloc_pts)
	const UShortArray& exp_order_seq
	  = probDescDB.get_usa("method.nond.expansion_order");
	short exp_coeffs_approach = (exp_order_seq.empty()) ?
	  Pecos::ORTHOG_LEAST_INTERPOLATION : Pecos::DEFAULT_REGRESSION;
	se_rep = new NonDPolynomialChaos(inbound_model,
	  exp_coeffs_approach, exp_order_seq, dim_pref,
	  probDescDB.get_sza("method.nond.collocation_points"), // pts sequence
	  probDescDB.get_real("method.nond.collocation_ratio"), // single scalar
	  randomSeed, EXTENDED_U, false, derivs,	
	  probDescDB.get_bool("method.nond.cross_validation"),
	  probDescDB.get_string("method.import_build_points_file"),
	  probDescDB.get_ushort("method.import_build_format"),
	  probDescDB.get_bool("method.import_build_active_only"));
      }
      mcmcDerivOrder = 7; // Hessian computations implemented for PCE
    }
    stochExpIterator.assign_rep(se_rep, false);
    // no CDF or PDF level mappings
    RealVectorArray empty_rv_array; // empty
    se_rep->requested_levels(empty_rv_array, empty_rv_array, empty_rv_array,
      empty_rv_array, respLevelTarget, respLevelTargetReduce, cdfFlag, false);
    // extract NonDExpansion's uSpaceModel for use in likelihood evals
    mcmcModel = stochExpIterator.algorithm_space_model(); // shared rep
    natafTransform = se_rep->variable_transformation();   // shared rep
    break;
  }

  case GP_EMULATOR: case KRIGING_EMULATOR: {
    mcmcModelHasSurrogate = true;
    String sample_reuse; String approx_type;
    if (emulatorType == GP_EMULATOR)
      { approx_type = "global_gaussian"; mcmcDerivOrder = 3; } // grad support
    else
      { approx_type = "global_kriging";  mcmcDerivOrder = 7; } // grad,Hess
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
    if (probDescDB.get_bool("method.derivative_usage")) {
      // derivatives for emulator construction (not emulator evaluation)
      if (inbound_model.gradient_type() != "none") data_order |= 2;
      if (inbound_model.hessian_type()  != "none") data_order |= 4;
    }
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    int samples = probDescDB.get_int("method.build_samples");
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_build_points_file");
    if (!import_pts_file.empty())
      { samples = 0; sample_reuse = "all"; }
     
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator; Model lhs_model;
    // NKM requires finite bounds for scaling and init of correlation lengths.
    // Default truncation is +/-10 sigma, which may be overly conservative for
    // these purposes, but +/-3 sigma has little to no effect in current tests.
    bool truncate_bnds = (emulatorType == KRIGING_EMULATOR);
    if (standardizedSpace)
      transform_model(inbound_model, lhs_model, truncate_bnds);//, 3.);
    else
      lhs_model = inbound_model; // shared rep
    // Unlike EGO-based approaches, use ACTIVE sampling mode to concentrate
    // samples in regions of higher prior density
    NonDLHSSampling* lhs_rep = new
      NonDLHSSampling(lhs_model, sample_type, samples, randomSeed,
        probDescDB.get_string("method.random_number_generator"));
    lhs_iterator.assign_rep(lhs_rep, false);

    // natafTransform is not fully updated at this point, but using
    // a shallow copy allows run time updates to propagate
    if (standardizedSpace)
      lhs_rep->initialize_random_variables(natafTransform); // shallow copy

    ActiveSet gp_set = lhs_model.current_response().active_set(); // copy
    gp_set.request_values(mcmcDerivOrder); // for misfit Hessian
    mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, lhs_model,
      gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
      outputLevel, sample_reuse, import_pts_file,
      probDescDB.get_ushort("method.import_build_format"),
      probDescDB.get_bool("method.import_build_active_only")), false);
    break;
  }

  case NO_EMULATOR:
    mcmcModelHasSurrogate = (inbound_model.model_type() == "surrogate");
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
    if (standardizedSpace) transform_model(inbound_model, mcmcModel);//dist bnds
    else                   mcmcModel = inbound_model; // shared rep

    if (mcmcModel.gradient_type() != "none") mcmcDerivOrder |= 2;
    if (mcmcModel.hessian_type()  != "none") mcmcDerivOrder |= 4;
    break;
  }

}


void NonDBayesCalibration::init_hyper_parameters()
{
  // Initialize sizing for hyperparameters (observation error), not
  // currently part of a RecastModel
  size_t num_resp_groups = 
    mcmcModel.current_response().shared_data().num_response_groups(); 
  if (obsErrorMultiplierMode == CALIBRATE_ONE)
    numHyperparams = 1;
  else if (obsErrorMultiplierMode == CALIBRATE_PER_EXPER)
    numHyperparams = expData.num_experiments();
  else if (obsErrorMultiplierMode == CALIBRATE_PER_RESP)
    numHyperparams = num_resp_groups;
  else if (obsErrorMultiplierMode == CALIBRATE_BOTH)
    numHyperparams = expData.num_experiments() * num_resp_groups;

  // Setup priors distributions on hyper-parameters
  if ( (invGammaAlphas.length()  > 1 &&
	invGammaAlphas.length() != numHyperparams) ||
       (invGammaAlphas.length() != invGammaBetas.length()) ) {
    Cerr << "\nError: hyperprior_alphas and hyperprior_betas must both have "
         << "length 1 or number of calibrated\n       error multipliers.\n";
    abort_handler(PARSE_ERROR);
  }
  invGammaDists.resize(numHyperparams);
  for (size_t i=0; i<numHyperparams; ++i) {
    // alpha = (mean/std_dev)^2 + 2
    // beta = mean*(alpha-1)
    // default:
    Real alpha = 102.0, beta = 103.0;  
    // however chosen to have mode = beta/(alpha+1) = 1.0 (initial point)
    //   mean = beta/(alpha-1) ~ 1.0
    //   s.d. ~ 0.1
    if (invGammaAlphas.length() == 1)
      { alpha = invGammaAlphas[0]; beta = invGammaBetas[0]; }
    else if (invGammaAlphas.length() == numHyperparams)
      { alpha = invGammaAlphas[i]; beta = invGammaBetas[i]; }
    // BMA TODO: could store only one inverse gamma if all parameters the same
    invGammaDists[i] = Pecos::RandomVariable(Pecos::INV_GAMMA);
    Pecos::InvGammaRandomVariable* rv_rep = 
      (Pecos::InvGammaRandomVariable*)(invGammaDists[i].random_variable_rep());
    rv_rep->update(alpha, beta);
  }
}


/** Construct optimizer for MAP pre-solve
    Emulator:     on by default; can be overridden with "pre_solve none"
    No emulator: off by default; can be activated  with "pre_solve {sqp,nip}"
                 relies on mapOptimizer ctor to enforce min derivative support
*/
void NonDBayesCalibration::construct_map_optimizer() 
{
  if ( mapOptAlgOverride == SUBMETHOD_SQP || 
       mapOptAlgOverride == SUBMETHOD_NIP ||
       ( emulatorType && mapOptAlgOverride != SUBMETHOD_NONE ) ) {

    size_t num_total_calib_terms = residualModel.num_primary_fns();
    Sizet2DArray vars_map_indices, primary_resp_map_indices(1),
      secondary_resp_map_indices;
    primary_resp_map_indices[0].resize(num_total_calib_terms);
    for (size_t i=0; i<num_total_calib_terms; ++i)
      primary_resp_map_indices[0][i] = i;
    bool nonlinear_vars_map = false; BoolDequeArray nonlinear_resp_map(1);
    nonlinear_resp_map[0] = BoolDeque(num_total_calib_terms, true);
    SizetArray recast_vc_totals;  // empty: no change in size
    BitArray all_relax_di, all_relax_dr; // empty: no discrete relaxation

    switch (mapOptAlgOverride) {
    case SUBMETHOD_SQP:
#ifndef HAVE_NPSOL
      Cerr << "\nWarning: this executable not configured with NPSOL SQP."
	   << "\n         MAP pre-solve not available." << std::endl;
      mapOptAlgOverride = SUBMETHOD_DEFAULT; // model,optimizer not constructed
#endif
      break;
    case SUBMETHOD_NIP:
#ifndef HAVE_OPTPP
      Cerr << "\nWarning: this executable not configured with OPT++ NIP."
	   << "\n         MAP pre-solve not available." << std::endl;
      mapOptAlgOverride = SUBMETHOD_DEFAULT; // model,optimizer not constructed
#endif
      break;
    case SUBMETHOD_DEFAULT: // use full Newton, if available
#ifdef HAVE_OPTPP
      mapOptAlgOverride = SUBMETHOD_NIP;
#elif HAVE_NPSOL
      mapOptAlgOverride = SUBMETHOD_SQP;
#else
      Cerr << "\nWarning: this executable not configured with NPSOL or OPT++."
	   << "\n         MAP pre-solve not available." << std::endl;
#endif
      break;
    }

    // recast ActiveSet requests if full-Newton NIP with gauss-Newton misfit
    // (avoids error in unsupported Hessian requests in Model::manage_asv())
    short nlp_resp_order = 3; // quasi-Newton optimization
    void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
    if (mapOptAlgOverride == SUBMETHOD_NIP) {
      nlp_resp_order = 7; // size RecastModel response for full Newton Hessian
      if (mcmcDerivOrder == 3) // map asrv for Gauss-Newton approx
        set_recast = gnewton_set_recast;
    }

    // RecastModel for bound-constrained argmin(misfit - log prior)
    if (mapOptAlgOverride)
      negLogPostModel.assign_rep(new 
	RecastModel(residualModel, vars_map_indices, recast_vc_totals, 
		    all_relax_di, all_relax_dr, nonlinear_vars_map, NULL,
		    set_recast, primary_resp_map_indices, 
		    secondary_resp_map_indices, 0, nlp_resp_order, 
		    nonlinear_resp_map, neg_log_post_resp_mapping, NULL),false);

    switch (mapOptAlgOverride) {
#ifdef HAVE_NPSOL
    case SUBMETHOD_SQP: {
      // SQP with BFGS Hessians
      int npsol_deriv_level = 3;
      mapOptimizer.assign_rep(new
	NPSOLOptimizer(negLogPostModel, npsol_deriv_level, convergenceTol),
	false);
      break;
    }
#endif
#ifdef HAVE_OPTPP
    case SUBMETHOD_NIP:
      // full Newton (OPTPP::OptBCNewton)
      mapOptimizer.assign_rep(new 
	SNLLOptimizer("optpp_newton", negLogPostModel), false);
      break;
#endif
    }
  }
}

NonDBayesCalibration::~NonDBayesCalibration()
{ }





void NonDBayesCalibration::core_run()
{
  nonDBayesInstance = this;

  if (adaptExpDesign)
    // use meta-iteration in this class
    calibrate_to_hifi();
  else
    // delegate to base class calibration
    calibrate();
  if (calModelDiscrepancy)
    // calibrate a model discrepancy function
    build_model_discrepancy();
    //print_discrepancy_results();
}


void NonDBayesCalibration::derived_init_communicators(ParLevLIter pl_iter)
{
  //iteratedModel.init_communicators(maxEvalConcurrency);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.init_communicators(pl_iter);              break;
  default:
    mcmcModel.init_communicators(pl_iter, maxEvalConcurrency); break;
  }

  if (!hifiSampler.is_null())
    hifiSampler.init_communicators(pl_iter);
}


void NonDBayesCalibration::derived_set_communicators(ParLevLIter pl_iter)
{
  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  //iteratedModel.set_communicators(maxEvalConcurrency);

  // stochExpIterator and mcmcModel use NoDBBaseConstructor,
  // so no need to manage DB list nodes at this level
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.set_communicators(pl_iter);              break;
  default:
    mcmcModel.set_communicators(pl_iter, maxEvalConcurrency); break;
  }

  if (!hifiSampler.is_null())
    hifiSampler.set_communicators(pl_iter);
}


void NonDBayesCalibration::derived_free_communicators(ParLevLIter pl_iter)
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR:
    stochExpIterator.free_communicators(pl_iter);              break;
  default:
    mcmcModel.free_communicators(pl_iter, maxEvalConcurrency); break;
  }

  //iteratedModel.free_communicators(maxEvalConcurrency);

  if (!hifiSampler.is_null())
    hifiSampler.free_communicators(pl_iter);
}


void NonDBayesCalibration::initialize_model()
{
  switch (emulatorType) {
  case PCE_EMULATOR: case SC_EMULATOR: {
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    stochExpIterator.run(pl_iter); break;
  }
  default: // GPs and NO_EMULATOR
    initialize_random_variable_parameters(); // standardizedSpace or not
    //initialize_final_statistics_gradients(); // not required
    if (standardizedSpace) transform_correlations();
    if (emulatorType)      mcmcModel.build_approximation();
    break;
  }
  if(posteriorStatsMutual)
    Cout << "Mutual Information estimation not yet implemented\n";
}


void NonDBayesCalibration::calibrate_to_hifi()
{
  RealVector vars = mcmcModel.continuous_variables();
  const RealVector& initial_point = RealVector(Teuchos::Copy, 
                              	    vars.values(), numContinuousVars);
  
  /* TODO:
     - Handling of hyperparameters
     - More efficient resizing/reconstruction
     - Use hierarhical surrogate eval modes
  */
  int num_exp = expData.num_experiments();
  int num_lhs_samples = std::max(initHifiSamples - num_exp, 0);
  if (num_lhs_samples > 0) {

    hifiSampler.run();

    const RealMatrix& all_samples = hifiSampler.all_samples();
    const IntResponseMap& all_responses = hifiSampler.all_responses();

    if (num_exp == 0) {
      // No file data; all initial hifi calibration data points come from LHS
      // BMA TODO: Once ExperimentData can be updated, post this into
      // expData directly
      ExperimentData exp_data(initHifiSamples, 
                              mcmcModel.current_response().shared_data(), 
                              all_samples, all_responses, outputLevel);
      expData = exp_data;
    }
    else {
      // If number of num_experiments is less than the number of desired initial
      // samples, run LHS to supplement
      IntRespMCIter responses_it = all_responses.begin();
      IntRespMCIter responses_end = all_responses.end();
      for (int i=0 ; responses_it != responses_end; ++responses_it, ++i) {
        RealVector col_vec = 
          Teuchos::getCol(Teuchos::Copy, const_cast<RealMatrix&>(all_samples),
                          i);
        expData.add_data(col_vec, responses_it->second.copy());
      }
    }

  }

  if (outputLevel >= DEBUG_OUTPUT)
    for (size_t i=0; i<initHifiSamples; i++)
      Cout << "Exp Data  i " << i << " value = " << expData.all_data(i);

  // need to initialize this from user input eventually
  size_t num_candidates = numCandidates; 
  RealMatrix design_matrix;
  //RealMatrix response_matrix;
  unsigned short sample_type = SUBMETHOD_LHS;
  bool vary_pattern = true;
  String rng("mt19937");
  Iterator lhs_iterator2;
  if (importCandPtsFile.empty()) {
    NonDLHSSampling* lhs_sampler_rep2;
    int randomSeed1 = randomSeed+1;
    lhs_sampler_rep2 =
      new NonDLHSSampling(hifiModel, sample_type, num_candidates, randomSeed1,
  			rng, vary_pattern, ACTIVE_UNIFORM);
    lhs_iterator2.assign_rep(lhs_sampler_rep2, false);
    lhs_iterator2.pre_run();
    //const RealMatrix design_matrix = lhs_iterator2.all_samples();
    design_matrix = lhs_iterator2.all_samples();
  }
  else {
    // BMA TODO: This should probably be cv() + div() + ...
    size_t num_designvars = hifiModel.tv();
    RealMatrix design_matrix_in;
    TabularIO::read_data_tabular(importCandPtsFile,
				 "user-provided candidate points",
				 design_matrix_in, num_designvars, 
				 importCandFormat, false);
    size_t num_candidates_in = design_matrix_in.numCols();
    if (num_candidates_in < num_candidates) {
      size_t new_candidates = num_candidates - num_candidates_in;
      NonDLHSSampling* lhs_sampler_rep2;
      int randomSeed1 = randomSeed+1;
      lhs_sampler_rep2 =
        new NonDLHSSampling(hifiModel, sample_type, new_candidates, randomSeed1,
    			rng, vary_pattern, ACTIVE_UNIFORM);
      lhs_iterator2.assign_rep(lhs_sampler_rep2, false);
      lhs_iterator2.pre_run();
      RealMatrix design_matrix_supp = lhs_iterator2.all_samples();
      design_matrix.shape(num_designvars, num_candidates);
      for (int i = 0; i < num_candidates_in; i++) {
	RealVector col_vec = Teuchos::getCol(Teuchos::Copy, design_matrix_in, 
	    				     i);
	Teuchos::setCol(col_vec, i, design_matrix);
      }
      for (int i = num_candidates_in; i < num_candidates; i++) {
	RealVector col_vec = Teuchos::getCol(Teuchos::Copy, design_matrix_supp,
	    			      	     int(i-num_candidates_in));
	Teuchos::setCol(col_vec, i, design_matrix);
      }
    }
    else if (num_candidates_in == num_candidates)  
      design_matrix = design_matrix_in;
    else {
      if (outputLevel >= VERBOSE_OUTPUT) {
	Cout << "\nWarning: Bayesian design of experiments only using the "
	     << "first " << num_candidates << " candidates in " 
	     << importCandPtsFile << '\n';
      }
      design_matrix.shape(num_designvars, num_candidates);
      for (int i = 0; i < num_candidates; i++) {
	RealVector col_vec = Teuchos::getCol(Teuchos::Copy, design_matrix_in, 
	    				     i);
	Teuchos::setCol(col_vec, i, design_matrix);
      }
    }
  }
  
  //RealVectorArray std_deviations;
  //expData.cov_std_deviation(std_deviations);
 
  bool stop_metric = false;
  size_t optimal_ind;
  //RealVector optimal_config;
  double max_MI;
  double prev_MI;
  double MIdiff;
  double MIrel;
  int max_hifi = (maxHifiEvals > -1.) ? maxHifiEvals : num_candidates;
  int num_hifi = 0;

  std::ofstream out_file("experimental_design_output.txt");

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Design Matrix   " << design_matrix << '\n';
    Cout << "Max high-fidelity model runs = " << max_hifi << "\n\n";
  }

  while (!stop_metric) {
    
    // EVALUATE STOPPING CRITERIA
    // check relative MI change
    if (num_hifi == 1)
      prev_MI = max_MI;
    else if (num_hifi > 1) {
      MIdiff = prev_MI - max_MI;
      MIrel = fabs(MIdiff/prev_MI);
      if (MIrel < 0.05) {
        stop_metric = true;
        Cout << "Experimental Design Stop Criteria met: "
	     << "Relative change in mutual information is \n"
	     << "sufficiently small \n" 
	     << '\n';
      }
      else
        prev_MI = max_MI;
    }
    // check remaining number of candidates
    if (num_candidates == 0) {
      stop_metric = true;
      Cout << "Experimental Design Stop Criteria met: "
	   << "Design candidates have been exhausted \n" 
	   << '\n';
    }
    // check number of hifi evaluations
    if (num_hifi == max_hifi) {
      stop_metric = true;
      Cout << "Experimental Design Stop Criteria met: "
	   << "Maximum number of hifi evaluations has \n"
	   << "been reached \n" 
	   << '\n';
    }

    // If the experiment data changed, need to update a number of
    // models that wrap it.  TODO: make this more lightweight instead
    // of reconstructing

    // BMA TODO: this doesn't permit use of hyperparameters (see main ctor)
    mcmcModel.continuous_variables(initial_point);
    residualModel.assign_rep
      (new DataTransformModel(mcmcModel, expData, numHyperparams, 
			      obsErrorMultiplierMode, mcmcDerivOrder), false);
    construct_map_optimizer();

    // Run the underlying calibration solver (MCMC)
    calibrate();

    if (outputLevel >= DEBUG_OUTPUT) {
      // Print chain moments
      StringArray combined_labels;
      copy_data(residualModel.continuous_variable_labels(), 
  	combined_labels);
      NonDSampling::print_moments(Cout, chainStats, RealMatrix(), 
	  "posterior variable", STANDARD_MOMENTS, combined_labels, false); 
      // Print response moments
      StringArray resp_labels = mcmcModel.current_response().function_labels();
      NonDSampling::print_moments(Cout, fnStats, RealMatrix(), 
          "response function", STANDARD_MOMENTS, resp_labels, false); 
    }

    if (!stop_metric || max_hifi == 0) {

      if (outputLevel >= DEBUG_OUTPUT) {
	Cout << "\n----------------------------------------------\n";
        Cout << "Begin Experimental Design Iteration " << num_hifi+1;
	Cout << "\n----------------------------------------------\n";
      }

      // After QUESO is run, get the posterior values of the samples; go
      // through all the designs and pick the one with maximum mutual
      // information
  
      // Filter posterior, aim for 5000 samples
      int num_mcmc_samples = acceptanceChain.numCols();
      int burn_in_post = int(0.2*num_mcmc_samples);
      int burned_in_post = num_mcmc_samples - burn_in_post;
      int num_skip;
      int num_filtered;
      int ind = 0;
      int it_cntr = 0;
      if (num_mcmc_samples < 18750) {
        num_skip = 3;
      }
      else {
        num_skip = int(burned_in_post/5000);
      }
      num_filtered = int(burned_in_post/num_skip);
      RealVector lofi_params(numContinuousVars);
      RealMatrix mi_chain(acceptanceChain.numRows(), num_filtered);
      for (int j=burn_in_post; j<num_mcmc_samples; j++) {
        ++it_cntr;
        if (it_cntr % num_skip == 0){
          lofi_params = Teuchos::getCol(Teuchos::View, acceptanceChain, j); 
          Teuchos::setCol(lofi_params, ind, mi_chain);
          ind++;
        }
      }

      // BMA: You can now use acceptanceChain/acceptedFnVals, though
      // need to be careful about what subset for this chain run (may
      // need indices to track)
      for (size_t i=0; i<num_candidates; i++) {
  
        RealVector xi_i = Teuchos::getCol(Teuchos::View, design_matrix, int(i));
        Model::inactive_variables(xi_i, mcmcModel);
  
        // BMA: The low fidelity model can now be referred to as
        // mcmcModel (it may be wrapped in a surrogate, but I think
        // that's what we want if the user said "emulator")
  
        // Declare a matrix to store the low fidelity responses
        //RealMatrix lofi_resp_mat(numFunctions, num_filtered);
        RealVector lofi_resp_vec(numFunctions);
        RealVector col_vec(numContinuousVars + numFunctions);
        RealMatrix Xmatrix(numContinuousVars + numFunctions, num_filtered);
        for (int j=0; j<num_filtered; j++) {
          // for each posterior sample, get the param values, and run the model
          lofi_params = Teuchos::getCol(Teuchos::View, mi_chain, j);
    	  mcmcModel.continuous_variables(lofi_params);
    	  mcmcModel.evaluate();
  
	  lofi_resp_vec = mcmcModel.current_response().function_values();
 	  //Teuchos::setCol(lofi_resp_vec, j, lofi_resp_mat);
          //concatenate posterior_theta and lofi_resp_mat into Xmatrix
          for (size_t k = 0; k < numContinuousVars; k++) {
            col_vec[k] = lofi_params[k];
          }
          for (size_t k = 0; k < numFunctions; k ++) {
            col_vec[numContinuousVars+k] = lofi_resp_vec[k];
          }
          Teuchos::setCol(col_vec, j, Xmatrix);
        }
        // calculate the mutual information b/w post theta and lofi responses
        Real MI = knn_mutual_info(Xmatrix, numContinuousVars, numFunctions);
	if (outputLevel >= DEBUG_OUTPUT) {
	  Cout << "\n----------------------------------------------\n";
          Cout << "Experimental Design Iteration "<<num_hifi+1<<" Progress";
	  Cout << "\n----------------------------------------------\n";
	  Cout << "Design candidate " << i << " = " << xi_i;
	  Cout << "Mutual Information = " << MI << '\n'; 
	}
  
        // Now track max MI:
        if (i == 0) {
	  max_MI = MI;
	  optimal_ind = i;
        }
        else {
          if ( MI > max_MI) {
            max_MI = MI;
	    optimal_ind = i;
          }
        }
      } // end for over the number of candidates
  
      // RUN HIFI MODEL WITH NEW POINT
      // TODO: add multiple points up to concurrency
      RealVector optimal_config = Teuchos::getCol(Teuchos::Copy, design_matrix, 
    					         int(optimal_ind));
      Model::active_variables(optimal_config, hifiModel);
      if (max_hifi > 0) 
        hifiModel.evaluate();
      expData.add_data(optimal_config, hifiModel.current_response().copy());
      num_hifi++;
      // update list of candidates
      remove_column(design_matrix, optimal_ind);
      --num_candidates;

      if (outputLevel >= VERBOSE_OUTPUT) {
	Cout << "\n----------------------------------------------\n";
        Cout << "Experimental Design Iteration " << num_hifi << " Complete";
	Cout << "\n----------------------------------------------\n";
	Cout << "Optimal design: " << optimal_config;
	Cout << "Mutual information = " << max_MI << '\n';
	Cout << "\n";
      }
      out_file << "ITERATION " << num_hifi << "\n";
      out_file << "Optimal Design: " << optimal_config;
      out_file << "Mutual Information = " << max_MI << '\n';
      if (max_hifi > 0) 
        out_file << "Hifi Response: " << hifiModel.current_response();
      out_file << "\n";
    } // end MI loop
  } // end while loop

}

void NonDBayesCalibration::build_model_discrepancy()
{
  // For now, use average params (unfiltered)
  RealMatrix acc_chain_transpose(acceptanceChain, Teuchos::TRANS);
  int num_cols = acc_chain_transpose.numCols();
  RealVector ave_params(num_cols);
  compute_col_means(acc_chain_transpose, ave_params); 
  mcmcModel.continuous_variables(ave_params);
  
  int num_exp = expData.num_experiments();
  size_t num_configvars = expData.config_vars()[0].length();
  RealMatrix allConfigInputs(num_configvars,num_exp);
  for (int i = 0; i < num_exp; i++) {
    RealVector config_vec = expData.config_vars()[i];
    Teuchos::setCol(config_vec, i, allConfigInputs);
  } 

  // Initialize DiscrepancyCorrection class
  IntSet fn_indices;
  // Hardcode for now, call id_surrogates eventually? See SurrogateModel.cpp 
  for (int i = 0; i < numFunctions; ++i)
    fn_indices.insert(i);
  DiscrepancyCorrection modelDisc;
  short corr_type = ADDITIVE_CORRECTION; 
  modelDisc.initialize(fn_indices, numFunctions, num_configvars, corr_type, 
       		       approxCorrectionOrder, discrepancyType);

  // Construct config var information
  Variables vars_copy = mcmcModel.current_variables().copy();
  std::pair<short, short> view(MIXED_STATE, EMPTY_VIEW);
  SizetArray vars_comps_totals(NUM_VC_TOTALS, 0);
  vars_comps_totals = mcmcModel.current_variables().shared_data().
    		      inactive_components_totals();
  SharedVariablesData svd(view, vars_comps_totals);
  Variables configvars(svd);
  VariablesArray configvar_array(num_exp);
  for (int i=0; i<num_exp; i++) {
    const RealVector& config_i = Teuchos::getCol(Teuchos::View, 
				 allConfigInputs, i);
    Model::inactive_variables(config_i, mcmcModel, vars_copy);
    configvars.continuous_variables(vars_copy.inactive_continuous_variables());
    configvars.discrete_int_variables(vars_copy.
				      inactive_discrete_int_variables());
    configvars.discrete_real_variables(vars_copy.
				       inactive_discrete_real_variables());
    configvar_array[i] = configvars.copy();
  }

  // Construct response information from expData and model 
  ResponseArray simresponse_array(num_exp);
  ResponseArray expresponse_array(num_exp);
  for (int i = 0; i<num_exp; i++){
    RealVector config_vec = Teuchos::getCol(Teuchos::View, allConfigInputs, 
		 	    i);
    Model::inactive_variables(config_vec, mcmcModel);
    mcmcModel.evaluate();
    simresponse_array[i] = mcmcModel.current_response().copy();
    expresponse_array[i] = expData.response(i);
  }
  //Cout << "sim response array = " << simresponse_array << '\n';
  //Cout << "exp response array = " << expresponse_array << '\n';
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  modelDisc.compute(configvar_array, expresponse_array, simresponse_array, 
      		    quiet_flag);
  
  // Construct config var information for prediction configs   
  int num_pred;
  RealMatrix configpred_mat; 
  RealVector config(1); //KAM: currently assume only one config var
  VariablesArray configpred_array;
  if (!importPredConfigs.empty()) {
    TabularIO::read_data_tabular(importPredConfigs,
				 "user-provided prediction configurations",
				 configpred_mat, num_configvars, 
				 importPredConfigFormat, false);
    num_pred = configpred_mat.numCols();
    configpred_array.resize(num_pred);
    for (int i = 0; i < num_pred; i++) {
      config = Teuchos::getCol(Teuchos::View, configpred_mat, i);
      configvars.continuous_variables(config);
      configpred_array[i] = configvars.copy();
    } 

  }
  else if (!predictionConfigList.empty()) {
    num_pred = predictionConfigList.length(); 
    configpred_array.resize(num_pred);
    configpred_mat.shapeUninitialized(num_configvars, num_pred);
    for (int i = 0; i < num_pred; i++) {
      config = predictionConfigList[i];
      configvars.continuous_variables(config);
      configpred_array[i] = configvars.copy();
      Teuchos::setCol(config, i, configpred_mat);
    } 
  }
  else {
    num_pred = ( numPredConfigs > 0) ? numPredConfigs : 10;
    configpred_array.resize(num_pred);
    configpred_mat.shapeUninitialized(num_configvars, num_pred);
    double config_step = (configUpperBnds[0]-configLowerBnds[0])/(num_pred-1);
    for (int i = 0; i < num_pred; i++){
      config = configLowerBnds[0] + config_step*i;
      configvars.continuous_variables(config);
      configpred_array[i] = configvars.copy();
      Teuchos::setCol(config, i, configpred_mat);
    }
  }

  // Compute dsicrepancy approx and corrected response
  correctedResponses.resize(num_pred);
  discrepancyResponses.resize(num_pred);
  Response zero_response = mcmcModel.current_response().copy();
  for (int i = 0; i < num_pred; i++) {
    for (size_t j = 0; j < numFunctions; j++) 
      zero_response.function_value(0,j);
    RealVector config_vec = Teuchos::getCol(Teuchos::View, configpred_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    mcmcModel.continuous_variables(ave_params); //KAM -delete later
    mcmcModel.evaluate();
    Variables configpred = configpred_array[i];
    Response simresponse_pred = mcmcModel.current_response();
    Cout << "Calculating model discrepancy";
    modelDisc.apply(configpred, zero_response, quiet_flag);
    discrepancyResponses[i] = zero_response.copy();
    Cout << "Correcting model response";
    modelDisc.apply(configpred, simresponse_pred, quiet_flag);
    correctedResponses[i] = simresponse_pred.copy();
  } 
  
  // Compute correction variance 
  RealMatrix discrep_var(num_pred, numFunctions);
  correctedVariances.shapeUninitialized(num_pred, numFunctions);
  modelDisc.compute_variance(configpred_array, discrep_var, quiet_flag);
  if (expData.variance_active()) {
    RealVectorArray exp_stddevs(num_exp*numFunctions);
    expData.cov_std_deviation(exp_stddevs); // one vector per experiment
    RealVector col_vec(num_pred);
    for (int i = 0; i < numFunctions; i++) {
      Real& max_var = exp_stddevs[0][i];
      for (int j = 0; j < num_exp; j++) 
       if (exp_stddevs[j][i] > max_var)
	 max_var = exp_stddevs[j][i];
      RealVector discrep_varvec = Teuchos::getCol(Teuchos::View, discrep_var,i);
      for (int j = 0; j < num_pred; j++) {
    	col_vec[j] = discrep_varvec[j] + pow(max_var, 2);
      }
      Teuchos::setCol(col_vec, i, correctedVariances);
    }
  }
  else {
    correctedVariances = discrep_var;
    Cout << "\nWarning: No variance information was provided in " 
         << scalarDataFilename << ".\n         Prediction variance computed "
	 << "contains only variance information\n         from the " 
	 << "discrepancy model.\n";
  }
  
  export_discrepancy(configpred_mat);
}

void NonDBayesCalibration::export_discrepancy(RealMatrix& 
    			   pred_config_mat)
{

  // Calculate number of predictions
  int num_pred = pred_config_mat.numCols();
  Variables output_vars = mcmcModel.current_variables().copy(); 
  const StringArray& resp_labels = 
    		     mcmcModel.current_response().function_labels();
  size_t wpp4 = write_precision+4;

  // Discrepancy responses file output
  unsigned short discrep_format = exportDiscrepFormat;
  String discrep_filename = 
    exportDiscrepFile.empty() ? "dakota_discrepancy_tabular.dat" : 
    exportDiscrepFile;
  std::ofstream discrep_stream;
  TabularIO::open_file(discrep_stream, discrep_filename, 
      		       "NonDBayesCalibration discrepancy response export");

  TabularIO::write_header_tabular(discrep_stream, output_vars, resp_labels, 
      				  "config_id", discrep_format);
  discrep_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(discrep_stream, i+1, 
				     mcmcModel.interface_id(), 
				     discrep_format);
    const RealVector& config_vec = Teuchos::getCol(Teuchos::View, 
						   pred_config_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    output_vars = mcmcModel.current_variables().copy();
    output_vars.write_tabular(discrep_stream);
    const RealVector& resp_vec = discrepancyResponses[i].function_values();
    for (size_t j = 0; j < numFunctions; ++j) 
      discrep_stream << std::setw(wpp4) << resp_vec[j] << ' ';
    discrep_stream << '\n';
  }
  TabularIO::close_file(discrep_stream, discrep_filename, 
      	 		"NonDBayesCalibration discrepancy response export");

  // Corrected model (model+discrep) file output
  unsigned short corrmodel_format = exportCorrModelFormat;
  String corrmodel_filename = 
    exportCorrModelFile.empty() ? "dakota_corrected_model_tabular.dat" : 
    exportCorrModelFile;
  std::ofstream corrmodel_stream;
  TabularIO::open_file(corrmodel_stream, corrmodel_filename, 
      		       "NonDBayesCalibration corrected model response export");

  TabularIO::write_header_tabular(corrmodel_stream, output_vars, resp_labels, 
      				  "config_id", corrmodel_format);
  corrmodel_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(corrmodel_stream, i+1, 
				     mcmcModel.interface_id(), 
				     corrmodel_format);
    const RealVector& config_vec = Teuchos::getCol(Teuchos::View, 
						   pred_config_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    output_vars = mcmcModel.current_variables().copy();
    output_vars.write_tabular(corrmodel_stream);
    const RealVector& resp_vec = correctedResponses[i].function_values();
    for (size_t j = 0; j < numFunctions; ++j) 
      corrmodel_stream << std::setw(wpp4) << resp_vec[j] << ' ';
    corrmodel_stream << '\n';
  }
  TabularIO::close_file(corrmodel_stream, corrmodel_filename, 
      	 		"NonDBayesCalibration corrected model response export");

  // Corrected model variances file output
  unsigned short discrepvar_format = exportCorrVarFormat;
  String var_filename = exportCorrVarFile.empty() ? 
    		 "dakota_discrepancy_variance_tabular.dat" : exportCorrVarFile;
  //discrep_filename = "dakota_corrected_variances.dat";
  std::ofstream discrepvar_stream;
  TabularIO::open_file(discrepvar_stream, var_filename, 
      		       "NonDBayesCalibration corrected model variance export");

  RealMatrix corrected_var_transpose(correctedVariances, Teuchos::TRANS);
  StringArray var_labels(numFunctions);
  for (int i = 0; i < numFunctions; i++) {
    std::stringstream s;
    s << resp_labels[i] << "_var";
    var_labels[i] = s.str();
  }
  TabularIO::write_header_tabular(discrepvar_stream, output_vars, var_labels, 
      				  "pred_config", discrepvar_format);
  discrepvar_stream << std::setprecision(write_precision)
    		 << std::resetiosflags(std::ios::floatfield);
  for (int i = 0; i < num_pred; ++i) {
    TabularIO::write_leading_columns(discrepvar_stream, i+1, 
				     mcmcModel.interface_id(), 
				     discrepvar_format);
    const RealVector& config_vec = Teuchos::getCol(Teuchos::View, 
						   pred_config_mat, i);
    Model::inactive_variables(config_vec, mcmcModel);
    output_vars = mcmcModel.current_variables().copy();
    output_vars.write_tabular(discrepvar_stream);
    const RealVector& var_vec = Teuchos::getCol(Teuchos::View, 
						corrected_var_transpose, i);
    for (size_t j = 0; j < numFunctions; ++j) 
      discrepvar_stream << std::setw(wpp4) << var_vec[j] << ' ';
    discrepvar_stream << '\n';
  }
  TabularIO::close_file(discrepvar_stream, var_filename, 
      	 		"NonDBayesCalibration corrected model variance export");
}

void NonDBayesCalibration::
extract_selected_posterior_samples(const std::vector<int> &points_to_keep,
				   const RealMatrix &samples_for_posterior_eval,
				   const RealVector &posterior_density,
				   RealMatrix &posterior_data ) const 
{
}

void NonDBayesCalibration::
export_posterior_samples_to_file( const std::string filename,
				  const RealMatrix &posterior_data ) const
{
}




/** Calculate the log-likelihood, accounting for contributions from
    covariance and hyperparameters, as well as constant term:

      log(L) = -1/2*Nr*log(2*pi) - 1/2*log(det(Cov)) - 1/2*r'(Cov^{-1})*r

    The passed residuals must already be size-adjusted, differenced
    with any data, if present, and scaled by covariance^{-1/2}. */
Real NonDBayesCalibration::
log_likelihood(const RealVector& residuals, const RealVector& all_params)
{
  // if needed, extract the trailing hyper-parameters
  RealVector hyper_params;
  if (numHyperparams > 0)
    hyper_params = RealVector(Teuchos::View, 
                              all_params.values() + numContinuousVars, 
                              numHyperparams);

  size_t num_total_calib_terms = residuals.length();
  Real half_nr_log2pi = num_total_calib_terms * HALF_LOG_2PI;
  Real half_log_det = 
    expData.half_log_cov_determinant(hyper_params, obsErrorMultiplierMode);

  // misfit defined as 1/2 r^T (mult^2*Gamma_d)^{-1} r
  Real misfit = residuals.dot( residuals ) / 2.0;

  Real log_like = -half_nr_log2pi - half_log_det - misfit;

  return log_like;
}


void NonDBayesCalibration::prior_cholesky_factorization()
{
  // factorization to be performed offline (init time) and used online
  int i, j, num_params = numContinuousVars + numHyperparams;
  priorCovCholFactor.shape(num_params, num_params); // init to 0

  if (!standardizedSpace && natafTransform.x_correlation()) {
    Teuchos::SerialSpdDenseSolver<int, Real> corr_solver;
    RealSymMatrix prior_cov_matrix;//= ();

    Cerr << "prior_cholesky_factorization() not yet implmented for this case."
	 << std::endl;
    abort_handler(-1);

    corr_solver.setMatrix( Teuchos::rcp(&prior_cov_matrix, false) );
    corr_solver.factor(); // Cholesky factorization (LL^T) in place
    // assign lower triangle
    for (i=0; i<num_params; ++i)
      for (j=0; j<=i; ++j)
	priorCovCholFactor(i, j) = prior_cov_matrix(i, j);
  }
  else {
    RealRealPairArray dist_moments = (standardizedSpace) ?
      natafTransform.u_moments() : natafTransform.x_moments();
    for (i=0; i<numContinuousVars; ++i)
      priorCovCholFactor(i,i) = dist_moments[i].second;
    // for now we assume a variance when the inv gamma has infinite moments
    for (i=0; i<numHyperparams; ++i)
      if (invGammaDists[i].parameter(Pecos::IGA_ALPHA) > 2.0)
        priorCovCholFactor(numContinuousVars + i, numContinuousVars + i) = 
          invGammaDists[i].standard_deviation();
      else
        priorCovCholFactor(numContinuousVars + i, numContinuousVars + i) =
          0.05*(invGammaDists[i].mode());
  }
}


void NonDBayesCalibration::
get_positive_definite_covariance_from_hessian(const RealSymMatrix &hessian,
					      const RealMatrix& prior_chol_fact,
					      RealSymMatrix &covariance,
					      short output_lev)
{
  // precondition H_misfit by computing L^T H_misfit L where L is the Cholesky
  // factor of the prior covariance.  Important notes:
  // > in other contexts, we compute the Hessian of the negative log posterior
  //   from the Hessian of the misfit minus the Hessian of log prior density.
  // > In the context of defining a MVN covariance, we use the fact that the
  //   Hessian of the negative log of a normal density is 1 / variance, such
  //   that the inverse Hessian is simply the prior covariance.
  //   >> Thus, we justify use of the prior covariance, even for cases (e.g.,
  //      uniform, exp) where the Hessian of the log prior density is zero.
  // > For uncorrelated priors, L is simply diag[sigma_i].

  // Option 1: if augmenting with Hessian of negative log prior
  //           Hess of neg log posterior = Hess of misfit - Hess of log prior
  //const RealVector& c_vars = mcmcModel.continuous_variables();
  //augment_hessian_with_log_prior(log_hess, c_vars);

  // Option 2: if preconditioning with prior covariance using L^T H L
  // can use incoming Hessian as both input and output since an internal
  // temporary is created for H L prior to creating symmetric L^T H L
  int num_rows = hessian.numRows();
  RealSymMatrix LT_H_L(num_rows, false); // copy
  Teuchos::symMatTripleProduct(Teuchos::TRANS, 1., hessian,
			       prior_chol_fact, LT_H_L);

  // Compute eigenvalue decomposition of matrix A=QDQ'
  // In general, the eigenvalue decomposition of a matrix is A=QDinv(Q).
  // For real symmetric matrices A = QDQ', i.e. inv(Q) = Q'
  RealVector eigenvalues; RealMatrix eigenvectors;
  symmetric_eigenvalue_decomposition( LT_H_L, eigenvalues, eigenvectors );

  // Find smallest positive eigenvalue
  //Real min_eigval = std::numeric_limits<double>::max();
  //for ( int i=0; i<num_rows; i++)
  //  if ( eigenvalues[i] > 0. )
  //    min_eigval = std::min( eigenvalues[i], min_eigval );

#ifdef DEBUG
  Cout << "eigenvalues from symmetric_eigenvalue_decomposition:\n";
  write_data(Cout, eigenvalues);
#endif

  /*
  // Ensure hessian is positive definite by setting all negative eigenvalues 
  // to be positive.
  Real eigenval_tol = 1.e-4; // TO DO: tie to prior bounds?
  int i, j, num_neglect = 0;
  for (i=0; i<num_rows; ++i)
    if ( eigenvalues[i] < eigenval_tol )
      { eigenvalues[i] = eigenval_tol; ++num_neglect; }
    else
      break;

  // The covariance matrix is the inverse of the hessian so scale eigenvectors
  // by Q*inv(D)
  RealMatrix scaled_eigenvectors(num_rows, num_rows, false); // don't init to 0
  for ( i=0; i<num_rows; ++i ) {
    for ( j=0; j<num_neglect; ++j )
      scaled_eigenvectors(i,j) = 0.;
    for ( j=num_neglect; j<num_rows; ++j )
      scaled_eigenvectors(i,j) = eigenvectors(i,j) / eigenvalues[j];
  }
  covariance.shapeUninitialized( num_rows );
  covariance.multiply( Teuchos::NO_TRANS, Teuchos::TRANS, 
		       1., scaled_eigenvectors, eigenvectors, 0. );
  */

  // Form V and D
  Real eigenval_tol = 0.; //1.e-4; // Petra2014 suggests tol=1 in Fig 5.2
  int n, r, num_neglect = 0;
  for (n=0; n<num_rows; ++n)
    if ( eigenvalues[n] <= eigenval_tol ) ++num_neglect;
    else                                  break;
  int num_low_rank = num_rows - num_neglect, offset_r;
  RealSymMatrix D(num_low_rank); // init to 0;    r x r diagonal matrix
  RealMatrix V(num_rows, num_low_rank, false); // n x r matrix for r retained
  for (r=0; r<num_low_rank; ++r) {
    offset_r = r + num_neglect;
    Real lambda = eigenvalues[offset_r];
    D(r,r) = lambda / (lambda + 1.); // Sherman-Morrison-Woodbury
    for (n=0; n<num_rows; ++n)
      V(n,r) = eigenvectors(n,offset_r); // copy column
  }

  // Form I - V D V^T
  covariance.shapeUninitialized(num_rows);
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, -1., D, V, covariance);
  for (n=0; n<num_rows; ++n)
    covariance(n,n) += 1.;

  // form inv(hessian) = L (I - V D V^T) L^T
  // can use covariance as both input and output (see above)
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, 1., covariance,
			       prior_chol_fact, covariance);

  if (output_lev > NORMAL_OUTPUT) {
    Cout << "Hessian of negative log-likelihood (from misfit):\n";
    write_data(Cout, hessian, true, true, true);
    //Cout << "2x2 determinant = " << hessian(0,0) * hessian(1,1) -
    //  hessian(0,1) * hessian(1,0) << '\n';

    //Cout << "Cholesky factor of prior covariance:\n";
    //write_data(Cout, prior_chol_fact, true, true, true);

    Cout << "Prior-preconditioned misfit Hessian:\n";
    write_data(Cout, LT_H_L, true, true, true);
    //Cout << "2x2 determinant = " << LT_H_L(0,0) * LT_H_L(1,1) -
    //  LT_H_L(0,1) * LT_H_L(1,0) << '\n';
    if (num_neglect)
      Cout << "Hessian decomposition neglects " << num_neglect
           << " eigenvalues based on " << eigenval_tol << " tolerance.\n";
  }
  if (output_lev >= NORMAL_OUTPUT) {
    Cout << "Positive definite covariance from inverse of Hessian:\n";
    write_data(Cout, covariance, true, true, true);
    //Cout << "2x2 determinant = " << covariance(0,0) * covariance(1,1) -
    //  covariance(0,1) * covariance(1,0) << '\n';
  }

  //return num_neglect;
}


/** Response mapping callback used within RecastModel for MAP
    pre-solve. Computes 

      -log(post) = -log(like) - log(prior); where
      -log(like) = 1/2*Nr*log(2*pi) + 1/2*log(det(Cov)) + 1/2*r'(Cov^{-1})*r
                 = 1/2*Nr*log(2*pi) + 1/2*log(det(Cov)) + misfit

    (misfit defined as 1/2 r^T (mult^2*Gamma_d)^{-1} r) The passed
    residual_resp has been differenced, interpolated, and
    covariance-scaled */
void NonDBayesCalibration::
neg_log_post_resp_mapping(const Variables& residual_vars,
                          const Variables& nlpost_vars,
                          const Response& residual_resp,
                          Response& nlpost_resp)
{
  const RealVector& c_vars = nlpost_vars.continuous_variables();
  short nlpost_req = nlpost_resp.active_set_request_vector()[0];
  bool output_flag = (nonDBayesInstance->outputLevel >= DEBUG_OUTPUT);
  // if needed, extract the trailing hyper-parameters
  RealVector hyper_params;
  if (nonDBayesInstance->numHyperparams > 0)
    hyper_params = 
      RealVector(Teuchos::View, 
		 c_vars.values() + nonDBayesInstance->numContinuousVars, 
		 nonDBayesInstance->numHyperparams);

  if (nlpost_req & 1) {
    const RealVector& residuals = residual_resp.function_values();
    Real nlp = -nonDBayesInstance->log_likelihood(residuals, c_vars) - 
      nonDBayesInstance->log_prior_density(c_vars);
    nlpost_resp.function_value(nlp, 0);
    if (output_flag)
      Cout << "MAP pre-solve: negative log posterior = " << nlp << std::endl;
  }

  if (nlpost_req & 2) {
    // avoid copy by updating gradient vector in place
    RealVector log_grad = nlpost_resp.function_gradient_view(0);
    // Gradient contribution from misfit
    nonDBayesInstance->
      expData.build_gradient_of_sum_square_residuals(residual_resp, log_grad);
    // Add the contribution from 1/2*log(det(Cov))
    nonDBayesInstance->expData.half_log_cov_det_gradient
      (hyper_params, nonDBayesInstance->obsErrorMultiplierMode, 
       nonDBayesInstance->numContinuousVars, log_grad);
    // Add the contribution from -log(prior)
    nonDBayesInstance->augment_gradient_with_log_prior(log_grad, c_vars);
    if (output_flag) {
      Cout << "MAP pre-solve: negative log posterior gradient:\n";
      write_data(Cout, log_grad);
    }
  }

  if (nlpost_req & 4) {
    // avoid copy by updating Hessian matrix in place
    RealSymMatrix log_hess = nlpost_resp.function_hessian_view(0);
    // Hessian contribution from misfit
    nonDBayesInstance->
      expData.build_hessian_of_sum_square_residuals(residual_resp, log_hess);
    // Add the contribution from 1/2*log(det(Cov))
    nonDBayesInstance->expData.half_log_cov_det_hessian
      (hyper_params, nonDBayesInstance->obsErrorMultiplierMode, 
       nonDBayesInstance->numContinuousVars, log_hess);
    // Add the contribution from -log(prior)
    nonDBayesInstance->augment_hessian_with_log_prior(log_hess, c_vars);
    if (output_flag) {
      Cout << "MAP pre-solve: negative log posterior Hessian:\n";
      write_data(Cout, log_hess);
    }
  }

  //Cout << "nlpost_resp:\n" << nlpost_resp;
}

void NonDBayesCalibration::compute_statistics()
{
  // mcmcchain is either acceptanceChain or filtered_chain
  // mcmcfnvals is either acceptedFnVals or filteredFnVals
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_samples = acceptanceChain.numCols();
  int num_filtered = int((num_samples-burnin)/num_skip);

  RealMatrix filtered_chain;
  if (burnInSamples > 0 || num_skip > 1) {

    filtered_chain.shapeUninitialized(acceptanceChain.numRows(), num_filtered);
    filter_chain(acceptanceChain, filtered_chain);

    filteredFnVals.shapeUninitialized(acceptedFnVals.numRows(), num_filtered);
    filter_fnvals(acceptedFnVals, filteredFnVals);

  }
  else {

    filtered_chain = 
      RealMatrix(Teuchos::View, acceptanceChain.values(), 
		 acceptanceChain.stride(), 
		 acceptanceChain.numRows(), acceptanceChain.numCols());

    filteredFnVals = 
      RealMatrix(Teuchos::View, acceptedFnVals.values(), acceptedFnVals.stride(),
		 acceptedFnVals.numRows(), acceptedFnVals.numCols());

  }

  NonDSampling::compute_moments(filtered_chain, chainStats, STANDARD_MOMENTS);
  NonDSampling::compute_moments(filteredFnVals,    fnStats, STANDARD_MOMENTS);
  if (outputLevel >= NORMAL_OUTPUT)
    compute_intervals();

  // Print tabular file for the filtered chain
  if (!exportMCMCFilename.empty() || outputLevel >= NORMAL_OUTPUT)
    export_chain(filtered_chain, filteredFnVals);

  if (posteriorStatsKL)
    kl_post_prior(acceptanceChain);
  if (posteriorStatsMutual)
    mutual_info_buildX();
}


void NonDBayesCalibration::filter_chain(RealMatrix& acceptance_chain, 
					RealMatrix& filtered_chain)
{
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int num_samples = acceptance_chain.numCols();
  int j = 0;
  for (int i = burnin; i < num_samples; ++i) {
    if (i % num_skip == 0) {
      RealVector param_vec = Teuchos::getCol(Teuchos::View, 
	  				acceptance_chain, i);
      Teuchos::setCol(param_vec, j, filtered_chain);
      ++j;
    }
  }
}

void NonDBayesCalibration::filter_fnvals(RealMatrix& accepted_fn_vals, 
    					 RealMatrix& filtered_fn_vals)
{
  int burnin = (burnInSamples > 0) ? burnInSamples : 0;
  int num_skip = (subSamplingPeriod > 0) ? subSamplingPeriod : 1;
  int num_samples = accepted_fn_vals.numCols();
  int j = 0;
  for (int i = burnin; i < num_samples; ++i) {
    if (i % num_skip == 0) {
      RealVector col_vec = Teuchos::getCol(Teuchos::View, 
	 	  			  accepted_fn_vals, i);
      Teuchos::setCol(col_vec, j, filtered_fn_vals);
      j++;
     }
   }
}

void NonDBayesCalibration::compute_intervals()
{
  std::ofstream interval_stream("dakota_mcmc_CredPredIntervals.dat");
  std::ostream& screen_stream = Cout;

  // Make accepted function values the rows instead of the columns
  RealMatrix filtered_fn_vals_transpose(filteredFnVals, Teuchos::TRANS);
  // Augment function values with experimental uncertainty for prediction ints
  int num_filtered = filteredFnVals.numCols();
  size_t num_exp = expData.num_experiments();
  size_t num_concatenated = num_exp*num_filtered;

  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  size_t width = write_precision+7;
  
  // Calculate +/- 2sigma credibility intervals
  RealVector Fn_ave(numFunctions), Fn_stdevs(numFunctions),
	     Cred_interval_minima(numFunctions), 
	     Cred_interval_maxima(numFunctions);
  compute_col_means(filtered_fn_vals_transpose, Fn_ave); 
  compute_col_stdevs(filtered_fn_vals_transpose, Fn_ave, Fn_stdevs);
  interval_stream << "Function aves = " <<Fn_ave << '\n';
  interval_stream << "Function st devs = " <<Fn_stdevs << '\n';
  interval_stream << "2 sigma Credibility Intervals\n";
  for(size_t i=0; i<numFunctions; ++i){
    Cred_interval_minima[i] = Fn_ave[i] - 2*Fn_stdevs[i];
    Cred_interval_maxima[i] = Fn_ave[i] + 2*Fn_stdevs[i];
    interval_stream << std::setw(width) << resp[i] << " ";
    interval_stream << Cred_interval_minima[i] << ", " << Cred_interval_maxima[i] 
               << '\n';
  }    
  interval_stream << "\n";
  
  // Calculate +/- 2sigma prediction intervals
  predVals.shapeUninitialized(numFunctions, num_concatenated);
  if (expData.variance_active()) {
    compute_prediction_vals(filteredFnVals, predVals, 
    			    num_filtered, num_exp, num_concatenated);
    RealVector Pred_ave(numFunctions), Pred_stdevs(numFunctions),
	       Pred_interval_minima(numFunctions), 
	       Pred_interval_maxima(numFunctions);
    RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
    compute_col_means(predVals_transpose, Pred_ave);
    compute_col_stdevs(predVals_transpose, Pred_ave, Pred_stdevs);
    interval_stream << "2 sigma Prediction Intervals\n";
    for(size_t i=0; i<numFunctions; ++i){
      Pred_interval_minima[i] = Pred_ave[i] - 2*Pred_stdevs[i];
      Pred_interval_maxima[i] = Pred_ave[i] + 2*Pred_stdevs[i];
      interval_stream << std::setw(width) << resp[i] << " ";
      interval_stream << Pred_interval_minima[i]<< ", "<<Pred_interval_maxima[i] 
	         << '\n';
    }
  }
    interval_stream << "\n";
  // Calculate intervals with sorting - print to screen and interval file
  size_t num_levels = 0;
  for(int i = 0; i < numFunctions; ++i){
    num_levels += requestedProbLevels[i].length();
  }
  if (num_levels > 0){
    print_intervals_file(interval_stream, filtered_fn_vals_transpose, 
      			   predVals, num_filtered, num_concatenated);
  }
#ifdef DEBUG
  if (expData.variance_active())
    write_data(interval_stream, predVals);
#endif
  interval_stream << "acceptedVals = " << acceptedFnVals << '\n';
  interval_stream << "predVals = " << predVals << '\n';
}

void NonDBayesCalibration::compute_prediction_vals
(RealMatrix& filtered_fn_vals, RealMatrix& predVals, 
int num_filtered, size_t num_exp, size_t num_concatenated)
{
  // Read std_dev and correl matrices if specified for experiments
  RealVectorArray std_deviations;
  RealSymMatrixArray correl_matrices;
  //if (calibrationData && expData.variance_active()){
  expData.cov_std_deviation(std_deviations);
  expData.cov_as_correlation(correl_matrices);
  //}
  
  // Augment function values with experimental uncertainty for prediction ints
  // Generate normal errors using LHS
  /*int num_res = residualModel.current_response().num_functions();
    RealVector means_vec(num_res), lower_bnds(num_res), upper_bnds(num_res);
    */
  RealVector means_vec(numFunctions), lower_bnds(numFunctions), 
	     upper_bnds(numFunctions);
  means_vec.putScalar(0.0);
  lower_bnds.putScalar(-DBL_MAX);
  upper_bnds.putScalar(DBL_MAX);
  RealMatrix lhs_normal_samples;
  unsigned short sample_type = SUBMETHOD_LHS;
  short sample_ranks_mode = 0; //IGNORE RANKS
  Pecos::LHSDriver lhsDriver; // the C++ wrapper for the F90 LHS library
  size_t e, s, r, cntr = 0;
  lhsDriver.seed(randomSeed);
  lhsDriver.initialize("lhs", sample_ranks_mode, true);
  for (e=0; e<num_exp; ++e) {
    //int lhs_seed = (randomSeed > 0) ? randomSeed : generate_system_seed();
    lhsDriver.generate_normal_samples(means_vec, std_deviations[e], lower_bnds,
              upper_bnds, num_filtered, correl_matrices[e],lhs_normal_samples);
    for (s=0; s<num_filtered; ++s, ++cntr)
      for (r=0; r<numFunctions; ++r)
	predVals(r,cntr) = filtered_fn_vals(r,s) + lhs_normal_samples(r,s);
  }
}

void NonDBayesCalibration::
compute_col_means(RealMatrix& matrix, RealVector& avg_vals)
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  avg_vals.resize(num_cols);
  
  RealVector ones_vec(num_rows);
  ones_vec.putScalar(1.0);
 
  for(int i=0; i<num_cols; ++i){
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    avg_vals(i) = col_vec.dot(ones_vec)/((Real) num_rows);
  }
}

void NonDBayesCalibration::
compute_col_stdevs(RealMatrix& matrix, RealVector& avg_vals, RealVector& std_devs)
{
  int num_cols = matrix.numCols();
  int num_rows = matrix.numRows();

  std_devs.resize(num_cols);
  RealVector res_vec(num_rows);

  for(int i=0; i<num_cols; ++i){
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View, matrix, i);
    for(int j = 0; j<num_rows; ++j){
      res_vec(j) = col_vec(j) - avg_vals(i);
    }
    std_devs(i) = std::sqrt(res_vec.dot(res_vec)/((Real) num_rows-1));
  }
}

/** Print tabular file with filtered chain, function values, and pred values */
void NonDBayesCalibration::
export_chain(RealMatrix& filtered_chain, RealMatrix& filtered_fn_vals)
{
  String mcmc_filename = 
    exportMCMCFilename.empty() ? "dakota_mcmc_tabular.dat" : exportMCMCFilename;
  std::ofstream export_mcmc_stream;
  TabularIO::open_file(export_mcmc_stream, mcmc_filename,
		       "NonDBayesCalibration chain export");

  // Use a Variables object for proper tabular formatting.
  // The residual model includes hyper-parameters, if present
  Variables output_vars = residualModel.current_variables().copy();

  // When outputting only chain responses
  const StringArray& resp_labels = 
    mcmcModel.current_response().function_labels();
  // When outputting experimental responses
  /*
  size_t num_exp = expData.num_experiments();
  StringArray resp_labels;
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  for (size_t i=0; i<num_exp+1; ++i){
    for (size_t k=0; k<numFunctions; ++k){
      resp_labels.push_back(resp[k]);
    }
  }
  */

  TabularIO::
    write_header_tabular(export_mcmc_stream, output_vars, 
			 resp_labels, "mcmc_id", exportMCMCFormat);

  size_t wpp4 = write_precision+4;
  export_mcmc_stream << std::setprecision(write_precision) 
		     << std::resetiosflags(std::ios::floatfield);

  int num_filtered = filtered_chain.numCols();
  for (int i=0; i<num_filtered; ++i) {
    TabularIO::write_leading_columns(export_mcmc_stream, i+1,
				     mcmcModel.interface_id(),
				     exportMCMCFormat);
    RealVector accept_pt = Teuchos::getCol(Teuchos::View, filtered_chain, i);
    output_vars.continuous_variables(accept_pt);
    output_vars.write_tabular(export_mcmc_stream);
    // Write function values to filtered_tabular
    RealVector col_vec = Teuchos::getCol(Teuchos::View, filtered_fn_vals, i);
    for (size_t j=0; j<numFunctions; ++j){
      export_mcmc_stream << std::setw(wpp4) << col_vec[j] << ' ';
    }      
    // Write predicted values to filtered_tabular
    // When outputting experimental responses 
    /*
    for (size_t j =0; j<num_exp; ++j){
      for (size_t k = 0; k<numFunctions; ++k){
	int col_index = j*num_filtered+i;
        const RealVector& col_vec = Teuchos::getCol(Teuchos::View, 
      				    predVals, col_index);
  	export_mcmc_stream << std::setw(wpp4) << col_vec[k] << ' ';
      }
    }
    */
    export_mcmc_stream << '\n';
  }

  TabularIO::close_file(export_mcmc_stream, mcmc_filename,
			"NonDQUESOBayesCalibration chain export");
}

void NonDBayesCalibration::print_intervals_file
(std::ostream& s, RealMatrix& filteredFnVals_transpose, 
 RealMatrix& predVals, int num_filtered, size_t num_concatenated)
{
  
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  size_t width = write_precision+7;
  double alpha;
  int lower_index;
  int upper_index;
  // Credibility Intervals
  for(int i = 0; i < numFunctions; ++i){
    // Sort function values 
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View,
				filteredFnVals_transpose, i);
    std::sort(col_vec.values(), col_vec.values() + num_filtered);
    // Write intervals
    size_t num_prob_levels = requestedProbLevels[i].length();
    if (num_prob_levels > 0){
      s << "Credibility Intervals for ";
      s << resp[i] << '\n';
      s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
      s << std::setw(width) << ' ' << " ----------------- -----------------\n";
      for (size_t j = 0; j < num_prob_levels; ++j){
        alpha = requestedProbLevels[i][j];
        lower_index = floor(alpha/2*(num_filtered));
        upper_index = num_filtered - lower_index;
        s << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[lower_index] << ' ' << std::setw(width) 
	  << alpha << '\n'
	  << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[upper_index] << ' '<< std::setw(width) 
	  << 1-alpha << '\n'
	  << std::setw(width) << ' ' <<  "        -----             -----\n";
      }
    }
  }
  // Prediction Intervals
  if (expData.variance_active()) {
    RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
    for(int i = 0; i < numFunctions; ++i){
      // Sort function values 
      const RealVector& col_vec1 = Teuchos::getCol(Teuchos::View, 
				   predVals_transpose, i);
      std::sort(col_vec1.values(), col_vec1.values() + num_concatenated);
      // Write intervals
      size_t num_prob_levels = requestedProbLevels[i].length();
      if (num_prob_levels > 0){
        s << "Prediction Intervals for ";
        s << resp[i] << '\n';
        s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
        s << std::setw(width) << ' ' << " ----------------- -----------------\n";
        for (size_t j = 0; j < num_prob_levels; ++j){
          alpha = requestedProbLevels[i][j];
          //s << "alpha = " << alpha << '\n';
          lower_index = floor(alpha/2*(num_concatenated));
          upper_index = num_concatenated - lower_index;
          s << std::setw(width) << ' ' << std::setw(width) 
	    << col_vec1[lower_index] << ' ' << std::setw(width) 
	    << alpha << '\n'
	    << std::setw(width) << ' ' << std::setw(width) 
	    << col_vec1[upper_index] << ' '<< std::setw(width) 
	    << 1-alpha << '\n'
	    << std::setw(width) << ' ' <<  "        -----             -----\n";
        }
      }
    }
  }
}

void NonDBayesCalibration::print_intervals_screen
(std::ostream& s, RealMatrix& filteredFnVals_transpose, 
 RealMatrix& predVals_transpose, int num_filtered)
{
  const StringArray& resp = mcmcModel.current_response().function_labels(); 
  size_t width = write_precision+7;
  double alpha;
  int lower_index;
  int upper_index;
  s << "\n";
  // Credibility Intervals
  for(int i = 0; i < numFunctions; ++i){
    // Sort function values 
    const RealVector& col_vec = Teuchos::getCol(Teuchos::View,
				filteredFnVals_transpose, i);
    std::sort(col_vec.values(), col_vec.values() + num_filtered);
    // Write intervals
    size_t num_prob_levels = requestedProbLevels[i].length();
    if (num_prob_levels > 0){
      s << "Credibility Intervals for ";
      s << resp[i] << '\n';
      s << std::setw(width) << ' ' << " Response Level    Probability Level\n";
      s << std::setw(width) << ' ' << " ----------------- -----------------\n";
      for (size_t j = 0; j < num_prob_levels; ++j){
        alpha = requestedProbLevels[i][j];
        lower_index = floor(alpha/2*(num_filtered));
        upper_index = num_filtered - lower_index;
        s << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[lower_index] << ' ' << std::setw(width) 
	  << alpha << '\n'
	  << std::setw(width) << ' ' << std::setw(width) 
	  << col_vec[upper_index] << ' '<< std::setw(width) 
	  << 1-alpha << '\n';
	  //<< std::setw(width) << ' ' <<  "        -----             -----\n";
      }
    }
  }
  // Prediction Intervals
  if (expData.variance_active() ) {
    size_t num_exp = expData.num_experiments();
    size_t num_concatenated = num_exp*num_filtered;
    for(int i = 0; i < numFunctions; ++i){
      // Sort function values 
      const RealVector& col_vec1 = Teuchos::getCol(Teuchos::View, 
				   predVals_transpose, i);
      std::sort(col_vec1.values(), col_vec1.values() + num_concatenated);
      // Write intervals
      size_t num_prob_levels = requestedProbLevels[i].length();
      if (num_prob_levels > 0){
        s << "Prediction Intervals for ";
        s << resp[i] << '\n';
        s << std::setw(width) <<' '<< " Response Level    Probability Level\n";
        s << std::setw(width) <<' '<< " ----------------- -----------------\n";
        for (size_t j = 0; j < num_prob_levels; ++j){
          alpha = requestedProbLevels[i][j];
          lower_index = floor(alpha/2*(num_concatenated));
          upper_index = num_concatenated - lower_index;
          s << std::setw(width) << ' ' << std::setw(width) 
	    << col_vec1[lower_index] << ' ' << std::setw(width) 
	    << alpha << '\n'
	    << std::setw(width) << ' ' << std::setw(width) 
	    << col_vec1[upper_index] << ' '<< std::setw(width) 
	    << 1-alpha << '\n';
	    //<< std::setw(width) << ' ' <<  "        -----             -----\n";
        }
      }
    }
  }
}

void NonDBayesCalibration::print_results(std::ostream& s)
{
  // Print chain moments
  StringArray combined_labels;
        copy_data(residualModel.continuous_variable_labels(), combined_labels);
  NonDSampling::print_moments(s, chainStats, RealMatrix(), 
      "posterior variable", STANDARD_MOMENTS, combined_labels, false); 
  // Print response moments
  StringArray resp_labels = mcmcModel.current_response().function_labels();
  NonDSampling::print_moments(s, fnStats, RealMatrix(), 
      "response function", STANDARD_MOMENTS, resp_labels, false); 
  
  // Print credibility and prediction intervals to screen
  int num_filtered = filteredFnVals.numCols();
  RealMatrix filteredFnVals_transpose(filteredFnVals, Teuchos::TRANS);
  RealMatrix predVals_transpose(predVals, Teuchos::TRANS);
  print_intervals_screen(s, filteredFnVals_transpose, 
    			 predVals_transpose, num_filtered);

  // Print posterior stats
  if(posteriorStatsKL)
    print_kl(s);
}

void NonDBayesCalibration::kl_post_prior(RealMatrix& acceptanceChain)
{
 
  // sub-sample posterior chain
  int num_params = numContinuousVars + numHyperparams;
  int num_post_samples = acceptanceChain.numCols();
  int burn_in_post = int(0.2*num_post_samples);
  int burned_in_post = num_post_samples - burn_in_post;
  RealMatrix knn_post_samples;
  RealMatrix prior_dist_samples;
  if (num_post_samples < 18750){ // Aiming for num_filtered = 5000 
    int num_skip = 3;
    int num_filtered = int(burned_in_post/num_skip);
    int num_prior_samples = num_filtered*125;
    knn_post_samples.shape(num_params, num_filtered);
    prior_dist_samples.shape(num_params, num_prior_samples);
    int j = 0;
    int it_cntr = 0;
    for (int i = burn_in_post+1; i < num_post_samples; ++i){
      ++it_cntr;
      if (it_cntr % num_skip == 0){
	RealVector param_vec = Teuchos::getCol(Teuchos::View,
	  			        acceptanceChain, i);
	Teuchos::setCol(param_vec, j, knn_post_samples);
	++j;
      }
    }
  }
  else{
    int num_skip = int(burned_in_post/5000);
    int num_filtered = int(burned_in_post/num_skip);
    int num_prior_samples = num_filtered*125;
    knn_post_samples.shapeUninitialized(num_params, num_filtered);
    prior_dist_samples.shapeUninitialized(num_params, num_prior_samples);
    int j = 0;
    int it_cntr = 0;
    for (int i = burn_in_post; i < num_post_samples; ++i){
      if (it_cntr % num_skip == 0){
	  ++it_cntr;
	  RealVector param_vec = Teuchos::getCol(Teuchos::View,
	    			        acceptanceChain, i);
	  Teuchos::setCol(param_vec, j, knn_post_samples);
	  j++;
      }
    }
  }
  
  // produce matrix of prior samples
  prior_sample_matrix(prior_dist_samples);
  // compute knn kl-div between prior and posterior
  kl_est = knn_kl_div(knn_post_samples, prior_dist_samples, numContinuousVars);
}

void NonDBayesCalibration::prior_sample_matrix(RealMatrix& prior_dist_samples)
{
  // Create matrix containing samples from the prior distribution
  boost::mt19937 rnumGenerator;
  int num_params = prior_dist_samples.numRows(); 
  int num_samples = prior_dist_samples.numCols();
  RealVector vec(num_params);
  rnumGenerator.seed(randomSeed);
  for(int i = 0; i < num_samples; ++i){
    prior_sample(rnumGenerator, vec);
    Teuchos::setCol(vec, i, prior_dist_samples);
  }
}

Real NonDBayesCalibration::knn_kl_div(RealMatrix& distX_samples,
    			 	RealMatrix& distY_samples, size_t dim)
{
  approxnn::normSelector::instance().method(approxnn::L2_NORM);

  size_t NX = distX_samples.numCols();
  size_t NY = distY_samples.numCols();
  //size_t dim = numContinuousVars; 
  
  // k is recorded for each distance so that if it needs to be adapted
  // (if kNN dist = 0), we can calculate the correction term 
  IntVector k_vec_XY(NX);
  k_vec_XY.putScalar(6); //k default set to 6
  IntVector k_vec_XX(NX);
  k_vec_XX.putScalar(7); //k default set to 6
  			 //1st neighbor is self, so need k+1 for XtoX
  double eps = 0.0; //default ann error
   
  // Copy distX samples into ANN data types
  ANNpointArray dataX;
  dataX = annAllocPts(NX, dim);
  RealVector col(dim);
  for (int i = 0; i < NX; i++){
    col = Teuchos::getCol(Teuchos::View, distX_samples, i);
    for (int j = 0; j < dim; j++){
      dataX[i][j] = col[j];
    }
  } 
  // Copy distY samples into ANN data types
  ANNpointArray dataY;
  dataY = annAllocPts(NY, dim);
  for (int i = 0; i < NY; i++){
    col = Teuchos::getCol(Teuchos::View, distY_samples, i);
    for (int j = 0; j < dim; j++){
      dataY[i][j] = col[j];
    }
  } 
  
  // calculate vector of kNN distances from dist1 to dist2
  RealVector XtoYdistances(NX);
  ann_dist(dataX, dataY, XtoYdistances, NX, NY, dim, k_vec_XY, eps);
  // calculate vector of kNN distances from dist1 to itself
  RealVector XtoXdistances(NX);
  ann_dist(dataX, dataX, XtoXdistances, NX, NX, dim, k_vec_XX, eps);
  
  double log_sum = 0;
  double digamma_sum = 0;
  double rat;
  for (int i = 0; i < NX; i++){
    rat = XtoYdistances[i]/XtoXdistances[i];
    log_sum += log(XtoYdistances[i]/XtoXdistances[i]);
    if (k_vec_XY[i] != (k_vec_XX[i]-1)){ //XtoX: first NN is self
      double psiX = boost::math::digamma(k_vec_XX[i]-1);
      double psiY = boost::math::digamma(k_vec_XY[i]);
      digamma_sum += psiX - psiY;
    }
  }
  Real Dkl_est = 0.0;
  Dkl_est = (double(dim)*log_sum + digamma_sum)/double(NX)
          + log( double(NY)/(double(NX)-1) );

  annDeallocPts( dataX );
  annDeallocPts( dataY );

  approxnn::normSelector::instance().reset();

  return Dkl_est;
}

void NonDBayesCalibration::mutual_info_buildX()
{
  /* Build matrix X, containing samples of the two distributions being
   * considered in the mutual info calculation. Each column has the form
   * X_i = [x1_i x2_i ... xn_i y1_i y2_i ... ym_i]
   */
   
  int num_params = numContinuousVars + numHyperparams;
  int num_samples = 1000;
  boost::mt19937 rnumGenerator;
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(2*num_params, num_samples);
  RealVector vec(num_params);
  RealVector col_vec(2*num_params);
  rnumGenerator.seed(randomSeed);
  for (int i = 0; i < num_samples; ++i){
    prior_sample(rnumGenerator, vec);
    for (int j = 0; j < num_params; ++j){
      col_vec[j] = vec[j];
    }
    prior_sample(rnumGenerator, vec);
    for (int j = 0; j < num_params; ++j){
      col_vec[j+1] = vec[j];
    }
    Teuchos::setCol(col_vec, i, Xmatrix);
  }

  // Test matrix
  /*
  int num_samples = acceptanceChain.numCols();
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(2*num_params, num_samples);
  RealVector vec(num_params);
  RealVector col_vec(2*num_params);
  for (int i = 0; i < num_samples-1; ++i){
    vec = Teuchos::getCol(Teuchos::View, acceptanceChain, i);
    for (int j = 0; j < num_params; ++j){
      col_vec[j] = vec[j]; //offset values by 1
    }
    vec = Teuchos::getCol(Teuchos::View, acceptanceChain, i+1);
    for (int j = 0; j < num_params; ++j){
      col_vec[j+num_params] = vec[j];
    }
    Teuchos::setCol(col_vec, i, Xmatrix);
  }
  // last column
  vec = Teuchos::getCol(Teuchos::View, acceptanceChain, num_samples-1);
  for (int j = 0; j < num_params; ++j){
    col_vec[j] = vec[j]; //offset values by 1
  }
  vec = Teuchos::getCol(Teuchos::View, acceptanceChain, 0);
  for (int j = 0; j < num_params; ++j){
    col_vec[j+num_params] = vec[j];
  }
  Teuchos::setCol(col_vec, num_samples-1, Xmatrix);
  */

  //test_stream << "Xmatrix = " << Xmatrix << '\n';


  Real mutualinfo_est = knn_mutual_info(Xmatrix, num_params, num_params);
  Cout << "MI est = " << mutualinfo_est << '\n';

}

Real NonDBayesCalibration::knn_mutual_info(RealMatrix& Xmatrix, int dimX,
    int dimY)
{
  approxnn::normSelector::instance().method(approxnn::LINF_NORM);

  //std::ofstream test_stream("kam1.txt");
  //test_stream << "Xmatrix = " << Xmatrix << '\n';
  //Cout << "Xmatrix = " << Xmatrix << '\n';

  int num_samples = Xmatrix.numCols();
  int dim = dimX + dimY;

  // Cast Xmatrix into ANN data structure
  ANNpointArray dataXY;
  dataXY = annAllocPts(num_samples, dim);
  RealVector col(dim);
  for (int i = 0; i < num_samples; i++){
    col = Teuchos::getCol(Teuchos::View, Xmatrix, i);
    for (int j = 0; j < dim; j++){
      dataXY[i][j] = col[j];
    }
  }

  /*
  // Normalize data
  ANNpoint meanXY, stdXY;
  meanXY = annAllocPt(dim); //means
  for (int i = 0; i < num_samples; i++){
    for(int j = 0; j < dim; j++){
      meanXY[j] += dataXY[i][j];
    }
  }
  for (int j = 0; j < dim; j++){
    meanXY[j] = meanXY[j]/double(num_samples);
  }
  stdXY = annAllocPt(dim); //standard deviations
  for (int i = 0; i < num_samples; i++){
    for (int j = 0; j < dim; j++){
      stdXY[j] += pow (dataXY[i][j] - meanXY[j], 2.0);
    }
  }
  for (int j = 0; j < dim; j++){
    stdXY[j] = sqrt( stdXY[j]/(double(num_samples)-1.0) );
  }
  for (int i = 0; i < num_samples; i++){
    for (int j = 0; j < dim; j++){
      dataXY[i][j] = ( dataXY[i][j] - meanXY[j] )/stdXY[j];
    }
  }
  */

  // Get knn-distances for Xmatrix
  RealVector XYdistances(num_samples);
  IntVector k_vec(num_samples);
  int k = 6;
  k_vec.putScalar(k); // for self distances, need k+1
  double eps = 0.0;
  ann_dist(dataXY, dataXY, XYdistances, num_samples, num_samples, dim, 
      	   k_vec, eps);
  
  // Build marginals
  ANNpointArray dataX, dataY;
  dataX = annAllocPts(num_samples, dimX);
  dataY = annAllocPts(num_samples, dimY);
  //RealMatrix chainX(dimX, num_samples);
  //RealMatrix chainY(dimY, num_samples);
  for(int i = 0; i < num_samples; i++){
    col = Teuchos::getCol(Teuchos::View, Xmatrix, i);
    //Cout << "col = " << col << '\n';
    for(int j = 0; j < dimX; j++){
      //dataX[i][j] = dataXY[i][j];
      //Cout << "col " << j << " = " << col[j] << '\n';
      //chainX[i][j] = col[j];
      dataX[i][j] = col[j];
    }
    for(int j = 0; j < dimY; j++){
      //dataY[i][j] = dataXY[i][dimX+j];
      //chainY[i][j] = col[dimX+j];
      dataY[i][j] = col[dimX+j];
    }
  }
  //Cout << "chainX = " << chainX;
  ANNkd_tree* kdTreeX;
  ANNkd_tree* kdTreeY;
  kdTreeX = new ANNkd_tree(dataX, num_samples, dimX);
  kdTreeY = new ANNkd_tree(dataY, num_samples, dimY);

  double marg_sum = 0.0;
  for(int i = 0; i < num_samples; i++){
    int n_x = kdTreeX->annkFRSearch(dataX[i], XYdistances[i], 0, NULL, 
				    NULL, eps);
    int n_y = kdTreeY->annkFRSearch(dataY[i], XYdistances[i], 0, NULL,
				    NULL, eps);
    double psiX = boost::math::digamma(n_x);
    double psiY = boost::math::digamma(n_y);
    //double psiX = boost::math::digamma(n_x+1);
    //double psiY = boost::math::digamma(n_y+1);
    marg_sum += psiX + psiY;
    //test_stream << "i = " << i << ", nx = " << n_x << ", ny = " << n_y << '\n';
    //Cout << "i = " << i << ", nx = " << n_x << ", ny = " << n_y << '\n';
    //test_stream << "psiX = " << psiX << '\n';
    //test_stream << "psiY = " << psiY << '\n';
  }
  double psik = boost::math::digamma(k);
  double psiN = boost::math::digamma(num_samples);
  double MI_est = psik - (marg_sum/double(num_samples)) + psiN;
  //test_stream << "psi_k = " << psik << '\n';
  //test_stream << "marg_sum = " << marg_sum << '\n';
  //test_stream << "psiN = " << psiN << '\n';
  //test_stream << "MI_est = " << MI_est << '\n';

  // Dealloc memory
  delete kdTreeX;
  delete kdTreeY;
  annDeallocPts(dataX);
  annDeallocPts(dataY);
  annDeallocPts(dataXY);

  approxnn::normSelector::instance().reset();

  // Compare to dkl
  /*
  double kl_est = knn_kl_div(Xmatrix, Xmatrix);
  test_stream << "KL = " << kl_est << '\n';
  Cout << "KL = " << kl_est << '\n';
  */
  return MI_est;

}

void NonDBayesCalibration::ann_dist(const ANNpointArray matrix1, 
     const ANNpointArray matrix2, RealVector& distances, int NX, int NY, 
     int dim2, IntVector& k_vec, double eps)
{

  ANNkd_tree* kdTree;
  kdTree = new ANNkd_tree( matrix2, NY, dim2 );
  for (unsigned int i = 0; i < NX; ++i){
    int k_i = k_vec[i] ;
    ANNdistArray knn_dist = new ANNdist[k_i+1];
    ANNidxArray knn_ind = new ANNidx[k_i+1];
    //calc min number of distances needed
    kdTree->annkSearch(matrix1[ i ], k_i+1, knn_ind, knn_dist, eps);
    double dist = knn_dist[k_i];
    if (dist == 0.0){
      ANNdistArray knn_dist_i = new ANNdist[NY];
      ANNidxArray knn_ind_i = new ANNidx[NY];
      //calc distances for whole array
      kdTree->annkSearch(matrix1[ i ], NY, knn_ind_i, knn_dist_i, eps); 
      for (unsigned int j = k_i+1; j < NY; ++j){
	if (knn_dist_i[j] > 0.0){
	  dist = knn_dist_i[j];
	  k_vec[i] = j;
	  break;
	}
      }
      delete [] knn_ind_i;
      delete [] knn_dist_i;
    }
    distances[i] = dist;
    delete [] knn_ind;
    delete [] knn_dist;
  }
  delete kdTree;
  annClose();
}

void NonDBayesCalibration::print_kl(std::ostream& s)
{
  s << "Information gained from prior to posterior = " << kl_est;
  s << '\n';
}

} // namespace Dakota
