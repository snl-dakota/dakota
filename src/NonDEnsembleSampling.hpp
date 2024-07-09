/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
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
  virtual void print_multimodel_summary(std::ostream& s,
    const String& summary_type, bool projections);
  virtual void print_multigroup_summary(std::ostream& s,
    const String& summary_type, bool projections);
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

  /// recover estimates of simulation cost using aggregated response
  /// metadata spanning one batch of samples
  void recover_online_cost(const IntResponseMap& all_resp);
  /// recover estimates of simulation cost using aggregated response
  /// metadata spanning multiple batches of samples
  void recover_online_cost(const IntIntResponse2DMap& batch_resp_map);

  /// enforce either a user cost specification or online cost recovery
  /// for each ACTIVE model within the sequence type
  void check_cost_options(const BitArray& cost_specs,
			  const SizetSizetPairArray& cost_md_indices,
			  short seq_type);

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
  /// increment samples array with a shared scalar
  void increment_samples(SizetArray& N_l, const SizetArray& incr);
  /// increment 2D samples array with a shared 1D array (additional dim is QoI)
  void increment_samples(Sizet2DArray& N_l, const SizetArray& incr);

  /// increment samples array with a shared scalar
  void increment_sums(Real* sum_l, const Real* incr, size_t len);

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
  Real estvar_ratios_to_avg_estvar(const RealVector& estvar_ratios,
				   const RealVector& var_H,
				   const SizetArray& N_H);

  /// initialize relaxFactor prior to iteration
  void reset_relaxation();
  /// update relaxFactor based on iteration number
  void advance_relaxation();

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
  void export_all_samples(const Model& model, const String& tabular_filename);

  /// convert uncentered raw moments (multilevel expectations) to
  /// standardized moments
  void convert_moments(const RealMatrix& raw_mom, RealMatrix& final_mom);

  /// convert uncentered (raw) moments to centered moments; unbiased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3,Real  rm4,
				     Real& cm1, Real& cm2, Real& cm3,Real& cm4);
  /// convert uncentered (raw) moments to centered moments; biased estimators
  static void uncentered_to_centered(Real  rm1, Real  rm2, Real  rm3, Real  rm4,
				     Real& cm1, Real& cm2, Real& cm3, Real& cm4,
				     size_t Nlq);
  /// convert uncentered (raw) moments to centered moments; biased estimators
  static void uncentered_to_centered(const Real* rm, Real* cm, size_t num_mom);
  /// convert uncentered (raw) moments to centered moments; biased estimators
  void uncentered_to_centered(const RealMatrix& raw_mom, RealMatrix& cent_mom);

  /// convert centered moments to standardized moments
  static void centered_to_standard(Real  cm1, Real  cm2, Real  cm3, Real  cm4,
			    Real& sm1, Real& sm2, Real& sm3, Real& sm4);
  /// convert centered moments to standardized moments
  static void centered_to_standard(const Real* cm, Real* sm, size_t num_mom);
  /// convert centered moments to standardized moments
  void centered_to_standard(const RealMatrix& cent_mom, RealMatrix& std_mom);

  /// detect, warn, and repair a negative central moment (for even orders)
  static void check_negative(Real& cm);

  //
  //- Heading: Data
  //

  /// type of model sequence enumerated with primary MF/ACV loop over steps
  short sequenceType;
  /// relative costs of model forms/resolution levels within a 1D sequence
  RealVector sequenceCost;

  /// number of approximation models managed by non-hierarchical iteratedModel
  size_t numApprox;

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
  /// OFFLINE_PILOT, ONLINE_PILOT_PROJECTION, or OFFLINE_PILOT_PROJECTION
  short pilotMgmtMode;

  /// indicates use of user-specified cost ratios, online cost recovery,
  /// or a combination
  short costSource;
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

  /// the current relaxation factor applied to the predicted sample
  /// increment; in typical use, this is an under-relaxation factor to
  /// mitigate over-estimation of the sample allocation based on an
  /// initial approximation to response covariance data
  Real relaxFactor;
  /// index into relaxFactorSequence
  size_t relaxIndex;
  /// a sequence of relaxation factors to use across ML/MF iterations
  /// (see DataMethod.hpp for usage notes)
  RealVector relaxFactorSequence;
  /// a recursive relaxation factor (see DataMethod.hpp for usage notes)
  Real relaxRecursiveFactor;

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

  /// accumulator of cost metadata per sample batch
  void accumulate_online_cost(const IntResponseMap& resp_map,
			      RealVector& accum_cost, SizetArray& num_cost);
  /// convert cost accumulations to averages
  void average_online_cost(const RealVector& accum_cost,
			   const SizetArray& num_cost, RealVector& seq_cost);

  /// cache state of seed sequence for use in seed_updated()
  size_t seedIndex;
};


