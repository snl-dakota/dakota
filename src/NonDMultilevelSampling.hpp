/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MULTILEVEL_SAMPLING_H
#define NOND_MULTILEVEL_SAMPLING_H

#include "NonDHierarchSampling.hpp"
#include "DataMethod.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#elif HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

namespace Dakota {

/// Performs Multilevel Monte Carlo sampling for uncertainty quantification.

/** Multilevel Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevelSampling: public virtual NonDHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevelSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  void print_variance_reduction(std::ostream& s);

  void nested_response_mappings(const RealMatrix& primary_coeffs,
				const RealMatrix& secondary_coeffs);

  bool discrepancy_sample_counts() const;

  //
  //- Heading: Member functions
  //

  /// evaluate multiple sample batches concurrently, where each batch involves
  /// either a single model or model pair
  void ml_increments(SizetArray& delta_N_l, String prepend);

  // helper that consolidates sequence advancement, sample generation,
  // sample export, and sample evaluation
  //void evaluate_ml_sample_increment(String prepend, unsigned short step);

  /// increment the equivalent number of HF evaluations based on new
  /// model evaluations
  void increment_ml_equivalent_cost(size_t new_N_l, Real lev_cost,
				    Real ref_cost, Real& equiv_hf);

  /// compute MLMC estimator variance from level QoI variances
  void compute_ml_estimator_variance(const RealMatrix&   var_Y,
				     const Sizet2DArray& num_Y,
				     RealVector& ml_est_var);

  /// recover variance from raw moments
  void recover_variance(const RealMatrix& moment_stats, RealVector& var_H);

  /// initialize the ML accumulators for computing means, variances, and
  /// covariances across resolution levels
  void initialize_ml_Ysums(IntRealMatrixMap& sum_Y, size_t num_lev,
			   size_t num_mom = 4);
  /// initialize the ML accumulators for computing means, variances, and
  /// covariances across resolution levels
  void initialize_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, size_t num_lev);

  /// update accumulators for multilevel telescoping running sums
  /// using set of model evaluations within allResponses
  void accumulate_ml_Ysums(const IntResponseMap& resp_map,
			   IntRealMatrixMap& sum_Y, RealMatrix& sum_YY,
			   size_t lev, size_t lev_offset, SizetArray& num_Y);
  /// update accumulators for multilevel telescoping running sums
  /// using set of model evaluations within allResponses
  void accumulate_ml_Ysums(const IntResponseMap& resp_map, RealMatrix& sum_Y,
			   RealMatrix& sum_YY, size_t lev, size_t lev_offset,
			   SizetArray& num_Y);
  /// update running QoI sums for one model (sum_Q) using set of model
  /// evaluations within allResponses; used for level 0 from other accumulators
  void accumulate_ml_Qsums(const IntResponseMap& resp_map,
			   IntRealMatrixMap& sum_Q, size_t lev,
			   SizetArray& num_Q);

  /// compute variance scalar from sum accumulators
  Real variance_Ysum(Real sum_Y, Real sum_YY, size_t Nlq);
  /// compute variance column vec (all QoI for one level) from sum accumulators
  void variance_Ysum(const Real* sum_Y, const Real* sum_YY,
		     const SizetArray& N_l, Real* var_Y);
  /// compute variance from sum accumulators
  Real variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
		     Real sum_Qlm1Qlm1, size_t Nlq);

  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			       const SizetArray& N_l);

  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy variances
  Real aggregate_mse_Yvar(const Real* var_Y, const SizetArray& N_l);
  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy sums
  Real aggregate_mse_Ysum(const Real* sum_Y, const Real* sum_YY,
			  const SizetArray& N_l);

  /// accumulate ML-only contributions (levels with no CV) to 2 raw moments
  void ml_Q_raw_moments(const RealMatrix& sum_Hl_1,const RealMatrix& sum_Hlm1_1,
			const RealMatrix& sum_Hl_2,const RealMatrix& sum_Hlm1_2,
			const Sizet2DArray& N_actual, size_t start, size_t end,
			RealMatrix& ml_raw_mom);
  /// accumulate ML-only contributions (levels with no CV) to 2 raw moments
  void ml_Y_raw_moments(const RealMatrix& sum_H1, const RealMatrix& sum_H2,
			const Sizet2DArray& N_l, size_t start, size_t end,
			RealMatrix& ml_raw_mom);
  /// accumulate ML-only contributions (levels with no CV) to 4 raw moments
  void ml_Y_raw_moments(const RealMatrix& sum_H1, const RealMatrix& sum_H2,
			const RealMatrix& sum_H3, const RealMatrix& sum_H4,
			const Sizet2DArray& N_l, size_t start, size_t end,
			RealMatrix& ml_raw_mom);

  /// manage response mode and active model key from {group,form,lev} triplet.
  /// seq_type defines the active dimension for a 1D model sequence.
  void configure_indices(unsigned short group, unsigned short form,
			 size_t lev, short seq_type);
  /// convert group and form and call overload
  void configure_indices(size_t group, size_t form, size_t lev, short seq_type);

  /// return (aggregate) level cost
  Real level_cost(const RealVector& cost, size_t step, size_t offset = 0);

  //
  //- Heading: Data
  //

  /// final estimator variance for output in print_variance_reduction()
  RealVector estVar;

private:

  //
  //- Heading: Helper functions
  //

  // Perform multilevel Monte Carlo across the discretization levels for a
  // particular model form using discrepancy accumulators (sum_Y)
  //void multilevel_mc_Ysum();

  /// Online iteration
  void multilevel_mc_online_pilot(); //_Qsum();
  /// Online allocations for all levels based on offline pilot
  void multilevel_mc_offline_pilot();
  /// Project estimator performance from an online or offline pilot sample
  void multilevel_mc_pilot_projection();

  /// define the truth and surrogate keys
  void assign_active_key();

  /// helper for shared code among offline-pilot and pilot-projection modes
  void evaluate_levels(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		       IntIntPairRealMatrixMap& sum_QlQlm1,
		       Sizet2DArray& N_actual_pilot,
		       Sizet2DArray& N_actual_online, SizetArray& N_alloc_pilot,
		       SizetArray& N_alloc_online, SizetArray& delta_N_l,
		       RealMatrix& var_Y, RealMatrix& var_qoi,
		       RealVector& eps_sq_div_2, bool increment_cost,
		       bool pilot_estvar);

  /// reset existing ML accumulators to zero for all keys
  void reset_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		      IntIntPairRealMatrixMap& sum_QlQlm1);

  // accumulate initial approximation to mean vector, for use as offsets in
  // subsequent accumulations
  //void accumulate_offsets(RealVector& mu);

  /// adds the response evaluations for the current step to 
  /// levQoisamplesmatrixMap. 
  void store_evaluations(const size_t step);

  /// update running QoI sums for two models (sum_Ql, sum_Qlm1) using set of
  /// model evaluations within allResponses
  void accumulate_ml_Qsums(const IntResponseMap& resp_map,
			   IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, size_t lev,
			   SizetArray& num_Q);

  /// increment the allocated samples counter
  size_t allocation_increment(size_t N_l_alloc, const Real* N_l_target);

  // compute the equivalent number of HF evaluations (includes any sim faults)
  Real compute_ml_equivalent_cost(const SizetArray& raw_N_l,
				  const RealVector& cost);

  /// populate finalStatErrors for MLMC based on Q sums
  void compute_error_estimates(const IntRealMatrixMap& sum_Ql,
			       const IntRealMatrixMap& sum_Qlm1,
			       const IntIntPairRealMatrixMap& sum_QlQlm1,
			       const Sizet2DArray& num_Q);

  /// for pilot projection, advance the sample counts and aggregate cost based
  /// on projected rather than actual samples
  void update_projected_samples(const SizetArray& delta_N_l,
				//Sizet2DArray& N_actual,
				SizetArray& N_alloc, const RealVector& cost,
				Real& delta_equiv_hf);

  /// compute variance from sum accumulators
  /// necessary for sample allocation optimization
  static Real variance_Ysum_static(Real sum_Y, Real sum_YY, /*Real offset,*/ 
  						size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  /// compute variance from sum accumulators
  /// necessary for sample allocation optimization
  static Real variance_Qsum_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, 
  			 Real sum_QlQlm1, Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, 
  			 bool compute_gradient, Real& grad);

  Real var_lev_l(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq);
	static Real var_lev_l_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, 
	      Real& grad);

  /// sum up variances for QoI (using sum_YY with means from sum_Y) based on 
  /// allocation target
  void aggregate_variance_target_Qsum(const IntRealMatrixMap&  sum_Ql, 
  								const IntRealMatrixMap&  sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, 
									RealMatrix& agg_var_qoi);



  /// wrapper for variance_Qsum 
  Real variance_mean_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);
  /// sum up variances across QoI for given level
  Real aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			       const Real* sum_QlQl,     const Real* sum_QlQlm1,
			       const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			       const size_t lev);
  /// evaluate variance for given level and QoI (using sum_YY with means from sum_Y)
  Real variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
		     const Real* sum_QlQl,     const Real* sum_QlQlm1,
		     const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
		     const size_t lev, const size_t qoi);
  /// evaluate variances for given level across set of QoI
  void variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
		     const Real* sum_QlQl,     const Real* sum_QlQlm1,
		     const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
		     const size_t lev, Real* var_Yl);
  /// wrapper for var_of_var_ml 
  Real variance_variance_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);

  /// wrapper for var_of_sigma_ml 
  Real variance_sigma_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);

  /// wrapper for var_of_scalarization_ml
  Real variance_scalarization_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);

  static Real compute_bootstrap_covariance(const size_t step, const size_t qoi, 
  								const IntRealMatrixMap& lev_qoisamplematrix_map, const Real N,
  								const bool compute_gradient, Real& grad, int* seed);

  static Real compute_cov_mean_sigma(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev, const bool compute_gradient, Real& grad_g);


  static RealVector compute_cov_mean_sigma_fd(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev);

  static Real compute_mean(const RealVector& samples);

  static Real compute_mean(const RealVector& samples, const bool compute_gradient, Real& grad);

  static Real compute_mean(const RealVector& samples, const Real N);

  static Real compute_mean(const RealVector& samples, const Real N, const bool compute_gradient, Real& grad);

  static Real compute_std(const RealVector& samples);

  static Real compute_std(const RealVector& samples, const bool compute_gradient, Real& grad);

  static Real compute_std(const RealVector& samples, const Real N);

  static Real compute_std(const RealVector& samples, const Real N, const bool compute_gradient, Real& grad);

  static Real compute_cov(const RealVector& samples_X, const RealVector& samples_hat);

  /// sum up Monte Carlo estimates for mean squared error (MSE) for
  /// QoI using discrepancy sums based on allocation target
  void aggregate_mse_target_Qsum(RealMatrix& agg_var_qoi, 
  								const Sizet2DArray& N_l, const size_t step, 
  								RealVector& estimator_var0_qoi);

  /// compute epsilon^2/2 term for each qoi based on reference estimator_var0 and relative convergence tolereance
  void set_convergence_tol(const RealVector& estimator_var0_qoi, 
  								const RealVector& cost, RealVector& eps_sq_div_2_qoi);

  /// compute sample allocation delta based on a budget constraint
  void compute_sample_allocation_target(const RealMatrix& var_qoi, 
    const RealVector& cost, const Sizet2DArray& N_actual,
    const SizetArray& N_alloc, SizetArray& delta_N_l);

  /// compute sample allocation delta based on current samples and based on allocation target. Single allocation target for each qoi, aggregated using max operation.
  void compute_sample_allocation_target(const IntRealMatrixMap& sum_Ql,
    const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
    const RealVector& eps_sq_div_2_in, const RealMatrix& var_qoi,
    const RealVector& cost, const Sizet2DArray& N_pilot,
    const Sizet2DArray& N_online, const SizetArray& N_alloc,
    SizetArray& delta_N_l);  

  // Roll up expected value estimators for central moments.  Final expected
  // value is sum of expected values from telescopic sum.  Note: raw moments
  // have no bias correction (no additional variance from an estimated center).
  void compute_moments(const IntRealMatrixMap& sum_Ql, 
		       const IntRealMatrixMap& sum_Qlm1, 
		       const IntIntPairRealMatrixMap& sum_QlQlm1, 
		       const Sizet2DArray& N_l);

  /// compute the unbiased product of two sampling means
  static Real unbiased_mean_product_pair(const Real sumQ1, const Real sumQ2, 
  								const Real sumQ1Q2, const size_t Nlq);
  /// compute the unbiased product of three sampling means
  static Real unbiased_mean_product_triplet(const Real sumQ1, const Real sumQ2, 
  								const Real sumQ3, const Real sumQ1Q2, const Real sumQ1Q3, 
  								const Real sumQ2Q3, const Real sumQ1Q2Q3, const size_t Nlq);
  /// compute the unbiased product of two pairs of products of sampling means
  static Real unbiased_mean_product_pairpair(const Real sumQ1, const Real sumQ2, 
  								const Real sumQ1Q2, const Real sumQ1sq, const Real sumQ2sq,
                  const Real sumQ1sqQ2, const Real sumQ1Q2sq, 
                  const Real sumQ1sqQ2sq, const size_t Nlq);

  static Real var_of_var_ml_l0(const IntRealMatrixMap& sum_Ql, 
  								const IntRealMatrixMap& sum_Qlm1, 
  								const IntIntPairRealMatrixMap& sum_QlQlm1, 
  								const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
  								const bool compute_gradient, Real& grad_g);

  static Real var_of_var_ml_lmax(const IntRealMatrixMap& sum_Ql, 
  								const IntRealMatrixMap& sum_Qlm1, 
  								const IntIntPairRealMatrixMap& sum_QlQlm1, 
  								const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
  								const bool compute_gradient, Real& grad_g);

  static Real var_of_var_ml_l(const IntRealMatrixMap& sum_Ql, 
  								const IntRealMatrixMap& sum_Qlm1, 
  								const IntIntPairRealMatrixMap& sum_QlQlm1, 
  								const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
  								const size_t lev, const bool compute_gradient, Real& grad_g);

  static Real compute_cov_meanl_varlmone(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1,
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev, const bool compute_gradient, Real& grad_g);

  static Real compute_cov_meanlmone_varl(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1,
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev, const bool compute_gradient, Real& grad_g);

  static Real compute_grad_cov_meanl_vark(const Real cov_mean_var,
                  const Real var_of_var, const Real var_of_sigma, 
                  const Real grad_var_of_var, const Real grad_var_of_sigma, 
                  const Real Nlq);

  ///OPTPP definition
  static void target_cost_objective_eval_optpp(int mode, int n, 
  								const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);
  static void target_cost_constraint_eval_optpp(int mode, int n, 
  								const RealVector& x, RealVector& g, RealMatrix& grad_g, 
  								int& result_mode);

  static void target_var_constraint_eval_optpp(int mode, int n, 
  								const RealVector& x, RealVector& g, RealMatrix& grad_g, 
  								int& result_mode);
  static void target_var_constraint_eval_logscale_optpp(int mode, int n, 
  								const RealVector& x, RealVector& g, RealMatrix& grad_g, 
  								int& result_mode);

  static void target_sigma_constraint_eval_optpp(int mode, int n, 
  								const RealVector& x, RealVector& g, RealMatrix& grad_g, 
  								int& result_mode);
  static void target_sigma_constraint_eval_logscale_optpp(int mode, int n, 
  								const RealVector& x, RealVector& g, RealMatrix& grad_g, 
  								int& result_mode);

  static void target_scalarization_constraint_eval_optpp(int mode, int n, 
  								const RealVector& x, RealVector& g, RealMatrix& grad_g, 
  								int& result_mode);
  static void target_scalarization_constraint_eval_logscale_optpp(int mode, 
  								int n, const RealVector& x, RealVector& g, RealMatrix& grad_g,
  								int& result_mode);

  static void target_var_objective_eval_optpp(int mode, int n, 
  								const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);
  static void target_var_objective_eval_logscale_optpp(int mode, int n, 
  								const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);

  static void target_sigma_objective_eval_optpp(int mode, int n, 
  								const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);
  static void target_sigma_objective_eval_logscale_optpp(int mode, int n, 
  								const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);

  static void target_scalarization_objective_eval_optpp(int mode, int n, 
  								const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);
  static void target_scalarization_objective_eval_logscale_optpp(int mode, 
  								int n, const RealVector& x, double& f, RealVector& grad_f, 
  								int& result_mode);


  static void target_scalarization_objective_eval_optpp_fd(int mode, int n, const RealVector& x, double& f, int& result_mode);

  /// NPSOL definition (Wrapper using OPTPP implementation above under the hood)
  static void target_cost_objective_eval_npsol(int& mode, int& n, double* x, 
  								double& f, double* gradf, int& nstate);
  static void target_cost_constraint_eval_npsol(int& mode, int& m, int& n, 
									int& ldJ, int* needc, double* x, double* g, double* grad_g, 
									int& nstate);

  static void target_var_constraint_eval_npsol(int& mode, int& m, int& n, 
  								int& ldJ, int* needc, double* x, double* g, double* grad_g, 
  								int& nstate);
  static void target_var_constraint_eval_logscale_npsol(int& mode, int& m, 
  								int& n, int& ldJ, int* needc, double* x, double* g, 
  								double* grad_g, int& nstate);

  static void target_sigma_constraint_eval_npsol(int& mode, int& m, int& n, 
  								int& ldJ, int* needc, double* x, double* g, double* grad_g, 
  								int& nstate);
  static void target_sigma_constraint_eval_logscale_npsol(int& mode, int& m, 
  								int& n, int& ldJ, int* needc, double* x, double* g, 
  								double* grad_g, int& nstate);

  static void target_scalarization_constraint_eval_npsol(int& mode, int& m, 
  								int& n, int& ldJ, int* needc, double* x, double* g, 
  								double* grad_g, int& nstate);
  static void target_scalarization_constraint_eval_logscale_npsol(int& mode, 
  								int& m, int& n, int& ldJ, int* needc, double* x, double* g, 
  								double* grad_g, int& nstate);

  static void target_var_objective_eval_npsol(int& mode, int& n, double* x, 
  								double& f, double* gradf, int& nstate);
  static void target_var_objective_eval_logscale_npsol(int& mode, int& n, 
  								double* x, double& f, double* gradf, int& nstate);

  static void target_sigma_objective_eval_npsol(int& mode, int& n, double* x, 
  								double& f, double* gradf, int& nstate);
  static void target_sigma_objective_eval_logscale_npsol(int& mode, int& n, 
  								double* x, double& f, double* gradf, int& nstate);

  static void target_scalarization_objective_eval_npsol(int& mode, int& n, 
  								double* x, double& f, double* gradf, int& nstate);
  static void target_scalarization_objective_eval_logscale_npsol(int& mode, 
  								int& n, double* x, double& f, double* gradf, int& nstate);

  void assign_static_member(const Real &conv_tol, size_t &qoi, 
  								const size_t &qoi_aggregation, const size_t &num_functions, 
  								const RealVector &level_cost_vec, 
  								const IntRealMatrixMap &sum_Ql,
                  const IntRealMatrixMap &sum_Qlm1, 
                  const IntIntPairRealMatrixMap &sum_QlQlm1,
                  const RealVector &pilot_samples, 
                  const RealMatrix &scalarization_response_mapping);

  void assign_static_member_problem18(Real &var_L_exact, Real &var_H_exact, 
  								Real &mu_four_L_exact, Real &mu_four_H_exact, 
  								Real &Ax, RealVector &level_cost_vec) const;

  static void target_var_constraint_eval_optpp_problem18(int mode, int n, 
  								const RealVector &x, RealVector &g, RealMatrix &grad_g, 
  								int &result_mode);
  static void target_sigma_constraint_eval_optpp_problem18(int mode, int n, 
  								const RealVector &x, RealVector &g, RealMatrix &grad_g, 
  								int &result_mode);
  static double exact_var_of_var_problem18(const RealVector &Nl);
  static double exact_var_of_sigma_problem18(const RealVector &Nl);
  //
  //- Heading: Data
  //

  unsigned short seq_index;

  /// store the allocation_target input specification, prior to run-time
  /// Options right now:
  ///   - Mean = First moment (Mean)
  ///   - Variance = Second moment (Variance or standard deviation depending on moments central or standard)
  short allocationTarget;

  /// option to switch on numerical optimization for solution of sample alloation
  /// of allocationTarget Variance
  bool useTargetVarianceOptimizationFlag;

  /// store the qoi_aggregation_norm input specification, prior to run-time
  /// Options right now:
  ///   - sum = aggregate the variance over all QoIs, compute samples from that
  ///   - max = take maximum sample allocation over QoIs for each level
  short qoiAggregation;

	/// store the convergence_tolerance_type input specification, prior to run-time
  /// Options right now:
  ///   - relative = computes reference tolerance in first iteration and sets convergence_tolerance as 
  ///								 reference tolerance * convergence_tol
  ///   - absolute = sets convergence tolerance from input
  short convergenceTolType;

  /// store the convergence_tolerance_target input specification, prior to run-time
  /// Options right now:
  ///   - variance_constraint = minimizes cost for equality constraint on variance of estimator (rhs of constraint from convergenceTol)
  ///   - cost_constraint = minizes variance of estimator for equality constraint on cost (rhs of constraint from convergenceTol)
  short convergenceTolTarget;

  RealVector convergenceTolVec;

  /// "scalarization" response_mapping matrix applied to the mlmc sample allocation
  /// when a scalarization, i.e. alpha_1 * mean + alpha_2 * sigma, is the target. 
  RealMatrix scalarizationCoeffs;

  /// Helper data structure to store intermedia sample allocations
  RealMatrix NTargetQoI;
  //RealMatrix NTargetQoIFN;

  IntRealMatrixMap levQoisamplesmatrixMap;
  bool storeEvals;
  int bootstrapSeed;

  short cov_approximation_type;
  enum {COV_BOOTSTRAP, COV_PEARSON, COV_CORRLIFT};
};


