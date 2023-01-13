/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDACVSampling
//- Description: Class for approximate control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

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


/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Approximate Control Variate (ACV) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

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
  void print_variance_reduction(std::ostream& s);

  /// return name of active optimizer method
  unsigned short uses_method() const;
  /// perform a numerical solver method switch due to a detected conflict
  void method_recourse();

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

  //
  //- Heading: member functions
  //

  void shared_increment(size_t iter);
  void shared_approx_increment(size_t iter);
  bool approx_increment(size_t iter, const SizetArray& approx_sequence,
			size_t start, size_t end);
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
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 size_t index, Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 size_t start, size_t end,Real& equiv_hf_evals);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 const SizetArray& approx_sequence,
				 size_t start, size_t end,Real& equiv_hf_evals);

  void increment_sample_range(SizetArray& N_L, size_t incr,
			      const SizetArray& approx_sequence,
			      size_t start, size_t end);

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
  
  void mfmc_analytic_solution(const RealMatrix& rho2_LH, const RealVector& cost,
			      RealMatrix& eval_ratios,
			      bool monotonic_r = false);
  void mfmc_reordered_analytic_solution(const RealMatrix& rho2_LH,
					const RealVector& cost,
					SizetArray& approx_sequence,
					RealMatrix& eval_ratios,
					bool monotonic_r);
  void cvmc_ensemble_solutions(const RealMatrix& rho2_LH,
			       const RealVector& cost, RealMatrix& eval_ratios);
  void nonhierarch_numerical_solution(const RealVector& cost,
				      const SizetArray& approx_sequence,
				      RealVector& avg_eval_ratios,
				      Real& avg_hf_target, size_t& num_samples,
				      Real& avg_estvar, Real& avg_estvar_ratio);

  Real allocate_budget(const RealVector& avg_eval_ratios,
		       const RealVector& cost);
  void scale_to_budget_with_pilot(RealVector& avg_eval_ratios,
				  const RealVector& cost, Real avg_N_H);

  /// helper function that supports optimization APIs passing design variables
  Real average_estimator_variance(const RealVector& cd_vars);

  void r_and_N_to_N_vec(const RealVector& avg_eval_ratios, Real N_H,
			RealVector& N_vec);
  void r_and_N_to_design_vars(const RealVector& avg_eval_ratios, Real N_H,
			      RealVector& cd_vars);

  /// define approx_sequence in increasing metric order
  bool ordered_approx_sequence(const RealVector& metric,
			       SizetArray& approx_sequence,
			       bool descending_keys = false);
  /// determine whether metric is in increasing order for all columns
  bool ordered_approx_sequence(const RealMatrix& metric);

  void apply_control(Real sum_L_shared, size_t num_shared, Real sum_L_refined,
		     size_t num_refined, Real beta, Real& H_raw_mom);

  /// promote scalar to 1D array
  void inflate(size_t N_0D, SizetArray& N_1D);
  /// promote 1D array to 2D array
  void inflate(const SizetArray& N_1D, Sizet2DArray& N_2D);
  /// promote vector of averaged values to full matrix
  void inflate(const RealVector& avg_eval_ratios, RealMatrix& eval_ratios);
  /// promote scalar to column vector
  void inflate(Real r_i, size_t num_rows, Real* eval_ratios_col);

  //
  //- Heading: Data
  //

  /// the minimizer used to minimize the estimator variance over parameters
  /// of number of truth model samples and approximation eval_ratios
  Iterator varianceMinimizer;
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

  /// tracks ordering of a metric (correlations, eval ratios) across set of
  /// approximations
  SizetArray approxSequence;

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
  /// ratio of final estimator variance (optimizer result averaged across QoI)
  /// and final MC estimator variance  (final varH / N_H averaged across QoI)
  Real avgEstVarRatio;

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

  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  Real linear_cost(const RealVector& N_vec);
  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  Real nonlinear_cost(const RealVector& r_and_N);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  void linear_cost_gradient(const RealVector& N_vec, RealVector& grad_c);
  /// constraint gradient helper fn shared by NPSOL/OPT++ static evaluators
  void nonlinear_cost_gradient(const RealVector& r_and_N, RealVector& grad_c);

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


inline unsigned short NonDNonHierarchSampling::uses_method() const
{ return optSubProblemSolver; }


