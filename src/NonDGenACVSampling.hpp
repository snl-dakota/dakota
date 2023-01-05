/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGenACVSampling
//- Description: Class for generalized approximate control variate sampling
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_GEN_ACV_SAMPLING_H
#define NOND_GEN_ACV_SAMPLING_H

#include "NonDACVSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {


/// Perform Approximate Control Variate Monte Carlo sampling for UQ.

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

  //void pre_run();
  void core_run();
  //void post_run(std::ostream& s);
  //void print_results(std::ostream& s, short results_state = FINAL_RESULTS);
  //void print_variance_reduction(std::ostream& s);

  void estimator_variance_ratios(const RealVector& N_vec,
				 RealVector& estvar_ratios);

  void precompute_acv_control(const RealVector& avg_eval_ratios,
			      const SizetArray& N_shared);
  void compute_acv_control_mq(RealMatrix& sum_L_base_m, Real sum_H_mq,
			      RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
			      size_t N_shared_q, size_t mom, size_t qoi,
			      RealVector& beta);

  //
  //- Heading: member functions
  //

  void generalized_acv_online_pilot();
  void generalized_acv_offline_pilot();
  void generalized_acv_pilot_projection();

private:

  //
  //- Heading: Helper functions
  //

  void generate_dags(UShortArraySet& model_graphs);

  void compute_parameterized_G_g(const RealVector& N_vec,
				 const UShortArray& dag);

  void invert_C_G_matrix(const RealSymMatrix& C, const RealMatrix& G,
			 RealMatrix& C_G_inv);
  void compute_c_g_vector(const RealMatrix& c, size_t qoi, const RealVector& g,
			  RealVector& c_g);
  void compute_R_sq(const RealMatrix& C_G_inv, const RealVector& c_g,
		    Real var_H_q, Real N_H, Real& R_sq_q);

  void compute_genacv_control(const RealSymMatrix& cov_LL, const RealMatrix& G,
			      const RealMatrix& cov_LH, const RealVector& g,
			      size_t qoi, RealVector& beta);
  void compute_genacv_control(RealMatrix& sum_L, Real sum_H_q,
			      RealSymMatrix& sum_LL_q, RealMatrix& sum_LH,
			      size_t N_shared_q, const RealMatrix& G,
			      const RealVector& g, size_t qoi,RealVector& beta);

  void update_best();
  void restore_best();
  //void reset_acv();

  //
  //- Heading: Data
  //

  /// the "G" matrix in Bomarito et al.
  RealMatrix GMat;
  /// the "g" vector in Bomarito et al.
  RealVector gVec;

  /// type of tunable recursion for defining set of DAGs: KL, SR, or MR
  short dagRecursionType;
  /// the set of admissible DAGs identifying the control variate
  /// targets for each model in the ensemble
  UShortArraySet modelDAGs;
  /// the active instance from within the set computed by generate_dags()
  UShortArraySet::const_iterator activeDAGIter;

  /// the best performing model graph among the set from generate_dags()
  UShortArraySet::const_iterator bestDAGIter;
  /// track estimator variance for best model graph
  Real bestAvgEstVar;
};


