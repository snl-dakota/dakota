/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_NONHIERARCH_SAMPLING_H
#define NOND_NONHIERARCH_SAMPLING_H

#include "NonDEnsembleSampling.hpp"
#include "MFSolutionData.hpp"


namespace Dakota {

// needs to be greater than FDSS to avoid potential FPE from N > N_i
// (especially for bounds-respecting central diff at N = pilot). Careful
// control of NPSOL is needed: {Central,Fwd} FDSS are now assigned.
#define RATIO_NUDGE 1.e-4


/// Base class for non-hierarchical ensemble-based Monte Carlo sampling.

/** Derived classes include MFMC, ACV, and GenACV. */

class NonDNonHierarchSampling: public NonDEnsembleSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDNonHierarchSampling(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, 
			  std::shared_ptr<Model> model);
  /// destructor
  ~NonDNonHierarchSampling() override;

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run() override;
  //void core_run() override;
  //void post_run(std::ostream& s) override;
  //void print_results(std::ostream& s,
  //                   short results_state = FINAL_RESULTS) override;
  //void print_variance_reduction(std::ostream& s) const override;

  /// return name of active optimizer method
  unsigned short uses_method() const override;
  /// perform a numerical solver method switch due to a detected conflict
  void method_recourse(unsigned short method_name) override;

  Real estimator_accuracy_metric() const override;
  //Real estimator_cost_metric() const override;

  //
  //- Heading: New virtual functions
  //

  /// return the MFSolutionData instance containing the final results
  virtual const MFSolutionData& final_solution_data() const = 0;

  /// helper function that supports optimization APIs passing design variables
  virtual void estimator_variances(const RealVector& cd_vars,
				   RealVector& est_var);
  /// helper function that supports optimization APIs passing design variables
  virtual void estimator_variance_gradients(const RealVector& cd_vars,
					    RealMatrix& ev_grads);
  /// helper function that supports optimization APIs passing design variables
  virtual void estimator_variances_and_gradients(const RealVector& cd_vars,
						 RealVector& est_var,
						 RealMatrix& ev_grads);

  /// compute estimator variance ratios from HF samples and oversample ratios
  virtual void estimator_variance_ratios(const RealVector& cd_vars,
					 RealVector& estvar_ratios);
  /// helper function that supports optimization APIs passing design variables
  virtual void estimator_variance_ratio_gradients(const RealVector& cd_vars,
						  RealMatrix& evr_grads);
  /// helper function that supports optimization APIs passing design variables
  virtual void estimator_variance_ratios_and_gradients(
    const RealVector& cd_vars, RealVector& estvar_ratios,
    RealMatrix& evr_grads);

