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
  startOrder(problem_db.get_sizet("model.c3function_train.start_order")),
  maxOrder(problem_db.get_sizet("model.c3function_train.max_order")),
  startRank(problem_db.get_sizet("model.c3function_train.start_rank")),
  kickRank(problem_db.get_sizet("model.c3function_train.kick_rank")),
  maxRank(problem_db.get_sizet("model.c3function_train.max_rank")),
  adaptRank(problem_db.get_bool("model.c3function_train.adapt_rank")),
  regressType(problem_db.get_short("model.surrogate.regression_type")),
  regressRegParam(problem_db.get_real("model.surrogate.regression_penalty")),
  roundingTol(problem_db.get_real("model.c3function_train.rounding_tolerance")),
  solverTol(problem_db.get_real("model.c3function_train.solver_tolerance")),
  maxSolverIterations(problem_db.get_int("model.max_solver_iterations")),
  crossMaxIter(
    problem_db.get_int("model.c3function_train.max_cross_iterations")),
  c3Verbosity(0),//problem_db.get_int("model.c3function_train.verbosity")),
  adaptConstruct(false), crossVal(false)
{
  // This ctor used for user-spec of DataFitSurrModel (surrogate global ft)

  approxOpts = multi_approx_opts_alloc(num_vars);
  oneApproxOpts = (struct OneApproxOpts **)
    malloc(num_vars * sizeof(struct OneApproxOpts *));
  for (size_t ii = 0; ii < num_vars; ii++){
    struct OpeOpts * opts = ope_opts_alloc(LEGENDRE);
    ope_opts_set_lb(opts,-2); // BUG?
    ope_opts_set_ub(opts, 2); // BUG?
    ope_opts_set_nparams(opts,startOrder+1); // startnum = startord + 1
    // Note: maxOrder unused for regression;
    //       to be used for adaptation by cross-approximation
    ope_opts_set_maxnum(opts,maxOrder+1);    //   maxnum =   maxord + 1
    oneApproxOpts[ii] = one_approx_opts_alloc(POLYNOMIAL,opts);
    multi_approx_opts_set_dim(approxOpts,ii,oneApproxOpts[ii]);
    // oneApproxOpts[ii] = NULL;
    // multi_approx_opts_set_dim(approxOpts,ii,oneApproxOpts[ii]);
  }
}

  
SharedC3ApproxData::
SharedC3ApproxData(const String& approx_type,
		   const UShortArray& approx_order, size_t num_vars,
		   short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level),
  // default values overridden by set_parameter
  startOrder(2), maxOrder(4), //maxnum(5),
  startRank(5), kickRank(2), maxRank(10), adaptRank(false),
  regressType(FT_LS), // non-regularized least sq
  solverTol(), roundingTol(1.e-10), crossMaxIter(5), maxSolverIterations(1000),
  c3Verbosity(0), adaptConstruct(false), crossVal(false)
{
  // This ctor used by lightweight/on-the-fly DataFitSurrModel ctor

  // short basis_type; approx_type_to_basis_type(approxType, basis_type);

  approxOpts = multi_approx_opts_alloc(num_vars);
  oneApproxOpts = (struct OneApproxOpts **)
    malloc(num_vars * sizeof(struct OneApproxOpts *));
  for (size_t ii = 0; ii < num_vars; ii++)
    oneApproxOpts[ii] = NULL;
}


SharedC3ApproxData::~SharedC3ApproxData()
{
  multi_approx_opts_free(approxOpts); approxOpts = NULL;

  for (size_t i=0; i<numVars; ++i) {
    one_approx_opts_free_deep(&oneApproxOpts[i]);
    oneApproxOpts[i] = NULL;
  }
  free(oneApproxOpts); oneApproxOpts = NULL;
}


void SharedC3ApproxData::
construct_basis(const Pecos::MultivariateDistribution& u_dist)
{
  const ShortArray& u_types = u_dist.random_variable_types();
  assert (u_types.size() == numVars);

  for (size_t i=0; i < numVars; ++i) {
    // printf("i = %zu\n",i);
    struct OpeOpts * opts = NULL;
    switch (u_types[i]) {
    case Pecos::STD_NORMAL:
      opts = ope_opts_alloc(HERMITE);
      break;
    case Pecos::STD_UNIFORM:
      opts = ope_opts_alloc(LEGENDRE);
      break;
    default:
      PCerr << "Error: unsupported u-space type (" << u_types[i] << ") in "
	    << "SharedC3ApproxData::distribution_parameters()" << std::endl;
      abort_handler(-1);
      break;
    }
    // printf("push_back\n");
    ope_opts_set_nparams(opts,startOrder+1); // startnum = startord + 1
    // Note: maxOrder unused for regression;
    //       to be used for adaptation by cross-approximation
    ope_opts_set_maxnum(opts,maxOrder+1);    //   maxnum =   maxord + 1
    one_approx_opts_free_deep(&oneApproxOpts[i]);
    oneApproxOpts[i] = one_approx_opts_alloc(POLYNOMIAL,opts);
    // printf("set i\n");
    multi_approx_opts_set_dim(approxOpts,i,oneApproxOpts[i]);
  }
}

    
size_t SharedC3ApproxData::pre_combine(short combine_type)
{
  Cerr << "Error: SharedC3ApproxData::pre_combine() not yet implemented."
       << std::endl;
  abort_handler(APPROX_ERROR);
  return 0;
}


void SharedC3ApproxData::post_combine(short combine_type)
{
  Cerr << "Error: SharedC3ApproxData::post_combine() not yet implemented."
       << std::endl;
  abort_handler(APPROX_ERROR);
}

} // namespace Dakota
