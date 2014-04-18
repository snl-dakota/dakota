/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedVariablesData
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "SharedVariablesData.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_util.hpp"

//#define REFCOUNT_DEBUG

static const char rcsId[]="@(#) $Id: SharedVariablesData.cpp 6886 2010-08-02 19:13:01Z mseldre $";

namespace Dakota {


/** This constructor is the one which must build the base class data for all
    derived classes.  get_variables() instantiates a derived class letter
    and the derived constructor selects this base class constructor in its 
    initialization list (to avoid the recursion of the base class constructor
    calling get_variables() again).  Since the letter IS the representation, 
    its representation pointer is set to NULL (an uninitialized pointer causes
    problems in ~Variables). */
SharedVariablesDataRep::
SharedVariablesDataRep(const ProblemDescDB& problem_db,
		       const std::pair<short,short>& view):
  variablesId(problem_db.get_string("variables.id")),
  variablesCompsTotals(12, 0), variablesView(view), cvStart(0), divStart(0),
  drvStart(0), icvStart(0), idivStart(0), idrvStart(0), numCV(0), numDIV(0),
  numDRV(0), numICV(0), numIDIV(0), numIDRV(0), referenceCount(1)
{
  size_t num_cdv, num_ddrv, num_ddsiv, num_ddsrv, num_dsrv, num_dssiv, count;
  // continuous design
  if (num_cdv = problem_db.get_sizet("variables.continuous_design")) {
    variablesComponents[CONTINUOUS_DESIGN] = num_cdv;
    variablesCompsTotals[0] += num_cdv;
  }
  // discrete integer design
  if (num_ddrv = problem_db.get_sizet("variables.discrete_design_range")) {
    variablesComponents[DISCRETE_DESIGN_RANGE] = num_ddrv;
    variablesCompsTotals[1] += num_ddrv;
  }
  if (num_ddsiv = problem_db.get_sizet("variables.discrete_design_set_int")) {
    variablesComponents[DISCRETE_DESIGN_SET_INT] = num_ddsiv;
    variablesCompsTotals[1] += num_ddsiv;
  }
  // discrete real design
  if (num_ddsrv = problem_db.get_sizet("variables.discrete_design_set_real")) {
    variablesComponents[DISCRETE_DESIGN_SET_REAL] = num_ddsrv;
    variablesCompsTotals[2] += num_ddsrv;
  }
  // continuous aleatory uncertain
  if (count = problem_db.get_sizet("variables.normal_uncertain")) {
    variablesComponents[NORMAL_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.lognormal_uncertain")) {
    variablesComponents[LOGNORMAL_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.uniform_uncertain")) {
    variablesComponents[UNIFORM_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.loguniform_uncertain")) {
    variablesComponents[LOGUNIFORM_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.triangular_uncertain")) {
    variablesComponents[TRIANGULAR_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.exponential_uncertain")) {
    variablesComponents[EXPONENTIAL_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.beta_uncertain")) {
    variablesComponents[BETA_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.gamma_uncertain")) {
    variablesComponents[GAMMA_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.gumbel_uncertain")) {
    variablesComponents[GUMBEL_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.frechet_uncertain")) {
    variablesComponents[FRECHET_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.weibull_uncertain")) {
    variablesComponents[WEIBULL_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  if (count = problem_db.get_sizet("variables.histogram_uncertain.bin")) {
    variablesComponents[HISTOGRAM_BIN_UNCERTAIN] = count;
    variablesCompsTotals[3] += count;
  }
  // discrete integer aleatory uncertain
  if (count = problem_db.get_sizet("variables.poisson_uncertain")) {
    variablesComponents[POISSON_UNCERTAIN] = count;
    variablesCompsTotals[4] += count;
  }
  if (count = problem_db.get_sizet("variables.binomial_uncertain")) {
    variablesComponents[BINOMIAL_UNCERTAIN] = count;
    variablesCompsTotals[4] += count;
  }
  if (count = problem_db.get_sizet("variables.negative_binomial_uncertain")) {
    variablesComponents[NEGATIVE_BINOMIAL_UNCERTAIN] = count;
    variablesCompsTotals[4] += count;
  }
  if (count = problem_db.get_sizet("variables.geometric_uncertain")) {
    variablesComponents[GEOMETRIC_UNCERTAIN] = count;
    variablesCompsTotals[4] += count;
  }
  if (count = problem_db.get_sizet("variables.hypergeometric_uncertain")) {
    variablesComponents[HYPERGEOMETRIC_UNCERTAIN] = count;
    variablesCompsTotals[4] += count;
  }
  // discrete real aleatory uncertain
  if (count = problem_db.get_sizet("variables.histogram_uncertain.point")) {
    variablesComponents[HISTOGRAM_POINT_UNCERTAIN] = count;
    variablesCompsTotals[5] += count;
  }
  // continuous epistemic uncertain
  if (count = problem_db.get_sizet("variables.continuous_interval_uncertain")) {
    variablesComponents[CONTINUOUS_INTERVAL_UNCERTAIN] = count;
    variablesCompsTotals[6] += count;
  }
  // discrete integer epistemic uncertain
  if (count = problem_db.get_sizet("variables.discrete_interval_uncertain")) {
    variablesComponents[DISCRETE_INTERVAL_UNCERTAIN] = count;
    variablesCompsTotals[7] += count;
  }
  if (count = problem_db.get_sizet("variables.discrete_uncertain_set_int")) {
    variablesComponents[DISCRETE_UNCERTAIN_SET_INT] = count;
    variablesCompsTotals[7] += count;
  }
  // discrete real epistemic uncertain
  if (count = problem_db.get_sizet("variables.discrete_uncertain_set_real")) {
    variablesComponents[DISCRETE_UNCERTAIN_SET_REAL] = count;
    variablesCompsTotals[8] += count;
  }
  // continuous state
  if (count = problem_db.get_sizet("variables.continuous_state")) {
    variablesComponents[CONTINUOUS_STATE] = count;
    variablesCompsTotals[9] += count;
  }
  // discrete integer state
  if (num_dsrv = problem_db.get_sizet("variables.discrete_state_range")) {
    variablesComponents[DISCRETE_STATE_RANGE] = num_dsrv;
    variablesCompsTotals[10] += num_dsrv;
  }
  if (num_dssiv = problem_db.get_sizet("variables.discrete_state_set_int")) {
    variablesComponents[DISCRETE_STATE_SET_INT] = num_dssiv;
    variablesCompsTotals[10] += num_dssiv;
  }
  // discrete real state
  if (count = problem_db.get_sizet("variables.discrete_state_set_real")) {
    variablesComponents[DISCRETE_STATE_SET_REAL] = count;
    variablesCompsTotals[11] += count;
  }

  size_t num_cauv = variablesCompsTotals[3], num_ceuv = variablesCompsTotals[6],
    num_acv   = variablesCompsTotals[0] + num_cauv + num_ceuv
              + variablesCompsTotals[9],
    num_dauiv = variablesCompsTotals[4], num_deuiv = variablesCompsTotals[7],
    num_adiv  = variablesCompsTotals[1] + num_dauiv + num_deuiv
              + variablesCompsTotals[10],
    num_daurv = variablesCompsTotals[5], num_deurv = variablesCompsTotals[8],
    num_adrv  = variablesCompsTotals[2] + num_daurv + num_deurv
              + variablesCompsTotals[11];

  bool relax = ( variablesView.first == RELAXED_ALL ||
    ( variablesView.first >= RELAXED_DESIGN &&
      variablesView.first <= RELAXED_STATE ) );
  if (relax)
    num_acv += num_adiv + num_adrv;
  else {
    allDiscreteIntLabels.resize(boost::extents[num_adiv]);
    allDiscreteRealLabels.resize(boost::extents[num_adrv]);
  }
  allContinuousLabels.resize(boost::extents[num_acv]);

  int acv_offset = 0, adiv_offset = 0, adrv_offset = 0;
  // design
  copy_data_partial(problem_db.get_sa("variables.continuous_design.labels"),
    allContinuousLabels, acv_offset);
  acv_offset += num_cdv;
  if (relax) {
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_design_range.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_ddrv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_design_set_int.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_ddsiv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_design_set_real.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_ddsrv;
  }
  else {
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_design_range.labels"),
      allDiscreteIntLabels, adiv_offset);
    adiv_offset += num_ddrv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_design_set_int.labels"),
      allDiscreteIntLabels, adiv_offset);
    adiv_offset += num_ddsiv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_design_set_real.labels"),
      allDiscreteRealLabels, adrv_offset);
    adrv_offset += num_ddsrv;
  }
  // aleatory uncertain
  copy_data_partial(problem_db.get_sa(
    "variables.continuous_aleatory_uncertain.labels"),
    allContinuousLabels, acv_offset);
  acv_offset += num_cauv;
  if (relax) {
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_aleatory_uncertain_int.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_dauiv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_aleatory_uncertain_real.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_daurv;
  }
  else {
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_aleatory_uncertain_int.labels"),
      allDiscreteIntLabels, adiv_offset);
    adiv_offset += num_dauiv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_aleatory_uncertain_real.labels"),
      allDiscreteRealLabels, adrv_offset);
    adrv_offset += num_daurv;
  }
  // epistemic uncertain
  copy_data_partial(problem_db.get_sa(
    "variables.continuous_epistemic_uncertain.labels"),
    allContinuousLabels, acv_offset);
  acv_offset += num_ceuv;
  if (relax) {
    copy_data_partial(problem_db.get_sa(
     "variables.discrete_epistemic_uncertain_int.labels"),
     allContinuousLabels, acv_offset);
    acv_offset += num_deuiv;
    copy_data_partial(problem_db.get_sa(
     "variables.discrete_epistemic_uncertain_real.labels"),
     allContinuousLabels, acv_offset);
    acv_offset += num_deurv;
  }
  else {
    copy_data_partial(problem_db.get_sa(
     "variables.discrete_epistemic_uncertain_int.labels"),
     allDiscreteIntLabels, adiv_offset);
    adiv_offset += num_deuiv;
    copy_data_partial(problem_db.get_sa(
     "variables.discrete_epistemic_uncertain_real.labels"),
     allDiscreteRealLabels, adrv_offset);
    adrv_offset += num_deurv;
  }
  // state
  copy_data_partial(problem_db.get_sa("variables.continuous_state.labels"),
    allContinuousLabels, acv_offset);
  if (relax) {
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_state_range.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_dsrv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_state_set_int.labels"),
      allContinuousLabels, acv_offset);
    acv_offset += num_dssiv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_state_set_real.labels"),
      allContinuousLabels, acv_offset);
  }
  else {
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_state_range.labels"),
      allDiscreteIntLabels, adiv_offset);
    adiv_offset += num_dsrv;
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_state_set_int.labels"),
      allDiscreteIntLabels, adiv_offset);
    copy_data_partial(problem_db.get_sa(
      "variables.discrete_state_set_real.labels"),
      allDiscreteRealLabels, adrv_offset);
  }

  initialize_all_continuous_types(relax);
  initialize_all_continuous_ids(relax);
  if (!relax) {
    initialize_all_discrete_int_types();
    //initialize_all_discrete_int_ids();
    initialize_all_discrete_real_types();
    //initialize_all_discrete_real_ids();
  }

#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesDataRep::SharedVariablesDataRep(problem_db, view) "
       << "called to build body object." << std::endl;
#endif
}


SharedVariablesDataRep::
SharedVariablesDataRep(const std::pair<short,short>& view,
		       const SizetArray& vars_comps_totals):
  variablesCompsTotals(vars_comps_totals), variablesView(view), cvStart(0),
  divStart(0), drvStart(0), icvStart(0), idivStart(0), idrvStart(0), numCV(0),
  numDIV(0), numDRV(0), numICV(0), numIDIV(0), numIDRV(0), referenceCount(1)
{
  // totals are sufficient to size variables
  bool relax = ( variablesView.first == RELAXED_ALL ||
    ( variablesView.first >= RELAXED_DESIGN &&
      variablesView.first <= RELAXED_STATE ) );
  size_all_continuous_labels(relax);
  if (!relax) {
    size_all_discrete_int_labels();
    size_all_discrete_real_labels();
  }

  // totals are sufficient for forming ids
  initialize_all_continuous_ids(relax);
  //if (!relax) {
  //  initialize_all_discrete_int_ids();
  //  initialize_all_discrete_real_ids();
  //}

  // totals are insufficient for forming types

#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesDataRep::SharedVariablesDataRep(view, "
       << "vars_comps_totals) called to build body object." << std::endl;
#endif
}


SharedVariablesDataRep::
SharedVariablesDataRep(const std::pair<short,short>& view,
		       const std::map<unsigned short, size_t>& vars_comps):
  variablesComponents(vars_comps), variablesView(view), cvStart(0),
  divStart(0), drvStart(0), icvStart(0), idivStart(0), idrvStart(0), numCV(0),
  numDIV(0), numDRV(0), numICV(0), numIDIV(0), numIDRV(0), referenceCount(1)
{
  components_to_totals();

  // totals are sufficient to size variables
  bool relax = ( variablesView.first == RELAXED_ALL ||
    ( variablesView.first >= RELAXED_DESIGN &&
      variablesView.first <= RELAXED_STATE ) );
  size_all_continuous_labels(relax);
  initialize_all_continuous_types(relax); // Can only use default ordering...
  initialize_all_continuous_ids(relax);
  if (!relax) {
    size_all_discrete_int_labels();
    size_all_discrete_real_labels();
    initialize_all_discrete_int_types();
    //initialize_all_discrete_int_ids();
    initialize_all_discrete_real_types();
    //initialize_all_discrete_real_ids();
  }

#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesDataRep::SharedVariablesDataRep(view, vars_comps) "
       << "called to build body object." << std::endl;
#endif
}


void SharedVariablesDataRep::components_to_totals()
{
  variablesCompsTotals.resize(12);

  // continuous design
  variablesCompsTotals[0] = vc_lookup(CONTINUOUS_DESIGN);
  // discrete integer design
  variablesCompsTotals[1] = vc_lookup(DISCRETE_DESIGN_RANGE)
    + vc_lookup(DISCRETE_DESIGN_SET_INT);
  // discrete real design
  variablesCompsTotals[2] = vc_lookup(DISCRETE_DESIGN_SET_REAL);
  // continuous aleatory uncertain
  variablesCompsTotals[3] = vc_lookup(NORMAL_UNCERTAIN)
    + vc_lookup(LOGNORMAL_UNCERTAIN)   + vc_lookup(UNIFORM_UNCERTAIN)
    + vc_lookup(LOGUNIFORM_UNCERTAIN)  + vc_lookup(TRIANGULAR_UNCERTAIN)
    + vc_lookup(EXPONENTIAL_UNCERTAIN) + vc_lookup(BETA_UNCERTAIN)
    + vc_lookup(GAMMA_UNCERTAIN)       + vc_lookup(GUMBEL_UNCERTAIN)
    + vc_lookup(FRECHET_UNCERTAIN)     + vc_lookup(WEIBULL_UNCERTAIN)
    + vc_lookup(HISTOGRAM_BIN_UNCERTAIN);
  // discrete integer aleatory uncertain
  variablesCompsTotals[4] = vc_lookup(POISSON_UNCERTAIN)
    + vc_lookup(BINOMIAL_UNCERTAIN)  + vc_lookup(NEGATIVE_BINOMIAL_UNCERTAIN)
    + vc_lookup(GEOMETRIC_UNCERTAIN) + vc_lookup(HYPERGEOMETRIC_UNCERTAIN);
  // discrete real aleatory uncertain
  variablesCompsTotals[5] = vc_lookup(HISTOGRAM_POINT_UNCERTAIN);
  // continuous epistemic uncertain
  variablesCompsTotals[6] = vc_lookup(CONTINUOUS_INTERVAL_UNCERTAIN);
  // discrete integer epistemic uncertain
  variablesCompsTotals[7] = vc_lookup(DISCRETE_INTERVAL_UNCERTAIN)
    + vc_lookup(DISCRETE_UNCERTAIN_SET_INT);
  // discrete real epistemic uncertain
  variablesCompsTotals[8] = vc_lookup(DISCRETE_UNCERTAIN_SET_REAL);
  // continuous state
  variablesCompsTotals[9] = vc_lookup(CONTINUOUS_STATE);
  // discrete integer state
  variablesCompsTotals[10] = vc_lookup(DISCRETE_STATE_RANGE)
    + vc_lookup(DISCRETE_STATE_SET_INT);
  // discrete real state
  variablesCompsTotals[11] = vc_lookup(DISCRETE_STATE_SET_REAL);
}


void SharedVariablesDataRep::initialize_all_continuous_types(bool relax)
{
  size_t i, acv_cntr = 0, num_cdv = vc_lookup(CONTINUOUS_DESIGN),
    num_nuv  = vc_lookup(NORMAL_UNCERTAIN),
    num_lnuv = vc_lookup(LOGNORMAL_UNCERTAIN),
    num_uuv  = vc_lookup(UNIFORM_UNCERTAIN),
    num_luuv = vc_lookup(LOGUNIFORM_UNCERTAIN),
    num_tuv  = vc_lookup(TRIANGULAR_UNCERTAIN),
    num_exuv = vc_lookup(EXPONENTIAL_UNCERTAIN),
    num_beuv = vc_lookup(BETA_UNCERTAIN), num_gauv = vc_lookup(GAMMA_UNCERTAIN),
    num_guuv = vc_lookup(GUMBEL_UNCERTAIN),
    num_fuv  = vc_lookup(FRECHET_UNCERTAIN),
    num_wuv  = vc_lookup(WEIBULL_UNCERTAIN),
    num_hbuv = vc_lookup(HISTOGRAM_BIN_UNCERTAIN),
    num_ciuv = vc_lookup(CONTINUOUS_INTERVAL_UNCERTAIN),
    num_csv  = vc_lookup(CONTINUOUS_STATE),
    num_acv  = variablesCompsTotals[0]  + variablesCompsTotals[3] +
               variablesCompsTotals[6]  + variablesCompsTotals[9];

  if (relax) // include discrete design/uncertain/state
    num_acv += variablesCompsTotals[1]  + variablesCompsTotals[2] +
               variablesCompsTotals[4]  + variablesCompsTotals[5] +
               variablesCompsTotals[7]  + variablesCompsTotals[8] +
               variablesCompsTotals[10] + variablesCompsTotals[11];
  allContinuousTypes.resize(boost::extents[num_acv]);

  // DESIGN
  for (i=0; i<num_cdv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = CONTINUOUS_DESIGN;
  if (relax) {
    size_t num_ddrv  = vc_lookup(DISCRETE_DESIGN_RANGE),
           num_ddsiv = vc_lookup(DISCRETE_DESIGN_SET_INT),
           num_ddsrv = vc_lookup(DISCRETE_DESIGN_SET_REAL);
    for (i=0; i<num_ddrv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_DESIGN_RANGE;
    for (i=0; i<num_ddsiv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_DESIGN_SET_INT;
    for (i=0; i<num_ddsrv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_DESIGN_SET_REAL;
  }

  // ALEATORY UNCERTAIN
  for (i=0; i<num_nuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = NORMAL_UNCERTAIN;
  for (i=0; i<num_lnuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = LOGNORMAL_UNCERTAIN;
  for (i=0; i<num_uuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = UNIFORM_UNCERTAIN;
  for (i=0; i<num_luuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = LOGUNIFORM_UNCERTAIN;
  for (i=0; i<num_tuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = TRIANGULAR_UNCERTAIN;
  for (i=0; i<num_exuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = EXPONENTIAL_UNCERTAIN;
  for (i=0; i<num_beuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = BETA_UNCERTAIN;
  for (i=0; i<num_gauv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = GAMMA_UNCERTAIN;
  for (i=0; i<num_guuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = GUMBEL_UNCERTAIN;
  for (i=0; i<num_fuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = FRECHET_UNCERTAIN;
  for (i=0; i<num_wuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = WEIBULL_UNCERTAIN;
  for (i=0; i<num_hbuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = HISTOGRAM_BIN_UNCERTAIN;
  if (relax) {
    size_t num_puv  = vc_lookup(POISSON_UNCERTAIN),
           num_biuv = vc_lookup(BINOMIAL_UNCERTAIN),
           num_nbuv = vc_lookup(NEGATIVE_BINOMIAL_UNCERTAIN),
           num_geuv = vc_lookup(GEOMETRIC_UNCERTAIN),
           num_hguv = vc_lookup(HYPERGEOMETRIC_UNCERTAIN),
           num_hpuv = vc_lookup(HISTOGRAM_POINT_UNCERTAIN);
    for (i=0; i<num_puv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = POISSON_UNCERTAIN;
    for (i=0; i<num_biuv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = BINOMIAL_UNCERTAIN;
    for (i=0; i<num_nbuv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = NEGATIVE_BINOMIAL_UNCERTAIN;
    for (i=0; i<num_geuv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = GEOMETRIC_UNCERTAIN;
    for (i=0; i<num_hguv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = HYPERGEOMETRIC_UNCERTAIN;
    for (i=0; i<num_hpuv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = HISTOGRAM_POINT_UNCERTAIN;
  }

  // EPISTEMIC UNCERTAIN
  for (i=0; i<num_ciuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = CONTINUOUS_INTERVAL_UNCERTAIN;
  if (relax) {
    size_t num_diuv  = vc_lookup(DISCRETE_INTERVAL_UNCERTAIN),
           num_dusiv = vc_lookup(DISCRETE_UNCERTAIN_SET_INT),
           num_dusrv = vc_lookup(DISCRETE_UNCERTAIN_SET_REAL);
    for (i=0; i<num_diuv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_INTERVAL_UNCERTAIN;
    for (i=0; i<num_dusiv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_UNCERTAIN_SET_INT;
    for (i=0; i<num_dusrv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_UNCERTAIN_SET_REAL;
  }

  // STATE
  for (i=0; i<num_csv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = CONTINUOUS_STATE;
  if (relax) {
    size_t num_dsrv  = vc_lookup(DISCRETE_STATE_RANGE),
           num_dssiv = vc_lookup(DISCRETE_STATE_SET_INT),
           num_dssrv = vc_lookup(DISCRETE_STATE_SET_REAL);
    for (i=0; i<num_dsrv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_STATE_RANGE;
    for (i=0; i<num_dssiv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_STATE_SET_INT;
    for (i=0; i<num_dssrv; ++i, ++acv_cntr)
      allContinuousTypes[acv_cntr] = DISCRETE_STATE_SET_REAL;
  }
}


void SharedVariablesDataRep::initialize_all_continuous_ids(bool relax)
{
  size_t i, id, acv_cntr = 0, num_cdv = variablesCompsTotals[0],
    num_ddv  = variablesCompsTotals[1] + variablesCompsTotals[2],
    num_cauv = variablesCompsTotals[3],
    num_dauv = variablesCompsTotals[4] + variablesCompsTotals[5],
    num_ceuv = variablesCompsTotals[6],
    num_deuv = variablesCompsTotals[7] + variablesCompsTotals[8],
    num_csv  = variablesCompsTotals[9],
    num_dsv  = variablesCompsTotals[10] + variablesCompsTotals[11],
    num_acv  = num_cdv + num_cauv + num_ceuv + num_csv;

  if (relax) // include discrete design/uncertain/state
    num_acv += num_ddv + num_dauv + num_deuv + num_dsv;
  allContinuousIds.resize(boost::extents[num_acv]);

  // DESIGN
  id = 1;
  for (i=0; i<num_cdv; ++i, ++acv_cntr, ++id)
    allContinuousIds[acv_cntr] = id;
  if (relax)
    for (i=0; i<num_ddv; ++i, ++acv_cntr, ++id)
      allContinuousIds[acv_cntr] = id;
  else
    id += num_ddv;

  // ALEATORY UNCERTAIN
  for (i=0; i<num_cauv; ++i, ++acv_cntr, ++id)
    allContinuousIds[acv_cntr] = id;
  if (relax)
    for (i=0; i<num_dauv; ++i, ++acv_cntr, ++id)
      allContinuousIds[acv_cntr] = id;
  else
    id += num_dauv;

  // EPISTEMIC UNCERTAIN
  for (i=0; i<num_ceuv; ++i, ++acv_cntr, ++id)
    allContinuousIds[acv_cntr] = id;
  if (relax)
    for (i=0; i<num_deuv; ++i, ++acv_cntr, ++id)
      allContinuousIds[acv_cntr] = id;
  else
    id += num_deuv;

  // STATE
  for (i=0; i<num_csv; ++i, ++acv_cntr, ++id)
    allContinuousIds[acv_cntr] = id;
  if (relax)
    for (i=0; i<num_dsv; ++i, ++acv_cntr, ++id)
      allContinuousIds[acv_cntr] = id;

  // initialize relaxedDiscreteIds
  if (relax) {
    size_t i, offset = 1, cntr = 0,
      num_mdv  = num_cdv  + num_ddv,  num_mauv = num_cauv + num_dauv,
      num_meuv = num_ceuv + num_deuv, num_muv  = num_mauv + num_meuv;
    switch (variablesView.first) {
    case RELAXED_ALL:
      relaxedDiscreteIds.resize(num_ddv + num_dauv + num_deuv + num_dsv);
      offset += num_cdv;
      for (i=0; i<num_ddv; ++i, ++cntr)
      relaxedDiscreteIds[cntr] = i + offset;
      offset += num_ddv + num_cauv;
      for (i=0; i<num_dauv; ++i, ++cntr)
	relaxedDiscreteIds[cntr] = i + offset;
      offset += num_dauv + num_ceuv;
      for (i=0; i<num_deuv; ++i, ++cntr)
	relaxedDiscreteIds[cntr] = i + offset;
      offset += num_deuv + num_csv;
      for (i=0; i<num_dsv; ++i, ++cntr)
	relaxedDiscreteIds[cntr] = i + offset;
      break;
    case RELAXED_DESIGN:
      relaxedDiscreteIds.resize(num_ddv);
      offset += num_cdv;
      for (i=0; i<num_ddv; ++i)
	relaxedDiscreteIds[i] = i + offset;
      break;
    case RELAXED_ALEATORY_UNCERTAIN:
      relaxedDiscreteIds.resize(num_dauv);
      offset += num_mdv + num_cauv;
      for (i=0; i<num_dauv; ++i, ++cntr)
	relaxedDiscreteIds[i] = i + offset;
      break;
    case RELAXED_EPISTEMIC_UNCERTAIN:
      relaxedDiscreteIds.resize(num_deuv);
      offset += num_mdv + num_mauv + num_ceuv;
      for (i=0; i<num_deuv; ++i, ++cntr)
	relaxedDiscreteIds[i] = i + offset;
      break;
    case RELAXED_UNCERTAIN:
      relaxedDiscreteIds.resize(num_dauv + num_deuv);
      offset += num_mdv + num_cauv;
      for (i=0; i<num_dauv; ++i, ++cntr)
	relaxedDiscreteIds[cntr] = i + offset;
      offset += num_dauv + num_ceuv;
      for (i=0; i<num_deuv; ++i, ++cntr)
	relaxedDiscreteIds[cntr] = i + offset;
      break;
    case RELAXED_STATE:
      relaxedDiscreteIds.resize(num_dsv);
      offset += num_mdv + num_muv + num_csv;
      for (i=0; i<num_dsv; ++i)
	relaxedDiscreteIds[i] = i + offset;
      break;
    }
  }
}


void SharedVariablesDataRep::initialize_all_discrete_int_types()
{
  allDiscreteIntTypes.resize(
    boost::extents[variablesCompsTotals[1] + variablesCompsTotals[4] + 
		   variablesCompsTotals[7] + variablesCompsTotals[10]]);

  size_t i, adiv_cntr = 0, num_ddrv = vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_puv   = vc_lookup(POISSON_UNCERTAIN),
    num_biuv  = vc_lookup(BINOMIAL_UNCERTAIN),
    num_nbuv  = vc_lookup(NEGATIVE_BINOMIAL_UNCERTAIN),
    num_geuv  = vc_lookup(GEOMETRIC_UNCERTAIN),
    num_hguv  = vc_lookup(HYPERGEOMETRIC_UNCERTAIN),
    num_diuv  = vc_lookup(DISCRETE_INTERVAL_UNCERTAIN),
    num_dusiv = vc_lookup(DISCRETE_UNCERTAIN_SET_INT),
    num_dsrv  = vc_lookup(DISCRETE_STATE_RANGE),
    num_dssiv = vc_lookup(DISCRETE_STATE_SET_INT);
  // DESIGN
  for (i=0; i<num_ddrv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = DISCRETE_DESIGN_RANGE;
  for (i=0; i<num_ddsiv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = DISCRETE_DESIGN_SET_INT;
  // ALEATORY UNCERTAIN
  for (i=0; i<num_puv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = POISSON_UNCERTAIN;
  for (i=0; i<num_biuv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = BINOMIAL_UNCERTAIN;
  for (i=0; i<num_nbuv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = NEGATIVE_BINOMIAL_UNCERTAIN;
  for (i=0; i<num_geuv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = GEOMETRIC_UNCERTAIN;
  for (i=0; i<num_hguv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = HYPERGEOMETRIC_UNCERTAIN;
  // EPISTEMIC UNCERTAIN
  for (i=0; i<num_diuv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = DISCRETE_INTERVAL_UNCERTAIN;
  for (i=0; i<num_dusiv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = DISCRETE_UNCERTAIN_SET_INT;
  // STATE
  for (i=0; i<num_dsrv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = DISCRETE_STATE_RANGE;
  for (i=0; i<num_dssiv; ++i, ++adiv_cntr)
    allDiscreteIntTypes[adiv_cntr] = DISCRETE_STATE_SET_INT;
}


void SharedVariablesDataRep::initialize_all_discrete_real_types()
{
  allDiscreteRealTypes.resize(
    boost::extents[variablesCompsTotals[2] + variablesCompsTotals[5] + 
		   variablesCompsTotals[8] + variablesCompsTotals[11]]);

  size_t i, adrv_cntr = 0, num_ddsrv = vc_lookup(DISCRETE_DESIGN_SET_REAL),
    num_hpuv  = vc_lookup(HISTOGRAM_POINT_UNCERTAIN),
    num_dusrv = vc_lookup(DISCRETE_UNCERTAIN_SET_REAL),
    num_dssrv = vc_lookup(DISCRETE_STATE_SET_REAL);
  // DESIGN
  for (i=0; i<num_ddsrv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = DISCRETE_DESIGN_SET_REAL;
  // ALEATORY UNCERTAIN
  for (i=0; i<num_hpuv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = HISTOGRAM_POINT_UNCERTAIN;
  // EPISTEMIC UNCERTAIN
  for (i=0; i<num_dusrv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = DISCRETE_UNCERTAIN_SET_REAL;
  // STATE
  for (i=0; i<num_dssrv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = DISCRETE_STATE_SET_REAL;
}


void SharedVariablesDataRep::initialize_active_components()
{
  size_t i, j,
    num_cdv   = variablesCompsTotals[0],  num_ddiv  = variablesCompsTotals[1],
    num_ddrv  = variablesCompsTotals[2],  num_cauv  = variablesCompsTotals[3],
    num_dauiv = variablesCompsTotals[4],  num_daurv = variablesCompsTotals[5],
    num_ceuv  = variablesCompsTotals[6],  num_deuiv = variablesCompsTotals[7],
    num_deurv = variablesCompsTotals[8],  num_csv   = variablesCompsTotals[9],
    num_dsiv  = variablesCompsTotals[10], num_dsrv  = variablesCompsTotals[11],
    cv_end    = cvStart  + numCV,  div_end  = divStart + numDIV,
    drv_end   = drvStart + numDRV, acv_cntr = 0, adiv_cntr = 0, adrv_cntr = 0;

  activeVarsCompsTotals.resize(12);

  // design
  activeVarsCompsTotals[0] = (acv_cntr  >=  cvStart &&  acv_cntr <  cv_end)
                           ? num_cdv : 0;
  activeVarsCompsTotals[1] = (adiv_cntr >= divStart && adiv_cntr < div_end)
                           ? num_ddiv : 0;
  activeVarsCompsTotals[2] = (adrv_cntr >= drvStart && adrv_cntr < drv_end)
                           ? num_ddrv : 0;
  acv_cntr += num_cdv; adiv_cntr += num_ddiv; adrv_cntr += num_ddrv;
  // aleatory uncertain
  activeVarsCompsTotals[3] = (acv_cntr  >=  cvStart &&  acv_cntr <  cv_end)
                           ? num_cauv : 0;
  activeVarsCompsTotals[4] = (adiv_cntr >= divStart && adiv_cntr < div_end)
                           ? num_dauiv : 0;
  activeVarsCompsTotals[5] = (adrv_cntr >= drvStart && adrv_cntr < drv_end)
                           ? num_daurv : 0;
  acv_cntr += num_cauv; adiv_cntr += num_dauiv; adrv_cntr += num_daurv;
  // epistemic uncertain
  activeVarsCompsTotals[6] = (acv_cntr  >=  cvStart &&  acv_cntr <  cv_end)
                           ? num_ceuv : 0;
  activeVarsCompsTotals[7] = (adiv_cntr >= divStart && adiv_cntr < div_end)
                           ? num_deuiv : 0;
  activeVarsCompsTotals[8] = (adrv_cntr >= drvStart && adrv_cntr < drv_end)
                           ? num_deurv : 0;
  acv_cntr += num_ceuv; adiv_cntr += num_deuiv; adrv_cntr += num_deurv;
  // state
  activeVarsCompsTotals[9]  = (acv_cntr  >=  cvStart &&  acv_cntr <  cv_end)
                            ? num_csv : 0;
  activeVarsCompsTotals[10] = (adiv_cntr >= divStart && adiv_cntr < div_end)
                            ? num_dsiv : 0;
  activeVarsCompsTotals[11] = (adrv_cntr >= drvStart && adrv_cntr < drv_end)
                            ? num_dsrv : 0;
  //acv_cntr += num_csv; adiv_cntr += num_dsiv; adrv_cntr += num_dsrv;
}


void SharedVariablesDataRep::initialize_inactive_components()
{
  size_t i, j,
    num_cdv   = variablesCompsTotals[0],  num_ddiv  = variablesCompsTotals[1],
    num_ddrv  = variablesCompsTotals[2],  num_cauv  = variablesCompsTotals[3],
    num_dauiv = variablesCompsTotals[4],  num_daurv = variablesCompsTotals[5],
    num_ceuv  = variablesCompsTotals[6],  num_deuiv = variablesCompsTotals[7],
    num_deurv = variablesCompsTotals[8],  num_csv   = variablesCompsTotals[9],
    num_dsiv  = variablesCompsTotals[10], num_dsrv  = variablesCompsTotals[11],
    icv_end   = icvStart  + numICV,  idiv_end = idivStart + numIDIV,
    idrv_end  = idrvStart + numIDRV, acv_cntr = 0, adiv_cntr = 0, adrv_cntr = 0;

  inactiveVarsCompsTotals.resize(12);

  // design
  inactiveVarsCompsTotals[0] = (acv_cntr  >=  icvStart &&  acv_cntr <  icv_end)
                             ? num_cdv : 0;
  inactiveVarsCompsTotals[1] = (adiv_cntr >= idivStart && adiv_cntr < idiv_end)
                             ? num_ddiv : 0;
  inactiveVarsCompsTotals[2] = (adrv_cntr >= idrvStart && adrv_cntr < idrv_end)
                             ? num_ddrv : 0;
  acv_cntr += num_cdv; adiv_cntr += num_ddiv; adrv_cntr += num_ddrv;
  // aleatory uncertain
  inactiveVarsCompsTotals[3] = (acv_cntr  >=  icvStart &&  acv_cntr <  icv_end)
                             ? num_cauv : 0;
  inactiveVarsCompsTotals[4] = (adiv_cntr >= idivStart && adiv_cntr < idiv_end)
                             ? num_dauiv : 0;
  inactiveVarsCompsTotals[5] = (adrv_cntr >= idrvStart && adrv_cntr < idrv_end)
                             ? num_daurv : 0;
  acv_cntr += num_cauv; adiv_cntr += num_dauiv; adrv_cntr += num_daurv;
  // epistemic uncertain
  inactiveVarsCompsTotals[6] = (acv_cntr  >=  icvStart &&  acv_cntr <  icv_end)
                             ? num_ceuv : 0;
  inactiveVarsCompsTotals[7] = (adiv_cntr >= idivStart && adiv_cntr < idiv_end)
                             ? num_deuiv : 0;
  inactiveVarsCompsTotals[8] = (adrv_cntr >= idrvStart && adrv_cntr < idrv_end)
                             ? num_deurv : 0;
  acv_cntr += num_ceuv; adiv_cntr += num_deuiv; adrv_cntr += num_deurv;
  // state
  inactiveVarsCompsTotals[9]  = (acv_cntr  >=  icvStart &&  acv_cntr <  icv_end)
                              ? num_csv : 0;
  inactiveVarsCompsTotals[10] = (adiv_cntr >= idivStart && adiv_cntr < idiv_end)
                              ? num_dsiv : 0;
  inactiveVarsCompsTotals[11] = (adrv_cntr >= idrvStart && adrv_cntr < idrv_end)
                              ? num_dsrv : 0;
  //acv_cntr += num_csv; adiv_cntr += num_dsiv; adrv_cntr += num_dsrv;
}


/** Deep copies are used when recasting changes the nature of a
    Variables set. */
SharedVariablesData SharedVariablesData::copy() const
{
  // the handle class instantiates a new handle and a new body and copies
  // current attributes into the new body

#ifdef REFCOUNT_DEBUG
  Cout << "SharedVariablesData::copy() called to generate a deep copy with no "
       << "representation sharing." << std::endl;
#endif

  SharedVariablesData svd; // new handle: referenceCount=1, svdRep=NULL

  if (svdRep) {
    svd.svdRep = new SharedVariablesDataRep();

    svd.svdRep->variablesId             = svdRep->variablesId;
    svd.svdRep->variablesComponents     = svdRep->variablesComponents;
    svd.svdRep->variablesCompsTotals    = svdRep->variablesCompsTotals;
    svd.svdRep->activeVarsCompsTotals   = svdRep->activeVarsCompsTotals;
    svd.svdRep->inactiveVarsCompsTotals = svdRep->inactiveVarsCompsTotals;
    svd.svdRep->variablesView           = svdRep->variablesView;
    svd.svdRep->cvStart   = svdRep->cvStart;
    svd.svdRep->divStart  = svdRep->divStart;
    svd.svdRep->drvStart  = svdRep->drvStart;
    svd.svdRep->icvStart  = svdRep->icvStart;
    svd.svdRep->idivStart = svdRep->idivStart;
    svd.svdRep->idrvStart = svdRep->idrvStart;
    size_t num_cv, num_div, num_drv;
    num_cv  = svd.svdRep->numCV  = svdRep->numCV;
    num_div = svd.svdRep->numDIV = svdRep->numDIV;
    num_drv = svd.svdRep->numDRV = svdRep->numDRV;
    svd.svdRep->numICV  = svdRep->numICV;
    svd.svdRep->numIDIV = svdRep->numIDIV;
    svd.svdRep->numIDRV = svdRep->numIDRV;

    // Boost MultiArrays must be resized prior to operator= assignment
    svd.svdRep->allContinuousLabels.resize(boost::extents[num_cv]);
    svd.svdRep->allContinuousLabels = svdRep->allContinuousLabels;
    svd.svdRep->allDiscreteIntLabels.resize(boost::extents[num_div]);
    svd.svdRep->allDiscreteIntLabels = svdRep->allDiscreteIntLabels;
    svd.svdRep->allDiscreteRealLabels.resize(boost::extents[num_drv]);
    svd.svdRep->allDiscreteRealLabels = svdRep->allDiscreteRealLabels;
    svd.svdRep->allContinuousTypes.resize(boost::extents[num_cv]);
    svd.svdRep->allContinuousTypes = svdRep->allContinuousTypes;
    svd.svdRep->allDiscreteIntTypes.resize(boost::extents[num_div]);
    svd.svdRep->allDiscreteIntTypes = svdRep->allDiscreteIntTypes;
    svd.svdRep->allDiscreteRealTypes.resize(boost::extents[num_drv]);
    svd.svdRep->allDiscreteRealTypes = svdRep->allDiscreteRealTypes;
    svd.svdRep->allContinuousIds.resize(boost::extents[num_cv]);
    svd.svdRep->allContinuousIds = svdRep->allContinuousIds;

    svd.svdRep->relaxedDiscreteIds = svdRep->relaxedDiscreteIds;
  }

  return svd;
}

} // namespace Dakota
