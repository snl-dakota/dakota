/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDExpansion
//- Description: Iterator base class for polynomial expansion methods for UQ
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_EXPANSION_H
#define NOND_EXPANSION_H

#include "DakotaNonD.H"


namespace Dakota {


/// Base class for polynomial chaos expansions (PCE) and stochastic
/// collocation (SC)

/** The NonDExpansion class provides a base class for methods that use
    polynomial expansions to approximate the effect of parameter
    uncertainties on response functions of interest. */

class NonDExpansion: public NonD
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDExpansion(Model& model);
  /// alternate constructor
  NonDExpansion(Model& model, short exp_coeffs_approach, short u_space_type,
		bool piecewise_basis, bool use_derivs);
  /// destructor
  ~NonDExpansion();

  //
  //- Heading: Virtual function redefinitions
  //

  /// perform a forward uncertainty propagation using PCE/SC methods
  void quantify_uncertainty(); // pure virtual fn. called by NonD
  /// print the final statistics
  void print_results(std::ostream& s);

  const Model& algorithm_space_model() const;

protected:

  //
  //- Heading: Virtual functions
  //

  /// perform error checks and mode overrides
  virtual void resolve_inputs(short& u_space_type, short& data_order);
  /// initialize uSpaceModel polynomial approximations with PCE/SC data
  virtual void initialize_u_space_model();
  /// initialize random variable definitions and final stats arrays
  virtual void initialize_expansion();
  /// form the expansion by calling uSpaceModel.build_approximation()
  virtual void compute_expansion();
  /// uniformly increment the expansion order (PCE only)
  virtual void increment_order();
  /// update an expansion; avoids overhead in compute_expansion()
  virtual void update_expansion();
  /// print expansion coefficients, as supported by derived instance
  virtual void print_coefficients(std::ostream& s);

  /// compute 2-norm of change in response covariance
  virtual Real compute_covariance_metric();
  /// compute 2-norm of change in final statistics
  virtual Real compute_final_statistics_metric();

  //
  //- Heading: Virtual function redefinitions
  //

  /// set covarianceControl defaults and shape respCovariance
  void initialize_response_covariance();
  /// update function values within finalStatistics
  void update_final_statistics();
  /// update function gradients within finalStatistics
  void update_final_statistics_gradients();

  //
  //- Heading: Member function definitions
  //

  /// common constructor code for initialization of natafTransform
  void initialize(short u_space_type);

  /// refine the reference expansion found by compute_expansion() using
  /// uniform/adaptive p-/h-refinement strategies
  void refine_expansion();

  /// update settings for subsequent passes in the case of multifidelity models
  void update_hierarchy();

  /// assign a NonDCubature instance within u_space_sampler
  void construct_cubature(Iterator& u_space_sampler, Model& g_u_model,
			  unsigned short cub_int_order);
  /// assign a NonDQuadrature instance within u_space_sampler based on
  /// a quad_order specification
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    const UShortArray& quad_order,
			    const RealVector& dim_pref);
  /// assign a NonDQuadrature instance within u_space_sampler that
  /// generates a filtered tensor product sample set
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    int filtered_samples, const RealVector& dim_pref);
  /// assign a NonDQuadrature instance within u_space_sampler that
  /// samples randomly from a tensor product multi-index
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    int random_samples, int seed,
			    const UShortArray& quad_order,
			    const RealVector& dim_pref);
  /// assign a NonDSparseGrid instance within u_space_sampler
  void construct_sparse_grid(Iterator& u_space_sampler, Model& g_u_model,
			     const UShortArray& ssg_level,
			     const RealVector& ssg_dim_pref);
  // assign a NonDIncremLHSSampling instance within u_space_sampler
  //void construct_incremental_lhs(Iterator& u_space_sampler, Model& u_model,
  //				 int num_samples, int seed, const String& rng);

  /// construct the expansionSampler operating on uSpaceModel
  void construct_expansion_sampler();

  /// calculate analytic and numerical statistics from the expansion
  void compute_statistics();

  //
  //- Heading: Data
  //

  /// Model representing the approximate response function in u-space, after
  /// u-space recasting and orthogonal polynomial data fit recursions
  Model uSpaceModel;

  /// method for collocation point generation and subsequent
  /// calculation of the expanion coefficients
  short expansionCoeffsApproach;

  /// number of invocations of quantify_uncertainty()
  size_t numUncertainQuant;

  /// number of truth samples performed on g_u_model to form the expansion
  int numSamplesOnModel;
  /// number of approximation samples performed on the polynomial
  /// expansion in order to estimate probabilities
  int numSamplesOnExpansion;

  /// flag for indicating state of \c nested and \c non_nested overrides of
  /// default rule nesting, which depends on the type of integration driver
  bool nestedRules;

  /// flag for \c piecewise specification, indicating usage of local
  /// basis polynomials within the stochastic expansion
  bool piecewiseBasis;

  /// flag for \c use_derivatives specification, indicating usage of derivative
  /// data (with respect to expansion variables) to enhance the calculation of
  /// the stochastic expansion.
  /** This is part of the method specification since the instantiation of the
      global data fit surrogate is implicit with no user specification.  This
      behavior is distinct from the usage of response derivatives with respect
      to auxilliary variables (design, epistemic) for computing derivatives of
      aleatory expansion statistics with respect to these variables. */
  bool useDerivs;

  /// refinement type: NO_REFINEMENT, P_REFINEMENT, or H_REFINEMENT
  short refineType;
  /// refinement control: NO_CONTROL, UNIFORM_CONTROL, LOCAL_ADAPTIVE_CONTROL,
  /// DIMENSION_ADAPTIVE_CONTROL_SOBOL, DIMENSION_ADAPTIVE_CONTROL_DECAY, or
  /// DIMENSION_ADAPTIVE_CONTROL_GENERALIZED
  short refineControl;

  /// symmetric matrix of analytic response covariance (full response
  /// covariance option)
  RealSymMatrix respCovariance;
  /// vector of response variances (diagonal response covariance option)
  RealVector respVariance;

  /// stores the initial variables data in u-space
  RealVector initialPtU;

