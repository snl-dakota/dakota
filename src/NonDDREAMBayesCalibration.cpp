/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDDREAMBayesCalibration
//- Description: Derived class for Bayesian inference using DREAM
//- Owner:       Brian Adams
//- Checked by:
//- Version:

#include "NonDDREAMBayesCalibration.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "PRPMultiIndex.hpp"

// BMA TODO: remove this header
// for uniform PDF and samples
//#include "pdflib.hpp"

// included to set seed in RNGLIB:
#include "rnglib.hpp"

// for core dream functionality
using std::string;
#include "dream.hpp"

static const char rcsId[]="@(#) $Id$";

// five forwards to the class static functions
// BMA TODO: change interface to pass function pointers and
// initializers and remove these globals

namespace dream {

/// forwarder to problem_size needed by DREAM
void 
problem_size (int &chain_num, int &cr_num, int &gen_num, int &pair_num, 
	      int &par_num)
{
  Dakota::NonDDREAMBayesCalibration::
    problem_size(chain_num, cr_num, gen_num, pair_num, par_num);
}

/// forwarder to problem_value needed by DREAM
void 
problem_value (std::string *chain_filename, std::string *gr_filename,
	       double &gr_threshold, int &jumpstep, double limits[], 
	       int par_num, int &printstep, std::string *restart_read_filename,
	       std::string *restart_write_filename)
{
  Dakota::NonDDREAMBayesCalibration::
    problem_value(chain_filename, gr_filename, gr_threshold, jumpstep, limits, 
		  par_num, printstep, restart_read_filename, 
		  restart_write_filename);
}

/// forwarder to prior_density needed by DREAM
double prior_density (int par_num, double zp[]) 
{
  return Dakota::NonDDREAMBayesCalibration::prior_density(par_num, zp); 
}

/// forwarder to prior_sample needed by DREAM
double* prior_sample (int par_num)
{
  return Dakota::NonDDREAMBayesCalibration::prior_sample(par_num);
}

/// forwarder to sample_likelihood needed by DREAM
double sample_likelihood (int par_num, double zp[])
{
  return Dakota::NonDDREAMBayesCalibration::sample_likelihood(par_num, zp);
}


} // namespace dream


