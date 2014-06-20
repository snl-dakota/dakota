/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        MixedVarConstraints
//- Description:  Class implementation
//- Owner:        Mike Eldred

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
  int start = 0;
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
  copy_data_partial(cdv_l_bnds, allContinuousLowerBnds, start);
  copy_data_partial(cdv_u_bnds, allContinuousUpperBnds, start);
  start += cdv_l_bnds.length();
  copy_data_partial(cauv_l_bnds, allContinuousLowerBnds, start);
  copy_data_partial(cauv_u_bnds, allContinuousUpperBnds, start);
  start += cauv_l_bnds.length();
  copy_data_partial(ceuv_l_bnds, allContinuousLowerBnds, start);
  copy_data_partial(ceuv_u_bnds, allContinuousUpperBnds, start);
  start += ceuv_l_bnds.length();
  copy_data_partial(csv_l_bnds, allContinuousLowerBnds, start);
  copy_data_partial(csv_u_bnds, allContinuousUpperBnds, start);
  //start += csv_l_bnds.length();

  start = 0;
  const IntVector& ddrv_l_bnds = problem_db.get_iv(
    "variables.discrete_design_range.lower_bounds");
  const IntVector& ddrv_u_bnds = problem_db.get_iv(
    "variables.discrete_design_range.upper_bounds");
  copy_data_partial(ddrv_l_bnds, allDiscreteIntLowerBnds, start);
  copy_data_partial(ddrv_u_bnds, allDiscreteIntUpperBnds, start);
  start += ddrv_l_bnds.length();
  const IntVector& ddsiv_l_bnds = problem_db.get_iv(
    "variables.discrete_design_set_int.lower_bounds");
  const IntVector& ddsiv_u_bnds = problem_db.get_iv(
    "variables.discrete_design_set_int.upper_bounds");
  copy_data_partial(ddsiv_l_bnds, allDiscreteIntLowerBnds, start);
  copy_data_partial(ddsiv_u_bnds, allDiscreteIntUpperBnds, start);
  start += ddsiv_l_bnds.length();
  const IntVector& dauiv_l_bnds = problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.lower_bounds");
  const IntVector& dauiv_u_bnds = problem_db.get_iv(
    "variables.discrete_aleatory_uncertain_int.upper_bounds");
  copy_data_partial(dauiv_l_bnds, allDiscreteIntLowerBnds, start);
  copy_data_partial(dauiv_u_bnds, allDiscreteIntUpperBnds, start);
  start += dauiv_l_bnds.length();
  const IntVector& deuiv_l_bnds = problem_db.get_iv(
    "variables.discrete_epistemic_uncertain_int.lower_bounds");
  const IntVector& deuiv_u_bnds = problem_db.get_iv(
    "variables.discrete_epistemic_uncertain_int.upper_bounds");
  copy_data_partial(deuiv_l_bnds, allDiscreteIntLowerBnds, start);
  copy_data_partial(deuiv_u_bnds, allDiscreteIntUpperBnds, start);
  start += deuiv_l_bnds.length();
  const IntVector& dsrv_l_bnds = problem_db.get_iv(
    "variables.discrete_state_range.lower_bounds");
  const IntVector& dsrv_u_bnds = problem_db.get_iv(
    "variables.discrete_state_range.upper_bounds");
  copy_data_partial(dsrv_l_bnds, allDiscreteIntLowerBnds, start);
  copy_data_partial(dsrv_u_bnds, allDiscreteIntUpperBnds, start);
  start += dsrv_l_bnds.length();
  const IntVector& dssiv_l_bnds = problem_db.get_iv(
    "variables.discrete_state_set_int.lower_bounds");
  const IntVector& dssiv_u_bnds = problem_db.get_iv(
    "variables.discrete_state_set_int.upper_bounds");
  copy_data_partial(dssiv_l_bnds, allDiscreteIntLowerBnds, start);
  copy_data_partial(dssiv_u_bnds, allDiscreteIntUpperBnds, start);
  //start += dssiv_l_bnds.length();

  // discrete string variable bounds?  (not needed for exclusively categorical)

  start = 0;
  const RealVector& ddsrv_l_bnds = problem_db.get_rv(
    "variables.discrete_design_set_real.lower_bounds");
  const RealVector& ddsrv_u_bnds = problem_db.get_rv(
    "variables.discrete_design_set_real.upper_bounds");
  copy_data_partial(ddsrv_l_bnds, allDiscreteRealLowerBnds, start);
  copy_data_partial(ddsrv_u_bnds, allDiscreteRealUpperBnds, start);
  start += ddsrv_l_bnds.length();
  const RealVector& daurv_l_bnds = problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.lower_bounds");
  const RealVector& daurv_u_bnds = problem_db.get_rv(
    "variables.discrete_aleatory_uncertain_real.upper_bounds");
   copy_data_partial(daurv_l_bnds, allDiscreteRealLowerBnds, start);
  copy_data_partial(daurv_u_bnds, allDiscreteRealUpperBnds, start);
  start += daurv_l_bnds.length();
  const RealVector& deurv_l_bnds = problem_db.get_rv(
    "variables.discrete_epistemic_uncertain_real.lower_bounds");
  const RealVector& deurv_u_bnds = problem_db.get_rv(
    "variables.discrete_epistemic_uncertain_real.upper_bounds");
  copy_data_partial(deurv_l_bnds, allDiscreteRealLowerBnds, start);
  copy_data_partial(deurv_u_bnds, allDiscreteRealUpperBnds, start);
  start += deurv_l_bnds.length();
  const RealVector& dssrv_l_bnds = problem_db.get_rv(
    "variables.discrete_state_set_real.lower_bounds");
  const RealVector& dssrv_u_bnds = problem_db.get_rv(
    "variables.discrete_state_set_real.upper_bounds");
  copy_data_partial(dssrv_l_bnds, allDiscreteRealLowerBnds, start);
  copy_data_partial(dssrv_u_bnds, allDiscreteRealUpperBnds, start);
  //start += dssrv_l_bnds.length();

#ifdef REFCOUNT_DEBUG
  const std::pair<short,short>& view = sharedVarsData.view();
  Cout << "MixedVarConstraints letter instantiated: view active = "
       << view.first << " inactive = " << view.second << endl;
#endif
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
