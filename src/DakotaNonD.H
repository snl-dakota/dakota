/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonD
//- Description: Base class for NonDeterministic branch
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

#ifndef DAKOTA_NOND_H
#define DAKOTA_NOND_H

#include "DakotaAnalyzer.H"
#ifdef DAKOTA_PECOS
#include "ProbabilityTransformation.hpp"
#endif // DAKOTA_PECOS

//#define DERIV_DEBUG


namespace Dakota {


/// Base class for all nondetermistic iterators (the DAKOTA/UQ branch).

/** The base class for nondeterministic iterators consolidates
    uncertain variable data and probabilistic utilities for inherited
    classes. */

class NonD: public Analyzer
{
public:

  //
  //- Heading: Utility routines
  //

  /// initialize natafTransform based on distribution data from iteratedModel
  void initialize_random_variables(short u_space_type);
#ifdef DAKOTA_PECOS
  /// alternate form: initialize natafTransform based on incoming data
  void initialize_random_variables(
    const Pecos::ProbabilityTransformation& transform);
#endif // DAKOTA_PECOS

  /// set requestedRespLevels, requestedProbLevels, requestedRelLevels,
  /// requestedGenRelLevels, respLevelTarget, and cdfFlag (used in
  /// combination with alternate ctors)
  void requested_levels(const RealVectorArray& req_resp_levels,
			const RealVectorArray& req_prob_levels,
			const RealVectorArray& req_rel_levels,
			const RealVectorArray& req_gen_rel_levels,
			short resp_lev_target, bool cdf_flag);

  /// set meanStats and stdDevStats
  void moments(const RealVector& means, const RealVector& std_devs);

  /// set distParamDerivs
  void distribution_parameter_derivatives(bool dist_param_derivs);

protected:

  //
  //- Heading: Constructors and destructor
  //

  NonD(Model& model); ///< constructor

  /// alternate constructor for sample generation and evaluation "on the fly"
  NonD(NoDBBaseConstructor, Model& model);
  /// alternate constructor for sample generation "on the fly"
  NonD(NoDBBaseConstructor, const RealVector& lower_bnds,
       const RealVector& upper_bnds);

  ~NonD(); ///< destructor

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void run();
  void finalize_run();

  // return the final uncertain variables from the nondeterministic iteration
  //const Variables& variables_results() const;
  /// return the final statistics from the nondeterministic iteration
  const Response& response_results() const;
  /// set the active set within finalStatistics
  void response_results_active_set(const ActiveSet& set);

  //
  //- Heading: New virtual member functions
  //

  /// performs a forward uncertainty propagation of parameter
  /// distributions into response statistics
  virtual void quantify_uncertainty() = 0;

  /// initializes finalStatistics for storing NonD final results
  virtual void initialize_final_statistics();

  //
  //- Heading: Utility routines
  //

  /// initializes ranVarTypesX and ranVarTypesU within natafTransform
  void initialize_random_variable_types(short u_space_type);
  /// initializes ranVarMeansX, ranVarStdDevsX, ranVarLowerBndsX,
  /// ranVarUpperBndsX, and ranVarAddtlParamsX within natafTransform
  void initialize_random_variable_parameters();
  /// initializes finalStatistics::functionGradients
  void initialize_final_statistics_gradients();

  /// static function for RecastModels used to map u-space variables
  /// from NonD Iterators to x-space variables for Model evaluations.
  static void vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars);

  /// static function for RecastModels used to map u-space ActiveSets
  /// from NonD Iterators to x-space ActiveSets for Model evaluations.
  static void set_u_to_x_mapping(const ActiveSet& u_set, ActiveSet& x_set);

  /// static function for RecastModels used to map x-space responses from
  /// Model evaluations to u-space responses for return to NonD Iterators.
  static void resp_x_to_u_mapping(const Variables& x_vars,
				  const Variables& u_vars,
				  const Response& x_response,
				  Response& u_response);

  //
  //- Heading: Data members
  //

  /// pointer to the active object instance used within static evaluator
  /// functions in order to avoid the need for static data
  static NonD* nondInstance;
  /// pointer containing previous value of nondInstance
  NonD* prevNondInstance;

  // ---------------------------------------------------------------------------
  // The following attributes are specific to X/Z/U space regardless of whether
  // u-space models are being iterated by a NonD method.  That is, they are
  // _not_ always derived from or in synch with the iteratedModel, and may be
  // passed in to the alternate form of initialize_random_variables() from
  // outside sources.

#ifdef DAKOTA_PECOS
  /// Nonlinear variable transformation that encapsulates the required
  /// data for performing transformations from X -> Z -> U and back.
  Pecos::ProbabilityTransformation natafTransform;
#endif // DAKOTA_PECOS
  // ---------------------------------------------------------------------------


  // ---------------------------------------------------------------------------
  // The following attributes are not dedicated to either X, Z, or U space.
  // Rather, they reflect the native Model space, which could be any of the
  // above.  If a specific X or U variables count is needed, then
  // ProbabilityTransformation::ranVarTypesX/U.count() should be used.

  /// number of continuous design variables (modeled using uniform
  /// distribution for All view modes)
  size_t numContDesVars;
  /// number of discrete integer design variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscIntDesVars;
  /// number of discrete real design variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscRealDesVars;
  /// total number of design variables
  size_t numDesignVars;
  /// number of continuous state variables (modeled using uniform
  /// distribution for All view modes)
  size_t numContStateVars;
  /// number of discrete integer state variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscIntStateVars;
  /// number of discrete real state variables (modeled using discrete
  /// histogram distributions for All view modes)
  size_t numDiscRealStateVars;
  /// total number of state variables
  size_t numStateVars;

