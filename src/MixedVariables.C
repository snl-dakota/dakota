/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MixedVariables
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "MixedVariables.H"
#include "ProblemDescDB.H"
#include "DakotaBinStream.H"
#include "data_io.h"
#include "data_util.h"

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
  size_t num_cdv = vc_totals[0], num_cauv = vc_totals[3],
    num_ceuv  = vc_totals[6],
    num_acv   = num_cdv + num_cauv + num_ceuv + vc_totals[9],
    num_ddrv  = sharedVarsData.vc_lookup(DISCRETE_DESIGN_RANGE),
    num_ddsiv = sharedVarsData.vc_lookup(DISCRETE_DESIGN_SET_INT),
    num_dauiv = vc_totals[4], num_deuiv = vc_totals[7],
    num_dsrv  = sharedVarsData.vc_lookup(DISCRETE_STATE_RANGE),
    num_adiv  = vc_totals[1] + num_dauiv + num_deuiv + vc_totals[10],
    num_daurv = vc_totals[5], num_deurv = vc_totals[8],
    num_ddsrv = vc_totals[2],
    num_adrv  = num_ddsrv + num_daurv + num_deurv + vc_totals[11];

  allContinuousVars.sizeUninitialized(num_acv);
  allDiscreteIntVars.sizeUninitialized(num_adiv);
  allDiscreteRealVars.sizeUninitialized(num_adrv);

  int start = 0;
  copy_data_partial(problem_db.get_rdv(
    "variables.continuous_design.initial_point"), allContinuousVars, start);
  start += num_cdv;
  copy_data_partial(problem_db.get_rdv(
    "variables.continuous_aleatory_uncertain.initial_point"),
    allContinuousVars, start);
  start += num_cauv;
  copy_data_partial(problem_db.get_rdv(
    "variables.continuous_epistemic_uncertain.initial_point"),
    allContinuousVars, start);
  start += num_ceuv;
  copy_data_partial(problem_db.get_rdv(
    "variables.continuous_state.initial_state"), allContinuousVars, start);

  start = 0;
  copy_data_partial(problem_db.get_idv(
    "variables.discrete_design_range.initial_point"),
    allDiscreteIntVars, start);
  start += num_ddrv;
  copy_data_partial(problem_db.get_idv(
    "variables.discrete_design_set_int.initial_point"),
    allDiscreteIntVars, start);
  start += num_ddsiv;
  copy_data_partial(problem_db.get_idv(
    "variables.discrete_aleatory_uncertain_int.initial_point"),
    allDiscreteIntVars, start);
  start += num_dauiv;
  //copy_data_partial(problem_db.get_idv(
  //  "variables.discrete_epistemic_uncertain_int.initial_point"),
  //  allDiscreteIntVars, start);
  //start += num_deuiv;
  copy_data_partial(problem_db.get_idv(
    "variables.discrete_state_range.initial_state"),
    allDiscreteIntVars, start);
  start += num_dsrv;
  copy_data_partial(problem_db.get_idv(
    "variables.discrete_state_set_int.initial_state"),
    allDiscreteIntVars, start);

  start = 0;
  copy_data_partial(problem_db.get_rdv(
    "variables.discrete_design_set_real.initial_point"),
    allDiscreteRealVars, start);
  start += num_ddsrv;
  copy_data_partial(problem_db.get_rdv(
    "variables.discrete_aleatory_uncertain_real.initial_point"),
    allDiscreteRealVars, start);
  start += num_daurv;
  //copy_data_partial(problem_db.get_rdv(
  //  "variables.discrete_epistemic_uncertain_real.initial_point"),
  //  allDiscreteRealVars, start);
  //start += num_deurv;
  copy_data_partial(problem_db.get_rdv(
    "variables.discrete_state_set_real.initial_state"),
    allDiscreteRealVars, start);

  // construct active/inactive views of all arrays
  build_views();

#ifdef REFCOUNT_DEBUG
  Cout << "Letter instantiated: variablesView active = " << variablesView.first
       << " inactive = " << variablesView.second << std::endl;
#endif
}


void MixedVariables::reshape(const SizetArray& vc_totals)
{
  size_t num_acv  = vc_totals[0] + vc_totals[3] + vc_totals[6] + vc_totals[9],
         num_adiv = vc_totals[1] + vc_totals[4] + vc_totals[7] + vc_totals[10],
         num_adrv = vc_totals[2] + vc_totals[5] + vc_totals[8] + vc_totals[11];

  allContinuousVars.resize(num_acv);
  allDiscreteIntVars.resize(num_adiv);
  allDiscreteRealVars.resize(num_adrv);

  build_views(); // construct active/inactive views of all arrays
}


