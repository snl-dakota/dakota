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
  size_t num_cdv = vc_totals[TOTAL_CDV], num_cauv = vc_totals[TOTAL_CAUV],
    num_ceuv  = vc_totals[TOTAL_CEUV],
    num_acv   = num_cdv + num_cauv + num_ceuv + vc_totals[TOTAL_CSV],
    num_ddrv  = sharedVarsData.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = sharedVarsData.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_dauiv = vc_totals[TOTAL_DAUIV], num_deuiv = vc_totals[TOTAL_DEUIV],
    num_dsrv  = sharedVarsData.vc_lookup(DISCRETE_STATE_RANGE),
    num_adiv  = vc_totals[TOTAL_DDIV] + num_dauiv + num_deuiv
              + vc_totals[TOTAL_DSIV],
    num_daurv = vc_totals[TOTAL_DAURV], num_deurv = vc_totals[TOTAL_DEURV],
    num_ddsrv = vc_totals[TOTAL_DDRV],
    num_adrv  = num_ddsrv + num_daurv + num_deurv + vc_totals[TOTAL_DSRV];

  allContinuousVars.sizeUninitialized(num_acv);
  allDiscreteIntVars.sizeUninitialized(num_adiv);
  allDiscreteRealVars.sizeUninitialized(num_adrv);

  int start = 0;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_design.initial_point"), allContinuousVars, start);
  start += num_cdv;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.initial_point"),
    allContinuousVars, start);
  start += num_cauv;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.initial_point"),
    allContinuousVars, start);
  start += num_ceuv;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_state.initial_state"), allContinuousVars, start);

  start = 0;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_design_range.initial_point"),
    allDiscreteIntVars, start);
  start += num_ddrv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_design_set_int.initial_point"),
    allDiscreteIntVars, start);
  start += num_ddsiv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.initial_point"),
    allDiscreteIntVars, start);
  start += num_dauiv;
  copy_data_partial(problem_db.get_iv(
   "variables.discrete_epistemic_uncertain_int.initial_point"),
   allDiscreteIntVars, start);
  start += num_deuiv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_state_range.initial_state"), allDiscreteIntVars, start);
  start += num_dsrv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_state_set_int.initial_state"),
    allDiscreteIntVars, start);

  start = 0;
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_design_set_real.initial_point"),
    allDiscreteRealVars, start);
  start += num_ddsrv;
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.initial_point"),
    allDiscreteRealVars, start);
  start += num_daurv;
  copy_data_partial(problem_db.get_rv(
   "variables.discrete_epistemic_uncertain_real.initial_point"),
   allDiscreteRealVars, start);
  start += num_deurv;
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_state_set_real.initial_state"),
    allDiscreteRealVars, start);

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
  allContinuousVars.resize(
    variablesCompsTotals[TOTAL_CDV]  + variablesCompsTotals[TOTAL_CAUV] +
    variablesCompsTotals[TOTAL_CEUV] + variablesCompsTotals[TOTAL_CSV]);
  allDiscreteIntVars.resize(allRelaxedDiscreteInt.size());
  allDiscreteStringVars.resize(
    variablesCompsTotals[TOTAL_DDSV]  + variablesCompsTotals[TOTAL_DAUSV] +
    variablesCompsTotals[TOTAL_DEUSV] + variablesCompsTotals[TOTAL_DSSV]);
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
    num_ddrv  = vc_totals[TOTAL_DDRV],  num_cauv  = vc_totals[TOTAL_CAUV],
    num_dauiv = vc_totals[TOTAL_DAUIV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deurv = vc_totals[TOTAL_DEURV], num_csv   = vc_totals[TOTAL_CSV],
    num_dsiv  = vc_totals[TOTAL_DSIV],  num_dsrv  = vc_totals[TOTAL_DSRV];

  // Initialize active views
  size_t cv_start, div_start, drv_start, num_cv, num_div, num_drv;
  switch (sharedVarsData.view().first) {
  case EMPTY:
    Cerr << "Error: active view cannot be EMPTY in MixedVariables."<< std::endl;
    abort_handler(-1);                                                break;
  case MIXED_ALL:
    // start at the beginning
    cv_start = div_start = drv_start = 0;
    num_cv  = num_cdv  + num_cauv  + num_ceuv  + num_csv;
    num_div = num_ddiv + num_dauiv + num_deuiv + num_dsiv;
    num_dsv = num_ddsv + num_dausv + num_deusv + num_dssv;
    num_drv = num_ddrv + num_daurv + num_deurv + num_dsrv;            break;
  case MIXED_DESIGN:
    // start at the beginning
    cv_start = div_start = drv_start = 0;
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
    discreteStringVars
      = StringVector(Teuchos::View, &allDiscreteStringVars[dsv_start], num_dsv);
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
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddrv  = vc_totals[TOTAL_DDRV],  num_cauv  = vc_totals[TOTAL_CAUV],
    num_dauiv = vc_totals[TOTAL_DAUIV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deurv = vc_totals[TOTAL_DEURV], num_csv   = vc_totals[TOTAL_CSV],
    num_dsiv  = vc_totals[TOTAL_DSIV],  num_dsrv  = vc_totals[TOTAL_DSRV];

  // Initialize inactive views
  size_t icv_start, idiv_start, idrv_start, num_icv, num_idiv, num_idrv;
  switch (sharedVarsData.view().second) {
  case EMPTY:
    icv_start = idiv_start = idrv_start = num_icv = num_idiv = num_idrv = 0;
    break;
  case MIXED_ALL:
    Cerr << "Error: inactive view cannot be MIXED_ALL in MixedVariables."
	 << std::endl;
    abort_handler(-1);                                                 break;
  case MIXED_DESIGN:
    // start at the beginning
    icv_start = idiv_start = idrv_start = 0;
    num_icv  = num_cdv;
    num_idiv = num_ddiv;
    num_idrv = num_ddrv;                                                break;
  case MIXED_ALEATORY_UNCERTAIN:
    // skip over the design variables
    icv_start  = num_cdv;  num_icv  = num_cauv;
    idiv_start = num_ddiv; num_idiv = num_dauiv;
    idrv_start = num_ddrv; num_idrv = num_daurv;                         break;
  case MIXED_EPISTEMIC_UNCERTAIN:
    // skip over the design and aleatory uncertain variables
    icv_start  = num_cdv  + num_cauv;  num_icv  = num_ceuv;
    idiv_start = num_ddiv + num_dauiv; num_idiv = num_deuiv;
    idrv_start = num_ddrv + num_daurv; num_idrv = num_deurv;             break;
  case MIXED_UNCERTAIN:
    // skip over the design variables
    icv_start  = num_cdv;  num_icv  = num_cauv  + num_ceuv;
    idiv_start = num_ddiv; num_idiv = num_dauiv + num_deuiv;
    idrv_start = num_ddrv; num_idrv = num_daurv + num_deurv;             break;
  case MIXED_STATE:
    // skip over all the design and uncertain variables
    icv_start  = num_cdv  + num_cauv  + num_ceuv;  num_icv  = num_csv;
    idiv_start = num_ddiv + num_dauiv + num_deuiv; num_idiv = num_dsiv;
    idrv_start = num_ddrv + num_daurv + num_deurv; num_idrv = num_dsrv;  break;
  }
  sharedVarsData.icv_start(icv_start);   sharedVarsData.icv(num_icv);
  sharedVarsData.idiv_start(idiv_start); sharedVarsData.idiv(num_idiv);
  sharedVarsData.idrv_start(idrv_start); sharedVarsData.idrv(num_idrv);
  sharedVarsData.initialize_inactive_components();
  if (num_icv)
    inactiveContinuousVars
      = RealVector(Teuchos::View, &allContinuousVars[icv_start],    num_icv);
  if (num_idiv)
    inactiveDiscreteIntVars
      = IntVector(Teuchos::View,  &allDiscreteIntVars[idiv_start],  num_idiv);
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
    num_ddrv  = vc_totals[TOTAL_DDRV],  num_cauv  = vc_totals[TOTAL_CAUV],
    num_dauiv = vc_totals[TOTAL_DAUIV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deurv = vc_totals[TOTAL_DEURV], num_csv   = vc_totals[TOTAL_CSV],
    num_dsiv  = vc_totals[TOTAL_DSIV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  read_data_partial(s, 0, num_cdv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, 0, num_ddiv, allDiscreteIntVars,
		    all_discrete_int_variable_labels());
  read_data_partial(s, 0, num_ddrv, allDiscreteRealVars,
		    all_discrete_real_variable_labels());
  read_data_partial(s, num_cdv, num_cauv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntVars,
		    all_discrete_int_variable_labels());
  read_data_partial(s, num_ddrv, num_daurv, allDiscreteRealVars,
		    all_discrete_real_variable_labels());
  read_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, num_ddiv+num_dauiv, num_deuiv, allDiscreteIntVars,
		    all_discrete_int_variable_labels());
  read_data_partial(s, num_ddrv+num_daurv, num_deurv, allDiscreteRealVars,
		    all_discrete_real_variable_labels());
  read_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv, allContinuousVars,
		    all_continuous_variable_labels());
  read_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		    allDiscreteIntVars, all_discrete_int_variable_labels());
  read_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		    allDiscreteRealVars, all_discrete_real_variable_labels());
}


