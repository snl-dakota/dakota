/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevBLUESampling
//- Description: Class for approximate control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_ML_BLUE_SAMPLING_H
#define NOND_ML_BLUE_SAMPLING_H

#include "NonDNonHierarchSampling.hpp"
//#include "DataMethod.hpp"


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
  void print_variance_reduction(std::ostream& s);

  void estimator_variance_ratios(const RealVector& r_and_N,
				 RealVector& estvar_ratios);

  //
  //- Heading: member functions
  //

  void initialize_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG);
  void initialize_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG);

  void initialize_blue_counts(SizetMatrixArray& num_G);
                            //, SizetSymMatrix2DArray& num_GG);

  void blue_raw_moments(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined,
		       IntRealVectorMap& sum_H,
		       IntRealSymMatrixArrayMap& sum_LL,
		       IntRealMatrixMap& sum_LH,
		       const RealVector& avg_eval_ratios,
		       const SizetArray& N_shared,
		       const Sizet2DArray& N_L_refined,
		       RealMatrix& H_raw_mom);

  void compute_blue_control_covariances(RealMatrix& sum_L, Real sum_H_q,
				       RealSymMatrix& sum_LL_q,
				       RealMatrix& sum_LH, size_t N_shared_q,
				       size_t qoi, RealSymMatrix& cov_LL,
				       RealMatrix& cov_LH);

  void update_projected_group_samples(const MFSolutionData& soln,
				      //const SizetArray& N_H_actual,
				      //size_t& N_H_alloc,
				      //SizetArray& delta_N_L_actual,
				      Real& delta_equiv_hf);

  Real update_hf_target(const RealVector& avg_eval_ratios,
			const RealVector& var_H, const RealVector& estvar0);

  void print_computed_solution(std::ostream& s, const MFSolutionData& soln);

private:

  //
  //- Heading: Helper functions
  //

  void ml_blue_online_pilot();
  void ml_blue_offline_pilot();
  void ml_blue_pilot_projection();

  void precompute_blue_control(const RealVector& avg_eval_ratios,
			      const SizetArray& N_shared);

  void compute_blue_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
			   RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
			   size_t N_shared_q, size_t mom, size_t qoi,
			   RealVector& beta);

  //void initialize_blue_covariances(IntRealSymMatrixArrayMap covLL,
  //				  IntRealMatrixMap& cov_LH,
  //				  IntRealVectorMap& var_H);

  void accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG,
			    SizetMatrixArray& num_G);
                          //, SizetSymMatrix2DArray& num_GG);
  void accumulate_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG,
			    SizetMatrixArray& num_G);
                          //, SizetSymMatrix2DArray& num_GG);

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

  void compute_ratios(const RealMatrix& var_L, MFSolutionData& soln);

  void blue_estvar_ratios(const RealSymMatrix& F, RealVector& estvar_ratios);
  //Real blue_estimator_variance(const RealVector& avg_eval_ratios,
  //			        Real avg_hf_target);

  void solve_for_blue_control(const RealSymMatrix& cov_LL,
			     const RealSymMatrix& F, const RealMatrix& cov_LH,
			     size_t qoi, RealVector& beta);

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& avg_hf_target);

  //
  //- Heading: Data
  //

  /// BLUE optimizes over a set of model groupings
  UShort2DArray modelGroups;
  /// aggregate cost of a sample for each of the model groupings
  RealVector modelGroupCost;

  /// covariance matrices for each model QoI and each model grouping (the C_k
  /// matrix in ML BLUE), organized as a numGroups x numFunctions array of
  /// symmetic numModels_k x numModels_k covariance matrices
  RealSymMatrix2DArray covG;

  /// final solution data for BLUE
  /// *** TO DO: generalize beyond DAGs?
  MFSolutionData blueSolnData;
};


inline Real NonDMultilevBLUESampling::estimator_accuracy_metric()
{ return blueSolnData.average_estimator_variance(); }


//inline Real NonDMultilevBLUESampling::estimator_cost_metric()
//{ return blueSolnData.equivalent_hf_allocation(); }


inline void NonDMultilevBLUESampling::
initialize_blue_sums(RealMatrixArray& sum_G, RealSymMatrix2DArray& sum_GG)
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


inline void NonDMultilevBLUESampling::
initialize_blue_sums(IntRealMatrixArrayMap& sum_G,
		     IntRealSymMatrix2DArrayMap& sum_GG)
{
  RealMatrixArray mat1;  RealSymMatrix2DArray mat2;
  initialize_blue_sums(mat1, mat2);
  for (int i=1; i<=4; ++i)
    { sum_G[i] = mat1; sum_GG[i] = mat2; } // copies
}


