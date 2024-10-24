/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_ML_BLUE_SAMPLING_H
#define NOND_ML_BLUE_SAMPLING_H

#include "NonDNonHierarchSampling.hpp"
//#include "DataMethod.hpp"

#define DIRECT_DIMENSION_LIMIT 64

namespace Dakota {


/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Approximate Control Variate (ACV) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevBLUESampling: public NonDNonHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevBLUESampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevBLUESampling();

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
  void print_multigroup_summary(std::ostream& s, const String& summary_type,
				bool projections);
  void print_variance_reduction(std::ostream& s);

  //void estimator_variance_ratios(const RealVector& r_and_N,
  //				   RealVector& estvar_ratios);
  Real average_estimator_variance(const RealVector& cd_vars);

  void numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
				 size_t& num_nln_con);
  void numerical_solution_bounds_constraints(const MFSolutionData& soln,
    RealVector& x0, RealVector& x_lb, RealVector& x_ub,
    RealVector& lin_ineq_lb, RealVector& lin_ineq_ub, RealVector& lin_eq_tgt,
    RealVector& nln_ineq_lb, RealVector& nln_ineq_ub, RealVector& nln_eq_tgt,
    RealMatrix& lin_ineq_coeffs, RealMatrix& lin_eq_coeffs);
  void derived_finite_solution_bounds(const RealVector& x0, RealVector& x_lb,
				      RealVector& x_ub, Real budget);

  Real linear_group_cost(const RealVector& cdv);
  void linear_group_cost_gradient(const RealVector& cdv, RealVector& grad_c);

  void apply_mc_reference(RealVector& mc_targets);

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

  void compute_C_inverse(const RealSymMatrix& cov_GG_gq,
			 RealSymMatrix& cov_GG_inv_gq,
			 size_t group, size_t qoi, Real& rcond);
  void compute_C_inverse(const RealSymMatrix2DArray& cov_GG,
			 RealSymMatrix2DArray& cov_GG_inv);
  void compute_Psi(const RealSymMatrix2DArray& cov_GG_inv,
		   const RealVector& N_G, RealSymMatrixArray& Psi);
  void compute_Psi(const RealSymMatrix2DArray& cov_GG_inv,
		   const Sizet2DArray& N_G, RealSymMatrixArray& Psi);
  /*
  void invert_Psi(RealSymMatrixArray& Psi);
  void compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
			   const RealVector& N_G_1D,
			   RealSymMatrixArray& Psi_inv);
  void compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
			   const Sizet2DArray& N_G_2D,
			   RealSymMatrixArray& Psi_inv);
  */
  void compute_y(const RealSymMatrix2DArray& cov_GG_inv,
		 const RealMatrixArray& sum_G, RealVectorArray& y);
  void compute_mu_hat(const RealSymMatrix2DArray& cov_GG_inv,
		      const RealMatrixArray& sum_G, const Sizet2DArray& N_G,
		      RealVectorArray& mu_hat);

  void estimator_variance(const RealVector& cd_vars, RealVector& estvar);

  void process_group_solution(MFSolutionData& soln,
			      const Sizet2DArray& N_G_actual,
			      const SizetArray& N_G_alloc,
			      SizetArray& delta_N_G);

  void blue_raw_moments(const IntRealMatrixArrayMap& sum_G_online,
			const IntRealSymMatrix2DArrayMap& sum_GG_online,
			const Sizet2DArray& N_G_online, RealMatrix& H_raw_mom);
  void blue_raw_moments(const IntRealMatrixArrayMap& sum_G_offline,
			const IntRealSymMatrix2DArrayMap& sum_GG_offline,
			const Sizet2DArray& N_G_offline,
			const IntRealMatrixArrayMap& sum_G_online,
			const IntRealSymMatrix2DArrayMap& sum_GG_online,
			const Sizet2DArray& N_G_online, RealMatrix& H_raw_mom);

  void finalize_counts(const Sizet2DArray& N_G_actual,
		       const SizetArray& N_G_alloc);

  void print_group_solution(std::ostream& s, const MFSolutionData& soln);
  void print_group_solution_variables(std::ostream& s,
				      const MFSolutionData& soln);

