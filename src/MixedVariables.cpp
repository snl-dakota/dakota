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
#include "dakota_tabular_io.hpp"

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
  int start = 0;
  const RealVector& cdv  = problem_db.get_rv(
    "variables.continuous_design.initial_point");
  const RealVector& cauv = problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.initial_point");
  const RealVector& ceuv = problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.initial_point");
  const RealVector& csv  = problem_db.get_rv(
    "variables.continuous_state.initial_state");
  copy_data_partial(cdv,  allContinuousVars, start); start += cdv.length();
  copy_data_partial(cauv, allContinuousVars, start); start += cauv.length();
  copy_data_partial(ceuv, allContinuousVars, start); start += ceuv.length();
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
  copy_data_partial(ddrv,  allDiscreteIntVars, start); start += ddrv.length();
  copy_data_partial(ddsiv, allDiscreteIntVars, start); start += ddsiv.length();
  copy_data_partial(dauiv, allDiscreteIntVars, start); start += dauiv.length();
  copy_data_partial(deuiv, allDiscreteIntVars, start); start += deuiv.length();
  copy_data_partial(dsrv,  allDiscreteIntVars, start); start += dsrv.length();
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
  copy_data_partial(ddsrv, allDiscreteRealVars, start); start += ddsrv.length();
  copy_data_partial(daurv, allDiscreteRealVars, start); start += daurv.length();
  copy_data_partial(deurv, allDiscreteRealVars, start); start += deurv.length();
  copy_data_partial(dssrv, allDiscreteRealVars, start);

#ifdef REFCOUNT_DEBUG
  const std::pair<short,short>& view = sharedVarsData.view();
  Cout << "MixedVariables letter instantiated: view active = "
       << view.first << " inactive = " << view.second << endl;
#endif
}


void MixedVariables::read(std::istream& s)
{ read_core(s, GeneralReader(), ALL_VARS); }


void MixedVariables::write(std::ostream& s, unsigned short vars_part) const
{ write_core(s, GeneralWriter(), vars_part); }


void MixedVariables::write_aprepro(std::ostream& s) const
{ write_core(s, ApreproWriter(), ALL_VARS); }


/** Tabular reader that reads data in order design, aleatory,
    epistemic, state according to counts in vc_totals (extract in
    order: cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv,
    csv/dsiv/dsrv, which might reflect active or all depending on
    context. Assumes container sized, since might be a view into a
    larger array. */
void MixedVariables::read_tabular(std::istream& s, unsigned short vars_part)
{ read_core(s, TabularReader(), vars_part); }


void MixedVariables::
write_tabular(std::ostream& s, unsigned short vars_part) const
{ write_core(s, TabularWriter(), vars_part); }


void MixedVariables::
write_tabular_labels(std::ostream& s, unsigned short vars_part) const
{ write_core(s, LabelsWriter(), vars_part); }


/** Reordering is required in all read/write cases that will be
    visible to the user since all derived vars classes should use the
    same CDV/DDV/UV/CSV/DSV ordering for clarity.  Neutral file I/O,
    binary streams, and packed buffers do not need to reorder (so long
    as read/write are consistent) since this data is not intended for
    public consumption. */
