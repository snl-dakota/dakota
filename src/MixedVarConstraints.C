/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MixedVarConstraints
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "MixedVarConstraints.H"
#include "ProblemDescDB.H"
#include "data_io.h"
#include "data_util.h"

static const char rcsId[]="@(#) $Id";


using namespace std;

namespace Dakota {

/** In this class, mixed continuous/discrete variables are used.  Most
    iterators/strategies use this approach, which is the default in
    Constraints::get_constraints(). */
MixedVarConstraints::
MixedVarConstraints(const ProblemDescDB& problem_db,
		    const SharedVariablesData& svd):
  Constraints(BaseConstructor(), problem_db, svd)
{
  const SizetArray& vc_totals = svd.components_totals();
  size_t num_cdv = vc_totals[0], num_cauv = vc_totals[3],
    num_ceuv  = vc_totals[6],
    num_acv   = num_cdv + num_cauv + num_ceuv + vc_totals[9],
    num_ddrv  = sharedVarsData.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = sharedVarsData.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_dauiv = vc_totals[4], num_deuiv = vc_totals[7],
    num_dsrv  = sharedVarsData.vc_lookup(DISCRETE_STATE_RANGE),
    num_adiv  = vc_totals[1] + num_dauiv + num_deuiv + vc_totals[10],
    num_ddsrv = vc_totals[2], num_daurv = vc_totals[5],
    num_deurv = vc_totals[8],
    num_adrv  = num_ddsrv + num_daurv + num_deurv + vc_totals[11];

  allContinuousLowerBnds.sizeUninitialized(num_acv);
  allContinuousUpperBnds.sizeUninitialized(num_acv);
  allDiscreteIntLowerBnds.sizeUninitialized(num_adiv);
  allDiscreteIntUpperBnds.sizeUninitialized(num_adiv);
  allDiscreteRealLowerBnds.sizeUninitialized(num_adrv);
  allDiscreteRealUpperBnds.sizeUninitialized(num_adrv);

  int start = 0;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_design.lower_bounds"), allContinuousLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_design.upper_bounds"), allContinuousUpperBnds, start);
  start += num_cdv;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.lower_bounds"),
    allContinuousLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.upper_bounds"),
    allContinuousUpperBnds, start);
  start += num_cauv;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.lower_bounds"),
    allContinuousLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.upper_bounds"),
    allContinuousUpperBnds, start);
  start += num_ceuv;
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_state.lower_bounds"), allContinuousLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.continuous_state.upper_bounds"), allContinuousUpperBnds, start);

  start = 0;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_design_range.lower_bounds"),
    allDiscreteIntLowerBnds, start);
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_design_range.upper_bounds"),
    allDiscreteIntUpperBnds, start);
  start += num_ddrv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_design_set_int.lower_bounds"),
    allDiscreteIntLowerBnds, start);
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_design_set_int.upper_bounds"),
    allDiscreteIntUpperBnds, start);
  start += num_ddsiv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.lower_bounds"),
    allDiscreteIntLowerBnds, start);
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.upper_bounds"),
    allDiscreteIntUpperBnds, start);
  start += num_dauiv;
  //copy_data_partial(problem_db.get_iv(
  //  "variables.discrete_epistemic_uncertain_int.lower_bounds"),
  //  allDiscreteIntLowerBnds, start);
  //copy_data_partial(problem_db.get_iv(
  //  "variables.discrete_epistemic_uncertain_int.upper_bounds"),
  //  allDiscreteIntUpperBnds, start);
  //start += num_deuiv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_state_range.lower_bounds"),
    allDiscreteIntLowerBnds, start);
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_state_range.upper_bounds"),
    allDiscreteIntUpperBnds, start);
  start += num_dsrv;
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_state_set_int.lower_bounds"),
    allDiscreteIntLowerBnds, start);
  copy_data_partial(problem_db.get_iv(
    "variables.discrete_state_set_int.upper_bounds"),
    allDiscreteIntUpperBnds, start);

  start = 0;
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_design_set_real.lower_bounds"),
    allDiscreteRealLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_design_set_real.upper_bounds"),
    allDiscreteRealUpperBnds, start);
  start += num_ddsrv;
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.lower_bounds"),
    allDiscreteRealLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.upper_bounds"),
    allDiscreteRealUpperBnds, start);
  start += num_daurv;
  //copy_data_partial(problem_db.get_rv(
  //  "variables.discrete_epistemic_uncertain_real.lower_bounds"),
  //  allDiscreteRealLowerBnds, start);
  //copy_data_partial(problem_db.get_rv(
  //  "variables.discrete_epistemic_uncertain_real.upper_bounds"),
  //  allDiscreteRealUpperBnds, start);
  //start += num_deurv;
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_state_set_real.lower_bounds"),
    allDiscreteRealLowerBnds, start);
  copy_data_partial(problem_db.get_rv(
    "variables.discrete_state_set_real.upper_bounds"),
    allDiscreteRealUpperBnds, start);

  // construct active/inactive views of all arrays
  build_views();

  // Manage linear constraints.  Verify specification sanity and set defaults
  // if needed.
  const std::pair<short,short>& view = sharedVarsData.view();
  if (view.first == MIXED_ALEATORY_UNCERTAIN  ||
      view.first == MIXED_EPISTEMIC_UNCERTAIN ||
      view.first == MIXED_UNCERTAIN) {
    if ( problem_db.get_rv("method.linear_inequality_constraints").length()
      || problem_db.get_rv("method.linear_equality_constraints").length() )
      Cerr << "Warning: linear constraints not supported with nondeterministic "
	   << "iterators\n         Input will be ignored.";
    numLinearIneqCons = numLinearEqCons = 0;
  }
  else
    manage_linear_constraints(problem_db);

