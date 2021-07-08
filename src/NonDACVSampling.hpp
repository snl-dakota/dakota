/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDACVSampling
//- Description: Class for approximate control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_ACV_SAMPLING_H
#define NOND_ACV_SAMPLING_H

#include "NonDEnsembleSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {

/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Approximate Control Variate (ACV) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDACVSampling: public NonDEnsembleSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDACVSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDACVSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  //bool resize();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  //
  //- Heading: member functions
  //

  void multifidelity_mc();
  void approximate_control_variate();

  void shared_increment(size_t iter);
  void shared_approx_increment(size_t iter);
  bool approx_increment(const RealMatrix& eval_ratios, const Sizet2DArray& N_L,
			const RealVector& hf_targets, size_t iter, size_t start,
			size_t end);
  bool approx_increment(const RealVector& avg_eval_ratios,
			const Sizet2DArray& N_L, Real hf_target,
			size_t iter, size_t start, size_t end);

  void allocate_budget(const RealMatrix& eval_ratios, const RealVector& cost,
		       RealVector& hf_targets);
  void allocate_budget(const RealVector& eval_ratios, const RealVector& cost,
		       Real& hf_target);

  void update_hf_target(const RealVector& avg_eval_ratios,
			const RealVector& cost, Real mse_ratio,
			const RealVector& var_H, const SizetArray& N_H,
			const RealVector& mse_iter0, Real& avg_hf_target);
  void update_hf_targets(const RealMatrix& eval_ratios, const RealVector& cost,
			 const RealVector& mse_ratios,  const RealVector& var_H,
			 const SizetArray& N_H, const RealVector& mse_iter0,
			 RealVector& hf_targets);

  void compute_ratios(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		      const RealMatrix& sum_LL,       const RealMatrix& sum_LH,
		      const RealVector& sum_HH,       const RealVector& cost,
		      const Sizet2DArray& N_L,        const SizetArray& N_H,
		      const Sizet2DArray& N_LH,       RealVector& var_H,
		      RealMatrix& rho2_LH,            RealMatrix& eval_ratios,
		      RealVector& mse_ratios);
  void compute_ratios(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		      const RealSymMatrixArray& sum_LL,
		      const RealMatrix& sum_LH, const RealVector& sum_HH,
		      const RealVector& cost,   const Sizet2DArray& N_L,
		      const SizetArray& N_H,    const SizetSymMatrixArray& N_LL,
		      const Sizet2DArray& N_LH, RealVector& avg_eval_ratios,
		      Real& mse_ratio);