private:

  //
  //- Heading: Helper functions
  //

  void ml_blue_online_pilot();
  void ml_blue_offline_pilot();
  void ml_blue_pilot_projection();

  void shared_covariance_iteration(IntRealMatrixArrayMap& sum_G,
				   IntRealSymMatrix2DArrayMap& sum_GG,
				   SizetArray& delta_N_G);
  void independent_covariance_iteration(IntRealMatrixArrayMap& sum_G,
					IntRealSymMatrix2DArrayMap& sum_GG,
					SizetArray& delta_N_G);

  void evaluate_pilot(RealMatrixArray& sum_G_pilot,
		      RealSymMatrix2DArray& sum_GG_pilot,
		      Sizet2DArray& N_shared_pilot, bool incr_cost);
  void evaluate_pilot(IntRealMatrixArrayMap& sum_G_pilot,
		      IntRealSymMatrix2DArrayMap& sum_GG_pilot,
		      Sizet2DArray& N_shared_pilot, bool incr_cost);

  void update_model_group_costs();
  void update_search_algorithm();

  void compute_allocations(MFSolutionData& soln, const Sizet2DArray& N_G_actual,
			   SizetArray& N_G_alloc, SizetArray& delta_N_G);
  void increment_allocations(const MFSolutionData& soln, SizetArray& N_G_alloc,
			     const SizetArray& delta_N_G);
  void increment_allocations(const MFSolutionData& soln, SizetArray& N_G_alloc,
			     const SizetArray& delta_N_G, size_t g);

  /// find group and model indices for HF reference variance
  void find_hf_sample_reference(const Sizet2DArray& N_G,  size_t& ref_group,
				size_t& ref_model_index);
  /// find group and model indices for HF reference variance
  void find_hf_sample_reference(const SizetArray& N_G,  size_t& ref_group,
				size_t& ref_model_index);
  /// find the best-conditioned group that contains the HF model
  size_t best_conditioned_hf_group();

  void enforce_bounds_linear_constraints(RealVector& soln_vars);
  void specify_parameter_bounds(RealVector& x_lb, RealVector& x_ub);
  void specify_initial_parameters(const MFSolutionData& soln, RealVector& x0,
				  const RealVector& x_lb,
				  const RealVector& x_ub);
  void specify_linear_constraints(RealVector& lin_ineq_lb,
				  RealVector& lin_ineq_ub,
				  RealVector& lin_eq_tgt,
				  RealMatrix& lin_ineq_coeffs,
				  RealMatrix& lin_eq_coeffs);
  void specify_nonlinear_constraints(RealVector& nln_ineq_lb,
				     RealVector& nln_ineq_ub,
				     RealVector& nln_eq_tgt);
  void enforce_augmented_linear_ineq_constraints(RealVector& soln_vars);

  void project_mc_estimator_variance(const RealSymMatrixArray& cov_GG_g,
				     size_t H_index,
				     const SizetArray& N_H_actual,
				     size_t delta_N_H, RealVector& proj_est_var,
				     SizetVector& proj_N_H);
  void project_mc_estimator_variance(const RealSymMatrixArray& cov_GG_g,
				     size_t H_index, Real N_H_actual,
				     Real delta_N_H, RealVector& proj_est_var);

  void accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG,
			    Sizet2DArray& num_G,
			    const IntIntResponse2DMap& batch_resp_map);
  void accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG,
			    Sizet2DArray& num_G, size_t group,
			    const IntResponseMap& resp_map);
  void accumulate_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG, Sizet2DArray& num_G,
			    const IntIntResponse2DMap& batch_resp_map);
  void accumulate_blue_sums(RealMatrix& sum_G, RealSymMatrixArray& sum_GG,
			    SizetArray& N_shared, size_t group,
			    const IntResponseMap& resp_map);

  void compute_GG_covariance(const RealMatrixArray& sum_G,
			     const RealSymMatrix2DArray& sum_GG,
			     const Sizet2DArray& N_G,
			     RealSymMatrix2DArray& cov_GG,
			     RealSymMatrix2DArray& cov_GG_inv,
			     const SizetArray& N_G_ref = SizetArray());
  void compute_GG_covariance(const RealMatrix& sum_G_g,
			     const RealSymMatrixArray& sum_GG_g,
			     const SizetArray& num_G_g,
			     RealSymMatrix2DArray& cov_GG,
			     RealSymMatrix2DArray& cov_GG_inv);

  void covariance_to_correlation_sq(const RealSymMatrixArray& cov_GG_g,
				    RealMatrix& rho2_LH);

  void analytic_initialization_from_mfmc(const RealMatrix& rho2_LH,
					 MFSolutionData& soln);
  void analytic_initialization_from_ensemble_cvmc(const RealMatrix& rho2_LH,
						  MFSolutionData& soln);

  void analytic_ratios_to_solution_variables(RealVector& avg_eval_ratios,
					     const SizetArray& ratios_to_groups,
					     MFSolutionData& soln);
  void ratios_target_to_solution_variables(const RealVector& avg_eval_ratios,
					   Real avg_hf_target,
					   const SizetArray& ratios_to_groups,
					   RealVector& soln_vars);

  /// return size of active subset of modelGroups defined by retainedModelGroups
  size_t num_active_groups() const;
  /// map group indexing from active (soln vars) to all (modelGroups),
  /// if needed due to retainedModelGroups
  size_t active_to_all_group(size_t active_index) const;
  /// map group indexing from all (modelGroups) to active (soln vars),
  /// if needed due to retainedModelGroups
  size_t all_to_active_group(size_t all_index)    const;
  /// prune modelGroups down to subset with best conditioned group covariances
  void prune_model_groups();

  void add_sub_matrix(Real coeff, const RealSymMatrix& sub_mat,
		      const UShortArray& subset, RealSymMatrix& mat);
  void add_sub_matvec(const RealSymMatrix& sub_mat, const RealMatrix& sub_mat2,
		      size_t sub_mat2_row, const UShortArray& subset,
		      RealVector& vec);

  void initialize_rsm2a(RealSymMatrix2DArray& rsm2a);
  void initialize_rsma(RealSymMatrixArray& rsma, bool init = true);
  void initialize_rva(RealVectorArray& rva, bool init = true);

  void enforce_nudge(RealVector& x);
  void enforce_diagonal_delta(RealSymMatrixArray& Psi);

  /*
  void compute_GG_statistics(RealMatrixArray& sum_G_pilot,
			     RealSymMatrix2DArray& sum_GG_pilot,
			     Sizet2DArray& N_shared_pilot,
			   //RealMatrixArray& var_G,
			     RealSymMatrix2DArray& cov_GG);
  void compute_G_variance(const RealMatrixArray& sum_G,
			  const RealSymMatrix2DArray& sum_GG,
			  const Sizet2DArray& num_G,
			  RealMatrixArray& var_G);

  void update_projected_group_samples(const MFSolutionData& soln,
				      const SizetArray& N_G_actual,
				      size_t& N_G_alloc,
				      SizetArray& delta_N_G,
				      Real& delta_equiv_hf);

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& avg_hf_target);
  */

  //
  //- Heading: Data
  //

  /// covariance matrices for each model QoI and each model grouping (the C_k
  /// matrix in ML BLUE), organized as a numGroups x numFunctions array of
  /// symmetic numModels_k x numModels_k covariance matrices
  RealSymMatrix2DArray covGG;
  /// in-place matrix inverses of covGG
  RealSymMatrix2DArray covGGinv;

  /// mode for pilot sampling: shared or independent
  short pilotGroupSampling;

  /// type of throttle for mitigating combinatorial growth in numGroups
  short groupThrottleType;
  /// group size threshold for groupThrottleType == GROUP_SIZE_THROTTLE
  unsigned short groupSizeThrottle;
  /// throttle the number of groups to this count based on ranking by
  /// condition number in group covariance
  size_t rCondBestThrottle;
  /// throttle the number of groups based on this tolerance for
  /// condition number in group covariance
  Real rCondTolThrottle;
  /// map from rcond to group number: pick the first rCondBestThrottle groups
  std::multimap<Real, size_t> groupCovCondMap;
  /// runtime group throttling due to covariance conditioning
  BitArray retainedModelGroups;

  /// counter for successful sample accumulations, per group and per QoI
  Sizet2DArray NGroupActual;
  /// counter for sample allocations, per group
  SizetArray   NGroupAlloc;

  /// final solution data for BLUE
  MFSolutionData blueSolnData;

  /// reference value for estimator variance: final HF variance over
  /// projected HF samples
  RealVector  projEstVarHF;
  /// reference value: projected HF samples
  SizetVector projNActualHF;
};


