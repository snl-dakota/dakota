/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevelSampling
//- Description: class for multilevel Monte Carlo sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_MULTILEVEL_SAMPLING_H
#define NOND_MULTILEVEL_SAMPLING_H

#include "NonDHierarchSampling.hpp"
#include "DataMethod.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#elif HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

namespace Dakota {

/// Performs Multilevel Monte Carlo sampling for uncertainty quantification.

/** Multilevel Monte Carlo (MLMC) is a variance-reduction technique
    that utilitizes lower fidelity simulations that have response QoI
    that are correlated with the high-fidelity response QoI. */

class NonDMultilevelSampling: public virtual NonDHierarchSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDMultilevelSampling();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  void nested_response_mappings(const RealMatrix& primary_coeffs,
				const RealMatrix& secondary_coeffs);

  //
  //- Heading: Member functions
  //

  /// helper that consolidates sequence advancement, sample generation,
  /// sample export, and sample evaluation
  void evaluate_ml_sample_increment(unsigned short step);

  /// increment the equivalent number of HF evaluations based on new
  /// model evaluations
  void increment_ml_equivalent_cost(size_t new_N_l, Real lev_cost,
				    Real ref_cost);

  /// update accumulators for multilevel telescoping running sums
  /// using set of model evaluations within allResponses
  void accumulate_ml_Ysums(IntRealMatrixMap& sum_Y, RealMatrix& sum_YY,
			   size_t lev, const RealVector& offset,
			   SizetArray& num_Y);
  /// update running QoI sums for one model (sum_Q) using set of model
  /// evaluations within allResponses; used for level 0 from other accumulators
  void accumulate_ml_Qsums(IntRealMatrixMap& sum_Q, size_t lev,
			   const RealVector& offset, SizetArray& num_Q);

  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			       const SizetArray& N_l);

  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy variances
  Real aggregate_mse_Yvar(const Real* var_Y, const SizetArray& N_l);
  /// sum up Monte Carlo estimates for mean squared error (MSE) across
  /// QoI using discrepancy sums
  Real aggregate_mse_Ysum(const Real* sum_Y, const Real* sum_YY,
			  const SizetArray& N_l);

  /// manage response mode and active model key from {group,form,lev} triplet.
  /// seq_type defines the active dimension for a 1D model sequence.
  void configure_indices(unsigned short group, unsigned short form,
			 size_t lev, short seq_type);
  /// convert group and form and call overload
  void configure_indices(size_t group, size_t form, size_t lev, short seq_type);

  /// return (aggregate) level cost
  Real level_cost(const RealVector& cost, size_t step);

