/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MULTIFIDELITY_SAMPLING_H
#define NOND_MULTIFIDELITY_SAMPLING_H

#include "NonDNonHierarchSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {


/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Multifidelity Monte Carlo (MFMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultifidelitySampling: public NonDNonHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultifidelitySampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultifidelitySampling();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  Real estimator_accuracy_metric();
  //Real estimator_cost_metric();
  void print_variance_reduction(std::ostream& s);

  void estimator_variance_ratios(const RealVector& r_and_N,
				 RealVector& estvar_ratios);

  void augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				       RealVector& lin_ineq_lb,
				       RealVector& lin_ineq_ub);
  Real augmented_linear_ineq_violations(const RealVector& cd_vars,
					const RealMatrix& lin_ineq_coeffs,
					const RealVector& lin_ineq_lb,
					const RealVector& lin_ineq_ub);

  //
  //- Heading: member functions
  //

  void multifidelity_mc();
  void multifidelity_mc_offline_pilot();
  void multifidelity_mc_pilot_projection();

  void mfmc_eval_ratios(const RealMatrix& var_L, const RealMatrix& rho2_LH,
			const RealVector& cost,  SizetArray& approx_sequence,
			DAGSolutionData& soln);
                      //bool for_warm_start = false);
  void mfmc_numerical_solution(const RealMatrix& var_L,
			       const RealMatrix& rho2_LH,
			       const RealVector& cost,
			       SizetArray& approx_sequence,
			       DAGSolutionData& soln);

  void approx_increments(IntRealMatrixMap& sum_L_baseline,
			 IntRealVectorMap& sum_H,  IntRealMatrixMap& sum_LL,
			 IntRealMatrixMap& sum_LH, const SizetArray& N_H_actual,
			 size_t N_H_alloc, const SizetArray& approx_sequence,
			 const DAGSolutionData& soln);
  bool mfmc_approx_increment(const DAGSolutionData& soln,
			     const Sizet2DArray& N_L_actual_refined,
			     SizetArray& N_L_alloc_refined, size_t iter,
			     const SizetArray& approx_sequence,
			     size_t start, size_t end);

  void update_hf_target(const RealVector& cost, DAGSolutionData& soln);
  void update_hf_target(const RealMatrix& rho2_LH,
			const SizetArray& approx_sequence,
			const RealVector& var_H, const RealVector& estvar_iter0,
			RealVector& estvar_ratios, DAGSolutionData& soln);

  void mfmc_estimator_variance(const RealMatrix& rho2_LH,
			       const RealVector& var_H, const SizetArray& N_H,
			       const SizetArray& approx_sequence,
			       RealVector& estvar_ratios,
			       DAGSolutionData& soln);
  //void mfmc_estvar_ratios(const RealMatrix& rho2_LH,
  // 			  const SizetArray& approx_sequence,
  // 			  const RealMatrix& eval_ratios,
  // 			  RealVector& estvar_ratios);
  void mfmc_estvar_ratios(const RealMatrix& rho2_LH,
			  const SizetArray& approx_sequence,
			  const RealVector& avg_eval_ratios,
			  RealVector& estvar_ratios);

