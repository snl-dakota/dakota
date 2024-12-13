/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_MULTILEV_CONTROL_VAR_SAMPLING_H
#define NOND_MULTILEV_CONTROL_VAR_SAMPLING_H

#include "NonDMultilevelSampling.hpp"
//#include "NonDMultifidelitySampling.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// Performs multilevel-multifidelity Monte Carlo sampling for
/// uncertainty quantification.

/** Multilevel-multifidelity Monte Carlo (MLMFMC) combines variance
    decay across model resolutions with variance reduction from a
    control variate across model fidelities. */

class NonDMultilevControlVarSampling: public NonDMultilevelSampling
                                  //, public NonDMultifidelitySampling
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
  void print_variance_reduction(std::ostream& s);

  bool discrepancy_sample_counts() const;

private:

  //
  //- Heading: Helper functions
  //

  // Perform multilevel Monte Carlo across levels in combination with
  // control variate Monte Carlo across model forms at each level; CV
  // computes correlations for Y (LH correlations for level discrepancies)
  //void multilevel_control_variate_mc_Ycorr();
  /// Perform multilevel Monte Carlo across levels in combination with
  /// control variate Monte Carlo across model forms at each level; CV
  /// computes correlations for Q (LH correlations for QoI)
  void multilevel_control_variate_mc_online_pilot(); //_Qcorr();
  /// Qcorr approach using a pilot sample treated as separate offline cost
  void multilevel_control_variate_mc_offline_pilot();
  /// Qcorr approach projecting estimator performance from a pilot sample
  void multilevel_control_variate_mc_pilot_projection();

  /// define the truth and surrogate keys
  void assign_active_key();

  /// helper for shared code among online and offline pilot modes
  void evaluate_levels(IntRealMatrixMap& sum_Ll_pilot,
		       IntRealMatrixMap& sum_Llm1_pilot,
		       IntRealMatrixMap& sum_Hl_pilot,
		       IntRealMatrixMap& sum_Hlm1_pilot,
		       IntRealMatrixMap& sum_Ll_Ll_pilot,
		       IntRealMatrixMap& sum_Ll_Llm1_pilot,
		       IntRealMatrixMap& sum_Llm1_Llm1_pilot,
		       IntRealMatrixMap& sum_Hl_Ll_pilot,
		       IntRealMatrixMap& sum_Hl_Llm1_pilot,
		       IntRealMatrixMap& sum_Hlm1_Ll_pilot,
		       IntRealMatrixMap& sum_Hlm1_Llm1_pilot,
		       IntRealMatrixMap& sum_Hl_Hl_pilot,
		       IntRealMatrixMap& sum_Hl_Hlm1_pilot,
		       IntRealMatrixMap& sum_Hlm1_Hlm1_pilot,
		       RealVectorArray& eval_ratios, RealMatrix& Lambda,
		       RealMatrix& var_YH,     SizetArray& delta_N_hf,
		       Sizet2DArray& N_actual, SizetArray& N_alloc,
		       RealVector& hf_targets, bool accumulate_cost,
		       bool pilot_estvar);
  /// helper for shared code among pilot-projection modes
  void evaluate_pilot(RealVectorArray& eval_ratios, RealMatrix& Lambda,
		      RealMatrix& var_YH,    Sizet2DArray& N_actual,
		      SizetArray& N_alloc,   RealVector& hf_targets,
		      RealMatrix& pilot_mom, bool accumulate_cost,
		      bool pilot_estvar);

  /// evaluate multiple sample batches concurrently, where each batch involves
  /// either a single level or level pair for both HF and LF models
  void mlmf_increments(const SizetArray& delta_N_l, String prepend);
  /// accumulate sample values for multiple sample batches coming from
  /// mlmf_increments()
  template <typename SumContainer1, typename SumContainer2> 
  void accumulate_increments(const SizetArray& delta_N_hf,
			     Sizet2DArray& N_actual, SizetArray& N_alloc,
			     const RealVector& hf_targets,
			     SumContainer1& sum_Ll, SumContainer1& sum_Llm1,
			     SumContainer2& sum_Hl, SumContainer2& sum_Hlm1,
			     SumContainer1& sum_Ll_Ll,
			     SumContainer1& sum_Ll_Llm1,
			     SumContainer1& sum_Llm1_Llm1,
			     SumContainer1& sum_Hl_Ll,
			     SumContainer1& sum_Hl_Llm1,
			     SumContainer1& sum_Hlm1_Ll,
			     SumContainer1& sum_Hlm1_Llm1,
			     SumContainer1& sum_Hl_Hl,
			     SumContainer1& sum_Hl_Hlm1,
			     SumContainer1& sum_Hlm1_Hlm1, bool incr_cost);

  /// evaluate multiple sample batches concurrently, where each batch involves
  /// either a single level or level pair for the LF model
  void lf_increments(const SizetArray& delta_N_lf, String prepend);
  /// accumulate sample values for multiple sample batches coming from
  /// lf_increments()
  template <typename SumContainer> 
  void accumulate_lf_increments(const SizetArray& delta_N_lf,
				Sizet2DArray& N_actual_lf,
				SizetArray&   N_alloc_lf,
				const RealVectorArray& lf_targets,
				SumContainer& sum_Ll_refined,
				SumContainer& sum_Llm1_refined);

  /// perform LF sample increment as indicated by evaluation ratios
  size_t lf_increment(const RealVector& eval_ratios, const SizetArray& N_lf,
		      Real hf_target, RealVector& lf_targets);
  /// perform LF sample increment as indicated by evaluation ratios
  size_t lf_increment(const RealVector& eval_ratios, size_t N_lf,
		      Real hf_target, RealVector& lf_targets);

  /// compute the equivalent number of HF evaluations (includes any sim faults)
  Real compute_mlmf_equivalent_cost(const SizetArray& raw_N_hf,
				    const RealVector& hf_cost,
				    const SizetArray& raw_N_lf,
				    const RealVector& lf_cost);
  /// increment the equivalent number of HF evaluations
  void increment_mlmf_equivalent_cost(size_t new_N_hf,  Real hf_lev_cost,
				      size_t new_N_lf,  Real lf_lev_cost,
				      Real hf_ref_cost, Real& equiv_hf);

  /// compute the variance of the MLMF estimator
  void compute_mlmf_estimator_variance(const RealMatrix&   var_Y,
				       const Sizet2DArray& num_Y,
				       const RealMatrix&  Lambda,
				       RealVector& mlmf_est_var);

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

  /// compute scalar variance and correlation parameters for control variates
  void compute_mf_correlation(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			      Real sum_HH, size_t N_shared, Real& var_H,
			      Real& rho2_LH);

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
		      //const RealMatrix& rho_dot2_LH,
		      size_t lev, RealMatrix& H_raw_mom);

  /// compute control variate parameters for all moments, levels, qoi
  void compute_mlmf_controls(IntRealMatrixMap& sum_Ll,
			     IntRealMatrixMap& sum_Llm1,
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
			     const Sizet2DArray& N_shared, size_t num_cv_lev,
			     RealVector2DArray& beta_2d,
			     RealVector2DArray& gamma_2d);
  /// compute control variate parameters for all QoI (given moment, level)
  void compute_mlmf_controls(const Real* sum_Ll_m, const Real* sum_Llm1_m,
			     const Real* sum_Hl_m, const Real* sum_Hlm1_m,
			     const Real* sum_Ll_Ll_m, const Real* sum_Ll_Llm1_m,
			     const Real* sum_Llm1_Llm1_m,
			     const Real* sum_Hl_Ll_m,
			     const Real* sum_Hl_Llm1_m,
			     const Real* sum_Hlm1_Ll_m,
			     const Real* sum_Hlm1_Llm1_m,
			     const Real* sum_Hl_Hl_m,
			     const Real* sum_Hl_Hlm1_m,
			     const Real* sum_Hlm1_Hlm1_m,
			     const SizetArray& N_shared,
			     RealVector& beta_dot, RealVector& gamma);
  /// compute scalar control variate parameters
  void compute_mlmf_control(Real sum_Ll, Real sum_Llm1, Real sum_Hl,
			    Real sum_Hlm1, Real sum_Ll_Ll, Real sum_Ll_Llm1,
			    Real sum_Llm1_Llm1, Real sum_Hl_Ll,
			    Real sum_Hl_Llm1, Real sum_Hlm1_Ll,
			    Real sum_Hlm1_Llm1, Real sum_Hl_Hl,
			    Real sum_Hl_Hlm1, Real sum_Hlm1_Hlm1,
			    size_t N_shared, Real& var_YHl, Real& rho_dot2_LH,
			    Real& beta_dot, Real& gamma);

  /// apply scalar control variate parameter (beta) to approximate HF moment
  void apply_mf_control(Real sum_H, Real sum_L_shared, size_t N_shared,
			Real sum_L_refined, size_t N_refined, Real beta,
			Real& H_raw_mom);
  /// apply matrix control variate parameter (beta) to approximate HF moment
  void apply_mf_controls(const Real* sum_H,           const Real* sum_L_shared,
			 const SizetArray& N_shared,  const Real* sum_L_refined,
			 const SizetArray& N_refined, const RealVector& beta,
			 RealMatrix& H_raw_mom, int m_index);

  /// apply control variate parameter (beta) for all moments, levels, qoi
  void apply_mlmf_controls(IntRealMatrixMap& sum_Ll, IntRealMatrixMap& sum_Llm1,
			   IntRealMatrixMap& sum_Hl, IntRealMatrixMap& sum_Hlm1,
			   const Sizet2DArray& N_shared,
			   IntRealMatrixMap& sum_Ll_refined,
			   IntRealMatrixMap& sum_Llm1_refined,
			   const Sizet2DArray& N_refined, size_t num_cv_lev,
			   const RealVector2DArray& beta_2d,
			   const RealVector2DArray& gamma_2d,
			   RealMatrix& H_raw_mom);
  /// apply control variate parameters for all QoI (given moment, level)
  void apply_mlmf_controls(const Real* sum_Hl, const Real* sum_Hlm1,
			   const Real* sum_Ll, const Real* sum_Llm1,
			   const SizetArray& N_shared,
			   const Real*   sum_Ll_refined,
			   const Real* sum_Llm1_refined,
			   const SizetArray& N_refined,
			   const RealVector& beta_dot, const RealVector& gamma,
			   RealMatrix& H_raw_mom, int m_index);
  /// apply scalar control variate parameter (beta) to approximate HF moment
  void apply_mlmf_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll,
			  Real sum_Llm1, size_t N_shared, Real sum_Ll_refined,
			  Real sum_Llm1_refined, size_t N_refined,
			  Real beta_dot, Real gamma, Real& H_raw_mom);

  /// for pilot projection mode, advance sample counts and accumulated cost
  void update_projected_samples(const RealVector& hf_targets,
				const RealVectorArray& eval_ratios,
				const Sizet2DArray& N_actual_hf,
				SizetArray& N_alloc_hf,
				const Sizet2DArray& N_actual_lf,
				SizetArray& N_alloc_lf,
				SizetArray& delta_N_actual_hf,
				//SizetArray& delta_N_actual_lf,
				Real& delta_equiv_hf);
  /// for pilot projection mode, advance sample counts and accumulated cost
  void update_projected_lf_samples(const RealVector& hf_targets,
				   const RealVectorArray& eval_ratios,
				   const Sizet2DArray& N_actual_lf,
				   SizetArray& N_alloc_lf,
				   //SizetArray& delta_N_actual_lf,
				   Real& delta_equiv_hf);

  /// initialize the MLMF accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlmf_sums(IntRealMatrixMap& sum_Ll,
			    IntRealMatrixMap& sum_Llm1,
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
			    size_t num_ml_lev, size_t num_cv_lev,
			    size_t num_mom = 4);
  /*
  /// initialize the MLMF accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_mlmf_sums(IntRealMatrixMap& sum_L_shared,
			    IntRealMatrixMap& sum_L_refined,
			    IntRealMatrixMap& sum_H,
			    IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
			    IntRealMatrixMap& sum_HH, size_t num_ml_lev,
			    size_t num_cv_lev, size_t num_mom = 4);
  */

  // update running discrepancy sums for one model (sum_Y) using
  // set of model evaluations within allResponses
  //void accumulate_mlmf_Ysums(const IntResponseMap& resp_map,
  // 			     IntRealMatrixMap& sum_Y, size_t lev,
  // 			     SizetArray& num_Y);
  // update running two-level discrepancy sums for two models (sum_L,
  // sum_H, sum_LL, sum_LH, and sum_HH) from set of low/high fidelity
  // model evaluations within mlmf_resp_map
  //void accumulate_mlmf_Ysums(const IntResponseMap& mlmf_resp_map,
  // 			     IntRealMatrixMap& sum_L_shared,
  // 			     IntRealMatrixMap& sum_L_refined,
  // 			     IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
  // 			     IntRealMatrixMap& sum_LH, IntRealMatrixMap& sum_HH,
  // 			     size_t lev, SizetArray& num_L, SizetArray& num_H);

  // Maps:
  /// update running QoI sums for one model at two levels (sum_Ql, sum_Qlm1)
  /// using set of model evaluations within allResponses
  void accumulate_mlmf_Qsums(const IntResponseMap& resp_map,
			     IntRealMatrixMap& sum_Ql,
			     IntRealMatrixMap& sum_Qlm1, size_t lev,
			     SizetArray& num_Q);
  /// update running QoI sums for two models (sum_L, sum_H, sum_LL, sum_LH,
  /// and sum_HH) from set of low/high fidelity model evaluations within
  /// mlmf_resp_map; used for level 0 from other accumulators
  void accumulate_mlmf_Qsums(const IntResponseMap& mlmf_resp_map,
			     IntRealMatrixMap& sum_L_shared,
			     IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
			     IntRealMatrixMap& sum_LH, IntRealMatrixMap& sum_HH,
			     size_t lev, SizetArray& N_shared);
  /// update running QoI sums for two models and two levels from set
  /// of low/high fidelity model evaluations within mlmf_resp_map
  void accumulate_mlmf_Qsums(const IntResponseMap& mlmf_resp_map,
			     IntRealMatrixMap& sum_Ll,
			     IntRealMatrixMap& sum_Llm1,
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
			     SizetArray& N_shared);

  // Matrices:
  /// update running QoI sums for two models (sum_L, sum_H, sum_LL, sum_LH,
  /// and sum_HH) from set of low/high fidelity model evaluations within
  /// mlmf_resp_map; used for level 0 from other accumulators
  void accumulate_mlmf_Qsums(const IntResponseMap& mlmf_resp_map,
			     RealMatrix& sum_L_shared, IntRealMatrixMap& sum_H,
			     RealMatrix& sum_LL, RealMatrix& sum_LH,
			     RealMatrix& sum_HH, size_t lev,
			     SizetArray& N_shared);
  /// update running QoI sums for two models and two levels from set
  /// of low/high fidelity model evaluations within mlmf_resp_map
  void accumulate_mlmf_Qsums(const IntResponseMap& mlmf_resp_map,
			     RealMatrix& sum_Ll,
			     RealMatrix& sum_Llm1,
			     IntRealMatrixMap& sum_Hl,
			     IntRealMatrixMap& sum_Hlm1,
			     RealMatrix& sum_Ll_Ll,
			     RealMatrix& sum_Ll_Llm1,
			     RealMatrix& sum_Llm1_Llm1,
			     RealMatrix& sum_Hl_Ll,
			     RealMatrix& sum_Hl_Llm1,
			     RealMatrix& sum_Hlm1_Ll,
			     RealMatrix& sum_Hlm1_Llm1,
			     RealMatrix& sum_Hl_Hl,
			     RealMatrix& sum_Hl_Hlm1,
			     RealMatrix& sum_Hlm1_Hlm1, size_t lev,
			     SizetArray& N_shared);

  //
  //- Heading: Data
  //

  //RealMatrix varYH;

  /// core_run() can delegate execution to either ML or CV if hierarchy
  /// does not support MLCV; in this case output must also be delegated
  short delegateMethod;
};


