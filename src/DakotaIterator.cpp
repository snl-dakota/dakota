/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Iterator
//- Description: Implementation code for the Iterator class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_data_io.hpp"
#include "DakotaIterator.hpp"
#include "MetaIterator.hpp"
#include "ConcurrentMetaIterator.hpp"
#include "CollabHybridMetaIterator.hpp"
#include "EmbedHybridMetaIterator.hpp"
#include "SeqHybridMetaIterator.hpp"
#include "ParamStudy.hpp"
#include "RichExtrapVerification.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDLocalReliability.hpp"
#include "NonDGlobalReliability.hpp"
#include "NonDLHSSampling.hpp"
#include "NonDAdaptImpSampling.hpp"
#include "NonDGPImpSampling.hpp"
#include "NonDMultilevelSampling.hpp"
#include "NonDGlobalEvidence.hpp"
#include "NonDLocalEvidence.hpp"
#include "NonDLHSEvidence.hpp"
#include "NonDGlobalSingleInterval.hpp"
#include "NonDLocalSingleInterval.hpp"
#include "NonDLHSSingleInterval.hpp"
#include "NonDPOFDarts.hpp"
#include "NonDRKDDarts.hpp"
#include "DataFitSurrBasedLocalMinimizer.hpp"
#include "HierarchSurrBasedLocalMinimizer.hpp"
#include "SurrBasedGlobalMinimizer.hpp"
#include "EffGlobalMinimizer.hpp"
#include "NonlinearCGOptimizer.hpp"
#ifdef HAVE_DDACE
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
#ifdef HAVE_NOMAD
#include "NomadOptimizer.hpp"
#endif
#ifdef HAVE_NOWPAC
#include "NOWPACOptimizer.hpp"
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
#ifdef HAVE_ACRO
#include "COLINOptimizer.hpp"
#include "PEBBLMinimizer.hpp"
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
#ifdef HAVE_QUESO_GPMSA
#include "NonDGPMSABayesCalibration.hpp"
#endif
#ifdef HAVE_QUESO
#include "NonDQUESOBayesCalibration.hpp"
#endif
#ifdef HAVE_DREAM
#include "NonDDREAMBayesCalibration.hpp"
#endif
#ifdef HAVE_ADAPTIVE_SAMPLING 
#include "NonDAdaptiveSampling.hpp"
#endif
#include "OptDartsOptimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "DakotaGraphics.hpp"
#include "ResultsManager.hpp"
#include "NonDWASABIBayesCalibration.hpp"

//#define REFCOUNT_DEBUG

static const char rcsId[]="@(#) $Id: DakotaIterator.cpp 7029 2010-10-22 00:17:02Z mseldre $";

namespace Dakota {

extern ProblemDescDB   dummy_db;        // defined in dakota_global_defs.cpp
extern ParallelLibrary dummy_lib;       // defined in dakota_global_defs.cpp
extern ResultsManager  iterator_results_db;


/** This constructor builds the base class data for all inherited
    iterators, including meta-iterators.  get_iterator() instantiates
    a derived class and the derived class selects this base class
    constructor in its initialization list (to avoid the recursion of
    the base class constructor calling get_iterator() again).  Since
    the letter IS the representation, its representation pointer is
    set to NULL (an uninitialized pointer causes problems in ~Iterator). */
Iterator::Iterator(BaseConstructor, ProblemDescDB& problem_db):
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  methodPCIter(parallelLib.parallel_configuration_iterator()),
  myModelLayers(0),
  methodName(probDescDB.get_ushort("method.algorithm")),
  convergenceTol(probDescDB.get_real("method.convergence_tolerance")),
  maxIterations(probDescDB.get_int("method.max_iterations")),
  maxFunctionEvals(probDescDB.get_int("method.max_function_evaluations")),
  subIteratorFlag(false),
  numFinalSolutions(probDescDB.get_sizet("method.final_solutions")),
  // Output verbosity is observed within Iterator (algorithm verbosity),
  // Model (synchronize/estimate_derivatives verbosity), Interface
  // (map/synch verbosity, file operations verbosity), and Approximation
  // (global data fit coefficient reporting) as follows:
  //               iterator     model  iface: map, file ops      approx
  //   "silent" :   silent      silent     silent     quiet      quiet
  //   "quiet"  :    quiet      quiet       quiet     quiet      quiet
  //   "normal" :   normal      normal     normal     quiet      quiet
  //   "verbose":   verbose     normal     verbose    verbose    verbose
  //   "debug"  :    debug      normal      debug     verbose    verbose
  // where "silent," "quiet", "verbose" and "debug" must be user specified and
  // "normal" is the default for no user specification.  Note that iterators
  // and interfaces have the most granularity in verbosity.
  outputLevel(probDescDB.get_short("method.output")), summaryOutputFlag(true),
  resultsDB(iterator_results_db), methodId(probDescDB.get_string("method.id")),
  iteratorRep(NULL), referenceCount(1)
{
  if (outputLevel >= VERBOSE_OUTPUT)
    Cout << "methodName = " << method_enum_to_string(methodName) << '\n';

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(BaseConstructor, ProblemDescDB& problem_db) "
       << "called to build letter base class\n";
#endif
}


/** This alternate constructor builds base class data for inherited iterators.
    It is used for on-the-fly instantiations for which DB queries cannot be
    used, and is not used for construction of meta-iterators. */
Iterator::
Iterator(NoDBBaseConstructor, unsigned short method_name, Model& model):
  probDescDB(dummy_db), parallelLib(model.parallel_library()),
  methodPCIter(parallelLib.parallel_configuration_iterator()),
  myModelLayers(0),
  iteratedModel(model), methodName(method_name), convergenceTol(0.0001),
  maxIterations(100), maxFunctionEvals(1000), maxEvalConcurrency(1),
  subIteratorFlag(false), numFinalSolutions(1),
  outputLevel(model.output_level()), summaryOutputFlag(false),
  resultsDB(iterator_results_db), methodId("NO_SPECIFICATION"),
  iteratorRep(NULL), referenceCount(1)
{
  //update_from_model(iteratedModel); // variable/response counts & checks

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(NoDBBaseConstructor) called to build letter base "
       << "class\n";
#endif
}


/** This alternate constructor builds base class data for inherited
    iterators.  It is used for on-the-fly instantiations for which DB
    queries cannot be used, and is not used for construction of
    meta-iterators.  It has no incoming model, so only sets up a
    minimal set of defaults. However, its use is preferable to the
    default constructor, which should remain as minimal as possible. */
Iterator::Iterator(NoDBBaseConstructor, unsigned short method_name):
  probDescDB(dummy_db), parallelLib(dummy_lib), 
  myModelLayers(0), methodName(method_name),
  convergenceTol(0.0001), maxIterations(100), maxFunctionEvals(1000),
  maxEvalConcurrency(1), subIteratorFlag(false), numFinalSolutions(1),
  outputLevel(NORMAL_OUTPUT), summaryOutputFlag(false),
  resultsDB(iterator_results_db), methodId("NO_SPECIFICATION"),
  iteratorRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(NoDBBaseConstructor) called to build letter base "
       << "class\n";
#endif
}


