/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Implementation code for NonDSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "data_types.h"
#include "system_defs.h"
#include "DakotaModel.H"
#include "DakotaResponse.H"
#include "NonDSampling.H"
#include "ProblemDescDB.H"
#include "SensAnalysisGlobal.H"
#ifdef DAKOTA_DDACE
#include "Distribution.h"
#elif defined(DAKOTA_UTILIB)
#include <utilib/seconds.h>
#endif
#include "pecos_data_types.hpp"
#include "pecos_stat_util.hpp"
#include <algorithm>

#include <boost/math/special_functions/fpclassify.hpp>

static const char rcsId[]="@(#) $Id: NonDSampling.C 7036 2010-10-22 23:20:24Z mseldre $";


namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called and
    probDescDB can be queried for settings from the method specification. */
NonDSampling::NonDSampling(Model& model): NonD(model),
  seedSpec(probDescDB.get_int("method.random_seed")), randomSeed(seedSpec),
  samplesSpec(probDescDB.get_int("method.samples")), numSamples(samplesSpec),
  rngName(probDescDB.get_string("method.random_number_generator")),
  sampleType(probDescDB.get_string("method.sample_type")),
  statsFlag(true), allDataFlag(false), sampleRanksMode(IGNORE_RANKS),
  varyPattern(!probDescDB.get_bool("method.fixed_seed")), numLHSRuns(0)
{
  if (numEpistemicUncVars && totalLevelRequests) {
    Cerr << "\nError: nond_sampling does not support level requests for "
	 << "analyses containing epistemic uncertainties." << std::endl;
    abort_handler(-1);
  }

  // Since the sampleType is shared with other iterators for other purposes, its
  // default in DataMethod.C is the NULL string.  Explicitly enforce the LHS
  // default here.
  if (sampleType.empty())
    sampleType = "lhs";

  // initialize finalStatistics using the default statistics set
  initialize_final_statistics();

  // update concurrency
  if (numSamples) // samples is now optional (default = 0)
    maxConcurrency *= numSamples;
}


/** This alternate constructor is used for generation and evaluation
    of on-the-fly sample sets. */
NonDSampling::
NonDSampling(NoDBBaseConstructor, Model& model, int samples, int seed,
	     const String& rng):
  NonD(NoDBBaseConstructor(), model), seedSpec(seed), randomSeed(seed),
  samplesSpec(samples), numSamples(samples), rngName(rng), sampleType("lhs"),
  statsFlag(false), allDataFlag(true), sampleRanksMode(IGNORE_RANKS),
  varyPattern(false), numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // not used but included for completeness
  if (numSamples) // samples is now optional (default = 0)
    maxConcurrency *= numSamples;
}


/** This alternate constructor is used by ConcurrentStrategy for
    generation of uniform, uncorrelated sample sets. */
NonDSampling::
NonDSampling(NoDBBaseConstructor, int samples, int seed, const String& rng,
	     const RealVector& lower_bnds, const RealVector& upper_bnds):
  NonD(NoDBBaseConstructor(), lower_bnds, upper_bnds), seedSpec(seed),
  randomSeed(seed), samplesSpec(samples), numSamples(samples), rngName(rng),
  sampleType("lhs"), statsFlag(false), allDataFlag(true),
  sampleRanksMode(IGNORE_RANKS), varyPattern(false), numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // not used but included for completeness
  if (numSamples) // samples is now optional (default = 0)
    maxConcurrency *= numSamples;
}


NonDSampling::~NonDSampling()
{ }


/** This version of get_parameter_sets() extracts data from the
    user-defined model in any of the four sampling modes. */
