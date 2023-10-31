/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SharedC3ApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

#include "pecos_math_util.hpp"
#include "pecos_stat_util.hpp"
#include "pecos_global_defs.hpp"

#include <assert.h>
//#define DEBUG

namespace Dakota {


SharedC3ApproxData::
SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars),
  respScaling(problem_db.get_bool("model.surrogate.response_scaling")),
  kickOrder(problem_db.get_ushort("model.c3function_train.kick_order")),
  maxOrder(problem_db.get_ushort("model.c3function_train.max_order")),
  adaptOrder(problem_db.get_bool("model.c3function_train.adapt_order")),
  startRank(problem_db.get_sizet("model.c3function_train.start_rank")),
  kickRank(problem_db.get_sizet("model.c3function_train.kick_rank")),
  maxRank(problem_db.get_sizet("model.c3function_train.max_rank")),
  adaptRank(problem_db.get_bool("model.c3function_train.adapt_rank")),
  regressType(problem_db.get_short("model.surrogate.regression_type")),
  regressRegParam(problem_db.get_real("model.surrogate.regression_penalty")),
  solverTol(problem_db.get_real("model.c3function_train.solver_tolerance")),
  solverRoundingTol(
    problem_db.get_real("model.c3function_train.solver_rounding_tolerance")),
  statsRoundingTol(
    problem_db.get_real("model.c3function_train.stats_rounding_tolerance")),
  maxSolverIterations(problem_db.get_sizet("model.max_solver_iterations")),
  crossMaxIter(
    problem_db.get_int("model.c3function_train.max_cross_iterations")),
  //adaptConstruct(false),
  c3AdvancementType(NO_C3_ADVANCEMENT),
  maxCVRankCandidates(problem_db.get_sizet(
    "model.c3function_train.cross_validation.max_rank_candidates")),
  maxCVOrderCandidates(problem_db.get_ushort(
    "model.c3function_train.cross_validation.max_order_candidates"))
{
  // This ctor used for user-spec of DataFitSurrModel (surrogate global FT
  // used by generic surrogate-based UQ in NonDSurrogateExpansion)

  RealVector dim_pref_spec; // isotropic for now, prior to XML support
  unsigned short start_o
    = problem_db.get_ushort("model.c3function_train.start_order");
  Pecos::dimension_preference_to_anisotropic_order(start_o,
    dim_pref_spec,//problem_db.get_rv("model.dimension_preference"),
    numVars, startOrders);

  multiApproxOpts = multi_approx_opts_alloc(num_vars);
  oneApproxOpts.assign(num_vars, NULL);

  //infer_max_cross_validation_ranges();
}


SharedC3ApproxData::
SharedC3ApproxData(const String& approx_type, const UShortArray& approx_order,
		   size_t num_vars, short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level),
  // default values overridden by set_parameter
  respScaling(false), startOrders(approx_order), kickOrder(1),
  maxOrder(USHRT_MAX), adaptOrder(false), startRank(2), kickRank(1),
  maxRank(SZ_MAX), adaptRank(false),
  regressType(FT_LS), // non-regularized least sq
  solverTol(1.e-10), solverRoundingTol(1.e-10), statsRoundingTol(1.e-10),
  maxSolverIterations(SZ_MAX), crossMaxIter(5), //adaptConstruct(false),
  c3AdvancementType(NO_C3_ADVANCEMENT), maxCVOrderCandidates(USHRT_MAX),
  maxCVRankCandidates(SZ_MAX)
{
  // This ctor used by lightweight/on-the-fly DataFitSurrModel ctor
  // (used to build an FT on top of a user model in NonDC3FuntionTrain)

  // short basis_type; approx_type_to_basis_type(approxType, basis_type);
  
  multiApproxOpts = multi_approx_opts_alloc(num_vars);
  oneApproxOpts.assign(num_vars, NULL);
}


SharedC3ApproxData::~SharedC3ApproxData()
{
  multi_approx_opts_free(multiApproxOpts); //multiApproxOpts = NULL;
  for (size_t i=0; i<numVars; ++i) {
    struct OneApproxOpts*& a_opts = oneApproxOpts[i]; // ref to ptr
    if (a_opts) one_approx_opts_free_deep(&a_opts); //a_opts = NULL;
  }
}