private:

  //
  //- Heading: Helper functions
  //

  // manage response mode and active model key from {group,form,lev} triplet.
  // seq_type defines the active dimension for a model sequence.
  //void configure_indices(size_t group,size_t form,size_t lev,short seq_type);

  void assign_active_key(size_t num_steps, size_t secondary_index,
			 bool multilev);

  void initialize_sums(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		       IntRealMatrixMap& sum_LH, RealVector& sum_HH);
  void initialize_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  IntRealVectorMap& sum_H,
			  IntRealMatrixMap& sum_LL,
			  IntRealMatrixMap& sum_LH, RealVector& sum_HH);
  void initialize_acv_sums(IntRealMatrixMap& sum_L_shared,
			   IntRealMatrixMap& sum_L_refined,
			   IntRealVectorMap& sum_H,
			   IntRealSymMatrixArrayMap& sum_LL,
			   IntRealMatrixMap& sum_LH, RealVector& sum_HH);

  void initialize_mf_counts(Sizet2DArray& N_L, SizetArray& N_H,
			    Sizet2DArray& N_LH);
  void initialize_acv_counts(Sizet2DArray&        N_L,  SizetArray&   N_H,
			     SizetSymMatrixArray& N_LL, Sizet2DArray& N_LH);

  //void initialize_acv_covariances(IntRealSymMatrixArrayMap covLL,
  //				  IntRealMatrixMap& cov_LH,
  //				  IntRealVectorMap& var_H);

  void accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined,
			  IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LL,
			  IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			  Sizet2DArray& num_L, SizetArray& num_H,
			  Sizet2DArray& num_LH);
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
			   IntRealMatrixMap& sum_L_refined,
			   IntRealVectorMap& sum_H,
			   IntRealSymMatrixArrayMap& sum_LL,
			   IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			   Sizet2DArray& num_L,  SizetArray& num_H,
			   SizetSymMatrixArray& num_LL, Sizet2DArray& num_LH);
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
			   IntRealMatrixMap& sum_L_refined,
			   IntRealSymMatrixArrayMap& sum_LL,
			   Sizet2DArray& num_L, SizetSymMatrixArray& num_LL);

  void accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
			  IntRealMatrixMap& sum_L_refined, Sizet2DArray& num_L,
			  size_t approx_start, size_t approx_end);
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_refined, Sizet2DArray& num_L,
			   size_t approx_start, size_t approx_end);

  void ensemble_sample_increment(size_t iter, size_t step);

  //void increment_samples(size_t new_N, size_t start, size_t end,
  //			   SizetArray& N_l);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 size_t start, size_t end);

  void compute_LH_correlation(const RealMatrix& sum_L_shared,
			      const RealVector& sum_H, const RealMatrix& sum_LL,
			      const RealMatrix& sum_LH,const RealVector& sum_HH,
			      const Sizet2DArray& N_L, const SizetArray& N_H,
			      const Sizet2DArray& N_LH, RealVector& var_H,
			      RealMatrix& rho2_LH);
  void compute_LH_covariance(const RealMatrix& sum_L_shared,
			     const RealVector& sum_H,  const RealMatrix& sum_LH,
			     const Sizet2DArray& N_L,  const SizetArray& N_H,
			     const Sizet2DArray& N_LH, RealMatrix& cov_LH);
  void compute_LL_covariance(const RealMatrix& sum_L_shared,
			     const RealSymMatrixArray& sum_LL,
			     const Sizet2DArray& N_L,
			     const SizetSymMatrixArray& N_LL,
			     RealSymMatrixArray& cov_LL);
  void compute_variance(const RealVector& sum_Q, const RealVector& sum_QQ,
			const SizetArray& N_Q,   RealVector& var_Q);
  void compute_variance(const RealMatrix& sum_L,
			const RealSymMatrixArray& sum_LL,
			const Sizet2DArray& N_L, RealMatrix& var_L);
  void covariance_to_correlation_sq(const RealMatrix& cov_LH,
				    const RealMatrix& var_L,
				    const RealVector& var_H,
				    RealMatrix& rho2_LH);

  void compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1,
			   Real sum_Q1Q2, Real sum_Q2Q2, size_t N_Q1,
			   size_t N_Q2, size_t N_Q1Q2, Real& var_Q2,
			   Real& rho2_Q1Q2);
  void compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2, size_t N_Q1,
			  size_t N_Q2, size_t N_Q1Q2, Real& cov_Q1Q2);
  void compute_variance(Real sum_Q, Real sum_QQ, size_t N_Q, Real& var_Q);
  
  void mfmc_eval_ratios(const RealMatrix& rho2_LH, const RealVector& cost,
			RealMatrix& eval_ratios);

  void compute_F_matrix(const RealVector& avg_eval_ratios, RealSymMatrix& F);
  void invert_CF(const RealSymMatrix& C, const RealSymMatrix& F,
		 RealSymMatrix& CF_inv);
  void compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
			size_t qoi, RealVector& A);
  void compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
			size_t qoi, Real var_H_q, RealVector& A);
  void compute_Rsq(const RealSymMatrix& CF_inv, const RealVector& A,
		   Real var_H_q, Real& R_sq_q);

  void mfmc_raw_moments(IntRealMatrixMap& sum_L_shared,
			IntRealMatrixMap& sum_L_refined,
			IntRealVectorMap& sum_H,  IntRealMatrixMap& sum_LL,
			IntRealMatrixMap& sum_LH, //const RealMatrix& rho2_LH,
			const Sizet2DArray& N_L,  const SizetArray& N_H,
			const Sizet2DArray& N_LH, RealMatrix& H_raw_mom);
  void acv_raw_moments(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		       IntRealSymMatrixArrayMap& sum_LL,
		       IntRealMatrixMap& sum_LH,
		       const RealVector& avg_eval_ratios,
		       const Sizet2DArray& N_L, const SizetArray& N_H,
		       const SizetSymMatrixArray& N_LL,
		       const Sizet2DArray& N_LH, RealMatrix& H_raw_mom);

  void compute_mfmc_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			    size_t N_L, size_t N_H, size_t N_LH, Real& beta);
  void compute_acv_control(const RealSymMatrix& cov_LL, const RealSymMatrix& F,
			   const RealMatrix& cov_LH, size_t qoi,
			   RealVector& beta);
  void compute_acv_control(RealMatrix& sum_L, Real sum_H_q,
			   RealSymMatrix& sum_LL_q, RealMatrix& sum_LH,
			   const Sizet2DArray& N_L, size_t N_H_q,
			   const SizetSymMatrix& N_LL_q,
			   const Sizet2DArray& N_LH, const RealSymMatrix& F,
			   size_t qoi, RealVector& beta);

  void apply_control(Real sum_L_shared, size_t N_shared, Real sum_L_refined,
		     size_t N_refined, Real beta, Real& H_raw_mom);

  /// helper function shared by NPSOL/OPT++ static evaluators
  Real objective_evaluator(const RealVector& avg_eval_ratios);
  /// dummy gradient evaluator function (throws an error for now)
  void gradient_evaluator(const RealVector& avg_eval_ratios,RealVector& grad_f);

  /// static function used by NPSOL for the objective function
  static void npsol_objective_evaluator(int& mode, int& n, double* x, double& f,
					double* grad_f, int& nstate);
  /// static function used by OPT++ for the objective function
  static void optpp_objective_evaluator(int mode, int n, const RealVector& x,
					double& f, RealVector& grad_f,
					int& result_mode);
  /// static function used by NPSOL for the nonlinear constraints, if present
  static void npsol_constraint_evaluator(int& mode, int& ncnln, int& n,
					 int& nrowj, int* needc, double* x,
					 double* c, double* cjac, int& nstate);
  /// static function used by OPT++ for the nonlinear constraints, if present
  static void optpp_constraint_evaluator(int mode, int n, const RealVector& x,
					 RealVector& g, RealMatrix& grad_g,
					 int& result_mode);

  //
  //- Heading: Data
  //

  /// approximate control variate algorithm selection: SUBMETHOD_ACV_{IS,MF,KL}
  unsigned short acvSubMethod;

  /// number of approximation models managed by non-hierarchical iteratedModel
  size_t numApprox;
  /// type of model sequence enumerated with primary MF/ACV loop over steps
  short sequenceType;
  /// setting for the inactive model dimension not traversed by primary MF/ACV
  /// loop over steps
  size_t secondaryIndex;

  /// variances for HF truth (length numFunctions)
  RealVector varH;
  /// covariances between each LF approximation and HF truth (the c
  /// vector in ACV); organized numFunctions x numApprox
  RealMatrix covLH;
  /// covariances among all LF approximations (the C matrix in ACV); organized
  /// as a numFunctions array of symmetic numApprox x numApprox matrices
  RealSymMatrixArray covLL;

  /// the minimizer used to minimize the estimator variance over parameters
  /// of number of truth model samples and approximation eval_ratios
  Iterator varianceMinimizer;

  /// pointer to NonDACV instance used in static member functions
  static NonDACVSampling* acvInstance;
};


