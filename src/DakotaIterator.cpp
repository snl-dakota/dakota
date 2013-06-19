/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Iterator
//- Description: Implementation code for the Iterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_data_io.hpp"
#include "DakotaIterator.hpp"
#include "ParamStudy.hpp"
#include "RichExtrapVerification.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDLocalReliability.hpp"
#include "NonDGlobalReliability.hpp"
#include "NonDLHSSampling.hpp"
#include "NonDIncremLHSSampling.hpp"
#include "NonDAdaptImpSampling.hpp"
#include "NonDGPImpSampling.hpp"
#include "NonDGlobalEvidence.hpp"
#include "NonDLocalEvidence.hpp"
#include "NonDLHSEvidence.hpp"
#include "NonDGlobalSingleInterval.hpp"
#include "NonDLocalSingleInterval.hpp"
#include "NonDLHSSingleInterval.hpp"
#include "NonDPOFDarts.hpp"
#include "SurrBasedLocalMinimizer.hpp"
#include "SurrBasedGlobalMinimizer.hpp"
#include "EffGlobalMinimizer.hpp"
#include "NonlinearCGOptimizer.hpp"
#include "NIDR_initdefs.h"
#ifdef DAKOTA_DDACE
#include "DDACEDesignCompExp.hpp"
#endif
#ifdef HAVE_FSUDACE
#include "FSUDesignCompExp.hpp"
#endif
#ifdef HAVE_PSUADE
#include "PSUADEDesignCompExp.hpp"
#endif
#ifdef HAVE_DOT
#include "DOTOptimizer.hpp"
#endif
#ifdef HAVE_CONMIN
#include "CONMINOptimizer.hpp"
#endif
#ifdef DAKOTA_DL_SOLVER
#include "DLSolver.hpp"
#endif
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#include "NLSSOLLeastSq.hpp"
#endif
#ifdef HAVE_NLPQL
#include "NLPQLPOptimizer.hpp"
#endif
#ifdef HAVE_NL2SOL
#include "NL2SOLLeastSq.hpp"
#endif
//#ifdef DAKOTA_RSQP
//#include "rSQPOptimizer.H"
//#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#include "SNLLLeastSq.hpp"
#endif
#ifdef DAKOTA_COLINY
#include "COLINOptimizer.hpp"
#endif
#ifdef DAKOTA_HOPS
#include "APPSOptimizer.hpp"
#endif
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif
#ifdef HAVE_JEGA
#include "JEGAOptimizer.hpp"
#endif
#ifdef DAKOTA_GPMSA
#include "NonDGPMSABayesCalibration.hpp"
#endif
#ifdef DAKOTA_QUESO
#include "NonDQUESOBayesCalibration.hpp"
#endif
#ifdef HAVE_DREAM
#include "NonDDREAMBayesCalibration.hpp"
#endif
#ifdef HAVE_ADAPTIVE_SAMPLING 
#include "NonDAdaptiveSampling.hpp"
#endif
#ifdef HAVE_ESM
#include "EfficientSubspaceMethod.hpp"
#endif
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaGraphics.hpp"
#include "ResultsManager.hpp"

static const char rcsId[]="@(#) $Id: DakotaIterator.cpp 7029 2010-10-22 00:17:02Z mseldre $";

