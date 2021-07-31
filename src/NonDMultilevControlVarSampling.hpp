/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevControlVarSampling
//- Description: class for multilevel-multifidelity sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTILEV_CONTROL_VAR_SAMPLING_H
#define NOND_MULTILEV_CONTROL_VAR_SAMPLING_H

#include "NonDMultilevelSampling.hpp"
#include "NonDControlVariateSampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Performs multilevel-multifidelity Monte Carlo sampling for
/// uncertainty quantification.

/** Multilevel-multifidelity Monte Carlo (MLMFMC) combines variance
    decay across model resolutions with variance reduction from a
    control variate across model fidelities. */

class NonDMultilevControlVarSampling: public NonDMultilevelSampling,
				      public NonDControlVariateSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevControlVarSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevControlVarSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

private:

  //
  //- Heading: Helper functions
  //

  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Y (LH correlations for level discrepancies)
  void multilevel_control_variate_mc_Ycorr();
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Q (LH correlations for QoI)
  void multilevel_control_variate_mc_Qcorr();

  /// compute the equivalent number of HF evaluations (includes any sim faults)
  void compute_mlmf_equivalent_cost(const SizetArray& raw_N_hf,
				    const RealVector& hf_cost,
				    const SizetArray& raw_N_lf,
				    const RealVector& lf_cost);
  /// increment the equivalent number of HF evaluations
  void increment_mlmf_equivalent_cost(size_t new_N_hf, Real hf_lev_cost,
				      size_t new_N_lf, Real lf_lev_cost,
				      Real hf_ref_cost);

  /// compute the LF/HF evaluation ratio, averaged over the QoI
  void compute_eval_ratios(RealMatrix& sum_L_shared, RealMatrix& sum_H,
			   RealMatrix& sum_LL, RealMatrix& sum_LH,
			   RealMatrix& sum_HH, Real cost_ratio, size_t lev,
			   const SizetArray& N_shared, RealMatrix& var_H,
			   RealMatrix& rho2_LH, RealVector& eval_ratios);
  /// compute the LF/HF evaluation ratio, averaged over the QoI
  void compute_eval_ratios(RealMatrix& sum_Ll,        RealMatrix& sum_Llm1,
			   RealMatrix& sum_Hl,        RealMatrix& sum_Hlm1,
			   RealMatrix& sum_Ll_Ll,     RealMatrix& sum_Ll_Llm1,
			   RealMatrix& sum_Llm1_Llm1, RealMatrix& sum_Hl_Ll,
			   RealMatrix& sum_Hl_Llm1,   RealMatrix& sum_Hlm1_Ll,
			   RealMatrix& sum_Hlm1_Llm1, RealMatrix& sum_Hl_Hl,
			   RealMatrix& sum_Hl_Hlm1,   RealMatrix& sum_Hlm1_Hlm1,
			   Real cost_ratio, size_t lev,
			   const SizetArray& N_shared, RealMatrix& var_YHl,
			   RealMatrix& rho_dot2_LH, RealVector& eval_ratios);

  /// apply control variate parameters for MLMF MC to estimate raw
  /// moment contributions
  void cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL,       IntRealMatrixMap& sum_LH,
		      const SizetArray& N_shared,
		      IntRealMatrixMap& sum_L_refined,
		      const SizetArray& N_refined, const RealMatrix& rho2_LH,
		      size_t lev, RealMatrix& H_raw_mom);
  /// apply control variate parameters for MLMF MC to estimate raw
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
  void compute_mlmf_control(Real sum_Ll, Real sum_Llm1, Real sum_Hl,
			    Real sum_Hlm1, Real sum_Ll_Ll, Real sum_Ll_Llm1,
			    Real sum_Llm1_Llm1, Real sum_Hl_Ll,
			    Real sum_Hl_Llm1, Real sum_Hlm1_Ll,
			    Real sum_Hlm1_Llm1, Real sum_Hl_Hl,
			    Real sum_Hl_Hlm1, Real sum_Hlm1_Hlm1,
			    size_t N_shared, Real& var_YH, Real& rho_dot2_LH,
			    Real& beta_dot, Real& gamma);
  /// compute matrix control variate parameters
  void compute_mlmf_control(const RealMatrix& sum_Ll,
			    const RealMatrix& sum_Llm1,
			    const RealMatrix& sum_Hl,
			    const RealMatrix& sum_Hlm1,
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
  void apply_mlmf_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll,
			  Real sum_Llm1, size_t N_shared, Real sum_Ll_refined,
			  Real sum_Llm1_refined, size_t N_refined,
			  Real beta_dot, Real gamma, Real& H_raw_mom);
  /// apply matrix control variate parameter (beta) to approximate HF moment
  void apply_mlmf_control(const RealMatrix& sum_Hl, const RealMatrix& sum_Hlm1,
			  const RealMatrix& sum_Ll, const RealMatrix& sum_Llm1,
			  const SizetArray& N_shared,
			  const RealMatrix& sum_Ll_refined,
			  const RealMatrix& sum_Llm1_refined,
			  const SizetArray& N_refined, size_t lev,
			  const RealVector& beta_dot, const RealVector& gamma,
			  RealVector& H_raw_mom);

  /// initialize the MLMF accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlmf_sums(IntRealMatrixMap& sum_L_shared,
			    IntRealMatrixMap& sum_L_refined,
			    IntRealMatrixMap& sum_H,
			    IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
			    IntRealMatrixMap& sum_HH, size_t num_ml_lev,
			    size_t num_cv_lev);
  /// initialize the MLMF accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlmf_sums(IntRealMatrixMap& sum_Ll,
			    IntRealMatrixMap& sum_Llm1,
			    IntRealMatrixMap& sum_Ll_refined,
			    IntRealMatrixMap& sum_Llm1_refined,
			    IntRealMatrixMap& sum_Hl,
			    IntRealMatrixMap& sum_Hlm1,
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
			    size_t num_ml_lev, size_t num_cv_lev);

  /// update running QoI sums for one model at two levels (sum_Ql, sum_Qlm1)
  /// using set of model evaluations within allResponses
  void accumulate_mlmf_Qsums(IntRealMatrixMap& sum_Ql,
			     IntRealMatrixMap& sum_Qlm1, size_t lev,
			     const RealVector& offset, SizetArray& num_Q);
  /// update running discrepancy sums for one model (sum_Y) using
  /// set of model evaluations within allResponses
  void accumulate_mlmf_Ysums(IntRealMatrixMap& sum_Y, size_t lev,
			     const RealVector& offset, SizetArray& num_Y);
  /// update running QoI sums for two models (sum_L, sum_H, sum_LL, sum_LH,
  /// and sum_HH) from set of low/high fidelity model evaluations within
  /// {lf,hf}_resp_map; used for level 0 from other accumulators
  void accumulate_mlmf_Qsums(const IntResponseMap& lf_resp_map,
			     const IntResponseMap& hf_resp_map,
			     IntRealMatrixMap& sum_L_shared,
			     IntRealMatrixMap& sum_L_refined,
			     IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
			     IntRealMatrixMap& sum_LH, IntRealMatrixMap& sum_HH,
			     size_t lev, const RealVector& lf_offset,
			     const RealVector& hf_offset, SizetArray& num_L,
			     SizetArray& num_H);
  /// update running two-level discrepancy sums for two models (sum_L,
  /// sum_H, sum_LL, sum_LH, and sum_HH) from set of low/high fidelity
  /// model evaluations within {lf,hf}resp_map
  void accumulate_mlmf_Ysums(const IntResponseMap& lf_resp_map,
			     const IntResponseMap& hf_resp_map,
			     IntRealMatrixMap& sum_L_shared,
			     IntRealMatrixMap& sum_L_refined,
			     IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
			     IntRealMatrixMap& sum_LH, IntRealMatrixMap& sum_HH,
			     size_t lev, const RealVector& lf_offset,
			     const RealVector& hf_offset,
			     SizetArray& num_L, SizetArray& num_H);
  /// update running QoI sums for two models and two levels from set
  /// of low/high fidelity model evaluations within {lf,hf}_resp_map
  void accumulate_mlmf_Qsums(const IntResponseMap& lf_resp_map,
			     const IntResponseMap& hf_resp_map,
			     IntRealMatrixMap& sum_Ll,
			     IntRealMatrixMap& sum_Llm1,
			     IntRealMatrixMap& sum_Ll_refined,
			     IntRealMatrixMap& sum_Llm1_refined,
			     IntRealMatrixMap& sum_Hl,
			     IntRealMatrixMap& sum_Hlm1,
			     IntRealMatrixMap& sum_Ll_Ll,
			     IntRealMatrixMap& sum_Ll_Llm1,
			     IntRealMatrixMap& sum_Llm1_Llm1,
			     IntRealMatrixMap& sum_Hl_Ll,
			     IntRealMatrixMap& sum_Hl_Llm1,
			     IntRealMatrixMap& sum_Hlm1_Ll,
			     IntRealMatrixMap& sum_Hlm1_Llm1,
			     IntRealMatrixMap& sum_Hl_Hl,
			     IntRealMatrixMap& sum_Hl_Hlm1,
			     IntRealMatrixMap& sum_Hlm1_Hlm1, size_t lev,
			     const RealVector& lf_offset,
			     const RealVector& hf_offset,
			     SizetArray& num_L, SizetArray& num_H);

  //
  //- Heading: Data
  //

};


