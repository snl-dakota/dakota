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
#include "ProblemDescDB.hpp"
#include "DakotaModel.hpp"
#include "ProbabilityTransformation.hpp"
#include "NonDSampling.hpp"

#include <boost/property_tree/ptree.hpp>

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMUQBayesCalibration::
NonDMUQBayesCalibration(ProblemDescDB& problem_db, Model& model):
  NonDBayesCalibration(problem_db, model),
  mcmcType(probDescDB.get_string("method.nond.mcmc_type"))
{
  // prior,likelihood could be objects of same class w/ different fns passed in
  thetaPtr      = std::make_shared<muq::Modeling::IdentityOperator>(numContinuousVars);
  priorPtr      = std::make_shared<MUQPriorInterface>();
  modelPtr      = std::make_shared<MUQModelInterface>(iteratedModel /* or model ? */);
  // Gaussian class inherited from Distribution could be helpful for likelihood
  likelihoodPtr = std::make_shared<MUQLikelihoodInterface>();
  posteriorPtr  = std::make_shared<muq::Modeling::DensityProduct>(2);

  //////////////////////
  // DEFINE THE GRAPH //
  //////////////////////
  workGraph.AddNode(thetaPtr,      "Parameters");
  workGraph.AddNode(likelihoodPtr, "Likelihood");
  workGraph.AddNode(priorPtr,      "Prior");
  workGraph.AddNode(posteriorPtr,       "Posterior");
  workGraph.AddNode(modelPtr,      "Forward Model"); // ***

  // Define graph: prior(theta) dependent on theta
  workGraph.AddEdge("Parameters", 0, "Prior", 0); // 0 = index of input,output

  // Define graph: simulation model G(theta) dependent on theta
  workGraph.AddEdge("Parameters", 0, "Forward Model", 0);

  // Define graph: likelihood dependent on simulation model G(theta);
  //               misfit = (G - G-bar)^T Gamma^{-1} (G - G-bar)
  //               (G-bar data dependence is currently hidden)
  workGraph.AddEdge("Forward Model", 0, "Likelihood", 0);
  // If "Approach 1", then likelihood has two inputs: G(theta) and data
  //workGraph.AddEdge("Data", 0, "Likelihood", 1); // if Approach 1

  // Define graph: posterior dependent on prior and likelihood;
  workGraph.AddEdge("Prior",      0, "Posterior", 0);
  workGraph.AddEdge("Likelihood", 0, "Posterior", 1);

  // More advanced graphs could be considered for, e.g.., Model selection.....

  ///////////////////////////////
  // DEFINE THE DESIRED OUTPUT //
  ///////////////////////////////
  // Register the desired end point / output from the graph: this registration
  // is distinct from creating a potentially larger graph.  It defines the
  // exact context that we care about at this time --> could define a
  // large graph-based ecosystem and then activate the relevant portion
  // for a particular study.
  //auto post_distrib = workGraph.CreateModPiece("Posterior");

  double hard_coded_mu  = 0.0;
  double hard_coded_cov = 0.0;
  auto post_distrib = std::make_shared<muq::Modeling::Gaussian>(hard_coded_mu, hard_coded_cov)->AsDensity(); // standard normal Gaussian

  // Dump out a visualization of the work graph
  if (outputLevel >= DEBUG_OUTPUT)
    workGraph.Visualize("muq_graph.pdf");

  /////////////////////////////////
  // DEFINE THE PROBLEM TO SOLVE //
  /////////////////////////////////
  samplingProbPtr = std::make_shared<muq::SamplingAlgorithms::SamplingProblem>(post_distrib);
  
  /////////////////////////////
  // DEFINE THE MCMC SAMPLER //
  /////////////////////////////
  // input specification is communicated using Boost property trees
  boost::property_tree::ptree pt; // TO DO: look at options...
  // *** Copied from MCMCTests.cpp example
  int N = 100;
  pt.put("NumSamples", N); // number of Monte Carlo samples
  pt.put("PrintLevel",0);
  pt.put("KernelList", "Kernel1"); // the transition kernel
  pt.put("Kernel1.Method","MHKernel"); // Metropolis-Hastings (have AM) (DR is in muq1, but maybe not yet available in MUQ2)
  pt.put("Kernel1.Proposal", "MyProposal"); // the proposal
  pt.put("Kernel1.MyProposal.Method", "MHProposal");
  pt.put("Kernel1.MyProposal.ProposalVariance", 0.5); // the variance of the isotropic MH proposal

  mcmcSamplerPtr = muq::SamplingAlgorithms::MCMCFactory::CreateSingleChain(pt, samplingProbPtr);
}


NonDMUQBayesCalibration::~NonDMUQBayesCalibration()
{ }


/** Perform the uncertainty quantification */
void NonDMUQBayesCalibration::calibrate()
{
  //mcmcSampleSetPtr = mcmcSamplerPtr->Run(init_pt);
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


void MUQModelInterface::
EvaluateImpl(muq::Modeling::ref_vector<Eigen::VectorXd> const& inputs)
{
//  //RealVector c_vars = dakotaModel.continuous_variables_view();
//  //copy_data(inputs.at(0), c_vars);
//
//  Eigen::VectorXd const& c_vars = inputs.at(0);
//  size_t i, num_cv = c_vars.size();
//  for (i=0; i<num_cv; ++i)
//    dakotaModel.continuous_variable(c_vars[i], i);
//
//  ActiveSet set(dakotaModel.response_size(), dakotaModel.cv());
//  if (useDerivs) set.request_values(3); // grads  processed in GradientImpl
//  else           set.request_values(1); // values processed below
//  dakotaModel.evaluate(set); // gradients for MALA et al.
//
//  const RealVector& fn_vals = dakotaModel.current_response().function_values();
//  outputs.resize(1);
//  Eigen::VectorXd& qoi = outputs.at(0);
//  qoi.resize(num_fns);
//  for (i=0; i<num_fns; ++i)
//    qoi[i] = fn_vals[i]; // both operator(),operator[], one is bounds-checked
}


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