private:

  //
  //- Heading: Convenience function definitions
  //

  /// compute average of total Sobol' indices (from VBD) across the
  /// response set for use as an anisotropy indicator
  void reduce_total_sobol_sets(RealVector& avg_sobol);
  /// compute minimum of spectral coefficient decay rates across the
  /// response set for use as an anisotropy indicator
  void reduce_decay_rate_sets(RealVector& min_decay);

  /// initialization of adaptive refinement using generalized sparse grids
  void initialize_sets();
  /// perform an adaptive refinement increment using generalized sparse grids
  Real increment_sets();
  /// finalization of adaptive refinement using generalized sparse grids
  void finalize_sets(bool converged_within_tol);

  /// calculate the response covariance (diagonal or full matrix)
  void compute_covariance();
  /// calculate respVariance or diagonal terms respCovariance(i,i)
  void compute_diagonal_variance();
  /// calculate respCovariance(i,j) for j<i
  void compute_off_diagonal_covariance();

  /// print expansion and numerical moments
  void print_moments(std::ostream& s);
  /// print respCovariance
  void print_covariance(std::ostream& s);
  /// print global sensitivity indices
  void print_sobol_indices(std::ostream& s);
  /// print local sensitivities evaluated at initialPtU
  void print_local_sensitivity(std::ostream& s);

  /// manage print of results following a refinement increment
  void compute_print_increment_results();
  /// manage print of results following a refinement increment
  void compute_print_iteration_results(bool initialize);
  /// manage print of results following convergence of iterative refinement
  void compute_print_converged_results(bool print_override = false);

  //
  //- Heading: Data
  //

  /// user override of default rule nesting: NO_NESTING_OVERRIDE,
  /// NESTED, or NON_NESTED
  short ruleNestingOverride;
  /// user override of default rule growth: NO_GROWTH_OVERRIDE,
  /// RESTRICTED, or UNRESTRICTED
  short ruleGrowthOverride;

  /// Iterator used for sampling on the uSpaceModel to generate approximate
  /// probability/reliability/response level statistics.  Currently this is
  /// an LHS sampling instance, but AIS could also be used.
  Iterator expansionSampler;
  /// Iterator used to refine the approximate probability estimates 
  /// generated by the expansionSampler using importance sampling
  Iterator importanceSampler;
  
  /// flag to indicate calculation of numerical statistics by sampling
  /// on the expansion
  bool expSampling;
  /// flag to use LHS sampling or MMAIS sampling on the expansion
  bool impSampling;

  /// derivative of the expansion with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix expGradsMeanX;

  /// enumeration for controlling VBD calculation and output: NO_VBD,
  /// UNIVARIATE_VBD, or ALL_VBD
  short vbdControl;
  /// tolerance for omitting output of small VBD indices
  Real vbdDropTol;

  /// enumeration for controlling response covariance calculation and
  /// output: {DEFAULT,DIAGONAL,FULL}_COVARIANCE
  short covarianceControl;
};


inline const Model& NonDExpansion::algorithm_space_model() const
{ return uSpaceModel; }


inline void NonDExpansion::print_coefficients(std::ostream& s)
{ /* default is no-op */ }

} // namespace Dakota

#endif
