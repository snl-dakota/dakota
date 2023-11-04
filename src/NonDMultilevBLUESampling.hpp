/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  void finite_solution_upper_bounds(Real remaining, RealVector& x_ub);

  //
  //- Heading: member functions
  //

  void initialize_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG);
  void initialize_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG);

  void initialize_blue_counts(SizetMatrixArray& num_G);
                            //, SizetSymMatrix2DArray& num_GG);

  void add_sub_matrix(Real coeff, const RealSymMatrix& sub_mat,
		      const UShortArray& subset, RealSymMatrix& mat);
  void add_sub_matvec(const RealSymMatrix& sub_mat, const RealMatrix& sub_mat2,
		      size_t sub_mat2_row, const UShortArray& subset,
		      RealVector& vec);

  void compute_C_inverse(const RealSymMatrix2DArray& cov_GG,
			 RealSymMatrix2DArray& cov_GG_inv);
  void compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
			   const RealVector& N_G, RealSymMatrixArray& Psi_inv);
  void compute_mu_hat(const RealSymMatrix2DArray& cov_GG_inv,
		      const RealSymMatrixArray& Psi_inv,
		      const RealMatrixArray& sum_G, RealVectorArray& mu_hat);

  void estimator_variance(const RealVector& cd_vars, RealVector& estvar);

  void process_group_solution(MFSolutionData& soln,
			      const SizetMatrixArray& N_G_actual,
			      const SizetArray& N_G_alloc,
			      SizetArray& delta_N_G);

  void blue_raw_moments(IntRealMatrixArrayMap& sum_G,
			IntRealSymMatrix2DArrayMap& sum_GG,
			const SizetMatrixArray& N_G_actual,
			RealMatrix& H_raw_mom);

  void finalize_counts(const SizetMatrixArray& N_G_actual,
		       const SizetArray& N_G_alloc);

  void print_computed_solution(std::ostream& s, const MFSolutionData& soln);

private:

  //
  //- Heading: Helper functions
  //

  void ml_blue_online_pilot();
  void ml_blue_offline_pilot();
  void ml_blue_pilot_projection();

  void group_increment(SizetArray& delta_N_G, size_t iter);
  void evaluate_pilot(RealMatrixArray& sum_G_pilot,
		      RealSymMatrix2DArray& sum_GG_pilot,
		      SizetMatrixArray& N_shared_pilot, bool incr_cost);

  void compute_allocations(MFSolutionData& soln,
			   const SizetMatrixArray& N_G_actual,
			   SizetArray& N_G_alloc, SizetArray& delta_N_G);

  /*
  void compute_blue_control_covariances(RealMatrix& sum_L, Real sum_H_q,
				       RealSymMatrix& sum_LL_q,
				       RealMatrix& sum_LH, size_t N_shared_q,
				       size_t qoi, RealSymMatrix& cov_LL,
				       RealMatrix& cov_LH);
  void precompute_blue_control(const RealVector& avg_eval_ratios,
			      const SizetArray& N_shared);
  void compute_blue_control(RealMatrix& sum_L_base_m, Real sum_H_mq,
			   RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
			   size_t N_shared_q, size_t mom, size_t qoi,
			   RealVector& beta);
  void solve_for_blue_control(const RealSymMatrix& cov_LL,
			     const RealSymMatrix& F, const RealMatrix& cov_LH,
			     size_t qoi, RealVector& beta);
  */

  void accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG,
			    SizetMatrixArray& num_G);
                          //, SizetSymMatrix2DArray& num_GG);
  void accumulate_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG,
			    SizetMatrixArray& num_G);
                          //, SizetSymMatrix2DArray& num_GG);

  void compute_GG_covariance(const RealMatrixArray& sum_G,
			     const RealSymMatrix2DArray& sum_GG,
			     const SizetMatrixArray& N_G,
			     RealSymMatrix2DArray& cov_GG);

  /*
  void compute_GG_statistics(RealMatrixArray& sum_G_pilot,
			     RealSymMatrix2DArray& sum_GG_pilot,
			     SizetMatrixArray& N_shared_pilot,
			   //RealMatrixArray& var_G,
			     RealSymMatrix2DArray& cov_GG);
  void compute_G_variance(const RealMatrixArray& sum_G,
			  const RealSymMatrix2DArray& sum_GG,
			  const SizetMatrixArray& num_G,
			  RealMatrixArray& var_G);

  void update_projected_group_samples(const MFSolutionData& soln,
				      const SizetArray& N_G_actual,
				      size_t& N_G_alloc,
				      SizetArray& delta_N_G,
				      Real& delta_equiv_hf);

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

  void scale_to_target(Real avg_N_H, const RealVector& cost,
		       RealVector& avg_eval_ratios, Real& avg_hf_target);
  */

  //
  //- Heading: Data
  //

  /// BLUE optimizes over a set of model groupings
  UShort2DArray modelGroups;

  /// covariance matrices for each model QoI and each model grouping (the C_k
  /// matrix in ML BLUE), organized as a numGroups x numFunctions array of
  /// symmetic numModels_k x numModels_k covariance matrices
  RealSymMatrix2DArray covGG;
  /// in-place matrix inverses of covGG
  RealSymMatrix2DArray covGGinv;

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

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "add_sub_matrix() aggregate =\n" << mat << std::endl;
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

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "add_sub_matvec() aggregate =\n" << vec << std::endl;
}


