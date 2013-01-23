/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include <boost/math/special_functions/fpclassify.hpp>

static const char rcsId[]="@(#) $Id: NonDSampling.cpp 7036 2010-10-22 23:20:24Z mseldre $";


namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator
    instantiation.  In this case, set_db_list_nodes has been called and
    probDescDB can be queried for settings from the method specification. */
NonDSampling::NonDSampling(Model& model): NonD(model),
  seedSpec(probDescDB.get_int("method.random_seed")), randomSeed(seedSpec),
  samplesSpec(probDescDB.get_int("method.samples")), samplesRef(samplesSpec),
  numSamples(samplesSpec),
  rngName(probDescDB.get_string("method.random_number_generator")),
  sampleType(probDescDB.get_string("method.sample_type")),
  statsFlag(true), allDataFlag(false), sampleRanksMode(IGNORE_RANKS),
  varyPattern(!probDescDB.get_bool("method.fixed_seed")), numLHSRuns(0)
{
  if (numEpistemicUncVars && totalLevelRequests) {
    Cerr << "\nError: sampling does not support level requests for "
	 << "analyses containing epistemic uncertainties." << std::endl;
    abort_handler(-1);
  }

  // Since the sampleType is shared with other iterators for other purposes,
  // its default in DataMethod.cpp is the NULL string.  Explicitly enforce the
  // LHS default here.
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
NonDSampling(NoDBBaseConstructor, Model& model, const String& sample_type,
	     int samples, int seed, const String& rng, bool vary_pattern):
  NonD(NoDBBaseConstructor(), model), seedSpec(seed), randomSeed(seed),
  samplesSpec(samples), samplesRef(samples), numSamples(samples), rngName(rng),
  sampleType(sample_type), statsFlag(false), allDataFlag(true),
  sampleRanksMode(IGNORE_RANKS), varyPattern(vary_pattern), numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // enforce LHS as default sample type
  if (sampleType.empty())
    sampleType = "lhs";

  // not used but included for completeness
  if (numSamples) // samples is now optional (default = 0)
    maxConcurrency *= numSamples;
}


/** This alternate constructor is used by ConcurrentStrategy for
    generation of uniform, uncorrelated sample sets. */
NonDSampling::
NonDSampling(NoDBBaseConstructor, const String& sample_type, int samples,
	     int seed, const String& rng, const RealVector& lower_bnds,
	     const RealVector& upper_bnds):
  NonD(NoDBBaseConstructor(), lower_bnds, upper_bnds), seedSpec(seed),
  randomSeed(seed), samplesSpec(samples), samplesRef(samples),
  numSamples(samples), rngName(rng), sampleType(sample_type), statsFlag(false),
  allDataFlag(true), sampleRanksMode(IGNORE_RANKS), varyPattern(true),
  numLHSRuns(0)
{
  subIteratorFlag = true; // suppress some output

  // enforce LHS as default sample type
  if (sampleType.empty())
    sampleType = "lhs";

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
					 numSamples, allSamples);
    }
    else if (samplingVarsMode == ALL_UNIFORM) {
      // sample uniformly from ALL lower/upper bnds with model in distinct view.
      // loss of sampleRanks control is OK since NonDIncremLHS uses ACTIVE mode.
      // TO DO: add support for uniform discrete
      lhsDriver.generate_uniform_samples(model.all_continuous_lower_bounds(),
					 model.all_continuous_upper_bounds(),
					 numSamples, allSamples);
    }
    else { // A, E, A+E UNCERTAIN
      // sample uniformly from {A,E,A+E} UNCERTAIN lower/upper bounds
      // with model using a non-corresponding view (corresponding views
      // handled in first case above)
      size_t start_acv, num_acv, dummy;
      mode_counts(model, start_acv, num_acv, dummy, dummy, dummy, dummy);
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
					 numSamples, allSamples);
    }
    break;
  case ALEATORY_UNCERTAIN:
    lhsDriver.generate_samples(model.aleatory_distribution_parameters(),
                               numSamples, allSamples);
    break;
  case EPISTEMIC_UNCERTAIN:
    lhsDriver.generate_samples(model.epistemic_distribution_parameters(),
                               numSamples, allSamples);
    break;
  case UNCERTAIN:
    lhsDriver.generate_samples(model.aleatory_distribution_parameters(),
			       model.epistemic_distribution_parameters(),
			       numSamples, allSamples);
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
    RealSetArray empty_rsa; IntSetArray empty_isa;
    const IntSetArray&  di_design_sets = (numDiscIntDesVars) ?
      model.discrete_design_set_int_values()  : empty_isa;
    const RealSetArray& dr_design_sets = (numDiscRealDesVars) ?
      model.discrete_design_set_real_values() : empty_rsa;
    const IntSetArray&  di_state_sets  = (numDiscIntStateVars) ?
      model.discrete_state_set_int_values()   : empty_isa;
    const RealSetArray& dr_state_sets  = (numDiscRealStateVars) ?
      model.discrete_state_set_real_values()  : empty_rsa;

    // Call LHS to generate the specified samples within the specified
    // distributions.  Use model distribution parameters unless ACTIVE
    // excludes {aleatory,epistemic,both} uncertain variables.
    if ( samplingVarsMode == ACTIVE &&
	 ( model_view == RELAXED_DESIGN || model_view == RELAXED_STATE ||
	   model_view ==   MIXED_DESIGN || model_view ==   MIXED_STATE ) ) {
      Pecos::AleatoryDistParams empty_adp; Pecos::EpistemicDistParams empty_edp;
      lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	ddriv_u_bnds, di_design_sets, dr_design_sets, csv_l_bnds, csv_u_bnds,
	dsriv_l_bnds, dsriv_u_bnds, di_state_sets, dr_state_sets, empty_adp,
	empty_edp, numSamples, allSamples, sampleRanks);
    }
    else if ( samplingVarsMode == ACTIVE &&
	      ( model_view == RELAXED_ALEATORY_UNCERTAIN ||
		model_view == MIXED_ALEATORY_UNCERTAIN ) ) {
      Pecos::EpistemicDistParams empty_edp;
      lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	ddriv_u_bnds, di_design_sets, dr_design_sets, csv_l_bnds, csv_u_bnds,
	dsriv_l_bnds, dsriv_u_bnds, di_state_sets, dr_state_sets,
	model.aleatory_distribution_parameters(), empty_edp, numSamples,
	allSamples, sampleRanks);
    }
    else if ( samplingVarsMode == ACTIVE &&
	      ( model_view == RELAXED_EPISTEMIC_UNCERTAIN ||
		model_view == MIXED_EPISTEMIC_UNCERTAIN ) ) {
      Pecos::AleatoryDistParams empty_adp;
      lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
	ddriv_u_bnds, di_design_sets, dr_design_sets, csv_l_bnds, csv_u_bnds,
	dsriv_l_bnds, dsriv_u_bnds, di_state_sets, dr_state_sets, empty_adp,
	model.epistemic_distribution_parameters(), numSamples, allSamples,
	sampleRanks);
    }
    else 
      lhsDriver.generate_samples(cdv_l_bnds, cdv_u_bnds, ddriv_l_bnds,
        ddriv_u_bnds, di_design_sets, dr_design_sets, csv_l_bnds, csv_u_bnds,
	dsriv_l_bnds, dsriv_u_bnds, di_state_sets, dr_state_sets,
	model.aleatory_distribution_parameters(),
	model.epistemic_distribution_parameters(), numSamples, allSamples,
	sampleRanks);
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
  initialize_lhs(true);
  lhsDriver.generate_uniform_samples(lower_bnds, upper_bnds,
				     numSamples, allSamples);
}