inline NonDMultilevControlVarSampling::~NonDMultilevControlVarSampling()
{ }


inline bool NonDMultilevControlVarSampling::discrepancy_sample_counts() const
{ return true; }


template <typename SumContainer1, typename SumContainer2> 
void NonDMultilevControlVarSampling::
accumulate_increments(const SizetArray& delta_N_hf, Sizet2DArray& N_actual_hf,
		      SizetArray& N_alloc_hf,    const RealVector& hf_targets,
		      SumContainer1& sum_Ll,        SumContainer1& sum_Llm1,
		      SumContainer2& sum_Hl,        SumContainer2& sum_Hlm1,
		      SumContainer1& sum_Ll_Ll,     SumContainer1& sum_Ll_Llm1,
		      SumContainer1& sum_Llm1_Llm1, SumContainer1& sum_Hl_Ll,
		      SumContainer1& sum_Hl_Llm1,   SumContainer1& sum_Hlm1_Ll,
		      SumContainer1& sum_Hlm1_Llm1, SumContainer1& sum_Hl_Hl,
		      SumContainer1& sum_Hl_Hlm1,
		      SumContainer1& sum_Hlm1_Hlm1, bool incr_cost)
{
  unsigned short group;
  size_t lev, N_alloc_l, num_mf = NLevActual.size(),
    num_hf_lev = iteratedModel.truth_model().solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel.surrogate_model().solution_levels()) : 0;
  Real hf_lev_cost, lf_lev_cost, hf_ref_cost = sequenceCost[numApprox];

  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

    numSamples = delta_N_hf[lev];
    if (!numSamples) continue;

    N_alloc_l = (backfillFailures && mlmfIter) ?
      one_sided_delta(N_alloc_hf[lev], hf_targets[lev], relaxFactor) :
      numSamples;
    N_alloc_hf[lev] += N_alloc_l;

    IntResponseMap& mlmf_resp_map = batchResponsesMap[lev];
    if (incr_cost) hf_lev_cost = level_cost(sequenceCost, lev, num_cv_lev);

    // control variate betwen LF and HF for this discretization level:
    // if unequal number of levels, LF levels are assigned as CV to the
    // leading set of HF levels, since these tend to have larger variance.
    if (lev < num_cv_lev) {
      // process previous and new set of allResponses for MLMF sums;
      accumulate_mlmf_Qsums(mlmf_resp_map, sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1,
			    sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll,
			    sum_Hl_Llm1, sum_Hlm1_Ll, sum_Hlm1_Llm1,
			    sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1, lev,
			    N_actual_hf[lev]); // N_actual_lf sync'd downstream
      if (incr_cost) {
	lf_lev_cost = level_cost(sequenceCost, lev);
	increment_mlmf_equivalent_cost(numSamples, hf_lev_cost, numSamples,
				       lf_lev_cost,hf_ref_cost, equivHFEvals);
      }
      //N_alloc_lf[lev] += numSamples; // N_alloc_lf sync'd downstream
    }
    else { // no LF for this level; accumulate only multilevel discrepancies
      // accumulate H sums for lev = 0, Y sums for lev > 0
      accumulate_ml_Ysums(mlmf_resp_map, sum_Hl, sum_Hl_Hl, lev, num_cv_lev,
			  N_actual_hf[lev]); // offet for HF lev
      if (incr_cost)
	increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost,
				     equivHFEvals);
    }
  }
  clear_batches();
}


