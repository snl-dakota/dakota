/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedVariablesData
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "SharedVariablesData.H"
#include "ProblemDescDB.H"
#include "data_util.h"

//#define REFCOUNT_DEBUG

static const char rcsId[]="@(#) $Id: SharedVariablesData.C 6886 2010-08-02 19:13:01Z mseldre $";

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
  idVariables(problem_db.get_string("variables.id")), variablesView(view),
  variablesCompsTotals(12, 0), referenceCount(1)
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
  if (count = problem_db.get_sizet("variables.interval_uncertain")) {
    variablesComponents[INTERVAL_UNCERTAIN] = count;
    variablesCompsTotals[6] += count;
  }
  // discrete integer epistemic uncertain
  //variablesCompsTotals[7] = 0;
  // discrete real epistemic uncertain
  //variablesCompsTotals[8] = 0;
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

  bool relax = ( variablesView.first == MERGED_ALL ||
    ( variablesView.first >= MERGED_DISTINCT_DESIGN &&
      variablesView.first <= MERGED_DISTINCT_STATE ) );
  if (relax)
    num_acv += num_adiv + num_adrv;
  else {
    allDiscreteIntLabels.resize(boost::extents[num_adiv]);
    allDiscreteRealLabels.resize(boost::extents[num_adrv]);
  }
  allContinuousLabels.resize(boost::extents[num_acv]);

  int cv_start = 0, div_start = 0, drv_start = 0;
  // design
  copy_data_partial(problem_db.get_dsa("variables.continuous_design.labels"),
    allContinuousLabels, cv_start);
  cv_start += num_cdv;
  if (relax) {
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_design_range.labels"), allContinuousLabels, cv_start);
    cv_start += num_ddrv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_design_set_int.labels"),
      allContinuousLabels, cv_start);
    cv_start += num_ddsiv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_design_set_real.labels"),
      allContinuousLabels, cv_start);
    cv_start += num_ddsrv;
  }
  else {
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_design_range.labels"),
      allDiscreteIntLabels, div_start);
    div_start += num_ddrv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_design_set_int.labels"),
      allDiscreteIntLabels, div_start);
    div_start += num_ddsiv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_design_set_real.labels"),
      allDiscreteRealLabels, drv_start);
    drv_start += num_ddsrv;
  }
  // aleatory uncertain
  copy_data_partial(problem_db.get_dsa(
    "variables.continuous_aleatory_uncertain.labels"),
    allContinuousLabels, cv_start);
  cv_start += num_cauv;
  if (relax) {
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_aleatory_uncertain_int.labels"),
      allContinuousLabels, cv_start);
    cv_start += num_dauiv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_aleatory_uncertain_real.labels"),
      allContinuousLabels, cv_start);
    cv_start += num_daurv;
  }
  else {
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_aleatory_uncertain_int.labels"),
      allDiscreteIntLabels, div_start);
    div_start += num_dauiv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_aleatory_uncertain_real.labels"),
      allDiscreteRealLabels, drv_start);
    drv_start += num_daurv;
  }
  // epistemic uncertain
  copy_data_partial(problem_db.get_dsa(
    "variables.continuous_epistemic_uncertain.labels"),
    allContinuousLabels, cv_start);
  cv_start += num_ceuv;
  //if (relax) {
    //copy_data_partial(problem_db.get_dsa(
    //  "variables.discrete_epistemic_uncertain_int.labels"),
    //  allContinuousLabels, cv_start);
    //cv_start += num_deuiv;
    //copy_data_partial(problem_db.get_dsa(
    //  "variables.discrete_epistemic_uncertain_real.labels"),
    //  allContinuousLabels, cv_start);
    //cv_start += num_deurv;
  //}
  //else {
    //copy_data_partial(problem_db.get_dsa(
    //  "variables.discrete_epistemic_uncertain_int.labels"),
    //  allDiscreteIntLabels, div_start);
    //div_start += num_deuiv;
    //copy_data_partial(problem_db.get_dsa(
    //  "variables.discrete_epistemic_uncertain_real.labels"),
    //  allDiscreteRealLabels, drv_start);
    //drv_start += num_deurv;
  //}
  // state
  copy_data_partial(problem_db.get_dsa("variables.continuous_state.labels"),
    allContinuousLabels, cv_start);
  if (relax) {
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_state_range.labels"), allContinuousLabels, cv_start);
    cv_start += num_dsrv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_state_set_int.labels"),
      allContinuousLabels, cv_start);
    cv_start += num_dssiv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_state_set_real.labels"),
      allContinuousLabels, cv_start);
  }
  else {
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_state_range.labels"),
      allDiscreteIntLabels, div_start);
    div_start += num_dsrv;
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_state_set_int.labels"),
      allDiscreteIntLabels, div_start);
    copy_data_partial(problem_db.get_dsa(
      "variables.discrete_state_set_real.labels"),
      allDiscreteRealLabels, drv_start);
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
  Cout << "SharedVariablesData::SharedVariablesData(ProblemDescDB) called to "
       << "build base class data for letter object." << std::endl;
