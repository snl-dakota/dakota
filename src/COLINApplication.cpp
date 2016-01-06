/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
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
//- Version: $Id: COLINApplication.cpp 6829 2010-06-18 23:10:23Z pdhough $
//

#include "dakota_system_defs.hpp"
#include "DakotaModel.hpp"

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

#include "COLINApplication.hpp"

namespace Dakota {

COLINApplication::COLINApplication(Model& model) :
  blockingSynch(true) // updated from COLINOptimizer
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

  // Get the upper and lower bounds on the discrete variables.

  size_t i, j, num_div = model.div(), num_drv = model.drv(),
    num_dsv = model.dsv(), num_dv = num_div + num_drv + num_dsv;
  _num_int_vars = num_dv;
  if (num_dv) {
    const BitArray&       di_set_bits = model.discrete_int_sets();
    const IntVector&      lower_bnds  = model.discrete_int_lower_bounds();
    const IntVector&      upper_bnds  = model.discrete_int_upper_bounds();
    const IntSetArray&    dsiv_values = model.discrete_set_int_values();
    const RealSetArray&   dsrv_values = model.discrete_set_real_values();
    const StringSetArray& dssv_values = model.discrete_set_string_values();
    // Need temporary storage in which to consolidate all types of
    // discrete variables;

    IntVector lower(num_dv), upper(num_dv);

    size_t dsi_cntr = 0;
    for (i=0; i<num_div; ++i) {
      if (di_set_bits[i]) {
	// this active discrete int var is a set type: map to integer
	// index sequence [0,num_items-1].
	lower[i] = 0;
	upper[i] = dsiv_values[dsi_cntr].size() - 1;
	++dsi_cntr;
      }
      else {
	// this active discrete int var is a range type: use range bounds
	lower[i] = lower_bnds[i];
	upper[i] = upper_bnds[i];
      }
    }

    // For real set variables, map to integer index sequence
    // [0,num_items-1].
    for (i=0; i<num_drv; ++i) {
      lower[i+num_div] = 0;
      upper[i+num_div] = dsrv_values[i].size() - 1;
    }

    // For string set variables, map to integer index sequence
    // [0,num_items-1].
    for (i=0; i<num_dsv; ++i) {
      lower[i+num_div+num_drv] = 0;
      upper[i+num_div+num_drv] = dssv_values[i].size() - 1;
    }

    // Now assign to the COLIN vectors.

    _int_lower_bounds = lower;
    _int_upper_bounds = upper;
  }

  _num_nonlinear_constraints = model.num_nonlinear_ineq_constraints() +
    model.num_nonlinear_eq_constraints();

  // For multiobjective, this will be taken from the RecastModel and
  // will be consistent with the COLIN iterator's view

  //_num_objectives = 
  //  model.num_functions() - num_nonlinear_constraints.as<size_t>();
  size_t numObj = 
    model.num_functions() - num_nonlinear_constraints.as<size_t>();
  _num_objectives = numObj;
 
  const BoolDeque& max_sense = model.primary_response_fn_sense();
  if (!max_sense.empty()) {
    // COLINApplication derived from a (general) multi-objective problem type of
    // Application<MO_MINLP2_problem>, so don't need to manage a scalar sense.
    //if (numObjectiveFns == 1)
    //  _sense = (max_sense[0]) ? colin::maximization : colin::minimization;
    //else {
    std::vector<colin::optimizationSense> min_max(numObj);
    for (i=0; i<numObj; ++i)
      min_max[i] = (max_sense[i]) ? colin::maximization : colin::minimization;
    _sense = min_max;
    //}
  }

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

    for (i=0; i<model.num_nonlinear_ineq_constraints(); i++)
      bounds[i] = ineq_lower[i];

    size_t ndx = model.num_nonlinear_ineq_constraints();
    for (i=0; i<model.num_nonlinear_eq_constraints(); i++, ndx++)
      bounds[ndx] = eq_targets[i];

    _nonlinear_constraint_lower_bounds = bounds;

    // Lower bounds and targets go together in COLIN upper bounds.

    for (i=0; i<model.num_nonlinear_ineq_constraints(); i++)
      bounds[i] = ineq_upper[i];

