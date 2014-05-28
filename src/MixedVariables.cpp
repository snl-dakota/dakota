/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MixedVariables
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "MixedVariables.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"

static const char rcsId[]="@(#) $Id";


namespace Dakota {

/** In this class, the distinct approach is used (design, uncertain, and
    state variable types and continuous and discrete domain types are 
    distinct).  Most iterators/strategies use this approach. */
MixedVariables::
MixedVariables(const ProblemDescDB& problem_db,
	       const std::pair<short,short>& view):
  Variables(BaseConstructor(), problem_db, view)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  allContinuousVars.sizeUninitialized(vc_totals[TOTAL_CDV] +
    vc_totals[TOTAL_CAUV] + vc_totals[TOTAL_CEUV] + vc_totals[TOTAL_CSV]);
  allDiscreteIntVars.sizeUninitialized(vc_totals[TOTAL_DDIV] +
    vc_totals[TOTAL_DAUIV] + vc_totals[TOTAL_DEUIV] + vc_totals[TOTAL_DSIV]);
  allDiscreteStringVars.sizeUninitialized(vc_totals[TOTAL_DDSV] +
    vc_totals[TOTAL_DAUSV] + vc_totals[TOTAL_DEUSV] + vc_totals[TOTAL_DSSV]);
  allDiscreteRealVars.sizeUninitialized(vc_totals[TOTAL_DDRV] +
    vc_totals[TOTAL_DAURV] + vc_totals[TOTAL_DEURV] + vc_totals[TOTAL_DSRV]);

  int start = 0;
  const RealVector& cdv  = problem_db.get_rv(
    "variables.continuous_design.initial_point");
  const RealVector& cauv = problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.initial_point");
  const RealVector& ceuv = problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.initial_point");
  const RealVector& csv  = problem_db.get_rv(
    "variables.continuous_state.initial_state");
  copy_data_partial(cdv,  allContinuousVars, start); start += cdv.size();
  copy_data_partial(cauv, allContinuousVars, start); start += cauv.size();
  copy_data_partial(ceuv, allContinuousVars, start); start += ceuv.size();
  copy_data_partial(csv,  allContinuousVars, start);

  start = 0;
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
  copy_data_partial(ddrv,  allDiscreteIntVars, start); start += ddrv.size();
  copy_data_partial(ddsiv, allDiscreteIntVars, start); start += ddsiv.size();
  copy_data_partial(dauiv, allDiscreteIntVars, start); start += dauiv.size();
  copy_data_partial(deuiv, allDiscreteIntVars, start); start += deuiv.size();
  copy_data_partial(dsrv,  allDiscreteIntVars, start); start += dsrv.size();
  copy_data_partial(dssiv, allDiscreteIntVars, start);

  start = 0;
  const StringArray& ddssv = problem_db.get_sa(
    "variables.discrete_design_set_string.initial_point");
  const StringArray& dausv = problem_db.get_sa(
    "variables.discrete_aleatory_uncertain_string.initial_point");
  const StringArray& deusv = problem_db.get_sa(
   "variables.discrete_epistemic_uncertain_string.initial_point");
  const StringArray& dsssv = problem_db.get_sa(
    "variables.discrete_state_set_string.initial_state");
  copy_data_partial(ddssv, allDiscreteStringVars, start); start += ddssv.size();
  copy_data_partial(dausv, allDiscreteStringVars, start); start += dausv.size();
  copy_data_partial(deusv, allDiscreteStringVars, start); start += deusv.size();
  copy_data_partial(dsssv, allDiscreteStringVars, start);

  start = 0;
  const RealVector& ddsrv = problem_db.get_rv(
    "variables.discrete_design_set_real.initial_point");
  const RealVector& daurv = problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.initial_point");
  const RealVector& deurv = problem_db.get_rv(
   "variables.discrete_epistemic_uncertain_real.initial_point");
  const RealVector& dssrv = problem_db.get_rv(
    "variables.discrete_state_set_real.initial_state");
  copy_data_partial(ddsrv, allDiscreteRealVars, start); start += ddsrv.size();
  copy_data_partial(daurv, allDiscreteRealVars, start); start += daurv.size();
  copy_data_partial(deurv, allDiscreteRealVars, start); start += deurv.size();
  copy_data_partial(dssrv, allDiscreteRealVars, start);

  // construct active/inactive views of all arrays
  build_views();

#ifdef REFCOUNT_DEBUG
  Cout << "Letter instantiated: variablesView active = "
       << sharedVarsData.view().first << " inactive = " 
       << sharedVarsData.view().second << std::endl;
#endif
}