inline NonDMultilevelSampling::~NonDMultilevelSampling()
{ }


inline void NonDMultilevelSampling::
configure_indices(size_t group, size_t form, size_t lev, short seq_type)
{
  // preserve special values across type conversions
  unsigned short grp = (group == SZ_MAX) ? USHRT_MAX : (unsigned short)group,
                 frm = (form  == SZ_MAX) ? USHRT_MAX : (unsigned short)form;
  configure_indices(grp, frm, lev, seq_type);
}


inline Real NonDMultilevelSampling::
level_cost(const RealVector& cost, size_t step, size_t offset)
{
  // discrepancies incur two level costs
  size_t offset_step = offset + step;
  return (step) ?
    cost[offset_step] + cost[offset_step-1] : // aggregated {HF,LF} mode
    cost[offset_step];                        //     uncorrected LF mode
}


inline bool NonDMultilevelSampling::discrepancy_sample_counts() const
{ return true; }


inline void NonDMultilevelSampling::
nested_response_mappings(const RealMatrix& primary_coeffs,
			 const RealMatrix& secondary_coeffs)
{
  if (scalarizationCoeffs.empty()){// && allocationTarget == TARGET_SCALARIZATION){
    if(primary_coeffs.empty()){
      Cerr << "\nPrimary_response_mapping should not be empty at this point. If you are sure this is correct, "
     << "you need to specify scalarization_response_mapping seperately in multilevel_sampling." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    if (primary_coeffs.numCols() != 2*numFunctions ||
          primary_coeffs.numRows() != 1){
      Cerr << "\nWrong size for primary_response_mapping. If you are sure, it is the right size, e.g.,"
     << " you are interested in quantiles, you need to specify scalarization_response_mapping seperately in multilevel_sampling." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    if (!secondary_coeffs.empty()){
      if (secondary_coeffs.numCols() != 2*numFunctions){
      Cerr << "\nWrong size for columns of secondary_response_mapping. If you are sure, it is the right size, e.g.,"
     << " you are interested in quantiles, you need to specify scalarization_response_mapping seperately in multilevel_sampling." << std::endl;
      abort_handler(METHOD_ERROR);
      }
    }
    size_t nb_rows = primary_coeffs.numRows() + secondary_coeffs.numRows();
    if (nb_rows > numFunctions){
      Cerr << "\nWrong size for rows of response_mapping. If you are sure, it is the right size, e.g.,"
     << ", you need to specify scalarization_response_mapping seperately in multilevel_sampling." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    size_t nb_cols = primary_coeffs.numCols();
    scalarizationCoeffs.reshape(numFunctions, nb_cols);
    for(size_t row_qoi = 0; row_qoi < numFunctions; ++row_qoi){
      scalarizationCoeffs(0, row_qoi*2) = primary_coeffs(0, row_qoi*2);
      scalarizationCoeffs(0, row_qoi*2+1) = primary_coeffs(0, row_qoi*2+1);
    }
    for(size_t qoi = 1; qoi < nb_rows; ++qoi){
      for(size_t row_qoi = 0; row_qoi < numFunctions; ++row_qoi){
	      scalarizationCoeffs(qoi, row_qoi*2)
	                    = secondary_coeffs(qoi-1, row_qoi*2);
	      scalarizationCoeffs(qoi, row_qoi*2+1)
	                    = secondary_coeffs(qoi-1, row_qoi*2+1);
      }
    }
    for(size_t qoi = nb_rows; qoi < numFunctions; ++qoi){
      for(size_t row_qoi = 0; row_qoi < numFunctions; ++row_qoi){
        scalarizationCoeffs(qoi, row_qoi*2)
                      = 0.;
        scalarizationCoeffs(qoi, row_qoi*2+1)
                      = 0.;
      }
    }
  }
}


inline Real NonDMultilevelSampling::
variance_Ysum(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq)
{
  // Note: precision loss in variance is difficult to avoid without
  // storing full sample history; must accumulate Y^2 across iterations
  // instead of (Y-mean)^2 since mean is updated on each iteration.

  Real var_Y = (sum_YY - sum_Y * sum_Y / Nlq) / (Nlq - 1); // unbiased

  if(var_Y < 0){
    Cerr << "NonDMultilevelSampling::variance_Ysum: var_Y < 0" << std::endl;
  }
  check_negative(var_Y);
  return var_Y;
}


inline void NonDMultilevelSampling::
variance_Ysum(const Real* sum_Y, const Real* sum_YY, const SizetArray& N_l,
	      Real* var_Y)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    var_Y[qoi] = variance_Ysum(sum_Y[qoi], sum_YY[qoi], N_l[qoi]);
}


inline Real NonDMultilevelSampling::
variance_Ysum_static(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Y = sum_Y / Nlq_pilot;
  // Note: precision loss in variance is difficult to avoid without
  // storing full sample history; must accumulate Y^2 across iterations
  // instead of (Y-mean)^2 since mean is updated on each iteration.
  Real variance_tmp = (sum_YY / Nlq_pilot - mu_Y * mu_Y); 

  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
    grad = 0;  //TODO_SCALARBUGFIX
  }

  Nlq = Nlq_pilot;  //TODO_SCALARBUGFIX
  Real var_Y = variance_tmp * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  if(var_Y < 0){
    Cerr << "NonDMultilevelSampling::variance_Ysum_static: var_Y < 0" << std::endl;
  }
  check_negative(var_Y);
  return var_Y;
}

inline Real NonDMultilevelSampling::
variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq)
{
  Real mu_Ql = sum_Ql / Nlq, mu_Qlm1 = sum_Qlm1 / Nlq;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real var_Q = ( sum_QlQl / Nlq - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq - mu_Qlm1 * mu_Qlm1 ) // var_Qlm1
    * (Real)Nlq / (Real)(Nlq - 1);
  if(var_Q < 0){
    Cerr << "NonDMultilevelSampling::variance_Qsum: var_Q < 0" << std::endl;
  }
  check_negative(var_Q);
  return var_Q; // Bessel's correction
}