namespace Dakota {

extern PRPCache data_pairs; // global container

//initialization of statics
NonDDREAMBayesCalibration* NonDDREAMBayesCalibration::nonDDREAMInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDDREAMBayesCalibration::
NonDDREAMBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  numChains(probDescDB.get_int("method.dream.num_chains")),
  numCR(probDescDB.get_int("method.dream.num_cr")),
  crossoverChainPairs(probDescDB.get_int("method.dream.crossover_chain_pairs")),
  grThreshold(probDescDB.get_real("method.dream.gr_threshold")),
  jumpStep(probDescDB.get_int("method.dream.jump_step"))
{ 
  // don't use max_function_evaluations, since we have num_samples
  // consider max_iterations = generations, and adjust as needed?

  Cout << "INFO (DREAM): requested samples = " << chainSamples
       << "\nINFO (DREAM): requested chains = " << numChains << std::endl;

  if (numChains < 3) {
    numChains = 3;
    Cout << "WARN (DREAM): Increasing requested chains to minimum (3)"
	 << std::endl;
  }

  numGenerations = std::floor((Real)chainSamples/numChains);
  if (numGenerations < 2) {
    numGenerations = 2;
    chainSamples = numGenerations * numChains;
    Cout << "WARN (DREAM): generations = samples / chains = " << numGenerations 
	 << " is less than 2.\n             setting generations = 2, for "
	 << chainSamples << " total samples." << std::endl;
  }
  else {
    chainSamples = numGenerations * numChains;
    Cout << "INFO (DREAM): will use " << numChains << " chains with "
	 << numGenerations << " generations,\nfor " << chainSamples 
	 << " total samples." << std::endl;
  }

  if (numCR < 1) {
    numCR = 1;
    Cout << "WARN (DREAM): num_cr < 1, resetting to 3 (default)." << std::endl;
  }

  if (crossoverChainPairs < 0) {
    numCR = 3;
    Cout << "WARN (DREAM): crossover_chain_pairs < 0, resetting to 3 (default)."
	 << std::endl;
  }

  if (grThreshold < 0.0) {
    grThreshold = 1.2;
    Cout << "WARN (DREAM): gr_threshold < 0.0, resetting to 1.2 (default)." 
	 << std::endl;
  }

  if (jumpStep < 1) {
    jumpStep = 5;
    Cout << "WARN (DREAM): jump_step < 1, resetting to 5 (default)." 
	 << std::endl;
  }
}


NonDDREAMBayesCalibration::~NonDDREAMBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDDREAMBayesCalibration::calibrate()
{
  // instantiate DREAM objects and execute
  nonDDREAMInstance = this;

  // diagnostic information
  Cout << "INFO (DREAM): Standardized space " << standardizedSpace << '\n';
  Cout << "INFO (DREAM): Num Samples " << chainSamples << '\n';
  Cout << "INFO (DREAM): Calibrating " << numHyperparams 
       << " error hyperparameters.\n";
 
  // initialize the mcmcModel (including emulator construction) if needed
  initialize_model();

  // Set seed in both local generator and the one underlying DREAM in RNGLIB
  // BMA TODO: Burkhardt says replace RNGLIB with Dakota RNG
  dream::set_seed(randomSeed, randomSeed);
  rnumGenerator.seed(randomSeed);

  // BMA TODO: share most of this code with QUESO class

  // BMA TODO: support independent options
  if (obsErrorMultiplierMode > 0 && !calibrationData) {
    Cerr << "\nError: you are attempting to calibrate the measurement error "
         << "but have not provided experimental data information."<<std::endl;
    abort_handler(METHOD_ERROR);
  }

  // DREAM likelihood needs fn_vals only
  // short request_value_needed = 1;
  // init_residual_response(request_value_needed);
  // BMA TODO: Make sure residualModel has necessary ASV/sizing


  ////////////////////////////////////////////////////////
  // Step 2 of 5: Instantiate the parameter domain
  ////////////////////////////////////////////////////////
  int total_num_params = numContinuousVars + numHyperparams;
  
  const RealVector& init_point = mcmcModel.continuous_variables();
  Cout << "Initial Points " << init_point << '\n';

  // resize, initializing to zero
  paramMins.size(total_num_params);
  paramMaxs.size(total_num_params);
  RealRealPairArray bnds = (standardizedSpace) ?
    natafTransform.u_bounds() : natafTransform.x_bounds();
  for (size_t i=0; i<numContinuousVars; ++i)
    { paramMins[i] = bnds[i].first; paramMaxs[i] = bnds[i].second; }
  // If calibrating error multipliers, the parameter domain is expanded to
  // estimate hyperparameters sigma^2 that multiply any user-provided covariance
  // BMA TODO: change from uniform to inverse gamma prior and allow control for 
  // cases where user didn't give covariance information
  for (size_t i=0; i<numHyperparams; ++i) {
    paramMins[numContinuousVars + i] = .01;
    paramMaxs[numContinuousVars + i] =  2.;
  }
 
  Cout << "INFO (DREAM): number hyperparams = " << numHyperparams << '\n';
  Cout << "INFO (DREAM): paramMins  " << paramMins << '\n';
  Cout << "INFO (DREAM): paramMaxs  " << paramMaxs << '\n';
  
  ////////////////////////////////////////////////////////
  // Step 3 of 5: Instantiate the likelihood function object
  ////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////
  // Step 4 of 5: Instantiate the inverse problem
  ////////////////////////////////////////////////////////
  
  /*
  // initialize the prior PDF and sampler
  // the prior is assumed uniform for DREAM
  priorDistributions.clear(); // clear since this is a run-time operation
  priorSamplers.clear();
  for (int i=0; i<total_num_params; ++i) {
    priorDistributions.push_back(boost::math::uniform(paramMins[i], paramMaxs[i]));
    priorSamplers.push_back(boost::uniform_real<double>(paramMins[i], paramMaxs[i]));
  }
  */
  
  ////////////////////////////////////////////////////////
  // Step 5 of 5: Solve the inverse problem
  ////////////////////////////////////////////////////////

  // BMA TODO: set initial point and proposal covariance if relevant

  // uqGslVectorClass paramInitials(paramSpace.zeroVector());
  // uqGslVectorClass covDiag(paramSpace.zeroVector());
  // //uqGslMatrixClass proposalCovMatrix(paramSpace.zeroVector());
  // for (int i=0;i<numContinuousVars;i++) {
  //   if (init_point[i])
  //     paramInitials[i]=init_point[i];
  //   else 
  //     paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
  // }
  // for (int i=numContinuousVars;i<total_num_params;i++) {
  //   paramInitials[i]=(paramMaxs[i]+paramMins[i])/2.0;
  // }
  // Cout << "initParams " << paramInitials << '\n';

  // uqGslMatrixClass* proposalCovMatrix = postRv.imageSet().vectorSpace().newProposalMatrix(&covDiag,&paramInitials); 
  // //uqGslMatrixClass proposalCovMatrix(covDiag);
  // Cout << "ProposalCovMatrix " << '\n'; 
  // for (size_t i=0;i<total_num_params;i++) {
  //   for (size_t j=0;j<total_num_params;j++) 
  //      Cout <<  (*proposalCovMatrix)(i,j) << "  " ; 
  // }
  // ip.solveWithBayesMetropolisHastings(calIpMhOptionsValues,
  //                                   paramInitials, proposalCovMatrix);

  Cout << "INFO (DREAM): Running DREAM for Bayesian inference." << std::endl;
  /// DREAM will callback to cache_chain to store the chain
  dream_main(cache_chain);

  // get the function values corresponding to the acceptance chain
  archive_acceptance_chain();

  // Generate useful stats from the posterior samples
  compute_statistics();

}