  /// within ensemble_numerical_solution(), define the number of
  /// solution variables and constraints
  virtual void numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
					 size_t& num_nln_con);
  /// within ensemble_numerical_solution(), define initial values, coefficients,
  /// bounds, and targets for solution variables and constraints
  virtual void numerical_solution_bounds_constraints(const MFSolutionData& soln,
    RealVector& x0, RealVector& x_lb, RealVector& x_ub,
    RealVector& lin_ineq_lb, RealVector& lin_ineq_ub, RealVector& lin_eq_tgt,
    RealVector& nln_ineq_lb, RealVector& nln_ineq_ub, RealVector& nln_eq_tgt,
    RealMatrix& lin_ineq_coeffs, RealMatrix& lin_eq_coeffs);

  /// portion of finite_solution_bounds() specific to derived class
  /// implementations
  virtual void derived_finite_solution_bounds(const RealVector& x0,
					      RealVector& x_lb,
					      RealVector& x_ub, Real budget);

  /// apply convergenceTol to estVarIter0 to form an estimate of required
  /// high-fidelity MC samples
  virtual void apply_mc_reference(RealVector& mc_targets);

  /// augment linear inequality constraints as required by derived algorithm
  virtual void augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
					       RealVector& lin_ineq_lb,
					       RealVector& lin_ineq_ub);
  /// return quadratic constraint violation for augmented linear
  /// inequality constraints
  virtual Real augmented_linear_ineq_violations(const RealVector& cd_vars,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_lb,
    const RealVector& lin_ineq_ub);
  /// enforce the augmented linear inequality constraints, usually to prevent
  /// numerical exceptions in the computation of estimator variance
  virtual void enforce_augmented_linear_ineq_constraints(RealVector& cd_vars);

  /// post-process minimization results to update MFSolutionData
  virtual void minimizer_results_to_solution_data(const RealVector& cv_star,
						  const RealVector& fn_star,
						  MFSolutionData& soln);

  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  virtual Real linear_model_cost(const RealVector& N_vec);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  virtual void linear_model_cost_gradient(const RealVector& N_vec,
					  RealVector& grad_c);
  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  virtual Real linear_group_cost(const RealVector& N_vec);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  virtual void linear_group_cost_gradient(const RealVector& N_vec,
					  RealVector& grad_c);
  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  virtual Real nonlinear_model_cost(const RealVector& r_and_N);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  virtual void nonlinear_model_cost_gradient(const RealVector& r_and_N,
					     RealVector& grad_c);

  virtual size_t num_approximations() const;

  //
  //- Heading: member functions
  //

  void shared_increment(String prepend);
  void shared_increment(String prepend, const UShortArray& approx_set);
  void shared_approx_increment(String prepend);

  bool approx_increment(String prepend, const SizetArray& approx_sequence,
			size_t start, size_t end);
  bool approx_increment(String prepend, const SizetArray& approx_sequence,
			size_t start, size_t end,
			const UShortArray& approx_set);
  bool approx_increment(String prepend, unsigned short root,
			const UShortSet& reverse_dag);

  void group_increments(SizetArray& delta_N_G, String prepend,
			bool reverse_order = false);

  void ensemble_sample_increment(const String& prepend, size_t step,
				 bool new_samples = true);
  void ensemble_sample_batch(const String& prepend, size_t step,
			     bool new_samples = true);

  size_t group_approx_increment(const RealVector& soln_vars,
				const UShortArray& approx_set,
				const Sizet2DArray& N_L_actual,
				SizetArray& N_L_alloc,
				const UShortArray& model_group);
  //size_t dag_approx_increment(const RealVector& soln_vars,
  // 			      const UShortArray& approx_set,
  // 			      const Sizet2DArray& N_L_actual,
  // 			      SizetArray& N_L_alloc, unsigned short root,
  // 			      const UShortSet& reverse_dag_set);

  /// compute estimator variances from ratios and MC reference variance
  void estimator_variances_from_ratios(const RealVector& cd_vars,
				       const RealVector& estvar_ratios,
				       RealVector& est_var);
  /// alternate API for updating both variance ratios and associated variances
  void estimator_variances_and_ratios(const RealVector& cd_vars,
				      RealVector& estvar_ratios,
				      RealVector& est_var);
  /// alternate API for updating both variance ratios and associated variances
  void estimator_variances_and_ratios(const RealVector& cd_vars,
				      MFSolutionData& soln);
  /// compute estimator variance from estimator variance ratios
  void estvar_ratios_to_estvar(const RealVector& estvar_ratios,
			       const RealVector& var_H, const SizetArray& N_H,
			       RealVector& estvar);
  /// compute estimator variance from estimator variance ratios
  void estvar_ratios_to_estvar(const RealVector& estvar_ratios,
			       const RealVector& var_H, Real N_H,
			       RealVector& estvar);
  /// compute estimator variance from estimator variance ratios
  void estvar_ratios_to_estvar(const RealVector& var_H, const SizetArray& N_H,
			       MFSolutionData& soln);

  /// evaluate the optimization metric formed from a reduction of
  /// estimator variances
  Real estimator_variance_metric(const RealVector& cd_vars);
  /// evaluate the gradient of the optimization metric formed from a
  /// reduction of estimator variances
  void estimator_variance_metric_gradient(const RealVector& cd_vars,
					  RealVector& evm_grad);
  /// evaluate the gradient of the optimization metric formed from a
  /// reduction of estimator variances
  void estimator_variance_metric_and_gradient(const RealVector& cd_vars,
					      Real& ev_metric,
					      RealVector& evm_grad);

  /// helper function for assigning dummy estimator variances when no
  /// numerical solve is performed
  void no_solve_variances(MFSolutionData& soln);
  /// helper function for competing initial guesses from MFMC and ensemble-CVMC
  void competed_initial_guesses(MFSolutionData& mf_soln,
				MFSolutionData& cv_soln,
				MFSolutionData& selected_soln);

  /// export allSamples for all Models included in ensemble batch evaluation
  void export_sample_sets(const String& prepend, size_t step);
  /// export allSamples to a tagged tabular file
  void export_all_samples(const String& root_prepend, const Model& model,
			  size_t iter, size_t step);

  /// When looping through a minimizer sequence/competition, this
  /// function enables per-minimizer updates to the parameter bounds,
  /// e.g. for providing a bounded domain for methods that require it,
  /// while removing it for those that don't
  void finite_solution_bounds(const RealVector& x0, RealVector& x_lb,
			      RealVector& x_ub);

  // manage response mode and active model key from {group,form,lev} triplet.
  // seq_type defines the active dimension for a model sequence.
  //void configure_indices(size_t group,size_t form,size_t lev,short seq_type);

  void assign_active_key();

  //void initialize_sums(IntRealMatrixMap& sum_Q);
  void initialize_sums(IntRealMatrixMap& sum_L_baseline,
		       IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LH,
		       RealVector& sum_HH);
  //void initialize_counts(Sizet2DArray& num_Q);
  void initialize_counts(Sizet2DArray& num_L_baseline, SizetArray& num_H,
			 Sizet2DArray& num_LH);

  void initialize_group_sums(RealMatrixArray& sum_G,
			     RealSymMatrix2DArray& sum_GG);
  void initialize_group_sums(RealMatrixArray& sum_G);
  void initialize_group_sums(IntRealMatrixArrayMap& sum_G,
			     IntRealSymMatrix2DArrayMap& sum_GG);
  void initialize_group_sums(IntRealMatrixArrayMap& sum_G);
  void initialize_group_counts(Sizet2DArray& num_G);

  void finalize_counts(const Sizet2DArray& N_L_actual,
		       const SizetArray&   N_L_alloc);

  Real compute_equivalent_cost(Real hf_target,
			       const RealVector& avg_eval_ratios,
			       const RealVector& cost);
  Real compute_equivalent_cost(Real hf_target,
			       const RealVector& avg_eval_ratios,
			       const RealVector& cost,
			       const UShortArray& approx_set);
  //Real compute_equivalent_cost(const RealVector& sample_counts,
  //			         const RealVector& cost);
  //Real compute_equivalent_cost(const RealVector& sample_counts,
  //			         const RealVector& cost,
  //                             const UShortArray& approx_set);

  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 size_t index, Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 size_t start, size_t end,Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 const SizetArray& approx_sequence,
				 size_t start, size_t end,Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 const SizetArray& approx_sequence,
				 size_t start, size_t end,
				 const UShortArray& approx_set,
				 Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 unsigned short root,
				 const UShortSet& reverse_dag,
				 Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 unsigned short root,
				 const UShortArray& approx_set,
				 Real& equiv_hf_evals);
  void increment_equivalent_cost(const SizetArray& delta_N_g,
				 const RealVector& group_cost, Real hf_cost,
				 Real& equiv_hf_evals);

  void increment_sample_range(SizetArray& N_L, size_t incr,
			      const SizetArray& approx_sequence,
			      size_t start, size_t end);
  void increment_sample_range(SizetArray& N_L, size_t incr,
			      const UShortArray& approx_set);
  void increment_sample_range(SizetArray& N_L, size_t incr,
			      const SizetArray& approx_sequence,
			      size_t start, size_t end,
			      const UShortArray& approx_set);
  void increment_sample_range(SizetArray& N_L, size_t incr, unsigned short root,
			      const UShortSet& reverse_dag);

  void accumulate_group_sums(IntRealMatrixArrayMap& sum_G, Sizet2DArray& num_G,
			     const IntIntResponse2DMap& batch_resp_map);
  void accumulate_group_sums(IntRealMatrixArrayMap& sum_G, Sizet2DArray& num_G,
			     size_t group, const IntResponseMap& resp_map);

  void ensemble_active_set(const UShortArray& model_set);

  /// define model form and resolution level indices
  void hf_indices(size_t& hf_form_index, size_t& hf_lev_index) const;

  void compute_variance(Real sum_Q, Real sum_QQ, size_t num_Q, Real& var_Q);
  void compute_variance(const RealVector& sum_Q, const RealVector& sum_QQ,
			const SizetArray& num_Q,       RealVector& var_Q);

  void compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1,
			   Real sum_Q1Q2, Real sum_Q2Q2, size_t N_shared,
			   Real& var_Q1,  Real& var_Q2,  Real& rho2_Q1Q2);
  void compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2,
			  size_t N_shared, Real& cov_Q1Q2);

  void covariance_to_correlation_sq(const RealMatrix& cov_LH,
				    const RealSymMatrixArray& cov_LL,
				    const RealVector& var_H,
				    RealMatrix& rho2_LH);

  void mfmc_analytic_solution(const UShortArray& approx_set,
			      const RealMatrix& rho2_LH, const RealVector& cost,
			      RealVector& avg_eval_ratios,
			      bool lower_bounded_r = true,
			      bool     monotonic_r = false);
  void mfmc_reordered_analytic_solution(const UShortArray& approx_set,
					const RealMatrix& rho2_LH,
					const RealVector& cost,
					SizetArray& corr_approx_sequence,
					RealVector& avg_eval_ratios,
					bool lower_bounded_r = true,
					bool     monotonic_r = false);

  void cvmc_ensemble_solutions(const RealMatrix& rho2_LH,
			       const RealVector& cost,
			       RealVector& avg_eval_ratios,
			       bool lower_bounded_r = true);

  void pick_mfmc_cvmc_solution(const MFSolutionData& mf_soln, //size_t mf_samp,
			       const MFSolutionData& cv_soln, //size_t cv_samp,
			       MFSolutionData& soln);//, size_t& num_samp);

  void ensemble_numerical_solution(MFSolutionData& soln);
  void configure_minimizers(RealVector& x0, RealVector& x_lb, RealVector& x_ub,
			    const RealVector& lin_ineq_lb,
			    const RealVector& lin_ineq_ub,
			    const RealVector& lin_eq_tgt,
			    const RealVector& nln_ineq_lb,
			    const RealVector& nln_ineq_ub,
			    const RealVector& nln_eq_tgt,
			    const RealMatrix& lin_ineq_coeffs,
			    const RealMatrix& lin_eq_coeffs);
  void run_minimizers(MFSolutionData& soln);
  void process_model_allocations(MFSolutionData& soln, size_t& num_samples);

  void root_reverse_dag_to_group(unsigned short root, const UShortSet& rev_dag,
				 UShortArray& model_group);
  void group_to_root_reverse_dag(const UShortArray& model_group,
				 unsigned short& root, UShortSet& rev_dag);

  void overlay_group_sums(const IntRealMatrixArrayMap& sum_G,
			  const Sizet2DArray& N_G_actual,
			  IntRealMatrixMap& sum_L_shared,
			  Sizet2DArray& N_L_actual_shared,
			  IntRealMatrixMap& sum_L_refined,
			  Sizet2DArray& N_L_actual_refined);

  //bool mfmc_model_grouping(const UShortArray& model_group) const;
  //bool cvmc_model_grouping(const UShortArray& model_group) const;
  void mfmc_model_group(size_t last_index, UShortArray& model_group) const;
  void mfmc_model_group(size_t last_index, const SizetArray& approx_sequence,
			UShortArray& model_group) const;
  void singleton_model_group(size_t index, UShortArray& model_group) const;
  void singleton_model_group(size_t index, const SizetArray& approx_sequence,
			     UShortArray& model_group) const;
  void cvmc_model_group(size_t index, UShortArray& model_group) const;
  void cvmc_model_group(size_t index, const SizetArray& approx_sequence,
			UShortArray& model_group) const;
  void mlmc_model_group(size_t index, UShortArray& model_group) const;
  void mlmc_model_group(size_t index, const SizetArray& approx_sequence,
			UShortArray& model_group) const;

  void update_model_group_costs();

  void print_group(std::ostream& s, size_t g) const;

  Real allocate_budget(const RealVector& avg_eval_ratios,
		       const RealVector& cost, Real budget);
  Real allocate_budget(const RealVector& avg_eval_ratios,
		       const RealVector& cost);
  Real allocate_budget(const UShortArray& approx_set,
		       const RealVector& avg_eval_ratios,
		       const RealVector& cost, Real budget);
  Real allocate_budget(const UShortArray& approx_set,
		       const RealVector& avg_eval_ratios,
		       const RealVector& cost);

  Real update_hf_target(const RealVector& estvar_ratios, size_t metric_index,
			const RealVector& var_H, const RealVector& estvar_0);
  Real update_hf_target(const RealVector& estvar_ratios, size_t metric_index,
			const RealVector& var_H);
  Real update_hf_target(const RealVector& estvar, size_t metric_index,
			const SizetArray& N_H, const RealVector& estvar_0);
  Real update_hf_target(const RealVector& estvar, size_t metric_index,
			const SizetArray& N_H);
  Real update_hf_target(const RealVector& estvar, size_t metric_index,
			size_t N_H, const RealVector& estvar_0);
  Real update_hf_target(const RealVector& estvar, size_t metric_index,
			size_t N_H);

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& hf_target,
		       Real budget, Real offline_N_lwr = 1.);
  void scale_to_budget_with_pilot(RealVector& avg_eval_ratios,
				  const RealVector& cost, Real avg_N_H,
				  Real budget);

  void cache_mc_reference();

  void enforce_bounds(RealVector& x0, const RealVector& x_lb,
		      const RealVector& x_ub);

  /// helper function that supports virtual print_variance_reduction(s)
  void print_estimator_performance(std::ostream& s,
				   const MFSolutionData& soln) const;

  void r_and_N_to_N_vec(const RealVector& avg_eval_ratios, Real N_H,
			RealVector& N_vec);
  void r_and_N_to_design_vars(const RealVector& avg_eval_ratios, Real N_H,
			      RealVector& cd_vars);
  void solution_to_design_vars(const MFSolutionData& soln, RealVector& cd_vars);

  /// define approx_sequence in increasing metric order
  bool ordered_approx_sequence(const RealVector& metric,
			       SizetArray& approx_sequence,
			       bool descending_keys = false);
  /// define approx_sequence in increasing metric order
  bool ordered_approx_sequence(const RealMatrix& metric,
			       SizetArray& approx_sequence,
			       bool descending_keys = false);
  /// determine whether metric is in increasing order by columns for all rows
  bool ordered_approx_sequence(const RealMatrix& metric);
  /// determine whether metric is in increasing order by active columns for
  /// all rows
  bool ordered_approx_sequence(const RealMatrix& metric,
			       const UShortArray& approx_set);

  void apply_controls(const IntRealVectorMap& sum_H_baseline,
		      const SizetArray& N_baseline,
		      const IntRealMatrixMap& sum_L_shared,
		      const Sizet2DArray& N_L_shared,
		      const IntRealMatrixMap& sum_L_refined,
		      const Sizet2DArray& N_L_refined,
		      const RealVector2DArray& beta);

  void apply_control(Real sum_L_shared, size_t num_shared, Real sum_L_refined,
		     size_t num_refined, Real beta, Real& H_raw_mom);

  /// identify if there are activeSet requests for model i
  bool active_set_for_model(size_t i);

  /// promote scalar to 1D array
  void inflate(size_t N_0D, SizetArray& N_1D);
  /// promote scalar to portion of 1D array
  void inflate(size_t N_0D, SizetArray& N_1D, const UShortArray& approx_set);
  /// promote 1D array to 2D array
  void inflate(const SizetArray& N_1D, Sizet2DArray& N_2D);
  /// promote 1D array to active portion of 2D array
  void inflate(const SizetArray& N_1D, Sizet2DArray& N_2D,
	       const UShortArray& approx_set);
  /// promote vector of averaged values to full matrix
  void inflate(const RealVector& avg_eval_ratios, RealMatrix& eval_ratios);
  /// promote scalar to column vector
  void inflate(Real r_i, size_t num_rows, Real* eval_ratios_col);
  /// promote active vector subset to full vector based on mask
  void inflate(const RealVector& vec, const BitArray& mask,
	       RealVector& inflated_vec);
  // promote active subset to full matrix based on mask
  //void inflate(const RealMatrix& mat, const BitArray& mask,
  //	         RealMatrix& inflated_mat);
  // promote active subset to full symmetric matrix based on mask
  //void inflate(const RealSymMatrix& mat, const BitArray& mask,
  //	         RealSymMatrix& inflated_mat);
  /// demote full vector to active subset based on mask
  void deflate(const RealVector& vec, const BitArray& mask,
	       RealVector& deflated_vec);
  /// demote full vector to active subset based on mask
  void deflate(const SizetArray& vec, const BitArray& mask,
	       RealVector& deflated_vec);

  /// compute a penalty merit function after an optimization solve
  Real nh_penalty_merit(const RealVector& c_vars, const RealVector& fn_vals);
  /// compute a penalty merit function after a MFSolutionData instance
  Real nh_penalty_merit(const MFSolutionData& soln);

  //
  //- Heading: Data
  //

  /// the minimizer(s) used to optimize the estimator variance over the number
  /// of truth model samples and approximation eval_ratios.  Minimizers are
  /// arranged in a sequence (first dimension) where each step in the sequence
  /// may have multiple competitors (second dimension)
  Iterator2DPtrArray varianceMinimizers;
  /// active indices for numerical solutions: varianceMinimizers[first][second]
  SizetSizetPair varMinIndices;

  /// variance minimization algorithm selection: SUBMETHOD_MFMC or
  /// SUBMETHOD_ACV_{IS,MF,RD}
  unsigned short mlmfSubMethod;

  /// number of model groupings (pairings, pyramid levels,
  /// tensor-product enumerations) used for sample allocations
  size_t numGroups;

  /// the set of model groupings used by the estimator, e.g. ML BLUE
  UShort2DArray modelGroups;
  /// aggregate cost of a sample for each of a set of model groupings
  /// (i.e. modelGroups)
  RealVector modelGroupCost;

  /// remaining budget for solution over active models/groups, following
  /// deduction of sunk (pilot) cost for inactive models/groups
  Real activeBudget;

  /// user specification to suppress any increments in the number of HF
  /// evaluations (e.g., because too expensive and no more can be performed)
  bool truthFixedByPilot;
  /// a subset of the non-hierarchcvial samplers provide analytic derivatives
  /// of estimator variance
  bool analyticEstVarDerivs;
  /// employ truncated SVD rather than Cholesky factorization for matrix solve
  bool hardenNumericSoln;

  /// for sample projections, the calculated increment in HF samples that
  /// would be evaluated if full iteration/statistics were pursued
  size_t deltaNActualHF;

  /// number of successful pilot evaluations of HF truth model (exclude faults)
  SizetArray numHIter0;
  /// scalar accuracy metric derived from estVarIter0 according to
  /// estVarMetricType
  Real estVarMetric0;

  //int fdCntr; // for debug FD gradient output (compared with analytic grad)