private:

  //
  //- Heading: Helper functions
  //

  /// Perform multilevel Monte Carlo across the discretization levels for a
  /// particular model form using discrepancy accumulators (sum_Y)
  void multilevel_mc_Ysum();
  /// Perform multilevel Monte Carlo across the discretization levels for a
  /// particular model form using QoI accumulators (sum_Q)
  void multilevel_mc_Qsum();

  /// initialize the ML accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_ml_Ysums(IntRealMatrixMap& sum_Y, size_t num_lev);
  /// initialize the ML accumulators for computing means, variances, and
  /// covariances across fidelity levels
  void initialize_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, size_t num_lev);

  /// accumulate initial approximation to mean vector, for use as offsets in
  /// subsequent accumulations
  void accumulate_offsets(RealVector& mu);

  /// update running QoI sums for one model (sum_Q) using set of model
  /// evaluations within allResponses; used for level 0 from other accumulators
  void accumulate_sums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		       IntIntPairRealMatrixMap& sum_QlQlm1, const size_t step,
		       const RealVectorArray& offset, Sizet2DArray& N_l);

  /// update running QoI sums for two models (sum_Ql, sum_Qlm1) using set of
  /// model evaluations within allResponses
  void accumulate_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			   IntIntPairRealMatrixMap& sum_QlQlm1, size_t lev,
			   const RealVector& offset, SizetArray& num_Q);

  // compute the equivalent number of HF evaluations (includes any sim faults)
  void compute_ml_equivalent_cost(const SizetArray& raw_N_l,
				  const RealVector& cost);

  /// populate finalStatErrors for MLMC based on Q sums
  void compute_error_estimates(const IntRealMatrixMap& sum_Ql,
			       const IntRealMatrixMap& sum_Qlm1,
			       const IntIntPairRealMatrixMap& sum_QlQlm1,
			       const Sizet2DArray& num_Q);

  /// compute variance from sum accumulators
  Real variance_Ysum(Real sum_Y, Real sum_YY, size_t Nlq);

  /// compute variance from sum accumulators, necessary for sample allocation optimization
  static Real variance_Ysum_static(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  /// compute variance from sum accumulators
  Real variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
		     Real sum_Qlm1Qlm1, size_t Nlq);
  /// compute variance from sum accumulators, necessary for sample allocation optimization
  static Real variance_Qsum_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  Real var_lev_l(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq);
	static Real var_lev_l_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad);

  /// sum up variances for QoI (using sum_YY with means from sum_Y) based on allocation target
  void aggregate_variance_target_Qsum(const IntRealMatrixMap&  sum_Ql, const IntRealMatrixMap&  sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, RealMatrix& agg_var_qoi);


  /// wrapper for aggregate_variance_Qsum 
  Real aggregate_variance_mean_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);
  /// sum up variances across QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
			       const Real* sum_QlQl,     const Real* sum_QlQlm1,
			       const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
			       const size_t lev);
  /// sum up variances for QoI (using sum_YY with means from sum_Y)
  Real aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                               const Real* sum_QlQl,     const Real* sum_QlQlm1,
                               const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                               const size_t lev, const size_t qoi);
  /// wrapper for var_of_var_ml 
  Real aggregate_variance_variance_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);

  /// wrapper for var_of_sigma_ml 
  Real aggregate_variance_sigma_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);

  /// wrapper for var_of_scalarization_ml
  Real aggregate_variance_scalarization_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, 
									const Sizet2DArray& N_l, const size_t step, const size_t qoi);

  /// sum up Monte Carlo estimates for mean squared error (MSE) for
  /// QoI using discrepancy sums based on allocation target
  void aggregate_mse_target_Qsum(RealMatrix& agg_var_qoi, 
						  const Sizet2DArray& N_l, const size_t step, RealVector& estimator_var0_qoi);

  /// compute epsilon^2/2 term for each qoi based on reference estimator_var0 and relative convergence tolereance
  void set_convergence_tol(const RealVector& estimator_var0_qoi, const RealVector& cost, RealVector& eps_sq_div_2_qoi);

  /// compute sample allocation delta based on a budget constraint
  void compute_sample_allocation_target(const RealMatrix& agg_var_qoi, 
    const RealVector& cost, const Sizet2DArray& N_l, SizetArray& delta_N_l);
  /// compute sample allocation delta based on current samples and based on allocation target. Single allocation target for each qoi, aggregated using max operation.
  void compute_sample_allocation_target(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
 									const IntIntPairRealMatrixMap& sum_QlQlm1, const RealVector& eps_sq_div_2, const RealMatrix& agg_var_qoi, 
  										const RealVector& cost, const Sizet2DArray& N_l, SizetArray& delta_N_l);
  
  // Roll up expected value estimators for central moments.  Final expected
  // value is sum of expected values from telescopic sum.  Note: raw moments
  // have no bias correction (no additional variance from an estimated center).
  void compute_moments(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1, const Sizet2DArray& N_l);

  /// compute the unbiased product of two sampling means
  static Real unbiased_mean_product_pair(const Real sumQ1, const Real sumQ2, const Real sumQ1Q2, const size_t Nlq);
  /// compute the unbiased product of three sampling means
  static Real unbiased_mean_product_triplet(const Real sumQ1, const Real sumQ2, const Real sumQ3,
                                                                    const Real sumQ1Q2, const Real sumQ1Q3, const Real sumQ2Q3, const Real sumQ1Q2Q3, const size_t Nlq);
  /// compute the unbiased product of two pairs of products of sampling means
  static Real unbiased_mean_product_pairpair(const Real sumQ1, const Real sumQ2, const Real sumQ1Q2,
                                                                     const Real sumQ1sq, const Real sumQ2sq,
                                                                     const Real sumQ1sqQ2, const Real sumQ1Q2sq, const Real sumQ1sqQ2sq, const size_t Nlq);

  static Real var_of_var_ml_l0(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1, const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const bool compute_gradient, Real& grad_g);

  static Real var_of_var_ml_lmax(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1, const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const bool compute_gradient, Real& grad_g);

  static Real var_of_var_ml_l(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1, const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const size_t lev, const bool compute_gradient, Real& grad_g);

  ///OPTPP definition
  static void target_cost_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_cost_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);

  static void target_var_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);
  static void target_var_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                               RealMatrix& grad_g, int& result_mode);


  static void target_sigma_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);
  static void target_sigma_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                               RealMatrix& grad_g, int& result_mode);

  static void target_scalarization_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode);
  static void target_scalarization_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                               RealMatrix& grad_g, int& result_mode);

  static void target_var_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_var_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);

  static void target_sigma_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_sigma_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);

  static void target_scalarization_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);
  static void target_scalarization_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode);

  /// NPSOL definition (Wrapper using OPTPP implementation above under the hood)
  static void target_cost_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_cost_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_var_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);
  static void target_var_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_sigma_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);
  static void target_sigma_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_scalarization_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);
  static void target_scalarization_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate);

  static void target_var_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_var_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);

  static void target_sigma_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_sigma_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);

  static void target_scalarization_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);
  static void target_scalarization_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate);

  void assign_static_member(const Real &conv_tol, size_t &qoi, const size_t &qoi_aggregation, const size_t &num_functions, const RealVector &level_cost_vec, const IntRealMatrixMap &sum_Ql,
                            const IntRealMatrixMap &sum_Qlm1, const IntIntPairRealMatrixMap &sum_QlQlm1,
                            const RealVector &pilot_samples, const RealMatrix &scalarization_response_mapping) const;

  void assign_static_member_problem18(Real &var_L_exact, Real &var_H_exact, Real &mu_four_L_exact, Real &mu_four_H_exact, Real &Ax, RealVector &level_cost_vec) const;

  static void target_var_constraint_eval_optpp_problem18(int mode, int n, const RealVector &x, RealVector &g, RealMatrix &grad_g, int &result_mode);
  static void target_sigma_constraint_eval_optpp_problem18(int mode, int n, const RealVector &x, RealVector &g, RealMatrix &grad_g, int &result_mode);
  static double exact_var_of_var_problem18(const RealVector &Nl);
  static double exact_var_of_sigma_problem18(const RealVector &Nl);
  //
  //- Heading: Data
  //

  unsigned short seq_index;

  /// store the allocation_target input specification, prior to run-time
  /// Options right now:
  ///   - Mean = First moment (Mean)
  ///   - Variance = Second moment (Variance or standard deviation depending on moments central or standard)
  short allocationTarget;

  /// option to switch on numerical optimization for solution of sample alloation
  /// of allocationTarget Variance
  bool useTargetVarianceOptimizationFlag;

  /// store the qoi_aggregation_norm input specification, prior to run-time
  /// Options right now:
  ///   - sum = aggregate the variance over all QoIs, compute samples from that
  ///   - max = take maximum sample allocation over QoIs for each level
  short qoiAggregation;

	/// store the convergence_tolerance_type input specification, prior to run-time
  /// Options right now:
  ///   - relative = computes reference tolerance in first iteration and sets convergence_tolerance as 
  ///								 reference tolerance * convergence_tol
  ///   - absolute = sets convergence tolerance from input
  short convergenceTolType;

  /// store the convergence_tolerance_target input specification, prior to run-time
  /// Options right now:
  ///   - variance_constraint = minimizes cost for equality constraint on variance of estimator (rhs of constraint from convergenceTol)
  ///   - cost_constraint = minizes variance of estimator for equality constraint on cost (rhs of constraint from convergenceTol)
  short convergenceTolTarget;

  RealVector convergenceTolVec;

  /// "scalarization" response_mapping matrix applied to the mlmc sample allocation
  /// when a scalarization, i.e. alpha_1 * mean + alpha_2 * sigma, is the target. 
  RealMatrix scalarizationCoeffs;

  /// Helper data structure to store intermedia sample allocations
  RealMatrix NTargetQoi;
  RealMatrix NTargetQoiFN;
};


