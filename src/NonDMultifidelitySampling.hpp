/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultifidelitySampling
//- Description: Implementation of multifidelity Monte Carlo
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTIFIDELITY_SAMPLING_H
#define NOND_MULTIFIDELITY_SAMPLING_H

#include "NonDHierarchSampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Performs Multifidelity Monte Carlo sampling for UQ.

/** Multifidelity Monte Carlo (MFMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultifidelitySampling: public virtual NonDHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultifidelitySampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultifidelitySampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: Member functions
  //

  /// perform final LF sample increment as indicated by the evaluation ratio
  bool lf_increment(Real avg_eval_ratio, const SizetArray& N_lf,
		    const SizetArray& N_hf, size_t iter, size_t lev);

  /// compute scalar control variate parameters
  void compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			  size_t N_shared, Real& beta);
  /// compute matrix control variate parameters
  void compute_mf_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
			  const RealMatrix& sum_LL, const RealMatrix& sum_LH,
			  const SizetArray& N_shared, size_t lev,
			  RealVector& beta);

  /// compute scalar variance and correlation parameters for control variates
  void compute_mf_correlation(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			      Real sum_HH, size_t N_shared, Real& var_H,
			      Real& rho2_LH);

  /// apply scalar control variate parameter (beta) to approximate HF moment
  void apply_mf_control(Real sum_H, Real sum_L_shared, size_t N_shared,
			Real sum_L_refined, size_t N_refined, Real beta,
			Real& H_raw_mom);
  /// apply matrix control variate parameter (beta) to approximate HF moment
  void apply_mf_control(const RealMatrix& sum_H, const RealMatrix& sum_L_shared,
			const SizetArray& N_shared,
			const RealMatrix& sum_L_refined,
			const SizetArray& N_refined, size_t lev,
			const RealVector& beta, RealVector& H_raw_mom);

  //
  //- Heading: Data
  //

  /// if defined, complete the final CV refinement when terminating MLCV based
  /// on maxIterations (the total number of refinements beyond the pilot sample
  /// will be one more for CV than for ML).  This approach is consistent with
  /// normal termination based on l1_norm(delta_N_hf) = 0.
  bool finalCVRefinement;

private:

  //
  //- Heading: Helper functions
  //

  /// Perform control variate Monte Carlo across two model forms
  void control_variate_mc(const Pecos::ActiveKey& active_key);

  /// perform a shared increment of LF and HF samples for purposes of
  /// computing/updating the evaluation ratio and the MSE ratio
  void shared_increment(size_t iter, size_t lev);

  /// initialize the CV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mf_sums(IntRealVectorMap& sum_L_shared,
			  IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
			  IntRealVectorMap& sum_LH);

  /// update running sums for one model (sum_L) using set of model
  /// evaluations within allResponses
  void accumulate_mf_sums(IntRealVectorMap& sum_L, const RealVector& offset,
			  SizetArray& num_L);
  /// update running sums for two models (sum_L, sum_H, and sum_LH)
  /// from set of low/high fidelity model evaluations within allResponses
  void accumulate_mf_sums(IntRealVectorMap& sum_L_shared,
			  IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H,  IntRealVectorMap& sum_LL,
			  IntRealVectorMap& sum_LH, RealVector& sum_HH,
			  const RealVector& offset, SizetArray& num_L,
			  SizetArray& num_H);

  /// compute the LF/HF evaluation ratio, averaged over the QoI
  Real eval_ratio(const RealVector& sum_L_shared, const RealVector& sum_H,
		  const RealVector& sum_LL,	  const RealVector& sum_LH,
		  const RealVector& sum_HH,       Real cost_ratio,
		  const SizetArray& N_shared,	  RealVector& var_H,
		  RealVector& rho2_LH);

  /// compute ratio of MC and CVMC mean squared errors, averaged over the QoI
  Real MSE_ratio(Real avg_eval_ratio, const RealVector& var_H,
		 const RealVector& rho2_LH, size_t iter,
		 const SizetArray& N_hf);

  /// compute control variate parameters for CVMC and estimate raw moments
  void cv_raw_moments(IntRealVectorMap& sum_L_shared, IntRealVectorMap& sum_H,
		      IntRealVectorMap& sum_LL, IntRealVectorMap& sum_LH,
		      const SizetArray& N_shared,
		      IntRealVectorMap& sum_L_refined,
		      const SizetArray& N_refined, const RealVector& rho2_LH,
		      RealMatrix& H_raw_mom);

  /// compute vector control variate parameters
  void compute_mf_control(const RealVector& sum_L, const RealVector& sum_H,
			  const RealVector& sum_LL, const RealVector& sum_LH,
			  const SizetArray& N_shared, RealVector& beta);

  /*
  /// compute vector variance and correlation parameters for control variates
  void compute_mf_correlation(const RealVector& sum_L, const RealVector& sum_H,
			      const RealVector& sum_LL,
			      const RealVector& sum_LH,
			      const RealVector& sum_HH,
			      const SizetArray& N_shared,
			      RealVector& var_H, RealVector& rho2_LH);
  */

  /// apply vector control variate parameter (beta) to approximate HF moment
  void apply_mf_control(const RealVector& sum_H, const RealVector& sum_L_shared,
			const SizetArray& N_shared,
			const RealVector& sum_L_refined,
			const SizetArray& N_refined, const RealVector& beta,
			RealVector& H_raw_mom);

  //
  //- Heading: Data
  //

  /// mean squared error of mean estimator from pilot sample MC on HF model
  RealVector mcMSEIter0;
};