/* Just use default = Inf and rely on spec for overrides
   > Simplest approach and most intuitive/consistent
   > Don't preclude per-QoI customization (e.g. extension of range to include
     a previous recovery) with override logic at init time
void SharedC3ApproxData::infer_max_cross_validation_ranges()
{
  // infer max CV candidates when it can be inferred from start,max spec.
  // Note: in the case where we want to grow the range (e.g., from 1 with
  // start initially == max), we will need to specify { start, max, max CV }.
  if (adaptRank) {
    bool no_max_r = (maxRank == SZ_MAX);
    if (maxCVRankCandidates == SZ_MAX) {
      if (no_max_r)
	maxCVRankCandidates = 5; // default of 5
      else {
	size_t max_cv = (maxRank - startRank) / kickRank + 1;
	maxCVRankCandidates = std::max(max_cv, (size_t)3); // lower bound of 3
      }
    }
    // Note: C3Approx::build() can be dynamic whereas this is only at init time
    //if (no_max_r)
    //  maxRank = startRank + kickRank * (maxCVRankCandidates - 1);
  }
  if (adaptOrder) {
    bool no_max_o = (maxOrder == USHRT_MAX);
    if (maxCVOrderCandidates == USHRT_MAX) {
      if (no_max_o)
	maxCVOrderCandidates = 5; // default of 5
      else {
	unsigned short start_o = find_max(startOrders),
	  max_cv = (maxOrder - start_o) / kickOrder + 1;
	maxCVOrderCandidates = std::max(max_cv, (unsigned short)3);// low bnd=3
      }
    }
    // Note: C3Approx::build() can be dynamic whereas this is only at init time
    //if (no_max_o)
    //  maxOrder = find_max(startOrders) + kickOrder * (maxCVOrderCandidates-1);
  }
}
*/


void SharedC3ApproxData::
construct_basis(const Pecos::MultivariateDistribution& mv_dist)
{
  const ShortArray& rv_types  = mv_dist.random_variable_types();
  const BitArray& active_vars = mv_dist.active_variables();
  bool no_mask = active_vars.empty();
  size_t i, av_cntr = 0, num_rv = rv_types.size(),
    num_active_rv = (no_mask) ? num_rv : active_vars.count();
  assert (num_active_rv == numVars);

  struct OpeOpts * o_opts;
  // uses startOrders at construct time (initialize_u_space_model()) since
  // startOrdersMap[key] not meaningful until run-time active key assignment
  const UShortArray& so = start_orders();
  size_t np, max_np = max_order(); ++max_np;//no overflow (default is USHRT_MAX)
  for (i=0; i<num_rv; ++i)
    if (no_mask || active_vars[i]) {
      switch (rv_types[i]) {
      case Pecos::STD_NORMAL:
	o_opts = ope_opts_alloc(HERMITE);  break;
      case Pecos::STD_UNIFORM:
	o_opts = ope_opts_alloc(LEGENDRE); break;
      default:
	o_opts = NULL;
	PCerr << "Error: unsupported RV type (" << rv_types[i] << ") in "
	      << "SharedC3ApproxData::distribution_parameters()" << std::endl;
	abort_handler(-1);                 break;
      }

      np = std::min((size_t)so[i] + 1, max_np);
      ope_opts_set_nparams(o_opts, np);     // startnum = startord + 1
      // Note: maxOrder not used for C3 regression (limits increment_order()
      // on Dakota side); to be used for adaptation by cross-approximation
      ope_opts_set_maxnum(o_opts,  max_np); //   maxnum =   maxord + 1
 
      struct OneApproxOpts*& a_opts = oneApproxOpts[av_cntr]; // ref to ptr
      if (a_opts) one_approx_opts_free_deep(&a_opts); // a_opts frees o_opts
      a_opts = one_approx_opts_alloc(POLYNOMIAL, o_opts);
      multi_approx_opts_set_dim(multiApproxOpts, av_cntr, a_opts);

      ++av_cntr;
    }

  // don't bother to assign formUpdated prior to an active key definition
  if (!activeKey.empty())
    formUpdated[activeKey] = true;
}


void SharedC3ApproxData::
update_basis(const UShortArray& start_orders, unsigned short max_order)
{
  size_t np, max_np = max_order; ++max_np; // no overflow (default is USHRT_MAX)
  for (size_t v=0; v<numVars; ++v) {
    struct OneApproxOpts*& a_opts = oneApproxOpts[v];
    np = (size_t)std::min(start_orders[v], max_order) + 1;
    one_approx_opts_set_nparams(a_opts, np);
    one_approx_opts_set_maxnum( a_opts, max_np);
  }

  formUpdated[activeKey] = true;
}