inline NonDMultilevelSampling::~NonDMultilevelSampling()
{ }


inline void NonDMultilevelSampling::
configure_indices(size_t group, size_t form, size_t lev, short seq_type)
{
  // preserve special values across type conversions
  unsigned short grp = (group == SZ_MAX) ? USHRT_MAX : (unsigned short)group,
                 frm = (form  == SZ_MAX) ? USHRT_MAX : (unsigned short)form;
  configure_indices(grp, frm, lev, seq_type);
}


inline Real NonDMultilevelSampling::
level_cost(const RealVector& cost, size_t step)
{
  // discrepancies incur two level costs
  return (step) ?
    cost[step] + cost[step-1] : // aggregated {HF,LF} mode
    cost[step];                 //     uncorrected LF mode
}


inline void NonDMultilevelSampling::
nested_response_mappings(const RealMatrix& primary_coeffs,
			 const RealMatrix& secondary_coeffs)
{
  if (scalarizationCoeffs.empty() && allocationTarget == TARGET_SCALARIZATION){
    if (primary_coeffs.numCols() != 2*numFunctions ||
	primary_coeffs.numRows() != 1 ||
	secondary_coeffs.numCols() != 2*numFunctions ||
	secondary_coeffs.numRows() != numFunctions-1){
      Cerr << "\nWrong size for primary or secondary_response_mapping. If you are sure, they are the right size, e.g.,"
	   << " you are interested in quantiles, you need to specify scalarization_response_mapping seperately in multilevel_sampling." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    scalarizationCoeffs.reshape(numFunctions, 2*numFunctions);
    for(size_t row_qoi = 0; row_qoi < numFunctions; ++row_qoi){
      scalarizationCoeffs(0, row_qoi*2) = primary_coeffs(0, row_qoi*2);
      scalarizationCoeffs(0, row_qoi*2+1) = primary_coeffs(0, row_qoi*2+1);
    }
    for(size_t qoi = 1; qoi < numFunctions; ++qoi){
      for(size_t row_qoi = 0; row_qoi < numFunctions; ++row_qoi){
	scalarizationCoeffs(qoi, row_qoi*2)
	  = secondary_coeffs(qoi-1, row_qoi*2);
	scalarizationCoeffs(qoi, row_qoi*2+1)
	  = secondary_coeffs(qoi-1, row_qoi*2+1);
      }
    }
  }
}


inline Real NonDMultilevelSampling::
variance_Ysum(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq)
{
  Real mu_Y = sum_Y / Nlq;
  // Note: precision loss in variance is difficult to avoid without
  // storing full sample history; must accumulate Y^2 across iterations
  // instead of (Y-mean)^2 since mean is updated on each iteration.
  Real var_Y = (sum_YY / Nlq - mu_Y * mu_Y)
             * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  check_negative(var_Y);
  return var_Y;

  /*
  Real new_mu_Y = mu_Y + offset;
  return var_Y
    //  + offset   * offset    // uncenter from old mu_hat
    //  - new_mu_Y * new_mu_Y; // recenter with new_mu_Y
    - mu_Y * mu_Y - 2. * mu_Y * offset; // cancel offset^2
  */
}

inline Real NonDMultilevelSampling::
variance_Ysum_static(Real sum_Y, Real sum_YY, /*Real offset,*/ size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Y = sum_Y / Nlq_pilot;
  // Note: precision loss in variance is difficult to avoid without
  // storing full sample history; must accumulate Y^2 across iterations
  // instead of (Y-mean)^2 since mean is updated on each iteration.
  Real variance_tmp = (sum_YY / Nlq_pilot - mu_Y * mu_Y); 

  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
  }

  Real var_Y = variance_tmp * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  check_negative(var_Y);
  return var_Y;

  /*
  Real new_mu_Y = mu_Y + offset;
  return var_Y
    //  + offset   * offset    // uncenter from old mu_hat
    //  - new_mu_Y * new_mu_Y; // recenter with new_mu_Y
    - mu_Y * mu_Y - 2. * mu_Y * offset; // cancel offset^2
  */
}

