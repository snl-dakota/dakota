/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_GEN_ACV_SAMPLING_H
#define NOND_GEN_ACV_SAMPLING_H

#include "NonDACVSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {


/// Perform Generalized Approximate Control Variate Monte Carlo sampling.

/** Generalized versions of Approximate Control Variate (ACV) that
    enumerate different model graphs (CV inter-relationships) and
    different model memberships. (Bomarito et al., 2022, JCP) */

class NonDGenACVSampling: public NonDACVSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDGenACVSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model);
  /// destructor
  ~NonDGenACVSampling() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run() override;
  void core_run() override;
  //void post_run(std::ostream& s) override;
  //void print_results(std::ostream& s,
  //                   short results_state = FINAL_RESULTS) override;

  Real inactive_budget_deduction() const override;

  void numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
				 size_t& num_nln_con) override;
  void numerical_solution_bounds_constraints(const MFSolutionData& soln,
    RealVector& x0, RealVector& x_lb, RealVector& x_ub,
    RealVector& lin_ineq_lb, RealVector& lin_ineq_ub, RealVector& lin_eq_tgt,
    RealVector& nln_ineq_lb, RealVector& nln_ineq_ub, RealVector& nln_eq_tgt,
    RealMatrix& lin_ineq_coeffs, RealMatrix& lin_eq_coeffs) override;

  void minimizer_results_to_solution_data(const RealVector& cv_star,
					  const RealVector& fn_star,
					  MFSolutionData& soln) override;

  Real linear_model_cost(const RealVector& N_vec) override;
  Real nonlinear_model_cost(const RealVector& r_and_N) override;
  void linear_model_cost_gradient(const RealVector& N_vec,
				  RealVector& grad_c) override;
  void nonlinear_model_cost_gradient(const RealVector& r_and_N,
				     RealVector& grad_c) override;
				     

  size_t num_approximations() const override;
  

  const MFSolutionData& final_solution_data() const override;

  void print_variance_reduction(std::ostream& s) const override;

  void estimator_variance_ratios(const RealVector& cd_vars,
				 RealVector& estvar_ratios) override;

  void augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				       RealVector& lin_ineq_lb,
				       RealVector& lin_ineq_ub) override;
  void enforce_augmented_linear_ineq_constraints(RealVector& cd_vars) override;

  //
  //- Heading: member functions
  //

