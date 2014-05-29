/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        RelaxedVariables
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "RelaxedVariables.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"

static const char rcsId[]="@(#) $Id";


namespace Dakota {

/** In this class, a relaxed data approach is used in which continuous
    and discrete arrays are combined into a single continuous array
    (integrality is relaxed; the converse of truncating reals is not
    currently supported but could be in the future if needed).
    Iterators/strategies which use this class include:
    BranchBndOptimizer.  Extract fundamental variable types and labels
    and merge continuous and discrete domains to create aggregate
    arrays and views.  */
RelaxedVariables::
RelaxedVariables(const ProblemDescDB& problem_db,
		const std::pair<short,short>& view):
  Variables(BaseConstructor(), problem_db, view)
{
  size_t num_acv, num_adiv, num_adsv, num_adrv;
  sharedVarsData.all_counts(num_acv, num_adiv, num_adsv, num_adrv);

  allContinuousVars.sizeUninitialized(num_acv);
  allDiscreteIntVars.sizeUninitialized(num_adiv);
  allDiscreteStringVars.sizeUninitialized(num_adsv);
  allDiscreteRealVars.sizeUninitialized(num_adrv);

  const RealVector& cdv  = problem_db.get_rv(
    "variables.continuous_design.initial_point");
  const RealVector& cauv = problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.initial_point");
  const RealVector& ceuv = problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.initial_point");
  const RealVector& csv  = problem_db.get_rv(
    "variables.continuous_state.initial_state");

  const IntVector& ddrv  = problem_db.get_iv(
    "variables.discrete_design_range.initial_point");
  const IntVector& ddsiv = problem_db.get_iv(
    "variables.discrete_design_set_int.initial_point");
  const IntVector& dauiv = problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.initial_point");
  const IntVector& deuiv = problem_db.get_iv(
   "variables.discrete_epistemic_uncertain_int.initial_point");
  const IntVector& dsrv  = problem_db.get_iv(
    "variables.discrete_state_range.initial_state");
  const IntVector& dssiv = problem_db.get_iv(
    "variables.discrete_state_set_int.initial_state");

  const StringArray& ddssv = problem_db.get_sa(
    "variables.discrete_design_set_string.initial_point");
  const StringArray& dausv = problem_db.get_sa(
    "variables.discrete_aleatory_uncertain_string.initial_point");
  const StringArray& deusv = problem_db.get_sa(
   "variables.discrete_epistemic_uncertain_string.initial_point");
  const StringArray& dsssv = problem_db.get_sa(
    "variables.discrete_state_set_string.initial_state");

  const RealVector& ddsrv = problem_db.get_rv(
    "variables.discrete_design_set_real.initial_point");
  const RealVector& daurv = problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.initial_point");
  const RealVector& deurv = problem_db.get_rv(
   "variables.discrete_epistemic_uncertain_real.initial_point");
  const RealVector& dssrv = problem_db.get_rv(
    "variables.discrete_state_set_real.initial_state");

  size_t i, ardi_cntr = 0, ardr_cntr = 0,
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0,
    num_ddrv  = ddrv.size(), num_ddsiv = ddsiv.size(), num_ddsrv = ddsrv.size(),
    num_dauiv = dauiv.size(), num_daurv = daurv.size(),
    num_deuiv = deuiv.size(), num_deurv = deurv.size(),
    num_dsrv  = dsrv.size(), num_dssiv = dssiv.size(), num_dssrv = dssrv.size();
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();
  copy_data_partial(cdv, allContinuousVars, acv_offset);
  acv_offset += cdv.size();
  for (i=0; i<num_ddrv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) allContinuousVars[acv_offset++]    = ddrv[i];
    else                         allDiscreteIntVars[adiv_offset++]  = ddrv[i];
  for (i=0; i<num_ddsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) allContinuousVars[acv_offset++]    = ddsiv[i];
    else                         allDiscreteIntVars[adiv_offset++]  = ddsiv[i];
  copy_data_partial(ddssv, allDiscreteStringVars, adsv_offset);
  adsv_offset += ddssv.size();
  for (i=0; i<num_ddsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) allContinuousVars[acv_offset++]    = ddsrv[i];
    else                         allDiscreteRealVars[adrv_offset++] = ddsrv[i];

  copy_data_partial(cauv, allContinuousVars, acv_offset);
  acv_offset += cauv.size();
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) allContinuousVars[acv_offset++]    = dauiv[i];
    else                         allDiscreteIntVars[adiv_offset++]  = dauiv[i];
  copy_data_partial(dausv, allDiscreteStringVars, adsv_offset);
  adsv_offset += dausv.size();
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) allContinuousVars[acv_offset++]    = daurv[i];
    else                         allDiscreteRealVars[adrv_offset++] = daurv[i];

  copy_data_partial(ceuv, allContinuousVars, acv_offset);
  acv_offset += ceuv.size();
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) allContinuousVars[acv_offset++]    = deuiv[i];
    else                         allDiscreteIntVars[adiv_offset++]  = deuiv[i];
  copy_data_partial(deusv, allDiscreteStringVars, adsv_offset);
  adsv_offset += deusv.size();
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) allContinuousVars[acv_offset++]    = deurv[i];
    else                         allDiscreteRealVars[adrv_offset++] = deurv[i];

  copy_data_partial(csv, allContinuousVars, acv_offset);
  acv_offset += csv.size();
  for (i=0; i<num_dsrv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) allContinuousVars[acv_offset++]    = dsrv[i];
    else                         allDiscreteIntVars[adiv_offset++]  = dsrv[i];
  for (i=0; i<num_dssiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) allContinuousVars[acv_offset++]    = dssiv[i];
    else                         allDiscreteIntVars[adiv_offset++]  = dssiv[i];
  copy_data_partial(dsssv, allDiscreteStringVars, adsv_offset);
  adsv_offset += dsssv.size();
  for (i=0; i<num_dssrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) allContinuousVars[acv_offset++]    = dssrv[i];
    else                         allDiscreteRealVars[adrv_offset++] = dssrv[i];

  // Construct active/inactive views of all arrays
  build_views();