private:

  //
  //- Heading: Helper functions
  //

  void initialize_mf_sums(IntRealMatrixMap& sum_L_baseline,
			  IntRealVectorMap& sum_H,
			  IntRealMatrixMap& sum_LL,
			  IntRealMatrixMap& sum_LH, RealVector& sum_HH);

  // shared_increment() cases:
  void accumulate_mf_sums(IntRealMatrixMap& sum_L_baseline,
			  IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LL,
			  IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			  SizetArray& N_shared);
  void accumulate_mf_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
			  RealMatrix& sum_LL, RealMatrix& sum_LH,
			  RealVector& sum_HH, SizetArray& N_shared);
  /*
  void accumulate_mf_sums(IntRealMatrixMap& sum_L_baseline,
			  IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LL,
			  IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			  Sizet2DArray& num_L_baseline, SizetArray& num_H,
			  Sizet2DArray& num_LH);
  void accumulate_mf_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
			  RealMatrix& sum_LL, RealMatrix& sum_LH,
			  RealVector& sum_HH, Sizet2DArray& num_L_baseline,
			  SizetArray& num_H,  Sizet2DArray& num_LH);
  */
  // approx_increment() case:
  void accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  Sizet2DArray& num_L_shared,
			  Sizet2DArray& num_L_refined,
			  const SizetArray& approx_sequence,
			  size_t sequence_start, size_t sequence_end);

  void compute_LH_correlation(const RealMatrix& sum_L_shared,
			      const RealVector& sum_H, const RealMatrix& sum_LL,
			      const RealMatrix& sum_LH,const RealVector& sum_HH,
			      const SizetArray& N_shared, RealMatrix& var_L,
			      RealVector& var_H, RealMatrix& rho2_LH);
  void correlation_sq_to_covariance(const RealMatrix& rho2_LH,
				    const RealMatrix& var_L,
				    const RealVector& var_H,
				    RealMatrix& cov_LH);
  void matrix_to_diagonal_array(const RealMatrix& var_L,
				RealSymMatrixArray& cov_LL);

  void mf_raw_moments(IntRealMatrixMap& sum_L_baseline,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined,
		      IntRealVectorMap& sum_H,  IntRealMatrixMap& sum_LL,
		      IntRealMatrixMap& sum_LH, //const RealMatrix& rho2_LH,
		      const Sizet2DArray& num_L_shared,
		      const Sizet2DArray& num_L_refined,
		      const SizetArray& num_H, RealMatrix& H_raw_mom);

  //void compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
  //			  size_t num_L, size_t num_H, size_t num_LH,Real& beta);

  void update_projected_lf_samples(const DAGSolutionData& soln,
				   const SizetArray& N_H_actual,
				   size_t& N_H_alloc,
				   //SizetArray& delta_N_L_actual,
				   Real& delta_equiv_hf);
  void update_projected_samples(const DAGSolutionData& soln,
				const SizetArray& N_H_actual,
				size_t& N_H_alloc, size_t& delta_N_H_actual,
				//SizetArray& delta_N_L_actual,
				Real& delta_equiv_hf);
  //
  //- Heading: Data
  //

  /// tracks ordering of a metric (correlations, eval ratios) across set of
  /// approximations
  SizetArray approxSequence;

  /// ratio of MFMC to MC estimator variance for the same HF samples,
  /// also known as (1 - R^2)
  RealVector estVarRatios;

  /// controls use of numerical solve option: either a fallback in case of
  /// model misordering (default = NUMERICAL_FALLBACK) or override for
  /// robustness, e.g., to pilot over-estimation (NUMERICAL_OVERRIDE)
  unsigned short numericalSolveMode;

  /// final solution data for MFMC (default DAG = 1,2,...,numApprox)
  DAGSolutionData mfmcSolnData;
};


inline Real NonDMultifidelitySampling::estimator_accuracy_metric()
{ return mfmcSolnData.avgEstVar; }


//inline Real NonDMultifidelitySampling::estimator_cost_metric()
//{ return mfmcSolnData.equivHFAlloc; }


inline void NonDMultifidelitySampling::
initialize_mf_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		   RealVector& sum_HH)
{
  initialize_sums(sum_L_baseline, sum_H, sum_LH, sum_HH);
  std::pair<int, RealMatrix> mat_pr;
  for (int i=1; i<=4; ++i) {
    mat_pr.first = i; // moment number
    sum_LL.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  }
}


inline void NonDMultifidelitySampling::
correlation_sq_to_covariance(const RealMatrix& rho2_LH, const RealMatrix& var_L,
			     const RealVector& var_H, RealMatrix& cov_LH)
{
  if (cov_LH.empty()) cov_LH.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;  Real var_H_q, cov_LH_aq;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    var_H_q = var_H[qoi];
    for (approx=0; approx<numApprox; ++approx)
      cov_LH(qoi,approx)
	= std::sqrt(rho2_LH(qoi,approx) * var_L(qoi,approx) * var_H_q);
  }
}


inline void NonDMultifidelitySampling::
matrix_to_diagonal_array(const RealMatrix& var_L, RealSymMatrixArray& cov_LL)
{
  size_t qoi, approx;
  if (cov_LL.empty()) {
    cov_LL.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi].shape(numApprox); // init to 0 for off-diagonal entries
  }

  Real cov_LH_aq;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    RealSymMatrix& cov_LL_q = cov_LL[qoi];
    for (approx=0; approx<numApprox; ++approx)
      cov_LL_q(approx,approx) = var_L(qoi,approx);
  }
}


/* Not active.  See notes in NonDNonHierarchSampling::compute_correlation()
inline void NonDMultifidelitySampling::
compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		   size_t num_L, size_t num_H, size_t num_LH, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]

  // beta^* = rho_LH sigma_H / sigma_L (same expression as two model case)
  //        = cov_LH / var_L  (since rho_LH = cov_LH / sigma_H / sigma_L)

  Real mu_L = sum_L / num_L;
  if (num_L != num_LH || num_H != num_LH) {
    Real mu_H = sum_H / num_H,
      var_L  = (sum_LL - mu_L * sum_L) / (Real)(num_L - 1), // bessel corr for L
    //var_H  = (sum_HH - mu_H * sum_H) / (Real)(num_H - 1), // bessel corr for H
      cov_LH = (sum_LH - mu_L * mu_H * num_LH) / (Real)(num_LH - 1);// bessel LH
    beta = cov_LH / var_L;
  }
  else // simplify: cancel shared terms
    beta = (sum_LH - mu_L * sum_H) / (sum_LL - mu_L * sum_L);

  //Cout << "compute_mf_control: num_L = " << num_L << " num_H = " << num_H
  //     << " num_LH = " << num_LH << " beta = " << beta;
}
*/

} // namespace Dakota

#endif