private:

  //
  //- Heading: Helper functions
  //

  /// generate sets of DAGs for the relevant combinations of active
  /// approximations
  void generate_ensembles_dags();
  /// remove sets of DAGs for discarded models after an online iteration
  void prune_ensembles(const UShortArray& active_approx_set);

  /// generate a set of DAGs for the provided root and subordinate nodes
  void generate_dags(unsigned short root, const UShortArray& nodes,
		     UShortArraySet& dag_set);
  /// recursively generate sub-trees for current root and subordinate nodes
  void generate_sub_trees(unsigned short root, const UShortArray& nodes,
			  unsigned short depth, UShortArray& dag,
			  UShortArraySet& model_dags);
  /// recursively generate sub-trees for current root and subordinate nodes,
  /// restricted to hierarchical DAGs (width = 1)
  void generate_hierarchical_sub_trees(unsigned short root,
				       const UShortArray& nodes,
				       unsigned short depth, UShortArray& dag,
				       UShortArraySet& model_dags);
  /// for the given DAG and active approimation set, generate the sets
  /// of source nodes that point to given targets
  void generate_reverse_dag(const UShortArray& approx_set,
			    const UShortArray& dag);
  /// create an ordered list of roots that enable ordered sample increments
  /// by ensuring that root sample levels are defined
  void unroll_reverse_dag_from_root(unsigned short root,
				    UShortList& ordered_list);
  /// create an ordered list of roots that enable ordered sample increments
  /// by ensuring that root sample levels are defined
  void unroll_reverse_dag_from_root(unsigned short root,
				    UShortArray& group);
  /// create an ordered list of roots that enable ordered sample increments
  /// by ensuring that root sample levels are defined (overloaded version
  /// factors in the over-sample ratios)
  void unroll_reverse_dag_from_root(unsigned short root,
				    const RealVector& avg_eval_ratios,
				    UShortList& root_list);

  void generalized_acv_online_pilot();
  void generalized_acv_offline_pilot();
  void generalized_acv_pilot_projection();

  void no_solve_solution();

  void approx_increments(IntRealMatrixMap& sum_L_baseline,
			 const SizetArray& N_H_actual, size_t N_H_alloc,
			 IntRealMatrixMap& sum_L_shared,
			 Sizet2DArray& N_L_actual_shared,
			 IntRealMatrixMap& sum_L_refined,
			 Sizet2DArray& N_L_actual_refined,
			 SizetArray& N_L_alloc, const MFSolutionData& soln);

  void update_model_groups();
  void update_model_groups(const UShortList& root_list);

  bool precompute_allocations();
  void compute_allocations(MFSolutionData& solution);

  void genacv_raw_moments(const IntRealMatrixMap& sum_L_covar,
			  const IntRealVectorMap& sum_H_covar,
			  const IntRealSymMatrixArrayMap& sum_LL_covar,
			  const IntRealMatrixMap& sum_LH_covar,
			  const SizetArray& N_covar,
			  const IntRealVectorMap& sum_H_baseline,
			  const SizetArray& N_baseline,
			  const IntRealMatrixMap& sum_L_shared,
			  const Sizet2DArray& N_L_shared, 
			  const IntRealMatrixMap& sum_L_refined,
			  const Sizet2DArray& N_L_refined,
			  const MFSolutionData& soln);

  void precompute_genacv_controls(const RealVector& soln_vars);

  void compute_genacv_control(const RealMatrix& sum_L_m, Real sum_H_mq,
			      const RealSymMatrix& sum_LL_mq,
			      const RealMatrix& sum_LH_m, size_t N_shared_q,
			      size_t mom, size_t qoi,
			      const UShortArray& approx_set, RealVector& beta);

  void analytic_initialization_from_mfmc(const UShortArray& approx_set,
					 const RealMatrix& rho2_LH,
					 Real avg_N_H, MFSolutionData& soln);
  void analytic_initialization_from_ensemble_cvmc(const UShortArray& approx_set,
						  const UShortArray& dag,
						  const UShortList& root_list,
						  const RealMatrix& rho2_LH,
						  Real avg_N_H,
						  MFSolutionData& soln);
  void cvmc_ensemble_solutions(const RealSymMatrixArray& cov_LL,
			       const RealMatrix& cov_LH,
			       const RealVector& var_H, const RealVector& cost,
			       const UShortArray& approx_set,
			       const UShortArray& dag,
			       const UShortList& root_list,
			       RealVector& avg_eval_ratios);

  void compute_parameterized_G_g(const RealVector& N_vec);
  void unroll_z1_z2(const RealVector& N_vec, RealVector& z1, RealVector& z2);

  /*
  void invert_C_G_matrix(const RealSymMatrix& C, const RealSymMatrix& G,
			 RealSymMatrix& C_G_inv);
  void compute_c_g_vector(const RealMatrix& c, size_t qoi, const RealVector& g,
			  RealVector& c_g);
  Real compute_R_sq(const RealSymMatrix& C_G_inv, const RealVector& c_g,
		    Real var_H_q, Real N_H);
  */
  void compute_C_G_c_g(const RealSymMatrix& C, const RealSymMatrix& G,
		       const RealMatrix&    c, const RealVector& g,
		       size_t qoi,             const UShortArray& approx_set,
		       RealSymMatrix& C_G,     RealVector& c_g);
  void solve_for_C_G_c_g(RealSymMatrix& C_G, RealVector& c_g, RealVector& lhs,
			 bool copy_C_G = true, bool copy_c_g = true);
  Real solve_for_triple_product(const RealSymMatrix& C,	const RealSymMatrix& G,
				const RealMatrix&    c, const RealVector& g,
				size_t qoi, const UShortArray& approx_set);
  Real compute_R_sq(const RealSymMatrix& C, const RealSymMatrix& G,
		    const RealMatrix&    c, const RealVector& g, size_t qoi,
		    const UShortArray& approx_set, Real var_H_q, Real N_H);

  void accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared,
			      IntRealMatrixMap& sum_L_refined,
			      Sizet2DArray& N_L_shared,
			      Sizet2DArray& N_L_refined, unsigned short root,
			      const UShortSet& reverse_dag);
  void accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared,
			      IntRealMatrixMap& sum_L_refined,
			      Sizet2DArray& N_L_shared,
			      Sizet2DArray& N_L_refined,
			      const SizetArray& approx_sequence,
			      size_t sequence_start, size_t sequence_end);
  void accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared,
			      IntRealVectorMap& sum_H,
			      IntRealSymMatrixArrayMap& sum_LL,
			      IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			      SizetArray& N_H_shared, Sizet2DArray& N_L_shared);

  //bool genacv_approx_increment(const MFSolutionData& soln,
  // 			         const Sizet2DArray& N_L_actual_refined,
  // 			         SizetArray& N_L_alloc, size_t iter,
  //                             const SizetArray& approx_sequence,
  // 			         size_t start, size_t end);

  void solve_for_genacv_control(const RealSymMatrix& cov_LL,
				const RealSymMatrix& G,
				const RealMatrix& cov_LH, const RealVector& g,
				size_t qoi, const UShortArray& approx_set,
				RealVector& beta);

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& avg_hf_target,
		       const UShortArray& approx_set,
		       const UShortList& root_list, Real budget,
		       Real offline_N_lwr = 1.);

  void enforce_augmented_linear_ineq_constraints(RealVector& avg_eval_ratios,
						 const UShortArray& approx_set,
						 const UShortList& root_list);

  void update_best(MFSolutionData& solution);
  void print_best();
  void restore_best();
  //void reset_acv();

  void inflate_approx_set(const UShortArray& approx_set, SizetArray& index_map);
  void inflate_variables(const RealVector& cd_vars, RealVector& N_vec,
			 const UShortArray& approx_set);

  // pretty-print DAG using right arrows from approx set
  void print_dag(const UShortArray& dag, const UShortArray& approx_set) const;

  //
  //- Heading: Data
  //

  /// sample allocations per approximation (differs from actual if failed sims;
  /// cached for use in inactive_budget_deduction())
  SizetArray NApproxAlloc;

  /// the "G" matrix in Bomarito et al.
  RealSymMatrix GMat;
  /// the "g" vector in Bomarito et al.
  RealVector gVec;

  //size_t compGgCntr; // debugging aide for finding a specific G,g solve

  /// type of tunable recursion for defining set of DAGs: KL, partial, or full
  short dagRecursionType;
  /// depth throttle for constraining set from generate_dags()
  unsigned short dagDepthLimit;
  /// width throttle for constraining set from generate_dags()
  /// (used only for MFMC with width = 1)
  unsigned short dagWidthLimit;
  /// option to enumerate combinations of approximation models
  short modelSelectType;

  /// mapping from a key of active model nodes to the set of admissible DAGs
  /// that define the control variate targets for each model in the ensemble
  std::map<UShortArray, UShortArraySet> modelDAGs;
  /// the active instance from enumeration of UShortArray keys in modelDAGs
  std::map<UShortArray, UShortArraySet>::const_iterator activeModelSetIter;
  /// the active instance from enumeration of UShortArraySets for each key
  /// in modelDAGs
  UShortArraySet::const_iterator activeDAGIter;

  /// reverse of active DAG: for each model, the set of models that point to it
  UShortSetArray reverseActiveDAG;
  /// an ordered set of root nodes that ensures targets are defined when
  /// unrolling dependent sources; allows unrolling of z^1,z^2 sample sets
  UShortList orderedRootList;

  /// the best performing model subset when model selection is active
  std::map<UShortArray, UShortArraySet>::const_iterator bestModelSetIter;
  /// the best performing model graph among the set from generate_dags()
  UShortArraySet::const_iterator bestDAGIter;
  /// the merit function value for the best solution, incorporating both
  /// estimator variance and budget (objective and constraint in some order)
  Real meritFnStar;

  /// book-keeping of previous numerical optimization solutions for each DAG;
  /// used for warm starting
  std::map<std::pair<UShortArray, UShortArray>, MFSolutionData> dagSolns;
};


