/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        RelaxedVarConstraints
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "RelaxedVarConstraints.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"

static const char rcsId[]="@(#) $Id";


namespace Dakota {

/** In this class, a relaxed data approach is used in which continuous
    and discrete arrays are combined into a single continuous array
    (integrality is relaxed; the converse of truncating reals is not
    currently supported but could be in the future if needed).
    Iterators which use this class include: BranchBndOptimizer. */
RelaxedVarConstraints::
RelaxedVarConstraints(const ProblemDescDB& problem_db,
		      const SharedVariablesData& svd):
  Constraints(BaseConstructor(), problem_db, svd)
{
  const RealVector& cdv_l_bnds = problem_db.get_rv(
    "variables.continuous_design.lower_bounds");
  const RealVector& cdv_u_bnds = problem_db.get_rv(
    "variables.continuous_design.upper_bounds");
  const RealVector& cauv_l_bnds = problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.lower_bounds");
  const RealVector& cauv_u_bnds = problem_db.get_rv(
    "variables.continuous_aleatory_uncertain.upper_bounds");
  const RealVector& ceuv_l_bnds = problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.lower_bounds");
  const RealVector& ceuv_u_bnds = problem_db.get_rv(
    "variables.continuous_epistemic_uncertain.upper_bounds");
  const RealVector& csv_l_bnds  = problem_db.get_rv(
    "variables.continuous_state.lower_bounds");
  const RealVector& csv_u_bnds  = problem_db.get_rv(
    "variables.continuous_state.upper_bounds");

  const IntVector& ddrv_l_bnds = problem_db.get_iv(
    "variables.discrete_design_range.lower_bounds");
  const IntVector& ddrv_u_bnds = problem_db.get_iv(
    "variables.discrete_design_range.upper_bounds");
  const IntVector& ddsiv_l_bnds = problem_db.get_iv(
    "variables.discrete_design_set_int.lower_bounds");
  const IntVector& ddsiv_u_bnds = problem_db.get_iv(
    "variables.discrete_design_set_int.upper_bounds");
  const IntVector& dauiv_l_bnds = problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.lower_bounds");
  const IntVector& dauiv_u_bnds = problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.upper_bounds");
  const IntVector& deuiv_l_bnds = problem_db.get_iv(
    "variables.discrete_epistemic_uncertain_int.lower_bounds");
  const IntVector& deuiv_u_bnds = problem_db.get_iv(
    "variables.discrete_epistemic_uncertain_int.upper_bounds");
  const IntVector& dsrv_l_bnds = problem_db.get_iv(
    "variables.discrete_state_range.lower_bounds");
  const IntVector& dsrv_u_bnds = problem_db.get_iv(
    "variables.discrete_state_range.upper_bounds");
  const IntVector& dssiv_l_bnds = problem_db.get_iv(
    "variables.discrete_state_set_int.lower_bounds");
  const IntVector& dssiv_u_bnds = problem_db.get_iv(
    "variables.discrete_state_set_int.upper_bounds");

  // discrete string variable bounds?  (not needed for exclusively categorical)

  const RealVector& ddsrv_l_bnds = problem_db.get_rv(
    "variables.discrete_design_set_real.lower_bounds");
  const RealVector& ddsrv_u_bnds = problem_db.get_rv(
    "variables.discrete_design_set_real.upper_bounds");
  const RealVector& daurv_l_bnds = problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.lower_bounds");
  const RealVector& daurv_u_bnds = problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.upper_bounds");
  const RealVector& deurv_l_bnds = problem_db.get_rv(
    "variables.discrete_epistemic_uncertain_real.lower_bounds");
  const RealVector& deurv_u_bnds = problem_db.get_rv(
    "variables.discrete_epistemic_uncertain_real.upper_bounds");
  const RealVector& dssrv_l_bnds = problem_db.get_rv(
    "variables.discrete_state_set_real.lower_bounds");
  const RealVector& dssrv_u_bnds = problem_db.get_rv(
    "variables.discrete_state_set_real.upper_bounds");

  size_t i, ardi_cntr = 0, ardr_cntr = 0,
    acv_offset = 0, adiv_offset = 0, adrv_offset = 0,
    num_ddrv  = ddrv_l_bnds.length(),  num_ddsiv = ddsiv_l_bnds.length(),
    num_ddsrv = ddsrv_l_bnds.length(), num_dauiv = dauiv_l_bnds.length(),
    num_daurv = daurv_l_bnds.length(), num_deuiv = deuiv_l_bnds.length(),
    num_deurv = deurv_l_bnds.length(), num_dsrv  = dsrv_l_bnds.length(),
    num_dssiv = dssiv_l_bnds.length(), num_dssrv = dssrv_l_bnds.length();
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  copy_data_partial(cdv_l_bnds, allContinuousLowerBnds, acv_offset);
  copy_data_partial(cdv_u_bnds, allContinuousUpperBnds, acv_offset);
  acv_offset += cdv_l_bnds.length();
  for (i=0; i<num_ddrv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) {
      allContinuousLowerBnds[acv_offset]   = (Real)ddrv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]   = (Real)ddrv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteIntLowerBnds[adiv_offset] = ddrv_l_bnds[i];
      allDiscreteIntUpperBnds[adiv_offset] = ddrv_u_bnds[i];
      ++adiv_offset;
    }
  for (i=0; i<num_ddsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) {
      allContinuousLowerBnds[acv_offset]   = (Real)ddsiv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]   = (Real)ddsiv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteIntLowerBnds[adiv_offset] = ddsiv_l_bnds[i];
      allDiscreteIntUpperBnds[adiv_offset] = ddsiv_u_bnds[i];
      ++adiv_offset;
    }
  for (i=0; i<num_ddsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) {
      allContinuousLowerBnds[acv_offset]    = ddsrv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]    = ddsrv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteRealLowerBnds[adiv_offset] = ddsrv_l_bnds[i];
      allDiscreteRealUpperBnds[adiv_offset] = ddsrv_u_bnds[i];
      ++adrv_offset;
    }

  copy_data_partial(cauv_l_bnds, allContinuousLowerBnds, acv_offset);
  copy_data_partial(cauv_u_bnds, allContinuousUpperBnds, acv_offset);
  acv_offset += cauv_l_bnds.length();
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) {
      allContinuousLowerBnds[acv_offset]   = (Real)dauiv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]   = (Real)dauiv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteIntLowerBnds[adiv_offset] = dauiv_l_bnds[i];
      allDiscreteIntUpperBnds[adiv_offset] = dauiv_u_bnds[i];
      ++adiv_offset;
    }
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) {
      allContinuousLowerBnds[acv_offset]    = daurv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]    = daurv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteRealLowerBnds[adiv_offset] = daurv_l_bnds[i];
      allDiscreteRealUpperBnds[adiv_offset] = daurv_u_bnds[i];
      ++adrv_offset;
    }

  copy_data_partial(ceuv_l_bnds, allContinuousLowerBnds, acv_offset);
  copy_data_partial(ceuv_u_bnds, allContinuousUpperBnds, acv_offset);
  acv_offset += ceuv_l_bnds.length();
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) {
      allContinuousLowerBnds[acv_offset]   = (Real)deuiv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]   = (Real)deuiv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteIntLowerBnds[adiv_offset] = deuiv_l_bnds[i];
      allDiscreteIntUpperBnds[adiv_offset] = deuiv_u_bnds[i];
      ++adiv_offset;
    }
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) {
      allContinuousLowerBnds[acv_offset]    = deurv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]    = deurv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteRealLowerBnds[adiv_offset] = deurv_l_bnds[i];
      allDiscreteRealUpperBnds[adiv_offset] = deurv_u_bnds[i];
      ++adrv_offset;
    }

  copy_data_partial(csv_l_bnds, allContinuousLowerBnds, acv_offset);
  copy_data_partial(csv_u_bnds, allContinuousUpperBnds, acv_offset);
  acv_offset += csv_l_bnds.length();
  for (i=0; i<num_dsrv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) {
      allContinuousLowerBnds[acv_offset]   = (Real)dsrv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]   = (Real)dsrv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteIntLowerBnds[adiv_offset] = dsrv_l_bnds[i];
      allDiscreteIntUpperBnds[adiv_offset] = dsrv_u_bnds[i];
      ++adiv_offset;
    }
  for (i=0; i<num_dssiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr]) {
      allContinuousLowerBnds[acv_offset]   = (Real)dssiv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]   = (Real)dssiv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteIntLowerBnds[adiv_offset] = dssiv_l_bnds[i];
      allDiscreteIntUpperBnds[adiv_offset] = dssiv_u_bnds[i];
      ++adiv_offset;
    }
  for (i=0; i<num_dssrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr]) {
      allContinuousLowerBnds[acv_offset]    = dssrv_l_bnds[i];
      allContinuousUpperBnds[acv_offset]    = dssrv_u_bnds[i];
      ++acv_offset;
    }
    else {
      allDiscreteRealLowerBnds[adiv_offset] = dssrv_l_bnds[i];
      allDiscreteRealUpperBnds[adiv_offset] = dssrv_u_bnds[i];
      ++adrv_offset;
    }

