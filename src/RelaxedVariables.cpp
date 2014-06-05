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


// Reordering is required in all read/write cases that will be visible to the
// user since all derived vars classes should use the same ordering for clarity.
// Neutral file I/O, binary streams, and packed buffers do not need to reorder
// (so long as read/write are consistent) since this data is not intended for
// public consumption.
void RelaxedVariables::read(std::istream& s)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  read_data_partial(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  read_data_partial(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_dausv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  read_data_partial(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_deusv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // state
  read_data_partial(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      read_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  read_data_partial(s, adsv_offset, num_dssv, allDiscreteStringVars,
		    adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      read_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);
}


void RelaxedVariables::write(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  write_data_partial(s, acv_offset, num_cdv, allContinuousVars, acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // aleatory uncertain
  write_data_partial(s, acv_offset, num_cauv, allContinuousVars, acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_dausv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // epistemic uncertain
  write_data_partial(s, acv_offset, num_ceuv, allContinuousVars, acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_deusv, allDiscreteStringVars,
		    adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);

  // state
  write_data_partial(s, acv_offset, num_csv, allContinuousVars, acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, 1,  allContinuousVars, acv_labels);
    else
      write_data_partial(s, adiv_offset++, 1, allDiscreteIntVars, adiv_labels);
  write_data_partial(s, adsv_offset, num_dssv, allDiscreteStringVars,
		    adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, 1,   allContinuousVars, acv_labels);
    else
      write_data_partial(s, adrv_offset++, 1, allDiscreteRealVars, adrv_labels);
}


void RelaxedVariables::write_aprepro(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  StringMultiArrayView  acv_labels = all_continuous_variable_labels();
  StringMultiArrayView adiv_labels = all_discrete_int_variable_labels();
  StringMultiArrayView adsv_labels = all_discrete_string_variable_labels();
  StringMultiArrayView adrv_labels = all_discrete_real_variable_labels();

  // design
  write_data_partial_aprepro(s, acv_offset, num_cdv, allContinuousVars,
			     acv_labels);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_ddsv, allDiscreteStringVars,
			     adsv_labels);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);

  // aleatory uncertain
  write_data_partial_aprepro(s, acv_offset, num_cauv, allContinuousVars,
			     acv_labels);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_dausv, allDiscreteStringVars,
			     adsv_labels);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);

  // epistemic uncertain
  write_data_partial_aprepro(s, acv_offset, num_ceuv, allContinuousVars,
			     acv_labels);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_deusv, allDiscreteStringVars,
			     adsv_labels);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);

  // state
  write_data_partial_aprepro(s, acv_offset, num_csv, allContinuousVars,
			     acv_labels);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,  allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adiv_offset++, 1, allDiscreteIntVars,
				 adiv_labels);
  write_data_partial_aprepro(s, adsv_offset, num_dssv, allDiscreteStringVars,
			     adsv_labels);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_aprepro(s,  acv_offset++, 1,   allContinuousVars,
				 acv_labels);
    else
      write_data_partial_aprepro(s, adrv_offset++, 1, allDiscreteRealVars,
				 adrv_labels);
}


/** Presumes variables object is appropriately sized to receive data */
void RelaxedVariables::read_tabular(std::istream& s)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // design
  read_data_partial_tabular(s, acv_offset, num_cdv, allContinuousVars);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_ddsv, allDiscreteStringVars);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // aleatory uncertain
  read_data_partial_tabular(s, acv_offset, num_cauv, allContinuousVars);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_dausv, allDiscreteStringVars);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // epistemic uncertain
  read_data_partial_tabular(s, acv_offset, num_ceuv, allContinuousVars);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_deusv, allDiscreteStringVars);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // state
  read_data_partial_tabular(s, acv_offset, num_csv, allContinuousVars);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      read_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  read_data_partial_tabular(s, adsv_offset, num_dssv, allDiscreteStringVars);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      read_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);
}


void RelaxedVariables::write_tabular(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    num_ddsv  = vc_totals[TOTAL_DDSV],  num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    num_dausv = vc_totals[TOTAL_DAUSV], num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    num_deusv = vc_totals[TOTAL_DEUSV], num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    num_dssv  = vc_totals[TOTAL_DSSV],  num_dsrv  = vc_totals[TOTAL_DSRV],
    i, ardi_cntr = 0, ardr_cntr = 0, acv_offset = 0, adiv_offset = 0,
    adsv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // design
  write_data_partial_tabular(s, acv_offset, num_cdv, allContinuousVars);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_ddsv, allDiscreteStringVars);
  adsv_offset += num_ddsv;
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // aleatory uncertain
  write_data_partial_tabular(s, acv_offset, num_cauv, allContinuousVars);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_dausv, allDiscreteStringVars);
  adsv_offset += num_dausv;
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // epistemic uncertain
  write_data_partial_tabular(s, acv_offset, num_ceuv, allContinuousVars);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_deusv, allDiscreteStringVars);
  adsv_offset += num_deusv;
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);

  // state
  write_data_partial_tabular(s, acv_offset, num_csv, allContinuousVars);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,  allContinuousVars);
    else
      write_data_partial_tabular(s, adiv_offset++, 1, allDiscreteIntVars);
  write_data_partial_tabular(s, adsv_offset, num_dssv, allDiscreteStringVars);
  //adsv_offset += num_dssv;
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial_tabular(s,  acv_offset++, 1,   allContinuousVars);
    else
      write_data_partial_tabular(s, adrv_offset++, 1, allDiscreteRealVars);
}

} // namespace Dakota