void MixedVariables::reshape(const SizetArray& vc_totals)
{
  allContinuousVars.resize(vc_totals[TOTAL_CDV]  + vc_totals[TOTAL_CAUV] +
			   vc_totals[TOTAL_CEUV] + vc_totals[TOTAL_CSV]);
  allDiscreteIntVars.resize(allRelaxedDiscreteInt.size());
  allDiscreteStringVars.resize(vc_totals[TOTAL_DDSV]  + vc_totals[TOTAL_DAUSV] +
			       vc_totals[TOTAL_DEUSV] + vc_totals[TOTAL_DSSV]);
  allDiscreteRealVars.resize(allRelaxedDiscreteReal.size());

  build_views(); // construct active/inactive views of all arrays
}


void MixedVariables::build_active_views()
{
  // Initialize continuousVarTypes/discreteVarTypes/continuousVarIds.
  // Don't bleed over any logic about supported view combinations; rather,
  // keep this class general and encapsulated.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];

  // Initialize active views
  size_t cv_start, div_start, dsv_start, drv_start,
    num_cv, num_div, num_dsv, num_drv;
  switch (sharedVarsData.view().first) {
  case EMPTY:
    Cerr << "Error: active view cannot be EMPTY in MixedVariables."<< std::endl;
    abort_handler(-1);                                                break;
  case MIXED_ALL:
    // start at the beginning
    cv_start = div_start = dsv_start = drv_start = 0;
    num_cv  = num_cdv  + num_cauv  + num_ceuv  + num_csv;
    num_div = num_ddiv + num_dauiv + num_deuiv + num_dsiv;
    num_dsv = num_ddsv + num_dausv + num_deusv + num_dssv;
    num_drv = num_ddrv + num_daurv + num_deurv + num_dsrv;            break;
  case MIXED_DESIGN:
    // start at the beginning
    cv_start = div_start = dsv_start = drv_start = 0;
    num_cv  = num_cdv;  num_div = num_ddiv;
    num_dsv = num_ddsv; num_drv = num_ddrv;                           break;
  case MIXED_ALEATORY_UNCERTAIN:
    // skip over the design variables
    cv_start  = num_cdv;  num_cv  = num_cauv;
    div_start = num_ddiv; num_div = num_dauiv;
    dsv_start = num_ddsv; num_dsv = num_dausv;
    drv_start = num_ddrv; num_drv = num_daurv;                        break;
  case MIXED_EPISTEMIC_UNCERTAIN:
    // skip over the design and aleatory uncertain variables
    cv_start  = num_cdv  + num_cauv;  num_cv  = num_ceuv;
    div_start = num_ddiv + num_dauiv; num_div = num_deuiv;
    dsv_start = num_ddsv + num_dausv; num_dsv = num_deusv;
    drv_start = num_ddrv + num_daurv; num_drv = num_deurv;            break;
  case MIXED_UNCERTAIN:
    // skip over the design variables
    cv_start  = num_cdv;  num_cv  = num_cauv  + num_ceuv;
    div_start = num_ddiv; num_div = num_dauiv + num_deuiv;
    dsv_start = num_ddsv; num_dsv = num_dausv + num_deusv;
    drv_start = num_ddrv; num_drv = num_daurv + num_deurv;            break;
  case MIXED_STATE:
    // skip over all the design and uncertain variables
    cv_start  = num_cdv  + num_cauv  + num_ceuv;  num_cv  = num_csv;
    div_start = num_ddiv + num_dauiv + num_deuiv; num_div = num_dsiv;
    dsv_start = num_ddsv + num_dausv + num_deusv; num_dsv = num_dssv;
    drv_start = num_ddrv + num_daurv + num_deurv; num_drv = num_dsrv; break;
  }
  sharedVarsData.cv_start(cv_start);   sharedVarsData.cv(num_cv);
  sharedVarsData.div_start(div_start); sharedVarsData.div(num_div);
  sharedVarsData.dsv_start(dsv_start); sharedVarsData.dsv(num_dsv);
  sharedVarsData.drv_start(drv_start); sharedVarsData.drv(num_drv);
  sharedVarsData.initialize_active_components();
  if (num_cv)
    continuousVars
      = RealVector(Teuchos::View, &allContinuousVars[cv_start], num_cv);
  if (num_div)
    discreteIntVars
      = IntVector(Teuchos::View, &allDiscreteIntVars[div_start], num_div);
  if (num_dsv)
    discreteStringVars = allDiscreteStringVars[boost::indices[
      idx_range(dsv_start, dsv_start+num_dsv)]];
  if (num_drv)
    discreteRealVars
      = RealVector(Teuchos::View, &allDiscreteRealVars[drv_start], num_drv);
}


