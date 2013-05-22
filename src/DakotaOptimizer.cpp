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

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaOptimizer.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "ProblemDescDB.hpp"
#include "RecastModel.hpp"

static const char rcsId[]="@(#) $Id: DakotaOptimizer.cpp 7031 2010-10-22 16:23:52Z mseldre $";


namespace Dakota {

extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
Optimizer* Optimizer::optimizerInstance(NULL);

/** This constructor extracts the inherited data for the optimizer
    branch and performs sanity checking on gradient and constraint
    settings. */
Optimizer::Optimizer(Model& model): Minimizer(model),
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

  bool local_nls_recast = false;  // recasting LSQ to Opt in Optimizer
  bool local_moo_recast = false;  // recasting multiple to single objective

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

  // update number of functions being Iterated; used at Minimizer level
  numIterPrimaryFns = numObjectiveFns; 
  // whether a local objective reduction is to be performed
  localObjectiveRecast = (local_nls_recast || local_moo_recast);
  // optimizationFlag indicates whether optimization or least-squares

  // Wrap the iteratedModel, which initially is the userDefinedModel,
  // in 0 -- 3 RecastModels, potentially resulting in reduce(scale(data(model)))

  // this might set weights based on exp std deviations!
  if (local_nls_recast && obsDataFlag) {
    data_transform_model(!iteratedModel.primary_response_fn_weights().empty());
    ++minimizerRecasts;
  }
  if (scaleFlag) {
    scale_model();
    ++minimizerRecasts;
  }
  if (localObjectiveRecast) {
    reduce_model(local_nls_recast, require_hessians);
    ++minimizerRecasts;
  }

  if (minimizerRecasts) {
    // for gradient-based Optimizers, maxConcurrency has already been determined
    // from derivative concurrency in the Iterator initializer, so initialize
    // communicators in the RecastModel.  For nongradient methods (many COLINY
    // methods, OPT++ PDS, eventually JEGA), maxConcurrency is defined in the
    // derived constructors, so init_communicators() is invoked there.
    if ( !( methodName == "optpp_pds" || methodName.begins("coliny_") ||
	    methodName == "moga"      || methodName == "soga" ) ) {
      bool recurse_flag = true;  // explicit default: recurse down models
      iteratedModel.init_communicators(maxConcurrency, recurse_flag);
    }
  }

  // Initialize a best variables instance; bestVariablesArray should
  // be in calling context; so initialized before any recasts
  bestVariablesArray.push_back(model.current_variables().copy());
}


Optimizer::Optimizer(NoDBBaseConstructor, Model& model):
  Minimizer(NoDBBaseConstructor(), model), numObjectiveFns(numUserPrimaryFns),
  localObjectiveRecast(false)
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
  numObjectiveFns(1), localObjectiveRecast(false)
{
  numContinuousVars   = num_cv;
  numDiscreteIntVars  = num_div;
  numDiscreteRealVars = num_drv;
  numFunctions        = numUserPrimaryFns + numNonlinearConstraints;
  optimizationFlag    = true;

  // The following "best" initializations are done here instead of in
  // Minimizer for this lightweight case
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

  // initialize the results archive for this dataset
  archive_allocate_best(num_best);

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
    
    // TODO: based on local_nls_recast due to SurrBasedMinimizer?
    const RealVector& best_fns = bestResponseArray[i].function_values(); 
    if (optimizationFlag) {
      if (numUserPrimaryFns > 1) s << "<<<<< Best objective functions "; 
      else                       s << "<<<<< Best objective function  "; 
      if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n"; 
      write_data_partial(s, 0, numUserPrimaryFns, best_fns); 
    }
    else {
      Real t = 0.;
      for(size_t j=0; j<numUserPrimaryFns; ++j) {
	const Real& t1 = best_fns[j];
	t += t1*t1;
      }
      s << "<<<<< Best residual norm ";
      if (num_best > 1) s << "(set " << i+1 << ") ";
      s << "= " << std::setw(write_precision+7)
	<< std::sqrt(t) << "; 0.5 * norm^2 = " << std::setw(write_precision+7)
	<< 0.5*t << '\n';
      if (numUserPrimaryFns > 1) s << "<<<<< Best residual terms "; 
      else                       s << "<<<<< Best residual term  "; 
      if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n"; 
      write_data_partial(s, 0, numUserPrimaryFns, best_fns); 
    }

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

    // pass data to the results archive
    archive_best(i, bestVariablesArray[i], bestResponseArray[i]);

  } 
}