#ifdef REFCOUNT_DEBUG
  Cout << "Letter instantiated: variablesView active = "
       << sharedVarsData.view().first << " inactive = "
       << sharedVarsData.view().second << std::endl;
#endif
}


void RelaxedVariables::reshape(const SizetArray& vc_totals)
{
  size_t i, num_acv = 0, num_totals = vc_totals.size();
  for (i=0; i<num_totals; ++i)
    num_acv += vc_totals[i];

  allContinuousVars.resize(num_acv);

  build_views(); // construct active/inactive views of all arrays
}


void RelaxedVariables::build_active_views()
{
  // Initialize active view vectors and counts.  Don't bleed over any logic
  // about supported view combinations; rather, keep this class general and
  // encapsulated.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, offset, cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv, start_relax_di = 0, start_relax_dr = 0,
    count_relax_di = 0, count_relax_dr = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // Initialize active views
  switch (sharedVarsData.view().first) {
  case EMPTY:
    Cerr << "Error: active view cannot be EMPTY in RelaxedVariables."
	 << std::endl; abort_handler(-1);             break;
  case RELAXED_ALL:
    // from head to tail
    cv_start  = 0; num_cv  = allContinuousVars.length();
    div_start = 0; num_div = allDiscreteIntVars.length();
    dsv_start = 0; num_dsv = allDiscreteStringVars.length();
    drv_start = 0; num_drv = allDiscreteRealVars.length();
    break;
  case RELAXED_DESIGN:
    // start at head
    for (i=0; i<num_ddiv; ++i)
      if (all_relax_di[i]) ++count_relax_di;
    for (i=0; i<num_ddrv; ++i)
      if (all_relax_dr[i]) ++count_relax_dr;
    cv_start  = 0; num_cv  = num_cdv  + count_relax_di + count_relax_dr;
    div_start = 0; num_div = num_ddiv - count_relax_di;
    dsv_start = 0; num_dsv = num_ddsv;
    drv_start = 0; num_drv = num_ddrv - count_relax_dr;
    break;
  case RELAXED_ALEATORY_UNCERTAIN:
    // skip over the design variables
    offset = num_ddiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_dauiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_daurv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    cv_start  = num_cdv   + start_relax_di + start_relax_dr;
    div_start = num_ddiv  - start_relax_di;
    dsv_start = num_ddsv;
    drv_start = num_ddrv  - start_relax_dr;
    num_cv    = num_cauv  + count_relax_di + count_relax_dr;
    num_div   = num_dauiv - count_relax_di;
    num_dsv   = num_dausv;
    num_drv   = num_daurv - count_relax_dr;
    break;
  case RELAXED_EPISTEMIC_UNCERTAIN:
    // skip over the design and aleatory variables
    offset = num_ddiv + num_dauiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_deuiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv + num_daurv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_deurv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    cv_start  = num_cdv   + num_cauv  + start_relax_di + start_relax_dr;
    div_start = num_ddiv  + num_dauiv - start_relax_di;
    dsv_start = num_ddsv  + num_dausv;
    drv_start = num_ddrv  + num_daurv - start_relax_dr;
    num_cv    = num_ceuv  + count_relax_di + count_relax_dr;
    num_div   = num_deuiv - count_relax_di;
    num_dsv   = num_deusv;
    num_drv   = num_deurv - count_relax_dr;
    break;
  case RELAXED_UNCERTAIN:
    // skip over the design variables
    offset = num_ddiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_dauiv+num_deuiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_daurv+num_deurv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    cv_start  = num_cdv   + start_relax_di + start_relax_dr;
    div_start = num_ddiv  - start_relax_di;
    dsv_start = num_ddsv;
    drv_start = num_ddrv  - start_relax_dr;
    num_cv    = num_cauv  + num_ceuv  + count_relax_di + count_relax_dr;
    num_div   = num_dauiv + num_deuiv - count_relax_di;
    num_dsv   = num_dausv + num_deusv;
    num_drv   = num_daurv + num_deurv - count_relax_dr;
    break;
  case RELAXED_STATE:
    // skip over the design and uncertain variables
    offset = num_ddiv + num_dauiv + num_deuiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_dsiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv + num_daurv + num_deurv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_dsrv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    cv_start  = num_cdv + num_cauv + num_ceuv + start_relax_di + start_relax_dr;
    div_start = num_ddiv + num_dauiv + num_deuiv - start_relax_di;
    dsv_start = num_ddsv + num_dausv + num_deusv;
    drv_start = num_ddrv + num_daurv + num_deurv - start_relax_dr;
    num_cv    = num_csv  + count_relax_di + count_relax_dr;
    num_div   = num_dsiv - count_relax_di;
    num_dsv   = num_dssv;
    num_drv   = num_dsrv - count_relax_dr;
    break;
  }
  sharedVarsData.cv_start(cv_start);   sharedVarsData.cv(num_cv);
  sharedVarsData.div_start(div_start); sharedVarsData.div(num_div);
  sharedVarsData.dsv_start(dsv_start); sharedVarsData.drv(num_dsv);
  sharedVarsData.drv_start(drv_start); sharedVarsData.drv(num_drv);
  sharedVarsData.initialize_active_components();
  if (num_cv)
    continuousVars
      = RealVector(Teuchos::View, &allContinuousVars[cv_start],    num_cv);
  if (num_div)
    discreteIntVars
      = IntVector(Teuchos::View,  &allDiscreteIntVars[div_start],  num_div);
  //if (num_dsv)
  //  discreteStringVars = allDiscreteStringVars[boost::indices[
  //    idx_range(dsv_start, dsv_start+num_dsv)]];
  if (num_drv)
    discreteRealVars
      = RealVector(Teuchos::View, &allDiscreteRealVars[drv_start], num_drv);
}


