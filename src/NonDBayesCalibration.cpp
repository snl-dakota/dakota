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
#include "NonDPolynomialChaos.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDLHSSampling.hpp"
#include "NPSOLOptimizer.hpp"
#include "SNLLOptimizer.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

// initialization of statics

// BMA, LPS: experimentation with hyperparameter prior; if works will
// integrate into Pecos
Real invgamma_alpha = 1.0;
Real invgamma_beta = 1.0;
boost::math::inverse_gamma_distribution<> NonDBayesCalibration::
invgammaDist(invgamma_alpha, invgamma_beta);

NonDBayesCalibration* NonDBayesCalibration::nonDBayesInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDBayesCalibration::
NonDBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDCalibration(problem_db, model),
  emulatorType(probDescDB.get_short("method.nond.emulator")),
  randomSeed(probDescDB.get_int("method.random_seed")),
  obsErrorMultiplierMode(
    probDescDB.get_ushort("method.nond.calibrate_error_mode")),
  numHyperparams(0), unifHyperparams(true),
  adaptPosteriorRefine(
    probDescDB.get_bool("method.nond.adaptive_posterior_refinement")),
  proposalCovarType(
    probDescDB.get_string("method.nond.proposal_covariance_type")),
  proposalCovarData(probDescDB.get_rv("method.nond.proposal_covariance_data")),
  proposalCovarFilename(
    probDescDB.get_string("method.nond.proposal_covariance_filename")),
  proposalCovarInputType(
    probDescDB.get_string("method.nond.proposal_covariance_input_type"))
{
  // assign default proposalCovarType
  if (proposalCovarType.empty()) {
    if (emulatorType) proposalCovarType = "derivatives"; // misfit Hessian
    else              proposalCovarType = "prior";       // prior covariance
  }

  // manage sample partitions and defaults
  int samples_spec = probDescDB.get_int("method.samples");
  if (proposalCovarType == "derivatives") {
    int pc_update_spec
      = probDescDB.get_int("method.nond.proposal_covariance_updates");
    if (pc_update_spec < 1) { // default partition: update every 100 samples
      numSamples  = 100;
      chainCycles = (int)floor((Real)samples_spec / (Real)numSamples + .5);
    }
    else { // partition as specified
      numSamples  = (int)floor((Real)samples_spec / (Real)pc_update_spec + .5);
      chainCycles = pc_update_spec;
    }
  }
  else
    { numSamples = samples_spec; chainCycles = 1; }

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

  // --------------------------------------------------
  // Construct mcmcModel (no emulation, GP, PCE, or SC)
  // for use in likelihood evaluations
  // --------------------------------------------------
  short mcmc_deriv_order = 1;
  switch (emulatorType) {

  case PCE_EMULATOR: case SC_EMULATOR: {
    const UShortArray& level_seq
      = probDescDB.get_usa("method.nond.sparse_grid_level");
    const RealVector& dim_pref
      = probDescDB.get_rv("method.nond.dimension_preference");    // not exposed
    bool derivs = probDescDB.get_bool("method.derivative_usage"); // not exposed
    NonDExpansion* se_rep;
    if (emulatorType == SC_EMULATOR) { // SC sparse grid interpolation
      se_rep = new NonDStochCollocation(iteratedModel,
	Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	false, derivs);
      mcmc_deriv_order = 3; // Hessian computations not yet implemented for SC
    }
    else {
      if (!level_seq.empty()) // PCE with spectral projection via sparse grid
	se_rep = new NonDPolynomialChaos(iteratedModel,
	  Pecos::COMBINED_SPARSE_GRID, level_seq, dim_pref, EXTENDED_U,
	  false, false);
      else { 
        // regression PCE: LeastSq/CS (exp_order,colloc_ratio), OLI (colloc_pts)
	const UShortArray& exp_order_seq
	  = probDescDB.get_usa("method.nond.expansion_order");
	short exp_coeffs_approach = (exp_order_seq.empty()) ?
	  Pecos::ORTHOG_LEAST_INTERPOLATION : Pecos::DEFAULT_REGRESSION;
	se_rep = new NonDPolynomialChaos(iteratedModel,
	  exp_coeffs_approach, exp_order_seq, dim_pref,
	  probDescDB.get_sza("method.nond.collocation_points"), // pts sequence
	  probDescDB.get_real("method.nond.collocation_ratio"), // single scalar
	  randomSeed, EXTENDED_U, false, derivs,	
	  probDescDB.get_bool("method.nond.cross_validation"), // not exposed
	  probDescDB.get_string("method.import_build_points_file"),
	  probDescDB.get_ushort("method.import_build_format"),
	  probDescDB.get_bool("method.import_build_active_only"));
      }
      mcmc_deriv_order = 7; // Hessian computations implemented for PCE
    }
    stochExpIterator.assign_rep(se_rep);
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
    String sample_reuse; String approx_type;
    if (emulatorType == GP_EMULATOR)
      { approx_type = "global_gaussian"; mcmc_deriv_order = 3; } // grad support
    else
      { approx_type = "global_kriging";  mcmc_deriv_order = 7; } // grad,Hess
    UShortArray approx_order; // not used by GP/kriging
    short corr_order = -1, data_order = 1, corr_type = NO_CORRECTION;
    if (probDescDB.get_bool("method.derivative_usage")) {
      // derivatives for emulator construction (not emulator evaluation)
      if (iteratedModel.gradient_type() != "none") data_order |= 2;
      if (iteratedModel.hessian_type()  != "none") data_order |= 4;
    }
    unsigned short sample_type = SUBMETHOD_DEFAULT;
    int samples = probDescDB.get_int("method.nond.emulator_samples");
    // get point samples file
    const String& import_pts_file
      = probDescDB.get_string("method.import_build_points_file");
    if (!import_pts_file.empty())
      { samples = 0; sample_reuse = "all"; }
     
    // Consider elevating lhsSampler from NonDGPMSABayesCalibration:
    Iterator lhs_iterator; Model lhs_model;
    if (standardizedSpace) transform_model(iteratedModel,lhs_model,true);//glbal
    else                   lhs_model = iteratedModel; // shared rep
    NonDLHSSampling* lhs_rep = new
      NonDLHSSampling(lhs_model, sample_type, samples, randomSeed,
        probDescDB.get_string("method.random_number_generator"), true,
	ACTIVE_UNIFORM);
    lhs_iterator.assign_rep(lhs_rep, false);

    // natafTransform is not fully updated at this point, but using
    // a shallow copy allows run time updates to propagate
    if (standardizedSpace)
      lhs_rep->initialize_random_variables(natafTransform); // shallow copy

    ActiveSet gp_set = lhs_model.current_response().active_set(); // copy
    gp_set.request_values(mcmc_deriv_order); // for misfit Hessian
    mcmcModel.assign_rep(new DataFitSurrModel(lhs_iterator, lhs_model,
      gp_set, approx_type, approx_order, corr_type, corr_order, data_order,
      outputLevel, sample_reuse, import_pts_file,
      probDescDB.get_ushort("method.import_build_format"),
      probDescDB.get_bool("method.import_build_active_only")), false);
    break;
  }

  case NO_EMULATOR:
    standardizedSpace = probDescDB.get_bool("method.nond.standardized_space");
    if (standardizedSpace) transform_model(iteratedModel, mcmcModel);// !global
    else                   mcmcModel = iteratedModel; // shared rep

    if (mcmcModel.gradient_type() != "none") mcmc_deriv_order |= 2;
    if (mcmcModel.hessian_type()  != "none") mcmc_deriv_order |= 4;
    break;
  }

  // Initialize sizing for hyperparameters (observation error), not
  // currently part of a RecastModel
  size_t num_resp_groups = 
    mcmcModel.current_response().shared_data().num_response_groups(); 
  if (obsErrorMultiplierMode == CALIBRATE_ONE)
    numHyperparams = 1;
  else if (obsErrorMultiplierMode == CALIBRATE_PER_EXPER)
    numHyperparams = numExperiments;
  else if (obsErrorMultiplierMode == CALIBRATE_PER_RESP)
    numHyperparams = num_resp_groups;
  else if (obsErrorMultiplierMode == CALIBRATE_BOTH)
    numHyperparams = numExperiments * num_resp_groups;

  // -------------------------------------
  // Construct sampler for posterior stats (only in NonDQUESOBayes for now)
  // -------------------------------------
  //RealMatrix acceptance_chain; // empty (allSamples not currently needed)
  //chainStatsSampler.assign_rep(new NonDSampling(mcmcModel, acceptance_chain),
  //			         false);

  // -------------------------------------
  // Construct optimizer for MAP pre-solve
  // -------------------------------------
  unsigned short opt_alg_override
    = probDescDB.get_ushort("method.nond.pre_solve_method");
  if (emulatorType || opt_alg_override) {

    Sizet2DArray vars_map_indices, primary_resp_map_indices(1),
      secondary_resp_map_indices;
    // BMA @ MSE: I believe this needs to be sized correctly based on total
    // residual size, not numFunctions
    primary_resp_map_indices[0].resize(numFunctions);
    for (size_t i=0; i<numFunctions; ++i)
      primary_resp_map_indices[0][i] = i;
    bool nonlinear_vars_map = false; BoolDequeArray nonlinear_resp_map(1);
    nonlinear_resp_map[0] = BoolDeque(numFunctions, true);
    SizetArray recast_vc_totals;  // empty: no change in size
    BitArray all_relax_di, all_relax_dr; // empty: no discrete relaxation

    switch (opt_alg_override) {
    case SUBMETHOD_SQP:
#ifndef HAVE_NPSOL
      Cerr << "\nWarning: this executable not configured with NPSOL SQP."
	   << "\n         MAP pre-solve not available." << std::endl;
      opt_alg_override = SUBMETHOD_DEFAULT; // model,optimizer not constructed
#endif
      break;
    case SUBMETHOD_NIP:
#ifndef HAVE_OPTPP
      Cerr << "\nWarning: this executable not configured with OPT++ NIP."
	   << "\n         MAP pre-solve not available." << std::endl;
      opt_alg_override = SUBMETHOD_DEFAULT; // model,optimizer not constructed
#endif
      break;
    case SUBMETHOD_DEFAULT: // use full Newton, if available
#ifdef HAVE_OPTPP
      opt_alg_override = SUBMETHOD_NIP;
#elif HAVE_NPSOL
      opt_alg_override = SUBMETHOD_SQP;
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
    if (opt_alg_override == SUBMETHOD_NIP) {
      nlp_resp_order = 7; // size RecastModel response for full Newton Hessian
      if (mcmc_deriv_order == 3) // map asrv for Gauss-Newton approx
	set_recast = gnewton_set_recast;
    }

    // RecastModel for bound-constrained argmin(misfit - log prior)
    if (opt_alg_override)
      negLogPostModel.assign_rep(new RecastModel(mcmcModel, vars_map_indices, 
        recast_vc_totals, all_relax_di, all_relax_dr, nonlinear_vars_map, NULL,
        set_recast, primary_resp_map_indices, secondary_resp_map_indices, 0,
        nlp_resp_order, nonlinear_resp_map, neg_log_post_resp_mapping, NULL),
        false);

    switch (opt_alg_override) {
#ifdef HAVE_NPSOL
    case SUBMETHOD_SQP: {
      int npsol_deriv_level = 3;
      mapOptimizer.assign_rep(new NPSOLOptimizer(negLogPostModel,
	npsol_deriv_level, convergenceTol), false); // SQP with BFGS Hessians
      break;
    }
#endif
#ifdef HAVE_OPTPP
    case SUBMETHOD_NIP:
      mapOptimizer.assign_rep(new SNLLOptimizer("optpp_newton",
	negLogPostModel), false); // full Newton (OPTPP::OptBCNewton)
      break;
#endif
    }
  }

  int mcmc_concurrency = 1; // prior to concurrent chains
  maxEvalConcurrency *= mcmc_concurrency;
}


NonDBayesCalibration::~NonDBayesCalibration()
{ }


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
}


