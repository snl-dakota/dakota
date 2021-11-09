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

#ifndef NOND_NONHIERARCH_SAMPLING_H
#define NOND_NONHIERARCH_SAMPLING_H

#include "NonDEnsembleSampling.hpp"
//#include "DataMethod.hpp"


namespace Dakota {

// special values for optSubProblemForm
enum { ANALYTIC_SOLUTION = 1, REORDERED_ANALYTIC_SOLUTION,
       R_ONLY_LINEAR_CONSTRAINT, N_VECTOR_LINEAR_CONSTRAINT,
       R_AND_N_NONLINEAR_CONSTRAINT };


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

  //
  //- Heading: member functions
  //

  void shared_increment(size_t iter);
  void shared_approx_increment(size_t iter);

  // manage response mode and active model key from {group,form,lev} triplet.
  // seq_type defines the active dimension for a model sequence.
  //void configure_indices(size_t group,size_t form,size_t lev,short seq_type);

  void assign_active_key(size_t num_steps, size_t secondary_index,
			 bool multilev);

  void initialize_sums(IntRealMatrixMap& sum_L_baseline,
		       IntRealVectorMap& sum_H, IntRealMatrixMap& sum_LH,
		       RealVector& sum_HH);
  void initialize_counts(Sizet2DArray& num_L_baseline, SizetArray& num_H,
			 Sizet2DArray& num_LH);
  void finalize_counts(Sizet2DArray& N_L);

  void ensemble_sample_increment(size_t iter, size_t step);

  void increment_samples(SizetArray& N_l, size_t num_samples);

  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 size_t start, size_t end);
  void increment_equivalent_cost(size_t new_samp, const RealVector& cost,
				 const SizetArray& model_sequence,
				 size_t start, size_t end);

  void compute_variance(Real sum_Q, Real sum_QQ, size_t num_Q, Real& var_Q);
  void compute_variance(const RealVector& sum_Q, const RealVector& sum_QQ,
			const SizetArray& num_Q,       RealVector& var_Q);

  void compute_correlation(Real sum_Q1, Real sum_Q2, Real sum_Q1Q1,
			   Real sum_Q1Q2, Real sum_Q2Q2, size_t num_Q1,
			   size_t num_Q2, size_t num_Q1Q2, Real& var_Q2,
			   Real& rho2_Q1Q2);
  void compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2,
			  size_t num_Q1, size_t num_Q2, size_t num_Q1Q2,
			  Real& cov_Q1Q2);
  
  void mfmc_eval_ratios(const RealMatrix& rho2_LH, const RealVector& cost,
			SizetArray& model_sequence, RealMatrix& eval_ratios,
			short& subprob_form, bool for_warm_start = false);

  void apply_control(Real sum_L_shared, size_t num_shared, Real sum_L_refined,
		     size_t num_refined, Real beta, Real& H_raw_mom);

  //
  //- Heading: Data
  //

  /// number of approximation models managed by non-hierarchical iteratedModel
  size_t numApprox;

  /// enumeration for solution modes: ONLINE_PILOT (default), OFFLINE_PILOT,
  /// PILOT_PROJECTION
  short solutionMode;
  /// formulation for optimization sub-problem that minimizes R^2 subject
  /// to different variable sets and different linear/nonlinear constraints
  short optSubProblemForm;

  /// type of model sequence enumerated with primary MF/ACV loop over steps
  short sequenceType;
  /// setting for the inactive model dimension not traversed by primary MF/ACV
  /// loop over steps
  size_t secondaryIndex;
  /// relative costs of models within sequence of steps
  RealVector sequenceCost;

  /// variances for HF truth (length numFunctions)
  RealVector varH;
  /// number of evaluations of HF truth model (length numFunctions)
  SizetArray numH;

  /// initial estimator variance from shared pilot (no CV reduction)
  RealVector mseIter0;

private:

  //
  //- Heading: helper functions
  //

  /// define model_sequence in increasing metric order
  bool ordered_model_sequence(const RealVector& metric,
			      SizetArray& model_sequence);
  /// determine whether metric is in increasing order for all columns
  bool ordered_model_sequence(const RealMatrix& metric);
};


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


inline void NonDNonHierarchSampling::finalize_counts(Sizet2DArray& N_L)
{
  // post final sample counts back to NLev (needed for final eval summary) by
  // aggregated into 2D array and then inserting into 3D
  N_L.push_back(numH);
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  inflate_final_samples(N_L, multilev, secondaryIndex, NLev);
}


inline void NonDNonHierarchSampling::
increment_samples(SizetArray& N_l, size_t new_samples)
{
  if (new_samples) {
    size_t q, nq = N_l.size();
    for (q=0; q<nq; ++q)
      N_l[q] += new_samples;
  }
}


inline void NonDNonHierarchSampling::
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


inline void NonDNonHierarchSampling::
increment_equivalent_cost(size_t new_samp, const RealVector& cost,
			  const SizetArray& model_sequence,
			  size_t start, size_t end)
{
  if (model_sequence.empty())
    increment_equivalent_cost(new_samp, cost, start, end);
  else {
    size_t i, len = cost.length(), hf_index = len-1, approx;
    Real cost_ref = cost[hf_index];
    if (end == len) // truth is always last
      { equivHFEvals += new_samp; --end; }
    for (i=start; i<end; ++i) {
      approx = model_sequence[i];
      equivHFEvals += (Real)new_samp * cost[approx] / cost_ref;
    }
  }
}


