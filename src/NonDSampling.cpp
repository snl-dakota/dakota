/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDSampling
//- Description: Implementation code for NonDSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDSampling.hpp"
#include "ProblemDescDB.hpp"
#include "SensAnalysisGlobal.hpp"
#include "pecos_data_types.hpp"
#include "pecos_stat_util.hpp"
#include <algorithm>

#include <boost/math/special_functions/beta.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
using boost::math::isfinite;

static const char rcsId[]="@(#) $Id: NonDSampling.cpp 7036 2010-10-22 23:20:24Z mseldre $";


namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called and
    probDescDB can be queried for settings from the method specification. */
NonDSampling::NonDSampling(ProblemDescDB& problem_db, Model& model):
  NonD(problem_db, model), seedSpec(probDescDB.get_int("method.random_seed")),
  randomSeed(seedSpec), samplesSpec(probDescDB.get_int("method.samples")),
  samplesRef(samplesSpec), numSamples(samplesSpec),
  rngName(probDescDB.get_string("method.random_number_generator")),
  sampleType(probDescDB.get_ushort("method.sample_type")), samplesIncrement(0),
  statsFlag(true), allDataFlag(false), samplingVarsMode(ACTIVE),
  sampleRanksMode(IGNORE_RANKS),
  varyPattern(!probDescDB.get_bool("method.fixed_seed")), 
  backfillFlag(probDescDB.get_bool("method.backfill")),
  wilksFlag(probDescDB.get_bool("method.wilks")),
  numLHSRuns(0)
{
  // pushed down as some derived classes (MLMC) use a MC default
  //if (!sampleType)
  //  sampleType = SUBMETHOD_LHS;

  if (epistemicStats && totalLevelRequests) {
    Cerr << "\nError: sampling does not support level requests for "
	 << "analyses containing epistemic uncertainties." << std::endl;
    abort_handler(-1);
  }

  // initialize finalStatistics using the default statistics set
  initialize_final_statistics();

  if ( wilksFlag ) {
    // Only works with sample_type of random
    // BMA: consider relaxing, despite no theory
    if ( sampleType != SUBMETHOD_RANDOM ) {
      Cerr << "Error: Wilks sample sizes require use of \"random\" sample_type." << std::endl;
      abort_handler(-1);
    }

    // Check for conflicting samples spec. Note that this still allows
    // a user to specify "samples = 0" alongside wilks
    if ( numSamples > 0 ) { 
      Cerr << "Error: Cannot specify both \"samples\" and \"wilks\"." 
	   << std::endl;
      abort_handler(-1);
    }

    // Wilks order statistics
    wilksOrder = probDescDB.get_ushort("method.order");
    // Wilks interval sidedness
    wilksSidedness = probDescDB.get_short("method.wilks.sided_interval");
    bool wilks_twosided = (wilksSidedness == TWO_SIDED);

    // Support multiple probability_levels
    Real max_prob_level = 0.0;
    for (size_t i=0; i<numFunctions; ++i) {
      size_t pl_len = requestedProbLevels[i].length();
      for (size_t j=0; j<pl_len; ++j) {
        if (requestedProbLevels[i][j] > max_prob_level)
          max_prob_level = requestedProbLevels[i][j] ;
      }
    }
    wilksAlpha = max_prob_level;
    if (wilksAlpha <= 0.0) // Assign a default if probability_levels unspecified
      wilksAlpha = 0.95;

    wilksBeta = probDescDB.get_real("method.confidence_level");
    if (wilksBeta <= 0.0) // Assign a default if probability_levels unspecified
      wilksBeta = 0.95;
    numSamples = compute_wilks_sample_size(wilksOrder, wilksAlpha, wilksBeta, wilks_twosided);
    samplesRef = numSamples;
  }

  // update concurrency
  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


/** This alternate constructor is used for generation and evaluation
    of on-the-fly sample sets. */
NonDSampling::
NonDSampling(unsigned short method_name, Model& model,
	     unsigned short sample_type, int samples, int seed,
	     const String& rng, bool vary_pattern, short sampling_vars_mode):
  NonD(method_name, model), seedSpec(seed), randomSeed(seed),
  samplesSpec(samples), samplesRef(samples), numSamples(samples), rngName(rng),
  sampleType(sample_type), wilksFlag(false), samplesIncrement(0), 
  statsFlag(false), allDataFlag(true),
  samplingVarsMode(sampling_vars_mode), sampleRanksMode(IGNORE_RANKS),
  varyPattern(vary_pattern), backfillFlag(false), numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // override default epistemicStats setting from NonD ctor
  bool aleatory_mode = (samplingVarsMode == ALEATORY_UNCERTAIN ||
			samplingVarsMode == ALEATORY_UNCERTAIN_UNIFORM);
  epistemicStats = (numEpistemicUncVars && !aleatory_mode);

  // enforce LHS as default sample type
  if (!sampleType)
    sampleType = SUBMETHOD_LHS;

  // not used but included for completeness
  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


/** This alternate constructor is used by ConcurrentStrategy for
    generation of uniform, uncorrelated sample sets. */
NonDSampling::
NonDSampling(unsigned short sample_type, int samples, int seed,
	     const String& rng, const RealVector& lower_bnds,
	     const RealVector& upper_bnds):
  NonD(RANDOM_SAMPLING, lower_bnds, upper_bnds), seedSpec(seed),
  randomSeed(seed), samplesSpec(samples), samplesRef(samples),
  numSamples(samples), rngName(rng), sampleType(sample_type), 
  wilksFlag(false), samplesIncrement(0), statsFlag(false),
  allDataFlag(true), samplingVarsMode(ACTIVE_UNIFORM),
  sampleRanksMode(IGNORE_RANKS), varyPattern(true), backfillFlag(false), 
  numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // enforce LHS as default sample type
  if (!sampleType)
    sampleType = SUBMETHOD_LHS;

  // not used but included for completeness
  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}

/** This alternate constructor is used by ConcurrentStrategy for
    generation of normal, correlated sample sets. */
NonDSampling::
NonDSampling(unsigned short sample_type, int samples, int seed,
	     const String& rng, const RealVector& means, 
             const RealVector& std_devs, const RealVector& lower_bnds,
	     const RealVector& upper_bnds, RealSymMatrix& correl):
  NonD(RANDOM_SAMPLING, lower_bnds, upper_bnds), seedSpec(seed),
  randomSeed(seed), samplesSpec(samples), samplesRef(samples),
  numSamples(samples), rngName(rng), sampleType(sample_type), 
  wilksFlag(false), samplesIncrement(0), statsFlag(false),
  allDataFlag(true), samplingVarsMode(ACTIVE),
  sampleRanksMode(IGNORE_RANKS), varyPattern(true), backfillFlag(false), 
  numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // enforce LHS as default sample type
  if (!sampleType)
    sampleType = SUBMETHOD_LHS;

  // not used but included for completeness
  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}

/** This alternate constructor defines allSamples from an incoming
    sample matrix. */
NonDSampling::
NonDSampling(Model& model, const RealMatrix& sample_matrix):
  NonD(LIST_SAMPLING, model), seedSpec(0), randomSeed(0),
  samplesSpec(sample_matrix.numCols()), sampleType(SUBMETHOD_DEFAULT),
  wilksFlag(false), samplesIncrement(0), statsFlag(true), allDataFlag(true),
  samplingVarsMode(ACTIVE), sampleRanksMode(IGNORE_RANKS),
  varyPattern(false), backfillFlag(false), numLHSRuns(0)
{
  allSamples = sample_matrix; compactMode = true;
  samplesRef = numSamples = samplesSpec;

  subIteratorFlag = true; // suppress some output

  // not used but included for completeness
  if (numSamples)
    maxEvalConcurrency *= numSamples;
}


NonDSampling::~NonDSampling()
{ }


void NonDSampling::transform_samples(bool x_to_u)
{
  // transform x_samples to u_samples for use by expansionSampler
  transform_samples(allSamples, x_to_u, numSamples);
}


void NonDSampling::transform_samples(RealMatrix& sample_matrix, bool x_to_u,
				     int num_samples)
{
  if (num_samples == 0)
    num_samples = sample_matrix.numCols();
  if (x_to_u)
    for (size_t i=0; i<num_samples; ++i) {
      RealVector x_samp(Teuchos::Copy, sample_matrix[i], numContinuousVars);
      RealVector u_samp(Teuchos::View, sample_matrix[i], numContinuousVars);
      natafTransform.trans_X_to_U(x_samp, u_samp);
    }
  else
    for (size_t i=0; i<num_samples; ++i) {
      RealVector u_samp(Teuchos::Copy, sample_matrix[i], numContinuousVars);
      RealVector x_samp(Teuchos::View, sample_matrix[i], numContinuousVars);
      natafTransform.trans_U_to_X(u_samp, x_samp);
    }
}


/** This version of get_parameter_sets() extracts data from the
    user-defined model in any of the four sampling modes and populates
    class member allSamples. */
void NonDSampling::get_parameter_sets(Model& model)
{
  get_parameter_sets(model, numSamples, allSamples);
}

