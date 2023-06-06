/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDQUESOBayesCalibration
//- Description: Derived class for Bayesian inference using QUESO
//- Owner:       Laura Swiler, Brian Adams
//- Checked by:
//- Version:

// place Dakota headers first to minimize influence of QUESO defines
#include "NonDQUESOBayesCalibration.hpp"
#include "NonDExpansion.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaModel.hpp"
#include "PRPMultiIndex.hpp"
// Dakota/QUESO interfaces
#include "QUESOImpl.hpp"
// finally list additional QUESO headers
#include "queso/StatisticalInverseProblem.h"
#include "queso/EnvironmentOptions.h"
#include "queso/GenericScalarFunction.h"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

extern PRPCache data_pairs; // global container

/// Static registration of RW TK with the QUESO TK factory
TKFactoryDIPC tk_factory_dipc("dakota_dipc_tk");
/// Static registration of Logit RW TK with the QUESO TK factory
TKFactoryDIPCLogit tk_factory_dipclogit("dakota_dipc_logit_tk");

// initialization of statics
NonDQUESOBayesCalibration* NonDQUESOBayesCalibration::nonDQUESOInstance(NULL);


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDQUESOBayesCalibration::
NonDQUESOBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  mcmcType(probDescDB.get_string("method.nond.mcmc_type")),
  propCovUpdatePeriod(probDescDB.get_int("method.nond.prop_cov_update_period")),
  precondRequestValue(0),
  logitTransform(probDescDB.get_bool("method.nond.logit_transform")),
  priorPropCovMult(probDescDB.get_real("method.prior_prop_cov_mult")),
  advancedOptionsFile(probDescDB.get_string("method.advanced_options_file"))
{
  bool found_error = false;

  // Assign default proposalCovarType (Only QUESO supports proposal
  // covariance updates and posterior adaptive surrogate updates for
  // now, hence this override is in this class)

  // BMA TODO: Consider unconditional default for simplicity
  if (proposalCovarType.empty()) {
    if (emulatorType) proposalCovarType = "derivatives"; // misfit Hessian
    else              proposalCovarType = "prior";       // prior covariance
  }

  if (priorPropCovMult < std::numeric_limits<double>::min() ||
      priorPropCovMult >= std::numeric_limits<double>::infinity()) {
    Cerr << "\nError: QUESO proposal covariance multiplier  = "
	 << priorPropCovMult << " not in [DBL_MIN, Inf).\n";
    found_error = true;
  }

  if (propCovUpdatePeriod < std::numeric_limits<int>::max() &&
      propCovUpdatePeriod >= chainSamples) {
    Cout << "\nWarning: QUESO proposal covariance update_period >= chain_samples;"
	 << "\n         no updates will occur." << std::endl;
  }

  if (!advancedOptionsFile.empty()) {
    if (boost::filesystem::exists(advancedOptionsFile)) {
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "Any QUESO options in file '" << advancedOptionsFile
	     << "' will override Dakota options." << std::endl;
    } else {
      Cerr << "\nError: QUESO options_file '" << advancedOptionsFile
	   << "' specified, but file not found.\n";
      found_error = true;
    }
  }

  // BMA TODO: Want to support these options independently
  if (obsErrorMultiplierMode > 0 && !calibrationData) {
    Cerr << "\nError: you are attempting to calibrate the measurement error " 
         << "but have not provided experimental data information." << std::endl;
    found_error = true;
  }

  if (found_error)
    abort_handler(METHOD_ERROR);

  init_queso_environment();
}


NonDQUESOBayesCalibration::~NonDQUESOBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDQUESOBayesCalibration::calibrate()
{
  // generate the sample chain that defines the joint posterior distribution
  map_pre_solve();
  run_chain();
}

  
void NonDQUESOBayesCalibration::map_pre_solve()
{
  // doing a double check here to avoid a double copy if not optimizing 
  // for MAP (this check happens again in base class map_pre_solve()). 
  if (mapOptimizer.is_null()) return;
  NonDBayesCalibration::map_pre_solve();

  // propagate MAP to paramInitials for starting point of MCMC chain.  
  // Note: specify_prior() happens once per calibrate(),
  // upstream of map_pre_solve()
  //copy_gsl(map_c_vars, *paramInitials);
  copy_gsl(mapSoln, *paramInitials);
}


void NonDQUESOBayesCalibration::run_chain()
{
  // define initial proposal covariance from misfit Hessian
  if (proposalCovarType == "derivatives")
    precondition_proposal(0);

  if (outputLevel >= NORMAL_OUTPUT) {
    Cout << "QUESO: Running chain with " << chainSamples << " samples."
	 << std::endl;
    if (propCovUpdatePeriod < std::numeric_limits<int>::max())
      Cout << "QUESO: Updating proposal covariance every "
	   << propCovUpdatePeriod << " samples." << std::endl;
  }
  run_queso_solver(); // solve inverse problem with MCMC 

  // always update bestSamples with highest posterior probability points
  log_best();
  // populate acceptanceChain, acceptedFnVals
  cache_chain();
}