inline void NonDMultilevBLUESampling::
initialize_blue_counts(SizetMatrixArray& num_G)//,SizetSymMatrix2DArray& num_GG)
{
  size_t g, num_groups = modelGroups.size(), num_models;
  num_G.resize(num_groups);  //num_GG.resize(num_groups);
  for (g=0; g<num_groups; ++g) {
    num_models = modelGroups[g].size();
    num_G[g].shape(numFunctions, num_models);
    //SizetSymMatrixArray& num_GG_g = num_GG[g];
    //num_GG_g.resize(numFunctions);
    //for (size_t qoi=0; qoi<numFunctions; ++qoi)
    //  num_GG_g[qoi].shape(num_models);
  }
}


inline void NonDMultilevBLUESampling::
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


inline void NonDMultilevBLUESampling::
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


inline void NonDMultilevBLUESampling::
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
	 << ") in NonDMultilevBLUESampling::compute_F_matrix()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "F matrix for sub-method " << mlmfSubMethod << ":\n" << F
	 << std::endl;
}


inline void NonDMultilevBLUESampling::
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


inline void NonDMultilevBLUESampling::
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
	 << ") in NonDMultilevBLUE::solve_for_C_F_c_f()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


inline Real NonDMultilevBLUESampling::
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


inline Real NonDMultilevBLUESampling::
compute_R_sq(const RealSymMatrix& C, const RealSymMatrix& F,
	     const RealMatrix& c, size_t qoi, Real var_H_q)
{ return solve_for_triple_product(C, F, c, qoi) / var_H_q; }


inline void NonDMultilevBLUESampling::
blue_estvar_ratios(const RealSymMatrix& F, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    estvar_ratios[qoi]
      = 1.;// - compute_R_sq(covLL[qoi], F, covLH, qoi, varH[qoi]);
}


inline void NonDMultilevBLUESampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  // map incoming continuous design vars into r_i factors and compute F
  RealSymMatrix F;
  RealVector r;  copy_data_partial(cd_vars, 0, (int)numApprox, r); // N_i
  r.scale(1./cd_vars[numApprox]); // r_i = N_i / N
  //compute_F_matrix(r, F);

  // compute ACV estimator variance given F
  blue_estvar_ratios(F, estvar_ratios);
}


inline void NonDMultilevBLUESampling::
compute_blue_control_covariances(RealMatrix& sum_L, Real sum_H_q,
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


inline void NonDMultilevBLUESampling::
solve_for_blue_control(const RealSymMatrix& cov_LL, const RealSymMatrix& F,
		      const RealMatrix& cov_LH, size_t qoi, RealVector& beta)
{
  RealSymMatrix C_F;  RealVector c_f;
  compute_C_F_c_f(cov_LL, F, cov_LH, qoi, C_F, c_f);
  solve_for_C_F_c_f(C_F, c_f, beta, false, false); // Ok to modify C_F,c_f

  //Cout << "solve_for_blue_control qoi " << qoi+1 << ": C_F\n" << C_F
  //     << "c_f\n" << c_f << "beta\n" << beta;
}


inline void NonDMultilevBLUESampling::
precompute_blue_control(const RealVector& avg_eval_ratios,
		       const SizetArray& N_shared)
{
  //compute_F_matrix(avg_eval_ratios, FMat);
}


inline void NonDMultilevBLUESampling::
compute_blue_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
		    RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
		    size_t N_shared_q, size_t mom, size_t qoi, RealVector& beta)
{
  /*
  if (mom == 1) // variances/covariances already computed for mean estimator
    solve_for_blue_control(covLL[qoi], FMat, covLH, qoi, beta);
  else { // compute variances/covariances for higher-order moment estimators
    // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
    // > cov_LH is sized for all qoi but only 1 row is used
    RealSymMatrix cov_LL; RealMatrix cov_LH;
    compute_blue_control_covariances(sum_L_base_m, sum_H_mq, sum_LL_mq, sum_LH_m,
				    N_shared_q, qoi, cov_LL, cov_LH);
    // forward to overload:
    solve_for_blue_control(cov_LL, FMat, cov_LH, qoi, beta);
  }
  */
}


inline void NonDMultilevBLUESampling::print_variance_reduction(std::ostream& s)
{ print_estimator_performance(s, blueSolnData); }

} // namespace Dakota

#endif
