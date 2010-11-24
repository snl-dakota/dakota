/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
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

COLINApplication::COLINApplication(Model& model) :
  blockingSynch(0)
{ set_problem(model);}


void COLINApplication::set_problem(Model& model) {

  iteratedModel = model;
  activeSet = model.current_response().active_set();

  // BMA NOTE: For COLIN to relax integer variables, must hand COLIN a
  // relaxed problem with continuous vars; see
  // Application_IntDomain::relaxed_application

  // set problem properties (problem size, constraints, etc); avoid
  // probDescDB so this function may be used with both the standard
  // and on-the-fly COLINOptimizer ctors

  _num_real_vars = model.cv();
  if (num_real_vars > 0)
  {
    _real_lower_bounds = model.continuous_lower_bounds();
    _real_upper_bounds = model.continuous_upper_bounds();
  }

  // discrete vars
  _num_int_vars = model.div();
  if (num_int_vars > 0)
  {
    _int_lower_bounds = model.discrete_int_lower_bounds();
    _int_upper_bounds = model.discrete_int_upper_bounds();
  }

  _num_nonlinear_constraints = model.num_nonlinear_ineq_constraints() +
    model.num_nonlinear_eq_constraints();

  // for multiobjective, this will be taken from the RecastModel and will be
  // consistent with the COLIN iterator's view
  _num_objectives = 
    model.num_functions() - num_nonlinear_constraints.as<size_t>();

  // nonlinear constraint bounds: assemble nonlinear inequality
  // followed by equality constraints
  if (num_nonlinear_constraints > 0) {

    RealVector bounds(num_nonlinear_constraints.as<size_t>());

    const RealVector& ineq_lower
      = model.nonlinear_ineq_constraint_lower_bounds();
    const RealVector& ineq_upper
      = model.nonlinear_ineq_constraint_upper_bounds();
    const RealVector& eq_targets
      = model.nonlinear_eq_constraint_targets();

    for (size_t i=0; i<model.num_nonlinear_ineq_constraints(); i++)
      bounds[i] = ineq_lower[i];

    size_t ndx = model.num_nonlinear_ineq_constraints();
    for (size_t i=0; i<model.num_nonlinear_eq_constraints(); i++, ndx++)
      bounds[ndx] = eq_targets[i];

    _nonlinear_constraint_lower_bounds = bounds;

    for (size_t i=0; i<model.num_nonlinear_ineq_constraints(); i++)
      bounds[i] = ineq_upper[i];

    _nonlinear_constraint_upper_bounds = bounds;
  }

  // PDH  Double check with Bill to see which (if any) algs support linear constraints.
  //      May want to make this a helper function called from COLINOptimizer.

  _num_linear_constraints = model.num_linear_ineq_constraints() +
    model.num_linear_eq_constraints();

  // linear constraint bounds: assemble linear inequality
  // followed by equality constraints
  if (num_linear_constraints > 0) {

    RealMatrix linear_coeffs( num_linear_constraints.as<size_t>(), 
                              domain_size.as<size_t>() );

    const RealMatrix& linear_ineq_coeffs = iteratedModel.linear_ineq_constraint_coeffs();
    const RealMatrix& linear_eq_coeffs = iteratedModel.linear_eq_constraint_coeffs();

    size_t ndx = 0;
    size_t ndy = 0;
    for (int i=0; i<model.num_linear_ineq_constraints(); i++) {
      for (int j=0; domain_size>j; j++)
	linear_coeffs[ndx][ndy++] = linear_ineq_coeffs[i][j];
      ndx++;
    }

    for (int i=0; i<model.num_linear_eq_constraints(); i++) {
      for (int j=0; domain_size>j; j++)
	linear_coeffs[ndx][ndy++] = linear_ineq_coeffs[i][j];
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

   for (size_t i=0; i<model.num_linear_ineq_constraints(); i++) 
     bounds[i] = lin_ineq_lower[i];

   ndx = model.num_linear_ineq_constraints();
   for (size_t i=0; i<model.num_linear_eq_constraints(); i++, ndx++)
     bounds[ndx] = lin_eq_targets[i];

   _linear_constraint_lower_bounds = bounds;

   for (size_t i=0; i<model.num_linear_ineq_constraints(); i++) 
     bounds[i] = lin_ineq_upper[i];

   _linear_constraint_upper_bounds = bounds;
  }
}

/** Schedule one or more requests at specified domain point, returning
    a DAKOTA-specific evaluation tracking ID.  The domain point is
    guaranteed to be compatible with data type specified by
    map_domain(...) */
utilib::Any COLINApplication::
spawn_evaluation_impl(const utilib::Any &domain,
		 const colin::AppRequest::request_map_t &requests,
		 utilib::seed_t &seed)
{
  colin_request_to_dakota_request(domain, requests, seed);

  // When the Model supports asynch evals, use them, otherwise
  // maintain backward behavior and block with compute_response

  iteratedModel.asynch_compute_response(activeSet);

  return(iteratedModel.evaluation_id());
}

void COLINApplication::
perform_evaluation_impl(const utilib::Any &domain,
                   const colin::AppRequest::request_map_t &requests,
                   utilib::seed_t &seed,
                   AppResponse::response_map_t &colin_responses)
{
  colin_request_to_dakota_request(domain, requests, seed);

  iteratedModel.compute_response(activeSet);

  dakota_response_to_colin_response(iteratedModel.current_response(), colin_responses);
}


bool COLINApplication::
evaluation_available()
{
  if (completedEvals.empty()) {
    if (iteratedModel.asynch_flag()) {

      // PDH  This will change when John is done with the COLIN concurrent evaluator.
      //      In particular, we don't want to call iteratedModel.synchronize().

      dakota_responses = (blockingSynch) ?
	iteratedModel.synchronize() : iteratedModel.synchronize_nowait();

      if (dakota_responses.empty())
	return false;
      else
	return true;
    }
    return false;
  }

  return true;
}

/** Collect a completed evaluation from DAKOTA.  Either specified
    spawned evalid or, if empty, the next available evaluation. Always
    returns the evalid of the response returned. */

utilib::Any COLINApplication::
collect_evaluation_impl(AppResponse::response_map_t &colin_responses,
		   utilib::seed_t &seed)
{
  int dakota_id = dakota_responses.begin()->first;
  Response first_response = dakota_responses.begin()->second;

  dakota_response_to_colin_response(first_response, colin_responses);
  dakota_responses.erase(dakota_id);

  return(dakota_id);
}

void COLINApplication::
colin_request_to_dakota_request(const utilib::Any &domain,
                   const colin::AppRequest::request_map_t &requests,
                   utilib::seed_t &seed)
{
  // Map COLIN info requests to DAKOTA objectives and constraints.

  const utilib::MixedIntVars& miv = domain.expose<MixedIntVars>();

  RealVector cdv;
  TypeManager()->lexical_cast(miv.Real(), cdv);
  iteratedModel.continuous_variables(cdv);

  IntVector ddv;
  TypeManager()->lexical_cast(miv.Integer(), ddv);
  iteratedModel.discrete_int_variables(ddv);

  // Map COLIN info requests (pair<ResponseInfo, *>) to DAKOTA
  // objectives and constraints; COLIN will always request ALL
  // functions or ALL constraints
  // BMA TODO: gradient information
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
      // no linear constraints since we'll send A and bounds
      // no gradients/Hessians for now
      EXCEPTION_MNGR(std::runtime_error,"invalid request from COLIN:"  
		     << colin::AppResponseInfo().name(req_it->first) );
  }

  // BMA TODO: track the scheduled evaluations; could always do
  // compute_response in serial case, but do asynch for simplicity
  activeSet.request_vector(asv);
}

void COLINApplication::
dakota_response_to_colin_response(const Response &dakota_response,
				  AppResponse::response_map_t &colin_responses)
{
  // Map DAKOTA objective and constraint values to COLIN response.

  const ShortArray& asv = dakota_response.active_set_request_vector();

  // if the DAKOTA response included functions, return them (colin
  // will always expect a full set of functions, so break as soon as
  // any is missing); may want to store a short-hand request vector
  // locally
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

  // if the DAKOTA response included constraints, return them
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