inline void NonDNonHierarchSampling::method_recourse()
{
  // NonDNonHierarchSampling numerical solves must protect use of Fortran
  // solvers at this level from conflicting with use at a higher level.
  // However, it is not necessary to check the other direction by defining
  // check_sub_iterator_conflict(), since solver execution does not span
  // any Model evaluations.

  bool err_flag = false;
  switch (optSubProblemSolver) {
  case SUBMETHOD_NPSOL:
#ifdef HAVE_OPTPP
    optSubProblemSolver = SUBMETHOD_OPTPP;
#else
    err_flag = true;
#endif
    break;
  case SUBMETHOD_OPTPP:
#ifdef HAVE_NPSOL
    optSubProblemSolver = SUBMETHOD_NPSOL;
#else
    err_flag = true;
#endif
    break;
  }

  if (err_flag) {
    Cerr << "\nError: method conflict detected in NonDNonHierarchSampling but "
	 << "no alternate solver available." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  else
    Cerr << "\nWarning: method recourse invoked in NonDNonHierarchSampling due "
	 << "to detected method conflict.\n\n";
}


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
  size_t approx, len = cost.length(), hf_index = len-1;
  Real cost_ref = cost[hf_index];
  Real equiv_hf_ratio = 1.; // apply avg_hf_target at end
  for (approx=0; approx<hf_index; ++approx)
    equiv_hf_ratio += avg_eval_ratios[approx] * cost[approx] / cost_ref;
  return equiv_hf_ratio * avg_hf_target;
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
  Real cost_ref = cost[hf_index];
  if (end == len)
    { equiv_hf_evals += new_samp; --end; }
  for (index=start; index<end; ++index)
    equiv_hf_evals += (Real)new_samp * cost[index] / cost_ref;
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
    Real cost_ref = cost[hf_index];
    if (end == len) // truth is always last
      { equiv_hf_evals += new_samp; --end; }
    for (i=start; i<end; ++i) {
      approx = approx_sequence[i];
      equiv_hf_evals += (Real)new_samp * cost[approx] / cost_ref;
    }
  }
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


inline void NonDNonHierarchSampling::
scale_to_budget_with_pilot(RealVector& avg_eval_ratios, const RealVector& cost,
			   Real avg_N_H)
{
  // retain the shape of an r* profile, but scale to budget constrained by
  // incurred pilot cost

  Real r_i, cost_r_i, factor, inner_prod = 0., cost_H = cost[numApprox],
    budget = (Real)maxFunctionEvals;
  for (size_t approx=0; approx<numApprox; ++approx)
    inner_prod += cost[approx] * avg_eval_ratios[approx]; // Sum(w_i r_i)
  factor = (budget / avg_N_H - 1.) / inner_prod * cost_H;
  //avg_eval_ratios.scale(factor); // can result in infeasible r_i < 1

  for (int i=numApprox-1; i>=0; --i) {
    r_i = avg_eval_ratios[i] * factor;
    if (r_i <= 1.) { // fix at 1+NUDGE and scale remaining r_i to reduced budget
      cost_r_i  = avg_eval_ratios[i] = 1. + RATIO_NUDGE;
      cost_r_i *= cost[i];
      budget   -= avg_N_H * cost_r_i / cost_H;  inner_prod -= cost_r_i;
      factor    = (budget / avg_N_H - 1.) / inner_prod * cost_H;
    }
    else
      avg_eval_ratios[i] = r_i;
    //Cout << " avg_eval_ratios[" << i << "] = " << avg_eval_ratios[i] << '\n';
  }
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Average evaluation ratios rescaled to budget:\n"
	 << avg_eval_ratios << std::endl;
}


inline void NonDNonHierarchSampling::
r_and_N_to_N_vec(const RealVector& avg_eval_ratios, Real N_H, RealVector& N_vec)
{
  N_vec.sizeUninitialized(numApprox+1);
  for (size_t i=0; i<numApprox; ++i)
    N_vec[i] = avg_eval_ratios[i] * N_H;
  N_vec[numApprox] = N_H;
}


inline void NonDNonHierarchSampling::
r_and_N_to_design_vars(const RealVector& avg_eval_ratios, Real N_H,
		       RealVector& cd_vars)
{
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // embed N at end of cd_vars for GenACV usage
  case R_AND_N_NONLINEAR_CONSTRAINT:
    cd_vars.sizeUninitialized(numApprox+1);
    copy_data_partial(avg_eval_ratios, cd_vars, 0);
    cd_vars[numApprox] = N_H;
    break;
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
ordered_approx_sequence(const RealMatrix& metric)//, bool descending_keys)
{
  size_t r, c, nr = metric.numRows(), nc = metric.numCols(), metric_order;
  std::multimap<Real, size_t> metric_map;
  std::multimap<Real, size_t>::iterator it;
  bool ordered = true;
  for (r=0; r<nr; ++r) {  // numFunctions
    metric_map.clear();
    for (c=0; c<nc; ++c) // numApprox
      metric_map.insert(std::pair<Real, size_t>(metric(r,c), c));
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

  //Cout <<  "sum_L_shared = "  << sum_L_shared
  //     << " sum_L_refined = " << sum_L_refined
  //     << " num_L_shared = "  << num_L_shared
  //     << " num_L_refined = " << num_L_refined << std::endl; 
}


inline void NonDNonHierarchSampling::inflate(size_t N_0D, SizetArray& N_1D)
{ N_1D.assign(numApprox, N_0D); }


inline void NonDNonHierarchSampling::
inflate(const SizetArray& N_1D, Sizet2DArray& N_2D)
{
  N_2D.resize(numApprox);
  for (size_t approx=0; approx<numApprox; ++approx)
    N_2D[approx] = N_1D;
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
