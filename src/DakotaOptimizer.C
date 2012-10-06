/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       Optimizer
//- Description: Implementation code for the Optimizer class
//- Owner:       Mike Eldred
//- Checked by:

#include "system_defs.h"
#include "data_io.h"
#include "DakotaModel.H"
#include "DakotaOptimizer.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "ProblemDescDB.H"
#include "RecastModel.H"

static const char rcsId[]="@(#) $Id: DakotaOptimizer.C 7031 2010-10-22 16:23:52Z mseldre $";


namespace Dakota {
  extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
Optimizer* Optimizer::optimizerInstance(NULL);

/** This constructor extracts the inherited data for the optimizer
    branch and performs sanity checking on gradient and constraint
    settings. */
Optimizer::Optimizer(Model& model): Minimizer(model),
  maximizeFlag(probDescDB.get_bool("method.maximize_flag")),
  numObjectiveFns(probDescDB.get_sizet("responses.num_objective_functions"))
{
  // Check for bound constraint support/requirement in method selection
  if ( boundConstraintFlag && methodName == "optpp_cg" ) {
    Cerr << "\nError: bound constraints not currently supported by "
         << methodName << ".\n       Please select a different method for "
         << "bound constrained problems." << std::endl;
    abort_handler(-1);
  }
 
  // Check for nongradient method with speculative flag set
  if ( speculativeFlag && ( methodName == "optpp_pds" ||
       methodName.begins("coliny_") || methodName.begins("ncsu_") ||
       methodName == "moga" || methodName == "soga" ) ) {
    Cerr << "\nWarning: Speculative specification for a nongradient-based "
	 << "optimizer is ignored.\n";
    speculativeFlag = false;
  }

  // Check for full Newton method w/o Hessian support (direct or via recast)
  bool require_hessians = false;
  size_t num_lsq = probDescDB.get_sizet("responses.num_least_squares_terms");
  if ( methodName == "optpp_newton") { // || ...) {
    require_hessians = true;
    if ( ( num_lsq == 0 && hessianType  == "none" ) ||
	 ( num_lsq      && gradientType == "none" ) ) {
      Cerr << "\nError: full Newton optimization requires objective Hessians."
	   << std::endl;
      abort_handler(-1);
    }
  }

  // Check for proper response function definition (optimization data set)
  // and manage requirements for local recasting
  numUserPrimaryFns = numFunctions - numNonlinearConstraints;
  bool local_nls_recast = false, local_moo_recast = false;
  if (numObjectiveFns == 0) { // no user spec for num_objective_functions
    optimizationFlag = false; // used to distinguish NLS from MOO
    // allow solution of NLS problems as single-objective optimization
    // through local recasting
    bool err_flag = false;
    if (num_lsq) {
      // Distinguish NLS case requiring a local recast from one where the
      // incoming model has already been recast (surrogate-based NLS with
      // "approx_subproblem single_objective").  Note that numFunctions
      // and numNonlinearConstraints have been set in the ctor chain based
      // on the incoming model.  Honoring the (rare) case of a user spec
      // of num_least_squares_terms = 1 requires special logic.
      if (numUserPrimaryFns == num_lsq) { // local recasting _may_ be required
	if (num_lsq == 1) // ambiguous; use local recast if not already recast
	  local_nls_recast = (model.model_type() != "recast");
	else              // not yet recast; local recasting is required
	  local_nls_recast = true;
      }
      else { // a recasting of LSQ has already occurred
	if (numUserPrimaryFns == 1) // since not equal, num_lsq > 1
	  local_nls_recast = false;
	else // prior recasting must be LSQ --> single obj fn
	  err_flag = true;
      }
    }
    else
      err_flag = true;
    if (err_flag) {
      Cerr << "\nError: responses specification is incompatible with "
	   << "optimization methods." << std::endl;
      abort_handler(-1);
    }
    if (local_nls_recast)
      Cerr << "Warning: coercing least squares data set into optimization data "
	   << "set." << std::endl;
    // For all NLS cases, reset numObjectiveFns to 1: optimizers see single
    // objective in recast model, whether local or prior recast.
    numObjectiveFns = 1;
  }
  else { // user specification for num_objective_functions
    optimizationFlag = true; // used to distinguish NLS from MOO
    if (numObjectiveFns > 1 && methodName != "moga")
      { local_moo_recast = true; numObjectiveFns = 1; }
  }

  // when scaling and/or single objective transformation is enabled, create a
  // RecastModel to map between [user/native] space and [iterator/scaled] space
  localObjectiveRecast = (local_nls_recast || local_moo_recast);
  if (scaleFlag || localObjectiveRecast) {

    numIterPrimaryFns = numObjectiveFns; // used at Minimizer level

    // user-space model becomes the sub-model of a RecastModel:
    SizetArray recast_vars_comps_total; // default: empty; no change in size
    iteratedModel.assign_rep(new
      RecastModel(model, recast_vars_comps_total, numObjectiveFns, 
		  numNonlinearConstraints, numNonlinearIneqConstraints), false);

    // initialize_scaling function needs to modify the iteratedModel
    if (scaleFlag)
      initialize_scaling();

    // setup recast model mappings and flags
    // recast map is all one to one unless single objective transformation
    size_t i;
    size_t num_recast_fns = numObjectiveFns + numNonlinearConstraints;
    Sizet2DArray var_map_indices(numContinuousVars), 
                 primary_resp_map_indices(numObjectiveFns), 
                 secondary_resp_map_indices(numNonlinearConstraints);
    bool nonlinear_vars_map = false;
    BoolDequeArray nonlinear_resp_map(num_recast_fns);

    for (i=0; i<numContinuousVars; i++) {
      var_map_indices[i].resize(1);
      var_map_indices[i][0] = i;
      if (varsScaleFlag && cvScaleTypes[i] & SCALE_LOG)
	nonlinear_vars_map = true;
    }

    if (localObjectiveRecast) {
      primary_resp_map_indices[0].resize(numUserPrimaryFns);
      nonlinear_resp_map[0].resize(numUserPrimaryFns);
      for (i=0; i<numUserPrimaryFns; i++) {
	primary_resp_map_indices[0][i] = i;
	nonlinear_resp_map[0][i] = ( local_nls_recast ||
	  ( primaryRespScaleFlag && responseScaleTypes[i] & SCALE_LOG ) );
      }

      // adjust active set vector to 1 + numNonlinearConstraints
      ShortArray asv(1 + numNonlinearConstraints, 1);
      activeSet.request_vector(asv);
    }
    else {
      // in this case num iterator and user obj fns are same
      for (i=0; i<numUserPrimaryFns; i++) {
	primary_resp_map_indices[i].resize(1);
	primary_resp_map_indices[i][0] = i;
	nonlinear_resp_map[i].resize(1);
	nonlinear_resp_map[i][0] = primaryRespScaleFlag &&
                                   responseScaleTypes[i] & SCALE_LOG;
      }
    }
    for (i=0; i<numNonlinearConstraints; i++) {
      secondary_resp_map_indices[i].resize(1);
      secondary_resp_map_indices[i][0] = numUserPrimaryFns + i;
      nonlinear_resp_map[numObjectiveFns+i].resize(1);
      nonlinear_resp_map[numObjectiveFns+i][0] = secondaryRespScaleFlag &&
	responseScaleTypes[numUserPrimaryFns + i] & SCALE_LOG;
    }

    // complete initialization of the RecastModel after alternate construction
    // may need response recast when variables are scaled (for grad, hess)
    void (*vars_recast) (const Variables&, Variables&)
      = (varsScaleFlag) ? variables_recast : NULL;
    void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&)
      = (local_nls_recast && require_hessians && hessianType == "none") ?
      gnewton_set_recast : NULL;
    void (*pri_resp_recast) (const Variables&, const Variables&,
                             const Response&, Response&)
      = (localObjectiveRecast || primaryRespScaleFlag || varsScaleFlag) ? 
      primary_resp_recast : NULL;
    void (*sec_resp_recast) (const Variables&, const Variables&,
                             const Response&, Response&)
      = (secondaryRespScaleFlag || varsScaleFlag) ? 
      secondary_resp_recast : NULL;
    RecastModel* recast_model_rep = (RecastModel*)iteratedModel.model_rep();
    recast_model_rep->initialize(var_map_indices, nonlinear_vars_map,
				 vars_recast, set_recast,
				 primary_resp_map_indices,
				 secondary_resp_map_indices, nonlinear_resp_map,
				 pri_resp_recast, sec_resp_recast);
    // if Gauss-Newton recasting, then the RecastModel Response needs to
    // allocate space for a Hessian (default copy of sub-model response
    // is insufficient).
    if (set_recast) {
      Response recast_resp = iteratedModel.current_response(); // shared rep
      recast_resp.reshape(num_recast_fns, numContinuousVars, true, true);
    }

    // for gradient-based Optimizers, maxConcurrency has already been determined
    // from derivative concurrency in the Iterator initializer, so initialize
    // communicators in the RecastModel.  For nongradient methods (many COLINY
    // methods, OPT++ PDS, eventually JEGA), maxConcurrency is defined in the
    // derived constructors, so init_communicators() is invoked there.
    if ( !( methodName == "optpp_pds" || methodName.begins("coliny_") ||
	    methodName == "moga"      || methodName == "soga" ) )
      iteratedModel.init_communicators(maxConcurrency);
  }
  else
    iteratedModel = model;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


Optimizer::Optimizer(NoDBBaseConstructor, Model& model):
  Minimizer(NoDBBaseConstructor(), model), numObjectiveFns(numUserPrimaryFns),
  maximizeFlag(false), localObjectiveRecast(false)
{
  if (numObjectiveFns > 1) {
    Cerr << "Error: on-the-fly Optimizer instantiations do not currently "
	 << "support multiple objective functions." << std::endl;
    abort_handler(-1);
  }

  optimizationFlag = true;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


Optimizer::
Optimizer(NoDBBaseConstructor, size_t num_cv, size_t num_div, size_t num_drv,
	  size_t num_lin_ineq, size_t num_lin_eq, size_t num_nln_ineq,
	  size_t num_nln_eq):
  Minimizer(NoDBBaseConstructor(), num_lin_ineq, num_lin_eq, num_nln_ineq,
	    num_nln_eq),
  numObjectiveFns(1), maximizeFlag(false), localObjectiveRecast(false)
{
  numContinuousVars   = num_cv;
  numDiscreteIntVars  = num_div;
  numDiscreteRealVars = num_drv;
  numFunctions        = numUserPrimaryFns + numNonlinearConstraints;
  optimizationFlag    = true;

  std::pair<short,short> view(MIXED_DESIGN, EMPTY);
  SizetArray vc_totals(12, 0);
  vc_totals[0] = num_cv; vc_totals[1] = num_div; vc_totals[2] = num_drv;
  SharedVariablesData svd(view, vc_totals);
  bestVariablesArray.push_back(Variables(svd));

  activeSet.reshape(numFunctions, numContinuousVars);
  activeSet.request_values(1); activeSet.derivative_start_value(1);
  bestResponseArray.push_back(Response(activeSet));
}


/** Redefines default iterator results printing to include
    optimization results (objective functions and constraints). */
void Optimizer::print_results(std::ostream& s)
{
  size_t i, num_best = bestVariablesArray.size();
  if (num_best != bestResponseArray.size()) {
    Cerr << "\nError: mismatch in lengths of bestVariables and bestResponses."
         << std::endl;
    abort_handler(-1); 
  } 

  const String& interface_id = iteratedModel.interface_id(); 
  int eval_id; 
  ActiveSet search_set(numFunctions, numContinuousVars); // asv = 1's
 
  // -------------------------------------
  // Single and Multipoint results summary
  // -------------------------------------
  for (i=0; i<num_best; ++i) { 
    // output best variables
    s << "<<<<< Best parameters          "; 
    if (num_best > 1) s << "(set " << i+1 << ") "; 
    s << "=\n" << bestVariablesArray[i]; 
    // output best response
    const RealVector& best_fns = bestResponseArray[i].function_values(); 
    if (numUserPrimaryFns > 1) s << "<<<<< Best objective functions "; 
    else                       s << "<<<<< Best objective function  "; 
    if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n"; 
    write_data_partial(s, 0, numUserPrimaryFns, best_fns); 
    size_t num_cons = numFunctions - numUserPrimaryFns; 
    if (num_cons) { 
      s << "<<<<< Best constraint values   "; 
      if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n"; 
      write_data_partial(s, numUserPrimaryFns, num_cons, best_fns); 
    } 
    // lookup evaluation id where best occurred.  This cannot be catalogued
    // directly because the optimizers track the best iterate internally and
    // return the best results after iteration completion.  Therfore, perform a
    // search in data_pairs to extract the evalId for the best fn eval.
    if (lookup_by_val(data_pairs, interface_id, bestVariablesArray[i], 
                      search_set, eval_id))
      s << "<<<<< Best data captured at function evaluation " << eval_id
        << "\n\n";
    else 
      s << "<<<<< Best data not found in evaluation cache\n\n"; 
  } 
}


/** Retrieve a MOO/NLS response based on the data returned by a single
    objective optimizer by performing a data_pairs search. This may
    get called even for a single user-specified function, since we may
    be recasting a single NLS residual into a squared objective. */
void Optimizer::
local_objective_recast_retrieve(const Variables& vars, Response& response) const
{
  Response desired_resp;
  if (lookup_by_val(data_pairs, iteratedModel.interface_id(), vars,
		    response.active_set(), desired_resp))
    response.update(desired_resp);
  else
    Cerr << "Warning: failure in recovery of final values for locally recast "
	 << "optimization." << std::endl;
}


/** Objective function map from user/native space to iterator/scaled/combined
    space using a RecastModel.  If resizing the response, copies the 
    constraint (secondary) data from native_response too */
void Optimizer::
primary_resp_recast(const Variables& native_vars, const Variables& scaled_vars,
		    const Response& native_response,
		    Response& iterator_response)
{
  if (optimizerInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n-----------------------------------";
    Cout << "\nPost-processing Function Evaluation";
    Cout << "\n-----------------------------------" << std::endl;
  }

  // need to scale if primary responses are scaled or (variables are
  // scaled and grad or hess requested)
  bool scale_transform_needed = optimizerInstance->primaryRespScaleFlag ||
    optimizerInstance->need_resp_trans_byvars(
      native_response.active_set_request_vector(), 0,
      optimizerInstance->numUserPrimaryFns);

  // Keeping scaling and weighted_sum transformation separate since
  // may eventually want more general transformations (e.g.,
  // goal-oriented).  Using weighted_sum yields a resized
  // iterator_response, but does not copy constraints
  if (optimizerInstance->localObjectiveRecast && scale_transform_needed) {
    // tmp_response contains the intermediate result with scaled
    // functions only (indices 0:(numUserPrimaryFns-1)) then apply
    // weighted objective sum
    Response tmp_response = native_response.copy();
    optimizerInstance->response_modify_n2s(native_vars, native_response,
      tmp_response, 0, 0, optimizerInstance->numUserPrimaryFns);
    const RealVector& native_wts = optimizerInstance->
      iteratedModel.subordinate_model().primary_response_fn_weights();
    optimizerInstance->objective_reduction(tmp_response, native_wts,
					   iterator_response);
  }
  else if (scale_transform_needed)
    optimizerInstance->response_modify_n2s(native_vars, native_response,
      iterator_response, 0, 0, optimizerInstance->numUserPrimaryFns);
  else if (optimizerInstance->localObjectiveRecast) {
    const RealVector& native_wts = optimizerInstance->
      iteratedModel.subordinate_model().primary_response_fn_weights();
    optimizerInstance->objective_reduction(native_response, native_wts,
					   iterator_response);
  }
  else {
    // could reach this if variables are scaled and only functions are requested
    iterator_response.update_partial(0, optimizerInstance->numUserPrimaryFns,
				     native_response, 0);
  }
}


/** This function is responsible for the mapping of multiple objective
    functions into a single objective for publishing to single-objective
    optimizers.  Used in DOTOptimizer, NPSOLOptimizer, SNLLOptimizer, and
    SGOPTApplication on every function evaluation.  The simple weighting
    approach (using primaryRespFnWts) is the only technique supported
    currently.  The weightings are used to scale function values,
    gradients, and Hessians as needed. */
void Optimizer::objective_reduction(const Response& full_response,
				    const RealVector& full_wts, 
				    Response& reduced_response) const
{
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Local single objective transformation:\n";

  short reduced_asv0 = reduced_response.active_set_request_vector()[0];
  if (reduced_asv0 & 1) { // build objective fn from full_response functions
    Real sum = objective(full_response.function_values(), full_wts);
    reduced_response.function_value(sum, 0);    
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "                     " << std::setw(write_precision+7) << sum
	   << " obj_fn\n";
  }

  if (reduced_asv0 & 2) { // build obj_grad from full_response gradients
    RealVector obj_grad = reduced_response.function_gradient_view(0);
    objective_gradient(full_response.function_values(),
		       full_response.function_gradients(), full_wts, obj_grad);
    if (outputLevel > NORMAL_OUTPUT) {
      write_col_vector_trans(Cout, 0, true, true, false,
			     reduced_response.function_gradients());
      Cout << " obj_fn gradient\n";
    }
  }

  if (reduced_asv0 & 4) { // build obj_hessian from full_response hessians
    RealSymMatrix obj_hessian = reduced_response.function_hessian_view(0);
    objective_hessian(full_response.function_values(),
		      full_response.function_gradients(),
		      full_response.function_hessians(), full_wts, obj_hessian);
    if (outputLevel > NORMAL_OUTPUT) {
      write_data(Cout, obj_hessian, true, true, false);
      Cout << " obj_fn Hessian\n";
    }
  }

  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;
}


/** Implements portions of initialize_run specific to Optimizers. This
    function should be invoked (or reimplemented) by any derived
    implementations of initialize_run() (which would otherwise hide
    it). */
void Optimizer::initialize_run()
{
  Minimizer::initialize_run();

  // pull any late updates into the RecastModel
  if (scaleFlag || localObjectiveRecast)
    iteratedModel.update_from_subordinate_model(false); // recursion not reqd

  // Track any previous object instance in case of recursion.  Note that
  // optimizerInstance and minimizerInstance must be tracked separately since
  // the previous optimizer and previous minimizer could be different instances
  // (e.g., for MCUU with NL2SOL for NLS and NPSOL for MPP search, the previous
  // minimizer is NL2SOL and the previous optimizer is NULL).
  prevOptInstance   = optimizerInstance;
  optimizerInstance = this;
}


/** Implements portions of post_run specific to Optimizers. This
    function should be invoked (or reimplemented) by any derived
    implementations of post_run() (which would otherwise hide it). */
void Optimizer::post_run(std::ostream& s)
{
   // scaling transformation needs to be performed on each best point
  size_t num_points = bestVariablesArray.size();
  if (num_points != bestResponseArray.size()) {
    Cerr << "\nError: mismatch in lengths of bestVariables and bestResponses."
	 << std::endl;
    abort_handler(-1);
  }

  for (size_t point_index = 0; point_index < num_points; ++point_index) {

    Variables& best_vars = bestVariablesArray[point_index];
    Response&  best_resp = bestResponseArray[point_index];

    // transform variables back to user space (for local obj recast or scaling)
    if (varsScaleFlag)
      best_vars.continuous_variables(
        modify_s2n(best_vars.continuous_variables(), cvScaleTypes,
		   cvScaleMultipliers, cvScaleOffsets));

    // transform responses back to user space (for local obj recast or scaling)
    if (localObjectiveRecast)
      local_objective_recast_retrieve(best_vars, best_resp);
    else if (primaryRespScaleFlag || secondaryRespScaleFlag) {
      Response tmp_response = best_resp.copy();
      if (primaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(), 0,
				 numUserPrimaryFns)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, 0, 0,
			    numUserPrimaryFns);
	best_resp.update_partial(0, numUserPrimaryFns, tmp_response, 0 );
      }
      if (secondaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(),
				 numUserPrimaryFns, numNonlinearConstraints)) {
	response_modify_s2n(best_vars, best_resp, tmp_response,
			    numUserPrimaryFns, numUserPrimaryFns,
			    numNonlinearConstraints);
	best_resp.update_partial(numUserPrimaryFns, numNonlinearConstraints,
				 tmp_response, numUserPrimaryFns);
      }
    }

  }

  Iterator::post_run(s);
}

} // namespace Dakota
