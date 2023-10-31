/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
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
  NonDGenACVSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDGenACVSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  void numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
				 size_t& num_nln_con);
  void numerical_solution_bounds_constraints(const DAGSolutionData& soln,
    const RealVector& cost, Real avg_N_H, RealVector& x0, RealVector& x_lb,
    RealVector& x_ub, RealVector& lin_ineq_lb, RealVector& lin_ineq_ub,
    RealVector& lin_eq_tgt, RealVector& nln_ineq_lb, RealVector& nln_ineq_ub,
    RealVector& nln_eq_tgt, RealMatrix& lin_ineq_coeffs,
    RealMatrix& lin_eq_coeffs);
  void finite_solution_bounds(const RealVector& cost, Real avg_N_H,
			      RealVector& x_lb, RealVector& x_ub);

  void recover_results(const RealVector& cv_star, const RealVector& fn_star,
		       Real& avg_estvar, RealVector& avg_eval_ratios,
		       Real& avg_hf_target, Real& equiv_hf_cost);

  Real linear_cost(const RealVector& N_vec);
  Real nonlinear_cost(const RealVector& r_and_N);
  void linear_cost_gradient(const RealVector& N_vec,RealVector& grad_c);
  void nonlinear_cost_gradient(const RealVector& r_and_N,
				       RealVector& grad_c);

  size_t num_approximations() const;

  Real estimator_accuracy_metric();
  //Real estimator_cost_metric();

  void print_variance_reduction(std::ostream& s);

  void estimator_variance_ratios(const RealVector& N_vec,
				 RealVector& estvar_ratios);

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

private:

  //
  //- Heading: Helper functions
  //

  /// generate sets of DAGs for the relevant combinations of active
  /// approximations
  void generate_ensembles_dags();
  /// generate a set of DAGs for the provided root and subordinate nodes
  void generate_dags(unsigned short root, const UShortArray& nodes,
		     UShortArraySet& dag_set);
  /// recursively generate sub-trees for current root and subordinate nodes
  void generate_sub_trees(unsigned short root, const UShortArray& nodes,
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
  /// by ensuring that root sample levels are defined (overloaded version
  /// factors in the over-sample ratios)
  void unroll_reverse_dag_from_root(unsigned short root,
				    const RealVector& avg_eval_ratios,
				    UShortList& root_list);

  void generalized_acv_online_pilot();
  void generalized_acv_offline_pilot();
  void generalized_acv_pilot_projection();

  void approx_increments(IntRealMatrixMap& sum_L_baselineH,
			 IntRealVectorMap& sum_H,
			 IntRealSymMatrixArrayMap& sum_LL,
			 IntRealMatrixMap& sum_LH, const SizetArray& N_H_actual,
			 size_t N_H_alloc, const DAGSolutionData& soln);

  void precompute_ratios();
  void compute_ratios(const RealMatrix& var_L, DAGSolutionData& solution);

  void genacv_raw_moments(IntRealMatrixMap& sum_L_baseline,
			  IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  IntRealVectorMap& sum_H,
			  IntRealSymMatrixArrayMap& sum_LL,
			  IntRealMatrixMap& sum_LH,
			  const RealVector& avg_eval_ratios,
			  const Sizet2DArray& N_L_shared,
			  const Sizet2DArray& N_L_refined,
			  const SizetArray& N_H, RealMatrix& H_raw_mom);

  void precompute_genacv_control(const RealVector& avg_eval_ratios,
				 const SizetArray& N_shared);
  void compute_genacv_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
			      RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
			      size_t N_shared_q, size_t mom, size_t qoi,
			      const UShortArray& approx_set, RealVector& beta);

  void analytic_initialization_from_mfmc(const UShortArray& approx_set,
					 Real avg_N_H, DAGSolutionData& soln);
  void analytic_initialization_from_ensemble_cvmc(const UShortArray& approx_set,
						  const UShortArray& dag,
						  const UShortList& root_list,
						  Real avg_N_H,
						  DAGSolutionData& soln);
  void cvmc_ensemble_solutions(const RealSymMatrixArray& cov_LL,
			       const RealMatrix& cov_LH,
			       const RealVector& var_H, const RealVector& cost,
			       const UShortArray& approx_set,
			       const UShortArray& dag,
			       const UShortList& root_list,
			       DAGSolutionData& soln);

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

  bool genacv_approx_increment(const DAGSolutionData& soln,
			       const Sizet2DArray& N_L_actual_refined,
			       SizetArray& N_L_alloc_refined,
			       size_t iter, const SizetArray& approx_sequence,
			       size_t start, size_t end);
  bool genacv_approx_increment(const DAGSolutionData& soln,
			       const Sizet2DArray& N_L_actual_refined,
			       SizetArray& N_L_alloc_refined,
			       size_t iter, unsigned short root,
			       const UShortSet& reverse_dag_set);

  void solve_for_genacv_control(const RealSymMatrix& cov_LL,
				const RealSymMatrix& G,
				const RealMatrix& cov_LH, const RealVector& g,
				size_t qoi, const UShortArray& approx_set,
				RealVector& beta);

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& avg_hf_target,
		       const UShortArray& approx_set,
		       const UShortList&  root_list);
  void enforce_linear_ineq_constraints(RealVector& avg_eval_ratios,
				       const UShortArray& approx_set,
				       const UShortList& root_list);

  void update_best(DAGSolutionData& solution);
  void restore_best();
  //void reset_acv();

  bool valid_variance(Real var) const;

  void inflate_approx_set(const UShortArray& approx_set, SizetArray& index_map);
  void inflate_variables(const RealVector& cd_vars, RealVector& N_vec,
			 const UShortArray& approx_set);

  //
  //- Heading: Data
  //

  /// the "G" matrix in Bomarito et al.
  RealSymMatrix GMat;
  /// the "g" vector in Bomarito et al.
  RealVector gVec;

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

  /// the best performing model graph among the set from generate_dags()
  std::map<UShortArray, UShortArraySet>::const_iterator bestModelSetIter;
  /// the best performing model graph among the set from generate_dags()
  UShortArraySet::const_iterator bestDAGIter;
  /// the merit function value for the best solution, incorporating both
  /// estimator variance and budget (objective and constraint in some order)
  Real meritFnStar;

  /// book-keeping of previous numerical optimization solutions for each DAG;
  /// used for warm starting
  std::map<std::pair<UShortArray, UShortArray>, DAGSolutionData> dagSolns;
};