  /// number of normal uncertain variables (native space)
  size_t numNormalVars;
  /// number of lognormal uncertain variables (native space)
  size_t numLognormalVars;
  /// number of uniform uncertain variables (native space)
  size_t numUniformVars;
  /// number of loguniform uncertain variables (native space)
  size_t numLoguniformVars;
  /// number of triangular uncertain variables (native space)
  size_t numTriangularVars;
  /// number of exponential uncertain variables (native space)
  size_t numExponentialVars;
  /// number of beta uncertain variables (native space)
  size_t numBetaVars;
  /// number of gamma uncertain variables (native space)
  size_t numGammaVars;
  /// number of gumbel uncertain variables (native space)
  size_t numGumbelVars;
  /// number of frechet uncertain variables (native space)
  size_t numFrechetVars;
  /// number of weibull uncertain variables (native space)
  size_t numWeibullVars;
  /// number of histogram bin uncertain variables (native space)
  size_t numHistogramBinVars;
  /// number of Poisson uncertain variables (native space)
  size_t numPoissonVars;
  /// number of binomial uncertain variables (native space)
  size_t numBinomialVars;
  /// number of negative binomial uncertain variables (native space)
  size_t numNegBinomialVars;
  /// number of geometric uncertain variables (native space)
  size_t numGeometricVars;
  /// number of hypergeometric uncertain variables (native space)
  size_t numHyperGeomVars;
  /// number of histogram point uncertain variables (native space)
  size_t numHistogramPtVars;
  /// number of interval uncertain variables (native space)
  size_t numIntervalVars;

  /// total number of aleatory uncertain variables (native space)
  size_t numContAleatUncVars;
  /// total number of aleatory uncertain variables (native space)
  size_t numDiscIntAleatUncVars;
  /// total number of aleatory uncertain variables (native space)
  size_t numDiscRealAleatUncVars;
  /// total number of aleatory uncertain variables (native space)
  size_t numAleatoryUncVars;
  /// total number of epistemic uncertain variables (native space)
  size_t numContEpistUncVars;
  /// total number of epistemic uncertain variables (native space)
  size_t numDiscIntEpistUncVars;
  /// total number of epistemic uncertain variables (native space)
  size_t numDiscRealEpistUncVars;
  /// total number of epistemic uncertain variables (native space)
  size_t numEpistemicUncVars;
  /// total number of uncertain variables (native space)
  size_t numUncertainVars;
  // ---------------------------------------------------------------------------

  size_t numResponseFunctions; ///< number of response functions

  /// means of response functions (calculated in compute_statistics())
  RealVector meanStats;

  /// std deviations of response functions (calculated in compute_statistics())
  RealVector stdDevStats;

  // map response level z -> probability level p, reliability level beta,
  // or generalized reliability level beta*

  /// requested response levels for all response functions
  RealVectorArray requestedRespLevels;
  /// output probability levels for all response functions resulting
  /// from requestedRespLevels
  RealVectorArray computedProbLevels;
  /// output reliability levels for all response functions resulting
  /// from requestedRespLevels
  RealVectorArray computedRelLevels;
  /// output generalized reliability levels for all response functions
  /// resulting from requestedRespLevels
  RealVectorArray computedGenRelLevels;
  /// indicates mapping of z->p (PROBABILITIES), z->beta (RELIABILITIES),
  /// or z->beta* (GEN_RELIABILITIES)
  short respLevelTarget;

  // map probability level p, reliability level beta, or generalized
  // reliability level beta* -> response level z

  /// requested probability levels for all response functions
  RealVectorArray requestedProbLevels;
  /// requested reliability levels for all response functions
  RealVectorArray requestedRelLevels;
  /// requested generalized reliability levels for all response functions
  RealVectorArray requestedGenRelLevels;
  /// output response levels for all response functions resulting from
  /// requestedProbLevels, requestedRelLevels, or requestedGenRelLevels
  RealVectorArray computedRespLevels;

  /// total number of levels specified within requestedRespLevels,
  /// requestedProbLevels, and requestedRelLevels
  size_t totalLevelRequests;

  /// flag for type of probabilities/reliabilities used in mappings:
  /// cumulative/CDF (true) or complementary/CCDF (false)
  bool cdfFlag;

  /// final statistics from the uncertainty propagation used in strategies:
  /// response means, standard deviations, and probabilities of failure
  Response finalStatistics;

private:

  /// convenience function for distributing a vector of levels among multiple
  /// response functions if a short-hand specification is employed.
  void distribute_levels(RealVectorArray& levels);

  //
  //- Heading: Data members
  //

  /// flags calculation of derivatives with respect to distribution
  /// parameters s within resp_x_to_u_mapping() using the chain rule
  /// df/dx dx/ds.  The default is to calculate derivatives with respect
  /// to standard random variables u using the chain rule df/dx dx/du.
  bool distParamDerivs;
};


inline NonD::~NonD()
{ }


inline void NonD::
moments(const RealVector& means, const RealVector& std_devs)
{ meanStats = means; stdDevStats = std_devs; }


inline void NonD::distribution_parameter_derivatives(bool dist_param_derivs)
{ distParamDerivs = dist_param_derivs; }


inline void NonD::initialize_run()
{ 
  Iterator::initialize_run();
  prevNondInstance = nondInstance; 
  nondInstance = this; 
}


inline void NonD::run()
{ bestVarsRespMap.clear(); quantify_uncertainty(); }


inline void NonD::finalize_run()
{ 
  nondInstance = prevNondInstance;
  Iterator::finalize_run(); // included for completeness
}


inline const Response& NonD::response_results() const
{ return finalStatistics; }


inline void NonD::response_results_active_set(const ActiveSet& set)
{ finalStatistics.active_set(set); }

} // namespace Dakota

#endif