template <typename SumContainer> 
void NonDMultilevControlVarSampling::
accumulate_lf_increments(const SizetArray& delta_N_lf,
			 Sizet2DArray& N_actual_lf, SizetArray& N_alloc_lf,
			 const RealVectorArray& lf_targets,
			 SumContainer& sum_Ll_refined,
			 SumContainer& sum_Llm1_refined)
{
  // All CV lf_increment() calls now follow convergence of ML iteration:
  // > Avoids early mis-estimation of LF increments
  // > Parallel scheduling benefits from one final large batch of refinements

  unsigned short group;
  size_t lev, num_mf = NLevActual.size(),
    num_hf_lev = iteratedModel.truth_model().solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel.surrogate_model().solution_levels()) : 0;
  Real hf_ref_cost = sequenceCost[numApprox];

  for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
    numSamples = delta_N_lf[lev];
    if (numSamples) {
      accumulate_mlmf_Qsums(batchResponsesMap[lev], sum_Ll_refined,
			    sum_Llm1_refined, lev, N_actual_lf[lev]);
      //if (incr_cost) // cost of LF increments always incurred
      increment_ml_equivalent_cost(numSamples, level_cost(sequenceCost, lev),
				   hf_ref_cost, equivHFEvals);
      N_alloc_lf[lev] += (backfillFailures) ?
	one_sided_delta(N_alloc_lf[lev], average(lf_targets[lev])) : numSamples;
    }
  }
  clear_batches();
}