namespace Dakota {

extern Graphics dakota_graphics; // defined in ParallelLibrary.cpp
extern ProblemDescDB dummy_db;    // defined in dakota_global_defs.cpp
extern ResultsManager iterator_results_db;

/** This constructor builds the base class data for all inherited
    iterators.  get_iterator() instantiates a derived class and the
    derived class selects this base class constructor in its
    initialization list (to avoid the recursion of the base class
    constructor calling get_iterator() again).  Since the letter IS
    the representation, its representation pointer is set to NULL (an
    uninitialized pointer causes problems in ~Iterator). */
Iterator::Iterator(BaseConstructor, Model& model):
  probDescDB(model.problem_description_db()),
  methodName(probDescDB.get_string("method.algorithm")),
  convergenceTol(probDescDB.get_real("method.convergence_tolerance")),
  maxIterations(probDescDB.get_int("method.max_iterations")),
  maxFunctionEvals(probDescDB.get_int("method.max_function_evaluations")),
  maxConcurrency(model.derivative_concurrency()),
  numFunctions(model.num_functions()), numContinuousVars(model.cv()),
  numDiscreteIntVars(model.div()), numDiscreteRealVars(model.drv()),
  activeSet(model.current_response().active_set()), subIteratorFlag(false),
  gradientType(probDescDB.get_string("responses.gradient_type")),
  methodSource(probDescDB.get_string("responses.method_source")),
  intervalType(probDescDB.get_string("responses.interval_type")),
  hessianType(probDescDB.get_string("responses.hessian_type")),
  fdGradStepSize(fdGradStepDflt),
  fdGradStepType(probDescDB.get_string("responses.fd_gradient_step_type")),
  fdHessByGradStepSize(0.001), fdHessByFnStepSize(0.002), 
  fdHessStepType(probDescDB.get_string("responses.fd_hessian_step_type")),
  numFinalSolutions(probDescDB.get_sizet("method.final_solutions")),
  // Output verbosity is observed within Iterator (algorithm verbosity),
  // Model (synchronize/estimate_derivatives verbosity), Interface
  // (map/synch verbosity), Approximation (global data fit coefficient
  // reporting), and AnalysisCode (file operations verbosity) as follows:
  //               iterator     model     interface   approx    file ops
  //   "silent" :   silent      silent     silent     quiet      quiet
  //   "quiet"  :    quiet      quiet       quiet     quiet      quiet
  //   "normal" :   normal      normal     normal     quiet      quiet
  //   "verbose":   verbose     normal     verbose    verbose    verbose
  //   "debug"  :    debug      normal      debug     verbose    verbose
  // where "silent," "quiet", "verbose" and "debug" must be user specified and
  // "normal" is the default for no user specification.  Note that iterators
  // and interfaces have the most granularity in verbosity.
  outputLevel(probDescDB.get_short("method.output")), summaryOutputFlag(true),
  writePrecision(probDescDB.get_int("strategy.output_precision")),
  resultsDB(iterator_results_db),
  methodId(probDescDB.get_string("method.id")), iteratorRep(NULL),
  referenceCount(1)
{
  // Check for active variables
  if ( numContinuousVars   <= 0 && numDiscreteIntVars <= 0 &&
       numDiscreteRealVars <= 0 ) {
    Cerr << "\nError: no active variables available in selected method.  Please"
	 << "\n       select variables that are active for " << methodName
	 << "." << std::endl;
    abort_handler(-1);
  }
  // Check for response functions
  if ( numFunctions <= 0 ) {
    Cerr << "\nError: number of response functions must be greater than zero."
	 << std::endl;
    abort_handler(-1);
  }

  // Populate gradient/Hessian attributes for use within the iterator hierarchy.
  // Note: the fd step size arrays specialize by variable, whereas the mixed
  // grads/Hessians specialize by function.
  Cout << "methodName = " << methodName << "\ngradientType = "
       << gradientType << '\n';
  if (gradientType == "numerical") {
    if (methodSource == "vendor") {
      const RealVector& fdgss
	= probDescDB.get_rv("responses.fd_gradient_step_size");
      if (fdgss.length()) // else use default from initializer list
	fdGradStepSize = fdgss[0];
    }
    Cout << "Numerical gradients using " << intervalType
	 << " differences\nto be calculated by the " << methodSource
	 << " finite difference routine.\n";
  }
  else if (gradientType == "mixed") {
    // Vendor numerical is no good in mixed mode except maybe for NPSOL/NLSSOL.
    if (methodSource == "vendor") {
      Cerr << "Error: Mixed gradient specification not currently valid with "
           << "vendor numerical.\nSelect dakota as method_source instead."
	   << std::endl;
      abort_handler(-1);
    }
    const IntList& mixed_grad_analytic_ids
      = probDescDB.get_il("responses.gradients.mixed.id_analytic");
    const IntList& mixed_grad_numerical_ids
      = probDescDB.get_il("responses.gradients.mixed.id_numerical");
    Cout << "Mixed gradients: analytic gradients for functions { ";
    for (ILCIter cit=mixed_grad_analytic_ids.begin();
	 cit!=mixed_grad_analytic_ids.end(); cit++)
      Cout << *cit << ' ';
    Cout << "} and\nnumerical gradients for functions { ";
    for (ILCIter cit=mixed_grad_numerical_ids.begin();
	 cit!=mixed_grad_numerical_ids.end(); cit++)
      Cout << *cit << ' ';
    Cout << "} using " << intervalType << " differences\ncalculated by the "
         << methodSource << " routine.\n";
  }

  Cout << "hessianType = " << hessianType << '\n';
  if ( hessianType == "numerical" || ( hessianType == "mixed" &&
      !probDescDB.get_il("responses.hessians.mixed.id_numerical").empty() ) ) {
    const RealVector& fdhss
      = probDescDB.get_rv("responses.fd_hessian_step_size");
    if (fdhss.length()) // else use defaults from initializer list
      fdHessByGradStepSize = fdHessByFnStepSize = fdhss[0];
  }

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(BaseConstructor, model) called to "
       << "build letter base class\n";
#endif
}


/** This alternate constructor builds base class data for inherited
    iterators.  It is used for on-the-fly instantiations for which DB
    queries cannot be used.  Therefore it only sets attributes taken
    from the incoming model.  Since there are no iterator-specific
    redefinitions of maxIterations or numFinalSolutions in
    NoDBBaseConstructor mode, go ahead and assign default value for
    all iterators. */
Iterator::Iterator(NoDBBaseConstructor, Model& model):
  probDescDB(dummy_db), convergenceTol(0.0001), maxIterations(100),
  maxFunctionEvals(1000), maxConcurrency(model.derivative_concurrency()),
  numFunctions(model.num_functions()), numContinuousVars(model.cv()),
  numDiscreteIntVars(model.div()), numDiscreteRealVars(model.drv()),
  activeSet(model.current_response().active_set()), subIteratorFlag(false),
  gradientType(model.gradient_type()), methodSource(model.method_source()),
  intervalType(model.interval_type()), hessianType(model.hessian_type()),
  fdGradStepSize(0.001), fdGradStepType("relative"), 
  fdHessByGradStepSize(0.001), fdHessByFnStepSize(0.002),
  fdHessStepType("relative"), numFinalSolutions(1),
  outputLevel(NORMAL_OUTPUT), summaryOutputFlag(false),
  writePrecision(0),  resultsDB(iterator_results_db), methodId("NO_DB_METHOD"),
  iteratorRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(NoDBBaseConstructor, model) called "
       << "to build letter base class\n";
#endif
}


