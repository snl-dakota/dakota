/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SIM::ParallelDirectApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "PluginParallelDirectApplicInterface.hpp"
#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include <algorithm>


namespace SIM {

#define POW_VAL 1.0 // text_book: 1.0 is nominal, 1.4 used for B&B testing


ParallelDirectApplicInterface::
ParallelDirectApplicInterface(const Dakota::ProblemDescDB& problem_db,
			      const MPI_Comm& analysis_comm):
  Dakota::DirectApplicInterface(problem_db)//, analysisComm(analysis_comm)
{
  // it is not necessary to update analysis communicator size/rank, since
  // these are inherited for use at run time.  The only purpose for passing
  // analysis_comm is for use initializing a parallel app within the ctor.

#ifdef MPI_DEBUG
  // For testing purposes, output size/rank of the incoming analysis_comm
  int rank, size;
  MPI_Comm_rank(analysis_comm, &rank);
  MPI_Comm_size(analysis_comm, &size);
  Cout << "In SIM::ParallelDirectApplicInterface ctor, rank = " << rank
       << " size = " << size << std::endl;
#endif // MPI_DEBUG
}


int ParallelDirectApplicInterface::derived_map_ac(const Dakota::String& ac_name)
{
#ifdef MPI_DEBUG
    Cout << "analysis server " << analysisServerId << " invoking " << ac_name
         << " within SIM::ParallelDirectApplicInterface." << std::endl;
#endif // MPI_DEBUG

  int fail_code = 0;
  if (ac_name == "plugin_text_book")
    fail_code = text_book(xC, directFnASV, fnVals, fnGrads, fnHessians);
  else  {
    Cerr << ac_name << " is not available as an analysis within "
         << "SIM::ParallelDirectApplicInterface." << std::endl;
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


void ParallelDirectApplicInterface::
wait_local_evaluations(Dakota::PRPQueue& prp_queue)
{
  // For each job in the processing queue, evaluate the response
  for (Dakota::PRPQueueIter prp_iter = prp_queue.begin();
       prp_iter != prp_queue.end(); prp_iter++) {
    // set local variable/set data, but update resp directly for efficiency
    set_local_data(prp_iter->variables(), prp_iter->active_set());
    Dakota::Response           resp     = prp_iter->response();// shared rep
    Dakota::RealVector         fn_vals  = resp.function_values_view();
    Dakota::RealMatrix         fn_grads = resp.function_gradients_view();
    Dakota::RealSymMatrixArray fn_hessians = resp.function_hessians_view();
    int fn_eval_id = prp_iter->eval_id();
    if (outputLevel > Dakota::SILENT_OUTPUT)
      Cout << "ParallelDirectApplicInterface:: evaluating function evaluation "
	   << fn_eval_id << " in batch mode." << std::endl;
    //if (ac_name == "plugin_text_book") { // not provided in this API
      text_book(xC, directFnASV, fn_vals, fn_grads, fn_hessians);
    //}
    //else {
    //  Cerr << ac_name << " is not available as an analysis within "
    //       << "SIM::SerialDirectApplicInterface." << std::endl;
    //  Dakota::abort_handler(Dakota::INTERFACE_ERROR);
    //}

    // indicate completion of job to ApplicationInterface schedulers
    completionSet.insert(fn_eval_id);
  }
}


int ParallelDirectApplicInterface::
text_book(const Dakota::RealVector& c_vars, const Dakota::ShortArray& asv,
	  Dakota::RealVector& fn_vals, Dakota::RealMatrix& fn_grads,
	  Dakota::RealSymMatrixArray& fn_hessians)
{
  size_t i, num_fns = asv.size();
  if (num_fns > 3) {
    Cerr << "Error: Bad number of functions in plug-in parallel direct "
	 << "interface." << std::endl;
    Dakota::abort_handler(Dakota::INTERFACE_ERROR);
  }
  // The presence of discrete variables can cause offsets in directFnDVV which
  // the text_book derivative logic does not currently account for.
  if (numADIV || numADRV) {
    Cerr << "Error: plug-in parallel direct interface assumes no discrete "
	 << "variables." << std::endl;
    Dakota::abort_handler(Dakota::INTERFACE_ERROR);
  }

  // **********************************
  // **** f: sum (x[i] - POWVAL)^4 ****
  // **********************************
  if (asv[0] & 1) {
    double local_val = 0.0;
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize)
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      local_val += std::pow(c_vars[i]-POW_VAL, 4);
    if (multiProcAnalysisFlag) {
      double global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fn_vals[0] = global_val;
    }
    else
      fn_vals[0] = local_val;
  }

  // ****************
  // **** df/dx: ****
  // ****************
  if (asv[0] & 2) {
    //for (i=0; i<numDerivVars; i++)
    //  fn_grads[0][i] = 4.*std::pow(c_vars[i]-POW_VAL,3);
    //fn_grads[0] = 0.;
    std::fill(fn_grads[0], fn_grads[0] + numDerivVars, 0.);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1;
      fn_grads[0][i] = 4.*std::pow(c_vars[var_index]-POW_VAL,3);
    }
    if (multiProcAnalysisFlag) {
      double* sum_fns = (analysisCommRank) ? NULL : new double [numDerivVars];
      parallelLib.reduce_sum_a(fn_grads[0], sum_fns, 
			       numDerivVars);
      if (analysisCommRank == 0) {
	Teuchos::setCol(
	  Teuchos::SerialDenseVector<int,double>(Teuchos::View, sum_fns,
                                                 numDerivVars), 0, fn_grads );
	delete [] sum_fns;
      }
    }
  }

  // ********************
  // **** d^2f/dx^2: ****
  // ********************
  if (asv[0] & 4) {
    fn_hessians[0] = 0.;
    //for (i=0; i<numDerivVars; i++)
    //  fn_hessians[0][i][i] = 12.*std::pow(c_vars[i]-POW_VAL,2);
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      size_t var_index = directFnDVV[i] - 1;
      fn_hessians[0](i,i) = 12.*std::pow(c_vars[var_index]-POW_VAL,2);
    }
    if (multiProcAnalysisFlag) {
      int num_doubles = numDerivVars * numDerivVars;
      double* local_fns = new double [num_doubles];
      std::copy(fn_hessians[0].values(), fn_hessians[0].values() + num_doubles,
                local_fns);
      double* sum_fns = (analysisCommRank) ? NULL : new double [num_doubles];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_doubles);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy(sum_fns, sum_fns + num_doubles, fn_hessians[0].values());
	delete [] sum_fns;
      }
    }
  }

  // **********************************
  // **** c1: x[0]*x[0] - 0.5*x[1] ****
  // **********************************
  if (num_fns > 1 && (asv[1] & 1)) {
    double local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val += c_vars[i]*c_vars[i];
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val -= 0.5*c_vars[i];
    }
    if (multiProcAnalysisFlag) {
      double global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fn_vals[1] = global_val;
    }
    else
      fn_vals[1] = local_val;
  }

  // *****************
  // **** dc1/dx: ****
  // *****************
  if (num_fns > 1 && (asv[1] & 2)) {
    //fn_grads[1] = 0.;
    std::fill(fn_grads[1], fn_grads[1] + numDerivVars, 0.);
    //fn_grads[1][0] = 2.*c_vars[0];
    //fn_grads[1][1] = -0.5;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fn_grads[1][i] = 2.*c_vars[0];
      else if (var_index == 1)
        fn_grads[1][i] = -0.5;
    }
    if (multiProcAnalysisFlag) {
      double* sum_fns = (analysisCommRank) ? NULL : new double [numDerivVars];
      parallelLib.reduce_sum_a(fn_grads[1], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	Teuchos::setCol(
	  Teuchos::SerialDenseVector<int,double>(Teuchos::View, sum_fns,
                                                 numDerivVars), 1, fn_grads );
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c1/dx^2: ****
  // *********************
  if (num_fns > 1 && (asv[1] & 4)) {
    fn_hessians[1] = 0.;
    //fn_hessians[1][0][0] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
	fn_hessians[1](i,i) = 2.;
    }
    if (multiProcAnalysisFlag) {
      int num_doubles = numDerivVars * numDerivVars;
      double* local_fns = new double [num_doubles];
      std::copy(fn_hessians[1].values(), fn_hessians[1].values() + num_doubles,
                local_fns);
      double* sum_fns = (analysisCommRank) ? NULL : new double [num_doubles];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_doubles);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy(sum_fns, sum_fns + num_doubles, fn_hessians[1].values());
	delete [] sum_fns;
      }
    }
  }

  // **********************************
  // **** c2: x[1]*x[1] - 0.5*x[0] ****
  // **********************************
  if (num_fns > 2 && (asv[2] & 1)) {
    double local_val = 0.0;
    // Definitely not the most efficient way to do this, but the point is to
    // demonstrate Comm communication.
    for (i=analysisCommRank; i<numVars; i+=analysisCommSize) {
      // orders all continuous vars followed by all discrete vars.  This is 
      // fine in the direct case so long as everything is self-consistent.
      if (i==0) // could be changed to i % 2 == 0 to get even vars.
        local_val -= 0.5*c_vars[i];
      else if (i==1) // could be changed to i % 2 == 1 to get odd vars
        local_val += c_vars[i]*c_vars[i];
    }
    if (multiProcAnalysisFlag) {
      double global_val = 0.0;
      parallelLib.reduce_sum_a(&local_val, &global_val, 1);
      // only analysisCommRank 0 has the correct sum.  This is OK (MPI_Allreduce
      // not needed) since only analysisCommRank 0 updates response for 
      // evalCommRank 0 in overlay_response.  evalCommRank 0 then returns the 
      // results to the iterator in ApplicationInterface::serve_evaluations().
      if (analysisCommRank == 0)
	fn_vals[2] = global_val;
    }
    else
      fn_vals[2] = local_val;
  }

  // *****************
  // **** dc2/dx: ****
  // *****************
  if (num_fns > 2 && (asv[2] & 2)) {
    //fn_grads[2] = 0.;
    std::fill(fn_grads[2], fn_grads[2] + numDerivVars, 0.);
    //fn_grads[2][0] = -0.5;
    //fn_grads[2][1] = 2.*c_vars[1];
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 0)
        fn_grads[2][i] = -0.5;
      else if (var_index == 1)
        fn_grads[2][i] = 2.*c_vars[1];
    }
    if (multiProcAnalysisFlag) {
      double* sum_fns = (analysisCommRank) ? NULL : new double [numDerivVars];
      parallelLib.reduce_sum_a(fn_grads[2], sum_fns,
			       numDerivVars);
      if (analysisCommRank == 0) {
	Teuchos::setCol(
	  Teuchos::SerialDenseVector<int,double>(Teuchos::View, sum_fns,
                                                 numDerivVars), 2, fn_grads );
	delete [] sum_fns;
      }
    }
  }

  // *********************
  // **** d^2c2/dx^2: ****
  // *********************
  if (num_fns > 2 && (asv[2] & 4)) {
    fn_hessians[2] = 0.;
    //fn_hessians[2][1][1] = 2.;
    for (i=analysisCommRank; i<numDerivVars; i+=analysisCommSize) {
      int var_index = directFnDVV[i] - 1; // assumes no discrete vars
      if (var_index == 1)
	fn_hessians[2](i,i) = 2.;
    }
    if (multiProcAnalysisFlag) {
      int num_doubles = numDerivVars * numDerivVars;
      double* local_fns = new double [num_doubles];
      std::copy(fn_hessians[2].values(), fn_hessians[2].values() + num_doubles,
                local_fns);
      double* sum_fns = (analysisCommRank) ? NULL : new double [num_doubles];
      parallelLib.reduce_sum_a(local_fns, sum_fns, num_doubles);
      delete [] local_fns;
      if (analysisCommRank == 0) {
        std::copy(sum_fns, sum_fns + num_doubles, fn_hessians[2].values());
	delete [] sum_fns;
      }
    }
  }

  return 0;
}

} // namespace SIM
