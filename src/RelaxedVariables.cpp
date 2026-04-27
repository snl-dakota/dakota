/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <nlohmann/json.hpp>
#include "RelaxedVariables.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"

static const char rcsId[]="@(#) $Id";

using json = nlohmann::json;





namespace Dakota {

namespace {

// helpers for constructor to get information from ProblemDescDB 
// and insert it into member variables

template <typename T>
int len(const T& v) { return v.length(); }
int len(const int& v) { return 1; }
int len(const StringArray& v) { return v.size(); }

const RealVector& get_rv(const ProblemDescDB& db, const char* key) {
  return db.get_rv(key);
}
const IntVector& get_iv(const ProblemDescDB& db, const char* key) {
  return db.get_iv(key);
}
const StringArray& get_sa(const ProblemDescDB& db, const char* key) {
  return db.get_sa(key);
}

template <typename Vec, typename Getter>
void copy_from_db(const ProblemDescDB& db,
    const std::initializer_list<const char*>& keys,
    Vec& dest, size_t& offset, Getter get)
{
  for (const auto& key : keys) {
    const auto& vals = get(db, key);
    copy_data_partial(vals, dest, offset);
    offset += len(vals);
  }
}

template <typename DiscVec, typename Getter>
void relax_from_db(const ProblemDescDB& db,
    const std::initializer_list<const char*>& keys,
    const BitArray& relax, size_t& relax_cntr,
    RealVector& cont, size_t& acv_offset,
    DiscVec& disc, size_t& disc_offset,
    Getter get)
{
  for (const auto& key : keys) {
    const auto& vals = get(db, key);
    for (int i = 0; i < vals.length(); ++i, ++relax_cntr)
      if (relax[relax_cntr])
        cont[acv_offset++] = (Real)vals[i];
      else
        disc[disc_offset++] = vals[i];
  }
}

} // anonymous namespace



/** In this class, a relaxed data approach is used in which continuous
    and discrete arrays are combined into a single continuous array
    (integrality is relaxed; the converse of truncating reals is not
    currently supported but could be in the future if needed).
    Iterators/strategies which use this class include:
    BranchBndOptimizer.  Extract fundamental variable types and labels
    and merge continuous and discrete domains to create aggregate
    arrays and views.  */
RelaxedVariables::
RelaxedVariables(const ProblemDescDB& problem_db, const ShortShortPair& view):
  Variables(BaseConstructor(), problem_db, view)
{
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0,
         ardi_cntr = 0, ardr_cntr = 0;

  // --- Design ---
  copy_from_db(problem_db, {
    "variables.continuous_design.initial_point"
  }, allContinuousVars, acv_offset, get_rv);

  relax_from_db(problem_db, {
    "variables.discrete_design_range.initial_point",
    "variables.discrete_design_set_int.initial_point"
  }, all_relax_di, ardi_cntr,
     allContinuousVars, acv_offset, allDiscreteIntVars, adiv_offset, get_iv);

  copy_from_db(problem_db, {
    "variables.discrete_design_set_string.initial_point"
  }, allDiscreteStringVars, adsv_offset, get_sa);

  relax_from_db(problem_db, {
    "variables.discrete_design_set_real.initial_point"
  }, all_relax_dr, ardr_cntr,
     allContinuousVars, acv_offset, allDiscreteRealVars, adrv_offset, get_rv);

  // --- Aleatory ---
  copy_from_db(problem_db, {
    "variables.normal_uncertain.initial_point",
    "variables.lognormal_uncertain.initial_point",
    "variables.uniform_uncertain.initial_point",
    "variables.loguniform_uncertain.initial_point",
    "variables.triangular_uncertain.initial_point",
    "variables.exponential_uncertain.initial_point",
    "variables.beta_uncertain.initial_point",
    "variables.gamma_uncertain.initial_point",
    "variables.gumbel_uncertain.initial_point",
    "variables.frechet_uncertain.initial_point",
    "variables.weibull_uncertain.initial_point",
    "variables.histogram_bin_uncertain.initial_point"
  }, allContinuousVars, acv_offset, get_rv);

  relax_from_db(problem_db, {
    "variables.poisson_uncertain.initial_point",
    "variables.binomial_uncertain.initial_point",
    "variables.negative_binomial_uncertain.initial_point",
    "variables.geometric_uncertain.initial_point",
    "variables.hypergeometric_uncertain.initial_point",
    "variables.histogram_uncertain.point_int.initial_point"
  }, all_relax_di, ardi_cntr,
     allContinuousVars, acv_offset, allDiscreteIntVars, adiv_offset, get_iv);

  copy_from_db(problem_db, {
    "variables.histogram_uncertain.point_string.initial_point"
  }, allDiscreteStringVars, adsv_offset, get_sa);

  relax_from_db(problem_db, {
    "variables.histogram_uncertain.point_real.initial_point"
  }, all_relax_dr, ardr_cntr,
     allContinuousVars, acv_offset, allDiscreteRealVars, adrv_offset, get_rv);

  // --- Epistemic ---
  copy_from_db(problem_db, {
    "variables.continuous_interval_uncertain.initial_point"
  }, allContinuousVars, acv_offset, get_rv);

  relax_from_db(problem_db, {
    "variables.discrete_interval_uncertain.initial_point",
    "variables.discrete_uncertain_set_int.initial_point"
  }, all_relax_di, ardi_cntr,
     allContinuousVars, acv_offset, allDiscreteIntVars, adiv_offset, get_iv);

  copy_from_db(problem_db, {
    "variables.discrete_uncertain_set_string.initial_point"
  }, allDiscreteStringVars, adsv_offset, get_sa);

  relax_from_db(problem_db, {
    "variables.discrete_uncertain_set_real.initial_point"
  }, all_relax_dr, ardr_cntr,
     allContinuousVars, acv_offset, allDiscreteRealVars, adrv_offset, get_rv);

  // --- State ---
  copy_from_db(problem_db, {
    "variables.continuous_state.initial_state"
  }, allContinuousVars, acv_offset, get_rv);

  relax_from_db(problem_db, {
    "variables.discrete_state_range.initial_state",
    "variables.discrete_state_set_int.initial_state"
  }, all_relax_di, ardi_cntr,
     allContinuousVars, acv_offset, allDiscreteIntVars, adiv_offset, get_iv);

  copy_from_db(problem_db, {
    "variables.discrete_state_set_string.initial_state"
  }, allDiscreteStringVars, adsv_offset, get_sa);

  relax_from_db(problem_db, {
    "variables.discrete_state_set_real.initial_state"
  }, all_relax_dr, ardr_cntr,
     allContinuousVars, acv_offset, allDiscreteRealVars, adrv_offset, get_rv);
}


void RelaxedVariables::read(std::istream& s)
{ read_core(s, GeneralReader(), ALL_VARS); }


/** Presumes variables object is appropriately sized to receive data */
void RelaxedVariables::read_tabular(std::istream& s, unsigned short vars_part)
{ read_core(s, TabularReader(), vars_part); }


void RelaxedVariables::write(std::ostream& s, unsigned short vars_part) const
{ write_core(s, GeneralWriter(), vars_part); }


void RelaxedVariables::write_aprepro(std::ostream& s) const
{ write_core(s, ApreproWriter(), ALL_VARS); }

//TEMP EMILIANO
void RelaxedVariables::write_json(json& s) const
{ write_core(s, JSONWriter(), ALL_VARS); }

void RelaxedVariables::
write_tabular(std::ostream& s, unsigned short vars_part) const
{ write_core(s, TabularWriter(), vars_part); }


void RelaxedVariables::
write_tabular_labels(std::ostream& s, unsigned short vars_part) const
{ write_core(s, LabelsWriter(), vars_part); }


void RelaxedVariables::
write_tabular_partial(std::ostream& s, size_t start_index,
		      size_t num_items) const//, unsigned short vars_part) const
{
  // assume ALL_VARS; don't consider vars_part for now

  const SizetArray& vc_totals = sharedVarsData.components_totals(); // ALL_VARS
  size_t end_index = start_index + num_items, av_cntr = 0,
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CDV],  vc_totals[TOTAL_DDIV],
			 vc_totals[TOTAL_DDSV], vc_totals[TOTAL_DDRV]))
    return;
  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CAUV],  vc_totals[TOTAL_DAUIV],
			 vc_totals[TOTAL_DAUSV], vc_totals[TOTAL_DAURV]))
    return;
  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CEUV],  vc_totals[TOTAL_DEUIV],
			 vc_totals[TOTAL_DEUSV], vc_totals[TOTAL_DEURV]))
    return;
  if (write_partial_core(s, TabularWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CSV],  vc_totals[TOTAL_DSIV],
			 vc_totals[TOTAL_DSSV], vc_totals[TOTAL_DSRV]))
    return;
}


