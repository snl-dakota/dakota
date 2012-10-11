/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       COLINApplication
//- Description: Implementation of the COLINApplication class, a
//-              specialized application class derived from COLIN's
//-              application class which redefines virtual evaluation
//-              functions with DAKOTA's response computation
//-              procedures.
//- Owner:       Patty Hough/John Siirola/Brian Adams
//- Checked by:
//- Version: $Id: COLINApplication.C 6829 2010-06-18 23:10:23Z pdhough $
//

#include "system_defs.h"
#include "DakotaModel.H"

#include <utilib/TypeManager.h>
using utilib::TypeManager;
#include <utilib/Any.h>
using utilib::Any;
#include <utilib/MixedIntVars.h>
using utilib::MixedIntVars;


#include <colin/AppResponseInfo.h>
using colin::mf_info;
using colin::nlcf_info;

#include <colin/AppRequest.h>
using colin::AppRequest;
#include <colin/AppResponse.h>
using colin::AppResponse;

using std::make_pair;

// implementation of COLINApplication class

#include "COLINApplication.H"

namespace Dakota {

  // Note that blockingSynch is not currently being used.  There are
  // two reasons: 1) synchronize_nowait() does not work when DAKOTA is
  // run using MPI, and 2) there is not a mechanism to set/use it in
  // COLIN 3.  We will make use of blockingSynch again after these
  // issues are resolved.

COLINApplication::COLINApplication(Model& model) :
  blockingSynch(0)
{ set_problem(model);}