void NonDSampling::get_parameter_sets(Model& model)
{
  initialize_lhs(true);

  short model_view = model.current_variables().view().first;
  if (samplingVarsMode == UNCERTAIN_UNIFORM ||
      samplingVarsMode == ACTIVE_UNIFORM    ||
      samplingVarsMode == ALL_UNIFORM) {
    if ( samplingVarsMode == ACTIVE_UNIFORM ||
	 ( samplingVarsMode == ALL_UNIFORM && 
	   ( model_view == MERGED_ALL || model_view == MIXED_ALL ) ) ||
	 ( samplingVarsMode == UNCERTAIN_UNIFORM && 
	   ( model_view == MERGED_DISTINCT_UNCERTAIN           ||
	     model_view == MERGED_DISTINCT_ALEATORY_UNCERTAIN  ||
	     model_view == MERGED_DISTINCT_EPISTEMIC_UNCERTAIN ||
	     model_view == MIXED_DISTINCT_UNCERTAIN            ||
	     model_view == MIXED_DISTINCT_ALEATORY_UNCERTAIN   ||
	     model_view == MIXED_DISTINCT_EPISTEMIC_UNCERTAIN ) ) ) {
      // sample uniformly from ACTIVE lower/upper bounds (regardless of model
      // view), from UNCERTAIN lower/upper bounds (with model in DISTINCT view),
      // or from ALL lower/upper bounds (with model in ALL view).
      // TO DO: verify loss of sampleRanks control is OK
      // TO DO: add support for uniform discrete design/uncertain/state
      //        (must manage ordering: discrete int/real among cdv/cuv/csv)
      lhsDriver.generate_uniform_samples(model.continuous_lower_bounds(),
					 model.continuous_upper_bounds(),
					 numSamples, allSamples);
    }
    else if (samplingVarsMode == ALL_UNIFORM) {
      // sample uniformly from ALL lower/upper bounds with model using
      // a DISTINCT view
      // TO DO: verify loss of sampleRanks control is OK
      // TO DO: add support for uniform discrete design/uncertain/state
      //        (must manage ordering: discrete int/real among cdv/cuv/csv)
      lhsDriver.generate_uniform_samples(model.all_continuous_lower_bounds(),
					 model.all_continuous_upper_bounds(),
					 numSamples, allSamples);
    }
    else if (samplingVarsMode == UNCERTAIN_UNIFORM) {
      // sample uniformly from UNCERTAIN lower/upper bounds with model
      // using a view other than
      // {MERGED,MIXED}_DISTINCT_{,_ALEATORY,_EPISTEMIC}UNCERTAIN
      RealVector uncertain_l_bnds, uncertain_u_bnds;
      size_t num_cuv = numContAleatUncVars + numContEpistUncVars;
      if (model_view == MERGED_ALL || model_view == MIXED_ALL) {
	const RealVector& c_l_bnds = model.continuous_lower_bounds();
	const RealVector& c_u_bnds = model.continuous_upper_bounds();
	uncertain_l_bnds = RealVector(Teuchos::View,
	  const_cast<Real*>(&c_l_bnds[numContDesVars]), num_cuv);
	uncertain_u_bnds = RealVector(Teuchos::View,
	  const_cast<Real*>(&c_u_bnds[numContDesVars]), num_cuv);
      }
      else { // MERGED/MIXED_DISTINCT_DESIGN or MERGED/MIXED_DISTINCT_STATE
	// This case should not occur; if it does, numContDesVars may not be set
	const RealVector& all_c_l_bnds = model.all_continuous_lower_bounds();
	const RealVector& all_c_u_bnds = model.all_continuous_upper_bounds();
	uncertain_l_bnds = RealVector(Teuchos::View,
	  const_cast<Real*>(&all_c_l_bnds[numContDesVars]), num_cuv);
	uncertain_u_bnds = RealVector(Teuchos::View,
	  const_cast<Real*>(&all_c_u_bnds[numContDesVars]), num_cuv);
      }
      // TO DO: verify loss of sampleRanks control is OK
      // TO DO: add support for uniform discrete design/uncertain/state
      //        (must manage ordering: discrete int/real among cdv/cuv/csv)
      lhsDriver.generate_uniform_samples(uncertain_l_bnds, uncertain_u_bnds,
					 numSamples, allSamples);
    }
  }
  else { // UNCERTAIN || ACTIVE || ALL

    // extract design and state bounds
    RealVector  cdv_l_bnds,   cdv_u_bnds,   csv_l_bnds,   csv_u_bnds;
    IntVector ddriv_l_bnds, ddriv_u_bnds, dsriv_l_bnds, dsriv_u_bnds;
    size_t num_ddsiv = model.discrete_design_set_int_values().size(),
      num_ddsrv = model.discrete_design_set_real_values().size(),
      num_ddriv = numDiscIntDesVars - num_ddsiv,
      num_cuv   = numContAleatUncVars + numContEpistUncVars,
      num_diuv  = numDiscIntAleatUncVars + numDiscIntEpistUncVars,
      num_dssiv = model.discrete_state_set_int_values().size(),
      num_dssrv = model.discrete_state_set_real_values().size(),
      num_dsriv = numDiscIntStateVars - num_dssiv;
    if ( samplingVarsMode == ACTIVE || ( samplingVarsMode == ALL && 
	 ( model_view == MERGED_ALL || model_view == MIXED_ALL ) ) ) {
      const RealVector& c_l_bnds = model.continuous_lower_bounds();
      const RealVector& c_u_bnds = model.continuous_upper_bounds();
      cdv_l_bnds = RealVector(Teuchos::View, c_l_bnds.values(), numContDesVars);
      cdv_u_bnds = RealVector(Teuchos::View, c_u_bnds.values(), numContDesVars);
      csv_l_bnds = RealVector(Teuchos::View,
	const_cast<Real*>(&c_l_bnds[numContDesVars+num_cuv]), numContStateVars);
      csv_u_bnds  = RealVector(Teuchos::View,
	const_cast<Real*>(&c_u_bnds[numContDesVars+num_cuv]), numContStateVars);
      const IntVector& di_l_bnds = model.discrete_int_lower_bounds();
      const IntVector& di_u_bnds = model.discrete_int_upper_bounds();
      ddriv_l_bnds = IntVector(Teuchos::View, di_l_bnds.values(), num_ddriv);
      ddriv_u_bnds = IntVector(Teuchos::View, di_u_bnds.values(), num_ddriv);
      dsriv_l_bnds = IntVector(Teuchos::View,
	const_cast<int*>(&di_l_bnds[numDiscIntDesVars+num_diuv]), num_dsriv);
      dsriv_u_bnds = IntVector(Teuchos::View,
	const_cast<int*>(&di_u_bnds[numDiscIntDesVars+num_diuv]), num_dsriv);
    }
    else if (samplingVarsMode == ALL) {
      const RealVector& all_c_l_bnds = model.all_continuous_lower_bounds();
      const RealVector& all_c_u_bnds = model.all_continuous_upper_bounds();
      cdv_l_bnds = RealVector(Teuchos::View, all_c_l_bnds.values(),
	numContDesVars);
      cdv_u_bnds = RealVector(Teuchos::View, all_c_u_bnds.values(),
	numContDesVars);
      csv_l_bnds = RealVector(Teuchos::View,
	const_cast<Real*>(&all_c_l_bnds[numContDesVars+num_cuv]),
	numContStateVars);
      csv_u_bnds  = RealVector(Teuchos::View,
	const_cast<Real*>(&all_c_u_bnds[numContDesVars+num_cuv]),
	numContStateVars);
      const IntVector& all_di_l_bnds = model.all_discrete_int_lower_bounds();
      const IntVector& all_di_u_bnds = model.all_discrete_int_upper_bounds();
      ddriv_l_bnds = IntVector(Teuchos::View, all_di_l_bnds.values(),num_ddriv);
      ddriv_u_bnds = IntVector(Teuchos::View, all_di_u_bnds.values(),num_ddriv);
      dsriv_l_bnds = IntVector(Teuchos::View,
	const_cast<int*>(&all_di_l_bnds[numDiscIntDesVars+num_diuv]),num_dsriv);
      dsriv_u_bnds = IntVector(Teuchos::View,
	const_cast<int*>(&all_di_u_bnds[numDiscIntDesVars+num_diuv]),num_dsriv);
    }

    // Call LHS to generate the specified samples within the specified
    // distributions.  Moved from constructors to allow publication of changes
    // after construction (e.g., sampling_reset() or change in UQ samples as OUU
    // progresses).  Note that this requires LHS to be more stable than before
    // (i.e., needs to be reentrant so that the same set of samples is generated
    // if nothing changes between quantify_uncertainty calls -->> important for
    // finite difference accuracy in OUU).
    lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
      ddriv_u_bnds, model.discrete_design_set_int_values(),
      model.discrete_design_set_real_values(), csv_l_bnds, csv_u_bnds,
      dsriv_l_bnds, dsriv_u_bnds, model.discrete_state_set_int_values(),
      model.discrete_state_set_real_values(), model.distribution_parameters(),
      numSamples, allSamples, sampleRanks);
  }
}


