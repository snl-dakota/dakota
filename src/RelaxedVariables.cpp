/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "RelaxedVariables.hpp"

#include <nlohmann/json.hpp>

#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"

static const char rcsId[] = "@(#) $Id";

using json = nlohmann::json;

namespace Dakota {

/** In this class, a relaxed data approach is used in which continuous
    and discrete arrays are combined into a single continuous array
    (integrality is relaxed; the converse of truncating reals is not
    currently supported but could be in the future if needed).
    Iterators/strategies which use this class include:
    BranchBndOptimizer.  Extract fundamental variable types and labels
    and merge continuous and discrete domains to create aggregate
    arrays and views.  */
RelaxedVariables::RelaxedVariables(const ProblemDescDB& problem_db,
                                   const ShortShortPair& view)
    : Variables(BaseConstructor(), problem_db, view) {
  const RealVector& cdv =
      problem_db.get_rv("variables.continuous_design.initial_point");
  const RealVector& cauv = problem_db.get_rv(
      "variables.continuous_aleatory_uncertain.initial_point");
  const RealVector& ceuv = problem_db.get_rv(
      "variables.continuous_epistemic_uncertain.initial_point");
  const RealVector& csv =
      problem_db.get_rv("variables.continuous_state.initial_state");

  const IntVector& ddrv =
      problem_db.get_iv("variables.discrete_design_range.initial_point");
  const IntVector& ddsiv =
      problem_db.get_iv("variables.discrete_design_set_int.initial_point");
  const IntVector& dauiv = problem_db.get_iv(
      "variables.discrete_aleatory_uncertain_int.initial_point");
  const IntVector& deuiv = problem_db.get_iv(
      "variables.discrete_epistemic_uncertain_int.initial_point");
  const IntVector& dsrv =
      problem_db.get_iv("variables.discrete_state_range.initial_state");
  const IntVector& dssiv =
      problem_db.get_iv("variables.discrete_state_set_int.initial_state");

  const StringArray& ddssv =
      problem_db.get_sa("variables.discrete_design_set_string.initial_point");
  const StringArray& dausv = problem_db.get_sa(
      "variables.discrete_aleatory_uncertain_string.initial_point");
  const StringArray& deusv = problem_db.get_sa(
      "variables.discrete_epistemic_uncertain_string.initial_point");
  const StringArray& dsssv =
      problem_db.get_sa("variables.discrete_state_set_string.initial_state");

  const RealVector& ddsrv =
      problem_db.get_rv("variables.discrete_design_set_real.initial_point");
  const RealVector& daurv = problem_db.get_rv(
      "variables.discrete_aleatory_uncertain_real.initial_point");
  const RealVector& deurv = problem_db.get_rv(
      "variables.discrete_epistemic_uncertain_real.initial_point");
  const RealVector& dssrv =
      problem_db.get_rv("variables.discrete_state_set_real.initial_state");

  size_t i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
            adsv_offset = 0, adrv_offset = 0, num_ddrv = ddrv.length(),
            num_ddsiv = ddsiv.length(), num_ddsrv = ddsrv.length(),
            num_dauiv = dauiv.length(), num_daurv = daurv.length(),
            num_deuiv = deuiv.length(), num_deurv = deurv.length(),
            num_dsrv = dsrv.length(), num_dssiv = dssiv.length(),
            num_dssrv = dssrv.length();
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();
  copy_data_partial(cdv, allContinuousVars, acv_offset);
  acv_offset += cdv.length();
  for (i = 0; i < num_ddrv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      allContinuousVars[acv_offset++] = (Real)ddrv[i];
    else
      allDiscreteIntVars[adiv_offset++] = ddrv[i];
  for (i = 0; i < num_ddsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      allContinuousVars[acv_offset++] = (Real)ddsiv[i];
    else
      allDiscreteIntVars[adiv_offset++] = ddsiv[i];
  copy_data_partial(ddssv, allDiscreteStringVars, adsv_offset);
  adsv_offset += ddssv.size();
  for (i = 0; i < num_ddsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      allContinuousVars[acv_offset++] = ddsrv[i];
    else
      allDiscreteRealVars[adrv_offset++] = ddsrv[i];

  copy_data_partial(cauv, allContinuousVars, acv_offset);
  acv_offset += cauv.length();
  for (i = 0; i < num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      allContinuousVars[acv_offset++] = (Real)dauiv[i];
    else
      allDiscreteIntVars[adiv_offset++] = dauiv[i];
  copy_data_partial(dausv, allDiscreteStringVars, adsv_offset);
  adsv_offset += dausv.size();
  for (i = 0; i < num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      allContinuousVars[acv_offset++] = daurv[i];
    else
      allDiscreteRealVars[adrv_offset++] = daurv[i];

  copy_data_partial(ceuv, allContinuousVars, acv_offset);
  acv_offset += ceuv.length();
  for (i = 0; i < num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      allContinuousVars[acv_offset++] = (Real)deuiv[i];
    else
      allDiscreteIntVars[adiv_offset++] = deuiv[i];
  copy_data_partial(deusv, allDiscreteStringVars, adsv_offset);
  adsv_offset += deusv.size();
  for (i = 0; i < num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      allContinuousVars[acv_offset++] = deurv[i];
    else
      allDiscreteRealVars[adrv_offset++] = deurv[i];

  copy_data_partial(csv, allContinuousVars, acv_offset);
  acv_offset += csv.length();
  for (i = 0; i < num_dsrv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      allContinuousVars[acv_offset++] = (Real)dsrv[i];
    else
      allDiscreteIntVars[adiv_offset++] = dsrv[i];
  for (i = 0; i < num_dssiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      allContinuousVars[acv_offset++] = (Real)dssiv[i];
    else
      allDiscreteIntVars[adiv_offset++] = dssiv[i];
  copy_data_partial(dsssv, allDiscreteStringVars, adsv_offset);
  adsv_offset += dsssv.size();
  for (i = 0; i < num_dssrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      allContinuousVars[acv_offset++] = dssrv[i];
    else
      allDiscreteRealVars[adrv_offset++] = dssrv[i];
}

void RelaxedVariables::read(std::istream& s) {
  read_core(s, GeneralReader(), ALL_VARS);
}

/** Presumes variables object is appropriately sized to receive data */
void RelaxedVariables::read_tabular(std::istream& s, unsigned short vars_part) {
  read_core(s, TabularReader(), vars_part);
}

void RelaxedVariables::write(std::ostream& s, unsigned short vars_part) const {
  write_core(s, GeneralWriter(), vars_part);
}

void RelaxedVariables::write_aprepro(std::ostream& s) const {
  write_core(s, ApreproWriter(), ALL_VARS);
}

// TEMP EMILIANO
void RelaxedVariables::write_json(json& s) const {
  write_core(s, JSONWriter(), ALL_VARS);
}

void RelaxedVariables::write_tabular(std::ostream& s,
                                     unsigned short vars_part) const {
  write_core(s, TabularWriter(), vars_part);
}

void RelaxedVariables::write_tabular_labels(std::ostream& s,
                                            unsigned short vars_part) const {
  write_core(s, LabelsWriter(), vars_part);
}

void RelaxedVariables::write_tabular_partial(
    std::ostream& s, size_t start_index,
    size_t num_items) const  //, unsigned short vars_part) const
{
  // assume ALL_VARS; don't consider vars_part for now

  const SizetArray& vc_totals = sharedVarsData.components_totals();  // ALL_VARS
  size_t end_index = start_index + num_items, av_cntr = 0, acv_offset = 0,
         adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CDV], vc_totals[TOTAL_DDIV],
                         vc_totals[TOTAL_DDSV], vc_totals[TOTAL_DDRV]))
    return;
  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CAUV], vc_totals[TOTAL_DAUIV],
                         vc_totals[TOTAL_DAUSV], vc_totals[TOTAL_DAURV]))
    return;
  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CEUV], vc_totals[TOTAL_DEUIV],
                         vc_totals[TOTAL_DEUSV], vc_totals[TOTAL_DEURV]))
    return;
  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CSV], vc_totals[TOTAL_DSIV],
                         vc_totals[TOTAL_DSSV], vc_totals[TOTAL_DSRV]))
    return;
}

