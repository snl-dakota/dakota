/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "MixedVarConstraints.hpp"
#include "ProblemDescDB.hpp"
#include "dakota_data_io.hpp"
#include "dakota_data_util.hpp"

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
  // --- Continuous variable bounds ---
  {
    int start = 0;
    // Helper: copy bounds from explicit keys into concatenated arrays
    auto copy_cv = [&](const String& lb_key, const String& ub_key) {
      const RealVector& lb = problem_db.get_rv(lb_key);
      const RealVector& ub = problem_db.get_rv(ub_key);
      copy_data_partial(lb, allContinuousLowerBnds, start);
      copy_data_partial(ub, allContinuousUpperBnds, start);
      start += lb.length();
    };
    // Helper: standard pattern using base + ".lower_bounds"/".upper_bounds"
    auto copy_cv_std = [&](const char* base) {
      copy_cv(String(base)+".lower_bounds", String(base)+".upper_bounds");
    };

    // design
    copy_cv_std("variables.continuous_design");
    // aleatory uncertain: normal/lognormal use inferred bounds
    copy_cv("variables.normal_uncertain.inferred_lower_bounds",
            "variables.normal_uncertain.inferred_upper_bounds");
    copy_cv("variables.lognormal_uncertain.inferred_lower_bounds",
            "variables.lognormal_uncertain.inferred_upper_bounds");
    // remaining aleatory uncertain use distribution bounds directly
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
    for (const char* base : cauv_remaining) copy_cv_std(base);
    // epistemic uncertain
    copy_cv_std("variables.continuous_interval_uncertain");
    // state
    copy_cv_std("variables.continuous_state");
  }

  // --- Discrete integer variable bounds ---
  {
    int start = 0;
    const char* div_keys[] = {
      // design
      "variables.discrete_design_range",
      "variables.discrete_design_set_int",
      // aleatory uncertain
      "variables.poisson_uncertain",
      "variables.binomial_uncertain",
      "variables.negative_binomial_uncertain",
      "variables.geometric_uncertain",
      "variables.hypergeometric_uncertain",
      "variables.histogram_uncertain.point_int",
      // epistemic uncertain
      "variables.discrete_interval_uncertain",
      "variables.discrete_uncertain_set_int",
      // state
      "variables.discrete_state_range",
      "variables.discrete_state_set_int"
    };
    for (const char* base : div_keys) {
      const IntVector& lb = problem_db.get_iv(String(base)+".lower_bounds");
      const IntVector& ub = problem_db.get_iv(String(base)+".upper_bounds");
      copy_data_partial(lb, allDiscreteIntLowerBnds, start);
      copy_data_partial(ub, allDiscreteIntUpperBnds, start);
      start += lb.length();
    }
  }

  // discrete string variable bounds?  (not needed for exclusively categorical)

  // --- Discrete real variable bounds ---
  {
    int start = 0;
    const char* drv_keys[] = {
      // design
      "variables.discrete_design_set_real",
      // aleatory uncertain
      "variables.histogram_uncertain.point_real",
      // epistemic uncertain
      "variables.discrete_uncertain_set_real",
      // state
      "variables.discrete_state_set_real"
    };
    for (const char* base : drv_keys) {
      const RealVector& lb = problem_db.get_rv(String(base)+".lower_bounds");
      const RealVector& ub = problem_db.get_rv(String(base)+".upper_bounds");
      copy_data_partial(lb, allDiscreteRealLowerBnds, start);
      copy_data_partial(ub, allDiscreteRealUpperBnds, start);
      start += lb.length();
    }
  }
}


void MixedVarConstraints::read(std::istream& s)
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
    acv_offset = 0, adiv_offset = 0, /*adsv_offset = 0,*/ adrv_offset = 0;

  // lower bounds
  read_data_partial(s, acv_offset,  num_cdv,  allContinuousLowerBnds);
  read_data_partial(s, adiv_offset, num_ddiv, allDiscreteIntLowerBnds);
  //read_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringLowerBnds);
  read_data_partial(s, adrv_offset, num_ddrv, allDiscreteRealLowerBnds);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  /*adsv_offset += num_ddsv;*/ adrv_offset += num_ddrv;

  read_data_partial(s, acv_offset,  num_cauv,  allContinuousLowerBnds);
  read_data_partial(s, adiv_offset, num_dauiv, allDiscreteIntLowerBnds);
  //read_data_partial(s, adsv_offset, num_dausv, allDiscreteStringLowerBnds);
  read_data_partial(s, adrv_offset, num_daurv, allDiscreteRealLowerBnds);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  /*adsv_offset += num_dausv;*/ adrv_offset += num_daurv;

  read_data_partial(s, acv_offset,  num_ceuv,  allContinuousLowerBnds);
  read_data_partial(s, adiv_offset, num_deuiv, allDiscreteIntLowerBnds);
  //read_data_partial(s, adsv_offset, num_deusv, allDiscreteStringLowerBnds);
  read_data_partial(s, adrv_offset, num_deurv, allDiscreteRealLowerBnds);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  /*adsv_offset += num_deusv;*/ adrv_offset += num_deurv;

  read_data_partial(s, acv_offset,  num_csv,  allContinuousLowerBnds);
  read_data_partial(s, adiv_offset, num_dsiv, allDiscreteIntLowerBnds);
  //read_data_partial(s, adsv_offset, num_dssv, allDiscreteStringLowerBnds);
  read_data_partial(s, adrv_offset, num_dsrv, allDiscreteRealLowerBnds);

  // upper bounds
  acv_offset = adiv_offset = /*adsv_offset =*/ adrv_offset = 0;
  read_data_partial(s, acv_offset,  num_cdv,  allContinuousUpperBnds);
  read_data_partial(s, adiv_offset, num_ddiv, allDiscreteIntUpperBnds);
  //read_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringUpperBnds);
  read_data_partial(s, adrv_offset, num_ddrv, allDiscreteRealUpperBnds);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  /*adsv_offset += num_ddsv;*/ adrv_offset += num_ddrv;

  read_data_partial(s, acv_offset,  num_cauv,  allContinuousUpperBnds);
  read_data_partial(s, adiv_offset, num_dauiv, allDiscreteIntUpperBnds);
  //read_data_partial(s, adsv_offset, num_dausv, allDiscreteStringUpperBnds);
  read_data_partial(s, adrv_offset, num_daurv, allDiscreteRealUpperBnds);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  /*adsv_offset += num_dausv;*/ adrv_offset += num_daurv;

  read_data_partial(s, acv_offset,  num_ceuv,  allContinuousUpperBnds);
  read_data_partial(s, adiv_offset, num_deuiv, allDiscreteIntUpperBnds);
  //read_data_partial(s, adsv_offset, num_deusv, allDiscreteStringUpperBnds);
  read_data_partial(s, adrv_offset, num_deurv, allDiscreteRealUpperBnds);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  /*adsv_offset += num_deusv;*/ adrv_offset += num_deurv;

  read_data_partial(s, acv_offset,  num_csv,  allContinuousUpperBnds);
  read_data_partial(s, adiv_offset, num_dsiv, allDiscreteIntUpperBnds);
  //read_data_partial(s, adsv_offset, num_dssv, allDiscreteStringUpperBnds);
  read_data_partial(s, adrv_offset, num_dsrv, allDiscreteRealUpperBnds);
}


