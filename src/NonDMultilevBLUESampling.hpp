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
  void print_multigroup_summary(std::ostream& s, const String& summary_type,
				bool projections);
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

  void initialize_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG);
  void initialize_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG);

  void initialize_blue_counts(Sizet2DArray& num_G);

  void compute_C_inverse(const RealSymMatrix2DArray& cov_GG,
			 RealSymMatrix2DArray& cov_GG_inv);
  void compute_Psi(const RealSymMatrix2DArray& cov_GG_inv,
		   const RealVector& N_G, RealSymMatrixArray& Psi);
  void compute_Psi(const RealSymMatrix2DArray& cov_GG_inv,
		   const Sizet2DArray& N_G, RealSymMatrixArray& Psi);
  /*
  void invert_Psi(RealSymMatrixArray& Psi);
  void compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
			   const RealVector& N_G_1D,
			   RealSymMatrixArray& Psi_inv);
  void compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
			   const Sizet2DArray& N_G_2D,
			   RealSymMatrixArray& Psi_inv);
  */
  void compute_y(const RealSymMatrix2DArray& cov_GG_inv,
		 const RealMatrixArray& sum_G, RealVectorArray& y);
  void compute_mu_hat(const RealSymMatrix2DArray& cov_GG_inv,
		      const RealMatrixArray& sum_G, const Sizet2DArray& N_G,
		      RealVectorArray& mu_hat);

  void estimator_variance(const RealVector& cd_vars, RealVector& estvar);

  void process_group_solution(MFSolutionData& soln,
			      const Sizet2DArray& N_G_actual,
			      const SizetArray& N_G_alloc,
			      SizetArray& delta_N_G);

  void blue_raw_moments(IntRealMatrixArrayMap& sum_G,
			IntRealSymMatrix2DArrayMap& sum_GG,
			const Sizet2DArray& N_G_actual, RealMatrix& H_raw_mom);

  void finalize_counts(const Sizet2DArray& N_G_actual,
		       const SizetArray& N_G_alloc);

  void print_group_solution(std::ostream& s, const MFSolutionData& soln);

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
		      Sizet2DArray& N_shared_pilot, bool incr_cost);

  void compute_allocations(MFSolutionData& soln, const Sizet2DArray& N_G_actual,
			   SizetArray& N_G_alloc, SizetArray& delta_N_G);

  void project_mc_estimator_variance(const RealSymMatrixArray& var_H,
				     const SizetArray& N_H_actual,
				     size_t delta_N_H, RealVector& proj_est_var,
				     SizetVector& proj_N_H);
  void project_mc_estimator_variance(const RealSymMatrixArray& var_H,
				     Real N_H_actual, Real delta_N_H,
				     RealVector& proj_est_var);

  void accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG,
			    Sizet2DArray& num_G);
  void accumulate_blue_sums(RealMatrixArray& sum_G,
			    RealSymMatrix2DArray& sum_GG, Sizet2DArray& num_G);

  void compute_GG_covariance(const RealMatrixArray& sum_G,
			     const RealSymMatrix2DArray& sum_GG,
			     const Sizet2DArray& N_G,
			     RealSymMatrix2DArray& cov_GG,
			     RealSymMatrix2DArray& cov_GG_inv);
  void covariance_to_correlation_sq(const RealSymMatrixArray& cov_GG_g,
				    RealMatrix& rho2_LH);

  void analytic_initialization_from_mfmc(const RealMatrix& rho2_LH,
					 MFSolutionData& soln);
  void analytic_initialization_from_ensemble_cvmc(const RealMatrix& rho2_LH,
						  MFSolutionData& soln);

  void add_sub_matrix(Real coeff, const RealSymMatrix& sub_mat,
		      const UShortArray& subset, RealSymMatrix& mat);
  void add_sub_matvec(const RealSymMatrix& sub_mat, const RealMatrix& sub_mat2,
		      size_t sub_mat2_row, const UShortArray& subset,
		      RealVector& vec);

  void initialize_rsm2a(RealSymMatrix2DArray& rsm2a);
  void initialize_rsma(RealSymMatrixArray& rsma, bool init = true);
  void initialize_rva(RealVectorArray& rva, bool init = true);
  
  bool mfmc_model_grouping(const UShortArray& model_group) const;
  bool cvmc_model_grouping(const UShortArray& model_group) const;

  void print_group(std::ostream& s, size_t g) const;

  /*
  void compute_GG_statistics(RealMatrixArray& sum_G_pilot,
			     RealSymMatrix2DArray& sum_GG_pilot,
			     Sizet2DArray& N_shared_pilot,
			   //RealMatrixArray& var_G,
			     RealSymMatrix2DArray& cov_GG);
  void compute_G_variance(const RealMatrixArray& sum_G,
			  const RealSymMatrix2DArray& sum_GG,
			  const Sizet2DArray& num_G,
			  RealMatrixArray& var_G);

  void update_projected_group_samples(const MFSolutionData& soln,
				      const SizetArray& N_G_actual,
				      size_t& N_G_alloc,
				      SizetArray& delta_N_G,
				      Real& delta_equiv_hf);

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

  /// counter for successful sample accumulations, per group and per QoI
  Sizet2DArray NGroupActual;
  /// counter for sample allocations, per group
  SizetArray   NGroupAlloc;

  /// final solution data for BLUE
  MFSolutionData blueSolnData;

  /// reference value for estimator variance: final HF variance over
  /// projected HF samples
  RealVector  projEstVarHF;
  /// reference value: projected HF samples
  SizetVector projNActualHF;
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
initialize_blue_counts(Sizet2DArray& num_G)
{
  size_t g, num_groups = modelGroups.size(), num_models;
  num_G.resize(num_groups);
  for (g=0; g<num_groups; ++g)
    num_G[g].assign(numFunctions, 0);
}


inline void NonDMultilevBLUESampling::
covariance_to_correlation_sq(const RealSymMatrixArray& cov_GG_g,
			     RealMatrix& rho2_LH)
{
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx;  Real var_H_q, var_L_q, cov_LH_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& cov_GG_gq = cov_GG_g[qoi];
    var_H_q = cov_GG_gq(numApprox, numApprox);
    for (approx=0; approx<numApprox; ++approx) {
      var_L_q  = cov_GG_gq(approx,    approx);
      cov_LH_q = cov_GG_gq(numApprox, approx);
      rho2_LH(qoi, approx) = cov_LH_q / var_L_q * cov_LH_q / var_H_q;
    }
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

  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "add_sub_matrix() aggregate =\n" << mat << std::endl;
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

  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "add_sub_matvec() aggregate =\n" << vec << std::endl;
}


inline void NonDMultilevBLUESampling::
initialize_rsm2a(RealSymMatrix2DArray& rsm2a)
{
  // size the first two dimensions, but defer RealSymMatrix::shape()

  size_t num_groups = modelGroups.size();
  if (rsm2a.size() != num_groups) {
    rsm2a.resize(num_groups);
    for (size_t g=0; g<num_groups; ++g)
      rsm2a[g].resize(numFunctions);
  }
}


inline void NonDMultilevBLUESampling::
initialize_rsma(RealSymMatrixArray& rsma, bool init)
{
  if (rsma.size() != numFunctions) {
    rsma.resize(numFunctions);
    size_t qoi, all_models = numApprox + 1;
    if (init)
      for (qoi=0; qoi<numFunctions; ++qoi)
	rsma[qoi].shape(all_models); // init to 0
    else
      for (qoi=0; qoi<numFunctions; ++qoi)
	rsma[qoi].shapeUninitialized(all_models); // init to 0
  }
  else if (init)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      rsma[qoi].putScalar(0.);
}


inline void NonDMultilevBLUESampling::
initialize_rva(RealVectorArray& rva, bool init)
{
  if (rva.size() != numFunctions) {
    rva.resize(numFunctions);
    size_t qoi, all_models = numApprox + 1;
    if (init)
      for (qoi=0; qoi<numFunctions; ++qoi)
	rva[qoi].size(all_models); // init to 0
    else
      for (qoi=0; qoi<numFunctions; ++qoi)
	rva[qoi].sizeUninitialized(all_models); // init to 0
  }
  else if (init)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      rva[qoi].putScalar(0.);
}


inline void NonDMultilevBLUESampling::
compute_C_inverse(const RealSymMatrix2DArray& cov_GG,
		  RealSymMatrix2DArray& cov_GG_inv)
{
  // cov matrices are sized according to group member size
  initialize_rsm2a(cov_GG_inv);

  RealSpdSolver spd_solver; // reuse since setMatrix resets internal state
  size_t q, g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    const RealSymMatrixArray& cov_GG_g =     cov_GG[g];
    RealSymMatrixArray&   cov_GG_inv_g = cov_GG_inv[g];
    for (q=0; q<numFunctions; ++q) {
      const RealSymMatrix& cov_GG_gq =     cov_GG_g[q];
      RealSymMatrix&   cov_GG_inv_gq = cov_GG_inv_g[q];
      if (cov_GG_gq.empty()) // insufficient samples to define cov_GG
	cov_GG_inv_gq.shape(0);
      else {
	cov_GG_inv_gq = cov_GG_gq; // copy for inversion in place
	spd_solver.setMatrix(Teuchos::rcp(&cov_GG_inv_gq, false));
	if (spd_solver.shouldEquilibrate())
	  spd_solver.factorWithEquilibration(true);
	int code = spd_solver.invert(); // in place
	if (code) {
	  Cerr << "Error: serial dense solver failure (LAPACK error code "
	       << code << ") in ML BLUE::compute_C_inverse()." << std::endl;
	  abort_handler(METHOD_ERROR);
	}
      }
    }
  }
}


/** This version used during numerical solution (1D vector of real
    design variables for group samples. */
