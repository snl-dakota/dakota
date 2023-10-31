/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_NONHIERARCH_SAMPLING_H
#define NOND_NONHIERARCH_SAMPLING_H

#include "NonDEnsembleSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {

// needs to be greater than FDSS to avoid potential FPE from N > N_i
// (especially for bounds-respecting central diff at N = pilot). Careful
// control of NPSOL is needed: {Central,Fwd} FDSS are now assigned.
#define RATIO_NUDGE 1.e-4

// special values for optSubProblemForm
enum { ANALYTIC_SOLUTION = 1, REORDERED_ANALYTIC_SOLUTION,
       R_ONLY_LINEAR_CONSTRAINT, N_VECTOR_LINEAR_CONSTRAINT,
       R_AND_N_NONLINEAR_CONSTRAINT, N_VECTOR_LINEAR_OBJECTIVE };

  
/// Container class for numerical solutions for a given DAG

/** Used for caching the optimization solution data that is required
    for restoring a selected (best) DAG. */

class DAGSolutionData
{
  //
  //- Heading: Friends
  //

  // GenACV can access attributes of the body class directly
  //friend class NonDGenACVSampling;

public:

  /// default constructor
  DAGSolutionData();
  // full constructor
  DAGSolutionData(const RealVector& avg_eval_ratios, Real avg_hf_target,
		  Real avg_est_var, Real avg_est_var_ratio, Real equiv_hf);
  /// copy constructor
  DAGSolutionData(const DAGSolutionData& sd);
  /// destructor
  ~DAGSolutionData();
  /// assignment operator
  DAGSolutionData& operator=(const DAGSolutionData&);

  //
  //- Heading: Data
  //

  // optimization variables:

  // average evaluation ratios for model graph
  RealVector avgEvalRatios;
  // average high-fidelity sample target for model graph
  Real avgHFTarget;

  // optimization results: (for budget constrained)

  /// average estimator variance for model graph
  Real avgEstVar;
  /// average estimator variance ratio (1 - R^2) for model graph: the
  /// ratio of final estimator variance (optimizer result) and final
  /// MC estimator variance using final N_H samples (not equivHF)
  Real avgEstVarRatio;

  // for accuracy constrained: total allocation for estimator in
  // equivalent HF units (may differ from total actual = equivHFEvals)
  Real equivHFAlloc;
};


inline DAGSolutionData::DAGSolutionData():
  avgHFTarget(0.), avgEstVar(DBL_MAX), avgEstVarRatio(1.), equivHFAlloc(0.)
{ }


inline DAGSolutionData::
DAGSolutionData(const RealVector& avg_eval_ratios, Real avg_hf_target,
		Real avg_est_var, Real avg_est_var_ratio, Real equiv_hf)
{
  avgEvalRatios = avg_eval_ratios;  avgHFTarget    = avg_hf_target;
  avgEstVar     = avg_est_var;      avgEstVarRatio = avg_est_var_ratio;
  equivHFAlloc  = equiv_hf;
}


inline DAGSolutionData::DAGSolutionData(const DAGSolutionData& sd)
{
  avgEvalRatios = sd.avgEvalRatios;  avgHFTarget    = sd.avgHFTarget;
  avgEstVar     = sd.avgEstVar;      avgEstVarRatio = sd.avgEstVarRatio;
  equivHFAlloc  = sd.equivHFAlloc;
}


inline DAGSolutionData::~DAGSolutionData()
{ }


inline DAGSolutionData& DAGSolutionData::operator=(const DAGSolutionData& sd)
{
  avgEvalRatios = sd.avgEvalRatios;  avgHFTarget    = sd.avgHFTarget;
  avgEstVar     = sd.avgEstVar;      avgEstVarRatio = sd.avgEstVarRatio;
  equivHFAlloc  = sd.equivHFAlloc;
  return *this;
}


/// Base class for non-hierarchical ensemble-based Monte Carlo sampling.

/** Derived classes include MFMC, ACV, and GenACV. */

