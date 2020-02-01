/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

#include <boost/property_tree/ptree.hpp>

namespace Dakota {


// initialization of statics, not being used yet
NonDMUQBayesCalibration* NonDMUQBayesCalibration::nonDMUQInstance(NULL);

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMUQBayesCalibration::
NonDMUQBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
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

  // return 0.0;
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

  std::cout << "Sample: " << all_params[0] << ", log_prior: " << log_prior << std::endl;

  return log_prior;

  // return 0.0;
}

/** Perform the uncertainty quantification */
void NonDMUQBayesCalibration::calibrate()
{
  size_t i, num_cv = numContinuousVars;

  // instantiate QUESO objects and execute
  nonDMUQInstance = this;

  // build the emulator and initialize transformations, as needed
  initialize_model();

  parameterPtr = std::make_shared<muq::Modeling::IdentityOperator>(numContinuousVars);
  posteriorPtr = std::make_shared<muq::Modeling::DensityProduct>(2);
  distPtr = std::make_shared<muq::Modeling::Distribution>(numContinuousVars);
  workGraph = std::make_shared<muq::Modeling::WorkGraph>();
  MUQLikelihoodPtr = std::make_shared<MUQLikelihood>(nonDMUQInstance, distPtr);
  MUQPriorPtr = std::make_shared<MUQPrior>(nonDMUQInstance, distPtr);

  //////////////////////
  // DEFINE THE GRAPH //
  //////////////////////
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
  int N = 1000;
  pt.put("NumSamples", N); // number of Monte Carlo samples
  pt.put("PrintLevel",0);
  pt.put("KernelList", "Kernel1"); // the transition kernel
  pt.put("Kernel1.Method","MHKernel"); // Metropolis-Hastings (have AM) (DR is in muq1, but maybe not yet available in MUQ2)
  pt.put("Kernel1.Proposal", "MyProposal"); // the proposal
  pt.put("Kernel1.MyProposal.Method", "MHProposal");
  pt.put("Kernel1.MyProposal.ProposalVariance", 0.00001); // the variance of the isotropic MH proposal

  auto dens = workGraph->CreateModPiece("Posterior");

  auto problem = std::make_shared<muq::SamplingAlgorithms::SamplingProblem>(dens);

  mcmc = std::make_shared<muq::SamplingAlgorithms::SingleChainMCMC>(pt,problem);

  const RealVector& init_point = nonDMUQInstance->mcmcModel.continuous_variables();
  Eigen::VectorXd init_pt(num_cv);
  for (i=0; i<num_cv; ++i)
    init_pt[i] = init_point[i];

  samps = mcmc->Run(init_pt);
}


void NonDMUQBayesCalibration::
print_results(std::ostream& s, short results_state)
{
  NonDBayesCalibration::print_results(s, results_state);

  // SampleCollection has been created (in core and in HDF5 file).
  // Output useful posterior stats.
  const std::string filename = "output.h5";
  samps->WriteToFile(filename);

  // momentStatistics() = samps->Mean();
  // momentStatistics() = samps->Variance(); // etc. (see docs)
  // StringArray resp_labels = mcmcModel.current_response().function_labels();
  // NonDSampling::print_moments(s, momentStatistics, RealMatrix(),
  //   "response function", STANDARD_MOMENTS, resp_labels, false);
}

} // namespace Dakota
