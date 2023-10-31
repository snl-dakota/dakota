/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SoleilDirectApplicInterface.hpp"
#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"


namespace StanfordPSAAP {

/** Redefine this for serial/blocking execution of single Soleil simulations. */
int SoleilDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
{
#ifdef MPI_DEBUG
  Cout << "analysis server " << analysisServerId << " invoking " << ac_name
       << " within StanfordPSAAP::SoleilDirectApplicInterface." << std::endl;
#endif // MPI_DEBUG

  if (multiProcAnalysisFlag) {
    Cerr << "Error: plugin serial direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    Dakota::abort_handler(-1);
  }

  int fail_code = 0;
  if (ac_name == "plugin_rosenbrock") {
    Dakota::RealVector fn_grad; 
    Dakota::RealSymMatrix fn_hess;
    if (directFnASV[0] & 2)
      fn_grad = Teuchos::getCol(Teuchos::View, fnGrads, 0);
    if (directFnASV[0] & 4)
      fn_hess = Dakota::RealSymMatrix(Teuchos::View, fnHessians[0],
				      fnHessians[0].numRows());
    fail_code = rosenbrock(xC, directFnASV[0], fnVals[0], fn_grad, fn_hess);
  }
  else {
    Cerr << ac_name << " is not available as an analysis within "
         << "StanfordPSAAP::SoleilDirectApplicInterface." << std::endl;
    Dakota::abort_handler(Dakota::INTERFACE_ERROR);
  }

  // Failure capturing
  if (fail_code) {
    std::string err_msg("Error evaluating plugin analysis_driver ");
    err_msg += ac_name;
    throw Dakota::FunctionEvalFailure(err_msg);
  }

  return 0;
}


/** Redefine this for (Legion-based) execution of a batch of Soleil 
    simulations.  The incoming prp_queue is defined from 
    ApplicationInterface::asynchLocalActivePRPQueue which is a local 
    subset of beforeSynchCorePRPQueue.  This function must complete at 
    least one job (whereas test_local_evaluations() may complete zero).  
    Populating completionSet results in decrementing the active queue 
    and backfilling as indicated by concurrency level.  For Soleil, we 
    should not limit the concurrency level and will not combine with MPI 
    scheduling --> incoming prp_queue is the full beforeSynchCorePRPQueue 
    (no MPI distribution + no throttling).  Further, we should complete
    the full local queue or we may need to distinguish still-running 
    jobs from incoming new ones. */
void SoleilDirectApplicInterface::
wait_local_evaluations(Dakota::PRPQueue& prp_queue)
{
  if (multiProcAnalysisFlag) {
    Cerr << "Error: plugin serial direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    Dakota::abort_handler(-1);
  }

  for (Dakota::PRPQueueIter prp_iter = prp_queue.begin();
       prp_iter != prp_queue.end(); prp_iter++) {
    // For each job in the processing queue, evaluate the response
    int fn_eval_id = prp_iter->eval_id();
    const Dakota::Variables& vars = prp_iter->variables();
    const Dakota::ActiveSet& set  = prp_iter->active_set();
    Dakota::Response         resp = prp_iter->response(); // shared rep
    if (outputLevel > Dakota::SILENT_OUTPUT)
      Cout << "SoleilDirectApplicInterface:: evaluating function evaluation "
	   << fn_eval_id << " in batch mode." << std::endl;
    Dakota::RealVector fn_grad; Dakota::RealSymMatrix fn_hess;
    //if (ac_name == "plugin_rosenbrock") { // not provided in this API
      short asv = set.request_vector()[0];
      Dakota::Real& fn_val = resp.function_value_view(0);
      if (asv & 2) fn_grad = resp.function_gradient_view(0);
      if (asv & 4) fn_hess = resp.function_hessian_view(0);
      rosenbrock(vars.continuous_variables(), asv, fn_val, fn_grad, fn_hess);
    //}
    //else {
    //  Cerr << ac_name << " is not available as an analysis within "
    //       << "StanfordPSAAP::SoleilDirectApplicInterface." << std::endl;
    //  Dakota::abort_handler(Dakota::INTERFACE_ERROR);
    //}

    // indicate completion of job to ApplicationInterface schedulers
    completionSet.insert(fn_eval_id);
  }
}


int SoleilDirectApplicInterface::
rosenbrock(const Dakota::RealVector& c_vars, short asv, Dakota::Real& fn_val, 
	   Dakota::RealVector& fn_grad, Dakota::RealSymMatrix& fn_hess)
{
  if (c_vars.length() != 2) {
    Cerr << "Error: Bad number of variables in rosenbrock direct fn."
	 << std::endl;
    Dakota::abort_handler(Dakota::INTERFACE_ERROR);
  }

  Dakota::Real x1 = c_vars[0], x2 = c_vars[1], f0 = x2 - x1*x1, f1 = 1. - x1;

  // **** f:
  if (asv & 1)
    fn_val = 100.*f0*f0 + f1*f1;

  // **** df/dx:
  if (asv & 2) {
    fn_grad[0] = -400.*f0*x1 - 2.*f1;
    fn_grad[1] =  200.*f0;
  }
    
  // **** d^2f/dx^2:
  if (asv & 4) {
    Dakota::Real fx = x2 - 3.*x1*x1;
    fn_hess(0,0) = -400.*fx + 2.;
    fn_hess(0,1) = fnHessians[0](1,0) = -400.*x1;
    fn_hess(1,1) =  200.;
  }

  return 0;
}

} // namespace StanfordPSAAP
