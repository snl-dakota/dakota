/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDControlVariateSampling
//- Description: Implementation of multifidelity Monte Carlo
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_CONTROL_VARIATE_SAMPLING_H
#define NOND_CONTROL_VARIATE_SAMPLING_H

#include "NonDHierarchSampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Performs Multifidelity Monte Carlo sampling for UQ.

/** Multifidelity Monte Carlo (MFMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDControlVariateSampling: public virtual NonDHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDControlVariateSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDControlVariateSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  void print_variance_reduction(std::ostream& s);

  //
  //- Heading: Member functions
  //

  /// perform LF sample increment as indicated by the evaluation ratio
  bool lf_increment(const RealVector& eval_ratios, const SizetArray& N_lf,
		    Real hf_target, RealVector& lf_targets,
		    size_t iter, size_t lev);
  /// perform final LF sample increment as indicated by the evaluation ratio
  bool lf_increment(const Pecos::ActiveKey& lf_key,
		    const RealVector& eval_ratios, const SizetArray& N_lf,
		    const RealVector& hf_targets, RealVector& lf_targets,
		    size_t iter, size_t lev);
  /// perform final LF sample increment as indicated by the evaluation ratio
  bool lf_increment(const Pecos::ActiveKey& lf_key,
		    const RealVector& eval_ratios, size_t N_lf,
		    const RealVector& hf_targets, RealVector& lf_targets,
		    size_t iter, size_t lev);

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

  // if defined, complete the final CV refinement when terminating MLCV based
  // on maxIterations (the total number of refinements beyond the pilot sample
  // will be one more for CV than for ML).  This approach is consistent with
  // normal termination based on l1_norm(delta_N_hf) = 0.
  //bool finalCVRefinement;

private:

  //
  //- Heading: Helper functions
  //

  /// Perform control variate Monte Carlo across two model forms,
  /// including pilot sample as online cost
  void control_variate_mc(const Pecos::ActiveKey& active_key);
  /// Perform control variate Monte Carlo across two model forms,
  /// segregating the pilot sample as separate offline cost
  void control_variate_mc_offline_pilot(const Pecos::ActiveKey& active_key);
  /// Perform control variate Monte Carlo across two model forms,
  /// projecting estimator performance based only on the pilot sample
  void control_variate_mc_pilot_projection(const Pecos::ActiveKey& active_key);

  /// helper for shared code among MLCV for offline-pilot and pilot-projection
  void evaluate_pilot(const Pecos::ActiveKey& active_key, Real& cost_ratio,
		      RealVector& eval_ratios, RealVector& var_H,
		      SizetArray& N_shared, RealVector& hf_targets,
		      bool accumulate_cost, bool pilot_estvar);

  /// define model form and resolution level indices
  void hf_lf_indices(size_t& hf_form_index, size_t& hf_lev_index,
		     size_t& lf_form_index, size_t& lf_lev_index);

  /// perform a shared increment of LF and HF samples for purposes of
  /// computing/updating the evaluation and estimator variance ratios
  void shared_increment(const Pecos::ActiveKey& agg_key,size_t iter,size_t lev);

  /// compute numSamples for LF sample increment
  void lf_allocate_samples(const RealVector& eval_ratios,
			   const SizetArray& N_lf, const RealVector& hf_targets,
			   RealVector& lf_targets);
  /// parameter set definition and evaluation for LF sample increment
  bool lf_perform_samples(size_t iter, size_t lev);

  /// update equivHFEvals from HF, LF evaluation counts
  void compute_mf_equivalent_cost(size_t raw_N_hf, size_t raw_N_lf,
				  Real cost_ratio);
  /// update equivHFEvals from HF, LF evaluation increment
  void increment_mf_equivalent_cost(size_t new_N_hf, size_t new_N_lf,
				    Real cost_ratio);
  /// update equivHFEvals from LF evaluation increment
  void increment_mf_equivalent_cost(size_t new_N_lf, Real cost_ratio);

  /// initialize the CV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mf_sums(IntRealVectorMap& sum_L_shared,
			//IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
			  IntRealVectorMap& sum_LH);

  /// update running sums for one model (sum_L) using set of model
  /// evaluations within allResponses
  void accumulate_mf_sums(IntRealVectorMap& sum_L, SizetArray& num_L);
  /// update running sums for two models from set of low/high fidelity
  /// model evaluations within allResponses
  void accumulate_mf_sums(IntRealVectorMap& sum_L_shared,
			//IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H,  IntRealVectorMap& sum_LL,
			  IntRealVectorMap& sum_LH, RealVector& sum_HH,
			  SizetArray& N_shared);
  /// update running sums for two models from set of low/high fidelity
  /// model evaluations within allResponses
  void accumulate_mf_sums(RealVector& sum_L,  RealVector& sum_H,
			  RealVector& sum_LL, RealVector& sum_LH,
			  RealVector& sum_HH, SizetArray& N_shared);

  /// scale sample profile to meeet a specified budget
  void allocate_budget(const RealVector& eval_ratios, Real cost_ratio,
		       RealVector& hf_targets);

  /// compute the LF/HF evaluation ratios across the QoI vector
  void compute_eval_ratios(const RealVector& sum_L_shared,
			   const RealVector& sum_H,  const RealVector& sum_LL,
			   const RealVector& sum_LH, const RealVector& sum_HH,
			   Real cost_ratio, const SizetArray& N_shared,
			   RealVector& var_H, RealVector& rho2_LH,
			   RealVector& eval_ratios);

  /// compute ratios of MC and CVMC mean squared errors across the QoI vector
  void compute_estvar_ratios(const RealVector& eval_ratios,
			   //const RealVector& var_H,
			     const RealVector& rho2_LH,
			   //size_t iter, const SizetArray& N_hf,
			     RealVector& estvar_ratios);

  /// compute control variate parameters for CVMC and estimate raw moments
  void cv_raw_moments(IntRealVectorMap& sum_L_shared, IntRealVectorMap& sum_H,
		      IntRealVectorMap& sum_LL, IntRealVectorMap& sum_LH,
		      const SizetArray& N_shared,
		      IntRealVectorMap& sum_L_refined,
		      const SizetArray& N_refined, //const RealVector& rho2_LH,
		      RealMatrix& H_raw_mom);

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

  /// for pilot-projection mode, update the same counts based on projections
  /// rather than accumulations
  void update_projected_samples(const RealVector& hf_targets,
				const RealVector& eval_ratios, Real cost_ratio,
				SizetArray& N_actual_hf,SizetArray& N_actual_lf,
				size_t&     N_alloc_hf, size_t&     N_alloc_lf);

  //
  //- Heading: Data
  //

  RealVector estVarRatios;
  SizetArray numHIter0;
};


inline NonDControlVariateSampling::~NonDControlVariateSampling()
{ }


inline void NonDControlVariateSampling::
compute_mf_equivalent_cost(size_t raw_N_hf, size_t raw_N_lf, Real cost_ratio)
{ equivHFEvals = raw_N_hf + (Real)raw_N_lf / cost_ratio; }


inline void NonDControlVariateSampling::
increment_mf_equivalent_cost(size_t new_N_hf, size_t new_N_lf, Real cost_ratio)
{ equivHFEvals += new_N_hf + (Real)new_N_lf / cost_ratio; }


inline void NonDControlVariateSampling::
increment_mf_equivalent_cost(size_t new_N_lf, Real cost_ratio)
{ equivHFEvals += (Real)new_N_lf / cost_ratio; }


inline void NonDControlVariateSampling::
compute_mf_correlation(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		       Real sum_HH, size_t N_shared, Real& var_H, Real& rho2_LH)
{
  //Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_L = (sum_LL - mu_L * sum_L),// / (Real)(N_shared - 1),
      cov_LH = (sum_LH - mu_L * sum_H);// / (Real)(N_shared - 1);
  var_H      = (sum_HH - mu_H * sum_H);// / (Real)(N_shared - 1);

  rho2_LH = cov_LH / var_L * cov_LH / var_H; // bessel corrs would cancel
  var_H  /= (Real)(N_shared - 1); // now apply denom portion of bessel
}


/*
inline void NonDControlVariateSampling::
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


inline void NonDControlVariateSampling::
apply_mf_control(Real sum_H, Real sum_L_shared, size_t N_shared,
		 Real sum_L_refined, size_t N_refined, Real beta,
		 Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom = sum_H / N_shared                    // mu_H from shared samples
            - beta * (sum_L_shared  / N_shared -  // mu_L from shared samples
		      sum_L_refined / N_refined); // refined_mu_L incl increment
}


inline void NonDControlVariateSampling::
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


inline void NonDControlVariateSampling::
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