/** This alternate constructor builds base class data for inherited
    iterators.  It is used for on-the-fly instantiations for which DB
    queries cannot be used. It has no incoming model, so only sets up
    a minimal set of defaults. However, its use is preferable to the
    default constructor, which should remain as minimal as
    possible. Since there are no iterator-specific redefinitions of
    maxIterations or numFinalSolutions in NoDBBaseConstructor mode, go
    ahead and assign default value for all iterators.*/
Iterator::Iterator(NoDBBaseConstructor): probDescDB(dummy_db),
  convergenceTol(0.0001), maxIterations(100), maxFunctionEvals(1000),
  maxConcurrency(1), subIteratorFlag(false), gradientType("none"),
  hessianType("none"), fdGradStepSize(0.001), fdHessByGradStepSize(0.001),
  fdHessByFnStepSize(0.002), numFinalSolutions(1), outputLevel(NORMAL_OUTPUT),
  summaryOutputFlag(false), writePrecision(0), resultsDB(iterator_results_db),
  methodId("NO_DB_METHOD"), iteratorRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(NoDBBaseConstructor) called to build letter base "
       << "class\n";
#endif
}


/** The default constructor is used in Vector<Iterator>
    instantiations and for initialization of Iterator objects
    contained in Strategy derived classes (see derived class
    header files).  iteratorRep is NULL in this case (a populated
    problem_db is needed to build a meaningful Iterator object).
    This makes it necessary to check for NULL pointers in the copy
    constructor, assignment operator, and destructor. */
Iterator::Iterator(): probDescDB(dummy_db), maxConcurrency(1),
  resultsDB(iterator_results_db), iteratorRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator() called to build empty envelope "
       << "base class object." << std::endl;
#endif
}


/** Used in iterator instantiations within strategy constructors.
    Envelope constructor only needs to extract enough data to properly
    execute get_iterator(), since letter holds the actual base class data. */