inline size_t NonDMultilevBLUESampling::num_active_groups() const
{
  if (retainedModelGroups.empty()) return modelGroups.size();
  else                             return retainedModelGroups.count();
}


inline size_t NonDMultilevBLUESampling::
active_to_all_group(size_t active_index) const
{
  if (retainedModelGroups.empty()) return active_index;
  else {
    size_t cntr = 0, g, num_groups = retainedModelGroups.size();
    for (g=0; g<num_groups; ++g)
      if (retainedModelGroups[g]) {
	if (cntr == active_index) return g;
	else                      ++cntr;
      }
  }
  return _NPOS;
}


inline size_t NonDMultilevBLUESampling::
all_to_active_group(size_t all_index) const
{
  if      ( retainedModelGroups.empty()   ) return all_index;
  else if (!retainedModelGroups[all_index]) return _NPOS;
  else {
    size_t g, cntr = 0;
    for (g=0; g<all_index; ++g)
      if (retainedModelGroups[g])
	++cntr;
    return cntr;
  }
}


inline Real NonDMultilevBLUESampling::estimator_accuracy_metric()
{ return blueSolnData.average_estimator_variance(); }


//inline Real NonDMultilevBLUESampling::estimator_cost_metric()
//{ return blueSolnData.equivalent_hf_allocation(); }