inline Real NonDGenACVSampling::inactive_budget_deduction() const
{
  // NLevAlloc[mf][rl] not avail until finalize_counts() --> cache NApproxAlloc

  const UShortArray& approx_set = activeModelSetIter->first;
  size_t num_approx = approx_set.size();
  if (num_approx == numApprox) return 0.;

  size_t lf_form_index, lf_lev_index, cntr = 0, N_L_alloc_a;
  Real cost_H = sequenceCost[numApprox], deduct = 0.;
  for (size_t approx=0; approx<numApprox; ++approx)
    if  (cntr < num_approx && approx == approx_set[cntr]) ++cntr; // ordered seq
    else deduct += sequenceCost[approx] * NApproxAlloc[approx] / cost_H;

  //Cout << "inactive budget deduction = " << deduct << std::endl;
  return deduct;
}


inline size_t NonDGenACVSampling::num_approximations() const
{ return activeModelSetIter->first.size(); }


inline const MFSolutionData& NonDGenACVSampling::final_solution_data() const
{
  std::pair<UShortArray, UShortArray>
    key(activeModelSetIter->first, *activeDAGIter);
  return dagSolns.at(key);
}


inline void NonDGenACVSampling::print_variance_reduction(std::ostream& s) const
{ print_estimator_performance(s, final_solution_data()); }


