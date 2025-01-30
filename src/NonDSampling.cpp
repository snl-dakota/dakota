/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_data_types.hpp"
#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"
#include "model_utils.hpp"
#include "DakotaResponse.hpp"
#include "DigitalNet.hpp"
#include "LDDriverAdapter.hpp"
#include "LHSDriverAdapter.hpp"
#include "NonDSampling.hpp"
#include "ProblemDescDB.hpp"
#include "Rank1Lattice.hpp"
#include "SamplerDriver.hpp"
#include "SensAnalysisGlobal.hpp"
#include "ProbabilityTransformation.hpp"
#include "dakota_stat_util.hpp"
#include "pecos_data_types.hpp"
#include "NormalRandomVariable.hpp"
#include "tolerance_intervals.hpp"
#include <algorithm>

#include <boost/math/special_functions/beta.hpp>

static const char rcsId[]="@(#) $Id: NonDSampling.cpp 7036 2010-10-22 23:20:24Z mseldre $";


namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called and
    probDescDB can be queried for settings from the method specification. */
NonDSampling::NonDSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonD(problem_db, model), seedSpec(probDescDB.get_int("method.random_seed")),
  randomSeed(seedSpec), samplesSpec(probDescDB.get_int("method.samples")),
  samplesRef(samplesSpec), numSamples(samplesSpec),
  rngName(probDescDB.get_string("method.random_number_generator")),
  sampleType(probDescDB.get_ushort("method.sample_type")), samplesIncrement(0),
  stdRegressionCoeffs(probDescDB.get_bool("method.std_regression_coeffs")),
  toleranceIntervalsFlag(probDescDB.get_bool("method.tolerance_intervals")),
  statsFlag(true), allDataFlag(false), samplingVarsMode(ACTIVE),
  sampleRanksMode(IGNORE_RANKS),
  varyPattern(!probDescDB.get_bool("method.fixed_seed")), 
  backfillDuplicates(probDescDB.get_bool("method.backfill")),
  wilksFlag(probDescDB.get_bool("method.wilks")), numLHSRuns(0),
  samplerDriver(
    ( problem_db.get_ushort("method.sample_type") == SUBMETHOD_LOW_DISCREPANCY_SAMPLING ) ?
    std::unique_ptr<SamplerDriver>(std::make_unique<LDDriverAdapter>(problem_db)) :
    std::unique_ptr<SamplerDriver>(std::make_unique<LHSDriverAdapter>()) )
{
  // pushed down as some derived classes (MLMC) use a MC default
  //if (!sampleType)
  //  sampleType = SUBMETHOD_LHS;

  if (epistemicStats && totalLevelRequests) {
    Cerr << "\nError: sampling does not support level requests for "
	 << "analyses containing epistemic uncertainties." << std::endl;
    abort_handler(METHOD_ERROR);
  }

#ifndef HAVE_DAKOTA_SURROGATES
  if ( stdRegressionCoeffs ) {
    Cerr << "Warning: Standardized Regression Coefficients are not available"
         << " for Dakota builds without the surrogates module enabled."
         << " Disabling requested output.\n";
  }
#endif

  if ( wilksFlag ) {
    // Only works with sample_type of random
    // BMA: consider relaxing, despite no theory
    if ( sampleType != SUBMETHOD_RANDOM ) {
      Cerr << "Error: Wilks sample sizes require use of \"random\" sample_type."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }

    // Check for conflicting samples spec. Note that this still allows
    // a user to specify "samples = 0" alongside wilks
    if ( numSamples > 0 ) { 
      Cerr << "Error: Cannot specify both \"samples\" and \"wilks\"."
	   << std::endl;
      abort_handler(METHOD_ERROR);
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
    numSamples = compute_wilks_sample_size(wilksOrder, wilksAlpha,
					   wilksBeta, wilks_twosided);
    samplesRef = numSamples;
  }

  if (toleranceIntervalsFlag) {
    tiCoverage = probDescDB.get_real("method.ti_coverage");
    tiConfidenceLevel = probDescDB.get_real("method.ti_confidence_level");

    tiNumValidSamples = 0;
  }

  // update concurrency
  if (numSamples) // samples is optional (default = 0)
    maxEvalConcurrency *= numSamples;
}


/** This alternate constructor is used for generation and evaluation
    of on-the-fly sample sets. */
NonDSampling::
NonDSampling(unsigned short method_name, std::shared_ptr<Model> model,
	     unsigned short sample_type, size_t samples, int seed,
	     const String& rng, bool vary_pattern, short sampling_vars_mode):
  NonD(method_name, model), seedSpec(seed), randomSeed(seed),
  samplesSpec(samples), samplesRef(samples), numSamples(samples), rngName(rng),
  sampleType(sample_type), wilksFlag(false), samplesIncrement(0), stdRegressionCoeffs(false),
  toleranceIntervalsFlag(false),
  statsFlag(false), allDataFlag(true), samplingVarsMode(sampling_vars_mode),
  sampleRanksMode(IGNORE_RANKS), varyPattern(vary_pattern),
  backfillDuplicates(false), numLHSRuns(0),
  samplerDriver(std::make_unique<LHSDriverAdapter>())
{
  subIteratorFlag = true; // suppress some output

  // override default epistemicStats setting from NonD ctor
  const Variables& vars = iteratedModel->current_variables();
  const SizetArray& ac_totals = vars.shared_data().active_components_totals();
  bool euv = (ac_totals[TOTAL_CEUV]  || ac_totals[TOTAL_DEUIV] ||
	      ac_totals[TOTAL_DEUSV] || ac_totals[TOTAL_DEURV]);
  bool aleatory_mode = (samplingVarsMode == ALEATORY_UNCERTAIN ||
			samplingVarsMode == ALEATORY_UNCERTAIN_UNIFORM);
  epistemicStats = (euv && !aleatory_mode);

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
NonDSampling(unsigned short sample_type, size_t samples, int seed,
	     const String& rng, const RealVector& lower_bnds,
	     const RealVector& upper_bnds):
  NonD(RANDOM_SAMPLING, lower_bnds, upper_bnds), seedSpec(seed),
  randomSeed(seed), samplesSpec(samples), samplesRef(samples),
  numSamples(samples), rngName(rng), sampleType(sample_type), wilksFlag(false),
  samplesIncrement(0), stdRegressionCoeffs(false),
  toleranceIntervalsFlag(false),
  statsFlag(false), allDataFlag(true),
  samplingVarsMode(ACTIVE_UNIFORM), sampleRanksMode(IGNORE_RANKS),
  varyPattern(true), backfillDuplicates(false), numLHSRuns(0),
  samplerDriver(std::make_unique<LHSDriverAdapter>())
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
NonDSampling(unsigned short sample_type, size_t samples, int seed,
	     const String& rng, const RealVector& means, 
             const RealVector& std_devs, const RealVector& lower_bnds,
	     const RealVector& upper_bnds, RealSymMatrix& correl):
  NonD(RANDOM_SAMPLING, lower_bnds, upper_bnds), seedSpec(seed),
  randomSeed(seed), samplesSpec(samples), samplesRef(samples),
  numSamples(samples), rngName(rng), sampleType(sample_type), wilksFlag(false),
  samplesIncrement(0), stdRegressionCoeffs(false),
  toleranceIntervalsFlag(false),
  statsFlag(false), allDataFlag(true),
  samplingVarsMode(ACTIVE), sampleRanksMode(IGNORE_RANKS), varyPattern(true),
  backfillDuplicates(false), numLHSRuns(0),
  samplerDriver(std::make_unique<LHSDriverAdapter>())
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
NonDSampling(std::shared_ptr<Model> model, const RealMatrix& sample_matrix):
  NonD(LIST_SAMPLING, model), seedSpec(0), randomSeed(0),
  samplesSpec(sample_matrix.numCols()), sampleType(SUBMETHOD_DEFAULT),
  wilksFlag(false), samplesIncrement(0), stdRegressionCoeffs(false),
  toleranceIntervalsFlag(false),
  statsFlag(true), allDataFlag(true),
  samplingVarsMode(ACTIVE), sampleRanksMode(IGNORE_RANKS),
  varyPattern(false), backfillDuplicates(false), numLHSRuns(0),
  samplerDriver(std::make_unique<LHSDriverAdapter>())
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


void NonDSampling::
transform_samples(Pecos::ProbabilityTransformation& nataf,
		  RealMatrix& sample_matrix, //size_t num_samples,
		  SizetMultiArrayConstView src_cv_ids,
		  SizetMultiArrayConstView tgt_cv_ids, bool x_to_u)
{
  // Since transform updates in place, we must not alter sample_matrix
  //if (num_samples == 0)
  //  num_samples = sample_matrix.numCols();
  //else if (sample_matrix.numCols() != num_samples)
  //  sample_matrix.shapeUninitialized(numContinuousVars, num_samples);

  size_t num_samples = sample_matrix.numCols();
  if (x_to_u)
    for (size_t i=0; i<num_samples; ++i) {
      RealVector x_samp(Teuchos::Copy, sample_matrix[i], numContinuousVars);
      RealVector u_samp(Teuchos::View, sample_matrix[i], numContinuousVars);
      nataf.trans_X_to_U(x_samp, src_cv_ids, u_samp, tgt_cv_ids);
    }
  else
    for (size_t i=0; i<num_samples; ++i) {
      RealVector u_samp(Teuchos::Copy, sample_matrix[i], numContinuousVars);
      RealVector x_samp(Teuchos::View, sample_matrix[i], numContinuousVars);
      nataf.trans_U_to_X(u_samp, src_cv_ids, x_samp, tgt_cv_ids);
    }
}


void NonDSampling::
get_parameter_sets(std::shared_ptr<Model> model, const size_t num_samples,
		   RealMatrix& design_matrix, bool write_msg)
{
  initialize_sample_driver(write_msg, num_samples);

  // Invoke LHSDriver to generate samples within the specified distributions

  switch (samplingVarsMode) {
  case ACTIVE_UNIFORM:  case ALL_UNIFORM:  case UNCERTAIN_UNIFORM:
  case ALEATORY_UNCERTAIN_UNIFORM:  case EPISTEMIC_UNCERTAIN_UNIFORM: {
    short model_view = model->current_variables().view().first;
    // Use LHSDriver::generate_uniform_samples() between lower/upper bounds
    RealSymMatrix corr; // uncorrelated samples
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
      samplerDriver->generate_uniform_samples(ModelUtils::continuous_lower_bounds(*model),
					 ModelUtils::continuous_upper_bounds(*model),
					 corr, num_samples, design_matrix); 
    }
    else if (samplingVarsMode == ALL_UNIFORM) {
      // sample uniformly from ALL lower/upper bnds with model in distinct view.
      // loss of sampleRanks control is OK since NonDIncremLHS uses ACTIVE mode.
      // TO DO: add support for uniform discrete
      samplerDriver->generate_uniform_samples(ModelUtils::all_continuous_lower_bounds(*model),
					 ModelUtils::all_continuous_upper_bounds(*model),
					 corr, num_samples, design_matrix); 
    }
    else { // A, E, A+E UNCERTAIN_UNIFORM
      // sample uniformly from {A,E,A+E} UNCERTAIN lower/upper bounds
      // with model using a non-corresponding view (corresponding views
      // handled in first case above)
      size_t start_acv, num_acv, dummy;
      mode_counts(model->current_variables(), start_acv, num_acv,
		  dummy, dummy, dummy, dummy, dummy, dummy);
      if (!num_acv) {
	Cerr << "Error: no active continuous variables for sampling in "
	     << "uniform mode" << std::endl;
	abort_handler(METHOD_ERROR);
      }
      const RealVector& all_c_l_bnds = ModelUtils::all_continuous_lower_bounds(*model);
      const RealVector& all_c_u_bnds = ModelUtils::all_continuous_upper_bounds(*model);
      RealVector uncertain_c_l_bnds(Teuchos::View,
	const_cast<Real*>(&all_c_l_bnds[start_acv]), num_acv);
      RealVector uncertain_c_u_bnds(Teuchos::View,
	const_cast<Real*>(&all_c_u_bnds[start_acv]), num_acv);
      // loss of sampleRanks control is OK since NonDIncremLHS uses ACTIVE mode
      // TO DO: add support for uniform discrete
      samplerDriver->generate_uniform_samples(uncertain_c_l_bnds, uncertain_c_u_bnds,
					 corr, num_samples, design_matrix); 
    }
    break;
  }

  case ACTIVE: { // utilize model view to sample active variables
    Pecos::MultivariateDistribution& mv_dist
      = model->multivariate_distribution();
    if (backfillDuplicates)
      samplerDriver->generate_unique_samples(model, num_samples, design_matrix,
        sampleRanks, mv_dist.active_variables(), mv_dist.active_correlations());
      // sampleRanks remains empty. See LHSDriver::generate_unique_samples()
    else
      samplerDriver->generate_samples(model, num_samples, design_matrix,
        sampleRanks, mv_dist.active_variables(), mv_dist.active_correlations());
    break;
  }

  case DESIGN: case ALEATORY_UNCERTAIN: case EPISTEMIC_UNCERTAIN:
  case UNCERTAIN: case STATE: case ALL: {
    // override active model view to sample alternate subset/superset
    BitArray active_vars, active_corr;
    mode_bits(model->current_variables(), active_vars, active_corr);
    Pecos::MultivariateDistribution& mv_dist
      = model->multivariate_distribution();
    if (backfillDuplicates)
      samplerDriver->generate_unique_samples(model, num_samples, design_matrix,
        sampleRanks, active_vars, active_corr);
      // sampleRanks remains empty. See LHSDriver::generate_unique_samples()
    else
      samplerDriver->generate_samples(model, num_samples, design_matrix,
        sampleRanks, active_vars, active_corr);
    break;
  }
  }
}


/** This version of get_parameter_sets() does not extract data from the
    user-defined model, but instead relies on the incoming bounded region
    definition.  It only support a UNIFORM sampling mode, where the
    distinction of ACTIVE_UNIFORM vs. ALL_UNIFORM is handled elsewhere. */
void NonDSampling::
get_parameter_sets(const RealVector& lower_bnds, const RealVector& upper_bnds)
{
  initialize_sample_driver(true, numSamples);
  RealSymMatrix correl; // uncorrelated
  samplerDriver->generate_uniform_samples(lower_bnds, upper_bnds, correl,
				     numSamples, allSamples);
}


/** This version of get_parameter_sets() does not extract data from the
    user-defined model, but instead relies on the incoming 
    definition.  It only supports the sampling of normal variables. */
void NonDSampling::
get_parameter_sets(const RealVector& means, const RealVector& std_devs, 
                   const RealVector& lower_bnds, const RealVector& upper_bnds,
                   RealSymMatrix& correl)
{
  initialize_sample_driver(true, numSamples);
  samplerDriver->generate_normal_samples(means, std_devs, lower_bnds, upper_bnds,
				    correl, numSamples, allSamples);
}


// Map the active variables from vars to sample_vars (a column in allSamples)
void NonDSampling::
sample_to_variables(const Real* sample_vars, Variables& vars, Model& model)
{
  // sample_vars are in RandomVariable order, which mirrors the XML spec
  // vars updates utilize all continuous,discrete {int,string,real} indexing

  if (vars.is_null())
    vars = Variables(model.current_variables().shared_data());

  const SharedVariablesData& svd = vars.shared_data();
  short vars_mode;
  switch (samplingVarsMode) {
  case ACTIVE:
    switch (vars.view().first) {
    case RELAXED_ALL:       case MIXED_ALL:       vars_mode = ALL;        break;
    case RELAXED_DESIGN:    case MIXED_DESIGN:    vars_mode = DESIGN;     break;
    case RELAXED_UNCERTAIN: case MIXED_UNCERTAIN: vars_mode = UNCERTAIN;  break;
    case RELAXED_ALEATORY_UNCERTAIN:   case MIXED_ALEATORY_UNCERTAIN:
      vars_mode = ALEATORY_UNCERTAIN;                                     break;
    case RELAXED_EPISTEMIC_UNCERTAIN:  case MIXED_EPISTEMIC_UNCERTAIN:
      vars_mode = EPISTEMIC_UNCERTAIN;                                    break;
    case RELAXED_STATE:     case MIXED_STATE:     vars_mode = STATE;      break;
    }
    break;
  case ACTIVE_UNIFORM:
    switch (vars.view().first) {
    case RELAXED_ALL:     case MIXED_ALL:    vars_mode = ALL_UNIFORM;     break;
    case RELAXED_DESIGN:  case MIXED_DESIGN:
      vars_mode = DESIGN/*_UNIFORM*/;                                     break;
    case RELAXED_UNCERTAIN:            case MIXED_UNCERTAIN:
      vars_mode = UNCERTAIN_UNIFORM;                                      break;
    case RELAXED_ALEATORY_UNCERTAIN:   case MIXED_ALEATORY_UNCERTAIN:
      vars_mode = ALEATORY_UNCERTAIN_UNIFORM;                             break;
    case RELAXED_EPISTEMIC_UNCERTAIN:  case MIXED_EPISTEMIC_UNCERTAIN:
      vars_mode = EPISTEMIC_UNCERTAIN_UNIFORM;                            break;
    case RELAXED_STATE:                case MIXED_STATE:
      vars_mode = STATE/*_UNIFORM*/;                                      break;
    }
    break;
  default:  vars_mode = samplingVarsMode;                                 break;
  }

  size_t cv_index, num_cv, div_index, num_div, dsv_index, num_dsv,
    drv_index, num_drv, samp_index = 0;
  switch (vars_mode) {
  case DESIGN:
    cv_index = div_index = dsv_index = drv_index = 0;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    break;
  //case DESIGN_UNIFORM:
  //  cv_index = div_index = dsv_index = drv_index = 0;
  //  svd.design_counts(num_cv, num_div, num_dsv, num_drv);
  //  sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
  // 		        dsv_index, num_dsv, drv_index, num_drv, samp_index);
  //  break;
  case ALEATORY_UNCERTAIN:
    // design vars define starting indices
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    // aleatory uncertain vars define counts
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    break;
  case ALEATORY_UNCERTAIN_UNIFORM:
    // continuous design vars define starting indices
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    // continuous aleatory uncertain vars define counts
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    break;
  case EPISTEMIC_UNCERTAIN:
    // design + aleatory uncertain vars define starting indices
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_index  += num_cv;   div_index += num_div;
    dsv_index += num_dsv;  drv_index += num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    break;
  case EPISTEMIC_UNCERTAIN_UNIFORM:
    // continuous design + aleatory uncertain vars define starting indices
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_index  += num_cv;   div_index += num_div;
    dsv_index += num_dsv;  drv_index += num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    break;
  case UNCERTAIN:
    // aleatory
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    // epistemic
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    break;
  case UNCERTAIN_UNIFORM:
    // aleatory
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    // epistemic
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    break;
  case STATE:
    svd.design_counts(cv_index, div_index, dsv_index, drv_index);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_index  += num_cv;  div_index += num_div;
    dsv_index += num_dsv; drv_index += num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_index  += num_cv;  div_index += num_div;
    dsv_index += num_dsv; drv_index += num_drv;
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    break;
  //case STATE_UNIFORM:
  //  svd.design_counts(cv_index, div_index, dsv_index, drv_index);
  //  svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
  //  cv_index  += num_cv;  div_index += num_div;
  //  dsv_index += num_dsv; drv_index += num_drv;
  //  svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
  //  cv_index  += num_cv;  div_index += num_div;
  //  dsv_index += num_dsv; drv_index += num_drv;
  //  svd.state_counts(num_cv, num_div, num_dsv, num_drv);
  //  sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
  //	      dsv_index, num_dsv, drv_index, num_drv, samp_index);
  //  break;
  case ALL:
    // design
    cv_index = div_index = dsv_index = drv_index = 0;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    // aleatory
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    // epistemic
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    // state
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		   dsv_index, num_dsv, drv_index, num_drv, samp_index, model);
    break;
  case ALL_UNIFORM:
    // design
    cv_index = div_index = dsv_index = drv_index = 0;
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    // aleatory
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    // epistemic
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    // state
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    sample_to_cv_type(sample_vars, vars, cv_index, num_cv, div_index, num_div,
		      dsv_index, num_dsv, drv_index, num_drv, samp_index);
    break;
  }
}