void MixedVariables::build_inactive_views()
{
  // Initialize continuousVarTypes/discreteVarTypes/continuousVarIds.
  // Don't bleed over any logic about supported view combinations; rather,
  // keep this class general and encapsulated.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t  num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];

  // Initialize inactive views
  size_t icv_start, idiv_start, idrv_start, num_icv, num_idiv, num_idrv;
  switch (sharedVarsData.view().second) {
  case EMPTY:
    icv_start = idiv_start = idrv_start = num_icv = num_idiv = num_idrv = 0;
    break;
  case MIXED_ALL:
    Cerr << "Error: inactive view cannot be MIXED_ALL in MixedVariables."
	 << std::endl;
    abort_handler(-1);                                                   break;
  case MIXED_DESIGN:
    // start at the beginning
    icv_start = idiv_start = idsv_start = idrv_start = 0;
    num_icv  = num_cdv;  num_idiv = num_ddiv;
    num_idsv = num_ddsv; num_idrv = num_ddrv;                            break;
  case MIXED_ALEATORY_UNCERTAIN:
    // skip over the design variables
    icv_start  = num_cdv;  num_icv  = num_cauv;
    idiv_start = num_ddiv; num_idiv = num_dauiv;
    idsv_start = num_ddsv; num_idsv = num_dausv;
    idrv_start = num_ddrv; num_idrv = num_daurv;                         break;
  case MIXED_EPISTEMIC_UNCERTAIN:
    // skip over the design and aleatory uncertain variables
    icv_start  = num_cdv  + num_cauv;  num_icv  = num_ceuv;
    idiv_start = num_ddiv + num_dauiv; num_idiv = num_deuiv;
    idsv_start = num_ddsv + num_dausv; num_idsv = num_deusv;
    idrv_start = num_ddrv + num_daurv; num_idrv = num_deurv;             break;
  case MIXED_UNCERTAIN:
    // skip over the design variables
    icv_start  = num_cdv;  num_icv  = num_cauv  + num_ceuv;
    idiv_start = num_ddiv; num_idiv = num_dauiv + num_deuiv;
    idsv_start = num_ddsv; num_idsv = num_dausv + num_deusv;
    idrv_start = num_ddrv; num_idrv = num_daurv + num_deurv;             break;
  case MIXED_STATE:
    // skip over all the design and uncertain variables
    icv_start  = num_cdv  + num_cauv  + num_ceuv;  num_icv  = num_csv;
    idiv_start = num_ddiv + num_dauiv + num_deuiv; num_idiv = num_dsiv;
    idsv_start = num_ddsv + num_dausv + num_deusv; num_idsv = num_dssv;
    idrv_start = num_ddrv + num_daurv + num_deurv; num_idrv = num_dsrv;  break;
  }
  sharedVarsData.icv_start(icv_start);   sharedVarsData.icv(num_icv);
  sharedVarsData.idiv_start(idiv_start); sharedVarsData.idiv(num_idiv);
  sharedVarsData.idsv_start(idsv_start); sharedVarsData.idsv(num_idsv);
  sharedVarsData.idrv_start(idrv_start); sharedVarsData.idrv(num_idrv);
  sharedVarsData.initialize_inactive_components();
  if (num_icv)
    inactiveContinuousVars
      = RealVector(Teuchos::View, &allContinuousVars[icv_start],    num_icv);
  if (num_idiv)
    inactiveDiscreteIntVars
      = IntVector(Teuchos::View,  &allDiscreteIntVars[idiv_start],  num_idiv);
  if (num_idsv)
    inactiveDiscreteStringVars = allDiscreteStringVars[boost::indices[
      idx_range(idsv_start, idsv_start+num_idsv)]];
  if (num_idrv)
    inactiveDiscreteRealVars
      = RealVector(Teuchos::View, &allDiscreteRealVars[idrv_start], num_idrv);
}


// Reordering is required in all read/write cases that will be visible to the
// user since all derived vars classes should use the same CDV/DDV/UV/CSV/DSV
// ordering for clarity.  Neutral file I/O, binary streams, and packed buffers
// do not need to reorder (so long as read/write are consistent) since this data
// is not intended for public consumption.
void MixedVariables::read(std::istream& s)
{
  // ASCII version.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  read_data_partial(s, 0, num_cdv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, 0, num_ddiv, allDiscreteIntVars,
		    all_discrete_int_variable_labels());
  read_data_partial(s, 0, num_ddsv, allDiscreteStringVars,
		    all_discrete_string_variable_labels());
  read_data_partial(s, 0, num_ddrv, allDiscreteRealVars,
		    all_discrete_real_variable_labels());
  read_data_partial(s, num_cdv, num_cauv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntVars,
		    all_discrete_int_variable_labels());
  read_data_partial(s, num_ddsv, num_dausv, allDiscreteStringVars,
		    all_discrete_string_variable_labels());
  read_data_partial(s, num_ddrv, num_daurv, allDiscreteRealVars,
		    all_discrete_real_variable_labels());
  read_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, num_ddiv+num_dauiv, num_deuiv, allDiscreteIntVars,
		    all_discrete_int_variable_labels());
  read_data_partial(s, num_ddsv+num_dausv, num_deusv, allDiscreteStringVars,
		    all_discrete_string_variable_labels());
  read_data_partial(s, num_ddrv+num_daurv, num_deurv, allDiscreteRealVars,
		    all_discrete_real_variable_labels());
  read_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		    allDiscreteIntVars, all_discrete_int_variable_labels());
  read_data_partial(s, num_ddsv+num_dausv+num_deusv, num_dssv,
		    allDiscreteStringVars,
		    all_discrete_string_variable_labels());
  read_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		    allDiscreteRealVars, all_discrete_real_variable_labels());
}