private:

  //
  //- Heading: helper functions
  //

  /// flattens contours of estimator_variance_metric() using std::log
  Real log_estvar_metric(const RealVector& cd_vars);
  /// gradient of log-metric returned by log_estvar_metric()
  void log_estvar_metric_gradient(const RealVector& cd_vars,
				  RealVector& log_ev_grad);
  /// both value and gradient of log-metric, avoiding duplicated calculations
  void log_estvar_metric_and_gradient(const RealVector& cd_vars, Real& log_ev,
				      RealVector& log_ev_grad);

  /// reduce gradients of estvar{,_ratios} to form gradient of estvar metric
  void estvar_gradients_to_metric_gradient(const RealVector& ev_vec,
					   const RealMatrix& ev_grad,
					   RealVector& evm_grad);

  /// compute a penalty merit function from objective, constraint, and
  /// constaint bound
  Real nh_penalty_merit(Real obj, Real nln_con, Real nln_u_bnd);

  /// static function used by NPSOL for the objective function
  static void npsol_objective(int& mode, int& n, double* x, double& f,
			      double* grad_f, int& nstate);
  /// static function used by NPSOL for the nonlinear constraints, if present
  static void npsol_constraint(int& mode, int& ncnln, int& n, int& nrowj,
			       int* needc, double* x, double* c, double* cjac,
			       int& nstate);

  /// static NLF1 function used by OPT++ for the objective function/gradient
  static void optpp_nlf1_objective(int mode, int n, const RealVector& x,
				   double& f, RealVector& grad_f,
				   int& result_mode);
  /// static FDNLF1 function used by OPT++ for the objective function
  static void optpp_fdnlf1_objective(int n, const RealVector& x, double& f,
				     int& result_mode);
  /// static NLF1 function used by OPT++ for the nonlinear constraints
  static void optpp_nlf1_constraint(int mode, int n, const RealVector& x,
				    RealVector& c, RealMatrix& grad_c,
				    int& result_mode);
  /// static FDNLF1 function used by OPT++ for the nonlinear constraints
  static void optpp_fdnlf1_constraint(int n, const RealVector& x,
				      RealVector& c, int& result_mode);

  /// static function used by DIRECT for its objective
  static Real direct_penalty_merit(const RealVector& cd_vars);

  /// static function used by MinimizerAdapterModel for response data
  /// (objective and nonlinear constraint, if present)
  static void response_evaluator(const Variables& vars, const ActiveSet& set,
				 Response& response);

  // bound x away from zero
  //void enforce_nudge(RealVector& x);

  //
  //- Heading: Data
  //

  /// pointer to NonDACV instance used in static member functions
  static NonDNonHierarchSampling* nonHierSampInstance;
};


inline size_t NonDNonHierarchSampling::num_approximations() const
{ return numApprox; }


inline unsigned short NonDNonHierarchSampling::uses_method() const
{ return optSubProblemSolver; }


inline Real NonDNonHierarchSampling::estimator_accuracy_metric() const
{ return final_solution_data().estimator_variance_metric(); }


//inline Real NonDNonHierarchSampling::estimator_cost_metric() const
//{ return final_solution_data().equivalent_hf_allocation(); }


/*
inline void NonDNonHierarchSampling::initialize_sums(IntRealMatrixMap& sum_Q)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> mat_pr;
  for (int i=1; i<=4; ++i) {
    mat_pr.first = i; // moment number
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size Real{Vector,Matrix} in place and init sums to 0
    sum_Q.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  }
}


inline void NonDNonHierarchSampling::initialize_counts(Sizet2DArray& num_Q)
{
  num_Q.resize(numApprox);
  for (size_t approx=0; approx<numApprox; ++approx)
    num_Q[approx].assign(numFunctions, 0);
}
*/


inline void NonDNonHierarchSampling::
initialize_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		IntRealMatrixMap& sum_LH,         RealVector&       sum_HH)
{
  // sum_* are running sums across all increments
  std::pair<int, RealVector> vec_pr; std::pair<int, RealMatrix> mat_pr;
  for (int i=1; i<=4; ++i) {
    vec_pr.first = mat_pr.first = i; // moment number
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size Real{Vector,Matrix} in place and init sums to 0
    sum_L_baseline.insert(mat_pr).first->second.shape(numFunctions, numApprox);
    sum_H.insert(vec_pr).first->second.size(numFunctions);
    sum_LH.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  }
  sum_HH.size(numFunctions);
}


inline void NonDNonHierarchSampling::
initialize_counts(Sizet2DArray& num_L_baseline, SizetArray& num_H,
		  Sizet2DArray& num_LH)
{
  num_H.assign(numFunctions, 0);
  num_L_baseline.resize(numApprox);  num_LH.resize(numApprox);
  for (size_t approx=0; approx<numApprox; ++approx) {
    num_L_baseline[approx].assign(numFunctions,0);
    num_LH[approx].assign(numFunctions,0);
  }
}


inline void NonDNonHierarchSampling::
initialize_group_sums(RealMatrixArray& sum_G, RealSymMatrix2DArray& sum_GG)
{
  // order indexing such that per-group structure is consistent with
  // other estimators
  size_t g, num_groups = modelGroups.size(), num_models;
  sum_G.resize(num_groups);  sum_GG.resize(num_groups);
  for (g=0; g<num_groups; ++g) {
    num_models = modelGroups[g].size();
    sum_G[g].shape(numFunctions, num_models);
    RealSymMatrixArray& sum_GG_g = sum_GG[g];
    sum_GG_g.resize(numFunctions);
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      sum_GG_g[qoi].shape(num_models);
  }
}


inline void NonDNonHierarchSampling::
initialize_group_sums(RealMatrixArray& sum_G)
{
  // order indexing such that per-group structure is consistent with
  // other estimators
  size_t g, num_groups = modelGroups.size();
  sum_G.resize(num_groups);
  for (g=0; g<num_groups; ++g)
    sum_G[g].shape(numFunctions, modelGroups[g].size());
}


inline void NonDNonHierarchSampling::
initialize_group_sums(IntRealMatrixArrayMap& sum_G,
		     IntRealSymMatrix2DArrayMap& sum_GG)
{
  RealMatrixArray mat1;  RealSymMatrix2DArray mat2;
  initialize_group_sums(mat1, mat2);
  for (int i=1; i<=4; ++i)
    { sum_G[i] = mat1; sum_GG[i] = mat2; } // copies
}


inline void NonDNonHierarchSampling::
initialize_group_sums(IntRealMatrixArrayMap& sum_G)
{
  RealMatrixArray mat1;
  initialize_group_sums(mat1);
  for (int i=1; i<=4; ++i)
    sum_G[i] = mat1; // copies
}


inline void NonDNonHierarchSampling::
initialize_group_counts(Sizet2DArray& num_G)
{
  size_t g, num_groups = modelGroups.size(), num_models;
  num_G.resize(num_groups);
  for (g=0; g<num_groups; ++g)
    num_G[g].assign(numFunctions, 0);
}


inline void NonDNonHierarchSampling::no_solve_variances(MFSolutionData& soln)
{
  // For offline pilot, the online EstVar is undefined prior to any online
  // samples, but should not happen (no budget used) unless bad convTol spec
  if (pilotMgmtMode == ONLINE_PILOT || pilotMgmtMode == ONLINE_PILOT_PROJECTION)
    soln.estimator_variances(estVarIter0);
  else // 0/0
    soln.initialize_estimator_variances(numFunctions,
      std::numeric_limits<Real>::quiet_NaN());
  soln.initialize_estimator_variance_ratios(numFunctions, 1.);
}


inline void NonDNonHierarchSampling::
competed_initial_guesses(MFSolutionData& mf_soln, MFSolutionData& cv_soln,
			 MFSolutionData& selected_soln)
{
  //if (multiStartACV) { // Run numerical solns from both starting points
  ensemble_numerical_solution(mf_soln);
  ensemble_numerical_solution(cv_soln);
  pick_mfmc_cvmc_solution(mf_soln, cv_soln, selected_soln);
  //}
  /*
  else { // Run one numerical soln from best of two starting points
    bool mfmc_init;
    if (budget_constrained) { // same cost, compare accuracy
      RealVector cdv;
      r_and_N_to_design_vars(mf_soln.avgEvalRatios,mf_soln.avgHFTarget,cdv);
      mf_soln.estVarMetric = estimator_variance_metric(cdv); // ACV or GenACV
      r_and_N_to_design_vars(cv_soln.avgEvalRatios,cv_soln.avgHFTarget,cdv);
      cv_soln.estVarMetric = estimator_variance_metric(cdv); // ACV or GenACV
      mfmc_init = (mf_soln.estVarMetric < cv_soln.estVarMetric);
    }
    else { // same accuracy (convergenceTol * estVarIter0), compare cost 
      mf_soln.equivHFAlloc = compute_equivalent_cost(mf_soln.avgHFTarget,
	mf_soln.avgEvalRatios, sequenceCost);
      cv_soln.equivHFAlloc = compute_equivalent_cost(cv_soln.avgHFTarget,
	cv_soln.avgEvalRatios, sequenceCost);
      mfmc_init = (mf_soln.equivHFAlloc < cv_soln.equivHFAlloc);
    }
    selected_soln = (mfmc_init) ? mf_soln : cv_soln;
    // Single solve initiated from lowest estvar
    ensemble_numerical_solution(selected_soln);
  }
  */
}


