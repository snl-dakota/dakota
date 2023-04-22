/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalInterval
//- Description: Class for interval bound estimation for epistemic UQ
//- Owner:       Laura Swiler
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "NonDLocalInterval.hpp"
#include "RecastModel.hpp"
#include "ProblemDescDB.hpp"
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif // HAVE_NPSOL
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif // HAVE_OPTPP
#include "ParallelLibrary.hpp"

//#define DEBUG

namespace Dakota {

// initialization of statics
NonDLocalInterval* NonDLocalInterval::nondLIInstance(NULL);


NonDLocalInterval::NonDLocalInterval(ProblemDescDB& problem_db, Model& model):
  NonDInterval(problem_db, model)
{
  bool err_flag = false;

  // Check for suitable active var types (discrete epistemic not supported)
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "\nError: discrete variables are not currently supported in "
	 << "NonDLocalInterval." << std::endl;
    err_flag = true;
  }
  if (numContinuousVars != numContIntervalVars) {
    Cerr << "\nError: only continuous interval distributions are currently "
	 << "supported in NonDLocalInterval." << std::endl;
    err_flag = true;
  }

  // Configure a RecastModel with one objective and no constraints using the
  // alternate minimalist constructor: the recast fn pointers are reset for
  // each level within the run fn.
  SizetArray recast_vars_comps_total;  // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  short recast_resp_order = 3; // gradient-based quasi-Newton optimizers
  const ShortShortPair& recast_view = iteratedModel.current_variables().view();
  minMaxModel.assign_rep(std::make_shared<RecastModel>
			 (iteratedModel, recast_vars_comps_total,
			  all_relax_di, all_relax_dr, recast_view, 1, 0, 0,
			  recast_resp_order));

  // instantiate the optimizer used to compute the output interval bounds
  switch (sub_optimizer_select(
	  probDescDB.get_ushort("method.nond.opt_subproblem_solver"))) {
  case SUBMETHOD_NPSOL: {
#ifdef HAVE_NPSOL
    int deriv_level = 3;
    minMaxOptimizer.assign_rep(std::make_shared<NPSOLOptimizer>
			       (minMaxModel, deriv_level, convergenceTol));
#endif // HAVE_NPSOL
    npsolFlag =  true; break;
  }
  case SUBMETHOD_OPTPP:
#ifdef HAVE_OPTPP
    minMaxOptimizer.assign_rep(std::make_shared<SNLLOptimizer>
			       ("optpp_q_newton", minMaxModel));
#endif // HAVE_OPTPP
    npsolFlag = false; break;
  default:
    npsolFlag = false; err_flag = true; break;
  }

  if (err_flag)
    abort_handler(METHOD_ERROR);
}


void NonDLocalInterval::check_sub_iterator_conflict()
{
  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator (which would result in data clashes since
  // Fortran does not support object independence).  Recurse through all
  // sub-models and test each sub-iterator for SOL presence.
  // Note 1: NPSOL/NLSSOL share code modules, so we check for both.
  // Note 2: forces lower-level to accommodate, even though this level may be
  //         the more flexible one in its ability to switch away from NPSOL.
  if (npsolFlag) {
    Iterator sub_iterator = iteratedModel.subordinate_iterator();
    if (!sub_iterator.is_null() && 
	 ( sub_iterator.method_name() ==  NPSOL_SQP ||
	   sub_iterator.method_name() == NLSSOL_SQP ||
	   sub_iterator.uses_method() == SUBMETHOD_NPSOL ||
	   sub_iterator.uses_method() == SUBMETHOD_NPSOL_OPTPP ) )
      sub_iterator.method_recourse();
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	   ( sub_iterator.method_name() ==  NPSOL_SQP ||
	     sub_iterator.method_name() == NLSSOL_SQP ||
	     sub_iterator.uses_method() == SUBMETHOD_NPSOL ||
	     sub_iterator.uses_method() == SUBMETHOD_NPSOL_OPTPP ) )
	sub_iterator.method_recourse();
    }
  }
}


NonDLocalInterval::~NonDLocalInterval()
{ }


void NonDLocalInterval::derived_init_communicators(ParLevLIter pl_iter)
{
  iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // miPLIndex needed in method_recourse() prior to assignment in
  // NonD::derived_set_communicators().  While derived_init_communicators()
  // may be invoked multiple times, this captures a consistent state to that
  // present during the invocation of check_sub_iterator_conflict().
  if (npsolFlag) miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);
  // minMaxOptimizer uses NoDBBaseConstructor, so no need to manage DB
  // list nodes at this level
  minMaxOptimizer.init_communicators(pl_iter);
}


void NonDLocalInterval::derived_set_communicators(ParLevLIter pl_iter)
{
  NonD::derived_set_communicators(pl_iter);

  // minMaxOptimizer uses NoDBBaseConstructor, so no need to manage DB
  // list nodes at this level
  minMaxOptimizer.set_communicators(pl_iter);
}


void NonDLocalInterval::derived_free_communicators(ParLevLIter pl_iter)
{
  minMaxOptimizer.free_communicators(pl_iter);

  iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}