inline NonDMultilevControlVarSampling::~NonDMultilevControlVarSampling()
{ }


inline void NonDMultilevControlVarSampling::
compute_mlmf_equivalent_cost(const SizetArray& raw_N_hf,
			     const RealVector& hf_cost,
			     const SizetArray& raw_N_lf,
			     const RealVector& lf_cost)
{
  equivHFEvals = 0.;
  if (raw_N_hf[0]) equivHFEvals += raw_N_hf[0] * hf_cost[0]; // 1st level
  if (raw_N_lf[0]) equivHFEvals += raw_N_lf[0] * lf_cost[0]; // 1st level
  size_t lev, num_hf_lev = raw_N_hf.size(), num_cv_lev = raw_N_lf.size();
  for (lev=1; lev<num_hf_lev; ++lev) // subsequent levels incur 2 model costs
    if (raw_N_hf[lev])
      equivHFEvals += raw_N_hf[lev] * (hf_cost[lev] + hf_cost[lev-1]);
  for (lev=1; lev<num_cv_lev; ++lev) // subsequent levels incur 2 model costs
    if (raw_N_lf[lev])
      equivHFEvals += raw_N_lf[lev] * (lf_cost[lev] + lf_cost[lev-1]);
  equivHFEvals /= hf_cost[num_hf_lev-1]; // normalize into equiv HF evals
}