  // BMA: Remove?
void NonDDREAMBayesCalibration::print_results(std::ostream& s)
{
  // Print variables and response function final stats
  NonDBayesCalibration::print_results(s);
  
  //additional DREAM output
}

// BMA TODO: share most of this code with QUESO, general hyperpriors, other distros
/** Static callback function to evaluate the likelihood */
double NonDDREAMBayesCalibration::sample_likelihood(int par_num, double zp[])
{
  RealVector all_params(Teuchos::View, zp, par_num);

  // DREAM searches in either the original space (default for GPs and no
  // emulator) or standardized space (PCE/SC, optional for GP/no emulator).  
  nonDDREAMInstance->residualModel.continuous_variables(all_params); 

  // Compute simulation response to use in likelihood 
  nonDDREAMInstance->residualModel.evaluate();
  const RealVector& residuals = 
    nonDDREAMInstance->residualModel.current_response().function_values();
  double log_like = nonDDREAMInstance->log_likelihood(residuals, all_params);

  if (nonDDREAMInstance->outputLevel >= DEBUG_OUTPUT) {
    Cout << "Log likelihood is " << log_like << " Likelihood is "
         << std::exp(log_like) << '\n';

    std::ofstream LogLikeOutput;
    LogLikeOutput.open("NonDDREAMLogLike.txt", std::ios::out | std::ios::app);
    // Note: parameter values are in scaled space, if scaling is
    // active; residuals may be scaled by covariance
    for (size_t i=0; i<par_num;  ++i) LogLikeOutput << zp[i] << ' ' ;
    for (size_t i=0; i<residuals.length(); ++i)
      LogLikeOutput << residuals(i) << ' ' ;
    LogLikeOutput << log_like << '\n';
    LogLikeOutput.close();
  }
  return log_like;
}


/** See documentation in DREAM examples) */			     
void NonDDREAMBayesCalibration::
problem_size(int &chain_num, int &cr_num, int &gen_num, int &pair_num,
	     int &par_num)
{
  chain_num = nonDDREAMInstance->numChains;
  cr_num    = nonDDREAMInstance->numCR;
  gen_num   = nonDDREAMInstance->numGenerations;
  pair_num  = nonDDREAMInstance->crossoverChainPairs;
  par_num   = nonDDREAMInstance->numContinuousVars + 
    nonDDREAMInstance->numHyperparams;

  return;
}

/** See documentation in DREAM examples) */			     
void  NonDDREAMBayesCalibration::
problem_value(string *chain_filename, string *gr_filename, double &gr_threshold,
	      int &jumpstep, double limits[], int par_num, int &printstep, 
	      string *restart_read_filename, string *restart_write_filename)
{
  int j;

  // BMA TODO: 
  // * Allow user to set the output filenames

  // parameters to expose to user...

  // DREAM will replace the zeros with unique file tags; use one
  // placeholder for 1--10 chains, two for 11--100 chains, etc.
  int chain_tag_len = 1;
  if (nonDDREAMInstance->numChains > 10)
    chain_tag_len = 
      (int) std::ceil(std::log10((double) nonDDREAMInstance->numChains));
  std::string chain_tag(chain_tag_len, '0');
  std::string chain_fname("dakota_dream_chain");
  chain_fname += chain_tag + ".txt";

  *chain_filename = chain_fname.c_str();
  *gr_filename = "dakota_dream_gr.txt";
  gr_threshold = nonDDREAMInstance->grThreshold;
  jumpstep = nonDDREAMInstance->jumpStep;

  for ( j = 0; j < par_num; j++ )
  {
    limits[0+j*2] = nonDDREAMInstance->paramMins[j];
    limits[1+j*2] = nonDDREAMInstance->paramMaxs[j];
    Cout << "min " << j << " = " << limits[0+j*2] << std::endl; 
    Cout << "max " << j << " = " << limits[1+j*2] << std::endl; 
  }

  // parameters to expose to user...
  printstep = 10;
  *restart_read_filename = "";
  *restart_write_filename = "dakota_dream_restart.txt";

  return;
}

/** See documentation in DREAM examples */
double  NonDDREAMBayesCalibration::prior_density ( int par_num, double zp[] )
{
  Dakota::RealVector vec(Teuchos::View, zp, par_num);
  return nonDBayesInstance->prior_density(vec);

  /*
  int i;
  double value = 1.0;

  for ( i = 0; i < par_num; i++ )    
  {
    //value = value * r8_uniform_pdf (nonDDREAMInstance->paramMins[i],
    //				    nonDDREAMInstance->paramMaxs[i], zp[i] );
    value *= boost::math::pdf(nonDDREAMInstance->priorDistributions[i], zp[i]);
  }

  return value;
  */
}


/** See documentation in DREAM examples */			     
double *  NonDDREAMBayesCalibration::prior_sample ( int par_num )
{
  // This is not a memory leak, as DREAM deallocates on their side (the API was
  // not designed for the injection of this function into the DREAM namespace)
  double *zp = ( double * ) malloc ( par_num * sizeof ( double ) );

  RealVector prior_dist_samples(Teuchos::View, zp, par_num);
  nonDBayesInstance->
    prior_sample(nonDDREAMInstance->rnumGenerator, prior_dist_samples);

  /*
  for ( int i = 0; i < par_num; i++ )
  {
    //zp[i] = r8_uniform_sample ( nonDDREAMInstance->paramMins[i], 
    //				  nonDDREAMInstance->paramMaxs[i] );
    zp[i] =
      nonDDREAMInstance->priorSamplers[i](nonDDREAMInstance->rnumGenerator);
  }
  */

  return zp;
}


/** Archive the chain from DREAM.  This default implementation is
    aggregating from the parallel chains in a round-robin fashion. */
void NonDDREAMBayesCalibration::cache_chain(const double* const z) 
{
  int par_num = 
    nonDDREAMInstance->numContinuousVars + nonDDREAMInstance->numHyperparams;
  int num_samples = 
    nonDDREAMInstance->numGenerations * nonDDREAMInstance->numChains;

  // TODO: verify whether the chain coming back is equivalent to an
  // accepted chain
  nonDDREAMInstance->acceptanceChain.shape(par_num, num_samples);

  // BMA: loops taken from dream.cpp chain_write; don't change
  // indexing for the z vector (may need to update where the samples
  // are placed in acceptanceChain)
  for (int k=0; k<nonDDREAMInstance->numGenerations; ++k)
    for (int j=0; j<nonDDREAMInstance->numChains; ++j) 
      for (int i=0; i<par_num; ++i)
        nonDDREAMInstance->acceptanceChain
	  (i, k*nonDDREAMInstance->numChains + j) =
	  z[i+j*par_num+k*par_num*nonDDREAMInstance->numChains];
}

void NonDDREAMBayesCalibration::archive_acceptance_chain()
{
  // temporaries for evals/lookups
  // the MCMC model omits the hyper params and residual transformations...
  Variables lookup_vars = mcmcModel.current_variables().copy();
  String interface_id = mcmcModel.interface_id();
  Response lookup_resp = mcmcModel.current_response().copy();
  ActiveSet lookup_as = lookup_resp.active_set();
  lookup_as.request_values(1);
  lookup_resp.active_set(lookup_as);
  ParamResponsePair lookup_pr(lookup_vars, interface_id, lookup_resp);
 
  int lookup_failures = 0, num_samples = acceptanceChain.numCols();
  acceptedFnVals.shapeUninitialized(numFunctions, num_samples);
  for (int sample_index=0; sample_index < num_samples; ++sample_index) {

    if (standardizedSpace) {
      // u_rv and x_rv omit any hyper-parameters
      RealVector u_rv(Teuchos::Copy, acceptanceChain[sample_index], 
		      numContinuousVars);
      RealVector x_rv(Teuchos::View, acceptanceChain[sample_index], 
		      numContinuousVars);
      natafTransform.trans_U_to_X(u_rv, x_rv);
      // trailing hyperparams are not transformed

      // surrogate needs u-space variables for eval
      if (mcmcModel.model_type() == "surrogate")
	lookup_vars.continuous_variables(u_rv);
      else
	lookup_vars.continuous_variables(x_rv);
    }
    else {
      RealVector x_rv(Teuchos::View, acceptanceChain[sample_index], 
		      numContinuousVars);
      lookup_vars.continuous_variables(x_rv);
    }

    // now retreive function values
    if (mcmcModelHasSurrogate) {
      mcmcModel.active_variables(lookup_vars);
      mcmcModel.evaluate(lookup_resp.active_set());
      const RealVector& fn_vals = mcmcModel.current_response().function_values();
      Teuchos::setCol(fn_vals, sample_index, acceptedFnVals);	
    }
    else {
      lookup_pr.variables(lookup_vars);
      PRPCacheHIter cache_it = lookup_by_val(data_pairs, lookup_pr);
      if (cache_it == data_pairs.get<hashed>().end()) {
	++lookup_failures;
	// Set NaN in the chain points to avoid misleading the user
	RealVector nan_fn_vals(mcmcModel.current_response().function_values().length());
	nan_fn_vals = std::numeric_limits<double>::quiet_NaN();
	Teuchos::setCol(nan_fn_vals, sample_index, acceptedFnVals);
      }
      else {
	const RealVector& fn_vals = cache_it->response().function_values();
	Teuchos::setCol(fn_vals, sample_index, acceptedFnVals);
      }
    }

  }
  if (lookup_failures > 0 && outputLevel > SILENT_OUTPUT)
    Cout << "Warning: could not retrieve function values for " 
	 << lookup_failures << " MCMC chain points." << std::endl;
}


} // namespace Dakota