inline void NonDGenACVSampling::
invert_C_G_matrix(const RealSymMatrix& C, const RealMatrix& G,
		  RealMatrix& C_G_inv)
{
  size_t i, j, n = C.numRows();
  if (C_G_inv.empty()) C_G_inv.shapeUninitialized(n, n);

  for (i=0; i<n; ++i)
    for (j=0; j<n; ++j)
      C_G_inv(i,j) = C(i,j) * G(i,j);

  RealSolver gen_solver;
  gen_solver.setMatrix(Teuchos::rcp(&C_G_inv, false));
  int code = gen_solver.invert(); // in place
  if (code) {
    Cerr << "Error: serial dense matrix inversion failure (LAPACK error code "
	 << code << ") in NonDACVSampling::invert_C_G_matrix()." << std::endl;
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
    c_g[i] = g(i) * c(qoi, i);
}


inline void NonDGenACVSampling::
compute_R_sq(const RealMatrix& C_G_inv, const RealVector& c_g, Real var_H_q,
	     Real N_H, Real& R_sq_q)
{
  //RealSymMatrix trip(1, false);
  //Teuchos::matTripleProduct(Teuchos::TRANS, 1./var_H_q, CF_inv, A, trip);
  //R_sq_q = trip(0,0);

  // compute triple product: Cov(\Delta,\hat{Q}_H)^T Cov(\Delta,\Delta)
  //                         Cov(\Delta,\hat{Q}_H) = c_g^T C_G_inv c_g
  // Est var = Var(\hat{Q}_H) - triple product = var_H_q / N_H - triple product
  // Est var = var_H_q / N_H ( 1 - N_H * triple product / var_H_q )
  // R^2     = N_H * triple product / var_H_q
  size_t i, j, num_approx = c_g.length();  Real sum;
  R_sq_q = 0.;
  for (i=0; i<num_approx; ++i) {
    sum = 0.;
    for (j=0; j<num_approx; ++j)
      sum += C_G_inv(i,j) * c_g[j];
    R_sq_q += c_g[i] * sum;
  }
  R_sq_q *= N_H / var_H_q;
}


inline void NonDGenACVSampling::
compute_genacv_control(const RealSymMatrix& cov_LL, const RealMatrix& G,
		       const RealMatrix& cov_LH, const RealVector& g,
		       size_t qoi, RealVector& beta)
{
  RealMatrix C_G_inv;  invert_C_G_matrix(covLL[qoi], G, C_G_inv);
  //Cout << "compute_genacv_control qoi " << qoi+1 << ": C_G_inv\n" << C_G_inv;
  RealVector c_g;      compute_c_g_vector(covLH, qoi, g, c_g);
  //Cout << "compute_genacv_control qoi " << qoi+1 << ": c_g\n" << c_g;

  size_t n = G.numRows();
  if (beta.length() != n) beta.size(n);
  beta.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., C_G_inv, c_g, 0.);
  //Cout << "compute_acv_control qoi " << qoi+1 << ": beta\n" << beta;
}


inline void NonDGenACVSampling::
compute_genacv_control(RealMatrix& sum_L, Real sum_H_q, RealSymMatrix& sum_LL_q,
		       RealMatrix& sum_LH, size_t N_shared_q,
		       const RealMatrix& G, const RealVector& g,
		       size_t qoi, RealVector& beta)
{
  // compute cov_LL, cov_LH, var_H across numApprox for a particular QoI
  // > cov_LH is sized for all qoi but only 1 row is used
  RealSymMatrix cov_LL; RealMatrix cov_LH;
  compute_acv_control_covariances(sum_L, sum_H_q, sum_LL_q, sum_LH, N_shared_q,
				  qoi, cov_LL, cov_LH);
  // forward to overload:
  compute_genacv_control(cov_LL, G, cov_LH, g, qoi, beta);
}


inline void NonDGenACVSampling::
precompute_acv_control(const RealVector& avg_eval_ratios,
		       const SizetArray& N_shared)
{
  // Note: while G,g have a more explicit dependence on N_shared[qoi] than F,
  // we mirror the averaged sample allocations and compute G,g once
  RealVector N_vec; //, g;  RealMatrix G;
  r_and_N_to_N_vec(avg_eval_ratios, average(N_shared), N_vec);
  compute_parameterized_G_g(N_vec, *activeDAGIter);
}


inline void NonDGenACVSampling::
compute_acv_control_mq(RealMatrix& sum_L_base_m, Real sum_H_mq,
		       RealSymMatrix& sum_LL_mq, RealMatrix& sum_LH_m,
		       size_t N_shared_q, size_t mom, size_t qoi,
		       RealVector& beta)
{
  if (mom == 1) // variances/covariances already computed for mean estimator
    compute_genacv_control(covLL[qoi], GMat, covLH, gVec, qoi, beta);
  else // compute variances/covariances for higher-order moment estimators
    compute_genacv_control(sum_L_base_m, sum_H_mq, sum_LL_mq, sum_LH_m,
			   N_shared_q, GMat, gVec, qoi, beta); // shared counts
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

} // namespace Dakota

#endif