/** The default constructor is used in Vector<Iterator> instantiations
    and for initialization of Iterator objects contained in
    meta-Iterators and Model recursions.  iteratorRep is NULL in this
    case, making it necessary to check for NULL pointers in the copy
    constructor, assignment operator, and destructor. */
Iterator::Iterator(): probDescDB(dummy_db), parallelLib(dummy_lib),
  resultsDB(iterator_results_db), myModelLayers(0), methodName(DEFAULT_METHOD),
  iteratorRep(NULL), referenceCount(1)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator() called to build empty envelope "
       << "base class object." << std::endl;
#endif
}


/** This constructor assigns a representation pointer and optionally
    increments its reference count.  It behaves the same as a default
    construction followed by assign_rep(). */
Iterator::Iterator(Iterator* iterator_rep, bool ref_count_incr):
  // same as default ctor above
  probDescDB(dummy_db), parallelLib(dummy_lib),
  resultsDB(iterator_results_db), myModelLayers(0), methodName(DEFAULT_METHOD),
  // bypass some logic in assign_rep():
  iteratorRep(iterator_rep), referenceCount(1)
{
  // relevant portion of assign_rep():
  if (iteratorRep && ref_count_incr)
    ++iteratorRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator() called to build empty envelope "
       << "base class object." << std::endl;
#endif
}


/** Envelope constructor only needs to extract enough data to properly
    execute get_iterator(), since letter holds the actual base class
    data.  This version is used for top-level ProblemDescDB-driven
    construction of all Iterators and MetaIterators, which construct
    their own Model instances. */
Iterator::Iterator(ProblemDescDB& problem_db):
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  resultsDB(iterator_results_db),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(ProblemDescDB&) called to instantiate "
       << "envelope." << std::endl;
#endif

  iteratorRep = get_iterator(problem_db);
  if ( !iteratorRep ) // bad name or insufficient memory
    abort_handler(METHOD_ERROR);
}


bool Iterator::resize()
{
  if (iteratorRep)
    return iteratorRep->resize(); // envelope fwd to letter
  else {
    // Update activeSet:
    activeSet = iteratedModel.current_response().active_set();

    return false; // No need to re-initialize communicators base on what
                  // was done here.
  }
}


/** Used only by the envelope constructor to initialize iteratorRep to
    the appropriate derived type, as given by the DB's method_name.
    Supports all iterators and meta-iterators.  These instantiations
    will NOT recurse on the Iterator(problem_db) constructor due to
    the use of BaseConstructor. */
Iterator* Iterator::get_iterator(ProblemDescDB& problem_db)
{
  unsigned short method_name = problem_db.get_ushort("method.algorithm");
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter: getting iterator case " << method_name
       << " = " << method_enum_to_string(method_name) << std::endl;
#endif

  // Meta-iterators support special constructors that are not bound to a Model
  // instance for top-level instantiation of general meta-iteration.  However,
  // they also support construction with a provided Model to allow use as a
  // component within an Iterator recursion.
  switch (method_name) {
  case HYBRID:
    switch (problem_db.get_ushort("method.sub_method")) {
    case SUBMETHOD_COLLABORATIVE:
      return new CollabHybridMetaIterator(problem_db);       break;
    case SUBMETHOD_EMBEDDED:
      return new EmbedHybridMetaIterator(problem_db);        break;
    case SUBMETHOD_SEQUENTIAL:
      return new SeqHybridMetaIterator(problem_db);          break;
    default:
      Cerr << "Invalid hybrid meta-iterator type." << std::endl;
      return NULL;                                           break;
    }
    break;
  case PARETO_SET: case MULTI_START:
    return new ConcurrentMetaIterator(problem_db);           break;
  default:
    // rather than create additional derived constructors for non-meta-iterators
    // that differ only in creation of their own Model instance, perform the
    // Model instantiation here and leverage the existing constructors.
    Model the_model = problem_db.get_model();
    return get_iterator(problem_db, the_model); break;
  }
}


/** Envelope constructor only needs to extract enough data to properly
    execute get_iterator(), since letter holds the actual base class
    data.  This version is used for ProblemDescDB-driven construction
    of Iterators that are passed a Model from a higher-level context
    (e.g., a MetaIterator instantiates its sub-iterator(s) by name
    instead of pointer and passes in its iteratedModel, since these
    sub-iterators lack their own model pointers). */
Iterator::Iterator(ProblemDescDB& problem_db, Model& model):
  probDescDB(problem_db), parallelLib(problem_db.parallel_library()),
  resultsDB(iterator_results_db),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(Model&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate iterator type
  iteratorRep = get_iterator(problem_db, model);
  if ( !iteratorRep ) // bad name or insufficient memory
    abort_handler(METHOD_ERROR);
}


/** Used only by the envelope constructor to initialize iteratorRep to
    the appropriate derived type.  Alternate construction of meta-iterators
    is supported to enable use of meta-iterators as components.  These
    instantiations will NOT recurse on the Iterator(problem_db, model)
    constructor due to the use of BaseConstructor. */