inline size_t NonDGenACVSampling::num_approximations() const
{ return activeModelSetIter->first.size(); }


inline Real NonDGenACVSampling::estimator_accuracy_metric()
{
  std::pair<UShortArray, UShortArray>
    key(activeModelSetIter->first, *activeDAGIter);
  return dagSolns[key].avgEstVar;
}


//inline Real NonDGenACVSampling::estimator_cost_metric()
//{
//  std::pair<UShortArray, UShortArray>
//    key(activeModelSetIter->first, *activeDAGIter);
//  return dagSolns[key].equivHFAlloc;
//}


inline void NonDGenACVSampling::print_variance_reduction(std::ostream& s)
{
  std::pair<UShortArray, UShortArray>
    key(activeModelSetIter->first, *activeDAGIter);
  print_estimator_performance(s, dagSolns[key]);
}


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
  if (spd_solver.shouldEquilibrate()) {
    spd_solver.factorWithEquilibration(true);
    // if (outputLevel >= DEBUG_OUTPUT) {
    //   Real rcond;  spd_solver.reciprocalConditionEstimate(rcond);
    //   Cout << "Equilibrating in GenACV::invert_C_G_matrix(): reciprocal "
    // 	   << "condition number = " << rcond << std::endl;
    // }
  }
  // Only useful for solve():
  //spd_solver.solveToRefinedSolution(true);
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
  if  (N_vec.length() == numSteps) N_vec = 0.;
  else N_vec.size(numSteps);
  for (i=0; i<num_approx; ++i)
    N_vec[approx_set[i]] = cd_vars[i];
  if (num_cdv == num_approx + 1)
    N_vec[numApprox] = cd_vars[num_approx];
  else {
    // N_H not provided so pull from latest counter values
    size_t hf_form_index, hf_lev_index;
    hf_indices(hf_form_index, hf_lev_index);
    // average_estimator_variance() uses actual (not alloc) to sync with varH
    // so use same prior to defining G,g in precompute_genacv_control() and
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


inline void NonDGenACVSampling::
solve_for_C_G_c_g(RealSymMatrix& C_G, RealVector& c_g, RealVector& lhs,
		  bool copy_C_G, bool copy_c_g)
{
  // The idea behind this approach is to leverage both the solution refinement
  // in solve() and equilibration during factorization (inverting C_G in place
  // can only leverage the latter).

  size_t n = c_g.length();
  lhs.size(n); // not sure if initialization matters here...

  RealSpdSolver spd_solver;  RealSymMatrix C_G_copy;  RealVector c_g_copy;
  // Matrix & RHS get altered by equilibration --> make copies if needed later
  if (copy_C_G) {
    C_G_copy = C_G; // Teuchos::Copy by default
    spd_solver.setMatrix(Teuchos::rcp(&C_G_copy, false));
  }
  else // Ok to modify C_G in place
    spd_solver.setMatrix(Teuchos::rcp(&C_G, false));
  if (copy_c_g) {
    c_g_copy = c_g; // Teuchos::Copy by default
    spd_solver.setVectors(Teuchos::rcp(&lhs, false),
			  Teuchos::rcp(&c_g_copy, false));
  }
  else // Ok to modify c_g in place
    spd_solver.setVectors(Teuchos::rcp(&lhs, false), Teuchos::rcp(&c_g, false));

  if (spd_solver.shouldEquilibrate())
    spd_solver.factorWithEquilibration(true);
  spd_solver.solveToRefinedSolution(true);
  int code = spd_solver.solve();
  if (code) {
    Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	 << ") in GenACV::solve_for_C_G_c_g()." << std::endl;
    abort_handler(METHOD_ERROR);
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
precompute_genacv_control(const RealVector& avg_eval_ratios,
			  const SizetArray& N_shared)
{
  // Note: while G,g have a more explicit dependence on N_shared[qoi] than F,
  // we mirror the averaged sample allocations and compute G,g once
  RealVector N_vec, inflate_N_vec;
  r_and_N_to_N_vec(avg_eval_ratios, average(N_shared), N_vec);
  inflate_variables(N_vec, inflate_N_vec, activeModelSetIter->first);
  compute_parameterized_G_g(inflate_N_vec);
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
compute_genacv_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
		       RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
		       size_t N_shared_q, size_t mom, size_t qoi,
		       const UShortArray& approx_set, RealVector& beta)
{
  if (mom == 1) // variances/covariances already computed for mean estimator
    solve_for_genacv_control(covLL[qoi], GMat, covLH, gVec, qoi,
			     approx_set, beta);
  else { // compute variances/covariances for higher-order moment estimators
    // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
    // > cov_LH is sized for all qoi but only 1 row is used
    RealSymMatrix cov_LL_mq; RealMatrix cov_LH_m;
    compute_acv_control_covariances(sum_L_base_m, sum_H_mq, sum_LL_mq, sum_LH_m,
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


inline bool NonDGenACVSampling::valid_variance(Real var) const
{ return (std::isfinite(var) && var > 0.); }

} // namespace Dakota

#endif