/*
// TO DO: avoid inefficiency from repeated lookups...
void NonDSampling::
sample_to_variables(const Real* sample_vars, Variables& vars, size_t acv_start,
		    size_t num_acv, size_t adiv_start, size_t num_adiv,
		    size_t adsv_start, size_t num_adsv, size_t adrv_start,
		    size_t num_adrv, const StringSetArray& dss_values)
{
  const SharedVariablesData& svd = vars.shared_data();

  // sampled continuous vars (by value)
  size_t i, end = acv_start + num_acv;
  for (i=acv_start; i<end; ++i)
    vars.all_continuous_variable(sample_vars[svd.acv_index_to_all_index(i)], i);
  // sampled discrete int vars (by value cast from Real)
  end = adiv_start + num_adiv;
  for (i=adiv_start; i<end; ++i)
    vars.all_discrete_int_variable(
      (int)sample_vars[svd.adiv_index_to_all_index(i)], i);
  // sampled discrete string vars (by index cast from Real)
  end = adsv_start + num_adsv;
  for (i=adsv_start; i<end; ++i)
    vars.all_discrete_string_variable(set_index_to_value(
      (size_t)sample_vars[svd.adsv_index_to_all_index(i)], dss_values[i]),i);
  // sampled discrete real vars (by value)
  end = adrv_start + num_adrv;
  for (i=adrv_start; i<end; ++i)
    vars.all_discrete_real_variable(
      sample_vars[svd.adrv_index_to_all_index(i)], i);
}
*/


