/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SNLLOptimizer
//- Description: Implementation code for the SNLLOptimizer class
//- Owner:       Pam Williams
//- Checked by:
//- Change Log:  

#include "DakotaModel.H"
#include "SNLLOptimizer.H"
#include "ProblemDescDB.H"
#include "OptNewton.h"
#include "OptQNewton.h"
#include "OptFDNewton.h"
#include "OptNewtonLike.h"
#include "OptBCNewton.h"
#include "OptBCQNewton.h"
#include "OptBCFDNewton.h"
#include "OptBCNewtonLike.h"
#include "OptNIPS.h"
#include "OptQNIPS.h"
#include "OptFDNIPS.h"
#include "OptNIPSLike.h"
#include "OptCG.h"
#include "OptPDS.h"
#include "OptLBFGS.h"
#include "NLF.h"
#include "NLP.h"
#include "OptppArray.h"

using namespace OPTPP;
using std::endl;

static const char rcsId[]="@(#) $Id: SNLLOptimizer.C 7029 2010-10-22 00:17:02Z mseldre $";


namespace Dakota {

SNLLOptimizer* SNLLOptimizer::snllOptInstance(NULL);

/// a (perhaps arbitrary) definition of large scale; choose a
/// large-scale algorithm if numVars >= LARGE_SCALE
const int LARGE_SCALE = 100;


/** This constructor is used for normal instantiations using data from
    the ProblemDescDB. */
SNLLOptimizer::SNLLOptimizer(Model& model): Optimizer(model), SNLLBase(model),
  nlfObjective(NULL), nlfConstraint(NULL), nlpConstraint(NULL),
  theOptimizer(NULL), setUpType("model")
{
  // convenience function from SNLLBase
  snll_pre_instantiate(boundConstraintFlag, numConstraints); // from SNLLBase

  // Instantiate NLF & Optimizer objects based on method & gradient selections

  // Parallel Direct Search (Dennis & Torczon from Rice Univ.)
  if (methodName == "optpp_pds") {
    // ************************************************************************
    // NOTE: the parallelism of PDS is peer partition, not master-slave/asynch.
    // That is, PDS cannot use a simple launch/synchronize protocol like most 
    // other optimizers in DAKOTA.  Rather it will be necessary to implement
    // a communicator partition for the PDS processors (with hidden layers of
    // underlying parallelism) as was done for PICO.
    // ************************************************************************
    if (numConstraints) {
      Cerr << "Error: optpp_pds does not support linear or nonlinear "
           << "constraints.\n       Please select a different method for "
           << "generally constrained problems." << endl;
      abort_handler(-1);
    }
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptPDS optimizer with NLF0 evaluator.\n";
    nlf0 = new NLF0(numContinuousVars, nlf0_evaluator, init_fn);
    nlfObjective = nlf0;
    optpds = new OptPDS(nlf0);
    int search_scheme_size
      = probDescDB.get_int("method.optpp.search_scheme_size");
    maxConcurrency *= search_scheme_size;
    // The following is not performed in the Optimizer constructor since
    // maxConcurrency is updated above. The matching free_communicators()
    // appears in the Optimizer destructor.
    if (minimizerRecast)
      iteratedModel.init_communicators(maxConcurrency);
    optpds->setSSS(search_scheme_size); 
    theOptimizer = optpds;
  }

  // Polak-Ribiere Conjugate Gradient (CG)
  else if (methodName == "optpp_cg") {
    if (numConstraints || boundConstraintFlag) {
      Cerr << "Error: optpp_cg does not support bound, linear, or nonlinear "
           << "constraints.\n       Please select a different method for "
           << "constrained problems." << endl;
      abort_handler(-1);
    }
    if (vendorNumericalGradFlag) {
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptCG optimizer with FDNLF1 evaluator.\n";
      fdnlf1 = new FDNLF1(numContinuousVars, nlf0_evaluator, init_fn);
      nlfObjective = fdnlf1;
      optcg = new OptCG(fdnlf1);
    }
    else {
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptCG optimizer with NLF1 evaluator.\n";
      nlf1 = new NLF1(numContinuousVars, nlf1_evaluator, init_fn);
      nlfObjective = nlf1;
      optcg = new OptCG(nlf1);
    }
    theOptimizer = optcg;
  }

  // quasi-Newton: unconstrained, bound-constrained, & nonlinear interior-point
  else if (methodName == "optpp_q_newton") {
    if (vendorNumericalGradFlag) {
      fdnlf1 = new FDNLF1(numContinuousVars, nlf0_evaluator, init_fn);
      nlfObjective = fdnlf1;
      if (numConstraints) { // nonlinear interior-point
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptQNIPS optimizer with FDNLF1 evaluator.\n";
        optqnips  = new OptQNIPS(fdnlf1);
        fdnlf1Con = new FDNLF1(numContinuousVars, numNonlinearConstraints,
                               constraint0_evaluator, init_fn);
        nlfConstraint = fdnlf1Con;
        nlpConstraint = new NLP(fdnlf1Con);
      }
      else if (boundConstraintFlag) { // bound-constrained
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptBCQNewton optimizer with FDNLF1 evaluator."
               << '\n';
        optbcqnewton = new OptBCQNewton(fdnlf1);
      }
      else { // unconstrained
        if(numContinuousVars < LARGE_SCALE){
          if (outputLevel == DEBUG_OUTPUT)
            Cout << "Instantiating OptQNewton optimizer with FDNLF1 evaluator."
               << '\n';
          optqnewton = new OptQNewton(fdnlf1);
        }
        else{
          if (outputLevel == DEBUG_OUTPUT)
            Cout << "Instantiating OptLBFGS optimizer with FDNLF1 evaluator.\n";
          optlbfgs = new OptLBFGS(fdnlf1);
        }
      }
    }
    else {
      nlf1 = new NLF1(numContinuousVars, nlf1_evaluator, init_fn);
      nlfObjective = nlf1;
      if (numConstraints) { // nonlinear interior-point
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptQNIPS optimizer with NLF1 evaluator.\n";
        optqnips = new OptQNIPS(nlf1);
        nlf1Con  = new NLF1(numContinuousVars, numNonlinearConstraints,
                            constraint1_evaluator, init_fn);
        nlfConstraint = nlf1Con;
        nlpConstraint = new NLP(nlf1Con);
      }
      else if (boundConstraintFlag) { // bound-constrained
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptBCQNewton optimizer with NLF1 evaluator.\n";
        optbcqnewton = new OptBCQNewton(nlf1);
      }
      else { // unconstrained
        if(numContinuousVars < LARGE_SCALE){
          if (outputLevel == DEBUG_OUTPUT)
            Cout << "Instantiating OptQNewton optimizer with NLF1 evaluator.\n";
          optqnewton = new OptQNewton(nlf1);
        }
        else{
          if (outputLevel == DEBUG_OUTPUT)
            Cout << "Instantiating OptLBFGS optimizer with NLF1 evaluator.\n";
          optlbfgs = new OptLBFGS(nlf1);
        }
      }
    }

    if (numConstraints) { // nonlinear interior-point
      theOptimizer = optqnips;
      //optqnips->setSearchStrategy(searchStrat);//search strategy not supported
      optqnips->setMeritFcn(meritFn);
      optqnips->setStepLengthToBdry(stepLenToBndry);
      optqnips->setCenteringParameter(centeringParam);
    }
    else if (boundConstraintFlag) { // bound-constrained
      theOptimizer = optbcqnewton;
      optbcqnewton->setSearchStrategy(searchStrat);
      if (searchStrat == TrustRegion) optbcqnewton->setTRSize(maxStep);
    }
    else { // unconstrained
      if(numContinuousVars < LARGE_SCALE){
        theOptimizer = optqnewton;
        optqnewton->setSearchStrategy(searchStrat);
        if (searchStrat == TrustRegion) optqnewton->setTRSize(maxStep);
      }
      else{
        theOptimizer = optlbfgs;
        //optlbfgs->setSearchStrategy(searchStrat);//search strat not supported
      }
    }
  }

  // finite-difference Newton: unconstrained, bound-constrained, & nonlinear
  // interior-point
  else if (methodName == "optpp_fd_newton" ) {
    if (vendorNumericalGradFlag) {
      fdnlf1 = new FDNLF1(numContinuousVars, nlf0_evaluator, init_fn);
      nlfObjective = fdnlf1;
      if (numConstraints) { // nonlinear interior-point
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptFDNIPS optimizer with FDNLF1 evaluator.\n";
        optfdnips = new OptFDNIPS(fdnlf1);
        fdnlf1Con = new FDNLF1(numContinuousVars, numNonlinearConstraints,
                               constraint0_evaluator, init_fn);
        nlfConstraint = fdnlf1Con;
        nlpConstraint = new NLP(fdnlf1Con);
      }
      else if (boundConstraintFlag) { // bound-constrained
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptBCFDNewton optimizer with FDNLF1 evaluator."
               << '\n';
        optbcfdnewton = new OptBCFDNewton(fdnlf1);
      }
      else { // unconstrained
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptFDNewton optimizer with FDNLF1 evaluator."
               << '\n';
        optfdnewton = new OptFDNewton(fdnlf1);
      }
    }
    else {
      nlf1 = new NLF1(numContinuousVars, nlf1_evaluator, init_fn);
      nlfObjective = nlf1;
      if (numConstraints) { // nonlinear interior-point
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptFDNIPS optimizer with NLF1 evaluator.\n";
        optfdnips = new OptFDNIPS(nlf1);
        nlf1Con = new NLF1(numContinuousVars, numNonlinearConstraints,
                           constraint1_evaluator, init_fn);
        nlfConstraint = nlf1Con;
        nlpConstraint = new NLP(nlf1Con);
      }
      else if (boundConstraintFlag) { // bound-constrained
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptBCFDNewton optimizer with NLF1 evaluator."
               << '\n';
        optbcfdnewton = new OptBCFDNewton(nlf1);
      }
      else { // unconstrained
        if (outputLevel == DEBUG_OUTPUT)
          Cout << "Instantiating OptFDNewton optimizer with NLF1 evaluator.\n";
        optfdnewton = new OptFDNewton(nlf1);
      }
    }

    if (numConstraints) { // nonlinear interior-point
      theOptimizer = optfdnips;
      //optfdnips->setSearchStrategy(searchStrat);// search strat. not supported
      optfdnips->setMeritFcn(meritFn);
      optfdnips->setStepLengthToBdry(stepLenToBndry);
      optfdnips->setCenteringParameter(centeringParam);
    }
    else if (boundConstraintFlag) { // bound-constrained
      theOptimizer = optbcfdnewton;
      optbcfdnewton->setSearchStrategy(searchStrat);
      if (searchStrat == TrustRegion) optbcfdnewton->setTRSize(maxStep);
    }
    else { // unconstrained
      theOptimizer = optfdnewton;
      optfdnewton->setSearchStrategy(searchStrat);  
      if (searchStrat == TrustRegion) optfdnewton->setTRSize(maxStep);
    }
  }

  // full Newton: unconstrained, bound-constrained, & nonlinear interior-point
  else if (methodName == "optpp_newton") {
    nlf2 = new NLF2(numContinuousVars, nlf2_evaluator, init_fn);
    nlfObjective = nlf2;
    if (numConstraints) { // nonlinear interior-point
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptNIPS optimizer with NLF2 evaluator.\n";
      optnips = new OptNIPS(nlf2);
      //optnips->setSearchStrategy(searchStrat);// search strategy not supported
      optnips->setMeritFcn(meritFn);
      optnips->setStepLengthToBdry(stepLenToBndry);
      optnips->setCenteringParameter(centeringParam);
      theOptimizer = optnips;

      nlf2Con = new NLF2(numContinuousVars, numNonlinearConstraints,
                         constraint2_evaluator, init_fn);
      nlfConstraint = nlf2Con;
      nlpConstraint = new NLP(nlf2Con);
    }
    else if (boundConstraintFlag) { // bound-constrained
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptBCNewton optimizer with NLF2 evaluator.\n";
      optbcnewton = new OptBCNewton(nlf2);
      optbcnewton->setSearchStrategy(searchStrat);
      if (searchStrat == TrustRegion) optbcnewton->setTRSize(maxStep);
      theOptimizer = optbcnewton;
    }
    else { // unconstrained
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptNewton optimizer with NLF2 evaluator.\n";
      optnewton = new OptNewton(nlf2);
      optnewton->setSearchStrategy(searchStrat); 
      if (searchStrat == TrustRegion) optnewton->setTRSize(maxStep);
      theOptimizer = optnewton;
    }
  }

  else {
    Cerr << "Method name " << methodName << " currently unavailable within\n"
	 << "DAKOTA's SNLLOptimizer implementation of OPT++." << endl;
    abort_handler(-1);
  }

  // convenience function from SNLLBase
  snll_post_instantiate(numContinuousVars, vendorNumericalGradFlag,
			intervalType, fdGradStepSize, maxIterations,
			maxFunctionEvals, convergenceTol, 
			probDescDB.get_real("method.optpp.gradient_tolerance"), 
			maxStep, boundConstraintFlag, numConstraints,
			outputLevel, theOptimizer, nlfObjective, fdnlf1,
			fdnlf1Con);
}


/** This is an alternate constructor for instantiations on the fly
    using a Model but no ProblemDescDB. */
SNLLOptimizer::SNLLOptimizer(const String& method_name, Model& model):
  Optimizer(NoDBBaseConstructor(), model), // use default SNLLBase ctor
  nlfObjective(NULL), nlfConstraint(NULL), nlpConstraint(NULL),
  theOptimizer(NULL), setUpType("model")
{
  methodName = method_name; // TO DO: expand supported methods beyond QNewton

  // convenience function from SNLLBase: use defaults since no specification
  snll_pre_instantiate(boundConstraintFlag, numConstraints);

  // quasi-Newton: unconstrained, bound-constrained, & nonlinear interior-point
  nlf1 = new NLF1(numContinuousVars, nlf1_evaluator, init_fn);
  nlfObjective = nlf1;
  if (numConstraints) { // nonlinear interior-point
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptQNIPS optimizer with NLF1 evaluator.\n";
    optqnips = new OptQNIPS(nlf1);
    nlf1Con  = new NLF1(numContinuousVars, numNonlinearConstraints,
			constraint1_evaluator, init_fn);
    nlfConstraint = nlf1Con;
    nlpConstraint = new NLP(nlf1Con);
    theOptimizer = optqnips;
    //optqnips->setSearchStrategy(searchStrat); // search strategy not supported
    optqnips->setMeritFcn(meritFn); // ArgaezTapia
    optqnips->setStepLengthToBdry(0.99995);
    optqnips->setCenteringParameter(0.2);
  }
  else if (boundConstraintFlag) { // bound-constrained
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptBCQNewton optimizer with NLF1 evaluator.\n";
    optbcqnewton = new OptBCQNewton(nlf1);
    theOptimizer = optbcqnewton;
    optbcqnewton->setSearchStrategy(searchStrat);
    if (searchStrat == TrustRegion) optbcqnewton->setTRSize(1000.);
  }
  else { // unconstrained
    if(numContinuousVars < LARGE_SCALE){
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptQNewton optimizer with NLF1 evaluator.\n";
      optqnewton = new OptQNewton(nlf1);
      theOptimizer = optqnewton;
      optqnewton->setSearchStrategy(searchStrat);
      if (searchStrat == TrustRegion) optqnewton->setTRSize(1000.);
    }
    else{
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptLBFGS optimizer with NLF1 evaluator.\n";
      optlbfgs = new OptLBFGS(nlf1);
      theOptimizer = optlbfgs;
      //optlbfgs->setSearchStrategy(searchStrat);//search strategy not supported
    }
  }

  // convenience function from SNLLBase: use defaults since no specification
  snll_post_instantiate(numContinuousVars, vendorNumericalGradFlag,
			intervalType, fdGradStepSize, maxIterations,
			maxFunctionEvals, convergenceTol, 1.e-4, 1000.,
			boundConstraintFlag, numConstraints, outputLevel,
			theOptimizer, nlfObjective, NULL, NULL);
}


/** This is an alternate constructor for performing an optimization using
    the passed in objective function and constraint function pointers. */
SNLLOptimizer::SNLLOptimizer(const RealVector& initial_pt, 
  const RealVector& var_l_bnds, const RealVector& var_u_bnds,
  const RealMatrix& lin_ineq_coeffs,
  const RealVector& lin_ineq_l_bnds,
  const RealVector& lin_ineq_u_bnds, const RealMatrix& lin_eq_coeffs,
  const RealVector& lin_eq_tgts, const RealVector& nln_ineq_l_bnds,
  const RealVector& nln_ineq_u_bnds, const RealVector& nln_eq_tgts, 
  void (*user_obj_eval) (int mode, int n, const RealVector& x,
			 double& f, RealVector& grad_f,
			 int& result_mode),
  void (*user_con_eval) (int mode, int n, const RealVector& x, 
			 RealVector& g, RealMatrix& grad_g,
			 int& result_mode) ): // use default SNLLBase ctor
  Optimizer(NoDBBaseConstructor(), initial_pt.length(), 0, 0,
	    lin_ineq_coeffs.numRows(), lin_eq_coeffs.numRows(),
	    nln_ineq_l_bnds.length(), nln_eq_tgts.length()),
  nlfObjective(NULL), nlfConstraint(NULL), nlpConstraint(NULL),
  theOptimizer(NULL), setUpType("user_functions"), initialPoint(initial_pt),
  lowerBounds(var_l_bnds), upperBounds(var_u_bnds)
{
  for (size_t i=0; i<numContinuousVars; i++)
    if (lowerBounds[i] > -bigRealBoundSize ||
        upperBounds[i] <  bigRealBoundSize) {
      boundConstraintFlag = true;
      break;
    }

  // convenience function from SNLLBase: use defaults since no specification
  snll_pre_instantiate(boundConstraintFlag, numConstraints);

  // quasi-Newton: unconstrained, bound-constrained, & nonlinear interior-point
  nlf1 = new NLF1(numContinuousVars, user_obj_eval, init_fn);
  nlfObjective = nlf1;
  if (numConstraints) { // nonlinear interior-point
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptQNIPS optimizer with NLF1 evaluator.\n";
    optqnips = new OptQNIPS(nlf1);
    nlf1Con  = new NLF1(numContinuousVars, numNonlinearConstraints,
			user_con_eval, init_fn);
    nlfConstraint = nlf1Con;
    nlpConstraint = new NLP(nlf1Con);
    theOptimizer = optqnips;
    //optqnips->setSearchStrategy(searchStrat); // search strategy not supported
    optqnips->setMeritFcn(meritFn); // ArgaezTapia
    optqnips->setStepLengthToBdry(0.99995);
    optqnips->setCenteringParameter(0.2);
  }
  else if (boundConstraintFlag) { // bound-constrained
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Instantiating OptBCQNewton optimizer with NLF1 evaluator.\n";
    optbcqnewton = new OptBCQNewton(nlf1);
    theOptimizer = optbcqnewton;
    optbcqnewton->setSearchStrategy(searchStrat);
    if (searchStrat == TrustRegion) optbcqnewton->setTRSize(1000.);
  }
  else { // unconstrained
    if(numContinuousVars < LARGE_SCALE){
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptQNewton optimizer with NLF1 evaluator.\n";
      optqnewton = new OptQNewton(nlf1);
      theOptimizer = optqnewton;
      optqnewton->setSearchStrategy(searchStrat);
      if (searchStrat == TrustRegion) optqnewton->setTRSize(1000.);
    }
    else{
      if (outputLevel == DEBUG_OUTPUT)
        Cout << "Instantiating OptLBFGS optimizer with NLF1 evaluator.\n";
      optlbfgs = new OptLBFGS(nlf1);
      theOptimizer = optlbfgs;
      //optlbfgs->setSearchStrategy(searchStrat);//search strategy not supported
    }
  }

  // convenience function from SNLLBase: use defaults since no specification
  snll_post_instantiate(numContinuousVars, false, "", 0., 100, 1000, 1.e-4,
			1.e-4, 1000., boundConstraintFlag, numConstraints,
			outputLevel, theOptimizer, nlfObjective, NULL, NULL);

  // this can be called from the ctor (avoids caching of constraint arrays
  // within the class) since no Model updates need to be captured
  snll_initialize_run(nlfObjective, nlpConstraint, initialPoint,
		      boundConstraintFlag, lowerBounds, upperBounds,
		      lin_ineq_coeffs, lin_ineq_l_bnds, lin_ineq_u_bnds,
		      lin_eq_coeffs, lin_eq_tgts, nln_ineq_l_bnds,
		      nln_ineq_u_bnds, nln_eq_tgts);
}


SNLLOptimizer::~SNLLOptimizer()
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


/** For use when DAKOTA computes f and gradients are not directly
    available.  This is used by nongradient-based optimizers such as
    PDS and by gradient-based optimizers in vendor numerical gradient
    mode (opt++'s internal finite difference routine is used). */
void SNLLOptimizer::
nlf0_evaluator(int n, const RealVector& x, double& f, int& result_mode)
{
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::nlf0_evaluator called with mode = 1";

  // Emulates NPSOLOptimizer::objective_eval() when nonlinear constraints are
  // present.  Unlike NPSOL, verify that vars are consistent since OPT++ does
  // not always have a 1-to-1 correspondence in evaluator calls.

  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::nlf0_evaluator vars = \n" << x;
  if (!snllOptInstance->numNonlinearConstraints ||
       lastFnEvalLocn != CONEvaluator || x != lastEvalVars) {
    // data not available from constraint0_evaluator() so perform
    // a new function evaluation.
    snllOptInstance->iteratedModel.continuous_variables(x);
    snllOptInstance->iteratedModel.compute_response();// default active set
    // Should constraints be evaluated (if present)?  Depends on what OPT++ is
    // doing.  Since we know this eval is not aligned with a preceding
    // constraint eval, assume for now that a matching constraint eval might
    // _follow_, such that evaluating the constraints now might save work
    // through duplication detection in the constraint evaluator.
    //ShortArray local_asv(snllOptInstance->numFunctions, mode);
    //if (snllOptInstance->numNonlinearConstraints) // turn off asv for constr's
    //  for (i=0; i<snllOptInstance->numNonlinearConstraints; i++)
    //    local_asv[i + snllOptInstance->numObjectiveFns] = 0;
    lastFnEvalLocn = NLFEvaluator;
  }

  const BoolDeque& max_sense
    = snllOptInstance->iteratedModel.primary_response_fn_sense();
  f = (!max_sense.empty() && max_sense[0]) ?
    -snllOptInstance->iteratedModel.current_response().function_value(0) :
     snllOptInstance->iteratedModel.current_response().function_value(0);
  result_mode = NLPFunction;
}


/** For use when DAKOTA computes f and df/dX (regardless of gradientType).
    Vendor numerical gradient case is handled by nlf0_evaluator. */
void SNLLOptimizer::
nlf1_evaluator(int mode, int n, const RealVector& x, double& f, 
               RealVector& grad_f, int& result_mode)
{
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::nlf1_evaluator called with mode = " << mode;

  // MODE OVERRIDE:
  // At the start of each cycle, OPT++ calls nlf1_evaluator successively with
  // mode=1 and then mode=2 so that the fn. and grad. tolerances can be checked
  // for convergence between data requests.  This requires 2 simulation calls
  // instead of 1 for each optimization cycle prior to convergence.  In the case
  // of expensive engineering simulations, this overhead can be avoided by
  // overriding mode and always returning a mode=3 data set.  On the last
  // optimization cycle, this reduction in simulation calls may come at the
  // expense of an unneeded gradient computation.  During the line/trust region
  // search, however, an OPT++ request for a fn evaluation does not always imply
  // that gradient information is also required.  In particular, OPT++ mode
  // requests have the following general patterns:
  //   P-R CG:                                          31212121212 *
  //   QNewton value-based line search & trust region:  31121211212
  //   QNewton gradient-based line search (exc. NIPS):  31212121212 *
  //   QNewton trust region PDS:                        311111112111111112
  //   FDNewton value-based line search & trust region: 311222122211222
  //   FDNewton gradient-based line search:             31212221222121222
  //   FDNewton trust region PDS:                       3111111122211111111222
  //     * mode override desirable
  // Thus, for most cases, we need to be able to distinguish the search 
  // direction computation phase from the line/trust region search phase (not
  // currently available) in order to do this right.  Only for the cases of
  // CG, QNewton with gradient-based line search, speculative gradients, and
  // constant asv settings should we override the mode on every evaluator call
  // (the latter two will waste computations during the line/trust region search
  // for the sake of parallel load balance or interface simplicity).
  // modeOverrideFlag is set in find_optimum() according to this logic.

  // Emulates NPSOLOptimizer::objective_eval() when nonlinear constraints are
  // present.  Unlike NPSOL, verify that mode and vars are consistent since
  // OPT++ does not always have a 1-to-1 correspondence in evaluator calls.
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::nlf1_evaluator vars = \n" << x;

  if (!snllOptInstance->numNonlinearConstraints ||
      lastFnEvalLocn != CONEvaluator || mode != lastEvalMode ||
      x != lastEvalVars) {
    // data not available from constraint0_evaluator() so perform
    // a new function evaluation.
    snllOptInstance->iteratedModel.continuous_variables(x);

    // Should constraints be evaluated (if present)?  Depends on what OPT++ is
    // doing.  Since we know this eval is not aligned with a preceding
    // constraint eval, assume for now that a matching constraint eval might
    // _follow_, such that evaluating the constraints now might save work
    // through duplication detection in the constraint evaluator.
    //ShortArray local_asv(snllOptInstance->numFunctions, mode);
    //if (snllOptInstance->numNonlinearConstraints) // turn off asv for constr's
    //  for (i=0; i<snllOptInstance->numNonlinearConstraints; i++)
    //    local_asv[i + snllOptInstance->numObjectiveFns] = 0;
    snllOptInstance->activeSet.request_values(mode);
    snllOptInstance->
      iteratedModel.compute_response(snllOptInstance->activeSet);
    lastFnEvalLocn = NLFEvaluator;
  }

  const Response& local_response
    = snllOptInstance->iteratedModel.current_response();
  const BoolDeque& max_sense
    = snllOptInstance->iteratedModel.primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);

