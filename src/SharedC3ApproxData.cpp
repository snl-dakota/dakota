/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
  maxOrder(problem_db.get_ushort("model.c3function_train.max_order")),
  startRankSpec(problem_db.get_sizet("model.c3function_train.start_rank")),
  kickRank(problem_db.get_sizet("model.c3function_train.kick_rank")),
  maxRankSpec(problem_db.get_sizet("model.c3function_train.max_rank")),
  adaptRank(problem_db.get_bool("model.c3function_train.adapt_rank")),
  regressType(problem_db.get_short("model.surrogate.regression_type")),
  regressRegParam(problem_db.get_real("model.surrogate.regression_penalty")),
  solverTol(problem_db.get_real("model.c3function_train.solver_tolerance")),
  roundingTol(problem_db.get_real("model.c3function_train.rounding_tolerance")),
  arithmeticTol(
    problem_db.get_real("model.c3function_train.arithmetic_tolerance")),
  maxSolverIterations(problem_db.get_int("model.max_solver_iterations")),
  crossMaxIter(
    problem_db.get_int("model.c3function_train.max_cross_iterations")),
  c3Verbosity(0),//problem_db.get_int("model.c3function_train.verbosity")),
  adaptConstruct(false), crossVal(false), c3RefineType(UNIFORM_START_ORDER)
{
  // This ctor used for user-spec of DataFitSurrModel (surrogate global FT
  // used by generic surrogate-based UQ in NonDSurrogateExpansion)

  RealVector dim_pref_spec; // isotropic for now, prior to XML support
  NonDIntegration::dimension_preference_to_anisotropic_order(
    problem_db.get_ushort("model.c3function_train.start_order"),
    dim_pref_spec,//problem_db.get_rv("model.dimension_preference"),
    numVars, startOrderSpec);

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
  startOrderSpec(approx_order), maxOrder(5),
  startRankSpec(2), kickRank(2), maxRankSpec(10), adaptRank(false),
  regressType(FT_LS), // non-regularized least sq
  solverTol(1.e-10), roundingTol(1.e-8), arithmeticTol(1.e-2),
  crossMaxIter(5), maxSolverIterations(-1), c3Verbosity(0),
  adaptConstruct(false), crossVal(false), c3RefineType(UNIFORM_START_ORDER)
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
  // uses startOrderSpec at construct time (initialize_u_space_model()) since
  // startOrder[activeKey] not meaningful until run time active key assignment
  const UShortArray& so = start_orders();
  size_t np, max_np = maxOrder + 1;
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

      np = so[i] + 1;
      ope_opts_set_nparams(o_opts, np);     // startnum = startord + 1
      // Note: maxOrder not used for regression (only limits increment_order());
      //       to be used for adaptation by cross-approximation
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
  // use startOrder[activeKey] for run time updates
  size_t np, max_np = max_order + 1;
  for (size_t i=0; i<numVars; ++i) {
    struct OneApproxOpts*& a_opts = oneApproxOpts[i];
    np = start_orders[i] + 1;
    one_approx_opts_set_nparams(a_opts, np);     // updated
    one_approx_opts_set_maxnum( a_opts, max_np); // not currently updated
  }

  formUpdated[activeKey] = true;
}


void SharedC3ApproxData::increment_order()
{
  switch (c3RefineType) {
  case UNIFORM_START_ORDER: {
    std::map<UShortArray, UShortArray>::iterator it
      = startOrders.find(activeKey);
    UShortArray& active_ord = it->second;  bool incremented = false;
    UShortArray  update_ord = active_ord; // copy
    for (size_t i=0; i<numVars; ++i) {
      unsigned short &a_ord = active_ord[i], &u_ord = update_ord[i];
      // default maxOrder is 10 (FT can be more conservative than PCE).
      // could consider a kickOrder (like kickRank), but other advancements
      // (regression PCE) use exp order increment of 1
      if (a_ord < maxOrder)
	{ ++u_ord; incremented = true; } // only communicate if in bounds
      else
	u_ord = maxOrder;
      // unconditional increment (preserve symmetry/reproducibility w/decrement)
      ++a_ord;
    }
    if (incremented)
      update_basis(update_ord, maxOrder);
    else
      Cerr << "Warning: SharedC3ApproxData::increment_order() cannot advance "
	   << "order beyond maxOrder for key:\n" << activeKey << std::endl;
    break;
  }
  case UNIFORM_START_RANK: {
    // To ensure symmetry with decrement, don't saturate at maxRank.
    // *** --> Must bound in C3Approximation::build() ***
    std::map<UShortArray, size_t>::iterator it = startRank.find(activeKey);
    ++it->second; break;
  }
  case UNIFORM_MAX_RANK: {
    std::map<UShortArray, size_t>::iterator it =   maxRank.find(activeKey);
    ++it->second; break;
  }
  }
}


void SharedC3ApproxData::decrement_order()
{
  // TO DO: this does not recover previous startOrder that has been limited
  // above by maxOrder (only matters if anisotropic dimPref).
  // > could increment/decrement shared data & only update_basis() if w/i bounds

  bool underflow = false;
  switch (c3RefineType) {
  case UNIFORM_START_ORDER: {
    std::map<UShortArray, UShortArray>::iterator it
      = startOrders.find(activeKey);
    UShortArray& active_ord = it->second;  bool decremented = false;
    UShortArray  update_ord = active_ord; // copy
    for (size_t i=0; i<numVars; ++i) {
      unsigned short &a_ord = active_ord[i], &u_ord = update_ord[i];
      if (a_ord <= maxOrder)
	{ --u_ord; decremented = true; } // only communicate if in bounds
      else
	u_ord = maxOrder;
      // unconditional decrement (preserve symmetry/reproducibility w/increment)
      if (a_ord)    --a_ord;
      else underflow = true;
    }
    if (decremented && !underflow)
      update_basis(update_ord, maxOrder);
    break;
  }
  case UNIFORM_START_RANK: {
    std::map<UShortArray, size_t>::iterator it = startRank.find(activeKey);
    size_t& s_rank = it->second;  
    if (s_rank)  --s_rank;
    else underflow = true;
    break;
  }
  case UNIFORM_MAX_RANK:
    std::map<UShortArray, size_t>::iterator it =   maxRank.find(activeKey);
    size_t& m_rank = it->second;  
    if (m_rank)  --m_rank;
    else underflow = true;
    break;
  }

  if (underflow) {
    Cerr << "Error: underflow in SharedC3ApproxData::decrement_order()."
	 << std::endl;
    abort_handler(APPROX_ERROR);
  }
}


void SharedC3ApproxData::pre_combine()
{
  combinedOrders.assign(numVars, 0);
  std::map<UShortArray, UShortArray>::const_iterator cit;  size_t i;
  for (cit=startOrders.begin(); cit!=startOrders.end(); ++cit) {
    const UShortArray& so = cit->second;
    for (i=0; i<numVars; ++i)
      if (so[i] > combinedOrders[i])
	combinedOrders[i] = so[i];
  }

  update_basis(combinedOrders, maxOrder);
}


//void SharedC3ApproxData::post_combine()
//{ update_basis(); } // restore to active

} // namespace Dakota
