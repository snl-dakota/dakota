/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
  void multilevel_mc_Ysum(unsigned short model_form);
  /// Perform multilevel Monte Carlo across the discretization levels for a
  /// particular model form using QoI accumulators (sum_Q)
  void multilevel_mc_Qsum(unsigned short model_form);
  /// Perform control variate Monte Carlo across two model forms
  void control_variate_mc(const Pecos::ActiveKey& active_key);
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Y (LH correlations for level discrepancies)
  void multilevel_control_variate_mc_Ycorr(unsigned short lf_model_form,
					   unsigned short hf_model_form);
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Q (LH correlations for QoI)
  void multilevel_control_variate_mc_Qcorr(unsigned short lf_model_form,
					   unsigned short hf_model_form);

  void evaluate_sample_increment(const unsigned short& step);

  /// perform a shared increment of LF and HF samples for purposes of
  /// computing/updating the evaluation ratio and the MSE ratio
  void shared_increment(size_t iter, size_t lev);
  /// perform final LF sample increment as indicated by the evaluation ratio
  bool lf_increment(Real avg_eval_ratio, const SizetArray& N_lf,
		    const SizetArray& N_hf, size_t iter, size_t lev);

  /// synchronize iteratedModel and activeSet on AGGREGATED_MODELS mode
  void aggregated_models_mode();
  /// synchronize iteratedModel and activeSet on BYPASS_SURROGATE mode
  void bypass_surrogate_mode();
  /// synchronize iteratedModel and activeSet on UNCORRECTED_SURROGATE mode
  void uncorrected_surrogate_mode();

  /// return (aggregate) level cost
  Real level_cost(const RealVector& cost, size_t step);

  /// advance any sequence specifications
  void assign_specification_sequence(size_t index);
  /// extract current random seed from randomSeedSeqSpec
  int random_seed(size_t index) const;

  /// manage response mode and active model key from {group,form,lev} triplet.
  /// s_index is the sequence index that defines the active dimension for a
  /// model sequence.
  void configure_indices(unsigned short group, unsigned short form,
			 size_t lev,           short seq_type);

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
  void accumulate_sums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, const size_t step,
			   const RealVectorArray& offset, Sizet2DArray& N_l);

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

  /// compute variance from sum accumulators, necessary for sample allocation optimization
  static Real variance_Ysum_static(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  /// compute variance from sum accumulators
  Real variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
		     Real sum_Qlm1Qlm1, size_t Nlq);
  /// compute variance from sum accumulators, necessary for sample allocation optimization
  static Real variance_Qsum_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  Real var_lev_l(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq);
	static Real var_lev_l_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  /// sum up variances for QoI (using sum_YY with means from sum_Y) based on allocation target
  void aggregate_variance_target_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, RealMatrix& agg_var_qoi);

  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			       const SizetArray& N_l);


  /// wrapper for aggregate_variance_Qsum 
  Real aggregate_variance_mean_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);
  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			       const Real* sum_QlQl,     const Real* sum_QlQlm1,
			       const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			       const size_t& lev);
  /// sum up variances for QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                               const Real* sum_QlQl,     const Real* sum_QlQlm1,
                               const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                               const size_t& lev, const size_t& qoi);
  /// wrapper for var_of_var_ml 
  Real aggregate_variance_variance_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);

  /// wrapper for var_of_sigma_ml 
  Real aggregate_variance_sigma_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);

  /// wrapper for var_of_scalarization_ml
  Real aggregate_variance_scalarization_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);

  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy variances
  Real aggregate_mse_Yvar(const Real* var_Y, const SizetArray& N_l);
  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy sums
  Real aggregate_mse_Ysum(const Real* sum_Y, const Real* sum_YY,
			  const SizetArray& N_l);

  /// sum up Monte Carlo estimates for mean squared error (MSE) for
  /// QoI using discrepancy sums based on allocation target
  void aggregate_mse_target_Qsum(RealMatrix& agg_var_qoi, 
						  const Sizet2DArray& N_l, const size_t& step, RealVector& estimator_var0_qoi);
  
  /// sum up Monte Carlo estimates for mean squared error (MSE) for
  /// QoI using discrepancy sums based on allocation target
  void aggregate_mse_target_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, RealVector& estimator_var0_qoi);

  Real aggregate_mse_mean_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
							IntIntPairRealMatrixMap sum_QlQlm1, 
							const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);

  Real aggregate_mse_variance_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
							IntIntPairRealMatrixMap sum_QlQlm1, 
							const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);

  Real aggregate_mse_sigma_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
							IntIntPairRealMatrixMap sum_QlQlm1, 
							const Sizet2DArray& N_l, const size_t& step, const size_t& qoi);

  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy sums
  Real aggregate_mse_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			  const Real* sum_QlQl,     const Real* sum_QlQlm1,
			  const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			  const size_t& lev);
  /// sum up Monte Carlo estimates for mean squared error (MSE) for
  /// QoI using discrepancy sums
  Real aggregate_mse_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                          const Real* sum_QlQl,     const Real* sum_QlQlm1,
                          const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                          const size_t& lev, const size_t& qoi);

  /// compute epsilon^2/2 term for each qoi based on reference estimator_var0 and relative convergence tolereance
  void set_convergence_tol(const RealVector& estimator_var0_qoi, const RealVector& cost, const RealVector& convergenceTolVec, RealVector& eps_sq_div_2_qoi);

  /// compute sample allocation delta based on current samples and based on allocation target. Single allocation target for each qoi, aggregated using max operation.
  void compute_sample_allocation_target(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, const RealVector& eps_sq_div_2, const RealMatrix& agg_var_qoi, 
  										const RealVector& cost, const Sizet2DArray& N_l, SizetArray& delta_N_l);
  
  // Roll up expected value estimators for central moments.  Final expected
  // value is sum of expected values from telescopic sum.  Note: raw moments
  // have no bias correction (no additional variance from an estimated center).
  void compute_moments(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1, const Sizet2DArray& N_l);

  // compute the equivalent number of HF evaluations (includes any sim faults)
  void compute_equiv_HF_evals(const SizetArray& raw_N_l, const RealVector& cost);
  
  /// convert uncentered (raw) moments to centered moments; biased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4,
			      size_t Nlq);
  /// convert uncentered (raw) moments to centered moments; unbiased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4);
  /// convert centered moments to standardized moments
  static void centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
			    Real& sm1, Real& sm2, Real& sm3, Real& sm4);
  /// detect, warn, and repair a negative central moment (for even orders)
  static void check_negative(Real& cm);

  /// compute sum of a set of observations
  Real sum(const Real* vec, size_t vec_len) const;
  /// compute average of a set of observations
  Real average(const Real* vec, size_t vec_len) const;
  /// compute average of a set of observations
  Real average(const RealVector& vec) const;
  /// compute average of a set of observations
  Real average(const SizetArray& sa) const;

  /// compute the unbiased product of two sampling means
  static Real unbiased_mean_product_pair(const Real& sumQ1, const Real& sumQ2, const Real& sumQ1Q2, const size_t& Nlq);
  /// compute the unbiased product of three sampling means
  static Real unbiased_mean_product_triplet(const Real& sumQ1, const Real& sumQ2, const Real& sumQ3,
                                                                    const Real& sumQ1Q2, const Real& sumQ1Q3, const Real& sumQ2Q3, const Real& sumQ1Q2Q3, const size_t& Nlq);
  /// compute the unbiased product of two pairs of products of sampling means
  static Real unbiased_mean_product_pairpair(const Real& sumQ1, const Real& sumQ2, const Real& sumQ1Q2,
                                                                     const Real& sumQ1sq, const Real& sumQ2sq,
                                                                     const Real& sumQ1sqQ2, const Real& sumQ1Q2sq, const Real& sumQ1sqQ2sq, const size_t& Nlq);

  static Real var_of_var_ml_l0(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1, const size_t& Nlq_pilot, const Real& Nlq, const size_t& qoi, const bool& compute_gradient, Real& grad_g);

  static Real var_of_var_ml_lmax(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1, const size_t& Nlq_pilot, const Real& Nlq, const size_t& qoi, const bool& compute_gradient, Real& grad_g);

  static Real var_of_var_ml_l(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1, const size_t& Nlq_pilot, const Real& Nlq, const size_t& qoi, const size_t& lev, const bool& compute_gradient, Real& grad_g);

  ///OPTPP definition
  static void target_cost_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_cost_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);

  static void target_var_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);
  static void target_var_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                               RealMatrix& grad_g, int& result_mode);


  static void target_sigma_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);
  static void target_sigma_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                               RealMatrix& grad_g, int& result_mode);

  static void target_scalarization_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);
  static void target_scalarization_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                               RealMatrix& grad_g, int& result_mode);

  static void target_var_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_var_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);

  static void target_sigma_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_sigma_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);

  static void target_scalarization_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_scalarization_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);

  /// NPSOL definition (Wrapper using OPTPP implementation above under the hood)
  static void target_cost_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_cost_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_var_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);
  static void target_var_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_sigma_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);
  static void target_sigma_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_scalarization_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);
  static void target_scalarization_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_var_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_var_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);

  static void target_sigma_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_sigma_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);

  static void target_scalarization_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_scalarization_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);

  void assign_static_member(Real &conv_tol, size_t &qoi, size_t &qoi_aggregation, size_t &num_functions, RealVector &level_cost_vec, IntRealMatrixMap &sum_Ql,
                            IntRealMatrixMap &sum_Qlm1, IntIntPairRealMatrixMap &sum_QlQlm1,
                            RealVector &pilot_samples, RealMatrix &scalarization_response_mapping) const;

  void assign_static_member_problem18(Real &var_L_exact, Real &var_H_exact, Real &mu_four_L_exact, Real &mu_four_H_exact, Real &Ax, RealVector &level_cost_vec) const;

  static void target_var_constraint_eval_optpp_problem18(int mode, int n, const RealVector &x, RealVector &g, RealMatrix &grad_g, int &result_mode);
  static void target_sigma_constraint_eval_optpp_problem18(int mode, int n, const RealVector &x, RealVector &g, RealMatrix &grad_g, int &result_mode);
  static double exact_var_of_var_problem18(const RealVector &Nl);
  static double exact_var_of_sigma_problem18(const RealVector &Nl);
  //
  //- Heading: Data
  //

  unsigned short seq_index;
  size_t max_iter;

  /// total number of successful sample evaluations (excluding faults)
  /// for each model form, discretization level, and QoI
  Sizet3DArray NLev;

  /// store the pilot_samples input specification, prior to run-time
  /// invocation of load_pilot_sample()
  SizetArray pilotSamples;

  /// user specification for seed_sequence
  SizetArray randomSeedSeqSpec;

  /// major iteration counter
  size_t mlmfIter;

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


  /// "scalarization" response_mapping matrix applied to the mlmc sample allocation
  /// when a scalarization, i.e. alpha_1 * mean + alpha_2 * sigma, is the target. 
  RealMatrix scalarization_response_mapping;

  /// Helper data structure to store intermedia sample allocations
  RealMatrix N_target_qoi;
  RealMatrix N_target_qoi_FN;
};