/** This version of get_parameter_sets() extracts data from the
    user-defined model in any of the four sampling modes and populates
    the specified design matrix. */
void NonDSampling::get_parameter_sets(Model& model, const int num_samples,
                                      RealMatrix& design_matrix)
{
  get_parameter_sets(model, num_samples, design_matrix, true);
}

void NonDSampling::get_parameter_sets(Model& model, const int num_samples,
                                      RealMatrix& design_matrix, bool write_msg)
{
  initialize_lhs(write_msg, num_samples);

  short model_view = model.current_variables().view().first;
  switch (samplingVarsMode) {
  case ACTIVE_UNIFORM: case ALL_UNIFORM: case UNCERTAIN_UNIFORM:
  case ALEATORY_UNCERTAIN_UNIFORM: case EPISTEMIC_UNCERTAIN_UNIFORM: 
    // Use LHSDriver::generate_uniform_samples() between lower/upper bounds
    if ( samplingVarsMode == ACTIVE_UNIFORM ||
	 ( samplingVarsMode == ALL_UNIFORM && 
	   ( model_view == RELAXED_ALL || model_view == MIXED_ALL ) ) ||
	 ( samplingVarsMode == UNCERTAIN_UNIFORM &&
	   ( model_view == RELAXED_UNCERTAIN ||
	     model_view == MIXED_UNCERTAIN ) ) ||
	 ( samplingVarsMode == ALEATORY_UNCERTAIN_UNIFORM &&
	   ( model_view == RELAXED_ALEATORY_UNCERTAIN ||
	     model_view == MIXED_ALEATORY_UNCERTAIN ) ) ||
	 ( samplingVarsMode == EPISTEMIC_UNCERTAIN_UNIFORM &&
	   ( model_view == RELAXED_EPISTEMIC_UNCERTAIN ||
	     model_view == MIXED_EPISTEMIC_UNCERTAIN ) ) ) {
      // sample uniformly from ACTIVE lower/upper bounds (regardless of model
      // view), from UNCERTAIN lower/upper bounds (with model in DISTINCT view),
      // or from ALL lower/upper bounds (with model in ALL view).
      // loss of sampleRanks control is OK since NonDIncremLHS uses ACTIVE mode.
      // TO DO: add support for uniform discrete
      lhsDriver.generate_uniform_samples(model.continuous_lower_bounds(),
					 model.continuous_upper_bounds(),
					 num_samples, design_matrix, 
					 backfillFlag);
    }
    else if (samplingVarsMode == ALL_UNIFORM) {
      // sample uniformly from ALL lower/upper bnds with model in distinct view.
      // loss of sampleRanks control is OK since NonDIncremLHS uses ACTIVE mode.
      // TO DO: add support for uniform discrete
      lhsDriver.generate_uniform_samples(model.all_continuous_lower_bounds(),
					 model.all_continuous_upper_bounds(),
					 num_samples, design_matrix, 
					 backfillFlag);
    }
    else { // A, E, A+E UNCERTAIN
      // sample uniformly from {A,E,A+E} UNCERTAIN lower/upper bounds
      // with model using a non-corresponding view (corresponding views
      // handled in first case above)
      size_t start_acv, num_acv, dummy;
      mode_counts(model, start_acv, num_acv, dummy, dummy, dummy, dummy,
		  dummy, dummy);
      if (!num_acv) {
	Cerr << "Error: no active continuous variables for sampling in "
	     << "uniform mode" << std::endl;
	abort_handler(-1);
      }
      const RealVector& all_c_l_bnds = model.all_continuous_lower_bounds();
      const RealVector& all_c_u_bnds = model.all_continuous_upper_bounds();
      RealVector uncertain_c_l_bnds(Teuchos::View,
	const_cast<Real*>(&all_c_l_bnds[start_acv]), num_acv);
      RealVector uncertain_c_u_bnds(Teuchos::View,
	const_cast<Real*>(&all_c_u_bnds[start_acv]), num_acv);
      // loss of sampleRanks control is OK since NonDIncremLHS uses ACTIVE mode
      // TO DO: add support for uniform discrete
      lhsDriver.generate_uniform_samples(uncertain_c_l_bnds, uncertain_c_u_bnds,
					 num_samples, design_matrix, 
					 backfillFlag);
    }
    break;
  case ALEATORY_UNCERTAIN:
    lhsDriver.generate_samples(model.aleatory_distribution_parameters(),
			       num_samples, design_matrix, backfillFlag);
    break;
  case EPISTEMIC_UNCERTAIN:
    lhsDriver.generate_samples(model.epistemic_distribution_parameters(),
			       num_samples, design_matrix, backfillFlag);
    break;
  case UNCERTAIN:
    lhsDriver.generate_samples(model.aleatory_distribution_parameters(),
			       model.epistemic_distribution_parameters(),
			       num_samples, design_matrix, backfillFlag);
    break;
  case ACTIVE: case ALL: {
    // extract design and state bounds
    RealVector  cdv_l_bnds,   cdv_u_bnds,   csv_l_bnds,   csv_u_bnds;
    IntVector ddriv_l_bnds, ddriv_u_bnds, dsriv_l_bnds, dsriv_u_bnds;
    const RealVector& all_c_l_bnds = model.all_continuous_lower_bounds();
    const RealVector& all_c_u_bnds = model.all_continuous_upper_bounds();
    if (numContDesVars) {
      cdv_l_bnds
	= RealVector(Teuchos::View, all_c_l_bnds.values(), numContDesVars);
      cdv_u_bnds
	= RealVector(Teuchos::View, all_c_u_bnds.values(), numContDesVars);
    }
    if (numContStateVars) {
      size_t cv_start = model.acv() - numContStateVars;
      csv_l_bnds = RealVector(Teuchos::View,
	const_cast<Real*>(&all_c_l_bnds[cv_start]), numContStateVars);
      csv_u_bnds = RealVector(Teuchos::View,
	const_cast<Real*>(&all_c_u_bnds[cv_start]), numContStateVars);
    }
    const IntVector& all_di_l_bnds = model.all_discrete_int_lower_bounds();
    const IntVector& all_di_u_bnds = model.all_discrete_int_upper_bounds();
    size_t num_ddriv = (numDiscIntDesVars) ?
      numDiscIntDesVars - model.discrete_design_set_int_values().size() : 0;
    if (num_ddriv) {
      ddriv_l_bnds = IntVector(Teuchos::View, all_di_l_bnds.values(),num_ddriv);
      ddriv_u_bnds = IntVector(Teuchos::View, all_di_u_bnds.values(),num_ddriv);
    }
    size_t num_dsriv = (numDiscIntStateVars) ?
      numDiscIntStateVars - model.discrete_state_set_int_values().size() : 0;
    if (num_dsriv) {
      size_t di_start = model.adiv() - numDiscIntStateVars;
      dsriv_l_bnds = IntVector(Teuchos::View,
	const_cast<int*>(&all_di_l_bnds[di_start]), num_dsriv);
      dsriv_u_bnds = IntVector(Teuchos::View,
	const_cast<int*>(&all_di_u_bnds[di_start]), num_dsriv);
    }
    IntSetArray empty_isa; StringSetArray empty_ssa; RealSetArray empty_rsa;
    const IntSetArray&    di_design_sets = (numDiscIntDesVars) ?
      model.discrete_design_set_int_values()    : empty_isa;
    const StringSetArray& ds_design_sets = (numDiscStringDesVars) ?
      model.discrete_design_set_string_values() : empty_ssa;
    const RealSetArray&   dr_design_sets = (numDiscRealDesVars) ?
      model.discrete_design_set_real_values()   : empty_rsa;
    const IntSetArray&    di_state_sets  = (numDiscIntStateVars) ?
      model.discrete_state_set_int_values()     : empty_isa;
    const StringSetArray& ds_state_sets  = (numDiscStringStateVars) ?
      model.discrete_state_set_string_values()  : empty_ssa;
    const RealSetArray&   dr_state_sets  = (numDiscRealStateVars) ?
      model.discrete_state_set_real_values()    : empty_rsa;    

    // Call LHS to generate the specified samples within the specified
    // distributions.  Use model distribution parameters unless ACTIVE
    // excludes {aleatory,epistemic,both} uncertain variables.
    if ( samplingVarsMode == ACTIVE &&
	 ( model_view == RELAXED_DESIGN || model_view == RELAXED_STATE ||
	   model_view ==   MIXED_DESIGN || model_view ==   MIXED_STATE ) ) {
      Pecos::AleatoryDistParams empty_adp; Pecos::EpistemicDistParams empty_edp;
      if ( !backfillFlag )
	lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
   	  ds_state_sets, dr_state_sets, empty_adp, empty_edp, num_samples,
	  design_matrix, sampleRanks);
      else
	lhsDriver.generate_unique_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
   	  ds_state_sets, dr_state_sets, empty_adp, empty_edp, num_samples,
	  design_matrix, sampleRanks);
    }
    else if ( samplingVarsMode == ACTIVE &&
	      ( model_view == RELAXED_ALEATORY_UNCERTAIN ||
		model_view == MIXED_ALEATORY_UNCERTAIN ) ) {
      Pecos::EpistemicDistParams empty_edp;
      if ( !backfillFlag )
	lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
  	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
	  ds_state_sets, dr_state_sets,model.aleatory_distribution_parameters(),
	  empty_edp, num_samples, design_matrix, sampleRanks);
      else
	lhsDriver.generate_unique_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
  	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
	  ds_state_sets, dr_state_sets,model.aleatory_distribution_parameters(),
	  empty_edp, num_samples, design_matrix, sampleRanks);
    }
    else if ( samplingVarsMode == ACTIVE &&
	      ( model_view == RELAXED_EPISTEMIC_UNCERTAIN ||
		model_view == MIXED_EPISTEMIC_UNCERTAIN ) ) {
      Pecos::AleatoryDistParams empty_adp;
      if ( !backfillFlag )
	lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
  	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
	  ds_state_sets, dr_state_sets, empty_adp,
	  model.epistemic_distribution_parameters(), num_samples, design_matrix,
	  sampleRanks);
      else
	lhsDriver.generate_unique_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
  	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
	  ds_state_sets, dr_state_sets, empty_adp,
	  model.epistemic_distribution_parameters(), num_samples, design_matrix,
	  sampleRanks);
    }
    else {
      if ( !backfillFlag )
	lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
	  ds_state_sets, dr_state_sets,model.aleatory_distribution_parameters(),
	  model.epistemic_distribution_parameters(), num_samples, design_matrix,
	  sampleRanks);
      else
	lhsDriver.generate_unique_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	  ddriv_u_bnds, di_design_sets, ds_design_sets, dr_design_sets,
	  csv_l_bnds, csv_u_bnds, dsriv_l_bnds, dsriv_u_bnds, di_state_sets,
	  ds_state_sets, dr_state_sets,model.aleatory_distribution_parameters(),
	  model.epistemic_distribution_parameters(), num_samples, design_matrix,
	  sampleRanks);
	  // warning sampleRanks will empty. 
	  // See comment in lhs_driver.cpp generate_unique_samples()
    }
    break;
  }
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
  initialize_lhs(true, numSamples);
  lhsDriver.generate_uniform_samples(lower_bnds, upper_bnds,
				     numSamples, allSamples);
}

