/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonDMultilevelPolynomialChaos
//- Description: Iterator to compute/employ Polynomial Chaos expansions
//- Owner:       Mike Eldred, Sandia National Laboratories

#ifndef NOND_MULTILEVEL_POLYNOMIAL_CHAOS_H
#define NOND_MULTILEVEL_POLYNOMIAL_CHAOS_H

#include "NonDPolynomialChaos.hpp"

namespace Dakota {

/// Nonintrusive polynomial chaos expansion approaches to uncertainty
/// quantification

/** The NonDMultilevelPolynomialChaos class uses a polynomial chaos
    expansion (PCE) approach to approximate the effect of parameter
    uncertainties on response functions of interest.  It utilizes the
    OrthogPolyApproximation class to manage multiple types of
    orthogonal polynomials within a Wiener-Askey scheme to PCE.  It
    supports PCE coefficient estimation via sampling, quadrature,
    point-collocation, and file import. */

class NonDMultilevelPolynomialChaos: public NonDPolynomialChaos
{
public:

  //
  //- Heading: Constructors and destructor
  //
 
  /// standard constructor
  NonDMultilevelPolynomialChaos(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for numerical integration (tensor, sparse, cubature)
  NonDMultilevelPolynomialChaos(/*unsigned short method_name,*/ Model& model,
				short exp_coeffs_approach,
				const UShortArray& num_int_seq,
				const RealVector& dim_pref, short u_space_type,
				short refine_type, short refine_control,
				short covar_control, short ml_alloc_cntl,
				short ml_discrep, short rule_nest,
				short rule_growth, bool piecewise_basis,
				bool use_derivs);
  /// alternate constructor for regression (least squares, CS, OLI)
  NonDMultilevelPolynomialChaos(unsigned short method_name, Model& model,
				short exp_coeffs_approach,
				const UShortArray& exp_order_seq,
				const RealVector& dim_pref,
				const SizetArray& colloc_pts_seq,
				Real colloc_ratio, const SizetArray& pilot,
				int seed, short u_space_type, short refine_type,
				short refine_control, short covar_control,
				short ml_alloc_cntl, short ml_discrep,
				//short rule_nest, short rule_growth,
				bool piecewise_basis, bool use_derivs,
				bool cv_flag,
				const String& import_build_pts_file,
				unsigned short import_build_format,
				bool import_build_active_only);
  /// destructor
  ~NonDMultilevelPolynomialChaos();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_u_space_model();
  void core_run();
  void assign_specification_sequence();
  void increment_specification_sequence();
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: Member functions
  //

  /// option within core_run() for multilevel allocation of samples
  /// (mirroring NonDMultilevelSampling for PCE regression)
  void multilevel_regression();

  /// increment the sequence in numSamplesOnModel for multilevel_regression()
  void increment_sample_sequence(size_t new_samp,size_t total_samp,size_t lev);

  /// Aggregate variance across the set of QoI for a particular model level
  void aggregate_variance(Real& agg_var_l);
  /// Retrieve the cardinality of the basis and the maximum number of sparse
  /// coefficients across the set of QoI for a particular model level
  void sparsity_metrics(size_t& cardinality_l, Real& sparsity_metric_l,
			Real power);

  /// compute delta_N_l for ESTIMATOR_VARIANCE case
  void compute_sample_increment(const RealVector& agg_var,
				const RealVector& cost, Real sum_root_var_cost,
				Real eps_sq_div_2, const SizetArray& N_l,
				SizetArray& delta_N_l);
  /// compute delta_N_l for RIP_SAMPLING case
  void compute_sample_increment(const SizetArray& cardinality, Real factor,
				const RealVector& sparsity,
				const SizetArray& N_l, SizetArray& delta_N_l);

private:

  //
  //- Heading: Utility functions
  //

  /// perform specification updates (shared code from
  // {assign,increment}_specification_sequence())
  void update_from_specification(bool update_exp, bool update_sampler,
				 bool update_from_ratio);

  /// scale sample profile new_N_l to retain shape while enforcing an upper
  /// bound of cardinality * factor
  void scale_profile(const SizetArray& cardinality, Real factor,
		     RealVector& new_N_l);

  /// verify supported and define default discrepancy emulation mode
  void assign_discrepancy_mode();
  /// define the surrogate response mode for a hierarchical model in 
  /// multilevel/multifidelity expansions
  void assign_hierarchical_response_mode();

  //
  //- Heading: Data
  //

  /// type of sample allocation scheme for discretization levels / model forms
  /// within multilevel / multifidelity methods
  short mlmfAllocControl;

  /// user specification for expansion_order (array for multifidelity)
  UShortArray expOrderSeqSpec;
  /// user specification for collocation_points (array for multifidelity)
  SizetArray collocPtsSeqSpec;
  /// user specification for expansion_samples (array for multifidelity)
  SizetArray expSamplesSeqSpec;
  /// user request of quadrature order
  UShortArray quadOrderSeqSpec;
  /// user request of sparse grid level
  UShortArray ssgLevelSeqSpec;
  /// sequence index for {expOrder,collocPts,expSamples}SeqSpec
  size_t sequenceIndex;

  /// rate parameter for estimator variance in ML PCE
  Real kappaEstimatorRate;
  /// scale parameter for estimator variance in ML PCE
  Real gammaEstimatorScale;

  /// number of initial samples specified by the user
  SizetArray pilotSamples;
};

} // namespace Dakota

#endif
