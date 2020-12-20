/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMUQBayesCalibration
//- Description: Derived class for Bayesian inference using MUQ
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "NonDMUQBayesCalibration.hpp"
#include "MUQ/Modeling/WorkGraphPiece.h"
#include "MUQ/Modeling/ModGraphPiece.h"
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "ProbabilityTransformation.hpp"
#include "NonDSampling.hpp"
#include "PRPMultiIndex.hpp"
#include "MUQ/Utilities/RandomGenerator.h"
#include "MUQ/Utilities/AnyHelpers.h"
#include "WorkdirHelper.hpp"


#include <boost/property_tree/ptree.hpp>

namespace Dakota {

extern PRPCache data_pairs; // global container


// initialization of statics, not being used yet
NonDMUQBayesCalibration* NonDMUQBayesCalibration::nonDMUQInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMUQBayesCalibration::
NonDMUQBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  numBestSamples(1),
  mcmcType(probDescDB.get_string("method.nond.mcmc_type"))
{
}


NonDMUQBayesCalibration::~NonDMUQBayesCalibration()
{ }


double MUQLikelihood::LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {

  // Extract parameter vector from MUQ's inputs vector
  // (which lives in the ModPiece base class)
  Eigen::VectorXd const& c_vars = inputs.at(0);
  size_t i, num_cv = c_vars.size();

  // Set the calibration variables and hyperparams in the outer
  // residualModel: note that this won't update the Variables object
  // in any inner models.
  RealVector& all_params = nonDMUQInstancePtr->residualModel.current_variables().continuous_variables_view();
  // Set parameter values in Dakota model object
  for (i=0; i<num_cv; ++i)
   all_params[i] = c_vars[i];

  nonDMUQInstancePtr->residualModel.evaluate();
  
  const RealVector& residuals = nonDMUQInstancePtr->residualModel.current_response().function_values();
  double log_like = nonDMUQInstancePtr->log_likelihood(residuals, all_params);
  
  if (nonDMUQInstancePtr->outputLevel >= DEBUG_OUTPUT) {
    Cout << "Log likelihood is " << log_like << " Likelihood is "
         << std::exp(log_like) << '\n';

    std::ofstream LogLikeOutput;
    LogLikeOutput.open("NonDMUQLogLike.txt", std::ios::out | std::ios::app);
    // Note: parameter values are in scaled space, if scaling is
    // active; residuals may be scaled by covariance
    size_t num_total_params = nonDMUQInstancePtr->numContinuousVars + nonDMUQInstancePtr->numHyperparams; 
    for (size_t i=0; i<num_total_params; ++i) 
      LogLikeOutput << c_vars[i] << ' ' ;
    for (size_t i=0; i<residuals.length(); ++i)
      LogLikeOutput << residuals[i] << ' ' ;
    LogLikeOutput << log_like << '\n';
    LogLikeOutput.close();
  }

  return log_like;
}


double MUQPrior::LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {
  
  // Extract parameter vector from MUQ's inputs vector
  // (which lives in the ModPiece base class)
  Eigen::VectorXd const& c_vars = inputs.at(0);
  size_t i, num_cv = c_vars.size();

  // Set the calibration variables and hyperparams in the outer
  // residualModel: note that this won't update the Variables object
  // in any inner models.
  RealVector& all_params = nonDMUQInstancePtr->residualModel.current_variables().continuous_variables_view();
  // Set parameter values in Dakota model object
  for (i=0; i<num_cv; ++i)
   all_params[i] = c_vars[i];

  double log_prior = nonDMUQInstancePtr->log_prior_density(all_params);

  return log_prior;
}

void NonDMUQBayesCalibration::specify_prior()
{
  nonDMUQInstance = this;
  distPtr = std::make_shared<muq::Modeling::Distribution>(numContinuousVars);
  MUQPriorPtr = std::make_shared<MUQPrior>(nonDMUQInstance, distPtr);
}

void NonDMUQBayesCalibration::specify_likelihood()
{
  MUQLikelihoodPtr = std::make_shared<MUQLikelihood>(nonDMUQInstance, distPtr);
}

void NonDMUQBayesCalibration::init_bayesian_solver()
{

  // instantiate QUESO objects and execute
  //nonDMUQInstance = this;

  // build the emulator and initialize transformations, as needed
  //initialize_model();

  //distPtr = std::make_shared<muq::Modeling::Distribution>(numContinuousVars);

  parameterPtr = std::make_shared<muq::Modeling::IdentityOperator>(numContinuousVars);
  workGraph = std::make_shared<muq::Modeling::WorkGraph>();
  //MUQPriorPtr = std::make_shared<MUQPrior>(nonDMUQInstance, distPtr);
  //MUQLikelihoodPtr = std::make_shared<MUQLikelihood>(nonDMUQInstance, distPtr);
  //posteriorPtr = std::make_shared<muq::Modeling::DensityProduct>(2);

  workGraph->AddNode(parameterPtr,      "Parameters");
  workGraph->AddNode(MUQLikelihoodPtr,  "Likelihood");
  workGraph->AddNode(MUQPriorPtr,       "Prior");
  workGraph->AddNode(posteriorPtr,      "Posterior");

  workGraph->AddEdge("Parameters", 0, "Prior", 0); // 0 = index of input,output
  workGraph->AddEdge("Parameters", 0, "Likelihood", 0); // 0 = index of input,output
  workGraph->AddEdge("Prior",      0, "Posterior", 0);
  workGraph->AddEdge("Likelihood", 0, "Posterior", 1);

  // Dump out a visualization of the work graph
  if (outputLevel >= DEBUG_OUTPUT)
    workGraph->Visualize("muq_graph.pdf");

  /////////////////////////////
  // DEFINE THE MCMC SAMPLER //
  /////////////////////////////
  // input specification is communicated using Boost property trees
  boost::property_tree::ptree pt; // TO DO: look at options...
  int N =  (chainSamples > 0) ? chainSamples : 1000;
  pt.put("NumSamples", N); // number of Monte Carlo samples
  pt.put("PrintLevel",0);
  pt.put("KernelList", "Kernel1"); // the transition kernel
  pt.put("Kernel1.Method","MHKernel");
  pt.put("Kernel1.Proposal", "MyProposal"); // the proposal

  // Metropolis-Hastings with or without adaptivity
  if (mcmcType == "metropolis_hastings")
      pt.put("Kernel1.MyProposal.Method","MHProposal");
  else if (mcmcType == "adaptive_metropolis")
      pt.put("Kernel1.MyProposal.Method","AMProposal");

  // the variance (diagonal) of the isotropic MH proposal, set to an arbitrary small value
  // due to current lack of ability to set MUQ's initial proposal covariance matrix
  pt.put("Kernel1.MyProposal.ProposalVariance", 0.001);
  pt.put("Kernel1.MyProposal.AdaptSteps",100);
  pt.put("Kernel1.MyProposal.AdaptStart",100);
  pt.put("Kernel1.MyProposal.AdaptScale",1.0);

  auto dens = workGraph->CreateModPiece("Posterior");

  auto problem = std::make_shared<muq::SamplingAlgorithms::SamplingProblem>(dens);

  mcmc = std::make_shared<muq::SamplingAlgorithms::SingleChainMCMC>(pt,problem);

}


void NonDMUQBayesCalibration::specify_posterior()
{
  posteriorPtr = std::make_shared<muq::Modeling::DensityProduct>(2);
}



/** Perform the uncertainty quantification */
void NonDMUQBayesCalibration::calibrate()
{
  int N =  (chainSamples > 0) ? chainSamples : 1000;
  size_t i, num_cv = numContinuousVars;
  const RealVector& init_point = nonDMUQInstance->mcmcModel.continuous_variables();
  Eigen::VectorXd init_pt(num_cv);
  for (i=0; i<num_cv; ++i)
    init_pt[i] = init_point[i];

  Cout << "Running Bayesian Calibration with MUQ " << mcmcType << " using "
       << N << " MCMC samples." << std::endl;

  muq::Utilities::RandomGenerator::SetSeed(randomSeed);

  samps = mcmc->Run(init_pt);

  bfs::path wd( WorkdirHelper::rel_to_abs("MUQDiagnostics") );
  WorkdirHelper::create_directory(wd, DIR_CLEAN);
  const std::string filename = "MUQDiagnostics/mcmc_output.h5";
  samps->WriteToFile(filename);

  // update bestSamples with highest posterior probability points
  log_best();

  // populate acceptanceChain, acceptedFnVals
  cache_chain();

  // Generate useful stats from the posterior samples
  //compute_statistics();
}

void NonDMUQBayesCalibration::log_best()
{
  bestSamples.clear();

  double log_posterior;
  Eigen::VectorXd sample;
  RealVector mcmc_rv(numContinuousVars, false);
  std::shared_ptr<muq::SamplingAlgorithms::SamplingState> states;

  // for each sample
  for( unsigned int i=0; i<samps->size(); ++i ) {

    states = samps->at(i);

    // get mcmc state vector
    sample = states->state.at(0);

    for (int j=0; j<numContinuousVars; ++j)
        mcmc_rv(j) = sample(j);

    auto it = states->meta.find("LogTarget");
    if( it != states->meta.end() ){
      log_posterior = muq::Utilities::AnyCast(states->meta["LogTarget"]);
      bestSamples.insert(std::make_pair(log_posterior, mcmc_rv));
      if (bestSamples.size() > numBestSamples)
        bestSamples.erase(bestSamples.begin()); // pop front (lowest prob)
    }

  }

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "bestSamples map:\n" << bestSamples << std::endl;
}

void NonDMUQBayesCalibration::
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
  Real log_prior = log_prior_density(best_sample), log_post = it->first;
  size_t num_total_calib_terms = residualModel.num_primary_fns();
  Real half_nr_log2pi = num_total_calib_terms * HALF_LOG_2PI;
  RealVector hyper_params(numHyperparams);
  Real half_log_det = 
    expData.half_log_cov_determinant(hyper_params, obsErrorMultiplierMode);
  Real misfit = (log_prior - log_post) - half_nr_log2pi - half_log_det;