Iterator::Iterator(Model& model):
  //iteratedModel(model), // no Model copy for envelope
  probDescDB(model.problem_description_db()),
  resultsDB(iterator_results_db),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(Model&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate iterator type
  iteratorRep = get_iterator(model);
  if ( !iteratorRep ) // bad name or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize iteratorRep to the
    appropriate derived type, as given by the methodName attribute. */
Iterator* Iterator::get_iterator(Model& model)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter: Getting iterator " << methodName
       << std::endl;
#endif

  // These instantiations will NOT recurse on the Iterator(model)
  // constructor due to the use of BaseConstructor.

  const String& method_name = probDescDB.get_string("method.algorithm");
  if (strends(method_name, "_parameter_study"))
    return new ParamStudy(model);
  else if (method_name == "richardson_extrap")
    return new RichExtrapVerification(model);
  else if (method_name == "nond_polynomial_chaos")
    return new NonDPolynomialChaos(model);
  else if (method_name == "nond_stoch_collocation")
    return new NonDStochCollocation(model);
  else if (method_name == "nond_sampling") {
    const String& sample_type = probDescDB.get_string("method.sample_type");
    if (strbegins(sample_type, "incremental"))
      return new NonDIncremLHSSampling(model);
    else
      return new NonDLHSSampling(model);
  }
  else if (method_name == "nond_importance_sampling")
    return new NonDAdaptImpSampling(model);
  else if (method_name == "nond_gpais")
    return new NonDGPImpSampling(model);
  else if (method_name == "surrogate_based_local")
    return new SurrBasedLocalMinimizer(model);
  else if (method_name == "surrogate_based_global")
    return new SurrBasedGlobalMinimizer(model);
  else if (method_name == "efficient_global")
    return new EffGlobalMinimizer(model);
  else if (method_name == "nond_global_reliability")
    return new NonDGlobalReliability(model);
  else if (method_name == "nond_local_reliability")
    return new NonDLocalReliability(model);
  else if (method_name == "nond_pof_darts")
    return new NonDPOFDarts(model);
  else if (method_name == "nond_global_evidence"){
    const String& nond_opt_alg
      = probDescDB.get_string("method.nond.optimization_algorithm");
    if (nond_opt_alg == "lhs")
      return new NonDLHSEvidence(model);
    else if (nond_opt_alg == "sbo" || nond_opt_alg == "ego" ||
	     nond_opt_alg == "ea"  || nond_opt_alg.empty())
      return new NonDGlobalEvidence(model);
    else
      return NULL;
  }
  else if (method_name == "nond_local_evidence")
    return new NonDLocalEvidence(model);
  else if (method_name == "nond_global_interval_est") {
    const String& nond_opt_alg
      = probDescDB.get_string("method.nond.optimization_algorithm");
    if (nond_opt_alg == "lhs")
      return new NonDLHSSingleInterval(model);
    else if (nond_opt_alg == "sbo" || nond_opt_alg == "ego" ||
	     nond_opt_alg == "ea"  || nond_opt_alg.empty())
      return new NonDGlobalSingleInterval(model);
    else
      return NULL;
  }
  else if (method_name == "nond_local_interval_est")
    return new NonDLocalSingleInterval(model);
  else if (method_name == "nond_bayes_calibration") {
    // TO DO: add sub_method to nond_bayes_calibration specification
    const String& sub_method_name
      = probDescDB.get_string("method.sub_method_name");
#ifdef DAKOTA_GPMSA
    if (sub_method_name == "gpmsa")
      return new NonDGPMSABayesCalibration(model);
#endif
#ifdef DAKOTA_QUESO
    if (sub_method_name == "queso")
      return new NonDQUESOBayesCalibration(model);
#endif
#ifdef HAVE_DREAM
    if (sub_method_name == "dream")
      return new NonDDREAMBayesCalibration(model);
#endif
    return NULL;
  }
  else if (method_name == "nonlinear_cg")
    return new NonlinearCGOptimizer(model);
#ifdef HAVE_OPTPP
  else if (strbegins(method_name, "optpp_")) {
    if (strends(method_name, "_g_newton")) return new SNLLLeastSq(model);
    else                               return new SNLLOptimizer(model);
  }
#endif
#ifdef DAKOTA_HOPS
  else if (method_name == "asynch_pattern_search")
    return new APPSOptimizer(model);
#endif
#ifdef DAKOTA_COLINY
  else if (strbegins(method_name, "coliny_"))
    return new COLINOptimizer(model);
#endif
#ifdef HAVE_JEGA
  else if (method_name == "moga" || method_name == "soga")
    return new JEGAOptimizer(model);
#endif
#ifdef DAKOTA_DL_SOLVER
  else if (method_name == "dl_solver")
    return new DLSolver(model);
#endif
#ifdef HAVE_NPSOL
  else if (method_name == "npsol_sqp")
    return new NPSOLOptimizer(model);
  else if (method_name == "nlssol_sqp")
    return new NLSSOLLeastSq(model);
#endif
#ifdef HAVE_NLPQL
  else if (method_name == "nlpql_sqp")
    return new NLPQLPOptimizer(model);
#endif
#ifdef HAVE_NL2SOL
  else if (method_name == "nl2sol")
    return new NL2SOLLeastSq(model);
#endif
//#ifdef DAKOTA_RSQP
//  else if (method_name == "reduced_sqp")
//    return new rSQPOptimizer(model);
//#endif
#ifdef HAVE_DOT
  else if (strbegins(method_name, "dot_"))
    return new DOTOptimizer(model);
#endif
#ifdef HAVE_CONMIN
  else if (strbegins(method_name, "conmin_"))
    return new CONMINOptimizer(model);
#endif
#ifdef DAKOTA_DDACE
  else if (method_name == "dace")
    return new DDACEDesignCompExp(model);
#endif
#ifdef HAVE_FSUDACE
  else if (strbegins(method_name, "fsu_"))
    return new FSUDesignCompExp(model);
#endif
#ifdef HAVE_PSUADE
  else if (strbegins(method_name, "psuade_"))
    return new PSUADEDesignCompExp(model);
#endif
#ifdef HAVE_NCSU
  else if (method_name == "ncsu_direct")
    return new NCSUOptimizer(model);
#endif
#ifdef HAVE_ADAPTIVE_SAMPLING 
  else if (method_name == "nond_adaptive_sampling")
    return new NonDAdaptiveSampling(model);
#endif
#ifdef HAVE_ESM 
  else if (method_name == "nond_efficient_subspace")
    return new EfficientSubspaceMethod(model);
#endif
   else {
    Cerr << "Invalid iterator: " << method_name << " not available."
	 << std::endl;
    return NULL;
  }
}


/** Used in sub-iterator instantiations within iterator constructors.
    Envelope constructor only needs to extract enough data to properly
    execute get_iterator(), since letter holds the actual base class data. */
Iterator::Iterator(const String& method_name, Model& model):
  //iteratedModel(model), // no Model copy for envelope
  probDescDB(model.problem_description_db()),
  resultsDB(iterator_results_db),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(Model&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate iterator type
  iteratorRep = get_iterator(method_name, model);
  if ( !iteratorRep ) // bad name or insufficient memory
    abort_handler(-1);
}


/** Used only by the envelope constructor to initialize iteratorRep to the
    appropriate derived type, as given by the passed method_name. */
Iterator* Iterator::get_iterator(const String& method_name, Model& model)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter: Getting iterator " <<  method_name
       << " by name." << std::endl;
