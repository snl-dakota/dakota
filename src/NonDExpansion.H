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

  NonDExpansion(Model& model);    ///< standard constructor
  NonDExpansion(Model& model, short exp_coeffs_approach, short u_space_type,
		bool use_derivs); ///< alternate constructor
  ~NonDExpansion();               ///< destructor

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

  /// initialize random variable definitions and final stats arrays
  virtual void initialize_expansion();
  /// form the expansion by calling uSpaceModel.build_approximation()
  virtual void compute_expansion();
  /// uniformly increment the expansion order (PCE only)
  virtual void increment_expansion();
  /// print expansion coefficients, as supported by derived instance
  virtual void print_coefficients(std::ostream& s);
  /// print expansion and numerical moments, as supported by derived instance
  virtual void print_moments(std::ostream& s);

  //
  //- Heading: Member function definitions
  //

  /// update the expansion by calling uSpaceModel.build_approximation();
  /// avoids unnecessary overhead in compute_expansion()
  void update_expansion();

  /// assign a NonDCubature instance within u_space_sampler
  void construct_cubature(Iterator& u_space_sampler, Model& g_u_model,
			  unsigned short cub_int_order);
  /// assign a NonDQuadrature instance within u_space_sampler based on
  /// a quad_order specification
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    const UShortArray& quad_order, bool piecewise_basis,
			    bool use_derivs);
  /// assign a NonDQuadrature instance within u_space_sampler based on
  /// the size of a filtered tensor product sample set
  void construct_quadrature(Iterator& u_space_sampler, Model& g_u_model,
			    int filtered_samples, bool piecewise_basis,
			    bool use_derivs);
  /// assign a NonDSparsegrid instance within u_space_sampler
  void construct_sparse_grid(Iterator& u_space_sampler, Model& g_u_model,
			     unsigned short ssg_level,
			     const RealVector& ssg_dim_pref,
			     bool piecewise_basis, bool use_derivs);
  // assign a NonDIncremLHSSampling instance within u_space_sampler
  //void construct_incremental_lhs(Iterator& u_space_sampler, Model& u_model,
  //				 int num_samples, int seed, const String& rng);

  /// assign a NonDCubature instance within u_space_sampler
  void construct_discrepancy_model(const Model& hierarch_model,
				   Model& discrepancy_model);

  /// initialize uSpaceModel polynomial approximations with PCE/SC data
  void initialize_u_space_model();
  /// construct the expansionSampler operating on uSpaceModel
  void construct_expansion_sampler();

  /// calculate analytic and numerical statistics from the expansion
  void compute_statistics();
  /// update finalStatistics
  void update_final_statistics();

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

  /// flag for \c use_derivatives specification, indicating usage of derivative
  /// data (with respect to expansion variables) to enhance the calculation of
  /// the stochastic expansion.
  /** This is part of the method specification since the instantiation of the
      global data fit surrogate is implicit with no user specification.  This
      behavior is distinct from the usage of response derivatives with respect
      to auxilliary variables (design, epistemic) for computing derivatives of
      aleatory expansion statistics with respect to these variables. */
  bool useDerivsFlag;

  /// refinement type: NO_REFINEMENT, P_REFINEMENT, or H_REFINEMENT
  short refineType;
  /// refinement control: NO_CONTROL, UNIFORM_CONTROL,
  /// DIMENSION_ADAPTIVE_TOTAL_SOBOL, DIMENSION_ADAPTIVE_SPECTRAL_DECAY, or
  /// DIMENSION_ADAPTIVE_GENERALIZED_SPARSE
  short refineControl;

private:

  //
  //- Heading: Convenience function definitions
  //

  /// helper function for common constructor code
  void initialize(Model& model, short u_space_type);

  /// static function for RecastModels used to map x-space responses from
  /// Model evaluations to u-space responses for return to NonD Iterators.
  static void discrepancy_mapping(const Variables& hierarch_vars,
				  const Variables& discrepancy_vars,
				  const Response&  hierarch_response,
				  Response&        discrepancy_response);

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

  /// compute 2-norm of change in response covariance
  Real compute_covariance_metric(const RealSymMatrix& resp_covar_ref);
  /// compute 2-norm of change in final statistics
  Real compute_final_statistics_metric(const RealVector& final_stats_ref);

  /// calculate respCovariance
  void compute_covariance();
  /// calculate respCovariance(i,j) for j<i
  void compute_off_diagonal_covariance();

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
  void compute_print_converged_results();

  //
  //- Heading: Data
  //

  /// user override of default rule nesting: NO_NESTING_OVERRIDE,
  /// NESTED, or NON_NESTED
  short ruleNestingOverride;
  /// user override of default rule growth: NO_GROWTH_OVERRIDE,
  /// RESTRICTED, or UNRESTRICTED
  short ruleGrowthOverride;

  /// flag to use LHS sampling or MMAIS sampling on the expansion
  bool impSampling;

  /// Iterator used for sampling on the uSpaceModel to generate approximate
  /// probability/reliability/response level statistics.  Currently this is
  /// an LHS sampling instance, but AIS could also be used.
  Iterator expansionSampler;

  /// Iterator used to refine the approximate probability estimates 
  /// generated by the expansionSampler using importance sampling
  Iterator importanceSampler;
  
  /// stores the initial variables data in u-space
  RealVector initialPtU;

  /// derivative of the expansion with respect to the x-space variables
  /// evaluated at the means (used as uncertainty importance metrics)
  RealMatrix expGradsMeanX;

  /// symmetric matrix of analytic response covariance
  RealSymMatrix respCovariance;

  /// flag to indicate calculation of numerical statistics by sampling
  /// on the expansion
  bool expSampling;

  /// enumeration for controlling VBD calculation and output: NO_VBD,
  /// UNIVARIATE_VBD, or ALL_VBD
  short vbdControl;
  /// tolerance for omitting output of small VBD indices
  Real vbdDropTol;
};


inline const Model& NonDExpansion::algorithm_space_model() const
{ return uSpaceModel; }


inline void NonDExpansion::print_coefficients(std::ostream& s)
{ /* default is no-op */ }


inline void NonDExpansion::print_moments(std::ostream& s)
{ /* default is no-op */ }

} // namespace Dakota

#endif