inline void NonDNonHierarchSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  Cerr << "Error: estimator_variance_ratios() not defined by derived class.\n"
       << std::endl;
  abort_handler(METHOD_ERROR);
}


inline void NonDNonHierarchSampling::
estimator_variance_ratio_gradients(const RealVector& cd_vars,
				   RealMatrix& evr_grads)
{
  // *** TO DO ***: this can be derived from
  //   ev = varH/N_H ev_ratios
  // where varH is fixed and N_H is a design variable for ACV,GenACV,MFMC

  Cerr << "Error: estimator_variance_ratio_gradients() not defined by derived "
       << "class.\n" << std::endl;
  abort_handler(METHOD_ERROR);
}


inline void NonDNonHierarchSampling::
estimator_variances(const RealVector& cd_vars, RealVector& est_var)
{
  RealVector estvar_ratios(numFunctions, false);
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual: MFMC,ACV,GenACV
  estimator_variances_from_ratios(cd_vars, estvar_ratios, est_var);
}


inline void NonDNonHierarchSampling::
estimator_variances_and_ratios(const RealVector& cd_vars,
			       RealVector& estvar_ratios, RealVector& estvar)
{
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual: MFMC,ACV,GenACV
  estimator_variances_from_ratios(cd_vars, estvar_ratios, estvar);
}


inline void NonDNonHierarchSampling::
estimator_variances_and_ratios(const RealVector& cd_vars, MFSolutionData& soln)
{
  RealVector estvar_ratios, estvar;
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual: MFMC,ACV,GenACV
  estimator_variances_from_ratios(cd_vars, estvar_ratios, estvar);
  soln.estimator_variances(estvar);
  soln.estimator_variance_ratios(estvar_ratios);
  //soln.update_estimator_variance_metric(estVarMetricType,
  //                                      estVarMetricNormOrder);
}


inline void NonDNonHierarchSampling::
estimator_variance_gradients(const RealVector& cd_vars, RealMatrix& ev_grads)
{
  //Cerr << "Warning: testing omission of estimator_variance_gradients() by "
  //     << "derived class.\n" << std::endl;
  ev_grads.shape(0,0);
}


inline void NonDNonHierarchSampling::
estimator_variances_and_gradients(const RealVector& cd_vars,
				  RealVector& est_var, RealMatrix& ev_grads)
{
  // default implementation
  estimator_variances(cd_vars, est_var);
  estimator_variance_gradients(cd_vars, ev_grads);
}


inline void NonDNonHierarchSampling::
estimator_variance_ratios_and_gradients(const RealVector& cd_vars,
					RealVector& estvar_ratios,
					RealMatrix& evr_grads)
{
  // default implementation
  estimator_variance_ratios(cd_vars, estvar_ratios);
  estimator_variance_ratio_gradients(cd_vars, evr_grads);
}


inline void NonDNonHierarchSampling::
estvar_ratios_to_estvar(const RealVector& estvar_ratios,
			const RealVector& var_H, const SizetArray& N_H,
			RealVector& estvar)
{
  if (estvar.length() != numFunctions) estvar.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    estvar[qoi] = estvar_ratios[qoi] * var_H[qoi] / N_H[qoi];
}


inline void NonDNonHierarchSampling::
estvar_ratios_to_estvar(const RealVector& estvar_ratios,
			const RealVector& var_H, Real N_H, RealVector& estvar)
{
  if (estvar.length() != numFunctions) estvar.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    estvar[qoi] = estvar_ratios[qoi] * var_H[qoi] / N_H;
}


inline void NonDNonHierarchSampling::
estvar_ratios_to_estvar(const RealVector& var_H, const SizetArray& N_H,
			MFSolutionData& soln)
{
  const RealVector& estvar_ratios = soln.estimator_variance_ratios();
  soln.initialize_estimator_variances(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    soln.estimator_variance(estvar_ratios[qoi] * var_H[qoi] / N_H[qoi], qoi);
}


inline Real NonDNonHierarchSampling::
estimator_variance_metric(const RealVector& cd_vars)
{
  RealVector estvar_ratios, estvar;  Real metric;  size_t metric_index;
  switch (estVarMetricType) {
  case AVG_ESTVAR_RATIO_METRIC: case NORM_ESTVAR_RATIO_METRIC:
  case MAX_ESTVAR_RATIO_METRIC:
    estimator_variance_ratios(cd_vars, estvar_ratios);//virtual: MFMC,ACV,GenACV
    break;
  default:
    estimator_variances(cd_vars, estvar); break;
  }
  MFSolutionData::update_estimator_variance_metric(estVarMetricType,
    estVarMetricNormOrder, estvar_ratios, estvar, metric, metric_index);
  return metric;
}


inline void NonDNonHierarchSampling::
estimator_variance_metric_gradient(const RealVector& cd_vars,
				   RealVector& evm_grad)
{
  RealVector ev_vec;  RealMatrix ev_grads;
  switch (estVarMetricType) {
  case NORM_ESTVAR_METRIC:       case MAX_ESTVAR_METRIC:
    estimator_variances_and_gradients(cd_vars, ev_vec, ev_grads);        break;
  case NORM_ESTVAR_RATIO_METRIC: case MAX_ESTVAR_RATIO_METRIC:
    estimator_variance_ratios_and_gradients(cd_vars, ev_vec, ev_grads);  break;
  default:
    estimator_variance_gradients(cd_vars, ev_grads);                     break;
  }

  estvar_gradients_to_metric_gradient(ev_vec, ev_grads, evm_grad);
}


inline void NonDNonHierarchSampling::
estimator_variance_metric_and_gradient(const RealVector& cd_vars,
				       Real& ev_metric, RealVector& evm_grad)
{
  RealVector estvar_ratios, estvar;  RealMatrix ev_grads;
  switch (estVarMetricType) {
  case AVG_ESTVAR_RATIO_METRIC: case NORM_ESTVAR_RATIO_METRIC:
  case MAX_ESTVAR_RATIO_METRIC:
    estimator_variance_ratios_and_gradients(cd_vars, estvar_ratios, ev_grads);
    estvar_gradients_to_metric_gradient(estvar_ratios, ev_grads, evm_grad);
    break;
  default:
    estimator_variances_and_gradients(cd_vars, estvar, ev_grads);
    estvar_gradients_to_metric_gradient(estvar, ev_grads, evm_grad);
    break;
  }

  size_t metric_index;
  MFSolutionData::update_estimator_variance_metric(estVarMetricType,
    estVarMetricNormOrder, estvar_ratios, estvar, ev_metric, metric_index);
}


inline Real NonDNonHierarchSampling::
log_estvar_metric(const RealVector& cd_vars)
{
  Real ev_metric = estimator_variance_metric(cd_vars);
  if (ev_metric > 0.)
    return std::log(ev_metric); // use log to flatten contours
  else
    return std::numeric_limits<Real>::quiet_NaN();//Pecos::LARGE_NUMBER;
}


inline void NonDNonHierarchSampling::
log_estvar_metric_gradient(const RealVector& cd_vars, RealVector& log_evm_grad)
{
  RealVector evm_grad;  estimator_variance_metric_gradient(cd_vars, evm_grad);
  if (evm_grad.empty()) // not supported by derived class
    { log_evm_grad.size(0); return; }

  size_t v, num_v = cd_vars.length();
  if (log_evm_grad.length() != num_v) log_evm_grad.sizeUninitialized(num_v);
  Real ev_metric = estimator_variance_metric(cd_vars);
  if (ev_metric > 0.)
    for (v=0; v<num_v; ++v)
      log_evm_grad[v] = evm_grad[v] / ev_metric;
  else
    log_evm_grad = 0.;
}


inline void NonDNonHierarchSampling::
log_estvar_metric_and_gradient(const RealVector& cd_vars, Real& log_evm,
			       RealVector& log_evm_grad)
{
  Real ev_metric;  RealVector evm_grad;
  estimator_variance_metric_and_gradient(cd_vars, ev_metric, evm_grad);

  log_evm = (ev_metric > 0.) ? std::log(ev_metric) :
    std::numeric_limits<Real>::quiet_NaN();//Pecos::LARGE_NUMBER;
  if (evm_grad.empty()) // not supported by derived class
    log_evm_grad.size(0);
  else {
    size_t v, num_v = cd_vars.length();
    if (log_evm_grad.length() != num_v) log_evm_grad.sizeUninitialized(num_v);
    if (ev_metric > 0.)
      for (v=0; v<num_v; ++v)
	log_evm_grad[v] = evm_grad[v] / ev_metric;
    else
      log_evm_grad = 0.;
  }
}


inline void NonDNonHierarchSampling::
finalize_counts(const Sizet2DArray& N_L_actual, const SizetArray& N_L_alloc)
{
  // post final sample counts back to NLev{Actual,Alloc} (for final summaries)

  // Note: key data is fixed for all non-hierarchical cases
  const Pecos::ActiveKey& active_key = iteratedModel->active_model_key();
  if (active_key.data_size() != numApprox + 1) {
    Cerr << "Error: inconsistent active key size in NonDNonHierarchSampling::"
	 << "finalize_counts()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  size_t approx, mf, rl;
  for (approx=0; approx<numApprox; ++approx) {
    mf = active_key.retrieve_model_form(approx);
    rl = active_key.retrieve_resolution_level(approx);

    NLevAlloc[mf][rl] += N_L_alloc[approx];
    SizetArray& N_l_actual_fl = NLevActual[mf][rl];
    if (N_l_actual_fl.empty()) N_l_actual_fl = N_L_actual[approx];
    else     increment_samples(N_l_actual_fl,  N_L_actual[approx]);
  }
}


inline Real NonDNonHierarchSampling::
compute_equivalent_cost(Real hf_target, const RealVector& avg_eval_ratios,
			const RealVector& cost)
{
  size_t a, hf_index = cost.length() - 1;
  Real cost_ref = cost[hf_index], rc_sum = 0.;
  for (a=0; a<numApprox; ++a)
    rc_sum += avg_eval_ratios[a] * cost[a];
  return hf_target * (rc_sum / cost_ref + 1.);
}


inline Real NonDNonHierarchSampling::
compute_equivalent_cost(Real hf_target, const RealVector& avg_eval_ratios,
			const RealVector& cost, const UShortArray& approx_set)
{
  size_t a, num_approx = approx_set.size(), hf_index = cost.length() - 1;
  Real cost_ref = cost[hf_index], rc_sum = 0.;
  for (a=0; a<num_approx; ++a)
    rc_sum += avg_eval_ratios[a] * cost[approx_set[a]];
  return hf_target * (rc_sum / cost_ref + 1.);
}


/* redundant with linear_model_cost():
inline Real NonDNonHierarchSampling::
compute_equivalent_cost(const RealVector& sample_counts, const RealVector& cost)
{
  Real nc_sum = 0.;
  for (unsigned short a=0; a<numApprox; ++a)
    nc_sum += sample_counts[a] * cost[a];
  return sample_counts[numApprox] + nc_sum / cost[numApprox];
}


inline Real NonDNonHierarchSampling::
compute_equivalent_cost(const RealVector& sample_counts, const RealVector& cost,
			const UShortArray& approx_set)
{
  size_t a, num_approx = approx_set.size(), hf_index = cost.length() - 1;
  Real nc_sum = 0.;
  for (a=0; a<num_approx; ++a)
    nc_sum += sample_counts[a] * cost[approx_set[a]];
  return sample_counts[num_approx] + nc_sum / cost[hf_index];
}
*/


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  size_t index,    Real& equiv_hf_evals)
{
  size_t len = cost.length(), hf_index = len-1;
  equiv_hf_evals += (index == hf_index) ? new_samp :
    (Real)new_samp * cost[index] / cost[hf_index];
}


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  size_t start, size_t end, Real& equiv_hf_evals)
{
  size_t index, len = cost.length(), hf_index = len-1;
  if (end == len)
    { equiv_hf_evals += (Real)new_samp; --end; }
  Real sum_cost = 0.;
  for (index=start; index<end; ++index)
    sum_cost += cost[index];
  equiv_hf_evals += (Real)new_samp * sum_cost / cost[hf_index];
}


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  const SizetArray& approx_sequence,
			  size_t start, size_t end, Real& equiv_hf_evals)
{
  if (approx_sequence.empty())
    increment_equivalent_cost(new_samp, cost, start, end, equiv_hf_evals);
  else {
    size_t i, len = cost.length(), hf_index = len-1, approx;
    bool ordered = approx_sequence.empty();
    // This fn is only used for LF sample increments:
    //if (end == len) // truth is always last
    //  { equiv_hf_evals += new_samp;  if (end) --end; }
    Real sum_cost = 0.;
    for (i=start; i<end; ++i) {
      approx = (ordered) ? i : approx_sequence[i];
      sum_cost += cost[approx];
    }
    equiv_hf_evals += (Real)new_samp * sum_cost / cost[hf_index];
  }
}


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  const SizetArray& approx_sequence,
			  size_t start, size_t end,
			  const UShortArray& approx_set, Real& equiv_hf_evals)
{
  size_t i, approx, num_approx = approx_set.size(), hf_index = cost.length()-1;
  bool ordered = approx_sequence.empty();
  // This fn is only used for LF sample increments:
  //if (end == num_approx+1) // truth is always last
  //  { equiv_hf_evals += new_samp;  if (end) --end; }
  Real sum_cost = 0.;
  for (i=start; i<end; ++i) {
    approx = (ordered) ? i : approx_sequence[i]; // compact indexing
    sum_cost += cost[approx_set[approx]];       // inflated indexing
  }
  equiv_hf_evals += (Real)new_samp * sum_cost / cost[hf_index];
}


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  unsigned short root, const UShortSet& reverse_dag,
			  Real& equiv_hf_evals)
{
  Real sum_cost = 0;
  if (root != USHRT_MAX) sum_cost += cost[root];
  UShortSet::const_iterator cit;
  for (cit=reverse_dag.begin(); cit!=reverse_dag.end(); ++cit)
    sum_cost += cost[*cit];
  size_t hf_index = cost.length() - 1;
  equiv_hf_evals += (Real)new_samp * sum_cost / cost[hf_index];
}


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  unsigned short root, const UShortArray& approx_set,
			  Real& equiv_hf_evals)
{
  Real sum_cost = 0;
  if (root != USHRT_MAX) sum_cost += cost[root];
  size_t i, num_approx = approx_set.size();
  for (i=0; i<num_approx; ++i)
    sum_cost += cost[approx_set[i]];
  size_t hf_index = cost.length() - 1;
  equiv_hf_evals += (Real)new_samp * sum_cost / cost[hf_index];
}