void SharedC3ApproxData::
update_basis(size_t v, unsigned short start_order, unsigned short max_order)
{
  size_t max_np = max_order; ++max_np; // no overflow (default is USHRT_MAX)
  size_t np = (size_t)std::min(start_order, max_order) + 1;

  struct OneApproxOpts*& a_opts = oneApproxOpts[v];
  one_approx_opts_set_nparams(a_opts, np);
  one_approx_opts_set_maxnum( a_opts, max_np);

  //formUpdated[activeKey] = true; // elevate to clients to avoid redundancy
}


bool SharedC3ApproxData::advancement_available()
{
  // START_*_ADVANCEMENT cases are tested first. If false, then each C3Approx
  // is tested for MAX_*_ADVANCEMENT cases (see ApproximationInterface::
  // advancement_available()).  This distributes the c3AdvancementType cases.

  switch (c3AdvancementType) {

  // these two options only require the shared data config:
  case START_ORDER_ADVANCEMENT: {
    const UShortArray& s_ord = start_orders();// adapted orders
    unsigned short     m_ord = max_order();
    size_t i, num_ord = s_ord.size();
    for (size_t i=0; i<num_ord; ++i)
      if (s_ord[i] < m_ord)
	return true;
    return false;  break;
  }
  case START_RANK_ADVANCEMENT:
    return (start_rank() < max_rank());  break;

  // MAX_*_ADVANCEMENT refine types are QoI-dependent: shared false induces
  // per-QoI checks (see ApproximationInterface::advancement_available()
  default:
    // clear state prior to accumulation across C3Approximations
    c3MaxRankAdvance[activeKey] = c3MaxOrderAdvance[activeKey] = false;
    //c3StartRankAdvance[activeKey] = c3StartOrderAdvance[activeKey] = false;
    return false;  break;
  }
}


void SharedC3ApproxData::increment_order()
{
  switch (c3AdvancementType) {
  case START_ORDER_ADVANCEMENT: {
    UShortArray& start_ord = start_orders();
    unsigned short max_ord = max_order(); // default is USHRT_MAX
    bool incremented = false;
    for (size_t v=0; v<numVars; ++v) {
      unsigned short &s_ord = start_ord[v], prev_ord = s_ord;
      // unconditional increment (preserve reproducibility w/decrement)
      // Note: dim_pref ratios are not preserved
      s_ord += kickOrder;
      if (prev_ord < max_ord) // increment occurs, but kick may be truncated
	incremented = true;
    }
    if (incremented)
      formUpdated[activeKey] = true;
    else
      Cerr << "Warning: SharedC3ApproxData::increment_order() cannot advance "
	   << "order beyond maximum allowable (" << max_ord << ") for key:\n"
	   << activeKey << std::endl;
    break;
  }
  case START_RANK_ADVANCEMENT: {
    // To ensure symmetry with decrement, don't saturate at maxRank
    // > Must bound start_ranks vector in C3Approximation::build()
    // > build() truncates kick to max: start_ranks = std::min(start_r, max_r) 
    size_t &start_r = start_rank();
    if (start_r < max_rank()) // increment occurs but kick might be truncated
      formUpdated[activeKey] = true;
    start_r += kickRank; // invertible in decrement_order()
    break;
  }
  // Rather than shared management of start rank/order decrements as for
  // max rank/order increments, limit the CV range for each QoI using
  // maxCV{Rank,Order}Candidates within C3Approximation::build().
  case MAX_RANK_ADVANCEMENT:
    if (increment_max_rank())// || decrement_start_rank())
      formUpdated[activeKey] = true;
    break;
  case MAX_ORDER_ADVANCEMENT:
    if (increment_max_order())// || decrement_start_order())
      formUpdated[activeKey] = true;
    break;
  case MAX_RANK_ORDER_ADVANCEMENT:
    // Prior to implementing a multi-index approach, we use heuristics.
    // > unconditionally advancing both is wasteful; only advance non-saturated
    // > could also consider only advancing one when both bounds are active:
    //   least saturated first with tie break to max rank (recovered ranks are
    //   heterogeneous anyway)
    bool incr_mr = increment_max_rank(),   incr_mo = increment_max_order();
    //   decr_sr = decrement_start_rank(), decr_so = decrement_start_order();
    if (incr_mr || incr_mo) // || decr_sr || decr_so)
      formUpdated[activeKey] = true;
    break;
  }
}