/*
inline void NonDGenACVSampling::
invert_C_G_matrix(const RealSymMatrix& C, const RealSymMatrix& G,
		  RealSymMatrix& C_G_inv)
{
  size_t i, j, n = C.numRows();
  if (C_G_inv.empty()) C_G_inv.shapeUninitialized(n);

  for (i=0; i<n; ++i)
    for (j=0; j<n; ++j)
      C_G_inv(i,j) = C(i,j) * G(i,j); // Ok for RealSymMatrix

  RealSpdSolver spd_solver;
  spd_solver.setMatrix(Teuchos::rcp(&C_G_inv, false));
  // Note: equilibration should not be used outside of the solve() context,
  // as the resulting inverse would be for the equilibrated matrix.  See
  // discussion in NonDMultilevBLUESampling::compute_C_inverse().
  int code = spd_solver.invert(); // inverts in place using factorization
  if (code) {
    Cerr << "Error: serial dense matrix inversion failure (LAPACK error code "
	 << code << ") in NonDGenACVSampling::invert_C_G_matrix()."<<std::endl;
    abort_handler(METHOD_ERROR);
  }
}


inline void NonDGenACVSampling::
compute_c_g_vector(const RealMatrix& c, size_t qoi, const RealVector& g,
		   RealVector& c_g)
{
  size_t i, num_approx = g.length();
  if (c_g.length() != num_approx) c_g.sizeUninitialized(num_approx);

  for (i=0; i<num_approx; ++i) // {g} o {c}
    c_g[i] = c(qoi, i) * g[i];
}


inline Real NonDGenACVSampling::
compute_R_sq(const RealSymMatrix& C_G_inv, const RealVector& c_g,
	     Real var_H_q, Real N_H)
{
  //RealSymMatrix trip(1, false);
  //Teuchos::matTripleProduct(Teuchos::TRANS, 1./var_H_q, CF_inv, A, trip);
  //R_sq_q = trip(0,0);

  // compute triple product: Cov(\Delta,\hat{Q}_H)^T Cov(\Delta,\Delta)
  //                         Cov(\Delta,\hat{Q}_H) = c_g^T C_G_inv c_g
  // Est var = Var(\hat{Q}_H) - triple product = var_H_q / N_H - triple product
  // Est var = var_H_q / N_H ( 1 - N_H * triple product / var_H_q )
  // R^2     = N_H * triple product / var_H_q
  size_t i, j, num_approx = c_g.length();  Real sum, trip_prod = 0.;
  for (i=0; i<num_approx; ++i) {
    sum = 0.;
    for (j=0; j<num_approx; ++j)
      sum += C_G_inv(i,j) * c_g[j];
    trip_prod += c_g[i] * sum;
  }
  return trip_prod * N_H / var_H_q;
}


inline void NonDGenACVSampling::
compute_genacv_control(const RealSymMatrix& cov_LL, const RealSymMatrix& G,
		       const RealMatrix& cov_LH, const RealVector& g,
		       size_t qoi, RealVector& beta)
{
  RealSymMatrix C_G_inv;  invert_C_G_matrix(covLL[qoi], G, C_G_inv);
  //Cout << "compute_genacv_control qoi " << qoi+1 << ": C_G_inv\n" << C_G_inv;
  RealVector c_g;      compute_c_g_vector(covLH, qoi, g, c_g);
  //Cout << "compute_genacv_control qoi " << qoi+1 << ": c_g\n" << c_g;

  size_t n = G.numRows();
  if (beta.length() != n) beta.size(n);
  beta.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., C_G_inv, c_g, 0.);
  //Cout << "compute_acv_control qoi " << qoi+1 << ": beta\n" << beta;
}


inline void NonDGenACVSampling::
compute_C_G_c_g(const RealSymMatrix& C, const RealSymMatrix& G,
		const RealMatrix&    c, const RealVector& g,
		size_t qoi, RealSymMatrix& C_G, RealVector& c_g)
{
  size_t i, j, n = C.numRows();
  C_G.shapeUninitialized(n);  c_g.sizeUninitialized(n);
  for (i=0; i<n; ++i) {
    c_g[i] = c(qoi, i) * g[i];
    for (j=0; j<=i; ++j)
      C_G(i,j) = C(i,j) * G(i,j); // Ok for RealSymMatrix
  }
}
*/