inline void NonDACVSampling::
initialize_sums(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_L_refined,
		IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LH,
		RealVector& sum_HH)
{
  // sum_* are running sums across all increments
  std::pair<int, RealVector> vec_pr; std::pair<int, RealMatrix> mat_pr;
  for (int i=1; i<=4; ++i) {
    vec_pr.first = mat_pr.first = i; // moment number
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size Real{Vector,Matrix} in place and init sums to 0
    sum_L_shared.insert(mat_pr).first->second.shape(numFunctions, numApprox);
    sum_L_refined.insert(mat_pr).first->second.shape(numFunctions, numApprox);
    sum_H.insert(vec_pr).first->second.size(numFunctions);
    sum_LH.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  }
  sum_HH.size(numFunctions);
}


inline void NonDACVSampling::
initialize_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		   RealVector& sum_HH)
{
  initialize_sums(sum_L_shared, sum_L_refined, sum_H, sum_LH, sum_HH);
  std::pair<int, RealMatrix> mat_pr;
  for (int i=1; i<=4; ++i) {
    mat_pr.first = i; // moment number
    sum_LL.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  }
}


inline void NonDACVSampling::
initialize_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealMatrixMap& sum_L_refined,  IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, IntRealMatrixMap& sum_LH,
		    RealVector& sum_HH)
{
  initialize_sums(sum_L_shared, sum_L_refined, sum_H, sum_LH, sum_HH);
  std::pair<int, RealSymMatrixArray> mat_array_pr;
  for (int i=1; i<=4; ++i) {
    mat_array_pr.first = i; // moment number
    RealSymMatrixArray& mat_array = sum_LL.insert(mat_array_pr).first->second;
    mat_array.resize(numFunctions);
    for (size_t j=0; j<numFunctions; ++j)
      mat_array[j].shape(numApprox);
  }
}


