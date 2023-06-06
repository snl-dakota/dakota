/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SNLLLeastSq
//- Description: Implementation code for the SNLLLeastSq class
//- Owner:       Pam Williams
//- Checked by:
//- Change Log:  

#include "SNLLLeastSq.hpp"
#include "DakotaModel.hpp"
#include "ScalingModel.hpp"
#include "ProblemDescDB.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "OptNewton.h"
#include "OptNewtonLike.h"
#include "OptBCNewton.h"
#include "OptBCNewtonLike.h"
#include "OptDHNIPS.h"
#include "OptNIPSLike.h"
#include "NLF.h"
#include "NLP.h"
#include "OptppArray.h"

static const char rcsId[]="@(#) $Id: SNLLLeastSq.cpp 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {
extern PRPCache data_pairs; // global container

SNLLLeastSq* SNLLLeastSq::snllLSqInstance(NULL);


SNLLLeastSq::SNLLLeastSq(ProblemDescDB& problem_db, Model& model):
  LeastSq(problem_db, model, std::shared_ptr<TraitsBase>(new SNLLLeastSqTraits())),
  SNLLBase(problem_db), nlfObjective(NULL),
  nlfConstraint(NULL), nlpConstraint(NULL), theOptimizer(NULL)
{
  // historical default convergence tolerance
  if (convergenceTol < 0.0) convergenceTol = 1.0e-4;

  // convenience function from SNLLBase
  snll_pre_instantiate(boundConstraintFlag, numConstraints);

  // Instantiate NLF & Optimizer objects based on method & gradient selections

  // Gauss-Newton: unconstrained, bound-constrained, & nonlinear interior-point
  if (methodName != OPTPP_G_NEWTON) {
    Cerr << "Method name " << method_enum_to_string(methodName)
	 << " currently unavailable within\nDAKOTA's SNLLLeastSq "
	 << "implementation of OPT++." << std::endl;
    abort_handler(-1);
  }

  // Gauss-Newton uses the full Newton optimizer with the addition of
  // code which computes f, df/dx, and d^2f/dx^2 as a function of the least
  // squares residuals and their Jacobian matrix (see nlf2_evaluator_gn).  
  // In order to support the finer granularity of data needed to exploit the
  // problem structure, Response is made up of numLeastSqTerms 
  // and constraints, rather than objective function(s) and constraints.

  if (vendorNumericalGradFlag) {
    Cerr << "Gauss-Newton does not support vendor numerical gradients.\n" 
	 << "Select dakota as method_source instead." << std::endl;
    abort_handler(-1);
  }

  nlf2 = new OPTPP::NLF2(numContinuousVars, nlf2_evaluator_gn, init_fn);
  nlfObjective = nlf2;
  nlfObjective->setModeOverride(true);
  if (numConstraints) { // nonlinear interior-point
    // **********************************************************************
    // NOTE: The combination of nlf2_evaluator_gn() with
    //       constraint1_evaluator_gn() has consistent derivative levels.
    //       OPT++ supports a mixed mode in which
    //       the optimization Hessian uses the Gauss-Newton approximation
    //       (full Newton optimizer) and the constraint Hessians use a 
    //       quasi-Newton approximation (quasi-Newton optimizer).
    // **********************************************************************
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptDHNIPS optimizer with NLF2 Gauss-Newton "
	   << "evaluator.\n";
    optdhnips = new OPTPP::OptDHNIPS(nlf2);
    //optdhnips->setSearchStrategy(searchStrat); // not supported
    optdhnips->setMeritFcn(meritFn);
    optdhnips->setStepLengthToBdry(stepLenToBndry);
    optdhnips->setCenteringParameter(centeringParam);
    theOptimizer = optdhnips;

    nlf1Con = new OPTPP::NLF1(numContinuousVars, numNonlinearConstraints,
			      constraint1_evaluator_gn, init_fn);
    nlfConstraint = nlf1Con;
    nlpConstraint = new OPTPP::NLP(nlf1Con);
  }
  else if (boundConstraintFlag) { // bound-constrained
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptBCNewton optimizer with NLF2 Gauss-Newton "
	   << "evaluator.\n";
    optbcnewton = new OPTPP::OptBCNewton(nlf2);
    optbcnewton->setSearchStrategy(searchStrat);
    if (searchStrat == OPTPP::TrustRegion) optbcnewton->setTRSize(maxStep);
    theOptimizer = optbcnewton;
  }
  else { // unconstrained
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptNewton optimizer with NLF2 Gauss-Newton "
	   << "evaluator.\n";
    optnewton = new OPTPP::OptNewton(nlf2);
    optnewton->setSearchStrategy(searchStrat);
    if (searchStrat == OPTPP::TrustRegion) optnewton->setTRSize(maxStep);
    theOptimizer = optnewton;
  }

  // convenience function from SNLLBase
  snll_post_instantiate(numContinuousVars, vendorNumericalGradFlag,
			iteratedModel.interval_type(),
			iteratedModel.fd_gradient_step_size(),
			maxIterations, maxFunctionEvals, convergenceTol,
			probDescDB.get_real("method.gradient_tolerance"),
			maxStep, boundConstraintFlag, numConstraints,
			outputLevel, theOptimizer, nlfObjective, NULL, NULL);
}