inline Real NonDMultilevelSampling::
variance_Qsum_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Ql = sum_Ql / Nlq_pilot, mu_Qlm1 = sum_Qlm1 / Nlq_pilot;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real variance_tmp = ( sum_QlQl / Nlq_pilot - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq_pilot - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq_pilot - mu_Qlm1 * mu_Qlm1 ); // var_Qlm1
 
  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq_pilot - 1.) * (Real)(Nlq_pilot - 1.));
    grad = 0; //TODO_SCALARBUGFIX
  }
  if(variance_tmp < 0){
    Cerr << "NonDMultilevelSampling::variance_Qsum: variance_tmp < 0" << std::endl;
  }
  check_negative(variance_tmp);

  Nlq = Nlq_pilot;  //TODO_SCALARBUGFIX
  return variance_tmp * (Real)Nlq / (Real)(Nlq - 1.); // Bessel's correction
}

inline Real NonDMultilevelSampling::
var_lev_l(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq)
{
  Real mu_Ql = sum_Ql / Nlq, mu_Qlm1 = sum_Qlm1 / Nlq;
  Real var_lev_Q = (sum_QlQl / Nlq - mu_Ql * mu_Ql)     // var_Ql
    			- (sum_Qlm1Qlm1 / Nlq - mu_Qlm1 * mu_Qlm1 ) // var_Qlm1
    * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  //if(var_lev_Q < 0){
  //  Cerr << "NonDMultilevelSampling::var_lev_l: var_lev_Q < 0: " << var_lev_Q << " But that should be fine.\n";
    //check_negative(var_of_var);
  //}
  return var_lev_Q; 
}