inline void NonDMultilevBLUESampling::update_model_group_costs()
{
  modelGroupCost.size(numGroups); // init to 0.

  size_t g, m, num_models;
  for (g=0; g<numGroups; ++g) {
    const UShortArray& models = modelGroups[g];  num_models = models.size();
    Real& group_cost_g = modelGroupCost[g];
    for (m=0; m<num_models; ++m)
      group_cost_g += sequenceCost[models[m]];
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "modelGroups:\n" << modelGroups
	 << "sequenceCost:\n" << sequenceCost
	 << "modelGroupCost:\n" << modelGroupCost;
}


inline void NonDMultilevBLUESampling::update_search_algorithm()
{
  size_t num_g = num_active_groups();  bool warn = false;
  if (num_g > DIRECT_DIMENSION_LIMIT) {
    // Note: any package config-related demotions to optSubProblemSolver have
    // already occurred in sub_optimizer_select(), so only need to be concerned
    // with dimension-related demotions
    switch (optSubProblemSolver) {
    case SUBMETHOD_DIRECT_NPSOL_OPTPP:
      optSubProblemSolver = SUBMETHOD_NPSOL_OPTPP;  warn = true;  break;
    case SUBMETHOD_DIRECT_NPSOL:
      optSubProblemSolver = SUBMETHOD_NPSOL;        warn = true;  break;
    case SUBMETHOD_DIRECT_OPTPP:
      optSubProblemSolver = SUBMETHOD_OPTPP;        warn = true;  break;
    //case SUBMETHOD_DIRECT:
    //case SUBMETHOD_EGO:
    //case SUBMETHOD_SBGO:
    //case SUBMETHOD_EA:
    }
  }
  if (warn)
    Cerr << "Warning: ML BLUE solver demoted to "
	 << submethod_enum_to_string(optSubProblemSolver)
	 << " due to solution dimension = " << num_g << std::endl;
}


inline void NonDMultilevBLUESampling::
increment_allocations(const MFSolutionData& soln, SizetArray& N_G_alloc,
		      const SizetArray& delta_N_G)
{
  // Alloc/Actual w.r.t. relaxation: both will track the under-relaxed target,
  // not soln.solution_variables(), continuing to differ only in timing
  // (allocate, then evaluate) and simulation faults
  // > Alternative: NGroupAlloc tracks soln.solution_variables() as being
  //   allocated by the numerical solve, whereas accumulations in NGroupActual
  //   will undershoot allocation due to under-relaxation.  Main downside is
  //   that the two counters diverge and can longer be interchanged.

  if (backfillFailures) { // don't use delta_N_G as it may include backfill
    SizetArray bf_delta_N_G;
    one_sided_delta(N_G_alloc, soln.solution_variables(),
		    bf_delta_N_G, relaxFactor); // match under-relaxation
    increment_samples(N_G_alloc, bf_delta_N_G);
  }
  else // delta_N_G is the allocation increment, including any under-relaxation
    increment_samples(N_G_alloc, delta_N_G);
}


inline void NonDMultilevBLUESampling::
increment_allocations(const MFSolutionData& soln, SizetArray& N_G_alloc,
		      const SizetArray& delta_N_G, size_t group)
{
  // only increments NGroupAlloc[group]

  if (backfillFailures) { // don't use delta_N_G as it may include backfill
    size_t& curr_g = N_G_alloc[group];
    Real diff_g = soln.solution_variable(group) - (Real)curr_g;
    if (diff_g > 0.)
      curr_g += (size_t)std::floor(relaxFactor * diff_g + .5);
  }
  else // delta_N_G is the allocation increment, including under-relaxation
    N_G_alloc[group] += delta_N_G[group];
}

/** Overload for 2D array: NGroupActual */
inline void NonDMultilevBLUESampling::
find_hf_sample_reference(const Sizet2DArray& N_G, size_t& ref_group,
			 size_t& ref_model_index)
{
  ref_group = ref_model_index = SZ_MAX;
  Real ref_samples = 0., group_samples;
  size_t g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    UShortArray& group_g = modelGroups[g];
    // find_index() is unnecessary assuming groups have ordered models
    if (group_g.back() == numApprox) { // HF model is present
      group_samples = average(N_G[g]);
      // Note: not protected from 1 sample -> Cov = nan from bessel corr
      if (group_samples > ref_samples) {
	ref_group   = g;  ref_model_index = group_g.size() - 1;
	ref_samples = group_samples;
      }
    }
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "HF sample reference located in group " << ref_group
	 << " at index " << ref_model_index << std::endl;
}


/** Overload for 1D array: NGroupAlloc */
inline void NonDMultilevBLUESampling::
find_hf_sample_reference(const SizetArray& N_G, size_t& ref_group,
			 size_t& ref_model_index)
{
  ref_group = ref_model_index = SZ_MAX;
  Real ref_samples = 0., group_samples;
  size_t g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    UShortArray& group_g = modelGroups[g];
    // find_index() is unnecessary assuming groups have ordered models
    if (group_g.back() == numApprox) { // HF model is present
      group_samples = N_G[g];
      // Note: not protected from 1 sample -> Cov = nan from bessel corr
      if (group_samples > ref_samples) {
	ref_group   = g;  ref_model_index = group_g.size() - 1;
	ref_samples = group_samples;
      }
    }
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "HF sample reference located in group " << ref_group
	 << " at index " << ref_model_index << std::endl;
}


inline size_t NonDMultilevBLUESampling::best_conditioned_hf_group()
{
  //size_t skip_front = numGroups - retainedModelGroups.count();
  //std::advance(rit, skip_front);

  std::multimap<Real, size_t>::reverse_iterator rit;  size_t group;
  for (rit = groupCovCondMap.rbegin(); rit!=groupCovCondMap.rend(); ++rit) {
    group = rit->second;
    if (modelGroups[group].back() == numApprox)
      return group;
  }
  /* This further distinguishes best conditioned group as retained or
     discarded, which is too specific -- not needed in all use cases
  for (rit = groupCovCondMap.rbegin(); rit!=groupCovCondMap.rend(); ++rit) {
    group = rit->second;
    if (modelGroups[group].back() == numApprox)
      return (retainedModelGroups[group]) ?
	_NPOS : // a HF group has already been retained
	group;  // return a discard to add (highest rcond)
  }
  */

  return _NPOS; // none available to retain (should not happen)
}


inline void NonDMultilevBLUESampling::enforce_nudge(RealVector& x)
{
  // Note: Using a numerical NUDGE is not essential since the group covariance
  // contributions overlap in Psi (single group drop-outs are not fatal).  On
  // the other hand, there doesn't seem to be much downside, so this provides
  // a degree of hardening for extreme drop-out cases.

  size_t i, len = x.length();
  Real lb = (maxFunctionEvals == SZ_MAX) ? RATIO_NUDGE :
    RATIO_NUDGE * std::sqrt(maxFunctionEvals); // hand-tuned heuristic
  for (i=0; i<len; ++i)
    if (x[i] < lb)
      x[i] = lb;
}


inline void NonDMultilevBLUESampling::
enforce_diagonal_delta(RealSymMatrixArray& Psi)
{
  // This has not been as effective as enforce_nudge(), and is not as directly
  // interpretable as lower-bounding of m_k

  size_t i, j, num_rsm = Psi.size();  int n;  Real delta = RATIO_NUDGE;
  for (i=0; i<num_rsm; ++i) {
    RealSymMatrix& Psi_i = Psi[i];  n = Psi_i.numRows();
    for (j=0; j<n; ++j)
      Psi_i(n,n) += delta;
  }
}


inline void NonDMultilevBLUESampling::
covariance_to_correlation_sq(const RealSymMatrixArray& cov_GG_g,
			     RealMatrix& rho2_LH)
{
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;  Real var_H_q, var_L_q, cov_LH_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& cov_GG_gq = cov_GG_g[qoi];
    var_H_q = cov_GG_gq(numApprox, numApprox);
    for (approx=0; approx<numApprox; ++approx) {
      var_L_q  = cov_GG_gq(approx,    approx);
      cov_LH_q = cov_GG_gq(numApprox, approx);
      rho2_LH(qoi, approx) = cov_LH_q / var_L_q * cov_LH_q / var_H_q;
    }
  }
}