  if (mode & 1) { // 1st bit is present, mode = 1 or 3
    f = (max_flag) ? -local_response.function_value(0) :
                      local_response.function_value(0);
    result_mode = NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2 or 3
    grad_f = local_response.function_gradient_copy(0);
    if (max_flag) grad_f *= -1.;
    result_mode |= NLPGradient;
  }
}


/** For use when DAKOTA receives f, df/dX, & d^2f/dx^2 from the
    ApplicationInterface (analytic only). Finite differencing does not
    make sense for a full Newton approach, since lack of analytic
    gradients & Hessian should dictate the use of quasi-newton or
    fd-newton.  Thus, there is no fdnlf2_evaluator for use with full
    Newton approaches, since it is preferable to use quasi-newton or
    fd-newton with nlf1.  Gauss-Newton does not fit this model; it uses
    nlf2_evaluator_gn instead of nlf2_evaluator. */
void SNLLOptimizer::
nlf2_evaluator(int mode, int n, const RealVector& x, double& f, 
               RealVector& grad_f, RealSymMatrix& hess_f, int& result_mode)
{
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::nlf2_evaluator called with mode = " << mode;

  // MODE OVERRIDE:
  // At the start of each cycle, OPT++ calls nlf2_evaluator successively with
  // mode=1, 2, and then 4 so that the fn. and grad. tolerances can be checked
  // for convergence between data requests.  This requires 3 simulation calls
  // instead of 1 for each optimization cycle prior to convergence.  In the case
  // of expensive engineering simulations, this overhead can be avoided by
  // overriding mode and always returning the full mode=7 data set.  On the last
  // optimization cycle, this reduction in simulation calls may come at the
  // expense of unneeded gradient/Hessian computations.  During the line/trust
  // region search, however, an OPT++ request for a fn evaluation does not
  // always imply that gradient/Hessian information is also required.  In
  // particular, OPT++ mode requests have the following general patterns, where
  // the Hessian is only evaluated at the start of a new cycle:
  //   Newton value-based line search & trust region: 7112411241124
  //   Newton gradient-based line search:             7121241212412124
  //   Newton trust region PDS:                       71111111241111111124
  // Thus, to do this right:
  //   Newton value-based line search & trust region: 7171717
  //   Newton gradient-based line search:             7373737
  //   Newton trust region PDS:                       7111111711111117
  // we need to be able to distinguish the search direction computation phase
  // from the line/trust region search phase (not currently available).  Only
  // in the cases of speculative gradients and constant asv selections should
  // we override the mode on every evaluator call, since this will waste
  // computations during the line/trust region search (for the sake of parallel
  // load balance or interface simplicity).  modeOverrideFlag is set in
  // find_optimum() according to this logic.

  // Emulates NPSOLOptimizer::objective_eval() when nonlinear constraints are
  // present.  Unlike NPSOL, verify that mode and vars are consistent since
  // OPT++ does not always have a 1-to-1 correspondence in evaluator calls.

  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::nlf2_evaluator vars = \n" << x;
  if (!snllOptInstance->numNonlinearConstraints ||
      lastFnEvalLocn != CONEvaluator || mode != lastEvalMode ||
      x != lastEvalVars) {
    // data not available from constraint0_evaluator() so perform
    // a new function evaluation.
    snllOptInstance->iteratedModel.continuous_variables(x);
    // Should constraints be evaluated (if present)?  Depends on what OPT++ is
    // doing.  Since we know this eval is not aligned with a preceding
    // constraint eval, assume for now that a matching constraint eval might
    // _follow_, such that evaluating the constraints now might save work
    // through duplication detection in the constraint evaluator.
    //ShortArray local_asv(snllOptInstance->numFunctions, mode);
    //if (snllOptInstance->numNonlinearConstraints) // turn off asv for constr's
    //  for (i=0; i<snllOptInstance->numNonlinearConstraints; i++)
    //    local_asv[i + snllOptInstance->numObjectiveFns] = 0;
    snllOptInstance->activeSet.request_values(mode);
    snllOptInstance->
      iteratedModel.compute_response(snllOptInstance->activeSet);
    lastFnEvalLocn = NLFEvaluator;
  }

  const Response& local_response
    = snllOptInstance->iteratedModel.current_response();
  const BoolDeque& max_sense
    = snllOptInstance->iteratedModel.primary_response_fn_sense();
  bool max_flag = (!max_sense.empty() && max_sense[0]);

  if (mode & 1) { // 1st bit is present, mode = 1, 3, 5, or 7
    f = (max_flag) ? -local_response.function_value(0) :
                      local_response.function_value(0);
    result_mode = NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2, 3, 6, or 7
    grad_f = local_response.function_gradient_copy(0);
    if (max_flag) grad_f *= -1.;
    result_mode |= NLPGradient;
  }
  if (mode & 4) { // 3rd bit is present, mode >= 4
    hess_f = local_response.function_hessian(0);
    if (max_flag) hess_f *= -1.;
    result_mode |= NLPHessian;
  }
}


