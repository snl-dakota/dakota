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
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // Initialize active views
  size_t cv_start, num_cv, div_start, num_div, dsv_start, num_dsv,
    drv_start, num_drv;
  switch (sharedVarsData.view().first) {
  case EMPTY:
    Cerr << "Error: active view cannot be EMPTY in RelaxedVariables."
	 << std::endl; abort_handler(-1);             break;
  case RELAXED_ALL:
    // head to tail
    cv_start  = 0; num_cv  = allContinuousVars.length();
    div_start = 0; num_div = allDiscreteIntVars.length();
    dsv_start = 0; num_dsv = allDiscreteStringVars.length();
    drv_start = 0; num_drv = allDiscreteRealVars.length();
    break;
  case RELAXED_DESIGN:
    // start at head
    cv_start  = 0; num_cv  = num_cdv  + num_relax_ddiv + num_relax_ddrv;
    div_start = 0; num_div = num_ddiv - num_relax_ddiv;
    dsv_start = 0; num_dsv = num_ddsv;
    drv_start = 0; num_drv = num_ddrv - num_relax_ddrv;
    break;
  case RELAXED_ALEATORY_UNCERTAIN:
    // skip over the relaxed design variables
    cv_start  = ; num_cv  = num_cauv  + num_relax_dauiv + num_relax_daurv;
    div_start = ; num_div = num_dauiv - num_relax_dauiv;
    dsv_start = ; num_dsv = num_dausv;
    drv_start = ; num_drv = num_daurv - num_relax_daurv;
    //cv_start = num_mdv; num_cv = num_mauv;
    break;
  case RELAXED_EPISTEMIC_UNCERTAIN:
    // skip over the relaxed design and aleatory variables
    cv_start  = ; num_cv  = num_ceuv  + num_relax_deuiv + num_relax_deurv;
    div_start = ; num_div = num_deuiv - num_relax_deuiv;
    dsv_start = ; num_dsv = num_deusv;
    drv_start = ; num_drv = num_deurv - num_relax_deurv;
    //cv_start = num_mdv+num_mauv;  num_cv = num_meuv;
    break;
  case RELAXED_UNCERTAIN:
    // skip over the relaxed design variables
    cv_start  = ;
    num_cv = num_cauv + num_relax_dauiv + num_relax_daurv
           + num_ceuv + num_relax_deuiv + num_relax_deurv;
    div_start = ;
    num_div = num_dauiv - num_relax_dauiv + num_deuiv - num_relax_deuiv;
    dsv_start = ; num_dsv = num_dausv + num_deusv;
    drv_start = ;
    num_drv = num_daurv - num_relax_daurv + num_deurv - num_relax_deurv;
    //cv_start = num_mdv; num_cv = num_muv;
    break;
  case RELAXED_STATE:
    // skip over the relaxed design and uncertain variables
    cv_start  = ; num_cv  = num_csv  + num_relax_dsiv + num_relax_dsrv;
    div_start = ; num_div = num_dsiv - num_relax_dsiv;
    dsv_start = ; num_dsv = num_dssv;
    drv_start = ; num_drv = num_dsrv - num_relax_dsrv;
    //cv_start = num_mdv + num_muv; num_cv = num_msv;
    break;
  }
  sharedVarsData.cv_start(cv_start);   sharedVarsData.cv(num_cv);
  sharedVarsData.div_start(div_start); sharedVarsData.div(num_div);
  sharedVarsData.dsv_start(dsv_start); sharedVarsData.drv(num_dsv);
  sharedVarsData.drv_start(drv_start); sharedVarsData.drv(num_drv);
  sharedVarsData.initialize_active_components();
  if (num_cv)
    continuousVars
      = RealVector(Teuchos::View, &allContinuousVars[cv_start], num_cv);
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
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // Initialize inactive views
  size_t icv_start, num_icv, idiv_start, num_idiv, idsv_start, num_idsv,
    idrv_start, num_idrv;
  switch (sharedVarsData.view().second) {
  case EMPTY:
    icv_start = num_icv = 0;                            break;
  case RELAXED_ALL:
    Cerr << "Error: inactive view cannot be RELAXED_ALL in RelaxedVariables."
	 << std::endl;
    abort_handler(-1);                                break;
  case RELAXED_DESIGN:
    // start at the beginning
    icv_start = 0;                  num_icv = num_mdv;  break;
  case RELAXED_ALEATORY_UNCERTAIN:
    // skip over the relaxed design variables
    icv_start = num_mdv;            num_icv = num_mauv; break;
  case RELAXED_EPISTEMIC_UNCERTAIN:
    // skip over the relaxed design and aleatory variables
    icv_start = num_mdv + num_mauv; num_icv = num_meuv; break;
  case RELAXED_UNCERTAIN:
    // skip over the relaxed design variables
    icv_start = num_mdv;            num_icv = num_muv;  break;
  case RELAXED_STATE:
    // skip over the relaxed design and uncertain variables
    icv_start = num_mdv + num_muv;  num_icv = num_msv;  break;
  }
  sharedVarsData.icv_start(icv_start);   sharedVarsData.icv(num_icv);
  sharedVarsData.idiv_start(idiv_start); sharedVarsData.idiv(num_idiv);
  sharedVarsData.idsv_start(idsv_start); sharedVarsData.idrv(num_idsv);
  sharedVarsData.idrv_start(idrv_start); sharedVarsData.idrv(num_idrv);
  sharedVarsData.initialize_inactive_components();
  if (num_icv)
    inactiveContinuousVars
      = RealVector(Teuchos::View, &allContinuousVars[icv_start], num_icv);
}