/** This version of get_parameter_sets() does not extract data from the
    user-defined model, but instead relies on the incoming bounded region
    definition.  It only support a UNIFORM sampling mode, where the
    distinction of ACTIVE_UNIFORM vs. ALL_UNIFORM is handled elsewhere. */
void NonDSampling::
get_parameter_sets(const RealVector& lower_bnds,
		   const RealVector& upper_bnds)
{
  initialize_lhs(true);
  lhsDriver.generate_uniform_samples(lower_bnds, upper_bnds,
				     numSamples, allSamples);
}


void NonDSampling::
update_model_from_sample(Model& model, const Real* sample_vars)
{
  size_t i, cv_start, num_cv, div_start, num_div, drv_start, num_drv;
  view_counts(model, cv_start, num_cv, div_start, num_div, drv_start, num_drv);

  // sampled continuous vars
  for (i=0; i<num_cv; ++i)
    model.all_continuous_variable(sample_vars[i], cv_start+i);
  // sampled discrete int vars
  size_t offset = num_cv;
  for (i=0; i<num_div; ++i)
    model.all_discrete_int_variable((int)sample_vars[i+offset], div_start+i);
  // sampled discrete real vars
  offset += num_div;
  for (i=0; i<num_drv; ++i)
    model.all_discrete_real_variable(sample_vars[i+offset], drv_start+i);
}


