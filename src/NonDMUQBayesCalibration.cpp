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

  // instantiate QUESO objects and execute
  nonDMUQInstance = this;

  // prior,likelihood could be objects of same class w/ different fns passed in
  thetaPtr      = std::make_shared<muq::Modeling::IdentityOperator>(numContinuousVars);
  posteriorPtr  = std::make_shared<muq::Modeling::DensityProduct>(2);
  workGraph = std::make_shared<muq::Modeling::WorkGraph>();

  // // hard-coded Gaussian prior, to be changed
  // Eigen::VectorXd mu(1);
  // mu(0) = 0.0;
  // Eigen::MatrixXd cov(1,1);
  // cov(0,0) = 1.0;
  // priorPtr      = std::make_shared<muq::Modeling::Gaussian>(mu, cov)->AsDensity();

  dakotaLogLikelihoodPtr = std::make_shared<DakotaLogLikelihood>(model, nonDMUQInstance);
  dakotaLogPriorPtr = std::make_shared<DakotaLogPrior>(problem_db, model);

  //////////////////////
  // DEFINE THE GRAPH //
  //////////////////////
  workGraph->AddNode(thetaPtr,      "Parameters");
  workGraph->AddNode(dakotaLogLikelihoodPtr->AsDensity(), "Likelihood");
  workGraph->AddNode(dakotaLogPriorPtr->AsDensity(),      "Prior");
  // workGraph->AddNode(priorPtr,      "Prior");
  workGraph->AddNode(posteriorPtr,  "Posterior");

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
}


NonDMUQBayesCalibration::~NonDMUQBayesCalibration()
{ }


double DakotaLogLikelihood::LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {

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


double DakotaLogPrior::LogDensityImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs) {
  
  // Extract parameter vector from MUQ's inputs vector
  // (which lives in the ModPiece base class)
  Eigen::VectorXd const& c_vars = inputs.at(0);
  size_t i, num_cv = c_vars.size();

  // Set the calibration variables and hyperparams in the outer
  // residualModel: note that this won't update the Variables object
  // in any inner models.
  RealVector& all_params = residualModel.current_variables().continuous_variables_view();
  // Set parameter values in Dakota model object
  for (i=0; i<num_cv; ++i)
   all_params[i] = c_vars[i];

  double log_prior = log_prior_density(all_params);

  std::cout << "Sample: " << all_params[0] << ", log_prior: " << log_prior << std::endl;

  return log_prior;
}


/** Perform the uncertainty quantification */
void DakotaLogPrior::calibrate() { }


/** Perform the uncertainty quantification */
void NonDMUQBayesCalibration::calibrate()
{

  Eigen::VectorXd init_pt;
  init_pt.resize(1);
  init_pt[0]=0.45;

  samps = mcmc->Run(init_pt);

  Eigen::VectorXd mean = samps->Mean();

  std::cout << mean(0) << std::endl;

  const std::string filename = "output.h5";

  // write the collection to file
  samps->WriteToFile(filename);

}


void NonDMUQBayesCalibration::
print_results(std::ostream& s, short results_state)
{
//  NonDBayesCalibration::print_results(s, results_state);
//
//  // SampleCollection has been created (in core and in HDF5 file).
//  // Output useful posterior stats.
//  mcmcSampleSetPtr->WriteToFile();
//
//  momentStatistics() = mcmcSampleSetPtr->Mean();
//  momentStatistics() = mcmcSampleSetPtr->Variance(); // etc. (see docs)
//  StringArray resp_labels = mcmcModel.current_response().function_labels();
//  NonDSampling::print_moments(s, momentStatistics, RealMatrix(),
//    "response function", STANDARD_MOMENTS, resp_labels, false);
}


// void MUQModelInterface::
// EvaluateImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs)
// {
//   // Extract parameter vector from MUQ's inputs vector
//   // (which lives in the ModPiece base class)
//   Eigen::VectorXd const& c_vars = inputs.at(0);
//   size_t i, num_cv = c_vars.size();
//   // Set parameter values in Dakota model object
//   for (i=0; i<num_cv; ++i)
//    dakotaModel.continuous_variable(c_vars[i], i);

//   ActiveSet set(dakotaModel.response_size(), dakotaModel.cv());
//   if (useDerivs)
//     set.request_values(3); // gradient information processed in GradientImpl
//   else           
//     set.request_values(1); // function values processed next
//   dakotaModel.evaluate(set);

//   const RealVector& fn_vals = dakotaModel.current_response().function_values();
//   size_t num_fns = fn_vals.length();

//   // Populate MUQ's outputs vector
//   // (which lives in the ModPiece base class)
//   outputs.resize(1);
//   Eigen::VectorXd& qoi = outputs.at(0);
//   qoi.resize(num_fns);
//   for (i=0; i<num_fns; ++i)
//    qoi[i] = fn_vals[i]; // both operator(),operator[], one is bounds-checked
// }


//void MUQModelInterface::
//GradientImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs, ...)
//{
////  // Derivative evaluations: ModPiece has Gradient/Jacobian()
////  // Q: can I write to both outputs/sensitivity in a single Evaluate()?  No.
////  // In the short term, cache info on Dakota side and return in Gradient().
////  // Longer term, Andy may consider a different all-at-once Eval interface.
////
////  // Eigen::VectorXd const& c_vars = inputs.at(0);
////  // size_t i, num_cv = c_vars.size();
////  // for (i=0; i<num_cv; ++i)
////  //   dakotaModel.continuous_variable(c_vars[i], i);
////
////  // Assuming this has already been evaluated...
////  //dakotaModel.evaluate(); // default ASV *** TO DO: gradients for MALA et al.?
////
////  const RealMatrix& fn_grads
////    = dakotaModel.current_response().function_gradients();
////  Eigen::MatrixXd& sens = sensitivity.at(0);
////  for (i=0; i<num_fns; ++i)
////    for (j=0; j<num_v; ++j)
////      sens(i,j) = fn_grads(i,j);
//}


// *** Asynch evaluator?  Batch evaluator?  Not yet.  Parser is an MPI wrapper,
//     and it aggregates results from multiple chains

} // namespace Dakota
