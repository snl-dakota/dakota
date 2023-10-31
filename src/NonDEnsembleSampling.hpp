/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_ENSEMBLE_SAMPLING_H
#define NOND_ENSEMBLE_SAMPLING_H

#include "NonDSampling.hpp"
#include "DataMethod.hpp"


namespace Dakota {

/// Base class for Monte Carlo sampling across Model ensembles.

/** Monte Carlo methods may employ model ensembles as variance-reduction
    techniques, utilitizing lower fidelity simulations that have response
    QoI that are correlated with the high-fidelity response QoI. */

class NonDEnsembleSampling: public NonDSampling
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NonDEnsembleSampling(ProblemDescDB& problem_db, Model& model);
  /// destructor (virtual declaration should be redundant with ~Iterator,
  /// but this is top of MLMF diamond so doesn't hurt to be explicit)
  ~NonDEnsembleSampling();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();

protected:

  //
  //- Heading: New virtual functions
  //

  virtual Real estimator_accuracy_metric() = 0;
  virtual void print_variance_reduction(std::ostream& s);

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();
  //void core_run();
  void post_run(std::ostream& s);
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

  void initialize_final_statistics();
  void update_final_statistics();

  bool seed_updated();
  void active_set_mapping();

  //
  //- Heading: Member functions
  //

  /// return cost metric for entry into finalStatistics
  Real estimator_cost_metric();

  /// advance any sequence specifications
  void assign_specification_sequence(size_t index);
  /// extract current random seed from randomSeedSeqSpec
  int seed_sequence(size_t index);

  /// synchronize activeSet with iteratedModel's response size
  void resize_active_set();

  /// increment samples array with a shared scalar
  void increment_samples(SizetArray& N_l, size_t incr);
  /// increment 2D samples array with a shared 1D array (additional dim is QoI)
  void increment_samples(Sizet2DArray& N_l, const SizetArray& incr);

  /// compute the variance of the mean estimator (Monte Carlo sample average)
  void compute_mc_estimator_variance(const RealVector& var_l,
				     const SizetArray& N_l,
				     RealVector& mc_est_var);
  /// compute the variance of the mean estimator (Monte Carlo sample average)
  /// after projection with additional samples (var_l remains fixed)
  void project_mc_estimator_variance(const RealVector& var_l,
				     const SizetArray& N_l, size_t new_samp,
				     RealVector& mc_est_var);

  /// convert estimator variance ratios to average estimator variance
  void estvar_ratios_to_avg_estvar(const RealVector& estvar_ratios,
				   const RealVector& var_H,
				   const SizetArray& N_H, Real& avg_est_var);

