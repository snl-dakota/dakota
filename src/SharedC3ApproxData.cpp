/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SharedC3ApproxData
//- Description:  Implementation code for SharedC3ApproxData class
//-               
//- Owner:        Mike Eldred

#include "SharedC3ApproxData.hpp"
#include "ProblemDescDB.hpp"
#include "NonDIntegration.hpp"

#include "pecos_stat_util.hpp"
#include "pecos_global_defs.hpp"

#include <assert.h>
//#define DEBUG

namespace Dakota {


SharedC3ApproxData::
SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars),
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
  maxSolverIterations(problem_db.get_int("model.max_solver_iterations")),
  crossMaxIter(
    problem_db.get_int("model.c3function_train.max_cross_iterations")),
  //adaptConstruct(false),
  c3RefineType(NO_C3_REFINEMENT)
{
  // This ctor used for user-spec of DataFitSurrModel (surrogate global FT
  // used by generic surrogate-based UQ in NonDSurrogateExpansion)

  RealVector dim_pref_spec; // isotropic for now, prior to XML support
  NonDIntegration::dimension_preference_to_anisotropic_order(
    problem_db.get_ushort("model.c3function_train.start_order"),
    dim_pref_spec,//problem_db.get_rv("model.dimension_preference"),
    numVars, startOrders);

  multiApproxOpts = multi_approx_opts_alloc(num_vars);

  //oneApproxOpts = (struct OneApproxOpts **)
  //  malloc(num_vars * sizeof(struct OneApproxOpts *));
  //for (size_t i=0; i<num_vars; ++i)
  //  oneApproxOpts[i] = NULL;
  oneApproxOpts.assign(num_vars, NULL);
}


SharedC3ApproxData::
SharedC3ApproxData(const String& approx_type, const UShortArray& approx_order,
		   size_t num_vars, short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level),
  // default values overridden by set_parameter
  startOrders(approx_order), kickOrder(1), maxOrder(USHRT_MAX),
  adaptOrder(false), startRank(2), kickRank(1),
  maxRank(std::numeric_limits<size_t>::max()), adaptRank(false),
  regressType(FT_LS), // non-regularized least sq
  solverTol(1.e-10), solverRoundingTol(1.e-10), statsRoundingTol(1.e-10),
  maxSolverIterations(-1), crossMaxIter(5), //adaptConstruct(false),
  c3RefineType(NO_C3_REFINEMENT)
{
  // This ctor used by lightweight/on-the-fly DataFitSurrModel ctor
  // (used to build an FT on top of a user model in NonDC3FuntionTrain)

  // short basis_type; approx_type_to_basis_type(approxType, basis_type);

  multiApproxOpts = multi_approx_opts_alloc(num_vars);

  //oneApproxOpts = (struct OneApproxOpts **)
  //  malloc(num_vars * sizeof(struct OneApproxOpts *));
  //for (size_t i=0; i<num_vars; ++i)
  //  oneApproxOpts[i] = NULL;
  oneApproxOpts.assign(num_vars, NULL);
}


SharedC3ApproxData::~SharedC3ApproxData()
{
  multi_approx_opts_free(multiApproxOpts); multiApproxOpts = NULL;
  for (size_t i=0; i<numVars; ++i) {
    struct OneApproxOpts*& a_opts = oneApproxOpts[i]; // ref to ptr
    if (a_opts) { one_approx_opts_free_deep(&a_opts); a_opts = NULL; }
  }
  //free(oneApproxOpts); oneApproxOpts = NULL;
}


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

  //formUpdated[activeKey] = true; // elevate to clients
}


void SharedC3ApproxData::increment_order()
{
  switch (c3RefineType) {
  case UNIFORM_START_ORDER: {
    UShortArray& start_ord = start_orders();
    unsigned short max_ord = max_order(); // default is USHRT_MAX
    bool incremented = false;
    for (size_t v=0; v<numVars; ++v) {
      unsigned short &s_ord = start_ord[v], prev_ord = s_ord;
      // unconditional increment (preserve reproducibility w/decrement)
      // Note: dim_pref ratios are not preserved
      s_ord += kickOrder;
      if (prev_ord < max_ord) { // increment occurs, but kick may be truncated
	incremented = true;
	update_basis(v, std::min(s_ord, max_ord), max_ord);
      }
    }
    if (incremented)
      formUpdated[activeKey] = true;
    else
      Cerr << "Warning: SharedC3ApproxData::increment_order() cannot advance "
	   << "order beyond maximum allowable (" << max_ord << ") for key:\n"
	   << activeKey << std::endl;
    break;
  }
  case UNIFORM_START_RANK: {
    // To ensure symmetry with decrement, don't saturate at maxRank
    // > Must bound start_ranks vector in C3Approximation::build()
    // > build() truncates kick to max: start_ranks = std::min(start_r, max_r) 
    size_t &start_r = start_rank();
    if (start_r < max_rank()) // increment occurs but kick might be truncated
      formUpdated[activeKey] = true;
    start_r += kickRank; // invertible in decrement_order()
    break;
  }
  case UNIFORM_MAX_RANK: {
    size_t& max_r = max_rank();     max_r += kickRank;
    formUpdated[activeKey] = true;  break;
  }
  case UNIFORM_MAX_ORDER: {
    unsigned short& max_o = max_order();  max_o += kickOrder;
    update_basis(start_orders(), max_o);  formUpdated[activeKey] = true;  break;
  }
  case UNIFORM_MAX_RANK_ORDER: {
    // prior to implementing a multi-index approach, advance both...
    size_t&         max_r = max_rank();   max_r += kickRank;
    unsigned short& max_o = max_order();  max_o += kickOrder;
    update_basis(start_orders(), max_o);  formUpdated[activeKey] = true;
    break;
  }
  }
}


void SharedC3ApproxData::decrement_order()
{
  bool bad_range = false;
  switch (c3RefineType) {
  case UNIFORM_START_ORDER: {
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
	  { update_basis(v, s_ord, max_ord); decremented = true; }
      }
    }
    if (!decremented) bad_range = true;
    break;
  }
  case UNIFORM_START_RANK: {
    size_t& start_r = start_rank();
    if  (start_r  < kickRank)  bad_range = true;// underflow (should not happen)
    else start_r -= kickRank;
    break;
  }
  case UNIFORM_MAX_RANK: {
    size_t& max_r = max_rank();
    if  (max_r  < kickRank)  bad_range = true;  // underflow (should not happen)
    else max_r -= kickRank;
    break;
  }
  case UNIFORM_MAX_ORDER: {
    unsigned short& max_o = max_order();
    if    (max_o  < kickOrder)  bad_range = true;
    else { max_o -= kickOrder;  update_basis(start_orders(), max_o); }
    break;
  }
  case UNIFORM_MAX_RANK_ORDER: {
    size_t& max_r = max_rank();  unsigned short& max_o = max_order();
    if (max_r < kickRank || max_o < kickOrder) bad_range = true;
    else {
      max_r -= kickRank;
      max_o -= kickOrder;  update_basis(start_orders(), max_o);
    }
    break;
  }
  }

  if (bad_range)
    Cerr << "Warning: SharedC3ApproxData::decrement_order() outside of valid "
	   << "range for key:\n" << activeKey << std::endl;
  else
    formUpdated[activeKey] = true;    
}


void SharedC3ApproxData::pre_combine()
{
  combinedOrders.assign(numVars, 0);
  std::map<UShortArray, UShortArray>::const_iterator cit;  size_t i;
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