inline void NonDACVSampling::
initialize_mf_counts(Sizet2DArray& N_L, SizetArray& N_H, Sizet2DArray& N_LH)
{
  N_H.assign(numFunctions, 0);
  N_L.resize(numApprox); N_LH.resize(numApprox);
  for (size_t approx=0; approx<numApprox; ++approx)
    { N_L[approx].assign(numFunctions,0); N_LH[approx].assign(numFunctions,0); }
}


inline void NonDACVSampling::
initialize_acv_counts(Sizet2DArray&        N_L,  SizetArray&   N_H,
		      SizetSymMatrixArray& N_LL, Sizet2DArray& N_LH)
{
  initialize_mf_counts(N_L, N_H, N_LH);
  N_LL.resize(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    N_LL[qoi].shape(numApprox);
}


/*
inline void NonDACVSampling::
initialize_acv_covariances(IntRealSymMatrixArrayMap covLL,
			   IntRealMatrixMap& cov_LH, IntRealVectorMap& var_H)
{
  std::pair<int, RealVector> vec_pr;  std::pair<int, RealMatrix> mat_pr;
  std::pair<int, RealSymMatrixArray> mat_array_pr;
  for (int i=1; i<=4; ++i) {
    vec_pr.first = mat_pr.first = mat_array_pr.first = i; // moment number
    var_H.insert(vec_pr).first->second.size(numFunctions);
    cov_LH.insert(mat_pr).first->second.shape(numFunctions, numApprox);
    RealSymMatrixArray& mat_array = covLL.insert(mat_array_pr).first->second;
    mat_array.resize(numFunctions);
    for (size_t j=0; j<numFunctions; ++j)
      mat_array[i].shape(numApprox);
  }
}
*/


inline void NonDACVSampling::
allocate_budget(const RealMatrix& eval_ratios, const RealVector& cost,
		RealVector& hf_targets)
{
  // Scale this profile based on specified budget (maxFunctionEvals) if needed
  // using N_H = maxFunctionEvals / cost^T eval_ratios
  // > Pilot case iter = 0: can only scale back after shared_increment().
  //   Optimal profile can be hidden by one_sided_delta() with pilot --> optimal
  //   shape emerges from initialization cost as for ML cases controlled by
  //   convTol (allow budget overshoot due to overlap of optimal with pilot,
  //   rather than strictly allocating remaining budget)

  if (hf_targets.empty()) hf_targets.sizeUninitialized(numFunctions);
  size_t qoi, approx;
  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // raw cost (un-normalized)
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * eval_ratios(qoi, approx);
    hf_targets[qoi] = budget / inner_prod * cost_H; // normalized to equivHF
  }
}


inline void NonDACVSampling::
allocate_budget(const RealVector& eval_ratios, const RealVector& cost,
		Real& hf_target)
{
  // version with scalar HF target (eval_ratios already averaged over QoI
  // due to formulation of optimization sub-problem)

  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  inner_prod = cost_H; // raw cost (un-normalized)
  for (size_t approx=0; approx<numApprox; ++approx)
    inner_prod += cost[approx] * eval_ratios[approx];
  hf_target = budget / inner_prod * cost_H; // normalized to equivHF
}


/*
inline void NonDACVSampling::
increment_samples(size_t new_N, size_t start, size_t end, SizetArray& N_l)
{
  size_t i, len = N_l.size();
  if (start > end || end > len) {
    Cerr << "Error: index range [" << start << "," << end << ") violates size ("
	 << len << ") in NonDACVSampling::increment_samples()" << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (i=start; i<end; ++i)
    N_l[i] += new_N;
}
*/