inline void NonDMultifidelitySampling::
compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		   size_t N_shared, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  //Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  //Real var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
  //    cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;

  // Cancel repeated N_shared and bessel_corr within beta = cov_LH / var_L:
  beta = (sum_LH - sum_L * sum_H / N_shared)
       / (sum_LL - sum_L * sum_L / N_shared);
}


inline void NonDMultifidelitySampling::
compute_mf_control(const RealVector& sum_L, const RealVector& sum_H,
		   const RealVector& sum_LL, const RealVector& sum_LH,
		   const SizetArray& N_shared, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mf_control(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		       N_shared[qoi], beta[qoi]);
}


inline void NonDMultifidelitySampling::
compute_mf_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
		   const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		   const SizetArray& N_shared, size_t lev, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mf_control(sum_L(qoi,lev), sum_H(qoi,lev), sum_LL(qoi,lev),
		       sum_LH(qoi,lev), N_shared[qoi], beta[qoi]);
}


inline void NonDMultifidelitySampling::
compute_mf_correlation(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		       Real sum_HH, size_t N_shared, Real& var_H, Real& rho2_LH)
{
  Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
      cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;
  var_H      = (sum_HH / N_shared - mu_H * mu_H) * bessel_corr;

  //beta  = cov_LH / var_L;
  rho2_LH = cov_LH / var_L * cov_LH / var_H;
}


/*
inline void NonDMultifidelitySampling::
compute_mf_correlation(const RealVector& sum_L, const RealVector& sum_H,
		       const RealVector& sum_LL, const RealVector& sum_LH,
		       const RealVector& sum_HH, const SizetArray& N_shared,
		       RealVector& var_H, RealVector& rho2_LH)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mf_correlation(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
			   sum_HH[qoi], N_shared[qoi], var_H[qoi],
			   rho2_LH[qoi]);
}
*/


inline void NonDMultifidelitySampling::
apply_mf_control(Real sum_H, Real sum_L_shared, size_t N_shared,
		 Real sum_L_refined, size_t N_refined, Real beta,
		 Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom = sum_H / N_shared                    // mu_H from shared samples
            - beta * (sum_L_shared  / N_shared -  // mu_L from shared samples
		      sum_L_refined / N_refined); // refined_mu_L incl increment
}


inline void NonDMultifidelitySampling::
apply_mf_control(const RealVector& sum_H,     const RealVector& sum_L_shared,
		 const SizetArray& N_shared,  const RealVector& sum_L_refined,
		 const SizetArray& N_refined, const RealVector& beta,
		 RealVector& H_raw_mom)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta = "
	 << std::setw(9) << beta[qoi] << '\n';
    apply_mf_control(sum_H[qoi], sum_L_shared[qoi], N_shared[qoi],
		     sum_L_refined[qoi], N_refined[qoi], beta[qoi],
		     H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}


inline void NonDMultifidelitySampling::
apply_mf_control(const RealMatrix& sum_H,    const RealMatrix& sum_L_shared,
		 const SizetArray& N_shared, const RealMatrix& sum_L_refined,
		 const SizetArray& N_refined, size_t lev,
		 const RealVector& beta, RealVector& H_raw_mom)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta = "
	 << std::setw(9) << beta[qoi] << '\n';
    apply_mf_control(sum_H(qoi,lev), sum_L_shared(qoi,lev), N_shared[qoi],
		     sum_L_refined(qoi,lev), N_refined[qoi], beta[qoi],
		     H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}

} // namespace Dakota

#endif