void NonDQUESOBayesCalibration::init_queso_environment()
{
  // NOTE:  for now we are assuming that DAKOTA will be run with 
  // mpiexec to call MPI_Init.  Eventually we need to generalize this 
  // and send QUESO the proper MPI subenvironments.

  // NOTE: To make this function re-entrant, have to free quesoEnv
  // first, then reset envOptionsValues, since destructor of quesoEnv
  // needs envOptionsValues:
  quesoEnv.reset();

  // Construct with default options
  // TODO: see if this can be a local, or if the env retains a pointer
  envOptionsValues = std::make_shared<QUESO::EnvOptionsValues>();

  // C++ API options may override defaults
  envOptionsValues->m_subDisplayFileName = "QuesoDiagnostics/display";
  envOptionsValues->m_subDisplayAllowedSet.insert(0);
  envOptionsValues->m_subDisplayAllowedSet.insert(1);
  envOptionsValues->m_displayVerbosity = 2;
  // From GPMSA: envOptionsValues->m_displayVerbosity     = 3;
  envOptionsValues->m_seed = randomSeed;
  // From GPMSA: envOptionsValues->m_identifyingString="dakota_foo.in"

  // File-based power user parameters have the final say
  //
  // Unfortunately, there's a chicken-and-egg problem where parsing
  // EnvOptionsValues requires an Environment, so we defer this parse
  // to the ctor...
  //
  // if (!advancedOptionsFile.empty())
  //   envOptionsValues->parse(*quesoEnv, "");
  const char* aof_cstr =
    advancedOptionsFile.empty() ? NULL : advancedOptionsFile.c_str();

  // BMA TODO: Remove ML-specific code now that we have advanced options parsing
#ifdef DAKOTA_HAVE_MPI
  // this prototype and MPI_COMM_SELF only available if Dakota/QUESO have MPI
  if (parallelLib.mpirun_flag()) {
    if (mcmcType == "multilevel")
      quesoEnv = std::make_shared<QUESO::FullEnvironment>
	(MPI_COMM_SELF, "ml.inp", "", nullptr);
    else // dram, dr, am, or mh
      quesoEnv = std::make_shared<QUESO::FullEnvironment>
	(MPI_COMM_SELF, aof_cstr, "", envOptionsValues.get());
  }
  else {
    if (mcmcType == "multilevel")
      quesoEnv = std::make_shared<QUESO::FullEnvironment>("ml.inp", "", nullptr);
    else // dram, dr, am, or mh
      quesoEnv = std::make_shared<QUESO::FullEnvironment>
	(aof_cstr, "", envOptionsValues.get());
  }
#else
  if (mcmcType == "multilevel")
    quesoEnv = std::make_shared<QUESO::FullEnvironment>("ml.inp", "", nullptr);
  else // dram, dr, am, or mh
    quesoEnv = std::make_shared<QUESO::FullEnvironment>
      (aof_cstr, "", envOptionsValues.get());
#endif
}


void NonDQUESOBayesCalibration::init_precond_request_value()
{
  // TNP 11/23/20 - seems like could be moved to the base class entirely, 
  // if MUQ would use this too
  
  // Gauss-Newton approximate Hessian requires gradients of residuals (rv=2)
  // full Hessian requires values/gradients/Hessians of residuals (rv=7)
  precondRequestValue = 0;
  switch (emulatorType) {
  case PCE_EMULATOR: case ML_PCE_EMULATOR: case MF_PCE_EMULATOR:
  case KRIGING_EMULATOR:
    precondRequestValue = 7; break; // emulator Hessian support
  case SC_EMULATOR: case MF_SC_EMULATOR: case GP_EMULATOR:
    precondRequestValue = 2; break; // emulator Hessians not supported yet
  case  NO_EMULATOR:
    // Note: mixed gradients/Hessians are still globally "on", regardless of
    // mixed computation approach
    if (mcmcModel.gradient_type() != "none")
      precondRequestValue |= 2; // gradients
    if (mcmcModel.hessian_type()  != "none")
      precondRequestValue |= 5; // values & Hessians
    break;
  }
}


void NonDQUESOBayesCalibration::specify_likelihood()
{
  // Instantiate the likelihood function object that computes [ln(function)]
  likelihoodFunctionObj = std::make_shared
    <QUESO::GenericScalarFunction<QUESO::GslVector,QUESO::GslMatrix>>
    ("like_", *paramDomain, &dakotaLogLikelihood, (void *)nullptr, true);
}


void NonDQUESOBayesCalibration::init_bayesian_solver()
{
  // TNP TODO: Would we ever need to update the proposal covariance stuff? May
  // want to move that into the posterior call....

  nonDQUESOInstance = this;

  // Sets proposal covariance options, ip options, mh options.
  tk_factory_dipc.set_callback(nonDQUESOInstance);
  tk_factory_dipclogit.set_callback(nonDQUESOInstance);
  
  // initialize the ASV request value for preconditioning and
  // construct a Response for use in residual computation
  if (proposalCovarType == "derivatives")
    init_precond_request_value();

  // may pull Hessian data from emulator
  init_proposal_covariance();

  // initialize proposal covariance (must follow parameter domain init)
  // This is the leading sub-matrix in the case of calibrating sigma terms
  if (proposalCovarType == "user") // either filename OR data values defined
    user_proposal_covariance(proposalCovarInputType, proposalCovarData,
			     proposalCovarFilename);
  else if (proposalCovarType == "prior")
    prior_proposal_covariance(); // prior selection or default for no emulator
  else // misfit Hessian-based proposal with prior preconditioning
    prior_cholesky_factorization();

  // define calIpOptionsValues
  set_ip_options();
  // Set options specific to MH algorithm
  set_mh_options();
}

void NonDQUESOBayesCalibration::specify_posterior(){
  // Create a pointer for the posterior RV and generate a QUESO IP
  // Must have called init_bayesian_solver at least once. 
 
  // Instantiate the inverse problem
  postRv =
    std::make_shared<QUESO::GenericVectorRV<QUESO::GslVector,QUESO::GslMatrix>>
    ("post_", *paramSpace);
  // Q: are Prior/posterior RVs copied by StatInvProblem, such that these
  //    instances can be local and allowed to go out of scope?
  // Inverse problem: instantiate it (posterior rv is instantiated internally)
  inverseProb = std::make_shared
    <QUESO::StatisticalInverseProblem<QUESO::GslVector,QUESO::GslMatrix>>
    ("", calIpOptionsValues.get(), *priorRv, *likelihoodFunctionObj, *postRv);
}