Iterator* Iterator::get_iterator(ProblemDescDB& problem_db, Model& model)
{
  unsigned short method_name = problem_db.get_ushort("method.algorithm");
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter: getting iterator case " << method_name
       << " = " << method_enum_to_string(method_name) << std::endl;
#endif

  switch (method_name) {
  case HYBRID:
    switch (problem_db.get_ushort("method.sub_method")) {
    case SUBMETHOD_COLLABORATIVE:
      return new CollabHybridMetaIterator(problem_db, model); break;
    case SUBMETHOD_EMBEDDED:
      return new EmbedHybridMetaIterator(problem_db, model);  break;
    case SUBMETHOD_SEQUENTIAL:
      return new SeqHybridMetaIterator(problem_db, model);    break;
    default:
      Cerr << "Invalid hybrid meta-iterator type." << std::endl;
      return NULL;                                            break;
    }
    break;
  case PARETO_SET: case MULTI_START:
    return new ConcurrentMetaIterator(problem_db, model); break;
  case CENTERED_PARAMETER_STUDY: case   LIST_PARAMETER_STUDY: 
  case MULTIDIM_PARAMETER_STUDY: case VECTOR_PARAMETER_STUDY: 
    return new ParamStudy(problem_db, model);             break;
  case RICHARDSON_EXTRAP:
    return new RichExtrapVerification(problem_db, model); break;
  case LOCAL_RELIABILITY:
    return new NonDLocalReliability(problem_db, model);   break;
  case LOCAL_INTERVAL_EST:
    return new NonDLocalSingleInterval(problem_db, model);break;
  case LOCAL_EVIDENCE:
    return new NonDLocalEvidence(problem_db, model);      break;
  case GLOBAL_RELIABILITY:
    return new NonDGlobalReliability(problem_db, model);  break;
  case GLOBAL_INTERVAL_EST:
    switch (probDescDB.get_ushort("method.sub_method")) {
    case SUBMETHOD_LHS:
      return new NonDLHSSingleInterval(problem_db, model); break;
    default: return new NonDGlobalSingleInterval(problem_db, model); break;
    } break;
  case GLOBAL_EVIDENCE:
    switch (probDescDB.get_ushort("method.sub_method")) {
    case SUBMETHOD_LHS: return new NonDLHSEvidence(problem_db, model);    break;
    default:            return new NonDGlobalEvidence(problem_db, model); break;
    } break;
  case POLYNOMIAL_CHAOS:
    return new NonDPolynomialChaos(problem_db, model);  break;
  case STOCH_COLLOCATION:
    return new NonDStochCollocation(problem_db, model); break;
  case BAYES_CALIBRATION:
    // TO DO: add sub_method to bayes_calibration specification
    switch (probDescDB.get_ushort("method.sub_method")) {
#ifdef HAVE_QUESO_GPMSA
    case SUBMETHOD_GPMSA:
      return new NonDGPMSABayesCalibration(problem_db, model); break;
#endif
#ifdef HAVE_QUESO
    case SUBMETHOD_QUESO:
      return new NonDQUESOBayesCalibration(problem_db, model); break;
#endif
#ifdef HAVE_DREAM
    case SUBMETHOD_DREAM:
      return new NonDDREAMBayesCalibration(problem_db, model); break;
#endif
    case SUBMETHOD_WASABI:
      return new NonDWASABIBayesCalibration(problem_db, model); break;
    default:
      Cerr << "Bayesian calibration selection not available." << std::endl;
      return NULL;                                            break;
    } break;
  case GPAIS:     return new NonDGPImpSampling(problem_db, model);     break;
  case POF_DARTS: return new NonDPOFDarts(problem_db, model);          break;
  case RKD_DARTS: return new NonDRKDDarts(problem_db, model);          break;
  case IMPORTANCE_SAMPLING:
    return new NonDAdaptImpSampling(problem_db, model);  break;
#ifdef HAVE_ADAPTIVE_SAMPLING
  case ADAPTIVE_SAMPLING:
    return new NonDAdaptiveSampling(problem_db, model);  break;
#endif
  case RANDOM_SAMPLING:
    return new NonDLHSSampling(problem_db, model); break;
  case MULTILEVEL_SAMPLING:
    return new NonDMultilevelSampling(problem_db, model); break;
  case DATA_FIT_SURROGATE_BASED_LOCAL:
    return new DataFitSurrBasedLocalMinimizer(problem_db, model);  break;
  case HIERARCH_SURROGATE_BASED_LOCAL:
    return new HierarchSurrBasedLocalMinimizer(problem_db, model);  break;
  case SURROGATE_BASED_LOCAL:
    if (model.surrogate_type() == "hierarchical")
      return new HierarchSurrBasedLocalMinimizer(problem_db, model);
    else
      return new DataFitSurrBasedLocalMinimizer(problem_db, model);
    break;
  case SURROGATE_BASED_GLOBAL:
    return new SurrBasedGlobalMinimizer(problem_db, model); break;
  case EFFICIENT_GLOBAL: return new EffGlobalMinimizer(problem_db, model);break;
  case NONLINEAR_CG: return new NonlinearCGOptimizer(problem_db, model);  break;
  case GENIE_OPT_DARTS: return new OptDartsOptimizer(problem_db, model); break;
  case GENIE_DIRECT: return new OptDartsOptimizer(problem_db, model); break;
#ifdef HAVE_OPTPP
  case OPTPP_G_NEWTON:
    return new SNLLLeastSq(problem_db, model);   break;
  case OPTPP_Q_NEWTON: case OPTPP_FD_NEWTON: case OPTPP_NEWTON:
  case OPTPP_CG:       case OPTPP_PDS:
    return new SNLLOptimizer(problem_db, model); break;
#endif
#ifdef DAKOTA_HOPS
  case ASYNCH_PATTERN_SEARCH:
    return new APPSOptimizer(problem_db, model); break;
#endif
#ifdef HAVE_ACRO
  case COLINY_BETA: case COLINY_COBYLA:         case COLINY_DIRECT:
  case COLINY_EA:   case COLINY_PATTERN_SEARCH: case COLINY_SOLIS_WETS:
    return new COLINOptimizer(problem_db, model); break;
  case BRANCH_AND_BOUND:
    return new PebbldMinimizer(problem_db, model); break;
#endif
#ifdef HAVE_JEGA
  case MOGA: case SOGA: return new JEGAOptimizer(problem_db, model); break;
#endif
#ifdef DAKOTA_DL_SOLVER
  case DL_SOLVER: return new DLSolver(problem_db, model); break;
#endif
#ifdef HAVE_NOMAD
  case MESH_ADAPTIVE_SEARCH:
    return new NomadOptimizer(problem_db, model); break;
#endif
#ifdef HAVE_NOWPAC
  case MIT_NOWPAC: case MIT_SNOWPAC:
    return new NOWPACOptimizer(problem_db, model); break;
#endif
#ifdef HAVE_NPSOL
  case NPSOL_SQP:   return new NPSOLOptimizer(problem_db, model);  break;
  case NLSSOL_SQP:  return new NLSSOLLeastSq(problem_db, model);   break;
#endif
#ifdef HAVE_NLPQL
  case NLPQL_SQP:   return new NLPQLPOptimizer(problem_db, model); break;
#endif
#ifdef HAVE_NL2SOL
  case NL2SOL:      return new NL2SOLLeastSq(problem_db, model);   break;
#endif
//#ifdef DAKOTA_RSQP
//  case REDUCED_SQP: return new rSQPOptimizer(problem_db, model);   break;
//#endif
#ifdef HAVE_DOT
  case DOT_BFGS: case DOT_FRCG: case DOT_MMFD: case DOT_SLP: case DOT_SQP:
    return new DOTOptimizer(problem_db, model); break;
#endif
#ifdef HAVE_CONMIN
  case CONMIN_FRCG: case CONMIN_MFD:
    return new CONMINOptimizer(problem_db, model); break;
#endif
#ifdef HAVE_DDACE
  case DACE: return new DDACEDesignCompExp(problem_db, model); break;
#endif
#ifdef HAVE_FSUDACE
  case FSU_CVT: case FSU_HALTON: case FSU_HAMMERSLEY:
    return new FSUDesignCompExp(problem_db, model); break;
#endif
#ifdef HAVE_PSUADE
  case PSUADE_MOAT: return new PSUADEDesignCompExp(problem_db, model); break;
#endif
#ifdef HAVE_NCSU
  case NCSU_DIRECT: return new NCSUOptimizer(problem_db, model);       break;
#endif
  default:
    Cerr << "Invalid iterator: " << method_enum_to_string(method_name)
	 << " not available." << std::endl;
    return NULL; break;
  }
}


/** Used in sub-iterator instantiations within iterator constructors.
    Envelope constructor only needs to extract enough data to properly
    execute get_iterator(), since letter holds the actual base class
    data.  This version is used for lightweight constructions without
    the ProblemDescDB. */
