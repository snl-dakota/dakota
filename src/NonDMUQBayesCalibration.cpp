/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
#include "MUQ/SamplingAlgorithms/MHProposal.h"
#include "MUQ/SamplingAlgorithms/AMProposal.h"
#include "MUQ/SamplingAlgorithms/MHKernel.h"
#include "MUQ/SamplingAlgorithms/DRKernel.h"
#include "MUQ/SamplingAlgorithms/MALAProposal.h"
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
  mcmcType(probDescDB.get_string("method.nond.mcmc_type")),
  priorPropCovMult(probDescDB.get_real("method.prior_prop_cov_mult")),
  drNumStages(probDescDB.get_int("method.nond.dr_num_stages")),
  drScaleType(probDescDB.get_string("method.nond.dr_scale_type")),
  drScale(probDescDB.get_real("method.nond.dr_scale")),
  amPeriodNumSteps(probDescDB.get_int("method.nond.am_period_num_steps")),
  amStartingStep(probDescDB.get_int("method.nond.am_starting_step")),
  amScale(probDescDB.get_real("method.nond.am_scale")),
  malaStepSize(probDescDB.get_real("method.nond.mala_step_size"))
{
  // default initial proposal covariance choice
  if (proposalCovarType.empty())
    proposalCovarType = "prior";

  if (mcmcType == "mala") {
    const String & grad_type  = residualModel.gradient_type();
    const String & method_src = residualModel.method_source();
    if (( grad_type == "analytic" ) ||
        ( grad_type == "mixed"    ) || 
        ( grad_type == "numerical" && method_src == "dakota" )) {
      // Ok
    }
    else {
      Cerr << "\nError: NonDMUQBayesCalibration::constructor(): MALA is being requested, but code will not able to compute gradient!"
           << " Current gradient type is '" << grad_type << "'."
           << " When MALA is requested, the three acceptable options for 'gradient type' are"
           << " (i) 'analytic', or"
           << " (ii) 'mixed', or"
           << " (iii) 'numerical' (with 'method source' equal to 'dakota')."
           << '\n';
      abort_handler(METHOD_ERROR);
    }
  }
}


NonDMUQBayesCalibration::~NonDMUQBayesCalibration()
{ }


double MUQLikelihood::LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {

  // Extract parameter vector from MUQ's inputs vector
  // (which lives in the ModPiece base class)
  Eigen::VectorXd const& c_vars = inputs.at(0);
  size_t num_cv = c_vars.size();

  // Set the calibration variables and hyperparams in the outer
  // residualModel: note that this won't update the Variables object
  // in any inner models.
  RealVector& all_params = nonDMUQInstancePtr->residualModel.current_variables().continuous_variables_view();
  // Set parameter values in Dakota model object
  for (size_t i(0); i < num_cv; ++i) {
    all_params[i] = c_vars[i];
  }

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
    for (size_t i(0); i < num_total_params; ++i)
      LogLikeOutput << c_vars[i] << ' ' ;
    for (size_t i(0); i < residuals.length(); ++i)
      LogLikeOutput << residuals[i] << ' ' ;
    LogLikeOutput << log_like << '\n';
    LogLikeOutput.close();
  }

  return log_like;
}