inline Real NonDMultilevelSampling::
variance_Qsum(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq)
{
  Real mu_Ql = sum_Ql / Nlq, mu_Qlm1 = sum_Qlm1 / Nlq;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real var_Q =(       sum_QlQl / Nlq - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq - mu_Qlm1 * mu_Qlm1 ) // var_Qlm1
    * (Real)Nlq / (Real)(Nlq - 1);
  check_negative(var_Q);
  return var_Q; // Bessel's correction
}


inline Real NonDMultilevelSampling::
variance_Qsum_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl, Real sum_QlQlm1,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Ql = sum_Ql / Nlq_pilot, mu_Qlm1 = sum_Qlm1 / Nlq_pilot;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real variance_tmp = (       sum_QlQl / Nlq_pilot - mu_Ql   * mu_Ql     // var_Ql
    - 2. * (   sum_QlQlm1 / Nlq_pilot - mu_Ql   * mu_Qlm1 ) // covar_QlQlm1
    +        sum_Qlm1Qlm1 / Nlq_pilot - mu_Qlm1 * mu_Qlm1 ); // var_Qlm1
  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
  }
  check_negative(variance_tmp);
  return variance_tmp * (Real)Nlq / (Real)(Nlq - 1.); // Bessel's correction
}

inline Real NonDMultilevelSampling::
var_lev_l(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq)
{
  Real mu_Ql = sum_Ql / Nlq, mu_Qlm1 = sum_Qlm1 / Nlq;
  Real var_lev_Q = (sum_QlQl / Nlq - mu_Ql * mu_Ql)     // var_Ql
    			- (sum_Qlm1Qlm1 / Nlq - mu_Qlm1 * mu_Qlm1 ) // var_Qlm1
    * (Real)Nlq / (Real)(Nlq - 1); // Bessel's correction
  check_negative(var_lev_Q);
  return var_lev_Q; 
}

