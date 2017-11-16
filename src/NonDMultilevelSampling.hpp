/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Wrapper class for Fortran 90 LHS library
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTILEVEL_SAMPLING_H
#define NOND_MULTILEVEL_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Performs Multilevel Monte Carlo sampling for uncertainty quantification.

/** Multilevel Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevelSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevelSampling();

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

  /// Perform multilevel Monte Carlo across the discretization levels for a
  /// particular model form using discrepancy accumulators (sum_Y)
  void multilevel_mc_Ysum(size_t model_form);
  /// Perform multilevel Monte Carlo across the discretization levels for a
  /// particular model form using QoI accumulators (sum_Q)
  void multilevel_mc_Qsum(size_t model_form);
  /// Perform control variate Monte Carlo across two model forms
  void control_variate_mc(const SizetSizetPair& lf_form_level,
			  const SizetSizetPair& hf_form_level);
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Y (LH correlations for level discrepancies)
  void multilevel_control_variate_mc_Ycorr(size_t lf_model_form,
					   size_t hf_model_form);
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Q (LH correlations for QoI)
  void multilevel_control_variate_mc_Qcorr(size_t lf_model_form,
					   size_t hf_model_form);

  /// perform a shared increment of LF and HF samples for purposes of
  /// computing/updating the evaluation ratio and the MSE ratio
  void shared_increment(size_t iter, size_t lev);
  /// perform final LF sample increment as indicated by the evaluation ratio
  bool lf_increment(Real avg_eval_ratio, const SizetArray& N_lf,
		    const SizetArray& N_hf, size_t iter, size_t lev);

  /// initialize the ML accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_ml_Ysums(IntRealMatrixMap& sum_Y, size_t num_lev);
  /// initialize the ML accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, size_t num_lev);

  /// initialize the CV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_cv_sums(IntRealVectorMap& sum_L_shared,
			  IntRealVectorMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
			  IntRealVectorMap& sum_LH);

  /// initialize the MLCV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlcv_sums(IntRealMatrixMap& sum_L_shared,
			    IntRealMatrixMap& sum_L_refined,
			    IntRealMatrixMap& sum_H,
			    IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
			    IntRealMatrixMap& sum_HH, size_t num_ml_lev,
			    size_t num_cv_lev);
  /// initialize the MLCV accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlcv_sums(IntRealMatrixMap& sum_Ll,
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

  /// accumulate initial approximation to mean vector, for use as offsets in
  /// subsequent accumulations
  void accumulate_offsets(RealVector& mu);

  /// update running QoI sums for one model (sum_Q) using set of model
  /// evaluations within allResponses; used for level 0 from other accumulators
  void accumulate_ml_Qsums(IntRealMatrixMap& sum_Q, size_t lev,
			   const RealVector& offset, SizetArray& num_Q);
  /// update accumulators for multilevel telescoping running sums
  /// using set of model evaluations within allResponses
  void accumulate_ml_Ysums(IntRealMatrixMap& sum_Y, RealMatrix& sum_YY,
			   size_t lev, const RealVector& offset,
			   SizetArray& num_Y);
  /// update running QoI sums for two models (sum_Ql, sum_Qlm1) using set of
  /// model evaluations within allResponses
  void accumulate_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, size_t lev,
			   const RealVector& offset, SizetArray& num_Q);

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

  /// update running QoI sums for one model at two levels (sum_Ql, sum_Qlm1)
  /// using set of model evaluations within allResponses
  void accumulate_mlcv_Qsums(IntRealMatrixMap& sum_Ql,
			     IntRealMatrixMap& sum_Qlm1, size_t lev,
			     const RealVector& offset, SizetArray& num_Q);
  /// update running discrepancy sums for one model (sum_Y) using
  /// set of model evaluations within allResponses
  void accumulate_mlcv_Ysums(IntRealMatrixMap& sum_Y, size_t lev,
			     const RealVector& offset, SizetArray& num_Y);
  /// update running QoI sums for two models (sum_L, sum_H, sum_LL, sum_LH,
  /// and sum_HH) from set of low/high fidelity model evaluations within
  /// {lf,hf}_resp_map; used for level 0 from other accumulators
  void accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
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
  void accumulate_mlcv_Ysums(const IntResponseMap& lf_resp_map,
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
  void accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
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

  /// export allSamples to tagged tabular file
  void export_all_samples(String root_prepend, const Model& model,
			  size_t iter, size_t lev);

  /// convert uncentered raw moments (multilevel expectations) to
  /// standardized moments
  void convert_moments(const RealMatrix& raw_mom, RealMatrix& final_mom);

  /// populate finalStatErrors for MLMC based on Q sums
  void compute_error_estimates(IntRealMatrixMap& sum_Ql,
			       IntRealMatrixMap& sum_Qlm1,
			       IntIntPairRealMatrixMap& sum_QlQlm1,
			       Sizet2DArray& num_Q);

  /// compute variance from sum accumulators
  Real variance_Ysum(Real sum_Y, Real sum_YY, size_t Nlq);
  /// compute variance from sum accumulators
  Real variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
		     Real sum_Qlm1Qlm1, size_t Nlq);

  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			       const SizetArray& N_l);
  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			       const Real* sum_QlQl,     const Real* sum_QlQlm1,
			       const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			       size_t lev);

  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy variances
  Real aggregate_mse_Yvar(const Real* var_Y, const SizetArray& N_l);
  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy sums
  Real aggregate_mse_Ysum(const Real* sum_Y, const Real* sum_YY,
			  const SizetArray& N_l);
  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy sums
  Real aggregate_mse_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			  const Real* sum_QlQl,     const Real* sum_QlQlm1,
			  const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			  size_t lev);

  /// convert uncentered (raw) moments to centered moments; biased estimators
  void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4,
			      size_t Nlq) const;
  /// convert uncentered (raw) moments to centered moments; unbiased estimators
  void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4) const;
  /// convert centered moments to standardized moments
  void centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
			    Real& sm1, Real& sm2, Real& sm3, Real& sm4) const;
  /// detect, warn, and repair a negative central moment (for even orders)
  void check_negative(Real& cm) const;

  /// compute sum of a set of observations
  Real sum(const Real* vec, size_t vec_len) const;
  /// compute average of a set of observations
  Real average(const Real* vec, size_t vec_len) const;
  /// compute average of a set of observations
  Real average(const RealVector& vec) const;
  /// compute average of a set of observations
  Real average(const SizetArray& sa) const;

  /// compute a one-sided sample increment to move current sampling level
  /// to a new target
  size_t one_sided_delta(Real current, Real target);

  //
  //- Heading: Data
  //

  /// total number of successful sample evaluations (excluding faults)
  /// for each model form, discretization level, and QoI
  Sizet3DArray NLev;

  /// store the pilot_samples input specification, prior to run-time
  /// invocation of load_pilot_sample()
  SizetArray pilotSamples;

  /// mean squared error of mean estimator from pilot sample MC on HF model
  RealVector mcMSEIter0;
  /// equivalent number of high fidelity evaluations accumulated using samples
  /// across multiple model forms and/or discretization levels
  Real equivHFEvals;

  /// if defined, complete the final CV refinement when terminating MLCV based
  /// on maxIterations (the total number of refinements beyond the pilot sample
  /// will be one more for CV than for ML).  This approach is consistent with
  /// normal termination based on l1_norm(delta_N_hf) = 0.
  bool finalCVRefinement;

  /// if defined, export each of the sample increments in ML, CV, MLCV
  /// using tagged tabular files
  bool exportSampleSets;
  /// format for exporting sample increments using tagged tabular files
  unsigned short exportSamplesFormat;
};


inline void NonDMultilevelSampling::
compute_control(const RealVector& sum_L, const RealVector& sum_H,
		const RealVector& sum_LL, const RealVector& sum_LH,
		const SizetArray& N_shared, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		    N_shared[qoi], beta[qoi]);
}


inline void NonDMultilevelSampling::
compute_control(const RealVector& sum_L, const RealVector& sum_H,
		const RealVector& sum_LL, const RealVector& sum_LH,
		const RealVector& sum_HH, const SizetArray& N_shared,
		RealVector& var_H, RealVector& rho2_LH)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		    sum_HH[qoi], N_shared[qoi], var_H[qoi], rho2_LH[qoi]);
}


inline void NonDMultilevelSampling::
compute_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
		const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		const SizetArray& N_shared, size_t lev, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_control(sum_L(qoi,lev), sum_H(qoi,lev), sum_LL(qoi,lev),
		    sum_LH(qoi,lev), N_shared[qoi], beta[qoi]);
}


inline void NonDMultilevelSampling::
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


inline void NonDMultilevelSampling::
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


inline void NonDMultilevelSampling::
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


inline void NonDMultilevelSampling::
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


inline Real NonDMultilevelSampling::
variance_Ysum(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq)
{
  Real mu_Y = sum_Y / Nlq;
  // Note: precision loss in variance is difficult to avoid without
  // storing full sample history; must accumulate Y^2 across iterations
  // instead of (Y-mean)^2 since mean is updated on each iteration.
  Real var_Y = (sum_YY / Nlq - mu_Y * mu_Y)
             * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  return var_Y;

  /*
  Real new_mu_Y = mu_Y + offset;
  return var_Y
    //  + offset   * offset    // uncenter from old mu_hat
    //  - new_mu_Y * new_mu_Y; // recenter with new_mu_Y
    - mu_Y * mu_Y - 2. * mu_Y * offset; // cancel offset^2
  */
}