void MixedVariables::write(std::ostream& s) const
{
  // ASCII version.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
    size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddrv  = vc_totals[TOTAL_DDRV],  num_cauv  = vc_totals[TOTAL_CAUV],
    num_dauiv = vc_totals[TOTAL_DAUIV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deurv = vc_totals[TOTAL_DEURV], num_csv   = vc_totals[TOTAL_CSV],
    num_dsiv  = vc_totals[TOTAL_DSIV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  write_data_partial(s, 0, num_cdv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, 0, num_ddiv, allDiscreteIntVars,
		     all_discrete_int_variable_labels());
  write_data_partial(s, 0, num_ddrv, allDiscreteRealVars,
		     all_discrete_real_variable_labels());
  write_data_partial(s, num_cdv, num_cauv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntVars,
		     all_discrete_int_variable_labels());
  write_data_partial(s, num_ddrv, num_daurv, allDiscreteRealVars,
		     all_discrete_real_variable_labels());
  write_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, num_ddiv+num_dauiv, num_deuiv, allDiscreteIntVars,
		     all_discrete_int_variable_labels());
  write_data_partial(s, num_ddrv+num_daurv, num_deurv, allDiscreteRealVars,
		     all_discrete_real_variable_labels());
  write_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv, allContinuousVars,
		     all_continuous_variable_labels());
  write_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		     allDiscreteIntVars, all_discrete_int_variable_labels());
  write_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		     allDiscreteRealVars, all_discrete_real_variable_labels());
}