inline void NonDMultilevBLUESampling::
compute_C_inverse(const RealSymMatrix2DArray& cov_GG,
		  RealSymMatrix2DArray& cov_GG_inv)
{
  // cov matrices are sized according to group member size

  size_t g, num_groups = modelGroups.size(), qoi;
  if (cov_GG_inv.size() != num_groups) {
    cov_GG_inv.resize(num_groups);
    for (g=0; g<num_groups; ++g)
      cov_GG_inv[g].resize(numFunctions);
    // Note: cov_GG_inv_gq gets copied below
  }

  RealSpdSolver spd_solver; // can this be reused?
  for (g=0; g<num_groups; ++g) {
    const RealSymMatrixArray& cov_GG_g =     cov_GG[g];
    RealSymMatrixArray&   cov_GG_inv_g = cov_GG_inv[g];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      cov_GG_inv_g[qoi] = cov_GG_g[qoi]; // copy for inversion in place
      spd_solver.setMatrix(Teuchos::rcp(&cov_GG_inv_g[qoi], false));
      if (spd_solver.shouldEquilibrate())
	spd_solver.factorWithEquilibration(true);
      int code = spd_solver.invert(); // in place
      if (code) {
	Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	     << ") in NonDMultilevBLUE::compute_C_inverse()." << std::endl;
	abort_handler(METHOD_ERROR);
      }
    }
  }
}