void NonDSampling::
view_counts(const Model& model, size_t& cv_start, size_t& num_cv,
	    size_t& div_start, size_t& num_div,
	    size_t& drv_start, size_t& num_drv) const
{
  cv_start = num_cv = div_start = num_div = drv_start = num_drv = 0;
  switch (samplingVarsMode) {
  case UNCERTAIN: {
    const Variables& vars = model.current_variables();
    short active_view = vars.view().first;
    bool all_vars = (active_view == MERGED_ALL || active_view == MIXED_ALL);
    if (all_vars) { // UNCERTAIN is a subset of ACTIVE
      cv_start  = numContDesVars;
      num_cv    = numContAleatUncVars + numContEpistUncVars;
      div_start = numDiscIntDesVars;
      num_div   = numDiscIntAleatUncVars + numDiscIntEpistUncVars;
      drv_start = numDiscRealDesVars;
      num_drv   = numDiscRealAleatUncVars + numDiscRealEpistUncVars;
    }
    else { // UNCERTAIN is the same as ACTIVE
      cv_start  = vars.cv_start();  num_cv  = vars.cv();
      div_start = vars.div_start(); num_div = vars.div();
      drv_start = vars.drv_start(); num_drv = vars.drv();
    }
    break;
  }
  case UNCERTAIN_UNIFORM: {
    const Variables& vars = model.current_variables();
    short active_view = vars.view().first;
    bool all_vars = (active_view == MERGED_ALL || active_view == MIXED_ALL);
    if (all_vars) { // UNCERTAIN is a subset of ACTIVE
      cv_start = numContDesVars;
      num_cv   = numContAleatUncVars + numContEpistUncVars;
    }
    else { // UNCERTAIN is the same as ACTIVE
      cv_start = vars.cv_start();
      num_cv   = vars.cv();
    }
    // UNIFORM views do not currently support discrete
    break;
  }
  case ACTIVE: {
    const Variables& vars = model.current_variables();
    cv_start  = vars.cv_start();  num_cv  = vars.cv();
    div_start = vars.div_start(); num_div = vars.div();
    drv_start = vars.drv_start(); num_drv = vars.drv();
    break;
  }
  case ACTIVE_UNIFORM: {
    const Variables& vars = model.current_variables();
    cv_start = vars.cv_start(); num_cv = vars.cv();
    // UNIFORM views do not currently support discrete
    break;
  }
  case ALL:
    num_cv = model.acv(); num_div = model.adiv(); num_drv = model.adrv();
    break;
  case ALL_UNIFORM:
    num_cv = model.acv();
    // UNIFORM views do not currently support discrete
    break;
  }
}


void NonDSampling::initialize_lhs(bool write_message)
{
  // keep track of number of LHS executions for this object
  numLHSRuns++;

  // Set seed value for input to LHS's random number generator.  Emulate DDACE
  // behavior in which a user-specified seed gives you repeatable behavior but
  // no specification gives you random behavior.  A system clock is used to
  // randomize in the no user specification case.  For cases where
  // get_parameter_sets() may be called multiple times for the same sampling
  // iterator (e.g., SBO), support a deterministic sequence of seed values.
  // This renders the study repeatable but the sampling pattern varies from
  // one run to the next.
  if (numLHSRuns == 1) { // set initial seed
    lhsDriver.rng(rngName);
    if (!seedSpec) { // no user specification: nonrepeatable behavior
      // Generate initial seed from a system clock.  NOTE: the system clock
      // should not used for multiple LHS calls since (1) clock granularity can
      // be too coarse (can repeat on subsequent runs for inexpensive test fns)
      // and (2) seed progression can be highly structured, which could induce
      // correlation between sample sets.  Instead, the clock-generated case
      // varies the seed below using the same approach as the user-specified
      // case.  This has the additional benefit that a random run can be
      // recreated by specifying the clock-generated seed in the input file.
      randomSeed = 1;
#ifdef DAKOTA_DDACE
      randomSeed += DistributionBase::timeSeed(); // microsecs, time of day
#elif defined(DAKOTA_UTILIB)
      randomSeed += (int)CurrentTime();           // secs, time of day
#else
      randomSeed += (int)clock();                 // clock ticks, exec time
#endif
    }
    lhsDriver.seed(randomSeed);
  }
  else if (varyPattern) // define sequence of seed values for numLHSRuns > 1
    lhsDriver.advance_seed_sequence();
  else // fixed_seed
    lhsDriver.seed(randomSeed); // reset original/machine-generated seed

  // Needed a way to turn this off when LHS sampling is being used in
  // NonDAdaptImpSampling because it gets written a _LOT_
  if (write_message) {
    Cout << "\nNonD " << sampleType << " Samples = " << numSamples;
    if (numLHSRuns == 1 || !varyPattern) {
      if (seedSpec) Cout << " Seed (user-specified) = ";
      else          Cout << " Seed (system-generated) = ";
      Cout << randomSeed << '\n';
    }
    else if (rngName == "rnum2") {
      if (seedSpec) Cout << " Seed (sequence from user-specified) = ";
      else          Cout << " Seed (sequence from system-generated) = ";
      Cout << lhsDriver.seed() << '\n';
    }
    else // default Boost Mersenne twister
      Cout << " Seed not reset from previous LHS execution\n";
  }

  lhsDriver.initialize(sampleType, sampleRanksMode, !subIteratorFlag);
}