inline Real NonDMultilevelSampling::
var_lev_l_static(Real sum_Ql, Real sum_Qlm1, Real sum_QlQl,
	      Real sum_Qlm1Qlm1, size_t Nlq_pilot, size_t Nlq, bool compute_gradient, Real& grad)
{
  Real mu_Ql = sum_Ql / Nlq_pilot, mu_Qlm1 = sum_Qlm1 / Nlq_pilot;
  //var_Y = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
  Real variance_tmp = (sum_QlQl / Nlq_pilot - mu_Ql   * mu_Ql)     // var_Ql
    										- (sum_Qlm1Qlm1 / Nlq_pilot - mu_Qlm1 * mu_Qlm1 ); // var_Qlm1
  if(compute_gradient){
  	grad = variance_tmp * (-1.) / ((Real)(Nlq - 1.) * (Real)(Nlq - 1.));
  }
  check_negative(variance_tmp);
  return variance_tmp * (Real)Nlq / (Real)(Nlq - 1.); // Bessel's correction
}

inline Real NonDMultilevelSampling::
aggregate_variance_Ysum(const Real* sum_Y, const Real* sum_YY,
			const SizetArray& N_l)
{
  Real agg_var_l = 0.;//, var_Y;
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
  for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
    agg_var_l += variance_Ysum(sum_Y[qoi], sum_YY[qoi], N_l[qoi]);
    //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
  //}
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
  return agg_var_l;
}

inline Real NonDMultilevelSampling::
aggregate_variance_Qsum(const Real* sum_Ql,      const Real* sum_Qlm1,
                       const Real* sum_QlQl,     const Real* sum_QlQlm1,
                       const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                       const size_t lev)
{
  Real agg_var_l = 0., var_Y;
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
  for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
    agg_var_l += aggregate_variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQl, sum_QlQlm1, sum_Qlm1Qlm1, N_l, lev, qoi);
    //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
  //}
  //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
  return agg_var_l;
}

inline Real NonDMultilevelSampling::
aggregate_variance_Qsum(const Real* sum_Ql,       const Real* sum_Qlm1,
                      const Real* sum_QlQl,     const Real* sum_QlQlm1,
                      const Real* sum_Qlm1Qlm1, const SizetArray& N_l,
                      const size_t lev, const size_t qoi)
{
       Real agg_var_l = 0., var_Y;
       //if (outputLevel >= DEBUG_OUTPUT)   Cout << "[ ";
       //for (size_t qoi=0; qoi<numFunctions; ++qoi) //{
         agg_var_l = (lev) ?
                      variance_Qsum(sum_Ql[qoi], sum_Qlm1[qoi], sum_QlQl[qoi], sum_QlQlm1[qoi],
                                    sum_Qlm1Qlm1[qoi], N_l[qoi]) :
                      variance_Ysum(sum_Ql[qoi], sum_QlQl[qoi], N_l[qoi]);
       //if (outputLevel >= DEBUG_OUTPUT) Cout << var_Y << ' ';
       //}
       //if (outputLevel >= DEBUG_OUTPUT)   Cout << "]\n";
       return agg_var_l;
}

inline Real NonDMultilevelSampling::
aggregate_mse_Yvar(const Real* var_Y, const SizetArray& N_l)
{
  Real agg_mse = 0.;
  // aggregate MC estimator variance for each QoI
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    agg_mse += var_Y[qoi] / N_l[qoi];
  return agg_mse;
}


inline Real NonDMultilevelSampling::
aggregate_mse_Ysum(const Real* sum_Y, const Real* sum_YY, const SizetArray& N_l)
{
  Real agg_mse = 0.;//, var_Y;
  // aggregate MC estimator variance for each QoI
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    agg_mse += variance_Ysum(sum_Y[qoi], sum_YY[qoi], N_l[qoi]) / N_l[qoi];
  return agg_mse;
}

inline void NonDMultilevelSampling::
aggregate_mse_target_Qsum(RealMatrix& agg_var_qoi, 
						  						const Sizet2DArray& N_l, const size_t step, RealVector& estimator_var0_qoi)
{
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
		estimator_var0_qoi[qoi] += agg_var_qoi(qoi, step)/N_l[step][qoi];
	}
}

inline void NonDMultilevelSampling::set_convergence_tol(const RealVector& estimator_var0_qoi, const RealVector& cost, RealVector& eps_sq_div_2_qoi)
{
	// compute epsilon target based on relative tolerance: total MSE = eps^2
	// which is equally apportioned (eps^2 / 2) among discretization MSE and
	// estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
	// discretization error, we compute an initial estimator variance and
	// then seek to reduce it by a relative_factor <= 1.
	for (size_t qoi = 0; qoi < numFunctions; ++qoi) {

	  if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
		  if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE){
				eps_sq_div_2_qoi[qoi] = estimator_var0_qoi[qoi] * convergenceTolVec[qoi];
			}else if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_ABSOLUTE){
				eps_sq_div_2_qoi[qoi] = convergenceTolVec[qoi];
			}else{
	  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolType is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}
		}else if (convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
			if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE){
				eps_sq_div_2_qoi[qoi] = cost[cost.length()-1] * convergenceTolVec[qoi]; //Relative cost with respect to convergenceTol evaluations on finest grid
			}else if(convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_ABSOLUTE){
				eps_sq_div_2_qoi[qoi] = convergenceTolVec[qoi]; //Direct cost
			}else{
	  	  Cerr << "NonDMultilevelSampling::set_convergence_tol: convergenceTolType is not known.\n";
	  	  abort_handler(INTERFACE_ERROR);
			}
		}else{
  	  Cout << "NonDMultilevelSampling::set_convergence_tol: convergenceTolTarget is not known.\n";
  	  abort_handler(INTERFACE_ERROR);
		}

	}
	if (outputLevel == DEBUG_OUTPUT)
	       Cout << "Epsilon squared target per QoI = " << eps_sq_div_2_qoi << std::endl;
}