inline void NonDNonHierarchSampling::
increment_equivalent_cost(const SizetArray& delta_N_g,
			  const RealVector& group_cost, Real hf_cost,
			  Real& equiv_hf_evals)
{
  // for group sampling, e.g. NonDMultilevBLUESampling::group_increments()

  size_t g, group_len = group_cost.length();
  Real sum = 0.;
  for (g=0; g<group_len; ++g)
    if (delta_N_g[g])
      sum += (Real)delta_N_g[g] * group_cost[g];
  equiv_hf_evals += sum / hf_cost;
}


inline void NonDNonHierarchSampling::
increment_sample_range(SizetArray& N_L, size_t incr,
		       const SizetArray& approx_sequence,
		       size_t start, size_t end)
{
  if (!incr) return;
  bool ordered = approx_sequence.empty();  size_t i, approx;
  for (i=start; i<end; ++i) {
    approx = (ordered) ? i : approx_sequence[i];
    N_L[approx] += incr;
  }
}


inline void NonDNonHierarchSampling::
increment_sample_range(SizetArray& N_L, size_t incr,
		       const UShortArray& approx_set)
{
  if (!incr) return;
  size_t i, num_approx = approx_set.size();
  for (i=0; i<num_approx; ++i)
    N_L[approx_set[i]] += incr;
}


inline void NonDNonHierarchSampling::
increment_sample_range(SizetArray& N_L, size_t incr,
		       const SizetArray& approx_sequence,
		       size_t start, size_t end, const UShortArray& approx_set)
{
  if (!incr) return;
  bool ordered = approx_sequence.empty();  size_t i, approx;
  for (i=start; i<end; ++i) {
    approx = (ordered) ? i : approx_sequence[i]; // compact indexing
    N_L[approx_set[approx]] += incr;            // inflated indexing
  }
}


inline void NonDNonHierarchSampling::
increment_sample_range(SizetArray& N_L, size_t incr, unsigned short root,
		       const UShortSet& reverse_dag)
{
  if (!incr) return;

  // Increment shared samples across a dependency: each z1[leaf] = z2[root]
  N_L[root] += incr; // root must not be numApprox
  UShortSet::const_iterator cit;
  for (cit=reverse_dag.begin(); cit!=reverse_dag.end(); ++cit)
    N_L[*cit] += incr;
}


inline void NonDNonHierarchSampling::
ensemble_active_set(const UShortArray& model_set)
{
  activeSet.request_values(0);
  size_t m, num_models = model_set.size(), start;
  for (m=0; m<num_models; ++m) {
    start = model_set[m] * numFunctions;
    activeSet.request_values(1, start, start+numFunctions);
  }
}


inline void NonDNonHierarchSampling::
root_reverse_dag_to_group(unsigned short root, const UShortSet& rev_dag,
			  UShortArray& model_group)
{
  model_group.clear();  model_group.reserve(rev_dag.size() + 1);
  model_group.insert(model_group.end(), rev_dag.begin(), rev_dag.end());
  model_group.push_back(root); // by convention
}


inline void NonDNonHierarchSampling::
group_to_root_reverse_dag(const UShortArray& model_group, unsigned short& root,
			  UShortSet& rev_dag)
{
  root = model_group.back(); // by convention
  rev_dag.clear();
  rev_dag.insert(model_group.begin(), --model_group.end());
}


inline void NonDNonHierarchSampling::
mfmc_model_group(size_t last_index, UShortArray& model_group) const
{
  // MFMC or ACV-MF: last index is the all-models group
  size_t m, num_models = last_index+1;
  model_group.resize(num_models);
  for (m=0; m<num_models; ++m)
    model_group[m] = m; // "pyramid" sequence from 0 to last in set
}


inline void NonDNonHierarchSampling::
mfmc_model_group(size_t last_index, const SizetArray& approx_sequence,
		 UShortArray& model_group) const
{
  if (approx_sequence.empty())
    { mfmc_model_group(last_index, model_group); return; }

  // MFMC or ACV-MF: last index is the all-models group
  // Note: we resequence the models within fixed pyramid groups (hierarchy is
  //   reordered prior to pyramid sampling), as opposed to reordering fixed
  //   modelGroups for root dominance (which destroys pyramid structure)
  size_t m, group_size = last_index+1, num_approx = approx_sequence.size(),
    seq_mapping_len = std::min(group_size, num_approx);
  model_group.resize(group_size);
  for (m=0; m<seq_mapping_len; ++m)
    model_group[m] = approx_sequence[m]; // low to high by sequence
  // truth model can be part of model group but is not part of approx sequence
  for (m=seq_mapping_len; m<group_size; ++m)
    model_group[m] = m;
  // Note: model_group is not a std::set.  In ML BLUE, the model ordering
  // within each group is irrelevant and they are ordered by convention for
  // group uniqueness.  Here, we allow them to be reordered by approx_sequence.
}


inline void NonDNonHierarchSampling::
singleton_model_group(size_t index, UShortArray& model_group) const
{ model_group.resize(1); model_group[0] = index; }


inline void NonDNonHierarchSampling::
singleton_model_group(size_t index, const SizetArray& approx_sequence,
		      UShortArray& model_group) const
{
  if (approx_sequence.empty())
    { singleton_model_group(index, model_group); return; }
  model_group.resize(1);
  model_group[0] = (index < approx_sequence.size()) ?
    approx_sequence[index] : index;
}


inline void NonDNonHierarchSampling::
cvmc_model_group(size_t index, UShortArray& model_group) const
{
  if (index < numApprox) singleton_model_group(index, model_group);
  else                   mfmc_model_group(numApprox,  model_group);
}


inline void NonDNonHierarchSampling::
cvmc_model_group(size_t index, const SizetArray& approx_sequence,
		 UShortArray& model_group) const
{
  if (approx_sequence.empty())
    { cvmc_model_group(index, model_group); return; }
  if (index < numApprox)
    singleton_model_group(index, approx_sequence, model_group);
  else
    mfmc_model_group(numApprox,  approx_sequence, model_group);
}


inline void NonDNonHierarchSampling::
mlmc_model_group(size_t index, UShortArray& model_group) const
{
  // MLMC or ACV-RD (ACV-IS differs in shared group)
  if (index == 0)
    { model_group.resize(1); model_group[0] = index; }
  else {
    model_group.resize(2);
    model_group[0] = index - 1; model_group[1] = index; // ordered low to high
  }
}