Iterator::Iterator(const String& method_string, Model& model):
  probDescDB(model.problem_description_db()),
  parallelLib(model.parallel_library()), resultsDB(iterator_results_db),
  referenceCount(1) // not used since this is the envelope, not the letter
{
#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::Iterator(Model&) called to instantiate "
       << "envelope." << std::endl;
#endif

  // Set the rep pointer to the appropriate iterator type
  iteratorRep = get_iterator(method_string, model);
  if ( !iteratorRep ) // bad name or insufficient memory
    abort_handler(METHOD_ERROR);
}


/** Used only by the envelope constructor to initialize iteratorRep to
    the appropriate derived type, as given by the passed method_string.
    Lightweight instantiations by name are supported by a subset of
    Iterators (primarily Minimizers). */
Iterator* Iterator::get_iterator(const String& method_string, Model& model)
{
#ifdef REFCOUNT_DEBUG
  Cout << "Envelope instantiating letter: getting iterator " <<  method_string
       << " by name." << std::endl;
#endif

  // These instantiations will NOT recurse on the Iterator(model)
  // constructor due to the use of BaseConstructor.

  //if (method_string == "data_fit_surrogate_based_local") {
  //  return new DataFitSurrBasedLocalMinimizer(model);
  //else if (method_string == "hierarch_surrogate_based_local") {
  //  return new HierarchSurrBasedLocalMinimizer(model);
  //else if (method_string == "surrogate_based_local") {
  //  return (model.surrogate_type() == "hierarchical) ?
  //    new HierarchSurrBasedLocalMinimizer(model) :
  //    new DataFitSurrBasedLocalMinimizer(model);
  //else if (method_string == "surrogate_based_global")
  //  return new SurrBasedGlobalMinimizer(model);
  //else if (method_string == "efficient_global")
  //  return new EffGlobalMinimizer(model);

  if (strbegins(method_string, "genie_"))
    return new OptDartsOptimizer(model);
#ifdef HAVE_OPTPP
  else if (strbegins(method_string, "optpp_")) {
    if (strends(method_string, "_g_newton"))
      return new SNLLLeastSq(method_string, model);
    else
      return new SNLLOptimizer(method_string, model);
  }
#endif
#ifdef DAKOTA_HOPS
  else if (method_string == "asynch_pattern_search")
    return new APPSOptimizer(model);
#endif
#ifdef HAVE_ACRO
  else if (strbegins(method_string, "coliny_"))
    return new COLINOptimizer(method_string, model);
  else if (method_string == "branch_and_bound")
    return new PebbldMinimizer(model);
#endif
#ifdef HAVE_JEGA
  //else if (method_string == "moga" || method_string == "soga")
  //  return new JEGAOptimizer(model);
#endif
#ifdef DAKOTA_DL_SOLVER
  //else if (method_string == "dl_solver")
  //  return new DLSolver(model);
#endif
#ifdef HAVE_NOMAD
  else if (method_string == "mesh_adaptive_search")
    return new NomadOptimizer(model);
#endif
#ifdef HAVE_NOWPAC
  else if (strends(method_string, "nowpac"))
    return new NOWPACOptimizer(model);
#endif
#ifdef HAVE_NPSOL
  else if (method_string == "npsol_sqp")
    return new NPSOLOptimizer(model);
  else if (method_string == "nlssol_sqp")
    return new NLSSOLLeastSq(model);
#endif
#ifdef HAVE_NLPQL
  else if (method_string == "nlpql_sqp")
    return new NLPQLPOptimizer(model);
#endif
#ifdef HAVE_NL2SOL
  else if (method_string == "nl2sol")
    return new NL2SOLLeastSq(model);
#endif
#ifdef HAVE_DOT
  else if (strbegins(method_string, "dot_"))
    return new DOTOptimizer(method_string, model);
#endif
#ifdef HAVE_CONMIN
  else if (strbegins(method_string, "conmin_"))
    return new CONMINOptimizer(method_string, model);
#endif
#ifdef HAVE_NCSU
  else if (method_string == "ncsu_direct")
    return new NCSUOptimizer(model);
#endif
  else {
    Cerr << "Invalid iterator: " << method_string << " not available by name."
	 << std::endl;
    return NULL;
  }
}


/** Copy constructor manages sharing of iteratorRep and incrementing
    of referenceCount. */
Iterator::Iterator(const Iterator& iterator):
  probDescDB(iterator.problem_description_db()),
  parallelLib(iterator.parallel_library()), resultsDB(iterator_results_db)
{
  // Increment new (no old to decrement)
  iteratorRep = iterator.iteratorRep;
  if (iteratorRep) // Check for an assignment of NULL
    ++iteratorRep->referenceCount;

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
      ++iteratorRep->referenceCount;
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
      abort_handler(METHOD_ERROR);
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
      ++iteratorRep->referenceCount;
  }

#ifdef REFCOUNT_DEBUG
  Cout << "Iterator::assign_rep(Iterator*)" << std::endl;
  if (iteratorRep)
    Cout << "iteratorRep referenceCount = " << iteratorRep->referenceCount
	 << std::endl;
#endif
}