  /** Set variable bounds and linear and nonlinear constraints.  This
      avoids using probDescDB, so it is called by both the standard
      and the on-the-fly COLINOptimizer constructors.*/

void COLINApplication::set_problem(Model& model) {

  iteratedModel = model;
  activeSet = model.current_response().active_set();

  // BMA NOTE: For COLIN to relax integer variables, must hand COLIN a
  // relaxed problem with continuous vars; see
  // Application_IntDomain::relaxed_application

  // Get the upper and lower bounds on the continuous variables.

  _num_real_vars = model.cv();
  if (num_real_vars > 0)
  {
    _real_lower_bounds = model.continuous_lower_bounds();
    _real_upper_bounds = model.continuous_upper_bounds();
  }

  // One specification type for discrete variables is a set of values.
  // Get that list of values if the user provided one.  Also,
  // determine the size of those sets and the number of non-set
  // integer variables.

  const IntSetArray& ddsiv_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsrv_values
    = iteratedModel.discrete_design_set_real_values();
  size_t i, num_ddsiv = ddsiv_values.size(), num_ddsrv = ddsrv_values.size(),
    num_ddrv = model.div() - num_ddsiv;

  // Get the upper and lower bounds on the discrete variables.

  _num_int_vars = model.div()+model.drv();
  if (num_int_vars > 0)
  {
    const IntVector& lower_bnds = iteratedModel.discrete_int_lower_bounds();
    const IntVector& upper_bnds = iteratedModel.discrete_int_upper_bounds();

    // Need temporary storage in which to consolidate all types of
    // discrete variables;

    IntVector lower(model.div()+model.drv());
    IntVector upper(model.div()+model.drv());

    // For non-set integer variables, just assign the values to
    // temporary storage.

    for (i=0; i<num_ddrv; ++i) {
      lower[i] = lower_bnds[i];
      upper[i] = upper_bnds[i];
    }

    // For integer set variables, map to integer index sequence
    // [0,num_items-1].

    size_t offset = num_ddrv;
    for (i=0; i<num_ddsiv; ++i) {
      lower[i+offset] = 0;
      upper[i+offset] = ddsiv_values[i].size() - 1;
    }

    // For real set variables, map to integer index sequence
    // [0,num_items-1].

    offset += num_ddsiv;
    for (i=0; i<num_ddsrv; ++i) {
      lower[i+offset] = 0;
      upper[i+offset] = ddsrv_values[i].size() - 1;
    }

    // Now assign to the COLIN vectors.

    _int_lower_bounds = lower;
    _int_upper_bounds = upper;
  }

  _num_nonlinear_constraints = model.num_nonlinear_ineq_constraints() +
    model.num_nonlinear_eq_constraints();

  // For multiobjective, this will be taken from the RecastModel and
  // will be consistent with the COLIN iterator's view

  _num_objectives = 
    model.num_functions() - num_nonlinear_constraints.as<size_t>();

  // Assemble nonlinear inequality constraints followed by equality
  // constraints.

  if (num_nonlinear_constraints > 0) {

    // Need temporary storage in which to consolidate inequality
    // bounds and equality targets.

    RealVector bounds(num_nonlinear_constraints.as<size_t>());

    // Get the upper and lower bounds for the inequalities and the
    // targets for the equalities.

    const RealVector& ineq_lower
      = model.nonlinear_ineq_constraint_lower_bounds();
    const RealVector& ineq_upper
      = model.nonlinear_ineq_constraint_upper_bounds();
    const RealVector& eq_targets
      = model.nonlinear_eq_constraint_targets();

    // Lower bounds and targets go together in COLIN lower bounds.

    for (size_t i=0; i<model.num_nonlinear_ineq_constraints(); i++)
      bounds[i] = ineq_lower[i];

    size_t ndx = model.num_nonlinear_ineq_constraints();
    for (size_t i=0; i<model.num_nonlinear_eq_constraints(); i++, ndx++)
      bounds[ndx] = eq_targets[i];

    _nonlinear_constraint_lower_bounds = bounds;

    // Lower bounds and targets go together in COLIN upper bounds.

    for (size_t i=0; i<model.num_nonlinear_ineq_constraints(); i++)
      bounds[i] = ineq_upper[i];

    ndx = model.num_nonlinear_ineq_constraints();
    for (size_t i=0; i<model.num_nonlinear_eq_constraints(); i++, ndx++)
      bounds[ndx] = eq_targets[i];

    _nonlinear_constraint_upper_bounds = bounds;
  }

  // Assemble linear inequality constraints followed by equality
  // constraints.  Linear constraints are implemented but not tested.
  // User beware.  Need to confirm which algorithms do and don't
  // handle linear constraints and how.

  _num_linear_constraints = model.num_linear_ineq_constraints() +
    model.num_linear_eq_constraints();

  if (num_linear_constraints > 0) {

    RealMatrix linear_coeffs( num_linear_constraints.as<size_t>(), 
                              domain_size.as<size_t>() );

    const RealMatrix& linear_ineq_coeffs = iteratedModel.linear_ineq_constraint_coeffs();
    const RealMatrix& linear_eq_coeffs = iteratedModel.linear_eq_constraint_coeffs();

    // Populate the coefficient matrix, first with inequality
    // coefficients, then with equality coefficients.

    size_t ndx = 0;
    size_t ndy = 0;
    for (int i=0; i<model.num_linear_ineq_constraints(); i++) {
      for (int j=0; domain_size>j; j++)
	linear_coeffs[ndx][ndy++] = linear_ineq_coeffs[i][j];
      ndx++;
    }

    for (int i=0; i<model.num_linear_eq_constraints(); i++) {
      for (int j=0; domain_size>j; j++)
	linear_coeffs[ndx][ndy++] = linear_eq_coeffs[i][j];
      ndx++;
    }
 
   _linear_constraint_matrix = linear_coeffs;

   RealVector bounds(num_linear_constraints.as<size_t>());

   const RealVector& lin_ineq_lower
     = model.linear_ineq_constraint_lower_bounds();
   const RealVector& lin_ineq_upper
     = model.linear_ineq_constraint_upper_bounds();
   const RealVector& lin_eq_targets
     = model.linear_eq_constraint_targets();

   // Lower bounds and equality targets go together in COLIN lower
   // bounds.

   for (size_t i=0; i<model.num_linear_ineq_constraints(); i++) 
     bounds[i] = lin_ineq_lower[i];

   ndx = model.num_linear_ineq_constraints();
   for (size_t i=0; i<model.num_linear_eq_constraints(); i++, ndx++)
     bounds[ndx] = lin_eq_targets[i];

   _linear_constraint_lower_bounds = bounds;

   // Upper bounds and equality targets go together in COLIN upper
   // bounds.

   for (size_t i=0; i<model.num_linear_ineq_constraints(); i++) 
     bounds[i] = lin_ineq_upper[i];

   ndx = model.num_linear_ineq_constraints();
   for (size_t i=0; i<model.num_linear_eq_constraints(); i++, ndx++)
     bounds[ndx] = lin_eq_targets[i];

   _linear_constraint_upper_bounds = bounds;
  }
}

/** Schedule one or more requests at specified domain point, returning
    a DAKOTA-specific evaluation tracking ID.  This is only called by
    COLIN's concurrent evaluator, which is only instantiated when the
    Model supports asynch evals.  The domain point is guaranteed to be
    compatible with data type specified by map_domain(...) */

utilib::Any COLINApplication::
spawn_evaluation_impl(const utilib::Any &domain,
		 const colin::AppRequest::request_map_t &requests,
		 utilib::seed_t &seed)
{
  // Transform COLIN request to point for which DAKOTA executes an
  // evaluation, launch the evaluation, and return the DAKOTA ID.

  colin_request_to_dakota_request(domain, requests, seed);

  iteratedModel.asynch_compute_response(activeSet);

  return(iteratedModel.evaluation_id());
}

/** Perform an evaluation at a specified domain point.  Wait for and
    return the response.  This is only called by COLIN's serial
    evaluator, which is only instantiated when the Model does not
    support asynch evals.  The domain point is guaranteed to be
    compatible with data type specified by map_domain(...) */

void COLINApplication::
perform_evaluation_impl(const utilib::Any &domain,
                   const colin::AppRequest::request_map_t &requests,
                   utilib::seed_t &seed,
		   colin::AppResponse::response_map_t &colin_responses)
{
  // Transform COLIN request to point for which DAKOTA executes an
  // evaluation, launch the evaluation, wait for the response, and
  // transform it to a COLIN response.

  colin_request_to_dakota_request(domain, requests, seed);

  iteratedModel.compute_response(activeSet);

  dakota_response_to_colin_response(iteratedModel.current_response(), colin_responses);
}

/** Check to see if any asynchronous evaluations have finished.  This
    is only called by COLIN's concurrent evaluator, which is only
    instantiated when the Model supports asynch evals. */

bool COLINApplication::
evaluation_available()
{
  // If there are already responses sitting around in the response
  // map, return true.  Otherwise, call synchronize() to collect
  // completed eval, populating response map in the process.

  // TODO: Make synchronize_nowait() work when DAKOTA is run with
  // MPI.  Once that is done, re-enable asynchronous PS through use of
  // blockingSynch.

  if (dakota_responses.empty()) {

    dakota_responses = iteratedModel.synchronize();
    //    dakota_responses = (blockingSynch) ?
    //      iteratedModel.synchronize() : iteratedModel.synchronize_nowait();

    if (dakota_responses.empty())
      return false;
  }
  
  return true;
}

/** Collect the next completed evaluation from DAKOTA.  Always returns
    the evalid of the response returned. */

utilib::Any COLINApplication::
collect_evaluation_impl(colin::AppResponse::response_map_t &colin_responses,
		   utilib::seed_t &seed)
{
  // Get the first response off the list.

  int dakota_id = dakota_responses.begin()->first;
  Response first_response = dakota_responses.begin()->second;

  // Transform the DAKOTA response to a COLIN response and the remove
  // it from the DAKOTA response map.

  dakota_response_to_colin_response(first_response, colin_responses);
  dakota_responses.erase(dakota_id);

  return(dakota_id);
}

