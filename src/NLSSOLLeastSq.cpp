/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NLSSOLLeastSq
//- Description: Implementation code for the NLSSOLLeastSq class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NLSSOLLeastSq.hpp"
#include "ProblemDescDB.hpp"
#include <algorithm>

static const char rcsId[]="@(#) $Id: NLSSOLLeastSq.cpp 7029 2010-10-22 00:17:02Z mseldre $";

#define NLSSOL_F77 F77_FUNC(nlssol,NLSSOL)
extern "C" void NLSSOL_F77(int& m, int& n, int& nclin, int& ncnln, int& nrowa,
			   int& nrowcj, int& nrowfj, int& nrowr, double* a,
			   double* bl, double* bu,
			   void (*funcon)(int& mode, int& ncnln, int& n,
					  int& nrowj, int* needc, double* x,
					  double* c, double* cjac, int& nstate),
			   void (*funobj)(int& mode, int& m, int& n,
					  int& nrowfj, double* x, double* f,
					  double* gradf, int& nstate),
			   int& inform, int& iter, int* istate, double* c,
			   double* cjac, double* y, double* f, double* fjac,
			   double* clambda, double& objf, double* r, double* x,
			   int* iw, int& leniw, double* w, int& lenw);

namespace Dakota {

NLSSOLLeastSq* NLSSOLLeastSq::nlssolInstance(NULL);


/** This is the primary constructor.  It accepts a Model reference. */
NLSSOLLeastSq::NLSSOLLeastSq(ProblemDescDB& problem_db, Model& model):
  LeastSq(problem_db, model), SOLBase(model)
{
  // invoke SOLBase set function (shared with NPSOLOptimizer)
  set_options(speculativeFlag, vendorNumericalGradFlag, outputLevel,
              probDescDB.get_int("method.npsol.verify_level"),
              probDescDB.get_real("method.function_precision"),
              probDescDB.get_real("method.npsol.linesearch_tolerance"),
              maxIterations, constraintTol, convergenceTol,
	      iteratedModel.gradient_type(),
	      iteratedModel.fd_gradient_step_size());
}


/** This is an alternate constructor which accepts a Model but does
    not have a supporting method specification from the ProblemDescDB. */
NLSSOLLeastSq::NLSSOLLeastSq(Model& model):
  LeastSq(NLSSOL_SQP, model), SOLBase(model)
{
  // invoke SOLBase set function (shared with NPSOLOptimizer)
  set_options(speculativeFlag, vendorNumericalGradFlag, outputLevel, -1,
	      1.e-10, 0.9, maxIterations, constraintTol, convergenceTol,
	      iteratedModel.gradient_type(),
	      iteratedModel.fd_gradient_step_size());
}


NLSSOLLeastSq::~NLSSOLLeastSq()
{
  // Virtual destructor handles referenceCount at Iterator level.
}


void NLSSOLLeastSq::
least_sq_eval(int& mode, int& m, int& n, int& nrowfj, double* x, double* f,
              double* gradf, int& nstate)
{
  // NLSSOL computes constraints first, then the least squares terms.  However, 
  // Dakota assumes that the least square and constraint function values are
  // all computed in a single pass. A numNonlinearConstraints check is
  // therefore needed to ensure that 1 and only 1 mapping occurs.

  // Handle special cases with asv_request (see SOLBase::constraint_eval)
  short asv_request = mode + 1; // default definition of asv_request

  //if ( !(solInstance->derivLevel & 1) && (asv_request & 2) ) { // more general
  if (nlssolInstance->vendorNumericalGradFlag && (asv_request & 2) ) {
    asv_request -= 2; // downgrade request
    if (nlssolInstance->numNonlinearConstraints == 0) { // else already printed
      Cout << "NLSSOL has requested least squares gradients for case of vendor "
	   << "numerical gradients.\n";
      if (asv_request)
	Cout << "Request will be downgraded to least squares values alone.\n"
	     << std::endl;
      else
	Cout << "Request will be ignored and no evaluation performed.\n"
	     << std::endl;
    }
  }

  if (asv_request && nlssolInstance->numNonlinearConstraints == 0) {
    // constraint_eval has not been called.  Therefore, set vars/asv
    // and perform an evaluate() prior to data recovery.
    RealVector local_des_vars(n);
    copy_data(x, n, local_des_vars);
    nlssolInstance->iteratedModel.continuous_variables(local_des_vars);
    nlssolInstance->activeSet.request_values(asv_request);
    nlssolInstance->iteratedModel.evaluate(nlssolInstance->activeSet);
    if (++nlssolInstance->fnEvalCntr == nlssolInstance->maxFunctionEvals) {
      mode = -1; // terminate NLSSOL (see mode discussion in "User-Supplied
      // Subroutines" section of NPSOL manual)
      Cout << "Iteration terminated: max_function_evaluations limit has been "
	   << "met." << std::endl;
    }
  }
  
  const Response& local_response
    = nlssolInstance->iteratedModel.current_response();
  if (asv_request & 1) {
    const RealVector& local_fn_vals = local_response.function_values();
    for (size_t i=0; i<m; i++)
      f[i] = local_fn_vals[i];
  }
  if (asv_request & 2) {
    const RealMatrix& local_fn_grads = local_response.function_gradients();
    // Loop order is reversed (j, then i) since Fortran matrix ordering is
    // reversed from C ordering.
    size_t cntr = 0;
    for (size_t j=0; j<n; j++)
      for (size_t i=0; i<m; i++)
	gradf[cntr++] = local_fn_grads(j,i);
  }
}


void NLSSOLLeastSq::core_run()
{
  //------------------------------------------------------------------
  //     Solve the problem.
  //------------------------------------------------------------------

  // set the object instance pointers for use within the static member fns
  NLSSOLLeastSq* prev_nls_instance = nlssolInstance;
  SOLBase*       prev_sol_instance = solInstance;
  nlssolInstance = this; solInstance = this; optLSqInstance = this;

  // set the constraint offset used in SOLBase::constraint_eval()
  constrOffset = numLeastSqTerms;

  fnEvalCntr = 0; // prevent current iterator from continuing previous counting

  // Use data structures in the NLSSOL call that are NOT updated in
  // constraint_eval or least_sq_eval [Using overlapping arrays causes
  // erroneous behavior].

  // casts for Fortran interface
  int num_cv = numContinuousVars;
  int num_least_sq_terms = numLeastSqTerms;
  int num_linear_constraints = numLinearConstraints;
  int num_nonlinear_constraints = numNonlinearConstraints;

  double     local_f_val = 0.;
  RealVector local_lsq_vals(numLeastSqTerms);
  RealVector local_lsq_offsets(numLeastSqTerms, true);
  double*    local_lsq_grads = new double [numLeastSqTerms*numContinuousVars];

  allocate_arrays(numContinuousVars, numNonlinearConstraints,
		  iteratedModel.linear_ineq_constraint_coeffs(),
		  iteratedModel.linear_eq_constraint_coeffs());
  allocate_workspace(numContinuousVars, numNonlinearConstraints,
                     numLinearConstraints, numLeastSqTerms);

  // NLSSOL requires a non-zero array size.  Therefore, size the local 
  // constraint arrays and matrices to a size of 1 if there are no nonlinear
  // constraints and to the proper size otherwise.
  RealVector local_c_vals(nlnConstraintArraySize);

  // initialize local_des_vars with DB initial point.  Variables are updated 
  // in constraint_eval/least_sq_eval
  RealVector local_des_vars;
  copy_data(iteratedModel.continuous_variables(), local_des_vars);

  // Augmentation of bounds appears here rather than in the constructor because
  // these bounds must be updated from model bounds each time an iterator is
  // run within the B&B minimizer.
  RealVector augmented_l_bnds, augmented_u_bnds;
  copy_data(iteratedModel.continuous_lower_bounds(), augmented_l_bnds);
  copy_data(iteratedModel.continuous_upper_bounds(), augmented_u_bnds);
  augment_bounds(augmented_l_bnds, augmented_u_bnds,
		 iteratedModel.linear_ineq_constraint_lower_bounds(),
		 iteratedModel.linear_ineq_constraint_upper_bounds(),
		 iteratedModel.linear_eq_constraint_targets(),
		 iteratedModel.nonlinear_ineq_constraint_lower_bounds(),
		 iteratedModel.nonlinear_ineq_constraint_upper_bounds(),
		 iteratedModel.nonlinear_eq_constraint_targets());

  NLSSOL_F77( num_least_sq_terms, num_cv, num_linear_constraints,
	      num_nonlinear_constraints, linConstraintArraySize,
	      nlnConstraintArraySize, num_least_sq_terms, num_cv,
	      linConstraintMatrixF77, augmented_l_bnds.values(),
	      augmented_u_bnds.values(), constraint_eval, least_sq_eval,
	      informResult, numberIterations, &constraintState[0],
	      local_c_vals.values(), constraintJacMatrixF77,
	      local_lsq_offsets.values(), local_lsq_vals.values(),
	      local_lsq_grads, &cLambda[0], local_f_val, upperFactorHessianF77,
	      local_des_vars.values(), &intWorkSpace[0], intWorkSpaceSize,
	      &realWorkSpace[0], realWorkSpaceSize );

  // NLSSOL completed. Do post-processing/output of final NLSSOL info and data:
  Cout << "\nNLSSOL exits with INFORM code = " << informResult
       << " (see \"Interpretation of output\" section of NPSOL manual)\n";

  deallocate_arrays(); // SOLBase deallocate fn (shared with NPSOLOptimizer)
  delete [] local_lsq_grads;

  // Set best variables and response for use at higher levels.
  // local_des_vars, local_lsq_vals, & local_c_vals contain the optimal design 
  // (not the final fn. eval) since NLSSOL performs this assignment internally 
  // prior to exiting (see "Subroutine npsol" section of NPSOL manual).
  bestVariablesArray.front().continuous_variables(local_des_vars);

  // If no interpolation, numUserPrimaryFns <= numLsqTerms.  Copy the
  // first block of inbound model fns to best.  If data transform,
  // will be further transformed back to user space (weights, scale,
  // data) if needed in LeastSq::post_run
  RealVector best_fns = bestResponseArray.front().function_values_view();
  // take care with each copy to not resize the best_fns
  if ( !(calibrationDataFlag && expData.interpolate_flag()) )
    copy_data_partial(local_lsq_vals, 0, (int)numUserPrimaryFns, best_fns, 0);
  if (numNonlinearConstraints)
    copy_data_partial(local_c_vals, 0, (int)nlnConstraintArraySize, best_fns,
		      (int)numUserPrimaryFns);

  /*
  // For better post-processing, could append fort.9 to dakota.out line
  // by line, but: THERE IS A PROBLEM WITH GETTING ALL OF THE FILE!
  // (FORTRAN output is lacking a final buffer flush?)
  Cout << "\nEcho NLSSOL's iteration output from fort.9 file:\n" << std::endl;
  std::ifstream npsol_fort_9( "fort.9" );
  char fort_9_line[255];
  while (npsol_fort_9) {
    npsol_fort_9.getline( fort_9_line, 255 );
    Cout << fort_9_line << '\n';
  }
  Cout << std::endl;
  */
  Cout << "\nNOTE: see Fortran device 9 file (fort.9 or ftn09)"
       << "\n      for complete NLSSOL iteration history." << std::endl;

  // restore in case of recursion
  nlssolInstance = prev_nls_instance;
  solInstance    = prev_sol_instance;
  optLSqInstance = prevMinInstance;
}

} // namespace Dakota