  /// compute scalar control variate parameters
  void compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
			  size_t N_shared, Real& beta);
  /// compute matrix control variate parameters
  void compute_mf_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
			  const RealMatrix& sum_LL, const RealMatrix& sum_LH,
			  const SizetArray& N_shared, size_t lev,
			  RealVector& beta);
  /// compute vector control variate parameters
  void compute_mf_control(const RealVector& sum_L, const RealVector& sum_H,
			  const RealVector& sum_LL, const RealVector& sum_LH,
			  const SizetArray& N_shared, RealVector& beta);

  /// export allSamples to tagged tabular file
  void export_all_samples(String root_prepend, const Model& model,
			  size_t iter, size_t step);

  /// convert uncentered raw moments (multilevel expectations) to
  /// standardized moments
  void convert_moments(const RealMatrix& raw_mom, RealMatrix& final_mom);

  /// convert uncentered (raw) moments to centered moments; biased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4,
			      size_t Nlq);
  /// convert uncentered (raw) moments to centered moments; unbiased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
			      Real& cm1, Real& cm2, Real& cm3, Real& cm4);
  /// convert centered moments to standardized moments
  static void centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
			    Real& sm1, Real& sm2, Real& sm3, Real& sm4);
  /// detect, warn, and repair a negative central moment (for even orders)
  static void check_negative(Real& cm);

  /// compute sum of a set of observations
  static Real sum(const Real* vec, size_t vec_len);
  /// compute average of a set of observations
  static Real average(const Real* vec, size_t vec_len);
  /// compute average of a set of observations
  static Real average(const RealVector& vec);
  /// compute average of a set of observations
  static Real average(const SizetArray& sa);
  /// compute row-averages for each column or column-averages for each row
  static void average(const RealMatrix& mat, size_t avg_index,
		      RealVector& avg_vec);

  //
  //- Heading: Data
  //

  /// number of model forms/resolution in the (outer) sequence
  size_t numSteps;
  /// type of model sequence enumerated with primary MF/ACV loop over steps
  short sequenceType;
  /// setting for the inactive model dimension not traversed by primary MF/ACV
  /// loop over steps
  size_t secondaryIndex;
  /// relative costs of model forms/resolution levels within a 1D sequence
  RealVector sequenceCost;

  /// total number of successful sample evaluations (excluding faults)
  /// for each model form, discretization level, and QoI
  Sizet3DArray NLevActual;
  /// total number of allocated sample evaluations (prior to any faults)
  /// for each model form and discretization level (same for all QoI)
  Sizet2DArray NLevAlloc;

  /// store the pilot_samples input specification, prior to run-time
  /// invocation of load_pilot_sample()
  SizetArray pilotSamples;
  /// enumeration for pilot management modes: ONLINE_PILOT (default),
  /// OFFLINE_PILOT, PILOT_PROJECTION
  short pilotMgmtMode;

  /// indicates use of online cost recovery rather than offline
  /// user-specified cost ratios
  bool onlineCost;
  /// indices of cost data within response metadata, one per model form
  SizetSizetPairArray costMetadataIndices;

  /// user specification for seed_sequence
  SizetArray randomSeedSeqSpec;

  /// major iteration counter
  size_t mlmfIter;

  /// (inactive) option to backfill simulation failures by comparing targets
  /// against successful sample completions rather than sample allocations
  bool backfillFailures;

  /// equivalent number of high fidelity evaluations accumulated using samples
  /// across multiple model forms and/or discretization levels
  Real equivHFEvals;
  /// for sample projections, the calculated increment in equivHFEvals that
  /// would be incurred if full iteration/statistics were needed
  Real deltaEquivHF;

  /// variances for HF truth (length numFunctions)
  RealVector varH;

  /// initial estimator variance from shared pilot (no CV reduction)
  RealVector estVarIter0;

  /// QOI_STATISTICS (moments, level mappings) or ESTIMATOR_PERFORMANCE
  /// (for model tuning of estVar,equivHFEvals by an outer loop)
  short finalStatsType;

  /// if defined, export each of the sample increments in ML, CV, MLCV
  /// using tagged tabular files
  bool exportSampleSets;
  /// format for exporting sample increments using tagged tabular files
  unsigned short exportSamplesFormat;

  // store the allocation_target input specification, prior to run-time
  // Options right now:
  //   - Mean = First moment (Mean)
  //   - Variance = Second moment (Variance or standard deviation depending
  //     on moments central or standard)
  //short allocationTarget;

  // store the qoi_aggregation_norm input specification, prior to run-time
  // Options right now:
  //   - sum = aggregate the variance over all QoIs, compute samples from that
  //   - max = take maximum sample allocation over QoIs for each level
  //short qoiAggregation;

private:

  //
  //- Heading: Helper functions
  //

  /// cache state of seed sequence for use in seed_updated()
  size_t seedIndex;
};


inline void NonDEnsembleSampling::print_variance_reduction(std::ostream& s)
{ } // default is no-op


inline Real NonDEnsembleSampling::estimator_cost_metric()
{ return equivHFEvals + deltaEquivHF; }


inline void NonDEnsembleSampling::resize_active_set()
{
  size_t m_resp_len = iteratedModel.response_size();
  if (activeSet.request_vector().size() != m_resp_len) {
    // synch activeSet with iteratedModel.response_size()
    activeSet.reshape(iteratedModel.response_size());
    activeSet.request_values(1);
  }
}


/** extract an active seed from a seed sequence */
inline int NonDEnsembleSampling::seed_sequence(size_t index)
{
  // return 0 for cases where seed is undefined or will not be updated

  size_t seq_len = randomSeedSeqSpec.size();
  if (seq_len == 0)
    seedIndex = SZ_MAX; // no spec -> non-repeatable samples
  else if (!varyPattern) // continually reset seed to specified value
    seedIndex = std::min(index, seq_len-1); // use end if sequence exhausted
  else if (mlmfIter == 0) // pilot sample: only advance until sequence exhausted
    seedIndex = (index < seq_len) ? index : SZ_MAX;
  else
    seedIndex = SZ_MAX;

  return (seedIndex == SZ_MAX) ? 0 : randomSeedSeqSpec[seedIndex];
}


/** extract an active seed from a seed sequence */
inline bool NonDEnsembleSampling::seed_updated()
{
  if   ( seedIndex == SZ_MAX) return false;  // no update
  else { seedIndex =  SZ_MAX; return true; } // consume most recent update

  /*
  size_t   seq_len = randomSeedSeqSpec.size();
  if      (seq_len  == 0) return false; // no spec -> non-repeatable
  else if (!varyPattern)  return true;  // fixed_seed: always reset to spec val
  else if (mlmfIter == 0 && index < seq_len) return true; // pilot iter
  else                    return false; // seed sequence exhausted, no update
  */
}