/** This version of get_parameter_sets() does not extract data from the
    user-defined model, but instead relies on the incoming 
    definition.  It only support the sampling of normal variables. */
void NonDSampling::
get_parameter_sets(const RealVector& means, const RealVector& std_devs, 
                   const RealVector& lower_bnds, const RealVector& upper_bnds,
                   RealSymMatrix& correl)
{
  initialize_lhs(true, numSamples);
  lhsDriver.generate_normal_samples(means, std_devs, lower_bnds, upper_bnds,
				    numSamples, correl, allSamples);
}


void NonDSampling::
update_model_from_sample(Model& model, const Real* sample_vars)
{
  size_t i, cntr = 0, cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(model, cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
	      drv_start, num_drv);

  // sampled continuous vars (by value)
  size_t end = cv_start + num_cv;
  for (i=cv_start; i<end; ++i, ++cntr)
    model.all_continuous_variable(sample_vars[cntr], i);
  // sampled discrete int vars (by value cast from Real)
  end = div_start + num_div;
  for (i=div_start; i<end; ++i, ++cntr)
    model.all_discrete_int_variable((int)sample_vars[cntr], i);
  // sampled discrete string vars (by index cast from Real)
  short active_view = model.current_variables().view().first;
  bool relax = (active_view == RELAXED_ALL ||
    ( active_view >= RELAXED_DESIGN && active_view <= RELAXED_STATE ) );
  short all_view = (relax) ? RELAXED_ALL : MIXED_ALL;
  const StringSetArray& all_dss_values
    = model.discrete_set_string_values(all_view);
  end = dsv_start + num_dsv;
  for (i=dsv_start; i<end; ++i, ++cntr)
    model.all_discrete_string_variable(set_index_to_value(
      (size_t)sample_vars[cntr], all_dss_values[i]), i);
  // sampled discrete real vars (by value)
  end = drv_start + num_drv;
  for (i=drv_start; i<end; ++i, ++cntr)
    model.all_discrete_real_variable(sample_vars[cntr], i);
}


// BMA TODO: consolidate with other use cases
void NonDSampling::
sample_to_variables(const Real* sample_vars, Variables& vars)
{
  size_t i, cntr = 0, cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div, dsv_start,
	      num_dsv, drv_start, num_drv);

  // BMA TODO: make sure inactive get updated too as needed?

  // sampled continuous vars (by value)
  size_t end = cv_start + num_cv;
  for (i=cv_start; i<end; ++i, ++cntr)
    vars.all_continuous_variable(sample_vars[cntr], i);
  // sampled discrete int vars (by value cast from Real)
  end = div_start + num_div;
  for (i=div_start; i<end; ++i, ++cntr)
    vars.all_discrete_int_variable((int)sample_vars[cntr], i);
  // sampled discrete string vars (by index cast from Real)
  short active_view = vars.view().first;
  bool relax = (active_view == RELAXED_ALL ||
    ( active_view >= RELAXED_DESIGN && active_view <= RELAXED_STATE ) );
  short all_view = (relax) ? RELAXED_ALL : MIXED_ALL;
  const StringSetArray& all_dss_values
    = iteratedModel.discrete_set_string_values(all_view);
  end = dsv_start + num_dsv;
  for (i=dsv_start; i<end; ++i, ++cntr)
    vars.all_discrete_string_variable(set_index_to_value(
      (size_t)sample_vars[cntr], all_dss_values[i]), i);
  // sampled discrete real vars (by value)
  end = drv_start + num_drv;
  for (i=drv_start; i<end; ++i, ++cntr)
    vars.all_discrete_real_variable(sample_vars[cntr], i);
}


// BMA TODO: consolidate with other use cases
/** Map the active variables from vars to sample_vars (column in allSamples) */
void NonDSampling::
variables_to_sample(const Variables& vars, Real* sample_vars)
{
  size_t cntr = 0;

  const RealVector& c_vars = vars.continuous_variables();
  for (size_t j=0; j<numContinuousVars; ++j, ++cntr)
    sample_vars[cntr] = c_vars[j]; // jth row of samples_matrix

  const IntVector& di_vars = vars.discrete_int_variables();
  for (size_t j=0; j<numDiscreteIntVars; ++j, ++cntr)
    sample_vars[cntr] = (Real) di_vars[j]; // jth row of samples_matrix

  // to help with mapping string variables
  // sampled discrete string vars (by index cast from Real)
  short active_view = vars.view().first;
  bool relax = (active_view == RELAXED_ALL ||
    ( active_view >= RELAXED_DESIGN && active_view <= RELAXED_STATE ) );
  short all_view = (relax) ? RELAXED_ALL : MIXED_ALL;
  const StringSetArray& all_dss_values
    = iteratedModel.discrete_set_string_values(all_view);

  // is care needed to manage active vs. all string variables?

  StringMultiArrayConstView ds_vars = vars.discrete_string_variables();
  for (size_t j=0; j<numDiscreteStringVars; ++j, ++cntr) // jth row
    sample_vars[cntr] = (Real)set_value_to_index(ds_vars[j], all_dss_values[j]);

  const RealVector& dr_vars = vars.discrete_real_variables();
  for (size_t j=0; j<numDiscreteRealVars; ++j, ++cntr)
    sample_vars[cntr] = (Real)dr_vars[j]; // jth row
}


/** This function and its helpers to follow are needed since NonDSampling
    supports a richer set of sampling modes than just the active variable 
    subset.  mode_counts() manages the samplingVarsMode setting, while its
    helper functions (view_{design,aleatory_uncertain,epistemic_uncertain,
    uncertain,state}_counts) manage the active variables view.  Similar 
    to the computation of starts and counts in creating active variable
    views, the results of this function are starts and counts for use
    within model.all_*() set/get functions. */
