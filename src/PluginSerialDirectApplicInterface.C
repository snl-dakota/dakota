/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SIM::SerialDirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.H"
#include "ParamResponsePair.H"
#include "PluginSerialDirectApplicInterface.H"
#include "system_defs.h"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"


namespace SIM {

int SerialDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
{
#ifdef MPI_DEBUG
  Cout << "analysis server " << analysisServerId << " invoking " << ac_name
       << " within SIM::SerialDirectApplicInterface." << std::endl;
#endif // MPI_DEBUG

  if (multiProcAnalysisFlag) {
    Cerr << "Error: plugin serial direct fn does not support multiprocessor "
	 << "analyses." << std::endl;
    Dakota::abort_handler(-1);
  }

  int fail_code = 0;
  if (ac_name == "plugin_rosenbrock") {
    Dakota::RealVector grad0 = Teuchos::getCol(Teuchos::View, fnGrads, 0);
    rosenbrock(xC, directFnASV[0], fnVals[0], grad0, fnHessians[0]);
  }
  else {
    Cerr << ac_name << " is not available as an analysis within "
         << "SIM::SerialDirectApplicInterface." << std::endl;
    Dakota::abort_handler(-1);
  }

  // Failure capturing
  if (fail_code)
    throw fail_code;

  return 0;
}


void SerialDirectApplicInterface::
derived_map_asynch(const Dakota::ParamResponsePair& pair)
{
  // no-op (just hides base class error throw). Jobs are run exclusively within
  // derived_synch(), prior to there existing true batch processing facilities.
}


void SerialDirectApplicInterface::derived_synch(Dakota::PRPQueue& prp_queue)
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
    const Dakota::Variables& vars = prp_iter->prp_parameters();
    const Dakota::ActiveSet& set  = prp_iter->active_set();
    Dakota::Response         resp = prp_iter->prp_response(); // shared rep
    if (outputLevel > Dakota::SILENT_OUTPUT)
      Cout << "SerialDirectApplicInterface:: evaluating function evaluation "
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
    //       << "SIM::SerialDirectApplicInterface." << std::endl;
    //  Dakota::abort_handler(-1);
    //}

    // indicate completion of job to ApplicationInterface schedulers
    completionSet.insert(fn_eval_id);
  }
}


/** For use by ApplicationInterface::serve_evaluations_asynch(), which can
    provide a batch processing capability within message passing schedulers
    (called using chain Strategy::run_iterator() --> Model::serve() -->
    ApplicationInterface::serve_evaluations() -->
    ApplicationInterface::serve_evaluations_asynch()). */
void SerialDirectApplicInterface::
derived_synch_nowait(Dakota::PRPQueue& prp_queue)
{ derived_synch(prp_queue); }


// Hide default error checks at ApplicationInterface level
void SerialDirectApplicInterface::
check_configuration(int max_iterator_concurrency)
{ }


void SerialDirectApplicInterface::
rosenbrock(const Dakota::RealVector& c_vars, short asv, Dakota::Real& fn_val, 
	   Dakota::RealVector& fn_grad, Dakota::RealSymMatrix& fn_hess)
{
  if (c_vars.length() != 2) {
    Cerr << "Error: Bad number of variables in rosenbrock direct fn."
	 << std::endl;
    Dakota::abort_handler(-1);
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
}

} // namespace SIM