inline void NonDEnsembleSampling::
print_multigroup_summary(std::ostream& s, const String& summary_type,
			 bool projections)
{ } // default is no-op


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


inline void NonDEnsembleSampling::
average_online_cost(const RealVector& accum_cost, const SizetArray& num_cost,
		    RealVector& seq_cost)
{
  // Finalize the average cost for the ensemble
  size_t step, num_steps = accum_cost.length();  unsigned short mf;
  //if (num_cost.size() != num_steps) { } // not possible in recover_online_cost
  if (seq_cost.length() != num_steps) seq_cost.sizeUninitialized(num_steps);
  const Pecos::ActiveKey& active_key = iteratedModel.active_model_key();

  for (step=0; step<num_steps; ++step) {
    mf = active_key.retrieve_model_form(step);
    if (costMetadataIndices[mf].first != SZ_MAX)
      seq_cost[step] = (num_cost[step]) ?
	accum_cost[step] / num_cost[step] : 0.;
    // else cost to be provided from metadata
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Online cost: accum_cost:\n" << accum_cost << "num_cost:\n"
	 << num_cost << "seq_cost:\n" << seq_cost << std::endl;
}


inline void NonDEnsembleSampling::
recover_online_cost(const IntResponseMap& resp_map)
{
  // uses one response map with QoI aggregation across all Models

  int len = numApprox + 1;
  RealVector accum_cost(len);                    // init to 0
  SizetArray num_cost;  num_cost.assign(len, 0); // init to 0

  accumulate_online_cost(resp_map, accum_cost, num_cost);
  average_online_cost(accum_cost, num_cost, sequenceCost);
}


inline void NonDEnsembleSampling::
recover_online_cost(const IntIntResponse2DMap& batch_resp_map)
{
  // uses multiple batches of resp maps with QoI aggregation across all Models

  int len = numApprox + 1;
  RealVector accum_cost(len);                    // init to 0
  SizetArray num_cost;  num_cost.assign(len, 0); // init to 0
  size_t b, num_batch = batch_resp_map.size();
  IntIntResponse2DMap::const_iterator b_it;

  for (b=0; b<num_batch; ++b) {
    b_it = batch_resp_map.find(b);
    if (b_it == batch_resp_map.end()) {
      Cerr << "Error: batch id lookup failure in recover_online_cost()"
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
    else
      accumulate_online_cost(b_it->second, accum_cost, num_cost);
  }
  average_online_cost(accum_cost, num_cost, sequenceCost);
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


inline Real NonDEnsembleSampling::
estvar_ratios_to_avg_estvar(const RealVector& estvar_ratios,
			    const RealVector& var_H, const SizetArray& N_H)
{
  RealVector est_var(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    est_var[qoi] = estvar_ratios[qoi] * var_H[qoi] / N_H[qoi];
  return average(est_var);
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
increment_samples(SizetArray& N_samp, const SizetArray& incr)
{
  size_t q, nq = N_samp.size();
  if (incr.size() != nq) {
    Cerr << "Error: inconsistent array sizes (" << nq << " target, "
	 << incr.size() << " increment) in NonDEnsembleSampling::"
	 << "increment_samples()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (q=0; q<nq; ++q)
    N_samp[q] += incr[q];
}


inline void NonDEnsembleSampling::
increment_samples(Sizet2DArray& N_samp, const SizetArray& incr)
{
  size_t l, nl = N_samp.size();
  if (incr.size() != nl) {
    Cerr << "Error: inconsistent array sizes (" << nl << " 2D target, "
	 << incr.size() << " increment) in NonDEnsembleSampling::"
	 << "increment_samples()." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (l=0; l<nl; ++l)
    increment_samples(N_samp[l], incr[l]);
}


inline void NonDEnsembleSampling::
increment_sums(Real* sum_l, const Real* incr, size_t len)
{
  for (size_t i=0; i<len; ++i)
    sum_l[i] += incr[i];
}


inline void NonDEnsembleSampling::reset_relaxation()
{
  if (relaxRecursiveFactor > 0.)
    relaxFactor = relaxRecursiveFactor;
  else if (!relaxFactorSequence.empty()) {
    relaxIndex = 0;    
    relaxFactor = relaxFactorSequence[relaxIndex];
  }
}


inline void NonDEnsembleSampling::advance_relaxation()
{
  if (relaxRecursiveFactor > 0. && relaxFactor < 1.)
    relaxFactor += relaxRecursiveFactor * (1. - relaxFactor);
  else if (!relaxFactorSequence.empty()) {
    ++relaxIndex;
    if (relaxIndex < relaxFactorSequence.length())
      relaxFactor = relaxFactorSequence[relaxIndex];
  }

  // advance factor to unity when encountering a max_iterations constraint
  // --> optimized accuracy/cost targeted by final iteration
  // --> avoids additional complexity in finalize_relaxation() below
  if (mlmfIter == maxIterations && relaxFactor != 1.) {
    Cerr << "Warning: finalizing relaxation factor due to active "
	 << "max_iterations constraint." << std::endl;
    relaxFactor = 1.;
  }
}


/* Ensure MFSolutionData consistency: estVar and solutionVars are from final
   numerical solve, which may be inconsistent with a relaxed sample increment.
   Can either (a) finalize relaxFactor during advancement above or (b) update
   both estVar and solutionVars for final sample state.  We go with (a) above
   for now, but this function can implement (b) by:
   > don't advance sample state to solutionVars (strictly observe user's
     relaxation and max_iterations specifications).  Instead:
   > reset estVar and solutionVars to the actual sample state, relaxing both
     N_H and N_L proportionally.
   This results in undershooting both accuracy and cost.
inline void NonDEnsembleSampling::
finalize_relaxation(MFSolutionData& soln)
{
  // if relaxation is inactive or complete, or sample increments are complete,
  // then soln vars and estvar (from numerical solve) are already consistent
  // with the sample increment state
  if (relaxFactor == 1. || numSamples == 0) return;

  // Else termination prior to completion of relaxation sequence --> solution
  // from numerical solve not fully realized and soln data needs adjustment
  // prior to final post-processing.

  //Real curr_estvar = soln.average_estimator_variance(); // back out ratio?
  const RealVector& soln_vars = soln.solution_variables();
  RealVector delta_N(numGroups);  size_t i;
  for (i=0; i<numGroups; ++i) {
    curr_i = (backfillFailures) ? average(NLevActual[i]) : NLevAlloc[i];
    tgt_i  = soln_vars[i];
    delta_N[i] = (tgt_i > curr_i) ? tgt_i - curr_i : 0.;
  }
  //if (zeros(delta_N)) return;

  // rather than rounded size_t from NLevActual, need either previous real-
  // valued solver soln or previous real-valued delta_N prior to relaxation
  //RealVector delta_N = soln_vars - prev_vars,
  //  relaxed_vars = prev_vars + relaxFactor * delta_N;
  RealVector relaxed_vars(numGroups, false);
  Real multiplier = 1. - relaxFactor;
  for (i=0; i<numGroups; ++i)
    relaxed_vars[i] = soln_vars[i] - multiplier * delta_N[i];
  Real relaxed_estvar = average_estimator_variance(relaxed_vars);

  // override last numerical solve with final (real-valued) state
  soln.solution_variables(relaxed_vars);
  soln.average_estimator_variance(relaxed_estvar);
  // ratio of averages rather that average of ratios
  if (pilotMgmtMode == ONLINE_PILOT || pilotMgmtMode == ONLINE_PILOT_PROJECTION)
    soln.average_estimator_variance_ratio(relaxed_estvar/average(estVarIter0));
}
*/


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


/** For single-level moment calculations without a sample count. */
inline void NonDEnsembleSampling::
uncentered_to_centered(const Real* rm, Real* cm, size_t num_mom)
{
  if (num_mom >= 1) {
    Real& cm1 = cm[0];
    cm1 = rm[0]; // mean
    if (num_mom >= 2) {
      Real& cm2 = cm[1];
      cm2 = rm[1] - cm1 * cm1; // variance
      if (num_mom >= 3) {
	Real& cm3 = cm[2];
	cm3 = rm[2] - cm1 * (3. * cm2 + cm1 * cm1);
	if (num_mom >= 4)
	  cm[3] = rm[3] - cm1 * (4. * cm3 + cm1 * (6. * cm2 + cm1 * cm1));
      }
    }
  }
}


/** Convert full matrix of moments. */
inline void NonDEnsembleSampling::
uncentered_to_centered(const RealMatrix& raw_mom, RealMatrix& cent_mom)
{
  // Note: raw_mom and cent_mom must be num_moments x numFunctions
  size_t qoi, num_mom = raw_mom.numRows();
  if (cent_mom.numRows() != num_mom)
    cent_mom.shapeUninitialized(num_mom, numFunctions);

  for (qoi=0; qoi<numFunctions; ++qoi)
    uncentered_to_centered(raw_mom[qoi], cent_mom[qoi], num_mom);
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
    Cerr << "\nWarning: central to standard conversion failed due to non-"
	 << "positive\n         variance.  Retaining central moments.\n";
    sm2 = 0.; sm3 = cm3; sm4 = cm4; // or assign NaN to sm{3,4}
  }
}


inline void NonDEnsembleSampling::
centered_to_standard(const Real* cm, Real* sm, size_t num_mom)
{
  if (num_mom >= 1) {
    sm[0] = cm[0]; // mean
    if (num_mom >= 2) {
      Real& sm2 = sm[1];  const Real& cm2 = cm[1];
      if (cm2 > 0.) {
	sm2 = std::sqrt(cm2); // std deviation
	if (num_mom >= 3) {
	  sm[2] = cm[2] / (cm2 * sm2); // skewness
	  if (num_mom >= 4)
	    sm[3] = cm[3] / (cm2 * cm2) - 3.; // excess kurtosis
	}
      }
      else {
	Cerr << "\nWarning: central to standard conversion failed due to non-"
	     << "positive\n         variance.  Retaining central moments.\n";
	sm2 = 0.; sm[2] = cm[2]; sm[3] = cm[3]; // or assign NaN to sm{3,4}
      }
    }
  }
}


/** Convert full matrix of moments. */
inline void NonDEnsembleSampling::
centered_to_standard(const RealMatrix& cent_mom, RealMatrix& std_mom)
{
  // Note: cent_mom and std_mom must be num_moments x numFunctions
  size_t qoi, num_mom = cent_mom.numRows();
  if (std_mom.numRows() != num_mom)
    std_mom.shapeUninitialized(num_mom, numFunctions);

  for (qoi=0; qoi<numFunctions; ++qoi)
    centered_to_standard(cent_mom[qoi], std_mom[qoi], num_mom);
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

} // namespace Dakota

#endif