void NonDLocalInterval::core_run()
{
  // set the object instance pointer for use within static member functions
  NonDLocalInterval* prev_instance = nondLIInstance;
  nondLIInstance = this;

  // *** TO DO: requires mapping pointers for correct updating logic ***
  //
  // now that vars/labels/bounds/targets have flowed down at run-time from
  // any higher level recursions, propagate them up local Model recursions
  // so that they are correct when they propagate back down.  There is no
  // need to recur below iteratedModel.
  size_t layers = 1;
  minMaxModel.update_from_subordinate_model(layers-1);

  RealVector min_initial_pt, max_initial_pt;
  copy_data(minMaxModel.continuous_variables(), min_initial_pt); // view->copy
  max_initial_pt = min_initial_pt; // copy->copy

  Sizet2DArray vars_map, primary_resp_map(1), secondary_resp_map;
  primary_resp_map[0].resize(1);
  BoolDequeArray nonlinear_resp_map(1);
  nonlinear_resp_map[0] = BoolDeque(numFunctions, false);
  BoolDeque max_sense(1);
  std::shared_ptr<RecastModel> model_rep =
    std::static_pointer_cast<RecastModel>(minMaxModel.model_rep());

  initialize(); // virtual fn for initializing loop controls

  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);

  for (respFnCntr=0; respFnCntr<numFunctions; ++respFnCntr) {

    primary_resp_map[0][0] = respFnCntr;
    model_rep->init_maps(vars_map, false, NULL, NULL, primary_resp_map,
			  secondary_resp_map, nonlinear_resp_map,
			  extract_objective, NULL);

    for (cellCntr=0; cellCntr<numCells; ++cellCntr) { 

      set_cell_bounds(); // virtual fn for setting bounds for local min/max

      // set minimization sense
      max_sense[0] = false;
      model_rep->primary_response_fn_sense(max_sense);
      // Execute local search and retrieve results
      Cout << "\n>>>>> Initiating local minimization\n";
      truncate_to_cell_bounds(min_initial_pt);
      minMaxModel.continuous_variables(min_initial_pt); // set starting pt
      minMaxOptimizer.run(pl_iter);
      if (numCells>1 && cellCntr<numCells-1)            // warm start next min
	copy_data(minMaxOptimizer.variables_results().continuous_variables(),
		  min_initial_pt);
      post_process_cell_results(false); // virtual fn: post-process min

      // set maximization sense
      max_sense[0] = true;
      model_rep->primary_response_fn_sense(max_sense);
      // Execute local search and retrieve results
      Cout << "\n>>>>> Initiating local maximization\n";
      truncate_to_cell_bounds(max_initial_pt);
      minMaxModel.continuous_variables(max_initial_pt); // set starting point
      minMaxOptimizer.run(pl_iter); 
      if (numCells>1 && cellCntr<numCells-1)            // warm start next max
	copy_data(minMaxOptimizer.variables_results().continuous_variables(),
		  max_initial_pt);
      post_process_cell_results(true); // virtual fn: post-process max
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
void NonDLocalInterval::post_process_cell_results(bool maximize)
{
  const Variables&    vars_star = minMaxOptimizer.variables_results();
  const RealVector& c_vars_star = vars_star.continuous_variables();
  Cout << "\nResults of local gradient-based optimization:\n"
       << "Final point             =\n" << c_vars_star;

  const RealVector& fns_star_approx
    = minMaxOptimizer.response_results().function_values();
  Cout << "Final response          =\n                     "
       << std::setw(write_precision+7) << fns_star_approx[0] << "\n";
}


void NonDLocalInterval::post_process_response_fn_results()
{ } // default is no-op


void NonDLocalInterval::post_process_final_results()
{ } // default is no-op


void NonDLocalInterval::
extract_objective(const Variables& sub_model_vars, const Variables& recast_vars,
		  const Response& sub_model_response, Response& recast_response)
{
  // pass through the sub_model_response data corresponding to respFnCntr
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (recast_asv[0] & 1)
    recast_response.function_value(
      sub_model_response.function_value(nondLIInstance->respFnCntr), 0);
  if (recast_asv[0] & 2)
    recast_response.function_gradient(
      sub_model_response.function_gradient_view(nondLIInstance->respFnCntr), 0);
  if (recast_asv[0] & 4)
    recast_response.function_hessian(
      sub_model_response.function_hessian(nondLIInstance->respFnCntr), 0);
}


void NonDLocalInterval::method_recourse()
{
  // see notes in NonDLocalReliability::method_recourse()

  Cerr << "\nWarning: method recourse invoked in NonDLocalInterval due to "
       << "detected method conflict.\n\n";
  if (npsolFlag) {
#ifdef HAVE_OPTPP
    ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    std::map<size_t, ParConfigLIter> pc_iter_map
      = minMaxOptimizer.parallel_configuration_iterator_map();
    minMaxOptimizer.assign_rep(std::make_shared<SNLLOptimizer>
			       ("optpp_q_newton", minMaxModel));
    minMaxOptimizer.parallel_configuration_iterator_map(pc_iter_map);
    minMaxOptimizer.init_communicators(pl_iter); // restore methodPCIter et al.
#else
    Cerr << "\nError: method recourse not possible in NonDLocalInterval "
	 << "(OPT++ NIP unavailable).\n";
    abort_handler(-1);
#endif
    npsolFlag = false;
  }
}

} // namespace Dakota