#endif

  // These instantiations will NOT recurse on the Iterator(model)
  // constructor due to the use of BaseConstructor.

  //if (method_name == "surrogate_based_local")
  //  return new SurrBasedLocalMinimizer(NoDBBaseConstructor(), model);
  //else if (method_name == "surrogate_based_global")
  //  return new SurrBasedGlobalMinimizer(NoDBBaseConstructor(), model);
  //else if (method_name == "efficient_global")
  //  return new EffGlobalMinimizer(NoDBBaseConstructor(), model);

  if (false) { } // dummy anchor for else blocks to avoid issues with #ifdef's
#ifdef HAVE_OPTPP
  else if (strbegins(method_name, "optpp_")) {
    if (strends(method_name, "_g_newton"))
      return new SNLLLeastSq(method_name, model);
    else
      return new SNLLOptimizer(method_name, model);
  }
#endif
#ifdef DAKOTA_HOPS
  else if (method_name == "asynch_pattern_search")
    return new APPSOptimizer(NoDBBaseConstructor(), model);
#endif
#ifdef DAKOTA_COLINY
  else if (strbegins(method_name, "coliny_"))
    return new COLINOptimizer(method_name, model);
#endif
#ifdef HAVE_JEGA
  //else if (method_name == "moga" || method_name == "soga")
  //  return new JEGAOptimizer(NoDBBaseConstructor(), model);
#endif
#ifdef DAKOTA_DL_SOLVER
  //else if (method_name == "dl_solver")
  //  return new DLSolver(NoDBBaseConstructor(), model);
#endif
#ifdef HAVE_NPSOL
  else if (method_name == "npsol_sqp")
    return new NPSOLOptimizer(NoDBBaseConstructor(), model);
  else if (method_name == "nlssol_sqp")
    return new NLSSOLLeastSq(NoDBBaseConstructor(), model);
#endif
#ifdef HAVE_NLPQL
  else if (method_name == "nlpql_sqp")
    return new NLPQLPOptimizer(NoDBBaseConstructor(), model);
#endif
#ifdef HAVE_NL2SOL
  else if (method_name == "nl2sol")
    return new NL2SOLLeastSq(NoDBBaseConstructor(), model);
#endif
#ifdef HAVE_DOT
  else if (strbegins(method_name, "dot_"))
    return new DOTOptimizer(NoDBBaseConstructor(), model);
#endif
#ifdef HAVE_CONMIN
  else if (strbegins(method_name, "conmin_"))
    return new CONMINOptimizer(NoDBBaseConstructor(), model);
#endif
#ifdef HAVE_NCSU
  else if (method_name == "ncsu_direct")
    return new NCSUOptimizer(NoDBBaseConstructor(), model);
#endif
  else {
    Cerr << "Invalid iterator: " << method_name << " not available by name."
	 << std::endl;
    return NULL;
  }
}


/** Copy constructor manages sharing of iteratorRep and incrementing
    of referenceCount. */
Iterator::Iterator(const Iterator& iterator):
  probDescDB(iterator.problem_description_db()),
  resultsDB(iterator_results_db)
{
  // Increment new (no old to decrement)
  iteratorRep = iterator.iteratorRep;
  if (iteratorRep) // Check for an assignment of NULL
    iteratorRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(Iterator&)" << std::endl;
  if (iteratorRep)
    Cout << "iteratorRep referenceCount = " << iteratorRep->referenceCount
	 << std::endl;
#endif
}


/** Assignment operator decrements referenceCount for old iteratorRep, assigns
    new iteratorRep, and increments referenceCount for new iteratorRep. */