#ifdef REFCOUNT_DEBUG
  const std::pair<short,short>& view = sharedVarsData.view();
  Cout << "RelaxedVarConstraints letter instantiated: view active = " 
       << view.first << " inactive = " << view.second << std::endl;
#endif
}


void RelaxedVarConstraints::read(std::istream& s)
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    /*num_ddsv = vc_totals[TOTAL_DDSV],*/   num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    /*num_dausv = vc_totals[TOTAL_DAUSV],*/ num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    /*num_deusv = vc_totals[TOTAL_DEUSV],*/ num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    /*num_dssv = vc_totals[TOTAL_DSSV],*/   num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, /*adsv_offset = 0,*/ adrv_offset = 0,
    ardi_cntr = 0, ardr_cntr = 0, i, len = 1;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // design
  read_data_partial(s, acv_offset, num_cdv, allContinuousLowerBnds);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);
  // aleatory uncertain
  read_data_partial(s, acv_offset, num_cauv, allContinuousLowerBnds);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);
  // epistemic uncertain
  read_data_partial(s, acv_offset, num_ceuv, allContinuousLowerBnds);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);
  // state
  read_data_partial(s, acv_offset, num_csv, allContinuousLowerBnds);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);

  // design
  acv_offset = adiv_offset = adrv_offset = ardi_cntr = ardr_cntr = 0;
  read_data_partial(s, acv_offset, num_cdv, allContinuousUpperBnds);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
  // aleatory uncertain
  read_data_partial(s, acv_offset, num_cauv, allContinuousUpperBnds);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
  // epistemic uncertain
  read_data_partial(s, acv_offset, num_ceuv, allContinuousUpperBnds);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
  // state
  read_data_partial(s, acv_offset, num_csv, allContinuousUpperBnds);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      read_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      read_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      read_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      read_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
}