bool SharedC3ApproxData::increment_max_rank()
{
  bool m_r_advance = c3MaxRankAdvance[activeKey];
  if (m_r_advance)
    { size_t& max_r = max_rank(); max_r += kickRank; }
  return m_r_advance;
}


bool SharedC3ApproxData::increment_max_order()
{
  bool m_o_advance = c3MaxOrderAdvance[activeKey];
  if (m_o_advance)
    { unsigned short& max_o = max_order(); max_o += kickOrder; }
  return m_o_advance;
}


/*
bool SharedC3ApproxData::decrement_start_rank()
{
  bool s_r_reduce = c3StartRankReduce[activeKey];
  if (s_r_reduce) // allow decrement of start if lower bound active
    { size_t& start_r = start_rank(); start_r -= kickRank; }
  return s_r_reduce;
}


bool SharedC3ApproxData::decrement_start_order()
{
  bool s_o_reduce = c3StartOrderReduce[activeKey];
  if (s_o_reduce) // allow decrement of start if lower bound active
    { unsigned short& start_o = start_order(); start_o -= kickOrder; }
  return s_o_reduce;
}
*/


void SharedC3ApproxData::decrement_order()
{
  bool bad_range = false;
  switch (c3AdvancementType) {
  case START_ORDER_ADVANCEMENT: {
    // always decrement and only update_basis() if within bounds
    // (preserve symmetry/reproducibility w/ increment)
    UShortArray& start_ord = start_orders();
    unsigned short max_ord = max_order(); // default is USHRT_MAX
    bool decremented = false;
    for (size_t v=0; v<numVars; ++v) {
      unsigned short &s_ord = start_ord[v];
      if (s_ord >= kickOrder) { // for completeness (should not happen)
	s_ord -= kickOrder; // preserve symmetry/reproducibility w/increment
	if (s_ord < max_ord) // only communicate if in bounds
	  decremented = true;
      }
    }
    if (decremented)  formUpdated[activeKey] = true;
    else              bad_range = true;
    break;
  }
  case START_RANK_ADVANCEMENT: {
    size_t& start_r = start_rank();
    if    (start_r  < kickRank)  bad_range = true;
    else { start_r -= kickRank;  formUpdated[activeKey] = true; }
    break;
  }
  case MAX_RANK_ADVANCEMENT: {
    size_t& max_r = max_rank();
    if    (max_r  < kickRank)  bad_range = true;
    else { max_r -= kickRank;  formUpdated[activeKey] = true; }
    break;
  }
  case MAX_ORDER_ADVANCEMENT: {
    unsigned short& max_o = max_order();
    if    (max_o  < kickOrder)  bad_range = true;
    else { max_o -= kickOrder;  formUpdated[activeKey] = true; }
    break;
  }
  case MAX_RANK_ORDER_ADVANCEMENT: {
    bool r_advance = c3MaxRankAdvance[activeKey],
         o_advance = c3MaxOrderAdvance[activeKey];
    if (r_advance) {
      size_t& max_r = max_rank();
      if  (max_r  < kickRank) bad_range = true;
      else max_r -= kickRank;
    }
    if (o_advance) {
      unsigned short& max_o = max_order();
      if  (max_o  < kickOrder) bad_range = true;
      else max_o -= kickOrder;
    }
    if (r_advance || o_advance) formUpdated[activeKey] = true;
    break;
  }
  }

  if (bad_range)
    Cerr << "Warning: SharedC3ApproxData::decrement_order() outside of valid "
	 << "range for key:\n" << activeKey << std::endl;
}


void SharedC3ApproxData::pre_combine()
{
  combinedOrders.assign(numVars, 0);
  std::map<Pecos::ActiveKey, UShortArray>::const_iterator cit;  size_t i;
  for (cit=startOrdersMap.begin(); cit!=startOrdersMap.end(); ++cit) {
    const UShortArray& so = cit->second;
    for (i=0; i<numVars; ++i)
      if (so[i] > combinedOrders[i])
	combinedOrders[i] = so[i];
  }

  update_basis(combinedOrders, max_order());
}


//void SharedC3ApproxData::post_combine()
//{ update_basis(); } // restore to active

} // namespace Dakota