void NonDSampling::
update_model_from_sample(Model& model, const Real* sample_vars)
{
  size_t i, cv_start, num_cv, div_start, num_div, drv_start, num_drv;
  mode_counts(model, cv_start, num_cv, div_start, num_div, drv_start, num_drv);

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


/** This function computes total design variable counts, not active counts,
    for use in defining offsets and counts within all variables arrays. */
void NonDSampling::
view_design_counts(const Model& model, size_t& num_cdv, size_t& num_didv,
		   size_t& num_drdv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: case RELAXED_DESIGN: case MIXED_DESIGN:
    // design vars are included in active counts from NonD
    num_cdv  = numContDesVars;
    num_didv = numDiscIntDesVars;
    num_drdv = numDiscRealDesVars; break;
  case RELAXED_EPISTEMIC_UNCERTAIN: case RELAXED_STATE: {
    // design vars are not included in active counts from NonD
    UShortMultiArrayConstView acv_types
      = model.all_continuous_variable_types();
    num_cdv = std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)CONTINUOUS_DESIGN) +
	      std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)DISCRETE_DESIGN_RANGE) +
	      std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)DISCRETE_DESIGN_SET_INT) +
	      std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)DISCRETE_DESIGN_SET_REAL);
    num_didv = num_drdv = 0; // no discrete since relaxed
    break;
  }
  case MIXED_EPISTEMIC_UNCERTAIN: case MIXED_STATE: {
    // design vars are not included in active counts from NonD
    UShortMultiArrayConstView acv_types
      = model.all_continuous_variable_types();
    UShortMultiArrayConstView adiv_types
      = model.all_discrete_int_variable_types();
    UShortMultiArrayConstView adrv_types
      = model.all_discrete_real_variable_types();
    num_cdv  = std::count(acv_types.begin(), acv_types.end(),
			  (unsigned short)CONTINUOUS_DESIGN);
    num_didv = std::count(adiv_types.begin(), adiv_types.end(),
			  (unsigned short)DISCRETE_DESIGN_RANGE) +
	       std::count(adiv_types.begin(), adiv_types.end(),
			  (unsigned short)DISCRETE_DESIGN_SET_INT);
    num_drdv = std::count(adrv_types.begin(), adrv_types.end(),
			  (unsigned short)DISCRETE_DESIGN_SET_REAL);
    break;
  }
  case RELAXED_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
  case   MIXED_UNCERTAIN: case   MIXED_ALEATORY_UNCERTAIN:
    num_cdv  = vars.cv_start();
    num_didv = vars.div_start();
    num_drdv = vars.drv_start(); break;
  }
}


/** This function computes total aleatory uncertain variable counts,
    not active counts, for use in defining offsets and counts within
    all variables arrays. */
void NonDSampling::
view_aleatory_uncertain_counts(const Model& model, size_t& num_cauv,
			       size_t& num_diauv,  size_t& num_drauv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: // UNCERTAIN = subset of ACTIVE
    num_cauv  = numContAleatUncVars;
    num_diauv = numDiscIntAleatUncVars;
    num_drauv = numDiscRealAleatUncVars; break;
  case RELAXED_DESIGN:    case RELAXED_STATE:
  case RELAXED_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN: {
    const Pecos::AleatoryDistParams& adp
      = model.aleatory_distribution_parameters();
    num_cauv = adp.cauv() + adp.dauv(); num_diauv = num_drauv = 0; break;
  }
  case MIXED_DESIGN:      case MIXED_STATE:
  case MIXED_UNCERTAIN:   case MIXED_EPISTEMIC_UNCERTAIN: {
    const Pecos::AleatoryDistParams& adp
      = model.aleatory_distribution_parameters();
    num_cauv  = adp.cauv();  num_diauv = adp.diauv();
    num_drauv = adp.drauv(); break;
  }
  case RELAXED_ALEATORY_UNCERTAIN: case MIXED_ALEATORY_UNCERTAIN:
    // ALEATORY_UNCERTAIN = same as ACTIVE
    num_cauv = vars.cv(); num_diauv = vars.div(); num_drauv = vars.drv(); break;
  }
}