void NonDBayesCalibration::
init_residual_response(short request_value_needed)
{
  // The residual response is sized based on total experiment data size.
  // It has to allocate space for derivatives in the case of preconditioned
  // proposal covariance.
  const Response& resp = mcmcModel.current_response();
  residualResponse = resp.copy(false);  // false: SRD can be shared until resize
  size_t total_residuals = 
    calibrationData ? expData.num_total_exppoints() : resp.num_functions();

  // reshape needed if change in size or derivative request; Response
  // only resizes contained data if needed
  residualResponse.reshape(total_residuals,
			   resp.active_set_derivative_vector().size(),
			   request_value_needed & 2, request_value_needed & 4);

  // TODO: fully map the active set vector as in Minimizer, or replace
  // with a RecastModel for data.  For now use the maximal request.
  ShortArray asv = residualResponse.active_set_request_vector();
  asv.assign(asv.size(), request_value_needed);
  residualResponse.active_set_request_vector(asv);
}


/** Converts a passed simulation or emulator response to a full set of
    residuals based on multiple experiments and/or interpolation.
    Called after any compute_response instead of in misfit and
    neg_log_post to avoid complex conditionals on ASV and data flags;
    may later be replaced with RecastModel. */
void NonDBayesCalibration::
update_residual_response(const Response& resp)
{
  if (calibrationData) {
    // BMA: perhaps a better name would be per_exp_asv?
    bool interrogate_field_data = ( expData.variance_type_active(MATRIX_SIGMA)
                                    || expData.interpolate_flag() );
    ShortArray total_asv =  
      expData.determine_active_request(residualResponse,interrogate_field_data);
    expData.form_residuals(resp, total_asv, residualResponse); 

    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Original response:\n" << resp << std::endl;
      Cout << "Residual response:\n" << residualResponse << std::endl;
    }
  }
  else
    residualResponse.update(resp);
}