inline Real NonDMultilevelSampling::
var_lev_l_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Ql = sum_Ql / Nlq_pilot, mu_Qlm1 = sum_Qlm1 / Nlq_pilot;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  //Cerr << "1NonDMultilevelSampling::var_lev_l_static: " << sum_QlQl << ", " << Nlq_pilot << ", " << mu_Ql << ", " <<" \n";
  //Cerr << "1NonDMultilevelSampling::var_lev_l_static: " << sum_Qlm1Qlm1 << ", " << Nlq_pilot << ", " << mu_Qlm1 << ", " <<" \n";
  Real var_lev_Q = (sum_QlQl / Nlq_pilot - mu_Ql   * mu_Ql)     // var_Ql
    										- (sum_Qlm1Qlm1 / Nlq_pilot - mu_Qlm1 * mu_Qlm1 ); // var_Qlm1
  //sCerr << "2NonDMultilevelSampling::var_lev_l_static: " << var_lev_Q << ", " << Nlq << " But that should be fine.\n";
  if(compute_gradient){
  	grad = var_lev_Q * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
    grad = 0;  //TODO_SCALARBUGFIX
  }
  var_lev_Q *= (Real)Nlq_pilot / (Real)(Nlq_pilot - 1.); // Bessel's correction

  //if(var_lev_Q < 0){
  //  Cerr << "NonDMultilevelSampling::var_lev_l_static: var_lev_Q < 0: " << var_lev_Q << " But that should be fine.\n";
    //check_negative(var_of_var);
  //}
  return var_lev_Q; 
}


inline Real NonDMultilevelSampling::
aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			const SizetArray& N_l)
{
  Real agg_var_l = 0.;//, var_Y;
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    agg_var_l += variance_Ysum(sum_Y[qoi], sum_YY[qoi], N_l[qoi]);
  return agg_var_l;
}


inline Real NonDMultilevelSampling::
aggregate_variance_Qsum(const Real* sum_Ql,      const Real* sum_Qlm1,
			const Real* sum_QlQl,     const Real* sum_QlQlm1,
			const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			const size_t lev)
{
  Real agg_var_l = 0.;//, var_Y;
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    agg_var_l += variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQl, sum_QlQlm1,
			       sum_Qlm1Qlm1, N_l, lev, qoi);
  return agg_var_l;
}


inline Real NonDMultilevelSampling::
variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
	      const Real* sum_QlQl,     const Real* sum_QlQlm1,
	      const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
	      const size_t lev, const size_t qoi)
{
  Real var_l = (lev) ?
    variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi], sum_QlQlm1[qoi],
		  sum_Qlm1Qlm1[qoi], N_l[qoi]) :
    variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], N_l[qoi]);
  return var_l;
}


inline void NonDMultilevelSampling::
variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
	      const Real* sum_QlQl,     const Real* sum_QlQlm1,
	      const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
	      const size_t lev, Real* var_Yl)
{
  if (lev)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      var_Yl[qoi] = variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi],
				  sum_QlQlm1[qoi], sum_Qlm1Qlm1[qoi], N_l[qoi]);
  else
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      var_Yl[qoi] = variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], N_l[qoi]);
 }


inline Real NonDMultilevelSampling::
aggregate_mse_Yvar(const Real* var_Y, const SizetArray& N_l)
{
  Real agg_mse = 0.;
  // aggregate MC estimator variance for each QoI
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    agg_mse += var_Y[qoi] / N_l[qoi];
  return agg_mse;
}


inline Real NonDMultilevelSampling::
aggregate_mse_Ysum(const Real* sum_Y, const Real* sum_YY, const SizetArray& N_l)
{
  Real agg_mse = 0.;//, var_Y;
  // aggregate MC estimator variance for each QoI
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    agg_mse += variance_Ysum(sum_Y[qoi], sum_YY[qoi], N_l[qoi]) / N_l[qoi];
  return agg_mse;
}


inline void NonDMultilevelSampling::
aggregate_mse_target_Qsum(RealMatrix& agg_var_qoi, const Sizet2DArray& N_l,
			  const size_t step, RealVector& estimator_var0_qoi)
{
  for (size_t qoi = 0; qoi < numFunctions; ++qoi)
    estimator_var0_qoi[qoi] += agg_var_qoi(qoi, step) / N_l[step][qoi];
}


inline void NonDMultilevelSampling::
set_convergence_tol(const RealVector& estimator_var0_qoi, const RealVector& cost, RealVector& eps_sq_div_2_qoi)
{
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
	  if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
      // compute epsilon target based on relative tolerance: total MSE = eps^2
      // which is equally apportioned (eps^2 / 2) among discretization MSE and
      // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
      // discretization error, we compute an initial estimator variance and
      // then seek to reduce it by a relative_factor <= 1.
		  if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE){
				eps_sq_div_2_qoi[qoi] = estimator_var0_qoi[qoi] * convergenceTolVec[qoi];
      // compute epsilon target based on absolute tolerance which is given.
			}else if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_ABSOLUTE){
				eps_sq_div_2_qoi[qoi] = convergenceTolVec[qoi];
			}else{
	  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolType is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}
		}else if (convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
      //Relative cost with respect to convergenceTol evaluations on finest grid
			if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE){
				eps_sq_div_2_qoi[qoi] = cost[cost.length()-1] * convergenceTolVec[qoi]; 
      //Absolute cost which is given.
			}else if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_ABSOLUTE){
				eps_sq_div_2_qoi[qoi] = convergenceTolVec[qoi]; 
			}else{
	  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolType is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}
		}else{
  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolTarget is not known.\n";
  	  abort_handler(INTERFACE_ERROR);
		}

	}
	if (outputLevel == DEBUG_OUTPUT)
	       Cout << "Epsilon squared target per QoI = " << eps_sq_div_2_qoi << std::endl;
}


inline void NonDMultilevelSampling::
ml_Q_raw_moments(const RealMatrix& sum_Hl_1, const RealMatrix& sum_Hlm1_1,
		 const RealMatrix& sum_Hl_2, const RealMatrix& sum_Hlm1_2,
		 const Sizet2DArray& N_actual, size_t start, size_t end,
		 RealMatrix& ml_raw_mom)
{
  // MLMC without CV: sum_H = HF Q sums for levels l and l - 1

  size_t qoi, lev;
  for (lev=start; lev<end; ++lev) {
    const SizetArray& N_l = N_actual[lev];
    const Real *sum_Hl_1l = sum_Hl_1[lev], *sum_Hl_2l = sum_Hl_2[lev];
    if (lev) {
      const Real *sum_Hlm1_1l = sum_Hlm1_1[lev], *sum_Hlm1_2l = sum_Hlm1_2[lev];
      for (qoi=0; qoi<numFunctions; ++qoi) {
	size_t N_lq = N_l[qoi];
	ml_raw_mom(0, qoi) += ( sum_Hl_1l[qoi] - sum_Hlm1_1l[qoi] ) / N_lq;
	ml_raw_mom(1, qoi) += ( sum_Hl_2l[qoi] - sum_Hlm1_2l[qoi] ) / N_lq;
      }
    }
    else
      for (qoi=0; qoi<numFunctions; ++qoi) {
	size_t N_lq = N_l[qoi];
	ml_raw_mom(0, qoi) += sum_Hl_1l[qoi] / N_lq;
	ml_raw_mom(1, qoi) += sum_Hl_2l[qoi] / N_lq;
      }
  }
}


inline void NonDMultilevelSampling::
ml_Y_raw_moments(const RealMatrix& sum_H1, const RealMatrix& sum_H2,
	       const Sizet2DArray& N_l, size_t start, size_t end,
	       RealMatrix& ml_raw_mom)
{
  // MLMC without CV: sum_H = HF Q sums for lev 0 and HF Y sums for lev > 0

  size_t qoi, lev;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    for (lev=start; lev<end; ++lev) {
      size_t Nlq = N_l[lev][qoi];
      ml_raw_mom(0, qoi) += sum_H1(qoi,lev) / Nlq;
      ml_raw_mom(1, qoi) += sum_H2(qoi,lev) / Nlq;
    }
  }
}


inline void NonDMultilevelSampling::
ml_Y_raw_moments(const RealMatrix& sum_H1, const RealMatrix& sum_H2,
	       const RealMatrix& sum_H3, const RealMatrix& sum_H4,
	       const Sizet2DArray& N_l, size_t start, size_t end,
	       RealMatrix& ml_raw_mom)
{
  // MLMC without CV: sum_H = HF Q sums for lev 0 and HF Y sums for lev > 0

  size_t qoi, lev;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    for (lev=start; lev<end; ++lev) {
      size_t Nlq = N_l[lev][qoi];
      ml_raw_mom(0, qoi) += sum_H1(qoi,lev) / Nlq;
      ml_raw_mom(1, qoi) += sum_H2(qoi,lev) / Nlq;
      ml_raw_mom(2, qoi) += sum_H3(qoi,lev) / Nlq;
      ml_raw_mom(3, qoi) += sum_H4(qoi,lev) / Nlq;
    }
  }
}


inline void NonDMultilevelSampling::
compute_ml_estimator_variance(const RealMatrix&   var_Y,
			      const Sizet2DArray& num_Y, RealVector& ml_est_var)
{
  ml_est_var.size(numFunctions); // init to 0
  size_t qoi, lev, num_lev = num_Y.size();
  for (lev=0; lev<num_lev; ++lev) {
    const Real*       var_Yl = var_Y[lev];
    const SizetArray& num_Yl = num_Y[lev];
    for (qoi=0; qoi<numFunctions; ++qoi)
      ml_est_var[qoi] += var_Yl[qoi] / num_Yl[qoi];
  }
}


inline void NonDMultilevelSampling::
update_projected_samples(const SizetArray& delta_N_l, //Sizet2DArray& N_actual,
			 SizetArray& N_alloc, const RealVector& cost,
			 Real& delta_equiv_hf)
{
  size_t actual_incr, alloc_incr, offline_N_lwr = 0,
    lev, num_lev = cost.length();
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION)
    offline_N_lwr = (finalStatsType == QOI_STATISTICS) ? 2 : 1;
  Real ref_cost = cost[num_lev-1];

  for (lev=0; lev<num_lev; ++lev) {
    actual_incr = std::max(delta_N_l[lev], offline_N_lwr);
    //increment_samples(N_actual[lev], actual_incr);
    increment_ml_equivalent_cost(actual_incr, level_cost(cost, lev), ref_cost,
				 delta_equiv_hf);
    if (backfillFailures) {
      alloc_incr = allocation_increment(N_alloc[lev], NTargetQoI[lev]);
      N_alloc[lev] += std::max(alloc_incr, offline_N_lwr);
    }
    else
      N_alloc[lev] += actual_incr;
  }
}


