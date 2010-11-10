/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
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

// initialization of static needed by RecastModel
Optimizer* Optimizer::optimizerInstance(NULL);

/** This constructor extracts the inherited data for the optimizer
    branch and performs sanity checking on gradient and constraint
    settings. */
Optimizer::Optimizer(Model& model): Minimizer(model),
  numObjectiveFns(probDescDB.get_sizet("responses.num_objective_functions"))
{
  // Check for proper function definition (optimization data set)
  if (numObjectiveFns <= 0) {
    if (probDescDB.get_sizet("responses.num_least_squares_terms")) {
      // allow this unusual case for surrogate-based NLS using
      // "approx_subproblem single_objective"
      Cerr << "Warning: coercing least squares data set into optimization data "
	   << "set." << std::endl;
      // numFunctions and numNonlinearConstraints have been set from
      // the incoming (recast) model
      numObjectiveFns = numFunctions - numNonlinearConstraints;
    }
    else {
      Cerr << "\nError: number of objective functions must be greater than "
	   << "zero for optimization methods." << std::endl;
      abort_handler(-1);
    }
  }

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

  bestVariablesArray.push_back(model.current_variables().copy());

  // Check for need to perform multiobjective combination
  multiObjFlag = ( numObjectiveFns > 1 && methodName != "moga" );

  // when scaling and/or multiobjective is enabled, create a RecastModel to 
  // map between [user/native] space and [iterator/scaled] space

  // set minimizer data for number of functions or least squares terms
  numUserPrimaryFns = numUserObjectiveFns = numObjectiveFns;
  if (scaleFlag || multiObjFlag) {

    // number of objective functions in the recast -- iterators see
    // single objective in the case of multiobjective opt
    numObjectiveFns = multiObjFlag ? 1 : numObjectiveFns;
    numIterPrimaryFns = numObjectiveFns;

    // user-space model becomes the sub-model of a RecastModel:
    iteratedModel.assign_rep(new RecastModel(model, numIterPrimaryFns, 
      numNonlinearConstraints, numNonlinearIneqConstraints), false);

    // initialize_scaling function needs to modify the iteratedModel
    if (scaleFlag)
      initialize_scaling();

    // setup recast model mappings and flags
    // recast map is all one to one unless multiobjective active
    size_t i;
    size_t num_recast_fns = numIterPrimaryFns + numNonlinearConstraints;
    Sizet2DArray var_map_indices(numContinuousVars), 
                 primary_resp_map_indices(numIterPrimaryFns), 
                 secondary_resp_map_indices(numNonlinearConstraints);
    bool nonlinear_vars_map = false;
    BoolDequeArray nonlinear_resp_map(num_recast_fns);

    for (i=0; i<numContinuousVars; i++) {
      var_map_indices[i].resize(1);
      var_map_indices[i][0] = i;
      if (varsScaleFlag && cvScaleTypes[i] & SCALE_LOG)
	nonlinear_vars_map = true;
    }

    if (multiObjFlag) {
      primary_resp_map_indices[0].resize(numUserObjectiveFns);
      nonlinear_resp_map[0].resize(numUserObjectiveFns);
      for (i=0; i<numUserObjectiveFns; i++) {
	primary_resp_map_indices[0][i] = i;
	nonlinear_resp_map[0][i] = primaryRespScaleFlag &&
	                           responseScaleTypes[i] & SCALE_LOG;
      }

      // adjust active set vector to 1 + numNonlinearConstraints
      ShortArray asv(1 + numNonlinearConstraints, 1);
      activeSet.request_vector(asv);
    }
    else {
      // in this case num iterator and user obj fns are same
      for (i=0; i<numUserObjectiveFns; i++) {
	primary_resp_map_indices[i].resize(1);
	primary_resp_map_indices[i][0] = i;
	nonlinear_resp_map[i].resize(1);
	nonlinear_resp_map[i][0] = primaryRespScaleFlag &&
                                   responseScaleTypes[i] & SCALE_LOG;
      }
    }
    for (i=0; i<numNonlinearConstraints; i++) {
      secondary_resp_map_indices[i].resize(1);
      secondary_resp_map_indices[i][0] = numUserObjectiveFns + i;
      nonlinear_resp_map[numObjectiveFns+i].resize(1);
      nonlinear_resp_map[numObjectiveFns+i][0] = secondaryRespScaleFlag &&
	responseScaleTypes[numUserObjectiveFns + i] & SCALE_LOG;
    }

    // complete initialization of the RecastModel after alternate construction
    // may need response recast when variables are scaled (for grad, hess)
    void (*vars_recast) (const Variables&, Variables&)
      = (varsScaleFlag) ? variables_recast : NULL;
    void (*pri_resp_recast) (const Variables&, const Variables&,
                             const Response&, Response&)
      = (multiObjFlag || primaryRespScaleFlag || varsScaleFlag) ? 
      primary_resp_recast : NULL;
    void (*sec_resp_recast) (const Variables&, const Variables&,
                             const Response&, Response&)
      = (secondaryRespScaleFlag || varsScaleFlag) ? 
      secondary_resp_recast : NULL;
    RecastModel* recast_model_rep = (RecastModel*)iteratedModel.model_rep();
    recast_model_rep->initialize(var_map_indices, nonlinear_vars_map,
				 vars_recast, NULL, primary_resp_map_indices,
				 secondary_resp_map_indices, nonlinear_resp_map,
				 pri_resp_recast, sec_resp_recast);
 
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
}


Optimizer::Optimizer(NoDBBaseConstructor, Model& model):
  Minimizer(NoDBBaseConstructor(), model),
  numObjectiveFns(numFunctions - numNonlinearConstraints),
  numUserObjectiveFns(numObjectiveFns), multiObjFlag(false)
{
  if (numObjectiveFns > 1) {
    Cerr << "Error: on-the-fly Optimizer instantiations do not currently "
	 << "support multiple objective functions." << std::endl;
    abort_handler(-1);
  }

  bestVariablesArray.push_back(model.current_variables().copy());
}


Optimizer::
Optimizer(NoDBBaseConstructor, size_t num_cv, size_t num_div, size_t num_drv,
	  size_t num_lin_ineq, size_t num_lin_eq, size_t num_nln_ineq,
	  size_t num_nln_eq):
  Minimizer(NoDBBaseConstructor(), num_lin_ineq, num_lin_eq, num_nln_ineq,
	    num_nln_eq),
  numObjectiveFns(1), numUserObjectiveFns(1), multiObjFlag(false)
{
  numContinuousVars   = num_cv;
  numDiscreteIntVars  = num_div;
  numDiscreteRealVars = num_drv;
  numFunctions        = numUserObjectiveFns + numNonlinearConstraints;

  std::pair<short,short> view(MIXED_DISTINCT_DESIGN, EMPTY);
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
  const String& interface_id = iteratedModel.interface_id();
  ParamResponsePair desired_pair;
  extern PRPCache data_pairs; // global container
  // ----------------------------
  // Single point results summary
  // ----------------------------
  // Print best parameters
  const Variables& best_vars = bestVariablesArray.front();
  s << "<<<<< Best parameters          =\n" << best_vars;

  // Print best response functions
  const RealVector& fn_vals_star = bestResponseArray.front().function_values();
  if (numUserObjectiveFns > 1)
    s << "<<<<< Best objective functions =\n";
  else
    s << "<<<<< Best objective function  =\n";
  write_data_partial(s, 0, numUserObjectiveFns, fn_vals_star);
  size_t num_cons = numFunctions - numUserObjectiveFns;
  if (num_cons) {
    s << "<<<<< Best constraint values   =\n";
    write_data_partial(s, numUserObjectiveFns, num_cons, fn_vals_star);
  }

  // Print fn. eval. number where best occurred.  This cannot be catalogued 
  // directly because the optimizers track the best iterate internally and 
  // return the best results after iteration completion.  Therfore, perform a
  // search in data_pairs to extract the evalId for the best fn. eval.
  extern PRPCache data_pairs; // global container
  int eval_id;
  // For multiobjective, activeSet is wrong size -> instantiate new ActiveSet
  ActiveSet search_set(numFunctions, numContinuousVars); // asv = 1's
  search_set.derivative_vector(activeSet.derivative_vector());
  if (lookup_by_val(data_pairs, interface_id, best_vars, search_set, eval_id))
    s << "<<<<< Best data captured at function evaluation " << eval_id
      << std::endl;
  else
    s << "<<<<< Best data not found in evaluation cache" << std::endl;

  size_t num_instances = bestVariablesArray.size();
  if (num_instances > 1 && outputLevel > NORMAL_OUTPUT &&
      !methodName.begins("coliny_")) { // hack: coliny needs bestResponseArray
    // ------------------------------
    // Multiple point results summary
    // ------------------------------
    s << '\n';//<<<<< Optimizer generated "<<num_instances<<" solution sets:\n";
    for (size_t i=1; i<num_instances; i++) {
      s << "<<<<< Best parameters          (set " << i+1 << ") =\n"
	<< bestVariablesArray[i];
      const RealVector& fn_vals_star = bestResponseArray[i].function_values();
      if (numUserObjectiveFns > 1)
	s << "<<<<< Best objective functions (set " << i+1 << ") =\n";
      else
	s << "<<<<< Best objective function  (set " << i+1 << ") =\n";
      write_data_partial(s, 0, numUserObjectiveFns, fn_vals_star);
      size_t num_cons = numFunctions - numUserObjectiveFns;
      if (num_cons) {
	s << "<<<<< Best constraint values   (set " << i+1 << ") =\n";
	write_data_partial(s, numUserObjectiveFns, num_cons, fn_vals_star);
      }
      if (lookup_by_val(data_pairs, interface_id, bestVariablesArray[i],
      			search_set, eval_id))
	s << "<<<<< Best data captured at function evaluation " << eval_id
	  << "\n\n";
      else
	s << "<<<<< Best data not found in evaluation cache\n\n";
    }
  }
}


/** Retrieve a full multiobjective response based on the data returned
    by a single objective optimizer by performing a data_pairs search. */
void Optimizer::
multi_objective_retrieve(const Variables& vars, Response& response) const
{
  // only needed for multiobjective optimization
  if (numUserObjectiveFns <= 1)
    Cerr << "Warning: multi_objective_retrieve() called for single objective "
	 << "optimization." << std::endl;
  else {
    extern PRPCache data_pairs; // global container
    Response desired_resp;
    if (lookup_by_val(data_pairs, iteratedModel.interface_id(), vars,
		      response.active_set(), desired_resp))
      response.update(desired_resp);
    else
      Cerr << "Warning: failure in recovery of final values for multiobjective "
	   << "optimization." << std::endl;
  }
}


/** Objective function map from user/native space to iterator/scaled/combined
    space using a RecastModel.  If resizing the response, copies the 
    constraint (secondary) data from native_response too */
void Optimizer::
primary_resp_recast(const Variables& native_vars,
		    const Variables& scaled_vars,
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
      optimizerInstance->numUserObjectiveFns);

  // Keeping scaling and weighted_sum transformation separate since
  // may eventually want more general transformations (e.g.,
  // goal-oriented).  Using weighted_sum yields a resized
  // iterator_response, but does not copy constraints
  if (optimizerInstance->multiObjFlag && scale_transform_needed) {

    // tmp_response contains the intermediate result with scaled
    // functions only (indices 0:(numUserObjectiveFns-1)) then apply
    // weighted objective sum
    Response tmp_response = native_response.copy();
    optimizerInstance->response_modify_n2s(native_vars, native_response,
      tmp_response, 0, 0, optimizerInstance->numUserObjectiveFns);
    const Model& sub_model
      = optimizerInstance->iteratedModel.subordinate_model();
    const RealVector& wts  = sub_model.primary_response_fn_weights();
    optimizerInstance->weighted_sum(tmp_response, iterator_response, wts);

  }
  else if (scale_transform_needed)

    optimizerInstance->response_modify_n2s(native_vars, native_response,
      iterator_response, 0, 0, optimizerInstance->numUserObjectiveFns);

  else if (optimizerInstance->multiObjFlag) {

    const Model& sub_model
      = optimizerInstance->iteratedModel.subordinate_model();
    const RealVector& wts  = sub_model.primary_response_fn_weights();
    optimizerInstance->weighted_sum(native_response, iterator_response, wts);
  }
  else {
    // could reach this if variables are scaled and only functions are requested
    iterator_response.update_partial(0, optimizerInstance->numUserObjectiveFns,
				     native_response, 0);
    return;
  }

}