inline void NonDMultilevControlVarSampling::
increment_mlmf_equivalent_cost(size_t new_N_hf, Real hf_lev_cost,
			       size_t new_N_lf, Real lf_lev_cost,
			       Real hf_ref_cost)
{
  // increment the equivalent number of HF evaluations
  Real incr = 0.;
  if (new_N_hf) incr += new_N_hf * hf_lev_cost;
  if (new_N_lf) incr += new_N_lf * lf_lev_cost;
  equivHFEvals += incr / hf_ref_cost; // normalize into equiv HF evals
}


inline void NonDMultilevControlVarSampling::
compute_mlmf_control(const RealMatrix& sum_Ll, const RealMatrix& sum_Llm1,
		     const RealMatrix& sum_Hl, const RealMatrix& sum_Hlm1,
		     const RealMatrix& sum_Ll_Ll, const RealMatrix& sum_Ll_Llm1,
		     const RealMatrix& sum_Llm1_Llm1,
		     const RealMatrix& sum_Hl_Ll, const RealMatrix& sum_Hl_Llm1,
		     const RealMatrix& sum_Hlm1_Ll,
		     const RealMatrix& sum_Hlm1_Llm1,
		     const RealMatrix& sum_Hl_Hl, const RealMatrix& sum_Hl_Hlm1,
		     const RealMatrix& sum_Hlm1_Hlm1,
		     const SizetArray& N_shared, size_t lev,
		     RealVector& beta_dot, RealVector& gamma)
{
  Real var_YH, rho_dot2_LH; // not needed for this context
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mlmf_control(sum_Ll(qoi,lev), sum_Llm1(qoi,lev), sum_Hl(qoi,lev),
			 sum_Hlm1(qoi,lev), sum_Ll_Ll(qoi,lev),
			 sum_Ll_Llm1(qoi,lev), sum_Llm1_Llm1(qoi,lev),
			 sum_Hl_Ll(qoi,lev), sum_Hl_Llm1(qoi,lev),
			 sum_Hlm1_Ll(qoi,lev), sum_Hlm1_Llm1(qoi,lev),
			 sum_Hl_Hl(qoi,lev), sum_Hl_Hlm1(qoi,lev),
			 sum_Hlm1_Hlm1(qoi,lev), N_shared[qoi], var_YH,
			 rho_dot2_LH, beta_dot[qoi], gamma[qoi]);
}


inline void NonDMultilevControlVarSampling::
apply_mlmf_control(const RealMatrix& sum_Hl, const RealMatrix& sum_Hlm1,
		   const RealMatrix& sum_Ll, const RealMatrix& sum_Llm1,
		   const SizetArray& N_shared, const RealMatrix& sum_Ll_refined,
		   const RealMatrix& sum_Llm1_refined,
		   const SizetArray& N_refined, size_t lev,
		   const RealVector& beta_dot, const RealVector& gamma,
		   RealVector& H_raw_mom)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Cout << "   QoI " << qoi+1 << ": control variate beta_dot = "
	 << std::setw(9) << beta_dot[qoi] << '\n';
    apply_mlmf_control(sum_Hl(qoi,lev), sum_Hlm1(qoi,lev), sum_Ll(qoi,lev),
		       sum_Llm1(qoi,lev), N_shared[qoi],
		       sum_Ll_refined(qoi,lev), sum_Llm1_refined(qoi,lev),
		       N_refined[qoi], beta_dot[qoi], gamma[qoi],
		       H_raw_mom[qoi]);
  }
  if (numFunctions > 1) Cout << '\n';
}

} // namespace Dakota

#endif