// Some inefficiency is acceptable here, as this is for one-time imports,
// so retain the compact approach of per-index conversion for now
void NonDSampling::
variables_to_sample(const Variables& vars, Real* sample_vars)
{
  size_t acv_start, num_acv, adiv_start, num_adiv, adsv_start, num_adsv,
    adrv_start, num_adrv;
  mode_counts(vars, acv_start, num_acv, adiv_start, num_adiv,
	      adsv_start, num_adsv, adrv_start, num_adrv);

  const SharedVariablesData& svd = vars.shared_data();

  // sampled continuous vars (by value)
  size_t i, end = acv_start + num_acv;
  for (i=acv_start; i<end; ++i)
    sample_vars[svd.acv_index_to_all_index(i)]
      = vars.all_continuous_variables()[i];
  // sampled discrete int vars (cast value to Real)
  end = adiv_start + num_adiv;
  for (i=adiv_start; i<end; ++i)
    sample_vars[svd.adiv_index_to_all_index(i)]
      = (Real)vars.all_discrete_int_variables()[i];
  // sampled discrete string vars (cast index to Real)
  if (num_adsv) { // incur overhead of extracting DSS values
    short active_view = vars.view().first;
    bool relax = (active_view == RELAXED_ALL ||
      ( active_view >= RELAXED_DESIGN && active_view <= RELAXED_STATE ) );
    short all_view = (relax) ? RELAXED_ALL : MIXED_ALL;
    const StringSetArray& dss_values
      = ModelUtils::discrete_set_string_values(*iteratedModel, all_view);
    end = adsv_start + num_adsv;
    for (i=adsv_start; i<end; ++i)
      sample_vars[svd.adsv_index_to_all_index(i)] = (Real)set_value_to_index(
	vars.all_discrete_string_variables()[i], dss_values[i]);
  }
  // sampled discrete real vars (by value)
  end = adrv_start + num_adrv;
  for (i=adrv_start; i<end; ++i)
    sample_vars[svd.adrv_index_to_all_index(i)]
      = vars.all_discrete_real_variables()[i];
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
mode_counts(const Variables& vars, size_t& cv_start,  size_t& num_cv,
	    size_t& div_start,     size_t& num_div,   size_t& dsv_start,
	    size_t& num_dsv,       size_t& drv_start, size_t& num_drv) const
{
  cv_start = div_start = dsv_start = drv_start = 0;
  num_cv   = num_div   = num_dsv   = num_drv   = 0;
  const SharedVariablesData& svd = vars.shared_data();
  size_t dummy;
  // Note: UNIFORM views do not currently support non-relaxed discrete
  switch (samplingVarsMode) {
  case DESIGN:
    // design vars define counts
    svd.design_counts(num_cv, num_div, num_dsv, num_drv);
    break;
  //case DESIGN_UNIFORM:
  //  // continuous design vars define counts
  //  svd.design_counts(num_cv, dummy, dummy, dummy);
  //  break;
  case ALEATORY_UNCERTAIN:
    // design vars define starting indices
    svd.design_counts(cv_start, div_start, dsv_start, drv_start);
    // aleatory uncertain vars define counts
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    break;
  case ALEATORY_UNCERTAIN_UNIFORM:
    // continuous design vars define starting indices
    svd.design_counts(cv_start, dummy, dummy, dummy);
    // continuous aleatory uncertain vars define counts
    svd.aleatory_uncertain_counts(num_cv, dummy, dummy, dummy);
    break;
  case EPISTEMIC_UNCERTAIN:
    // design + aleatory uncertain vars define starting indices
    svd.design_counts(cv_start, div_start, dsv_start, drv_start);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_start  += num_cv;  div_start += num_div;
    dsv_start += num_dsv; drv_start += num_drv;
    // epistemic uncertain vars define counts
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    break;
  case EPISTEMIC_UNCERTAIN_UNIFORM:
    // continuous design + aleatory uncertain vars define starting indices
    svd.design_counts(cv_start, dummy, dummy, dummy);
    svd.aleatory_uncertain_counts(num_cv, dummy, dummy, dummy);
    cv_start += num_cv;
    // continuous epistemic uncertain vars define counts
    svd.epistemic_uncertain_counts(num_cv, dummy, dummy, dummy);
    break;
  case UNCERTAIN:
    // design vars define starting indices
    svd.design_counts(cv_start, div_start, dsv_start, drv_start);
    // aleatory+epistemic uncertain vars define counts
    svd.uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    break;
  case UNCERTAIN_UNIFORM:
    // continuous design vars define starting indices
    svd.design_counts(cv_start, dummy, dummy, dummy);
    // continuous aleatory+epistemic uncertain vars define counts
    svd.uncertain_counts(num_cv, dummy, dummy, dummy);
    break;
  case STATE:
    // design + aleatory + epistemic vars define starting indices
    svd.design_counts(cv_start, div_start, dsv_start, drv_start);
    svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_start  += num_cv;  div_start += num_div;
    dsv_start += num_dsv; drv_start += num_drv;
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    cv_start  += num_cv;  div_start += num_div;
    dsv_start += num_dsv; drv_start += num_drv;
    // state vars define counts
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    break;
  //case STATE_UNIFORM:
  //  // continuous design + aleatory + epistemic vars define starting indices
  //  svd.design_counts(cv_start, dummy, dummy, dummy);
  //  svd.aleatory_uncertain_counts(num_cv, dummy, dummy, dummy);
  //  cv_start += num_cv;
  //  svd.epistemic_uncertain_counts(num_cv, dummy, dummy, dummy);
  //  cv_start += num_cv;
  //  // continuous state vars define counts
  //  svd.state_counts(num_cv, dummy, dummy, dummy);
  //  break;
  case ACTIVE:
    cv_start  = vars.cv_start();  num_cv  = vars.cv();
    div_start = vars.div_start(); num_div = vars.div();
    dsv_start = vars.dsv_start(); num_dsv = vars.dsv();
    drv_start = vars.drv_start(); num_drv = vars.drv();   break;
  case ACTIVE_UNIFORM:
    cv_start  = vars.cv_start();  num_cv  = vars.cv();    break;
  case ALL:
    num_cv    = vars.acv();       num_div = vars.adiv();
    num_dsv   = vars.adsv();      num_drv = vars.adrv();  break;
  case ALL_UNIFORM:
    num_cv    = vars.acv();                               break;
  }
}


void NonDSampling::
mode_bits(const Variables& vars, BitArray& active_vars,
	  BitArray& active_corr) const
{
  const SharedVariablesData& svd = vars.shared_data();
  size_t num_v = vars.tv(), num_cv, num_div, num_dsv, num_drv,
    num_dv, num_auv, num_euv;
  svd.design_counts(num_cv, num_div, num_dsv, num_drv);
  num_dv = num_cv + num_div + num_dsv + num_drv,
  svd.aleatory_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
  num_auv = num_cv + num_div + num_dsv + num_drv;
  active_corr.resize(num_v, false);
  assign_value(active_corr, true, num_dv, num_auv);

  switch (samplingVarsMode) {
  case DESIGN:
    active_vars.resize(num_v, false);
    assign_value(active_vars, true, 0, num_dv);  break;
  case ALEATORY_UNCERTAIN:
    active_vars = active_corr;  break;
  case EPISTEMIC_UNCERTAIN:
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    active_vars.resize(num_v, false);
    assign_value(active_vars, true, num_dv + num_auv,
		 num_cv + num_div + num_dsv + num_drv);
    break;
  case UNCERTAIN:
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    active_vars.resize(num_v, false);
    assign_value(active_vars, true, num_dv,
		 num_auv + num_cv + num_div + num_dsv + num_drv);
    break;
  case STATE:
    svd.epistemic_uncertain_counts(num_cv, num_div, num_dsv, num_drv);
    num_euv = num_cv + num_div + num_dsv + num_drv;
    svd.state_counts(num_cv, num_div, num_dsv, num_drv);
    active_vars.resize(num_v, false);
    assign_value(active_vars, true, num_dv + num_auv + num_euv,
		 num_cv + num_div + num_dsv + num_drv);
    break;
  case ALL:
    active_vars.clear(); break; // no subset, all are active
  //case ALEATORY_UNCERTAIN_UNIFORM:
  //case EPISTEMIC_UNCERTAIN_UNIFORM:
  //case UNCERTAIN_UNIFORM:
  default:
    Cerr << "Error: unsupported sampling mode in NonDSampling::mode_bits()."
	 << std::endl;
    abort_handler(METHOD_ERROR);  break;
  }
}


void NonDSampling::
initialize_sample_driver(bool write_message, size_t num_samples)
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout <<  "Initialize: numLHSRuns = " << numLHSRuns << " seedSpec = "
	 << seedSpec << " randomSeed = " << randomSeed << " varyPattern = "
	 << varyPattern << std::endl;

  // Set seed value for input to LHS's RNG: a user-specified seed gives
  // repeatable behavior but no specification gives random behavior based on
  // querying a system clock.  For cases where get_parameter_sets() may be
  // called multiple times for the same sampling iterator (e.g., SBO), support
  // a deterministic sequence of seed values to allow the sampling pattern to
  // vary from one run to the next in a repeatable manner (required for RNG
  // without a persistent state, such as rnum2).
  bool seed_assigned = false, seed_advanced = false;
  if (numLHSRuns == 0) { // set initial seed
    samplerDriver->rng(rngName);
    if (!seedSpec) { // no user specification --> nonrepeatable behavior
      // Generate initial seed from a system clock.  NOTE: the system clock
      // should not used for multiple LHS calls since (1) clock granularity can
      // be too coarse (can repeat on subsequent runs for inexpensive test fns)
      // and (2) seed progression can be highly structured, which could induce
      // correlation between sample sets.  Instead, the clock-generated case
      // varies the seed below using the same approach as the user-specified
      // case.  This has the additional benefit that a random run can be
      // recreated by specifying the clock-generated seed in the input file.
      randomSeed = generate_system_seed();
    }
    samplerDriver->seed(randomSeed);  seed_assigned = true;
    seed_advanced = seed_updated(); // track seedIndex for ensemble samplers
  }
  // We must distinguish two advancement use cases and allow them to co-exist:
  // > an update to NonDSampling::randomSeed due to random_seed_sequence spec
  // > an update to Pecos::LHSDriver::randomSeed using LHSDriver::
  //   advance_seed_sequence() in support of varyPattern for rnum2
  else if (seed_updated()) // random_seed_sequence advance
    { seedSpec = randomSeed; samplerDriver->seed(randomSeed); seed_assigned = true; }
  else if (varyPattern && rngName == "rnum2") // vary pattern by advancing seed
    { samplerDriver->advance_seed_sequence();                 seed_advanced = true; }
  else if (!varyPattern) // reset orig / machine-generated (don't continue RNG)
    { samplerDriver->seed(randomSeed);                        seed_assigned = true; }

  // Needed a way to turn this off when LHS sampling is being used in
  // NonDAdaptImpSampling because it gets written a _LOT_
  String sample_string = submethod_enum_to_string(sampleType);
  if (write_message) {
    Cout << "\nNonD " << sample_string << " Samples = " << num_samples;
    if (seed_assigned) {
      if (seedSpec) Cout << " Seed (user-specified) = ";
      else          Cout << " Seed (system-generated) = ";
      Cout << randomSeed << '\n';
    }
    else if (seed_advanced) {
      if (seedSpec) Cout << " Seed (sequence from user-specified) = ";
      else          Cout << " Seed (sequence from system-generated) = ";
      Cout << samplerDriver->seed() << '\n';
    }
    else // default Boost Mersenne twister
      Cout << " Seed not reset from previous LHS execution\n";
  }

  samplerDriver->initialize(sample_string, sampleRanksMode, !subIteratorFlag);
  ++numLHSRuns;
}