inline void NonDMultilevBLUESampling::
compute_Psi(const RealSymMatrix2DArray& cov_GG_inv, const RealVector& N_G,
	    RealSymMatrixArray& Psi)
{
  // Psi accumulated across groups and sized according to full model ensemble
  initialize_rsma(Psi);

  size_t qoi, g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    // In the context of numerical optimization, we should not need to protect
    // cov_GG_inv against n_g = 0.
    Real N_g = N_G[g];
    if (N_g > 0.) {
      const UShortArray&            models_g = modelGroups[g];
      const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
      for (qoi=0; qoi<numFunctions; ++qoi)
	add_sub_matrix(N_g, cov_GG_inv_g[qoi], models_g, Psi[qoi]); // *** can become indefinite here when N_g --> 0, which depends on online/offline pilot integration strategy
    }
  }
}


/** This version used during numerical solution (1D vector of real
    design variables for group samples. */
inline void NonDMultilevBLUESampling::
compute_Psi(const RealSymMatrix2DArray& cov_GG_inv, const Sizet2DArray& N_G,
	    RealSymMatrixArray& Psi)
{
  // Psi accumulated across groups and sized according to full model ensemble
  initialize_rsma(Psi);

  size_t qoi, g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    const SizetArray&                  N_g =         N_G[g];
    const UShortArray&            models_g = modelGroups[g];
    const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
    for (qoi=0; qoi<numFunctions; ++qoi)
      if (N_g[qoi]) {
	Real N_gq = N_g[qoi];
	add_sub_matrix(N_gq, cov_GG_inv_g[qoi], models_g, Psi[qoi]); // *** can become indefinite here when n_g --> 0, which depends on online/offline pilot integration strategy
      }
  }
}