inline void NonDEnsembleSampling::
compute_mc_estimator_variance(const RealVector& var_l, const SizetArray& N_l,
			      RealVector& mc_est_var)
{
  // Defines initial estvar for use as a fixed reference (MC variance from
  // iteration 0 pilot sample) for comparing against convergenceTol
  mc_est_var.sizeUninitialized(numFunctions);
  size_t qoi, N_l_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    N_l_q = N_l[qoi]; // can be zero in offline pilot cases
    mc_est_var[qoi] = (N_l_q) ? var_l[qoi] / N_l_q : DBL_MAX;
  }
}


inline void NonDEnsembleSampling::
estvar_ratios_to_avg_estvar(const RealVector& estvar_ratios,
			    const RealVector& var_H, const SizetArray& N_H,
			    Real& avg_est_var)
{
  RealVector est_var(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    est_var[qoi] = estvar_ratios[qoi] * var_H[qoi] / N_H[qoi];
  avg_est_var = average(est_var);
}


inline void NonDEnsembleSampling::
increment_samples(SizetArray& N_samp, size_t incr)
{
  if (!incr) return;
  size_t q, nq = N_samp.size();
  for (q=0; q<nq; ++q)
    N_samp[q] += incr;
}


inline void NonDEnsembleSampling::
increment_samples(Sizet2DArray& N_samp, const SizetArray& incr)
{
  size_t l, nl = N_samp.size();
  if (incr.size() != nl) {
    Cerr << "Error: inconsistent array sizes in NonDEnsembleSampling::"
	 << "increment_samples()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (l=0; l<nl; ++l)
    increment_samples(N_samp[l], incr[l]);
}


inline void NonDEnsembleSampling::
compute_mf_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		   size_t N_shared, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real  mu_L =  sum_L  / N_shared; //, mu_H = sum_H / N_shared,
  //   var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
  //  cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;

  // beta^* = rho_LH sigma_H / sigma_L
  //        = cov_LH / var_L  (since rho_LH = cov_LH / sigma_H / sigma_L)
  // Cancel shared terms within cov_LH / var_L:
  beta = (sum_LH - mu_L * sum_H) / (sum_LL - mu_L * sum_L);
}


inline void NonDEnsembleSampling::
compute_mf_control(const RealVector& sum_L, const RealVector& sum_H,
		   const RealVector& sum_LL, const RealVector& sum_LH,
		   const SizetArray& N_shared, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mf_control(sum_L[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		       N_shared[qoi], beta[qoi]);
}


inline void NonDEnsembleSampling::
compute_mf_control(const RealMatrix& sum_L,  const RealMatrix& sum_H,
		   const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		   const SizetArray& N_shared, size_t lev, RealVector& beta)
{
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    compute_mf_control(sum_L(qoi,lev), sum_H(qoi,lev), sum_LL(qoi,lev),
		       sum_LH(qoi,lev), N_shared[qoi], beta[qoi]);
}


inline void NonDEnsembleSampling::
project_mc_estimator_variance(const RealVector& var_l, const SizetArray& N_l,
			      size_t new_samp, RealVector& mc_est_var)
{
  // Defines projected estvar for use as a consistent reference
  mc_est_var.sizeUninitialized(numFunctions);
  size_t qoi, N_l_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    N_l_q = N_l[qoi] + new_samp;
    mc_est_var[qoi] = (N_l_q) ? var_l[qoi] / N_l_q : DBL_MAX;
  }
}


/** For single-level moment calculations without a sample count. */
inline void NonDEnsembleSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
		       Real& cm1, Real& cm2, Real& cm3, Real& cm4)
{
  // convert from uncentered ("raw") to centered moments for a single level

  // For moments from sampling:
  // > Raw moments are unbiased. Central moments are unbiased for an exact mean
  //   (i.e., the samples represent the full population).
  // > For sampling a portion of the population, central moments {2,3,4} are 
  //   biased estimators since the mean is approximated.  The conversion to
  //   unbiased requires a correction based on the number of samples, as
  //   implemented in the function following this one.

  cm1 = rm1;             // mean
  cm2 = rm2 - cm1 * cm1; // variance 

  cm3 = rm3 - cm1 * (3. * cm2 + cm1 * cm1);                         // using cm
  //  = rm3 - cm1 * (3. * rm2 - 2. * cm1 * cm1);                    // using rm

  // the 4th moment is the central moment (non-excess, not cumulant)
  cm4 = rm4 - cm1 * (4. * cm3 + cm1 * (6. * cm2 + cm1 * cm1));      // using cm
  //  = rm4 - cm1 * (4. * rm3 - cm1 * (6. * rm2 - 3. * cm1 * cm1)); // using rm
}


