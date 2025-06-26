/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "iterator_utils.hpp"

#include "CollabHybridMetaIterator.hpp"
#include "ConcurrentMetaIterator.hpp"
#include "DakotaIterator.hpp"
#include "DataFitSurrBasedLocalMinimizer.hpp"
#include "DataMethod.hpp"
#include "EffGlobalMinimizer.hpp"
#include "EmbedHybridMetaIterator.hpp"
#include "HierarchSurrBasedLocalMinimizer.hpp"
#include "MetaIterator.hpp"
#include "NonDAdaptImpSampling.hpp"
#include "NonDGPImpSampling.hpp"
#include "NonDGenACVSampling.hpp"
#include "NonDGlobalEvidence.hpp"
#include "NonDGlobalReliability.hpp"
#include "NonDGlobalSingleInterval.hpp"
#include "NonDLHSEvidence.hpp"
#include "NonDLHSSampling.hpp"
#include "NonDLHSSingleInterval.hpp"
#include "NonDLocalEvidence.hpp"
#include "NonDLocalReliability.hpp"
#include "NonDLocalSingleInterval.hpp"
#include "NonDMultifidelitySampling.hpp"
#include "NonDMultilevBLUESampling.hpp"
#include "NonDMultilevControlVarSampling.hpp"
#include "NonDMultilevelPolynomialChaos.hpp"
#include "NonDMultilevelStochCollocation.hpp"
#include "NonDPOFDarts.hpp"
#include "NonDPolynomialChaos.hpp"
#include "NonDRKDDarts.hpp"
#include "NonDStochCollocation.hpp"
#include "NonDSurrogateExpansion.hpp"
#include "NonlinearCGOptimizer.hpp"
#include "ParamStudy.hpp"
#include "RichExtrapVerification.hpp"
#include "SeqHybridMetaIterator.hpp"
#include "SurrBasedGlobalMinimizer.hpp"
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
#include "NLSSOLLeastSq.hpp"
#include "NPSOLOptimizer.hpp"
#endif
#ifdef HAVE_NLPQL
#include "NLPQLPOptimizer.hpp"
#endif
#ifdef HAVE_NL2SOL
#include "NL2SOLLeastSq.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLLeastSq.hpp"
#include "SNLLOptimizer.hpp"
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
#ifdef HAVE_ROL
#include "ROLOptimizer.hpp"
#endif
#ifdef HAVE_DEMO_TPL
#include "DemoOptimizer.hpp"
#endif
#ifdef HAVE_JEGA
#include "JEGAOptimizer.hpp"
#endif
#ifdef HAVE_C3
#include "NonDC3FunctionTrain.hpp"
#include "NonDMultilevelFunctionTrain.hpp"
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
#ifdef HAVE_MUQ
#include "NonDMUQBayesCalibration.hpp"
#endif
#include "NonDWASABIBayesCalibration.hpp"
#include "OptDartsOptimizer.hpp"