#ifdef REFCOUNT_DEBUG
  Cout << "Letter instantiated: view active = " << view.first << " inactive = "
       << view.second << endl;
#endif
}


void MixedVarConstraints::reshape(const SizetArray& vc_totals)
{
  size_t num_acv  = vc_totals[0] + vc_totals[3] + vc_totals[6] + vc_totals[9],
         num_adiv = vc_totals[1] + vc_totals[4] + vc_totals[7] + vc_totals[10],
         num_adrv = vc_totals[2] + vc_totals[5] + vc_totals[8] + vc_totals[11];

  allContinuousLowerBnds.resize(num_acv);
  allContinuousUpperBnds.resize(num_acv);
  allDiscreteIntLowerBnds.resize(num_adiv);
  allDiscreteIntUpperBnds.resize(num_adiv);
  allDiscreteRealLowerBnds.resize(num_adrv);
  allDiscreteRealUpperBnds.resize(num_adrv);

  build_views();

  // base class portion uses size of active views
  Constraints::reshape(vc_totals);
}


void MixedVarConstraints::build_active_views()
{
  // Initialize continuousVarTypes/discreteVarTypes/continuousVarIds.
  // Don't bleed over any logic about supported view combinations; rather,
  // keep this class general and encapsulated.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv  = vc_totals[11];

  // Initialize active views
  size_t cv_start, div_start, drv_start, num_cv, num_div, num_drv;
  switch (sharedVarsData.view().first) {
  case EMPTY:
    Cerr << "Error: active view cannot be EMPTY in MixedVarConstraints."
	 << std::endl;
    abort_handler(-1);                                                break;
  case MIXED_ALL:
    cv_start = div_start = drv_start = 0;
    num_cv  = num_cdv  + num_cauv  + num_ceuv  + num_csv;
    num_div = num_ddiv + num_dauiv + num_deuiv + num_dsiv;
    num_drv = num_ddrv + num_daurv + num_deurv + num_dsrv;            break;
  case MIXED_DESIGN:
    cv_start = div_start = drv_start = 0;
    num_cv  = num_cdv;
    num_div = num_ddiv;
    num_drv = num_ddrv;                                               break;
  case MIXED_ALEATORY_UNCERTAIN:
    cv_start  = num_cdv;  num_cv  = num_cauv;
    div_start = num_ddiv; num_div = num_dauiv;
    drv_start = num_ddrv; num_drv = num_daurv;                        break;
  case MIXED_EPISTEMIC_UNCERTAIN:
    cv_start  = num_cdv  + num_cauv;  num_cv  = num_ceuv;
    div_start = num_ddiv + num_dauiv; num_div = num_deuiv;
    drv_start = num_ddrv + num_daurv; num_drv = num_deurv;            break;
  case MIXED_UNCERTAIN:
    cv_start  = num_cdv;  num_cv  = num_cauv + num_ceuv;
    div_start = num_ddiv; num_div = num_dauiv + num_deuiv;
    drv_start = num_ddrv; num_drv = num_daurv + num_deurv;            break;
  case MIXED_STATE:
    cv_start  = num_cdv  + num_cauv  + num_ceuv;  num_cv  = num_csv;
    div_start = num_ddiv + num_dauiv + num_deuiv; num_div = num_dsiv;
    drv_start = num_ddrv + num_daurv + num_deurv; num_drv = num_dsrv; break;
  }
  if (num_cv) {
    continuousLowerBnds = RealVector(Teuchos::View,
      &allContinuousLowerBnds[cv_start], num_cv);
    continuousUpperBnds = RealVector(Teuchos::View,
      &allContinuousUpperBnds[cv_start], num_cv);
  }
  if (num_div) {
    discreteIntLowerBnds = IntVector(Teuchos::View,
      &allDiscreteIntLowerBnds[div_start], num_div);
    discreteIntUpperBnds = IntVector(Teuchos::View,
      &allDiscreteIntUpperBnds[div_start], num_div);
  }
  if (num_drv) {
    discreteRealLowerBnds = RealVector(Teuchos::View,
      &allDiscreteRealLowerBnds[drv_start], num_drv);
    discreteRealUpperBnds = RealVector(Teuchos::View,
      &allDiscreteRealUpperBnds[drv_start], num_drv);
  }
}