inline Real NonDMultilevelSampling::
variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq)
{
  Real mu_Ql = sum_Ql / Nlq, mu_Qlm1 = sum_Qlm1 / Nlq;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  return (       sum_QlQl / Nlq - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq - mu_Qlm1 * mu_Qlm1 ) // var_Qlm1
    * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
}


inline Real NonDMultilevelSampling::
aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			const SizetArray& N_l)
{
  Real agg_var_l = 0.;//, var_Y;
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
  for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
    agg_var_l += variance_Ysum(sum_Y[qoi], sum_YY[qoi], N_l[qoi]);
    //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
  //}
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
  return agg_var_l;
}


inline Real NonDMultilevelSampling::
aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			const Real* sum_QlQl,     const Real* sum_QlQlm1,
			const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			size_t lev)
{
  Real agg_var_l = 0., var_Y;
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
  for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
    agg_var_l += (lev) ?
      variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi], sum_QlQlm1[qoi],
		    sum_Qlm1Qlm1[qoi], N_l[qoi]) :
      variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], N_l[qoi]);
    //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
  //}
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
  return agg_var_l;
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


inline Real NonDMultilevelSampling::
aggregate_mse_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
		   const Real* sum_QlQl,     const Real* sum_QlQlm1,
		   const Real* sum_Qlm1Qlm1, const SizetArray& N_l, size_t lev)
{
  Real agg_mse = 0., mu_Ql, mu_Qlm1, var_Y; size_t Nlq;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    Nlq = N_l[qoi];
    var_Y = (lev) ?
      variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi], sum_QlQlm1[qoi],
		    sum_Qlm1Qlm1[qoi], Nlq) :
      variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], Nlq);
    agg_mse += var_Y / Nlq; // aggregate MC estimator variance for each QoI
  }
  return agg_mse;
}