inline void NonDACVSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  size_t start, size_t end)
{
  size_t i, len = cost.length(), hf_index = len-1;
  Real cost_ref = cost[hf_index];
  if (end == len)
    { equivHFEvals += new_samp; --end; }
  for (i=start; i<end; ++i)
    equivHFEvals += (Real)new_samp * cost[i] / cost_ref;
}


inline void NonDACVSampling::
compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1, Real sum_Q1Q2,
		    Real sum_Q2Q2, size_t N_Q1, size_t N_Q2, size_t N_Q1Q2,
		    Real& var_Q2, Real& rho2_Q1Q2)
{
  Real bessel_corr_Q1   = (Real)N_Q1   / (Real)(N_Q1   - 1),
       bessel_corr_Q2   = (Real)N_Q2   / (Real)(N_Q2   - 1),
       bessel_corr_Q1Q2 = (Real)N_Q1Q2 / (Real)(N_Q1Q2 - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / N_Q1, mu_Q2 = sum_Q2 / N_Q2;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_Q1 = (sum_Q1Q1 / N_Q1   - mu_Q1 * mu_Q1) * bessel_corr_Q1,
     cov_Q1Q2 = (sum_Q1Q2 / N_Q1Q2 - mu_Q1 * mu_Q2) * bessel_corr_Q1Q2; // *** TO DO: review Bessel correction online --> not the same N to pull out over N-1 ...
  var_Q2      = (sum_Q2Q2 / N_Q2   - mu_Q2 * mu_Q2) * bessel_corr_Q2;

  //beta  = cov_Q1Q2 / var_Q1;
  rho2_Q1Q2 = cov_Q1Q2 / var_Q1 * cov_Q1Q2 / var_Q2;
}


inline void NonDACVSampling::
compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2,
		   size_t N_Q1, size_t N_Q2, size_t N_Q1Q2, Real& cov_Q1Q2)
{
  Real bessel_corr_Q1   = (Real)N_Q1   / (Real)(N_Q1   - 1),
       bessel_corr_Q2   = (Real)N_Q2   / (Real)(N_Q2   - 1),
       bessel_corr_Q1Q2 = (Real)N_Q1Q2 / (Real)(N_Q1Q2 - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / N_Q1,  mu_Q2 = sum_Q2 / N_Q2;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  cov_Q1Q2 = (sum_Q1Q2 / N_Q1Q2 - mu_Q1 * mu_Q2) * bessel_corr_Q1Q2; // *** TO DO: review Bessel correction online --> not the same N to pull out over N-1 ...
  //Cout << "compute_covariance: sum_Q1 = " << sum_Q1 << " N_Q1 = " << N_Q1
  //     << " sum_Q2 = " << sum_Q2 << " N_Q2 = " << N_Q2
  //     << " sum_Q1Q2 = " << sum_Q1Q2 << " N_Q1Q2 = " << N_Q1Q2 << std::endl;
}


inline void NonDACVSampling::
compute_variance(Real sum_Q, Real sum_QQ, size_t N_Q, Real& var_Q)
		 //size_t N_QQ, // this count is the same as N_Q
{
  Real bessel_corr_Q = (Real)N_Q / (Real)(N_Q - 1); // N_QQ is same as N_Q

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_Q = sum_Q / N_Q;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  var_Q = (sum_QQ / N_Q - mu_Q * mu_Q) * bessel_corr_Q;
  //Cout << "compute_variance: sum_Q = " << sum_Q << " N_Q = " << N_Q
  //     << " sum_QQ = " << sum_QQ << " N_QQ = " << N_Q << std::endl;
}


inline void NonDACVSampling::
compute_variance(const RealVector& sum_Q, const RealVector& sum_QQ,
		 const SizetArray& N_Q,   RealVector& var_Q)
{
  if (var_Q.empty()) var_Q.sizeUninitialized(numFunctions);

  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_variance(sum_Q[qoi], sum_QQ[qoi], N_Q[qoi], var_Q[qoi]);
}


inline void NonDACVSampling::
compute_variance(const RealMatrix& sum_L, const RealSymMatrixArray& sum_LL,
		 const Sizet2DArray& N_L, RealMatrix& var_L)
{
  if (var_L.empty()) var_L.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;
  for (qoi=0; qoi<numFunctions; ++qoi)
    for (approx=0; approx<numApprox; ++approx)
      compute_variance(sum_L(qoi,approx), sum_LL[qoi](approx,approx),
		       N_L[approx][qoi], var_L(qoi,approx));
}


inline void NonDACVSampling::
covariance_to_correlation_sq(const RealMatrix& cov_LH, const RealMatrix& var_L,
			     const RealVector& var_H, RealMatrix& rho2_LH)
{
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;  Real var_H_q, cov_LH_aq;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    var_H_q = var_H[qoi];
    for (approx=0; approx<numApprox; ++approx) {
      cov_LH_aq = cov_LH(qoi,approx);
      rho2_LH(qoi,approx) = cov_LH_aq / var_L(qoi,approx) * cov_LH_aq / var_H_q;
    }
  }
}


