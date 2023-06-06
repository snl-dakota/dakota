/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
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

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  Real estimator_accuracy_metric();
  //Real estimator_cost_metric();
  void print_variance_reduction(std::ostream& s);

  void estimator_variance_ratios(const RealVector& r_and_N,
				 RealVector& estvar_ratios);

  void augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				       RealVector& lin_ineq_lb,
				       RealVector& lin_ineq_ub);

  //
  //- Heading: member functions
  //

  void initialize_acv_sums(RealMatrix& sum_L, RealVector& sum_H,
			   RealSymMatrixArray& sum_LL, RealMatrix& sum_LH,
			   RealVector& sum_HH);
  void initialize_acv_sums(IntRealMatrixMap& sum_L_baseline,
			   IntRealVectorMap& sum_H,
			   IntRealSymMatrixArrayMap& sum_LL,
			   IntRealMatrixMap& sum_LH, RealVector& sum_HH);

  void evaluate_pilot(RealMatrix& sum_L_pilot, RealVector& sum_H_pilot,
		      RealSymMatrixArray& sum_LL_pilot,
		      RealMatrix& sum_LH_pilot, RealVector& sum_HH_pilot,
		      SizetArray& N_shared_pilot, bool incr_cost);

  void compute_LH_statistics(RealMatrix& sum_L_pilot, RealVector& sum_H_pilot,
			     RealSymMatrixArray& sum_LL_pilot,
			     RealMatrix& sum_LH_pilot, RealVector& sum_HH_pilot,
			     SizetArray& N_shared_pilot, RealMatrix& var_L,
			     RealVector& var_H, RealSymMatrixArray& cov_LL,
			     RealMatrix& cov_LH);

  // shared_increment() cases:
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline,
			   IntRealVectorMap& sum_H,
			   IntRealSymMatrixArrayMap& sum_LL,
			   IntRealMatrixMap& sum_LH, RealVector& sum_HH,
			   SizetArray& N_shared);
  // approx_increment() cases:
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_refined,
			   Sizet2DArray& N_L_refined,
			   const SizetArray& approx_sequence,
			   size_t approx_start, size_t approx_end);
  void accumulate_acv_sums(IntRealMatrixMap& sum_L, Sizet2DArray& N_L_actual,
			   const RealVector& fn_vals, size_t approx);

  bool acv_approx_increment(const DAGSolutionData& soln,
			    const Sizet2DArray& N_L_actual_refined,
			    SizetArray& N_L_alloc_refined, size_t iter,
			    const SizetArray& approx_sequence,
			    size_t start, size_t end);

  void acv_raw_moments(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined,
		       IntRealVectorMap& sum_H,
		       IntRealSymMatrixArrayMap& sum_LL,
		       IntRealMatrixMap& sum_LH,
		       const RealVector& avg_eval_ratios,
		       const SizetArray& N_shared,
		       const Sizet2DArray& N_L_refined,
		       RealMatrix& H_raw_mom);

  void compute_acv_control_covariances(RealMatrix& sum_L, Real sum_H_q,
				       RealSymMatrix& sum_LL_q,
				       RealMatrix& sum_LH, size_t N_shared_q,
				       size_t qoi, RealSymMatrix& cov_LL,
				       RealMatrix& cov_LH);

  void update_projected_lf_samples(Real avg_hf_targets,
				   const RealVector& avg_eval_ratios,
				   const SizetArray& N_H_actual,
				   size_t& N_H_alloc,
				   //SizetArray& delta_N_L_actual,
				   Real& delta_equiv_hf);
  void update_projected_samples(Real avg_hf_targets,
				const RealVector& avg_eval_ratios,
				const SizetArray& N_H_actual, size_t& N_H_alloc,
				size_t& delta_N_H_actual,
				//SizetArray& delta_N_L_actual,
				Real& delta_equiv_hf);

  Real update_hf_target(const RealVector& avg_eval_ratios,
			const RealVector& var_H, const RealVector& estvar0);

  void covariance_to_correlation_sq(const RealMatrix& cov_LH,
				    const RealMatrix& var_L,
				    const RealVector& var_H,
				    RealMatrix& rho2_LH);

  void cache_mc_reference();

  void analytic_initialization_from_mfmc(Real avg_N_H, DAGSolutionData& soln);

  void pick_mfmc_cvmc_solution(const DAGSolutionData& mf_soln, size_t mf_samp,
			       const DAGSolutionData& cv_soln, size_t cv_samp,
			       DAGSolutionData& soln, size_t& num_samp);

  void print_computed_solution(std::ostream& s, const DAGSolutionData& soln);

