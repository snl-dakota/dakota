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

#include "NonDNonHierarchSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {

// special values for optSubProblemForm
enum { R_ONLY_LINEAR_CONSTRAINT=1, N_VECTOR_LINEAR_CONSTRAINT,
       R_AND_N_NONLINEAR_CONSTRAINT };

#define RATIO_NUDGE 1.e-4


/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

/** Approximate Control Variate (ACV) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDACVSampling: public NonDNonHierarchSampling
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
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  void print_variance_reduction(std::ostream& s);

  //
  //- Heading: member functions
  //

  void approximate_control_variate();
  void approximate_control_variate_offline_pilot();
  void approximate_control_variate_pilot_projection();

  bool approx_increment(const RealVector& avg_eval_ratios,
			const Sizet2DArray& N_L_refined, Real hf_target,
			size_t iter, size_t start, size_t end);
  void approx_increments(IntRealMatrixMap& sum_L_baselineH,
			 IntRealVectorMap& sum_H,
			 IntRealSymMatrixArrayMap& sum_LL,
			 IntRealMatrixMap& sum_LH,
			 const Sizet2DArray& N_L_baselineH,
			 const SizetSymMatrixArray& N_LL,
			 const Sizet2DArray& N_LH,
			 const RealVector& avg_eval_ratios, Real avg_hf_target);

  Real allocate_budget(const RealVector& avg_eval_ratios,
		       const RealVector& cost);
  void scale_to_budget_with_pilot(Real budget, RealVector& avg_eval_ratios,
				  const RealVector& cost, Real avg_N_H);

  void compute_ratios(const SizetArray& N_H,   const RealMatrix& var_L,
		      const RealVector& var_H, const RealMatrix& cov_LH,
		      const RealVector& cost,  RealVector& avg_eval_ratios,
		      Real& avg_acv_estvar,    Real& avg_estvar_ratio,
		      Real& avg_hf_target);

private:

  //
  //- Heading: Helper functions
  //

  void initialize_acv_sums(IntRealMatrixMap& sum_L_baseline,
			   IntRealVectorMap& sum_H,
			   IntRealSymMatrixArrayMap& sum_LL,
			   IntRealMatrixMap& sum_LH, RealVector& sum_HH);

  void initialize_acv_counts(Sizet2DArray& num_L_baseline, SizetArray&  num_H,
			     SizetSymMatrixArray& num_LL, Sizet2DArray& num_LH);

  //void initialize_acv_covariances(IntRealSymMatrixArrayMap covLL,
  //				  IntRealMatrixMap& cov_LH,
  //				  IntRealVectorMap& var_H);

  // shared_increment() cases:
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline,
			   IntRealVectorMap& sum_H,
			   IntRealSymMatrixArrayMap& sum_LL,
			   IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			   Sizet2DArray& num_L_baseline,  SizetArray& num_H,
			   SizetSymMatrixArray& num_LL, Sizet2DArray& num_LH);
  void accumulate_acv_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
			   RealSymMatrixArray& sum_LL, RealMatrix& sum_LH,
			   RealVector& sum_HH, Sizet2DArray& num_L_baseline,
			   SizetArray& num_H,  SizetSymMatrixArray& num_LL,
			   Sizet2DArray& num_LH);
  // shared_approx_increment() case:
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline,
			   IntRealSymMatrixArrayMap& sum_LL,
			   Sizet2DArray& num_L_baseline,
			   SizetSymMatrixArray& num_LL);
  // approx_increment() cases:
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_refined,
			   Sizet2DArray& num_L_refined,
			   size_t approx_start, size_t approx_end);

  void compute_LH_covariance(const RealMatrix& sum_L_shared,
			     const RealVector& sum_H,  const RealMatrix& sum_LH,
			     const Sizet2DArray& num_L, const SizetArray& num_H,
			     const Sizet2DArray& num_LH, RealMatrix& cov_LH);
  void compute_LL_covariance(const RealMatrix& sum_L_shared,
			     const RealSymMatrixArray& sum_LL,
			     const Sizet2DArray& num_L,
			     const SizetSymMatrixArray& num_LL,
			     RealSymMatrixArray& cov_LL);
  void covariance_to_correlation_sq(const RealMatrix& cov_LH,
				    const RealMatrix& var_L,
				    const RealVector& var_H,
				    RealMatrix& rho2_LH);

  void compute_L_variance(const RealMatrix& sum_L,
			  const RealSymMatrixArray& sum_LL,
			  const Sizet2DArray& num_L, RealMatrix& var_L);

  void compute_F_matrix(const RealVector& avg_eval_ratios, RealSymMatrix& F);
  void invert_CF(const RealSymMatrix& C, const RealSymMatrix& F,
		 RealSymMatrix& CF_inv);
  void compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
			size_t qoi, RealVector& A);
  void compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
			size_t qoi, Real var_H_q, RealVector& A);
  void compute_Rsq(const RealSymMatrix& CF_inv, const RealVector& A,
		   Real var_H_q, Real& R_sq_q);

  void acv_raw_moments(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		       IntRealSymMatrixArrayMap& sum_LL,
		       IntRealMatrixMap& sum_LH,
		       const RealVector& avg_eval_ratios,
		       const Sizet2DArray& num_L_shared,
		       const Sizet2DArray& num_L_refined,
		       const SizetArray& num_H,
		       const SizetSymMatrixArray& num_LL,
		       const Sizet2DArray& num_LH, RealMatrix& H_raw_mom);

  void compute_acv_control(const RealSymMatrix& cov_LL, const RealSymMatrix& F,
			   const RealMatrix& cov_LH, size_t qoi,
			   RealVector& beta);
  void compute_acv_control(RealMatrix& sum_L, Real sum_H_q,
			   RealSymMatrix& sum_LL_q, RealMatrix& sum_LH,
			   const Sizet2DArray& num_L, size_t num_H_q,
			   const SizetSymMatrix& num_LL_q,
			   const Sizet2DArray& num_LH, const RealSymMatrix& F,
			   size_t qoi, RealVector& beta);

  void update_projected_samples(Real avg_hf_target,
				const RealVector& avg_eval_ratios,
				SizetArray& N_H_projected,
				Sizet2DArray& N_L_projected);

  /// objective helper function shared by NPSOL/OPT++ static evaluators
  Real objective_function(const RealVector& r_and_N);
  //void objective_gradient(const RealVector& r_and_N, RealVector& obj_grad);
  /// constraint helper function shared by NPSOL/OPT++ static evaluators
  Real nonlinear_constraint(const RealVector& r_and_N);
  /// constraint gradient helper function shared by NPSOL/OPT++
  /// static evaluators
  void nonlinear_constraint_gradient(const RealVector& r_and_N,
				     RealVector& grad_c);

  /// static function used by NPSOL for the objective function
  static void npsol_objective_evaluator(int& mode, int& n, double* x, double& f,
					double* grad_f, int& nstate);
  /// static function used by OPT++ for the objective function
  static void optpp_objective_evaluator(int n, const RealVector& x,
					double& f, int& result_mode);
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
  /// formulation for optimization sub-problem that minimizes R^2 subject
  /// to different variable sets and different linear/nonlinear constraints
  unsigned short optSubProblemForm;
  /// user specification to suppress any increments in the number of HF
  /// evaluations (e.g., because too expensive and no more can be performed)
  bool truthFixedByPilot;

  /// covariances between each LF approximation and HF truth (the c
  /// vector in ACV); organized numFunctions x numApprox
  RealMatrix covLH;
  /// covariances among all LF approximations (the C matrix in ACV); organized
  /// as a numFunctions array of symmetic numApprox x numApprox matrices
  RealSymMatrixArray covLL;

  /// the minimizer used to minimize the estimator variance over parameters
  /// of number of truth model samples and approximation eval_ratios
  Iterator varianceMinimizer;

  /// final estimator variance (minimization result), averaged across QoI
  Real avgACVEstVar;
  /// ratio of final ACV estimator variance (minimization result averaged
  /// across QoI) and final MC estimator variance (final varH / numH)
  Real avgMSERatio;

  /// pointer to NonDACV instance used in static member functions
  static NonDACVSampling* acvInstance;
};


inline void NonDACVSampling::
initialize_acv_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, IntRealMatrixMap& sum_LH,
		    RealVector& sum_HH)
{
  initialize_sums(sum_L_baseline, sum_H, sum_LH, sum_HH);
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
initialize_acv_counts(Sizet2DArray& num_L_baseline,  SizetArray& num_H,
		      SizetSymMatrixArray& num_LL, Sizet2DArray& num_LH)
{
  initialize_counts(num_L_baseline, num_H, num_LH);
  num_LL.resize(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    num_LL[qoi].shape(numApprox);
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


inline Real NonDACVSampling::
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


inline void NonDACVSampling::
scale_to_budget_with_pilot(Real budget, RealVector& avg_eval_ratios,
			   const RealVector& cost, Real avg_N_H)
{
  // retain the shape of an r* profile, but scale to budget constrained by
  // incurred pilot cost

  Real inner_prod = 0., cost_H = cost[numApprox], r_i, cost_r_i, factor;
  for (size_t approx=0; approx<numApprox; ++approx)
    inner_prod += cost[approx] * avg_eval_ratios[approx]; // Sum(w_i r_i)
  factor = (budget / avg_N_H - 1.) / inner_prod * cost_H;
  //avg_eval_ratios.scale(factor); // can result in infeasible r_i < 1

  for (int i=numApprox-1; i>=0; --i) {
    r_i = avg_eval_ratios[i] * factor;
    if (r_i <= 1.) { // fix at 1+NUDGE and scale remaining r_i to reduced budget
      cost_r_i  = avg_eval_ratios[i] = 1. + RATIO_NUDGE;
      cost_r_i *= cost[i];
      budget -= avg_N_H * cost_r_i / cost_H;  inner_prod -= cost_r_i;
      factor = (budget / avg_N_H - 1.) / inner_prod * cost_H;
    }
    else
      avg_eval_ratios[i] = r_i;
    //Cout << " avg_eval_ratios[" << i << "] = " << avg_eval_ratios[i] << '\n';
  }
}


inline void NonDACVSampling::
compute_L_variance(const RealMatrix& sum_L, const RealSymMatrixArray& sum_LL,
		   const Sizet2DArray& num_L, RealMatrix& var_L)
{
  if (var_L.empty()) var_L.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;
  for (qoi=0; qoi<numFunctions; ++qoi)
    for (approx=0; approx<numApprox; ++approx)
      compute_variance(sum_L(qoi,approx), sum_LL[qoi](approx,approx),
		       num_L[approx][qoi], var_L(qoi,approx));
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
compute_F_matrix(const RealVector& r_and_N, RealSymMatrix& F)
{
  size_t i, j;
  if (F.empty()) F.shapeUninitialized(numApprox);

  switch (acvSubMethod) {
  case SUBMETHOD_ACV_IS: {
    Real ri_ratio;
    for (i=0; i<numApprox; ++i) {
      F(i,i)   = ri_ratio = (r_and_N[i] - 1.) / r_and_N[i];
      for (j=0; j<i; ++j)
	F(i,j) = ri_ratio * (r_and_N[j] - 1.) / r_and_N[j];
    }
    break;
  }
  case SUBMETHOD_ACV_MF: {
    Real ri, min_r;
    for (i=0; i<numApprox; ++i) {
      ri = r_and_N[i];  F(i,i) = (ri - 1.) / ri;
      for (j=0; j<i; ++j) {
	min_r = std::min(ri, r_and_N[j]);
	F(i,j) = (min_r - 1.) / min_r;
      }
    }
    break;
  }
  default:
    Cerr << "Error: bad sub-method name (" << acvSubMethod
	 << ") in NonDACVSampling::compute_F_matrix()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "F matrix for sub-method " << acvSubMethod << ":\n" << F
	 << std::endl;
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
		    RealMatrix& sum_LH, const Sizet2DArray& num_L,
		    size_t num_H_q, const SizetSymMatrix& num_LL_q,
		    const Sizet2DArray& num_LH, const RealSymMatrix& F,
		    size_t qoi, RealVector& beta)
{
  // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
  // > cov_LH is sized for all qoi but only 1 row is used
  size_t approx, approx2, num_L_aq;  Real sum_L_aq;
  RealSymMatrix cov_LL(numApprox); RealMatrix cov_LH(numFunctions, numApprox);

  for (approx=0; approx<numApprox; ++approx) {
    num_L_aq = num_L[approx][qoi];  sum_L_aq = sum_L(qoi,approx);
    compute_covariance(sum_L_aq, sum_H_q, sum_LH(qoi,approx), num_L_aq,
		       num_H_q, num_LH[approx][qoi], cov_LH(qoi,approx));
    compute_variance(sum_L_aq, sum_LL_q(approx,approx), num_L_aq,
		     cov_LL(approx,approx));
    for (approx2=0; approx2<approx; ++approx2)
      compute_covariance(sum_L_aq, sum_L(qoi,approx2), sum_LL_q(approx,approx2),
			 num_L_aq, num_L[approx2][qoi],num_LL_q(approx,approx2),
			 cov_LL(approx,approx2));
  }

  // forward to overload:
  compute_acv_control(cov_LL, F, cov_LH, qoi, beta);
}

} // namespace Dakota

#endif