void MixedVariables::build_active_views()
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
  switch (sharedVarsData.view().first) {
  case EMPTY:
    Cerr << "Error: active view cannot be EMPTY in MixedVariables."
	 << std::endl;
    abort_handler(-1);                                              break;
  case MIXED_ALL:
    cvStart = divStart = drvStart = 0;
    numCV  = num_cdv  + num_cauv  + num_ceuv  + num_csv;
    numDIV = num_ddiv + num_dauiv + num_deuiv + num_dsiv;
    numDRV = num_ddrv + num_daurv + num_deurv + num_dsrv;           break;
  case MIXED_DISTINCT_DESIGN:
    cvStart = divStart = drvStart = 0;
    numCV  = num_cdv;
    numDIV = num_ddiv;
    numDRV = num_ddrv;                                              break;
  case MIXED_DISTINCT_ALEATORY_UNCERTAIN:
    cvStart  = num_cdv;  numCV  = num_cauv;
    divStart = num_ddiv; numDIV = num_dauiv;
    drvStart = num_ddrv; numDRV = num_daurv;                        break;
  case MIXED_DISTINCT_EPISTEMIC_UNCERTAIN:
    cvStart  = num_cdv  + num_cauv;  numCV  = num_ceuv;
    divStart = num_ddiv + num_dauiv; numDIV = num_deuiv;
    drvStart = num_ddrv + num_daurv; numDRV = num_deurv;            break;
  case MIXED_DISTINCT_UNCERTAIN:
    cvStart  = num_cdv;  numCV  = num_cauv + num_ceuv;
    divStart = num_ddiv; numDIV = num_dauiv + num_deuiv;
    drvStart = num_ddrv; numDRV = num_daurv + num_deurv;            break;
  case MIXED_DISTINCT_STATE:
    cvStart  = num_cdv  + num_cauv  + num_ceuv;  numCV  = num_csv;
    divStart = num_ddiv + num_dauiv + num_deuiv; numDIV = num_dsiv;
    drvStart = num_ddrv + num_daurv + num_deurv; numDRV = num_dsrv; break;
  }
  if (numCV)
    continuousVars
      = RealVector(Teuchos::View, &allContinuousVars[cvStart], numCV);
  if (numDIV)
    discreteIntVars
      = IntVector(Teuchos::View, &allDiscreteIntVars[divStart], numDIV);
  if (numDRV)
    discreteRealVars
      = RealVector(Teuchos::View, &allDiscreteRealVars[drvStart], numDRV);
}


void MixedVariables::build_inactive_views()
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
  switch (sharedVarsData.view().second) {
  case EMPTY:
    icvStart = idivStart = idrvStart = numICV = numIDIV = numIDRV = 0; break;
  case MIXED_ALL:
    Cerr << "Error: inactive view cannot be MIXED_ALL in MixedVariables."
	 << std::endl;
    abort_handler(-1);                                                 break;
  case MIXED_DISTINCT_DESIGN:
    icvStart = idivStart = idrvStart = 0;
    numICV  = num_cdv;
    numIDIV = num_ddiv;
    numIDRV = num_ddrv;                                                break;
  case MIXED_DISTINCT_ALEATORY_UNCERTAIN:
    icvStart  = num_cdv;  numICV  = num_cauv;
    idivStart = num_ddiv; numIDIV = num_dauiv;
    idrvStart = num_ddrv; numIDRV = num_daurv;                         break;
  case MIXED_DISTINCT_EPISTEMIC_UNCERTAIN:
    icvStart  = num_cdv  + num_cauv;  numICV  = num_ceuv;
    idivStart = num_ddiv + num_dauiv; numIDIV = num_deuiv;
    idrvStart = num_ddrv + num_daurv; numIDRV = num_deurv;             break;
  case MIXED_DISTINCT_UNCERTAIN:
    icvStart  = num_cdv;  numICV  = num_cauv + num_ceuv;
    idivStart = num_ddiv; numIDIV = num_dauiv + num_deuiv;
    idrvStart = num_ddrv; numIDRV = num_daurv + num_deurv;             break;
  case MIXED_DISTINCT_STATE:
    icvStart  = num_cdv  + num_cauv  + num_ceuv;  numICV  = num_csv;
    idivStart = num_ddiv + num_dauiv + num_deuiv; numIDIV = num_dsiv;
    idrvStart = num_ddrv + num_daurv + num_deurv; numIDRV = num_dsrv;  break;
  }
  if (numICV)
    inactiveContinuousVars
      = RealVector(Teuchos::View, &allContinuousVars[icvStart],    numICV);
  if (numIDIV)
    inactiveDiscreteIntVars
      = IntVector(Teuchos::View,  &allDiscreteIntVars[idivStart],  numIDIV);
  if (numIDRV)
    inactiveDiscreteRealVars
      = RealVector(Teuchos::View, &allDiscreteRealVars[idrvStart], numIDRV);
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
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv = vc_totals[11];
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
  //read_data_partial(s, num_ddiv+num_dauiv, num_deuiv, allDiscreteIntVars,
  //		      all_discrete_int_variable_labels());
  //read_data_partial(s, num_ddrv+num_daurv, num_deurv, allDiscreteRealVars,
    //		      all_discrete_real_variable_labels());
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
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv = vc_totals[11];
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
  //write_data_partial(s, num_ddiv+num_dauiv, num_deuiv, allDiscreteIntVars,
  //		       all_discrete_int_variable_labels());
  //write_data_partial(s, num_ddrv+num_daurv, num_deurv, allDiscreteRealVars,
  //		       all_discrete_real_variable_labels());
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
  size_t num_cdv = vc_totals[0], num_ddiv = vc_totals[1],
    num_ddrv  = vc_totals[2], num_cauv = vc_totals[3], num_dauiv = vc_totals[4],
    num_daurv = vc_totals[5], num_ceuv = vc_totals[6], num_deuiv = vc_totals[7],
    num_deurv = vc_totals[8], num_csv  = vc_totals[9], num_dsiv = vc_totals[10],
    num_dsrv = vc_totals[11];
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
  //write_data_partial_aprepro(s, num_ddiv+num_dauiv, num_deuiv,
  //			       allDiscreteIntVars,
  //                           all_discrete_int_variable_labels());
  //write_data_partial_aprepro(s, num_ddrv+num_daurv, num_deurv,
  //			       allDiscreteRealVars,
  //                           all_discrete_real_variable_labels());
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