class NonDNonHierarchSampling: public NonDEnsembleSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDNonHierarchSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDNonHierarchSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  //void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  //void print_variance_reduction(std::ostream& s);

  /// return name of active optimizer method
  unsigned short uses_method() const;
  /// perform a numerical solver method switch due to a detected conflict
  void method_recourse(unsigned short method_name);

  //
  //- Heading: New virtual functions
  //

  /// compute estimator variance ratios from HF samples and oversample ratios
  virtual void estimator_variance_ratios(const RealVector& r_and_N,
					 RealVector& estvar_ratios) = 0;
  /// augment linear inequality constraints as required by derived algorithm
  virtual void augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
					       RealVector& lin_ineq_lb,
					       RealVector& lin_ineq_ub) = 0;
  /// return quadratic constraint violation for augment linear
  /// inequality constraints
  virtual Real augmented_linear_ineq_violations(const RealVector& cd_vars,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_lb,
    const RealVector& lin_ineq_ub) = 0;

  virtual void numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
					 size_t& num_nln_con);
  virtual void numerical_solution_bounds_constraints(
    const DAGSolutionData& soln, const RealVector& cost, Real avg_N_H,
    RealVector& x0, RealVector& x_lb, RealVector& x_ub,
    RealVector& lin_ineq_lb, RealVector& lin_ineq_ub, RealVector& lin_eq_tgt,
    RealVector& nln_ineq_lb, RealVector& nln_ineq_ub, RealVector& nln_eq_tgt,
    RealMatrix& lin_ineq_coeffs, RealMatrix& lin_eq_coeffs);
  /// When looping through a minimizer sequence/competition, this
  /// function enables per-minimizer updates to the parameter bounds,
  /// e.g. for providing a bounded domain for methods that require it,
  /// while removing it for those that don't
  virtual void finite_solution_bounds(const RealVector& cost, Real avg_N_H,
				      RealVector& x_lb, RealVector& x_ub);

  /// post-process optimization final results to recover solution data
  virtual void recover_results(const RealVector& cv_star,
			       const RealVector& fn_star, Real& avg_estvar,
			       RealVector& avg_eval_ratios,
			       Real& avg_hf_target, Real& equiv_hf_cost);

  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  virtual Real linear_cost(const RealVector& N_vec);
  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  virtual Real nonlinear_cost(const RealVector& r_and_N);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  virtual void linear_cost_gradient(const RealVector& N_vec,RealVector& grad_c);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  virtual void nonlinear_cost_gradient(const RealVector& r_and_N,
				       RealVector& grad_c);

  virtual size_t num_approximations() const;

  //
  //- Heading: member functions
  //

  void shared_increment(size_t iter);
  void shared_increment(size_t iter, const UShortArray& approx_set);
  void shared_approx_increment(size_t iter);
  bool approx_increment(size_t iter, const SizetArray& approx_sequence,
			size_t start, size_t end);
  bool approx_increment(size_t iter, const SizetArray& approx_sequence,
			size_t start, size_t end,
			const UShortArray& approx_set);
  bool approx_increment(size_t iter, unsigned short root,
			const UShortSet& reverse_dag);
  void ensemble_sample_increment(size_t iter, size_t step);

  // manage response mode and active model key from {group,form,lev} triplet.
  // seq_type defines the active dimension for a model sequence.
  //void configure_indices(size_t group,size_t form,size_t lev,short seq_type);

  void assign_active_key(bool multilev);

  /// recover estimates of simulation cost using aggregated response metadata
  void recover_online_cost(RealVector& seq_cost);

  void initialize_sums(IntRealMatrixMap& sum_L_baseline,
		       IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LH,
		       RealVector& sum_HH);
  void initialize_counts(Sizet2DArray& num_L_baseline, SizetArray& num_H,
			 Sizet2DArray& num_LH);
  void finalize_counts(Sizet2DArray& N_L_actual, SizetArray& N_L_alloc);

  Real compute_equivalent_cost(Real avg_hf_target,
			       const RealVector& avg_eval_ratios,
			       const RealVector& cost);
  Real compute_equivalent_cost(Real avg_hf_target,
			       const RealVector& avg_eval_ratios,
			       const RealVector& cost,
			       const UShortArray& approx_set);

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

  void increment_sample_range(SizetArray& N_L, size_t incr,
			      const SizetArray& approx_sequence,
			      size_t start, size_t end);
  void increment_sample_range(SizetArray& N_L, size_t incr, unsigned short root,
			      const UShortSet& reverse_dag);
  void increment_sample_range(SizetArray& N_L, size_t incr,
			      const SizetArray& approx_sequence,
			      size_t start, size_t end,
			      const UShortArray& approx_set);

  /// define model form and resolution level indices
  void hf_indices(size_t& hf_form_index, size_t& hf_lev_index);

  void compute_variance(Real sum_Q, Real sum_QQ, size_t num_Q, Real& var_Q);
  void compute_variance(const RealVector& sum_Q, const RealVector& sum_QQ,
			const SizetArray& num_Q,       RealVector& var_Q);

  void compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1,
			   Real sum_Q1Q2, Real sum_Q2Q2, size_t N_shared,
			   Real& var_Q1,  Real& var_Q2,  Real& rho2_Q1Q2);
  void compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2,
			  size_t N_shared, Real& cov_Q1Q2);
  
  void mfmc_analytic_solution(const UShortArray& approx_set,
			      const RealMatrix& rho2_LH, const RealVector& cost,
			      DAGSolutionData& soln, bool monotonic_r = false);
  void mfmc_reordered_analytic_solution(const UShortArray& approx_set,
					const RealMatrix& rho2_LH,
					const RealVector& cost,
					SizetArray& approx_sequence,
					DAGSolutionData& soln,
					bool monotonic_r = false);

  void ensemble_numerical_solution(const RealVector& cost,
				   DAGSolutionData& soln, size_t& num_samples);
  void configure_minimizers(const RealVector& cost, Real avg_N_H,
			    RealVector& x0, RealVector& x_lb, RealVector& x_ub,
			    RealVector& lin_ineq_lb, RealVector& lin_ineq_ub,
			    RealVector& lin_eq_tgt,  RealVector& nln_ineq_lb,
			    RealVector& nln_ineq_ub, RealVector& nln_eq_tgt,
			    RealMatrix& lin_ineq_coeffs,
			    RealMatrix& lin_eq_coeffs);
  void run_minimizers(DAGSolutionData& soln);

  Real allocate_budget(const RealVector& avg_eval_ratios,
		       const RealVector& cost);
  Real allocate_budget(const UShortArray& approx_set,
		       const RealVector& avg_eval_ratios,
		       const RealVector& cost);

  void scale_to_budget_with_pilot(RealVector& avg_eval_ratios,
				  const RealVector& cost, Real avg_N_H);

  /// helper function that supports optimization APIs passing design variables
  Real average_estimator_variance(const RealVector& cd_vars);
  /// helper function that supports virtual print_variance_reduction(s)
  void print_estimator_performance(std::ostream& s,
				   const DAGSolutionData& soln);

  void r_and_N_to_N_vec(const RealVector& avg_eval_ratios, Real N_H,
			RealVector& N_vec);
  void r_and_N_to_design_vars(const RealVector& avg_eval_ratios, Real N_H,
			      RealVector& cd_vars);

  /// define approx_sequence in increasing metric order
  bool ordered_approx_sequence(const RealVector& metric,
			       SizetArray& approx_sequence,
			       bool descending_keys = false);
  /// determine whether metric is in increasing order by columns for all rows
  bool ordered_approx_sequence(const RealMatrix& metric);
  /// determine whether metric is in increasing order by active columns for
  /// all rows
  bool ordered_approx_sequence(const RealMatrix& metric,
			       const UShortArray& approx_set);

  void apply_control(Real sum_L_shared, size_t num_shared, Real sum_L_refined,
		     size_t num_refined, Real beta, Real& H_raw_mom);

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

  /// compute a penalty merit function after an optimization solve
  Real nh_penalty_merit(const RealVector& c_vars, const RealVector& fn_vals);
  /// compute a penalty merit function after a DAGSolutionData instance
  Real nh_penalty_merit(const DAGSolutionData& soln);

  //
  //- Heading: Data
  //

  /// the minimizer(s) used to optimize the estimator variance over the number
  /// of truth model samples and approximation eval_ratios.  Minimizers are
  /// arranged in a sequence (first dimension) where each step in the sequence
  /// may have multiple competitors (second dimension)
  Iterator2DArray varianceMinimizers;
  /// active indices for numerical solutions: varianceMinimizers[first][second]
  SizetSizetPair varMinIndices;

  /// variance minimization algorithm selection: SUBMETHOD_MFMC or
  /// SUBMETHOD_ACV_{IS,MF,KL}
  unsigned short mlmfSubMethod;

  /// number of approximation models managed by non-hierarchical iteratedModel
  size_t numApprox;

  /// formulation for optimization sub-problem that minimizes R^2 subject
  /// to different variable sets and different linear/nonlinear constraints
  short optSubProblemForm;
  /// SQP or NIP
  unsigned short optSubProblemSolver;
  /// user specification to suppress any increments in the number of HF
  /// evaluations (e.g., because too expensive and no more can be performed)
  bool truthFixedByPilot;

  /// covariances between each LF approximation and HF truth (the c
  /// vector in ACV); organized numFunctions x numApprox
  RealMatrix covLH;
  /// covariances among all LF approximations (the C matrix in ACV); organized
  /// as a numFunctions array of symmetic numApprox x numApprox matrices
  RealSymMatrixArray covLL;
  /// squared Pearson correlations among approximations and truth
  RealMatrix rho2LH;

  /// for sample projections, the calculated increment in HF samples that
  /// would be evaluated if full iteration/statistics were pursued
  size_t deltaNActualHF;

  /// number of successful pilot evaluations of HF truth model (exclude faults)
  SizetArray numHIter0;

