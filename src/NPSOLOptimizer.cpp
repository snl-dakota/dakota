/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NPSOLOptimizer.hpp"
#include "ProblemDescDB.hpp"
#include <algorithm>
#include <sstream>

static const char rcsId[]="@(#) $Id: NPSOLOptimizer.cpp 7029 2010-10-22 00:17:02Z mseldre $";

#define NPSOL_F77   F77_FUNC(npsol,NPSOL)
// BMA (20160315): Changed to use Fortran 2003 ISO C bindings.
// The Fortran symbol will be lowercase with same name as if in C
//#define NPOPTN2_F77 F77_FUNC(npoptn2,NPOPTN2)
#define NPOPTN2_F77 npoptn2

extern "C" {

void NPSOL_F77( int& n, int& nclin, int& ncnln, int& nrowa, int& nrowj,
		int& nrowr, double* a, double* bl, double* bu,
		void (*funcon)(int& mode, int& ncnln, int& n, int& nrowj,
			       int* needc, double* x, double* c, double* cjac, 
			       int& nstate),
		void (*funobj)(int& mode, int& n, double* x, double& f,
			       double* gradf, int& nstate),
		int& inform, int& iter, int* istate, double* c, double* cjac,
		double* clambda, double& objf, double* grad, double* r,
		double* x, int* iw, int& leniw, double* w, int& lenw );

// NOTE: replacing void* function pointers with void(NPSOLOptimizer::*) member
// function pointers in the declaration above (in order to try to remove static
// requirement) will compile and link, but this pointer format is not 
// acceptible to the Fortran and the call to NPSOL core dumps with an "Illegal
// Instruction" message.  Also, trying to cast from a member function pointer 
// to a function pointer is not allowed by the compiler.

void NPOPTN2_F77( const char* option_string );

//void mcenvn( int& idmy1, int& idmy2, double& eps, int& idmy3, double& rdmy);

}