void NonDSampling::
mode_counts(const Model& model, size_t& cv_start,  size_t& num_cv,
	    size_t& div_start,  size_t& num_div,   size_t& dsv_start,
	    size_t& num_dsv,    size_t& drv_start, size_t& num_drv) const
{
  cv_start = div_start = dsv_start = drv_start = 0;
  num_cv   = num_div   = num_dsv   = num_drv   = 0;
  switch (samplingVarsMode) {
  case ALEATORY_UNCERTAIN:
    // design vars define starting indices
    view_design_counts(model, cv_start, div_start, dsv_start, drv_start);
    // A uncertain vars define counts
    view_aleatory_uncertain_counts(model, num_cv, num_div, num_dsv, num_drv);
    break;
  case ALEATORY_UNCERTAIN_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    size_t dummy;
    // continuous design vars define starting indices
    view_design_counts(model, cv_start, dummy, dummy, dummy);
    // continuous A uncertain vars define counts
    view_aleatory_uncertain_counts(model, num_cv, dummy, dummy, dummy);   break;
  }
  case EPISTEMIC_UNCERTAIN: {
    // design + A uncertain vars define starting indices
    size_t num_cdv,  num_ddiv,  num_ddsv,  num_ddrv,
           num_cauv, num_dauiv, num_dausv, num_daurv;
    view_design_counts(model, num_cdv, num_ddiv, num_ddsv, num_ddrv);
    view_aleatory_uncertain_counts(model, num_cauv, num_dauiv, num_dausv,
				   num_daurv);
    cv_start  = num_cdv  + num_cauv;  div_start = num_ddiv + num_dauiv;
    dsv_start = num_ddsv + num_dausv; drv_start = num_ddrv + num_daurv;
    // E uncertain vars define counts
    view_epistemic_uncertain_counts(model, num_cv, num_div, num_dsv, num_drv);
    break;
  }
  case EPISTEMIC_UNCERTAIN_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    // continuous design + A uncertain vars define starting indices
    size_t num_cdv, num_cauv, dummy;
    view_design_counts(model, num_cdv, dummy, dummy, dummy);
    view_aleatory_uncertain_counts(model, num_cauv, dummy, dummy, dummy);
    cv_start = num_cdv + num_cauv;
    // continuous E uncertain vars define counts
    view_epistemic_uncertain_counts(model, num_cv, dummy, dummy, dummy);  break;
  }
  case UNCERTAIN:
    // design vars define starting indices
    view_design_counts(model, cv_start, div_start, dsv_start, drv_start);
    // A+E uncertain vars define counts
    view_uncertain_counts(model, num_cv, num_div, num_dsv, num_drv);      break;
  case UNCERTAIN_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    size_t dummy;
    // continuous design vars define starting indices
    view_design_counts(model, cv_start, dummy, dummy, dummy);
    // continuous A+E uncertain vars define counts
    view_uncertain_counts(model, num_cv, dummy, dummy, dummy);            break;
  }
  case ACTIVE: {
    const Variables& vars = model.current_variables();
    cv_start  = vars.cv_start();  num_cv  = vars.cv();
    div_start = vars.div_start(); num_div = vars.div();
    dsv_start = vars.dsv_start(); num_dsv = vars.dsv();
    drv_start = vars.drv_start(); num_drv = vars.drv();                   break;
  }
  case ACTIVE_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    const Variables& vars = model.current_variables();
    cv_start = vars.cv_start(); num_cv = vars.cv();                       break;
  }
  case ALL:
    num_cv  = model.acv();  num_div = model.adiv();
    num_dsv = model.adsv(); num_drv = model.adrv();                       break;
  case ALL_UNIFORM:
    // UNIFORM views do not currently support non-relaxed discrete
    num_cv = model.acv();                                                 break;
  }
}


/** This function computes total design variable counts, not active counts,
    for use in defining offsets and counts within all variables arrays. */
void NonDSampling::
view_design_counts(const Model& model, size_t& num_cdv, size_t& num_ddiv,
		   size_t& num_ddsv, size_t& num_ddrv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: case RELAXED_DESIGN: case MIXED_DESIGN:
    // design vars are included in active counts from NonD and relaxation
    // counts have already been applied
    num_cdv  = numContDesVars;       num_ddiv = numDiscIntDesVars;
    num_ddsv = numDiscStringDesVars; num_ddrv = numDiscRealDesVars; break;
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_STATE:
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_STATE:
    // design vars are not included in active counts from NonD
    vars.shared_data().design_counts(num_cdv, num_ddiv, num_ddsv, num_ddrv);
    break;
  case RELAXED_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
  case   MIXED_UNCERTAIN: case   MIXED_ALEATORY_UNCERTAIN:
    num_cdv  = vars.cv_start();  num_ddiv = vars.div_start();
    num_ddsv = vars.dsv_start(); num_ddrv = vars.drv_start(); break;
  }
}


/** This function computes total aleatory uncertain variable counts,
    not active counts, for use in defining offsets and counts within
    all variables arrays. */
void NonDSampling::
view_aleatory_uncertain_counts(const Model& model, size_t& num_cauv,
			       size_t& num_dauiv, size_t& num_dausv,
			       size_t& num_daurv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case RELAXED_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
  case   MIXED_ALL: case   MIXED_UNCERTAIN: case   MIXED_ALEATORY_UNCERTAIN:
    // aleatory vars are included in active counts from NonD and relaxation
    // counts have already been applied
    num_cauv  = numContAleatUncVars;       num_dauiv = numDiscIntAleatUncVars;
    num_dausv = numDiscStringAleatUncVars; num_daurv = numDiscRealAleatUncVars;
    break;
  case RELAXED_DESIGN: case RELAXED_STATE: case RELAXED_EPISTEMIC_UNCERTAIN:
  case MIXED_DESIGN:   case MIXED_STATE:   case MIXED_EPISTEMIC_UNCERTAIN:
    vars.shared_data().aleatory_uncertain_counts(num_cauv,  num_dauiv,
						 num_dausv, num_daurv);
    break;
  }
}


/** This function computes total epistemic uncertain variable counts,
    not active counts, for use in defining offsets and counts within
    all variables arrays. */
void NonDSampling::
view_epistemic_uncertain_counts(const Model& model, size_t& num_ceuv,
				size_t& num_deuiv, size_t& num_deusv,
				size_t& num_deurv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case RELAXED_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
  case   MIXED_ALL: case   MIXED_UNCERTAIN: case   MIXED_EPISTEMIC_UNCERTAIN:
    num_ceuv  = numContEpistUncVars;       num_deuiv = numDiscIntEpistUncVars;
    num_deusv = numDiscStringEpistUncVars; num_deurv = numDiscRealEpistUncVars;
    break;
  case RELAXED_DESIGN: case RELAXED_ALEATORY_UNCERTAIN: case RELAXED_STATE:
  case MIXED_DESIGN:   case MIXED_ALEATORY_UNCERTAIN:   case MIXED_STATE:
    vars.shared_data().epistemic_uncertain_counts(num_ceuv,  num_deuiv,
						  num_deusv, num_deurv);
    break;
  }
}


/** This function computes total uncertain variable counts, not active counts,
    for use in defining offsets and counts within all variables arrays. */
void NonDSampling::
view_uncertain_counts(const Model& model, size_t& num_cuv, size_t& num_duiv,
		      size_t& num_dusv, size_t& num_durv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: // UNCERTAIN = subset of ACTIVE
    num_cuv  = numContAleatUncVars       + numContEpistUncVars;
    num_duiv = numDiscIntAleatUncVars    + numDiscIntEpistUncVars;
    num_dusv = numDiscStringAleatUncVars + numDiscStringEpistUncVars;
    num_durv = numDiscRealAleatUncVars   + numDiscRealEpistUncVars;      break;
  case RELAXED_DESIGN:             case RELAXED_STATE:
  case RELAXED_ALEATORY_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
  case MIXED_DESIGN:               case MIXED_STATE:
  case MIXED_ALEATORY_UNCERTAIN:   case MIXED_EPISTEMIC_UNCERTAIN:
    vars.shared_data().uncertain_counts(num_cuv, num_duiv, num_dusv, num_durv);
    break;
  case RELAXED_UNCERTAIN: case MIXED_UNCERTAIN: // UNCERTAIN = same as ACTIVE
    num_cuv  = vars.cv();  num_duiv = vars.div();
    num_dusv = vars.dsv(); num_durv = vars.drv(); break;
  }
}


void NonDSampling::
view_state_counts(const Model& model, size_t& num_csv, size_t& num_dsiv,
		  size_t& num_dssv, size_t& num_dsrv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: case RELAXED_STATE: case MIXED_STATE:
    // state vars are included in active counts from NonD and relaxation
    // counts have already been applied
    num_csv  = numContStateVars;       num_dsiv = numDiscIntStateVars;
    num_dssv = numDiscStringStateVars; num_dsrv = numDiscRealStateVars; break;
  case RELAXED_ALEATORY_UNCERTAIN: case RELAXED_DESIGN:
  case   MIXED_ALEATORY_UNCERTAIN: case   MIXED_DESIGN:
    // state vars are not included in active counts from NonD
    vars.shared_data().state_counts(num_csv, num_dsiv, num_dssv, num_dsrv);
    break;
  case RELAXED_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
  case   MIXED_UNCERTAIN: case   MIXED_EPISTEMIC_UNCERTAIN:
    num_csv  = vars.acv()  - vars.cv_start()  - vars.cv();
    num_dsiv = vars.adiv() - vars.div_start() - vars.div();
    num_dssv = vars.adsv() - vars.dsv_start() - vars.dsv();
    num_dsrv = vars.adrv() - vars.drv_start() - vars.drv(); break;
  }
}


void NonDSampling::initialize_lhs(bool write_message, int num_samples)
{
  // keep track of number of LHS executions for this object
  ++numLHSRuns;

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
    if (!seedSpec) // no user specification --> nonrepeatable behavior
      randomSeed = generate_system_seed();
    lhsDriver.seed(randomSeed);
  }
  else if (varyPattern) // define sequence of seed values for numLHSRuns > 1
    lhsDriver.advance_seed_sequence();
  else // fixed_seed
    lhsDriver.seed(randomSeed); // reset original/machine-generated seed

  // Needed a way to turn this off when LHS sampling is being used in
  // NonDAdaptImpSampling because it gets written a _LOT_
  String sample_string = submethod_enum_to_string(sampleType);
  if (write_message) {
    Cout << "\nNonD " << sample_string << " Samples = " << num_samples;
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

  lhsDriver.initialize(sample_string, sampleRanksMode, !subIteratorFlag);
}