/** This version used during numerical solution (1D vector of real
    design variables for group samples.
inline void NonDMultilevBLUESampling::invert_Psi(RealSymMatrixArray& Psi)
{
  // Psi-inverse is used for computing both estimator variance (during numerical
  // solve) and y-hat / mu-hat (after solve), so invert now without a RHS so
  // Psi-inverse can be used in multiple places without additional tracking
  RealSpdSolver spd_solver; // reuse since setMatrix resets internal state
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    spd_solver.setMatrix(Teuchos::rcp(&Psi[qoi], false));
    if (spd_solver.shouldEquilibrate())
      spd_solver.factorWithEquilibration(true);
    int code = spd_solver.invert(); // in place
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in NonDMultilevBLUE::compute_Psi_inverse()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "In compute_Psi_inverse(), Psi_inv:\n" << Psi << std::endl;
}


// This version used during numerical solution (1D vector of real design vars).
inline void NonDMultilevBLUESampling::
compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
		    const RealVector& N_G_1D, RealSymMatrixArray& Psi_inv)
{
  compute_Psi(cov_GG_inv, N_G_1D, Psi_inv);
  invert_Psi(Psi_inv);
}


// This version used during final results (2D array of actual samples).
inline void NonDMultilevBLUESampling::
compute_Psi_inverse(const RealSymMatrix2DArray& cov_GG_inv,
		    const Sizet2DArray& N_G_2D, RealSymMatrixArray& Psi_inv)
{
  compute_Psi(cov_GG_inv, N_G_2D, Psi_inv);
  invert_Psi(Psi_inv);
}
*/


inline void NonDMultilevBLUESampling::
compute_y(const RealSymMatrix2DArray& cov_GG_inv, const RealMatrixArray& sum_G,
	  RealVectorArray& y)
{
  // y accumulated across groups and sized according to full model ensemble
  initialize_rva(y);

  size_t q, g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    //form_R(g, R_g);
    const UShortArray&            models_g = modelGroups[g];
    const RealSymMatrixArray& cov_GG_inv_g =  cov_GG_inv[g];
    const RealMatrix&              sum_G_g =       sum_G[g];
    for (q=0; q<numFunctions; ++q) {
      //copy_row_vector(sum_G_g, qoi, sum_G_gq);
      //add_sub_matvec(C_gq_inv, sum_G_gq, models_g, y[q]);
      // pass row for sum_G_g to avoid copy:
      const RealSymMatrix& cov_GG_inv_gq = cov_GG_inv_g[q];
      if (!cov_GG_inv_gq.empty())
	add_sub_matvec(cov_GG_inv_gq, sum_G_g, q, models_g, y[q]);
    }
  }
}