/* Think this is unnecessary: perform subsetting directly
inline void NonDMultilevBLUESampling::form_R(size_t k, RealMatrix& R_k)
{
  // R_k \in Real |G_k| x \ell

  const UShortArray& models_k = modelGroups[k];
  size_t i, model, num_r = models_k.size(), // "|G_k|"
    num_c = numApprox+1;// "\ell"
  if  (R_k.numRows() == num_r && R_k.numCols() == num_c) R_k = 0.;
  else R_k.shape(num_r, num_c); // init to 0

  for (i=0; i<num_r; ++i)
    R_k(i, models_k[i]) = 1.; // R_k is active row subset of identity

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "R matrix for group " << i << ":\n" << R_k << std::endl;
}
*/


inline void NonDMultilevBLUESampling::
add_sub_matrix(Real coeff, const RealSymMatrix& sub_mat,
	       const UShortArray& subset, RealSymMatrix& mat)
{
  size_t r, c, model, num_sub = subset.size();
  for (r=0; r<num_sub; ++r)
    for (c=0; c<=r; ++c)
      mat(subset[r], subset[c]) += coeff * sub_mat(r,c);

  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "add_sub_matrix() aggregate =\n" << mat << std::endl;
}


inline void NonDMultilevBLUESampling::
add_sub_matvec(const RealSymMatrix& sub_mat, const RealMatrix& sub_mat2,
	       size_t sub_mat2_row, const UShortArray& subset, RealVector& vec)
{
  size_t r, c, model, num_sub = subset.size();  Real sub_matvec;
  for (r=0; r<num_sub; ++r) {
    sub_matvec = 0.;
    for (c=0; c<num_sub; ++c)
      sub_matvec += sub_mat(r,c) * sub_mat2(sub_mat2_row,c);
    vec[subset[r]] += sub_matvec;
  }

  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "add_sub_matvec() aggregate =\n" << vec << std::endl;
}


