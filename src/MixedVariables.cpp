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


// Reordering is required in all read/write cases that will be visible to the
// user since all derived vars classes should use the same CDV/DDV/UV/CSV/DSV
// ordering for clarity.  Neutral file I/O, binary streams, and packed buffers
// do not need to reorder (so long as read/write are consistent) since this data
// is not intended for public consumption.
void MixedVariables::read(std::istream& s)
{
  // ASCII version
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  read_data_partial(s, acv_offset,  num_cdv,  allContinuousVars,   acv_labels);
  read_data_partial(s, adiv_offset, num_ddiv, allDiscreteIntVars,  adiv_labels);
  read_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringVars,
		    adsv_labels);
  read_data_partial(s, adrv_offset, num_ddrv, allDiscreteRealVars, adrv_labels);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  read_data_partial(s, acv_offset,  num_cauv,  allContinuousVars,  acv_labels);
  read_data_partial(s, adiv_offset, num_dauiv, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_dausv, allDiscreteStringVars,
		    adsv_labels);
  read_data_partial(s, adrv_offset, num_daurv, allDiscreteRealVars,
		    adrv_labels);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  read_data_partial(s, acv_offset,  num_ceuv,  allContinuousVars,  acv_labels);
  read_data_partial(s, adiv_offset, num_deuiv, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_deusv, allDiscreteStringVars,
		    adsv_labels);
  read_data_partial(s, adrv_offset, num_deurv, allDiscreteRealVars,
		    adrv_labels);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  read_data_partial(s, acv_offset,  num_csv,  allContinuousVars,   acv_labels);
  read_data_partial(s, adiv_offset, num_dsiv, allDiscreteIntVars,  adiv_labels);
  read_data_partial(s, adsv_offset, num_dssv, allDiscreteStringVars,
		    adsv_labels);
  read_data_partial(s, adrv_offset, num_dsrv, allDiscreteRealVars, adrv_labels);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}


void MixedVariables::write(std::ostream& s) const
{
  // ASCII version
  const SizetArray& vc_totals = sharedVarsData.components_totals();
    size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  write_data_partial(s, acv_offset,  num_cdv,  allContinuousVars,  acv_labels);
  write_data_partial(s, adiv_offset, num_ddiv, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringVars,
		     adsv_labels);
  write_data_partial(s, adrv_offset, num_ddrv, allDiscreteRealVars,
		     adrv_labels);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  write_data_partial(s, acv_offset,  num_cauv,  allContinuousVars, acv_labels);
  write_data_partial(s, adiv_offset, num_dauiv, allDiscreteIntVars,
		     adiv_labels);
  write_data_partial(s, adsv_offset, num_dausv, allDiscreteStringVars,
		     adsv_labels);
  write_data_partial(s, adrv_offset, num_daurv, allDiscreteRealVars,
		     adrv_labels);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  write_data_partial(s, acv_offset,  num_ceuv,  allContinuousVars, acv_labels);
  write_data_partial(s, adiv_offset, num_deuiv, allDiscreteIntVars,
		     adiv_labels);
  write_data_partial(s, adsv_offset, num_deusv, allDiscreteStringVars,
		     adsv_labels);
  write_data_partial(s, adrv_offset, num_deurv, allDiscreteRealVars,
		     adrv_labels);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  write_data_partial(s, acv_offset,  num_csv,  allContinuousVars,  acv_labels);
  write_data_partial(s, adiv_offset, num_dsiv, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_dssv, allDiscreteStringVars,
		     adsv_labels);
  write_data_partial(s, adrv_offset, num_dsrv, allDiscreteRealVars,
		     adrv_labels);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}