inline void NonDMultilevelSampling::aggregated_models_mode()
{
  if (iteratedModel.surrogate_response_mode() != AGGREGATED_MODELS) {
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // set HF,LF
    // synch activeSet with iteratedModel.response_size()
    activeSet.reshape(2*numFunctions);
    activeSet.request_values(1);
  }
}


inline void NonDMultilevelSampling::bypass_surrogate_mode()
{
  if (iteratedModel.surrogate_response_mode() != BYPASS_SURROGATE) {
    iteratedModel.surrogate_response_mode(BYPASS_SURROGATE); // HF
    activeSet.reshape(numFunctions);// synch with model.response_size()
  }
}


inline void NonDMultilevelSampling::uncorrected_surrogate_mode()
{
  if (iteratedModel.surrogate_response_mode() != UNCORRECTED_SURROGATE) {
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // LF
    activeSet.reshape(numFunctions);// synch with model.response_size()
  }
}


inline Real NonDMultilevelSampling::
level_cost(const RealVector& cost, size_t step)
{
  // discrepancies incur two level costs
  return (step) ?
    cost[step] + cost[step-1] : // aggregated {HF,LF} mode
    cost[step];                 //     uncorrected LF mode
}


/** extract an active seed from a seed sequence */
inline int NonDMultilevelSampling::random_seed(size_t index) const
{
  // return 0 for cases where seed is undefined or will not be updated

  if (randomSeedSeqSpec.empty()) return 0; // no spec -> non-repeatable samples
  else if (!varyPattern) // continually reset seed to specified value
    return (index < randomSeedSeqSpec.size()) ?
      randomSeedSeqSpec[index] : randomSeedSeqSpec.back();
  // only set sequence of seeds for first pass, then let RNG state continue
  else if (mlmfIter == 0 && index < randomSeedSeqSpec.size()) // pilot iter only
    return randomSeedSeqSpec[index];
  else return 0; // seed sequence exhausted, do not update
}


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
  check_negative(var_Y);
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
variance_Ysum_static(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Y = sum_Y / Nlq_pilot;
  // Note: precision loss in variance is difficult to avoid without
  // storing full sample history; must accumulate Y^2 across iterations
  // instead of (Y-mean)^2 since mean is updated on each iteration.
  Real variance_tmp = (sum_YY / Nlq_pilot - mu_Y * mu_Y); 

  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
  }

  Real var_Y = variance_tmp * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  check_negative(var_Y);
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
  Real var_Q =(       sum_QlQl / Nlq - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq - mu_Qlm1 * mu_Qlm1 ) // var_Qlm1
    * (Real)Nlq / (Real)(Nlq - 1);
  check_negative(var_Q);
  return var_Q; // Bessel's correction
}


inline Real NonDMultilevelSampling::
variance_Qsum_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Ql = sum_Ql / Nlq_pilot, mu_Qlm1 = sum_Qlm1 / Nlq_pilot;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real variance_tmp = (       sum_QlQl / Nlq_pilot - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq_pilot - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq_pilot - mu_Qlm1 * mu_Qlm1 ); // var_Qlm1
  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
  }
  check_negative(variance_tmp);
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
  check_negative(var_lev_Q);
  return var_lev_Q; 
}

inline Real NonDMultilevelSampling::
var_lev_l_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Ql = sum_Ql / Nlq_pilot, mu_Qlm1 = sum_Qlm1 / Nlq_pilot;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real variance_tmp = (sum_QlQl / Nlq_pilot - mu_Ql   * mu_Ql)     // var_Ql
    										- (sum_Qlm1Qlm1 / Nlq_pilot - mu_Qlm1 * mu_Qlm1 ); // var_Qlm1
  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
  }
  check_negative(variance_tmp);
  return variance_tmp * (Real)Nlq / (Real)(Nlq - 1.); // Bessel's correction
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

