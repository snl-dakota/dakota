/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include "DakotaNonD.H"

namespace Dakota {


/// Base class for the reliability methods within DAKOTA/UQ

/** The NonDReliability class provides a base class for
    NonDLocalReliability, which implements traditional MPP-based
    reliability methods, and NonDGlobalReliability, which implements
    global limit state search using Gaussian process models in
    combination with multimodal importance sampling. */

class NonDReliability: public NonD
{
protected:

  //
  //- Heading: Constructors and destructor
  //

  NonDReliability(Model& model); ///< constructor
  ~NonDReliability();            ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  /// initialize graphics customized for reliability methods
  void initialize_graphics(bool graph_2d, bool tabular_data,
			   const String& tabular_file);

  const Model& algorithm_space_model() const;

  //
  //- Heading: New virtual functions
  //

  /// update requestedCDFRelLevel for use in PMA_constraint_eval()
  virtual void update_pma_reliability_level();

  //
  //- Heading: Objective/constraint evaluators passed to RecastModel
  //

  /// static function used as the objective function in the Reliability
  /// Index Approach (RIA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the objective function of (norm u)^2.
  static void RIA_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  /// static function used as the constraint function in the Reliability
  /// Index Approach (RIA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the constraint of G(u) = response level.
  static void RIA_constraint_eval(const Variables& sub_model_vars,
				  const Variables& recast_vars,
				  const Response& sub_model_response,
				  Response& recast_response);

  /// static function used as the objective function in the Performance
  /// Measure Approach (PMA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the objective function of G(u).
  static void PMA_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  /// static function used as the constraint function in the Performance
  /// Measure Approach (PMA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the constraint of (norm u)^2 = beta^2.
  static void PMA_constraint_eval(const Variables& sub_model_vars,
				  const Variables& recast_vars,
				  const Response& sub_model_response,
				  Response& recast_response);

  //
  //- Heading: other mapping functions passed to RecastModel
  //

  /// static function used to augment the sub-model ASV requests when a
  /// beta-bar constraint target update is required for second-order PMA
  static void PMA2_set_mapping(const Variables& recast_vars,
			       const ActiveSet& recast_set,
			       ActiveSet& sub_model_set);

  //
  //- Heading: Data members
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDReliability* nondRelInstance;

  /// Model representing the limit state in u-space, after any
  /// recastings and data fits
  Model uSpaceModel;
  /// RecastModel which formulates the optimization subproblem: RIA, PMA, EGO
  Model mppModel;
  /// Iterator which optimizes the mppModel
  Iterator mppOptimizer;

  /// the MPP search type selection: MV, x/u-space AMV, x/u-space AMV+,
  /// x/u-space TANA, x/u-space EGO, or NO_APPROX
  short mppSearchType;

  /// importance sampling instance used to compute/refine probabilities
  Iterator importanceSampler;
  /// integration refinement type (NO_INT_REFINE, IS, AIS, or MMAIS)
  /// provided by \c refinement specification
  short integrationRefinement;
  // number of samples for integration refinement
  //int refinementSamples;
  // random seed for integration refinement
  //int refinementSeed;

  /// number of invocations of quantify_uncertainty()
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

  /// the response level target for the current response function
  Real requestedRespLevel;
  /// the CDF probability level target for the current response function
  Real requestedCDFProbLevel;
  /// the CDF reliability level target for the current response function
  Real requestedCDFRelLevel;
  /// output response level calculated
  Real computedRespLevel;
  // output probability level calculated
  //Real computedProbLevel;
  /// output reliability level calculated
  Real computedRelLevel;
};


inline const Model& NonDReliability::algorithm_space_model() const
{ return uSpaceModel; }


inline void NonDReliability::update_pma_reliability_level()
{ }

} // namespace Dakota

#endif