// Reordering is required in all read/write cases that will be visible to the
// user since all derived vars classes should use the same CDV/DDV/UV/CSV/DSV
// ordering for clarity.  Neutral file I/O, binary streams, and packed buffers
// do not need to reorder (so long as read/write are consistent) since this data
// is not intended for public consumption.
void RelaxedVariables::read(std::istream& s)
{
  read_data(s, allContinuousVars, all_continuous_variable_labels());
  // TO DO
}


void RelaxedVariables::write(std::ostream& s) const
{
  write_data(s, allContinuousVars, all_continuous_variable_labels());
  // TO DO
}


void RelaxedVariables::write_aprepro(std::ostream& s) const
{
  write_data_aprepro(s, allContinuousVars, all_continuous_variable_labels());
  // TO DO
}


/** Presumes variables object is appropriately sized to receive data */
void RelaxedVariables::read_tabular(std::istream& s)
{
  // TO DO: should we be fully consistent with tabular formats?

  // This simple choice will differ from MixedVars ordering, but tabular 
  // ordering was already different...  
  read_data_tabular(s, allContinuousVars);
  read_data_tabular(s, allDiscreteIntVars);
  read_data_tabular(s, allDiscreteStringVars);
  read_data_tabular(s, allDiscreteRealVars);

  /*
  // Previous implementation standardized on allCV,allDIV,allDSV,allDRV
  // ordering for consistency with MixedVars, but this was inconsistent
  // with non-tabular ordering.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  read_data_partial_tabular(s, 0, num_cdv, allContinuousVars);
  read_data_partial_tabular(s, num_dv, num_cauv, allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_auv, num_ceuv, allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_uv, num_csv, allContinuousVars);
  read_data_partial_tabular(s, num_cdv, num_ddiv, allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_cauv, num_dauiv, allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_auv+num_ceuv, num_deuiv,
			    allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_uv+num_csv, num_dsiv,
			    allContinuousVars);
  read_data_partial_tabular(s, num_cdv+num_ddiv, num_ddrv, allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_cauv+num_dauiv, num_daurv,
			    allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_auv+num_ceuv+num_deuiv, num_deurv,
			    allContinuousVars);
  read_data_partial_tabular(s, num_dv+num_uv+num_csv+num_dsiv, num_dsrv,
			    allContinuousVars);
  */
}


void RelaxedVariables::write_tabular(std::ostream& s) const
{
  // TO DO: should we be fully consistent with tabular formats?

  // This simple choice will differ from MixedVars ordering, but tabular 
  // ordering was already different from other orderings.  For neutral files,
  // variables type key could define a unique convenient ordering.
  write_data_tabular(s, allContinuousVars);
  write_data_tabular(s, allDiscreteIntVars);
  write_data_tabular(s, allDiscreteStringVars);
  write_data_tabular(s, allDiscreteRealVars);

  /*
  // Previous implementation standardized on allCV,allDIV,allDSV,allDRV
  // ordering for consistency with MixedVars, but this was inconsistent
  // with non-tabular ordering.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  write_data_partial_tabular(s, 0, num_cdv, allContinuousVars);
  write_data_partial_tabular(s, num_dv, num_cauv, allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_auv, num_ceuv, allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_uv, num_csv, allContinuousVars);
  write_data_partial_tabular(s, num_cdv, num_ddiv, allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_cauv, num_dauiv, allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_auv+num_ceuv, num_deuiv,
			     allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_uv+num_csv, num_dsiv,
			     allContinuousVars);
  write_data_partial_tabular(s, num_cdv+num_ddiv, num_ddrv, allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_cauv+num_dauiv, num_daurv,
			     allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_auv+num_ceuv+num_deuiv, num_deurv,
			     allContinuousVars);
  write_data_partial_tabular(s, num_dv+num_uv+num_csv+num_dsiv, num_dsrv,
			     allContinuousVars);
  */
}

} // namespace Dakota