void NonDSampling::
compute_statistics(const RealMatrix&    vars_samples,
		   const ResponseArray& resp_samples)
{
  if (numEpistemicUncVars) // Epistemic/mixed
    compute_intervals(resp_samples); // compute min/max response intervals
  else { // Aleatory
    // compute means and std deviations with confidence intervals
    compute_moments(resp_samples);
    // compute CDF/CCDF mappings of z to p/beta and p/beta to z
    if (totalLevelRequests)
      compute_distribution_mappings(resp_samples);
  }
  if (!subIteratorFlag)
    nonDSampCorr.compute_correlations(vars_samples, resp_samples);
  if (!finalStatistics.is_null())
    update_final_statistics();
}


void NonDSampling::compute_intervals(const ResponseArray& samples)
{
  // For the samples array, calculate min/max response intervals

  using boost::math::isfinite;
  size_t i, j, num_obs = samples.size(), num_samp;
  const StringArray& resp_labels = iteratedModel.response_labels();

  if (minValues.empty()) minValues.resize(numFunctions);
  if (maxValues.empty()) maxValues.resize(numFunctions);
  for (i=0; i<numFunctions; ++i) {
    num_samp = 0;
    Real min = DBL_MAX, max = -DBL_MAX;
    for (j=0; j<num_obs; j++) {
      const Real& sample = samples[j].function_value(i);
      if (isfinite(sample)) { // neither NaN nor +/-Inf
	if (sample < min) min = sample;
	if (sample > max) max = sample;
	++num_samp;
      }
    }
    minValues[i] = min;
    maxValues[i] = max;
    if (num_samp != num_obs)
      Cerr << "Warning: sampling statistics for " << resp_labels[i] << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";
  }
}