inline void NonDMultilevelSampling::
increment_ml_equivalent_cost(size_t new_N_l, Real lev_cost, Real ref_cost,
			     Real& equiv_hf)
{
  // increment the equivalent number of HF evaluations
  if (new_N_l) {
    equiv_hf += new_N_l * lev_cost / ref_cost; // normalize into equiv HF
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "ML incremented by " << new_N_l << " level samples.  "
	   << "equivalent HF evals = " << equiv_hf << std::endl;
  }
}


inline Real NonDMultilevelSampling::
compute_ml_equivalent_cost(const SizetArray& raw_N_l, const RealVector& cost)
{
  size_t step, num_steps = raw_N_l.size();  Real equiv_hf = 0.;
  equiv_hf = raw_N_l[0] * cost[0]; // first level is single eval
  for (step=1; step<num_steps; ++step) // subsequent levels incur 2 model costs
    equiv_hf += raw_N_l[step] * (cost[step] + cost[step - 1]);
  equiv_hf /= cost[num_steps - 1]; // normalize into equivalent HF evals
  return equiv_hf;
}


inline void NonDMultilevelSampling::
recover_variance(const RealMatrix& moment_stats, RealVector& var_H)
{
  if (var_H.empty()) var_H.sizeUninitialized(numFunctions);

  if (finalMomentsType == Pecos::CENTRAL_MOMENTS)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      var_H[qoi] = moment_stats(1, qoi); // central already computed
  else {
    Real stdev_q;
    for (size_t qoi=0; qoi<numFunctions; ++qoi) {
      stdev_q    = moment_stats(1, qoi);
      var_H[qoi] = stdev_q * stdev_q;    // standardized to central
    }
  }
}


/*
inline void NonDMultilevelSampling::accumulate_offsets(RealVector& mu)
{
  IntRespMCIter r_it = allResponses.begin();
  size_t qoi, num_samp, num_fns = r_it->second.num_functions();
  mu.sizeUninitialized(num_fns);
  Real q_l, sum;
  for (qoi=0; qoi<num_fns; ++qoi) {
    num_samp = 0; sum = 0.;
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      q_l = r_it->second.function_value(qoi);
      if (std::isfinite(q_l)) // neither NaN nor +/-Inf
	{ sum += q_l; ++num_samp; }
    }
    mu[qoi] = sum / num_samp;
  }
}
*/


inline Real NonDMultilevelSampling::var_of_var_ml_l0(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
                                                      const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const bool compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm2l_sq, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1), //.at() returns a const member
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);

  uncentered_to_centered(sum_Q1l(qoi, 0) / Nlq_pilot, sum_Q2l(qoi, 0) / Nlq_pilot,
                         sum_Q3l(qoi, 0) / Nlq_pilot, sum_Q4l(qoi, 0) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);

  cm2l_sq = cm2l * cm2l;
  var_of_var = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);

  if(compute_gradient) {
    grad_g = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
             ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
             - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
               ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq;
  }

  //Cout << "NonDMultilevelSampling::var_of_var_ml_l0: (Qoi, 0): " << qoi << ", 0" << ") Var[Var]: " << var_of_var << std::endl;
  if(var_of_var < 0){
    Cerr << "NonDMultilevelSampling::var_of_var_ml_l0(qoi = " << qoi << "): var_of_var < 0.";
    check_negative(var_of_var);
  }
  return var_of_var;
}


inline Real NonDMultilevelSampling::var_of_var_ml_lmax(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
                                                     const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const bool compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm2l_sq, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);

  uncentered_to_centered(sum_Q1l(qoi, 1) / Nlq_pilot, sum_Q2l(qoi, 1) / Nlq_pilot,
                         sum_Q3l(qoi, 1) / Nlq_pilot, sum_Q4l(qoi, 1) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);

  cm2l_sq = cm2l * cm2l;
  var_of_var = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);

  if(compute_gradient) {
    grad_g = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
             ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
             - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
               ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq;
  }

  if(var_of_var < 0){
    Cerr << "NonDMultilevelSampling::var_of_var_ml_lmax(qoi = " << qoi << "): var_of_var < 0.";
    check_negative(var_of_var);
  }
  return var_of_var;
}