inline void NonDMultilevelSampling::
aggregate_variance_target_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
									IntIntPairRealMatrixMap sum_QlQlm1, 
								const Sizet2DArray& N_l, const size_t& step, RealMatrix& agg_var_qoi)
{
	// compute estimator variance from current sample accumulation:
	if (outputLevel >= DEBUG_OUTPUT) Cout << "variance of Y[" << step << "]: ";
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
		if (allocationTarget == TARGET_MEAN) {
			agg_var_qoi(qoi, step) = aggregate_variance_mean_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
		} else if (allocationTarget == TARGET_VARIANCE) {
			agg_var_qoi(qoi, step) = aggregate_variance_variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi); 
		} else if (allocationTarget == TARGET_SIGMA) {
			agg_var_qoi(qoi, step) = aggregate_variance_sigma_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi); 
		} else if (allocationTarget == TARGET_SCALARIZATION){
			agg_var_qoi(qoi, step) = aggregate_variance_scalarization_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi); 
			Cout << "\n\taggregate_variance_target_Qsum: " << qoi << ", " << step << ", with agg_var_qoi: " << agg_var_qoi(qoi, step) << std::endl;
	  }else{
		    Cout << "NonDMultilevelSampling::aggregate_variance_target_Qsum: allocationTarget is not known.\n";
		    abort_handler(INTERFACE_ERROR);
		}
		check_negative(agg_var_qoi(qoi, step));
	}
}

inline Real NonDMultilevelSampling::aggregate_variance_mean_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
  IntIntPair pr11(1, 1);
	Real agg_var_l = 0.;
	agg_var_l = aggregate_variance_Qsum(sum_Ql[1][step], sum_Qlm1[1][step],
		                                                   sum_Ql[2][step], sum_QlQlm1[pr11][step],
		                                                   sum_Qlm1[2][step],
                                                           N_l[step], step, qoi);

	return agg_var_l;
}

inline Real NonDMultilevelSampling::
aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                       const Real* sum_QlQl,     const Real* sum_QlQlm1,
                       const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                       const size_t& lev)
{
  Real agg_var_l = 0., var_Y;
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
  for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
  	agg_var_l += aggregate_variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQl, sum_QlQlm1, sum_Qlm1Qlm1, N_l, lev, qoi);
    //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
  //}
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
  return agg_var_l;
}

inline Real NonDMultilevelSampling::
aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                      const Real* sum_QlQl,     const Real* sum_QlQlm1,
                      const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                      const size_t& lev, const size_t& qoi)
{
       Real agg_var_l = 0., var_Y;
       //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
       //for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
         agg_var_l = (lev) ?
                      variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi], sum_QlQlm1[qoi],
                                    sum_Qlm1Qlm1[qoi], N_l[qoi]) :
                      variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], N_l[qoi]);
       //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
       //}
       //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
       return agg_var_l;
}

inline Real NonDMultilevelSampling::aggregate_variance_variance_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
	Real place_holder;
	Real agg_var_l = 0.;
	agg_var_l = ((step == 0) ? var_of_var_ml_l0(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                                         N_l[step][qoi], qoi, false, place_holder)
	                                      : var_of_var_ml_l(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                                        N_l[step][qoi], qoi, step, false, place_holder)) *
	                          N_l[step][qoi]; //As described in the paper by Krumscheid, Pisaroni, Nobile

	return agg_var_l;
}

inline Real NonDMultilevelSampling::aggregate_variance_sigma_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
	Real place_holder;
	Real agg_var_l = 0.;
	Real var_l = 0;
  IntIntPair pr11(1, 1);

	agg_var_l = ((step == 0) ? var_of_var_ml_l0(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                                         N_l[step][qoi], qoi, false, place_holder)
	                                      : var_of_var_ml_l(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                                        N_l[step][qoi], qoi, step, false, place_holder)); 
 	var_l = var_lev_l(sum_Ql[1][step][qoi], sum_Qlm1[1][step][qoi], sum_Ql[2][step][qoi], sum_Qlm1[2][step][qoi], N_l[step][qoi]);
 	if(var_l <= 0)
 		return 0;
	return 1./(4. * var_l) * agg_var_l * N_l[step][qoi]; //Multiplication by N_l as described in the paper by Krumscheid, Pisaroni, Nobile
}

inline Real NonDMultilevelSampling::aggregate_variance_scalarization_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
	Real upper_bound_cov_of_mean_sigma = 0;
	Real var_of_mean_l = 0.;
	Real var_of_sigma_l = 0;
	Real var_of_scalarization_l = 0;
	size_t numFormulations = 2;
	RealMatrix alpha_mean_scalarization(numFormulations, numFunctions);
	RealMatrix alpha_sigma_scalarization(numFormulations, numFunctions);

	var_of_mean_l = aggregate_variance_mean_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
	var_of_sigma_l = aggregate_variance_sigma_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
  upper_bound_cov_of_mean_sigma = std::sqrt(var_of_mean_l*var_of_sigma_l);
  var_of_scalarization_l = scalarization_response_mapping(qoi, 0) * scalarization_response_mapping(qoi, 0) * var_of_mean_l 
  												+ scalarization_response_mapping(qoi, 1) * scalarization_response_mapping(qoi, 1) * var_of_sigma_l;
  												+ 2.0 * scalarization_response_mapping(qoi, 0) * scalarization_response_mapping(qoi, 1) * upper_bound_cov_of_mean_sigma;

	return var_of_scalarization_l; //Multiplication by N_l as described in the paper by Krumscheid, Pisaroni, Nobile is already done in submethods
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
aggregate_mse_target_Qsum(RealMatrix& agg_var_qoi, 
						  						const Sizet2DArray& N_l, const size_t& step, RealVector& estimator_var0_qoi)
{
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
		estimator_var0_qoi[qoi] += agg_var_qoi(qoi, step)/N_l[step][qoi];
	}
}

inline void NonDMultilevelSampling::
aggregate_mse_target_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
													IntIntPairRealMatrixMap sum_QlQlm1, 
													const Sizet2DArray& N_l, const size_t& step, RealVector& estimator_var0_qoi)
{
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
		if (allocationTarget == TARGET_MEAN) {
			estimator_var0_qoi[qoi] += aggregate_mse_mean_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
		} else if (allocationTarget == TARGET_VARIANCE) {
			estimator_var0_qoi[qoi] += aggregate_mse_variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
		} else if (allocationTarget == TARGET_SIGMA) {
			estimator_var0_qoi[qoi] += aggregate_mse_sigma_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
		}else{
	        Cout << "NonDMultilevelSampling::aggregate_mse_target_Qsum: allocationTarget is not known.\n";
	        abort_handler(INTERFACE_ERROR);
    }
	}
}

inline Real NonDMultilevelSampling::
aggregate_mse_mean_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
							IntIntPairRealMatrixMap sum_QlQlm1, 
							const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
	IntIntPair pr11(1, 1);
	Real agg_mse_l = aggregate_mse_Qsum(sum_Ql[1][step], sum_Qlm1[1][step],
                                                            sum_Ql[2][step], sum_QlQlm1[pr11][step], sum_Qlm1[2][step],
                                                            N_l[step], step, qoi);
	return agg_mse_l;
}

inline Real NonDMultilevelSampling::
aggregate_mse_variance_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
							IntIntPairRealMatrixMap sum_QlQlm1, 
							const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
	Real place_holder;
	Real agg_mse_l = ((step == 0) ? var_of_var_ml_l0(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                            N_l[step][qoi], qoi, false, place_holder)
	                         : var_of_var_ml_l(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                           N_l[step][qoi], qoi, step, false, place_holder));
	return agg_mse_l;
}