/** For use when DAKOTA computes g and gradients are not directly
    available.  This is used by nongradient-based optimizers and by
    gradient-based optimizers in vendor numerical gradient mode
    (opt++'s internal finite difference routine is used). */
void SNLLOptimizer::
constraint0_evaluator(int n, const RealVector& x, RealVector& g,
		      int& result_mode)
{
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::constraint0_evaluator called with mode = 1";

  // set model variables prior to compute_response()
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::constraint0_evaluator vars = \n" << x;
  snllOptInstance->iteratedModel.continuous_variables(x);

  snllOptInstance->iteratedModel.compute_response(); // default active set
  lastFnEvalLocn = CONEvaluator;
  lastEvalVars   = x;

  snllOptInstance->copy_con_vals_dak_to_optpp(
    snllOptInstance->iteratedModel.current_response().function_values(), g,
    snllOptInstance->numObjectiveFns);
  result_mode = NLPFunction;
}


/** For use when DAKOTA computes g and dg/dX (regardless of gradientType).
    Vendor numerical gradient case is handled by constraint0_evaluator. */
void SNLLOptimizer::
constraint1_evaluator(int mode, int n, const RealVector& x, RealVector& g,
		      RealMatrix& grad_g, int& result_mode)
{ 
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::constraint1_evaluator called with mode = "
         << mode;

  // set model variables and active set prior to compute_response()
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::constraint1_evaluator vars = \n" << x;
  snllOptInstance->iteratedModel.continuous_variables(x);

  snllOptInstance->activeSet.request_values(mode);
  snllOptInstance->
    iteratedModel.compute_response(snllOptInstance->activeSet);
  lastFnEvalLocn = CONEvaluator;
  lastEvalMode   = mode;
  lastEvalVars   = x;

  const Response& local_response
    = snllOptInstance->iteratedModel.current_response();
  if (mode & 1) { // 1st bit is present, mode = 1 or 3
    snllOptInstance->
      copy_con_vals_dak_to_optpp(local_response.function_values(), g,
				 snllOptInstance->numObjectiveFns);
    result_mode = NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2 or 3
    snllOptInstance->copy_con_grad(local_response.function_gradients(), grad_g,
				   snllOptInstance->numObjectiveFns);
    result_mode |= NLPGradient;
  }
}