inline Real NonDMultilevelSampling::var_of_var_ml_l(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
                                                    const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const size_t lev, const bool compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm1lm1, cm2lm1,
      cm3lm1, cm4lm1, cm1l_sq, cm2l_sq, cm2lm1_sq,
      mu_Q2l, mu_Q2lm1, mu_Q2lQ2lm1,
      mu_Q1lm1_mu_Q2lQ1lm1, mu_Q1lm1_mu_Q1lm1_muQ2l, mu_Q1l_mu_Q1lQ2lm1, mu_Q1l_mu_Q1l_mu_Q2lm1,
      mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1, mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1, mu_Q2l_muQ2lm1, mu_Q1lQ1lm1_mu_Q1lQ1lm1,
      mu_P2lP2lm1, var_P2l, var_P2lm1, covar_P2lP2lm1, term, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);

  mu_Q2l = sum_Q2l(qoi, lev) / Nlq_pilot;
  uncentered_to_centered(sum_Q1l(qoi, lev) / Nlq_pilot, mu_Q2l,
                         sum_Q3l(qoi, lev) / Nlq_pilot, sum_Q4l(qoi, lev) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);
  mu_Q2lm1 = sum_Q2lm1(qoi, lev) / Nlq_pilot;
  uncentered_to_centered(sum_Q1lm1(qoi, lev) / Nlq_pilot, mu_Q2lm1,
                         sum_Q3lm1(qoi, lev) / Nlq_pilot, sum_Q4lm1(qoi, lev) / Nlq_pilot,
                         cm1lm1, cm2lm1, cm3lm1, cm4lm1, Nlq_pilot);
  cm2l_sq = cm2l * cm2l;
  cm2lm1_sq = cm2lm1 * cm2lm1;


  // [fm] bias correction for var_P2l and var_P2lm1
  var_P2l = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);
  var_P2lm1 = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4lm1 - (Nlq - 3.) / (Nlq - 1.) * cm2lm1_sq);

  //[fm] unbiased products of mean
  mu_Q2lQ2lm1 = sum_Q2lQ2lm1(qoi, lev) / Nlq_pilot;
  mu_Q1lm1_mu_Q2lQ1lm1 = unbiased_mean_product_pair(sum_Q1lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                    sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1lm1_mu_Q1lm1_muQ2l = unbiased_mean_product_triplet(sum_Q1lm1(qoi, lev), sum_Q1lm1(qoi, lev),
                                                          sum_Q2l(qoi, lev),
                                                          sum_Q2lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                          sum_Q2lQ1lm1(qoi, lev),
                                                          sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1l_mu_Q1lQ2lm1 = unbiased_mean_product_pair(sum_Q1l(qoi, lev), sum_Q1lQ2lm1(qoi, lev), sum_Q2lQ2lm1(qoi, lev),
                                                  Nlq_pilot);
  mu_Q1l_mu_Q1l_mu_Q2lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1l(qoi, lev), sum_Q2lm1(qoi, lev),
                                                         sum_Q2l(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                         sum_Q1lQ2lm1(qoi, lev),
                                                         sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                             sum_Q1lQ1lm1(qoi, lev),
                                                             sum_Q1lQ1lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                             sum_Q1lQ2lm1(qoi, lev),
                                                             sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1 = unbiased_mean_product_pairpair(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                                  sum_Q1lQ1lm1(qoi, lev),
                                                                  sum_Q2l(qoi, lev), sum_Q2lm1(qoi, lev),
                                                                  sum_Q2lQ1lm1(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                                  sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q2l_muQ2lm1 = unbiased_mean_product_pair(sum_Q2l(qoi, lev), sum_Q2lm1(qoi, lev), sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_P2lP2lm1 = mu_Q2lQ2lm1 //E[QL2 Ql2]
                - 2. * mu_Q1lm1_mu_Q2lQ1lm1 //E[Ql] E[QL2Ql]
                + 2. * mu_Q1lm1_mu_Q1lm1_muQ2l //E[Ql]2 E[QL2]
                - 2. * mu_Q1l_mu_Q1lQ2lm1 //E[QL] E[QLQl2]
                + 2. * mu_Q1l_mu_Q1l_mu_Q2lm1 //E[QL]2 E[Ql2]
                + 4. * mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 //E[QL] E[Ql] E[QLQl]
                - 4. * mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1 //E[QL]2 E[Ql]2
                - mu_Q2l_muQ2lm1; //E[QL2] E[Ql2]

  // [fm] unbiased by opening up the square and compute three different term
  mu_Q1lQ1lm1_mu_Q1lQ1lm1 = unbiased_mean_product_pair(sum_Q1lQ1lm1(qoi, lev), sum_Q1lQ1lm1(qoi, lev),
                                                       sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  term = mu_Q1lQ1lm1_mu_Q1lQ1lm1 - 2. * mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 + mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1;

  //[fm] Using only unbiased estimators the sum is also unbiased
  covar_P2lP2lm1
      = (mu_P2lP2lm1 + term / (Nlq - 1.)) / Nlq;

  var_of_var = var_P2l + var_P2lm1 - 2. * covar_P2lP2lm1 ;

  if(compute_gradient) {
    grad_g = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
                     ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
                     - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
                       ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq
                     + ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
                       ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4lm1
                     - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
                       ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2lm1_sq
                     - 2. * (-1. / (Nlq * Nlq) * mu_P2lP2lm1 +
                             (-2. * Nlq + 1.) / ((Nlq * Nlq - Nlq) * (Nlq * Nlq - Nlq)) * term);
  }

  //Cout << "NonDMultilevelSampling::var_of_var_ml_l: (Qoi, lev): " << qoi << ", " << lev << ") Var[Var]: " << var_of_var << std::endl;
  if(var_of_var < 0){
    Cerr << "NonDMultilevelSampling::var_of_var_ml_l(qoi, lev) = (" << qoi << ", " << lev << "): var_of_var < 0.";
    check_negative(var_of_var);
  }
  return var_of_var;
}

inline Real NonDMultilevelSampling::compute_cov_mean_sigma(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev, const bool compute_gradient, Real& grad_g){
  Real mu_Q2l = 0, mu_Q2lm1 = 0, cm1l = 0, cm2l = 0, cm3l = 0, cm4l = 0,
       cm1lm1 = 0, cm2lm1 = 0, cm3lm1 = 0, cm4lm1 = 0;

  Real var_var_l = 0, var_var_lm1 = 0;
  Real grad_var_var_l = 0, grad_var_var_lm1 = 0;
  Real var_sigma_l = 0, var_sigma_lm1 = 0;
  Real grad_var_sigma_l = 0, grad_var_sigma_lm1 = 0;
  Real cov_meanl_varl = 0, cov_meanlm1_varlm1 = 0, 
        cov_meanl_varlm1 = 0, cov_meanlm1_varl = 0;
  Real cov_meanl_sigmal = 0, cov_meanlm1_sigmalm1 = 0, 
        cov_meanl_sigmalm1 = 0, cov_meanlm1_sigmal = 0, cov_mean_sigma = 0;
  Real grad_cov_meanl_varl = 0, grad_cov_meanl_varlm1 = 0, 
        grad_cov_meanlm1_varl = 0, grad_cov_meanlm1_varlm1 = 0;

  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4);

  mu_Q2l = sum_Q2l(qoi, lev) / Nlq_pilot;
  uncentered_to_centered(sum_Q1l(qoi, lev) / Nlq_pilot, mu_Q2l,
                         sum_Q3l(qoi, lev) / Nlq_pilot, sum_Q4l(qoi, lev) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);
  if(lev > 0){
    mu_Q2lm1 = sum_Q2lm1(qoi, lev) / Nlq_pilot;
    uncentered_to_centered(sum_Q1lm1(qoi, lev) / Nlq_pilot, mu_Q2lm1,
                           sum_Q3lm1(qoi, lev) / Nlq_pilot, sum_Q4lm1(qoi, lev) / Nlq_pilot,
                           cm1lm1, cm2lm1, cm3lm1, cm4lm1, Nlq_pilot);
  }

  // Var[var] at level l and lm1
  const Real cm2l_sq = cm2l * cm2l;
  var_var_l = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);
  if(compute_gradient) {
    grad_var_var_l = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
             ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
             - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
               ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq;
  }

  if(lev > 0){
    const Real cm2lm1_sq = cm2lm1 * cm2lm1;
    var_var_lm1 = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4lm1 - (Nlq - 3.) / (Nlq - 1.) * cm2lm1_sq);
    if(compute_gradient) {
      grad_var_var_lm1 =((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
             ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4lm1
             - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
               ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2lm1_sq;
    }
  }

  // Var[sigma] at level l and lm1 using delta method
  var_sigma_l = var_var_l > 0 ? 1./(4.*cm2l)*var_var_l : 0;
  if(compute_gradient) {
    grad_var_sigma_l =  1./(4.*cm2l) * grad_var_var_l;
  }
  if(lev > 0){
    var_sigma_lm1 = var_var_lm1 > 0 ? 1./(4.*cm2lm1)*var_var_lm1 : 0;
    if(compute_gradient){
      grad_var_sigma_lm1 = var_var_lm1 > 0 ? 1./(4.*cm2lm1)*grad_var_var_lm1 : 0;
    }
  } 

  //Cov[mean, var] at level (l, l), (lm1, l), (l, lm1), (lm1, lm1)
  cov_meanl_varl = cm3l/Nlq;
  grad_cov_meanl_varl = (-1)*cm3l/(Nlq*Nlq);
  if(lev > 0){
    grad_cov_meanlm1_varlm1 = (-1)*cm3lm1/(Nlq*Nlq);
    cov_meanlm1_varl = compute_cov_meanlmone_varl(sum_Ql, sum_Qlm1, sum_QlQlm1, 
                                  Nlq_pilot, Nlq, qoi, lev, compute_gradient, grad_cov_meanlm1_varl);
    cov_meanl_varlm1 = compute_cov_meanl_varlmone(sum_Ql, sum_Qlm1, sum_QlQlm1, 
                                  Nlq_pilot, Nlq, qoi, lev, compute_gradient, grad_cov_meanl_varlm1);
    cov_meanlm1_varlm1 = cm3lm1/Nlq;
  }

  //Compute cov_mean_var to cov_mean_sigma transformation using the assumption 
  //that pearson correlation is the same for both, i.e. p[mean, sigma] = p[mean, var]
  cov_meanl_sigmal = var_var_l > 0 ? cov_meanl_varl*std::sqrt(var_sigma_l/var_var_l) : 0;
  if(lev > 0){
    cov_meanlm1_sigmal = var_var_l > 0 ? cov_meanlm1_varl*std::sqrt(var_sigma_l/var_var_l) : 0;
    cov_meanl_sigmalm1 = var_var_lm1 > 0 ? cov_meanl_varlm1*std::sqrt(var_sigma_lm1/var_var_lm1) : 0;
    cov_meanlm1_sigmalm1 = var_var_lm1 > 0 ? cov_meanlm1_varlm1*std::sqrt(var_sigma_lm1/var_var_lm1) : 0;
  }

  if(compute_gradient) {
    grad_cov_meanl_varl = var_var_l > 0 && var_sigma_l > 0 ? compute_grad_cov_meanl_vark(cov_meanl_varl, var_var_l, 
                  var_sigma_l, grad_var_var_l, grad_var_sigma_l, Nlq) : 0;
    if(lev > 0){
      grad_cov_meanlm1_varl = var_var_l > 0 && var_sigma_l > 0 ? compute_grad_cov_meanl_vark(cov_meanlm1_varl, var_var_l, 
                  var_sigma_l, grad_var_var_l, grad_var_sigma_l, Nlq) : 0;
      grad_cov_meanl_varlm1 =  var_var_lm1 > 0 && var_sigma_lm1 > 0 ? compute_grad_cov_meanl_vark(cov_meanl_varlm1, var_var_lm1, 
                  var_sigma_lm1, grad_var_var_lm1, grad_var_sigma_lm1, Nlq) : 0;
      grad_cov_meanlm1_varlm1 =  var_var_lm1 > 0 && var_sigma_lm1 > 0 ? compute_grad_cov_meanl_vark(cov_meanlm1_varlm1, var_var_lm1, 
                  var_sigma_lm1, grad_var_var_lm1, grad_var_sigma_lm1, Nlq) : 0;
    }
  }

  //Compute final cov_mean_sigma from the term
  cov_mean_sigma = cov_meanl_sigmal;
  if(lev > 0){
    cov_mean_sigma += cov_meanlm1_sigmalm1
                    - cov_meanlm1_sigmal
                    - cov_meanl_sigmalm1;
  }

  if(compute_gradient) {
    grad_g = grad_cov_meanl_varl;
    if(lev > 0){
      grad_g += grad_cov_meanlm1_varlm1 
              - grad_cov_meanlm1_varl 
              - grad_cov_meanl_varlm1;
    }
  } 

  
  /*if(compute_gradient){
    Real h = 1e-8;
    RealVector cov_mean_sigma_fd_plus = compute_cov_mean_sigma_fd(sum_Ql, sum_Qlm1, 
                    sum_QlQlm1, Nlq_pilot, Nlq+h, qoi, lev);
    RealVector cov_mean_sigma_fd_minus = compute_cov_mean_sigma_fd(sum_Ql, sum_Qlm1, 
                    sum_QlQlm1, Nlq_pilot, Nlq-h, qoi, lev);

    Cout << "QOI LEV: " << qoi << ", " << lev << std::endl;
    Cout << "\t\t\t var_var_l: " << var_var_l << ", var_var_lm1: " << var_var_lm1 << std::endl;
    Cout << "\t\t\t var_sigma_l: " << var_sigma_l << ", var_sigma_lm1: " << var_sigma_lm1 << std::endl;
    Cout << "\t\t\t grad_cov_mean_sigma: " << grad_g << ", " << (cov_mean_sigma_fd_plus[0] - cov_mean_sigma_fd_minus[0])/(2*h) << std::endl;
    Cout << "\t\t\t grad_cov_meanl_sigmal: " << grad_cov_meanl_varl << ", " << (cov_mean_sigma_fd_plus[1] - cov_mean_sigma_fd_minus[1])/(2*h) << std::endl;
    Cout << "\t\t\t grad_cov_meanlm1_sigmal: " << grad_cov_meanlm1_varl << ", " << (cov_mean_sigma_fd_plus[2] - cov_mean_sigma_fd_minus[2])/(2*h) << std::endl;
    Cout << "\t\t\t grad_cov_meanl_sigmalm1: " << grad_cov_meanl_varlm1 << ", " << (cov_mean_sigma_fd_plus[3] - cov_mean_sigma_fd_minus[3])/(2*h) << std::endl;
    Cout << "\t\t\t grad_cov_meanlm1_sigmalm1: " << grad_cov_meanlm1_varlm1 << ", " << (cov_mean_sigma_fd_plus[4] - cov_mean_sigma_fd_minus[4])/(2*h) << std::endl;
  }*/
  
  /*
  if(lev > 2){
    std::ofstream myfile;
    if(qoi == 0)
      myfile.open("covariance_file_problem18_4lev.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 1)
      myfile.open("covariance_file_cantilever_stress.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 2)
      myfile.open("covariance_file_cantilever_displ.txt", std::ofstream::out | std::ofstream::app);  
    myfile << std::setprecision(16) << qoi << ", " << lev << ", " << cov_mean_sigma << ",  "
           << cov_meanl_sigmal;
    myfile                             << ", " << cov_meanlm1_sigmalm1 << ", " << cov_meanlm1_sigmal << ", " << cov_meanl_sigmalm1;
    myfile << std::setprecision(16) << ", " << cov_meanl_varl   << ", " << cov_meanlm1_varlm1 << ", " << cov_meanlm1_varl << ", " << cov_meanl_varlm1;
    myfile << std::setprecision(16) << ", " << cm3l   << ", " << cm3lm1 << ", " << std::sqrt(var_var_l) << ", " << std::sqrt(var_var_lm1);
    myfile << std::setprecision(16) << ", " << std::sqrt(var_sigma_l) << ", " << std::sqrt(var_sigma_lm1);
    myfile << std::setprecision(16) << std::endl;
    myfile.close();
  }
  else if(lev > 0){
    std::ofstream myfile;
    if(qoi == 0)
      myfile.open("covariance_file_problem18.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 1)
      myfile.open("covariance_file_cantilever_stress.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 2)
      myfile.open("covariance_file_cantilever_displ.txt", std::ofstream::out | std::ofstream::app);  
    myfile << std::setprecision(16) << qoi << ", " << lev << ", " << cov_mean_sigma << ",  "
           << cov_meanl_sigmal;
    myfile                             << ", " << cov_meanlm1_sigmalm1 << ", " << cov_meanlm1_sigmal << ", " << cov_meanl_sigmalm1;
    myfile << std::setprecision(16) << ", " << cov_meanl_varl   << ", " << cov_meanlm1_varlm1 << ", " << cov_meanlm1_varl << ", " << cov_meanl_varlm1;
    myfile << std::setprecision(16) << ", " << cm3l   << ", " << cm3lm1 << ", " << std::sqrt(var_var_l) << ", " << std::sqrt(var_var_lm1);
    myfile << std::setprecision(16) << ", " << std::sqrt(var_sigma_l) << ", " << std::sqrt(var_sigma_lm1);
    myfile << std::setprecision(16) << std::endl;
    myfile.close();
  }*/
       
  if(std::isnan(cov_mean_sigma)){
    Cerr << "Cov_mean_sigma is nan since variance is zero or negative for qoi: " << qoi << " with values: cm2l: " << cm2l << " and cm2lm1: " << cm2lm1;
    Cerr << ". Setting to zero. \n";
    cov_mean_sigma = 0;
  }

  return cov_mean_sigma;
}

inline RealVector NonDMultilevelSampling::compute_cov_mean_sigma_fd(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev){
  Real mu_Q2l = 0, mu_Q2lm1 = 0, cm1l = 0, cm2l = 0, cm3l = 0, cm4l = 0,
       cm1lm1 = 0, cm2lm1 = 0, cm3lm1 = 0, cm4lm1 = 0;
  RealVector cov_values(5);
  Real var_var_l = 0, var_var_lm1 = 0;
  Real var_sigma_l = 0, var_sigma_lm1 = 0;
  Real cov_meanl_varl = 0, cov_meanlm1_varlm1 = 0, 
        cov_meanl_varlm1 = 0, cov_meanlm1_varl = 0;
  Real cov_meanl_sigmal = 0, cov_meanlm1_sigmalm1 = 0, 
        cov_meanl_sigmalm1 = 0, cov_meanlm1_sigmal = 0, cov_mean_sigma = 0;

  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4);

  mu_Q2l = sum_Q2l(qoi, lev) / Nlq_pilot;
  uncentered_to_centered(sum_Q1l(qoi, lev) / Nlq_pilot, mu_Q2l,
                         sum_Q3l(qoi, lev) / Nlq_pilot, sum_Q4l(qoi, lev) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);
  if(lev > 0){
    mu_Q2lm1 = sum_Q2lm1(qoi, lev) / Nlq_pilot;
    uncentered_to_centered(sum_Q1lm1(qoi, lev) / Nlq_pilot, mu_Q2lm1,
                           sum_Q3lm1(qoi, lev) / Nlq_pilot, sum_Q4lm1(qoi, lev) / Nlq_pilot,
                           cm1lm1, cm2lm1, cm3lm1, cm4lm1, Nlq_pilot);
  }

  // Var[var] at level l and lm1
  const Real cm2l_sq = cm2l * cm2l;
  var_var_l = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);

  if(lev > 0){
    const Real cm2lm1_sq = cm2lm1 * cm2lm1;
    var_var_lm1 = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4lm1 - (Nlq - 3.) / (Nlq - 1.) * cm2lm1_sq);
  }

  // Var[sigma] at level l and lm1 using delta method
  var_sigma_l = var_var_l > 0 ? 1./(4.*cm2l)*var_var_l : 0;
  if(lev > 0){
    var_sigma_lm1 = var_var_lm1 > 0 ? 1./(4.*cm2lm1)*var_var_lm1 : 0;
  } 

  //Cov[mean, var] at level (l, l), (lm1, l), (l, lm1), (lm1, lm1)
  cov_meanl_varl = cm3l/Nlq;
  Real dummy_grad = 0;
  if(lev > 0){
    cov_meanlm1_varlm1 = cm3lm1/Nlq;
    cov_meanl_varlm1 = compute_cov_meanl_varlmone(sum_Ql, sum_Qlm1, sum_QlQlm1, 
                                  Nlq_pilot, Nlq, qoi, lev, false, dummy_grad);
    cov_meanlm1_varl = compute_cov_meanlmone_varl(sum_Ql, sum_Qlm1, sum_QlQlm1, 
                                  Nlq_pilot, Nlq, qoi, lev, false, dummy_grad);
  }

  //Compute cov_mean_var to cov_mean_sigma transformation
  cov_meanl_sigmal = var_var_l > 0 ? cov_meanl_varl*std::sqrt(var_sigma_l/var_var_l) : 0;
  if(lev > 0){
    cov_meanlm1_sigmal = var_var_l > 0 ?  cov_meanlm1_varl*std::sqrt(var_sigma_l/var_var_l) : 0;
    cov_meanl_sigmalm1 = var_var_lm1 > 0 ?  cov_meanl_varlm1*std::sqrt(var_sigma_lm1/var_var_lm1) : 0;
    cov_meanlm1_sigmalm1 = var_var_lm1 > 0 ?  cov_meanlm1_varlm1*std::sqrt(var_sigma_lm1/var_var_lm1) : 0;
  }

  //Compute final cov_mean_sigma from the term
  cov_mean_sigma = cov_meanl_sigmal;
  if(lev > 0){
    cov_mean_sigma += cov_meanlm1_sigmalm1
                    - cov_meanlm1_sigmal
                    - cov_meanl_sigmalm1;
  }

  cov_values[0] = cov_mean_sigma;
  cov_values[1] = cov_meanl_sigmal;
  cov_values[2] = cov_meanlm1_sigmal;
  cov_values[3] = cov_meanl_sigmalm1;
  cov_values[4] = cov_meanlm1_sigmalm1;

  return cov_values;
}

inline Real NonDMultilevelSampling::compute_cov_meanl_varlmone(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1,
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev, const bool compute_gradient, Real& grad_g){
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1), &sum_Q2lm1 = sum_Qlm1.at(2);
  IntIntPair pr11(1, 1), pr12(1, 2);
  const RealMatrix &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12);

  Real term1_mu_Q1lQ2lm1 = sum_Q1lQ2lm1(qoi, lev)/Nlq_pilot; // mean(Ql.*(Qlmone).^2)
  Real term2_mu_Q1l_mu_Q2lm1 = unbiased_mean_product_pair(sum_Q1l(qoi, lev), sum_Q2lm1(qoi, lev),
                                                    sum_Q1lQ2lm1(qoi, lev), Nlq_pilot); //mean(Ql)*mean(Qlmone.^2)
  Real term3_mu_Q1lm1_mu_Q1lQ1lm1 = unbiased_mean_product_pair(sum_Q1lm1(qoi, lev), sum_Q1lQ1lm1(qoi, lev),
                                                    sum_Q1lQ2lm1(qoi, lev), Nlq_pilot); //mean(Qlmone)*mean(Ql.*Qlmone)
  Real term4_mu_Q1l_mu_Q1lm1_mu_Q1lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev), sum_Q1lm1(qoi, lev),
                                                          sum_Q1lQ1lm1(qoi, lev), sum_Q1lQ1lm1(qoi, lev), sum_Q2lm1(qoi, lev),
                                                          sum_Q1lQ2lm1(qoi, lev), Nlq_pilot); //mean(Ql)*mean(Qlmone)^2

  Real cov_numerator = (term1_mu_Q1lQ2lm1 - term2_mu_Q1l_mu_Q2lm1 
                            - 2.*term3_mu_Q1lm1_mu_Q1lQ1lm1 + 2.*term4_mu_Q1l_mu_Q1lm1_mu_Q1lm1);
  /*
  if(lev > 2){
    std::ofstream myfile;
    if(qoi == 0)
      myfile.open("covariance_file_covmeanlvarlmoneterms_problem18_4lev.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 1)
      myfile.open("covariance_file_covmeanlvarlmoneterms_cantilever_stress.txt", std::ofstream::out | std::ofstream::app);  
    if(qoi == 2)
      myfile.open("covariance_file_covmeanlvarlmoneterms_cantilever_displ.txt", std::ofstream::out | std::ofstream::app);  
    myfile << qoi << ", " << lev;
    myfile << std::setprecision(16) << ", " << term1_mu_Q1lQ2lm1   << ", " << term2_mu_Q1l_mu_Q2lm1 << ", " << 2.*term3_mu_Q1lm1_mu_Q1lQ1lm1 << ", " << 2.*term4_mu_Q1l_mu_Q1lm1_mu_Q1lm1;
    myfile << std::endl;
    myfile.close();
  }
  else if(lev > 0){
    std::ofstream myfile;
    if(qoi == 0)
      myfile.open("covariance_file_covmeanlvarlmoneterms_problem18.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 1)
      myfile.open("covariance_file_covmeanlvarlmoneterms_cantilever_stress.txt", std::ofstream::out | std::ofstream::app);  
    if(qoi == 2)
      myfile.open("covariance_file_covmeanlvarlmoneterms_cantilever_displ.txt", std::ofstream::out | std::ofstream::app); 
    myfile << qoi << ", " << lev;
    myfile << std::setprecision(16) << ", " << term1_mu_Q1lQ2lm1   << ", " << term2_mu_Q1l_mu_Q2lm1 << ", " << 2.*term3_mu_Q1lm1_mu_Q1lQ1lm1 << ", " << 2.*term4_mu_Q1l_mu_Q1lm1_mu_Q1lm1;
    myfile << std::endl;
    myfile.close();
  }*/

  if(compute_gradient) {
    grad_g = (-1)*cov_numerator/(Nlq*Nlq);
  }
  return cov_numerator/Nlq;
}