/** Map ASV/DVV requests in final statistics into activeSet for use in
    evaluate_parameter_sets() */
void NonDSampling::active_set_mapping()
{
  // Adapted from NonDExpansion::compute_expansion()

  // Note: the ASV within finalStatistics corresponds to the stats vector,
  // not the QoI vector, but the DVV carries over.
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  size_t num_final_stats = final_asv.size();
  if (!num_final_stats) // finalStatistics not initialized
    return;             // leave activeSet as is; nothing to augment

  // activeSet ASV/DVV can include active-variable derivatives for surrogate
  // creation (use_derivatives spec option).  Cannot easily support both this
  // and moment gradients w.r.t. inactive variables without new logic for an
  // aggregate DVV; however, the former case occurs in DataFitSurrModel
  // contexts and the latter case occurs in NestedModel contexts:
  // > for now, augment the incoming ASV (preserve the DataFitSurrModel case)
  //   with any moment grad requirements and only overwrite the incoming DVV
  //   when moment gradients are required (support the NestedModel case).
  // > Model recursions that embed a derivative-enhanced DataFitSurrModel
  //   within a NestedModel may dictate additional care...
  ShortArray sampler_asv = activeSet.request_vector();//(numFunctions, 0);
  bool assign_dvv = false, qoi_fn, qoi_grad, moment1_grad, moment2_grad;
  size_t i, j, rl_len, pl_len, bl_len, gl_len, total_i, cntr = 0,
    moment_offset = (finalMomentsType) ? 2 : 0;
  for (i=0; i<numFunctions; ++i) {

    if (totalLevelRequests) {
      rl_len = requestedRespLevels[i].length();
      pl_len = requestedProbLevels[i].length();
      bl_len = requestedRelLevels[i].length();
      gl_len = requestedGenRelLevels[i].length();
    }
    else // requested level arrays may not be sized
      rl_len = pl_len = bl_len = gl_len = 0;

    // map final_asv value bits into qoi_fns requirements
    qoi_fn = qoi_grad = moment1_grad = moment2_grad = false;
    total_i = moment_offset + rl_len + pl_len + bl_len + gl_len;
    for (j=0; j<total_i; ++j)
      if (final_asv[cntr+j] & 1)
        { qoi_fn = true; break; }

    // map final_asv gradient bits into moment grad requirements
    if (finalMomentsType) {
      if (final_asv[cntr++] & 2) moment1_grad = true;
      if (final_asv[cntr++] & 2) moment2_grad = true;
    }
    if (respLevelTarget == RELIABILITIES)
      for (j=0; j<rl_len; ++j) // dbeta/ds requires mu,sigma,dmu/ds,dsigma/ds
	if (final_asv[cntr+j] & 2)
	  { moment1_grad = moment2_grad = qoi_fn = true; break;}
    cntr += rl_len + pl_len;
    for (j=0; j<bl_len; ++j)   // dz/ds requires dmu/ds, dsigma/ds
      if (final_asv[cntr+j] & 2)
	{ moment1_grad = moment2_grad = true; break; }
    cntr += bl_len + gl_len;
    if (moment1_grad) assign_dvv = qoi_grad = true;
    if (moment2_grad) assign_dvv = qoi_grad = qoi_fn = true;

    // map qoi_{fn,grad} requirements into ASV settings
    if (qoi_fn)                    sampler_asv[i] |= 1;
    if (qoi_grad /*|| useDerivs*/) sampler_asv[i] |= 2; // future aggregation...
  }
  activeSet.request_vector(sampler_asv);

  if (assign_dvv)
    activeSet.derivative_vector(finalStatistics.active_set_derivative_vector());
  //else leave DVV as default active cv's (from Iterator::update_from_model())
}