inline void NonDMultilevelSampling::accumulate_offsets(RealVector& mu)
{
  using boost::math::isfinite;
  IntRespMCIter r_it = allResponses.begin();
  size_t qoi, num_samp, num_fns = r_it->second.num_functions();
  mu.sizeUninitialized(num_fns);
  Real q_l, sum;
  for (qoi=0; qoi<num_fns; ++qoi) {
    num_samp = 0; sum = 0.;
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      q_l = r_it->second.function_value(qoi);
      if (isfinite(q_l)) // neither NaN nor +/-Inf
	{ sum += q_l; ++num_samp; }
    }
    mu[qoi] = sum / num_samp;
  }
}


/** For single-level moment calculations with a scalar Nlq. */
inline void NonDMultilevelSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
		       Real& cm1, Real& cm2, Real& cm3, Real& cm4) const
{
  // convert from uncentered ("raw") to centered moments for a single level

  // For moments from sampling:
  // > Raw moments are unbiased. Central moments are unbiased for an exact mean
  //   (i.e., the samples represent the full population).
  // > For sampling a portion of the population, central moments {2,3,4} are 
  //   biased estimators since the mean is approximated.  The conversion to
  //   unbiased requires a correction based on the number of samples, as
  //   implemented in the subsequent function.

  cm1 = rm1;             // mean
  cm2 = rm2 - cm1 * cm1; // variance

  cm3 = rm3 - cm1 * (3. * cm2 + cm1 * cm1);                         // using cm
  //  = rm3 - cm1 * (3. * rm2 - 2. * cm1 * cm1);                    // using rm

  // the 4th moment is the central moment (non-excess, not cumulant)
  cm4 = rm4 - cm1 * (4. * cm3 + cm1 * (6. * cm2 + cm1 * cm1));      // using cm
  //  = rm4 - cm1 * (4. * rm3 - cm1 * (6. * rm2 - 3. * cm1 * cm1)); // using rm
}