inline Real NonDMultilevelSampling::compute_cov_meanlmone_varl(const IntRealMatrixMap& sum_Ql, 
                  const IntRealMatrixMap& sum_Qlm1,
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const size_t Nlq_pilot, const Real Nlq, const size_t qoi, 
                  const size_t lev, const bool compute_gradient, Real& grad_g){
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1), &sum_Q2l = sum_Ql.at(2);
  IntIntPair pr11(1, 1), pr21(2, 1);
  const RealMatrix &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21);

  /*
  cov_meanlmone_varl_term1_unbiased = @(Ql, Qlmone) mean(Qlmone.*(Ql).^2);
  cov_meanlmone_varl_term2_unbiased = @(Ql, Qlmone) mean_mean_unbiased(Qlmone, Ql.^2, size(Ql, 2));
  cov_meanlmone_varl_term3_unbiased = @(Ql, Qlmone) 2*mean_mean_unbiased(Ql, Qlmone.*Ql, size(Ql, 2)); 
  cov_meanlmone_varl_term4_unbiased = @(Ql, Qlmone) 2*mean_mean_mean_unbiased(Qlmone, Ql, Ql, size(Ql, 2));
  */
                
  Real term1_mu_Q2lQ1lm1 = sum_Q2lQ1lm1(qoi, lev)/Nlq_pilot; //mean(Qlmone.*(Ql).^2)
  Real term2_mu_Q2l_mu_Q1lm1 = unbiased_mean_product_pair(sum_Q2l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                    sum_Q2lQ1lm1(qoi, lev), Nlq_pilot); //mean(Qlmone)*mean(Ql.^2)
  Real term3_mu_Q1l_mu_Q1lQ1lm1 = unbiased_mean_product_pair(sum_Q1l(qoi, lev), sum_Q1lQ1lm1(qoi, lev),
                                                    sum_Q2lQ1lm1(qoi, lev), Nlq_pilot); //mean(Ql)*mean(Qlmone.*Ql)
  Real term4_mu_Q1l_mu_Q1l_mu_Q1lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                          sum_Q2l(qoi, lev), sum_Q1lQ1lm1(qoi, lev), sum_Q1lQ1lm1(qoi, lev),
                                                          sum_Q2lQ1lm1(qoi, lev), Nlq_pilot); //mean(Qlmone)*mean(Ql)^2

  Real cov_numerator = (term1_mu_Q2lQ1lm1 - term2_mu_Q2l_mu_Q1lm1 
                            - 2.*term3_mu_Q1l_mu_Q1lQ1lm1 + 2.*term4_mu_Q1l_mu_Q1l_mu_Q1lm1);
  /*
  if(lev > 2){
    std::ofstream myfile;
    if(qoi == 0)
      myfile.open("covariance_file_covmeanlmonevarlterms_problem18_4lev.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 1)
      myfile.open("covariance_file_covmeanlmonevarlterms_cantilever_stress.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 2) 
      myfile.open("covariance_file_covmeanlmonevarlterms_cantilever_displ.txt", std::ofstream::out | std::ofstream::app); 
    myfile << qoi << ", " << lev;
    myfile << std::setprecision(16) << ", " << term1_mu_Q2lQ1lm1   << ", " << term2_mu_Q2l_mu_Q1lm1 << ", " << 2.*term3_mu_Q1l_mu_Q1lQ1lm1 << ", " << 2.*term4_mu_Q1l_mu_Q1l_mu_Q1lm1;
    myfile << std::endl;
    myfile.close();
  }
  else if(lev > 0){
    std::ofstream myfile;
    if(qoi == 0)
      myfile.open("covariance_file_covmeanlmonevarlterms_problem18.txt", std::ofstream::out | std::ofstream::app);
    if(qoi == 1)
      myfile.open("covariance_file_covmeanlmonevarlterms_cantilever_stress.txt", std::ofstream::out | std::ofstream::app); 
    if(qoi == 2) 
      myfile.open("covariance_file_covmeanlmonevarlterms_cantilever_displ.txt", std::ofstream::out | std::ofstream::app);  
    myfile << qoi << ", " << lev;
    myfile << std::setprecision(16) << ", " << term1_mu_Q2lQ1lm1   << ", " << term2_mu_Q2l_mu_Q1lm1 << ", " << 2.*term3_mu_Q1l_mu_Q1lQ1lm1 << ", " << 2.*term4_mu_Q1l_mu_Q1l_mu_Q1lm1;
    myfile << std::endl;
    myfile.close();
  }*/

  if(compute_gradient) {
    grad_g = (-1)*cov_numerator/(Nlq*Nlq);
  }
  return cov_numerator/Nlq;

}