void RelaxedVariables::build_inactive_views()
{
  // Initialize inactive view vectors and counts.  Don't bleed over any logic
  // about supported view combinations; rather, keep this class general and
  // encapsulated.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV], 
    i, offset, icv_start, num_icv, idiv_start, num_idiv, idsv_start, num_idsv,
    idrv_start, num_idrv, start_relax_di = 0, start_relax_dr = 0,
    count_relax_di = 0, count_relax_dr = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // Initialize inactive views
  switch (sharedVarsData.view().second) {
  case EMPTY:
    icv_start = num_icv = idiv_start = num_idiv = idsv_start = num_idsv =
      idrv_start = num_idrv = 0;
    break;
  case RELAXED_ALL:
    Cerr << "Error: inactive view cannot be RELAXED_ALL in RelaxedVariables."
	 << std::endl;
    abort_handler(-1);
    break;
  case RELAXED_DESIGN:
    // start at head
    for (i=0; i<num_ddiv; ++i)
      if (all_relax_di[i]) ++count_relax_di;
    for (i=0; i<num_ddrv; ++i)
      if (all_relax_dr[i]) ++count_relax_dr;
    icv_start  = 0; num_icv  = num_cdv  + count_relax_di + count_relax_dr;
    idiv_start = 0; num_idiv = num_ddiv - count_relax_di;
    idsv_start = 0; num_idsv = num_ddsv;
    idrv_start = 0; num_idrv = num_ddrv - count_relax_dr;
    break;
  case RELAXED_ALEATORY_UNCERTAIN:
    // skip over the design variables
    offset = num_ddiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_dauiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_daurv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    icv_start  = num_cdv   + start_relax_di + start_relax_dr;
    idiv_start = num_ddiv  - start_relax_di;
    idsv_start = num_ddsv;
    idrv_start = num_ddrv  - start_relax_dr;
    num_icv    = num_cauv  + count_relax_di + count_relax_dr;
    num_idiv   = num_dauiv - count_relax_di;
    num_idsv   = num_dausv;
    num_idrv   = num_daurv - count_relax_dr;
    break;
  case RELAXED_EPISTEMIC_UNCERTAIN:
    // skip over the design and aleatory variables
    offset = num_ddiv + num_dauiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_deuiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv + num_daurv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_deurv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    icv_start  = num_cdv   + num_cauv  + start_relax_di + start_relax_dr;
    idiv_start = num_ddiv  + num_dauiv - start_relax_di;
    idsv_start = num_ddsv  + num_dausv;
    idrv_start = num_ddrv  + num_daurv - start_relax_dr;
    num_icv    = num_ceuv  + count_relax_di + count_relax_dr;
    num_idiv   = num_deuiv - count_relax_di;
    num_idsv   = num_deusv;
    num_idrv   = num_deurv - count_relax_dr;
    break;
  case RELAXED_UNCERTAIN:
    // skip over the design variables
    offset = num_ddiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_dauiv+num_deuiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_daurv+num_deurv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    icv_start  = num_cdv   + start_relax_di + start_relax_dr;
    idiv_start = num_ddiv  - start_relax_di;
    idsv_start = num_ddsv;
    idrv_start = num_ddrv  - start_relax_dr;
    num_icv    = num_cauv  + num_ceuv  + count_relax_di + count_relax_dr;
    num_idiv   = num_dauiv + num_deuiv - count_relax_di;
    num_idsv   = num_dausv + num_deusv;
    num_idrv   = num_daurv + num_deurv - count_relax_dr;
    break;
  case RELAXED_STATE:
    // skip over the design and uncertain variables
    offset = num_ddiv + num_dauiv + num_deuiv;
    for (i=0; i<offset; ++i)
      if (all_relax_di[i])        ++start_relax_di;
    for (i=0; i<num_dsiv; ++i)
      if (all_relax_di[offset+i]) ++count_relax_di;
    offset = num_ddrv + num_daurv + num_deurv;
    for (i=0; i<offset; ++i)
      if (all_relax_dr[i])        ++start_relax_dr;
    for (i=0; i<num_dsrv; ++i)
      if (all_relax_dr[offset+i]) ++count_relax_dr;
    cv_start  = num_cdv + num_cauv + num_ceuv + start_relax_di + start_relax_dr;
    div_start = num_ddiv + num_dauiv + num_deuiv - start_relax_di;
    dsv_start = num_ddsv + num_dausv + num_deusv;
    drv_start = num_ddrv + num_daurv + num_deurv - start_relax_dr;
    num_cv    = num_csv  + count_relax_di + count_relax_dr;
    num_div   = num_dsiv - count_relax_di;
    num_dsv   = num_dssv;
    num_drv   = num_dsrv - count_relax_dr;
    break;
  }
  sharedVarsData.icv_start(icv_start);   sharedVarsData.icv(num_icv);
  sharedVarsData.idiv_start(idiv_start); sharedVarsData.idiv(num_idiv);
  sharedVarsData.idsv_start(idsv_start); sharedVarsData.idrv(num_idsv);
  sharedVarsData.idrv_start(idrv_start); sharedVarsData.idrv(num_idrv);
  sharedVarsData.initialize_inactive_components();
  if (num_icv)
    inactiveContinuousVars
      = RealVector(Teuchos::View, &allContinuousVars[icv_start],    num_icv);
  if (num_idiv)
    inactiveDiscreteIntVars
      = IntVector(Teuchos::View,  &allDiscreteIntVars[idiv_start],  num_idiv);
  //if (num_idsv)
  //  inactiveDiscreteStringVars = allDiscreteStringVars[boost::indices[
  //    idx_range(idsv_start, idsv_start+num_idsv)]];
  if (num_idrv)
    inactiveDiscreteRealVars
      = RealVector(Teuchos::View, &allDiscreteRealVars[idrv_start], num_idrv);
}