inline void NonDNonHierarchSampling::
mlmc_model_group(size_t index, const SizetArray& approx_sequence,
		 UShortArray& model_group) const
{
  if (approx_sequence.empty())
    { mlmc_model_group(index, model_group); return; }
  // MLMC or ACV-RD (ACV-IS differs in shared group)
  if (index == 0)
    singleton_model_group(index, approx_sequence, model_group);
  else { // model pair ordered low to high
    model_group.resize(2);
    model_group[0] = (index - 1 < approx_sequence.size()) ?
      approx_sequence[index - 1] : index - 1;
    model_group[1] = (index < approx_sequence.size()) ?
      approx_sequence[index]     : index;
  }
}


inline void NonDNonHierarchSampling::
print_group(std::ostream& s, size_t g) const
{
  const UShortArray& group_g = modelGroups[g];
  size_t m, num_models = group_g.size();
  s << " (models";
  for (m=0; m<num_models; ++m)
    s << ' ' << group_g[m];
  s << ")\n";
}


inline Real NonDNonHierarchSampling::
allocate_budget(const RealVector& avg_eval_ratios, const RealVector& cost,
		Real budget)
{
  // version with scalar HF target (eval_ratios already averaged over QoI
  // due to formulation of optimization sub-problem)

  Real cost_H = cost[numApprox], inner_prod = cost_H;// raw cost (un-normalized)
  for (size_t approx=0; approx<numApprox; ++approx)
    inner_prod += cost[approx] * avg_eval_ratios[approx];
  Real hf_target = budget / inner_prod * cost_H; // normalized to equivHF
  return hf_target;
}


inline Real NonDNonHierarchSampling::
allocate_budget(const RealVector& avg_eval_ratios, const RealVector& cost)
{ return allocate_budget(avg_eval_ratios, cost, activeBudget); }


inline Real NonDNonHierarchSampling::
allocate_budget(const UShortArray& approx_set,const RealVector& avg_eval_ratios,
		const RealVector& cost,	Real budget)
{
  // version with scalar HF target (eval_ratios already averaged over QoI
  // due to formulation of optimization sub-problem)

  Real cost_H = cost[numApprox], inner_prod = cost_H;// raw cost (un-normalized)
  size_t a, num_approx = approx_set.size();
  for (a=0; a<num_approx; ++a)
    inner_prod += cost[approx_set[a]] * avg_eval_ratios[a];
  Real hf_target = budget / inner_prod * cost_H; // normalized to equivHF
  return hf_target;
}


inline Real NonDNonHierarchSampling::
allocate_budget(const UShortArray& approx_set,
		const RealVector& avg_eval_ratios, const RealVector& cost)
{ return allocate_budget(approx_set, avg_eval_ratios, cost, activeBudget); }


inline void NonDNonHierarchSampling::
pick_mfmc_cvmc_solution(const MFSolutionData& mf_soln, //size_t mf_samp,
			const MFSolutionData& cv_soln, //size_t cv_samp,
			MFSolutionData& soln) //, size_t& num_samp)
{
  Cout << "Best numerical solution initiated from ";
  if (nh_penalty_merit(mf_soln) < nh_penalty_merit(cv_soln)) {
    Cout << "analytic MFMC.\n" << std::endl;
    soln = mf_soln;  //num_samp = mf_samp;
  }
  else {
    Cout << "ensemble of pairwise CVMC.\n" << std::endl;
    soln = cv_soln;  //num_samp = cv_samp;
  }
}


inline void NonDNonHierarchSampling::cache_mc_reference()
{
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];

  // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
  // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
  // (This differs from MLMC EstVar^0 which uses pilot for all levels.)
  // Note: could revisit this for case of lf_shared_pilot > hf_shared_pilot.
  compute_mc_estimator_variance(varH, N_H_actual, estVarIter0);
  numHIter0 = N_H_actual;
  // This simply manages average vs. max of estVarIter0 (cached as
  // estVarMetric0) used in accuracy constraint bounds.  It does not
  // update the bound for a runtime change in the max metric_index, which
  // would require moving the variable bound value inside the constraint.
  MFSolutionData::update_estimator_variance_metric(estVarMetricType,
    estVarMetricNormOrder, estVarIter0, estVarMetric0);
}


inline void NonDNonHierarchSampling::apply_mc_reference(RealVector& mc_targets)
{
  // base implementation for use when varH is available

  if (mc_targets.length() != numFunctions)
    mc_targets.sizeUninitialized(numFunctions);
  switch (convergenceTolType) {
  case ABSOLUTE_CONVERGENCE_TOLERANCE:
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      mc_targets[qoi] = varH[qoi] / convergenceTol;
    break;
  default: // relative tolerance
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      mc_targets[qoi] = varH[qoi] / (convergenceTol * estVarIter0[qoi]);
    break;
  }
}


inline void NonDNonHierarchSampling::
enforce_bounds(RealVector& x0, const RealVector& x_lb, const RealVector& x_ub)
{
  size_t i, len = x0.length();
  if (x_lb.length() != len || x_ub.length() != len) {
    Cerr << "Error: inconsistent bound sizes in enforce_bounds(): (0,l,u) = ("
	 << len << "," << x_lb.length() << "," << x_ub.length() << ")."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (i=0; i<len; ++i) {
    Real x_lb_i = x_lb[i], x_ub_i = x_ub[i];
    if (x_lb_i > x_ub_i) {
      Cerr << "Error: inconsistent bound values in enforce_bounds(): (l,u) = ("
	   << x_lb_i << "," << x_ub_i << ")." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    Real& x0_i = x0[i];
    if      (x0_i < x_lb_i)  x0_i = x_lb_i;
    else if (x0_i > x_ub_i)  x0_i = x_ub_i;
  }
}


inline void NonDNonHierarchSampling::
r_and_N_to_N_vec(const RealVector& avg_eval_ratios, Real N_H, RealVector& N_vec)
{
  size_t num_approx = avg_eval_ratios.length(), vec_len = num_approx+1;
  if (N_vec.length() != vec_len) N_vec.sizeUninitialized(vec_len);
  for (size_t i=0; i<num_approx; ++i)
    N_vec[i] = avg_eval_ratios[i] * N_H;
  N_vec[num_approx] = N_H;
}


inline void NonDNonHierarchSampling::
r_and_N_to_design_vars(const RealVector& avg_eval_ratios, Real N_H,
		       RealVector& cd_vars)
{
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // embed N at end of cd_vars for GenACV usage
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    size_t num_approx = avg_eval_ratios.length();
    cd_vars.sizeUninitialized(num_approx+1);
    copy_data_partial(avg_eval_ratios, cd_vars, 0);
    cd_vars[num_approx] = N_H;
    break;
  }
  case N_MODEL_LINEAR_OBJECTIVE:  case N_MODEL_LINEAR_CONSTRAINT:
    r_and_N_to_N_vec(avg_eval_ratios, N_H, cd_vars);  break;
  }
}


inline void NonDNonHierarchSampling::
solution_to_design_vars(const MFSolutionData& soln, RealVector& cd_vars)
{
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // embed N at end of cd_vars for GenACV usage
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    RealVector r = soln.solution_ratios(); size_t num_r = r.length();
    cd_vars.sizeUninitialized(num_r+1);    copy_data_partial(r, cd_vars, 0);
    cd_vars[num_r] = soln.solution_reference();
    break;
  }
  default: // N_i for models or groups
    copy_data(soln.solution_variables(), cd_vars);  break;
  }
}


inline Real NonDNonHierarchSampling::
update_hf_target(const RealVector& estvar_ratios, size_t metric_index,
		 const RealVector& var_H, const RealVector& estvar_0)
{
  // Definitions: (estvar_0 <- estVarIter0, N_0 <- numHIter0, reuse var_H)
  //   estvar     = estvar_ratio  var_H / N_H    (current)
  //   estvar_tgt = estvar_ratio  var_H / N_tgt  (future)
  //   estvar_0   = estvar_ratio0 var_H / N_0    (pilot)
  //     where estvar_ratio0 = 1 when a MC reference is used for estVarIter0

  // Accuracy target:
  //   estvar_tgt = convTol estvar_0
  // Expand estvar_ratios and solve for N_tgt:
  //   estvar_ratio var_H / N_tgt = convTol estvar_ratio0 var_H / N_0
  //   estvar_ratio   N_0 / N_tgt = convTol estvar_ratio0
  //   N_tgt  = estvar_ratio N_0 / (convTol estvar_ratio0)   [not used]
  // Convert estvar_ratio0 back to incoming estvar_0:
  //   N_tgt = estvar_ratio N_0 / (convTol estvar_0 N_0 / var_H)
  //   N_tgt = estvar_ratio var_H / (convTol estvar_0)       [this case]

  Real hf_target;
  switch (estVarMetricType) {
  case   DEFAULT_ESTVAR_METRIC:  case AVG_ESTVAR_METRIC:
  case AVG_ESTVAR_RATIO_METRIC:
    hf_target = 0.;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      hf_target += estvar_ratios[qoi] * var_H[qoi] / estvar_0[qoi];
    hf_target /= convergenceTol * numFunctions;
    break;
  case MAX_ESTVAR_METRIC:  case MAX_ESTVAR_RATIO_METRIC:
    hf_target = estvar_ratios[metric_index] * var_H[metric_index]
              / ( convergenceTol * estvar_0[metric_index] );
    break;
  }
  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": HF target = " << hf_target << std::endl;
  return hf_target;
}


inline Real NonDNonHierarchSampling::
update_hf_target(const RealVector& estvar_ratios, size_t metric_index,
		 const RealVector& var_H)
{
  Real hf_target;
  switch (estVarMetricType) {
  case   DEFAULT_ESTVAR_METRIC:  case AVG_ESTVAR_METRIC:
  case AVG_ESTVAR_RATIO_METRIC:
    hf_target = 0.;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      hf_target += estvar_ratios[qoi] * var_H[qoi];
    hf_target /= convergenceTol * numFunctions;
    break;
  case MAX_ESTVAR_METRIC:  case MAX_ESTVAR_RATIO_METRIC:
    hf_target = estvar_ratios[metric_index] * var_H[metric_index]
              / convergenceTol;
    break;
  }
  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": HF target = " << hf_target << std::endl;
  return hf_target;
}


inline Real NonDNonHierarchSampling::
update_hf_target(const RealVector& estvar, size_t metric_index,
		 const SizetArray& N_H, const RealVector& estvar_0)
{
  // Starting from last expression above: convert estvar_ratio back to
  // incoming estvar using estvar_ratio var_H = estvar N_H
  //   N_tgt = estvar N_H / (convTol estvar_0)               [this case]

  Real hf_target;
  switch (estVarMetricType) {
  case   DEFAULT_ESTVAR_METRIC:  case AVG_ESTVAR_METRIC:
  case AVG_ESTVAR_RATIO_METRIC:
    hf_target = 0.;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      hf_target += estvar[qoi] * N_H[qoi] / estvar_0[qoi];
    hf_target /= convergenceTol * numFunctions;
    break;
  case MAX_ESTVAR_METRIC:  case MAX_ESTVAR_RATIO_METRIC:
    hf_target =    estvar[metric_index] * N_H[metric_index]
              / (estvar_0[metric_index] * convergenceTol);
    break;
  }
  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": HF target = " << hf_target << std::endl;
  return hf_target;
}


