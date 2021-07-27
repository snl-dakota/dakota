/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDACVSampling
//- Description: Class for approximate control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

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

  //
  //- Heading: member functions
  //

  void multifidelity_mc();

  bool approx_increment(const RealMatrix& eval_ratios,
			const Sizet2DArray& N_L_refined,
			const RealVector& hf_targets, size_t iter,
			size_t start, size_t end);

  void allocate_budget(const RealMatrix& eval_ratios, const RealVector& cost,
		       RealVector& hf_targets);

  void update_hf_targets(const RealMatrix& eval_ratios, const RealVector& cost,
			 const RealVector& mse_ratios,  const RealVector& var_H,
			 const SizetArray& N_H, const RealVector& mse_iter0,
			 RealVector& hf_targets);
  void update_hf_targets(const SizetArray& N_H, RealVector& hf_targets);

  void compute_ratios(const RealMatrix& sum_L_baseline, const RealVector& sum_H,
		      const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		      const RealVector& sum_HH, const RealVector& cost,
		      const Sizet2DArray& N_L_baseline, const SizetArray& N_H,
		      const Sizet2DArray& N_LH, RealVector& var_H,
		      RealMatrix& rho2_LH,      RealMatrix& eval_ratios,
		      RealVector& mse_ratios);

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
			  Sizet2DArray& num_L_baseline, SizetArray& num_H,
			  Sizet2DArray& num_LH);
  // approx_increment() cases:
  void accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  Sizet2DArray& num_L_shared,
			  Sizet2DArray& num_L_refined,
			  size_t approx_start, size_t approx_end);

  void compute_LH_correlation(const RealMatrix& sum_L_shared,
			      const RealVector& sum_H, const RealMatrix& sum_LL,
			      const RealMatrix& sum_LH,const RealVector& sum_HH,
			      const Sizet2DArray& num_L,const SizetArray& num_H,
			      const Sizet2DArray& num_LH, RealVector& var_H,
			      RealMatrix& rho2_LH);
  
  void mfmc_raw_moments(IntRealMatrixMap& sum_L_baseline,
			IntRealMatrixMap& sum_L_shared,
			IntRealMatrixMap& sum_L_refined,
			IntRealVectorMap& sum_H,  IntRealMatrixMap& sum_LL,
			IntRealMatrixMap& sum_LH, //const RealMatrix& rho2_LH,
			const Sizet2DArray& num_L_baseline,
			const Sizet2DArray& num_L_shared,
			const Sizet2DArray& num_L_refined,
			const SizetArray& num_H,
			const Sizet2DArray& num_LH, RealMatrix& H_raw_mom);

  void compute_mfmc_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			    size_t num_L, size_t num_H, size_t num_LH,
			    Real& beta);

  //
  //- Heading: Data
  //

};


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
allocate_budget(const RealMatrix& eval_ratios, const RealVector& cost,
		RealVector& hf_targets)
{
  // Scale this profile based on specified budget (maxFunctionEvals) if needed
  // using N_H = maxFunctionEvals / cost^T eval_ratios
  // > Pilot case iter = 0: can only scale back after shared_increment().
  //   Optimal profile can be hidden by one_sided_delta() with pilot --> optimal
  //   shape emerges from initialization cost as for ML cases controlled by
  //   convTol (allow budget overshoot due to overlap of optimal with pilot,
  //   rather than strictly allocating remaining budget)

  if (hf_targets.empty()) hf_targets.sizeUninitialized(numFunctions);
  size_t qoi, approx;
  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // raw cost (un-normalized)
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * eval_ratios(qoi, approx);
    hf_targets[qoi] = budget / inner_prod * cost_H; // normalized to equivHF
  }
}


inline void NonDMultifidelitySampling::
update_hf_targets(const SizetArray& N_H, RealVector& hf_targets)
{
  size_t i, len = N_H.size();
  if (hf_targets.length() != len) hf_targets.sizeUninitialized(len);
  for (i=0; i<len; ++i)
    hf_targets[i] = (Real)N_H[i];
}


inline void NonDMultifidelitySampling::
compute_mfmc_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		     size_t num_L, size_t num_H, size_t num_LH, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  //Real bessel_corr_L  = (Real)num_L  / (Real)(num_L  - 1),
  //     bessel_corr_H  = (Real)num_H  / (Real)(num_H  - 1);
  //     bessel_corr_LH = (Real)num_LH / (Real)(num_LH - 1);
  Real  mu_L  =  sum_L  / num_L,   mu_H = sum_H / num_H,
       var_L  = (sum_LL / num_L  - mu_L * mu_L),// * bessel_corr_L, // defer
     //var_H  = (sum_HH / num_H  - mu_H * mu_H),// * bessel_corr_H, // defer
       cov_LH = (sum_LH / num_LH - mu_L * mu_H);// * bessel_corr_LH;// defer

  // beta^* = rho_LH sigma_H / sigma_L (same expression as two model case)
  //        = cov_LH / var_L  (since rho_LH = cov_LH / sigma_H / sigma_L)
  // Allow different sample counts --> don't cancel bessel_corr:
  beta = cov_LH / var_L;

  //Cout << "compute_mfmc_control: num_L = " << num_L << " num_H = " << num_H
  //     << " num_LH = " << num_LH << std::endl;
  //Cout << "compute_mfmc_control: beta w/o bessel = " << beta;
  //var_L  *= bessel_corr_L;
  //cov_LH *= bessel_corr_LH;
  //Real beta_incl = cov_LH / var_L; // includes bessel corrs
  //Cout << " beta w/ bessel = " << beta_incl << " ratio = " << beta/beta_incl << std::endl;
}

} // namespace Dakota

#endif