inline void NonDMultilevelSampling::
increment_ml_equivalent_cost(size_t new_N_l, Real lev_cost, Real ref_cost)
{
  // increment the equivalent number of HF evaluations
  if (new_N_l)
    equivHFEvals += new_N_l * lev_cost / ref_cost; // normalize into equiv HF
}


inline void NonDMultilevelSampling::
compute_ml_equivalent_cost(const SizetArray& raw_N_l, const RealVector& cost)
{
  size_t step, num_steps = raw_N_l.size();
  equivHFEvals = raw_N_l[0] * cost[0]; // first level is single eval
  for (step=1; step<num_steps; ++step) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_l[step] * (cost[step] + cost[step - 1]);
  equivHFEvals /= cost[num_steps - 1]; // normalize into equivalent HF evals
}


inline void NonDMultilevelSampling::accumulate_offsets(RealVector& mu)
{
  IntRespMCIter r_it = allResponses.begin();
  size_t qoi, num_samp, num_fns = r_it->second.num_functions();
  mu.sizeUninitialized(num_fns);
  Real q_l, sum;
  for (qoi=0; qoi<num_fns; ++qoi) {
    num_samp = 0; sum = 0.;
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      q_l = r_it->second.function_value(qoi);
      if (std::isfinite(q_l)) // neither NaN nor +/-Inf
	{ sum += q_l; ++num_samp; }
    }
    mu[qoi] = sum / num_samp;
  }
}


inline Real NonDMultilevelSampling::var_of_var_ml_l0(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
                                                      const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const bool compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm2l_sq, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1), //.at() returns a const member
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);

  uncentered_to_centered(sum_Q1l(qoi, 0) / Nlq_pilot, sum_Q2l(qoi, 0) / Nlq_pilot,
                         sum_Q3l(qoi, 0) / Nlq_pilot, sum_Q4l(qoi, 0) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);

  cm2l_sq = cm2l * cm2l;
  var_of_var = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);

  if(compute_gradient) {
    grad_g = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
             ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
             - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
               ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq;
  }

  check_negative(var_of_var);
  return var_of_var;
}


inline Real NonDMultilevelSampling::var_of_var_ml_lmax(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
                                                     const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const bool compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm2l_sq, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);

  uncentered_to_centered(sum_Q1l(qoi, 1) / Nlq_pilot, sum_Q2l(qoi, 1) / Nlq_pilot,
                         sum_Q3l(qoi, 1) / Nlq_pilot, sum_Q4l(qoi, 1) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);

  cm2l_sq = cm2l * cm2l;
  var_of_var = (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);

  if(compute_gradient) {
    grad_g = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
             ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
             - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
               ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq;
  }

  check_negative(var_of_var);
  return var_of_var;
}

