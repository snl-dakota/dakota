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

class NonDMultifidelitySampling: public NonDHierarchSampling
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

  bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void core_run();
  void post_run(std::ostream& s);
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

private:

  //
  //- Heading: Helper functions
  //

  /// Perform control variate Monte Carlo across two model forms
  void control_variate_mc(const Pecos::ActiveKey& active_key);

  /// perform a shared increment of LF and HF samples for purposes of
  /// computing/updating the evaluation ratio and the MSE ratio
  void shared_increment(size_t iter, size_t lev);

  /// return (aggregate) level cost
  Real level_cost(const RealVector& cost, size_t step);

  /// initialize the CV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_cv_sums(IntRealVectorMap& sum_L_shared,
			  IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
			  IntRealVectorMap& sum_LH);

  /// update running sums for one model (sum_L) using set of model
  /// evaluations within allResponses
  void accumulate_cv_sums(IntRealVectorMap& sum_L, const RealVector& offset,
			  SizetArray& num_L);
  /// update running sums for two models (sum_L, sum_H, and sum_LH)
  /// from set of low/high fidelity model evaluations within allResponses
  void accumulate_cv_sums(IntRealVectorMap& sum_L_shared,
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
  /// compute the LF/HF evaluation ratio, averaged over the QoI
  Real eval_ratio(RealMatrix& sum_L_shared, RealMatrix& sum_H,
		  RealMatrix& sum_LL, RealMatrix& sum_LH, RealMatrix& sum_HH,
		  Real cost_ratio, size_t lev, const SizetArray& N_shared,
		  RealMatrix& var_H, RealMatrix& rho2_LH);
  /// compute the LF/HF evaluation ratio, averaged over the QoI
  Real eval_ratio(RealMatrix& sum_Ll,          RealMatrix& sum_Llm1,
		  RealMatrix& sum_Hl,          RealMatrix& sum_Hlm1,
		  RealMatrix& sum_Ll_Ll,       RealMatrix& sum_Ll_Llm1,
		  RealMatrix& sum_Llm1_Llm1,   RealMatrix& sum_Hl_Ll,
		  RealMatrix& sum_Hl_Llm1,     RealMatrix& sum_Hlm1_Ll,
		  RealMatrix& sum_Hlm1_Llm1,   RealMatrix& sum_Hl_Hl,
		  RealMatrix& sum_Hl_Hlm1,     RealMatrix& sum_Hlm1_Hlm1,
		  Real cost_ratio, size_t lev, const SizetArray& N_shared,
		  RealMatrix& var_YHl,         RealMatrix& rho_dot2_LH);

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
  /// apply control variate parameters for MLCVMC to estimate raw
  /// moment contributions
  void cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL,       IntRealMatrixMap& sum_LH,
		      const SizetArray& N_shared,
		      IntRealMatrixMap& sum_L_refined,
		      const SizetArray& N_refined, const RealMatrix& rho2_LH,
		      size_t lev, RealMatrix& H_raw_mom);
  /// apply control variate parameters for MLCVMC to estimate raw
  /// moment contributions
  void cv_raw_moments(IntRealMatrixMap& sum_Ll, IntRealMatrixMap& sum_Llm1,
		      IntRealMatrixMap& sum_Hl, IntRealMatrixMap& sum_Hlm1,
		      IntRealMatrixMap& sum_Ll_Ll,
		      IntRealMatrixMap& sum_Ll_Llm1,
		      IntRealMatrixMap& sum_Llm1_Llm1,
		      IntRealMatrixMap& sum_Hl_Ll,
		      IntRealMatrixMap& sum_Hl_Llm1,
		      IntRealMatrixMap& sum_Hlm1_Ll,
		      IntRealMatrixMap& sum_Hlm1_Llm1,
		      IntRealMatrixMap& sum_Hl_Hl,
		      IntRealMatrixMap& sum_Hl_Hlm1,
		      IntRealMatrixMap& sum_Hlm1_Hlm1,
		      const SizetArray& N_shared,
		      IntRealMatrixMap& sum_Ll_refined,
		      IntRealMatrixMap& sum_Llm1_refined,
		      const SizetArray& N_refined,
		      const RealMatrix& rho_dot2_LH, size_t lev,
		      RealMatrix& H_raw_mom);

  /// compute scalar control variate parameters
  void compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		       size_t N_shared, Real& beta);
  /// compute scalar variance and correlation parameters for control variates
  void compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		       Real sum_HH, size_t N_shared, Real& var_H,
		       Real& rho2_LH);
  /// compute scalar control variate parameters
  void compute_control(Real sum_Ll, Real sum_Llm1, Real sum_Hl, Real sum_Hlm1,
		       Real sum_Ll_Ll, Real sum_Ll_Llm1, Real sum_Llm1_Llm1,
		       Real sum_Hl_Ll, Real sum_Hl_Llm1, Real sum_Hlm1_Ll,
		       Real sum_Hlm1_Llm1, Real sum_Hl_Hl, Real sum_Hl_Hlm1,
		       Real sum_Hlm1_Hlm1, size_t N_shared, Real& var_YH,
		       Real& rho_dot2_LH, Real& beta_dot, Real& gamma);
  /// compute vector control variate parameters
  void compute_control(const RealVector& sum_L, const RealVector& sum_H,
		       const RealVector& sum_LL, const RealVector& sum_LH,
		       const SizetArray& N_shared, RealVector& beta);
  /// compute vector variance and correlation parameters for control variates
  void compute_control(const RealVector& sum_L, const RealVector& sum_H,
		       const RealVector& sum_LL, const RealVector& sum_LH,
		       const RealVector& sum_HH, const SizetArray& N_shared,
		       RealVector& var_H, RealVector& rho2_LH);
  /// compute matrix control variate parameters
  void compute_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
		       const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		       const SizetArray& N_shared, size_t lev,
		       RealVector& beta);
  /// compute matrix control variate parameters
  void compute_control(const RealMatrix& sum_Ll, const RealMatrix& sum_Llm1,
		       const RealMatrix& sum_Hl, const RealMatrix& sum_Hlm1,
		       const RealMatrix& sum_Ll_Ll,
		       const RealMatrix& sum_Ll_Llm1,
		       const RealMatrix& sum_Llm1_Llm1,
		       const RealMatrix& sum_Hl_Ll,
		       const RealMatrix& sum_Hl_Llm1,
		       const RealMatrix& sum_Hlm1_Ll,
		       const RealMatrix& sum_Hlm1_Llm1,
		       const RealMatrix& sum_Hl_Hl,
		       const RealMatrix& sum_Hl_Hlm1,
		       const RealMatrix& sum_Hlm1_Hlm1,
		       const SizetArray& N_shared, size_t lev,
		       RealVector& beta_dot, RealVector& gamma);

  /// apply scalar control variate parameter (beta) to approximate HF moment
  void apply_control(Real sum_H, Real sum_L_shared, size_t N_shared,
		     Real sum_L_refined, size_t N_refined, Real beta,
		     Real& H_raw_mom);
  /// apply scalar control variate parameter (beta) to approximate HF moment
  void apply_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll, Real sum_Llm1,
		     size_t N_shared, Real sum_Ll_refined,
		     Real sum_Llm1_refined, size_t N_refined, Real beta_dot,
		     Real gamma, Real& H_raw_mom);
  /// apply vector control variate parameter (beta) to approximate HF moment
  void apply_control(const RealVector& sum_H, const RealVector& sum_L_shared,
		     const SizetArray& N_shared,
		     const RealVector& sum_L_refined,
		     const SizetArray& N_refined, const RealVector& beta,
		     RealVector& H_raw_mom);
  /// apply matrix control variate parameter (beta) to approximate HF moment
  void apply_control(const RealMatrix& sum_H, const RealMatrix& sum_L_shared,
		     const SizetArray& N_shared,
		     const RealMatrix& sum_L_refined,
		     const SizetArray& N_refined, size_t lev,
		     const RealVector& beta, RealVector& H_raw_mom);
  /// apply matrix control variate parameter (beta) to approximate HF moment
  void apply_control(const RealMatrix& sum_Hl, const RealMatrix& sum_Hlm1,
		     const RealMatrix& sum_Ll, const RealMatrix& sum_Llm1,
		     const SizetArray& N_shared,
		     const RealMatrix& sum_Ll_refined,
		     const RealMatrix& sum_Llm1_refined,
		     const SizetArray& N_refined, size_t lev,
		     const RealVector& beta_dot, const RealVector& gamma,
		     RealVector& H_raw_mom);

  //
  //- Heading: Data
  //

  /// mean squared error of mean estimator from pilot sample MC on HF model
  RealVector mcMSEIter0;

};