SNLLLeastSq::SNLLLeastSq(const String& method_name, Model& model):
  LeastSq(OPTPP_G_NEWTON, model, std::shared_ptr<TraitsBase>(new SNLLLeastSqTraits())), // use default SNLLBase ctor
  nlfObjective(NULL), nlfConstraint(NULL), nlpConstraint(NULL),
  theOptimizer(NULL)
{
  // convenience function from SNLLBase
  snll_pre_instantiate(boundConstraintFlag, numConstraints);

  // Instantiate NLF & Optimizer objects based on method & gradient selections

  // Gauss-Newton: unconstrained, bound-constrained, & nonlinear interior-point
  if (method_name != "optpp_g_newton") {
    Cerr << "Error: Method name " << method_name << " unsupported in "
	 << "SNLLLeastSq lightweight construction by name." << std::endl;
    abort_handler(-1);
  }

  // Gauss-Newton uses the full Newton optimizer with the addition of
  // code which computes f, df/dx, and d^2f/dx^2 as a function of the least
  // squares residuals and their Jacobian matrix (see nlf2_evaluator_gn).  
  // In order to support the finer granularity of data needed to exploit the
  // problem structure, Response is made up of numLeastSqTerms 
  // and constraints, rather than objective function(s) and constraints.

  if (vendorNumericalGradFlag) {
    Cerr << "Gauss-Newton does not support vendor numerical gradients.\n" 
	 << "Select dakota as method_source instead." << std::endl;
    abort_handler(-1);
  }

  nlf2 = new OPTPP::NLF2(numContinuousVars, nlf2_evaluator_gn, init_fn);
  nlfObjective = nlf2;
  nlfObjective->setModeOverride(true);
  if (numConstraints) { // nonlinear interior-point
    // **********************************************************************
    // NOTE: The combination of nlf2_evaluator_gn() with
    //       constraint1_evaluator_gn() has consistent derivative levels.
    //       OPT++ supports a mixed mode in which
    //       the optimization Hessian uses the Gauss-Newton approximation
    //       (full Newton optimizer) and the constraint Hessians use a 
    //       quasi-Newton approximation (quasi-Newton optimizer).
    // **********************************************************************
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptDHNIPS optimizer with NLF2 Gauss-Newton "
	   << "evaluator.\n";
    optdhnips = new OPTPP::OptDHNIPS(nlf2);
    //optdhnips->setSearchStrategy(searchStrat);      // not supported
    optdhnips->setMeritFcn(meritFn);                  // ArgaezTapia
    optdhnips->setStepLengthToBdry(stepLenToBndry);   // 0.99995
    optdhnips->setCenteringParameter(centeringParam); // 0.2
    theOptimizer = optdhnips;

    nlf1Con = new OPTPP::NLF1(numContinuousVars, numNonlinearConstraints,
			      constraint1_evaluator_gn, init_fn);
    nlfConstraint = nlf1Con;
    nlpConstraint = new OPTPP::NLP(nlf1Con);
  }
  else if (boundConstraintFlag) { // bound-constrained
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptBCNewton optimizer with NLF2 Gauss-Newton "
	   << "evaluator.\n";
    optbcnewton = new OPTPP::OptBCNewton(nlf2);
    optbcnewton->setSearchStrategy(searchStrat); // see snll_pre_instantiate
    if (searchStrat == OPTPP::TrustRegion)
      optbcnewton->setTRSize(maxStep); // 1000.
    theOptimizer = optbcnewton;
  }
  else { // unconstrained
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptNewton optimizer with NLF2 Gauss-Newton "
	   << "evaluator.\n";
    optnewton = new OPTPP::OptNewton(nlf2);
    optnewton->setSearchStrategy(searchStrat); // see snll_pre_instantiate
    if (searchStrat == OPTPP::TrustRegion)
      optnewton->setTRSize(maxStep); // 1000.
    theOptimizer = optnewton;
  }

  // convenience function from SNLLBase
  snll_post_instantiate(numContinuousVars, vendorNumericalGradFlag,
			iteratedModel.interval_type(),
			iteratedModel.fd_gradient_step_size(),
			maxIterations, maxFunctionEvals, convergenceTol, 1.e-4,
			1000., boundConstraintFlag, numConstraints, outputLevel,
			theOptimizer, nlfObjective, NULL, NULL);
}