Iterator Iterator::operator=(const Iterator& iterator)
{
  if (iteratorRep != iterator.iteratorRep) { // normal case: old != new
    // Decrement old
    if (iteratorRep) // Check for NULL
      if ( --iteratorRep->referenceCount == 0 )
	delete iteratorRep;
    // Assign and increment new
    iteratorRep = iterator.iteratorRep;
    if (iteratorRep) // Check for NULL
      iteratorRep->referenceCount++;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::operator=(Iterator&)" << std::endl;
  if (iteratorRep)
    Cout << "iteratorRep referenceCount = " << iteratorRep->referenceCount
	 << std::endl;
#endif

  return *this; // calls copy constructor since returned by value
}


/** Destructor decrements referenceCount and only deletes iteratorRep
    when referenceCount reaches zero. */
Iterator::~Iterator()
{
  if (iteratorRep) { // Check for NULL pointer
    --iteratorRep->referenceCount;
#ifdef REFCOUNT_DEBUG
    Cout << "iteratorRep referenceCount decremented to "
         << iteratorRep->referenceCount << std::endl;
#endif
    if (iteratorRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting iteratorRep" << std::endl;
#endif
      delete iteratorRep;
    }
  }
}


/** Similar to the assignment operator, the assign_rep() function
    decrements referenceCount for the old iteratorRep and assigns the
    new iteratorRep.  It is different in that it is used for
    publishing derived class letters to existing envelopes, as opposed
    to sharing representations among multiple envelopes (in
    particular, assign_rep is passed a letter object and operator= is
    passed an envelope object).  Letter assignment supports two models as
    governed by ref_count_incr:

    \li ref_count_incr = true (default): the incoming letter belongs to
    another envelope.  In this case, increment the reference count in the
    normal manner so that deallocation of the letter is handled properly.

    \li ref_count_incr = false: the incoming letter is instantiated on the
    fly and has no envelope.  This case is modeled after get_iterator():
    a letter is dynamically allocated using new and passed into assign_rep,
    the letter's reference count is not incremented, and the letter is not
    remotely deleted (its memory management is passed over to the envelope). */
void Iterator::assign_rep(Iterator* iterator_rep, bool ref_count_incr)
{
  if (iteratorRep == iterator_rep) {
    // if ref_count_incr = true (rep from another envelope), do nothing as
    // referenceCount should already be correct (see also operator= logic).
    // if ref_count_incr = false (rep from on the fly), then this is an error.
    if (!ref_count_incr) {
      Cerr << "Error: duplicated iterator_rep pointer assignment without "
	   << "reference count increment in Iterator::assign_rep()."
	   << std::endl;
      abort_handler(-1);
    }
  }
  else { // normal case: old != new
    // Decrement old
    if (iteratorRep) // Check for NULL
      if ( --iteratorRep->referenceCount == 0 )
	delete iteratorRep;
    // Assign new
    iteratorRep = iterator_rep;
    // Increment new
    if (iteratorRep && ref_count_incr) // Check for NULL & honor ref_count_incr
      iteratorRep->referenceCount++;
  }

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::assign_rep(Iterator*)" << std::endl;
  if (iteratorRep)
    Cout << "iteratorRep referenceCount = " << iteratorRep->referenceCount
	 << std::endl;
#endif
}


/** Iterator supports a
    construct/initialize-run/pre-run/run/post-run/finalize-run/destruct
    progression. This member (non-virtual) function sequences these
    run phases; it accepts an ostream, but controls verbosity with
    outputLevel */
void Iterator::run_iterator(std::ostream& s)
{
  if (iteratorRep)
    iteratorRep->run_iterator(s); // envelope fwd to letter
  else {
    // Since iterator executions are mixed with direct model evaluations in
    // some methods/strategies (e.g., SBO and local/global reliability), avoid
    // having to reset the parallel configuration for the direct model evals
    // by eliminating configuration modifications within an iterator execution.
    //ParallelLibrary& parallel_lib = iteratedModel.parallel_library();
    //ParConfigLIter prev_pc = parallel_lib.parallel_configuration_iterator();

    // the same iterator might run multiple times, or need a unique ID
    // due to name/id duplication, so increment the execution number
    // for this name/id pair
    execNum = ResultsID::instance().increment_id(method_name(), method_id());

    initialize_run();
    if (summaryOutputFlag)
      s << "\n>>>>> Running "  << methodName <<" iterator.\n";
    if (iteratedModel.parallel_library().command_line_pre_run()) {
      if (summaryOutputFlag && outputLevel > NORMAL_OUTPUT)
	s << "\n>>>>> " << methodName <<": pre-run phase.\n";
      pre_run();
      pre_output(); // for now, the helper manages whether output is needed
    }
    if (iteratedModel.parallel_library().command_line_run()) {
      // run_input();
      if (summaryOutputFlag && outputLevel > NORMAL_OUTPUT)
	s << "\n>>>>> " << methodName <<": core run phase.\n";
      run();
      // run_output();
    }
    if (iteratedModel.parallel_library().command_line_post_run()) {
      post_input();
      if (summaryOutputFlag && outputLevel > NORMAL_OUTPUT)
	s << "\n>>>>> " << methodName <<": post-run phase.\n";
      post_run(s);
    }
    if (summaryOutputFlag)
      s << "\n<<<<< Iterator " << methodName <<" completed.\n";
    finalize_run();

    //parallel_lib.parallel_configuration_iterator(prev_pc); // reset
  }
}


/** Perform initialization phases of run sequence, like allocating
    memory and setting instance pointers.  Commonly used in
    sub-iterator executions.  This is a virtual function; when
    re-implementing, a derived class must call its nearest parent's
    initialize_run(), typically _before_ performing its own
    implementation steps. */
void Iterator::initialize_run()
{
  if (iteratorRep)
    iteratorRep->initialize_run(); // envelope fwd to letter
  else {

    // Verify that iteratedModel is not null (default ctor and some
    // NoDBBaseConstructor ctors leave iteratedModel uninitialized).
    if (!iteratedModel.is_null()) {
      // update context data that is outside scope of local DB specifications.
      // This is needed for reused objects.
      //iteratedModel.db_scope_reset(); // TO DO: need better name?

      // Do not reset the evaluation reference for sub-iterators
      // (previously managed via presence/absence of ostream)
      //if (!subIteratorFlag)
      if (summaryOutputFlag)
	iteratedModel.set_evaluation_reference();

      // Set the active parallel configuration within the Model
      iteratedModel.set_communicators(maxConcurrency);
    }

    // Handle late availability of model's asynchEvalFlag in this way
    // (previously initialized in Iterator constructor, this info is
    // now unavailable until after model.set_communicators())
    asynchFlag = iteratedModel.asynch_flag();

  }
}


/** pre-run phase, which a derived iterator may optionally
    reimplement; when not present, pre-run is likely integrated into
    the derived run function.  This is a virtual function; when
    re-implementing, a derived class must call its nearest parent's
    pre_run(), if implemented, typically _before_ performing its own
    implementation steps. */
void Iterator::pre_run()
{
  if (iteratorRep)
    iteratorRep->pre_run(); // envelope fwd to letter
  // else base class default behavior is no-op
}


/** Virtual run function for the iterator class hierarchy.  All
    derived classes need to redefine it. */
void Iterator::run()
{
  if (iteratorRep)
    iteratorRep->run(); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual run() function.\n"
         << "No default iteration defined at base class." << std::endl;
    abort_handler(-1);
  }
}