  s <<   "<<<<< Best misfit              ="
    << "\n                     " << std::setw(wpp7) << misfit
    << "\n<<<<< Best log prior           =" 
    << "\n                     " << std::setw(wpp7) << log_prior
    << "\n<<<<< Best log posterior       ="
    << "\n                     " << std::setw(wpp7) << log_post << std::endl;
  
  // Print final stats for variables and responses 
  NonDBayesCalibration::print_results(s, results_state);
}

void NonDMUQBayesCalibration::
print_variables(std::ostream& s, const RealVector& c_vars)
{
  StringMultiArrayConstView cv_labels =
    iteratedModel.continuous_variable_labels();
  // the residualModel includes any hyper-parameters
  StringArray combined_labels;
  copy_data(residualModel.continuous_variable_labels(), combined_labels);

  size_t wpp7 = write_precision+7;

  // print MAP for continuous random variables
  if (standardizedSpace) {
    RealVector u_rv(Teuchos::View, c_vars.values(), numContinuousVars);
    RealVector x_rv;
    mcmcModel.probability_transformation().trans_U_to_X(u_rv, x_rv);
    write_data(Cout, x_rv, cv_labels);
  }
  else
    for (size_t j=0; j<numContinuousVars; ++j)
      s << "                     " << std::setw(wpp7) << c_vars[j]
  << ' ' << cv_labels[j] << '\n';
  // print MAP for hyper-parameters (e.g., observation error params)
  for (size_t j=0; j<numHyperparams; ++j)
    s << "                     " << std::setw(wpp7)
      << c_vars[numContinuousVars+j] << ' '
      << combined_labels[numContinuousVars + j] << '\n';
}