String Iterator::method_enum_to_string(unsigned short method_name) const
{
  switch (method_name) {
  case HYBRID:                  return String("hybrid"); break;
  case PARETO_SET:              return String("pareto_set"); break;
  case MULTI_START:             return String("multi_start"); break;
  case CENTERED_PARAMETER_STUDY:return String("centered_parameter_study");break;
  case LIST_PARAMETER_STUDY:    return String("list_parameter_study");    break;
  case MULTIDIM_PARAMETER_STUDY:return String("multidim_parameter_study");break;
  case VECTOR_PARAMETER_STUDY:  return String("vector_parameter_study");  break;
  case RICHARDSON_EXTRAP:       return String("richardson_extrap"); break;
  case LOCAL_RELIABILITY:       return String("local_reliability"); break;
  case LOCAL_INTERVAL_EST:      return String("local_interval_est"); break;
  case LOCAL_EVIDENCE:          return String("local_evidence"); break;
  case GLOBAL_RELIABILITY:      return String("global_reliability"); break;
  case GLOBAL_INTERVAL_EST:     return String("global_interval_est"); break;
  case GLOBAL_EVIDENCE:         return String("global_evidence"); break;
  case POLYNOMIAL_CHAOS:        return String("polynomial_chaos"); break;
  case STOCH_COLLOCATION:       return String("stoch_collocation"); break;
  case BAYES_CALIBRATION:       return String("bayes_calibration"); break;
  case CUBATURE_INTEGRATION:    return String("cubature"); break;
  case QUADRATURE_INTEGRATION:  return String("quadrature"); break;
  case SPARSE_GRID_INTEGRATION: return String("sparse_grid"); break;
  case GPAIS:                   return String("gpais"); break;
  case POF_DARTS:               return String("pof_darts"); break;
  case RKD_DARTS:               return String("rkd_darts"); break;
  case IMPORTANCE_SAMPLING:     return String("importance_sampling"); break;
  case ADAPTIVE_SAMPLING:       return String("adaptive_sampling"); break;
  case RANDOM_SAMPLING:         return String("random_sampling"); break;
  case MULTILEVEL_SAMPLING:     return String("multilevel_sampling"); break;
  case LIST_SAMPLING:           return String("list_sampling"); break;
  case SURROGATE_BASED_LOCAL:   return String("surrogate_based_local"); break;
  case DATA_FIT_SURROGATE_BASED_LOCAL:
    return String("data_fit_surrogate_based_local"); break;
  case HIERARCH_SURROGATE_BASED_LOCAL:
    return String("hierarch_surrogate_based_local"); break;
  case SURROGATE_BASED_GLOBAL:  return String("surrogate_based_global"); break;
  case EFFICIENT_GLOBAL:        return String("efficient_global"); break;
  case NONLINEAR_CG:            return String("nonlinear_cg"); break;
  case GENIE_DIRECT:            return String("genie_direct"); break;
  case GENIE_OPT_DARTS:         return String("genie_opt_darts"); break;
  case OPTPP_G_NEWTON:          return String("optpp_g_newton"); break;
  case OPTPP_Q_NEWTON:          return String("optpp_q_newton"); break;
  case OPTPP_FD_NEWTON:         return String("optpp_fd_newton"); break;
  case OPTPP_NEWTON:            return String("optpp_newton"); break;
  case OPTPP_CG:                return String("optpp_cg"); break;
  case OPTPP_PDS:               return String("optpp_pds"); break;
  case ASYNCH_PATTERN_SEARCH:   return String("asynch_pattern_search"); break;
  case COLINY_BETA:             return String("coliny_beta"); break;
  case COLINY_COBYLA:           return String("coliny_cobyla"); break;
  case COLINY_DIRECT:           return String("coliny_direct"); break;
  case COLINY_EA:               return String("coliny_ea"); break;
  case COLINY_PATTERN_SEARCH:   return String("coliny_pattern_search"); break;
  case COLINY_SOLIS_WETS:       return String("coliny_solis_wets"); break;
  case BRANCH_AND_BOUND:        return String("branch_and_bound"); break;
  case MOGA:                    return String("moga"); break;
  case SOGA:                    return String("soga"); break;
  case DL_SOLVER:               return String("dl_solver"); break;
  case MESH_ADAPTIVE_SEARCH:    return String("mesh_adaptive_search"); break;
  case MIT_NOWPAC:              return String("nowpac"); break;
  case MIT_SNOWPAC:             return String("snowpac"); break;
  case NPSOL_SQP:               return String("npsol_sqp"); break;
  case NLSSOL_SQP:              return String("nlssol_sqp"); break;
  case NLPQL_SQP:               return String("nlpql_sqp"); break;
  case NL2SOL:                  return String("nl2sol"); break;
  //case REDUCED_SQP:           return String("reduced_sqp"); break;
  case DOT_BFGS:                return String("dot_bfgs"); break;
  case DOT_FRCG:                return String("dot_frcg"); break;
  case DOT_MMFD:                return String("dot_mmfd"); break;
  case DOT_SLP:                 return String("dot_slp"); break;
  case DOT_SQP:                 return String("dot_sqp"); break;
  case CONMIN_FRCG:             return String("conmin_frcg"); break;
  case CONMIN_MFD:              return String("conmin_mfd"); break;
  case DACE:                    return String("dace"); break;
  case FSU_CVT:                 return String("fsu_cvt"); break;
  case FSU_HALTON:              return String("fsu_halton"); break;
  case FSU_HAMMERSLEY:          return String("fsu_hammersley"); break;
  case PSUADE_MOAT:             return String("psuade_moat"); break;
  case NCSU_DIRECT:             return String("ncsu_direct"); break;
  default:
    Cerr << "Invalid method conversion: case " << method_name
	 << " not available." << std::endl;
    abort_handler(METHOD_ERROR); return String(); break;
  }
}


unsigned short Iterator::method_string_to_enum(const String& method_name) const
{
  if (method_name == "hybrid")  return HYBRID;
  else if (method_name == "pareto_set")  return PARETO_SET;
  else if (method_name == "multi_start") return MULTI_START;
  else if (method_name == "centered_parameter_study")
    return CENTERED_PARAMETER_STUDY;
  else if (method_name == "list_parameter_study")
    return LIST_PARAMETER_STUDY;
  else if (method_name == "multidim_parameter_study")
    return MULTIDIM_PARAMETER_STUDY;
  else if (method_name == "vector_parameter_study")
    return VECTOR_PARAMETER_STUDY;
  else if (method_name == "richardson_extrap")     return RICHARDSON_EXTRAP;
  else if (method_name == "local_reliability")     return LOCAL_RELIABILITY;
  else if (method_name == "local_interval_est")    return LOCAL_INTERVAL_EST;
  else if (method_name == "local_evidence")        return LOCAL_EVIDENCE;
  else if (method_name == "global_reliability")    return GLOBAL_RELIABILITY;
  else if (method_name == "global_interval_est")   return GLOBAL_INTERVAL_EST;
  else if (method_name == "global_evidence")       return GLOBAL_EVIDENCE;
  else if (method_name == "polynomial_chaos")      return POLYNOMIAL_CHAOS;
  else if (method_name == "stoch_collocation")     return STOCH_COLLOCATION;
  else if (method_name == "bayes_calibration")     return BAYES_CALIBRATION;
  else if (method_name == "cubature")    return CUBATURE_INTEGRATION;
  else if (method_name == "quadrature")  return QUADRATURE_INTEGRATION;
  else if (method_name == "sparse_grid") return SPARSE_GRID_INTEGRATION;
  else if (method_name == "gpais")                 return GPAIS;
  else if (method_name == "pof_darts")             return POF_DARTS;
  else if (method_name == "rkd_darts")             return RKD_DARTS;
  else if (method_name == "importance_sampling")   return IMPORTANCE_SAMPLING;
  else if (method_name == "adaptive_sampling")     return ADAPTIVE_SAMPLING;
  else if (method_name == "random_sampling")       return RANDOM_SAMPLING;
  else if (method_name == "multilevel_sampling")   return MULTILEVEL_SAMPLING;
  else if (method_name == "list_sampling")         return LIST_SAMPLING;
  else if (method_name == "surrogate_based_local") return SURROGATE_BASED_LOCAL;
  else if (method_name == "data_fit_surrogate_based_local")
    return DATA_FIT_SURROGATE_BASED_LOCAL;
  else if (method_name == "hierarch_surrogate_based_local")
    return HIERARCH_SURROGATE_BASED_LOCAL;
  else if (method_name == "surrogate_based_global")
    return SURROGATE_BASED_GLOBAL;
  else if (method_name == "efficient_global") return EFFICIENT_GLOBAL;
  else if (method_name == "nonlinear_cg")     return NONLINEAR_CG;
  else if (method_name == "genie_opt_darts")  return GENIE_OPT_DARTS;
  else if (method_name == "genie_direct")     return GENIE_DIRECT;
  else if (method_name == "optpp_g_newton")   return OPTPP_G_NEWTON;
  else if (method_name == "optpp_q_newton")   return OPTPP_Q_NEWTON;
  else if (method_name == "optpp_fd_newton")  return OPTPP_FD_NEWTON;
  else if (method_name == "optpp_newton")     return OPTPP_NEWTON;
  else if (method_name == "optpp_cg")         return OPTPP_CG;
  else if (method_name == "optpp_pds")        return OPTPP_PDS;
  else if (method_name == "asynch_pattern_search") return ASYNCH_PATTERN_SEARCH;
  else if (method_name == "coliny_beta")      return COLINY_BETA;
  else if (method_name == "coliny_cobyla")    return COLINY_COBYLA;
  else if (method_name == "coliny_direct")    return COLINY_DIRECT;
  else if (method_name == "coliny_ea")        return COLINY_EA;
  else if (method_name == "coliny_pattern_search") return COLINY_PATTERN_SEARCH;
  else if (method_name == "coliny_solis_wets")     return COLINY_SOLIS_WETS;
  else if (method_name == "branch_and_bound")      return BRANCH_AND_BOUND;
  else if (method_name == "moga")             return MOGA;
  else if (method_name == "soga")             return SOGA;
  else if (method_name == "dl_solver")        return DL_SOLVER;
  else if (method_name == "mesh_adaptive_search")  return MESH_ADAPTIVE_SEARCH;
  else if (method_name == "nowpac")           return MIT_NOWPAC;
  else if (method_name == "snowpac")          return MIT_SNOWPAC;
  else if (method_name == "npsol_sqp")        return NPSOL_SQP;
  else if (method_name == "nlssol_sqp")       return NLSSOL_SQP;
  else if (method_name == "nlpql_sqp")        return NLPQL_SQP;
  else if (method_name == "nl2sol")           return NL2SOL;
  //else if (method_name == "reduced_sqp")    return REDUCED_SQP;
  else if (method_name == "dot_bfgs")         return DOT_BFGS;
  else if (method_name == "dot_frcg")         return DOT_FRCG;
  else if (method_name == "dot_mmfd")         return DOT_MMFD;
  else if (method_name == "dot_slp")          return DOT_SLP;
  else if (method_name == "dot_sqp")          return DOT_SQP;
  else if (method_name == "conmin_frcg")      return CONMIN_FRCG;
  else if (method_name == "conmin_mfd")       return CONMIN_MFD;
  else if (method_name == "dace")             return DACE;
  else if (method_name == "fsu_cvt")          return FSU_CVT;
  else if (method_name == "fsu_halton")       return FSU_HALTON;
  else if (method_name == "fsu_hammersley")   return FSU_HAMMERSLEY;
  else if (method_name == "psuade_moat")      return PSUADE_MOAT;
  else if (method_name == "ncsu_direct")      return NCSU_DIRECT;
  else if (method_name == "genie_opt_darts")  return GENIE_OPT_DARTS;
  else if (method_name == "genie_direct")     return GENIE_DIRECT;
  else {
    Cerr << "Invalid method conversion: " << method_name << " not available."
	 << std::endl;
    abort_handler(METHOD_ERROR); return 0;
  }
}