/** Post-run phase, which a derived iterator may optionally
    reimplement; when not present, post-run is likely integrated into
    run.  This is a virtual function; when re-implementing, a derived
    class must call its nearest parent's post_run(), typically _after_
    performing its own implementation steps. */
  void Iterator::post_run(std::ostream& s)
{
  if (iteratorRep)
    iteratorRep->post_run(s); // envelope fwd to letter
  else {

    if (summaryOutputFlag) {
      // Print the function evaluation summary for all Iterators
      if (!iteratedModel.is_null())
	iteratedModel.print_evaluation_summary(s); // full hdr, relative counts

      // The remaining final results output varies by iterator branch
      print_results(s);
    }

  }

  resultsDB.write_databases();

}


/** Optional: perform finalization phases of run sequence, like
    deallocating memory and resetting instance pointers.  Commonly
    used in sub-iterator executions.  This is a virtual function; when
    re-implementing, a derived class must call its nearest parent's
    finalize_run(), typically _after_ performing its own
    implementation steps. */
void Iterator::finalize_run()
{
  if (iteratorRep)
    iteratorRep->finalize_run(); // envelope fwd to letter
  // else base class default behavior is no-op
}


void Iterator::reset()
{
  if (iteratorRep)
    iteratorRep->reset(); // envelope fwd to letter
  // else base class default behavior is no-op
}


const Variables& Iterator::variables_results() const
{
  return (iteratorRep) ?
    iteratorRep->variables_results() : // envelope fwd to letter
    bestVariablesArray.front(); // default implementation if no override
}

const VariablesArray& Iterator::variables_array_results()
{
  return (iteratorRep) ?
    iteratorRep->variables_array_results() : // envelope fwd to letter
    bestVariablesArray; // default implementation if no override
}


const Response& Iterator::response_results() const
{
  return (iteratorRep) ?
    iteratorRep->response_results() : // envelope fwd to letter
    bestResponseArray.front(); // default implementation if no override
}


const ResponseArray& Iterator::response_array_results()
{
  return (iteratorRep) ?
    iteratorRep->response_array_results() : // envelope fwd to letter
    bestResponseArray; // default implementation if no override
}


void Iterator::response_results_active_set(const ActiveSet& set)
{
  if (iteratorRep)
    iteratorRep->response_results_active_set(set); // envelope fwd to letter
  else // default implementation if no override
    bestResponseArray.front().active_set(set);
}


bool Iterator::accepts_multiple_points() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->accepts_multiple_points();
  else // default for letter lacking virtual fn redefinition
    return false;
}


bool Iterator::returns_multiple_points() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->returns_multiple_points();
  else // default for letter lacking virtual fn redefinition
    return false;
}


void Iterator::initial_points(const VariablesArray& pts)
{
  if(iteratorRep) // envelope fwd to letter
    iteratorRep->initial_points(pts);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine initial_points virtual fn.\n"
	 << "No default defined at base class." << std::endl;
    abort_handler(-1);
  }
}


const VariablesArray& Iterator::initial_points() const
{
  if(!iteratorRep) { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine initial_points "
            "virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(-1);
  }

  return iteratorRep->initial_points(); // envelope fwd to letter
}


/** This is a convenience function for encapsulating graphics
    initialization operations.  It does not require a strategyRep
    forward since it is only used by letter objects. */
void Iterator::
initialize_graphics(bool graph_2d, bool tabular_data,
		    const String& tabular_file)
{
  if (iteratorRep)
    iteratorRep->initialize_graphics(graph_2d, tabular_data, tabular_file);
  else { // no redefinition of virtual fn., use default initialization
    if (graph_2d)     // initialize the 2D plots
      dakota_graphics.create_plots_2d(
	iteratedModel.current_variables(), iteratedModel.current_response());
    if (tabular_data) // initialize the tabular data file
      dakota_graphics.create_tabular_datastream(
	iteratedModel.current_variables(), iteratedModel.current_response(),
	tabular_file);
    if (graph_2d || tabular_data) // turn out automatic graphics logging
      iteratedModel.auto_graphics(true);
  }
}