inline bool NonDNonHierarchSampling::
ordered_model_sequence(const RealVector& metric, SizetArray& model_sequence)
{
  size_t i, len = metric.length(), metric_order;  bool ordered = true;
  std::map<Real, size_t> metric_map; std::map<Real, size_t>::iterator it;
  for (i=0; i<len; ++i)
    metric_map[metric[i]] = i; // increasing order (e.g. correlation)
  if (metric_map.size() != len) { // unexpected redundancy
    Cerr << "Error: redundant correlation in MFMC model ordering." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  ordered = true;  model_sequence.resize(len);
  for (i=0, it=metric_map.begin(); it!=metric_map.end(); ++it, ++i) {
    model_sequence[i] = metric_order = it->second;
    if (i != metric_order) ordered = false;
  }
  if (ordered) model_sequence.clear();
  return ordered;
}


inline bool NonDNonHierarchSampling::
ordered_model_sequence(const RealMatrix& metric)
{
  size_t r, c, nr = metric.numRows(), nc = metric.numCols(), metric_order;
  std::map<Real, size_t> metric_map; std::map<Real, size_t>::iterator it;
  bool ordered = true;
  for (r=0; r<nr; ++r) {  // numFunctions
    metric_map.clear();
    for (c=0; c<nc; ++c) // numApprox
      metric_map[metric(r,c)] = c; // order by increasing corr
    if (metric_map.size() != nc) { // unexpected redundancy
      Cerr << "Error: redundant metric in model ordering." << std::endl;
      abort_handler(METHOD_ERROR);
    }
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
  Real bessel_corr_Q = (Real)num_Q / (Real)(num_Q - 1); // num_QQ same as num_Q

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_Q = sum_Q / num_Q;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  var_Q = (sum_QQ / num_Q - mu_Q * mu_Q) * bessel_corr_Q;

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
		    Real sum_Q2Q2, size_t num_Q1, size_t num_Q2, size_t num_Q1Q2,
		    Real& var_Q2, Real& rho2_Q1Q2)
{
  Real //bessel_corr_Q1   = (Real)num_Q1   / (Real)(num_Q1   - 1),
         bessel_corr_Q2   = (Real)num_Q2   / (Real)(num_Q2   - 1);
       //bessel_corr_Q1Q2 = (Real)num_Q1Q2 / (Real)(num_Q1Q2 - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / num_Q1, mu_Q2 = sum_Q2 / num_Q2;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_Q1 = (sum_Q1Q1 / num_Q1   - mu_Q1 * mu_Q1),// * bessel_corr_Q1,
     cov_Q1Q2 = (sum_Q1Q2 / num_Q1Q2 - mu_Q1 * mu_Q2);// * bessel_corr_Q1Q2; // *** TO DO: review Bessel correction derivation for fault tolerance --> not the same N to pull out over N-1
  var_Q2      = (sum_Q2Q2 / num_Q2   - mu_Q2 * mu_Q2);// * bessel_corr_Q2;

  //beta  = cov_Q1Q2 / var_Q1;
  rho2_Q1Q2 = cov_Q1Q2 / var_Q1 * cov_Q1Q2 / var_Q2; // bessel corrs cancel
  var_Q2   *= bessel_corr_Q2; // now apply corr where required
  //Cout << "compute_correlation: sum_Q1 = " << sum_Q1 << " sum_Q2 = " << sum_Q2
  //     << " sum_Q1Q2 = " << sum_Q1Q2  << " num_Q1 = " << num_Q1 <<" num_Q2 = "
  //     << num_Q2 << " num_Q1Q2 = " << num_Q1Q2 << std::endl;

  //Cout << "compute_correlation: rho2_Q1Q2 w/o bessel = " << rho2_Q1Q2;
  //var_Q1   *= bessel_corr_Q1;
  //cov_Q1Q2 *= bessel_corr_Q1Q2;
  //Real rho2_Q1Q2_incl = cov_Q1Q2 / var_Q1 * cov_Q1Q2 / var_Q2; // incl bessel
  //Cout << " rho2_Q1Q2 w/ bessel = " << rho2_Q1Q2_incl << " ratio = "
  //     << rho2_Q1Q2/rho2_Q1Q2_incl << std::endl;
}


inline void NonDNonHierarchSampling::
compute_covariance(Real sum_Q1, Real sum_Q2, Real sum_Q1Q2, size_t num_Q1,
		   size_t num_Q2, size_t num_Q1Q2, Real& cov_Q1Q2)
{
  Real //bessel_corr_Q1 = (Real)num_Q1   / (Real)(num_Q1   - 1),
       //bessel_corr_Q2 = (Real)num_Q2   / (Real)(num_Q2   - 1),
       bessel_corr_Q1Q2 = (Real)num_Q1Q2 / (Real)(num_Q1Q2 - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_Q1 = sum_Q1 / num_Q1,  mu_Q2 = sum_Q2 / num_Q2;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  cov_Q1Q2 = (sum_Q1Q2 / num_Q1Q2 - mu_Q1 * mu_Q2) * bessel_corr_Q1Q2; // *** TO DO: review Bessel correction derivation for fault tolerance --> not the same N to pull out over N-1

  //Cout << "compute_covariance: sum_Q1 = " << sum_Q1 << " sum_Q2 = " << sum_Q2
  //     << " sum_Q1Q2 = " << sum_Q1Q2 << " num_Q1 = " << num_Q1 << " num_Q2 = "
  //     << num_Q2 << " num_Q1Q2 = " << num_Q1Q2 << " cov_Q1Q2 = " << cov_Q1Q2
  //     << std::endl;
}


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

} // namespace Dakota

#endif