void RelaxedVarConstraints::write(std::ostream& s) const
{
  const SizetArray& vc_totals = sharedVarsData.components_totals();
  size_t num_cdv = vc_totals[TOTAL_CDV], num_ddiv = vc_totals[TOTAL_DDIV],
    /*num_ddsv = vc_totals[TOTAL_DDSV],*/   num_ddrv  = vc_totals[TOTAL_DDRV],
    num_cauv  = vc_totals[TOTAL_CAUV],  num_dauiv = vc_totals[TOTAL_DAUIV],
    /*num_dausv = vc_totals[TOTAL_DAUSV],*/ num_daurv = vc_totals[TOTAL_DAURV],
    num_ceuv  = vc_totals[TOTAL_CEUV],  num_deuiv = vc_totals[TOTAL_DEUIV],
    /*num_deusv = vc_totals[TOTAL_DEUSV],*/ num_deurv = vc_totals[TOTAL_DEURV],
    num_csv   = vc_totals[TOTAL_CSV],   num_dsiv  = vc_totals[TOTAL_DSIV],
    /*num_dssv = vc_totals[TOTAL_DSSV],*/   num_dsrv  = vc_totals[TOTAL_DSRV],
    acv_offset = 0, adiv_offset = 0, /*adsv_offset = 0,*/ adrv_offset = 0,
    ardi_cntr = 0, ardr_cntr = 0, i, len = 1;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // design
  write_data_partial(s, acv_offset, num_cdv, allContinuousLowerBnds);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);
  // aleatory uncertain
  write_data_partial(s, acv_offset, num_cauv, allContinuousLowerBnds);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);
  // epistemic uncertain
  write_data_partial(s, acv_offset, num_ceuv, allContinuousLowerBnds);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);
  // state
  write_data_partial(s, acv_offset, num_csv, allContinuousLowerBnds);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousLowerBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntLowerBnds);
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousLowerBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealLowerBnds);

  // design
  acv_offset = adiv_offset = adrv_offset = ardi_cntr = ardr_cntr = 0;
  write_data_partial(s, acv_offset, num_cdv, allContinuousUpperBnds);
  acv_offset += num_cdv;
  for (i=0; i<num_ddiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_ddrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
  // aleatory uncertain
  write_data_partial(s, acv_offset, num_cauv, allContinuousUpperBnds);
  acv_offset += num_cauv;
  for (i=0; i<num_dauiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_daurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
  // epistemic uncertain
  write_data_partial(s, acv_offset, num_ceuv, allContinuousUpperBnds);
  acv_offset += num_ceuv;
  for (i=0; i<num_deuiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_deurv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
  // state
  write_data_partial(s, acv_offset, num_csv, allContinuousUpperBnds);
  acv_offset += num_csv;
  for (i=0; i<num_dsiv; ++i, ++ardi_cntr)
    if (all_relax_di[ardi_cntr])
      write_data_partial(s,  acv_offset++, len,  allContinuousUpperBnds);
    else
      write_data_partial(s, adiv_offset++, len, allDiscreteIntUpperBnds);
  for (i=0; i<num_dsrv; ++i, ++ardr_cntr)
    if (all_relax_dr[ardr_cntr])
      write_data_partial(s,  acv_offset++, len,   allContinuousUpperBnds);
    else
      write_data_partial(s, adrv_offset++, len, allDiscreteRealUpperBnds);
}

} // namespace Dakota