inline Real NonDMultilevelSampling::
aggregate_mse_sigma_Qsum(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
							IntIntPairRealMatrixMap sum_QlQlm1, 
							const Sizet2DArray& N_l, const size_t& step, const size_t& qoi)
{
	Real place_holder;
	Real agg_var_l = 0.;
	Real var_l = 0;
  IntIntPair pr11(1, 1);

	agg_var_l = ((step == 0) ? var_of_var_ml_l0(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                                         N_l[step][qoi], qoi, false, place_holder)
	                                      : var_of_var_ml_l(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
	                                                        N_l[step][qoi], qoi, step, false, place_holder)); 
 	var_l = var_lev_l(sum_Ql[1][step][qoi], sum_Qlm1[1][step][qoi], sum_Ql[2][step][qoi], sum_Qlm1[2][step][qoi], N_l[step][qoi]);

	return 1./(4. * var_l) * agg_var_l; //Multiplication by N_l as described in the paper by Krumscheid, Pisaroni, Nobile
}

inline Real NonDMultilevelSampling::
aggregate_mse_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                  const Real* sum_QlQl,     const Real* sum_QlQlm1,
                  const Real* sum_Qlm1Qlm1, const SizetArray& N_l, const size_t& lev)
{
  Real agg_mse = 0., mu_Ql, mu_Qlm1, var_Y; size_t Nlq;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
  	agg_mse += aggregate_mse_Qsum(sum_Ql, sum_Qlm1, sum_QlQl, sum_QlQlm1, sum_Qlm1Qlm1, N_l, lev, qoi);
  }
  return agg_mse;
}

inline Real NonDMultilevelSampling::
aggregate_mse_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                   const Real* sum_QlQl,     const Real* sum_QlQlm1,
                   const Real* sum_Qlm1Qlm1, const SizetArray& N_l, const size_t& lev, const size_t& qoi)
{
  Real agg_mse = 0., mu_Ql, mu_Qlm1, var_Y; size_t Nlq;
  Nlq = N_l[qoi];
  var_Y = (lev) ?
          variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi], sum_QlQlm1[qoi],
                        sum_Qlm1Qlm1[qoi], Nlq) :
          variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], Nlq);
  agg_mse += var_Y / Nlq; // aggregate MC estimator variance for each QoI

  return agg_mse;
}

inline void NonDMultilevelSampling::set_convergence_tol(const RealVector& estimator_var0_qoi, const RealVector& cost, const RealVector& convergenceTolVec, RealVector& eps_sq_div_2_qoi)
{
	// compute epsilon target based on relative tolerance: total MSE = eps^2
	// which is equally apportioned (eps^2 / 2) among discretization MSE and
	// estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
	// discretization error, we compute an initial estimator variance and
	// then seek to reduce it by a relative_factor <= 1.
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {

	  if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
		  if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE){
				eps_sq_div_2_qoi[qoi] = estimator_var0_qoi[qoi] * convergenceTolVec[qoi];
			}else if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_ABSOLUTE){
				eps_sq_div_2_qoi[qoi] = convergenceTolVec[qoi];
			}else{
	  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolType is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}
		}else if (convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
			if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE){
				eps_sq_div_2_qoi[qoi] = cost[cost.length()-1] * convergenceTolVec[qoi]; //Relative cost with respect to convergenceTol evaluations on finest grid
			}else if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_ABSOLUTE){
				eps_sq_div_2_qoi[qoi] = convergenceTolVec[qoi]; //Direct cost
			}else{
	  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolType is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}
		}else{
  	  Cout << "NonDMultilevelSampling::set_convergence_tol: convergenceTolTarget is not known.\n";
  	  abort_handler(INTERFACE_ERROR);
		}

	}
	if (outputLevel == DEBUG_OUTPUT)
	       Cout << "Epsilon squared target per QoI = " << eps_sq_div_2_qoi << std::endl;
}