void RelaxedVariables::
write_tabular_partial_labels(std::ostream& s, size_t start_index,
			     size_t num_items) const
{
  // assume ALL_VARS; don't consider vars_part for now

  const SizetArray& vc_totals = sharedVarsData.components_totals(); // ALL_VARS
  size_t end_index = start_index + num_items, av_cntr = 0,
    acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CDV],  vc_totals[TOTAL_DDIV],
			 vc_totals[TOTAL_DDSV], vc_totals[TOTAL_DDRV]))
    return;
  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CAUV],  vc_totals[TOTAL_DAUIV],
			 vc_totals[TOTAL_DAUSV], vc_totals[TOTAL_DAURV]))
    return;
  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CEUV],  vc_totals[TOTAL_DEUIV],
			 vc_totals[TOTAL_DEUSV], vc_totals[TOTAL_DEURV]))
    return;
  if (write_partial_core(s, LabelsWriter(), start_index, end_index, acv_offset,
			 adiv_offset, adsv_offset, adrv_offset, av_cntr,
			 vc_totals[TOTAL_CSV],  vc_totals[TOTAL_DSIV],
			 vc_totals[TOTAL_DSSV], vc_totals[TOTAL_DSRV]))
    return;
}


/** Reordering is required in all read/write cases that will be
    visible to the user since all derived vars classes should use the
    same ordering for clarity.  Neutral file I/O, binary streams, and
    packed buffers do not need to reorder (so long as read/write are
    consistent) since this data is not intended for public
    consumption. */