inline Real NonDMultilevelSampling::compute_grad_cov_meanl_vark(const Real cov_mean_var,
                  const Real var_of_var, const Real var_of_sigma, 
                  const Real grad_var_of_var, const Real grad_var_of_sigma, 
                  const Real Nlq){
  Real enumerator = 0, denominator = 0;
  Real term1 = 0, term2 = 0, term3 = 0;
/*
  term1 = Nlq * grad_var_of_sigma * std::sqrt(var_of_var/var_of_sigma);
  term2 = std::sqrt(var_of_var*var_of_sigma);
  term3 = Nlq * grad_var_of_var / std::sqrt(var_of_var);
  enumerator = term1 - term2 - term3;
  denominator = Nlq * var_of_var; 
*/
  term1 = Nlq*var_of_var*grad_var_of_sigma;
  term2 = var_of_sigma*(Nlq * grad_var_of_var + 2.*var_of_var);
  enumerator = term1 - term2;
  denominator = 2*Nlq*var_of_var*var_of_var*sqrt(var_of_sigma/var_of_var);

  return cov_mean_var * enumerator/denominator;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_pair(const Real sumQ1, const Real sumQ2, const Real sumQ1Q2, const size_t Nlq)
{
  Real mean1, mean2, bessel_corr1, bessel_corr2 = 0.;

  mean1 = 1./Nlq * 1./Nlq * sumQ1 * sumQ2;
  mean2 = 1./Nlq * sumQ1Q2;
  bessel_corr1 = (Real)Nlq / ((Real)Nlq - 1.);
  bessel_corr2 = 1. / ((Real)Nlq - 1.);

  return bessel_corr1*mean1 - bessel_corr2*mean2;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_triplet(const Real sumQ1, const Real sumQ2, const Real sumQ3,
                                                                  const Real sumQ1Q2, const Real sumQ1Q3, const Real sumQ2Q3, const Real sumQ1Q2Q3, const size_t Nlq)
{
  Real mean1, mean2, mean3, bessel_corr1, bessel_corr2, bessel_corr3 = 0.;

  mean1 = 1./Nlq * 1./Nlq * 1./Nlq * sumQ1 * sumQ2 * sumQ3;
  mean2 = unbiased_mean_product_pair(sumQ1Q2, sumQ3, sumQ1Q2Q3, Nlq);
  mean2 += unbiased_mean_product_pair(sumQ2Q3, sumQ1, sumQ1Q2Q3, Nlq);
  mean2 += unbiased_mean_product_pair(sumQ1Q3, sumQ2, sumQ1Q2Q3, Nlq);
  mean3 = 1./((Real)Nlq) * sumQ1Q2Q3;
  bessel_corr1 = ((Real)Nlq * (Real)Nlq)/((Nlq - 1.)*(Nlq - 2.));
  bessel_corr2 = 1./(Nlq - 2.);
  bessel_corr3 = 1./((Nlq - 1.)*(Nlq - 2.));

  return bessel_corr1 * mean1 - bessel_corr2 * mean2 - bessel_corr3 * mean3;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_pairpair(const Real sumQ1, const Real sumQ2, const Real sumQ1Q2,
                                                                   const Real sumQ1sq, const Real sumQ2sq,
                                                                   const Real sumQ1sqQ2, const Real sumQ1Q2sq, const Real sumQ1sqQ2sq, const size_t Nlq)
{
  Real mean1, mean2, mean3, mean4, bessel_corr1, bessel_corr2, bessel_corr3, bessel_corr4 = 0.;

  mean1 = 1./Nlq * 1./Nlq * 1./Nlq * 1./Nlq * sumQ1 * sumQ1 * sumQ2 * sumQ2;

  mean2 = unbiased_mean_product_triplet(sumQ1sq, sumQ2, sumQ2, sumQ1sqQ2, sumQ1sqQ2, sumQ2sq, sumQ1sqQ2sq, Nlq);
  mean2 += 4. * unbiased_mean_product_triplet(sumQ1Q2, sumQ1, sumQ2, sumQ1sqQ2, sumQ1Q2sq, sumQ1Q2, sumQ1sqQ2sq, Nlq);
  mean2 += unbiased_mean_product_triplet(sumQ1, sumQ1, sumQ2sq, sumQ1sq, sumQ1Q2sq, sumQ1Q2sq, sumQ1sqQ2sq, Nlq);

  mean3 = unbiased_mean_product_pair(sumQ1sq, sumQ2sq, sumQ1sqQ2sq, Nlq);
  mean3 += 2. * unbiased_mean_product_pair(sumQ1Q2, sumQ1Q2, sumQ1sqQ2sq, Nlq);
  mean3 += 2. * unbiased_mean_product_pair(sumQ1sqQ2, sumQ2, sumQ1sqQ2sq, Nlq);
  mean3 += 2. * unbiased_mean_product_pair(sumQ1, sumQ1Q2sq, sumQ1sqQ2sq, Nlq);

  mean4 = 1./Nlq * sumQ1sqQ2sq;

  bessel_corr1 = ((Real)Nlq * (Real)Nlq * (Real)Nlq)/((Nlq - 1.)*(Nlq - 2.)*(Nlq - 3.));
  bessel_corr2 = 1./(Nlq - 3.);
  bessel_corr3 = 1./((Nlq - 2.)*(Nlq - 3.));
  bessel_corr4 = 1./((Nlq - 1.)*(Nlq - 2.)*(Nlq - 3.));

  return bessel_corr1 * mean1 - bessel_corr2 * mean2 - bessel_corr3 * mean3 - bessel_corr4 * mean4;
}

} // namespace Dakota

#endif