inline void NonDMultilevBLUESampling::
initialize_rsm2a(RealSymMatrix2DArray& rsm2a)
{
  // size the first two dimensions, but defer RealSymMatrix::shape()

  size_t num_groups = modelGroups.size();
  if (rsm2a.size() != num_groups) {
    rsm2a.resize(num_groups);
    for (size_t g=0; g<num_groups; ++g)
      rsm2a[g].resize(numFunctions);
  }
}


inline void NonDMultilevBLUESampling::
initialize_rsma(RealSymMatrixArray& rsma, bool init)
{
  if (rsma.size() != numFunctions) {
    rsma.resize(numFunctions);
    size_t qoi, all_models = numApprox + 1;
    if (init)
      for (qoi=0; qoi<numFunctions; ++qoi)
	rsma[qoi].shape(all_models); // init to 0
    else
      for (qoi=0; qoi<numFunctions; ++qoi)
	rsma[qoi].shapeUninitialized(all_models);
  }
  else if (init)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      rsma[qoi].putScalar(0.);
}


inline void NonDMultilevBLUESampling::
initialize_rva(RealVectorArray& rva, bool init)
{
  if (rva.size() != numFunctions) {
    rva.resize(numFunctions);
    size_t qoi, all_models = numApprox + 1;
    if (init)
      for (qoi=0; qoi<numFunctions; ++qoi)
	rva[qoi].size(all_models); // init to 0
    else
      for (qoi=0; qoi<numFunctions; ++qoi)
	rva[qoi].sizeUninitialized(all_models);
  }
  else if (init)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      rva[qoi].putScalar(0.);
}


inline Real NonDMultilevBLUESampling::linear_group_cost(const RealVector& cdv)
{
  if (retainedModelGroups.empty())
    return NonDNonHierarchSampling::linear_group_cost(cdv);

  // linear objective: N + Sum(w_i N_i) / w
  Real lin_obj = 0.;  size_t i, cntr = 0;
  for (i=0; i<numGroups; ++i)
    if (retainedModelGroups[i])
      lin_obj += modelGroupCost[i] * cdv[cntr++]; // Sum(w_i N_i)
  lin_obj /= sequenceCost[numApprox];// N + Sum / w
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear group cost = " << lin_obj << std::endl;
  return lin_obj;
}