inline Real NonDNonHierarchSampling::
update_hf_target(const RealVector& estvar, size_t metric_index,
		 const SizetArray& N_H)
{
  // Starting from last expression above: convert estvar_ratio back to
  // incoming estvar using estvar_ratio var_H = estvar N_H
  //   N_tgt = estvar N_H / (convTol estvar_0)               [this case]

  Real hf_target;
  switch (estVarMetricType) {
  case   DEFAULT_ESTVAR_METRIC:  case AVG_ESTVAR_METRIC:
  case AVG_ESTVAR_RATIO_METRIC:
    hf_target = 0.;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      hf_target += estvar[qoi] * N_H[qoi];
    hf_target /= convergenceTol * numFunctions;
    break;
  case MAX_ESTVAR_METRIC:  case MAX_ESTVAR_RATIO_METRIC:
    hf_target = estvar[metric_index] * N_H[metric_index] / convergenceTol;
    break;
  }
  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": HF target = " << hf_target << std::endl;
  return hf_target;
}


inline Real NonDNonHierarchSampling::
update_hf_target(const RealVector& estvar, size_t metric_index,
		 size_t N_H, const RealVector& estvar_0)
{
  SizetArray N_H_array;  N_H_array.assign(estvar.length(), N_H);
  return update_hf_target(estvar, metric_index, N_H_array, estvar_0);
}


inline Real NonDNonHierarchSampling::
update_hf_target(const RealVector& estvar, size_t metric_index,
		 size_t N_H)
{
  SizetArray N_H_array;  N_H_array.assign(estvar.length(), N_H);
  return update_hf_target(estvar, metric_index, N_H_array);
}


inline bool NonDNonHierarchSampling::
ordered_approx_sequence(const RealVector& metric, SizetArray& approx_sequence,
			bool descending_keys)
{
  size_t i, len = metric.length(), metric_order;  bool ordered = true;
  std::multimap<Real, size_t>::iterator it;
  approx_sequence.resize(len);
  if (descending_keys) {
    std::multimap<Real, size_t, std::greater<Real> > descending_map;
    for (i=0; i<len; ++i) // keys arranged in decreasing order
      descending_map.insert(std::pair<Real, size_t>(metric[i], i));
    for (i=0, it=descending_map.begin(); it!=descending_map.end(); ++it, ++i) {
      approx_sequence[i] = metric_order = it->second;
      if (i != metric_order) ordered = false;
    }
  }
  else {
    std::multimap<Real, size_t> ascending_map; // default ascending keys
    for (i=0; i<len; ++i) // keys arranged in increasing order
      ascending_map.insert(std::pair<Real, size_t>(metric[i], i));
    for (i=0, it=ascending_map.begin(); it!=ascending_map.end(); ++it, ++i) {
      approx_sequence[i] = metric_order = it->second;
      if (i != metric_order) ordered = false;
    }
  }
  if (ordered) approx_sequence.clear();
  return ordered;
}


inline bool NonDNonHierarchSampling::
ordered_approx_sequence(const RealMatrix& metric, SizetArray& approx_sequence,
			bool descending_keys)
{
  // metric needs to be num QoI x num metrics
  RealVector avg_metric;
  average(metric, 0, avg_metric); // index 0: average over rows for each col
  return ordered_approx_sequence(avg_metric, approx_sequence, descending_keys);
}


inline bool NonDNonHierarchSampling::
ordered_approx_sequence(const RealMatrix& metric)
{
  // this checks for metric ordering for each QoI (not averaged QoI)
  size_t r, c, nr = metric.numRows(), nc = metric.numCols(), metric_order;
  std::multimap<Real, size_t> metric_map;
  std::multimap<Real, size_t>::iterator it;
  bool ordered = true;
  for (r=0; r<nr; ++r) { // numFunctions
    metric_map.clear();
    for (c=0; c<nc; ++c) // numApprox
      metric_map.insert(std::pair<Real, size_t>(metric(r,c), c));
    for (c=0, it=metric_map.begin(); it!=metric_map.end(); ++it, ++c)
      if (c != it->second) { ordered = false; break; }
    if (!ordered) break;
  }
  return ordered;
}


inline bool NonDNonHierarchSampling::
ordered_approx_sequence(const RealMatrix& metric, const UShortArray& approx_set)
{
  // this checks for metric ordering for each QoI (not averaged QoI)
  size_t r, c, nr = metric.numRows(), nc = approx_set.size(), metric_order;
  std::multimap<Real, size_t> metric_map;
  std::multimap<Real, size_t>::iterator it;
  bool ordered = true;
  for (r=0; r<nr; ++r) { // numFunctions
    metric_map.clear();
    for (c=0; c<nc; ++c) // num_approx
      metric_map.insert(std::pair<Real, size_t>(metric(r,approx_set[c]), c));
    for (c=0, it=metric_map.begin(); it!=metric_map.end(); ++it, ++c)
      if (c != it->second) { ordered = false; break; }
    if (!ordered) break;
  }
  return ordered;
}


inline void NonDNonHierarchSampling::
compute_variance(Real sum_Q, Real sum_QQ, size_t num_Q, Real& var_Q)
		 //size_t num_QQ, // this count is the same as num_Q
{
  if (num_Q <= 1)
    var_Q = (num_Q) ? 0. : std::numeric_limits<double>::quiet_NaN();
  // unbiased sample variance estimator = 1/(N-1) sum[(Q_i - Q-bar)^2]
  // = 1/(N-1) [ sum_QQ - N Q-bar^2 ] = 1/(N-1) [ sum_QQ - sum_Q^2 / N ]
  else
    var_Q = (sum_QQ - sum_Q * sum_Q / num_Q) / (num_Q - 1);

  //Cout << "compute_variance: sum_Q = " << sum_Q << " sum_QQ = " << sum_QQ
  //     << " num_Q = " << num_Q << " var_Q = " << var_Q << std::endl;
}


inline void NonDNonHierarchSampling::
compute_variance(const RealVector& sum_Q, const RealVector& sum_QQ,
		 const SizetArray& num_Q,   RealVector& var_Q)
{
  if (var_Q.empty()) var_Q.sizeUninitialized(numFunctions);

  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_variance(sum_Q[qoi], sum_QQ[qoi], num_Q[qoi], var_Q[qoi]);
}


inline void NonDNonHierarchSampling::
compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1, Real sum_Q1Q2,
		    Real sum_Q2Q2, size_t N_shared, Real& var_Q1,
		    Real& var_Q2, Real& rho2_Q1Q2)
{
  if (N_shared <= 1) {
    Real nan = std::numeric_limits<double>::quiet_NaN();
    if (N_shared) var_Q1 = var_Q2 = 0.;
    else          var_Q1 = var_Q2 = nan;
    rho2_Q1Q2 = nan;  return;
  }

  // unbiased mean estimator
  Real mu_Q1 = sum_Q1 / N_shared, mu_Q2 = sum_Q2 / N_shared,
     num_sm1 = (Real)(N_shared - 1);
  // unbiased sample covariance = 1/(N-1) sum[(X_i - X-bar)(Y_i - Y-bar)]
  // = 1/(N-1) [ sum_XY - N X-bar Y-bar ] = 1/(N-1) [ sum_XY - sum_X sum_Y / N ]
  var_Q1 = (sum_Q1Q1 - mu_Q1 * sum_Q1);       // / num_sm1;
  var_Q2 = (sum_Q2Q2 - mu_Q2 * sum_Q2);       // / num_sm1;
  Real cov_Q1Q2 = (sum_Q1Q2 - mu_Q1 * sum_Q2);// / num_sm1;

  rho2_Q1Q2 = cov_Q1Q2 / var_Q1 * cov_Q1Q2 / var_Q2; // divisors cancel
  var_Q1   /= num_sm1; // now apply divisor
  var_Q2   /= num_sm1; // now apply divisor

  //Cout << "compute_correlation: rho2_Q1Q2 = " << rho2_Q1Q2;
}


inline void NonDNonHierarchSampling::
compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2, size_t N_shared,
		   Real& cov_Q1Q2)
{
  if (N_shared <= 1)
    cov_Q1Q2 = (N_shared) ? 0. : std::numeric_limits<double>::quiet_NaN();
  else {
    // unbiased mean X-bar = 1/N * sum
    //Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1),
    //  mu_Q1 = sum_Q1 / N_shared,  mu_Q2 = sum_Q2 / N_shared;
    // unbiased sample covariance = 1/(N-1) sum[(X_i - X-bar)(Y_i - Y-bar)]
    // = 1/(N-1) [N RawMom_XY - N X-bar Y-bar] = bessel[RawMom_XY - X-bar Y-bar]
    //cov_Q1Q2 = (sum_Q1Q2 / N_shared - mu_Q1 * mu_Q2) * bessel_corr;

    // be consistent with term cancellations in compute_{variance,correlation}
    cov_Q1Q2 = (sum_Q1Q2 - sum_Q1 * sum_Q2 / N_shared) / (N_shared - 1);
  }

  //Cout << "compute_covariance: sum_Q1 = " << sum_Q1 << " sum_Q2 = " << sum_Q2
  //     << " sum_Q1Q2 = " << sum_Q1Q2 << " num_shared = " << num_shared
  //     << " cov_Q1Q2 = " << cov_Q1Q2 << std::endl;
}


inline void NonDNonHierarchSampling::
covariance_to_correlation_sq(const RealMatrix& cov_LH,
			     const RealSymMatrixArray& cov_LL,
			     const RealVector& var_H, RealMatrix& rho2_LH)
{
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;  Real var_H_q, cov_LH_aq;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    var_H_q = var_H[qoi];
    const RealSymMatrix& cov_LL_q = cov_LL[qoi];
    for (approx=0; approx<numApprox; ++approx) {
      cov_LH_aq = cov_LH(qoi,approx);
      rho2_LH(qoi,approx) = cov_LH_aq / cov_LL_q(approx,approx)
	                  * cov_LH_aq / var_H_q;
    }
  }
}