/** Retrieve a MOO/NLS response based on the data returned by a single
    objective optimizer by performing a data_pairs search. This may
    get called even for a single user-specified function, since we may
    be recasting a single NLS residual into a squared objective. */
void Optimizer::
local_objective_recast_retrieve(const Variables& vars, Response& response) const
{
  // TODO: if reduced, the active set may have the wrong size
  Response desired_resp;
  if (lookup_by_val(data_pairs, iteratedModel.interface_id(), vars,
		    response.active_set(), desired_resp))
    response.update(desired_resp);
  else
    Cerr << "Warning: failure in recovery of final values for locally recast "
	 << "optimization." << std::endl;
}



/** Reduce model for least-squares or multi-objective transformation.
    Doesn't map variables, or secondary responses.  Maps active set
    for Gauss-Newton.  Maps primary responses to single objective so
    user vs. iterated matters. */
void Optimizer::reduce_model(bool local_nls_recast, bool require_hessians)
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing reduction transformation" << std::endl;

  // numObjectiveFns is as seen by this iteration after reduction
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
  }

  // reduce to a single primary response, with all user primary
  // responses contributing
  primary_resp_map_indices[0].resize(numUserPrimaryFns);
  nonlinear_resp_map[0].resize(numUserPrimaryFns);
  for (i=0; i<numUserPrimaryFns; i++) {
    primary_resp_map_indices[0][i] = i;
    nonlinear_resp_map[0][i] = (local_nls_recast);
  }

  // adjust active set vector to 1 + numNonlinearConstraints
  ShortArray asv(1 + numNonlinearConstraints, 1);
  activeSet.request_vector(asv);

  for (i=0; i<numNonlinearConstraints; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = numUserPrimaryFns + i;
    nonlinear_resp_map[numObjectiveFns+i].resize(1);
    nonlinear_resp_map[numObjectiveFns+i][0] = false;
  }

  void (*vars_recast) (const Variables&, Variables&) = NULL;
  // recast active set if needed for Gauss-Newton LSQ
  void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&)
    = (local_nls_recast && require_hessians && hessianType == "none") ?
    gnewton_set_recast : NULL;
  void (*pri_resp_recast) (const Variables&, const Variables&,
			   const Response&, Response&) = primary_resp_reducer;
  void (*sec_resp_recast) (const Variables&, const Variables&,
			   const Response&, Response&) = secondary_resp_copier;

  size_t recast_secondary_offset = numNonlinearIneqConstraints;
  SizetArray recast_vars_comps_total; // default: empty; no change in size

  iteratedModel.assign_rep(new
    RecastModel(iteratedModel, var_map_indices, recast_vars_comps_total, 
		nonlinear_vars_map, vars_recast, set_recast, 
		primary_resp_map_indices, secondary_resp_map_indices, 
		recast_secondary_offset, nonlinear_resp_map, 
		pri_resp_recast, sec_resp_recast), false);

  // if Gauss-Newton recasting, then the RecastModel Response needs to
  // allocate space for a Hessian (default copy of sub-model response
  // is insufficient).
  if (set_recast) {
    Response recast_resp = iteratedModel.current_response(); // shared rep
    recast_resp.reshape(num_recast_fns, numContinuousVars, true, true);
  }

  // This transformation consumes weights, so the resulting wrapped
  // model doesn't need them any longer, however don't want to recurse
  // and wipe out in sub-models.  Be explicit in case later
  // update_from_sub_model is used instead.
  bool recurse_flag = false;
  iteratedModel.primary_response_fn_weights(RealVector(), recurse_flag);

  // an empty RecastModel::primaryRespFnSense would be sufficient
  // (reflects the minimize default), but might as well be explicit.
  BoolDeque max_sense(1, false);
  iteratedModel.primary_response_fn_sense(max_sense);

}


/** Objective function map from multiple primary responses (objective
    or residuals) to a single objective. Currently supports weighted
    sum; may later want more general transformations, e.g.,
    goal-oriented */
void Optimizer::
primary_resp_reducer(const Variables& full_vars, const Variables& reduced_vars,
		     const Response& full_response,
		     Response& reduced_response)
{
  if (optimizerInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n--------------------------------------------------------";
    Cout << "\nPost-processing Function Evaluation: Objective Reduction";
    Cout << "\n--------------------------------------------------------" 
	 << std::endl;
  }

  Model& sub_model = optimizerInstance->iteratedModel.subordinate_model();
  optimizerInstance->
    objective_reduction(full_response, sub_model.primary_response_fn_sense(),
			sub_model.primary_response_fn_weights(), 
			reduced_response);
}