void MixedVariables::write(std::ostream& s) const
{
  // ASCII version.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
    size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  write_data_partial(s, 0, num_cdv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, 0, num_ddiv, allDiscreteIntVars,
		     all_discrete_int_variable_labels());
  write_data_partial(s, 0, num_ddsv, allDiscreteStringVars,
		     all_discrete_string_variable_labels());
  write_data_partial(s, 0, num_ddrv, allDiscreteRealVars,
		     all_discrete_real_variable_labels());
  write_data_partial(s, num_cdv, num_cauv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntVars,
		     all_discrete_int_variable_labels());
  write_data_partial(s, num_ddsv, num_dausv, allDiscreteStringVars,
		     all_discrete_string_variable_labels());
  write_data_partial(s, num_ddrv, num_daurv, allDiscreteRealVars,
		     all_discrete_real_variable_labels());
  write_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, num_ddiv+num_dauiv, num_deuiv, allDiscreteIntVars,
		     all_discrete_int_variable_labels());
  write_data_partial(s, num_ddsv+num_dausv, num_deusv, allDiscreteStringVars,
		     all_discrete_string_variable_labels());
  write_data_partial(s, num_ddrv+num_daurv, num_deurv, allDiscreteRealVars,
		     all_discrete_real_variable_labels());
  write_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		     allDiscreteIntVars, all_discrete_int_variable_labels());
  write_data_partial(s, num_ddsv+num_dausv+num_deusv, num_dssv,
		     allDiscreteStringVars,
		     all_discrete_string_variable_labels());
  write_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		     allDiscreteRealVars, all_discrete_real_variable_labels());
}


void MixedVariables::write_aprepro(std::ostream& s) const
{
  // ASCII version in APREPRO/DPREPRO format.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV;
  write_data_partial_aprepro(s, 0, num_cdv, allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, 0, num_ddiv, allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, 0, num_ddsv, allDiscreteStringVars,
			     all_discrete_string_variable_labels());
  write_data_partial_aprepro(s, 0, num_ddrv, allDiscreteRealVars,
			     all_discrete_real_variable_labels());
  write_data_partial_aprepro(s, num_cdv, num_cauv, allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, num_ddiv, num_dauiv, allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, num_ddsv, num_dausv, allDiscreteStringVars,
			     all_discrete_string_variable_labels());
  write_data_partial_aprepro(s, num_ddrv, num_daurv, allDiscreteRealVars,
			     all_discrete_real_variable_labels());
  write_data_partial_aprepro(s, num_cdv+num_cauv, num_ceuv, allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, num_ddiv+num_dauiv, num_deuiv,
			     allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, num_ddsv+num_dausv, num_deusv,
			     allDiscreteStringVars,
			     all_discrete_string_variable_labels());
  write_data_partial_aprepro(s, num_ddrv+num_daurv, num_deurv,
			     allDiscreteRealVars,
			     all_discrete_real_variable_labels());
  write_data_partial_aprepro(s, num_cdv+num_cauv+num_ceuv, num_csv,
			     allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
			     allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, num_ddsv+num_dausv+num_deusv, num_dssv,
			     allDiscreteStringVars,
			     all_discrete_string_variable_labels());
  write_data_partial_aprepro(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
			     allDiscreteRealVars,
			     all_discrete_real_variable_labels());
}

/** Presumes variables object is already appropriately sized to receive! */
void MixedVariables::read_tabular(std::istream& s)
{
  // ASCII version for tabular file I/O.
  read_data_tabular(s, allContinuousVars);
  read_data_tabular(s, allDiscreteIntVars);
  read_data_tabular(s, allDiscreteStringVars);
  read_data_tabular(s, allDiscreteRealVars);
}

void MixedVariables::write_tabular(std::ostream& s) const
{
  // ASCII version for tabular file I/O.
  write_data_tabular(s, allContinuousVars);
  write_data_tabular(s, allDiscreteIntVars);
  write_data_tabular(s, allDiscreteStringVars);
  write_data_tabular(s, allDiscreteRealVars);
}

} // namespace Dakota