inline void NonDMultilevBLUESampling::
linear_group_cost_gradient(const RealVector& cdv, RealVector& grad_c)
{
  if (retainedModelGroups.empty()) {
    NonDNonHierarchSampling::linear_group_cost_gradient(cdv, grad_c);
    return;
  }

  Real cost_H = sequenceCost[numApprox];  size_t i, cntr = 0;
  for (i=0; i<numGroups; ++i)
    if (retainedModelGroups[i])
      grad_c[cntr++] = modelGroupCost[i] / cost_H;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear group cost gradient:\n" << grad_c << std::endl;
}


inline void NonDMultilevBLUESampling::
compute_C_inverse(const RealSymMatrix2DArray& cov_GG,
		  RealSymMatrix2DArray& cov_GG_inv)
{
  // cov matrices are sized according to group member size
  initialize_rsm2a(cov_GG_inv);
  bool rcond_throttle = (groupThrottleType == RCOND_TOLERANCE_THROTTLE ||
			 groupThrottleType == RCOND_BEST_COUNT_THROTTLE);
  if (rcond_throttle) groupCovCondMap.clear();

  size_t q, g, num_groups = modelGroups.size();
  RealVector rcond(numFunctions);
  for (g=0; g<num_groups; ++g) {
    const RealSymMatrixArray& cov_GG_g = cov_GG[g];
    RealSymMatrixArray&   cov_GG_inv_g = cov_GG_inv[g];
    for (q=0; q<numFunctions; ++q)
      compute_C_inverse(cov_GG_g[q], cov_GG_inv_g[q], g, q, rcond[q]);
    if (rcond_throttle)
      groupCovCondMap.insert(std::pair<Real,size_t>(average(rcond), g));
  }
}


/** This version used during numerical solution (1D vector of real
    design variables for group samples. */
inline void NonDMultilevBLUESampling::
compute_Psi(const RealSymMatrix2DArray& cov_GG_inv, const RealVector& cdv,
	    RealSymMatrixArray& Psi)
{
  // Psi accumulated across groups and sized according to full model ensemble
  initialize_rsma(Psi);

  size_t qoi, g, num_groups = modelGroups.size(), v_index = 0;
  bool no_retain_throttle = retainedModelGroups.empty();
  for (g=0; g<num_groups; ++g) {
    // In the context of numerical optimization, we should not need to protect
    // cov_GG_inv against n_g = 0.
    if (no_retain_throttle || retainedModelGroups[g]) {
      Real v_i = cdv[v_index++];
      if (v_i > 0.) {
	const UShortArray&            models_g = modelGroups[g];
	const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
	for (qoi=0; qoi<numFunctions; ++qoi) {
	  const RealSymMatrix& cov_GG_inv_gq = cov_GG_inv_g[qoi];
	  if (!cov_GG_inv_gq.empty())
	    add_sub_matrix(v_i, cov_GG_inv_gq, models_g, Psi[qoi]); // *** can become indefinite here when N_g --> 0, which depends on online/offline pilot integration strategy
	}
      }
    }
  }
  // Add \delta I (Schaden & Ullmann, 2020)
  //enforce_diagonal_delta(Psi);
}


inline void NonDMultilevBLUESampling::
compute_Psi(const RealSymMatrix2DArray& cov_GG_inv, const Sizet2DArray& N_G,
	    RealSymMatrixArray& Psi)
{
  // Psi accumulated across groups and sized according to full model ensemble
  initialize_rsma(Psi);

  size_t qoi, g, num_groups = modelGroups.size();
  bool no_retain_throttle = retainedModelGroups.empty();
  for (g=0; g<num_groups; ++g) {
    if (no_retain_throttle || retainedModelGroups[g]) {
      const SizetArray&                  N_g =         N_G[g];
      const UShortArray&            models_g = modelGroups[g];
      const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
      for (qoi=0; qoi<numFunctions; ++qoi) {
	const RealSymMatrix& cov_GG_inv_gq = cov_GG_inv_g[qoi];
	size_t N_gq = N_g[qoi];
	if (N_gq && !cov_GG_inv_gq.empty())
	  add_sub_matrix((Real)N_gq, cov_GG_inv_gq, models_g, Psi[qoi]); // *** can become indefinite here when N_gq --> 0, which depends on online/offline pilot integration strategy
      }
    }
  }
  // Add \delta I (Schaden & Ullmann, 2020)
  //enforce_diagonal_delta(Psi);
}