  /** Map COLIN info requests to DAKOTA objectives and constraints. */

void COLINApplication::
colin_request_to_dakota_request(const utilib::Any &domain,
                   const colin::AppRequest::request_map_t &requests,
                   utilib::seed_t &seed)
{
  // One specification type for discrete variables is a set of values.
  // Get that list of values if the user provided one.  Also,
  // determine the size of those sets and the number of non-set
  // integer variables.

  const IntSetArray& ddsiv_values
    = iteratedModel.discrete_design_set_int_values();
  const RealSetArray& ddsrv_values
    = iteratedModel.discrete_design_set_real_values();
  size_t num_ddsiv = ddsiv_values.size(), num_ddsrv = ddsrv_values.size(),
    num_ddrv = iteratedModel.div() - num_ddsiv, offset;

  // Get the mixed variables for the point.

  const utilib::MixedIntVars& miv = domain.expose<MixedIntVars>();

  // Cast the continuous variables from COLIN to DAKOTA.

  RealVector cdv;
  TypeManager()->lexical_cast(miv.Real(), cdv);
  iteratedModel.continuous_variables(cdv);

  // Cast the discrete variables from COLIN to a temporary vector.

  IntVector ddv;
  TypeManager()->lexical_cast(miv.Integer(), ddv);

  IntVector   intVariableHolder(iteratedModel.div());
  RealVector realVariableHolder(iteratedModel.drv());

  // Assign non-set integer variables to DAKOTA integer variables.

  for (size_t j=0; j<num_ddrv; j++)
    intVariableHolder[j] = ddv[j];

  // Remember, COLIN is operating on the index for the set discrete
  // variables.  Get the integer values associated with each index and
  // assign them to DAKOTA integer variables.

  offset = num_ddrv;
  for (size_t j=0; j<num_ddsiv; j++) {
    int colin_index = ddv[j+offset];
    intVariableHolder[j+offset] = set_index_to_value(colin_index, ddsiv_values[j]);
  }

  iteratedModel.discrete_int_variables(intVariableHolder);

  // Likewise for the real set discrete variables.

  offset += num_ddsiv;
  for (size_t j=0; j<num_ddsrv; j++) {
    int colin_index = ddv[j+offset];
    realVariableHolder[j] = set_index_to_value(colin_index, ddsrv_values[j]);
  }

  iteratedModel.discrete_real_variables(realVariableHolder);

  // Map COLIN info requests (pair<ResponseInfo, *>) to DAKOTA
  // objectives and constraints.  COLIN will always request ALL
  // functions or ALL constraints

  // TODO: gradient support

  ShortArray asv(iteratedModel.num_functions());

  AppRequest::request_map_t::const_iterator req_it  = requests.begin();
  AppRequest::request_map_t::const_iterator req_end = requests.end();
  size_t numObj = num_objectives;
  size_t numCons = num_nonlinear_constraints;
  for (; req_it != req_end; ++req_it) {
    
    // mf_info: all multiobjective functions

    if (req_it->first == mf_info)
      for(size_t i=0; i<numObj; i++)
	asv[i] = 1;    
  
    // nlcf_info (lb <= g(x) <= ub)

    else if (req_it->first == nlcf_info)
      for(size_t i=0; i<numCons; i++)
	asv[i+numObj] = 1;

    else
      // No linear constraints since we'll send A and bounds.
      // No gradients/Hessians for now.

      EXCEPTION_MNGR(std::runtime_error,"invalid request from COLIN:"  
		     << colin::AppResponseInfo().name(req_it->first) );
  }
}