private:

  //
  //- Heading: Helper functions
  //

  void approximate_control_variate_online_pilot();
  void approximate_control_variate_offline_pilot();
  void approximate_control_variate_pilot_projection();

  void approx_increments(IntRealMatrixMap& sum_L_baselineH,
			 IntRealVectorMap& sum_H,
			 IntRealSymMatrixArrayMap& sum_LL,
			 IntRealMatrixMap& sum_LH, const SizetArray& N_H_actual,
			 size_t N_H_alloc, const DAGSolutionData& soln);

  void precompute_acv_control(const RealVector& avg_eval_ratios,
			      const SizetArray& N_shared);

  void compute_acv_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
			   RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
			   size_t N_shared_q, size_t mom, size_t qoi,
			   RealVector& beta);

  void analytic_initialization_from_ensemble_cvmc(Real avg_N_H,
						  DAGSolutionData& soln);
  void cvmc_ensemble_solutions(const RealMatrix& rho2_LH,
			       const RealVector& cost, DAGSolutionData& soln);

  void initialize_acv_counts(SizetArray& num_H, SizetSymMatrixArray& num_LL);

  //void initialize_acv_covariances(IntRealSymMatrixArrayMap covLL,
  //				  IntRealMatrixMap& cov_LH,
  //				  IntRealVectorMap& var_H);

  // shared_increment() cases:
  void accumulate_acv_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
			   RealSymMatrixArray& sum_LL, RealMatrix& sum_LH,
			   RealVector& sum_HH, SizetArray& N_shared);
  // shared_approx_increment() case:
  void accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline,
			   IntRealSymMatrixArrayMap& sum_LL,
			   Sizet2DArray& N_L_shared);
  // approx_increment() cases:

  void compute_LH_covariance(const RealMatrix& sum_L_shared,
			     const RealVector& sum_H, const RealMatrix& sum_LH,
			     const SizetArray& N_shared, RealMatrix& cov_LH);
  void compute_LL_covariance(const RealMatrix& sum_L_shared,
			     const RealSymMatrixArray& sum_LL,
			     const SizetArray& N_shared,
			     RealSymMatrixArray& cov_LL);

  void compute_L_variance(const RealMatrix& sum_L,
			  const RealSymMatrixArray& sum_LL,
			  const SizetArray& num_L, RealMatrix& var_L);

  void compute_F_matrix(const RealVector& avg_eval_ratios, RealSymMatrix& F);
  /*
  void invert_CF(const RealSymMatrix& C, const RealSymMatrix& F,
		 RealSymMatrix& CF_inv);
  void compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
			size_t qoi, RealVector& A);
  void compute_A_vector(const RealSymMatrix& F, const RealMatrix& c,
			size_t qoi, Real var_H_q, RealVector& A);
  Real compute_R_sq(const RealSymMatrix& CF_inv, const RealVector& A,
		    Real var_H_q);
  */
  void compute_C_F_c_f(const RealSymMatrix& C, const RealSymMatrix& F,
		       const RealMatrix& c, size_t qoi,
		       RealSymMatrix& C_F, RealVector& c_f);
  void solve_for_C_F_c_f(RealSymMatrix& C_F, RealVector& c_f,
			 RealVector& lhs, bool copy_C_F = true,
			 bool copy_c_f = true);
  Real solve_for_triple_product(const RealSymMatrix& C, const RealSymMatrix& F,
				const RealMatrix&    c, size_t qoi);
  Real compute_R_sq(const RealSymMatrix& C, const RealSymMatrix& F,
		    const RealMatrix& c, size_t qoi, Real var_H_q);

  void compute_ratios(const RealMatrix& var_L, DAGSolutionData& soln);

  void acv_estvar_ratios(const RealSymMatrix& F, RealVector& estvar_ratios);
  //Real acv_estimator_variance(const RealVector& avg_eval_ratios,
  //			        Real avg_hf_target);

  void solve_for_acv_control(const RealSymMatrix& cov_LL,
			     const RealSymMatrix& F, const RealMatrix& cov_LH,
			     size_t qoi, RealVector& beta);

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& avg_hf_target);

  //
  //- Heading: Data
  //

  // option for performing multiple ACV optimizations and taking the best
  //bool multiStartACV;

  /// the "F" matrix from Gorodetsky JCP paper
  RealSymMatrix FMat;

  /// final solution data for ACV (default DAG = numApprox,...,numApprox)
  DAGSolutionData acvSolnData;
};