inline Real NonDMultilevControlVarSampling::
compute_mlmf_equivalent_cost(const SizetArray& raw_N_hf,
			     const RealVector& hf_cost,
			     const SizetArray& raw_N_lf,
			     const RealVector& lf_cost)
{
  Real equiv_hf = 0.;
  if (raw_N_hf[0]) equiv_hf += raw_N_hf[0] * hf_cost[0]; // 1st level
  if (raw_N_lf[0]) equiv_hf += raw_N_lf[0] * lf_cost[0]; // 1st level
  size_t lev, num_hf_lev = raw_N_hf.size(), num_cv_lev = raw_N_lf.size();
  for (lev=1; lev<num_hf_lev; ++lev) // subsequent levels incur 2 model costs
    if (raw_N_hf[lev])
      equiv_hf += raw_N_hf[lev] * (hf_cost[lev] + hf_cost[lev-1]);
  for (lev=1; lev<num_cv_lev; ++lev) // subsequent levels incur 2 model costs
    if (raw_N_lf[lev])
      equiv_hf += raw_N_lf[lev] * (lf_cost[lev] + lf_cost[lev-1]);
  equiv_hf /= hf_cost[num_hf_lev-1]; // normalize into equiv HF evals
  return equiv_hf;
}


inline void NonDMultilevControlVarSampling::
increment_mlmf_equivalent_cost(size_t new_N_hf, Real hf_lev_cost,
			       size_t new_N_lf, Real lf_lev_cost,
			       Real hf_ref_cost, Real& equiv_hf)
{
  // increment the equivalent number of HF evaluations
  Real incr = 0.;
  if (new_N_hf) incr += new_N_hf * hf_lev_cost;
  if (new_N_lf) incr += new_N_lf * lf_lev_cost;
  equiv_hf += incr / hf_ref_cost; // normalize into equiv HF evals

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "MLMF incremented by " << new_N_hf << " HF and " << new_N_lf
	 << " LF samples.  equivalent HF evals = " << equiv_hf << std::endl;
}