namespace Dakota {

NPSOLOptimizer* NPSOLOptimizer::npsolInstance(NULL);


/** This is the primary constructor.  It accepts a Model reference. */
NPSOLOptimizer::NPSOLOptimizer(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model):
  Optimizer(problem_db, parallel_lib, model, std::shared_ptr<TraitsBase>(new NPSOLTraits())),
  SOLBase(model), setUpType("model")
{
  // invoke SOLBase set function (shared with NLSSOLLeastSq)
  set_options(speculativeFlag, vendorNumericalGradFlag, outputLevel,
              probDescDB.get_int("method.npsol.verify_level"),
              probDescDB.get_real("method.function_precision"),
              probDescDB.get_real("method.npsol.linesearch_tolerance"),
              maxIterations, constraintTol, convergenceTol,
	      iteratedModel->gradient_type(),
	      iteratedModel->fd_gradient_step_size());
}


/** This is an alternate constructor which accepts a Model but does
    not have a supporting method specification from the ProblemDescDB. */
NPSOLOptimizer::NPSOLOptimizer(std::shared_ptr<Model> model):
  Optimizer(NPSOL_SQP, model, std::shared_ptr<TraitsBase>(new NPSOLTraits())),
  SOLBase(model), setUpType("model")
{
  // invoke SOLBase set function (shared with NLSSOLLeastSq)
  set_options(speculativeFlag, vendorNumericalGradFlag, outputLevel, -1,
	      1.e-10, 0.9, maxIterations, constraintTol, convergenceTol,
	      iteratedModel->gradient_type(),
	      iteratedModel->fd_gradient_step_size());
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
NPSOLOptimizer::
NPSOLOptimizer(std::shared_ptr<Model> model, int derivative_level, Real conv_tol):
  Optimizer(NPSOL_SQP, model, std::shared_ptr<TraitsBase>(new NPSOLTraits())),
  SOLBase(model), setUpType("model")
{
  // Set NPSOL options (mostly use defaults)
  send_sol_option("Verify Level                = -1");
  send_sol_option("Major Print Level           = 0");

  // assign the derivative_level passed in
  send_sol_option("Derivative Level            = " +
		  std::to_string(derivative_level));

  // assign the conv_tol passed in
  if (conv_tol > 0.) // conv_tol < 0 can be passed to use the NPSOL default
    send_sol_option("Optimality Tolerance        = " +
		    std::to_string(conv_tol));
}


/** This is an alternate constructor for performing an optimization using
    the passed in objective function and constraint function pointers. */
NPSOLOptimizer::
NPSOLOptimizer(const RealVector& cv_initial,
	       const RealVector& cv_lower_bnds,
	       const RealVector& cv_upper_bnds,
	       const RealMatrix& lin_ineq_coeffs,
	       const RealVector& lin_ineq_lower_bnds,
	       const RealVector& lin_ineq_upper_bnds,
	       const RealMatrix& lin_eq_coeffs,
	       const RealVector& lin_eq_targets,
	       const RealVector& nonlin_ineq_lower_bnds,
	       const RealVector& nonlin_ineq_upper_bnds,
	       const RealVector& nonlin_eq_targets,
	       void (*user_obj_eval) (int&, int&, double*, double&,
				      double*, int&),
	       void (*user_con_eval) (int&, int&, int&, int&, int*,
				      double*, double*, double*, int&),
	       int derivative_level, Real conv_tol, size_t max_iter, Real fdss,
	       Real fn_precision, Real feas_tol, Real lin_feas_tol,
	       Real nonlin_feas_tol):
  // use SOLBase default ctor
  Optimizer(NPSOL_SQP, cv_initial.length(), 0, 0, 0,
	    lin_ineq_coeffs.numRows(), lin_eq_coeffs.numRows(),
	    nonlin_ineq_lower_bnds.length(), nonlin_eq_targets.length(),
            std::shared_ptr<TraitsBase>(new NPSOLTraits())),
  setUpType("user_functions"), userObjectiveEval(user_obj_eval),
  userConstraintEval(user_con_eval), linIneqCoeffs(lin_ineq_coeffs),
  linEqCoeffs(lin_eq_coeffs)
{
  allocate_arrays(numContinuousVars, numNonlinearConstraints,
		  lin_ineq_coeffs, lin_eq_coeffs);
  initial_point(cv_initial);
  aggregate_bounds(cv_lower_bnds, cv_upper_bnds, lin_ineq_lower_bnds,
		   lin_ineq_upper_bnds, lin_eq_targets, nonlin_ineq_lower_bnds,
		   nonlin_ineq_upper_bnds, nonlin_eq_targets,
		   lowerBounds, upperBounds);

  // Set NPSOL options (mostly use defaults)
  send_sol_option(  "Verify Level                = -1");

  if (outputLevel >= QUIET_OUTPUT)
    send_sol_option("Major Print Level           = 0");
  if (outputLevel >= NORMAL_OUTPUT)
    send_sol_option("Minor Print Level           = 0");

  // Set Derivative Level = 3 for user-supplied gradients, 0 for NPSOL
  // vendor-numerical, ...
  send_sol_option(  "Derivative Level            = " +
		  std::to_string(derivative_level));

  if (fdss > 0.) {
    String fdss_str(std::to_string(fdss));
    send_sol_option("Difference Interval         = " + fdss_str);
    send_sol_option("Central Difference Interval = " + fdss_str);
  }
  if (max_iter > 0)
    send_sol_option("Major Iteration Limit       = " +
		    std::to_string(max_iter));
  if (conv_tol > 0.) // conv_tol <= 0. results in internal NPSOL default
    send_sol_option("Optimality Tolerance        = " +
		    std::to_string(conv_tol));
  if (fn_precision > 0.)
    send_sol_option("Function Precision          = " +
		    std::to_string(fn_precision));
  if (feas_tol > 0.)
    send_sol_option("Feasibility Tolerance       = " +
		    std::to_string(feas_tol));
  if (lin_feas_tol > 0.)
    send_sol_option("Linear Feasibility Tolerance = " +
		    std::to_string(lin_feas_tol));
  if (nonlin_feas_tol > 0.)
    send_sol_option("Nonlinear Feasibility Tolerance = " +
		    std::to_string(nonlin_feas_tol));
}


NPSOLOptimizer::~NPSOLOptimizer()
{
  // invoke SOLBase deallocate function (shared with NLSSOLLeastSq)
  if (setUpType == "user_functions")
    deallocate_arrays();
}


#ifdef HAVE_DYNLIB_FACTORIES
NPSOLOptimizer* new_NPSOLOptimizer(ProblemDescDB& problem_db)
{
#ifdef DAKOTA_DYNLIB
  not_available("NPSOL");
  return 0;
#else
  return new NPSOLOptimizer(problem_db);
#endif
}

NPSOLOptimizer* new_NPSOLOptimizer1(Model& model)
{
#ifdef DAKOTA_DYNLIB
  not_available("NPSOL");
  return 0;
#else
  return new NPSOLOptimizer(model);
#endif
}

NPSOLOptimizer* new_NPSOLOptimizer2(Model& model, int derivative_level,
                                    Real conv_tol)
{
#ifdef DAKOTA_DYNLIB
  not_available("NPSOL");
  return 0;
#else
  return new NPSOLOptimizer(model, derivative_level, conv_tol);
#endif
}

NPSOLOptimizer* new_NPSOLOptimizer3(const RealVector& cv_initial,
  const RealVector& cv_lower_bnds,
  const RealVector& cv_upper_bnds,
  const RealMatrix& lin_ineq_coeffs,
  const RealVector& lin_ineq_lower_bnds,
  const RealVector& lin_ineq_upper_bnds,
  const RealMatrix& lin_eq_coeffs,
  const RealVector& lin_eq_targets,
  const RealVector& nonlin_ineq_lower_bnds,
  const RealVector& nonlin_ineq_upper_bnds,
  const RealVector& nonlin_eq_targets,
  void (*user_obj_eval) (int&, int&, double*, double&, double*, int&),
  void (*user_con_eval) (int&, int&, int&, int&, int*, double*, double*,
                         double*, int&),
  int derivative_level, Real conv_tol)
{
#ifdef DAKOTA_DYNLIB
  not_available("NPSOL");
  return 0;
#else
  return new NPSOLOptimizer(cv_initial, cv_lower_bnds, cv_upper_bnds,
	       lin_ineq_coeffs, lin_ineq_lower_bnds, lin_ineq_upper_bnds,
	       lin_eq_coeffs, lin_eq_targets, nonlin_ineq_lower_bnds,
	       nonlin_ineq_upper_bnds, nonlin_eq_targets, user_obj_eval,
	       user_con_eval, derivative_level, conv_tol);
#endif // DAKOTA_DYNLIB
}
#endif // HAVE_DYNLIB_FACTORIES


void NPSOLOptimizer::
update_callback_data(const RealVector& cv_initial,
		     const RealVector& cv_lower_bnds,
		     const RealVector& cv_upper_bnds,
		     const RealMatrix& lin_ineq_coeffs,
		     const RealVector& lin_ineq_l_bnds,
		     const RealVector& lin_ineq_u_bnds,
		     const RealMatrix& lin_eq_coeffs,
		     const RealVector& lin_eq_targets,
		     const RealVector& nln_ineq_l_bnds,
		     const RealVector& nln_ineq_u_bnds,
		     const RealVector& nln_eq_targets)
{
  enforce_null_model();

  bool reshape = false;
  size_t num_cv  = cv_initial.length(),
    num_lin_ineq = lin_ineq_coeffs.numRows(),
    num_lin_eq   = lin_eq_coeffs.numRows(),
    num_nln_ineq = nln_ineq_l_bnds.length(),
    num_nln_eq   = nln_eq_targets.length();
  if (numContinuousVars != num_cv)
    { numContinuousVars  = num_cv; reshape = true; }
  if (numLinearIneqConstraints != num_lin_ineq ||
      numLinearEqConstraints   != num_lin_eq)
    { numLinearIneqConstraints  = num_lin_ineq;
      numLinearEqConstraints    = num_lin_eq; reshape = true; }
  if (numNonlinearIneqConstraints != num_nln_ineq ||
      numNonlinearEqConstraints   != num_nln_eq)
    { numNonlinearIneqConstraints  = num_nln_ineq;
      numNonlinearEqConstraints    = num_nln_eq; reshape = true; }
  numLinearConstraints = numLinearIneqConstraints + numLinearEqConstraints;
  numNonlinearConstraints
    = numNonlinearIneqConstraints + numNonlinearEqConstraints;
  numConstraints = numNonlinearConstraints + numLinearConstraints;
  numFunctions = numObjectiveFns + numNonlinearConstraints;

  linIneqCoeffs = lin_ineq_coeffs;  linEqCoeffs = lin_eq_coeffs;
  //linIneqLowerBnds = lin_ineq_l_bnds;  linIneqUpperBnds = lin_ineq_u_bnds;
  //linEqTargets     = lin_eq_targets;

  //nlnIneqLowerBnds = nln_ineq_l_bnds;  nlnIneqUpperBnds = nln_ineq_u_bnds;
  //nlnEqTargets     = nln_eq_targets;

  initial_point(cv_initial);
  aggregate_bounds(cv_lower_bnds, cv_upper_bnds, lin_ineq_l_bnds,
		   lin_ineq_u_bnds, lin_eq_targets, nln_ineq_l_bnds,
		   nln_ineq_u_bnds, nln_eq_targets, lowerBounds, upperBounds);
  if (reshape)
    reshape_best(numContinuousVars, numFunctions);
}


void NPSOLOptimizer::
objective_eval(int& mode, int& n, double* x, double& f, double* gradf,
	       int& nstate)
{
  // NPSOL computes constraints first, then the objective function.  However, 
  // Dakota assumes that the objective and constraint function values are all 
  // computed in a single fn. evaluation.  A numNonlinearConstraints check is
  // therefore needed to ensure that 1 and only 1 mapping occurs.

  // Handle special cases with asv_request (see SOLBase::constraint_eval)
  short asv_request = mode + 1; // default definition of asv_request

  //if ( !(solInstance->derivLevel & 1) && (asv_request & 2) ) { // more general
  if (npsolInstance->vendorNumericalGradFlag && (asv_request & 2) ) {
    asv_request -= 2; // downgrade request
    if (npsolInstance->numNonlinearConstraints == 0) { // else already printed
      Cout << "NPSOL has requested objective gradient for case of vendor "
	   << "numerical gradients.\n";
      if (asv_request)
	Cout << "Request will be downgraded to objective value alone.\n"
             << std::endl;
      else
	Cout << "Request will be ignored and no evaluation performed.\n"
             << std::endl;
    }
  }

  if (asv_request && npsolInstance->numNonlinearConstraints == 0) {
    // constraint_eval has not been called.  Therefore, set vars/asv
    // and perform an evaluate() prior to data recovery.
    RealVector local_des_vars(n, false);
    copy_data(x, n, local_des_vars);
    ModelUtils::continuous_variables(*npsolInstance->iteratedModel, local_des_vars);
    npsolInstance->activeSet.request_values(asv_request);
    npsolInstance->
      iteratedModel->evaluate(npsolInstance->activeSet);
    if (++npsolInstance->fnEvalCntr == npsolInstance->maxFunctionEvals) {
      mode = -1; // terminate NPSOL (see mode discussion in "User-Supplied
	         // Subroutines" section of NPSOL manual)
      Cout << "Iteration terminated: max_function_evaluations limit has been "
	   << "met." << std::endl;
    }
  }
  
  const Response& local_response
    = npsolInstance->iteratedModel->current_response();
  // Any MOO/NLS recasting is responsible for setting the scalar min/max
  // sense within the recast.
  const BoolDeque& max_sense
    = npsolInstance->iteratedModel->primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);
  if (asv_request & 1)
    f = (max_flag) ? -local_response.function_value(0) :
                      local_response.function_value(0);
  if (asv_request & 2) {
    const Real* local_grad = local_response.function_gradient(0);
    if (max_flag)
      for (size_t i=0; i<n; ++i)
	gradf[i] = -local_grad[i];
    else
      std::copy(local_grad, local_grad + n, gradf);
  }
}


void NPSOLOptimizer::check_sub_iterator_conflict()
{
  // Run-time check (NestedModel::subIterator is constructed in init_comms())
  if (setUpType == "model")
    SOLBase::check_sub_iterator_conflict(*iteratedModel, methodName);
}


void NPSOLOptimizer::core_run()
{
  if (setUpType == "model")
    find_optimum_on_model();
  else if (setUpType == "user_functions")
    find_optimum_on_user_functions();
  else {
    Cerr << "Error: bad setUpType in NPSOLOptimizer::core_run()."
         << std::endl;
    abort_handler(-1);
  }
}


void NPSOLOptimizer::send_sol_option(std::string sol_option)
{
  // The subroutine npoptn2 in file sol_optn_wrapper.f accepts a string of 
  // length 72 (the max that NPSOL accepts) which is then passed along to
  // the npoptn routine in NPSOL. Therefore, strings passed to npoptn2 need
  // to be of length 72 (thus, the use of data() rather than c_str()).
  sol_option.resize(72, ' ');
  NPOPTN2_F77(sol_option.data()); // NO Null terminator with std::string::data()
}


void NPSOLOptimizer::find_optimum_on_model()
{
  //------------------------------------------------------------------
  //     Solve the problem.
  //------------------------------------------------------------------

  // set the object instance pointers for use within the static member fns
  NPSOLOptimizer* prev_nps_instance = npsolInstance;
  SOLBase*        prev_sol_instance = solInstance;
  npsolInstance = this; solInstance = this; optLSqInstance = this;

  // Augmentation of bounds appears here rather than in the constructor because
  // set the constraint offset used in SOLBase::constraint_eval()
  constrOffset = numObjectiveFns;

  fnEvalCntr = 0; // prevent current iterator from continuing previous counting

  // casts for Fortran interface
  int num_cv = numContinuousVars;
  int num_linear_constraints = numLinearConstraints;
  int num_nonlinear_constraints = numNonlinearConstraints;

  double     local_f_val = 0.;
  RealVector local_f_grad(numContinuousVars, true);

  allocate_arrays(numContinuousVars, numNonlinearConstraints,
		  ModelUtils::linear_ineq_constraint_coeffs(*iteratedModel),
		  ModelUtils::linear_eq_constraint_coeffs(*iteratedModel));
  allocate_workspace(numContinuousVars, numNonlinearConstraints,
                     numLinearConstraints, 0);

  // NPSOL requires a non-zero array size.  Therefore, size the local 
  // constraint arrays and matrices to a size of 1 if there are no nonlinear
  // constraints and to the proper size otherwise.
  RealVector local_c_vals(nlnConstraintArraySize);

  // initialize local_des_vars with DB initial point.  Variables are updated 
  // in constraint_eval/objective_eval
  RealVector local_des_vars;
  copy_data(ModelUtils::continuous_variables(*iteratedModel), local_des_vars);

  // these bounds must be updated from model bounds each time an iterator is
  // run within the B&B minimizer.
  RealVector augmented_l_bnds, augmented_u_bnds;
  aggregate_bounds(ModelUtils::continuous_lower_bounds(*iteratedModel),
		   ModelUtils::continuous_upper_bounds(*iteratedModel),
		   ModelUtils::linear_ineq_constraint_lower_bounds(*iteratedModel),
		   ModelUtils::linear_ineq_constraint_upper_bounds(*iteratedModel),
		   ModelUtils::linear_eq_constraint_targets(*iteratedModel),
		   ModelUtils::nonlinear_ineq_constraint_lower_bounds(*iteratedModel),
		   ModelUtils::nonlinear_ineq_constraint_upper_bounds(*iteratedModel),
		   ModelUtils::nonlinear_eq_constraint_targets(*iteratedModel),
		   augmented_l_bnds, augmented_u_bnds);

  NPSOL_F77( num_cv, num_linear_constraints, num_nonlinear_constraints, 
	     linConstraintArraySize, nlnConstraintArraySize, num_cv, 
	     linConstraintMatrixF77, augmented_l_bnds.values(),
	     augmented_u_bnds.values(), constraint_eval, objective_eval,
	     informResult, numberIterations, &constraintState[0],
	     local_c_vals.values(), constraintJacMatrixF77, &cLambda[0],
	     local_f_val, local_f_grad.values(), upperFactorHessianF77,
	     local_des_vars.values(), &intWorkSpace[0], intWorkSpaceSize,
	     &realWorkSpace[0], realWorkSpaceSize );

  // NPSOL completed. Do post-processing/output of final NPSOL info and data:
  Cout << "\nNPSOL exits with INFORM code = " << informResult
       << " (see \"Interpretation of output\" section in NPSOL manual)\n";

  // invoke SOLBase deallocate function (shared with NLSSOLLeastSq)
  deallocate_arrays();

  // Set best variables and response for use at higher levels.
  // local_des_vars, local_f_val, & local_c_vals contain the optimal design 
  // (not the final fn. eval) since NPSOL performs this assignment internally 
  // prior to exiting (see "Subroutine npsol" section of NPSOL manual).
  bestVariablesArray.front().continuous_variables(local_des_vars);
  RealVector best_fns(bestResponseArray.front().num_functions());
  if (localObjectiveRecast) {
    // local_objective_recast_retrieve() is used in Optimizer::post_run()
  }
  else {
    if(iteratedModel) {
      const BoolDeque& max_sense = iteratedModel->primary_response_fn_sense();
      best_fns[0] = (!max_sense.empty() && max_sense[0]) ?
        -local_f_val : local_f_val;
    } else {
      best_fns[0] = local_f_val;
    }
  }
  if (numNonlinearConstraints) // numUserPrimaryFns is 1 if no recast
    copy_data_partial(local_c_vals, best_fns, numUserPrimaryFns);
  bestResponseArray.front().function_values(best_fns);

  /*
  // For better post-processing, could append fort.9 to dakota.out line
  // by line, but: THERE IS A PROBLEM WITH GETTING ALL OF THE FILE!
  // (FORTRAN output is lacking a final buffer flush?)
  Cout << "\nEcho NPSOL's iteration output from fort.9 file:\n" << std::endl;
  ifstream npsol_fort_9( "fort.9" );
  char fort_9_line[255];
  while (npsol_fort_9) {
    npsol_fort_9.getline( fort_9_line, 255 );
    Cout << fort_9_line << '\n';
  }
  Cout << std::endl;
  */
  Cout << "\nNOTE: see Fortran device 9 file (fort.9 or ftn09)"
       << "\n      for complete NPSOL iteration history." << std::endl;

  // restore in case of recursion
  npsolInstance  = prev_nps_instance;
  solInstance    = prev_sol_instance;
  optLSqInstance = prevMinInstance;
}


void NPSOLOptimizer::find_optimum_on_user_functions()
{
  //------------------------------------------------------------------
  //     Solve the problem.
  //------------------------------------------------------------------

  // ints for Fortran interface
  int i, num_cv = numContinuousVars,
    num_lin_constraints = numLinearConstraints,
    num_nln_constraints = numNonlinearConstraints;

  allocate_arrays(num_cv, num_nln_constraints, linIneqCoeffs, linEqCoeffs);
  allocate_workspace(num_cv, num_nln_constraints, num_lin_constraints, 0);

  double     local_f_val = 0.;
  RealVector local_des_vars, local_f_grad(numContinuousVars, true),
    local_c_vals(nlnConstraintArraySize);
  copy_data(initialPoint, local_des_vars); // preserve initialPoint

  NPSOL_F77( num_cv, num_lin_constraints, num_nln_constraints, 
	     linConstraintArraySize, nlnConstraintArraySize, num_cv, 
	     linConstraintMatrixF77, lowerBounds.values(), upperBounds.values(),
	     userConstraintEval, userObjectiveEval, informResult,
	     numberIterations, &constraintState[0], local_c_vals.values(),
	     constraintJacMatrixF77, &cLambda[0], local_f_val,
	     local_f_grad.values(), upperFactorHessianF77,
	     local_des_vars.values(), &intWorkSpace[0], intWorkSpaceSize,
	     &realWorkSpace[0], realWorkSpaceSize );

  // NPSOL completed. Do post-processing/output of final NPSOL info and data:
  Cout << "\nNPSOL exits with INFORM code = " << informResult
       << " (see \"Interpretation of output\" section in NPSOL manual)\n";

  // invoke SOLBase deallocate function (shared with NLSSOLLeastSq)
  deallocate_arrays();

  bestVariablesArray.front().continuous_variables(local_des_vars);
  // user-functions mode is restricted to single-objective optimization
  RealVector best_fns(numFunctions, false);
  best_fns[0] = local_f_val;
  if (numNonlinearConstraints)
    copy_data_partial(local_c_vals, best_fns, 1);
  bestResponseArray.front().function_values(best_fns);
}


// This override exists purely to prevent an optimizer/minimizer from
// declaring sources when it's being used to evaluate a user-defined
// function (e.g. finding the correlation lengths of Dakota's GP).
void NPSOLOptimizer::declare_sources() {
  if(setUpType == "user_functions") 
    return;
  else
    Iterator::declare_sources();
}
 
} // namespace Dakota