inline Real NonDMultifidelitySampling::
level_cost(const RealVector& cost, size_t step)
{
  // discrepancies incur two level costs
  return (step) ?
    cost[step] + cost[step-1] : // aggregated {HF,LF} mode
    cost[step];                 //     uncorrected LF mode
}


inline void NonDMultifidelitySampling::
compute_control(const RealVector& sum_L, const RealVector& sum_H,
		const RealVector& sum_LL, const RealVector& sum_LH,
		const SizetArray& N_shared, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		    N_shared[qoi], beta[qoi]);
}


inline void NonDMultifidelitySampling::
compute_control(const RealVector& sum_L, const RealVector& sum_H,
		const RealVector& sum_LL, const RealVector& sum_LH,
		const RealVector& sum_HH, const SizetArray& N_shared,
		RealVector& var_H, RealVector& rho2_LH)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		    sum_HH[qoi], N_shared[qoi], var_H[qoi], rho2_LH[qoi]);
}


inline void NonDMultifidelitySampling::
compute_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
		const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		const SizetArray& N_shared, size_t lev, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_L(qoi,lev), sum_H(qoi,lev), sum_LL(qoi,lev),
		    sum_LH(qoi,lev), N_shared[qoi], beta[qoi]);
}


inline void NonDMultifidelitySampling::
compute_control(const RealMatrix& sum_Ll,        const RealMatrix& sum_Llm1,
		const RealMatrix& sum_Hl,        const RealMatrix& sum_Hlm1,
		const RealMatrix& sum_Ll_Ll,     const RealMatrix& sum_Ll_Llm1,
		const RealMatrix& sum_Llm1_Llm1, const RealMatrix& sum_Hl_Ll,
		const RealMatrix& sum_Hl_Llm1,   const RealMatrix& sum_Hlm1_Ll,
		const RealMatrix& sum_Hlm1_Llm1, const RealMatrix& sum_Hl_Hl,
		const RealMatrix& sum_Hl_Hlm1, const RealMatrix& sum_Hlm1_Hlm1,
		const SizetArray& N_shared, size_t lev, RealVector& beta_dot,
		RealVector& gamma)
{
  Real var_YH, rho_dot2_LH; // not needed for this context
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_Ll(qoi,lev), sum_Llm1(qoi,lev), sum_Hl(qoi,lev),
		    sum_Hlm1(qoi,lev), sum_Ll_Ll(qoi,lev), sum_Ll_Llm1(qoi,lev),
		    sum_Llm1_Llm1(qoi,lev), sum_Hl_Ll(qoi,lev),
		    sum_Hl_Llm1(qoi,lev), sum_Hlm1_Ll(qoi,lev),
		    sum_Hlm1_Llm1(qoi,lev), sum_Hl_Hl(qoi,lev),
		    sum_Hl_Hlm1(qoi,lev), sum_Hlm1_Hlm1(qoi,lev),
		    N_shared[qoi], var_YH, rho_dot2_LH,
		    beta_dot[qoi], gamma[qoi]);
}


