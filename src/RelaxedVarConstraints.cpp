/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  size_t i, ardi_cntr = 0, ardr_cntr = 0,
    acv_offset = 0, adiv_offset = 0, adrv_offset = 0;
  const BitArray& all_relax_di = sharedVarsData.all_relaxed_discrete_int();
  const BitArray& all_relax_dr = sharedVarsData.all_relaxed_discrete_real();

  // Helper lambda: copy continuous bounds from explicit keys
  auto copy_cv = [&](const String& lb_key, const String& ub_key) {
    const RealVector& lb = problem_db.get_rv(lb_key);
    const RealVector& ub = problem_db.get_rv(ub_key);
    copy_data_partial(lb, allContinuousLowerBnds, acv_offset);
    copy_data_partial(ub, allContinuousUpperBnds, acv_offset);
    acv_offset += lb.length();
  };
  // Helper: standard pattern using base + ".lower_bounds"/".upper_bounds"
  auto copy_cv_std = [&](const char* base) {
    copy_cv(String(base)+".lower_bounds", String(base)+".upper_bounds");
  };

  // Helper lambda: relax discrete int bounds from a per-type key
  auto relax_div = [&](const char* base) {
    const IntVector& lb = problem_db.get_iv(String(base)+".lower_bounds");
    const IntVector& ub = problem_db.get_iv(String(base)+".upper_bounds");
    for (i=0; i<lb.length(); ++i, ++ardi_cntr)
      if (all_relax_di[ardi_cntr]) {
	allContinuousLowerBnds[acv_offset]   = (Real)lb[i];
	allContinuousUpperBnds[acv_offset]   = (Real)ub[i];
	++acv_offset;
      }
      else {
	allDiscreteIntLowerBnds[adiv_offset] = lb[i];
	allDiscreteIntUpperBnds[adiv_offset] = ub[i];
	++adiv_offset;
      }
  };

  // Helper lambda: relax discrete real bounds from a per-type key
  auto relax_drv = [&](const char* base) {
    const RealVector& lb = problem_db.get_rv(String(base)+".lower_bounds");
    const RealVector& ub = problem_db.get_rv(String(base)+".upper_bounds");
    for (i=0; i<lb.length(); ++i, ++ardr_cntr)
      if (all_relax_dr[ardr_cntr]) {
	allContinuousLowerBnds[acv_offset]    = lb[i];
	allContinuousUpperBnds[acv_offset]    = ub[i];
	++acv_offset;
      }
      else {
	allDiscreteRealLowerBnds[adrv_offset] = lb[i];
	allDiscreteRealUpperBnds[adrv_offset] = ub[i];
	++adrv_offset;
      }
  };

  // --- design ---
  copy_cv_std("variables.continuous_design");
  relax_div("variables.discrete_design_range");
  relax_div("variables.discrete_design_set_int");
  relax_drv("variables.discrete_design_set_real");

  // --- aleatory uncertain ---
  // normal/lognormal use inferred bounds
  copy_cv("variables.normal_uncertain.inferred_lower_bounds",
          "variables.normal_uncertain.inferred_upper_bounds");
  copy_cv("variables.lognormal_uncertain.inferred_lower_bounds",
          "variables.lognormal_uncertain.inferred_upper_bounds");
  {
    const char* cauv_remaining[] = {
      "variables.uniform_uncertain",
      "variables.loguniform_uncertain",
      "variables.triangular_uncertain",
      "variables.exponential_uncertain",
      "variables.beta_uncertain",
      "variables.gamma_uncertain",
      "variables.gumbel_uncertain",
      "variables.frechet_uncertain",
      "variables.weibull_uncertain",
      "variables.histogram_bin_uncertain"
    };
    for (const char* key : cauv_remaining) copy_cv_std(key);
  }
  {
    const char* dauiv_keys[] = {
      "variables.poisson_uncertain",
      "variables.binomial_uncertain",
      "variables.negative_binomial_uncertain",
      "variables.geometric_uncertain",
      "variables.hypergeometric_uncertain",
      "variables.histogram_uncertain.point_int"
    };
    for (const char* key : dauiv_keys) relax_div(key);
  }
  relax_drv("variables.histogram_uncertain.point_real");

  // --- epistemic uncertain ---
  copy_cv_std("variables.continuous_interval_uncertain");
  {
    const char* deuiv_keys[] = {
      "variables.discrete_interval_uncertain",
      "variables.discrete_uncertain_set_int"
    };
    for (const char* key : deuiv_keys) relax_div(key);
  }
  relax_drv("variables.discrete_uncertain_set_real");

  // --- state ---
  copy_cv_std("variables.continuous_state");
  relax_div("variables.discrete_state_range");
  relax_div("variables.discrete_state_set_int");
  relax_drv("variables.discrete_state_set_real");
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