String Iterator::submethod_enum_to_string(unsigned short submethod_name) const
{
  switch (submethod_name) {
  case SUBMETHOD_COLLABORATIVE: return String("collaborative"); break;
  case SUBMETHOD_EMBEDDED: return String("embedded"); break;
  case SUBMETHOD_SEQUENTIAL: return String("sequential"); break;
  case SUBMETHOD_LHS: return String("lhs"); break;
  case SUBMETHOD_RANDOM: return String("random"); break;
  case SUBMETHOD_BOX_BEHNKEN: return String("box_behnken"); break;
  case SUBMETHOD_CENTRAL_COMPOSITE: return String("central_composite"); break;
  case SUBMETHOD_GRID: return String("grid"); break; 
  case SUBMETHOD_OA_LHS: return String("oa_lhs"); break;
  case SUBMETHOD_OAS: return String("oas"); break;
  case SUBMETHOD_DREAM: return String("dream"); break;
  case SUBMETHOD_WASABI: return String("wasabi"); break;
  case SUBMETHOD_GPMSA: return String("gpmsa"); break;
  case SUBMETHOD_QUESO: return String("queso"); break;
  case SUBMETHOD_NIP: return String("nip"); break;
  case SUBMETHOD_SQP: return String("sqp"); break;
  case SUBMETHOD_EA: return String("ea"); break;
  case SUBMETHOD_EGO: return String("ego"); break;
  case SUBMETHOD_SBO: return String("sbo"); break;
  case SUBMETHOD_CONVERGE_ORDER: return String("converge_order"); break; 
  case SUBMETHOD_CONVERGE_QOI: return String("converge_qoi"); break;
  case SUBMETHOD_ESTIMATE_ORDER: return String("estimate_order"); break;
  default:
    Cerr << "Invalid submethod conversion: case " << submethod_name
	 << " not available." << std::endl;
    abort_handler(METHOD_ERROR); return String(); break;
  }
}


void Iterator::update_from_model(const Model& model)
{
  //bool err_flag = false;

  maxEvalConcurrency = model.derivative_concurrency(); // baseline

  // default activeSet is all specified response data is active
  activeSet = model.current_response().active_set(); // copy

  // bestResponseArray initialized in "user space" prior to any recasts
  Response best_resp = model.current_response().copy();
  // Set bestResponse ASV to vector of 1's since only fnValues are used.
  // NOTE: bestResponse initialization above may allocate space for gradients
  // or Hessians (which wastes space in current usage).
  ShortArray asv(best_resp.num_functions(), 1);
  best_resp.active_set_request_vector(asv);
  bestResponseArray.push_back(best_resp);

  //if (err_flag)
  //  abort_handler(METHOD_ERROR);
}


void Iterator::run(ParLevLIter pl_iter)
{
  if (iteratorRep)
    iteratorRep->run(pl_iter); // envelope fwd to letter
  else {
    set_communicators(pl_iter);
    run(); // invoke overloaded version
  }
}


/** Iterator supports a construct/initialize-run/pre-run/core-run/post-run/
    finalize-run/destruct progression. This member (non-virtual) function
    sequences these run phases. */