void NonDSampling::compute_moments(const ResponseArray& samples)
{
  // For the samples array, calculate means and standard deviations
  // with confidence intervals

  using boost::math::isfinite;
  size_t i, j, num_obs = samples.size(), num_samp;
  Real sum, var, skew, kurt;
  const StringArray& resp_labels = iteratedModel.response_labels();

  if (meanStats.empty())           meanStats.resize(numFunctions);
  if (stdDevStats.empty())         stdDevStats.resize(numFunctions);
  if (skewnessStats.empty())       skewnessStats.resize(numFunctions);
  if (kurtosisStats.empty())       kurtosisStats.resize(numFunctions);
  if (mean95CIDeltas.empty())      mean95CIDeltas.resize(numFunctions);
  if (stdDev95CILowerBnds.empty()) stdDev95CILowerBnds.resize(numFunctions);
  if (stdDev95CIUpperBnds.empty()) stdDev95CIUpperBnds.resize(numFunctions);

  for (i=0; i<numFunctions; ++i) {

    num_samp  = 0;
    sum = var = skew = kurt = 0.;
    // means
    for (j=0; j<num_obs; j++) {
      const Real& sample = samples[j].function_value(i);
      if (isfinite(sample)) { // neither NaN nor +/-Inf
	sum += sample;
	++num_samp;
      }
    }

    if (num_samp != num_obs)
      Cerr << "Warning: sampling statistics for " << resp_labels[i] << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";
    if (!num_samp) {
      Cerr << "Error: Number of samples for " << resp_labels[i]
	   << " must be nonzero for moment calculation in NonDSampling::"
	   << "compute_statistics()." << std::endl;
      abort_handler(-1);
    }

    meanStats[i] = sum/((Real)num_samp);

    // accumulate variance, skewness, and kurtosis
    Real centered_fn, pow_fn;
    for (j=0; j<num_obs; ++j) {
      const Real& sample = samples[j].function_value(i);
      if (isfinite(sample)) { // neither NaN nor +/-Inf
	pow_fn  = centered_fn = sample - meanStats[i];
	pow_fn *= centered_fn; var  += pow_fn;
	pow_fn *= centered_fn; skew += pow_fn;
	pow_fn *= centered_fn; kurt += pow_fn;
      }
    }
    // sample std deviation
    stdDevStats[i] = (num_samp > 1) ? std::sqrt(var/(Real)(num_samp-1)) : 0.;
    // sample skewness
    skewnessStats[i] = (num_samp > 3) ? 
      skew/(Real)num_samp/std::pow(var/(Real)num_samp,1.5) : 0.;
    // population skewness 
    skewnessStats[i]
      *= std::sqrt((Real)(num_samp*(num_samp-1)))/(Real)(num_samp-2);
    // sample kurtosis
    kurtosisStats[i] = (num_samp > 4) ?
      (Real)((num_samp+1)*num_samp*(num_samp-1))*kurt/
      (Real)((num_samp-2)*(num_samp-3)*var*var) : 0.;
    // population kurtosis
    kurtosisStats[i] -=
      3.*std::pow((Real)(num_samp-1),2)/(Real)((num_samp-2)*(num_samp-3));

    if (num_samp > 1) {
      // 95% confidence intervals (2-sided interval, not 1-sided limit)
      Real dof = num_samp - 1, s = stdDevStats[i];
//#ifdef HAVE_BOOST
      // mean: the better formula does not assume known variance but requires
      // a function for the Student's t-distr. with (num_samp-1) degrees of
      // freedom (Haldar & Mahadevan, p. 127).
      Pecos::students_t_dist t_dist(dof);
      mean95CIDeltas[i]
	= s*(bmth::quantile(t_dist,0.975))/std::sqrt((Real)num_samp);
      // std dev: chi-square distribution with (num_samp-1) degrees of freedom
      // (Haldar & Mahadevan, p. 132).
      Pecos::chi_squared_dist chisq(dof);
      stdDev95CILowerBnds[i] = s*std::sqrt(dof/bmth::quantile(chisq,0.975));
      stdDev95CIUpperBnds[i] = s*std::sqrt(dof/bmth::quantile(chisq,0.025));
/*
#elif HAVE_GSL
      // mean: the better formula does not assume known variance but requires
      // a function for the Student's t-distr. with (num_samp-1) degrees of
      // freedom (Haldar & Mahadevan, p. 127).
      mean95CIDeltas[i]
	= s*gsl_cdf_tdist_Pinv(0.975,dof)/std::sqrt((Real)num_samp);
      // std dev: chi-square distribution with (num_samp-1) degrees of freedom
      // (Haldar & Mahadevan, p. 132).
      stdDev95CILowerBnds[i] = s*std::sqrt(dof/gsl_cdf_chisq_Pinv(0.975, dof));
      stdDev95CIUpperBnds[i] = s*std::sqrt(dof/gsl_cdf_chisq_Pinv(0.025, dof));
#else
      // mean: k_(alpha/2) = Phi^(-1)(0.975) = 1.96 (Haldar & Mahadevan,
      // p. 123).  This simple formula assumes a known variance, which
      // requires a sample of sufficient size (i.e., greater than 10).
      mean95CIDeltas[i] = 1.96*stdDevStats[i]/std::sqrt((Real)num_samp);
#endif // HAVE_BOOST
*/
    }
    else {
      mean95CIDeltas = 0.;
      stdDev95CILowerBnds = stdDev95CIUpperBnds = stdDevStats;
    }
  }
}