SNLLLeastSq::~SNLLLeastSq()
{
  // free allocated memory

  // OPT++ uses virtual destructors, so the delete can be performed at the
  // base class level. 
  theOptimizer->cleanup(); 
  delete nlfObjective;
  // TO DO: deallocate constraint attribute pointers (needed w/ SmartPtr?).
  if (nlfConstraint)
    delete nlfConstraint;
  //if (nlpConstraint)
  //  delete nlpConstraint;
  delete theOptimizer;
}


/** This nlf2 evaluator function is used for the Gauss-Newton method
    in order to exploit the special structure of the nonlinear least
    squares problem.  Here, fx = sum (T_i - Tbar_i)^2 and
    Response is made up of residual functions and their
    gradients along with any nonlinear constraints.  The objective
    function and its gradient vector and Hessian matrix are computed
    directly from the residual functions and their derivatives (which
    are returned from the Response object). */
void SNLLLeastSq::
nlf2_evaluator_gn(int mode, int n, const RealVector& x, double& f,
		  RealVector& grad_f, RealSymMatrix& hess_f, int& result_mode)
{
  if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLLeastSq::nlf2_evaluator_gn called with mode = " << mode
         << '\n';

  // A mapping of the mode to the asv is required since mode governs the
  // objective function requests whereas the asv governs the requests for the
  // least squares residual functions.  This switch implementation makes no
  // use of a priori knowledge and will NOT prevent duplicated computations
  // caused by 1,2,4 mode request ordering.
  short lsq_mode;
  switch (mode) {
  case 0: case 1: case 3: // no modification needed
    lsq_mode = mode;
    break;
  case 2: case 5: case 6: case 7: // local_fn_vals & local_fn_grads both needed
    lsq_mode = 3;
    break;
  case 4: // only local_fn_grads needed in hess_f computation
    lsq_mode = 2;
    break;
  default:
    Cerr << "Bad mode input from OPT++ in nlf2_evaluator_gn." << std::endl;
    abort_handler(-1);
  }
  //Cout << "New nlf2_evaluator_gn call: mode = " << mode 
  //     << ", asv_mode required = " << lsq_mode << '\n';

  // Emulates NPSOLOptimizer::objective_f_eval() when nonlinear constraints are
  // present.  Unlike NPSOL, verify that mode and vars are consistent since
  // OPT++ does not always have a 1-to-1 correspondence in evaluator calls.
  int i, j, k;
  if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLLeastSq::nlf2_evaluator_gn vars = \n" << x;
  if (!snllLSqInstance->numNonlinearConstraints ||
      lastFnEvalLocn != CON_EVALUATOR || lsq_mode != lastEvalMode ||
      x != lastEvalVars) {
    // data not available from constraint evaluator, so perform
    // a new function evaluation.
    snllLSqInstance->iteratedModel.continuous_variables(x);
    ShortArray local_asv(snllLSqInstance->numFunctions, lsq_mode);
    // Should constraints be evaluated (if present)?  Depends on what OPT++
    // is doing.  Since we know this eval is not aligned with a preceding
    // constraint eval, assume for now that a matching constraint eval might
    // not occur (if it were to occur, then evaluating the constraints now
    // might save work through duplication detection in the constraint
    // evaluator).  NOTE: with the addition of the disaggregated Hessian NIPS
    // capability, evaluating constraints requires the use of a mode which
    // drops the Hessian bit.
    for (i=snllLSqInstance->numLeastSqTerms;
	 i<snllLSqInstance->numFunctions; i++)
      local_asv[i] = 0; //mode & 3; // nonlinear constraints (if present)

    snllLSqInstance->activeSet.request_vector(local_asv);
    snllLSqInstance->iteratedModel.evaluate(snllLSqInstance->activeSet);
    lastFnEvalLocn = NLF_EVALUATOR;
  }
  const Response& local_response
    = snllLSqInstance->iteratedModel.current_response();

  // Go ahead and always retrieve the references even though this data may not
  // have been requested in the local_asv (in which case it contains 0's).
  const RealVector& local_fn_vals  = local_response.function_values();
  const RealMatrix& local_fn_grads = local_response.function_gradients();

  // Sum the squared residuals to get the objective function = {R}'{R}.
  if (mode & 1) { // 1st bit is present, mode = 1, 3, 5, or 7
    f = 0.;
    for (i=0; i<snllLSqInstance->numLeastSqTerms; i++)
      f += local_fn_vals[i] * local_fn_vals[i];
    if (snllLSqInstance->outputLevel > NORMAL_OUTPUT)
      Cout << "    nlf2_evaluator_gn results: objective fn. =\n   " 
	   << std::setw(write_precision+7) << f << '\n';
    result_mode = OPTPP::NLPFunction;
  }

  // Get the gradient of the objective = 2*[J]'{R}.
  if (mode & 2) { // 2nd bit is present, mode = 2, 3, 6, or 7
    if (snllLSqInstance->outputLevel > NORMAL_OUTPUT)
      Cout << "    nlf2_evaluator_gn results: objective fn. gradient =\n [ ";
    for (i=0; i<n; i++) {
      grad_f(i) = 0.;
      for (j=0; j<snllLSqInstance->numLeastSqTerms; j++)
        grad_f(i) += 2. * local_fn_grads(i,j) * local_fn_vals[j];
      if (snllLSqInstance->outputLevel > NORMAL_OUTPUT)
	Cout << std::setw(write_precision+7) << grad_f(i) << ' ';
    }
    if (snllLSqInstance->outputLevel > NORMAL_OUTPUT)
      Cout << "]\n";
    result_mode |= OPTPP::NLPGradient;
  }

  // Get a Hessian approximation = 2*[J]'[J].
  if (mode & 4) { // 3rd bit is present, mode >= 4
    for (i=0; i<n; i++) {
      for (j=0; j<=i; j++) {
        Real dtmp = 0.;
        for (k=0; k<snllLSqInstance->numLeastSqTerms; k++)
          dtmp += 2. * local_fn_grads(i,k) * local_fn_grads(j,k);
        hess_f(i, j) = dtmp;
      }
      //for (j=0; j<i; j++)
      //  hess_f(j, i) = hess_f(i, j); // fill in symmetric values
    }
    if (snllLSqInstance->outputLevel > NORMAL_OUTPUT)
      Cout << "    nlf2_evaluator_gn results: objective fn. Hessian =\n"
	   << hess_f;
    result_mode |= OPTPP::NLPHessian;
  }
  Cout << std::endl;
}