void Iterator::run()
{
  if (iteratorRep)
    iteratorRep->run(); // envelope fwd to letter
  else {
    // the same iterator might run multiple times, or need a unique ID due to
    // name/id duplication, so increment execution number for this name/id pair
    String method_string = method_enum_to_string(methodName);
    execNum = ResultsID::instance().increment_id(method_string, method_id());

    initialize_run();
    if (summaryOutputFlag)
      Cout << "\n>>>>> Running "  << method_string <<" iterator.\n";
    if (parallelLib.command_line_pre_run()) {
      if (summaryOutputFlag && outputLevel > NORMAL_OUTPUT)
	Cout << "\n>>>>> " << method_string <<": pre-run phase.\n";
      pre_run();
      pre_output(); // for now, the helper manages whether output is needed
    }
    if (parallelLib.command_line_run()) {
      //core_input();
      if (summaryOutputFlag && outputLevel > NORMAL_OUTPUT)
	Cout << "\n>>>>> " << method_string <<": core run phase.\n";
      core_run();
      //core_output();
    }
    if (parallelLib.command_line_post_run()) {
      post_input();
      if (summaryOutputFlag && outputLevel > NORMAL_OUTPUT)
	Cout << "\n>>>>> " << method_string <<": post-run phase.\n";
      post_run(Cout); // stream could be passed through run() API if needed
    }
    if (summaryOutputFlag)
      Cout << "\n<<<<< Iterator " << method_string <<" completed.\n";
    finalize_run();
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
  // else base class default behavior is no-op
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
void Iterator::core_run()
{
  if (iteratorRep)
    iteratorRep->core_run(); // envelope fwd to letter
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: Letter lacking redefinition of virtual core_run() function."
	 << "\nNo default iteration defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
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
  // else base class default behavior is no-op
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


void Iterator::resize_communicators(ParLevLIter pl_iter, bool reinit_comms)
{
  bool multiproc = (pl_iter->server_communicator_size() > 1);
  if (reinit_comms) {
    // Free communicators before we rebuild:
    short mapping_code;
    if (multiproc) {
      mapping_code = FREE_COMMS;
      parallelLib.bcast(mapping_code, *pl_iter);
      parallelLib.bcast(maxEvalConcurrency, *pl_iter);
    }
    free_communicators(pl_iter);

    // Re-initialize communicators:
    if (multiproc) {
      mapping_code = INIT_COMMS;
      parallelLib.bcast(mapping_code, *pl_iter);
    }
    init_communicators(pl_iter);
    if (multiproc) iteratedModel.stop_init_communicators(pl_iter);
  }

  // update message lengths for send/receive of parallel jobs (normally
  // performed once in Model::init_communicators() just after construct time)
  iteratedModel.estimate_message_lengths();
  if (multiproc) {
    short mapping_code = ESTIMATE_MESSAGE_LENGTHS;
    parallelLib.bcast(mapping_code, *pl_iter);
  }
}


void Iterator::init_communicators(ParLevLIter pl_iter)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->init_communicators(pl_iter);
  else {
    // Don't need to include maxEvalConcurrency since this is constant for
    // a particular Iterator/Model pair: Models can be reused in different
    // contexts, but non-meta-iterators should only have a single Model,
    // providing derivative concurrency, and a single derived max-concurrency.
    //SizetIntPair key(pl_index, maxEvalConcurrency);
    size_t pl_index = parallelLib.parallel_level_index(pl_iter);
    std::map<size_t, ParConfigLIter>::iterator map_iter
      = methodPCIterMap.find(pl_index);
    if (map_iter == methodPCIterMap.end()) { // this config does not exist
      parallelLib.increment_parallel_configuration(pl_iter);
      methodPCIterMap[pl_index] = methodPCIter
	= parallelLib.parallel_configuration_iterator();
      derived_init_communicators(pl_iter);
    }
    else {
      methodPCIter = map_iter->second;
      // don't need to invoke derived_init_communicators() since each unique
      // init only needs to be recursed once (see also Model::init_comms)
    }

    // After partitioning is complete, output tags for concurrent
    // iterators are established.  Initialize the eval id prefix for
    // this Iterator and its underlying Model.  This may get appended
    // to by any runtime updates as eval ids change.
    eval_tag_prefix(parallelLib.output_manager().build_output_tag());
 }
}


void Iterator::derived_init_communicators(ParLevLIter pl_iter)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->derived_init_communicators(pl_iter);
  else if (!iteratedModel.is_null()) // default: init comms for iteratedModel
    iteratedModel.init_communicators(pl_iter, maxEvalConcurrency); // recurse
}


void Iterator::set_communicators(ParLevLIter pl_iter)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->set_communicators(pl_iter);
  else {
    // set the comms within Iterator: methodPCIter, miPLIndex, & derived.
    // > miPLIndex is the index within ParallelConfiguration::miPLIters
    // > pl_index (key within methodPCIterMap) is the index within
    //   ParallelLibrary::parallelLevels (is configuration independent)
    size_t pl_index = parallelLib.parallel_level_index(pl_iter);
    std::map<size_t, ParConfigLIter>::iterator map_iter
      = methodPCIterMap.find(pl_index);
    if (map_iter == methodPCIterMap.end()) { // this config does not exist
      Cerr << "Error: failure in parallel configuration lookup in "
           << "Iterator::set_communicators() for pl_index = " << pl_index << "." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    else
      methodPCIter = map_iter->second;

    // This corresponds to pl_iter, prior to sub-iterator partitioning, and
    // is therefore not that useful (misleading in MetaIterators).
    //miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);

    // Unlike init_comms, set_comms DOES need to be recursed each time to 
    // activate the correct comms at each level of the recursion.
    derived_set_communicators(pl_iter);
  }
}


void Iterator::derived_set_communicators(ParLevLIter pl_iter)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->derived_set_communicators(pl_iter);
  else if (!iteratedModel.is_null()) // default: set comms within iteratedModel
    iteratedModel.set_communicators(pl_iter, maxEvalConcurrency);  // recurse
}


void Iterator::free_communicators(ParLevLIter pl_iter)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->free_communicators(pl_iter);
  else {
    // Note: deallocations do not utilize reference counting -> the _first_
    // call to free a particular configuration deallocates it and all
    // subsequent calls are ignored (to prevent multiple deallocations).
    size_t pl_index = parallelLib.parallel_level_index(pl_iter);
    std::map<size_t, ParConfigLIter>::iterator map_iter
      = methodPCIterMap.find(pl_index);
    if (map_iter != methodPCIterMap.end()) { // this config still exists
      methodPCIter = map_iter->second;
      // Like derived_init, derived_free can be protected inside found block
      derived_free_communicators(pl_iter);
      methodPCIterMap.erase(pl_index);
    }
  }
}


void Iterator::derived_free_communicators(ParLevLIter pl_iter)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->derived_free_communicators(pl_iter);
  else if (!iteratedModel.is_null()) // default: free comms on iteratedModel
    iteratedModel.free_communicators(pl_iter, maxEvalConcurrency); // recurse
}


void Iterator::reset()
{
  if (iteratorRep)
    iteratorRep->reset(); // envelope fwd to letter
  // else base class default behavior is no-op
}