inline void NonDMultilevelSampling::compute_sample_allocation_target(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, 
 									IntIntPairRealMatrixMap sum_QlQlm1, const RealVector& eps_sq_div_2_in, 
  										const RealMatrix& agg_var_qoi_in, const RealVector& cost, 
  										const Sizet2DArray& N_l, SizetArray& delta_N_l)
{
	size_t num_steps = agg_var_qoi_in.numCols();
	RealVector level_cost_vec(num_steps);
	RealVector sum_sqrt_var_cost;
	RealMatrix delta_N_l_qoi;
	RealVector eps_sq_div_2;
	RealMatrix agg_var_qoi;

	size_t nb_aggregation_qois = 0;
	double underrelaxation_factor = static_cast<double>(mlmfIter + 1)/static_cast<double>(max_iter + 1);
  if (qoiAggregation==QOI_AGGREGATION_SUM) {
		nb_aggregation_qois = 1;
		eps_sq_div_2.size(nb_aggregation_qois);
		sum_sqrt_var_cost.size(nb_aggregation_qois);
		agg_var_qoi.shape(nb_aggregation_qois, num_steps);
		eps_sq_div_2[0] = 0;
		sum_sqrt_var_cost[0] = 0;

		for (size_t step = 0; step < num_steps ; ++step) {
			agg_var_qoi(0, step) = 0;
			for (size_t qoi = 0; qoi < numFunctions ; ++qoi) {
				agg_var_qoi(0, step) += agg_var_qoi_in(qoi, step);
			}
			sum_sqrt_var_cost[0] += std::sqrt(agg_var_qoi(0, step) * level_cost(cost, step));
		}
		for (size_t qoi = 0; qoi < numFunctions ; ++qoi) {
			eps_sq_div_2[0] += eps_sq_div_2_in[qoi];
		}

	}else if (qoiAggregation==QOI_AGGREGATION_MAX) {
		nb_aggregation_qois = numFunctions;
		eps_sq_div_2.size(nb_aggregation_qois);
		sum_sqrt_var_cost.size(nb_aggregation_qois);
		agg_var_qoi.shape(nb_aggregation_qois, num_steps);

		for (size_t qoi = 0; qoi < nb_aggregation_qois ; ++qoi) {
			eps_sq_div_2[qoi] = eps_sq_div_2_in[qoi];
			sum_sqrt_var_cost[qoi] = 0;
			for (size_t step = 0; step < num_steps ; ++step) {
				sum_sqrt_var_cost[qoi] += std::sqrt(agg_var_qoi_in(qoi, step) * level_cost(cost, step));

				agg_var_qoi(qoi, step) = agg_var_qoi_in(qoi, step);
				if (outputLevel == DEBUG_OUTPUT){
					Cout << "\n\tN_target for Qoi: " << qoi << ", with agg_var_qoi_in: " << agg_var_qoi_in(qoi, step) << std::endl;
					Cout << "\n\tN_target for Qoi: " << qoi << ", with level_cost: " << level_cost(cost, step) << std::endl;
					Cout << "\n\tN_target for Qoi: " << qoi << ", with agg_var_qoi: " << sum_sqrt_var_cost << std::endl;
				}
			}
		}
	}else{
  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: qoiAggregation is not known.\n";
  	  abort_handler(INTERFACE_ERROR);
	}

	N_target_qoi.shape(nb_aggregation_qois, num_steps);
	N_target_qoi_FN.shape(nb_aggregation_qois, num_steps);
	delta_N_l_qoi.shape(nb_aggregation_qois, num_steps);

  for (size_t qoi = 0; qoi < nb_aggregation_qois; ++qoi) {
  	Real fact_qoi = sum_sqrt_var_cost[qoi]/eps_sq_div_2[qoi];
		if (outputLevel == DEBUG_OUTPUT){
			Cout << "\n\tN_target for Qoi: " << qoi << ", with sum_sqrt_var_cost: " << sum_sqrt_var_cost[qoi] << std::endl;
			Cout << "\n\tN_target for Qoi: " << qoi << ", with eps_sq_div_2: " << eps_sq_div_2[qoi] << std::endl;
			Cout << "\n\tN_target for Qoi: " << qoi << ", with lagrange: " << fact_qoi << std::endl;
		}
		for (size_t step = 0; step < num_steps; ++step) {
    	level_cost_vec[step] = level_cost(cost, step);
	    if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
				N_target_qoi(qoi, step) = std::sqrt(agg_var_qoi(qoi, step) / level_cost_vec[step]) * fact_qoi;
			}else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
				N_target_qoi(qoi, step) = std::sqrt(agg_var_qoi(qoi, step) / level_cost_vec[step]) * (1./fact_qoi);
			}else{
	  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}

			N_target_qoi(qoi, step) = N_target_qoi(qoi, step) < 6 ? 6 : N_target_qoi(qoi, step);
			N_target_qoi_FN(qoi, step) = N_target_qoi(qoi, step);
			if (outputLevel == DEBUG_OUTPUT) {
				Cout << "\t\tVar of target: " << agg_var_qoi(qoi, step) << std::endl;
				Cout << "\t\tCost: " << level_cost_vec[step] << "\n";
				Cout << "\t\tN_target_qoi: " << N_target_qoi(qoi, step) << "\n";
			}
		}
    bool have_npsol = false, have_optpp = false;
		#ifdef HAVE_NPSOL
		    have_npsol = true;
		#endif
		#ifdef HAVE_OPTPP
		    have_optpp = true;
		#endif
		if( (allocationTarget == TARGET_VARIANCE || allocationTarget == TARGET_SIGMA || allocationTarget == TARGET_SCALARIZATION) && (have_npsol || have_optpp) && useTargetVarianceOptimizationFlag){
			size_t qoi_copy = qoi;
			size_t qoiAggregation_copy = qoiAggregation;
			size_t numFunctions_copy = numFunctions;
	    if (outputLevel == DEBUG_OUTPUT) {
    		Cout << "Numerical Optimization for sample allocation targeting " << (allocationTarget == TARGET_VARIANCE ? "variance" : (allocationTarget == TARGET_SIGMA ? "sigma" : "scalarization")) << " using " << (have_npsol ? "NPSOL" : "OPTPP") << std::endl;
        Cout << "\t\t\t Convergence Target " << eps_sq_div_2 << std::endl;
      }
			RealVector initial_point, pilot_samples;
			initial_point.size(num_steps);
			pilot_samples.size(num_steps);
			for (size_t step = 0; step < num_steps; ++step) {
				pilot_samples[step] = N_l[step][qoi];
				initial_point[step] = N_target_qoi(qoi, step); //N_l[step][qoi];//8. > N_target_qoi(qoi, step) ? 8 : N_target_qoi(qoi, step); 
			}

			RealVector var_lower_bnds, var_upper_bnds, lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets,
			  nonlin_ineq_lower_bnds, nonlin_ineq_upper_bnds, nonlin_eq_targets;
			RealMatrix lin_ineq_coeffs, lin_eq_coeffs;

			//Bound constraints only allowing positive values for Nlq
			var_lower_bnds.size(num_steps); //init to 0
			for (size_t step = 0; step < N_l.size(); ++step) {
				var_lower_bnds[step] = 6.;
			}

			var_upper_bnds.size(num_steps); //init to 0
			var_upper_bnds.putScalar(1e10); //Set to high upper bound

			//Number of linear inequality constraints = 0
			lin_ineq_coeffs.shape(0, 0);
			lin_ineq_lower_bnds.size(0);
			lin_ineq_upper_bnds.size(0);

			//Number of linear equality constraints = 0
			lin_eq_coeffs.shape(0, 0);
			lin_eq_targets.size(0);
			//Number of nonlinear inequality bound constraints = 0
			nonlin_ineq_lower_bnds.size(0);
			nonlin_ineq_upper_bnds.size(0);
			//Number of nonlinear equality constraints = 1, s.t. c_eq: c_1(Nlq) = convergenceTol;
			nonlin_eq_targets.size(1); //init to 0
			nonlin_eq_targets[0] = eps_sq_div_2[qoi]; //convergenceTol;

			assign_static_member(nonlin_eq_targets[0], qoi_copy, qoiAggregation_copy, numFunctions_copy, level_cost_vec, sum_Ql, 
													 sum_Qlm1, sum_QlQlm1, pilot_samples, scalarization_response_mapping);

			std::unique_ptr<Iterator> optimizer;

			void (*objective_function_npsol_ptr) (int&, int&, double*, double&, double*, int&) = nullptr;
			void (*constraint_function_npsol_ptr) (int&, int&, int&, int&, int*, double*, double*, double*, int&) = nullptr;
			void (*objective_function_optpp_ptr) (int, int, const RealVector&, double&, RealVector&, int&) = nullptr;
			void (*constraint_function_optpp_ptr) (int, int, const RealVector&, RealVector&, RealMatrix&, int&) = nullptr;

			#ifdef HAVE_NPSOL
	    	if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
					objective_function_npsol_ptr = &target_cost_objective_eval_npsol;
					switch(allocationTarget){
						case TARGET_VARIANCE:
							constraint_function_npsol_ptr = &target_var_constraint_eval_npsol;
							break;
						case TARGET_SIGMA:
							constraint_function_npsol_ptr = &target_sigma_constraint_eval_npsol;
							break;
						case TARGET_SCALARIZATION:
							constraint_function_npsol_ptr = &target_scalarization_constraint_eval_npsol;
							break;
						default:
							 break;
					}
				}else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
					constraint_function_npsol_ptr = &target_cost_constraint_eval_npsol;
					switch(allocationTarget){
						case TARGET_VARIANCE:
							objective_function_npsol_ptr = &target_var_objective_eval_npsol;
							break;
						case TARGET_SIGMA:
							objective_function_npsol_ptr = &target_sigma_objective_eval_npsol;
							break;
						case TARGET_SCALARIZATION:
							objective_function_npsol_ptr = &target_scalarization_objective_eval_npsol;
							break;
						default:
							 break;
					}
				}else{
		  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
		  	  abort_handler(INTERFACE_ERROR);
				}
				optimizer.reset(new NPSOLOptimizer(initial_point,
		                             var_lower_bnds, var_upper_bnds,
		                             lin_ineq_coeffs, lin_ineq_lower_bnds,
		                             lin_ineq_upper_bnds, lin_eq_coeffs,
		                             lin_eq_targets, nonlin_ineq_lower_bnds,
		                             nonlin_ineq_upper_bnds, nonlin_eq_targets,
		                             objective_function_npsol_ptr,
		                             constraint_function_npsol_ptr,
		                             3, 1e-15) //derivative_level = 3 means user_supplied gradients
		                             );
			#elif HAVE_OPTPP
	    	if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
					objective_function_optpp_ptr = &target_cost_objective_eval_optpp;
					switch(allocationTarget){
						case TARGET_VARIANCE:
							constraint_function_optpp_ptr = &target_var_constraint_eval_optpp;
							break;
						case TARGET_SIGMA:
							constraint_function_optpp_ptr = &target_sigma_constraint_eval_optpp;
							break;
						case TARGET_SCALARIZATION:
							constraint_function_optpp_ptr = &target_scalarization_constraint_eval_optpp;
							break;
						default:
							 break;
					}
				}else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
					constraint_function_optpp_ptr = &target_cost_constraint_eval_optpp;
					switch(allocationTarget){
						case TARGET_VARIANCE:
							objective_function_optpp_ptr = &target_var_objective_eval_optpp;
							break;
						case TARGET_SIGMA:
							objective_function_optpp_ptr = &target_sigma_objective_eval_optpp;
							break;
						case TARGET_SCALARIZATION:
							objective_function_optpp_ptr = &target_scalarization_objective_eval_optpp;
							break;
						default:
							 break;
					}
				}else{
		  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
		  	  abort_handler(INTERFACE_ERROR);
				}
				optimizer.reset(new SNLLOptimizer(initial_point,
		                            var_lower_bnds, var_upper_bnds,
		                            lin_ineq_coeffs, lin_ineq_lower_bnds,
		                            lin_ineq_upper_bnds, lin_eq_coeffs,
		                            lin_eq_targets,nonlin_ineq_lower_bnds,
		                            nonlin_ineq_upper_bnds, nonlin_eq_targets,
		                            objective_function_optpp_ptr,
		                            constraint_function_optpp_ptr,
		                            100000, 100000, 1.e-14,
		                            1.e-14, 100000)
		                            );
			#endif
			optimizer->output_level(DEBUG_OUTPUT);
			optimizer->run();


			//Cout << optimizer->all_variables() << std::endl;
			if (outputLevel == DEBUG_OUTPUT) {
				Cout << "Optimization Run: Initial point: \n";
				for (int i = 0; i < initial_point.length(); ++i) {
					Cout << initial_point[i] << " ";
				}
				Cout << "\nOptimization Run. Best point: \n";
				Cout << optimizer->variables_results().continuous_variables() << std::endl;
				Cout << "Objective: " << optimizer->response_results().function_value(0) << std::endl;
				Cout << "Constraint: " << optimizer->response_results().function_value(1) << std::endl;
				Cout << "Relative Constraint violation: " << std::abs(1 - optimizer->response_results().function_value(1)/nonlin_eq_targets[0]) << std::endl;
				Cout << "\n";
			}

			if(std::abs(1. - optimizer->response_results().function_value(1)/nonlin_eq_targets[0]) > 1.0e-5){
				if (outputLevel == DEBUG_OUTPUT) Cout << "Relative Constraint violation violated: Switching to log scale " << std::endl;
				for (size_t step = 0; step < num_steps; ++step) {
				  initial_point[step] = N_target_qoi(qoi, step); //N_target_qoi(qoi, step); //N_l[step][qoi]; //8. > N_target_qoi(qoi, step) ? 8 : N_target_qoi(qoi, step); //optimizer->variables_results().continuous_variable(step) > pilot_samples[step] ? optimizer->variables_results().continuous_variable(step) : pilot_samples[step];
				}
				nonlin_eq_targets[0] = std::log(eps_sq_div_2[qoi]); //std::log(convergenceTol);
				#ifdef HAVE_NPSOL
		    	if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
					objective_function_npsol_ptr = &target_cost_objective_eval_npsol;
					switch(allocationTarget){
						case TARGET_VARIANCE:
							constraint_function_npsol_ptr = &target_var_constraint_eval_logscale_npsol;
							break;
						case TARGET_SIGMA:
							constraint_function_npsol_ptr = &target_sigma_constraint_eval_logscale_npsol;
							break;
						case TARGET_SCALARIZATION:
							constraint_function_npsol_ptr = &target_scalarization_constraint_eval_logscale_npsol;
							break;
						default:
							 break;
					}
					}else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
						constraint_function_npsol_ptr = &target_cost_constraint_eval_npsol;
						switch(allocationTarget){
							case TARGET_VARIANCE:
								objective_function_npsol_ptr = &target_var_objective_eval_logscale_npsol;
								break;
							case TARGET_SIGMA:
								objective_function_npsol_ptr = &target_sigma_objective_eval_logscale_npsol;
								break;
							case TARGET_SCALARIZATION:
								objective_function_npsol_ptr = &target_scalarization_objective_eval_logscale_npsol;
								break;
							default:
								 break;
						}
					}else{
			  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
			  	  abort_handler(INTERFACE_ERROR);
					}
					optimizer.reset(new NPSOLOptimizer(initial_point,
					                               var_lower_bnds, var_upper_bnds,
					                               lin_ineq_coeffs, lin_ineq_lower_bnds,
					                               lin_ineq_upper_bnds, lin_eq_coeffs,
					                               lin_eq_targets, nonlin_ineq_lower_bnds,
					                               nonlin_ineq_upper_bnds, nonlin_eq_targets,
					                               objective_function_npsol_ptr,
					                               constraint_function_npsol_ptr,
					                               3, 1e-15)
					                               ); //derivative_level = 3 means user_supplied gradients
				#elif HAVE_OPTPP
			    if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
						objective_function_optpp_ptr = &target_cost_objective_eval_optpp;
						switch(allocationTarget){
							case TARGET_VARIANCE:
								constraint_function_optpp_ptr = &target_var_constraint_eval_logscale_optpp;
								break;
							case TARGET_SIGMA:
								constraint_function_optpp_ptr = &target_sigma_constraint_eval_logscale_optpp;
								break;
							case TARGET_SCALARIZATION:
								constraint_function_optpp_ptr = &target_scalarization_constraint_eval_logscale_optpp;
								break;
							default:
								 break;
						}
					}else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
						constraint_function_optpp_ptr = &target_cost_constraint_eval_optpp;
						switch(allocationTarget){
							case TARGET_VARIANCE:
								objective_function_optpp_ptr = &target_var_objective_eval_logscale_optpp;
								break;
							case TARGET_SIGMA:
								objective_function_optpp_ptr = &target_sigma_objective_eval_logscale_optpp;
								break;
							case TARGET_SCALARIZATION:
								objective_function_optpp_ptr = &target_scalarization_objective_eval_logscale_optpp;
								break;
							default:
								 break;
						}
					}else{
			  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
			  	  abort_handler(INTERFACE_ERROR);
					}
					optimizer.reset(new SNLLOptimizer(initial_point,
					            var_lower_bnds,      var_upper_bnds,
					            lin_ineq_coeffs, lin_ineq_lower_bnds,
					            lin_ineq_upper_bnds, lin_eq_coeffs,
					            lin_eq_targets,     nonlin_ineq_lower_bnds,
					            nonlin_ineq_upper_bnds, nonlin_eq_targets,
					            objective_function_optpp_ptr,
					            constraint_function_optpp_ptr,
					            100000, 100000, 1.e-14,
					            1.e-14, 100000));
				#endif
				optimizer->run();
				if (outputLevel == DEBUG_OUTPUT) {
					Cout << "Log Optimization Run: Initial point: \n";
					for (int i = 0; i < initial_point.length(); ++i) {
						Cout << initial_point[i] << " ";
					}
					Cout << "\nLog Optimization Run. Best point: \n";
					Cout << optimizer->variables_results().continuous_variables() << std::endl;
					Cout << "Objective: " << optimizer->response_results().function_value(0) << std::endl;
					Cout << "Constraint: " << optimizer->response_results().function_value(1) << std::endl;
					Cout << "Relative Constraint violation: " << std::abs(1 - optimizer->response_results().function_value(1)/nonlin_eq_targets[0]) << std::endl;
					Cout << "\n";
				}
			}

			for (size_t step=0; step<num_steps; ++step) {
			  N_target_qoi(qoi, step) = optimizer->variables_results().continuous_variable(step);
			}

			if (outputLevel == DEBUG_OUTPUT) {
			  Cout << "Final Optimization results: \n";
			  Cout << N_target_qoi << std::endl<< std::endl;
			}
		}
	}

	for (size_t qoi = 0; qoi < nb_aggregation_qois; ++qoi) {
		for (size_t step = 0; step < num_steps; ++step) {
		    if(allocationTarget == TARGET_MEAN){
		      delta_N_l_qoi(qoi, step) = one_sided_delta(N_l[step][qoi], N_target_qoi(qoi, step));
		    }else if (allocationTarget == TARGET_VARIANCE || allocationTarget == TARGET_SIGMA || allocationTarget == TARGET_SCALARIZATION){
		    	if(max_iter==1){
		    		delta_N_l_qoi(qoi, step) = one_sided_delta(N_l[step][qoi], N_target_qoi(qoi, step));
		    	}else{
			      delta_N_l_qoi(qoi, step) = std::min(N_l[step][qoi]*3, one_sided_delta(N_l[step][qoi], N_target_qoi(qoi, step)));
			      delta_N_l_qoi(qoi, step) = delta_N_l_qoi(qoi, step) > 1 
			      														?  
			      													 	delta_N_l_qoi(qoi, step)*underrelaxation_factor > 1 
			      													 		?
			      													 			delta_N_l_qoi(qoi, step)*underrelaxation_factor 
			      													 		: 
			      													 			1
			      													 	:
			      													 	delta_N_l_qoi(qoi, step);
		      }
		    }else{
		      Cout << "NonDMultilevelSampling::compute_sample_allocation_target: allocationTarget is not implemented.\n";
		      abort_handler(INTERFACE_ERROR);
		    }
	  	}
	}
	if (qoiAggregation==QOI_AGGREGATION_SUM) {
		for (size_t step = 0; step < num_steps; ++step) {
			delta_N_l[step] = delta_N_l_qoi(0, step);
		}
	}else if (qoiAggregation==QOI_AGGREGATION_MAX) {
		Real max_qoi_idx = -1, max_cost = -1, cur_cost = 0;
		for (size_t step = 0; step < num_steps; ++step) {
			max_qoi_idx = 0;
			for (size_t qoi = 1; qoi < nb_aggregation_qois; ++qoi) {
				max_qoi_idx = delta_N_l_qoi(qoi, step) > delta_N_l_qoi(max_qoi_idx, step) ? qoi : max_qoi_idx;
			}
			//max_qoi_idx = 1;
			delta_N_l[step] = delta_N_l_qoi(max_qoi_idx, step);
	   }
	}else{
  	  Cout << "NonDMultilevelSampling::compute_sample_allocation_target: qoiAggregation is not known.\n";
  	  abort_handler(INTERFACE_ERROR);
	}
}