#endif
}


SharedVariablesDataRep::
SharedVariablesDataRep(const std::pair<short,short>& view,
		       const SizetArray& vars_comps_totals):
  variablesView(view), variablesCompsTotals(vars_comps_totals),
  referenceCount(1)
{
  // totals are sufficient to size variables
  bool relax = ( variablesView.first == MERGED_ALL ||
    ( variablesView.first >= MERGED_DISTINCT_DESIGN &&
      variablesView.first <= MERGED_DISTINCT_STATE ) );
  size_all_continuous_labels(relax);
  if (!relax) {
    size_all_discrete_int_labels();
    size_all_discrete_real_labels();
  }

  // totals are sufficient for forming ids
  initialize_all_continuous_ids(relax);

  // totals are insufficient for forming types
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
    num_iuv  = vc_lookup(INTERVAL_UNCERTAIN),
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
  for (i=0; i<num_iuv; ++i, ++acv_cntr)
    allContinuousTypes[acv_cntr] = INTERVAL_UNCERTAIN;

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

  // initialize mergedDiscreteIds
  if (relax) {
    size_t i, offset = 1, cntr = 0,
      num_mdv  = num_cdv  + num_ddv,  num_mauv = num_cauv + num_dauv,
      num_meuv = num_ceuv + num_deuv, num_muv  = num_mauv + num_meuv;
    switch (variablesView.first) {
    case MERGED_ALL:
      mergedDiscreteIds.resize(num_ddv + num_dauv + num_deuv + num_dsv);
      offset += num_cdv;
      for (i=0; i<num_ddv; ++i, ++cntr)
      mergedDiscreteIds[cntr] = i + offset;
      offset += num_ddv + num_cauv;
      for (i=0; i<num_dauv; ++i, ++cntr)
	mergedDiscreteIds[cntr] = i + offset;
      offset += num_dauv + num_ceuv;
      for (i=0; i<num_deuv; ++i, ++cntr)
	mergedDiscreteIds[cntr] = i + offset;
      offset += num_deuv + num_csv;
      for (i=0; i<num_dsv; ++i, ++cntr)
	mergedDiscreteIds[cntr] = i + offset;
      break;
    case MERGED_DISTINCT_DESIGN:
      mergedDiscreteIds.resize(num_ddv);
      offset += num_cdv;
      for (i=0; i<num_ddv; ++i)
	mergedDiscreteIds[i] = i + offset;
      break;
    case MERGED_DISTINCT_ALEATORY_UNCERTAIN:
      mergedDiscreteIds.resize(num_dauv);
      offset += num_mdv + num_cauv;
      for (i=0; i<num_dauv; ++i, ++cntr)
	mergedDiscreteIds[i] = i + offset;
      break;
    case MERGED_DISTINCT_EPISTEMIC_UNCERTAIN:
      mergedDiscreteIds.resize(num_deuv);
      offset += num_mdv + num_mauv + num_ceuv;
      for (i=0; i<num_deuv; ++i, ++cntr)
	mergedDiscreteIds[i] = i + offset;
      break;
    case MERGED_DISTINCT_UNCERTAIN:
      mergedDiscreteIds.resize(num_dauv + num_deuv);
      offset += num_mdv + num_cauv;
      for (i=0; i<num_dauv; ++i, ++cntr)
	mergedDiscreteIds[cntr] = i + offset;
      offset += num_dauv + num_ceuv;
      for (i=0; i<num_deuv; ++i, ++cntr)
	mergedDiscreteIds[cntr] = i + offset;
      break;
    case MERGED_DISTINCT_STATE:
      mergedDiscreteIds.resize(num_dsv);
      offset += num_mdv + num_muv + num_csv;
      for (i=0; i<num_dsv; ++i)
	mergedDiscreteIds[i] = i + offset;
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
    num_dssrv = vc_lookup(DISCRETE_STATE_SET_REAL);
  // DESIGN
  for (i=0; i<num_ddsrv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = DISCRETE_DESIGN_SET_REAL;
  // ALEATORY UNCERTAIN
  for (i=0; i<num_hpuv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = HISTOGRAM_POINT_UNCERTAIN;
  // STATE
  for (i=0; i<num_dssrv; ++i, ++adrv_cntr)
    allDiscreteRealTypes[adrv_cntr] = DISCRETE_STATE_SET_REAL;
}

} // namespace Dakota