void Iterator::initialize_iterator(int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->initialize_iterator(job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine initialize_iterator virtual "
	 << "fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void Iterator::pack_parameters_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->pack_parameters_buffer(send_buffer, job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine pack_parameters_buffer "
	 << "virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void Iterator::
unpack_parameters_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->unpack_parameters_buffer(recv_buffer, job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine unpack_parameters_buffer "
	 << "virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void Iterator::
unpack_parameters_initialize(MPIUnpackBuffer& recv_buffer, int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->unpack_parameters_initialize(recv_buffer, job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine unpack_parameters_initialize"
	 << " virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void Iterator::pack_results_buffer(MPIPackBuffer& send_buffer, int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->pack_results_buffer(send_buffer, job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine pack_results_buffer virtual "
	 << "fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void Iterator::
unpack_results_buffer(MPIUnpackBuffer& recv_buffer, int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->unpack_results_buffer(recv_buffer, job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine unpack_results_buffer "
	 << "virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void Iterator::update_local_results(int job_index)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->update_local_results(job_index);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine update_local_results "
	 << "virtual  fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
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


const RealVector& Iterator::response_error_estimates() const
{
  // no default implementation if no override
  if (!iteratorRep) {
    Cerr << "Error: letter class does not redefine response_error_estimates "
	 << "virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  
  return iteratorRep->response_error_estimates(); // envelope fwd to letter
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
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->initial_points(pts);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine initial_points virtual fn.\n"
	 << "No default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


const VariablesArray& Iterator::initial_points() const
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine initial_points "
            "virtual fn.\nNo default defined at base class." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return iteratorRep->initial_points(); // envelope fwd to letter
}


/** For Gauss-Newton Hessian requests, activate the 2 bit and mask the 4 bit. */
void Iterator::
gnewton_set_recast(const Variables& recast_vars, const ActiveSet& recast_set,
		   ActiveSet& sub_model_set)
{
  // AUGMENT standard mappings in RecastModel::set_mapping()
  const ShortArray& sub_model_asv = sub_model_set.request_vector();
  size_t i, num_sm_fns = sub_model_asv.size();
  for (i=0; i<num_sm_fns; ++i)
    if (sub_model_asv[i] & 4) { // add 2 bit and remove 4 bit
      short sm_asv_val = ( (sub_model_asv[i] | 2) & 3);
      sub_model_set.request_value(sm_asv_val, i);
    }
}


/** This is a convenience function for encapsulating graphics
    initialization operations. It is overridden by derived classes
    that specialize the graphics display. */
void Iterator::initialize_graphics(int iterator_server_id)
{
  if (iteratorRep)
    iteratorRep->initialize_graphics(iterator_server_id);
  else { // no redefinition of virtual fn., use default initialization
    OutputManager& mgr = parallelLib.output_manager();
    Graphics& dakota_graphics = mgr.graphics();
    const Variables& vars = iteratedModel.current_variables();
    const Response&  resp = iteratedModel.current_response();
    bool auto_log = false;

    // For graphics, limit (currently) to server id 1, for both ded master
    // (parent partition rank 1) and peer partitions (parent partition rank 0)
    if (mgr.graph2DFlag && iterator_server_id == 1) { // initialize the 2D plots
      dakota_graphics.create_plots_2d(vars, resp);
      auto_log = true;
    }

    // initialize the tabular data file on all iterator masters
    if (mgr.tabularDataFlag) {
      mgr.create_tabular_datastream(vars, resp);
      auto_log = true;
    }

    if (auto_log) // turn out automatic graphics logging
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


int Iterator::num_samples() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->num_samples();
  else  // default for Minimizers / MetaIterators
    return 0;
}


void Iterator::
sampling_reset(int min_samples, bool all_data_flag, bool stats_flag)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->sampling_reset(min_samples, all_data_flag, stats_flag);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine sampling_reset() virtual "
         << "fn.\nThis iterator does not support sampling." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}

void Iterator::
sampling_reference(int samples_ref)
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->sampling_reference(samples_ref);
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine sampling_reference() "
	 << "virtual fn.\nThis iterator does not support sampling."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
}

void Iterator::
sampling_increment()
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->sampling_increment();
  else { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine sampling_increment() "
	 << "virtual fn.\nThis iterator does not support incremental sampling."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
}

unsigned short Iterator::sampling_scheme() const
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine sampling_scheme() virtual "
         << "fn.\nThis iterator does not support sampling." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return iteratorRep->sampling_scheme(); // envelope fwd to letter
}


const Model& Iterator::algorithm_space_model() const
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.!
    Cerr << "Error: letter class does not redefine algorithm_space_model() "
         << "virtual fn.\nThis iterator does not support a single model "
	 << "instance." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  else // envelope fwd to letter
    return iteratorRep->algorithm_space_model();
}


/** This is used to avoid clashes in state between non-object-oriented
    (i.e., F77, C) iterator executions, when such iterators could
    potentially be executing simulataneously (e.g., nested execution).
    It is not an issue (and a used method is not reported) in cases
    where a helper execution is completed before a lower level one
    could be initiated; an example of this is DIRECT for maximization
    of expected improvement: the EIF maximization is completed before
    a new point evaluation (which could include nested iteration) is
    performed. */
unsigned short Iterator::uses_method() const
{
  if (iteratorRep) // envelope fwd to letter
    return iteratorRep->uses_method();
  else // default definition (letter lacking redefinition of virtual fn.)
    return DEFAULT_METHOD; // 0: no enabling iterator for this iterator
}


void Iterator::method_recourse()
{
  if (iteratorRep) // envelope fwd to letter
    iteratorRep->method_recourse();
  else { // default definition (letter lacking redefinition of virtual fn.)
    Cerr << "Error: no method recourse defined for detected method conflict.\n"
	 << "       Please revise method selections." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


const VariablesArray& Iterator::all_variables()
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: letter class does not redefine all_variables() virtual fn."
         << "\n       This iterator does not support variables histories."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return iteratorRep->all_variables(); // envelope fwd to letter
}


const RealMatrix& Iterator::all_samples()
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: letter class does not redefine all_samples() virtual fn."
         << "\n       This iterator does not support sample histories."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return iteratorRep->all_samples(); // envelope fwd to letter
}


const IntResponseMap& Iterator::all_responses() const
{
  if (!iteratorRep) { // letter lacking redefinition of virtual fn.
    Cerr << "Error: letter class does not redefine all_responses() virtual fn."
         << "\n       This iterator does not support response histories."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  return iteratorRep->all_responses(); // envelope fwd to letter
}


bool Iterator::compact_mode() const
{ return (iteratorRep) ? iteratorRep->compact_mode() : false; }


IntIntPair Iterator::estimate_partition_bounds()
{
  return (iteratorRep) ?
    iteratorRep->estimate_partition_bounds() : // MetaIterators
    iteratedModel.estimate_partition_bounds(maxEvalConcurrency); // default defn
}


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
  return(boost::make_tuple(method_enum_to_string(methodName),
			   method_id(), execNum));
}


//void Iterator::pre_output(const String& filename)
void Iterator::pre_output()
{
  if (iteratorRep)
    iteratorRep->pre_output();
  else {
    // distinguish between defaulted pre-run and user-specified
    if (!parallelLib.command_line_user_modes())
      return;

    const String& filename = parallelLib.command_line_pre_run_output();
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
    if (!parallelLib.command_line_user_modes())
      return;

    const String& filename = parallelLib.command_line_post_run_input();
    if (outputLevel > QUIET_OUTPUT) {
      if (filename.empty())
	Cout << "\nPost-run phase initialized: no input requested.\n"
	     << std::endl;
      else {
	// this should be unreachable due to command-line parsing
	Cerr << "\nError: method " << method_enum_to_string(methodName)
	     << " does not support post-run file input." << std::endl;
	abort_handler(METHOD_ERROR);
      }
    }
  }
}

/** This prepend may need to become a virtual function if the tagging
    should propagate to other subModels or helper Iterators an
    Iterator may contain. */
void Iterator::eval_tag_prefix(const String& eval_id_str)
{
  if (iteratorRep)
    iteratorRep->eval_tag_prefix(eval_id_str);
  else
    iteratedModel.eval_tag_prefix(eval_id_str);
}

} // namespace Dakota
