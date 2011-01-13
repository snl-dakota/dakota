/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "system_defs.h"
#include "data_io.h"
#include "NonDLocalInterval.H"
#include "RecastModel.H"
#include "ProblemDescDB.H"
#ifdef DAKOTA_NPSOL
#include "NPSOLOptimizer.H"
#endif // DAKOTA_NPSOL
#ifdef DAKOTA_OPTPP
#include "SNLLOptimizer.H"
#endif // DAKOTA_NPSOL

//#define DEBUG

namespace Dakota {

// initialization of statics
NonDLocalInterval* NonDLocalInterval::nondLIInstance(NULL);


NonDLocalInterval::NonDLocalInterval(Model& model): NonDInterval(model)
{
  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level within the run fn.
  minMaxModel.assign_rep(new RecastModel(iteratedModel, 1, 0, 0), false);

#if !defined(DAKOTA_NPSOL) && !defined(DAKOTA_OPTPP)
  Cerr << "Error: this executable not configured with NPSOL or OPT++.\n"
       << "       NonDLocalInterval requires a gradient-based optimizer."
       << std::endl;
  abort_handler(-1);
#endif
  const String& opt_algorithm
    = probDescDB.get_string("method.nond.optimization_algorithm");
  if (opt_algorithm == "sqp") {
#ifdef DAKOTA_NPSOL
    npsolFlag = true;
#else
    Cerr << "\nError: this executable not configured with NPSOL SQP.\n"
	 << "         Please select OPT++ NIP within local_interval_est."
	 << std::endl;
    abort_handler(-1);
#endif
  }
  else if (opt_algorithm == "nip") {
#ifdef DAKOTA_OPTPP
    npsolFlag = false;
#else
    Cerr << "\nError: this executable not configured with OPT++ NIP.\n"
	 << "         please select NPSOL SQP within local_interval_est."
	 << std::endl;
    abort_handler(-1);
#endif
  }
  else if (opt_algorithm.empty()) {
#ifdef DAKOTA_NPSOL
    npsolFlag = true;
#elif DAKOTA_OPTPP
    npsolFlag = false;
#endif
  }

  // instantiate the optimizer used to compute the output interval bounds
  if (npsolFlag) {
#ifdef DAKOTA_NPSOL  
    int npsol_deriv_level = 3;
    minMaxOptimizer.assign_rep(new_NPSOLOptimizer2(minMaxModel,
				npsol_deriv_level, convergenceTol), false);
#endif // DAKOTA_NPSOL
  }
  else {
#ifdef DAKOTA_OPTPP
    minMaxOptimizer.assign_rep(new
      SNLLOptimizer("optpp_q_newton", minMaxModel), false);
#endif // DAKOTA_OPTPP
  }

  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for SOL presence.
  // Note 1: This check is performed for DOT, CONMIN, and SOLBase, but not
  //         for LHS since it is only active in pre-processing.
  // Note 2: NPSOL/NLSSOL on the outer loop with NonDLocalReliability on the
  //         inner loop precludes all NPSOL-based MPP searches;
  //         NonDLocalReliability on the outer loop with NPSOL/NLSSOL on an
  //         inner loop is only a problem for the no_approx MPP search (since
  //         iteratedModel is not invoked w/i an approx-based MPP search).
  if (npsolFlag) {
    Iterator sub_iterator = iteratedModel.subordinate_iterator();
    if (!sub_iterator.is_null() && 
	 ( sub_iterator.method_name().ends("sol_sqp") ||
	   sub_iterator.uses_method().ends("sol_sqp") ) )
      sub_iterator.method_recourse();
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	   ( sub_iterator.method_name().ends("sol_sqp") ||
	     sub_iterator.uses_method().ends("sol_sqp") ) )
	sub_iterator.method_recourse();
    }
  }

  minMaxModel.init_communicators(minMaxOptimizer.maximum_concurrency());
}


NonDLocalInterval::~NonDLocalInterval()
{  
  // deallocate communicators for minMaxOptimizer on minMaxModel
  minMaxModel.free_communicators(minMaxOptimizer.maximum_concurrency());
}