/** This function is responsible for the mapping of multiple objective
    functions into a single objective for publishing to single-objective
    optimizers.  Used in DOTOptimizer, NPSOLOptimizer, SNLLOptimizer, and
    SGOPTApplication on every function evaluation.  The simple weighting
    approach (using primaryRespFnWts) is the only technique supported
    currently.  The weightings are used to scale function values,
    gradients, and Hessians as needed. */
void Optimizer::weighted_sum(const Response& full_response, 
			     Response& reduced_response,
			     const RealVector& multiobj_wts) const
{
  if (numUserObjectiveFns <= 1) {
    Cerr << "\nError: Optimizer::weighted_sum() reached with single objective." 
	 << std::endl;
    abort_handler(-1);
  }
  // Since we're no longer returning a new Response object, the following now
  // assumes the reduced_response is already prepared (arrays sized, asv 
  // populated, etc.)

  size_t i, j, k;
  const ShortArray& reduced_asv = reduced_response.active_set_request_vector();

  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Multiobjective transformation:\n";

  const RealVector& full_fn_vals = full_response.function_values();
  if (reduced_asv[0] & 1) {
    Real sum = 0.;
    if (multiobj_wts.empty()) {
      for (i=0; i<numUserObjectiveFns; i++)
	sum += full_fn_vals[i];
      sum /= (Real)numUserObjectiveFns; // default wt = 1/n
    }
    else {
      for (i=0; i<numUserObjectiveFns; i++)
	sum += full_fn_vals[i] * multiobj_wts[i];

    }
    reduced_response.function_value(sum, 0);    
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "                     " << std::setw(write_precision+7) << sum
	   << " obj_fn\n";
  }

  // build new_fn_grads from full_fn_grads
  const RealMatrix&  full_fn_grads  = full_response.function_gradients();
  RealMatrix new_fn_grads = reduced_response.function_gradients();
  if (new_fn_grads.numCols() != 0) { // gradients active
    if (reduced_asv[0] & 2) {
      if (outputLevel > NORMAL_OUTPUT)
	Cout << " [ ";
      for (j=0; j<numContinuousVars; j++) {
        Real sum = 0.;
	if (multiobj_wts.empty()) {
	  for (i=0; i<numUserObjectiveFns; i++)
	    sum += full_fn_grads[i][j];
	  new_fn_grads[0][j] = sum / (Real)numUserObjectiveFns;
	}
	else {
	  for (i=0; i<numUserObjectiveFns; i++)
	    sum += full_fn_grads[i][j] * multiobj_wts[i];
	  new_fn_grads[0][j] = sum;
	}
	if (outputLevel > NORMAL_OUTPUT)
	  Cout << std::setw(write_precision+7) << sum << ' ';
      }
      if (outputLevel > NORMAL_OUTPUT)
	Cout << " ] obj_fn gradient\n";
    }
    reduced_response.function_gradients(new_fn_grads);
  }

  // build new_fn_hessians from full_fn_hessians
  const RealSymMatrixArray& full_fn_hessians = full_response.function_hessians();
  RealSymMatrixArray new_fn_hessians = reduced_response.function_hessians();
  if (new_fn_hessians.size()) { // hessians active
    if (reduced_asv[0] & 4) {
      if (outputLevel > NORMAL_OUTPUT)
	Cout << "[[ ";
      for (j=0; j<numContinuousVars; j++) {
        for (k=0; k<numContinuousVars; k++) {
          Real sum = 0.;
	  if (multiobj_wts.empty()) {
	    for (i=0; i<numUserObjectiveFns; i++)
	      sum += full_fn_hessians[i](j,k);
	    new_fn_hessians[0](j,k) = sum / (Real)numUserObjectiveFns;
	  }
	  else {
	    for (i=0; i<numUserObjectiveFns; i++)
	      sum += full_fn_hessians[i](j,k) * multiobj_wts[i];
	    new_fn_hessians[0](j,k) = sum;
	  }
	  if (outputLevel > NORMAL_OUTPUT)
	    Cout << std::setw(write_precision+7) << sum << ' ';
	}
	if (j != numContinuousVars-1)
	  if (outputLevel > NORMAL_OUTPUT)
	    Cout << "\n   ";
      }
      if (outputLevel > NORMAL_OUTPUT)
	Cout << " ]] obj_fn Hessian\n";
    }
    reduced_response.function_hessians(new_fn_hessians);
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
  if (scaleFlag || multiObjFlag)
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

    // transform variables back to user space (for multiobj or scaling)
    if (varsScaleFlag)
      best_vars.continuous_variables(
        modify_s2n(best_vars.continuous_variables(), cvScaleTypes,
		   cvScaleMultipliers, cvScaleOffsets));

    // transform responses back to user space (for multiobj or scaling)
    if (multiObjFlag && methodName != "moga")
      multi_objective_retrieve(best_vars, best_resp);
    else if (primaryRespScaleFlag || secondaryRespScaleFlag) {

      Response tmp_response = best_resp.copy();
      if (primaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(), 0,
				 numUserObjectiveFns)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, 0, 0,
			    numUserObjectiveFns);
	best_resp.update_partial(0, numUserObjectiveFns, tmp_response, 0 );
      }
      if (secondaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(),
				 numUserObjectiveFns, 
				 numNonlinearConstraints)) {
	response_modify_s2n(best_vars, best_resp, tmp_response,
			    numUserObjectiveFns, numUserObjectiveFns,
			    numNonlinearConstraints);
	best_resp.update_partial(numUserObjectiveFns, numNonlinearConstraints,
				 tmp_response, numUserObjectiveFns);
      }
    }

  }

  Iterator::post_run(s);
}

} // namespace Dakota