// Reordering is required in all read/write cases that will be visible to the
// user since all derived vars classes should use the same ordering for clarity.
// Neutral file I/O, binary streams, and packed buffers do not need to reorder
// (so long as read/write are consistent) since this data is not intended for
// public consumption.
void RelaxedVariables::read(std::istream& s)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  read_data_partial(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  read_data_partial(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_dausv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  read_data_partial(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_deusv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // state
  read_data_partial(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_dssv, allDiscreteStringVars,
		    adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);
}


void RelaxedVariables::write(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  write_data_partial(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  write_data_partial(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_dausv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  write_data_partial(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_deusv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // state
  write_data_partial(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_dssv, allDiscreteStringVars,
		    adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);
}


void RelaxedVariables::write_aprepro(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  write_data_partial_aprepro(s, acv_offset, num_cdv, allContinuousVars,
			     acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_ddsv, allDiscreteStringVars,
			     adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);

  // aleatory uncertain
  write_data_partial_aprepro(s, acv_offset, num_cauv, allContinuousVars,
			     acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_dausv, allDiscreteStringVars,
			     adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);

  // epistemic uncertain
  write_data_partial_aprepro(s, acv_offset, num_ceuv, allContinuousVars,
			     acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_deusv, allDiscreteStringVars,
			     adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);

  // state
  write_data_partial_aprepro(s, acv_offset, num_csv, allContinuousVars,
			     acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_dssv, allDiscreteStringVars,
			     adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);
}


/** Presumes variables object is appropriately sized to receive data */
void RelaxedVariables::read_tabular(std::istream& s)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // design
  read_data_partial_tabular(s, acv_offset, num_cdv, allContinuousVars);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_ddsv, allDiscreteStringVars);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // aleatory uncertain
  read_data_partial_tabular(s, acv_offset, num_cauv, allContinuousVars);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_dausv, allDiscreteStringVars);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // epistemic uncertain
  read_data_partial_tabular(s, acv_offset, num_ceuv, allContinuousVars);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_deusv, allDiscreteStringVars);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // state
  read_data_partial_tabular(s, acv_offset, num_csv, allContinuousVars);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_dssv, allDiscreteStringVars);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);
}


void RelaxedVariables::write_tabular(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // design
  write_data_partial_tabular(s, acv_offset, num_cdv, allContinuousVars);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_ddsv, allDiscreteStringVars);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // aleatory uncertain
  write_data_partial_tabular(s, acv_offset, num_cauv, allContinuousVars);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_dausv, allDiscreteStringVars);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // epistemic uncertain
  write_data_partial_tabular(s, acv_offset, num_ceuv, allContinuousVars);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_deusv, allDiscreteStringVars);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // state
  write_data_partial_tabular(s, acv_offset, num_csv, allContinuousVars);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_dssv, allDiscreteStringVars);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);
}

} // namespace Dakota