private:

  //
  //- Heading: helper functions
  //

  Real update_hf_target(Real avg_estvar, Real avg_N_H,
			const RealVector& estvar0);
  Real update_hf_target(Real avg_estvar, const SizetArray& N_H,
			const RealVector& estvar0);

  /// flattens contours of average_estimator_variance() using std::log
  Real log_average_estvar(const RealVector& cd_vars);

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
finalize_counts(Sizet2DArray& N_L_actual, SizetArray& N_L_alloc)
{
  // post final sample counts back to NLev{Actual,Alloc} (for final summaries)

  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);

  // Aggregate N_H into 2D N_L array and then insert into 3D:
  //N_L_actual.push_back(N_H);
  //inflate_sequence_samples(N_L_actual, multilev, secondaryIndex, NLevActual);

  // Update LF counts only as HF counts are directly updated by reference:
  inflate_approx_samples(N_L_actual, multilev, secondaryIndex, NLevActual);
  inflate_approx_samples(N_L_alloc,  multilev, secondaryIndex, NLevAlloc);
}


inline Real NonDNonHierarchSampling::
compute_equivalent_cost(Real avg_hf_target, const RealVector& avg_eval_ratios,
			const RealVector& cost)
{
  size_t a, hf_index = cost.length() - 1;
  Real cost_ref = cost[hf_index], rc_sum = 0.;
  for (a=0; a<numApprox; ++a)
    rc_sum += avg_eval_ratios[a] * cost[a];
  return avg_hf_target * (rc_sum / cost_ref + 1.);
}