namespace Dakota {
namespace IteratorUtils {
/** Used only by the envelope constructor to initialize iteratorRep to
the appropriate derived type, as given by the DB's method_name.
Supports all iterators and meta-iterators.  These instantiations
will NOT recurse on the Iterator(problem_db) constructor due to
the use of BaseConstructor. */
std::shared_ptr<Iterator> get_iterator(ProblemDescDB& problem_db,
                                       ParallelLibrary& parallel_lib) {
  unsigned short method_name = problem_db.get_ushort("method.algorithm");

  // Meta-iterators support special constructors that are not bound to a Model
  // instance for top-level instantiation of general meta-iteration.  However,
  // they also support construction with a provided Model to allow use as a
  // component within an Iterator recursion.
  switch (method_name) {
    case HYBRID:
      switch (problem_db.get_ushort("method.sub_method")) {
        case SUBMETHOD_COLLABORATIVE:
          return std::make_shared<CollabHybridMetaIterator>(problem_db,
                                                            parallel_lib);
          break;
        case SUBMETHOD_EMBEDDED:
          return std::make_shared<EmbedHybridMetaIterator>(problem_db,
                                                           parallel_lib);
          break;
        case SUBMETHOD_SEQUENTIAL:
          return std::make_shared<SeqHybridMetaIterator>(problem_db,
                                                         parallel_lib);
          break;
        default:
          // rather than create additional derived constructors for
          // non-meta-iterators that differ only in creation of their own Model
          // instance, perform the Model instantiation here and leverage the
          // existing constructors.
          return get_iterator(problem_db, parallel_lib,
                              Model::get_model(problem_db, parallel_lib));
          break;
      }
      break;
    case PARETO_SET:
    case MULTI_START:
      return std::make_shared<ConcurrentMetaIterator>(problem_db, parallel_lib);
      break;
    default:
      // rather than create additional derived constructors for
      // non-meta-iterators that differ only in creation of their own Model
      // instance, perform the Model instantiation here and leverage the
      // existing constructors.
      return get_iterator(problem_db, parallel_lib,
                          Model::get_model(problem_db, parallel_lib));
      break;
  }
}

/** Used only by the envelope constructor to initialize iteratorRep to
the appropriate derived type.  Alternate construction of meta-iterators
is supported to enable use of meta-iterators as components.  These
instantiations will NOT recurse on the Iterator(problem_db, parallel_lib, model)
constructor due to the use of BaseConstructor. */
std::shared_ptr<Iterator> get_iterator(ProblemDescDB& problem_db,
                                       ParallelLibrary& parallel_lib,
                                       std::shared_ptr<Model> model) {
  unsigned short method_name = problem_db.get_ushort("method.algorithm");

  switch (method_name) {
    case HYBRID:
      switch (problem_db.get_ushort("method.sub_method")) {
        case SUBMETHOD_COLLABORATIVE:
          return std::make_shared<CollabHybridMetaIterator>(
              problem_db, parallel_lib, model);
          break;
        case SUBMETHOD_EMBEDDED:
          return std::make_shared<EmbedHybridMetaIterator>(problem_db,
                                                           parallel_lib, model);
          break;
        case SUBMETHOD_SEQUENTIAL:
          return std::make_shared<SeqHybridMetaIterator>(problem_db,
                                                         parallel_lib, model);
          break;
        default:
          Cerr << "Invalid hybrid meta-iterator type." << std::endl;
          return std::shared_ptr<Iterator>();
          break;
      }
      break;
    case PARETO_SET:
    case MULTI_START:
      return std::make_shared<ConcurrentMetaIterator>(problem_db, parallel_lib,
                                                      model);
      break;
    case CENTERED_PARAMETER_STUDY:
    case LIST_PARAMETER_STUDY:
    case MULTIDIM_PARAMETER_STUDY:
    case VECTOR_PARAMETER_STUDY:
      return std::make_shared<ParamStudy>(problem_db, parallel_lib, model);
      break;
    case RICHARDSON_EXTRAP:
      return std::make_shared<RichExtrapVerification>(problem_db, parallel_lib,
                                                      model);
      break;
    case LOCAL_RELIABILITY:
      return std::make_shared<NonDLocalReliability>(problem_db, parallel_lib,
                                                    model);
      break;
    case LOCAL_INTERVAL_EST:
      return std::make_shared<NonDLocalSingleInterval>(problem_db, parallel_lib,
                                                       model);
      break;
    case LOCAL_EVIDENCE:
      return std::make_shared<NonDLocalEvidence>(problem_db, parallel_lib,
                                                 model);
      break;
    case GLOBAL_RELIABILITY:
      return std::make_shared<NonDGlobalReliability>(problem_db, parallel_lib,
                                                     model);
      break;
    case GLOBAL_INTERVAL_EST:
      switch (problem_db.get_ushort("method.nond.opt_subproblem_solver")) {
        case SUBMETHOD_LHS:
          return std::make_shared<NonDLHSSingleInterval>(problem_db,
                                                         parallel_lib, model);
          break;
        default:
          return std::make_shared<NonDGlobalSingleInterval>(
              problem_db, parallel_lib, model);
          break;
      }
      break;
    case GLOBAL_EVIDENCE:
      switch (problem_db.get_ushort("method.nond.opt_subproblem_solver")) {
        case SUBMETHOD_LHS:
          return std::make_shared<NonDLHSEvidence>(problem_db, parallel_lib,
                                                   model);
          break;
        default:
          return std::make_shared<NonDGlobalEvidence>(problem_db, parallel_lib,
                                                      model);
          break;
      }
      break;
    case POLYNOMIAL_CHAOS:
      return std::make_shared<NonDPolynomialChaos>(problem_db, parallel_lib,
                                                   model);
      break;
    case MULTILEVEL_POLYNOMIAL_CHAOS:
    case MULTIFIDELITY_POLYNOMIAL_CHAOS:
      return std::make_shared<NonDMultilevelPolynomialChaos>(
          problem_db, parallel_lib, model);
      break;
    case STOCH_COLLOCATION:
      return std::make_shared<NonDStochCollocation>(problem_db, parallel_lib,
                                                    model);
      break;
    case MULTIFIDELITY_STOCH_COLLOCATION:
      return std::make_shared<NonDMultilevelStochCollocation>(
          problem_db, parallel_lib, model);
      break;
#ifdef HAVE_C3
    case C3_FUNCTION_TRAIN:
      return std::make_shared<NonDC3FunctionTrain>(problem_db, parallel_lib,
                                                   model);
      break;
    case MULTILEVEL_FUNCTION_TRAIN:
    case MULTIFIDELITY_FUNCTION_TRAIN:
      return std::make_shared<NonDMultilevelFunctionTrain>(problem_db,
                                                           parallel_lib, model);
      break;
#endif
    case SURROGATE_BASED_UQ:
      return std::make_shared<NonDSurrogateExpansion>(problem_db, parallel_lib,
                                                      model);
      break;
    case BAYES_CALIBRATION:
      // TO DO: add sub_method to bayes_calibration specification
      switch (problem_db.get_ushort("method.sub_method")) {
        case SUBMETHOD_GPMSA:
#ifdef HAVE_QUESO_GPMSA
          return std::make_shared<NonDGPMSABayesCalibration>(
              problem_db, parallel_lib, model);
          break;
#else
          Cerr << "\nError: QUESO/GPMSA Bayesian calibration method "
                  "unavailable.\n"
               << "(Not enabled in some Dakota distributions due to dependence "
                  "on "
               << "GSL;\ncan be enabled when compiling from source code.)\n";
          return std::shared_ptr<Iterator>();
          break;
#endif
        case SUBMETHOD_QUESO:
#ifdef HAVE_QUESO
          return std::make_shared<NonDQUESOBayesCalibration>(
              problem_db, parallel_lib, model);
          break;
#else
          Cerr << "\nError: QUESO Bayesian calibration method unavailable.\n"
               << "(Not enabled in some Dakota distributions due to dependence "
                  "on "
               << "GSL;\ncan be enabled when compiling from source code.)\n";
          return std::shared_ptr<Iterator>();
          break;
#endif
#ifdef HAVE_DREAM
        case SUBMETHOD_DREAM: {
          std::shared_ptr<Iterator> sp =
              std::make_shared<NonDDREAMBayesCalibration>(problem_db,
                                                          parallel_lib, model);
          return sp;
          break;
        }
#endif
#ifdef HAVE_MUQ
        case SUBMETHOD_MUQ:
          return std::make_shared<NonDMUQBayesCalibration>(problem_db,
                                                           parallel_lib, model);
          break;
#endif
        case SUBMETHOD_WASABI:
          return std::make_shared<NonDWASABIBayesCalibration>(
              problem_db, parallel_lib, model);
          break;
        default:
          Cerr << "\nError: Bayesian calibration method '"
               << Iterator::submethod_enum_to_string(
                      problem_db.get_ushort("method.sub_method"))
               << "' unavailable.\n";
          return std::shared_ptr<Iterator>();
          break;
      }
      break;
    case GPAIS:
      return std::make_shared<NonDGPImpSampling>(problem_db, parallel_lib,
                                                 model);
      break;
    case POF_DARTS:
      return std::make_shared<NonDPOFDarts>(problem_db, parallel_lib, model);
      break;
    case RKD_DARTS:
      return std::make_shared<NonDRKDDarts>(problem_db, parallel_lib, model);
      break;
    case IMPORTANCE_SAMPLING:
      return std::make_shared<NonDAdaptImpSampling>(problem_db, parallel_lib,
                                                    model);
      break;
#ifdef HAVE_ADAPTIVE_SAMPLING
    case ADAPTIVE_SAMPLING:
      return std::make_shared<NonDAdaptiveSampling>(problem_db, parallel_lib,
                                                    model);
      break;
#endif
    // #ifdef HAVE_MUQ
    //   case MUQ_SAMPLING:
    //     return std::make_shared<NonDMUQBayesCalibration>(problem_db,
    //     parallel_lib, model);break;
    // #endif
    case RANDOM_SAMPLING: {
      std::shared_ptr<Iterator> sp =
          std::make_shared<NonDLHSSampling>(problem_db, parallel_lib, model);
      return sp;
      break;
    }
    case MULTILEVEL_SAMPLING:
      // Similar to MFMC below, spec options could trigger promotion to GenACV
      // (which is then restricted to hierarchical DAGs for MLMC consistency)
      // Note that recursion/selection is not available w/o weighting.
      if (problem_db.get_ushort("method.sub_method") == SUBMETHOD_WEIGHTED_MLMC)
        return std::make_shared<NonDGenACVSampling>(problem_db, parallel_lib,
                                                    model);
      else
        return std::make_shared<NonDMultilevelSampling>(problem_db,
                                                        parallel_lib, model);
      break;
    case MULTIFIDELITY_SAMPLING:
      if (problem_db.get_short("method.nond.search_model_graphs.recursion") ||
          problem_db.get_short("method.nond.search_model_graphs.selection"))
        return std::make_shared<NonDGenACVSampling>(problem_db, parallel_lib,
                                                    model);
      else  // Note that numerical MFMC reorders models on the fly, similar to
            // enumeration of hierarchical DAGs (more efficient, less smooth?)
        return std::make_shared<NonDMultifidelitySampling>(problem_db,
                                                           parallel_lib, model);
      break;
    case MULTILEVEL_MULTIFIDELITY_SAMPLING:
      return std::make_shared<NonDMultilevControlVarSampling>(
          problem_db, parallel_lib, model);
      break;
    case APPROX_CONTROL_VARIATE:
      if (problem_db.get_short("method.nond.search_model_graphs.recursion") ||
          problem_db.get_short("method.nond.search_model_graphs.selection") ||
          problem_db.get_ushort("method.sub_method") == SUBMETHOD_ACV_RD)
        // RD is promoted since we want MLMC hierarch rather than ACV peer DAG
        return std::make_shared<NonDGenACVSampling>(problem_db, parallel_lib,
                                                    model);
      else
        return std::make_shared<NonDACVSampling>(problem_db, parallel_lib,
                                                 model);
      break;
    case GEN_APPROX_CONTROL_VARIATE:  // not currently reachable
      return std::make_shared<NonDGenACVSampling>(problem_db, parallel_lib,
                                                  model);
      break;
    case MULTILEVEL_BLUE:
      return std::make_shared<NonDMultilevBLUESampling>(problem_db,
                                                        parallel_lib, model);
      break;
    case DATA_FIT_SURROGATE_BASED_LOCAL:
      return std::make_shared<DataFitSurrBasedLocalMinimizer>(
          problem_db, parallel_lib, model);
      break;
    case HIERARCH_SURROGATE_BASED_LOCAL:
      return std::make_shared<HierarchSurrBasedLocalMinimizer>(
          problem_db, parallel_lib, model);
      break;
    case SURROGATE_BASED_LOCAL:
      if (model->surrogate_type() == "ensemble")
        return std::make_shared<HierarchSurrBasedLocalMinimizer>(
            problem_db, parallel_lib, model);
      else
        return std::make_shared<DataFitSurrBasedLocalMinimizer>(
            problem_db, parallel_lib, model);
      break;
    case SURROGATE_BASED_GLOBAL:
      return std::make_shared<SurrBasedGlobalMinimizer>(problem_db,
                                                        parallel_lib, model);
      break;
    case EFFICIENT_GLOBAL:
      return std::make_shared<EffGlobalMinimizer>(problem_db, parallel_lib,
                                                  model);
      break;
    case NONLINEAR_CG:
      return std::make_shared<NonlinearCGOptimizer>(problem_db, parallel_lib,
                                                    model);
      break;
    case GENIE_OPT_DARTS:
      return std::make_shared<OptDartsOptimizer>(problem_db, parallel_lib,
                                                 model);
      break;
    case GENIE_DIRECT:
      return std::make_shared<OptDartsOptimizer>(problem_db, parallel_lib,
                                                 model);
      break;
#ifdef HAVE_OPTPP
    case OPTPP_G_NEWTON:
      return std::make_shared<SNLLLeastSq>(problem_db, parallel_lib, model);
      break;
    case OPTPP_Q_NEWTON:
    case OPTPP_FD_NEWTON:
    case OPTPP_NEWTON:
    case OPTPP_CG:
    case OPTPP_PDS:
      return std::make_shared<SNLLOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef DAKOTA_HOPS
    case ASYNCH_PATTERN_SEARCH:
      return std::make_shared<APPSOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_ACRO
    case COLINY_BETA:
    case COLINY_COBYLA:
    case COLINY_DIRECT:
    case COLINY_EA:
    case COLINY_PATTERN_SEARCH:
    case COLINY_SOLIS_WETS:
      return std::make_shared<COLINOptimizer>(problem_db, parallel_lib, model);
      break;
    case BRANCH_AND_BOUND:
      return std::make_shared<PebbldMinimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_JEGA
    case MOGA:
    case SOGA:
      return std::make_shared<JEGAOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef DAKOTA_DL_SOLVER
    case DL_SOLVER:
      return std::make_shared<DLSolver>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_NOMAD
    case MESH_ADAPTIVE_SEARCH:
      return std::make_shared<NomadOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_NOWPAC
    case MIT_NOWPAC:
    case MIT_SNOWPAC:
      return std::make_shared<NOWPACOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_NPSOL
    case NPSOL_SQP:
      return std::make_shared<NPSOLOptimizer>(problem_db, parallel_lib, model);
      break;
    case NLSSOL_SQP:
      return std::make_shared<NLSSOLLeastSq>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_NLPQL
    case NLPQL_SQP:
      return std::make_shared<NLPQLPOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_NL2SOL
    case NL2SOL:
      return std::make_shared<NL2SOLLeastSq>(problem_db, parallel_lib, model);
      break;
#endif
// #ifdef DAKOTA_RSQP
//   case REDUCED_SQP: return std::make_shared<rSQPOptimizer>(problem_db,
//   parallel_lib, model);   break;
// #endif
#ifdef HAVE_DOT
    case DOT_BFGS:
    case DOT_FRCG:
    case DOT_MMFD:
    case DOT_SLP:
    case DOT_SQP:
      return std::make_shared<DOTOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_CONMIN
    case CONMIN_FRCG:
    case CONMIN_MFD:
      return std::make_shared<CONMINOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_DDACE
    case DACE:
      return std::make_shared<DDACEDesignCompExp>(problem_db, parallel_lib,
                                                  model);
      break;
#endif
#ifdef HAVE_FSUDACE
    case FSU_CVT:
    case FSU_HALTON:
    case FSU_HAMMERSLEY:
      return std::make_shared<FSUDesignCompExp>(problem_db, parallel_lib,
                                                model);
      break;
#endif
#ifdef HAVE_PSUADE
    case PSUADE_MOAT:
      return std::make_shared<PSUADEDesignCompExp>(problem_db, parallel_lib,
                                                   model);
      break;
#endif
#ifdef HAVE_NCSU
    case NCSU_DIRECT:
      return std::make_shared<NCSUOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_ROL
    case ROL:
      return std::make_shared<ROLOptimizer>(problem_db, parallel_lib, model);
      break;
#endif
#ifdef HAVE_DEMO_TPL
    case DEMO_TPL:
      return std::make_shared<DemoTPLOptimizer>(problem_db, parallel_lib,
                                                model);
      break;
#endif
    default:
      switch (method_name) {
        case NPSOL_SQP:
        case NLPQL_SQP:
        case DOT_BFGS:
        case DOT_FRCG:
        case DOT_MMFD:
        case DOT_SLP:
        case DOT_SQP:
          Cerr << "Method " << Iterator::method_enum_to_string(method_name)
               << " not available; requires a separate software license."
               << "\nCONMIN or OPT++ methods may be suitable alternatives.\n";
          break;
        case NLSSOL_SQP:
          Cerr << "Method " << Iterator::method_enum_to_string(method_name)
               << " not available; requires a separate software license."
               << "\nnl2sol or optpp_g_newton may be suitable alternatives.\n";
          break;
        default:
          Cerr << "Method " << Iterator::method_enum_to_string(method_name)
               << " not available.\n";
          break;
      }
      return std::shared_ptr<Iterator>();
      break;
  }
}

/** Used only by the envelope constructor to initialize iteratorRep to
    the appropriate derived type, as given by the passed method_string.
    Lightweight instantiations by name are supported by a subset of
    Iterators (primarily Minimizers). */
std::shared_ptr<Iterator> get_iterator(const String& method_string,
                                       std::shared_ptr<Model> model) {
  // These instantiations will NOT recurse on the Iterator(model)
  // constructor due to the use of BaseConstructor.

  // if (method_string == "data_fit_surrogate_based_local") {
  //   return std::make_shared<DataFitSurrBasedLocalMinimizer(model);
  // else if (method_string == "hierarch_surrogate_based_local") {
  //   return std::make_shared<HierarchSurrBasedLocalMinimizer(model);
  // else if (method_string == "surrogate_based_local") {
  //   return (model.surrogate_type() == "hierarchical) ?
  //     std::make_shared<HierarchSurrBasedLocalMinimizer(model) :
  //     std::make_shared<DataFitSurrBasedLocalMinimizer(model);
  // else if (method_string == "surrogate_based_global")
  //   return std::make_shared<SurrBasedGlobalMinimizer(model);
  // else if (method_string == "efficient_global")
  //   return std::make_shared<EffGlobalMinimizer(model);

  if (strbegins(method_string, "genie_"))
    return std::make_shared<OptDartsOptimizer>(model);
#ifdef HAVE_OPTPP
  else if (strbegins(method_string, "optpp_")) {
    if (strends(method_string, "_g_newton"))
      return std::make_shared<SNLLLeastSq>(method_string, model);
    else
      return std::make_shared<SNLLOptimizer>(method_string, model);
  }
#endif
#ifdef DAKOTA_HOPS
  else if (method_string == "asynch_pattern_search")
    return std::make_shared<APPSOptimizer>(model);
#endif
#ifdef HAVE_ACRO
  else if (strbegins(method_string, "coliny_"))
    return std::make_shared<COLINOptimizer>(method_string, model);
  else if (method_string == "branch_and_bound")
    return std::make_shared<PebbldMinimizer>(model);
#endif
#ifdef HAVE_JEGA
// else if (method_string == "moga" || method_string == "soga")
//   return std::make_shared<JEGAOptimizer>(model);
#endif
#ifdef DAKOTA_DL_SOLVER
// else if (method_string == "dl_solver")
//   return std::make_shared<DLSolver>(model);
#endif
#ifdef HAVE_NOMAD
  else if (method_string == "mesh_adaptive_search")
    return std::make_shared<NomadOptimizer>(model);
#endif
#ifdef HAVE_NOWPAC
  else if (strends(method_string, "nowpac"))
    return std::make_shared<NOWPACOptimizer>(model);
#endif
#ifdef HAVE_NPSOL
  else if (method_string == "npsol_sqp")
    return std::make_shared<NPSOLOptimizer>(model);
  else if (method_string == "nlssol_sqp")
    return std::make_shared<NLSSOLLeastSq>(model);
#endif
#ifdef HAVE_NLPQL
  else if (method_string == "nlpql_sqp")
    return std::make_shared<NLPQLPOptimizer>(model);
#endif
#ifdef HAVE_NL2SOL
  else if (method_string == "nl2sol")
    return std::make_shared<NL2SOLLeastSq>(model);
#endif
#ifdef HAVE_DOT
  else if (strbegins(method_string, "dot_"))
    return std::make_shared<DOTOptimizer>(method_string, model);
#endif
#ifdef HAVE_CONMIN
  else if (strbegins(method_string, "conmin_"))
    return std::make_shared<CONMINOptimizer>(method_string, model);
#endif
#ifdef HAVE_NCSU
  else if (method_string == "ncsu_direct")
    return std::make_shared<NCSUOptimizer>(model);
#endif
#ifdef HAVE_NCSU
  else if (method_string == "ncsu_direct")
    return std::make_shared<NCSUOptimizer>(model);
#endif
#ifdef HAVE_ROL
  else if (method_string == "rol")
    return std::make_shared<ROLOptimizer>(method_string, model);
#endif
  else {
    if (method_string == "npsol_sqp" || method_string == "nlpql_sqp" ||
        strbegins(method_string, "dot_"))
      Cerr << "Method " << method_string
           << " not available by name; requires a separate software license."
           << "\nCONMIN or OPT++ methods may be suitable alternatives.\n";
    else if (method_string == "nlssol_sqp")
      Cerr << "Method " << method_string
           << " not available by name; requires a separate software license."
           << "\nnl2sol may be a suitable alternative.\n";
    else
      Cerr << "Method " << method_string << " not available by name.\n";
    return std::shared_ptr<Iterator>();
  }
}

}  // namespace IteratorUtils
}  // namespace Dakota
