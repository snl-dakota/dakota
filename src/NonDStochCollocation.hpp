/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_STOCH_COLLOCATION_H
#define NOND_STOCH_COLLOCATION_H

#include "NonDExpansion.hpp"


namespace Dakota {


/// Nonintrusive stochastic collocation approaches to uncertainty
/// quantification

/** The NonDStochCollocation class uses a stochastic collocation (SC)
    approach to approximate the effect of parameter uncertainties on
    response functions of interest.  It utilizes the
    InterpPolyApproximation class to manage multidimensional Lagrange
    polynomial interpolants. */

class NonDStochCollocation: public NonDExpansion
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDStochCollocation(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NonDStochCollocation(Model& model, short exp_coeffs_approach,
		       unsigned short num_int, const RealVector& dim_pref,
		       short u_space_type, short refine_type,
		       short refine_control, short covar_control,
		       short rule_nest, short rule_growth,
		       bool piecewise_basis, bool use_derivs);
  /// destructor
  ~NonDStochCollocation();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: Constructors
  //

  /// short-cut ctor allowing derived class to replace logic in base class ctor
  /// (method_name is not necessary, rather it is just a convenient overload
  /// allowing the derived ML SC class to bypass the standard SC ctor)
  NonDStochCollocation(unsigned short method_name, ProblemDescDB& problem_db,
		       Model& model);
  /// short-cut ctor allowing derived class to replace logic in base class ctor
  NonDStochCollocation(unsigned short method_name, Model& model,
		       short exp_coeffs_approach, const RealVector& dim_pref,
		       short refine_type, short refine_control,
		       short covar_control, short ml_alloc_control,
		       short ml_discrep, short rule_nest, short rule_growth,
		       bool piecewise_basis, bool use_derivs);

  //
  //- Heading: Virtual function redefinitions
  //

  void resolve_inputs(short& u_space_type, short& data_order);
  void initialize_u_space_model();

  //void initialize_expansion();
  //void compute_expansion();

  Real compute_covariance_metric(bool revert, bool print_metric);
  Real compute_level_mappings_metric(bool revert, bool print_metric);
  //Real compute_final_statistics_metric(bool revert, bool print_metric);

  //void compute_statistics(short results_state = FINAL_RESULTS);
  //void pull_candidate(RealVector& stats_star);

  //
  //- Heading: Member functions
  //

  /// initialize covariance pairings by passing all pointers for
  /// approximation j to approximation i
  void initialize_covariance();

  /// helper function to compute deltaRespVariance
  void compute_delta_mean(bool update_ref);
  /// helper function to compute deltaRespVariance
  void compute_delta_variance(bool update_ref, bool print_metric);
  /// helper function to compute deltaRespCovariance
  void compute_delta_covariance(bool update_ref, bool print_metric);

  /// update analytic level mappings; this uses a lightweight approach
  /// for incremental statistics (no derivatives, no finalStatistics update)
  void analytic_delta_level_mappings(const RealVector& level_maps_ref,
				     RealVector& level_maps_new);

  /// configure u_space_sampler based on numerical integration specification
  void config_integration(unsigned short quad_order, unsigned short ssg_level,
			  const RealVector& dim_pref, short u_space_type, 
			  Iterator& u_space_sampler, Model& g_u_model);
  /// configure u_space_sampler based on expansion coefficients approach
  void config_integration(short exp_coeffs_approach, unsigned short num_int,
			  const RealVector& dim_pref, Iterator& u_space_sampler,
			  Model& g_u_model);
  /// define approx_type based on expansion settings
  void config_approximation_type(String& approx_type);

private:

  //
  //- Heading: Utility functions
  //

  //
  //- Heading: Data
  //

  /// change in response means induced by a refinement candidate
  RealVector deltaRespMean;
  /// change in (DIAGONAL) response variance induced by a refinement candidate
  RealVector deltaRespVariance;
  /// change in (FULL) response covariance induced by a refinement candidate
  RealSymMatrix deltaRespCovariance;
  /// change in response means induced by a refinement candidate
  RealVector deltaLevelMaps;
};

} // namespace Dakota

#endif