inline void NonDMultilevelSampling::compute_moments(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1, const Sizet2DArray& N_l)
{
	//RealMatrix Q_raw_mom(numFunctions, 4);
	RealMatrix &sum_Q1l = sum_Ql[1], &sum_Q2l = sum_Ql[2],
	    &sum_Q3l = sum_Ql[3], &sum_Q4l = sum_Ql[4],
	    &sum_Q1lm1 = sum_Qlm1[1], &sum_Q2lm1 = sum_Qlm1[2],
	    &sum_Q3lm1 = sum_Qlm1[3], &sum_Q4lm1 = sum_Qlm1[4];
  IntIntPair pr11(1, 1);
	Real cm1, cm2, cm3, cm4, cm1l, cm2l, cm3l, cm4l;
	size_t num_steps = sum_Q1l.numCols();
	assert(sum_Q1l.numCols() == sum_Q2l.numCols() && sum_Q2l.numCols() == sum_Q3l.numCols()&& sum_Q3l.numCols() == sum_Q4l.numCols());

	if (momentStats.empty())
	  momentStats.shapeUninitialized(4, numFunctions);
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
  	cm1 = cm2 = cm3 = cm4 = 0.;
	  for (size_t step=0; step<num_steps; ++step) {
	    size_t Nlq = N_l[step][qoi];
	    // roll up unbiased moments centered on level mean
	    uncentered_to_centered(sum_Q1l(qoi, step) / Nlq, sum_Q2l(qoi, step) / Nlq,
	                           sum_Q3l(qoi, step) / Nlq, sum_Q4l(qoi, step) / Nlq,
	                           cm1l, cm2l, cm3l, cm4l, Nlq);

	    cm1 += cm1l;
	    cm2 += cm2l;
	    cm3 += cm3l;
	    cm4 += cm4l;

	    if (outputLevel == DEBUG_OUTPUT)
	    Cout << "CM_l   for level " << step << ": "
	         << cm1l << ' ' << cm2l << ' ' << cm3l << ' ' << cm4l << '\n';
	    if (step) {
	      uncentered_to_centered(sum_Q1lm1(qoi, step) / Nlq, sum_Q2lm1(qoi, step) / Nlq,
	                             sum_Q3lm1(qoi, step) / Nlq, sum_Q4lm1(qoi, step) / Nlq,
	                             cm1l, cm2l, cm3l, cm4l, Nlq);
	      cm1 -= cm1l;
	      cm2 -= cm2l; 
	      cm3 -= cm3l;
	      cm4 -= cm4l;
	      if (outputLevel == DEBUG_OUTPUT)
	      Cout << "CM_lm1 for level " << step << ": "
	           << cm1l << ' ' << cm2l << ' ' << cm3l << ' ' << cm4l << '\n';
	    }
	  }
	  check_negative(cm2);
	  check_negative(cm4);
	  Real *mom_q = momentStats[qoi];
	  if (finalMomentsType == CENTRAL_MOMENTS) {
	    mom_q[0] = cm1;
	    mom_q[1] = cm2;
	    mom_q[2] = cm3;
	    mom_q[3] = cm4;
	  } else
	    centered_to_standard(cm1, cm2, cm3, cm4,
	                         mom_q[0], mom_q[1], mom_q[2], mom_q[3]);
	}
}