  /** Map DAKOTA objective and constraint values to COLIN response. */

void COLINApplication::
dakota_response_to_colin_response(const Response &dakota_response,
				  colin::AppResponse::response_map_t &colin_responses)
{

  const ShortArray& asv = dakota_response.active_set_request_vector();

  // If the DAKOTA response included functions, return them.  COLIN
  // will always expect a full set of functions, so break as soon as
  // any is missing.  May want to store a short-hand request vector
  // locally.

  bool fns = true;
  size_t numObj = num_objectives;
  utilib::Any fnvals;
  RealVector &fnvals_v = fnvals.set<RealVector>();
  fnvals_v.resize(numObj);
  for(size_t i=0; i<numObj; i++) {
    if ( ! (asv[i] & 1) ) {
      fns = false;
      break;
    }
    fnvals_v[i] = dakota_response.function_value(i);
  }
  if (fns) {
    colin_responses.insert(make_pair(mf_info, fnvals));
  }

  // If the DAKOTA response included constraints, return them.  COLIN
  // will always expect a full set of functions, so break as soon as
  // any is missing.

  bool cons= true;
  size_t numCons = num_nonlinear_constraints;
  RealVector &cons_v = fnvals.set<RealVector>();
  cons_v.resize(numCons);
  for(size_t i=0; i<numCons; i++) {
    if ( ! (asv[i+numObj] & 1) ) {
      cons = false;
      break;
    }
    cons_v[i] = dakota_response.function_value(i+numObj);
  }
  if (cons) {
    colin_responses.insert(make_pair(nlcf_info, fnvals));
  }
}

  /** Map the domain point into data type desired by this application
      context (utilib::MixedIntVars).  This data type can be exposed
      from the Any &domain presented to spawn and collect. */

bool COLINApplication::map_domain (const utilib::Any &src, utilib::Any &native,
				   bool forward) const 
{ 
  static_cast<void>(forward);
  return TypeManager()->lexical_cast(src, native, typeid(MixedIntVars)) == 0;
}


} // namespace Dakota