/** Default implementation generates allResponses from either allSamples
    or allVariables. */
void NonDSampling::core_run()
{
  bool log_resp_flag = (allDataFlag || statsFlag), log_best_flag = false;
  evaluate_parameter_sets(iteratedModel, log_resp_flag, log_best_flag);
}


void NonDSampling::
compute_statistics(const RealMatrix&     vars_samples,
		   const IntResponseMap& resp_samples)
{
  StringMultiArrayConstView
    acv_labels  = iteratedModel.all_continuous_variable_labels(),
    adiv_labels = iteratedModel.all_discrete_int_variable_labels(),
    adsv_labels = iteratedModel.all_discrete_string_variable_labels(),
    adrv_labels = iteratedModel.all_discrete_real_variable_labels();
  size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
	      dsv_start, num_dsv, drv_start, num_drv);
  StringMultiArrayConstView
    cv_labels  =
      acv_labels[boost::indices[idx_range(cv_start, cv_start+num_cv)]],
    div_labels =
      adiv_labels[boost::indices[idx_range(div_start, div_start+num_div)]],
    dsv_labels =
      adsv_labels[boost::indices[idx_range(dsv_start, dsv_start+num_dsv)]],
    drv_labels =
      adrv_labels[boost::indices[idx_range(drv_start, drv_start+num_drv)]];

  // archive the active variables with the results
  if (resultsDB.active()) {
    if (num_cv)
      resultsDB.insert(run_identifier(), resultsNames.cv_labels, cv_labels);
    if (num_div)
      resultsDB.insert(run_identifier(), resultsNames.div_labels, div_labels);
    if (num_dsv)
      resultsDB.insert(run_identifier(), resultsNames.dsv_labels, dsv_labels);
    if (num_drv)
      resultsDB.insert(run_identifier(), resultsNames.drv_labels, drv_labels);
    resultsDB.insert(run_identifier(), resultsNames.fn_labels, 
		     iteratedModel.response_labels());
  }

  if (epistemicStats) // Epistemic/mixed
    compute_intervals(resp_samples); // compute min/max response intervals
  else { // Aleatory
    // compute means and std deviations with confidence intervals
    compute_moments(resp_samples);
    // compute CDF/CCDF mappings of z to p/beta and p/beta to z
    if (totalLevelRequests)
      compute_level_mappings(resp_samples);
  }

  if (!subIteratorFlag) {
    nonDSampCorr.compute_correlations(vars_samples, resp_samples);
    // archive the correlations to the results DB
    nonDSampCorr.archive_correlations(run_identifier(), resultsDB, cv_labels,
				      div_labels, dsv_labels, drv_labels,
				      iteratedModel.response_labels());
  }

  // push results into finalStatistics
  update_final_statistics();
}


void NonDSampling::
compute_intervals(RealRealPairArray& extreme_fns, const IntResponseMap& samples)
{
  // For the samples array, calculate min/max response intervals

  size_t i, j, num_obs = samples.size(), num_samp;
  const StringArray& resp_labels = iteratedModel.response_labels();

  if (extreme_fns.empty()) extreme_fns.resize(numFunctions);
  IntRespMCIter it;
  for (i=0; i<numFunctions; ++i) {
    num_samp = 0;
    Real min = DBL_MAX, max = -DBL_MAX;
    for (it=samples.begin(); it!=samples.end(); ++it) {
      Real sample = it->second.function_value(i);
      if (isfinite(sample)) { // neither NaN nor +/-Inf
	if (sample < min) min = sample;
	if (sample > max) max = sample;
	++num_samp;
      }
    }
    extreme_fns[i].first  = min;
    extreme_fns[i].second = max;
    if (num_samp != num_obs)
      Cerr << "Warning: sampling statistics for " << resp_labels[i] << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";
  }

  if (resultsDB.active()) {
    MetaDataType md;
    md["Row Labels"] = make_metadatavalue("Min", "Max");
    md["Column Labels"] = make_metadatavalue(resp_labels);
    resultsDB.insert(run_identifier(), resultsNames.extreme_values, 
		     extreme_fns, md);
  }
}


void NonDSampling::
compute_moments(const IntResponseMap& samples, RealMatrix& moment_stats,
		RealMatrix& moment_conf_ints, short moments_type,
		const StringArray& labels)
{
  // For the samples array, calculate means and standard deviations
  // with confidence intervals

  RealMatrix samples_rm; SizetArray sample_counts;
  samples_rm.shapeUninitialized(numFunctions, samples.size());
  IntRespMCIter it;
  size_t i, j;
  for (it=samples.begin(), j=0; it!=samples.end(); ++it, ++j) {
    const RealVector& fn_vals = it->second.function_values();
    for (i=0; i<numFunctions; ++i)
      samples_rm(i,j) = fn_vals[i];
  }
  compute_moments(samples_rm, sample_counts, moment_stats, moments_type,labels);
  compute_moment_confidence_intervals(moment_stats, moment_conf_ints,
				      sample_counts, moments_type);
  if (resultsDB.active()) {
    archive_moments(moment_stats, moments_type, labels);
    archive_moment_confidence_intervals(moment_conf_ints, moments_type, labels);
  }
}


void NonDSampling::
compute_moments(const RealMatrix& samples, SizetArray& sample_counts,
		RealMatrix& moment_stats, short moments_type,
		const StringArray& labels)
{
  // For a samples matrix, calculate mean, standard deviation,
  // skewness, and kurtosis

  size_t i, j, num_qoi = samples.numRows(), num_obs = samples.numCols();
  Real sum, cm2, cm3, cm4, sample;

  if (moment_stats.empty()) moment_stats.shapeUninitialized(4, num_qoi);
  if (sample_counts.size() != num_qoi) sample_counts.resize(num_qoi);

  for (i=0; i<num_qoi; ++i) {
    Real*  moments_i = moment_stats[i];
    size_t& num_samp = sample_counts[i];

    accumulate_mean(samples, i, num_samp, moments_i[0]);
    if (num_samp != num_obs)
      Cerr << "Warning: sampling statistics for " << labels[i] << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";

    if (num_samp)
      accumulate_moments(samples, i, moments_i, moments_type);
    else {
      Cerr << "Warning: Number of samples for " << labels[i]
	   << " must be nonzero for moment calculation in NonDSampling::"
	   << "compute_moments().\n";
      //abort_handler(METHOD_ERROR);
      for (int j=0; j<4; ++j)
	moments_i[j] = std::numeric_limits<double>::quiet_NaN();
    }
  }
}


void NonDSampling::
compute_moments(const RealMatrix& samples, RealMatrix& moment_stats,
		short moments_type)
{
  // For a samples matrix, calculate mean, standard deviation,
  // skewness, and kurtosis

  size_t i, num_qoi = samples.numRows(), num_obs = samples.numCols(), num_samp;

  if (moment_stats.empty()) moment_stats.shapeUninitialized(4, num_qoi);

  for (i=0; i<num_qoi; ++i) {
    Real* moments_i = moment_stats[i];

    accumulate_mean(samples, i, num_samp, moments_i[0]);
    if (num_samp != num_obs)
      Cerr << "Warning: sampling statistics for quantity " << i+1 << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";

    if (num_samp)
      accumulate_moments(samples, i, moments_i, moments_type);
    else {
      Cerr << "Warning: Number of samples for quantity " << i+1
	   << " must be nonzero in NonDSampling::compute_moments().\n";
      //abort_handler(METHOD_ERROR);
      for (size_t j=0; j<4; ++j)
	moments_i[j] = std::numeric_limits<double>::quiet_NaN();
    }
  }
}