inline Real NonDMultilevelSampling::var_of_var_ml_l(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1,
                                                    const size_t Nlq_pilot, const Real Nlq, const size_t qoi, const size_t lev, const bool compute_gradient, Real& grad_g)
{
  Real cm1l, cm2l, cm3l, cm4l, cm1lm1, cm2lm1,
      cm3lm1, cm4lm1, cm1l_sq, cm2l_sq, cm2lm1_sq,
      mu_Q2l, mu_Q2lm1, mu_Q2lQ2lm1,
      mu_Q1lm1_mu_Q2lQ1lm1, mu_Q1lm1_mu_Q1lm1_muQ2l, mu_Q1l_mu_Q1lQ2lm1, mu_Q1l_mu_Q1l_mu_Q2lm1,
      mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1, mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1, mu_Q2l_muQ2lm1, mu_Q1lQ1lm1_mu_Q1lQ1lm1,
      mu_P2lP2lm1, var_P2l, var_P2lm1, covar_P2lP2lm1, term, var_of_var;

  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);

  mu_Q2l = sum_Q2l(qoi, lev) / Nlq_pilot;
  uncentered_to_centered(sum_Q1l(qoi, lev) / Nlq_pilot, mu_Q2l,
                         sum_Q3l(qoi, lev) / Nlq_pilot, sum_Q4l(qoi, lev) / Nlq_pilot,
                         cm1l, cm2l, cm3l, cm4l, Nlq_pilot);
  mu_Q2lm1 = sum_Q2lm1(qoi, lev) / Nlq_pilot;
  uncentered_to_centered(sum_Q1lm1(qoi, lev) / Nlq_pilot, mu_Q2lm1,
                         sum_Q3lm1(qoi, lev) / Nlq_pilot, sum_Q4lm1(qoi, lev) / Nlq_pilot,
                         cm1lm1, cm2lm1, cm3lm1, cm4lm1, Nlq_pilot);
  cm2l_sq = cm2l * cm2l;
  cm2lm1_sq = cm2lm1 * cm2lm1;


  // [fm] bias correction for var_P2l and var_P2lm1
  var_P2l = Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);
  var_P2lm1 =
      Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4lm1 - (Nlq - 3.) / (Nlq - 1.) * cm2lm1_sq);

  //[fm] unbiased products of mean
  mu_Q2lQ2lm1 = sum_Q2lQ2lm1(qoi, lev) / Nlq_pilot;
  mu_Q1lm1_mu_Q2lQ1lm1 = unbiased_mean_product_pair(sum_Q1lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                    sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1lm1_mu_Q1lm1_muQ2l = unbiased_mean_product_triplet(sum_Q1lm1(qoi, lev), sum_Q1lm1(qoi, lev),
                                                          sum_Q2l(qoi, lev),
                                                          sum_Q2lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                          sum_Q2lQ1lm1(qoi, lev),
                                                          sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1l_mu_Q1lQ2lm1 = unbiased_mean_product_pair(sum_Q1l(qoi, lev), sum_Q1lQ2lm1(qoi, lev), sum_Q2lQ2lm1(qoi, lev),
                                                  Nlq_pilot);
  mu_Q1l_mu_Q1l_mu_Q2lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1l(qoi, lev), sum_Q2lm1(qoi, lev),
                                                         sum_Q2l(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                         sum_Q1lQ2lm1(qoi, lev),
                                                         sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                             sum_Q1lQ1lm1(qoi, lev),
                                                             sum_Q1lQ1lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                             sum_Q1lQ2lm1(qoi, lev),
                                                             sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1 = unbiased_mean_product_pairpair(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                                  sum_Q1lQ1lm1(qoi, lev),
                                                                  sum_Q2l(qoi, lev), sum_Q2lm1(qoi, lev),
                                                                  sum_Q2lQ1lm1(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                                  sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_Q2l_muQ2lm1 = unbiased_mean_product_pair(sum_Q2l(qoi, lev), sum_Q2lm1(qoi, lev), sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  mu_P2lP2lm1 = mu_Q2lQ2lm1 //E[QL2 Ql2]
                - 2. * mu_Q1lm1_mu_Q2lQ1lm1 //E[Ql] E[QL2Ql]
                + 2. * mu_Q1lm1_mu_Q1lm1_muQ2l //E[Ql]2 E[QL2]
                - 2. * mu_Q1l_mu_Q1lQ2lm1 //E[QL] E[QLQl2]
                + 2. * mu_Q1l_mu_Q1l_mu_Q2lm1 //E[QL]2 E[Ql2]
                + 4. * mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 //E[QL] E[Ql] E[QLQl]
                - 4. * mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1 //E[QL]2 E[Ql]2
                - mu_Q2l_muQ2lm1; //E[QL2] E[Ql2]

  // [fm] unbiased by opening up the square and compute three different term
  mu_Q1lQ1lm1_mu_Q1lQ1lm1 = unbiased_mean_product_pair(sum_Q1lQ1lm1(qoi, lev), sum_Q1lQ1lm1(qoi, lev),
                                                       sum_Q2lQ2lm1(qoi, lev), Nlq_pilot);
  term = mu_Q1lQ1lm1_mu_Q1lQ1lm1 - 2. * mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 + mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1;

  //[fm] Using only unbiased estimators the sum is also unbiased
  covar_P2lP2lm1
      = mu_P2lP2lm1 + term / (Nlq - 1.);

  var_of_var = (var_P2l + var_P2lm1 - 2. * covar_P2lP2lm1) / Nlq;

  if(compute_gradient) {
    grad_g = ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
                     ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4l
                     - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
                       ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2l_sq
                     + ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 1.) * (2. * Nlq - 2.)) /
                       ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm4lm1
                     - ((Nlq * Nlq - 2. * Nlq + 3.) - (Nlq - 3.) * (2. * Nlq - 2.)) /
                       ((Nlq * Nlq - 2. * Nlq + 3.) * (Nlq * Nlq - 2. * Nlq + 3.)) * cm2lm1_sq
                     - 2. * (-1. / (Nlq * Nlq) * mu_P2lP2lm1 +
                             (-2. * Nlq + 1.) / ((Nlq * Nlq - Nlq) * (Nlq * Nlq - Nlq)) * term);
    }

  check_negative(var_of_var);
  return var_of_var;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_pair(const Real sumQ1, const Real sumQ2, const Real sumQ1Q2, const size_t Nlq)
{
  Real mean1, mean2, bessel_corr1, bessel_corr2 = 0.;

  mean1 = 1./Nlq * 1./Nlq * sumQ1 * sumQ2;
  mean2 = 1./Nlq * sumQ1Q2;
  bessel_corr1 = (Real)Nlq / ((Real)Nlq - 1.);
  bessel_corr2 = 1. / ((Real)Nlq - 1.);

  return bessel_corr1*mean1 - bessel_corr2*mean2;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_triplet(const Real sumQ1, const Real sumQ2, const Real sumQ3,
                                                                  const Real sumQ1Q2, const Real sumQ1Q3, const Real sumQ2Q3, const Real sumQ1Q2Q3, const size_t Nlq)
{
  Real mean1, mean2, mean3, bessel_corr1, bessel_corr2, bessel_corr3 = 0.;

  mean1 = 1./Nlq * 1./Nlq * 1./Nlq * sumQ1 * sumQ2 * sumQ3;
  mean2 = unbiased_mean_product_pair(sumQ1Q2, sumQ3, sumQ1Q2Q3, Nlq);
  mean2 += unbiased_mean_product_pair(sumQ2Q3, sumQ1, sumQ1Q2Q3, Nlq);
  mean2 += unbiased_mean_product_pair(sumQ1Q3, sumQ2, sumQ1Q2Q3, Nlq);
  mean3 = 1./((Real)Nlq) * sumQ1Q2Q3;
  bessel_corr1 = (Nlq * Nlq)/((Nlq - 1.)*(Nlq - 2.));
  bessel_corr2 = 1./(Nlq - 2.);
  bessel_corr3 = 1./((Nlq - 1.)*(Nlq - 2.));

  return bessel_corr1 * mean1 - bessel_corr2 * mean2 - bessel_corr3 * mean3;
}

inline Real NonDMultilevelSampling::unbiased_mean_product_pairpair(const Real sumQ1, const Real sumQ2, const Real sumQ1Q2,
                                                                   const Real sumQ1sq, const Real sumQ2sq,
                                                                   const Real sumQ1sqQ2, const Real sumQ1Q2sq, const Real sumQ1sqQ2sq, const size_t Nlq)
{
  Real mean1, mean2, mean3, mean4, bessel_corr1, bessel_corr2, bessel_corr3, bessel_corr4 = 0.;

  mean1 = 1./Nlq * 1./Nlq * 1./Nlq * 1./Nlq * sumQ1 * sumQ1 * sumQ2 * sumQ2;

  mean2 = unbiased_mean_product_triplet(sumQ1sq, sumQ2, sumQ2, sumQ1sqQ2, sumQ1sqQ2, sumQ2sq, sumQ1sqQ2sq, Nlq);
  mean2 += 4. * unbiased_mean_product_triplet(sumQ1Q2, sumQ1, sumQ2, sumQ1sqQ2, sumQ1Q2sq, sumQ1Q2, sumQ1sqQ2sq, Nlq);
  mean2 += unbiased_mean_product_triplet(sumQ1, sumQ1, sumQ2sq, sumQ1sq, sumQ1Q2sq, sumQ1Q2sq, sumQ1sqQ2sq, Nlq);

  mean3 = unbiased_mean_product_pair(sumQ1sq, sumQ2sq, sumQ1sqQ2sq, Nlq);
  mean3 += 2. * unbiased_mean_product_pair(sumQ1Q2, sumQ1Q2, sumQ1sqQ2sq, Nlq);
  mean3 += 2. * unbiased_mean_product_pair(sumQ1sqQ2, sumQ2, sumQ1sqQ2sq, Nlq);
  mean3 += 2. * unbiased_mean_product_pair(sumQ1, sumQ1Q2sq, sumQ1sqQ2sq, Nlq);

  mean4 = 1./Nlq * sumQ1sqQ2sq;

  bessel_corr1 = (Nlq * Nlq * Nlq)/((Nlq - 1.)*(Nlq - 2.)*(Nlq - 3.));
  bessel_corr2 = 1./(Nlq - 3.);
  bessel_corr3 = 1./((Nlq - 2.)*(Nlq - 3.));
  bessel_corr4 = 1./((Nlq - 1.)*(Nlq - 2.)*(Nlq - 3.));

  return bessel_corr1 * mean1 - bessel_corr2 * mean2 - bessel_corr3 * mean3 - bessel_corr4 * mean4;
}

} // namespace Dakota

#endif