inline Real NonDNonHierarchSampling::
compute_equivalent_cost(Real avg_hf_target, const RealVector& avg_eval_ratios,
			const RealVector& cost, const UShortArray& approx_set)
{
  size_t a, num_approx = approx_set.size(), hf_index = cost.length() - 1;
  Real cost_ref = cost[hf_index], rc_sum = 0.;
  for (a=0; a<num_approx; ++a)
    rc_sum += avg_eval_ratios[a] * cost[approx_set[a]];
  return avg_hf_target * (rc_sum / cost_ref + 1.);
}


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
    { equiv_hf_evals += new_samp; --end; }
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


inline Real NonDNonHierarchSampling::
allocate_budget(const RealVector& avg_eval_ratios, const RealVector& cost)
{
  // version with scalar HF target (eval_ratios already averaged over QoI
  // due to formulation of optimization sub-problem)

  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  inner_prod = cost_H; // raw cost (un-normalized)
  for (size_t approx=0; approx<numApprox; ++approx)
    inner_prod += cost[approx] * avg_eval_ratios[approx];
  Real avg_hf_target = budget / inner_prod * cost_H; // normalized to equivHF
  return avg_hf_target;
}


inline Real NonDNonHierarchSampling::
allocate_budget(const UShortArray& approx_set,
		const RealVector& avg_eval_ratios, const RealVector& cost)
{
  // version with scalar HF target (eval_ratios already averaged over QoI
  // due to formulation of optimization sub-problem)

  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  inner_prod = cost_H; // raw cost (un-normalized)
  size_t a, num_approx = approx_set.size();
  for (a=0; a<num_approx; ++a)
    inner_prod += cost[approx_set[a]] * avg_eval_ratios[a];
  Real avg_hf_target = budget / inner_prod * cost_H; // normalized to equivHF
  return avg_hf_target;
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
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
    r_and_N_to_N_vec(avg_eval_ratios, N_H, cd_vars);  break;
  }
}