inline void NonDACVSampling::
compute_mfmc_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		     size_t N_L, size_t N_H, size_t N_LH, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real bessel_corr_L  = (Real)N_L  / (Real)(N_L  - 1),
     //bessel_corr_H  = (Real)N_H  / (Real)(N_H  - 1);
       bessel_corr_LH = (Real)N_LH / (Real)(N_LH - 1);
  Real  mu_L  =  sum_L  / N_L,   mu_H = sum_H / N_H,
       var_L  = (sum_LL / N_L  - mu_L * mu_L) * bessel_corr_L,
     //var_H  = (sum_HH / N_H  - mu_H * mu_H) * bessel_corr_H,
       cov_LH = (sum_LH / N_LH - mu_L * mu_H) * bessel_corr_LH;

  // beta^* = rho_LH sigma_H / sigma_L (same expression as two model case)
  //        = cov_LH / var_L  (since rho_LH = cov_LH / sigma_H / sigma_L)
  // Allow different sample counts --> don't cancel bessel_corr:
  beta = cov_LH / var_L;
}


inline void NonDACVSampling::
compute_F_matrix(const RealVector& avg_eval_ratios, RealSymMatrix& F)
{
  size_t i, j, num_approx = avg_eval_ratios.length();
  if (F.empty()) F.shapeUninitialized(num_approx);

  switch (acvSubMethod) {
  case SUBMETHOD_ACV_IS: {
    Real wi_ratio;
    for (i=0; i<num_approx; ++i) {
      F(i,i)   = wi_ratio = (avg_eval_ratios[i] - 1.) / avg_eval_ratios[i];
      for (j=0; j<i; ++j)
	F(i,j) = wi_ratio * (avg_eval_ratios[j] - 1.) / avg_eval_ratios[j];
    }
    break;
  }
  case SUBMETHOD_ACV_MF: {
    Real wi, min_w;
    for (i=0; i<num_approx; ++i) {
      wi = avg_eval_ratios[i];  F(i,i) = (wi - 1.) / wi;
      for (j=0; j<i; ++j) {
	min_w = std::min(wi, avg_eval_ratios[j]);
	F(i,j) = (min_w - 1.) / min_w;
      }
    }
    break;
  }
  default:
    Cerr << "Error: bad sub-method name (" << acvSubMethod
	 << ") in NonDACVSampling::compute_F_matrix()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }
}


inline void NonDACVSampling::
invert_CF(const RealSymMatrix& C, const RealSymMatrix& F, RealSymMatrix& CF_inv)
{
  size_t i, j, n = C.numRows();
  if (CF_inv.empty()) CF_inv.shapeUninitialized(n);

  for (i=0; i<n; ++i)
    for (j=0; j<=i; ++j) {
      CF_inv(i,j) = C(i,j) * F(i,j);
      //Cout << "invert_CF: C(" << i << ',' << j << ") = " << C(i,j)
      //     << " F("  << i << ',' << j << ") = " << F(i,j)
      //     << " CF(" << i << ',' << j << ") = " << CF_inv(i,j) << '\n';
    }

  RealSpdSolver spd_solver;
  spd_solver.setMatrix(Teuchos::rcp(&CF_inv, false));
  spd_solver.invert(); // in place
}


inline void NonDACVSampling::
compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
		 size_t qoi, RealVector& A)
{
  size_t i, num_approx = F.numRows();
  if (A.length() != num_approx) A.sizeUninitialized(num_approx);

  for (i=0; i<num_approx; ++i) // diag(F) o c-bar
    A[i] = F(i,i) * c(qoi, i); // this version defers c-bar scaling
}


