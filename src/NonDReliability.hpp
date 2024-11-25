/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_RELIABILITY_H
#define NOND_RELIABILITY_H

#include <type_traits>
#include "dakota_system_defs.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaNonD.hpp"
#include "DataFitSurrModel.hpp"
#include "ProbabilityTransformModel.hpp"
#include "ParallelLibrary.hpp"

namespace Dakota {


/// Base class for the reliability methods within DAKOTA/UQ

/** The NonDReliability class provides a base class for
    NonDLocalReliability, which implements traditional MPP-based
    reliability methods, and NonDGlobalReliability, which implements
    global limit state search using Gaussian process models in
    combination with multimodal importance sampling. */

//template <typename T>
//using EnableIfDFSorPTM = typename std::enable_if<std::is_same<T, DataFitSurrModel>::value || 
//  std::is_same<T, ProbabilityTransformModel>::value >;

//template <typename T, typename Enable = void>
//class NonDReliability;

template<typename T>
class NonDReliability: public NonD
{
public:

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDReliability(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDReliability() override;                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void nested_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2) override;

  bool resize() override;
  //void pre_run();
  void post_run(std::ostream& s) override;

  const Model& algorithm_space_model() const override;

  //
  //- Heading: Data members
  //

  /// Model representing the limit state in u-space, after any
  /// recastings and data fits
  std::shared_ptr<T> uSpaceModel;
  /// RecastModel which formulates the optimization subproblem: RIA, PMA, EGO
  Model mppModel;
  /// Iterator which optimizes the mppModel
  Iterator mppOptimizer;

  /// the MPP search type selection:
  /// Local: MV, x/u-space {AMV,AMV+,TANA,QMEA} or NO_APPROX
  /// Global x/u-space EGRA
  unsigned short mppSearchType;

  /// importance sampling instance used to compute/refine probabilities
  Iterator importanceSampler;
  /// integration refinement type (NO_INT_REFINE, IS, AIS, or MMAIS)
  /// provided by \c refinement specification
  unsigned short integrationRefinement;
  // number of samples for integration refinement
  //int refinementSamples;
  // random seed for integration refinement
  //int refinementSeed;

  /// number of invocations of core_run()
  size_t numRelAnalyses;

  /// number of approximation cycles for the current respFnCount/levelCount
  size_t approxIters;
  /// indicates convergence of approximation-based iterations
  bool approxConverged;

  /// counter for which response function is being analyzed
  int respFnCount;
  /// counter for which response/probability level is being analyzed
  size_t levelCount;
  /// counter for which final statistic is being computed
  size_t statCount;

  /// flag indicating maximization of G(u) within PMA formulation
  bool pmaMaximizeG;

  /// the {response,reliability,generalized reliability} level target
  /// for the current response function
  Real requestedTargetLevel;
};


template<typename T>
NonDReliability<T>::NonDReliability(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model),
  mppSearchType(probDescDB.get_ushort("method.sub_method")),
  integrationRefinement(
    probDescDB.get_ushort("method.nond.integration_refinement")),
  numRelAnalyses(0)
  //refinementSamples(probDescDB.get_int("method.samples")),
  //refinementSeed(probDescDB.get_int("method.random_seed"))
{
  static_assert(std::is_same<T, DataFitSurrModel>::value || std::is_same<T, ProbabilityTransformModel>::value);
  // Check for suitable distribution types.
  if (numDiscreteIntVars || numDiscreteStringVars || numDiscreteRealVars) {
    Cerr << "Error: discrete random variables are not supported in reliability "
	 << "methods." << std::endl;
    abort_handler(-1);
  }

  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);
}

template<typename T>
NonDReliability<T>::~NonDReliability()
{ }

template<typename T>
inline void NonDReliability<T>::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  uSpaceModel->nested_variable_mappings(c_index1, di_index1, ds_index1,
				       dr_index1, c_target2, di_target2,
				       ds_target2, dr_target2);
}


template<typename T>
bool NonDReliability<T>::resize()
{
  bool parent_reinit_comms = NonD::resize();

  initialize_final_statistics(); // default statistics set

  // RealVectors are sized within derived classes
  computedRespLevels.resize(numFunctions);
  computedProbLevels.resize(numFunctions);
  computedGenRelLevels.resize(numFunctions);

  return parent_reinit_comms;
}

template<typename T>
inline const Model& NonDReliability<T>::algorithm_space_model() const
{ 
  return *uSpaceModel;
}

template<typename T>
void NonDReliability<T>::post_run(std::ostream& s)
{
  ++numRelAnalyses;

  if (!mppModel.is_null() && mppModel.mapping_initialized()) {
    /*bool var_size_changed =*/ mppModel.finalize_mapping();
    //if (var_size_changed) resize();
  }

  Analyzer::post_run(s);
}



} // namespace Dakota

#endif