/** This function computes total epistemic uncertain variable counts,
    not active counts, for use in defining offsets and counts within
    all variables arrays. */
void NonDSampling::
view_epistemic_uncertain_counts(const Model& model, size_t& num_ceuv,
				size_t& num_dieuv,  size_t& num_dreuv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: // UNCERTAIN = subset of ACTIVE
    num_ceuv  = numContEpistUncVars;
    num_dieuv = numDiscIntEpistUncVars;
    num_dreuv = numDiscRealEpistUncVars;                                  break;
  case RELAXED_DESIGN:             case RELAXED_STATE:
  case RELAXED_ALEATORY_UNCERTAIN: case RELAXED_UNCERTAIN: {
    const Pecos::EpistemicDistParams& edp
      = model.epistemic_distribution_parameters();
    num_ceuv = edp.ceuv() + edp.deuv(); num_dieuv = num_dreuv = 0;        break;
  }
  case MIXED_DESIGN:               case MIXED_STATE:
  case MIXED_ALEATORY_UNCERTAIN:   case MIXED_UNCERTAIN: {
    const Pecos::EpistemicDistParams& edp
      = model.epistemic_distribution_parameters();
    num_ceuv = edp.ceuv(); num_dieuv = edp.dieuv(); num_dreuv = edp.dreuv();
    break;
  }
  case RELAXED_EPISTEMIC_UNCERTAIN: case MIXED_EPISTEMIC_UNCERTAIN:
    // EPISTEMIC_UNCERTAIN = same as ACTIVE
    num_ceuv = vars.cv(); num_dieuv = vars.div(); num_dreuv = vars.drv(); break;
  }
}


/** This function computes total uncertain variable counts, not active counts,
    for use in defining offsets and counts within all variables arrays. */
void NonDSampling::
view_uncertain_counts(const Model& model, size_t& num_cuv, size_t& num_diuv,
		      size_t& num_druv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: // UNCERTAIN = subset of ACTIVE
    num_cuv  = numContAleatUncVars     + numContEpistUncVars;
    num_diuv = numDiscIntAleatUncVars  + numDiscIntEpistUncVars;
    num_druv = numDiscRealAleatUncVars + numDiscRealEpistUncVars;      break;
  case RELAXED_DESIGN:             case RELAXED_STATE:
  case RELAXED_ALEATORY_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN: {
    const Pecos::AleatoryDistParams& adp
      = model.aleatory_distribution_parameters();
    const Pecos::EpistemicDistParams& edp
      = model.epistemic_distribution_parameters();
    num_cuv  = adp.cauv() + adp.dauv() + edp.ceuv() + edp.deuv();
    num_diuv = num_druv = 0;                                           break;
  }
  case MIXED_DESIGN:               case MIXED_STATE:
  case MIXED_ALEATORY_UNCERTAIN:   case MIXED_EPISTEMIC_UNCERTAIN: {
    const Pecos::AleatoryDistParams& adp
      = model.aleatory_distribution_parameters();
    const Pecos::EpistemicDistParams& edp
      = model.epistemic_distribution_parameters();
    num_cuv  = adp.cauv()  + edp.ceuv();
    num_diuv = adp.diauv() + edp.dieuv();
    num_druv = adp.drauv() + edp.dreuv();                              break;
  }
  case RELAXED_UNCERTAIN: case MIXED_UNCERTAIN: // UNCERTAIN = same as ACTIVE
    num_cuv = vars.cv(); num_diuv = vars.div(); num_druv = vars.drv(); break;
  }
}


void NonDSampling::
view_state_counts(const Model& model, size_t& num_csv, size_t& num_disv,
		  size_t& num_drsv) const
{
  const Variables& vars = model.current_variables();
  short active_view = vars.view().first;
  switch (active_view) {
  case RELAXED_ALL: case MIXED_ALL: case RELAXED_STATE: case MIXED_STATE:
    // state vars are included in active counts from NonD
    num_csv  = numContStateVars;
    num_disv = numDiscIntStateVars;
    num_drsv = numDiscRealStateVars; break;
  case RELAXED_ALEATORY_UNCERTAIN: case RELAXED_DESIGN: {
    // state vars are not included in active counts from NonD
    UShortMultiArrayConstView acv_types
      = model.all_continuous_variable_types();
    num_csv = std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)CONTINUOUS_STATE) +
	      std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)DISCRETE_STATE_RANGE) +
	      std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)DISCRETE_STATE_SET_INT) +
	      std::count(acv_types.begin(), acv_types.end(),
			 (unsigned short)DISCRETE_STATE_SET_REAL);
    num_disv = num_drsv = 0; // no discrete since relaxed
    break;
  }
  case MIXED_ALEATORY_UNCERTAIN: case MIXED_DESIGN: {
    // state vars are not included in active counts from NonD
    UShortMultiArrayConstView acv_types
      = model.all_continuous_variable_types();
    UShortMultiArrayConstView adiv_types
      = model.all_discrete_int_variable_types();
    UShortMultiArrayConstView adrv_types
      = model.all_discrete_real_variable_types();
    num_csv  = std::count(acv_types.begin(), acv_types.end(),
			  (unsigned short)CONTINUOUS_STATE);
    num_disv = std::count(adiv_types.begin(), adiv_types.end(),
			  (unsigned short)DISCRETE_STATE_RANGE) +
	       std::count(adiv_types.begin(), adiv_types.end(),
			  (unsigned short)DISCRETE_STATE_SET_INT);
    num_drsv = std::count(adrv_types.begin(), adrv_types.end(),
			  (unsigned short)DISCRETE_STATE_SET_REAL);
    break;
  }
  case RELAXED_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
  case   MIXED_UNCERTAIN: case   MIXED_EPISTEMIC_UNCERTAIN:
    num_csv  = vars.acv()  - vars.cv_start()  - vars.cv();
    num_disv = vars.adiv() - vars.div_start() - vars.div();
    num_drsv = vars.adrv() - vars.drv_start() - vars.drv(); break;
  }
}