void NonDSampling::
compute_moment_confidence_intervals(const RealMatrix& moment_stats,
				    RealMatrix& moment_conf_ints,
				    const SizetArray& sample_counts,
				    short moments_type)
{
  size_t i, num_qoi = moment_stats.numCols();
  if (moment_conf_ints.empty())
    moment_conf_ints.shapeUninitialized(4, num_qoi);

  Real mean, std_dev, var, qnan = std::numeric_limits<double>::quiet_NaN();
  for (i=0; i<num_qoi; ++i) {
    if (sample_counts[i] > 1) {
      const Real* moment_i = moment_stats[i];
      Real*    moment_ci_i = moment_conf_ints[i];
      mean = moment_i[0];
      if (moments_type == CENTRAL_MOMENTS)
        { var = moment_i[1]; std_dev = std::sqrt(var); }
      else
        { std_dev = moment_i[1]; var = std_dev * std_dev; }
      if (mean == qnan || std_dev == qnan || var == qnan)
	for (size_t j=0; j<4; ++j)
	  moment_ci_i[j] = qnan;
      else {
	// 95% confidence intervals (2-sided interval, not 1-sided limit)
	Real ns = (Real)sample_counts[i], dof = ns - 1.;
	// mean: the better formula does not assume known variance but requires
	// a function for the Student's t-distr. with (num_samp-1) degrees of
	// freedom (Haldar & Mahadevan, p. 127).
	Pecos::students_t_dist t_dist(dof);
	Real mean_ci_delta = std_dev*bmth::quantile(t_dist,0.975)/std::sqrt(ns);
	moment_ci_i[0] = mean - mean_ci_delta;
	moment_ci_i[1] = mean + mean_ci_delta;
	// std dev: chi-square distribution with (num_samp-1) degrees of freedom
	// (Haldar & Mahadevan, p. 132).
	Pecos::chi_squared_dist chisq(dof);
	Real z_975 = bmth::quantile(chisq, 0.975),
	     z_025 = bmth::quantile(chisq, 0.025);
	if (moments_type == CENTRAL_MOMENTS) {
	  moment_ci_i[2] = var*dof/z_975;
	  moment_ci_i[3] = var*dof/z_025;
	}
	else {
	  moment_ci_i[2] = std_dev*std::sqrt(dof/z_975);
	  moment_ci_i[3] = std_dev*std::sqrt(dof/z_025);
	}
      }
    }
    else
      for (size_t j=0; j<4; ++j)
	moment_conf_ints(j,i) = 0.;
  }
}


void NonDSampling::
accumulate_mean(const RealMatrix& samples, size_t q,
		size_t& num_samp, Real& mean)
{
  num_samp = 0;
  Real sum = 0.;

  size_t s, num_obs = samples.numCols(); Real sample;
  for (s=0; s<num_obs; ++s) {
    sample = samples(q,s);
    if (isfinite(sample)) { // neither NaN nor +/-Inf
      sum += sample;
      ++num_samp;
    }
  }

  if (num_samp)
    mean = sum / (Real)num_samp;
}


void NonDSampling::
accumulate_moments(const RealMatrix& samples, size_t q,
		   Real* moments, short moments_type)
{
  // accumulate central moments (e.g., variance)
  size_t s, num_obs = samples.numCols(), num_samp = 0;
  Real& mean = moments[0]; // already computed in accumulate_mean()
  Real sample, centered_fn, pow_fn, cm2 = 0., cm3 = 0., cm4 = 0.;
  for (s=0; s<num_obs; ++s) {
    sample = samples(q,s);
    if (isfinite(sample)) { // neither NaN nor +/-Inf
      pow_fn  = centered_fn = sample - mean;
      pow_fn *= centered_fn; cm2 += pow_fn; // variance
      pow_fn *= centered_fn; cm3 += pow_fn; // 3rd central moment
      pow_fn *= centered_fn; cm4 += pow_fn; // 4th central moment
      ++num_samp;
    }
  }
  Real ns = (Real)num_samp, np1 = ns + 1., nm1 = ns - 1., nm2 = ns - 2.,
      nm3 = ns - 3.;

  // biased central moment estimators (bypass and use raw sums below):
  //cm2 /= ns; cm3 /= ns; cm4 /= ns;

  // unbiased moment estimators (central and standardized):
  bool central = (moments_type == CENTRAL_MOMENTS), pos_var = (cm2 > 0.);
  if (num_samp > 1 && pos_var)
    moments[1] = (central) ? cm2 / nm1 : // unbiased central
      std::sqrt(cm2/nm1);                // unbiased standard
  else
    moments[1] = 0.;
  if (num_samp > 2 && pos_var)
    moments[2] = (central) ? cm3 * ns / (nm1*nm2) :        // unbiased central
      cm3 * ns * std::sqrt(nm1) / (nm2*std::pow(cm2,1.5)); // unbiased standard
  else
    moments[2] = 0.;
  if (num_samp > 3 && pos_var)
    moments[3] = (central) ?
      (ns*np1*cm4 - 3.*nm1*cm2*cm2)/(nm1*nm2*nm3) :      // unbiased central
      nm1 * (np1*ns*cm4/(cm2*cm2) - 3.*nm1) / (nm2*nm3); // unbiased standard
  else
    moments[3] = (central) ? 0. : -3.;
}


void NonDSampling::
archive_moments(const RealMatrix& moment_stats, short moments_type,
		const StringArray& labels)
{
  // archive the moments to results DB
  MetaDataType md_moments;
  md_moments["Row Labels"] = (moments_type == CENTRAL_MOMENTS) ?
    make_metadatavalue("Mean", "Variance", "3rdCentral", "4thCentral") :
    make_metadatavalue("Mean", "Standard Deviation", "Skewness", "Kurtosis");
  md_moments["Column Labels"] = make_metadatavalue(labels);
  resultsDB.insert(run_identifier(), resultsNames.moments_std, moment_stats,
		   md_moments);
}


void NonDSampling::
archive_moment_confidence_intervals(const RealMatrix& moment_conf_ints,
				    short moments_type,
				    const StringArray& labels)
{
  // archive the confidence intervals to results DB
  MetaDataType md;
  md["Row Labels"] = (moments_type == CENTRAL_MOMENTS) ?
    make_metadatavalue("LowerCI_Mean", "UpperCI_Mean",
		       "LowerCI_Variance", "UpperCI_Variance") :
    make_metadatavalue("LowerCI_Mean", "UpperCI_Mean",
		       "LowerCI_StdDev", "UpperCI_StdDev");
  md["Column Labels"] = make_metadatavalue(labels);
  resultsDB.insert(run_identifier(), resultsNames.moment_cis,
		   moment_conf_ints, md);
}


int NonDSampling::compute_wilks_sample_size(unsigned short order, Real alpha, 
					    Real beta, bool twosided)
{
  Real rorder = (Real) order;

  if( !twosided && (order==1) )
    return std::ceil(std::log(1.0-beta)/std::log(alpha));

  Real n = rorder + 1.0;
  if( twosided ) {
    n = 2.0*rorder;
    while( boost::math::ibeta<Real>(n-2.0*rorder+1.0, 2.0*rorder, alpha) > 
	   1.0-beta )
      n += 1.0;
  }
  else {
    while( boost::math::ibeta<Real>(rorder, n-rorder+1.0, 1-alpha) < beta )
      n += 1.0;
  }

  return std::ceil(n);
}


Real NonDSampling::compute_wilks_residual(unsigned short order, int nsamples, Real alpha, Real beta, bool twosided)
{
  Real rorder = (Real) order;

  if( !twosided && (order==1) )
    return std::log(1.0-beta)/std::log(alpha) - (Real) nsamples;

  if( twosided )
    return boost::math::ibeta<Real>((Real)nsamples-2.0*rorder+1.0, 2.0*rorder, alpha) - (1.0-beta);
  else
    return boost::math::ibeta<Real>(rorder, (Real)nsamples-rorder+1.0, 1-alpha) - beta;
}


Real NonDSampling::compute_wilks_alpha(unsigned short order, int nsamples, Real beta, bool twosided)
{
  Real rorder = (Real) order;

  Real alpha_l = get_wilks_alpha_min(); // initial lower bound
  Real alpha_u = get_wilks_alpha_max(); // initial upper bound
  Real resid_l = compute_wilks_residual(order, nsamples, alpha_l, beta, twosided);
  Real resid_u = compute_wilks_residual(order, nsamples, alpha_u, beta, twosided);
  if( resid_l*resid_u > 0 )
    throw std::runtime_error("Cannot obtain valid bounds for wilks alpha bisection.");

  //Cout << "\nalpha = " << alpha << ", resid = " << resid << std::endl;
  Real tol = 1.e-10;
  //Real eps = 1.e-8;
  Real alpha = alpha_l;
  Real resid = resid_l;

  while( std::fabs(resid) > tol )
  {
    // Newton approach is too fragile - RWH
    //Real resid_plus  = compute_wilks_residual(order, nsamples, alpha+eps, beta, twosided);
    //Real resid_minus = compute_wilks_residual(order, nsamples, alpha-eps, beta, twosided);
    //alpha -= resid/( (resid_plus-resid_minus)/(2.0*eps) );
    alpha = 0.5*(alpha_l+alpha_u);
    resid = compute_wilks_residual(order, nsamples, alpha, beta, twosided);
    if( resid*resid_u > 0 ) {
      alpha_u = alpha;
      resid_u = resid;
    }
    else {
      alpha_l = alpha;
      resid_l = resid;
    }
    //Cout << "alpha = " << alpha << ", resid = " << resid << std::endl;
  }
  //Cout << "Converged: alpha = " << alpha << ", resid = " << resid << std::endl;

  return alpha;
}


