/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDReliability
//- Description: Base class for reliability methods within DAKOTA/UQ
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_RELIABILITY_H
#define NOND_RELIABILITY_H

#include "DakotaNonD.hpp"

namespace Dakota {


/// Base class for the reliability methods within DAKOTA/UQ

/** The NonDReliability class provides a base class for
    NonDLocalReliability, which implements traditional MPP-based
    reliability methods, and NonDGlobalReliability, which implements
    global limit state search using Gaussian process models in
    combination with multimodal importance sampling. */

class NonDReliability: public NonD
{
public:
  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDReliability(ProblemDescDB& problem_db, Model& model); ///< constructor
  ~NonDReliability();                                       ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  /// initialize graphics customized for reliability methods
  void initialize_graphics(int iterator_server_id = 1);

  const Model& algorithm_space_model() const;

  //
  //- Heading: Data members
  //

  /// Model representing the limit state in u-space, after any
  /// recastings and data fits
  Model uSpaceModel;
  /// RecastModel which formulates the optimization subproblem: RIA, PMA, EGO
  Model mppModel;
  /// Iterator which optimizes the mppModel
  Iterator mppOptimizer;

  /// the MPP search type selection: MV, x/u-space AMV, x/u-space AMV+,
  /// x/u-space TANA, x/u-space EGO, or NO_APPROX
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


inline const Model& NonDReliability::algorithm_space_model() const
{ return uSpaceModel; }

} // namespace Dakota

#endif