void MixedVarConstraints::write(std::ostream& s) const
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
    acv_offset = 0, adiv_offset = 0, /*adsv_offset = 0,*/ adrv_offset = 0;

  // lower bounds
  write_data_partial(s, acv_offset,  num_cdv,  allContinuousLowerBnds);
  write_data_partial(s, adiv_offset, num_ddiv, allDiscreteIntLowerBnds);
  //write_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringLowerBnds);
  write_data_partial(s, adrv_offset, num_ddrv, allDiscreteRealLowerBnds);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  /*adsv_offset += num_ddsv;*/ adrv_offset += num_ddrv;

  write_data_partial(s, acv_offset,  num_cauv,  allContinuousLowerBnds);
  write_data_partial(s, adiv_offset, num_dauiv, allDiscreteIntLowerBnds);
  //write_data_partial(s, adsv_offset, num_dausv, allDiscreteStringLowerBnds);
  write_data_partial(s, adrv_offset, num_daurv, allDiscreteRealLowerBnds);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  /*adsv_offset += num_dausv;*/ adrv_offset += num_daurv;

  write_data_partial(s, acv_offset,  num_ceuv,  allContinuousLowerBnds);
  write_data_partial(s, adiv_offset, num_deuiv, allDiscreteIntLowerBnds);
  //write_data_partial(s, adsv_offset, num_deusv, allDiscreteStringLowerBnds);
  write_data_partial(s, adrv_offset, num_deurv, allDiscreteRealLowerBnds);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  /*adsv_offset += num_deusv;*/ adrv_offset += num_deurv;

  write_data_partial(s, acv_offset,  num_csv,  allContinuousLowerBnds);
  write_data_partial(s, adiv_offset, num_dsiv, allDiscreteIntLowerBnds);
  //write_data_partial(s, adsv_offset, num_dssv, allDiscreteStringLowerBnds);
  write_data_partial(s, adrv_offset, num_dsrv, allDiscreteRealLowerBnds);

  // upper bounds
  acv_offset = adiv_offset = /*adsv_offset =*/ adrv_offset = 0;
  write_data_partial(s, acv_offset,  num_cdv,  allContinuousUpperBnds);
  write_data_partial(s, adiv_offset, num_ddiv, allDiscreteIntUpperBnds);
  //write_data_partial(s, adsv_offset, num_ddsv, allDiscreteStringUpperBnds);
  write_data_partial(s, adrv_offset, num_ddrv, allDiscreteRealUpperBnds);
  acv_offset  += num_cdv;  adiv_offset += num_ddiv;
  /*adsv_offset += num_ddsv;*/ adrv_offset += num_ddrv;

  write_data_partial(s, acv_offset,  num_cauv,  allContinuousUpperBnds);
  write_data_partial(s, adiv_offset, num_dauiv, allDiscreteIntUpperBnds);
  //write_data_partial(s, adsv_offset, num_dausv, allDiscreteStringUpperBnds);
  write_data_partial(s, adrv_offset, num_daurv, allDiscreteRealUpperBnds);
  acv_offset  += num_cauv;  adiv_offset += num_dauiv;
  /*adsv_offset += num_dausv;*/ adrv_offset += num_daurv;

  write_data_partial(s, acv_offset,  num_ceuv,  allContinuousUpperBnds);
  write_data_partial(s, adiv_offset, num_deuiv, allDiscreteIntUpperBnds);
  //write_data_partial(s, adsv_offset, num_deusv, allDiscreteStringUpperBnds);
  write_data_partial(s, adrv_offset, num_deurv, allDiscreteRealUpperBnds);
  acv_offset  += num_ceuv;  adiv_offset += num_deuiv;
  /*adsv_offset += num_deusv;*/ adrv_offset += num_deurv;

  write_data_partial(s, acv_offset,  num_csv,  allContinuousUpperBnds);
  write_data_partial(s, adiv_offset, num_dsiv, allDiscreteIntUpperBnds);
  //write_data_partial(s, adsv_offset, num_dssv, allDiscreteStringUpperBnds);
  write_data_partial(s, adrv_offset, num_dsrv, allDiscreteRealUpperBnds);
}

} // namespace Dakota