Eigen::VectorXd MUQLikelihood::GradLogDensityImpl(unsigned int wrt,
                                                  muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {
  Eigen::VectorXd const & input_vec(inputs.at(0));
  size_t input_vec_size(input_vec.size());
  Eigen::VectorXd output_vec(input_vec_size);

  {
    RealVector & model_params(nonDMUQInstancePtr->mcmcModel.current_variables().continuous_variables_view());
    for (size_t i(0); i < input_vec_size; ++i) {
      model_params[i] = input_vec[i];
    }

    nonDMUQInstancePtr->mcmcModel.evaluate();
  }

  {
    RealVector & model_params(nonDMUQInstancePtr->residualModel.current_variables().continuous_variables_view());
    for (size_t i(0); i < input_vec_size; ++i) {
      model_params[i] = input_vec[i];
    }

    ActiveSet active_set = nonDMUQInstancePtr->residualModel.current_response().active_set();
    active_set.request_values(3); // gradient, value
    nonDMUQInstancePtr->residualModel.evaluate(active_set);
  }

  const RealVector & residuals(nonDMUQInstancePtr->residualModel.current_response().function_values());
  size_t num_residuals = residuals.numRows();
  RealVector tmp_vec(input_vec_size,true);
  RealVector dakota_result_vec(input_vec_size,true);
  for (size_t r(0); r < num_residuals; ++r) {
    tmp_vec = nonDMUQInstancePtr->residualModel.current_response().function_gradient_view(r);
    tmp_vec *= residuals[r];
    dakota_result_vec -= tmp_vec;
  }

  for (size_t i(0); i < input_vec_size; ++i) {
    output_vec[i] = dakota_result_vec[i];
  }

  return output_vec;
}

double MUQPrior::LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {
  
  // Extract parameter vector from MUQ's inputs vector
  // (which lives in the ModPiece base class)
  Eigen::VectorXd const& c_vars = inputs.at(0);
  size_t num_cv = c_vars.size();

  // Set the calibration variables and hyperparams in the outer
  // residualModel: note that this won't update the Variables object
  // in any inner models.
  RealVector& all_params = nonDMUQInstancePtr->residualModel.current_variables().continuous_variables_view();
  // Set parameter values in Dakota model object
  for (size_t i(0); i < num_cv; ++i)
    all_params[i] = c_vars[i];

  double log_prior = nonDMUQInstancePtr->log_prior_density(all_params);

  return log_prior;
}

Eigen::VectorXd MUQPrior::GradLogDensityImpl(unsigned int wrt,
                                             muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {
  Eigen::VectorXd const & input_vec(inputs.at(0));
  size_t input_vec_size(input_vec.size());
  Eigen::VectorXd output_vec(input_vec_size);

  RealVector & model_params(nonDMUQInstancePtr->residualModel.current_variables().continuous_variables_view());
  for (size_t i(0); i < input_vec_size; ++i) {
    model_params[i] = input_vec[i];
  }

  RealVector dakota_result_vec(input_vec_size,true);
  nonDMUQInstancePtr->augment_gradient_with_log_prior(dakota_result_vec, model_params);
  dakota_result_vec *= -1.;

  for (size_t i(0); i < input_vec_size; ++i) {
    output_vec[i] = dakota_result_vec[i];
  }

  return output_vec;
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
  init_proposal_covariance();

  parameterPtr = std::make_shared<muq::Modeling::IdentityOperator>(numContinuousVars);
  workGraph = std::make_shared<muq::Modeling::WorkGraph>();

  workGraph->AddNode(parameterPtr,     "Parameters");
  workGraph->AddNode(MUQLikelihoodPtr, "Likelihood");
  workGraph->AddNode(MUQPriorPtr,      "Prior");
  workGraph->AddNode(posteriorPtr,     "Posterior");

  workGraph->AddEdge("Parameters", 0, "Prior",      0); // 0 = index of input,output
  workGraph->AddEdge("Parameters", 0, "Likelihood", 0); // 0 = index of input,output
  workGraph->AddEdge("Prior",      0, "Posterior",  0);
  workGraph->AddEdge("Likelihood", 0, "Posterior",  1);

  // Dump out a visualization of the work graph
  if (outputLevel >= DEBUG_OUTPUT)
    workGraph->Visualize("muq_graph.pdf");

  /////////////////////////////
  // DEFINE THE MCMC SAMPLER //
  /////////////////////////////
  // input specification is communicated using Boost property trees
  boost::property_tree::ptree pt; // TO DO: look at options...
  int N =  (chainSamples > 0) ? chainSamples : 1000;
  pt.put("NumSamples", N);
  pt.put("PrintLevel",0);

  pt.put("KernelList", "Kernel1"); // the transition kernel
  if (mcmcType == "metropolis_hastings" || mcmcType == "adaptive_metropolis" || mcmcType == "mala") {
    pt.put("Kernel1.Method","MHKernel");
  }
  else if (mcmcType == "delayed_rejection" || mcmcType == "dram") {
    pt.put("Kernel1.Method","DRKernel");
  }

  // Delayed rejection knobs
  if (mcmcType == "delayed_rejection" || mcmcType == "dram") {
    pt.put("Kernel1.NumStages",drNumStages);
    pt.put("Kernel1.Scale",drScale);
    pt.put("Kernel1.ScaleType",drScaleType);
  }

  pt.put("Kernel1.Proposal", "MyProposal"); // the proposal
  // Metropolis-Hastings/DR with or without adaptivity
  if (mcmcType == "metropolis_hastings" || mcmcType == "delayed_rejection") {
    pt.put("Kernel1.MyProposal.Method","MHProposal");
  }
  else if (mcmcType == "adaptive_metropolis" || mcmcType == "dram") {
    pt.put("Kernel1.MyProposal.Method","AMProposal");
  }
  else if (mcmcType == "mala") {
    pt.put("Kernel1.MyProposal.Method","MALAProposal");
  }

  // MALA knobs
  if (mcmcType == "mala") {
    pt.put("Kernel1.MyProposal.StepSize",malaStepSize);
  }

  // Adaptive Metropolis knobs
  if (mcmcType == "adaptive_metropolis" || mcmcType == "dram") {
    pt.put("Kernel1.MyProposal.AdaptSteps",amPeriodNumSteps);
    pt.put("Kernel1.MyProposal.AdaptStart",amStartingStep);
    pt.put("Kernel1.MyProposal.AdaptScale",amScale);
  }

  boost::property_tree::ptree kernOpts = pt.get_child("Kernel1");

  auto dens = workGraph->CreateModPiece("Posterior");

  auto problem = std::make_shared<muq::SamplingAlgorithms::SamplingProblem>(dens);

  Eigen::VectorXd propMu = Eigen::VectorXd::Zero(numContinuousVars);
  auto propDist = std::make_shared<muq::Modeling::Gaussian>(propMu, proposalCovMatrix);
  
  // Use the Gaussian proposal distribution to define an MCMC proposal class
  std::shared_ptr<muq::SamplingAlgorithms::MCMCProposal> proposal;
  if (mcmcType == "metropolis_hastings" || mcmcType == "delayed_rejection") {
    proposal = std::make_shared<muq::SamplingAlgorithms::MHProposal>(kernOpts.get_child("MyProposal"), problem, propDist);
  }
  else if (mcmcType == "adaptive_metropolis" || mcmcType == "dram") {
    proposal = std::make_shared<muq::SamplingAlgorithms::AMProposal>(kernOpts.get_child("MyProposal"), problem, proposalCovMatrix);
  }
  else if (mcmcType == "mala") {
    proposal = std::make_shared<muq::SamplingAlgorithms::MALAProposal>(kernOpts.get_child("MyProposal"), problem, propDist);
  }

  // Construct the Metropolis-Hastings (MH) Markov transition kernel using the proposal
  std::vector<std::shared_ptr<muq::SamplingAlgorithms::TransitionKernel>> kernels(1);

  if (mcmcType == "metropolis_hastings" || mcmcType == "adaptive_metropolis") {
    kernels.at(0) = std::make_shared<muq::SamplingAlgorithms::MHKernel>(kernOpts, problem, proposal);
  }
  else if (mcmcType == "delayed_rejection" || mcmcType == "dram") {
    std::vector<std::shared_ptr<muq::SamplingAlgorithms::MCMCProposal>> proposals;
    proposals.resize(drNumStages, proposal);

    // scales for DR
    std::vector<double> scales(drNumStages,1.0);
    if (drScaleType == "Power") {
      for (int i(0); i < drNumStages; ++i) {
        scales.at(i) = drScale / std::pow(2.0, double(i));
      }
    }
    else if (drScaleType == "Linear") {
      for (int i(0); i < drNumStages; ++i) {
        scales.at(i) = drScale / (double(i)+1.0);
      }
    }

    kernels.at(0) = std::make_shared<muq::SamplingAlgorithms::DRKernel>(kernOpts, problem, proposals, scales);
  }
  else if (mcmcType == "mala") {
    kernels.at(0) = std::make_shared<muq::SamplingAlgorithms::MHKernel>(kernOpts, problem, proposal);
  }

  mcmc = std::make_shared<muq::SamplingAlgorithms::SingleChainMCMC>(pt,kernels);
}


void NonDMUQBayesCalibration::specify_posterior()
{
  posteriorPtr = std::make_shared<muq::Modeling::DensityProduct>(2);
}



/** Perform the uncertainty quantification */
void NonDMUQBayesCalibration::calibrate()
{
  int N =  (chainSamples > 0) ? chainSamples : 1000;
  size_t num_cv = numContinuousVars;
  const RealVector& init_point = ModelUtils::continuous_variables(nonDMUQInstance->mcmcModel);
  Eigen::VectorXd init_pt(num_cv);
  for (size_t i(0); i < num_cv; ++i)
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
  for (unsigned int i(0); i < samps->size(); ++i) {

    states = samps->at(i);

    // get mcmc state vector
    sample = states->state.at(0);

    for (int j(0); j < numContinuousVars; ++j)
      mcmc_rv(j) = sample(j);

    auto it = states->meta.find("LogTarget");
    if ( it != states->meta.end() ) {
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
    ModelUtils::continuous_variable_labels(iteratedModel);
  // the residualModel includes any hyper-parameters
  StringArray combined_labels;
  copy_data(ModelUtils::continuous_variable_labels(residualModel), combined_labels);

  size_t wpp7 = write_precision+7;

  // print MAP for continuous random variables
  if (standardizedSpace) {
    RealVector u_rv(Teuchos::View, c_vars.values(), numContinuousVars);
    RealVector x_rv;
    mcmcModel.trans_U_to_X(u_rv, x_rv);
    write_data(Cout, x_rv, cv_labels);
  }
  else
    for (size_t j(0); j < numContinuousVars; ++j)
      s << "                     " << std::setw(wpp7) << c_vars[j]
  << ' ' << cv_labels[j] << '\n';
  // print MAP for hyper-parameters (e.g., observation error params)
  for (size_t j(0); j < numHyperparams; ++j)
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
  for (int i(0); i < num_mcmc; ++i) {

    // translate the output matrix into x- or u-space lookup vars and
    // x-space acceptanceChain
    if (standardizedSpace) {
      // u_rv and x_rv omit any hyper-parameters
      RealVector u_rv(numContinuousVars, false);
      for (int j(0); j < numContinuousVars; ++j)
        u_rv(j) = chain(j,i);

      Real* acc_chain_i = acceptanceChain[i];
      RealVector x_rv(Teuchos::View, acc_chain_i, numContinuousVars);
      mcmcModel.trans_U_to_X(u_rv, x_rv);
      for (int j(numContinuousVars); j < num_params; ++j) {

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
      for (int j(0); j < numContinuousVars; ++j) {
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
      ModelUtils::active_variables(nonDMUQInstance->mcmcModel, lookup_vars);
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

void NonDMUQBayesCalibration::init_proposal_covariance()
{
  // Size our MUQ covariance matrix and initialize trailing diagonal if
  // calibrating error hyperparams
  proposalCovMatrix =
    Eigen::MatrixXd::Zero(numContinuousVars+numHyperparams,numContinuousVars+numHyperparams);
  if (numHyperparams > 0) {
    // all hyperparams utilize inverse gamma priors, which may not
    // have finite variance; use std_dev = 0.05 * mode
    Real alpha;
    for (int i(0); i < numHyperparams; ++i) {
      invGammaDists[i].pull_parameter(Pecos::IGA_ALPHA, alpha);
      proposalCovMatrix(numContinuousVars + i, numContinuousVars + i) = 
  (alpha > 2.) ? invGammaDists[i].variance() :
  std::pow(0.05*init_point[numContinuousVars + i], 2.);
    }
  }

  // initialize proposal covariance (must follow parameter domain init)
  // This is the leading sub-matrix in the case of calibrating sigma terms
  if (proposalCovarType == "user") // either filename OR data values defined
    user_proposal_covariance(proposalCovarInputType, proposalCovarData,
           proposalCovarFilename);
  else if (proposalCovarType == "prior")
    prior_proposal_covariance(); // prior selection or default for no emulator
  else {
    Cerr << "\nError: MUQ init_proposal_covariance(): proposal covariance type, "
      << proposalCovarType << ", not supported" << '\n';
    abort_handler(METHOD_ERROR);
  }

  // validate that provided data is a valid covariance matrix
  validate_proposal();
}

/** Must be called after paramMins/paramMaxs set above */
void NonDMUQBayesCalibration::prior_proposal_covariance()
{
  // diagonal covariance from variance of prior marginals
  RealVector dist_var = mcmcModel.multivariate_distribution().variances();
  // SVD index conversion is more general, but not required for current uses
  //const SharedVariablesData& svd= mcmcModel.current_variables().shared_data();
  for (int i(0); i < numContinuousVars; ++i)
    proposalCovMatrix(i,i) = priorPropCovMult * dist_var[i];

  if (outputLevel > NORMAL_OUTPUT) {
    //Cout << "Diagonal elements of the proposal covariance sent to MUQ";
    //if (standardizedSpace) Cout << " (scaled space)";
    //Cout << '\n' << covDiag << '\n';
    Cout << "MUQ ProposalCovMatrix"; 
    if (standardizedSpace) Cout << " (scaled space)";
    Cout << '\n'; 
    for (size_t i(0); i < numContinuousVars; ++i) {
      for (size_t j(0); j < numContinuousVars; ++j) 
  Cout <<  proposalCovMatrix(i,j) << "  "; 
      Cout << '\n'; 
    }
  }
}


/** This function will convert user-specified cov_type = "diagonal" |
    "matrix" data from either cov_data or cov_filename and populate a
    full Eigen::MatrixXd in proposalCovMatrix with the covariance. */
void NonDMUQBayesCalibration::
user_proposal_covariance(const String& input_fmt, const RealVector& cov_data, 
       const String& cov_filename)
{
  // TODO: transform user covariance for use in standardized probability space
  if (standardizedSpace)
    throw std::runtime_error("user-defined proposal covariance is invalid for use in transformed probability spaces.");
  // Note: if instead a warning, then fallback to prior_proposal_covariance()

  bool use_file = !cov_filename.empty();

  // Sanity check
  if ( ("diagonal" != input_fmt) &&
       ("matrix"   != input_fmt) )
    throw std::runtime_error("User-specified covariance must have type of either \"diagonal\" of \"matrix\".  You have \""+input_fmt+"\".");

  // Sanity check
  if ( cov_data.length() && use_file )
    throw std::runtime_error("You cannot provide both covariance values and a covariance data filename.");
     
  // Read in a general way and then check that the data is consistent
  RealVectorArray values_from_file;
  if ( use_file ) {
    std::ifstream s;
    TabularIO::open_file(s, cov_filename, "read_muq_covariance_data");
    bool row_major = false;
    read_unsized_data(s, values_from_file, row_major);
  }

  if ( "diagonal" == input_fmt ) {
    if ( use_file ) {
      // Allow either row or column data layout
      bool row_data = false;
      // Sanity checks
      if ( values_from_file.size() != 1 ) {
        if ( values_from_file.size() == numContinuousVars ) 
          row_data = true;
        else
          throw std::runtime_error("\"diagonal\" MUQ covariance file data should have either 1 column (or row) and "
              +convert_to_string(numContinuousVars)+" rows (or columns).");
      }
      if ( row_data ) {
        for ( int i(0); i < numContinuousVars; ++i )
          proposalCovMatrix(i,i) = values_from_file[i](0);
      }
      else {
        if ( values_from_file[0].length() != numContinuousVars )
          throw std::runtime_error("\"diagonal\" MUQ covariance file data should have "
              +convert_to_string(numContinuousVars)+" rows.  Found "
              +convert_to_string(values_from_file[0].length())+" rows.");
        for (int i(0); i < numContinuousVars; ++i) {
          proposalCovMatrix(i,i) = values_from_file[0](i);
        }
      }
    }
    else {
      // Sanity check
      if ( numContinuousVars != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(numContinuousVars)
                                 +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      for (int i(0); i < numContinuousVars; ++i) {
        proposalCovMatrix(i,i) = cov_data(i);
      }
    }
  }
  else { // "matrix" == input_fmt
    if ( use_file ) {
      // Sanity checks
      if ( values_from_file.size() != numContinuousVars ) 
        throw std::runtime_error("\"matrix\" MUQ covariance file data should have "
                                 +convert_to_string(numContinuousVars)+" columns.  Found "
                                 +convert_to_string(values_from_file.size())+" columns.");
      if ( values_from_file[0].length() != numContinuousVars )
        throw std::runtime_error("\"matrix\" MUQ covariance file data should have "
                                 +convert_to_string(numContinuousVars)+" rows.  Found "
                                 +convert_to_string(values_from_file[0].length())+" rows.");
      for (int i(0); i < numContinuousVars; ++i)
        for (int j(0); j < numContinuousVars; ++j)
          proposalCovMatrix(i,j) = values_from_file[i](j);
    }
    else {
      // Sanity check
      if ( numContinuousVars*numContinuousVars != cov_data.length() )
        throw std::runtime_error("Expected num covariance values is "+convert_to_string(numContinuousVars*numContinuousVars)
            +" but incoming vector provides "+convert_to_string(cov_data.length())+".");
      int count = 0;
      for (int i(0); i < numContinuousVars; ++i)
        for (int j(0); j < numContinuousVars; ++j)
          proposalCovMatrix(i,j) = cov_data[count++];
    }
  }
}

void NonDMUQBayesCalibration::validate_proposal()
{
  // validate that provided data is a valid covariance matrix
  
  if (outputLevel > NORMAL_OUTPUT) { 
    Cout << "Proposal Covariance " << '\n';
    Cout << proposalCovMatrix << std::endl;
  }

  // test symmetry
  Eigen::MatrixXd test_mat = proposalCovMatrix.transpose();
  if (!proposalCovMatrix.isApprox(test_mat))
    throw std::runtime_error("MUQ covariance matrix is not symmetric.");

  // test PD part of SPD
  Eigen::LLT<Eigen::MatrixXd> lltOfA(proposalCovMatrix); // compute the Cholesky decomposition of A
  if (lltOfA.info() == Eigen::NumericalIssue)
    throw std::runtime_error("MUQ covariance data is not SPD.");
}

} // namespace Dakota