/** This function is responsible for the mapping of multiple objective
    functions into a single objective for publishing to single-objective
    optimizers.  Used in DOTOptimizer, NPSOLOptimizer, SNLLOptimizer, and
    SGOPTApplication on every function evaluation.  The simple weighting
    approach (using primaryRespFnWts) is the only technique supported
    currently.  The weightings are used to scale function values,
    gradients, and Hessians as needed. */
void Optimizer::objective_reduction(const Response& full_response,
				    const BoolDeque& sense, 
				    const RealVector& full_wts, 
				    Response& reduced_response) const
{
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "Local single objective transformation:\n";

  short reduced_asv0 = reduced_response.active_set_request_vector()[0];
  if (reduced_asv0 & 1) { // build objective fn from full_response functions
    Real sum = objective(full_response.function_values(), sense, full_wts);
    reduced_response.function_value(sum, 0);    
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "                     " << std::setw(write_precision+7) << sum
	   << " obj_fn\n";
  }

  if (reduced_asv0 & 2) { // build obj_grad from full_response gradients
    RealVector obj_grad = reduced_response.function_gradient_view(0);
    objective_gradient(full_response.function_values(),
		       full_response.function_gradients(), sense, full_wts,
		       obj_grad);
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
		      full_response.function_hessians(), sense, full_wts,
		      obj_hessian);
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

  // TODO: This predates the multi-recast, so only goes down zero or
  // one levels.  Further recursion and possible transformations are
  // needed in the multi-recast case.  Can we make this update
  // sufficiently flexible to catch the subIterator and inactive case?

  // pull any late updates into the RecastModel
  if (minimizerRecasts)
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

  // Note: bestArrays are in iteratorSpace which may be single
  // objective and we don't unapply any data transformation on
  // residuals as the user may want to see them.

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

    // Reverse transformations on each point in best data: expand
    // (unreduce), unscale, but leave differenced with data (for
    // LeastSq problems, always report the residuals)

    // transform variables back to user space (for local obj recast or scaling)
    // must do before lookup in retrieve, which is in user space
    if (varsScaleFlag)
      best_vars.continuous_variables(
        modify_s2n(best_vars.continuous_variables(), cvScaleTypes,
		   cvScaleMultipliers, cvScaleOffsets));

    // transform primary responses back to user space via lookup for
    // local obj recast or simply via scaling transform
    
    // retrieve the user space primary functions via lookup and
    // unscale constraints if needed
    if (localObjectiveRecast) {
      local_objective_recast_retrieve(best_vars, best_resp);
      if (secondaryRespScaleFlag || 
	  need_resp_trans_byvars(best_resp.active_set_request_vector(),
				 numUserPrimaryFns, numNonlinearConstraints)) {
	Response tmp_response = best_resp.copy();
	response_modify_s2n(best_vars, best_resp, tmp_response,
			    numUserPrimaryFns, numNonlinearConstraints);
	best_resp.update_partial(numUserPrimaryFns, numNonlinearConstraints,
				 tmp_response, numUserPrimaryFns);
      }
    }
    // just unscale if needed
    else if (primaryRespScaleFlag || secondaryRespScaleFlag ||
	     need_resp_trans_byvars(best_resp.active_set_request_vector(), 0,
				    numUserPrimaryFns+numNonlinearConstraints)){
      Response tmp_response = best_resp.copy();
      if (primaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(), 0,
				 numUserPrimaryFns)) {
	response_modify_s2n(best_vars, best_resp, tmp_response, 0, 
			    numUserPrimaryFns);
	best_resp.update_partial(0, numUserPrimaryFns, tmp_response, 0 );
      }
      if (secondaryRespScaleFlag || 
	  need_resp_trans_byvars(tmp_response.active_set_request_vector(),
				 numUserPrimaryFns, numNonlinearConstraints)) {
	response_modify_s2n(best_vars, best_resp, tmp_response,
			    numUserPrimaryFns, numNonlinearConstraints);
	best_resp.update_partial(numUserPrimaryFns, numNonlinearConstraints,
				 tmp_response, numUserPrimaryFns);
      }
    }

    // if looked up in DB, need to reapply the data transformation so
    // user will see final residuals
    if (/* local_nls_recast && (implicit in localObjectiveRecast) */ 
	localObjectiveRecast && obsDataFlag) {
      const RealVector& fn_vals = best_resp.function_values();
      for (size_t i=0; i<numUserPrimaryFns; ++i)
	best_resp.function_value(fn_vals[i] - obsData[i], i);
    }
 
  }

  Iterator::post_run(s);
}

} // namespace Dakota