inline void NonDGenACVSampling::
inflate_approx_set(const UShortArray& approx_set, SizetArray& index_map)
{
  // inflate from compact approx_set to index_map[0,numApprox)
  size_t i, num_approx_set = approx_set.size();
  index_map.assign(numApprox, SZ_MAX);
  for (i=0; i<num_approx_set; ++i)
    index_map[approx_set[i]] = i; // maps src/tgt from inflated to compact
}


inline void NonDGenACVSampling::
inflate_variables(const RealVector& cd_vars, RealVector& N_vec,
		  const UShortArray& approx_set)
{
  size_t i, num_approx = approx_set.size(), num_cdv = cd_vars.length();
  if  (N_vec.length() == numGroups) N_vec = 0.;
  else N_vec.size(numGroups);
  for (i=0; i<num_approx; ++i)
    N_vec[approx_set[i]] = cd_vars[i];
  if (num_cdv == num_approx + 1)
    N_vec[numApprox] = cd_vars[num_approx];
  else {
    // N_H not provided so pull from latest counter values
    size_t hf_form_index, hf_lev_index;
    hf_indices(hf_form_index, hf_lev_index);
    // estimator_variance_metric() uses actual (not alloc) to sync with varH
    // so use same prior to defining G,g in precompute_genacv_controls() and
    // estimator_variance_ratios()
    N_vec[numApprox] = //(backfillFailures) ?
      average(NLevActual[hf_form_index][hf_lev_index]);// :
      //NLevAlloc[hf_form_index][hf_lev_index];
  }
}


inline void NonDGenACVSampling::
compute_C_G_c_g(const RealSymMatrix& C, const RealSymMatrix& G,
		const RealMatrix&    c, const RealVector& g,
		size_t qoi,             const UShortArray& approx_set,
		RealSymMatrix& C_G,     RealVector& c_g)
{
  size_t i, j, n = G.numRows();  unsigned short approx_i;
  C_G.shapeUninitialized(n);  c_g.sizeUninitialized(n);
  for (i=0; i<n; ++i) {
    approx_i = approx_set[i];
    c_g[i] = c(qoi, approx_i) * g[i];
    for (j=0; j<=i; ++j)
      C_G(i,j) = C(approx_i,approx_set[j]) * G(i,j); // Ok for RealSymMatrix
  }
}