    ndx = model.num_nonlinear_ineq_constraints();
    for (i=0; i<model.num_nonlinear_eq_constraints(); i++, ndx++)
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

    const RealMatrix& linear_ineq_coeffs = model.linear_ineq_constraint_coeffs();
    const RealMatrix& linear_eq_coeffs = model.linear_eq_constraint_coeffs();

    // Populate the coefficient matrix, first with inequality
    // coefficients, then with equality coefficients.

    size_t ndx = 0, ndy = 0;
    for (i=0; i<model.num_linear_ineq_constraints(); i++) {
      for (j=0; domain_size>j; j++)
	linear_coeffs(ndx,ndy++) = linear_ineq_coeffs(i,j);
      ndx++;
    }

    for (i=0; i<model.num_linear_eq_constraints(); i++) {
      for (j=0; domain_size>j; j++)
	linear_coeffs(ndx,ndy++) = linear_eq_coeffs(i,j);
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

   for (i=0; i<model.num_linear_ineq_constraints(); i++) 
     bounds[i] = lin_ineq_lower[i];

   ndx = model.num_linear_ineq_constraints();
   for (i=0; i<model.num_linear_eq_constraints(); i++, ndx++)
     bounds[ndx] = lin_eq_targets[i];

   _linear_constraint_lower_bounds = bounds;

   // Upper bounds and equality targets go together in COLIN upper
   // bounds.

   for (i=0; i<model.num_linear_ineq_constraints(); i++) 
     bounds[i] = lin_ineq_upper[i];

   ndx = model.num_linear_ineq_constraints();
   for (i=0; i<model.num_linear_eq_constraints(); i++, ndx++)
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

  iteratedModel.evaluate_nowait();

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

  iteratedModel.evaluate();

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

  if (dakota_responses.empty()) {

    dakota_responses = (blockingSynch) ?
      iteratedModel.synchronize() : iteratedModel.synchronize_nowait();

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
  // Get the mixed variables for the point.

  const utilib::MixedIntVars& miv = domain.expose<MixedIntVars>();

  // Cast the continuous variables from COLIN to DAKOTA.

  RealVector cdv;
  TypeManager()->lexical_cast(miv.Real(), cdv);
  iteratedModel.continuous_variables(cdv);

  // Cast the discrete variables from COLIN to a temporary vector.

  IntVector ddv;
  TypeManager()->lexical_cast(miv.Integer(), ddv);

  // One specification type for discrete variables is a set of values.
  // Get that list of values if the user provided one.

  const     BitArray&   di_set_bits = iteratedModel.discrete_int_sets();
  const IntSetArray&    dsiv_values = iteratedModel.discrete_set_int_values();
  const RealSetArray&   dsrv_values = iteratedModel.discrete_set_real_values();
  const StringSetArray& dssv_values = iteratedModel.discrete_set_string_values();

  // Assign COLIN integer variables to DAKOTA discrete integer variables.
  // Remember, COLIN is operating on the index for the set discrete
  // variables.  Get the integer values associated with each index and
  // assign them to DAKOTA integer variables.

  size_t j, dsi_cntr, num_div = iteratedModel.div(),
    num_drv = iteratedModel.drv(), num_dsv = iteratedModel.dsv();
  for (j=0, dsi_cntr=0; j<num_div; ++j) {
    if (di_set_bits[j]) { // this active discrete int var is a set type
      int dakota_value = set_index_to_value(ddv[j], dsiv_values[dsi_cntr]);
      iteratedModel.discrete_int_variable(dakota_value, j);
      ++dsi_cntr;
    }
    else                  // this active discrete int var is a range type
      iteratedModel.discrete_int_variable(ddv[j], j);
  }

  // Likewise for the real set discrete variables.

  for (size_t j=0; j<num_drv; ++j) {
    Real dakota_value = set_index_to_value(ddv[j+num_div], dsrv_values[j]);
    iteratedModel.discrete_real_variable(dakota_value, j);
  }

  // Likelikewise for the string set discrete variables
  for (size_t j=0; j<num_dsv; ++j) {
    String dakota_value = set_index_to_value(ddv[j+num_div+num_drv], dssv_values[j]);
    iteratedModel.discrete_string_variable(dakota_value, j);
  }

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