/** For use when DAKOTA computes g, dg/dX, & d^2g/dx^2 (analytic only). */
void SNLLOptimizer::
constraint2_evaluator(int mode, int n, const RealVector& x, RealVector& g,
		      RealMatrix& grad_g,
		      OPTPP::OptppArray<RealSymMatrix>& hess_g,
		      int& result_mode)
{ 
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::constraint2_evaluator called with mode = "
         << mode;

  // set model variables and active set prior to compute_response().
  if (snllOptInstance->outputLevel == DEBUG_OUTPUT)
    Cout << "\nSNLLOptimizer::constraint2_evaluator vars = \n" << x;
  snllOptInstance->iteratedModel.continuous_variables(x);

  snllOptInstance->activeSet.request_values(mode);
  snllOptInstance->
    iteratedModel.compute_response(snllOptInstance->activeSet);
  lastFnEvalLocn = CONEvaluator;
  lastEvalMode   = mode;
  lastEvalVars   = x;

  const Response& local_response
    = snllOptInstance->iteratedModel.current_response();
  if (mode & 1) { // 1st bit is present, mode = 1, 3, 5, or 7
    snllOptInstance->
      copy_con_vals_dak_to_optpp(local_response.function_values(), g,
				 snllOptInstance->numObjectiveFns);
    result_mode = NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2, 3, 6, or 7
    snllOptInstance->copy_con_grad(local_response.function_gradients(), grad_g,
				   snllOptInstance->numObjectiveFns);
    result_mode |= NLPGradient;
  }
  if (mode & 4) { // 3rd bit is present, mode >= 4
    snllOptInstance->copy_con_hess(local_response.function_hessians(), hess_g,
				   snllOptInstance->numObjectiveFns);
    result_mode |= NLPHessian;
  }
}