inline Real NonDGenACVSampling::
solve_for_triple_product(const RealSymMatrix& C, const RealSymMatrix& G,
			 const RealMatrix&    c, const RealVector& g,
			 size_t qoi, const UShortArray& approx_set)
{
  RealSymMatrix C_G;  RealVector c_g, lhs;
  compute_C_G_c_g(C, G, c, g, qoi, approx_set, C_G, c_g);
  solve_for_C_G_c_g(C_G, c_g, lhs, false, true); // retain c_g for use below

  size_t i, n = G.numRows();
  Real trip_prod = 0.;
  for (i=0; i<n; ++i)
    trip_prod += c_g(i) * lhs(i);
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "GenACV::solve_for_triple_product(): C-G =\n" << C_G
  // 	   << "RHS c-g =\n" << c_g << "LHS soln =\n" << lhs
  // 	   << "triple product = " << trip_prod << std::endl;
  return trip_prod;
}


inline Real NonDGenACVSampling::
compute_R_sq(const RealSymMatrix& C, const RealSymMatrix& G,
	     const RealMatrix& c, const RealVector& g, size_t qoi,
	     const UShortArray& approx_set, Real var_H_q, Real N_H)
{ return solve_for_triple_product(C, G, c, g, qoi, approx_set) * N_H / var_H_q;}


inline void NonDGenACVSampling::
precompute_genacv_controls(const RealVector& soln_vars)
{
  // Note: while G,g have a more explicit dependence on N_shared[qoi] than F,
  // we mirror the averaged sample allocations and compute G,g once
  RealVector inflate_soln_vars;
  inflate_variables(soln_vars, inflate_soln_vars, activeModelSetIter->first);
  compute_parameterized_G_g(inflate_soln_vars);
}


inline void NonDGenACVSampling::
solve_for_genacv_control(const RealSymMatrix& cov_LL, const RealSymMatrix& G,
			 const RealMatrix& cov_LH, const RealVector& g,
			 size_t qoi, const UShortArray& approx_set,
			 RealVector& beta)
{
  RealSymMatrix C_G;  RealVector c_g;
  compute_C_G_c_g(cov_LL, G, cov_LH, g, qoi, approx_set, C_G, c_g);
  solve_for_C_G_c_g(C_G, c_g, beta, false, false); // Ok to modify C_G,c_g

  //Cout << "compute_genacv_control qoi " << qoi+1 << ": C_G\n" << C_G
  //     << "c_g\n" << c_g << "beta\n" << beta;
}


inline void NonDGenACVSampling::
compute_genacv_control(const RealMatrix& sum_L_m, Real sum_H_mq,
		       const RealSymMatrix& sum_LL_mq,
		       const RealMatrix& sum_LH_m, size_t N_shared_q,
		       size_t mom, size_t qoi, const UShortArray& approx_set,
		       RealVector& beta)
{
  if (mom == 1) // online|offline covariances available for mean
    solve_for_genacv_control(covLL[qoi], GMat, covLH, gVec, qoi,
			     approx_set, beta);
  else { // compute variances/covariances for higher-order moment estimators
    // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
    // > cov_LH is sized for all qoi but only 1 row is used
    RealSymMatrix cov_LL_mq; RealMatrix cov_LH_m;
    compute_acv_control_covariances(sum_L_m, sum_H_mq, sum_LL_mq, sum_LH_m,
				    N_shared_q, qoi, cov_LL_mq, cov_LH_m);
    solve_for_genacv_control(cov_LL_mq, GMat, cov_LH_m, gVec, qoi,
			     approx_set, beta);
  }
}


/*
inline void NonDGenACVSampling::reset_acv()
{
  // from pre_run() up the hierarchy:
  mlmfIter = numLHSRuns = deltaNActualHF = 0;
  equivHFEvals = deltaEquivHF = 0.;
  seedSpec = randomSeed = seed_sequence(0); // (re)set seeds to sequence

  // Moved inside main loop:
  //numSamples = pilotSamples[numApprox];
  // Note: other sample counters are reset at top of each acv_*_pilot() call
}
*/


inline void NonDGenACVSampling::
print_dag(const UShortArray& dag, const UShortArray& approx_set) const
{
  size_t a, num_approx = approx_set.size();
  for (a=0; a<num_approx; ++a)
    Cout << "                     " << approx_set[a] << " -> " << dag[a] <<'\n';
  //Cout << std::endl;
}

} // namespace Dakota

#endif