inline Real NonDNonHierarchSampling::
update_hf_target(Real avg_estvar, Real avg_N_H, const RealVector& estvar0)
{
  // Note: there is a circular dependency between estvar_ratios and hf_targets
  RealVector hf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    hf_targets[qoi] = avg_estvar * avg_N_H
                    / (convergenceTol * estvar0[qoi]);
  Real avg_hf_target = average(hf_targets);
  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": average HF target = " << avg_hf_target << std::endl;
  return avg_hf_target;
}


inline Real NonDNonHierarchSampling::
update_hf_target(Real avg_estvar, const SizetArray& N_H,
		 const RealVector& estvar0)
{
  // Note: there is a circular dependency between estvar_ratios and hf_targets
  RealVector hf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    hf_targets[qoi] = avg_estvar * N_H[qoi]
                    / (convergenceTol * estvar0[qoi]);
  Real avg_hf_target = average(hf_targets);
  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": average HF target = " << avg_hf_target << std::endl;
  return avg_hf_target;
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
  // unbiased sample variance estimator = 1/(N-1) sum[(Q_i - Q-bar)^2]
  // = 1/(N-1) [ sum_QQ - N Q-bar^2 ] = 1/(N-1) [ sum_QQ - sum_Q^2 / N ]
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
  Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1);

  // unbiased mean X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / N_shared,  mu_Q2 = sum_Q2 / N_shared;
  // unbiased sample covariance = 1/(N-1) sum[(X_i - X-bar)(Y_i - Y-bar)]
  // = 1/(N-1) [N RawMom_XY - N X-bar Y-bar] = bessel [RawMom_XY - X-bar Y-bar]
  cov_Q1Q2 = (sum_Q1Q2 / N_shared - mu_Q1 * mu_Q2) * bessel_corr;

  //Cout << "compute_covariance: sum_Q1 = " << sum_Q1 << " sum_Q2 = " << sum_Q2
  //     << " sum_Q1Q2 = " << sum_Q1Q2 << " num_shared = " << num_shared
  //     << " cov_Q1Q2 = " << cov_Q1Q2 << std::endl;
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

  //Cout <<  "apply_control: sum_L_shared = "  << sum_L_shared
  //     << " sum_L_refined = " << sum_L_refined
  //     << " num_L_shared = "  << num_L_shared
  //     << " num_L_refined = " << num_L_refined << std::endl;
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

} // namespace Dakota

#endif