/** Default implementation generates allResponses from either allSamples
    or allVariables. */
void NonDSampling::core_run()
{
  Cout << "Hello from NonDSampling::core_run" << std::endl;
  bool log_resp_flag = (allDataFlag || statsFlag), log_best_flag = false;
  evaluate_parameter_sets(*iteratedModel, log_resp_flag, log_best_flag);
}


void NonDSampling::
compute_statistics(const RealMatrix&     vars_samples,
		   const IntResponseMap& resp_samples)
{
  StringMultiArrayConstView
    acv_labels  = ModelUtils::all_continuous_variable_labels(*iteratedModel),
    adiv_labels = ModelUtils::all_discrete_int_variable_labels(*iteratedModel),
    adsv_labels = ModelUtils::all_discrete_string_variable_labels(*iteratedModel),
    adrv_labels = ModelUtils::all_discrete_real_variable_labels(*iteratedModel);
  size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  mode_counts(iteratedModel->current_variables(), cv_start, num_cv,
	      div_start, num_div, dsv_start, num_dsv, drv_start, num_drv);
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
		     ModelUtils::response_labels(*iteratedModel));
  }

  if (epistemicStats) { // Epistemic/mixed
    compute_intervals(resp_samples); // compute min/max response intervals
  }
  else { // Aleatory
    // compute means and std deviations with confidence intervals
    compute_moments(resp_samples);
    // compute CDF/CCDF mappings of z to p/beta and p/beta to z
    if (totalLevelRequests)
      compute_level_mappings(resp_samples);
  }

  if (!subIteratorFlag) {
    nonDSampCorr.compute_correlations(vars_samples, resp_samples);
  }

  if (stdRegressionCoeffs) {
    nonDSampCorr.compute_std_regress_coeffs(vars_samples, resp_samples);
  }

  if (toleranceIntervalsFlag) {
    computeDSTIEN( resp_samples
                 , tiCoverage
                 , 1. - tiConfidenceLevel
                 , tiNumValidSamples           // Output
                 , tiDstienMus                 // Output
                 , tiDeltaMultiplicativeFactor // Output
                 , tiSampleSigmas              // Output
                 , tiDstienSigmas              // Output
                 );
  }

  // push results into finalStatistics
  update_final_statistics();
}