inline void NonDMultilevelSampling::compute_equiv_HF_evals(const SizetArray& raw_N_l, const RealVector& cost){
	size_t num_steps = raw_N_l.size();
	equivHFEvals = raw_N_l[0] * cost[0]; // first level is single eval
	for (size_t step=1; step<num_steps; ++step)// subsequent levels incur 2 model costs
	  equivHFEvals += raw_N_l[step] * (cost[step] + cost[step - 1]);
	equivHFEvals /= cost[num_steps - 1]; // normalize into equivalent HF evals
}


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


/** For single-level moment calculations with a scalar Nlq. */
inline void NonDMultilevelSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
		       Real& cm1, Real& cm2, Real& cm3, Real& cm4)
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
		       Real& cm2, Real& cm3, Real& cm4, size_t Nlq)
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
    
    //cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * cm2 * cm2 )
    //    / (n_sq - 3. * Nlq + 3.);
    //[fm] account for bias correction due to cm2^2 term
    cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * (n_sq - Nlq) / (n_sq - 2. * Nlq + 3) * cm2 * cm2 )
        / ( (n_sq - 3. * Nlq + 3.) - (6. * Nlq - 9.) * (n_sq - Nlq) / (Nlq * (n_sq - 2. * Nlq + 3.)) );
  }
  else
    Cerr << "Warning: due to small sample size, resorting to biased estimator "
	 << "conversion in NonDMultilevelSampling::uncentered_to_centered().\n";
}