void RelaxedVariables::write_tabular_partial_labels(std::ostream& s,
                                                    size_t start_index,
                                                    size_t num_items) const {
  // assume ALL_VARS; don't consider vars_part for now

  const SizetArray& vc_totals = sharedVarsData.components_totals();  // ALL_VARS
  size_t end_index = start_index + num_items, av_cntr = 0, acv_offset = 0,
         adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CDV], vc_totals[TOTAL_DDIV],
                         vc_totals[TOTAL_DDSV], vc_totals[TOTAL_DDRV]))
    return;
  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CAUV], vc_totals[TOTAL_DAUIV],
                         vc_totals[TOTAL_DAUSV], vc_totals[TOTAL_DAURV]))
    return;
  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CEUV], vc_totals[TOTAL_DEUIV],
                         vc_totals[TOTAL_DEUSV], vc_totals[TOTAL_DEURV]))
    return;
  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
                         adiv_offset, adsv_offset, adrv_offset, av_cntr,
                         vc_totals[TOTAL_CSV], vc_totals[TOTAL_DSIV],
                         vc_totals[TOTAL_DSSV], vc_totals[TOTAL_DSRV]))
    return;
}

/** Reordering is required in all read/write cases that will be
    visible to the user since all derived vars classes should use the
    same ordering for clarity.  Neutral file I/O, binary streams, and
    packed buffers do not need to reorder (so long as read/write are
    consistent) since this data is not intended for public
    consumption. */