/** Calculate the log-likelihood, accounting for contributions from
    covariance and hyperparameters, as well as constant term:

      log(L) = -1/2*Nr*log(2*pi) - 1/2*log(det(Cov)) - 1/2*r'(Cov^{-1})*r

    The passed residual_resp must already be differenced with any
    data, if present. */
Real NonDBayesCalibration::
log_likelihood(const Response& residual_resp, const RealVector& hyper_params)
{
  Real half_nr_log2pi = residual_resp.num_functions() * HALF_LOG_2PI;
  // BMA TODO: compute log(det(Cov)) directly as product to avoid overflow
  Real half_log_det = 
    std::log( expData.scaled_cov_determinant(hyper_params, 
					     obsErrorMultiplierMode) ) / 2.0;
  Real log_like = 
    -half_nr_log2pi - half_log_det - misfit(residual_resp, hyper_params);

  return log_like;
}


/** Calculate the misfit portion of the likelihood, accounting for
    optional user-provided observation error covariance and/or
    calibrated multipliers on the errors. The passed residual_resp
    must already be differenced with any data, if present.*/
Real NonDBayesCalibration::
misfit(const Response& residual_resp, const RealVector& hyper_params)
{
  // For now, mirror the flow we'd move to if posing this via nested
  // RecastModels (even though it makes extra copies).  Previously variance 
  // data OR calibrated errors were applied; now they are independent
  
  // On entry to this function, residual_resp contains the expanded,
  // potentially interpolated, set of residuals (from
  // update_residual_response)
  RealVector residuals = residual_resp.function_values();

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Residuals in misfit:\n" << residuals << std::endl;

  // scale by inverse of user-provided observation error
  if (expData.variance_active()) {  // r <- Gamma_d^{-1/2} r
    // BMA: perhaps a better name would be per_exp_asv?
    bool interrogate_field_data = ( expData.variance_type_active(MATRIX_SIGMA)
				    || expData.interpolate_flag() );
    // TODO: Make this call robust to zero and single experiment cases
    ShortArray total_asv =  
      expData.determine_active_request(residual_resp, 
				       interrogate_field_data);
    // scale the residuals without changing member residualResponse
    expData.scale_residuals(residual_resp, total_asv, residuals);

    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Variance-weighted residuals:\n" << residuals << std::endl;
  }

  if (obsErrorMultiplierMode > 0) {  
    // r <- r ./ mult, where mult might be per-block
    // scale by mult, whether 1, per-experiment, per-response, or both
    // for now, the multiplier calibration mode is a method-related
    // parameter; could instead have a by-index interface here...
    expData.scale_residuals(hyper_params, obsErrorMultiplierMode, residuals);

    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Multiplier-weighted residuals:\n" << residuals << std::endl;
  }

  // misfit defined as 1/2 r^T (mult^2*Gamma_d)^{-1} r
  return residuals.dot( residuals ) / 2.0;
}