void MixedVariables::write_aprepro(std::ostream& s) const
{
  // ASCII version in APREPRO/DPREPRO format
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  write_data_partial_aprepro(s, acv_offset,  num_cdv,  allContinuousVars,
			     acv_labels);
  write_data_partial_aprepro(s, adiv_offset, num_ddiv, allDiscreteIntVars,
			     adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_ddsv, allDiscreteStringVars,
			     adsv_labels);
  write_data_partial_aprepro(s, adrv_offset, num_ddrv, allDiscreteRealVars,
			     adrv_labels);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  write_data_partial_aprepro(s, acv_offset,  num_cauv,  allContinuousVars,
			     acv_labels);
  write_data_partial_aprepro(s, adiv_offset, num_dauiv, allDiscreteIntVars,
			     adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_dausv, allDiscreteStringVars,
			     adsv_labels);
  write_data_partial_aprepro(s, adrv_offset, num_daurv, allDiscreteRealVars,
			     adrv_labels);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  write_data_partial_aprepro(s, acv_offset,  num_ceuv,  allContinuousVars,
			     acv_labels);
  write_data_partial_aprepro(s, adiv_offset, num_deuiv, allDiscreteIntVars,
			     adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_deusv, allDiscreteStringVars,
			     adsv_labels);
  write_data_partial_aprepro(s, adrv_offset, num_deurv, allDiscreteRealVars,
			     adrv_labels);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  write_data_partial_aprepro(s, acv_offset,  num_csv,  allContinuousVars,
			     acv_labels);
  write_data_partial_aprepro(s, adiv_offset, num_dsiv, allDiscreteIntVars,
			     adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_dssv, allDiscreteStringVars,
			     adsv_labels);
  write_data_partial_aprepro(s, adrv_offset, num_dsrv, allDiscreteRealVars,
			     adrv_labels);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}


/** Presumes variables object is already appropriately sized to receive! */
void MixedVariables::read_tabular(std::istream& s)
{
  // ASCII version for tabular file I/O
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  read_data_partial_tabular(s, acv_offset,  num_cdv,  allContinuousVars);
  read_data_partial_tabular(s, adiv_offset, num_ddiv, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_ddsv, allDiscreteStringVars);
  read_data_partial_tabular(s, adrv_offset, num_ddrv, allDiscreteRealVars);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  read_data_partial_tabular(s, acv_offset,  num_cauv,  allContinuousVars);
  read_data_partial_tabular(s, adiv_offset, num_dauiv, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_dausv, allDiscreteStringVars);
  read_data_partial_tabular(s, adrv_offset, num_daurv, allDiscreteRealVars);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  read_data_partial_tabular(s, acv_offset,  num_ceuv,  allContinuousVars);
  read_data_partial_tabular(s, adiv_offset, num_deuiv, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_deusv, allDiscreteStringVars);
  read_data_partial_tabular(s, adrv_offset, num_deurv, allDiscreteRealVars);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  read_data_partial_tabular(s, acv_offset,  num_csv,  allContinuousVars);
  read_data_partial_tabular(s, adiv_offset, num_dsiv, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_dssv, allDiscreteStringVars);
  read_data_partial_tabular(s, adrv_offset, num_dsrv, allDiscreteRealVars);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}


void MixedVariables::write_tabular(std::ostream& s) const
{
  // ASCII version for tabular file I/O
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  write_data_partial_tabular(s, acv_offset,  num_cdv,  allContinuousVars);
  write_data_partial_tabular(s, adiv_offset, num_ddiv, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_ddsv, allDiscreteStringVars);
  write_data_partial_tabular(s, adrv_offset, num_ddrv, allDiscreteRealVars);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  write_data_partial_tabular(s, acv_offset,  num_cauv,  allContinuousVars);
  write_data_partial_tabular(s, adiv_offset, num_dauiv, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_dausv, allDiscreteStringVars);
  write_data_partial_tabular(s, adrv_offset, num_daurv, allDiscreteRealVars);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  write_data_partial_tabular(s, acv_offset,  num_ceuv,  allContinuousVars);
  write_data_partial_tabular(s, adiv_offset, num_deuiv, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_deusv, allDiscreteStringVars);
  write_data_partial_tabular(s, adrv_offset, num_deurv, allDiscreteRealVars);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  write_data_partial_tabular(s, acv_offset,  num_csv,  allContinuousVars);
  write_data_partial_tabular(s, adiv_offset, num_dsiv, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_dssv, allDiscreteStringVars);
  write_data_partial_tabular(s, adrv_offset, num_dsrv, allDiscreteRealVars);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}

} // namespace Dakota