template<typename Reader>
void MixedVariables::read_core(std::istream& s, Reader read_handler,
                               unsigned short vars_part)
{
  SizetArray vc_totals;
  size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;
  if (vars_part == ACTIVE_VARS) {
    vc_totals = sharedVarsData.active_components_totals();
    acv_offset = sharedVarsData.cv_start();
    adiv_offset = sharedVarsData.div_start();
    adsv_offset = sharedVarsData.dsv_start();
    adrv_offset = sharedVarsData.drv_start();
  }
  else if (vars_part == INACTIVE_VARS) {
    vc_totals = sharedVarsData.inactive_components_totals();
    acv_offset = sharedVarsData.icv_start();
    adiv_offset = sharedVarsData.idiv_start();
    adsv_offset = sharedVarsData.idsv_start();
    adrv_offset = sharedVarsData.idrv_start();  }
  else {
    // default: ALL_VARS, offsets start at 0;
    vc_totals = sharedVarsData.components_totals();
  }

  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  read_handler(s, acv_offset,  num_cdv,  allContinuousVars,     acv_labels);
  read_handler(s, adiv_offset, num_ddiv, allDiscreteIntVars,    adiv_labels);
  read_handler(s, adsv_offset, num_ddsv, allDiscreteStringVars, adsv_labels);
  read_handler(s, adrv_offset, num_ddrv, allDiscreteRealVars,   adrv_labels);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  read_handler(s, acv_offset,  num_cauv,  allContinuousVars,    acv_labels);
  read_handler(s, adiv_offset, num_dauiv, allDiscreteIntVars,   adiv_labels);
  read_handler(s, adsv_offset, num_dausv, allDiscreteStringVars, adsv_labels);
  read_handler(s, adrv_offset, num_daurv, allDiscreteRealVars,   adrv_labels);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  read_handler(s, acv_offset,  num_ceuv,  allContinuousVars,     acv_labels);
  read_handler(s, adiv_offset, num_deuiv, allDiscreteIntVars,    adiv_labels);
  read_handler(s, adsv_offset, num_deusv, allDiscreteStringVars, adsv_labels);
  read_handler(s, adrv_offset, num_deurv, allDiscreteRealVars,   adrv_labels);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  read_handler(s, acv_offset,  num_csv,  allContinuousVars,     acv_labels);
  read_handler(s, adiv_offset, num_dsiv, allDiscreteIntVars,    adiv_labels);
  read_handler(s, adsv_offset, num_dssv, allDiscreteStringVars, adsv_labels);
  read_handler(s, adrv_offset, num_dsrv, allDiscreteRealVars,   adrv_labels);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}


template<typename Writer>
void MixedVariables::write_core(std::ostream& s, Writer write_handler,
                                unsigned short vars_part) const
{
  SizetArray vc_totals;
  size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;
  if (vars_part == ACTIVE_VARS) {
    vc_totals = sharedVarsData.active_components_totals();
    acv_offset = sharedVarsData.cv_start();
    adiv_offset = sharedVarsData.div_start();
    adsv_offset = sharedVarsData.dsv_start();
    adrv_offset = sharedVarsData.drv_start();
  }
  else if (vars_part == INACTIVE_VARS) {
    vc_totals = sharedVarsData.inactive_components_totals();
    acv_offset = sharedVarsData.icv_start();
    adiv_offset = sharedVarsData.idiv_start();
    adsv_offset = sharedVarsData.idsv_start();
    adrv_offset = sharedVarsData.idrv_start();  }
  else {
    // default: ALL_VARS, offsets start at 0;
    vc_totals = sharedVarsData.components_totals();
  }

  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV];

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // write design variables
  write_handler(s, acv_offset,  num_cdv,  allContinuousVars,     acv_labels);
  write_handler(s, adiv_offset, num_ddiv, allDiscreteIntVars,    adiv_labels);
  write_handler(s, adsv_offset, num_ddsv, allDiscreteStringVars, adsv_labels);
  write_handler(s, adrv_offset, num_ddrv, allDiscreteRealVars,   adrv_labels);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  adsv_offset += num_ddsv; adrv_offset += num_ddrv;

  // write aleatory uncertain variables
  write_handler(s, acv_offset,  num_cauv,  allContinuousVars,     acv_labels);
  write_handler(s, adiv_offset, num_dauiv, allDiscreteIntVars,    adiv_labels);
  write_handler(s, adsv_offset, num_dausv, allDiscreteStringVars, adsv_labels);
  write_handler(s, adrv_offset, num_daurv, allDiscreteRealVars,   adrv_labels);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  adsv_offset += num_dausv; adrv_offset += num_daurv;

  // write epistemic uncertain variables
  write_handler(s, acv_offset,  num_ceuv,  allContinuousVars,     acv_labels);
  write_handler(s, adiv_offset, num_deuiv, allDiscreteIntVars,    adiv_labels);
  write_handler(s, adsv_offset, num_deusv, allDiscreteStringVars, adsv_labels);
  write_handler(s, adrv_offset, num_deurv, allDiscreteRealVars,   adrv_labels);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  adsv_offset += num_deusv; adrv_offset += num_deurv;

  // write state variables
  write_handler(s, acv_offset,  num_csv,  allContinuousVars,     acv_labels);
  write_handler(s, adiv_offset, num_dsiv, allDiscreteIntVars,    adiv_labels);
  write_handler(s, adsv_offset, num_dssv, allDiscreteStringVars, adsv_labels);
  write_handler(s, adrv_offset, num_dsrv, allDiscreteRealVars,   adrv_labels);
  //acv_offset  += num_csv;  adiv_offset += num_dsiv;
  //adsv_offset += num_dssv; adrv_offset += num_dsrv;
}

} // namespace Dakota