void NonDQUESOBayesCalibration::precondition_proposal(unsigned int chain_index)
{
  if (!precondRequestValue) {
    Cerr << "Error: response derivative specification required for proposal "
         << "preconditioning." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // extract GSL sample vector from QUESO vector sequence:
  QUESO::GslVector curr_params(paramSpace->zeroVector());

  if (chain_index == 0)
    curr_params = *paramInitials;
  else {
    const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>& 
      mcmc_chain = inverseProb->chain();
   
    if (chain_index >= mcmc_chain.subSequenceSize()) {
      Cerr << "\nError: QUESO precondition_proposal index out of bounds\n";
      abort_handler(METHOD_ERROR);
    }

    mcmc_chain.getPositionValues(chain_index, curr_params);
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "New center:\n" << curr_params << "Log likelihood = "
	   << inverseProb->logLikelihoodValues()[chain_index] << std::endl;
  }

  // update mcmcModel's continuous variables from curr_params (current MCMC
  // chain position)
  copy_gsl_partial
    (curr_params, 0,
     residualModel.current_variables().continuous_variables_view());
  // update request vector values
  const Response& residual_resp = residualModel.current_response();
  ActiveSet set = residual_resp.active_set(); // copy
  set.request_values(precondRequestValue);
  // compute response (emulator case echoed to Cout if outputLevel > NORMAL)
  residualModel.evaluate(set);

  // compute Hessian of log-likelihood misfit r^T Gamma^{-1} r
  RealSymMatrix log_hess;//(numContinuousVars); // init to 0
  RealSymMatrix prop_covar;

  expData.build_hessian_of_sum_square_residuals
    (residualModel.current_response(), log_hess);
  //bool fallback =
    get_positive_definite_covariance_from_hessian(log_hess, prop_covar);

  /*
  if ( fallback && (precondRequestValue & 4) ) { // fall back if indefinite
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Falling back from full misfit Hessian to Gauss-Newton misfit "
	   << "Hessian.\n";
    // BMA @JDJ, @MSE: I think this asv needs to be length of the
    // total number of residuals (residualResponse.num_functions or
    // expData.num_total_exppoints())
    ShortArray asrv_override(numFunctions, 2); // override asrv in response
    expData.build_hessian_of_sum_square_residuals(residualResponse,
						  asrv_override, log_hess);
    get_positive_definite_covariance_from_hessian(log_hess, prop_covar);
  }
  */

  // pack GSL proposalCovMatrix
  int i, j, nv = log_hess.numRows();
  if (!proposalCovMatrix) {
    proposalCovMatrix = std::make_shared<QUESO::GslMatrix>
      (paramSpace->zeroVector());
    if (paramSpace->dimGlobal() != nv || 
	paramSpace->dimGlobal() != nv+numFunctions) {
      Cerr << "Error: Queso vector space is not consistent with proposal "
	   << "covariance dimension." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  for (i=0; i<nv; ++i )
    for (j=0; j<nv; ++j )
      (*proposalCovMatrix)(i,j) = prop_covar(i,j);
}


void NonDQUESOBayesCalibration::run_queso_solver()
{
  if (outputLevel >= DEBUG_OUTPUT) {
    //  Cout << "QUESO final ENV options:\n" << *envOptionsValues << std::endl;
    Cout << "QUESO final SIP options:\n" << *calIpOptionsValues << std::endl;
    Cout << "QUESO final MH options:\n" << *calIpMhOptionsValues << std::endl;
  }

  Cout << "Running Bayesian Calibration with QUESO " << mcmcType << " using "
       << calIpMhOptionsValues->m_rawChainSize << " MCMC samples." << std::endl;
  if (outputLevel > NORMAL_OUTPUT && numHyperparams > 0)
    Cout << "\n  Calibrating " << numHyperparams << " error hyperparameters." 
	 << std::endl;

  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////
  if (mcmcType == "multilevel")
    inverseProb->solveWithBayesMLSampling();
  else
    inverseProb->solveWithBayesMetropolisHastings(calIpMhOptionsValues.get(),
						  *paramInitials, 
						  proposalCovMatrix.get());

  Cout << "QUESO MCMC chain completed.  MCMC details are concatenated within "
       << "the QuesoDiagnostics directory:\n"
       << "  display_sub0.txt contains MCMC diagnostics.\n";
  if (standardizedSpace)
    Cout << "  Caution: Matlab files contain the chain values in "
	 << "standardized probability space.\n";
  else
    Cout << "  Matlab files contain the chain values.\n";
  //   << "The files to load in Matlab are\nfile_cal_ip_raw.m (the actual " 
  //   << "chain) or file_cal_ip_filt.m (the filtered chain,\nwhich contains " 
  //   << "every 20th step in the chain.\n"
  //   << "NOTE:  the chain values in these Matlab files are currently " 
  //   << "in scaled space. \n  You will have to transform them back to "
  //   << "original space by:\n"
  //   << "lower_bounds + chain_values * (upper_bounds - lower_bounds)\n"
  //   << "The rejection rate is in the tgaCalOutput file.\n"
  //   << "We hope to improve the postprocessing of the chains by the " 
  //   << "next Dakota release.\n";
}


/** Populate all of acceptanceChain(num_params, chainSamples)
    acceptedFnVals(numFunctions, chainSamples) */
void NonDQUESOBayesCalibration::cache_chain()
{
  acceptanceChain.shapeUninitialized(numContinuousVars + numHyperparams,
				     chainSamples);
  acceptedFnVals.shapeUninitialized(numFunctions, chainSamples);

  // temporaries for evals/lookups
  // the MCMC model omits the hyper params and residual transformations...
  Variables lookup_vars = mcmcModel.current_variables().copy();
  String   interface_id = mcmcModel.interface_id();
  Response  lookup_resp = mcmcModel.current_response().copy();
  ActiveSet   lookup_as = lookup_resp.active_set();
  lookup_as.request_values(1);
  lookup_resp.active_set(lookup_as);
  ParamResponsePair lookup_pr(lookup_vars, interface_id, lookup_resp);

  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>&
    mcmc_chain = inverseProb->chain();
  unsigned int num_mcmc = mcmc_chain.subSequenceSize();
  if (num_mcmc != chainSamples) {
    Cerr << "\nError: QUESO cache_chain(): chain length is " << num_mcmc
	 << "; expected " << chainSamples << '\n';
    abort_handler(METHOD_ERROR);
  }

  // The posterior may include GPMSA hyper-parameters, so use the postRv space
  //  QUESO::GslVector qv(paramSpace->zeroVector());
  QUESO::GslVector qv(postRv->imageSet().vectorSpace().zeroVector());

  unsigned int lookup_failures = 0;
  unsigned int num_params = numContinuousVars + numHyperparams;
  for (int i=0; i<num_mcmc; ++i) {

    // translate the QUESO vector into x- or u-space lookup vars and
    // x-space acceptanceChain
    mcmc_chain.getPositionValues(i, qv); // extract GSLVector from sequence
    if (standardizedSpace) {
      // u_rv and x_rv omit any hyper-parameters
      RealVector u_rv(numContinuousVars, false);
      copy_gsl_partial(qv, 0, u_rv);
      Real* acc_chain_i = acceptanceChain[i];
      RealVector x_rv(Teuchos::View, acc_chain_i, numContinuousVars);
      mcmcModel.trans_U_to_X(u_rv, x_rv);
      for (int j=numContinuousVars; j<num_params; ++j)
	acc_chain_i[j] = qv[j]; // trailing hyperparams are not transformed

      // surrogate needs u-space variables for eval
      if (mcmcModel.model_type() == "surrogate")
	lookup_vars.continuous_variables(u_rv);
      else
	lookup_vars.continuous_variables(x_rv);
    }
    else {
      // A view that includes calibration params and Dakota-managed
      // hyper-parameters, to facilitate copying from the longer qv
      // into acceptanceChain:
      RealVector theta_hp(Teuchos::View, acceptanceChain[i], 
			  numContinuousVars + numHyperparams);
      copy_gsl_partial(qv, 0, theta_hp);
      // lookup vars only need the calibration parameters
      RealVector x_rv(Teuchos::View, acceptanceChain[i], 
		      numContinuousVars);
      lookup_vars.continuous_variables(x_rv);
    }

    // now retreive function values

    // NOTE: The MCMC may be PCE/SC model, DataFitSurrModel, or raw
    // model, but it's type may be a ProbabilityTransform wrapper if
    // standardizedSpace is active.  mcmcModelHasSurrogate controls
    // model re-evals.  This is not sufficiently general, e.g., if the
    // mcmcModel is an EnsembleSurrModel, could perform costly re-eval.

    // TODO: Consider doing lookup first, then surrogate re-eval, or
    // querying a more complete eval database when available...

    if (mcmcModelHasSurrogate) {
      mcmcModel.active_variables(lookup_vars);
      mcmcModel.evaluate(lookup_resp.active_set());
      const RealVector& fn_vals = mcmcModel.current_response().function_values();
      Teuchos::setCol(fn_vals, i, acceptedFnVals);
    }
    else {
      lookup_pr.variables(lookup_vars);
      PRPCacheHIter cache_it = lookup_by_val(data_pairs, lookup_pr);
      if (cache_it == data_pairs.get<hashed>().end()) {
	++lookup_failures;
	// Set NaN in the chain points to avoid misleading the user
	RealVector nan_fn_vals(mcmcModel.current_response().function_values().length());
	nan_fn_vals = std::numeric_limits<double>::quiet_NaN();
	Teuchos::setCol(nan_fn_vals, i, acceptedFnVals);
      }
      else {
	const RealVector& fn_vals = cache_it->response().function_values();
	Teuchos::setCol(fn_vals, i, acceptedFnVals);
      }
    }

  }
  if (lookup_failures > 0 && outputLevel > SILENT_OUTPUT)
    Cout << "Warning: could not retrieve function values for " 
	 << lookup_failures << " MCMC chain points." << std::endl;
}



void NonDQUESOBayesCalibration::log_best()
{
  bestSamples.clear();
  // BMA TODO: Need to transform chain back to unscaled space for reporting
  // to user; possibly also in auxilliary data files for user consumption.

  // to get the full acceptance chain, m_filteredChainGenerate is set to
  // false in set_invpb_mh_options()

  // Note: the QUESO VectorSequence class has a number of helpful filtering
  // and statistics functions.

  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>& mcmc_chain
    = inverseProb->chain();
  const QUESO::ScalarSequence<double>& loglike_vals
    = inverseProb->logLikelihoodValues();
  unsigned int num_mcmc = mcmc_chain.subSequenceSize();
  if (num_mcmc != loglike_vals.subSequenceSize()) {
    Cerr << "Error (NonDQUESO): final mcmc chain has length " << num_mcmc 
	 << "\n                 but likelihood set has length"
	 << loglike_vals.subSequenceSize() << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // TO DO: want to keep different samples with same likelihood, but not 
  // replicate samples with same likelihood (from rejection); for now, use
  // a std::map since the latter is unlikely.
  QUESO::GslVector mcmc_sample(paramSpace->zeroVector());
  RealVector mcmc_rv;
  for (size_t chain_pos = 0; chain_pos < num_mcmc; ++chain_pos) {
    // extract GSL sample vector from QUESO vector sequence:
    mcmc_chain.getPositionValues(chain_pos, mcmc_sample);
    // evaluate log of posterior from log likelihood and log prior:
    Real log_prior     = log_prior_density(mcmc_sample),
         log_posterior = loglike_vals[chain_pos] + log_prior;
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "MCMC sample: " << mcmc_sample << " log prior = " << log_prior
	   << " log posterior = " << log_posterior << std::endl;
    // sort ascending by log posterior (highest prob are last) and retain
    // batch_size samples
    copy_gsl(mcmc_sample, mcmc_rv);
    bestSamples.insert(std::make_pair(log_posterior, mcmc_rv));
    if (bestSamples.size() > batchSize)
      bestSamples.erase(bestSamples.begin()); // pop front (lowest prob)
  }
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "bestSamples map:\n" << bestSamples << std::endl;
}


void NonDQUESOBayesCalibration::filter_chain_by_conditioning()
{
  const QUESO::BaseVectorSequence<QUESO::GslVector,QUESO::GslMatrix>&
    mcmc_chain = inverseProb->chain();
  unsigned int num_mcmc = mcmc_chain.subSequenceSize();


  // filter chain -or- extract full chain and sort on likelihood values
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Extracting unique samples from MCMC chain containing "
	 << num_mcmc << " samples.\n";

  // BMA TODO: determine if the conditioning alg can handle duplicates
  // Note this isn't an array of unique samples, rather has no consecutive repeats
  // May not be needed any longer...
  RealVectorArray unique_samples;

  QUESO::GslVector q_sample(paramSpace->zeroVector()),
              prev_q_sample(paramSpace->zeroVector());

  RealVector empty_rv;
  mcmc_chain.getPositionValues(0, prev_q_sample);// extract vector from sequence
  unique_samples.push_back(empty_rv);             // copy empty vector
  copy_gsl(prev_q_sample, unique_samples.back()); // update in place

  // else first sample is same as last sample from previous chain
  //  for (size_t s=1; s<num_mcmc; ++s) {
  for (size_t s=1; s<num_mcmc; ++s) {
    mcmc_chain.getPositionValues(s, q_sample);  // extract vector from sequence
    if (!equal_gsl(q_sample, prev_q_sample)) {
      unique_samples.push_back(empty_rv);        // copy empty vector
      copy_gsl(q_sample, unique_samples.back()); // update in place
      prev_q_sample = q_sample;
    }
  }

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "Filtering chain by matrix conditioning: extracting best "
	 << batchSize << " from aggregate MCMC chain containing "
	 << unique_samples.size() << " samples.\n";
  std::shared_ptr<NonDExpansion> nond_exp =
    std::static_pointer_cast<NonDExpansion>(stochExpIterator.iterator_rep());
  nond_exp->select_refinement_points(unique_samples, batchSize, allSamples);
}


/** Initialize the calibration parameter domain (paramSpace,
    paramMins/paramMaxs, paramDomain, paramInitials, priorRV) */
void NonDQUESOBayesCalibration::specify_prior()
{
  nonDQUESOInstance = this;

  // If calibrating error multipliers, the parameter domain is expanded to
  // estimate hyperparameters sigma^2 that multiply any user-provided covariance
  paramSpace = std::make_shared
    <QUESO::VectorSpace<QUESO::GslVector,QUESO::GslMatrix>>
    (*quesoEnv, "param_", numContinuousVars + numHyperparams, nullptr);

  QUESO::GslVector paramMins(paramSpace->zeroVector()),
                   paramMaxs(paramSpace->zeroVector());
  // mcmcModel may use a different view, so map from all view in mcmcModel to
  // active in iteratedModel
  RealRealPairArray bnds
    = mcmcModel.multivariate_distribution().distribution_bounds(); // all RV
  // Use SVD to convert active CV index (calibration params) to all index (RVs)
  const SharedVariablesData& svd
    = iteratedModel.current_variables().shared_data();
  for (size_t i=0; i<numContinuousVars; ++i) {
    const RealRealPair& bnds_i = bnds[svd.cv_index_to_all_index(i)];
    paramMins[i] = bnds_i.first;  paramMaxs[i] = bnds_i.second;
  }
  for (size_t i=0; i<numHyperparams; ++i) {
    // inverse gamma is defined on [0,inf), but we may have to divide
    // responses by up to mult^{5/2}, so bound away from 0.
    // real_min()^{2/5} = 8.68836e-124, but we can't anticipate the
    // problem scale.  Arbitrarily choose 1.0e-100 for now.
    paramMins[numContinuousVars + i] = 1.0e-100;
    paramMaxs[numContinuousVars + i] = std::numeric_limits<Real>::infinity();
  }
  paramDomain = std::make_shared
    <QUESO::BoxSubset<QUESO::GslVector,QUESO::GslMatrix>>
     ("param_", *paramSpace, paramMins, paramMaxs);

  paramInitials = std::make_shared<QUESO::GslVector>(paramSpace->zeroVector());
  // paramInitials started from mapSoln, which encompasses either:
  // > most recent solution from map_pre_solve(), if used, or
  // > initial guess (user-specified or default initial values)
  copy_gsl(mapSoln, *paramInitials);

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Initial Parameter values sent to QUESO (may be in scaled)\n"
	 << *paramInitials << "\nParameter bounds sent to QUESO (may be scaled)"
	 << ":\nparamMins " << paramMins << "\nparamMaxs " << paramMaxs << '\n';

  // new approach supports arbitrary priors:
  priorRv = std::make_shared<QuesoVectorRV<QUESO::GslVector,QUESO::GslMatrix>> 
  ("prior_", *paramDomain, nonDQUESOInstance);
}


void NonDQUESOBayesCalibration::init_proposal_covariance()
{
  // Size our Queso covariance matrix and initialize trailing diagonal if
  // calibrating error hyperparams
  proposalCovMatrix =
    std::make_shared<QUESO::GslMatrix>(paramSpace->zeroVector());
  if (numHyperparams > 0) {
    // all hyperparams utilize inverse gamma priors, which may not
    // have finite variance; use std_dev = 0.05 * mode
    Real alpha;
    for (int i=0; i<numHyperparams; ++i) {
      invGammaDists[i].pull_parameter(Pecos::IGA_ALPHA, alpha);
      (*proposalCovMatrix)(numContinuousVars + i, numContinuousVars + i) = 
	(alpha > 2.) ? invGammaDists[i].variance() :
	std::pow(0.05*(*paramInitials)[numContinuousVars + i], 2.);
    }
  }

  // initialize proposal covariance (must follow parameter domain init)
  // This is the leading sub-matrix in the case of calibrating sigma terms
  if (proposalCovarType == "user") // either filename OR data values defined
    user_proposal_covariance(proposalCovarInputType, proposalCovarData,
			     proposalCovarFilename);
  else if (proposalCovarType == "prior")
    prior_proposal_covariance(); // prior selection or default for no emulator
  else // misfit Hessian-based proposal with prior preconditioning
    prior_cholesky_factorization();
}


/** Must be called after paramMins/paramMaxs set above */
void NonDQUESOBayesCalibration::prior_proposal_covariance()
{
  //int num_params = paramSpace->dimGlobal();
  //QUESO::GslVector covDiag(paramSpace->zeroVector());

  // diagonal covariance from variance of prior marginals
  RealVector dist_var = mcmcModel.multivariate_distribution().variances();
  // SVD index conversion is more general, but not required for current uses
  //const SharedVariablesData& svd= mcmcModel.current_variables().shared_data();
  for (int i=0; i<numContinuousVars; ++i)
    (*proposalCovMatrix)(i,i) = priorPropCovMult * dist_var[i];
      //* dist_var[svd.cv_index_to_active_index(i)];
  //proposalCovMatrix.reset(new QUESO::GslMatrix(covDiag));

  if (outputLevel > NORMAL_OUTPUT) {
    //Cout << "Diagonal elements of the proposal covariance sent to QUESO";
    //if (standardizedSpace) Cout << " (scaled space)";
    //Cout << '\n' << covDiag << '\n';
    Cout << "QUESO ProposalCovMatrix"; 
    if (standardizedSpace) Cout << " (scaled space)";
    Cout << '\n'; 
    for (size_t i=0; i<numContinuousVars; ++i) {
      for (size_t j=0; j<numContinuousVars; ++j) 
	Cout <<  (*proposalCovMatrix)(i,j) << "  "; 
      Cout << '\n'; 
    }
  }

  validate_proposal();
}


/** This function will convert user-specified cov_type = "diagonal" |
    "matrix" data from either cov_data or cov_filename and populate a
    full QUESO::GslMatrix* in proposalCovMatrix with the covariance. */
void NonDQUESOBayesCalibration::
user_proposal_covariance(const String& input_fmt, const RealVector& cov_data, 
			 const String& cov_filename)
{
  // TODO: transform user covariance for use in standardized probability space
  if (standardizedSpace)
    throw std::runtime_error("user-defined proposal covariance is invalid for use in transformed probability spaces.");
  // Note: if instead a warning, then fallback to prior_proposal_covariance()

  bool use_file = !cov_filename.empty();

  // Sanity check
  if( ("diagonal" != input_fmt) &&
      ("matrix"   != input_fmt) )
    throw std::runtime_error("User-specified covariance must have type of either \"diagonal\" of \"matrix\".  You have \""+input_fmt+"\".");

  // Sanity check
  if( cov_data.length() && use_file )
    throw std::runtime_error("You cannot provide both covariance values and a covariance data filename.");

  // Size our Queso covariance matrix
  //proposalCovMatrix.reset(new QUESO::GslMatrix(paramSpace->zeroVector()));

  // Sanity check
  /*if( (proposalCovMatrix->numRowsLocal()  != numContinuousVars) || 
      (proposalCovMatrix->numRowsGlobal() != numContinuousVars) || 
      (proposalCovMatrix->numCols()       != numContinuousVars)   )
    throw std::runtime_error("Queso vector space is not consistent with parameter dimension.");
  */      
  // Read in a general way and then check that the data is consistent
  RealVectorArray values_from_file;
  if( use_file )
  {
    std::ifstream s;
    TabularIO::open_file(s, cov_filename, "read_queso_covariance_data");
    bool row_major = false;
    read_unsized_data(s, values_from_file, row_major);
  }

  if( "diagonal" == input_fmt )
  {
    if( use_file ) {
      // Allow either row or column data layout
      bool row_data = false;
      // Sanity checks
      if( values_from_file.size() != 1 ) {
        if( values_from_file.size() == numContinuousVars ) 
          row_data = true;
        else
          throw std::runtime_error("\"diagonal\" Queso covariance file data should have either 1 column (or row) and "
              +convert_to_string(numContinuousVars)+" rows (or columns).");
      }
      if( row_data ) {
        for( int i=0; i<numContinuousVars; ++i )
          (*proposalCovMatrix)(i,i) = values_from_file[i](0);
      }
      else {
        if( values_from_file[0].length() != numContinuousVars )
          throw std::runtime_error("\"diagonal\" Queso covariance file data should have "
              +convert_to_string(numContinuousVars)+" rows.  Found "
              +convert_to_string(values_from_file[0].length())+" rows.");
        for( int i=0; i<numContinuousVars; ++i )
          (*proposalCovMatrix)(i,i) = values_from_file[0](i);
      }
    }
    else {
      // Sanity check
      if( numContinuousVars != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(numContinuousVars)
                                 +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      for( int i=0; i<numContinuousVars; ++i )
        (*proposalCovMatrix)(i,i) = cov_data(i);
    }
  }
  else // "matrix" == input_fmt
  {
    if( use_file ) {
      // Sanity checks
      if( values_from_file.size() != numContinuousVars ) 
        throw std::runtime_error("\"matrix\" Queso covariance file data should have "
                                 +convert_to_string(numContinuousVars)+" columns.  Found "
                                 +convert_to_string(values_from_file.size())+" columns.");
      if( values_from_file[0].length() != numContinuousVars )
        throw std::runtime_error("\"matrix\" Queso covariance file data should have "
                                 +convert_to_string(numContinuousVars)+" rows.  Found "
                                 +convert_to_string(values_from_file[0].length())+" rows.");
      for( int i=0; i<numContinuousVars; ++i )
        for( int j=0; j<numContinuousVars; ++j )
          (*proposalCovMatrix)(i,j) = values_from_file[i](j);
    }
    else {
      // Sanity check
      if( numContinuousVars*numContinuousVars != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(numContinuousVars*numContinuousVars)
            +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      int count = 0;
      for( int i=0; i<numContinuousVars; ++i )
        for( int j=0; j<numContinuousVars; ++j )
          (*proposalCovMatrix)(i,j) = cov_data[count++];
    }
  }

  // validate that provided data is a valid covariance matrix
  validate_proposal();
}


void NonDQUESOBayesCalibration::validate_proposal()
{
  // validate that provided data is a valid covariance matrix
  
  if (outputLevel > NORMAL_OUTPUT ) { 
    Cout << "Proposal Covariance " << '\n';
    proposalCovMatrix->print(Cout);
    Cout << std::endl;
  }

  // test symmetry
  QUESO::GslMatrix test_mat = proposalCovMatrix->transpose();
  test_mat -= *proposalCovMatrix;
  if( test_mat.normMax() > 1.e-14 )
    throw std::runtime_error("Queso covariance matrix is not symmetric.");

  // test PD part of SPD
  test_mat = *proposalCovMatrix;
  int ierr = test_mat.chol();
  if( ierr == QUESO::UQ_MATRIX_IS_NOT_POS_DEFINITE_RC)
    throw std::runtime_error("Queso covariance data is not SPD.");
}


/// set inverse problem options common to all solvers
void NonDQUESOBayesCalibration::set_ip_options() 
{
  // Construct with default options
  calIpOptionsValues = std::make_shared<QUESO::SipOptionsValues>();

  // C++ API options may override defaults
  //definitely want to retain computeSolution
  calIpOptionsValues->m_computeSolution    = true;
  calIpOptionsValues->m_dataOutputFileName = "QuesoDiagnostics/invpb_output";
  calIpOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpOptionsValues->m_dataOutputAllowedSet.insert(1);

  // File-based power user parameters have the final say
  if (!advancedOptionsFile.empty())
    calIpOptionsValues->parse(*quesoEnv, "");

 if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nIP Final Options:" << *calIpOptionsValues << std::endl;
}


void NonDQUESOBayesCalibration::set_mh_options() 
{
  // Construct with default options
  calIpMhOptionsValues = std::make_shared<QUESO::MhOptionsValues>();

  // C++ API options may override defaults
  calIpMhOptionsValues->m_dataOutputFileName = "QuesoDiagnostics/mh_output";
  calIpMhOptionsValues->m_dataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_dataOutputAllowedSet.insert(1);

  calIpMhOptionsValues->m_rawChainDataInputFileName   = ".";
  calIpMhOptionsValues->m_rawChainSize = (chainSamples > 0) ? chainSamples : 1000;
  //calIpMhOptionsValues->m_rawChainGenerateExtra     = false;
  //calIpMhOptionsValues->m_rawChainDisplayPeriod     = 20000;
  //calIpMhOptionsValues->m_rawChainMeasureRunTimes   = true;
  calIpMhOptionsValues->m_rawChainDataOutputFileName  = "QuesoDiagnostics/raw_chain";
  calIpMhOptionsValues->m_rawChainDataOutputAllowedSet.insert(0);
  calIpMhOptionsValues->m_rawChainDataOutputAllowedSet.insert(1);
  // NO LONGER SUPPORTED.  calIpMhOptionsValues->m_rawChainComputeStats = true;

  //calIpMhOptionsValues->m_displayCandidates         = false;
  calIpMhOptionsValues->m_putOutOfBoundsInChain       = false;
  //calIpMhOptionsValues->m_tkUseLocalHessian         = false;
  //calIpMhOptionsValues->m_tkUseNewtonComponent      = true;

  // delayed rejection option:
  calIpMhOptionsValues->m_drMaxNumExtraStages
    = (mcmcType == "delayed_rejection" || mcmcType == "dram") ? 1 : 0;
  calIpMhOptionsValues->m_drScalesForExtraStages.resize(1);
  calIpMhOptionsValues->m_drScalesForExtraStages[0] = 5;
  //calIpMhOptionsValues->m_drScalesForExtraStages[1] = 10;
  //calIpMhOptionsValues->m_drScalesForExtraStages[2] = 20;

  // adaptive metropolis option:
  calIpMhOptionsValues->m_amInitialNonAdaptInterval
    = (mcmcType == "adaptive_metropolis" || mcmcType == "dram") ? 100 : 0;
  calIpMhOptionsValues->m_amAdaptInterval           = 100;
  calIpMhOptionsValues->m_amEta                     = 2.88;
  calIpMhOptionsValues->m_amEpsilon                 = 1.e-8;

  // chain management options:
  if (false) { // TO DO: add user spec to support chain filtering?
    // filtering the chain for final output affects the inverseProb->chain()
    // as well as the QuesoDiagnostics directory.  For now, we turn this off
    // to promote reporting the best MAP estimate.
    calIpMhOptionsValues->m_filteredChainGenerate         = true;
    calIpMhOptionsValues->m_filteredChainDiscardedPortion = 0.;
    calIpMhOptionsValues->m_filteredChainLag              = 20;
    calIpMhOptionsValues->
      m_filteredChainDataOutputFileName = "QuesoDiagnostics/filtered_chain";
    calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(0);
    calIpMhOptionsValues->m_filteredChainDataOutputAllowedSet.insert(1);
    //calIpMhOptionsValues->m_filteredChainComputeStats   = true;
  }
  else //if (adaptPosteriorRefine || chainCycles > 1)
    // In this case, we process the full chain for maximizing posterior
    // probability or point spacing / linear system conditioning
    calIpMhOptionsValues->m_filteredChainGenerate         = false;

  // Logit transform addresses high rejection rates in corners of
  // bounded domains.  It is potentially redundant in some cases,
  // e.g., WIENER u-space type.
  if (logitTransform) {
    calIpMhOptionsValues->m_algorithm = "logit_random_walk";
    calIpMhOptionsValues->m_tk = "logit_random_walk";
    calIpMhOptionsValues->m_doLogitTransform = true;  // deprecated
  }
  else {
    calIpMhOptionsValues->m_algorithm = "random_walk";
    calIpMhOptionsValues->m_tk = "random_walk";
    calIpMhOptionsValues->m_doLogitTransform = false;  // deprecated
  }

  // Use custom TK for derivative-based proposal updates
  if (proposalCovarType == "derivatives" &&
      propCovUpdatePeriod < std::numeric_limits<int>::max()) {
    if (logitTransform)
      calIpMhOptionsValues->m_tk = "dakota_dipc_logit_tk";
    else
      calIpMhOptionsValues->m_tk = "dakota_dipc_tk";
    calIpMhOptionsValues->m_updateInterval = propCovUpdatePeriod;
  }

  // File-based power user parameters have the final say
  // The options are typically prefixed with ip_mh, so prepend an "ip_" prefix
  // from the IP options:
  if (!advancedOptionsFile.empty())
    calIpMhOptionsValues->parse(*quesoEnv, calIpOptionsValues->m_prefix);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nMH Final Options:" << *calIpMhOptionsValues << std::endl;
 }


void NonDQUESOBayesCalibration::
print_results(std::ostream& s, short results_state)
{
  if (bestSamples.empty()) return;

  // ----------------------------------------
  // Output best sample which appoximates MAP
  // ----------------------------------------
  std::/*multi*/map<Real, RealVector>::const_iterator it = --bestSamples.end();
  //std::pair<Real, RealVector>& best = bestSamples.back();
  const RealVector& best_sample = it->second;

  size_t wpp7 = write_precision+7;
  s << "<<<<< Best parameters          =\n";
  print_variables(s, best_sample);

  // print corresponding response data; here we recover the misfit
  // instead of re-computing it
  QUESO::GslVector qv(paramSpace->zeroVector());
  copy_gsl(best_sample, qv);
  Real log_prior = log_prior_density(qv), log_post = it->first;
  size_t num_total_calib_terms = residualModel.num_primary_fns();
  Real half_nr_log2pi = num_total_calib_terms * HALF_LOG_2PI;
  RealVector hyper_params(numHyperparams);
  copy_gsl_partial(qv, numContinuousVars, hyper_params);
  Real half_log_det = 
    expData.half_log_cov_determinant(hyper_params, obsErrorMultiplierMode);
  // misfit = -log(L) - 1/2*Nr*log(2*pi) - 1/2*log(det(Cov))
  Real misfit = (log_prior - log_post) - half_nr_log2pi - half_log_det;

  s <<   "<<<<< Best misfit              ="
    << "\n                     " << std::setw(wpp7) << misfit
    << "\n<<<<< Best log prior           =" 
    << "\n                     " << std::setw(wpp7) << log_prior
    << "\n<<<<< Best log posterior       ="
    << "\n                     " << std::setw(wpp7) << log_post << std::endl;

  /*
  // --------------------------
  // Multipoint results summary
  // --------------------------
  std::map<Real, RealVector>::const_iterator it;
  size_t i, j, num_best = bestSamples.size(), wpp7 = write_precision+7;
  for (it=bestSamples.begin(), i=1; it!=bestSamples.end(); ++it, ++i) {
    s << "<<<<< Best parameters          ";
    if (num_best > 1) s << "(set " << i << ") ";
    s << "=\n";
    RealVector best_sample = it->second;
    for (j=0; j<numContinuousVars; ++j)
      s << "                     " << std::setw(wpp7) << best_sample[j] << '\n';
    s << "<<<<< Best log posterior       ";
    if (num_best > 1) s << "(set " << i << ") ";
    s << "=\n                     " << std::setw(wpp7) << it->first << '\n';
  }
  */
  
  // Print final stats for variables and responses 
  NonDBayesCalibration::print_results(s, results_state);
}

double NonDQUESOBayesCalibration::
dakotaLogLikelihood(const QUESO::GslVector& paramValues,
		    const QUESO::GslVector* paramDirection,
		    const void* functionDataPtr, QUESO::GslVector* gradVector,
		    QUESO::GslMatrix* hessianMatrix,
		    QUESO::GslVector* hessianEffect)
{
  // The GP/KRIGING/NO EMULATOR may use an unstandardized space (original)
  // and the PCE or SC cases always use standardized space.
  //
  // We had discussed having QUESO search in the original space:  this may 
  // difficult for high dimensional spaces depending on the scaling, 
  // because QUESO calculates the volume of the hypercube in which it is 
  // searching and will stop if it is too small (e.g. if one input is 
  // of small magnitude, searching in the original space will not be viable).

  // Set the calibration variables and hyperparams in the outer
  // residualModel: note that this won't update the Variables object
  // in any inner models.
  RealVector& all_params = nonDQUESOInstance->
    residualModel.current_variables().continuous_variables_view();
  nonDQUESOInstance->copy_gsl(paramValues, all_params);

  nonDQUESOInstance->residualModel.evaluate();
  
  const RealVector& residuals = 
    nonDQUESOInstance->residualModel.current_response().function_values();
  double log_like = nonDQUESOInstance->log_likelihood(residuals, all_params);
  
  if (nonDQUESOInstance->outputLevel >= DEBUG_OUTPUT) {
    Cout << "Log likelihood is " << log_like << " Likelihood is "
         << std::exp(log_like) << '\n';

    std::ofstream LogLikeOutput;
    LogLikeOutput.open("NonDQUESOLogLike.txt", std::ios::out | std::ios::app);
    // Note: parameter values are in scaled space, if scaling is
    // active; residuals may be scaled by covariance
    size_t num_total_params = 
      nonDQUESOInstance->numContinuousVars + nonDQUESOInstance->numHyperparams; 
    for (size_t i=0; i<num_total_params; ++i) 
      LogLikeOutput << paramValues[i] << ' ' ;
    for (size_t i=0; i<residuals.length(); ++i)
      LogLikeOutput << residuals[i] << ' ' ;
    LogLikeOutput << log_like << '\n';
    LogLikeOutput.close();
  }

  return log_like;
}


void NonDQUESOBayesCalibration::
copy_gsl(const QUESO::GslVector& qv, RealVector& rv)
{
  size_t i, size_qv = qv.sizeLocal();
  if (size_qv != rv.length())
    rv.sizeUninitialized(size_qv);
  for (i=0; i<size_qv; ++i)
    rv[i] = qv[i];
}


void NonDQUESOBayesCalibration::
copy_gsl(const RealVector& rv, QUESO::GslVector& qv)
{
  size_t i, size_rv = rv.length();
  // this resize is not general, but is adequate for current uses:
  if (size_rv != qv.sizeLocal())
    qv = paramSpace->zeroVector();//(size_rv);
  for (i=0; i<size_rv; ++i)
    qv[i] = rv[i];
}


void NonDQUESOBayesCalibration::
copy_gsl_partial(const QUESO::GslVector& qv, size_t start_qv, RealVector& rv)
{
  // copy part of qv into all of rv
  size_t ri, qi, size_rv = rv.length();
  for (ri=0, qi=start_qv; ri<size_rv; ++ri, ++qi)
    rv[ri] = qv[qi];
}


void NonDQUESOBayesCalibration::
copy_gsl_partial(const RealVector& rv, QUESO::GslVector& qv, size_t start_qv)
{
  // copy all of rv into part of qv
  size_t ri, qi, size_rv = rv.length();
  for (ri=0, qi=start_qv; ri<size_rv; ++ri, ++qi)
    qv[qi] = rv[ri];
}


void NonDQUESOBayesCalibration::
copy_gsl(const QUESO::GslVector& qv, RealMatrix& rm, int col)
{
  size_t i, size_qv = qv.sizeLocal();
  if (col < 0 || col >= rm.numCols() || size_qv != rm.numRows()) {
    Cerr << "Error: inconsistent matrix access in copy_gsl()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  Real* rm_c = rm[col];
  for (i=0; i<size_qv; ++i)
    rm_c[i] = qv[i];
}


bool NonDQUESOBayesCalibration::
equal_gsl(const QUESO::GslVector& qv1, const QUESO::GslVector& qv2)
{
  size_t size_qv1 = qv1.sizeLocal();
  if (size_qv1 != qv2.sizeLocal())
    return false;
  for (size_t i=0; i<size_qv1; ++i)
    if (qv1[i] != qv2[i])
      return false;
  return true;
}


} // namespace Dakota