template<typename Reader>
void RelaxedVariables::read_core(std::istream& s, Reader read_handler,
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
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, len = 1, ardi_cntr = 0, ardr_cntr = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  read_handler(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s,  acv_offset++, len,  allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_ddsv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s,  acv_offset++, len,   allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  read_handler(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s,  acv_offset++, len,  allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_dausv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s,  acv_offset++, len,   allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  read_handler(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s,  acv_offset++, len,  allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_deusv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s,  acv_offset++, len,   allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // state
  read_handler(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_handler(s,  acv_offset++, len,  allContinuousVars, acv_labels);
    else
      read_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  read_handler(s, adsv_offset, num_dssv, allDiscreteStringVars, adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_handler(s,  acv_offset++, len,   allContinuousVars, acv_labels);
    else
      read_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);
}

template<typename Writer, typename Stream>
void RelaxedVariables::write_core(Stream& s, Writer write_handler, 
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
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, len = 1, ardi_cntr = 0, ardr_cntr = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  write_handler(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_ddsv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  write_handler(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_dausv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  write_handler(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_deusv, allDiscreteStringVars, adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);

  // state
  write_handler(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adiv_offset++, len, allDiscreteIntVars, adiv_labels);
  write_handler(s, adsv_offset, num_dssv, allDiscreteStringVars, adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_handler(s,  acv_offset++, len, allContinuousVars, acv_labels);
    else
      write_handler(s, adrv_offset++, len, allDiscreteRealVars, adrv_labels);
}


template<typename Writer, typename Stream>
bool RelaxedVariables::
write_partial_core(Stream& s, Writer write_handler, size_t start_index,
		   size_t end_index, size_t& acv_offset, size_t& adiv_offset,
		   size_t& adsv_offset, size_t& adrv_offset, size_t& av_cntr,
		   size_t num_cv, size_t num_div, size_t num_dsv,
		   size_t num_drv) const
{
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();
  size_t i, ardi_cntr = 0, ardr_cntr = 0;

  // write continuous variables
  StringMultiArrayView acv_labels = all_continuous_variable_labels();
  for (i=0; i<num_cv; ++i, ++av_cntr, ++acv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
    else if (av_cntr >= end_index)
      return true;

  // write discrete int variables (potentially relaxed)
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  for (i=0; i<num_div; ++i, ++ardi_cntr, ++av_cntr) {
    bool relax = all_relax_di[ardi_cntr];
    if (av_cntr >= start_index && av_cntr < end_index) {
      if (relax) write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
      else write_handler(s, adiv_offset, 1, allDiscreteIntVars, adiv_labels);
    }
    else if (av_cntr >= end_index)
      return true;
    if (relax) ++acv_offset;
    else      ++adiv_offset;
  }

  // write discrete string variables
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  for (i=0; i<num_dsv; ++i, ++av_cntr, ++adsv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, adsv_offset, 1, allDiscreteStringVars, adsv_labels);
    else if (av_cntr >= end_index)
      return true;

  // write discrete real variables (potentially relaxed)
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();
  for (i=0; i<num_drv; ++i, ++ardr_cntr, ++av_cntr) {
    bool relax = all_relax_dr[ardr_cntr];
    if (av_cntr >= start_index && av_cntr < end_index) {
      if (relax) write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
      else write_handler(s, adrv_offset, 1, allDiscreteRealVars, adrv_labels);
    }
    else if (av_cntr >= end_index)
      return true;
    if (relax) ++acv_offset;
    else      ++adrv_offset;
  }

  return false;
}

} // namespace Dakota