/*
inline void NonDNonHierarchSampling::
compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1, Real sum_Q1Q2,
		    Real sum_Q2Q2, size_t num_Q1, size_t num_Q2,
		    size_t num_Q1Q2, Real& var_Q1, Real& var_Q2,
		    Real& rho2_Q1Q2)
{
  // This approach has been deactivated.  While desirable to support
  // fine-grained fault tolerance, the derivation for unbiased covariance
  // via Bessel correction assumes that means, variances, and covariance
  // are computed from the same sample:
  // >> "Bessel's correction is only necessary when the population mean is
  //    unknown, and one is estimating both population mean and population
  //    variance from a given sample, using the sample mean to estimate the
  //    population mean. In that case there are n degrees of freedom in a sample
  //    of n points, and simultaneous estimation of mean and variance means one
  //    degree of freedom goes to the sample mean and the remaining n − 1
  //    degrees of freedom (the residuals) go to the sample variance. However,
  //    if the population mean is known, then the deviations of the observations
  //    from the population mean have n degrees of freedom (because the mean is
  //    not being estimated – the deviations are not residuals but errors) and
  //    Bessel's correction is not applicable."
  //
  // While it is likely possible to derive an unbiased estimation of covariance
  // using more resolved means (N_Q1 and/or N_Q2 > N_Q1Q2, with correction that
  // would fall somewhere in between Bessel correction and no correction given
  // a known mean), this complexity would cascade to higher moments and is not
  // currently justified.
  // >> One viable alternative would be to define consistent and finite
  //    sample sets PAIRWISE, but this would require migrating away from
  //    a single pass through the accumulators.  It also would result in
  //    covariance matrices that contain terms of variable accuracy.

  // The separate Bessel corrections below are insufficent since the outer
  // covariance sum is over N_Q1Q2 while the inner simplification to
  // N X-bar Y-bar requires N_Q1 and N_Q2 (these moments do not use N_Q1Q2
  // and we do not accumulate the required sums over N_Q1Q2).
  Real bessel_corr_Q1   = (Real)num_Q1   / (Real)(num_Q1   - 1),
       bessel_corr_Q2   = (Real)num_Q2   / (Real)(num_Q2   - 1),
       bessel_corr_Q1Q2 = (Real)num_Q1Q2 / (Real)(num_Q1Q2 - 1);

  // unbiased mean X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / num_Q1, mu_Q2 = sum_Q2 / num_Q2;
  // unbiased sample covariance = 1/(N-1) sum[(X_i - X-bar)(Y_i - Y-bar)]
  // = 1/(N-1) [N RawMom_XY - N X-bar Y-bar] = bessel [RawMom_XY - X-bar Y-bar]
  var_Q1 = (sum_Q1Q1 / num_Q1   - mu_Q1 * mu_Q1) * bessel_corr_Q1,
  var_Q2 = (sum_Q2Q2 / num_Q2   - mu_Q2 * mu_Q2) * bessel_corr_Q2;
  Real cov_Q1Q2 = (sum_Q1Q2 / num_Q1Q2 - mu_Q1 * mu_Q2) * bessel_corr_Q1Q2;

  rho2_Q1Q2 = cov_Q1Q2 / var_Q1 * cov_Q1Q2 / var_Q2;
}


inline void NonDNonHierarchSampling::
compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2, size_t num_Q1,
		   size_t num_Q2, size_t num_Q1Q2, Real& cov_Q1Q2)
{
  // This approach is deprecated (see compute_correlation above)

  Real //bessel_corr_Q1 = (Real)num_Q1   / (Real)(num_Q1   - 1),
       //bessel_corr_Q2 = (Real)num_Q2   / (Real)(num_Q2   - 1),
       bessel_corr_Q1Q2 = (Real)num_Q1Q2 / (Real)(num_Q1Q2 - 1);

  // unbiased mean X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / num_Q1,  mu_Q2 = sum_Q2 / num_Q2;
  // unbiased sample covariance = 1/(N-1) sum[(X_i - X-bar)(Y_i - Y-bar)]
  // = 1/(N-1) [N RawMom_XY - N X-bar Y-bar] = bessel [RawMom_XY - X-bar Y-bar]
  cov_Q1Q2 = (sum_Q1Q2 / num_Q1Q2 - mu_Q1 * mu_Q2) * bessel_corr_Q1Q2;

  //Cout << "compute_covariance: sum_Q1 = " << sum_Q1 << " sum_Q2 = " << sum_Q2
  //     << " sum_Q1Q2 = " << sum_Q1Q2 << " num_Q1 = " << num_Q1 << " num_Q2 = "
  //     << num_Q2 << " num_Q1Q2 = " << num_Q1Q2 << " cov_Q1Q2 = " << cov_Q1Q2
  //     << std::endl;
}
*/


inline void NonDNonHierarchSampling::
apply_control(Real sum_L_shared, size_t num_L_shared, Real sum_L_refined,
	      size_t num_L_refined, Real beta, Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom -= beta * (sum_L_shared  / num_L_shared - // mu from shared samples
		       sum_L_refined / num_L_refined);// refined mu w/ increment

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "apply_control(): sum_L_sh = " << sum_L_shared
	 << " sum_L_ref = " << sum_L_refined
	 << " num_L_sh = "  << num_L_shared
	 << " num_L_ref = " << num_L_refined << std::endl;
}


inline bool NonDNonHierarchSampling::active_set_for_model(size_t i)
{
  size_t qoi, start = numFunctions*i, end = start+numFunctions;
  const ShortArray& asv = activeSet.request_vector();
  for (qoi=start; qoi<end; ++qoi)
    if (asv[qoi])
      return true;
  return false;
}


inline void NonDNonHierarchSampling::inflate(size_t N_0D, SizetArray& N_1D)
{ N_1D.assign(numApprox, N_0D); }


inline void NonDNonHierarchSampling::
inflate(size_t N_0D, SizetArray& N_1D, const UShortArray& approx_set)
{
  N_1D.assign(numApprox, 0);
  size_t i, num_approx = approx_set.size();
  for (i=0; i<num_approx; ++i)
    N_1D[approx_set[i]] = N_0D;
}


inline void NonDNonHierarchSampling::
inflate(const SizetArray& N_1D, Sizet2DArray& N_2D)
{
  N_2D.resize(numApprox);
  for (size_t approx=0; approx<numApprox; ++approx)
    N_2D[approx] = N_1D;
}


inline void NonDNonHierarchSampling::
inflate(const SizetArray& N_1D, Sizet2DArray& N_2D,
	const UShortArray& approx_set)
{
  N_2D.clear();
  N_2D.resize(numApprox);
  size_t i, num_approx = approx_set.size();
  for (i=0; i<num_approx; ++i)
    N_2D[approx_set[i]] = N_1D;
  // Not needed so long as empty 1D arrays are allowed:
  //for (i=0; i<numApprox; ++i)
  //  if (N_2D[i].empty())
  //    N_2D[i].assign(numFunctions, 0);
}


inline void NonDNonHierarchSampling::
inflate(const RealVector& avg_eval_ratios, RealMatrix& eval_ratios)
{
  // inflate avg_eval_ratios back to eval_ratios
  size_t qoi, approx;  Real r_i, *eval_ratios_a;
  for (approx=0; approx<numApprox; ++approx) {
    r_i = avg_eval_ratios[approx];
    eval_ratios_a = eval_ratios[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      eval_ratios_a[qoi] = r_i;
  }
}


inline void NonDNonHierarchSampling::
inflate(Real r_i, size_t num_rows, Real* eval_ratios_col)
{
  // inflate scalar to column vector
  for (size_t row=0; row<num_rows; ++row)
    eval_ratios_col[row] = r_i;
}


inline void NonDNonHierarchSampling::
inflate(const RealVector& vec, const BitArray& mask, RealVector& inflated_vec)
{
  if (mask.empty())
    copy_data(vec, inflated_vec);
  else {
    size_t i, cntr = 0, inflated_len = mask.size();
    inflated_vec.size(inflated_len); // init to 0
    for (i=0; i<inflated_len; ++i)
      if (mask[i])
	inflated_vec[i] = vec[cntr++];
  }
}


/*
inline void NonDNonHierarchSampling::
inflate(const RealMatrix& mat, const BitArray& mask, RealMatrix& inflated_mat)
{
  if (mask.empty())
    copy_data(mat, inflated_mat);
  else {
    size_t i, j, i_cntr = 0, j_cntr, inflated_len = mask.size();
    if (inflated_mat.numRows() != inflated_len ||
	inflated_mat.numCols() != inflated_len)
      inflated_mat.shape(inflated_len, inflated_len); // init to 0
    else inflated_mat = 0.;
    for (i=0; i<inflated_len; ++i)
      if (mask[i]) {
	for (j=0, j_cntr=0; j<inflated_len; ++j)
	  if (mask[j])
	    { inflated_mat(i,j) = mat(i_cntr,j_cntr); ++j_cntr; }
	++i_cntr;
      }
  }
}


inline void NonDNonHierarchSampling::
inflate(const RealSymMatrix& mat, const BitArray& mask,
	RealSymMatrix& inflated_mat)
{
  if (mask.empty())
    copy_data(mat, inflated_mat);
  else {
    size_t i, j, i_cntr = 0, j_cntr, inflated_len = mask.size();
    if (inflated_mat.numRows() != inflated_len)
      inflated_mat.shape(inflated_len); // init to 0
    else inflated_mat = 0.;
    for (i=0; i<inflated_len; ++i)
      if (mask[i]) {
	for (j=0, j_cntr=0; j<=i; ++j)
	  if (mask[j])
	    { inflated_mat(i,j) = mat(i_cntr,j_cntr); ++j_cntr; }
	++i_cntr;
      }
  }
}
*/


inline void NonDNonHierarchSampling::
deflate(const RealVector& vec, const BitArray& mask, RealVector& deflated_vec)
{
  if (mask.empty())
    copy_data(vec, deflated_vec);
  else {
    size_t i, cntr = 0, len = vec.length(), deflate_len = mask.count();
    deflated_vec.sizeUninitialized(deflate_len); // init to 0
    for (i=0; i<len; ++i)
      if (mask[i])
	deflated_vec[cntr++] = vec[i];
  }
}


inline void NonDNonHierarchSampling::
deflate(const SizetArray& vec, const BitArray& mask, RealVector& deflated_vec)
{
  if (mask.empty())
    copy_data(vec, deflated_vec);
  else {
    size_t i, cntr = 0, len = vec.size(), deflate_len = mask.count();
    deflated_vec.sizeUninitialized(deflate_len); // init to 0
    for (i=0; i<len; ++i)
      if (mask[i])
	deflated_vec[cntr++] = (Real)vec[i];
  }
}


/*
inline void NonDNonHierarchSampling::enforce_nudge(RealVector& x)
{
  size_t i, len = x.length();
  Real lb = //(maxFunctionEvals == SZ_MAX) ?
    RATIO_NUDGE;
  //RATIO_NUDGE * std::sqrt(maxFunctionEvals); // hand-tuned heuristic
  for (i=0; i<len; ++i)
    if (x[i] < lb)
      x[i] = lb;
}
*/

} // namespace Dakota

#endif
