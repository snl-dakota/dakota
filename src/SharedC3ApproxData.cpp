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
  solverRoundingTol(
    problem_db.get_real("model.c3function_train.rounding_tolerance")),
  statsRoundingTol(
    problem_db.get_real("model.c3function_train.arithmetic_tolerance")),
  maxSolverIterations(problem_db.get_int("model.max_solver_iterations")),
  crossMaxIter(
    problem_db.get_int("model.c3function_train.max_cross_iterations")),
  adaptConstruct(false), crossVal(false), c3RefineType(NO_C3_REFINEMENT)
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
  startOrderSpec(approx_order), maxOrder(USHRT_MAX), startRankSpec(2),
  kickRank(1), maxRankSpec(std::numeric_limits<size_t>::max()),
  adaptRank(false), regressType(FT_LS), // non-regularized least sq
  solverTol(1.e-10), solverRoundingTol(1.e-8), statsRoundingTol(1.e-8),
  crossMaxIter(5), maxSolverIterations(-1), adaptConstruct(false),
  crossVal(false), c3RefineType(NO_C3_REFINEMENT)
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
  size_t np, max_np = maxOrder; ++max_np; // no overflow (default is USHRT_MAX)
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
  size_t np, max_np = max_order; ++max_np; // no overflow (default is USHRT_MAX)
  for (size_t v=0; v<numVars; ++v) {
    struct OneApproxOpts*& a_opts = oneApproxOpts[v];
    np = std::min((size_t)start_orders[v] + 1, max_np);
    one_approx_opts_set_nparams(a_opts, np);     // updated
    one_approx_opts_set_maxnum( a_opts, max_np); // not currently updated
  }

  formUpdated[activeKey] = true;
}


void SharedC3ApproxData::
update_basis(size_t v, unsigned short start_order, unsigned short max_order)
{
  // use startOrder[activeKey] for run time updates
  size_t max_np = max_order; ++max_np; // no overflow (default is USHRT_MAX)
  size_t np = std::min((size_t)start_order + 1, max_np);

  struct OneApproxOpts*& a_opts = oneApproxOpts[v];
  one_approx_opts_set_nparams(a_opts, np);     // updated
  one_approx_opts_set_maxnum( a_opts, max_np); // not currently updated

  //formUpdated[activeKey] = true; // elevate to clients
}


void SharedC3ApproxData::increment_order()
{
  switch (c3RefineType) {
  case UNIFORM_START_ORDER: {
    std::map<UShortArray, UShortArray>::iterator it
      = startOrders.find(activeKey);
    UShortArray& start_ord = it->second;  bool incremented = false;
    for (size_t v=0; v<numVars; ++v) {
      unsigned short &s_ord = start_ord[v];
      // unconditional increment (preserve symmetry/reproducibility w/decrement)
      ++s_ord; //s_ord += kickOrder;
      // default maxOrder is USHRT_MAX.  kickOrder not used since other exp
      // order increments (i.e., regression PCE) advance by 1
      if (s_ord <= maxOrder) // only communicate if in bounds
	{ incremented = true; update_basis(v, s_ord, maxOrder); }
    }
    if (incremented)
      formUpdated[activeKey] = true;
    else
      Cerr << "Warning: SharedC3ApproxData::increment_order() cannot advance "
	   << "order beyond maxOrder for key:\n" << activeKey << std::endl;
    break;
  }
  case UNIFORM_START_RANK: {
    // To ensure symmetry with decrement, don't saturate at maxRank
    // > Must bound start_ranks vector in C3Approximation::build()
    std::map<UShortArray, size_t>::iterator it = startRank.find(activeKey);
    it->second += kickRank;  formUpdated[activeKey] = true;  break;
  }
  case UNIFORM_MAX_RANK: {
    std::map<UShortArray, size_t>::iterator it =   maxRank.find(activeKey);
    it->second += kickRank;  formUpdated[activeKey] = true;  break;
  }
  }
}


void SharedC3ApproxData::decrement_order()
{
  // TO DO: this does not recover previous startOrder that has been limited
  // above by maxOrder (only matters if anisotropic dimPref).
  // > could increment/decrement shared data & only update_basis() if w/i bounds

  bool bad_range = false;
  switch (c3RefineType) {
  case UNIFORM_START_ORDER: {
    std::map<UShortArray, UShortArray>::iterator it
      = startOrders.find(activeKey);
    UShortArray& start_ord = it->second;  bool decremented = false;
    for (size_t v=0; v<numVars; ++v) {
      unsigned short &s_ord = start_ord[v];
      if (s_ord) { // prevent underflow
	--s_ord; // preserve symmetry/reproducibility w/increment
	if (s_ord < maxOrder) // only communicate if in bounds
	  { update_basis(v, s_ord, maxOrder); decremented = true; }
      }
    }
    if (decremented) formUpdated[activeKey] = true;
    else             bad_range = true;
    break;
  }
  case UNIFORM_START_RANK: {
    std::map<UShortArray, size_t>::iterator it = startRank.find(activeKey);
    size_t& s_rank = it->second;  
    if (s_rank) { s_rank -= kickRank; formUpdated[activeKey] = true; }
    else          bad_range = true;
    break;
  }
  case UNIFORM_MAX_RANK:
    std::map<UShortArray, size_t>::iterator it =   maxRank.find(activeKey);
    size_t& m_rank = it->second;  
    if (m_rank) { m_rank -= kickRank; formUpdated[activeKey] = true; }
    else          bad_range = true;
    break;
  }

  if (bad_range)
    Cerr << "Warning: SharedC3ApproxData::decrement_order() outside of valid "
	   << "range for key:\n" << activeKey << std::endl;
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


size_t SharedC3ApproxData::max_order_regression_size()
{
  unsigned short max_o = max_order();
  UShortArray max_orders; RealVector dim_pref;// isotropic for now (no XML spec)
  NonDIntegration::dimension_preference_to_anisotropic_order(max_o,
    dim_pref, numVars, max_orders);
  return regression_size(numVars, start_rank(), max_rank(), max_orders, max_o);
}


size_t SharedC3ApproxData::max_regression_size()
{
  size_t max_r = max_rank();  unsigned short max_o = max_order();
  UShortArray max_orders; RealVector dim_pref;// isotropic for now (no XML spec)
  NonDIntegration::dimension_preference_to_anisotropic_order(max_o,
    dim_pref, numVars, max_orders);
  return regression_size(numVars, max_r, max_r, max_orders, max_o);
}

} // namespace Dakota