/** Populate all of acceptanceChain(num_params, chainSamples)
    acceptedFnVals(numFunctions, chainSamples) */
void NonDMUQBayesCalibration::cache_chain()
{
  acceptanceChain.shapeUninitialized(numContinuousVars + numHyperparams,
             chainSamples);
  acceptedFnVals.shapeUninitialized(numFunctions, chainSamples);

  // temporaries for evals/lookups
  // the MCMC model omits the hyper params and residual transformations...
  Variables lookup_vars = nonDMUQInstance->mcmcModel.current_variables().copy();
  String   interface_id = nonDMUQInstance->mcmcModel.interface_id();
  Response  lookup_resp = nonDMUQInstance->mcmcModel.current_response().copy();
  ActiveSet   lookup_as = lookup_resp.active_set();
  lookup_as.request_values(1);
  lookup_resp.active_set(lookup_as);
  ParamResponsePair lookup_pr(lookup_vars, interface_id, lookup_resp);


  Eigen::MatrixXd const& chain = samps->AsMatrix();

  unsigned int num_mcmc = chain.cols();
  if (num_mcmc != chainSamples) {
    Cerr << "\nError: MUQ cache_chain(): chain length is " << num_mcmc
   << "; expected " << chainSamples << '\n';
    abort_handler(METHOD_ERROR);
  }

  unsigned int lookup_failures = 0;
  unsigned int num_params = numContinuousVars + numHyperparams;
  for (int i=0; i<num_mcmc; ++i) {

    // translate the output matrix into x- or u-space lookup vars and
    // x-space acceptanceChain
    if (standardizedSpace) {
      // u_rv and x_rv omit any hyper-parameters
      RealVector u_rv(numContinuousVars, false);
      for (int j=0; j<numContinuousVars; ++j)
        u_rv(j) = chain(j,i);

      Real* acc_chain_i = acceptanceChain[i];
      RealVector x_rv(Teuchos::View, acc_chain_i, numContinuousVars);
      mcmcModel.probability_transformation().trans_U_to_X(u_rv, x_rv);
      for (int j=numContinuousVars; j<num_params; ++j){

        acc_chain_i[j] = chain(j,i); // trailing hyperparams are not transformed
        
      }
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
      for (int j=0; j<numContinuousVars; ++j){
        theta_hp(j) = chain(j,i);
        // std::cout << chain(j,i) << std::endl;
      }
      // lookup vars only need the calibration parameters
      RealVector x_rv(Teuchos::View, acceptanceChain[i], 
          numContinuousVars);
      lookup_vars.continuous_variables(x_rv);
    }

    // now retreive function values

    if (mcmcModelHasSurrogate) {
      nonDMUQInstance->mcmcModel.active_variables(lookup_vars);
      nonDMUQInstance->mcmcModel.evaluate(lookup_resp.active_set());
      const RealVector& fn_vals = nonDMUQInstance->mcmcModel.current_response().function_values();
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

} // namespace Dakota