void MixedVarConstraints::build_inactive_views()
{
  // Initialize continuousVarTypes/discreteVarTypes/continuousVarIds.
  // Don't bleed over any logic about supported view combinations; rather,
  // keep this class general and encapsulated.
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv  = vc_totals[11];

  // Initialize inactive views
  size_t icv_start, idiv_start, idrv_start, num_icv, num_idiv, num_idrv;
  switch (sharedVarsData.view().second) {
  case EMPTY:
    icv_start = idiv_start = idrv_start = num_icv = num_idiv = num_idrv = 0;
    break;
  case MIXED_ALL:
    Cerr << "Error: inactive view cannot be MIXED_ALL in MixedVarConstraints."
	 << std::endl;
    abort_handler(-1);                                                  break;
  case MIXED_DESIGN:
    icv_start = idiv_start = idrv_start = 0;
    num_icv  = num_cdv;
    num_idiv = num_ddiv;
    num_idrv = num_ddrv;                                                break;
  case MIXED_ALEATORY_UNCERTAIN:
    icv_start  = num_cdv;  num_icv  = num_cauv;
    idiv_start = num_ddiv; num_idiv = num_dauiv;
    idrv_start = num_ddrv; num_idrv = num_daurv;                        break;
  case MIXED_EPISTEMIC_UNCERTAIN:
    icv_start  = num_cdv  + num_cauv;  num_icv  = num_ceuv;
    idiv_start = num_ddiv + num_dauiv; num_idiv = num_deuiv;
    idrv_start = num_ddrv + num_daurv; num_idrv = num_deurv;            break;
  case MIXED_UNCERTAIN:
    icv_start  = num_cdv;  num_icv  = num_cauv + num_ceuv;
    idiv_start = num_ddiv; num_idiv = num_dauiv + num_deuiv;
    idrv_start = num_ddrv; num_idrv = num_daurv + num_deurv;            break;
  case MIXED_STATE:
    icv_start  = num_cdv  + num_cauv  + num_ceuv;  num_icv  = num_csv;
    idiv_start = num_ddiv + num_dauiv + num_deuiv; num_idiv = num_dsiv;
    idrv_start = num_ddrv + num_daurv + num_deurv; num_idrv = num_dsrv; break;
  }
  if (num_icv) {
    inactiveContinuousLowerBnds = RealVector(Teuchos::View,
      &allContinuousLowerBnds[icv_start], num_icv);
    inactiveContinuousUpperBnds = RealVector(Teuchos::View,
      &allContinuousUpperBnds[icv_start], num_icv);
  }
  if (num_idiv) {
    inactiveDiscreteIntLowerBnds = IntVector(Teuchos::View,
      &allDiscreteIntLowerBnds[idiv_start], num_idiv);
    inactiveDiscreteIntUpperBnds = IntVector(Teuchos::View,
      &allDiscreteIntUpperBnds[idiv_start], num_idiv);
  }
  if (num_idrv) {
    inactiveDiscreteRealLowerBnds = RealVector(Teuchos::View,
      &allDiscreteRealLowerBnds[idrv_start], num_idrv);
    inactiveDiscreteRealUpperBnds = RealVector(Teuchos::View,
      &allDiscreteRealUpperBnds[idrv_start], num_idrv);
  }
}