void NonDSampling::compute_distribution_mappings(const ResponseArray& samples)
{
  // Size the output arrays here instead of in the ctor in order to support
  // alternate sampling ctors.
  initialize_distribution_mappings();

  // For the samples array, calculate the following statistics:
  // > CDF/CCDF mappings of response levels to probability/reliability levels
  // > CDF/CCDF mappings of probability/reliability levels to response levels
  using boost::math::isfinite;
  size_t i, j, k, num_obs = samples.size(), num_samp;
  const StringArray& resp_labels = iteratedModel.response_labels();
  RealArray sorted_samples; // STL-based array for sorting

  for (i=0; i<numFunctions; ++i) {

    // CDF/CCDF mappings: z -> p/beta/beta* and p/beta/beta* -> z
    size_t rl_len = requestedRespLevels[i].length(),
           pl_len = requestedProbLevels[i].length(),
           bl_len = requestedRelLevels[i].length(),
           gl_len = requestedGenRelLevels[i].length();

    num_samp = 0;
    if (rl_len) {
      switch (respLevelTarget) {
      case PROBABILITIES: case GEN_RELIABILITIES: {
	// z -> p/beta* (based on binning)
	SizetArray less_eq_z(rl_len, 0);
	for (j=0; j<num_obs; ++j) {
	  const Real& sample = samples[j].function_value(i);
	  if (isfinite(sample)) {
	    ++num_samp;
	    const RealVector& req_rl_i = requestedRespLevels[i];
	    for (k=0; k<rl_len; ++k)
	      if (sample < req_rl_i[k])
		less_eq_z[k] += 1;
	  }
	}
	for (j=0; j<rl_len; ++j) {
	  Real cdf_prob = (Real)less_eq_z[j]/(Real)num_samp;
	  Real computed_prob = (cdfFlag) ? cdf_prob : 1. - cdf_prob;
	  if (respLevelTarget == PROBABILITIES)
	    computedProbLevels[i][j] = computed_prob;
	  else
	    computedGenRelLevels[i][j] = -Pecos::Phi_inverse(computed_prob);
	}
	break;
      }
      case RELIABILITIES: // z -> beta (based on moment projection)
	for (j=0; j<rl_len; j++) {
	  const Real& z = requestedRespLevels[i][j];
	  if (stdDevStats[i] > 1.e-25) {
	    Real ratio = (meanStats[i] - z)/stdDevStats[i];
	    computedRelLevels[i][j] = (cdfFlag) ? ratio : -ratio;
	  }
	  else
	    computedRelLevels[i][j] = ( (cdfFlag && meanStats[i] <= z) ||
	      (!cdfFlag && meanStats[i] > z) ) ? -1.e50 : 1.e50;
	}
	break;
      }
    }
    if (pl_len || gl_len) { // sort samples array for p/beta* -> z mappings
      // create array of finite samples for sorting
      if (rl_len && respLevelTarget != RELIABILITIES) { // num_samp available
	sorted_samples.resize(num_samp);
	size_t cntr = 0;
	for (j=0; j<num_obs; j++) {
	  const Real& sample = samples[j].function_value(i);
	  if (isfinite(sample))
	    sorted_samples[cntr++] = sample;
	}
      }
      else { // num_samp not yet available
	sorted_samples.clear();
	sorted_samples.reserve(num_obs);
	for (j=0; j<num_obs; j++) {
	  const Real& sample = samples[j].function_value(i);
	  if (isfinite(sample)) {
	    ++num_samp;
	    sorted_samples.push_back(sample);
	  }
	}
      }
      // sort in ascending order
      std::sort(sorted_samples.begin(),sorted_samples.end());
    }
    for (j=0; j<pl_len+gl_len; j++) { // p/beta* -> z
      Real p = (j<pl_len) ? requestedProbLevels[i][j] :
	Pecos::Phi(-requestedGenRelLevels[i][j-pl_len]);
      // since each sample has 1/N probability, a probability level can be
      // directly converted to an index within a sorted array (index =~ p * N)
      Real cdf_p_x_obs = (cdfFlag) ? p*(Real)num_samp : (1.-p)*(Real)num_samp;
      // convert to an int and round down using std::floor().  Apply a small
      // numerical adjustment so that probabilities on the boundaries
      // (common with round probabilities and factor of 10 samples)
      // are consistently rounded down (consistent with CDF p(g<=z)).
      Real order = (cdf_p_x_obs > .9)
	         ? std::pow(10., ceil(std::log10(cdf_p_x_obs))) : 0.;
      int index = (int)std::floor(cdf_p_x_obs - order*DBL_EPSILON);
      // clip at array ends due to possible roundoff effects
      if (index < 0)         index = 0;
      if (index >= num_samp) index = num_samp - 1;
      if (j<pl_len)
	computedRespLevels[i][j] = sorted_samples[index];
      else
	computedRespLevels[i][j+bl_len] = sorted_samples[index];
    }
    for (j=0; j<bl_len; j++) { // beta -> z
      const Real& beta = requestedRelLevels[i][j];
      computedRespLevels[i][j+pl_len] = (cdfFlag) ?
	meanStats[i] - beta * stdDevStats[i] :
	meanStats[i] + beta * stdDevStats[i];
    }
  }
}