void MixedVariables::write_aprepro(std::ostream& s) const
{
  // ASCII version in APREPRO/DPREPRO format.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddrv  = vc_totals[TOTAL_DDRV],  num_cauv  = vc_totals[TOTAL_CAUV],
    num_dauiv = vc_totals[TOTAL_DAUIV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deurv = vc_totals[TOTAL_DEURV], num_csv   = vc_totals[TOTAL_CSV],
    num_dsiv  = vc_totals[TOTAL_DSIV],  num_dsrv  = vc_totals[TOTAL_DSRV];
  write_data_partial_aprepro(s, 0, num_cdv, allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, 0, num_ddiv, allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, 0, num_ddrv, allDiscreteRealVars,
			     all_discrete_real_variable_labels());
  write_data_partial_aprepro(s, num_cdv, num_cauv, allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, num_ddiv, num_dauiv, allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, num_ddrv, num_daurv, allDiscreteRealVars,
			     all_discrete_real_variable_labels());
  write_data_partial_aprepro(s, num_cdv+num_cauv, num_ceuv, allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, num_ddiv+num_dauiv, num_deuiv,
			     allDiscreteIntVars,
			     all_discrete_int_variable_labels());
  write_data_partial_aprepro(s, num_ddrv+num_daurv, num_deurv,
			     allDiscreteRealVars,
			     all_discrete_real_variable_labels());
  write_data_partial_aprepro(s, num_cdv+num_cauv+num_ceuv, num_csv,
			     allContinuousVars,
			     all_continuous_variable_labels());
  write_data_partial_aprepro(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
			     allDiscreteIntVars,
			     all_discrete_int_variable_labels());
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
  read_data_tabular(s, allDiscreteRealVars);
}

void MixedVariables::write_tabular(std::ostream& s) const
{
  // ASCII version for tabular file I/O.
  write_data_tabular(s, allContinuousVars);
  write_data_tabular(s, allDiscreteIntVars);
  write_data_tabular(s, allDiscreteRealVars);
}

} // namespace Dakota