inline void NonDMultifidelitySampling::
apply_control(const RealVector& sum_H, const RealVector& sum_L_shared,
	      const SizetArray& N_shared,  const RealVector& sum_L_refined,
	      const SizetArray& N_refined, const RealVector& beta,
	      RealVector& H_raw_mom)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta = "
	 << std::setw(9) << beta[qoi] << '\n';
    apply_control(sum_H[qoi], sum_L_shared[qoi], N_shared[qoi],
		  sum_L_refined[qoi], N_refined[qoi], beta[qoi],
		  H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}


inline void NonDMultifidelitySampling::
apply_control(const RealMatrix& sum_H, const RealMatrix& sum_L_shared,
	      const SizetArray& N_shared,  const RealMatrix& sum_L_refined,
	      const SizetArray& N_refined, size_t lev, const RealVector& beta,
	      RealVector& H_raw_mom)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta = "
	 << std::setw(9) << beta[qoi] << '\n';
    apply_control(sum_H(qoi,lev), sum_L_shared(qoi,lev), N_shared[qoi],
		  sum_L_refined(qoi,lev), N_refined[qoi], beta[qoi],
		  H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}


inline void NonDMultifidelitySampling::
apply_control(const RealMatrix& sum_Hl, const RealMatrix& sum_Hlm1,
	      const RealMatrix& sum_Ll, const RealMatrix& sum_Llm1,
	      const SizetArray& N_shared,  const RealMatrix& sum_Ll_refined,
	      const RealMatrix& sum_Llm1_refined, const SizetArray& N_refined,
	      size_t lev, const RealVector& beta_dot, const RealVector& gamma,
	      RealVector& H_raw_mom)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta_dot = "
	 << std::setw(9) << beta_dot[qoi] << '\n';
    apply_control(sum_Hl(qoi,lev), sum_Hlm1(qoi,lev), sum_Ll(qoi,lev),
		  sum_Llm1(qoi,lev), N_shared[qoi], sum_Ll_refined(qoi,lev),
		  sum_Llm1_refined(qoi,lev), N_refined[qoi], beta_dot[qoi],
		  gamma[qoi], H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}

} // namespace Dakota

#endif