/** This version used during numerical solution (1D vector of real
    design variables for group samples.
inline void NonDMultilevBLUESampling::invert_Psi(RealSymMatrixArray& Psi)
{
  // Psi-inverse is used for computing both estimator variance (during numerical
  // solve) and y-hat / mu-hat (after solve), so invert now without a RHS so
  // Psi-inverse can be used in multiple places without additional tracking
  RealSpdSolver spd_solver; // reuse since setMatrix resets internal state
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    spd_solver.setMatrix(Teuchos::rcp(&Psi[qoi], false));
    // Note: equilibration should not be used outside of the solve() context,
    // as the resulting inverse would be for the equilibrated matrix.  See
    // discussion above in compute_C_inverse().
    int code = spd_solver.invert(); // in place
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in NonDMultilevBLUE::compute_Psi_inverse()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "In compute_Psi_inverse(), Psi_inv:\n" << Psi << std::endl;
}


// This version used during numerical solution (1D vector of real design vars).
inline void NonDMultilevBLUESampling::
compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
		    const RealVector& N_G_1D, RealSymMatrixArray& Psi_inv{
  compute_Psi(cov_GG_inv, N_G_1D, Psi_inv);
  invert_Psi(Psi_inv);
}


// This version used during final results (2D array of actual samples).
inline void NonDMultilevBLUESampling::
compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
		    const Sizet2DArray& N_G_2D, RealSymMatrixArray& Psi_inv)
{
  compute_Psi(cov_GG_inv, N_G_2D, Psi_inv);
  invert_Psi(Psi_inv);
}
*/


inline void NonDMultilevBLUESampling::
compute_y(const RealSymMatrix2DArray& cov_GG_inv, const RealMatrixArray& sum_G,
	  RealVectorArray& y)
{
  // y accumulated across groups and sized according to full model ensemble
  initialize_rva(y);

  size_t q, g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    //form_R(g, R_g);
    const UShortArray&            models_g = modelGroups[g];
    const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
    const RealMatrix&              sum_G_g =       sum_G[g];
    for (q=0; q<numFunctions; ++q) {
      //copy_row_vector(sum_G_g, qoi, sum_G_gq);
      //add_sub_matvec(C_gq_inv, sum_G_gq, models_g, y[q]);
      // pass row for sum_G_g to avoid copy:
      const RealSymMatrix& cov_GG_inv_gq = cov_GG_inv_g[q];
      if (!cov_GG_inv_gq.empty())
	add_sub_matvec(cov_GG_inv_gq, sum_G_g, q, models_g, y[q]);
    }
  }
}


inline Real NonDMultilevBLUESampling::
average_estimator_variance(const RealVector& cd_vars)
{
  // redefinition of virtual fn used by NonDNonHierarch::log_average_estvar(),
  // which is used by optimizer objective/constraint callbacks

  RealVector estvar;
  estimator_variance(cd_vars, estvar);
  return average(estvar);
}


inline void NonDMultilevBLUESampling::apply_mc_reference(RealVector& mc_targets)
{
  // derived implementation (varH is not used by ML BLUE)

  // used by NonDNonHierarchSampling::finite_solution_bounds() for accuracy
  // constrained cases, which are disallowed for offline pilot modes

  // mirrors find_hf_sample_reference() logic in print_variance_reduction()
  size_t ref_group, ref_model_index;
  switch (pilotMgmtMode) {
  case OFFLINE_PILOT:  case OFFLINE_PILOT_PROJECTION:
    // should not happen (estVarIter0 not available)
    ref_group = numGroups - 1;  ref_model_index = numApprox;             break;
  default:
    find_hf_sample_reference(NGroupActual, ref_group, ref_model_index);  break;
  }

  const RealSymMatrixArray& cov_GG_g = covGG[ref_group];
  if (mc_targets.length() != numFunctions)
    mc_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    mc_targets[qoi] = cov_GG_g[qoi](ref_model_index,ref_model_index)
                    / ( convergenceTol * estVarIter0[qoi] );
}

} // namespace Dakota

#endif