void NonDSampling::
compute_intervals(RealRealPairArray& extreme_fns, const IntResponseMap& samples)
{
  // For the samples array, calculate min/max response intervals

  size_t i, j, num_obs = samples.size(), num_samp;
  const StringArray& resp_labels = ModelUtils::response_labels(*iteratedModel);

  extreme_fns.resize(numFunctions);
  IntRespMCIter it;
  for (i=0; i<numFunctions; ++i) {
    num_samp = 0;
    Real min = DBL_MAX, max = -DBL_MAX;
    for (it=samples.begin(); it!=samples.end(); ++it) {
      Real sample = it->second.function_value(i);
      if (std::isfinite(sample)) { // neither NaN nor +/-Inf
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
		RealMatrix& moment_grads, RealMatrix& moment_conf_ints,
		short moments_type, const StringArray& labels)
{
  // For the samples array, calculate 1st four moments and confidence intervals

  // if subIteratorFlag, final_asv will be general.  If not a sub-iterator, then
  // NonD::initialize_final_statistics() sets default request vector to 1's.
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();

  // if statsFlag, always compute moments for output regardless of final ASV.
  // else define moment requirements from final_asv and finalMomentsType.
  bool mom_fns = statsFlag, mom_grads = false;
  size_t i, l, m, cntr, num_lev, num_obs = samples.size();
  for (i=0, cntr=0; i<numFunctions; ++i) {
    if (finalMomentsType) { // only compute moments if needed
      for (m=0; m<2; ++m, ++cntr) {
        if (final_asv[cntr] & 1) mom_fns   = true;
        if (final_asv[cntr] & 2) mom_grads = true;
      }
    }
    if (respLevelTarget == RELIABILITIES) {
      num_lev = requestedRespLevels[i].length();
      for (l=0; l<num_lev; ++l, ++cntr) {
        if (final_asv[cntr] & 1) mom_fns = true;
        // dbeta/ds requires mu,sigma,dmu/ds,dsigma/ds
        if (final_asv[cntr] & 2) mom_fns = mom_grads = true;
      }
    }
    else
      cntr += requestedRespLevels[i].length();
    cntr += requestedProbLevels[i].length();
    num_lev = requestedRelLevels[i].length();
    for (l=0; l<num_lev; ++l, ++cntr) {
      if (final_asv[cntr] & 1) mom_fns = true;
      // dz/ds requires dmu/ds, dsigma/ds
      if (final_asv[cntr] & 2) mom_grads = true;
    }
    cntr += requestedGenRelLevels[i].length();
  }
  if (!mom_fns && !mom_grads)
    return;

  RealVectorArray fn_samples(num_obs);
  SizetArray sample_counts;
  IntRespMCIter it;
  for (it=samples.begin(), i=0; it!=samples.end(); ++it, ++i)
    fn_samples[i] = it->second.function_values_view();

  if (mom_fns) {
    compute_moments(fn_samples,sample_counts,moment_stats,moments_type,labels);
    compute_moment_confidence_intervals(moment_stats, moment_conf_ints,
					sample_counts, moments_type);
    functionMomentsComputed = true;
  }

  if (mom_grads) {
    RealMatrixArray grad_samples(num_obs);
    for (it=samples.begin(), i=0; it!=samples.end(); ++it, ++i)
      grad_samples[i] = it->second.function_gradients_view();
    compute_moment_gradients(fn_samples, grad_samples, moment_stats,
			     moment_grads, moments_type);
  }
}


void NonDSampling::
compute_moments(const RealVectorArray& fn_samples, SizetArray& sample_counts,
		RealMatrix& moment_stats, short moments_type,
		const StringArray& labels)
{
  size_t i, j, num_obs = fn_samples.size(), num_qoi;
  if (num_obs)
    num_qoi = fn_samples[0].length();
  else {
    Cerr << "Error: empty samples array in NonDSampling::compute_moments()."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }
  Real sum, cm2, cm3, cm4, sample;

  if (moment_stats.empty()) moment_stats.shapeUninitialized(4, num_qoi);
  sample_counts.resize(num_qoi);

  for (i=0; i<num_qoi; ++i) {
    size_t& num_samp = sample_counts[i];
    Real*  moments_i =  moment_stats[i];
    Pecos::accumulate_mean(fn_samples, i, num_samp, moments_i[0]);
    if (num_samp != num_obs)
      Cerr << "Warning: sampling statistics for " << labels[i] << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";

    if (num_samp)
      Pecos::accumulate_moments(fn_samples, i, moments_type, moments_i);
    else {
      Cerr << "Warning: Number of samples for " << labels[i]
	   << " must be nonzero for moment calculation in NonDSampling::"
	   << "compute_moments().\n";
      for (int j=0; j<4; ++j)
	moments_i[j] = std::numeric_limits<double>::quiet_NaN();
    }
  }
}


void NonDSampling::
compute_moments(const RealVectorArray& fn_samples, RealMatrix& moment_stats,
		short moments_type)
{
  size_t i, j, num_obs = fn_samples.size(), num_qoi, num_samp;
  if (num_obs)
    num_qoi = fn_samples[0].length();
  else {
    Cerr << "Error: empty samples array in NonDSampling::compute_moments()."
	 << std::endl;
    abort_handler(METHOD_ERROR);
  }

  if (moment_stats.empty()) moment_stats.shapeUninitialized(4, num_qoi);

  for (i=0; i<num_qoi; ++i) {

    Real* moments_i = moment_stats[i];
    Pecos::accumulate_mean(fn_samples, i, num_samp, moments_i[0]);
    if (num_samp != num_obs) {
      std::cout << "In NonDSampling::compute_moments(2)"
	        << ", Warning: sampling statistics for quantity " << i+1 << " omit "
                << num_obs-num_samp << " failed evaluations out of " << num_obs
		<< " samples." << std::endl;

      Cerr << "Warning: sampling statistics for quantity " << i+1 << " omit "
	   << num_obs-num_samp << " failed evaluations out of " << num_obs
	   << " samples.\n";
    }

    if (num_samp)
      Pecos::accumulate_moments(fn_samples, i, moments_type, moments_i);
    else {
      Cerr << "Warning: Number of samples for quantity " << i+1
	   << " must be nonzero in NonDSampling::compute_moments().\n";
      for (size_t j=0; j<4; ++j)
	moments_i[j] = std::numeric_limits<double>::quiet_NaN();
    }
  }
}


void NonDSampling::
compute_moments(const RealMatrix& fn_samples, RealMatrix& moment_stats,
		short moments_type)
{
  int i, num_qoi = fn_samples.numRows(), num_obs = fn_samples.numCols();
  RealVectorArray rva_samples(num_obs);
  for (i=0; i<num_obs; ++i)
    rva_samples[i]
      = RealVector(Teuchos::View, const_cast<Real*>(fn_samples[i]), num_qoi);
  compute_moments(rva_samples, moment_stats, moments_type);
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
      if (moments_type == Pecos::CENTRAL_MOMENTS)
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
	if (moments_type == Pecos::CENTRAL_MOMENTS) {
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
compute_moment_gradients(const RealVectorArray& fn_samples,
			 const RealMatrixArray& grad_samples,
			 const RealMatrix& moment_stats,
			 RealMatrix& moment_grads, short moments_type)
{
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  size_t q, cntr = 0;
  int m1_index, m2_index, num_mom = 2*numFunctions,
    num_deriv_vars = finalStatistics.active_set_derivative_vector().size();
  if (moment_grads.numRows() != num_deriv_vars ||
      moment_grads.numRows() != num_mom)
  //moment_grads.shape(num_deriv_vars, num_mom); // init to 0.
    moment_grads.shapeUninitialized(num_deriv_vars, num_mom);
  //else
  //  moment_grads = 0.;

  for (q=0; q<numFunctions; ++q) {
    m1_index = 2*q; m2_index = m1_index + 1;
    // Compute moment_grads
    if (std::min(fn_samples.size(), grad_samples.size()) == 0) {
      Cerr << "Error: empty samples array in NonDSampling::"
	   << "compute_moment_gradients()" << std::endl;
      abort_handler(METHOD_ERROR);
    }
    Pecos::accumulate_moment_gradients(fn_samples, grad_samples, q,
      moments_type, moment_stats(0,q), moment_stats(1,q),
      moment_grads[m1_index], moment_grads[m2_index]);
    // Assign moment_grads into finalStatistics (could do this in one step,
    // but retain API of moment_stats and moment_grads as their own matrices).
    // > NonDExpansion and NonDLocalReliability do not store moment grads as
    //   member variables; they update finalStats directly in NonDExpansion::
    //   compute_analytic_statistics() and NonDLocalRel::update_level_data().
    //   Note: NonDExpansion::update_final_statistics_gradients() provides
    //   post-processing for special cases (combined vars, insertion).
    // > NonDSampling could retain a class member or use a local variable for
    //   moment_grads (currently a local, whereas momentStats is a member)
    if (moments_type) {
      if (final_asv[cntr] & 2) // moment 1 (mean) gradient
	finalStatistics.function_gradient(
	  Teuchos::getCol(Teuchos::View, moment_grads, m1_index), cntr);
      ++cntr;
      if (final_asv[cntr] & 2) // moment 2 (var or stdev) gradient
	finalStatistics.function_gradient(
	  Teuchos::getCol(Teuchos::View, moment_grads, m2_index), cntr);
      cntr += 1 +
	requestedRespLevels[q].length() + requestedProbLevels[q].length() +
	requestedRelLevels[q].length()  + requestedGenRelLevels[q].length();
    }
  }
}


void NonDSampling::
archive_moments(size_t inc_id)
{
  if(!resultsDB.active()) return;
 
  const StringArray &labels = ModelUtils::response_labels(*iteratedModel);

  // archive the moments to results DB
  MetaDataType md_moments;
  md_moments["Row Labels"] = (finalMomentsType == Pecos::CENTRAL_MOMENTS) ?
    make_metadatavalue("Mean", "Variance", "3rdCentral", "4thCentral") :
    make_metadatavalue("Mean", "Standard Deviation", "Skewness", "Kurtosis");
  md_moments["Column Labels"] = make_metadatavalue(labels);
  resultsDB.insert(run_identifier(), resultsNames.moments_std, momentStats,
		   md_moments);
  
  // send to prototype hdf5DB, too
  StringArray location;
  if(inc_id) location.push_back(String("increment:") + std::to_string(inc_id));
  location.push_back("moments");
  location.push_back("");
  for(int i = 0; i < numFunctions; ++i) {
    location.back() = labels[i]; 
    DimScaleMap scales;
    if(finalMomentsType == Pecos::CENTRAL_MOMENTS)
      scales.emplace(0, 
                     StringScale("moments",
                     {"mean", "variance", "third_central", "fourth_central"},
                     ScaleScope::SHARED));
    else
      scales.emplace(0,
                     StringScale("moments", 
                     {"mean", "std_deviation", "skewness", "kurtosis"},
                     ScaleScope::SHARED));
    // extract column or row of moment_stats
    resultsDB.insert(run_identifier(), location, 
        Teuchos::getCol<int,double>(Teuchos::View, *const_cast<RealMatrix*>(&momentStats), i), scales);
  }
}


void NonDSampling::
archive_moment_confidence_intervals(size_t inc_id)
{
  if(!resultsDB.active())
    return;

  const StringArray &labels = ModelUtils::response_labels(*iteratedModel);
  // archive the confidence intervals to results DB
  MetaDataType md;
  md["Row Labels"] = (finalMomentsType == Pecos::CENTRAL_MOMENTS) ?
    make_metadatavalue("LowerCI_Mean", "UpperCI_Mean",
		       "LowerCI_Variance", "UpperCI_Variance") :
    make_metadatavalue("LowerCI_Mean", "UpperCI_Mean",
		       "LowerCI_StdDev", "UpperCI_StdDev");
  md["Column Labels"] = make_metadatavalue(labels);
  resultsDB.insert(run_identifier(), resultsNames.moment_cis,
		   momentCIs, md);
  // archive in HDF5. Store in a 2d dataset with mean and var/stdev on the 1st dimension
  // and lower and upper bounds on the 2nd dimension
  DimScaleMap scales;
  scales.emplace(0, StringScale("bounds", {"lower", "upper"})); 
  if(finalMomentsType == Pecos::CENTRAL_MOMENTS)
    scales.emplace(1, StringScale("moments", {"mean", "variance"})); 
  else
    scales.emplace(1, StringScale("moments", {"mean", "std_deviation"})); 

  StringArray location;
  if(inc_id) location.push_back(String("increment:") + std::to_string(inc_id));
  location.push_back("moment_confidence_intervals");
  location.push_back("");
  for(int i = 0; i < numFunctions; ++i) { // loop over responses
    location.back() = labels[i];
    RealMatrix ci(2,2,false);
    ci(0,0) = momentCIs(0,i);
    ci(1,0) = momentCIs(1,i);
    ci(0,1) = momentCIs(2,i);
    ci(1,1) = momentCIs(3,i);
    resultsDB.insert(run_identifier(), location, ci, scales);
  }
}

void NonDSampling::
archive_extreme_responses(size_t inc_id) {
  const StringArray &labels = ModelUtils::response_labels(*iteratedModel);
  StringArray location;
  if(inc_id) location.push_back(String("increment:") + std::to_string(inc_id));
  location.push_back("extreme_responses");
  location.push_back("");
  DimScaleMap scales;
  scales.emplace(0, StringScale("extremes", {"minimum", "maximum"})); 
  for(int i = 0; i < numFunctions; ++i) { // loop over responses
    location.back() = labels[i];
    RealVector extreme_values(2);
    extreme_values[0] = extremeValues[i].first;
    extreme_values[1] = extremeValues[i].second;
    resultsDB.insert(run_identifier(), location, extreme_values, scales);
  }
}

int NonDSampling::
compute_wilks_sample_size(unsigned short order, Real alpha, Real beta,
			  bool twosided)
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


Real NonDSampling::
compute_wilks_residual(unsigned short order, int nsamples, Real alpha,
		       Real beta, bool twosided)
{
  Real rorder = (Real) order;

  if( !twosided && (order==1) )
    return std::log(1.0-beta)/std::log(alpha) - (Real) nsamples;

  if( twosided )
    return boost::math::ibeta<Real>((Real)nsamples-2.0*rorder+1.0, 2.0*rorder, alpha) - (1.0-beta);
  else
    return boost::math::ibeta<Real>(rorder, (Real)nsamples-rorder+1.0, 1-alpha) - beta;
}


Real NonDSampling::
compute_wilks_alpha(unsigned short order, int nsamples, Real beta,
		    bool twosided)
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


Real NonDSampling::
compute_wilks_beta(unsigned short order, int nsamples, Real alpha,
		   bool twosided)
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
  const StringArray& resp_labels = ModelUtils::response_labels(*iteratedModel);
  std::multiset<Real> sorted_samples; // STL-based array for sorting
  SizetArray bins; Real min, max, sample;

  // check if moments are required, and if so, compute them now
  if (momentStats.empty()) {
    bool need_moments = false;
    for (i=0; i<numFunctions; ++i)
      if ( !requestedRelLevels[i].empty() ||
	   ( !requestedRespLevels[i].empty() &&
	     respLevelTarget == RELIABILITIES ) )
	{ need_moments = true; break; }
    if (need_moments) {
      Cerr << "Error: required moments not available in compute_distribution_"
	   << "mappings().  Call compute_moments() first." << std::endl;
      abort_handler(METHOD_ERROR);
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
  const ShortArray& final_asv = finalStatistics.active_set_request_vector();
  bool extrapolated_mappings = false,
    central_mom = (finalMomentsType == Pecos::CENTRAL_MOMENTS);
  size_t cntr = 0,
    num_deriv_vars = finalStatistics.active_set_derivative_vector().size(),
    moment_offset = (finalMomentsType) ? 2 : 0;
  RealVector mean_grad, mom2_grad;
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
	if (std::isfinite(sample))
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
	if (std::isfinite(sample)) {
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

    cntr += moment_offset;
    // ----------------
    // Process mappings
    // ----------------
    if (rl_len) {
      switch (respLevelTarget) {
      case PROBABILITIES: case GEN_RELIABILITIES: // z -> p/beta* (from binning)
	bin_accumulator = 0;
	for (j=0; j<rl_len; ++j, ++cntr) { // compute CDF/CCDF p/beta*
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
	Real mean  = momentStats(0,i);
	Real stdev = (central_mom) ?
	  std::sqrt(momentStats(1,i)) : momentStats(1,i);
	if (!momentGrads.empty()) {
	  int i2 = 2*i;
	  mean_grad = Teuchos::getCol(Teuchos::View, momentGrads, i2);
	  mom2_grad = Teuchos::getCol(Teuchos::View, momentGrads, i2+1);
	}
	for (j=0; j<rl_len; j++, ++cntr) {
	  // *** beta
	  Real z_bar = requestedRespLevels[i][j];
	  if (!Pecos::is_small(stdev))
	    computedRelLevels[i][j] = (cdfFlag) ?
	      (mean - z_bar)/stdev : (z_bar - mean)/stdev;
	  else
	    computedRelLevels[i][j]
	      = ( (cdfFlag && mean <= z_bar) || (!cdfFlag && mean > z_bar) )
	      ? -Pecos::LARGE_NUMBER : Pecos::LARGE_NUMBER;
	  // *** beta gradient
	  if (final_asv[cntr] & 2) {
	    RealVector beta_grad = finalStatistics.function_gradient_view(cntr);
	    if (!Pecos::is_small(stdev)) {
	      for (k=0; k<num_deriv_vars; ++k) {
		Real stdev_grad = (central_mom) ?
		  mom2_grad[k] / (2.*stdev) : mom2_grad[k];
		Real dratio_dx = (stdev*mean_grad[k] - (mean-z_bar)*stdev_grad)
		               / std::pow(stdev, 2);
		beta_grad[k] = (cdfFlag) ? dratio_dx : -dratio_dx;
	      }
	    }
	    else
	      beta_grad = 0.;
	  }
	}
	break;
      }
      }
    }
    for (j=0; j<pl_len+gl_len; j++, ++cntr) { // p/beta* -> z
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
      Real mean  = momentStats(0,i);
      Real stdev = (finalMomentsType == Pecos::CENTRAL_MOMENTS) ?
	std::sqrt(momentStats(1,i)) : momentStats(1,i);
      if (!momentGrads.empty()) {
	int i2 = 2*i;
	mean_grad = Teuchos::getCol(Teuchos::View, momentGrads, i2);
	mom2_grad = Teuchos::getCol(Teuchos::View, momentGrads, i2+1);
      }
      for (j=0; j<bl_len; j++, ++cntr) {
	// beta_bar -> z
	Real beta_bar = requestedRelLevels[i][j];
	computedRespLevels[i][j+pl_len] = (cdfFlag) ?
	  mean - beta_bar * stdev : mean + beta_bar * stdev;
	// *** z gradient
	if (final_asv[cntr] & 2) {
	  RealVector z_grad = finalStatistics.function_gradient_view(cntr);
	  for (k=0; k<num_deriv_vars; ++k) {
	    Real stdev_grad = (central_mom) ?
	      mom2_grad[k] / (2.*stdev) : mom2_grad[k];
	    z_grad[k] = (cdfFlag) ? mean_grad[k] - beta_bar * stdev_grad
	                          : mean_grad[k] + beta_bar * stdev_grad;
	  }
	}
      }
    }
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
  if (epistemicStats) { // output only min & max values in the epistemic case
    print_intervals(s);
  }
  else {
    print_moments(s);
    if (totalLevelRequests) {
      print_level_mappings(s);
      print_system_mappings(s);
    }
  }

  if (!subIteratorFlag) {
    nonDSampCorr.print_correlations(s, iteratedModel->current_variables().ordered_labels(ACTIVE_VARS), ModelUtils::response_labels(*iteratedModel));
  }

  if (wilksFlag) {
    if (epistemicStats) {
      Cerr << "Warning: Wilks printing requested in conjunction with epstemic variables" << std::endl;
    }

    print_wilks_stastics(s);
  }

  if (stdRegressionCoeffs) {
    if (epistemicStats) {
      Cerr << "Warning: std regression coefficients printing requested in conjunction with epstemic variables" << std::endl;
    }

    nonDSampCorr.print_std_regress_coeffs(s, iteratedModel->current_variables().ordered_labels(ACTIVE_VARS), ModelUtils::response_labels(*iteratedModel));
  }

  if (toleranceIntervalsFlag) {
    if (epistemicStats) {
      Cerr << "Warning: tolerance intervals printing requested in conjunction with epstemic variables" << std::endl;
    }

    print_tolerance_intervals_statistics(s);
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
	      const RealMatrix& moment_cis, String qoi_type, short moments_type,
	      const StringArray& moment_labels, bool print_cis)
{
  size_t i, j, width = write_precision+7, num_qoi = moment_stats.numCols(),
    num_moments = moment_stats.numRows(), num_ci  = moment_cis.numRows();

  s << "\nSample moment statistics for each " << qoi_type << ":\n"
    << std::scientific << std::setprecision(write_precision)
    << std::setw(width+15) << "Mean";
  if (moments_type == Pecos::CENTRAL_MOMENTS)
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
  if (print_cis && num_ci) {
    // output 95% confidence intervals as (,) interval
    s << "\n95% confidence intervals for each " << qoi_type << ":\n"
      << std::setw(width+15) << "LowerCI_Mean"
      << std::setw(width+1)  << "UpperCI_Mean";
    if (num_ci > 2) {
      if (moments_type == Pecos::CENTRAL_MOMENTS)
	s << std::setw(width+1) << "LowerCI_Variance"
	  << std::setw(width+2) << "UpperCI_Variance";
      else
	s << std::setw(width+1) << "LowerCI_StdDev"
	  << std::setw(width+2) << "UpperCI_StdDev";
    }
    s << '\n';
    for (i=0; i<num_qoi; ++i) {
      const Real* moment_ci_i = moment_cis[i];
      s << std::setw(14) << moment_labels[i];
      for (j=0; j<num_ci; ++j)
	s << ' ' << std::setw(width) << moment_ci_i[j];
      s << '\n';
    }
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
      << " for "  << ModelUtils::response_labels(*iteratedModel)[fn_index] << ":\n\n";

    if(wilks_twosided) {
      s << "    Coverage Level     Lower Bound        Upper Bound     Number of Samples\n"
        << "    --------------  -----------------  -----------------  -----------------\n";
    } else {    
      s << "    Coverage Level       " << (wilksSidedness == ONE_SIDED_UPPER ? "Upper" : "Lower")
        << " Bound     Number of Samples\n"
        << "    --------------   -----------------  -----------------\n";
    } 

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
        if (std::isfinite(sample)) // neither NaN nor +/-Inf
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

void NonDSampling::
print_tolerance_intervals_statistics(std::ostream& s) const
{
  const size_t width = write_precision+7;
  const size_t response_label_width = 14;
  s << "-----------------------------------------------------------------------------"
    << std::endl
    << "Double-sided tolerance interval equivalent normal results"
    << " with coverage = "     << std::fixed << std::setprecision(2) << 100.0*tiCoverage        << "%"
    << ", confidence level = " << std::fixed << std::setprecision(2) << 100.0*tiConfidenceLevel << "%"
    << ", and "                << tiNumValidSamples << " valid samples"
    << std::endl
    << "Double-sided tolerance interval equivalent normal statistics for each response function:"
    << std::endl
    << std::setw(response_label_width + width + 1) << "Sample Mean mu"
    << std::setw(width+1)  << "Sample Stdev s"
    << std::setw(width+1)  << "Stdev Mult. f"
    << std::setw(width+1)  << "LowerEnd=mu-f*s"
    << std::setw(width+1)  << "UpperEnd=mu+f*s"
    << std::setw(width+1)  << "Eq. Norm. Stdev"
    << std::endl
    << std::scientific << std::setprecision(write_precision);
  for (size_t i = 0; i < numFunctions; ++i) {
    s << std::setw(response_label_width)
      << ModelUtils::response_labels(*iteratedModel)[i]
      << ' ' << std::setw(width) << tiDstienMus[i]
      << ' ' << std::setw(width) << tiSampleSigmas[i]
      << ' ' << std::setw(width) << tiDeltaMultiplicativeFactor
      << ' ' << std::setw(width) << tiDstienMus[i] - tiDeltaMultiplicativeFactor * tiSampleSigmas[i]
      << ' ' << std::setw(width) << tiDstienMus[i] + tiDeltaMultiplicativeFactor * tiSampleSigmas[i]
      << ' ' << std::setw(width) << tiDstienSigmas[i]
      << std::endl;
  }
}

void NonDSampling::
archive_tolerance_intervals(size_t inc_id)
{
  StringArray location;
  if (inc_id != 0) {
    location.push_back(String("increment:") + std::to_string(inc_id));
  }
  location.push_back("tolerance_intervals");
  location.push_back("");

  Teuchos::SerialDenseVector<int,double> tmpValues(6);
  for(size_t i = 0; i < numFunctions; ++i) {
    location.back() = ModelUtils::response_labels(*iteratedModel)[i];
    DimScaleMap scales;
    scales.emplace( 0
                  , StringScale( "tolerance_intervals"
                               , { "sample_mean"
                                 , "sample_stdev"
                                 , "multiplicative_factor_f"
                                 , "TI_lower_end"
                                 , "TI_upper_end"
                                 , "TI_equiv_normal_stdev"
                                 }
                               , ScaleScope::SHARED
                               )
                  );
    tmpValues[0] = tiDstienMus[i];
    tmpValues[1] = tiSampleSigmas[i];
    tmpValues[2] = tiDeltaMultiplicativeFactor;
    tmpValues[3] = tiDstienMus[i] - tiDeltaMultiplicativeFactor * tiSampleSigmas[i];
    tmpValues[4] = tiDstienMus[i] + tiDeltaMultiplicativeFactor * tiSampleSigmas[i];
    tmpValues[5] = tiDstienSigmas[i];

    resultsDB.insert( run_identifier()
                    , location
                    , tmpValues
                    , scales
                    );
  }

  AttributeArray ns_attr({ResultAttribute<int>("valid_samples", tiNumValidSamples)});
  if (inc_id == 0) {
    StringArray location({String("tolerance_intervals")});
    resultsDB.add_metadata_to_object(run_identifier(), location, ns_attr);
  }
  else {
    StringArray location({String("increment:") + std::to_string(inc_id), String("tolerance_intervals")});
    resultsDB.add_metadata_to_object(run_identifier(), location, ns_attr);
  }
}

} // namespace Dakota