inline Real NonDACVSampling::estimator_accuracy_metric()
{ return acvSolnData.avgEstVar; }


//inline Real NonDACVSampling::estimator_cost_metric()
//{ return mfmcSolnData.equivHFAlloc; }


inline void NonDACVSampling::
initialize_acv_sums(RealMatrix& sum_L, RealVector& sum_H,
		    RealSymMatrixArray& sum_LL, RealMatrix& sum_LH,
		    RealVector& sum_HH)
{
  sum_L.shape(numFunctions, numApprox);
  sum_H.size(numFunctions);
  sum_LL.resize(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    sum_LL[qoi].shape(numApprox);
  sum_LH.shape(numFunctions, numApprox);
  sum_HH.size(numFunctions);
}


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
initialize_acv_counts(SizetArray& num_H, SizetSymMatrixArray& num_LL)
{
  num_H.assign(numFunctions, 0);
  num_LL.resize(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    num_LL[qoi].shape(numApprox);
}


/*
inline void NonDACVSampling::
initialize_acv_counts(Sizet2DArray& num_L_baseline,  SizetArray& num_H,
		      SizetSymMatrixArray& num_LL, Sizet2DArray& num_LH)
{
  initialize_counts(num_L_baseline, num_H, num_LH);
  num_LL.resize(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    num_LL[qoi].shape(numApprox);
}


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
compute_L_variance(const RealMatrix& sum_L, const RealSymMatrixArray& sum_LL,
		   const SizetArray& num_L, RealMatrix& var_L)
{
  if (var_L.empty()) var_L.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx, num_L_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    num_L_q = num_L[qoi];
    for (approx=0; approx<numApprox; ++approx)
      compute_variance(sum_L(qoi,approx), sum_LL[qoi](approx,approx),
		       num_L_q, var_L(qoi,approx));
  }
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
scale_to_target(Real avg_N_H, const RealVector& cost,
		RealVector& avg_eval_ratios, Real& avg_hf_target)
{
  // scale to enforce budget constraint.  Since the profile does not emerge
  // from pilot in ACV, don't select an infeasible initial guess:
  // > if N* < N_pilot, scale back r* --> initial = scaled_r*,N_pilot
  // > if N* > N_pilot, use initial = r*,N*
  avg_hf_target = allocate_budget(avg_eval_ratios, cost); // r* --> N*
  if (pilotMgmtMode == OFFLINE_PILOT) {
    Real offline_N_lwr = 2.;
    if (avg_N_H < offline_N_lwr) avg_N_H = offline_N_lwr;
  }
  if (avg_N_H > avg_hf_target) {// replace N* with N_pilot, rescale r* to budget
    avg_hf_target = avg_N_H;
    scale_to_budget_with_pilot(avg_eval_ratios, cost, avg_hf_target);
  }
}


inline void NonDACVSampling::
compute_F_matrix(const RealVector& r_and_N, RealSymMatrix& F)
{
  size_t i, j;
  if (F.empty()) F.shapeUninitialized(numApprox);

  switch (mlmfSubMethod) {
  case SUBMETHOD_MFMC: { // diagonal (see Eq. 16 in JCP ACV paper)
    size_t num_am1 = numApprox - 1;  Real r_i, r_ip1;
    for (i=0; i<num_am1; ++i) {
      r_i = r_and_N[i]; r_ip1 = r_and_N[i+1];
      F(i,i) = (r_i - r_ip1) / (r_i * r_ip1);
    }
    r_i = r_and_N[num_am1]; //r_ip1 = 1.;
    F(num_am1,num_am1) = (r_i - 1.) / r_i;
    break;
  }
  case SUBMETHOD_ACV_IS: { // Eq. 30
    Real ri_ratio;
    for (i=0; i<numApprox; ++i) {
      F(i,i)   = ri_ratio = (r_and_N[i] - 1.) / r_and_N[i];
      for (j=0; j<i; ++j)
	F(i,j) = ri_ratio * (r_and_N[j] - 1.) / r_and_N[j];
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // Eq. 34
    Real r_i, min_r;
    for (i=0; i<numApprox; ++i) {
      r_i = r_and_N[i];  F(i,i) = (r_i - 1.) / r_i;
      for (j=0; j<i; ++j) {
	min_r = std::min(r_i, r_and_N[j]);
	F(i,j) = (min_r - 1.) / min_r;
      }
    }
    break;
  }
  //case SUBMETHOD_ACV_RD: // TO DO
  default:
    Cerr << "Error: bad sub-method name (" << mlmfSubMethod
	 << ") in NonDACVSampling::compute_F_matrix()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "F matrix for sub-method " << mlmfSubMethod << ":\n" << F
	 << std::endl;
}


/*
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
  if (spd_solver.shouldEquilibrate()) {
    spd_solver.factorWithEquilibration(true);
    //if (outputLevel >= DEBUG_OUTPUT) {
    //  Real rcond;  spd_solver.reciprocalConditionEstimate(rcond);
    //  Cout << "Equilibrating in NonDACVSampling::invert_C_F(): reciprocal "
    //	     << "condition number = " << rcond << std::endl;
    //}
  }
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


inline Real NonDACVSampling::
compute_R_sq(const RealSymMatrix& CF_inv, const RealVector& A, Real var_H_q)
{
  RealSymMatrix trip(1, false);
  Teuchos::symMatTripleProduct(Teuchos::TRANS, 1./var_H_q, CF_inv, A, trip);
  return trip(0,0);

  //size_t i, j, num_approx = CF_inv.numRows();
  //Real sum, approx_inner_prod = 0.;
  //for (i=0; i<num_approx; ++i) {
  //  sum = 0.;
  //  for (j=0; j<num_approx; ++j)
  //    sum += CF_inv(i,j) * A[j];
  //  approx_inner_prod += A[i] * sum;
  //}
  //return approx_inner_prod / varH[qoi]; // c-bar normalization
}


inline void NonDACVSampling::
acv_estvar_ratios(const RealSymMatrix& F, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  RealSymMatrix CF_inv;  RealVector A;  Real R_sq;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    invert_CF(covLL[qoi], F, CF_inv);
    //Cout << "Objective eval: CF inverse =\n" << CF_inv << std::endl;
    compute_A_vector(F, covLH, qoi, A);    // defer c-bar scaling
    //Cout << "Objective eval: A =\n" << A << std::endl;
    R_sq = compute_R_sq(CF_inv, A, varH[qoi]); // apply scaling^2
    //Cout << "Objective eval: varH[" << qoi << "] = " << varH[qoi]
    //     << " Rsq[" << qoi << "] =\n" << R_sq << std::endl;
    estvar_ratios[qoi] = (1. - R_sq);
  }
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
*/


inline void NonDACVSampling::
compute_C_F_c_f(const RealSymMatrix& C, const RealSymMatrix& F,
		const RealMatrix& c, size_t qoi,
		RealSymMatrix& C_F, RealVector& c_f)
{
  size_t i, j, n = C.numRows();
  C_F.shapeUninitialized(n);  c_f.sizeUninitialized(n);
  for (i=0; i<n; ++i) {
    c_f[i] = c(qoi, i) * F(i,i);
    for (j=0; j<=i; ++j)
      C_F(i,j) = C(i,j) * F(i,j);
  }
}


inline void NonDACVSampling::
solve_for_C_F_c_f(RealSymMatrix& C_F, RealVector& c_f, RealVector& lhs,
		  bool copy_C_F, bool copy_c_f)
{
  // The idea behind this approach is to leverage both the solution refinement
  // in solve() and equilibration during factorization (inverting C_F in place
  // can only leverage the latter).

  size_t n = c_f.length();
  lhs.size(n); // not sure if initialization matters here...

  RealSpdSolver spd_solver;  RealSymMatrix C_F_copy;  RealVector c_f_copy;
  // Matrix & RHS get altered by equilibration --> make copies if needed later
  if (copy_C_F) {
    C_F_copy = C_F; // Teuchos::Copy by default
    spd_solver.setMatrix(Teuchos::rcp(&C_F_copy, false));
  }
  else // Ok to modify C_F in place
    spd_solver.setMatrix(Teuchos::rcp(&C_F, false));
  if (copy_c_f) {
    c_f_copy = c_f; // Teuchos::Copy by default
    spd_solver.setVectors(Teuchos::rcp(&lhs, false),
			  Teuchos::rcp(&c_f_copy, false));
  }
  else // Ok to modify c_f in place
    spd_solver.setVectors(Teuchos::rcp(&lhs, false), Teuchos::rcp(&c_f, false));

  //Cout << "C_F before:\n" << C_F << "c_f before:\n" << c_f;
  if (spd_solver.shouldEquilibrate())
    spd_solver.factorWithEquilibration(true);
  spd_solver.solveToRefinedSolution(true);
  int code = spd_solver.solve();
  //Cout << "C_F after:\n" << C_F << "c_f after:\n" << c_f;
  if (code) {
    Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	 << ") in NonDACV::solve_for_C_F_c_f()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


inline Real NonDACVSampling::
solve_for_triple_product(const RealSymMatrix& C, const RealSymMatrix& F,
			 const RealMatrix&    c, size_t qoi)
{
  RealSymMatrix C_F;  RealVector c_f, lhs;
  compute_C_F_c_f(C, F, c, qoi, C_F, c_f);
  solve_for_C_F_c_f(C_F, c_f, lhs, false, true); // retain c_f for use below

  size_t i, n = C.numRows();
  Real trip_prod = 0.;
  for (i=0; i<n; ++i)
    trip_prod += c_f(i) * lhs(i);
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "ACV::solve_for_triple_product(): C-F =\n" << C_F
  // 	   << "RHS c-f =\n" << c_f << "LHS soln =\n" << lhs
  // 	   << "triple product = " << trip_prod << std::endl;
  return trip_prod;
}


inline Real NonDACVSampling::
compute_R_sq(const RealSymMatrix& C, const RealSymMatrix& F,
	     const RealMatrix& c, size_t qoi, Real var_H_q)
{ return solve_for_triple_product(C, F, c, qoi) / var_H_q; }


inline void NonDACVSampling::
acv_estvar_ratios(const RealSymMatrix& F, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    estvar_ratios[qoi]
      = 1. - compute_R_sq(covLL[qoi], F, covLH, qoi, varH[qoi]);
}


inline void NonDACVSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  // map incoming continuous design vars into r_i factors and compute F
  RealSymMatrix F;
  switch (optSubProblemForm) {
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT: {
    RealVector r;  copy_data_partial(cd_vars, 0, (int)numApprox, r); // N_i
    r.scale(1./cd_vars[numApprox]); // r_i = N_i / N
    compute_F_matrix(r, F);
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT: // N is a vector constant for opt sub-problem
  case R_AND_N_NONLINEAR_CONSTRAINT:
    compute_F_matrix(cd_vars, F); // admits r as leading numApprox terms
    break;
  }
  // compute ACV estimator variance given F
  acv_estvar_ratios(F, estvar_ratios);
}


inline void NonDACVSampling::
compute_acv_control_covariances(RealMatrix& sum_L, Real sum_H_q,
				RealSymMatrix& sum_LL_q, RealMatrix& sum_LH,
				size_t N_shared_q, size_t qoi,
				RealSymMatrix& cov_LL, RealMatrix& cov_LH)
{
  // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
  // > cov_LH is sized for all qoi but only 1 row is used
  cov_LL.shape(numApprox); cov_LH.shape(numFunctions, numApprox);
  size_t approx, approx2;  Real sum_L_aq;
  for (approx=0; approx<numApprox; ++approx) {
    sum_L_aq = sum_L(qoi,approx);
    compute_covariance(sum_L_aq, sum_H_q, sum_LH(qoi,approx), N_shared_q,
		       cov_LH(qoi,approx));
    compute_variance(sum_L_aq, sum_LL_q(approx,approx), N_shared_q,
		     cov_LL(approx,approx));
    for (approx2=0; approx2<approx; ++approx2)
      compute_covariance(sum_L_aq, sum_L(qoi,approx2), sum_LL_q(approx,approx2),
			 N_shared_q, cov_LL(approx,approx2));
  }
}


inline void NonDACVSampling::
solve_for_acv_control(const RealSymMatrix& cov_LL, const RealSymMatrix& F,
		      const RealMatrix& cov_LH, size_t qoi, RealVector& beta)
{
  RealSymMatrix C_F;  RealVector c_f;
  compute_C_F_c_f(cov_LL, F, cov_LH, qoi, C_F, c_f);
  solve_for_C_F_c_f(C_F, c_f, beta, false, false); // Ok to modify C_F,c_f

  //Cout << "solve_for_acv_control qoi " << qoi+1 << ": C_F\n" << C_F
  //     << "c_f\n" << c_f << "beta\n" << beta;
}


inline void NonDACVSampling::
precompute_acv_control(const RealVector& avg_eval_ratios,
		       const SizetArray& N_shared)
{
  //RealSymMatrix F;
  compute_F_matrix(avg_eval_ratios, FMat);
}


inline void NonDACVSampling::
compute_acv_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
		    RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
		    size_t N_shared_q, size_t mom, size_t qoi, RealVector& beta)
{
  if (mom == 1) // variances/covariances already computed for mean estimator
    solve_for_acv_control(covLL[qoi], FMat, covLH, qoi, beta);
  else { // compute variances/covariances for higher-order moment estimators
    // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
    // > cov_LH is sized for all qoi but only 1 row is used
    RealSymMatrix cov_LL; RealMatrix cov_LH;
    compute_acv_control_covariances(sum_L_base_m, sum_H_mq, sum_LL_mq, sum_LH_m,
				    N_shared_q, qoi, cov_LL, cov_LH);
    // forward to overload:
    solve_for_acv_control(cov_LL, FMat, cov_LH, qoi, beta);
  }
}


inline void NonDACVSampling::print_variance_reduction(std::ostream& s)
{ print_estimator_performance(s, acvSolnData); }

} // namespace Dakota

#endif