void NonDSampling::
mode_counts(const Model& model, size_t& cv_start, size_t& num_cv,
	    size_t& div_start, size_t& num_div,
	    size_t& drv_start, size_t& num_drv) const
{
  cv_start = num_cv = div_start = num_div = drv_start = num_drv = 0;
  switch (samplingVarsMode) {
  case ALEATORY_UNCERTAIN:
    // design vars define starting indices
    view_design_counts(model, cv_start, div_start, drv_start);
    // A uncertain vars define counts
    view_aleatory_uncertain_counts(model, num_cv, num_div, num_drv);      break;
  case ALEATORY_UNCERTAIN_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    size_t dummy;
    // continuous design vars define starting indices
    view_design_counts(model, cv_start, dummy, dummy);
    // continuous A uncertain vars define counts
    view_aleatory_uncertain_counts(model, num_cv, dummy, dummy);          break;
  }
  case EPISTEMIC_UNCERTAIN: {
    // design + A uncertain vars define starting indices
    size_t num_cdv, num_ddiv, num_ddrv, num_cauv, num_diauv, num_drauv;
    view_design_counts(model, num_cdv, num_ddiv, num_ddrv);
    view_aleatory_uncertain_counts(model, num_cauv, num_diauv, num_drauv);
    cv_start  = num_cdv  + num_cauv;
    div_start = num_ddiv + num_diauv;
    drv_start = num_ddrv + num_drauv;
    // E uncertain vars define counts
    view_epistemic_uncertain_counts(model, num_cv, num_div, num_drv);     break;
  }
  case EPISTEMIC_UNCERTAIN_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    // continuous design + A uncertain vars define starting indices
    size_t num_cdv, num_cauv, dummy;
    view_design_counts(model, num_cdv, dummy, dummy);
    view_aleatory_uncertain_counts(model, num_cauv, dummy, dummy);
    cv_start = num_cdv + num_cauv;
    // continuous E uncertain vars define counts
    view_epistemic_uncertain_counts(model, num_cv, dummy, dummy);         break;
  }
  case UNCERTAIN:
    // design vars define starting indices
    view_design_counts(model, cv_start, div_start, drv_start);
    // A+E uncertain vars define counts
    view_uncertain_counts(model, num_cv, num_div, num_drv);               break;
  case UNCERTAIN_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    size_t dummy;
    // continuous design vars define starting indices
    view_design_counts(model, cv_start, dummy, dummy);
    // continuous A+E uncertain vars define counts
    view_uncertain_counts(model, num_cv, dummy, dummy);                   break;
  }
  case ACTIVE: {
    const Variables& vars = model.current_variables();
    cv_start  = vars.cv_start();  num_cv  = vars.cv();
    div_start = vars.div_start(); num_div = vars.div();
    drv_start = vars.drv_start(); num_drv = vars.drv();                   break;
  }
  case ACTIVE_UNIFORM: {
    // UNIFORM views do not currently support non-relaxed discrete
    const Variables& vars = model.current_variables();
    cv_start = vars.cv_start(); num_cv = vars.cv();                       break;
  }
  case ALL:
    num_cv = model.acv(); num_div = model.adiv(); num_drv = model.adrv(); break;
  case ALL_UNIFORM:
    // UNIFORM views do not currently support non-relaxed discrete
    num_cv = model.acv();                                                 break;
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
compute_statistics(const RealMatrix&     vars_samples,
		   const IntResponseMap& resp_samples)
{
  StringMultiArrayConstView
    acv_labels  = iteratedModel.all_continuous_variable_labels(),
    adiv_labels = iteratedModel.all_discrete_int_variable_labels(),
    adrv_labels = iteratedModel.all_discrete_real_variable_labels();
  size_t cv_start, num_cv, div_start, num_div, drv_start, num_drv;
  mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
	      drv_start, num_drv);
  StringMultiArrayConstView
    cv_labels  =
      acv_labels[boost::indices[idx_range(cv_start, cv_start+num_cv)]],
    div_labels =
      adiv_labels[boost::indices[idx_range(div_start, div_start+num_div)]],
    drv_labels =
      adrv_labels[boost::indices[idx_range(drv_start, drv_start+num_drv)]];

  // archive the active variables with the results
  if (resultsDB.active()) {
    if (num_cv)
      resultsDB.insert(run_identifier(), resultsNames.cv_labels, cv_labels);
    if (num_div)
      resultsDB.insert(run_identifier(), resultsNames.div_labels, div_labels);
    if (num_cv)
      resultsDB.insert(run_identifier(), resultsNames.drv_labels, drv_labels);
    resultsDB.insert(run_identifier(), resultsNames.fn_labels, 
		     iteratedModel.response_labels());
  }

  if (numEpistemicUncVars) // Epistemic/mixed
    compute_intervals(resp_samples); // compute min/max response intervals
  else { // Aleatory
    // compute means and std deviations with confidence intervals
    compute_moments(resp_samples);
    // compute CDF/CCDF mappings of z to p/beta and p/beta to z
    if (totalLevelRequests)
      compute_distribution_mappings(resp_samples);
  }

  if (!subIteratorFlag) {
    nonDSampCorr.compute_correlations(vars_samples, resp_samples);
    // archive the correlations to the results DB
    nonDSampCorr.archive_correlations(run_identifier(), resultsDB,
				      cv_labels, div_labels, drv_labels,
				      iteratedModel.response_labels());
  }
  if (!finalStatistics.is_null())
    update_final_statistics();
}