void MixedVarConstraints::read(std::istream& s)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv  = vc_totals[11];

  read_data_partial(s, 0, num_cdv, allContinuousLowerBnds);
  read_data_partial(s, 0, num_ddiv, allDiscreteIntLowerBnds);
  read_data_partial(s, 0, num_ddrv, allDiscreteRealLowerBnds);
  read_data_partial(s, num_cdv, num_cauv, allContinuousLowerBnds);
  read_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntLowerBnds);
  read_data_partial(s, num_ddrv, num_daurv, allDiscreteRealLowerBnds);
  read_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousLowerBnds);
  //read_data_partial(s, num_ddiv+num_dauiv, num_deuiv,
  //                  allDiscreteIntLowerBnds);
  //read_data_partial(s, num_ddrv+num_daurv, num_deurv,
  //                  allDiscreteRealLowerBnds);
  read_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv,
		    allContinuousLowerBnds);
  read_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		    allDiscreteIntLowerBnds);
  read_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		    allDiscreteRealLowerBnds);

  read_data_partial(s, 0, num_cdv, allContinuousUpperBnds);
  read_data_partial(s, 0, num_ddiv, allDiscreteIntUpperBnds);
  read_data_partial(s, 0, num_ddrv, allDiscreteRealUpperBnds);
  read_data_partial(s, num_cdv, num_cauv, allContinuousUpperBnds);
  read_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntUpperBnds);
  read_data_partial(s, num_ddrv, num_daurv, allDiscreteRealUpperBnds);
  read_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousUpperBnds);
  //read_data_partial(s, num_ddiv+num_dauiv, num_deuiv,
  //                  allDiscreteIntUpperBnds);
  //read_data_partial(s, num_ddrv+num_daurv, num_deurv,
  //                  allDiscreteRealUpperBnds);
  read_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv,
		    allContinuousUpperBnds);
  read_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		    allDiscreteIntUpperBnds);
  read_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		    allDiscreteRealUpperBnds);
}


void MixedVarConstraints::write(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv  = vc_totals[11];

  write_data_partial(s, 0, num_cdv, allContinuousLowerBnds);
  write_data_partial(s, 0, num_ddiv, allDiscreteIntLowerBnds);
  write_data_partial(s, 0, num_ddrv, allDiscreteRealLowerBnds);
  write_data_partial(s, num_cdv, num_cauv, allContinuousLowerBnds);
  write_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntLowerBnds);
  write_data_partial(s, num_ddrv, num_daurv, allDiscreteRealLowerBnds);
  write_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousLowerBnds);
  //write_data_partial(s, num_ddiv+num_dauiv, num_deuiv,
  //                   allDiscreteIntLowerBnds);
  //write_data_partial(s, num_ddrv+num_daurv, num_deurv,
  //		       allDiscreteRealLowerBnds);
  write_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv,
		     allContinuousLowerBnds);
  write_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		    allDiscreteIntLowerBnds);
  write_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		    allDiscreteRealLowerBnds);

  write_data_partial(s, 0, num_cdv, allContinuousUpperBnds);
  write_data_partial(s, 0, num_ddiv, allDiscreteIntUpperBnds);
  write_data_partial(s, 0, num_ddrv, allDiscreteRealUpperBnds);
  write_data_partial(s, num_cdv, num_cauv, allContinuousUpperBnds);
  write_data_partial(s, num_ddiv, num_dauiv, allDiscreteIntUpperBnds);
  write_data_partial(s, num_ddrv, num_daurv, allDiscreteRealUpperBnds);
  write_data_partial(s, num_cdv+num_cauv, num_ceuv, allContinuousUpperBnds);
  //write_data_partial(s, num_ddiv+num_dauiv, num_deuiv,
  //                   allDiscreteIntUpperBnds);
  //write_data_partial(s, num_ddrv+num_daurv, num_deurv,
  //		       allDiscreteRealUpperBnds);
  write_data_partial(s, num_cdv+num_cauv+num_ceuv, num_csv,
		     allContinuousUpperBnds);
  write_data_partial(s, num_ddiv+num_dauiv+num_deuiv, num_dsiv,
		    allDiscreteIntUpperBnds);
  write_data_partial(s, num_ddrv+num_daurv+num_deurv, num_dsrv,
		    allDiscreteRealUpperBnds);
}

} // namespace Dakota