void SNLLOptimizer::initialize_run()
{
  Optimizer::initialize_run();

  // set the object instance pointer for use within the static member fns
  prevSnllOptInstance = snllOptInstance;
  optLSqInstance      = snllOptInstance = this;

  if (setUpType == "model") {

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
    // > Mode override is desirable for QNewton with gradient-based line search,
    //   but NIPS optimizers do not support search strategy specification.
    // > Without mode override, _all_ NIPS methods are inconsistent in their
    //   objective and constraint evaluator calls, which previously led to bad
    //   data when the objective evaluator assumed that the constraint evaluator
    //   was called previously w/ the same mode (more checks are now in place).
    // > For now, we will not activate the OPTPP mode override when 
    //   OPTPP computes the finite difference approximations, 
    //   because it results in an inordinate amount of duplicate function 
    //   evaluations.
    if ( ( speculativeFlag || constantASVFlag || numNonlinearConstraints ||
        methodName == "optpp_cg" || ( methodName == "optpp_q_newton" &&
        searchMethod == "gradient_based_line_search" && !numConstraints))
        && methodSource != "vendor" )
      nlfObjective->setModeOverride(true);
  }
}


void SNLLOptimizer::find_optimum()
{ theOptimizer->optimize(); }


void SNLLOptimizer::post_run(std::ostream& s)
{
  Cout << "********************************************************" <<  '\n';
  Cout << "             OPT++ TERMINATION CRITERION                " <<  '\n';
  if(theOptimizer->getReturnCode() > 0)
     Cout << "\t  SUCCESS - " << methodName << " converged to a solution" 
          << '\n';
  else
     Cout << "\t  FAILURE - " << methodName << "terminated         " << '\n';
  Cout <<  theOptimizer->getMesg() <<  '\n';
  Cout << "********************************************************" << '\n';
  theOptimizer->printStatus((char*)"Solution from Opt++");

  // Retrieve the best design point and corresponding response data for use at
  // the strategy level.

  // best variables is updated using a convenience function from SNLLBase
  snll_post_run(nlfObjective);

  // update best response to contain the final objectives/constraints
  if (!localObjectiveRecast) { // else local_objective_recast_retrieve()
                               // is used in Optimizer::post_run()
    RealVector best_fns(numFunctions);
    const BoolDeque& max_sense = iteratedModel.primary_response_fn_sense();
    // see opt++/libopt/nlp.h
    best_fns[0] = (!max_sense.empty() && max_sense[0]) ?
      -nlfObjective->getF() : nlfObjective->getF();
    // OPT++ expects nonlinear equations followed by nonlinear inequalities.
    // Therefore, reorder the constraint values.
    copy_con_vals_optpp_to_dak(nlfObjective->getConstraintValue(), best_fns, 1);
    bestResponseArray.front().function_values(best_fns);
  }

  // Best functions now contain objective with correct sense and
  // constraints; this will handle any unscaling:
  Optimizer::post_run(s);
}


void SNLLOptimizer::finalize_run()
{
  // reset in case of recursion
  theOptimizer->reset();

  // restore in case of recursion
  optLSqInstance  = prevMinInstance;
  snllOptInstance = prevSnllOptInstance;

  Optimizer::finalize_run();
}


} // namespace Dakota