void NonDLocalInterval::quantify_uncertainty()
{
  // set the object instance pointer for use within static member functions
  NonDLocalInterval* prev_instance = nondLIInstance;
  nondLIInstance = this;

  // *** TO DO: requires mapping pointers for correct updating logic ***
  minMaxModel.update_from_subordinate_model();

  RealVector min_initial_pt, max_initial_pt;
  copy_data(minMaxModel.continuous_variables(), min_initial_pt); // view->copy
  max_initial_pt = min_initial_pt; // copy->copy

  Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
  primary_resp_map[0].resize(1);
  BoolDequeArray nonlinear_resp_map(1);
  nonlinear_resp_map[0] = BoolDeque(numFunctions, false);
  RecastModel* model_rep = (RecastModel*)minMaxModel.model_rep();

  initialize(); // virtual fn for initializing loop controls

  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {

    primary_resp_map[0][0] = respFnCntr;

    for (cellCntr=0; cellCntr<numCells; ++cellCntr) { 

      set_cell_bounds(); // virtual fn for setting bounds for local min/max

      // re-initialize recast model function pointers
      model_rep->initialize(vars_map, false, NULL, NULL, primary_resp_map,
			    secondary_resp_map, nonlinear_resp_map,
			    objective_min, NULL);
      // Execute local search and retrieve results
      Cout << "\n>>>>> Initiating local minimization\n";
      truncate_to_cell_bounds(min_initial_pt);
      minMaxModel.continuous_variables(min_initial_pt); // set starting pt
      // no summary output since on-the-fly constructed:
      minMaxOptimizer.run_iterator(Cout);
      if (numCells>1 && cellCntr<numCells-1)            // warm start next min
	copy_data(minMaxOptimizer.variables_results().continuous_variables(),
		  min_initial_pt);
      post_process_cell_results(true); // virtual fn: post-process min

      // re-initialize recast model function pointers
      model_rep->initialize(vars_map, false, NULL, NULL, primary_resp_map,
			    secondary_resp_map, nonlinear_resp_map,
			    objective_max, NULL);
      // Execute local search and retrieve results
      Cout << "\n>>>>> Initiating local maximization\n";
      truncate_to_cell_bounds(max_initial_pt);
      minMaxModel.continuous_variables(max_initial_pt); // set starting point
      // no summary output since on-the-fly constructed:
      minMaxOptimizer.run_iterator(Cout); 
      if (numCells>1 && cellCntr<numCells-1)            // warm start next max
	copy_data(minMaxOptimizer.variables_results().continuous_variables(),
		  max_initial_pt);
      post_process_cell_results(false); // virtual fn: post-process max
    }
    post_process_response_fn_results(); // virtual fn: post-process respFn
  }
  post_process_final_results(); // virtual fn: final post-processing

  // restore in case of recursion
  nondLIInstance = prev_instance;
}


void NonDLocalInterval::initialize()
{ } // default is no-op


void NonDLocalInterval::set_cell_bounds()
{ } // default is no-op


void NonDLocalInterval::truncate_to_cell_bounds(RealVector& initial_pt)
{ } // default is no-op


// default is partial output; invoked by derived class implementations
void NonDLocalInterval::post_process_cell_results(bool minimize)
{
  const Variables&    vars_star = minMaxOptimizer.variables_results();
  const RealVector& c_vars_star = vars_star.continuous_variables();
  Cout << "\nResults of local gradient-based optimization:\n"
       << "Final point             =\n";
  write_data(Cout, c_vars_star);

  const RealVector& fns_star_approx
    = minMaxOptimizer.response_results().function_values();
  Real fn_star = (minimize) ? fns_star_approx[0] : -fns_star_approx[0];
  Cout << "Final response          =\n                     "
       << std::setw(write_precision+7) << fn_star << "\n";
}


void NonDLocalInterval::post_process_response_fn_results()
{ } // default is no-op


void NonDLocalInterval::post_process_final_results()
{ } // default is no-op


void NonDLocalInterval::
objective_min(const Variables& sub_model_vars, const Variables& recast_vars,
	      const Response& sub_model_response, Response& recast_response)
{
  // pass through the sub_model_response data corresponding to respFnCntr
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1)
    recast_response.function_value(
      sub_model_response.function_value(nondLIInstance->respFnCntr), 0);
  if (recast_asv[0] & 2)
    recast_response.function_gradient(
      sub_model_response.function_gradient(nondLIInstance->respFnCntr), 0);
  if (recast_asv[0] & 4)
    recast_response.function_hessian(
      sub_model_response.function_hessian(nondLIInstance->respFnCntr), 0);
}


void NonDLocalInterval::
objective_max(const Variables& sub_model_vars, const Variables& recast_vars,
	      const Response& sub_model_response, Response& recast_response)
{
  // negate the sub_model_response data corresponding to respFnCntr
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1)
    recast_response.function_value(
      -sub_model_response.function_value(nondLIInstance->respFnCntr), 0);
  if (recast_asv[0] & 2) {
    RealVector sm_fn_grad
      = sub_model_response.function_gradient(nondLIInstance->respFnCntr);
    sm_fn_grad *= -1.;
    recast_response.function_gradient(sm_fn_grad, 0);
  }
  if (recast_asv[0] & 4) {
    RealSymMatrix sm_fn_hess
      = sub_model_response.function_hessian(nondLIInstance->respFnCntr);
    sm_fn_hess *= -1.;
    recast_response.function_hessian(sm_fn_hess, 0);
  }
}


void NonDLocalInterval::method_recourse()
{
  Cerr << "\nWarning: method recourse invoked in NonDLocalInterval due to "
       << "detected method conflict.\n\n";
  if (npsolFlag) {
    // if NPSOL already assigned, then reassign; otherwise just set the flag.
#ifdef DAKOTA_OPTPP
    minMaxModel.free_communicators(minMaxOptimizer.maximum_concurrency());
    minMaxOptimizer.assign_rep(
      new SNLLOptimizer("optpp_q_newton", minMaxModel), false);
    minMaxModel.init_communicators(minMaxOptimizer.maximum_concurrency());
#else
    Cerr << "\nError: method recourse not possible in NonDLocalInterval "
	 << "(OPT++ NIP unavailable).\n";
    abort_handler(-1);
#endif
    npsolFlag = false;
  }
}

} // namespace Dakota