/** For single-level moment calculations with a scalar Nlq. */
inline void NonDEnsembleSampling::
uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4, Real& cm1,
		       Real& cm2, Real& cm3, Real& cm4, size_t Nlq)
{
  // convert from uncentered ("raw") to centered moments for a single level

  // Biased central moment estimators:
  uncentered_to_centered(rm1, rm2, rm3, rm4, cm1, cm2, cm3, cm4);

  // Bias corrections for population-based estimators w/ estimated means:
  if (Nlq > 3) {
    Real cm1_sq = cm1 * cm1;
    Real nm1 = Nlq - 1., nm2 = Nlq - 2., n_sq = Nlq * Nlq;
    cm2 *= Nlq / nm1; // unbiased population variance from Bessel's correction
    cm3 *= n_sq / (nm1 * nm2);
    // From "Modeling with Data," Klemens 2009 (Appendix M).
    // Notes:
    // (1) this is 4th central moment (non-excess, unnormalized),
    //     which differs from the fourth cumulant (excess, unnormalized)
    // (2) cm2 is now unbiased within following conversion:
    
    //cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * cm2 * cm2 )
    //    / (n_sq - 3. * Nlq + 3.);
    //[fm] account for bias correction due to cm2^2 term
    cm4 = ( n_sq * Nlq * cm4 / nm1 - (6. * Nlq - 9.) * (n_sq - Nlq)
	    / (n_sq - 2. * Nlq + 3) * cm2 * cm2 )
        / ( (n_sq - 3. * Nlq + 3.) - (6. * Nlq - 9.) * (n_sq - Nlq)
	    / (Nlq * (n_sq - 2. * Nlq + 3.)) );
  }
  else
    Cerr << "Warning: due to small sample size, resorting to biased estimator "
	 << "conversion in NonDEnsembleSampling::uncentered_to_centered().\n";
}


inline void NonDEnsembleSampling::
centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
		     Real& sm1, Real& sm2, Real& sm3, Real& sm4)
{
  // convert from centered to standardized moments
  sm1 = cm1;                      // mean
  if (cm2 > 0.) {
    sm2 = std::sqrt(cm2);         // std deviation
    sm3 = cm3 / (cm2 * sm2);      // skewness
    sm4 = cm4 / (cm2 * cm2) - 3.; // excess kurtosis
  }
  else {
    Cerr << "\nWarning: central to standard conversion failed due to "
	 << "non-positive\n         variance.  Retaining central moments.\n";
    sm2 = 0.; sm3 = cm3; sm4 = cm4; // or assign NaN to sm{3,4}
  }
}


inline void NonDEnsembleSampling::check_negative(Real& cm)
{
  if (cm < 0.) {
    Cerr << "\nWarning: central moment less than zero (" << cm << ").  "
	 << "Repairing to zero.\n";
    cm = 0.;
    // TO DO:  consider hard error if COV < -tol (pass in mean and cm order)
  }
}


inline Real NonDEnsembleSampling::sum(const Real* vec, size_t vec_len)
{
  Real sum = 0.;
  for (size_t i=0; i<vec_len; ++i)
    sum += vec[i];
  return sum;
}


inline Real NonDEnsembleSampling::
average(const Real* vec, size_t vec_len)
{ return (vec_len == 1) ? vec[0] : sum(vec, vec_len) / (Real)vec_len; }


inline Real NonDEnsembleSampling::average(const RealVector& vec)
{ return average(vec.values(), vec.length()); }


inline void NonDEnsembleSampling::
average(const RealMatrix& mat, size_t avg_index, RealVector& avg_vec)
{
  size_t i, j, nr = mat.numRows(), nc = mat.numCols();
  switch (avg_index) {
  case 0: // average over index 0, retaining index 1
    avg_vec.sizeUninitialized(nc);
    for (i=0; i<nc; ++i)
      avg_vec[i] = average(mat[i], nr); // average over rows for each col vec
    break;
  case 1:
    avg_vec.size(nr); // init to 0
    for (i=0; i<nr; ++i) {
      Real& avg_i = avg_vec[i];         // average over cols for each row vec
      if (nc == 1)
	avg_i = mat(i,0);
      else {
	for (j=0; j<nc; ++j)
	  avg_i += mat(i,j);
	avg_i /= nc;
      }
    }
    break;
  default:
    Cerr << "Error: bad averaging index (" << avg_index
	 << ") in NonDEnsembleSampling::average(RealMatrix)." << std::endl;
    abort_handler(-1); break;
  }
}


inline Real NonDEnsembleSampling::average(const SizetArray& sa)
{
  size_t len = sa.size();
  if (len == 1)
    return (Real)sa[0];
  else {
    size_t sum = 0;
    for (size_t i=0; i<len; ++i)
      sum += sa[i];
    return (Real)sum / (Real)len;
  }
}

} // namespace Dakota

#endif