inline void NonDMultilevelSampling::
centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
		     Real& sm1, Real& sm2, Real& sm3, Real& sm4)
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


inline void NonDMultilevelSampling::check_negative(Real& cm)
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

inline Real NonDMultilevelSampling::var_of_var_ml_l0(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1,
                                                      const size_t& Nlq_pilot, const Real& Nlq, const size_t& qoi, const bool& compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm2l_sq, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  RealMatrix &sum_Q1l = sum_Ql[1], &sum_Q1lm1 = sum_Qlm1[1],
      &sum_Q2l = sum_Ql[2], &sum_Q2lm1 = sum_Qlm1[2],
      &sum_Q3l = sum_Ql[3], &sum_Q3lm1 = sum_Qlm1[3],
      &sum_Q4l = sum_Ql[4], &sum_Q4lm1 = sum_Qlm1[4],
      &sum_Q1lQ1lm1 = sum_QlQlm1[pr11], &sum_Q1lQ2lm1 = sum_QlQlm1[pr12],
      &sum_Q2lQ1lm1 = sum_QlQlm1[pr21], &sum_Q2lQ2lm1 = sum_QlQlm1[pr22];

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

  check_negative(var_of_var);
  return var_of_var;
}


inline Real NonDMultilevelSampling::var_of_var_ml_lmax(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1,
                                                     const size_t& Nlq_pilot, const Real& Nlq, const size_t& qoi, const bool& compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm2l_sq, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  RealMatrix &sum_Q1l = sum_Ql[1], &sum_Q1lm1 = sum_Qlm1[1],
      &sum_Q2l = sum_Ql[2], &sum_Q2lm1 = sum_Qlm1[2],
      &sum_Q3l = sum_Ql[3], &sum_Q3lm1 = sum_Qlm1[3],
      &sum_Q4l = sum_Ql[4], &sum_Q4lm1 = sum_Qlm1[4],
      &sum_Q1lQ1lm1 = sum_QlQlm1[pr11], &sum_Q1lQ2lm1 = sum_QlQlm1[pr12],
      &sum_Q2lQ1lm1 = sum_QlQlm1[pr21], &sum_Q2lQ2lm1 = sum_QlQlm1[pr22];

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

  check_negative(var_of_var);
  return var_of_var;
}

inline Real NonDMultilevelSampling::var_of_var_ml_l(IntRealMatrixMap sum_Ql, IntRealMatrixMap sum_Qlm1, IntIntPairRealMatrixMap sum_QlQlm1,
                                                    const size_t& Nlq_pilot, const Real& Nlq, const size_t& qoi, const size_t& lev, const bool& compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm1lm1, cm2lm1,
      cm3lm1, cm4lm1, cm1l_sq, cm2l_sq, cm2lm1_sq,
      mu_Q2l, mu_Q2lm1, mu_Q2lQ2lm1,
      mu_Q1lm1_mu_Q2lQ1lm1, mu_Q1lm1_mu_Q1lm1_muQ2l, mu_Q1l_mu_Q1lQ2lm1, mu_Q1l_mu_Q1l_mu_Q2lm1,
      mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1, mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1, mu_Q2l_muQ2lm1, mu_Q1lQ1lm1_mu_Q1lQ1lm1,
      mu_P2lP2lm1, var_P2l, var_P2lm1, covar_P2lP2lm1, term, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql[1], &sum_Q1lm1 = sum_Qlm1[1],
      &sum_Q2l = sum_Ql[2], &sum_Q2lm1 = sum_Qlm1[2],
      &sum_Q3l = sum_Ql[3], &sum_Q3lm1 = sum_Qlm1[3],
      &sum_Q4l = sum_Ql[4], &sum_Q4lm1 = sum_Qlm1[4],
      &sum_Q1lQ1lm1 = sum_QlQlm1[pr11], &sum_Q1lQ2lm1 = sum_QlQlm1[pr12],
      &sum_Q2lQ1lm1 = sum_QlQlm1[pr21], &sum_Q2lQ2lm1 = sum_QlQlm1[pr22];

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
  var_P2l = Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);
  var_P2lm1 =
      Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4lm1 - (Nlq - 3.) / (Nlq - 1.) * cm2lm1_sq);

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
      = mu_P2lP2lm1 + term / (Nlq - 1.);

  var_of_var = (var_P2l + var_P2lm1 - 2. * covar_P2lP2lm1) / Nlq;

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

  check_negative(var_of_var);
  return var_of_var;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_pair(const Real& sumQ1, const Real& sumQ2, const Real& sumQ1Q2, const size_t& Nlq)
{
  Real mean1, mean2, bessel_corr1, bessel_corr2 = 0.;

  mean1 = 1./Nlq * 1./Nlq * sumQ1 * sumQ2;
  mean2 = 1./Nlq * sumQ1Q2;
  bessel_corr1 = (Real)Nlq / ((Real)Nlq - 1.);
  bessel_corr2 = 1. / ((Real)Nlq - 1.);

  return bessel_corr1*mean1 - bessel_corr2*mean2;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_triplet(const Real& sumQ1, const Real& sumQ2, const Real& sumQ3,
                                                                  const Real& sumQ1Q2, const Real& sumQ1Q3, const Real& sumQ2Q3, const Real& sumQ1Q2Q3, const size_t& Nlq)
{
  Real mean1, mean2, mean3, bessel_corr1, bessel_corr2, bessel_corr3 = 0.;

  mean1 = 1./Nlq * 1./Nlq * 1./Nlq * sumQ1 * sumQ2 * sumQ3;
  mean2 = unbiased_mean_product_pair(sumQ1Q2, sumQ3, sumQ1Q2Q3, Nlq);
  mean2 += unbiased_mean_product_pair(sumQ2Q3, sumQ1, sumQ1Q2Q3, Nlq);
  mean2 += unbiased_mean_product_pair(sumQ1Q3, sumQ2, sumQ1Q2Q3, Nlq);
  mean3 = 1./((Real)Nlq) * sumQ1Q2Q3;
  bessel_corr1 = (Nlq * Nlq)/((Nlq - 1.)*(Nlq - 2.));
  bessel_corr2 = 1./(Nlq - 2.);
  bessel_corr3 = 1./((Nlq - 1.)*(Nlq - 2.));

  return bessel_corr1 * mean1 - bessel_corr2 * mean2 - bessel_corr3 * mean3;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_pairpair(const Real& sumQ1, const Real& sumQ2, const Real& sumQ1Q2,
                                                                   const Real& sumQ1sq, const Real& sumQ2sq,
                                                                   const Real& sumQ1sqQ2, const Real& sumQ1Q2sq, const Real& sumQ1sqQ2sq, const size_t& Nlq)
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

  bessel_corr1 = (Nlq * Nlq * Nlq)/((Nlq - 1.)*(Nlq - 2.)*(Nlq - 3.));
  bessel_corr2 = 1./(Nlq - 3.);
  bessel_corr3 = 1./((Nlq - 2.)*(Nlq - 3.));
  bessel_corr4 = 1./((Nlq - 1.)*(Nlq - 2.)*(Nlq - 3.));

  return bessel_corr1 * mean1 - bessel_corr2 * mean2 - bessel_corr3 * mean3 - bessel_corr4 * mean4;
}

} // namespace Dakota

#endif