/** This virtual function provides additional iterator-specific final results
    outputs beyond the function evaluation summary printed in finalize_run(). */
void Iterator::print_results(std::ostream& s)
{
  if (iteratorRep)
    iteratorRep->print_results(s); // envelope fwd to letter
  // else default base class output is nothing additional beyond the fn
  // evaluation summary printed in finalize_run()
}


/** Return current number of evaluation points.  Since the calculation
    of samples, collocation points, etc. might be costly, provide a
    default implementation here that backs out from the maxConcurrency.
    May be (is) overridden by derived classes. */
int Iterator::num_samples() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->num_samples();
  else  // default base class implementation
    return maxConcurrency/iteratedModel.derivative_concurrency();
}


void Iterator::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->sampling_reset(min_samples, all_data_flag, stats_flag);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine sampling_reset() virtual "
         << "fn.\nThis iterator does not support sampling." << std::endl;
    abort_handler(-1);
  }
}


const String& Iterator::sampling_scheme() const
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine sampling_scheme() virtual "
         << "fn.\nThis iterator does not support sampling." << std::endl;
    abort_handler(-1);
  }

  return iteratorRep->sampling_scheme(); // envelope fwd to letter
}


const Model& Iterator::algorithm_space_model() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->algorithm_space_model();
  else // default definition (letter lacking redefinition of virtual fn.)
    return iteratedModel; // captures any recasts within Optimizer/LeastSq
}


String Iterator::uses_method() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->uses_method();
  else // default definition (letter lacking redefinition of virtual fn.)
    return String(); // null: no enabling iterator for this iterator
}


void Iterator::method_recourse()
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->method_recourse();
  else { // default definition (letter lacking redefinition of virtual fn.)
    Cerr << "Error: no method recourse defined for detected method conflict.\n"
	 << "       Please revise method selections." << std::endl;
    abort_handler(-1);
  }
}


const VariablesArray& Iterator::all_variables()
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: letter class does not redefine all_variables() virtual fn."
         << "\n       This iterator does not support variables histories."
	 << std::endl;
    abort_handler(-1);
  }

  return iteratorRep->all_variables(); // envelope fwd to letter
}


const RealMatrix& Iterator::all_samples()
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: letter class does not redefine all_samples() virtual fn."
         << "\n       This iterator does not support sample histories."
	 << std::endl;
    abort_handler(-1);
  }

  return iteratorRep->all_samples(); // envelope fwd to letter
}


const IntResponseMap& Iterator::all_responses() const
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: letter class does not redefine all_responses() virtual fn."
         << "\n       This iterator does not support response histories."
	 << std::endl;
    abort_handler(-1);
  }

  return iteratorRep->all_responses(); // envelope fwd to letter
}


bool Iterator::compact_mode() const
{ return (iteratorRep) ? iteratorRep->compact_mode() : false; }


void Iterator::sub_iterator_flag(bool si_flag)
{
  // Implementation in .cpp due to need for DataMethod.hpp
  // If outputLevel were to change after call to this function, would
  // need to recheck/set status of summaryOyutputFlag
  if (iteratorRep)
    iteratorRep->sub_iterator_flag(si_flag);
  else {
    subIteratorFlag = si_flag;
    // enable summary output for verbose sub-iterators
    summaryOutputFlag = (subIteratorFlag && outputLevel > NORMAL_OUTPUT);
  }
}


StrStrSizet Iterator::run_identifier() const
{
  return(boost::make_tuple(method_name(), method_id(), execNum));
}


//void Iterator::pre_output(const String& filename)
void Iterator::pre_output()
{
  if (iteratorRep)
    iteratorRep->pre_output();
  else {
    // distinguish between defaulted pre-run and user-specified
    if (!iteratedModel.parallel_library().command_line_user_modes())
      return;

    const String& filename =
      iteratedModel.parallel_library().command_line_pre_run_output();
    if (filename.empty()) {
      if (outputLevel > QUIET_OUTPUT)
	Cout << "\nPre-run phase complete: no output requested.\n" << std::endl;
      return;
    }

    Cerr << "Error: letter class does not redefine pre_output() virtual fn."
	 << "\n        This iterator does not support pre-run output."
	 << std::endl;
  }
}


void Iterator::post_input()
{
  if (iteratorRep)
    iteratorRep->post_input(); // envelope fwd to letter
  else {

    // distinguish between defaulted post-run and user-specified
    if (!iteratedModel.parallel_library().command_line_user_modes())
      return;

    const String& filename = 
      iteratedModel.parallel_library().command_line_post_run_input();

    if (outputLevel > QUIET_OUTPUT) {
      if (filename.empty())
	Cout << "\nPost-run phase initialized: no input requested.\n"
	     << std::endl;
      else {
	// this should be unreachable due to command-line parsing
	Cerr << "\nError: method " << methodName << " does not support post-run"
	     << " file input." << std::endl;
	abort_handler(-1);
      }
    }
  }
}

} // namespace Dakota