inline void NonDACVSampling::
compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
		 size_t qoi, Real var_H_q, RealVector& A)
{
  compute_A_vector(F, c, qoi, A); // first use unscaled overload
  A.scale(1./std::sqrt(var_H_q)); // scale from c to c-bar
}


inline void NonDACVSampling::
compute_Rsq(const RealSymMatrix& CF_inv, const RealVector& A, Real var_H_q,
	    Real& R_sq_q)
{
  RealSymMatrix trip(1, false);
  Teuchos::symMatTripleProduct(Teuchos::TRANS, 1./var_H_q, CF_inv, A, trip);
  R_sq_q = trip(0,0);

  /*
  size_t i, j, num_approx = CF_inv.numRows();
  R_sq_q = 0.;
  for (i=0; i<num_approx; ++i)
    for (j=0; j<num_approx; ++j)
      R_sq_q += A[i] * CF_inv(i,j) * A[j];
  R_sq_q /= varH[qoi]; // c-bar normalization
  */
}


inline void NonDACVSampling::
compute_acv_control(const RealSymMatrix& cov_LL, const RealSymMatrix& F,
		    const RealMatrix& cov_LH, size_t qoi, RealVector& beta)
{
  RealSymMatrix CF_inv;  RealVector A;
  invert_CF(cov_LL, F, CF_inv);
  //Cout << "compute_acv_control qoi " << qoi+1 << ": CF_inv\n" << CF_inv;
  compute_A_vector(F, cov_LH, qoi, A); // no scaling (uses c, not c-bar)
  //Cout << "compute_acv_control qoi " << qoi+1 << ": A\n" << A;

  size_t n = F.numRows();
  if (beta.length() != n) beta.size(n);
  beta.multiply(Teuchos::LEFT_SIDE, 1., CF_inv, A, 0.); // for SymMatrix mult
  //Cout << "compute_acv_control qoi " << qoi+1 << ": beta\n" << beta;
}


inline void NonDACVSampling::
compute_acv_control(RealMatrix& sum_L, Real sum_H_q, RealSymMatrix& sum_LL_q,
		    RealMatrix& sum_LH, const Sizet2DArray& N_L, size_t N_H_q,
		    const SizetSymMatrix& N_LL_q, const Sizet2DArray& N_LH,
		    const RealSymMatrix& F, size_t qoi, RealVector& beta)
{
  // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
  // > cov_LH is sized for all qoi but only 1 row is used
  size_t approx, approx2, N_L_aq;  Real sum_L_aq;
  RealSymMatrix cov_LL(numApprox); RealMatrix cov_LH(numFunctions, numApprox);

  for (approx=0; approx<numApprox; ++approx) {
    N_L_aq = N_L[approx][qoi];  sum_L_aq = sum_L(qoi,approx);
    compute_covariance(sum_L_aq, sum_H_q, sum_LH(qoi,approx), /*N_L_aq*/N_H_q,
		       N_H_q, N_LH[approx][qoi], cov_LH(qoi,approx));
    compute_variance(sum_L_aq, sum_LL_q(approx,approx), /*N_L_aq*/N_H_q,
		     cov_LL(approx,approx));
    for (approx2=0; approx2<approx; ++approx2)
      compute_covariance(sum_L_aq, sum_L(qoi,approx2), sum_LL_q(approx,approx2),
			 /*N_L_aq*/N_H_q, /*N_L[approx2][qoi]*/N_H_q, N_LL_q(approx,approx2),
			 cov_LL(approx,approx2));
  }

  // forward to overload:
  compute_acv_control(cov_LL, F, cov_LH, qoi, beta);
}


inline void NonDACVSampling::
apply_control(Real sum_L_shared, size_t N_shared, Real sum_L_refined,
	      size_t N_refined, Real beta, Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom -= beta * (sum_L_shared  / N_shared -  // mu from shared samples
		       sum_L_refined / N_refined); // refined mu incl increment
}

} // namespace Dakota

#endif