void SNLLLeastSq::
constraint1_evaluator_gn(int mode, int n, const RealVector& x, RealVector& g,
			 RealMatrix& grad_g, int& result_mode)
{ 
  if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLLeastSq::constraint1_evaluator_gn called with mode = "
         << mode;

  // Set lsq_mode for ASV value for least squares terms
  short lsq_mode;
  switch (mode) {
  case 0: case 1: case 3: // no modification needed
    lsq_mode = mode;
    break;
  case 2: // local_fn_vals & local_fn_grads both needed
    lsq_mode = 3;
    break;
  default:
    Cerr << "Bad mode input from OPT++ in nlf2_evaluator_gn." << std::endl;
    abort_handler(-1);
  }

  // set model variables and asv prior to evaluate()
  if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLLeastSq::constraint1_evaluator_gn vars = \n"
         << x;
  snllLSqInstance->iteratedModel.continuous_variables(x);

  size_t i;
  ShortArray local_asv(snllLSqInstance->numFunctions, lsq_mode);
  for (i=snllLSqInstance->numLeastSqTerms; i<snllLSqInstance->numFunctions; i++)
    local_asv[i] = mode; // nonlinear constraints
  snllLSqInstance->activeSet.request_vector(local_asv);
  snllLSqInstance->iteratedModel.evaluate(snllLSqInstance->activeSet);
  lastFnEvalLocn = CON_EVALUATOR;
  lastEvalMode   = lsq_mode;
  lastEvalVars   = x;

  const Response& local_response
    = snllLSqInstance->iteratedModel.current_response();
  if (mode & 1) { // 1st bit is present, mode = 1, 3, 5, or 7
    snllLSqInstance->copy_con_vals_dak_to_optpp(
      local_response.function_values(), g, snllLSqInstance->numLeastSqTerms);
    result_mode = OPTPP::NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2, 3, 6, or 7
    snllLSqInstance->copy_con_grad(local_response.function_gradients(), grad_g,
				   snllLSqInstance->numLeastSqTerms);
    result_mode |= OPTPP::NLPGradient;
  }
}