/** For single-level moment calculations with a scalar Nlq. */
inline void NonDMultilevelSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4, Real& cm1,
		       Real& cm2, Real& cm3, Real& cm4, size_t Nlq) const
{
  // convert from uncentered ("raw") to centered moments for a single level

  // Biased central moment estimators:
  uncentered_to_centered(rm1, rm2, rm3, rm4, cm1, cm2, cm3, cm4);

  // Bias corrections for population-based estimators w/ estimated means:
  if (Nlq > 3) {
    Real cm1_sq = cm1 * cm1;
    Real nm1 = Nlq - 1., nm2 = Nlq - 2., n_sq = Nlq * Nlq;
    cm2 *= Nlq / nm1; // unbiased population variance from Bessel's correction
    cm3 *= n_sq / (nm1 * nm2);
    // From "Modeling with Data," Klemens 2009 (Appendix M).
    // Notes:
    // (1) this is 4th central moment (non-excess, unnormalized),
    //     which differs from the fourth cumulant (excess, unnormalized)
    // (2) cm2 is now unbiased within following conversion:
    cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * cm2 * cm2 )
        / (n_sq - 3. * Nlq + 3.);
  }
  else
    Cerr << "Warning: due to small sample size, resorting to biased estimator "
	 << "conversion in NonDMultilevelSampling::uncentered_to_centered().\n";
}


inline void NonDMultilevelSampling::
centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
		     Real& sm1, Real& sm2, Real& sm3, Real& sm4) const
{
  // convert from centered to standardized moments
  sm1 = cm1;                    // mean
  if (cm2 > 0.) {
    sm2 = std::sqrt(cm2);         // std deviation
    sm3 = cm3 / (cm2 * sm2);      // skewness
    sm4 = cm4 / (cm2 * cm2) - 3.; // excess kurtosis
  }
  else {
    Cerr << "\nWarning: central to standard conversion failed due to "
	 << "non-positive\n         variance.  Retaining central moments.\n";
    sm2 = 0.; sm3 = cm3; sm4 = cm4; // or assign NaN to sm{3,4}
  }
}


inline void NonDMultilevelSampling::check_negative(Real& cm) const
{
  if (cm < 0.) {
    Cerr << "\nWarning: central moment less than zero (" << cm << ").  "
	 << "Repairing to zero.\n";
    cm = 0.;
    // TO DO:  consider hard error if COV < -tol (pass in mean and cm order)
  }
}


inline Real NonDMultilevelSampling::sum(const Real* vec, size_t vec_len) const
{
  Real sum = 0.;
  for (size_t i=0; i<vec_len; ++i)
    sum += vec[i];
  return sum;
}


inline Real NonDMultilevelSampling::
average(const Real* vec, size_t vec_len) const
{ return sum(vec, vec_len) / (Real)vec_len; }


inline Real NonDMultilevelSampling::average(const RealVector& vec) const
{ return average(vec.values(), vec.length()); }


inline Real NonDMultilevelSampling::average(const SizetArray& sa) const
{
  size_t i, len = sa.size(), sum = 0;
  for (i=0; i<len; ++i)
    sum += sa[i];
  return (Real)sum / (Real)len;
}


inline size_t NonDMultilevelSampling::one_sided_delta(Real current, Real target)
{ return (target > current) ? (size_t)std::floor(target - current + .5) : 0; }

} // namespace Dakota

#endif
