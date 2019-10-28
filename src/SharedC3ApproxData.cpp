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

SharedC3ApproxData::SharedC3ApproxData()
{
  this->constructed = 0;
  printf("In the default constructor\n");
}

// *** This ctor is not currently active:
  
SharedC3ApproxData::
SharedC3ApproxData(ProblemDescDB& problem_db, size_t num_vars):
  SharedApproxData(BaseConstructor(), problem_db, num_vars),
  startOrder(problem_db.get_sizet("model.c3function_train.start_order")),
  maxOrder(problem_db.get_sizet("model.c3function_train.max_order")),
  startRank(problem_db.get_sizet("model.c3function_train.start_rank")),
  kickRank(problem_db.get_sizet("model.c3function_train.kick_rank")),
  maxRank(problem_db.get_sizet("model.c3function_train.max_rank")),
  adaptRank(problem_db.get_bool("model.c3function_train.adapt_rank")),
  roundingTol(problem_db.get_real("model.c3function_train.rounding_tolerance")),
  solverTol(problem_db.get_real("model.c3function_train.solver_tolerance")),
  maxIterations(1000),//(problem_db.get_int("model.max_iterations")),
  crossMaxIter(
    problem_db.get_sizet("model.c3function_train.max_cross_iterations")),
  verbose(0)//problem_db.get_sizet("model.c3function_train.verbosity")),
{
  // printf("CONSTRUCTING SHAREDC3APPROX\n");

  // std::cout << "Solver Tolerance is " << solverTol << std::endl;
  this->approxOpts = multi_approx_opts_alloc(num_vars);
  this->oneApproxOpts = (struct OneApproxOpts **)
    malloc(num_vars * sizeof(struct OneApproxOpts *));
  for (size_t ii = 0; ii < num_vars; ii++){
    struct OpeOpts * opts = ope_opts_alloc(LEGENDRE);
    ope_opts_set_lb(opts,-2);
    ope_opts_set_ub(opts,2);
    ope_opts_set_nparams(opts,maxOrder+1); // maxnum = maxord + 1
    this->oneApproxOpts[ii] = one_approx_opts_alloc(POLYNOMIAL,opts);
    multi_approx_opts_set_dim(this->approxOpts,ii,this->oneApproxOpts[ii]);
    // this->oneApproxOpts[ii] = NULL;
    // multi_approx_opts_set_dim(this->approxOpts,ii,this->oneApproxOpts[ii]);
  }
  this->constructed = 1;
}

// *** This ctor is active due to lightweight DataFitSurrModel ctor:
  
SharedC3ApproxData::
SharedC3ApproxData(const String& approx_type,
		   const UShortArray& approx_order, size_t num_vars,
		   short data_order, short output_level):
  SharedApproxData(NoDBBaseConstructor(), approx_type, num_vars, data_order,
		   output_level)
{
  // short basis_type; approx_type_to_basis_type(approxType, basis_type);

  // printf("IN THIS CONSTRUCTOR\n");
  this->approxOpts = multi_approx_opts_alloc(num_vars);
  this->oneApproxOpts = (struct OneApproxOpts **)
    malloc(num_vars * sizeof(struct OneApproxOpts *));
  for (size_t ii = 0; ii < num_vars; ii++){
    this->oneApproxOpts[ii] = NULL;
  }
  this->constructed = 1;

  // default values overridden by set_parameter
  this->startOrder = 2;
  this->maxOrder = 4; // maxnum = 5

  this->startRank = 5;
  this->kickRank = 2;
  this->maxRank = 10;
  this->adaptRank = false;

  this->solverTol = 1e-10;
  this->roundingTol = 1e-10;
  this->maxIterations = 1000;
  this->crossMaxIter = 5;
  this->verbose = 0;
}

SharedC3ApproxData::~SharedC3ApproxData()
{
  multi_approx_opts_free(this->approxOpts); this->approxOpts = NULL;

  for (size_t ii = 0; ii < this->numVars; ii++){
    one_approx_opts_free_deep(&(this->oneApproxOpts[ii]));
    this->oneApproxOpts[ii] = NULL;
  }
  free(this->oneApproxOpts); this->oneApproxOpts = NULL;
}

void SharedC3ApproxData::set_parameter(String var, void * val)
{
  if (var.compare("start_poly_order") == 0)
    this->startOrder= *(size_t*)val;
  else if (var.compare("max_poly_order") == 0)
    this->maxOrder = *(size_t*)val;
  else if (var.compare("start_rank") == 0)
    this->startRank = *(size_t*)val;
  else if (var.compare("kick_rank") == 0)
    this->kickRank = *(size_t*)val;
  else if (var.compare("max_rank") == 0)
    this->maxRank = *(size_t*)val;
  else if (var.compare("adapt_rank") == 0)
    this->adaptRank = *(bool*)val;
  else if (var.compare("solver_tol") == 0)
    this->solverTol = *(double*)val;
  else if (var.compare("rounding_tol") == 0)
    this->roundingTol = *(double*)val;
  else if (var.compare("max_cross_iterations") == 0)
    this->crossMaxIter = *(size_t*)val;
  else if (var.compare("max_iterations") == 0)
    this->maxIterations = *(size_t*)val;
  else if (var.compare("verbose") == 0)
    this->verbose = *(size_t*)val;
  else
    std::cerr << "Unrecognized approximation parameter: " << var << std::endl;
}

void SharedC3ApproxData::
construct_basis(const Pecos::MultivariateDistribution& u_dist)
{
  // printf("constructed = %d\n",this->constructed);
  const ShortArray& u_types = u_dist.random_variable_types();
  assert (u_types.size() == this->numVars);

  for (size_t i=0; i < this->numVars; ++i){
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
    ope_opts_set_nparams(opts,this->maxOrder+1);
    one_approx_opts_free_deep(&(this->oneApproxOpts[i]));
    this->oneApproxOpts[i] = one_approx_opts_alloc(POLYNOMIAL,opts);
    // printf("set i\n");
    multi_approx_opts_set_dim(this->approxOpts,i,this->oneApproxOpts[i]);
  }
  // do nothing
}

void SharedC3ApproxData::store(size_t index)
{
  size_t stored_len = storeOne.size();
  if (index == _NPOS || index == stored_len) { // append
    storeOne.push_back(this->oneApproxOpts);
    storeMulti.push_back(this->approxOpts);
  }
  else if (index < stored_len) { // replace
    storeOne[index] = this->oneApproxOpts;
    storeMulti[index] = this->approxOpts;
  }

}
    
size_t SharedC3ApproxData::pre_combine(short combine_type)
{
  (void) (combine_type);
  return 0;
}

void SharedC3ApproxData::post_combine(short combine_type)
{
  (void) (combine_type);
}

void SharedC3ApproxData::restore(size_t index)
{
  (void) (index);        
}

void SharedC3ApproxData::remove_stored(size_t index)
{
  (void)(index);
}

} // namespace Dakota