void NonDSampling::compute_intervals(const IntResponseMap& samples)
{
  // For the samples array, calculate min/max response intervals

  using boost::math::isfinite;
  size_t i, j, num_obs = samples.size(), num_samp;
  const StringArray& resp_labels = iteratedModel.response_labels();

  if (extremeValues.empty()) extremeValues.shapeUninitialized(2, numFunctions);
  IntRespMCIter it;
  for (i=0; i<numFunctions; ++i) {
    num_samp = 0;
    Real min = DBL_MAX, max = -DBL_MAX;
    for (it=samples.begin(); it!=samples.end(); ++it) {
      const Real& sample = it->second.function_value(i);
      if (isfinite(sample)) { // neither NaN nor +/-Inf
	if (sample < min) min = sample;
	if (sample > max) max = sample;
	++num_samp;
      }
    }
    extremeValues(0, i) = min;
    extremeValues(1, i) = max;
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
		     extremeValues, md);
  }
}


void NonDSampling::compute_moments(const IntResponseMap& samples)
{
  // For the samples array, calculate means and standard deviations
  // with confidence intervals

  using boost::math::isfinite;
  size_t i, j, num_obs = samples.size(), num_samp;
  Real sum, var, skew, kurt;
  const StringArray& resp_labels = iteratedModel.response_labels();

  if (momentStats.empty()) momentStats.shapeUninitialized(4, numFunctions);
  if (momentCIs.empty()) momentCIs.shapeUninitialized(4, numFunctions);

  IntRespMCIter it;
  for (i=0; i<numFunctions; ++i) {

    num_samp  = 0;
    sum = var = skew = kurt = 0.;
    // means
    for (it=samples.begin(); it!=samples.end(); ++it) {
      const Real& sample = it->second.function_value(i);
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

    Real* moments_i = momentStats[i];
    Real& mean = moments_i[0];
    mean = sum/((Real)num_samp);

    // accumulate variance, skewness, and kurtosis
    Real centered_fn, pow_fn;
    for (it=samples.begin(); it!=samples.end(); ++it) {
      const Real& sample = it->second.function_value(i);
      if (isfinite(sample)) { // neither NaN nor +/-Inf
	pow_fn  = centered_fn = sample - mean;
	pow_fn *= centered_fn; var  += pow_fn;
	pow_fn *= centered_fn; skew += pow_fn;
	pow_fn *= centered_fn; kurt += pow_fn;
      }
    }

    // sample std deviation
    Real& std_dev = moments_i[1];
    std_dev = (num_samp > 1) ? std::sqrt(var/(Real)(num_samp-1)) : 0.;

    // skewness
    moments_i[2] = (num_samp > 2 && var > 0.) ? 
      // sample skewness
      skew/(Real)num_samp/std::pow(var/(Real)num_samp,1.5) *
      // population skewness 
      std::sqrt((Real)(num_samp*(num_samp-1)))/(Real)(num_samp-2) :
      // for no variation, central moment is zero
      0.;

    // kurtosis
    moments_i[3] = (num_samp > 3 && var > 0.) ?
      // sample kurtosis
      (Real)((num_samp+1)*num_samp*(num_samp-1))*kurt/
      (Real)((num_samp-2)*(num_samp-3)*var*var) -
      // population kurtosis
      3.*std::pow((Real)(num_samp-1),2)/(Real)((num_samp-2)*(num_samp-3)) :
      // for no variation, central moment is zero minus excess kurtosis
      -3.;

    if (num_samp > 1) {
      // 95% confidence intervals (2-sided interval, not 1-sided limit)
      Real dof = num_samp - 1;
//#ifdef HAVE_BOOST
      // mean: the better formula does not assume known variance but requires
      // a function for the Student's t-distr. with (num_samp-1) degrees of
      // freedom (Haldar & Mahadevan, p. 127).
      Pecos::students_t_dist t_dist(dof);
      Real mean_ci_delta = 
	std_dev*bmth::quantile(t_dist,0.975)/std::sqrt((Real)num_samp);
      momentCIs(0,i) = mean - mean_ci_delta;
      momentCIs(1,i) = mean + mean_ci_delta;
      // std dev: chi-square distribution with (num_samp-1) degrees of freedom
      // (Haldar & Mahadevan, p. 132).
      Pecos::chi_squared_dist chisq(dof);
      momentCIs(2,i) = std_dev*std::sqrt(dof/bmth::quantile(chisq, 0.975));
      momentCIs(3,i) = std_dev*std::sqrt(dof/bmth::quantile(chisq, 0.025));
/*
#elif HAVE_GSL
      // mean: the better formula does not assume known variance but requires
      // a function for the Student's t-distr. with (num_samp-1) degrees of
      // freedom (Haldar & Mahadevan, p. 127).
      mean95CIDeltas[i]
	= std_dev*gsl_cdf_tdist_Pinv(0.975,dof)/std::sqrt((Real)num_samp);
      // std dev: chi-square distribution with (num_samp-1) degrees of freedom
      // (Haldar & Mahadevan, p. 132).
      stdDev95CILowerBnds[i]
        = std_dev*std::sqrt(dof/gsl_cdf_chisq_Pinv(0.975, dof));
      stdDev95CIUpperBnds[i]
        = std_dev*std::sqrt(dof/gsl_cdf_chisq_Pinv(0.025, dof));
#else
      // mean: k_(alpha/2) = Phi^(-1)(0.975) = 1.96 (Haldar & Mahadevan,
      // p. 123).  This simple formula assumes a known variance, which
      // requires a sample of sufficient size (i.e., greater than 10).
      mean95CIDeltas[i] = 1.96*std_dev/std::sqrt((Real)num_samp);
#endif // HAVE_BOOST
*/
    }
    else
      momentCIs(0,i) = momentCIs(1,i) = momentCIs(2,i) = momentCIs(3,i) = 0.0;
  }

  if (resultsDB.active()) {
    // archive the moments to results DB
    MetaDataType md_moments;
    md_moments["Row Labels"] = 
      make_metadatavalue("Mean", "Standard Deviation", "Skewness", "Kurtosis");
    md_moments["Column Labels"] = make_metadatavalue(resp_labels);
    resultsDB.insert(run_identifier(), resultsNames.moments_std, 
		     momentStats, md_moments);
    // archive the confidence intervals to results DB
    MetaDataType md;
    md["Row Labels"] = 
      make_metadatavalue("LowerCI_Mean", "UpperCI_Mean", "LowerCI_StdDev", 
			 "UpperCI_StdDev");
    md["Column Labels"] = make_metadatavalue(resp_labels);
    resultsDB.insert(run_identifier(), resultsNames.moment_cis, momentCIs, md);
  }
}


void NonDSampling::compute_distribution_mappings(const IntResponseMap& samples)
{
  // Size the output arrays here instead of in the ctor in order to support
  // alternate sampling ctors.
  initialize_distribution_mappings();
  archive_allocate_mappings();
  if (pdfOutput) {
    computedPDFAbscissas.resize(numFunctions);
    computedPDFOrdinates.resize(numFunctions);
    archive_allocate_pdf();
  }

  // For the samples array, calculate the following statistics:
  // > CDF/CCDF mappings of response levels to probability/reliability levels
  // > CDF/CCDF mappings of probability/reliability levels to response levels
  using boost::math::isfinite;
  size_t i, j, k, num_obs = samples.size(), num_samp, bin_accumulator;
  const StringArray& resp_labels = iteratedModel.response_labels();
  RealArray sorted_samples; // STL-based array for sorting
  SizetArray bins; Real min, max;

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
      abort_handler(-1);
      // Issue with the following approach is that subsequent invocations of
      // compute_distribution_mappings() without compute_moments() would not
      // be detected and old moments would be used.  Performing more rigorous
      // bookkeeping of moment updates is overkill for current use cases.
      //Cerr << "Warning: moments not available in compute_distribution_"
      //     << "mappings(); computing them now." << std::endl;
      //compute_moments(samples);
    }
  }

  IntRespMCIter it;
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
      sorted_samples.clear(); sorted_samples.reserve(num_obs);
      for (it=samples.begin(); it!=samples.end(); ++it) {
	const Real& sample = it->second.function_value(i);
	if (isfinite(sample))
	  { ++num_samp; sorted_samples.push_back(sample); }
      }
      // sort in ascending order
      std::sort(sorted_samples.begin(), sorted_samples.end());
      if (pdfOutput)
	{ min = sorted_samples[0]; max = sorted_samples[num_samp-1]; }
      // in case of rl_len mixed with pl_len/gl_len, bin using sorted array.
      // Note: all bins open on right end due to use of less than.
      if (rl_len && respLevelTarget != RELIABILITIES) {
	const RealVector& req_rl_i = requestedRespLevels[i];
        bins.assign(rl_len+1, 0); size_t samp_cntr = 0;
	for (j=0; j<rl_len; ++j)
	  while (samp_cntr<num_samp && sorted_samples[samp_cntr]<req_rl_i[j])
	    { ++bins[j]; ++samp_cntr; }
	if (num_samp > samp_cntr)
	  bins[rl_len] += num_samp - samp_cntr;
      }
    }
    else if (rl_len && respLevelTarget != RELIABILITIES) {
      // in case of rl_len without pl_len/gl_len, bin from original sample set
      const RealVector& req_rl_i = requestedRespLevels[i];
      bins.assign(rl_len+1, 0); min = DBL_MAX; max = -DBL_MAX;
      for (it=samples.begin(); it!=samples.end(); ++it) {
	const Real& sample = it->second.function_value(i);
	if (isfinite(sample)) {
	  ++num_samp;
	  if (pdfOutput) {
	    if (sample < min) min = sample;
	    if (sample > max) max = sample;
	  }
	  // 1st PDF bin from -inf to 1st resp lev; last PDF bin from last resp
	  // lev to +inf. Note: all bins open on right end due to use of <.
	  bool found = false;
	  for (k=0; k<rl_len; ++k)
	    if (sample < req_rl_i[k])
	      { ++bins[k]; found = true; break; }
	  if (!found)
	    ++bins[rl_len];
	}
      }
    }

    // ----------------
    // Process mappings
    // ----------------
    if (rl_len) {
      switch (respLevelTarget) {
      case PROBABILITIES: case GEN_RELIABILITIES: {
	// z -> p/beta* (based on binning)
	bin_accumulator = 0;
	for (j=0; j<rl_len; ++j) { // compute CDF/CCDF p/beta*
	  bin_accumulator += bins[j];
	  Real cdf_prob = (Real)bin_accumulator/(Real)num_samp;
	  Real computed_prob = (cdfFlag) ? cdf_prob : 1. - cdf_prob;
	  if (respLevelTarget == PROBABILITIES)
	    computedProbLevels[i][j]   =  computed_prob;
	  else
	    computedGenRelLevels[i][j] = -Pecos::Phi_inverse(computed_prob);
	}
	break;
      }
      case RELIABILITIES: { // z -> beta (based on moment projection)
	Real& mean = momentStats(0,i); Real& std_dev = momentStats(1,i);
	for (j=0; j<rl_len; j++) {
	  const Real& z = requestedRespLevels[i][j];
	  if (std_dev > Pecos::SMALL_NUMBER) {
	    Real ratio = (mean - z)/std_dev;
	    computedRelLevels[i][j] = (cdfFlag) ? ratio : -ratio;
	  }
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
    if (bl_len) {
      Real& mean = momentStats(0,i); Real& std_dev = momentStats(1,i);
      for (j=0; j<bl_len; j++) { // beta -> z
	const Real& beta = requestedRelLevels[i][j];
	computedRespLevels[i][j+pl_len] = (cdfFlag) ?
	  mean - beta * std_dev : mean + beta * std_dev;
      }
    }

    // archive the mappings from response levels
    archive_from_resp(i);
    // archive the mappings to response levels
    archive_to_resp(i);

    // ---------------------------------------------------------------------
    // Post-process for PDF incorporating all requested/computed resp levels
    // ---------------------------------------------------------------------
    if (pdfOutput) {
      size_t req_comp_rl_len = pl_len + gl_len;
      if (respLevelTarget != RELIABILITIES) req_comp_rl_len += rl_len;
      if (req_comp_rl_len) {
	RealVector pdf_all_rlevs;
	if (pl_len || gl_len) {
	  // merge all requested & computed rlevs into pdf rlevs and sort
	  pdf_all_rlevs.sizeUninitialized(req_comp_rl_len);
	  // merge requested/computed --> pdf_all_rlevs
	  int offset = 0;
	  if (rl_len && respLevelTarget != RELIABILITIES) {
	    copy_data_partial(requestedRespLevels[i], pdf_all_rlevs, 0);
	    offset += rl_len;
	  }
	  if (pl_len) {
	    copy_data_partial(computedRespLevels[i], 0, (int)pl_len,
			      pdf_all_rlevs, offset);
	    offset += pl_len;
	  }
	  if (gl_len)
	    copy_data_partial(computedRespLevels[i], (int)(pl_len+bl_len),
			      (int)gl_len, pdf_all_rlevs, offset);
	  // sort combined array; retain unique entries; update req_comp_rl_len
	  Real* start = pdf_all_rlevs.values();
	  std::sort(start, start+req_comp_rl_len);
	  req_comp_rl_len = std::distance(start,
	    std::unique(start, start+req_comp_rl_len));
	  // (re)compute bins from sorted_samples.  Note that these bins are
	  // open on right end due to use of strictly less than.
	  bins.assign(req_comp_rl_len+1, 0); size_t samp_cntr = 0;
	  for (j=0; j<req_comp_rl_len; ++j)
	    while (samp_cntr < num_samp &&
		   sorted_samples[samp_cntr] < pdf_all_rlevs[j])
	      { ++bins[j]; ++samp_cntr; }
	  if (num_samp > samp_cntr)
	    bins[req_comp_rl_len] += num_samp - samp_cntr;
	}
	RealVector& pdf_rlevs = (pl_len || gl_len) ?
	  pdf_all_rlevs : requestedRespLevels[i];
	size_t last_rl_index = req_comp_rl_len-1;
	const Real& lev_0    = pdf_rlevs[0];
	const Real& lev_last = pdf_rlevs[last_rl_index];
	// to properly sum to 1, final PDF bin must be closed on right end.
	// --> where the max sample value defines the last response level,
	//     move any max samples on right boundary inside last PDF bin.
	if (max <= lev_last && bins[req_comp_rl_len]) {
	  bins[req_comp_rl_len-1] += bins[req_comp_rl_len];
	  bins[req_comp_rl_len]    = 0;
	}

	// compute computedPDF{Abscissas,Ordinates} from bin counts and widths
	size_t pdf_size = last_rl_index;
	if (min < lev_0)    ++pdf_size;
	if (max > lev_last) ++pdf_size;
	RealVector& abs_i = computedPDFAbscissas[i]; abs_i.resize(pdf_size+1);
	RealVector& ord_i = computedPDFOrdinates[i]; ord_i.resize(pdf_size);
	size_t offset = 0;
	if (min < lev_0) {
	  abs_i[0] = min;
	  ord_i[0] = (Real)bins[0]/(Real)num_samp/(lev_0 - min);
	  offset = 1;
	}
	for (j=0; j<last_rl_index; ++j) {
	  abs_i[j+offset] = pdf_rlevs[j];
	  ord_i[j+offset]
	    = (Real)bins[j+1]/(Real)num_samp/(pdf_rlevs[j+1] - pdf_rlevs[j]);
	}
	if (max > lev_last) {
	  abs_i[pdf_size-1] = pdf_rlevs[last_rl_index];
	  abs_i[pdf_size]   = max;
	  ord_i[pdf_size-1]
	    = (Real)bins[req_comp_rl_len]/(Real)num_samp/(max - lev_last);
	}
	else
	  abs_i[pdf_size] = pdf_rlevs[last_rl_index];
      }
      archive_pdf(i);
    }
  }
}


void NonDSampling::update_final_statistics()
{
  //if (finalStatistics.is_null())
  //  initialize_final_statistics();

  if (numEpistemicUncVars) {
    size_t i, cntr = 0;
    for (i=0; i<numFunctions; ++i) {
      finalStatistics.function_value(extremeValues(0, i), cntr++);
      finalStatistics.function_value(extremeValues(1, i), cntr++);
    }
  }
  else // moments + level mappings
    NonD::update_final_statistics();
}


void NonDSampling::print_statistics(std::ostream& s) const
{
  if (numEpistemicUncVars) // output only min & max values in the epistemic case
    print_intervals(s);
  else {
    print_moments(s);
    if (totalLevelRequests) {
      print_distribution_mappings(s);
      if (pdfOutput)
	print_pdf_mappings(s);
      print_system_mappings(s);
    }
  }
  if (!subIteratorFlag) {
    StringMultiArrayConstView
      acv_labels  = iteratedModel.all_continuous_variable_labels(),
      adiv_labels = iteratedModel.all_discrete_int_variable_labels(),
      adrv_labels = iteratedModel.all_discrete_real_variable_labels();
    size_t cv_start, num_cv, div_start, num_div, drv_start, num_drv;
    mode_counts(iteratedModel, cv_start, num_cv, div_start, num_div,
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

  s << std::scientific << std::setprecision(write_precision)
    << "\nMin and Max values for each response function:\n";
  for (size_t i=0; i<numFunctions; ++i)
    s << resp_labels[i] << ":  Min = " << extremeValues(0, i)
      << "  Max = " << extremeValues(1, i) << '\n';
}


void NonDSampling::print_moments(std::ostream& s) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();

  s << std::scientific << std::setprecision(write_precision);

  size_t i, j, width = write_precision+7;
 
  s << "\nMoment-based statistics for each response function:\n"
    << std::setw(width+15) << "Mean"     << std::setw(width+1) << "Std Dev"
    << std::setw(width+1)  << "Skewness" << std::setw(width+2) << "Kurtosis\n";
  //<< std::setw(width+2)  << "Coeff of Var\n";
  for (i=0; i<numFunctions; ++i) {
    const Real* moments_i = momentStats[i];
    s << std::setw(14) << resp_labels[i];
    for (j=0; j<4; ++j)
      s << ' ' << std::setw(width) << moments_i[j];
    s << '\n';
  }
  if (numSamples > 1) {
    // output 95% confidence intervals as (,) interval
    s << "\n95% confidence intervals for each response function:\n"
      << std::setw(width+15) << "LowerCI_Mean" << std::setw(width+1)
      << "UpperCI_Mean" << std::setw(width+1)  << "LowerCI_StdDev" 
      << std::setw(width+2) << "UpperCI_StdDev\n";
    for (i=0; i<numFunctions; ++i)
      s << std::setw(14) << resp_labels[i]
	<< ' ' << std::setw(width) << momentCIs(0, i)
	<< ' ' << std::setw(width) << momentCIs(1, i)
	<< ' ' << std::setw(width) << momentCIs(2, i)
	<< ' ' << std::setw(width) << momentCIs(3, i) << '\n';
  }
}


void NonDSampling::print_pdf_mappings(std::ostream& s) const
{
  const StringArray& resp_labels = iteratedModel.response_labels();

  // output CDF/CCDF probabilities resulting from binning or CDF/CCDF
  // reliabilities resulting from number of std devs separating mean & target
  s << std::scientific << std::setprecision(write_precision)
    << "\nProbability Density Function (PDF) histograms for each response "
    << "function:\n";
  size_t i, j, width = write_precision+7;
  for (i=0; i<numFunctions; ++i) {
    if (!requestedRespLevels[i].empty() || !computedRespLevels[i].empty()) {
      s << "PDF for " << resp_labels[i] << ":\n"
	<< "          Bin Lower          Bin Upper      Density Value\n"
	<< "          ---------          ---------      -------------\n";

      size_t pdf_len = computedPDFOrdinates[i].length();
      for (j=0; j<pdf_len; ++j)
	s << "  " << std::setw(width) << computedPDFAbscissas[i][j] << "  "
	  << std::setw(width) << computedPDFAbscissas[i][j+1] << "  "
	  << std::setw(width) << computedPDFOrdinates[i][j] << '\n';
    }
  }
}


void NonDSampling::archive_allocate_pdf() // const
{
  if (!resultsDB.active())  return;

  // pdf per function, possibly empty
  MetaDataType md;
  md["Array Spans"] = make_metadatavalue("Response Functions");
  md["Row Labels"] = 
    make_metadatavalue("Bin Lower", "Bin Upper", "Density Value");
  resultsDB.array_allocate<RealMatrix>
    (run_identifier(), resultsNames.pdf_histograms, numFunctions, md);
}


void NonDSampling::archive_pdf(size_t i) // const
{
  if (!resultsDB.active()) return;

  size_t pdf_len = computedPDFOrdinates[i].length();
  RealMatrix pdf(3, pdf_len);
  for (size_t j=0; j<pdf_len; ++j) {
    pdf(0, j) = computedPDFAbscissas[i][j];
    pdf(1, j) = computedPDFAbscissas[i][j+1];
    pdf(2, j) = computedPDFOrdinates[i][j];
  }
  
  resultsDB.array_insert<RealMatrix>
    (run_identifier(), resultsNames.pdf_histograms, i, pdf);
}

} // namespace Dakota