void NonDSampling::update_final_statistics()
{
  //if (finalStatistics.is_null())
  //  initialize_final_statistics();

  size_t i, j, cntr = 0;
  if (numEpistemicUncVars) {
    for (i=0; i<numFunctions; ++i) {
      finalStatistics.function_value(minValues[i], cntr++);
      finalStatistics.function_value(maxValues[i], cntr++);
    }
  }
  else {
    for (i=0; i<numFunctions; ++i) {
      // final stats from compute_moments()
      if (!meanStats.empty() && !stdDevStats.empty()) {
	finalStatistics.function_value(meanStats[i],   cntr++);
	finalStatistics.function_value(stdDevStats[i], cntr++);
      }
      else
	cntr += 2;
      // final stats from compute_distribution_mappings()
      size_t rl_len = requestedRespLevels[i].length();
      for (j=0; j<rl_len; j++)
	switch (respLevelTarget) {
	case PROBABILITIES:
	  finalStatistics.function_value(computedProbLevels[i][j], cntr++);
	  break;
	case RELIABILITIES:
	  finalStatistics.function_value(computedRelLevels[i][j], cntr++);
	  break;
	case GEN_RELIABILITIES:
	  finalStatistics.function_value(computedGenRelLevels[i][j], cntr++);
	  break;
	}
      size_t pl_bl_gl_len = requestedProbLevels[i].length() +
	requestedRelLevels[i].length() + requestedGenRelLevels[i].length();
      for (j=0; j<pl_bl_gl_len; j++)
	finalStatistics.function_value(computedRespLevels[i][j], cntr++);
    }
  }
}


void NonDSampling::print_statistics(std::ostream& s) const
{
  if (numEpistemicUncVars) // output only min & max values in the epistemic case
    print_intervals(s);
  else {
    print_moments(s);
    if (totalLevelRequests)
      print_distribution_mappings(s);
  }
  if (!subIteratorFlag) {
    StringMultiArrayConstView
      acv_labels  = iteratedModel.all_continuous_variable_labels(),
      adiv_labels = iteratedModel.all_discrete_int_variable_labels(),
      adrv_labels = iteratedModel.all_discrete_real_variable_labels();
    size_t cv_start, num_cv, div_start, num_div, drv_start, num_drv;
    view_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
		drv_start, num_drv);
    StringMultiArrayConstView
      cv_labels  =
      acv_labels[boost::indices[idx_range(cv_start, cv_start+num_cv)]],
      div_labels =
      adiv_labels[boost::indices[idx_range(div_start, div_start+num_div)]],
      drv_labels =
      adrv_labels[boost::indices[idx_range(drv_start, drv_start+num_drv)]];
    nonDSampCorr.print_correlations(s, cv_labels, div_labels, drv_labels,
      iteratedModel.response_labels());
  }
}


void NonDSampling::print_intervals(std::ostream& s) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();

  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision)
    << "\nMin and Max values for each response function:\n";
  for (size_t i=0; i<numFunctions; ++i)
    s << resp_labels[i] << ":  Min = " << minValues[i]
      << "  Max = " << maxValues[i] << '\n';
}


void NonDSampling::print_moments(std::ostream& s) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();

  s.setf(std::ios::scientific);
  s << std::setprecision(write_precision);

  size_t i, j, width = write_precision+7;
 
  s << "\nMoment-based statistics for each response function:\n"
    << std::setw(width+15) << "Mean"     << std::setw(width+1) << "Std Dev"
    << std::setw(width+1)  << "Skewness" << std::setw(width+2) << "Kurtosis\n";
  //<< std::setw(width+2)  << "Coeff of Var\n";
  for (i=0; i<numFunctions; ++i)
    s << std::setw(14) << resp_labels[i]
      << ' ' << std::setw(width) << meanStats[i]
      << ' ' << std::setw(width) << stdDevStats[i]
      << ' ' << std::setw(width) << skewnessStats[i]
      << ' ' << std::setw(width) << kurtosisStats[i] << '\n';

  if (numSamples > 1) {
    // output 95% confidence intervals as (,) interval
    s << "\n95% confidence intervals for each response function:\n"
      << std::setw(width+15) << "LowerCI_Mean" << std::setw(width+1)
      << "UpperCI_Mean" << std::setw(width+1)  << "LowerCI_StdDev" 
      << std::setw(width+2) << "UpperCI_StdDev\n";
    for (i=0; i<numFunctions; ++i)
      s << std::setw(14) << resp_labels[i]
	<< ' ' << std::setw(width) << meanStats[i] - mean95CIDeltas[i]
	<< ' ' << std::setw(width) << meanStats[i] + mean95CIDeltas[i]
	<< ' ' << std::setw(width) << stdDev95CILowerBnds[i]
	<< ' ' << std::setw(width) << stdDev95CIUpperBnds[i] << '\n';
  }
}

} // namespace Dakota