inline void NonDMultilevControlVarSampling::
compute_mlmf_estimator_variance(const RealMatrix&   var_Y,
				const Sizet2DArray& num_Y,
				const RealMatrix&  Lambda,
				RealVector& mlmf_est_var)
{
  mlmf_est_var.size(numFunctions); // init to 0
  size_t qoi, lev, num_lev = num_Y.size(), num_cv_lev = Lambda.numCols();
  for (lev=0; lev<num_lev; ++lev) {
    const Real*       var_Yl = var_Y[lev];
    const SizetArray& num_Yl = num_Y[lev];
    if (lev < num_cv_lev) { // control variate with LF model for this level
      const Real* Lambda_l = Lambda[lev];
      for (qoi=0; qoi<numFunctions; ++qoi)
	mlmf_est_var[qoi] += var_Yl[qoi] / num_Yl[qoi] * Lambda_l[qoi];
    }
    else // no control variate for this level
      for (qoi=0; qoi<numFunctions; ++qoi)
	mlmf_est_var[qoi] += var_Yl[qoi] / num_Yl[qoi];
  }
}


inline void NonDMultilevControlVarSampling::
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


inline void NonDMultilevControlVarSampling::
apply_mf_control(Real sum_H, Real sum_L_shared, size_t N_shared,
		 Real sum_L_refined, size_t N_refined, Real beta,
		 Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom += sum_H / N_shared                   // mu_H from shared samples
             - beta * (sum_L_shared  / N_shared - // mu_L from shared samples
		       sum_L_refined / N_refined);// refined_mu_L incl increment
}


inline void NonDMultilevControlVarSampling::
apply_mlmf_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll, Real sum_Llm1,
		   size_t N_shared,  Real sum_Ll_refined,
		   Real sum_Llm1_refined, size_t N_refined, Real beta_dot,
		   Real gamma, Real& H_raw_mom)
{
  // updated LF expectations following final sample increment:
  Real mu_Hl = sum_Hl / N_shared,  mu_Hlm1 = sum_Hlm1 / N_shared,
       mu_Ll = sum_Ll / N_shared,  mu_Llm1 = sum_Llm1 / N_shared;
  Real refined_mu_Ll   =   sum_Ll_refined / N_refined;
  Real refined_mu_Llm1 = sum_Llm1_refined / N_refined;

  // apply control for HF uncentered raw moment estimates:
  Real mu_YH            = mu_Hl - mu_Hlm1;
  Real mu_YLdot         = gamma *         mu_Ll -         mu_Llm1;
  Real refined_mu_YLdot = gamma * refined_mu_Ll - refined_mu_Llm1;
  H_raw_mom            += mu_YH - beta_dot * (mu_YLdot - refined_mu_YLdot);
}

} // namespace Dakota

#endif