template <typename Reader>
void RelaxedVariables::read_core(std::istream& s, Reader read_handler,
                                 unsigned short vars_part) {
  SizetArray vc_totals;
  size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;
  if (vars_part == ACTIVE_VARS) {
    vc_totals = sharedVarsData.active_components_totals();
    acv_offset = sharedVarsData.cv_start();
    adiv_offset = sharedVarsData.div_start();
    adsv_offset = sharedVarsData.dsv_start();
    adrv_offset = sharedVarsData.drv_start();
  } else if (vars_part == INACTIVE_VARS) {
    vc_totals = sharedVarsData.inactive_components_totals();
    acv_offset = sharedVarsData.icv_start();
    adiv_offset = sharedVarsData.idiv_start();
    adsv_offset = sharedVarsData.idsv_start();
    adrv_offset = sharedVarsData.idrv_start();
  } else {
    // default: ALL_VARS, offsets start at 0;
    vc_totals = sharedVarsData.components_totals();
  }

  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
         num_ddsv = vc_totals[TOTAL_DDSV], num_ddrv = vc_totals[TOTAL_DDRV],
         num_cauv = vc_totals[TOTAL_CAUV], num_dauiv = vc_totals[TOTAL_DAUIV],
         num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
         num_ceuv = vc_totals[TOTAL_CEUV], num_deuiv = vc_totals[TOTAL_DEUIV],
         num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
         num_csv = vc_totals[TOTAL_CSV], num_dsiv = vc_totals[TOTAL_DSIV],
         num_dssv = vc_totals[TOTAL_DSSV], num_dsrv = vc_totals[TOTAL_DSRV], i,
         len = 1, ardi_cntr = 0, ardr_cntr = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  read_handler(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i = 0; i < num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_ddsv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_ddsv;
  for (i = 0; i < num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  read_handler(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i = 0; i < num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_dausv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_dausv;
  for (i = 0; i < num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  read_handler(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i = 0; i < num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_deusv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_deusv;
  for (i = 0; i < num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // state
  read_handler(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i = 0; i < num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_dssv, allDiscreteStringVars, adsv_labels);
  // adsv_offset += num_dssv;
  for (i = 0; i < num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);
}

template <typename Writer, typename Stream>
void RelaxedVariables::write_core(Stream& s, Writer write_handler,
                                  unsigned short vars_part) const {
  SizetArray vc_totals;
  size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;
  if (vars_part == ACTIVE_VARS) {
    vc_totals = sharedVarsData.active_components_totals();
    acv_offset = sharedVarsData.cv_start();
    adiv_offset = sharedVarsData.div_start();
    adsv_offset = sharedVarsData.dsv_start();
    adrv_offset = sharedVarsData.drv_start();
  } else if (vars_part == INACTIVE_VARS) {
    vc_totals = sharedVarsData.inactive_components_totals();
    acv_offset = sharedVarsData.icv_start();
    adiv_offset = sharedVarsData.idiv_start();
    adsv_offset = sharedVarsData.idsv_start();
    adrv_offset = sharedVarsData.idrv_start();
  } else {
    // default: ALL_VARS, offsets start at 0;
    vc_totals = sharedVarsData.components_totals();
  }

  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
         num_ddsv = vc_totals[TOTAL_DDSV], num_ddrv = vc_totals[TOTAL_DDRV],
         num_cauv = vc_totals[TOTAL_CAUV], num_dauiv = vc_totals[TOTAL_DAUIV],
         num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
         num_ceuv = vc_totals[TOTAL_CEUV], num_deuiv = vc_totals[TOTAL_DEUIV],
         num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
         num_csv = vc_totals[TOTAL_CSV], num_dsiv = vc_totals[TOTAL_DSIV],
         num_dssv = vc_totals[TOTAL_DSSV], num_dsrv = vc_totals[TOTAL_DSRV], i,
         len = 1, ardi_cntr = 0, ardr_cntr = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  write_handler(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i = 0; i < num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_ddsv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_ddsv;
  for (i = 0; i < num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  write_handler(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i = 0; i < num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_dausv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_dausv;
  for (i = 0; i < num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  write_handler(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i = 0; i < num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_deusv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_deusv;
  for (i = 0; i < num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // state
  write_handler(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i = 0; i < num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_dssv, allDiscreteStringVars, adsv_labels);
  // adsv_offset += num_dssv;
  for (i = 0; i < num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s, acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);
}

template <typename Writer, typename Stream>
bool RelaxedVariables::write_partial_core(
    Stream& s, Writer write_handler, size_t start_index, size_t end_index,
    size_t& acv_offset, size_t& adiv_offset, size_t& adsv_offset,
    size_t& adrv_offset, size_t& av_cntr, size_t num_cv, size_t num_div,
    size_t num_dsv, size_t num_drv) const {
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();
  size_t i, ardi_cntr = 0, ardr_cntr = 0;

  // write continuous variables
  StringMultiArrayView acv_labels = all_continuous_variable_labels();
  for (i = 0; i < num_cv; ++i, ++av_cntr, ++acv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
    else if (av_cntr >= end_index)
      return true;

  // write discrete int variables (potentially relaxed)
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  for (i = 0; i < num_div; ++i, ++ardi_cntr, ++av_cntr) {
    bool relax = all_relax_di[ardi_cntr];
    if (av_cntr >= start_index && av_cntr < end_index) {
      if (relax)
        write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
      else
        write_handler(s, adiv_offset, 1, allDiscreteIntVars, adiv_labels);
    } else if (av_cntr >= end_index)
      return true;
    if (relax)
      ++acv_offset;
    else
      ++adiv_offset;
  }

  // write discrete string variables
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  for (i = 0; i < num_dsv; ++i, ++av_cntr, ++adsv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, adsv_offset, 1, allDiscreteStringVars, adsv_labels);
    else if (av_cntr >= end_index)
      return true;

  // write discrete real variables (potentially relaxed)
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();
  for (i = 0; i < num_drv; ++i, ++ardr_cntr, ++av_cntr) {
    bool relax = all_relax_dr[ardr_cntr];
    if (av_cntr >= start_index && av_cntr < end_index) {
      if (relax)
        write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
      else
        write_handler(s, adrv_offset, 1, allDiscreteRealVars, adrv_labels);
    } else if (av_cntr >= end_index)
      return true;
    if (relax)
      ++acv_offset;
    else
      ++adrv_offset;
  }

  return false;
}

}  // namespace Dakota