Real NonDSampling::compute_wilks_beta(unsigned short order, int nsamples, Real alpha, bool twosided)
{
  Real rorder = (Real) order;

  Real beta_l = get_wilks_beta_min(); // initial lower bound
  Real beta_u = get_wilks_beta_max(); // initial upper bound
  Real resid_l = compute_wilks_residual(order, nsamples, alpha, beta_l, twosided);
  Real resid_u = compute_wilks_residual(order, nsamples, alpha, beta_u, twosided);
  if( resid_l*resid_u > 0 )
    throw std::runtime_error("Cannot obtain valid bounds for wilks beta bisection.");

  Real tol = 1.e-10;
  Real beta = beta_l;
  Real resid = resid_l;

  while( std::fabs(resid) > tol )
  {
    beta = 0.5*(beta_l+beta_u);
    resid = compute_wilks_residual(order, nsamples, alpha, beta, twosided);
    if( resid*resid_u > 0 ) {
      beta_u = beta;
      resid_u = resid;
    }
    else {
      beta_l = beta;
      resid_l = resid;
    }
    //Cout << "beta = " << beta << ", resid = " << resid << std::endl;
  }
  //Cout << "Converged: beta = " << beta << ", resid = " << resid << std::endl;

  return beta;
}


/** Computes CDF/CCDF based on sample binning.  A PDF is inferred from a
    CDF/CCDF within compute_densities() after level computation. */
void NonDSampling::compute_level_mappings(const IntResponseMap& samples)
{
  // Size the output arrays here instead of in the ctor in order to support
  // alternate sampling ctors.
  initialize_level_mappings();
  archive_allocate_mappings();

  // For the samples array, calculate the following statistics:
  // > CDF/CCDF mappings of response levels to probability/reliability levels
  // > CDF/CCDF mappings of probability/reliability levels to response levels
  size_t i, j, k, num_obs = samples.size(), num_samp, bin_accumulator;
  const StringArray& resp_labels = iteratedModel.response_labels();
  std::multiset<Real> sorted_samples; // STL-based array for sorting
  SizetArray bins; Real min, max, sample;

  // check if moments are required, and if so, compute them now
  if (finalMomentStats.empty()) {
    bool need_moments = false;
    for (i=0; i<numFunctions; ++i)
      if ( !requestedRelLevels[i].empty() ||
	   ( !requestedRespLevels[i].empty() &&
	     respLevelTarget == RELIABILITIES ) )
	{ need_moments = true; break; }
    if (need_moments) {
      Cerr << "Error: required moments not available in compute_distribution_"
	   << "mappings().  Call compute_moments() first." << std::endl;
      abort_handler(-1);
      // Issue with the following approach is that subsequent invocations of
      // compute_level_mappings() without compute_moments() would not be
      // detected and old moments would be used.  Performing more rigorous
      // bookkeeping of moment updates is overkill for current use cases.
      //Cerr << "Warning: moments not available in compute_distribution_"
      //     << "mappings(); computing them now." << std::endl;
      //compute_moments(samples);
    }
  }

  if (pdfOutput) extremeValues.resize(numFunctions);
  IntRespMCIter s_it; std::multiset<Real>::iterator ss_it;
  bool extrapolated_mappings = false;
  for (i=0; i<numFunctions; ++i) {

    // CDF/CCDF mappings: z -> p/beta/beta* and p/beta/beta* -> z
    size_t rl_len = requestedRespLevels[i].length(),
           pl_len = requestedProbLevels[i].length(),
           bl_len = requestedRelLevels[i].length(),
           gl_len = requestedGenRelLevels[i].length();

    // ----------------------------------------------------------------------
    // Preliminaries: define finite subset, sort (if needed), and bin samples
    // ----------------------------------------------------------------------
    num_samp = 0;
    if (pl_len || gl_len) { // sort samples array for p/beta* -> z mappings
      sorted_samples.clear();
      for (s_it=samples.begin(); s_it!=samples.end(); ++s_it) {
        sample = s_it->second.function_value(i);
	if (isfinite(sample))
	  { ++num_samp; sorted_samples.insert(sample); }
      }
      // sort in ascending order
      if (pdfOutput)
        { min = *sorted_samples.begin(); max = *(--sorted_samples.end()); }
      // in case of rl_len mixed with pl_len/gl_len, bin using sorted array.
      if (rl_len && respLevelTarget != RELIABILITIES) {
	const RealVector& req_rl_i = requestedRespLevels[i];
        bins.assign(rl_len+1, 0); ss_it = sorted_samples.begin();
	for (j=0; j<rl_len; ++j)
	  while (ss_it!=sorted_samples.end() && *ss_it <= req_rl_i[j])// p(g<=z)
	    { ++bins[j]; ++ss_it; }
	bins[rl_len] += std::distance(ss_it, sorted_samples.end());
      }
    }
    else if (rl_len && respLevelTarget != RELIABILITIES) {
      // in case of rl_len without pl_len/gl_len, bin from original sample set
      const RealVector& req_rl_i = requestedRespLevels[i];
      bins.assign(rl_len+1, 0); min = DBL_MAX; max = -DBL_MAX;
      for (s_it=samples.begin(); s_it!=samples.end(); ++s_it) {
	sample = s_it->second.function_value(i);
	if (isfinite(sample)) {
	  ++num_samp;
	  if (pdfOutput) {
	    if (sample < min) min = sample;
	    if (sample > max) max = sample;
	  }
	  // 1st PDF bin from -inf to 1st resp lev; last PDF bin from last resp
	  // lev to +inf.
	  bool found = false;
	  for (k=0; k<rl_len; ++k)
	    if (sample <= req_rl_i[k]) // cumulative p(g<=z)
	      { ++bins[k]; found = true; break; }
	  if (!found)
	    ++bins[rl_len];
	}
      }
    }
    if (pdfOutput)
      { extremeValues[i].first = min; extremeValues[i].second = max; }

    // ----------------
    // Process mappings
    // ----------------
    if (rl_len) {
      switch (respLevelTarget) {
      case PROBABILITIES: case GEN_RELIABILITIES: // z -> p/beta* (from binning)
	bin_accumulator = 0;
	for (j=0; j<rl_len; ++j) { // compute CDF/CCDF p/beta*
	  bin_accumulator += bins[j];
	  Real cdf_prob = (Real)bin_accumulator/(Real)num_samp;
	  Real computed_prob = (cdfFlag) ? cdf_prob : 1. - cdf_prob;
	  if (respLevelTarget == PROBABILITIES)
	    computedProbLevels[i][j] = computed_prob;
	  else
	    computedGenRelLevels[i][j]
	      = -Pecos::NormalRandomVariable::inverse_std_cdf(computed_prob);
	}
	break;
      case RELIABILITIES: { // z -> beta (from moment projection)
	Real mean = finalMomentStats(0,i), std_dev = finalMomentStats(1,i);
	for (j=0; j<rl_len; j++) {
	  Real z = requestedRespLevels[i][j];
	  if (std_dev > Pecos::SMALL_NUMBER)
	    computedRelLevels[i][j] = (cdfFlag) ?
	      (mean - z)/std_dev : (z - mean)/std_dev;
	  else
	    computedRelLevels[i][j]
	      = ( (cdfFlag && mean <= z) || (!cdfFlag && mean > z) )
	      ? -Pecos::LARGE_NUMBER : Pecos::LARGE_NUMBER;
	}
	break;
      }
      }
    }
    for (j=0; j<pl_len+gl_len; j++) { // p/beta* -> z
      Real p = (j<pl_len) ? requestedProbLevels[i][j] :	Pecos::
	NormalRandomVariable::std_cdf(-requestedGenRelLevels[i][j-pl_len]);
      Real p_cdf = (cdfFlag) ? p : 1. - p;
      // since each sample has 1/N probability, p can be directly converted
      // to an index within sorted_samples (id = p * N; index = id - 1)
      // Note 1: duplicate samples are not aggregated (separate id increments).
      // Note 2: since p_cdf(min_sample) = 1/N and p_cdf(max_sample) = 1, we
      //   extrapolate to the left of min, but not to the right of max.
      //   id < 1 indicates this extrapolation left of the min sample.
      // Note 3: we exclude any extrapolated z from extremeValues; should we
      //   omit any out-of-bounds resp levels within NonD::compute_densities()?
      //   --> PDF estimation based only on z->p binning or p->z interpolation
      //       within the sample bounds.
      Real cdf_incr_id = p_cdf * (Real)num_samp, lo_id;
      ss_it = sorted_samples.begin();
      if (cdf_incr_id < 1.) { // extrapolate left of min sample using 1st slope
	lo_id = 1.; extrapolated_mappings = true;
	Cerr << "Warning: extrapolation required for response " << i+1;
	if (j<pl_len) Cerr <<    " for probability level " << j+1       <<".\n";
	else Cerr << " for generalized reliability level " << j+1-pl_len<<".\n";
      }
      else { // linear interpolation between closest neighbors in sequence
        lo_id = std::floor(cdf_incr_id);
	std::advance(ss_it, (size_t)lo_id - 1);
      }
      Real z, z_lo = *ss_it; ++ss_it;
      if (ss_it == sorted_samples.end()) z = z_lo;
      else          z = z_lo + (cdf_incr_id - lo_id) * (*ss_it - z_lo);
      if (j<pl_len) computedRespLevels[i][j] = z;
      else          computedRespLevels[i][j+bl_len] = z;
    }
    if (bl_len) {
      Real mean = finalMomentStats(0,i), std_dev = finalMomentStats(1,i);
      for (j=0; j<bl_len; j++) { // beta -> z
	Real beta = requestedRelLevels[i][j];
	computedRespLevels[i][j+pl_len] = (cdfFlag) ?
	  mean - beta * std_dev : mean + beta * std_dev;
      }
    }

    // archive the mappings from response levels
    archive_from_resp(i);
    // archive the mappings to response levels
    archive_to_resp(i);
  }

  if (extrapolated_mappings)
    Cerr << "Warning: extrapolations required to evaluate inverse mappings.  "
	 << "Consistent slope\n         (uniform density) assumed for "
	 << "extrapolation into distribution tail.\n\n";

  // post-process computed z/p/beta* levels to form PDFs (prob_refined and
  // all_levels_computed default to false).  embedding this call within
  // compute_level_mappings() simplifies management of min/max.
  compute_densities(extremeValues);
}