/** Response mapping callback used within RecastModel for MAP pre-solve. */
void NonDBayesCalibration::
neg_log_post_resp_mapping(const Variables& model_vars,
                          const Variables& nlpost_vars,
                          const Response& model_resp,
                          Response& nlpost_resp)
{
  if (nonDBayesInstance->obsErrorMultiplierMode > 0) {
    Cerr << "Error: observation error calibration unsupported in MAP pre-solve."
         << std::endl;
    abort_handler(-1);
  }

  // BMA: This is called in the context of the post compute-response
  // transformation in the RecastModel. Perhaps redesign to avoid
  // modifying class data (residualResponse) in this context.
  nonDBayesInstance->update_residual_response(model_resp);

  const RealVector& c_vars = nlpost_vars.continuous_variables();
  short nlpost_req = nlpost_resp.active_set_request_vector()[0];
  bool output_flag = (nonDBayesInstance->outputLevel >= DEBUG_OUTPUT);
  if (nlpost_req & 1) {
    RealVector hyper_params; // TO DO:
    // > cleanest approach may be to roll this into a variable recasting within
    //   negLogPostModel
    // > however, QUESO/DREAM do not roll this into a model recast, but instead
    //   augment the solver domains and then map the additional variables that
    //   flow into the likelihood into the hyper_params.
    //if (nonDBayesInstance->calibrateSigma) {
    //  RealVector x(Teuchos::View, c_vars.data(), numContinuousVars);
    //  RealVector hyper_params(Teuchos::View, &c_vars[numContinuousVars],
    //    numFunctions);
    //}

    // BMA TODO: update to include all likelihood terms (derivatives
    // w.r.t. hyperparams will be a problem due to det(Cov))
    Real nlp = 
      nonDBayesInstance->misfit(nonDBayesInstance->residualResponse, 
                                hyper_params) - 
      nonDBayesInstance->log_prior_density(c_vars);
    nlpost_resp.function_value(nlp, 0);
    if (output_flag)
      Cout << "MAP pre-solve: negative log posterior = " << nlp << std::endl;
  }

  if (nlpost_req & 2) {
    // avoid copy by updating gradient vector in place
    RealVector log_grad = nlpost_resp.function_gradient_view(0);
    nonDBayesInstance->expData.build_gradient_of_sum_square_residuals(
      nonDBayesInstance->residualResponse, log_grad);
    nonDBayesInstance->augment_gradient_with_log_prior(log_grad, c_vars);
    if (output_flag) {
      Cout << "MAP pre-solve: negative log posterior gradient:\n";
      write_data(Cout, log_grad);
    }
  }

  if (nlpost_req & 4) {
    // avoid copy by updating Hessian matrix in place
    RealSymMatrix log_hess = nlpost_resp.function_hessian_view(0);
    nonDBayesInstance->expData.build_hessian_of_sum_square_residuals(
      nonDBayesInstance->residualResponse, log_hess);
    nonDBayesInstance->augment_hessian_with_log_prior(log_hess, c_vars);
    if (output_flag) {
      Cout << "MAP pre-solve: negative log posterior Hessian:\n";
      write_data(Cout, log_hess);
    }
  }

  //Cout << "nlpost_resp:\n" << nlpost_resp;
}


//void NonDBayesCalibration::print_results(std::ostream& s)
//{ Cout << "Posterior sample results " << '\n'; }

} // namespace Dakota
