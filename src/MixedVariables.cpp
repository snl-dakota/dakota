/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <nlohmann/json.hpp>
#include "MixedVariables.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"
#include "dakota_tabular_io.hpp"

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

} // anonymous namespace

/** In this class, the distinct approach is used (design, uncertain, and
    state variable types and continuous and discrete domain types are 
    distinct).  Most iterators/strategies use this approach. */
MixedVariables::
MixedVariables(const ProblemDescDB& problem_db, const ShortShortPair& view):
  Variables(BaseConstructor(), problem_db, view)
{
 size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;

  // --- Continuous variables ---
  copy_from_db(problem_db, {
    "variables.continuous_design.initial_point",
    // continuous aleatory uncertain
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
    "variables.histogram_bin_uncertain.initial_point",
    // continuous epistemic uncertain
    "variables.continuous_interval_uncertain.initial_point",
    // continuous state
    "variables.continuous_state.initial_state"
  }, allContinuousVars, acv_offset, get_rv);

  std::cout << "MixedVariables, after writing allContinuousVars.\n";
  for(int i = 0; i < len(allContinuousVars); ++i)
    std::cout << allContinuousVars[i] << std::endl;
  std::cout << "MixedVariables, done writing\n";


  // --- Discrete integer variables ---
  copy_from_db(problem_db, {
    "variables.discrete_design_range.initial_point",
    "variables.discrete_design_set_int.initial_point",
    // discrete aleatory uncertain int
    "variables.poisson_uncertain.initial_point",
    "variables.binomial_uncertain.initial_point",
    "variables.negative_binomial_uncertain.initial_point",
    "variables.geometric_uncertain.initial_point",
    "variables.hypergeometric_uncertain.initial_point",
    "variables.histogram_uncertain.point_int.initial_point",
    // discrete epistemic uncertain int
    "variables.discrete_interval_uncertain.initial_point",
    "variables.discrete_uncertain_set_int.initial_point",
    // discrete state int
    "variables.discrete_state_range.initial_state",
    "variables.discrete_state_set_int.initial_state"
  }, allDiscreteIntVars, adiv_offset, get_iv);

  // --- Discrete string variables ---
  copy_from_db(problem_db, {
    "variables.discrete_design_set_string.initial_point",
    // discrete aleatory uncertain string
    "variables.histogram_uncertain.point_string.initial_point",
    // discrete epistemic uncertain string
    "variables.discrete_uncertain_set_string.initial_point",
    // discrete state string
    "variables.discrete_state_set_string.initial_state"
  }, allDiscreteStringVars, adsv_offset, get_sa);

  // --- Discrete real variables ---
  copy_from_db(problem_db, {
    "variables.discrete_design_set_real.initial_point",
    // discrete aleatory uncertain real
    "variables.histogram_uncertain.point_real.initial_point",
    // discrete epistemic uncertain real
    "variables.discrete_uncertain_set_real.initial_point",
    // discrete state real
    "variables.discrete_state_set_real.initial_state"
  }, allDiscreteRealVars, adrv_offset, get_rv);
}


void MixedVariables::read(std::istream& s)
{ read_core(s, GeneralReader(), ALL_VARS); }


/** Tabular reader that reads data in order design, aleatory,
    epistemic, state according to counts in vc_totals (extract in
    order: cdv/ddiv/ddrv, cauv/dauiv/daurv, ceuv/deuiv/deurv,
    csv/dsiv/dsrv, which might reflect active or all depending on
    context. Assumes container sized, since might be a view into a
    larger array. */
void MixedVariables::read_tabular(std::istream& s, unsigned short vars_part)
{ read_core(s, TabularReader(), vars_part); }


void MixedVariables::write(std::ostream& s, unsigned short vars_part) const
{ write_core(s, GeneralWriter(), vars_part); }


void MixedVariables::write_aprepro(std::ostream& s) const
{ write_core(s, ApreproWriter(), ALL_VARS); }


void MixedVariables::write_json(json& s) const
{ write_core(s, JSONWriter(), ALL_VARS); }

void MixedVariables::
write_tabular(std::ostream& s, unsigned short vars_part) const
{ write_core(s, TabularWriter(), vars_part); }


void MixedVariables::
write_tabular_labels(std::ostream& s, unsigned short vars_part) const
{ write_core(s, LabelsWriter(), vars_part); }


void MixedVariables::
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


void MixedVariables::
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


template<typename Writer, typename Stream>
void MixedVariables::write_core(Stream& s, Writer write_handler,
                                unsigned short vars_part) const
{
  SizetArray vc_totals;
  size_t acv_offset = 0, adiv_offset = 0, adsv_offset = 0, adrv_offset = 0;
  if (vars_part == ACTIVE_VARS) {
    vc_totals   = sharedVarsData.active_components_totals();
    acv_offset  = sharedVarsData.cv_start();
    adiv_offset = sharedVarsData.div_start();
    adsv_offset = sharedVarsData.dsv_start();
    adrv_offset = sharedVarsData.drv_start();
  }
  else if (vars_part == INACTIVE_VARS) {
    vc_totals   = sharedVarsData.inactive_components_totals();
    acv_offset  = sharedVarsData.icv_start();
    adiv_offset = sharedVarsData.idiv_start();
    adsv_offset = sharedVarsData.idsv_start();
    adrv_offset = sharedVarsData.idrv_start();  }
  else // default: ALL_VARS, offsets start at 0
    vc_totals = sharedVarsData.components_totals();

  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv  = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],   num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],   num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV],  num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],   num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV],  num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],    num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],   num_dsrv  = vc_totals[TOTAL_DSRV];

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


template<typename Writer, typename Stream>
bool MixedVariables::
write_partial_core(Stream& s, Writer write_handler, size_t start_index,
		   size_t end_index, size_t& acv_offset, size_t& adiv_offset,
		   size_t& adsv_offset, size_t& adrv_offset, size_t& av_cntr,
		   size_t num_cv, size_t num_div, size_t num_dsv,
		   size_t num_drv) const
{
  size_t i;
  StringMultiArrayView acv_labels = all_continuous_variable_labels();
  for (i=0; i<num_cv; ++i, ++av_cntr, ++acv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, acv_offset, 1, allContinuousVars, acv_labels);
    else if (av_cntr >= end_index)
      return true;

  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  for (i=0; i<num_div; ++i, ++av_cntr, ++adiv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, adiv_offset, 1, allDiscreteIntVars, adiv_labels);
    else if (av_cntr >= end_index)
      return true;

  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  for (i=0; i<num_dsv; ++i, ++av_cntr, ++adsv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, adsv_offset, 1, allDiscreteStringVars, adsv_labels);
    else if (av_cntr >= end_index)
      return true;

  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();
  for (i=0; i<num_drv; ++i, ++av_cntr, ++adrv_offset)
    if (av_cntr >= start_index && av_cntr < end_index)
      write_handler(s, adrv_offset, 1, allDiscreteRealVars, adrv_labels);
    else if (av_cntr >= end_index)
      return true;

  return false;
}

} // namespace Dakota