inline void NonDMultilevBLUESampling::
compute_mu_hat(const RealSymMatrix2DArray& cov_GG_inv,
	       const RealMatrixArray& sum_G, const Sizet2DArray& N_G,
	       RealVectorArray& mu_hat)
{
  // accumulate Psi but don't invert in place
  RealSymMatrixArray Psi;
  compute_Psi(cov_GG_inv, N_G, Psi);

  // Only need to form y when solving for mu-hat:
  RealVectorArray y;
  compute_y(cov_GG_inv, sum_G, y);

  initialize_rva(mu_hat, false);
  size_t q, r, c, g, num_groups = modelGroups.size();
  RealSpdSolver spd_solver;
  for (q=0; q<numFunctions; ++q) {
    // Leverage both the soln refinement in solve() and equilibration during
    // factorization (inverting Psi in place can only leverage the latter).
    spd_solver.setMatrix(Teuchos::rcp(&Psi[q], false)); // resets solver state
    spd_solver.setVectors(Teuchos::rcp(&mu_hat[q], false),
			  Teuchos::rcp(&y[q], false));
    if (spd_solver.shouldEquilibrate())
      spd_solver.factorWithEquilibration(true);
    spd_solver.solveToRefinedSolution(true);
    int code = spd_solver.solve();
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in ML BLUE compute_mu_hat()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
}


inline void NonDMultilevBLUESampling::
estimator_variance(const RealVector& cd_vars, RealVector& estvar)
{
  if (estvar.empty()) estvar.sizeUninitialized(numFunctions);

  // This approach leverages both the solution refinement in solve() and
  // equilibration during factorization (inverting Psi in place can only
  // leverage the latter).  It seems to work much more reliably.
  RealSymMatrixArray Psi;
  compute_Psi(covGGinv, cd_vars, Psi);

  RealSpdSolver spd_solver;
  size_t q, all_models = numApprox + 1;
  RealVector e_last(all_models, false), estvar_q(all_models, false);

  for (q=0; q<numFunctions; ++q) {

    // e_last is equilbrated in place, so must be reset
    e_last.putScalar(0.); e_last[numApprox] = 1.;

    spd_solver.setMatrix( Teuchos::rcp(&Psi[q],   false));// resets solver state
    spd_solver.setVectors(Teuchos::rcp(&estvar_q, false),
			  Teuchos::rcp(&e_last,   false));
    if (spd_solver.shouldEquilibrate())
      spd_solver.factorWithEquilibration(true);
    spd_solver.solveToRefinedSolution(true);
    int code = spd_solver.solve();
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in ML BLUE estimator_variance()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    estvar[q] = estvar_q[numApprox];
  }

  /* This approach suffers from poor performance, either from conditioning
     issues or misunderstood Teuchos solver behavior.
  RealSymMatrixArray Psi_inv;
  compute_Psi_inverse(covGGinv, cd_vars, Psi_inv);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    estvar[qoi] = Psi_inv[qoi](numApprox,numApprox); // e_l^T Psi-inverse e_l
  */
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
		 const Sizet2DArray& N_G_actual, RealMatrix& H_raw_mom)
{
  RealVectorArray mu_hat;
  for (int mom=1; mom<=4; ++mom) {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    RealMatrixArray& sum_G_m = sum_G[mom];
    if (mom == 1 && pilotMgmtMode != OFFLINE_PILOT) // reuse online covar data
      compute_mu_hat(covGGinv, sum_G_m, N_G_actual, mu_hat);
    else { // generate new covariance data
      RealSymMatrix2DArray& sum_GG_m = sum_GG[mom];
      RealSymMatrix2DArray cov_GG, cov_GG_inv;
      compute_GG_covariance(sum_G_m, sum_GG_m, N_G_actual, cov_GG, cov_GG_inv);
      compute_mu_hat(cov_GG_inv, sum_G_m, N_G_actual, mu_hat);
    }
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      H_raw_mom(qoi, mom-1) = mu_hat[qoi][numApprox]; // last model
  }
}


inline bool NonDMultilevBLUESampling::
mfmc_model_grouping(const UShortArray& model_group) const
{
  // For case where all models are active in MFMC (no model selection a priori)
  size_t i, num_models = model_group.size();
  for (i=0; i<num_models; ++i)
    if (model_group[i] != i) // "pyramid" sequence from 0 to last in set
      return false;
  return true;
}


inline bool NonDMultilevBLUESampling::
cvmc_model_grouping(const UShortArray& model_group) const
{ return (model_group.size() == 2 && model_group[1] == numApprox); }


inline void NonDMultilevBLUESampling::
print_group(std::ostream& s, size_t g) const
{
  const UShortArray& group_g = modelGroups[g];
  size_t m, num_models = group_g.size();
  s << " (models";
  for (m=0; m<num_models; ++m)
    s << ' ' << group_g[m];
  s << ")\n";
}

} // namespace Dakota

#endif
