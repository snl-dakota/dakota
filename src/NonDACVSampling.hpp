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

#ifndef NOND_ACV_SAMPLING_H
#define NOND_ACV_SAMPLING_H

#include "NonDEnsembleSampling.hpp"


namespace Dakota {

/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Approximate Control Variate (ACV) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDACVSampling: public NonDEnsembleSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDACVSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDACVSampling();

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
  void approx_control_variate_multifidelity();

  void shared_increment(size_t iter, size_t step);
  bool approx_increment(const RealMatrix& eval_ratios, const Sizet2DArray& N_lf,
			const RealVector& hf_targets, size_t iter, size_t start,
			size_t end);
  void ensemble_sample_increment(size_t iter, size_t step);

  void allocate_budget(const RealMatrix& eval_ratios, const RealVector& cost,
		       RealVector& hf_targets);

private:

  //
  //- Heading: Helper functions
  //

  // manage response mode and active model key from {group,form,lev} triplet.
  // seq_type defines the active dimension for a model sequence.
  //void configure_indices(size_t group,size_t form,size_t lev,short seq_type);

  void initialize_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  IntRealVectorMap& sum_H,
			  IntRealMatrixMap& sum_LL, //IntRealVectorMap& sum_HH,
			  IntRealMatrixMap& sum_LH);
  void accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined, size_t approx_end,
			  //const RealVector& offset,
			  Sizet2DArray& num_L);
  void accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LL,
			  IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			  //const RealVector& offset,
			  Sizet2DArray& num_L, SizetArray& num_H,
			  Sizet2DArray& num_LH);

  void increment_mf_samples(size_t new_N, size_t start, size_t end,
			    SizetArray& N_l);
  void increment_mf_equivalent_cost(size_t new_samp, size_t start, size_t end,
				    const RealVector& cost);

  void compute_eval_ratios(const RealMatrix& sum_L_shared,
			   const RealVector& sum_H,  const RealMatrix& sum_LL,
			   const RealMatrix& sum_LH, const RealVector& sum_HH,
			   const RealVector& cost,   const Sizet2DArray& N_L,
			   const SizetArray& N_H,    const Sizet2DArray& N_LH,
			   RealVector& var_H, RealMatrix& rho2_LH,
			   RealMatrix& eval_ratios);
  void compute_MSE_ratios(const RealMatrix& eval_ratios,
			  const RealMatrix& rho2_LH, const RealVector& cost,
			  RealVector& mse_ratios);
  void compute_mf_correlation(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			      Real sum_HH, size_t N_L, size_t N_H, size_t N_LH,
			      Real& var_H, Real& rho2_LH);

  void cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealVectorMap& sum_H,
		      IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_L_refined,
		      const RealMatrix& rho2_LH, const Sizet2DArray& N_L,
		      const SizetArray& N_H,     const Sizet2DArray& N_LH,
		      RealMatrix& H_raw_mom);
  void compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			  size_t N_L, size_t N_H, size_t N_LH, Real& beta);
  void apply_mf_control(Real sum_L_shared, size_t N_shared, Real sum_L_refined,
			size_t N_refined, Real beta, Real& H_raw_mom);

  //
  //- Heading: Data
  //

  /// number of approximation models managed by non-hierarchical iteratedModel
  size_t numApprox;
};


inline void NonDACVSampling::
increment_mf_samples(size_t new_N, size_t start, size_t end, SizetArray& N_l)
{
  size_t i, len = N_l.size();
  if (start > end || end > len) {
    Cerr << "Error: index range [" << start << "," << end << ") violates size ("
	 << len << ") in NonDACVSampling::increment_mf_samples()" << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (i=start; i<end; ++i)
    N_l[i] += new_N;
}


inline void NonDACVSampling::
increment_mf_equivalent_cost(size_t new_samp, size_t start, size_t end,
			     const RealVector& cost)
{
  size_t i, len = cost.length(), hf_index = len-1;
  Real cost_ref = cost[hf_index];
  if (end == len)
    { equivHFEvals += new_samp; --end; }
  for (i=start; i<end; ++i)
    equivHFEvals += (Real)new_samp * cost[i] / cost_ref;
}


inline void NonDACVSampling::
compute_mf_correlation(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		       Real sum_HH, size_t N_L, size_t N_H, size_t N_LH,
		       Real& var_H, Real& rho2_LH)
{
  Real bessel_corr_L  = (Real)N_L  / (Real)(N_L  - 1),
       bessel_corr_H  = (Real)N_H  / (Real)(N_H  - 1),
       bessel_corr_LH = (Real)N_LH / (Real)(N_LH - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_L, mu_H = sum_H / N_H;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_L = (sum_LL / N_L  - mu_L * mu_L) * bessel_corr_L,
      cov_LH = (sum_LH / N_LH - mu_L * mu_H) * bessel_corr_LH; // *** TO DO: review Bessel correction online --> not the same N to pull out over N-1 ...
  var_H      = (sum_HH / N_H  - mu_H * mu_H) * bessel_corr_H;

  //beta  = cov_LH / var_L;
  rho2_LH = cov_LH / var_L * cov_LH / var_H;
}


inline void NonDACVSampling::
compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		   size_t N_L, size_t N_H, size_t N_LH, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real bessel_corr_L  = (Real)N_L  / (Real)(N_L  - 1),
     //bessel_corr_H  = (Real)N_H  / (Real)(N_H  - 1);
       bessel_corr_LH = (Real)N_LH / (Real)(N_LH - 1);
  Real  mu_L  =  sum_L  / N_L,   mu_H = sum_H / N_H,
       var_L  = (sum_LL / N_L  - mu_L * mu_L) * bessel_corr_L,
     //var_H  = (sum_HH / N_H  - mu_H * mu_H) * bessel_corr_H,
       cov_LH = (sum_LH / N_LH - mu_L * mu_H) * bessel_corr_LH;

  // beta^* = rho_LH sigma_H / sigma_L (same expression as two model case)
  //        = cov_LH / var_L  (since rho_LH = cov_LH / sigma_H / sigma_L)
  // Allow different sample counts --> don't cancel bessel_corr:
  beta = cov_LH / var_L;
}


inline void NonDACVSampling::
apply_mf_control(Real sum_L_shared, size_t N_shared, Real sum_L_refined,
		 size_t N_refined, Real beta, Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom -= beta * (sum_L_shared  / N_shared -  // mu from shared samples
		       sum_L_refined / N_refined); // refined mu incl increment
}


/*
inline void NonDACVSampling::
compute_mf_control(const RealMatrix& sum_L,  const RealVector& sum_H,
		   const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		   const SizetArray& N_shared, size_t approx, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mf_control(sum_L(qoi,approx), sum_H[qoi], sum_LL(qoi,approx),
		       sum_LH(qoi,approx), N_shared[qoi], beta[qoi]);
}


inline void NonDACVSampling::
apply_mf_control(const RealVector& sum_H,    const RealMatrix& sum_L_shared,
		 const SizetArray& N_shared, const RealMatrix& sum_L_refined,
		 const Sizet2DArray& N_refined, size_t approx,
		 const RealVector& beta, RealVector& H_raw_mom)
{
  const SizetArray& N_ref_a = N_refined[approx];
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta = "
	 << std::setw(9) << beta[qoi] << '\n';
    apply_mf_control(sum_H[qoi], sum_L_shared(qoi,approx), N_shared[qoi],
		     sum_L_refined(qoi,approx), N_ref_a[qoi], beta[qoi],
		     H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}
*/

} // namespace Dakota

#endif
