/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include "DakotaAnalyzer.hpp"
#include "ProbabilityTransformation.hpp"
#include "DataMethod.hpp"

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
  /// alternate form: initialize natafTransform based on incoming data
  void initialize_random_variables(
    const Pecos::ProbabilityTransformation& transform);

  /// set requestedRespLevels, requestedProbLevels, requestedRelLevels,
  /// requestedGenRelLevels, respLevelTarget, and cdfFlag (used in
  /// combination with alternate ctors)
  void requested_levels(const RealVectorArray& req_resp_levels,
			const RealVectorArray& req_prob_levels,
			const RealVectorArray& req_rel_levels,
			const RealVectorArray& req_gen_rel_levels,
			short resp_lev_tgt, short resp_lev_tgt_reduce,
			bool cdf_flag);

  /// set distParamDerivs
  void distribution_parameter_derivatives(bool dist_param_derivs);

  //
  //- Heading: Set/get routines
  //

  /// get pdfOutput
  bool pdf_output() const;
  /// set pdfOutput
  void pdf_output(bool output);

  /// return natafTransform
  Pecos::ProbabilityTransformation& variable_transformation();

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonD(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for sample generation and evaluation "on the fly"
  NonD(unsigned short method_name, Model& model);
  /// alternate constructor for sample generation "on the fly"
  NonD(unsigned short method_name, const RealVector& lower_bnds,
       const RealVector& upper_bnds);

  ~NonD(); ///< destructor

  //
  //- Heading: Virtual member function redefinitions
  //

  void initialize_run();
  void core_run();
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

  /// initializes respCovariance
  virtual void initialize_response_covariance();
  /// initializes finalStatistics for storing NonD final results
  virtual void initialize_final_statistics();
  /// update finalStatistics::functionValues
  virtual void update_final_statistics();

  //
  //- Heading: Utility routines
  //

  /// create a system-generated unique seed (when a seed is unspecified)
  int generate_system_seed();

  /// instantiate natafTransform
  void initialize_random_variable_transformation();
  /// initializes ranVarTypesX and ranVarTypesU within natafTransform
  void initialize_random_variable_types(short u_space_type);
  /// initializes ranVarMeansX, ranVarStdDevsX, ranVarLowerBndsX,
  /// ranVarUpperBndsX, and ranVarAddtlParamsX within natafTransform
  void initialize_random_variable_parameters();
  /// propagate iteratedModel correlations to natafTransform
  void initialize_random_variable_correlations();
  /// verify that correlation warping supported by Der Kiureghian & Liu
  /// for given variable types
  void verify_correlation_support();

  /// initializes finalStatistics::functionGradients
  void initialize_final_statistics_gradients();
  /// update finalStatistics::functionValues from momentStats and
  /// computed{Prob,Rel,GenRel,Resp}Levels
  void update_aleatory_final_statistics();
  /// update system metrics from component metrics within finalStatistics
  void update_system_final_statistics();
  /// update finalStatistics::functionGradients
  void update_system_final_statistics_gradients();

  /// size computed{Resp,Prob,Rel,GenRel}Levels
  void initialize_distribution_mappings();
  /// prints the z/p/beta/beta* mappings reflected in
  /// {requested,computed}{Resp,Prob,Rel,GenRel}Levels
  void print_distribution_mappings(std::ostream& s) const;
  /// print system series/parallel mappings for response levels
  void print_system_mappings(std::ostream& s) const;

  /// recast x_model from x-space to u-space to create u_model
  void transform_model(Model& x_model, Model& u_model,
		       bool global_bounds = false, Real bound = 10.);
  /// assign a NonDLHSSampling instance within u_space_sampler
  void construct_lhs(Iterator& u_space_sampler, Model& u_model,
		     unsigned short sample_type, int num_samples, int seed,
		     const String& rng, bool vary_pattern,
		     short sampling_vars_mode = ACTIVE);

  /// static function for RecastModels used for forward mapping of u-space
  /// variables from NonD Iterators to x-space variables for Model evaluations
  static void vars_u_to_x_mapping(const Variables& u_vars, Variables& x_vars);
  /// static function for RecastModels used for inverse mapping of x-space
  /// variables from data import to u-space variables for NonD Iterators
  static void vars_x_to_u_mapping(const Variables& x_vars, Variables& u_vars);

  /// static function for RecastModels used to map u-space ActiveSets
  /// from NonD Iterators to x-space ActiveSets for Model evaluations
  static void set_u_to_x_mapping(const Variables& u_vars,
				 const ActiveSet& u_set, ActiveSet& x_set);

  /// static function for RecastModels used to map x-space responses from
  /// Model evaluations to u-space responses for return to NonD Iterator.
  static void resp_x_to_u_mapping(const Variables& x_vars,
				  const Variables& u_vars,
				  const Response& x_response,
				  Response& u_response);

  /// allocate results array storage for distribution mappings
  void archive_allocate_mappings();
  /// archive the mappings from specified response levels for specified fn
  void archive_from_resp(size_t fn_index);
  /// archive the mappings to computed response levels for specified fn
  void archive_to_resp(size_t fn_index);

  //
  //- Heading: Data members
  //

  /// pointer to the active object instance used within static evaluator
  /// functions in order to avoid the need for static data
  static NonD* nondInstance;
  /// pointer containing previous value of nondInstance
  NonD* prevNondInstance;

  /// Nonlinear variable transformation that encapsulates the required
  /// data for performing transformations from X -> Z -> U and back.
  Pecos::ProbabilityTransformation natafTransform;

  // The following variable counts reflect the native Model space, which could
  // correspond to either X or U space.  If a specific X or U variables count
  // is needed, then natafTransform.ranVarTypesX/U.count() should be used.

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
  /// number of continuous interval uncertain variables (native space)
  size_t numContIntervalVars;
  /// number of discrete interval uncertain variables (native space)
  size_t numDiscIntervalVars;
  /// number of discrete integer set uncertain variables (native space)
  size_t numDiscSetIntUncVars;
  /// number of discrete real set uncertain variables (native space)
  size_t numDiscSetRealUncVars;

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

  /// flag for computing interval-type metrics instead of integrated
  /// metrics If any epistemic variables are active in a metric
  /// evaluation, then this flag is set.
  bool epistemicStats;

  /// moments of response functions (mean, std deviation, skewness, and
  /// kurtosis calculated in compute_moments()), indexed as (moment,fn)
  RealMatrix momentStats;

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
  /// indicates component or system series/parallel failure metrics
  short respLevelTargetReduce;

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

  /// flag for managing output of response probability density functions (PDFs)
  bool pdfOutput;

  /// final statistics from the uncertainty propagation used in strategies:
  /// response means, standard deviations, and probabilities of failure
  Response finalStatistics;

private:

  /// convenience function for distributing a vector of levels among multiple
  /// response functions if a short-hand specification is employed.
  void distribute_levels(RealVectorArray& levels, bool ascending = true);

  /// Write distribution mappings to a file for a single response
  void distribution_mappings_file(size_t fn_index) const;

  /// Print distribution mapping for a single response function to ostream
  void print_distribution_map(size_t fn_index, std::ostream& s) const;


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


inline void NonD::distribution_parameter_derivatives(bool dist_param_derivs)
{ distParamDerivs = dist_param_derivs; }


inline bool NonD::pdf_output() const
{ return pdfOutput; }


inline void NonD::pdf_output(bool output)
{ pdfOutput = output; }


inline Pecos::ProbabilityTransformation& NonD::variable_transformation()
{ return natafTransform; }


inline void NonD::initialize_run()
{ 
  Analyzer::initialize_run();
  prevNondInstance = nondInstance; 
  nondInstance = this; 
}


inline void NonD::core_run()
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
