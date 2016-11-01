/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
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


Optimizer::Optimizer(ProblemDescDB& problem_db, Model& model):
  Minimizer(problem_db, model),
  // initial value from Minimizer as accounts for fields and transformations
  numObjectiveFns(numUserPrimaryFns), localObjectiveRecast(false)
{
  optimizationFlag = true; // default; may be overridden below

  bool err_flag = false;
  // Check for correct bit associated within methodName
  if ( !(methodName & OPTIMIZER_BIT) ) {
    Cerr << "\nError: optimizer bit not activated for method instantiation "
	 << "within Optimizer branch." << std::endl;
    err_flag = true;
  }
  // Check for bound constraint support/requirement in method selection
  if ( boundConstraintFlag && methodName == OPTPP_CG ) {
    Cerr << "\nError: bound constraints not currently supported by "
         << method_enum_to_string(methodName) << ".\n       Please select a "
	 << "different method for bound constrained problems." << std::endl;
    err_flag = true;
  }
  // RWH: Check for finite bound constraints for Coliny global optimizers
  if ( (methodName == COLINY_DIRECT) || (methodName == COLINY_EA) ) {
    const RealVector& c_l_bnds = model.continuous_lower_bounds();
    const RealVector& c_u_bnds = model.continuous_upper_bounds();
    for (size_t i=0; i<numContinuousVars; ++i)
      if (c_l_bnds[i] <= -bigRealBoundSize || c_u_bnds[i] >= bigRealBoundSize) {
        Cerr << "\nError: finite bound constraints are required for global optimizer "
          << method_enum_to_string(methodName) << ".\n       Please specify both "
          << "lower_bounds and upper_bounds." << std::endl;
        err_flag = true;
        break;
      }
  }
 
  // Check for nongradient method with speculative flag set
  if ( speculativeFlag && methodName < NONLINEAR_CG ) {
    Cerr << "\nWarning: Speculative specification for a nongradient-based "
	 << "optimizer is ignored.\n";
    speculativeFlag = false;
  }

  // Check for full Newton method w/o Hessian support (direct or via recast)
  bool require_hessians = false;
  bool have_lsq = (model.primary_fn_type() == CALIB_TERMS);
  if (methodName == OPTPP_NEWTON) { // || ...) {
    require_hessians = true;
    if ( ( !have_lsq && iteratedModel.hessian_type()  == "none" ) ||
	 (  have_lsq && iteratedModel.gradient_type() == "none" ) ) {
      Cerr << "\nError: full Newton optimization requires objective Hessians."
	   << std::endl;
      err_flag = true;
    }
  }

  // Initialize a best variables instance; bestVariablesArray should
  // be in calling context; so initialized before any recasts
  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  // Check for proper response function definition (optimization or
  // calibration) and manage local recasting (necessary if inbound
  // model contains calibration terms)
  if (have_lsq) {
    // use local recast to solve NLS problems as single-objective optimization
    Cerr << "Warning: coercing least squares data set into optimization data "
	 << "set." << std::endl;
    optimizationFlag = false; // used to distinguish NLS from MOO
    localObjectiveRecast = true;
  }
  else if (model.primary_fn_type() == OBJECTIVE_FNS) {
    // we allow SOGA to manage weighted multiple objectives where
    // possible, so we can better retrieve final results
    if (numUserPrimaryFns > 1 && 
	(methodName != MOGA && methodName != SOGA))
      localObjectiveRecast = true;
  }
  else {
    Cerr << "\nError: responses specification is incompatible with "
	 << "optimization methods." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  // TODO: can't allow experimental data with SBNLS for now!  Can we
  // data transform, scale, weight?

  // Wrap the iteratedModel in 0 -- 3 RecastModels, potentially resulting
  // in reduce(scale(data(model)))
  if (calibrationDataFlag)
    data_transform_model();
  if (scaleFlag)
    scale_model();
  if (localObjectiveRecast)
    reduce_model(have_lsq, require_hessians);
}


Optimizer::Optimizer(unsigned short method_name, Model& model):
  Minimizer(method_name, model), numObjectiveFns(numUserPrimaryFns),
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
Optimizer(unsigned short method_name, size_t num_cv, size_t num_div,
	  size_t num_dsv, size_t num_drv, size_t num_lin_ineq,
	  size_t num_lin_eq, size_t num_nln_ineq, size_t num_nln_eq):
  Minimizer(method_name, num_lin_ineq, num_lin_eq, num_nln_ineq, num_nln_eq),
  numObjectiveFns(1), localObjectiveRecast(false)
{
  numContinuousVars     = num_cv;
  numDiscreteIntVars    = num_div;
  numDiscreteStringVars = num_dsv;
  numDiscreteRealVars   = num_drv;
  numFunctions          = numUserPrimaryFns + numNonlinearConstraints;
  optimizationFlag      = true;

  // The following "best" initializations are done here instead of in
  // Minimizer for this lightweight case
  std::pair<short,short> view(MIXED_DESIGN, EMPTY_VIEW);
  SizetArray vc_totals(NUM_VC_TOTALS, 0);
  vc_totals[TOTAL_CDV] = num_cv;  vc_totals[TOTAL_DDIV] = num_div;
  vc_totals[TOTAL_DDSV] = num_dsv; vc_totals[TOTAL_DDRV] = num_drv;
  BitArray all_relax_di, all_relax_dr; // empty: no relaxation of discrete
  SharedVariablesData svd(view, vc_totals, all_relax_di, all_relax_dr);
  bestVariablesArray.push_back(Variables(svd));

  activeSet.reshape(numFunctions, numContinuousVars);
  activeSet.request_values(1); activeSet.derivative_start_value(1);
  bestResponseArray.push_back(Response(SIMULATION_RESPONSE, activeSet));
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

  // must search in the inbound Model's space (and even that may not
  // suffice if there are additional recastings underlying this
  // Optimizer's Model) to find the function evaluation ID number
  Model orig_model = original_model();
  const String& interface_id = orig_model.interface_id(); 
  // use asv = 1's
  ActiveSet search_set(orig_model.num_functions(), numContinuousVars);
 
  // -------------------------------------
  // Single and Multipoint results summary
  // -------------------------------------
  for (i=0; i<num_best; ++i) { 
    // output best variables
    const Variables& best_vars = bestVariablesArray[i];
    s << "<<<<< Best parameters          "; 
    if (num_best > 1) s << "(set " << i+1 << ") "; 
    s << "=\n" << best_vars;
    
    // output best response
    // TODO: based on local_nls_recast due to SurrBasedMinimizer?
    const RealVector& best_fns = bestResponseArray[i].function_values(); 

    if (optimizationFlag) {
      if (numUserPrimaryFns > 1) s << "<<<<< Best objective functions "; 
      else                       s << "<<<<< Best objective function  "; 
      if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n"; 
      write_data_partial(s, (size_t)0, numUserPrimaryFns, best_fns); 
    }
    else {
      if (calibrationDataFlag) {
        // TODO: approximate models with interpolation of field data may
        // not have recovered the correct best residuals
        DataTransformModel* dt_model_rep = 
          static_cast<DataTransformModel*>(dataTransformModel.model_rep());
        dt_model_rep->print_best_responses(s, best_vars, bestResponseArray[i],
                                           num_best, i);
      }
      else {
        // the original model had least squares terms
        const RealVector& lsq_weights 
          = orig_model.primary_response_fn_weights();
        print_residuals(numUserPrimaryFns, best_fns, lsq_weights, 
                        num_best, i, s);
      }
    }

    if (numNonlinearConstraints) { 
      s << "<<<<< Best constraint values   "; 
      if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n"; 
      write_data_partial(s, numUserPrimaryFns, numNonlinearConstraints, 
                         best_fns);
    } 
    // lookup evaluation id where best occurred.  This cannot be catalogued
    // directly because the optimizers track the best iterate internally and
    // return the best results after iteration completion.  Therfore, perform a
    // search in data_pairs to extract the evalId for the best fn eval.
    PRPCacheHIter cache_it = lookup_by_val(data_pairs, interface_id,
                                           best_vars, search_set);
    if (cache_it == data_pairs.get<hashed>().end())
      s << "<<<<< Best data not found in evaluation cache\n\n";
    else {
      int eval_id = cache_it->eval_id();
      if (eval_id > 0)
	s << "<<<<< Best data captured at function evaluation " << eval_id
	  << "\n\n";
      else // should not occur
	s << "<<<<< Best data not found in evaluations from current execution,"
	  << "\n      but retrieved from restart archive with evaluation id "
	  << -eval_id << "\n\n";
    }

    // pass data to the results archive
    archive_best(i, best_vars, bestResponseArray[i]);
  }
}


/** Reduce model for least-squares or multi-objective transformation.
    Doesn't map variables, or secondary responses.  Maps active set
    for Gauss-Newton.  Maps primary responses to single objective so
    user vs. iterated matters. */
void Optimizer::reduce_model(bool local_nls_recast, bool require_hessians)
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing reduction transformation" << std::endl;

  // numObjectiveFns is as seen by this Iterator after reduction
  // update Minimizer sizes as well
  numIterPrimaryFns = numObjectiveFns = 1;
  numFunctions = numObjectiveFns + numNonlinearConstraints;

  size_t i, num_recast_fns = numFunctions;
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
  if (local_nls_recast) {
    // TODO: when reduce_model is called, model.primary_functions() should
    // have the right size... Add model.secondary_functions() as well?
    size_t total_calib_terms = (calibrationDataFlag) ?
      numTotalCalibTerms : numUserPrimaryFns;
    primary_resp_map_indices[0].resize(total_calib_terms);
    nonlinear_resp_map[0].resize(total_calib_terms);
    for (i=0; i<total_calib_terms; i++) {
      primary_resp_map_indices[0][i] = i;
      nonlinear_resp_map[0][i] = (local_nls_recast);
    }

    // adjust active set vector to 1 + numNonlinearConstraints
    ShortArray asv(1 + numNonlinearConstraints, 1);
    activeSet.request_vector(asv);

    for (i=0; i<numNonlinearConstraints; i++) {
      secondary_resp_map_indices[i].resize(1);
      secondary_resp_map_indices[i][0] = total_calib_terms + i;
      nonlinear_resp_map[numObjectiveFns+i].resize(1);
      nonlinear_resp_map[numObjectiveFns+i][0] = false;
    }
  }
  else {
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
  }
  void (*vars_recast) (const Variables&, Variables&) = NULL;
  // recast active set if needed for Gauss-Newton LSQ
  void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&)
    = (local_nls_recast && require_hessians &&
       iteratedModel.hessian_type() == "none") ? gnewton_set_recast : NULL;
  void (*pri_resp_recast) (const Variables&, const Variables&,
                           const Response&, Response&) = primary_resp_reducer;
  void (*sec_resp_recast) (const Variables&, const Variables&,
                           const Response&, Response&) = NULL;

  size_t recast_secondary_offset = numNonlinearIneqConstraints;
  SizetArray recast_vars_comps_total; // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  const Response& orig_resp = iteratedModel.current_response();
  short recast_resp_order = 1; // may differ from orig response
  if (!orig_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (require_hessians)                        recast_resp_order |= 4;

  iteratedModel.assign_rep(new
    RecastModel(iteratedModel, var_map_indices, recast_vars_comps_total, 
		all_relax_di, all_relax_dr, nonlinear_vars_map, vars_recast,
		set_recast, primary_resp_map_indices,
		secondary_resp_map_indices, recast_secondary_offset,
		recast_resp_order, nonlinear_resp_map, pri_resp_recast,
		sec_resp_recast), false);
  ++myModelLayers;


  // if Gauss-Newton recasting, then the RecastModel Response needs to
  // allocate space for a Hessian (default copy of sub-model response
  // is insufficient).
  if (set_recast) {
    Response recast_resp = iteratedModel.current_response(); // shared rep
    recast_resp.reshape(num_recast_fns, numContinuousVars, true, true);
  }

  // this recast results in a single primary response of type objective
  iteratedModel.primary_fn_type(OBJECTIVE_FNS);

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
  // BMA TODO: review whether the model should provide all this information
  size_t num_fns = full_response.num_functions() - numConstraints;
  short reduced_asv0 = reduced_response.active_set_request_vector()[0];
  if (reduced_asv0 & 1) { // build objective fn from full_response functions
    Real sum = objective(full_response.function_values(), num_fns, sense, full_wts);
    reduced_response.function_value(sum, 0);    
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "                     " << std::setw(write_precision+7) << sum
	   << " obj_fn\n";
  }

  if (reduced_asv0 & 2) { // build obj_grad from full_response gradients
    RealVector obj_grad = reduced_response.function_gradient_view(0);
    objective_gradient(full_response.function_values(), num_fns,
		       full_response.function_gradients(), sense, full_wts,
		       obj_grad);
    if (outputLevel > NORMAL_OUTPUT) {
      write_col_vector_trans(Cout, 0, reduced_response.function_gradients(),
			     true, true, false);
      Cout << " obj_fn gradient\n";
    }
  }

  if (reduced_asv0 & 4) { // build obj_hessian from full_response hessians
    RealSymMatrix obj_hessian = reduced_response.function_hessian_view(0);
    objective_hessian(full_response.function_values(), num_fns,
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

  // pull any late updates into the RecastModel; may need to update
  // from the underlying user model in case of hybrid methods, so
  // should recurse through any local transformations
  if (myModelLayers > 0)
    iteratedModel.update_from_subordinate_model(myModelLayers-1);

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
  // Note: Historically bestResponseArray data varied: simulation
  // responses or data-differenced residuals.  Now it's standardized
  // to return in the same space as the original inbound Model.

  // Default is to assume the Iterator posted in the transformed space
  // and either map back, or where needed, lookup.

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
    // (unreduce via lookup), unscale, remove data (lookup)

    // transform variables back to user space (for local obj recast or scaling)
    // must do before lookup in retrieve, which is in user space
    if (scaleFlag) {
      ScalingModel* scale_model_rep = 
        static_cast<ScalingModel*>(scalingModel.model_rep());
      best_vars.continuous_variables
        (scale_model_rep->cv_scaled2native(best_vars.continuous_variables()));
    }

    // transform primary responses back to user space via lookup for
    // local obj recast or simply via scaling transform
    
    // retrieve the user space
    // primary functions via lookup and unscale constraints if needed
    // this will retrieve primary functions of size best (possibly
    // expanded, but not differenced with replicate data)
    if (localObjectiveRecast) {

      local_recast_retrieve(best_vars, best_resp);
      // BMA TODO: if retrieved the best fns/cons from DB, why unscaling cons?
      // Would need this if looking up only primary fns in DB
      // if (secondaryRespScaleFlag || 
      // 	  need_resp_trans_byvars(best_resp.active_set_request_vector(),
      // 				 numUserPrimaryFns, numNonlinearConstraints)) {
      // 	Response tmp_response = best_resp.copy();
      // 	response_modify_s2n(best_vars, best_resp, tmp_response,
      // 			    numUserPrimaryFns, numNonlinearConstraints);
      // 	best_resp.update_partial(numUserPrimaryFns, numNonlinearConstraints,
      // 				 tmp_response, numUserPrimaryFns);
      // }
      //    }
    }
    // just unscale if needed
    else if (scaleFlag) {
      // ScalingModel manages which transformations are needed
      ScalingModel* scale_model_rep = 
        static_cast<ScalingModel*>(scalingModel.model_rep());
      scale_model_rep->resp_scaled2native(best_vars, best_resp);
    }
  }
  
  Minimizer::post_run(s);
}

} // namespace Dakota