void NonDSampling::update_final_statistics()
{
  if (finalStatistics.is_null()) // some ctor chains do not track final stats
    return;

  if (epistemicStats) {
    size_t i, cntr = 0;
    for (i=0; i<numFunctions; ++i) {
      finalStatistics.function_value(extremeValues[i].first,  cntr++);
      finalStatistics.function_value(extremeValues[i].second, cntr++);
    }
  }
  else // moments + level mappings
    NonD::update_final_statistics();
}


void NonDSampling::print_statistics(std::ostream& s) const
{
  if (epistemicStats) // output only min & max values in the epistemic case
    print_intervals(s);
  else {
    print_moments(s);
    if (totalLevelRequests) {
      print_level_mappings(s);
      print_system_mappings(s);
    }
    if( wilksFlag )
      print_wilks_stastics(s); //, "response function", iteratedModel.response_labels());
  }
  if (!subIteratorFlag) {
    StringMultiArrayConstView
      acv_labels  = iteratedModel.all_continuous_variable_labels(),
      adiv_labels = iteratedModel.all_discrete_int_variable_labels(),
      adsv_labels = iteratedModel.all_discrete_string_variable_labels(),
      adrv_labels = iteratedModel.all_discrete_real_variable_labels();
    size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
      drv_start, num_drv;
    mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
		dsv_start, num_dsv, drv_start, num_drv);
    StringMultiArrayConstView
      cv_labels  =
        acv_labels[boost::indices[idx_range(cv_start, cv_start+num_cv)]],
      div_labels =
        adiv_labels[boost::indices[idx_range(div_start, div_start+num_div)]],
      dsv_labels =
        adsv_labels[boost::indices[idx_range(dsv_start, dsv_start+num_dsv)]],
      drv_labels =
        adrv_labels[boost::indices[idx_range(drv_start, drv_start+num_drv)]];
    nonDSampCorr.print_correlations(s, cv_labels, div_labels, dsv_labels,
				    drv_labels,iteratedModel.response_labels());
  }
}


void NonDSampling::
print_intervals(std::ostream& s, String qoi_type,
		const StringArray& interval_labels) const
{
  s << std::scientific << std::setprecision(write_precision)
    << "\nMin and Max samples for each " << qoi_type << ":\n";
  size_t i, num_qoi = extremeValues.size();
  for (size_t i=0; i<num_qoi; ++i)
    s << interval_labels[i] << ":  Min = " << extremeValues[i].first
      << "  Max = " << extremeValues[i].second << '\n';
}


void NonDSampling::
print_moments(std::ostream& s, const RealMatrix& moment_stats,
	      const RealMatrix moment_cis, String qoi_type, short moments_type,
	      const StringArray& moment_labels, bool print_cis)
{
  size_t i, j, width = write_precision+7, num_moments = moment_stats.numRows(),
    num_qoi = moment_stats.numCols();

  s << "\nSample moment statistics for each " << qoi_type << ":\n"
    << std::scientific << std::setprecision(write_precision)
    << std::setw(width+15) << "Mean";
  if (moments_type == CENTRAL_MOMENTS)
    s << std::setw(width+1) << "Variance" << std::setw(width+1) << "3rdCentral"
      << std::setw(width+2) << "4thCentral\n";
  else
    s << std::setw(width+1) << "Std Dev" << std::setw(width+1)  << "Skewness"
      << std::setw(width+2) << "Kurtosis\n";
  //<< std::setw(width+2)  << "Coeff of Var\n";
  for (i=0; i<num_qoi; ++i) {
    const Real* moments_i = moment_stats[i];
    s << std::setw(14) << moment_labels[i];
    for (j=0; j<num_moments; ++j)
      s << ' ' << std::setw(width) << moments_i[j];
    s << '\n';
  }
  if (print_cis && !moment_cis.empty()) {
    // output 95% confidence intervals as (,) interval
    s << "\n95% confidence intervals for each " << qoi_type << ":\n"
      << std::setw(width+15) << "LowerCI_Mean" << std::setw(width+1)
      << "UpperCI_Mean" << std::setw(width+1);
    if (moments_type == CENTRAL_MOMENTS)
      s << "LowerCI_Variance" << std::setw(width+2) << "UpperCI_Variance\n";
    else
      s << "LowerCI_StdDev"   << std::setw(width+2) << "UpperCI_StdDev\n";
    for (i=0; i<num_qoi; ++i)
      s << std::setw(14) << moment_labels[i]
	<< ' ' << std::setw(width) << moment_cis(0, i)
	<< ' ' << std::setw(width) << moment_cis(1, i)
	<< ' ' << std::setw(width) << moment_cis(2, i)
	<< ' ' << std::setw(width) << moment_cis(3, i) << '\n';
  }
}


void NonDSampling::
print_wilks_stastics(std::ostream& s) const
{
  //std::multiset<Real> sorted_resp;
  //IntRespMCIter it2;
  //std::multiset<Real>::const_iterator cit2;
  //for (int i=0; i<numFunctions; ++i) {
  //  sorted_resp.clear();
  //  for( it2 = allResponses.begin(); it2!=allResponses.end(); ++it2)
  //  {
  //    Cout << "It #" << it2->first << "\t" << it2->second.function_value(i) << std::endl;
  //    sorted_resp.insert(it2->second.function_value(i));
  //  }
  //  int count = 0;
  //  for( it2 = allResponses.begin(), cit2 = sorted_resp.begin(); cit2 != sorted_resp.end(); ++cit2, ++count, ++it2 )
  //    Cout << "It #" << count << "\t" << it2->second.function_value(i) << "\t" << *cit2 << std::endl;
  //}

  bool wilks_twosided = (wilksSidedness == TWO_SIDED);

  size_t j, width = write_precision+7, w2p2 = 2*width+2, w3p4 = 3*width+4;

  std::multiset<Real> sorted_resp_subset;
  std::multiset<Real>::const_iterator cit;
  std::multiset<Real>::const_reverse_iterator crit;
  IntRespMCIter it;
  int n;
  Real min, max;

  for (size_t fn_index=0; fn_index<numFunctions; ++fn_index) {
    s << "\n\n" << "Wilks Statistics for "
      << (wilks_twosided ? "Two-" : "One-") << "Sided "
      << 100.0*wilksBeta << "% Confidence Level, Order = " << wilksOrder 
      << " for "  << iteratedModel.response_labels()[fn_index] << ":\n\n";

    std::string one_sided_bound_label = (wilksSidedness == ONE_SIDED_UPPER ? "Upper" : "Lower");
    s << "    Coverage Level     " << (wilks_twosided ? "Lower Bound      " : "")  << "  " << one_sided_bound_label << " Bound     Number of Samples"
      << "\n    --------------  "  << (wilks_twosided ? "----------------- " : "") << " -----------------  ----------------- \n";

    // Create a default probability level if none given
    RealVector prob_levels;
    size_t num_prob_levels = requestedProbLevels[fn_index].length();
    if( 0 == num_prob_levels ) {
      num_prob_levels = 1;
      prob_levels.resize(1);
      prob_levels[0] = 0.95; // default probability level
    }
    else
      prob_levels = requestedProbLevels[fn_index];

    for (j=0; j<num_prob_levels; ++j)
    {
      Real prob_level = prob_levels[j];
      int num_samples = compute_wilks_sample_size(wilksOrder, prob_level, wilksBeta, wilks_twosided);

      // Grab the first num_samples subset in sorted order (could also randomly sample) - RWH
      sorted_resp_subset.clear();
      for (n=0, it=allResponses.begin(); n<num_samples; ++n, ++it)
      {
        Real sample = it->second.function_value(fn_index);
        if (isfinite(sample)) // neither NaN nor +/-Inf
          sorted_resp_subset.insert(sample);
      }
      cit = sorted_resp_subset.begin();
      crit = sorted_resp_subset.rbegin();
      for( int i=0; i<wilksOrder-1; ++i, ++cit, ++crit ) continue;
      min = *cit;
      max = *crit;

      s << "  " << std::setw(width) << prob_levels[j];
      if( wilks_twosided )
        s << "  " << min;
      s << "   " << ( (wilks_twosided || wilksSidedness == ONE_SIDED_UPPER) ? max : min )
        << "        " << num_samples
        << '\n';
    }
  }
}

} // namespace Dakota