void SNLLLeastSq::
constraint2_evaluator_gn(int mode, int n, const RealVector& x, RealVector& g,
			 RealMatrix& grad_g,
			 OPTPP::OptppArray<RealSymMatrix>& hess_g,
			 int& result_mode)
{ 
  if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLLeastSq::constraint2_evaluator_gn called with mode = "
         << mode;

  // See nlf2_evaluator() for mode override logic.
  if (mode != 7 && modeOverrideFlag) {
    mode = 7;
    if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
      Cout << "\nEvaluation mode overridden to include full mode = 7 data set.";
  }

  // Set lsq_mode for ASV value for least squares terms
  short lsq_mode;
  switch (mode) {
  case 0: case 1: case 3: // no modification needed
    lsq_mode = mode;
    break;
  case 2: case 5: case 6: case 7: // local_fn_vals & local_fn_grads both needed
    lsq_mode = 3;
    break;
  case 4: // only local_fn_grads needed in hess_f computation
    lsq_mode = 2;
    break;
  default:
    Cerr << "Bad mode input from OPT++ in nlf2_evaluator_gn." << std::endl;
    abort_handler(-1);
  }

  // set model variables and asv prior to evaluate()
  if (snllLSqInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLLeastSq::constraint2_evaluator_gn vars = \n"
         << x;
  snllLSqInstance->iteratedModel.continuous_variables(x);

  size_t i;
  ShortArray local_asv(snllLSqInstance->numFunctions, lsq_mode);
  for (i=snllLSqInstance->numLeastSqTerms; i<snllLSqInstance->numFunctions; i++)
    local_asv[i] = mode; // nonlinear constraints
  snllLSqInstance->activeSet.request_vector(local_asv);
  snllLSqInstance->iteratedModel.evaluate(snllLSqInstance->activeSet);
  lastFnEvalLocn = CON_EVALUATOR;
  lastEvalMode   = lsq_mode;
  lastEvalVars   = x;

  const Response& local_response
    = snllLSqInstance->iteratedModel.current_response();
  if (mode & 1) { // 1st bit is present, mode = 1, 3, 5, or 7
    snllLSqInstance->
      copy_con_vals_dak_to_optpp(local_response.function_values(), g,
				 snllLSqInstance->numLeastSqTerms);
    result_mode = OPTPP::NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2, 3, 6, or 7
    snllLSqInstance->copy_con_grad(local_response.function_gradients(), grad_g,
				   snllLSqInstance->numLeastSqTerms);
    result_mode |= OPTPP::NLPGradient;
  }
  if (mode & 4) { // 3rd bit is present, mode >= 4
    snllLSqInstance->copy_con_hess(local_response.function_hessians(), hess_g,
				   snllLSqInstance->numLeastSqTerms);
    result_mode |= OPTPP::NLPHessian;
  }
}


void SNLLLeastSq::initialize_run()
{
  LeastSq::initialize_run();

  // set the object instance pointer for use within the static member fns
  prevSnllLSqInstance = snllLSqInstance;
  optLSqInstance      = snllLSqInstance = this;

  // convenience function from SNLLBase
  snll_initialize_run(nlfObjective, nlpConstraint,
		      iteratedModel.continuous_variables(), 
		      boundConstraintFlag, 
		      iteratedModel.continuous_lower_bounds(),
		      iteratedModel.continuous_upper_bounds(),
		      iteratedModel.linear_ineq_constraint_coeffs(),
		      iteratedModel.linear_ineq_constraint_lower_bounds(),
		      iteratedModel.linear_ineq_constraint_upper_bounds(),
		      iteratedModel.linear_eq_constraint_coeffs(),
		      iteratedModel.linear_eq_constraint_targets(),
		      iteratedModel.nonlinear_ineq_constraint_lower_bounds(),
		      iteratedModel.nonlinear_ineq_constraint_upper_bounds(),
		      iteratedModel.nonlinear_eq_constraint_targets());

  // set modeOverrideFlag based on method/search strategy, speculative 
  // gradient, or constant asv selections.  Notes:
  // > GNewton override is always desirable, and mode override now works
  //   for constraint2 evaluators (previously, OptppArray of Hessian matrices
  //   was not sized properly if mode did not contain 4).
  // > Without mode override, _all_ NIPS methods are inconsistent in their
  //   objective and constraint evaluator calls, which previously led to bad
  //   data when the objective evaluator assumed that the constraint evaluator
  //   was called previously with the same mode (more checks are now in place).
  //if ( speculativeFlag || constantASVFlag || numNonlinearConstraints ||
  //     methodName == OPTPP_G_NEWTON )
  modeOverrideFlag = true;
}


void SNLLLeastSq::core_run()
{
  theOptimizer->optimize();

  // BMA NOTE: casting away the constness as done historically in DakotaString  
  String status("Solution from Opt++");
  char* nonconst_status = (char *) status.c_str();
  theOptimizer->printStatus(nonconst_status);

  // Retrieve the best design point and corresponding response data for use at
  // the strategy level.

  // best variables is updated using a convenience function from SNLLBase
  // Always populate bestVariables; DakotaLeastSq will unscale if needed
  snll_post_run(nlfObjective);

  // Update best response to contain the final lsq terms.  Since OPT++
  // has no knowledge of these terms, the OPT++ final design variables
  // must be matched to final lsq terms using data_pairs.find().  This
  // lookup is handled by DakotaLeastSq::post_run()
  retrievedIterPriFns = false;

  // Always populate bestResponse with constraints; DakotaLeastSq will
  // unscale if needed.
  // This must use an offset of the # user/native functions since the
  // bestResponseArray is in original space

  // OPT++ expects nonlinear equations followed by nonlinear inequalities.
  // Therefore, reorder the constraint values, and store them.
  if (numNonlinearConstraints) {
    RealVector best_fns = bestResponseArray.front().function_values_view();
    copy_con_vals_optpp_to_dak(nlfObjective->getConstraintValue(), best_fns,
			       numUserPrimaryFns);
  }
}


void SNLLLeastSq::finalize_run()
{
  reset(); // MSE: PDH supports trying to move this up into pre_run() ...

  // restore in case of recursion
  optLSqInstance  = prevMinInstance;
  snllLSqInstance = prevSnllLSqInstance;

  LeastSq::finalize_run();
}


void SNLLLeastSq::reset()
{
  // reset in case of recursion
  theOptimizer->reset();

  // clear constraints
  snll_finalize_run(nlfObjective);

  // reset last{FnEvalLocn,EvalMode,EvalVars}
  reset_base();
}

} // namespace Dakota