inline void NonDMultilevBLUESampling::
compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
		    const RealVector& N_G, RealSymMatrixArray& Psi_inv)
{
  // Psi matrix accumulates across groups and is sized according to
  // full model ensemble

  size_t g, num_groups = modelGroups.size(), all_models = numApprox + 1, qoi;
  if (Psi_inv.size() != numFunctions) {
    Psi_inv.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      Psi_inv[qoi].shape(all_models); // init to 0
  }

  for (g=0; g<num_groups; ++g) {
    //form_R(g, R_g); // bypass R_k^T Cinv R_k using more direct subset mgmt
    Real n_g = N_G[g];
    const UShortArray&            models_g = modelGroups[g];
    const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
    for (qoi=0; qoi<numFunctions; ++qoi)
      add_sub_matrix(n_g, cov_GG_inv_g[qoi], models_g, Psi_inv[qoi]);
  }

  // Psi-inverse is used for computing both estimator variance (during numerical
  // solve) and y-hat / mu-hat (after solve), so invert now without a RHS so
  // Psi-inverse can be used in multiple places without additional tracking
  RealSpdSolver spd_solver; // can this be reused?
  for (qoi=0; qoi<numFunctions; ++qoi) {
    spd_solver.setMatrix(Teuchos::rcp(&Psi_inv[qoi], false));
    if (spd_solver.shouldEquilibrate())
      spd_solver.factorWithEquilibration(true);
    int code = spd_solver.invert(); // in place
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in NonDMultilevBLUE::compute_Psi_inverse()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
}


inline void NonDMultilevBLUESampling::
compute_mu_hat(const RealSymMatrix2DArray& cov_GG_inv,
	       const RealSymMatrixArray& Psi_inv, const RealMatrixArray& sum_G,
	       RealVectorArray& mu_hat)
{
  // y-hat and mu-hat vectors accumulate across groups and are sized
  // according to full model ensemble

  size_t qoi, r, c, g, num_groups = modelGroups.size(),
    all_models = numApprox + 1;

  // Only need to form y-hat when solving for mu-hat:
  RealVectorArray y_hat(numFunctions);
  for (qoi=0; qoi<numFunctions; ++qoi)
    y_hat[qoi].size(all_models); // init to 0
  for (g=0; g<num_groups; ++g) {
    //form_R(g, R_g);
    const UShortArray&            models_g = modelGroups[g];
    const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
    const RealMatrix&              sum_G_g =       sum_G[g];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      //copy_row_vector(sum_G_g, qoi, sum_G_gq);
      //add_sub_matvec(C_gq_inv, sum_G_gq, models_g, y_hat[qoi]);
      // pass row for sum_G_g to avoid copy:
      add_sub_matvec(cov_GG_inv_g[qoi], sum_G_g, qoi, models_g, y_hat[qoi]);
    }
  }

  if (mu_hat.size() != numFunctions) {
    mu_hat.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      mu_hat[qoi].sizeUninitialized(all_models);
  }
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& Psi_inv_q = Psi_inv[qoi];
    RealVector&  y_hat_q =  y_hat[qoi];
    RealVector& mu_hat_q = mu_hat[qoi];
    for (r=0; r<all_models; ++r) { //r=0; // Only using 1st row...
      Real& mu_hat_qr = mu_hat_q[r];  mu_hat_qr = 0.;
      for (c=0; c<all_models; ++c)
	mu_hat_qr += Psi_inv_q(r, c) * y_hat_q[c];// inactive Upper is protected
    }
  }

  /*
  // The idea behind this approach is to leverage both the solution refinement
  // in solve() and equilibration during factorization (inverting Psi in place
  // can only leverage the latter).
  if (spd_solver.shouldEquilibrate())
    spd_solver.factorWithEquilibration(true);
  spd_solver.solveToRefinedSolution(true);
  int code = spd_solver.solve();
  if (code) { ... }
  */
}


inline void NonDMultilevBLUESampling::
estimator_variance(const RealVector& cd_vars, RealVector& estvar)
{
  //RealSymMatrix2DArray cov_GG_inv;
  compute_C_inverse(covGG, covGGinv);// *** TO DO: precompute on each iteration
  RealSymMatrixArray Psi_inv;
  compute_Psi_inverse(covGGinv, cd_vars, Psi_inv);

  if (estvar.empty()) estvar.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    estvar[qoi] = Psi_inv[qoi](0,0); // e1^T Psi-inverse e1
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


inline void NonDMultilevBLUESampling::
blue_raw_moments(IntRealMatrixArrayMap& sum_G,
		 IntRealSymMatrix2DArrayMap& sum_GG,
		 const SizetMatrixArray& N_G_actual, // *** TO DO
		 RealMatrix& H_raw_mom)
{
  //RealSymMatrix2DArray cov_GG_inv;  // Reuse for moment 1
  //compute_C_inverse(covGG, covGGinv);// TO DO: precompute for each sample loop
  RealSymMatrixArray Psi_inv;  RealVectorArray mu_hat;
  for (int mom=1; mom<=4; ++mom) {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    RealMatrixArray& sum_G_m = sum_G[mom];
    if (mom == 1) { // reuse covariance data
      //compute_Psi_inverse(covGGinv, N_G_actual, Psi_inv); *** uses RV (cdv)
      compute_mu_hat(covGGinv, Psi_inv, sum_G_m, mu_hat);
      for (size_t qoi=0; qoi<numFunctions; ++qoi)
	H_raw_mom(qoi, mom-1) = mu_hat[qoi][0]; // *** TO DO: check model ordering --> differs from ML BLUE refs --> probably last entry i/o first
    }
    else { // generate new covariance data
      RealSymMatrix2DArray& sum_GG_m = sum_GG[mom];
      
    }
  }
}


inline void NonDMultilevBLUESampling::print_variance_reduction(std::ostream& s)
{ print_estimator_performance(s, blueSolnData); }

} // namespace Dakota

#endif
