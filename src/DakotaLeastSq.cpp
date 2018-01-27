/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       LeastSq
//- Description: Implementation code for the LeastSq class
//- Owner:       Mike Eldred
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DataTransformModel.hpp"
#include "ScalingModel.hpp"
#include "DakotaLeastSq.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "ProblemDescDB.hpp"
#include "RecastModel.hpp"
#include "Teuchos_LAPACK.hpp"
#include "pecos_stat_util.hpp"

static const char rcsId[]="@(#) $Id: DakotaLeastSq.cpp 7031 2010-10-22 16:23:52Z mseldre $";


using namespace std;

namespace Dakota {
  extern PRPCache data_pairs; // global container

// initialization of static needed by RecastModel
LeastSq* LeastSq::leastSqInstance(NULL);

/** This constructor extracts the inherited data for the least squares
    branch and performs sanity checking on gradient and constraint
    settings. */
LeastSq::LeastSq(ProblemDescDB& problem_db, Model& model, std::shared_ptr<TraitsBase> traits):
  Minimizer(problem_db, model, traits),
  // initial value from Minimizer as accounts for fields and transformations
  numLeastSqTerms(numUserPrimaryFns),
  weightFlag(!iteratedModel.primary_response_fn_weights().empty()),
				// TODO: wrong because of recasting layers
  retrievedIterPriFns(false)
{
  optimizationFlag  = false;

  bool err_flag = false;
  // Check for proper function definition
  if (model.primary_fn_type() != CALIB_TERMS) {
    Cerr << "\nError: model must have calibration terms to apply least squares "
	 << "methods." << std::endl;
    err_flag = true;
  }
  // Check for correct bit associated within methodName
  if ( !(methodName & LEASTSQ_BIT) ) {
    Cerr << "\nError: least squares bit not activated for method instantiation "
	 << "within LeastSq branch." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  // Initialize a best variables instance; bestVariablesArray should
  // be in calling context; so initialized before any recasts
  bestVariablesArray.push_back(iteratedModel.current_variables().copy());

  // Wrap the iteratedModel in 0 -- 3 RecastModels, potentially resulting
  // in weight(scale(data(model)))
  if (calibrationDataFlag) {
    data_transform_model();
    // update local data sizes
    numLeastSqTerms = numTotalCalibTerms;
  }
  if (scaleFlag)
    scale_model();
  if (weightFlag)
    weight_model();
}


LeastSq::LeastSq(unsigned short method_name, Model& model, std::shared_ptr<TraitsBase> traits):
  Minimizer(method_name, model, traits),
  numLeastSqTerms(numFunctions - numNonlinearConstraints),
  weightFlag(false) //(!model.primary_response_fn_weights().empty()), // TO DO
{
  bool err_flag = false;
  // Check for proper function definition
  if (numLeastSqTerms <= 0) {
    Cerr << "\nError: number of least squares terms must be greater than zero "
         << "for least squares methods." << std::endl;
    err_flag = true;
  }

  if (!model.primary_response_fn_weights().empty()) { // TO DO: support this
    Cerr << "Error: on-the-fly LeastSq instantiations do not currently support "
	 << "residual weightings." << std::endl;
    err_flag = true;
  }

  if (err_flag)
    abort_handler(-1);

  optimizationFlag  = false;

  // Initialize a best variables instance
  bestVariablesArray.push_back(model.current_variables().copy());
}


/** Setup Recast for weighting model.  The weighting transformation
    doesn't resize, so use numUserPrimaryFns.  No vars, active set or
    secondary mapping.  All indices are one-to-one mapped (no change
    in counts). */
void LeastSq::weight_model()
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing weighting transformation" << std::endl;

  // we assume sqrt(w_i) will be applied to each residual, therefore:
  const RealVector& lsq_weights = iteratedModel.primary_response_fn_weights();
  for (int i=0; i<lsq_weights.length(); ++i)
    if (lsq_weights[i] < 0) {
      Cerr << "\nError: Calibration term weights must be nonnegative. Specified "
	   << "weights are:\n" << lsq_weights << '\n';
      abort_handler(-1);
    }

  // the size of the recast is the total numLeastSqTerms being iterated
  size_t i;
  Sizet2DArray var_map_indices(numContinuousVars), 
    primary_resp_map_indices(numLeastSqTerms), 
    secondary_resp_map_indices(numNonlinearConstraints);
  bool nonlinear_vars_map = false;
  BoolDequeArray nonlinear_resp_map(numLeastSqTerms+numNonlinearConstraints);

  for (i=0; i<numContinuousVars; i++) {
    var_map_indices[i].resize(1);
    var_map_indices[i][0] = i;
  }
  for (i=0; i<numLeastSqTerms; i++) {
    primary_resp_map_indices[i].resize(1);
    primary_resp_map_indices[i][0] = i;
    nonlinear_resp_map[i].resize(1);
    nonlinear_resp_map[i][0] = false;
  }
  for (i=0; i<numNonlinearConstraints; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = numLeastSqTerms + i;
    nonlinear_resp_map[numLeastSqTerms+i].resize(1);
    nonlinear_resp_map[numLeastSqTerms+i][0] = false;
  }

  void (*vars_recast) (const Variables&, Variables&) = NULL;
  void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
  void (*pri_resp_recast) (const Variables&, const Variables&,
                           const Response&, Response&)
    = primary_resp_weighter;
  void (*sec_resp_recast) (const Variables&, const Variables&,
                           const Response&, Response&) = NULL;

  size_t recast_secondary_offset = numNonlinearIneqConstraints;
  SizetArray recast_vars_comps_total;  // default: empty; no change in size
  BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relaxation
  const Response& curr_resp = iteratedModel.current_response();
  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  iteratedModel.assign_rep(new
    RecastModel(iteratedModel, var_map_indices, recast_vars_comps_total, 
		all_relax_di, all_relax_dr, nonlinear_vars_map, vars_recast,
		set_recast, primary_resp_map_indices,
		secondary_resp_map_indices, recast_secondary_offset,
		recast_resp_order, nonlinear_resp_map, pri_resp_recast,
		sec_resp_recast), false);
  ++myModelLayers;

  // This transformation consumes weights, so the resulting wrapped
  // model doesn't need them any longer, however don't want to recurse
  // and wipe out in sub-models.  Be explicit in case later
  // update_from_model() is used instead.
  bool recurse_flag = false;
  iteratedModel.primary_response_fn_weights(RealVector(), recurse_flag);

  // Preserve sense through the weighting transformation.
  const BoolDeque& submodel_sense = 
    iteratedModel.subordinate_model().primary_response_fn_sense();
  iteratedModel.primary_response_fn_sense(submodel_sense);
}


/** Redefines default iterator results printing to include nonlinear
    least squares results (residual terms and constraints). */
void LeastSq::print_results(std::ostream& s, short results_state)
{
  // Seq: print from native variables through to residuals as worked on:
  //  * Best native parameters: LeastSq::print_results
  //
  //  * IF DataTransform: DataTransformModel::print_best_responses
  //     - Best configs and native model responses (residuals or
  //       functions, prior to data transform),
  //     - [covariance-weighted] residuals
  //     - residual norms
  //
  //    ELSE: Accounted for by final iterator space residuals
  //
  //  * Skip: scaled residuals
  //
  //  * Final iterator space residuals and residual norms (accounting
  //    for scaling/weighting): DakotaLeastSq::print_results

  // TODO: Need to add residual norm to baselines

  // archive the single best point
  size_t num_best = 1, best_ind = 0;
  archive_allocate_best(num_best);
  archive_best(best_ind, bestVariablesArray.front(), bestResponseArray.front());

  // Print best calibration parameters.  Include any inactive
  // variables unless they are used as experiment configuration
  // variables since there's no "best" configuration.
  const Variables& best_vars = bestVariablesArray.front();
  if (expData.config_vars().size() == 0)
    s << "<<<<< Best parameters          =\n" << best_vars;
  else {
    s << "<<<<< Best parameters (experiment config variables omitted) =\n";
    bool active_only = true;
    best_vars.write(s, ACTIVE_VARS);
  }

  // after post-run, responses should be back in user model space
  // (no// scaling, or weighting); however TODO: they don't account
  // for the multiple config cases.  They do contain the user-space
  // constraints in all cases.
  const RealVector& best_fns = bestResponseArray.front().function_values();

  if (calibrationDataFlag) {
    // This will print the original model responses (possibly
    // per-config) and the full set of data-transformed residuals,
    // possibly scaled by sigma^-1/2.  This should be correct in
    // interpolation cases as well.
    DataTransformModel* dt_model_rep =
      static_cast<DataTransformModel*>(dataTransformModel.model_rep());
    dt_model_rep->print_best_responses(s, best_vars, bestResponseArray.front(),
                                       num_best, best_ind);
  }
  else {
    // otherwise the residuals worked on are same size/type as the
    // inbound Model, that is the original model had least squares
    // terms and numLeastSqTerms == numTotalCalibTerms

    // only need clarify if transformations
    if (scaleFlag || weightFlag)
      s << "Original (as-posed) response:\n";
    // always print the native residuals
    print_residuals(numUserPrimaryFns, best_fns, RealVector(),
		    num_best, best_ind, s);
  }

  // TODO: this may be per-experiment configuration, but there is no
  // management of it in the problem formulation.  Need to explicitly
  // disallow.
  if (numNonlinearConstraints) {
    s << "<<<<< Best constraint values   =\n";
    write_data_partial(s, numUserPrimaryFns, numNonlinearConstraints, best_fns);
  }

  // Print fn. eval. number where best occurred.  This cannot be catalogued 
  // directly because the optimizers track the best iterate internally and 
  // return the best results after iteration completion.  Therfore, perform a
  // search in the data_pairs cache to extract the evalId for the best fn. eval.

  // TODO: for multi-config, there won't be a single best would have
  // to check whether there are distinct configs

  // must search in the inbound Model's space (and even that may not
  // suffice if there are additional recastings underlying this
  // solver's Model) to find the function evaluation ID number
  Model orig_model = original_model();
  const String& interface_id = orig_model.interface_id(); 
  // use asv = 1's
  ActiveSet search_set(orig_model.num_functions(), numContinuousVars);

  activeSet.request_values(1);
  PRPCacheHIter cache_it = lookup_by_val(data_pairs,
    iteratedModel.interface_id(), best_vars, activeSet);
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
 
  // Print confidence intervals for each estimated parameter. 
  // These CIs are based on a linear approximation of the underlying 
  // nonlinear model, and are individual CIs, not joint CIs.  
  // Currently there is no weighting matrix, but that can be added. 

  if (!confBoundsLower.empty() && !confBoundsUpper.empty()) {
    // BMA: Not sure this warning is needed
    if (expData.num_experiments() > 1) {
      s << "Warning: Confidence intervals may be inaccurate when "
        << "num_experiments > 1\n";
    }

    s << "Confidence Intervals on Calibrated Parameters:\n";

    StringMultiArrayConstView cv_labels
      = iteratedModel.continuous_variable_labels();
    for (size_t i = 0; i < numContinuousVars; i++)
      s << std::setw(14) << cv_labels[i] << ": [ "
	<< setw(write_precision+6) << confBoundsLower[i] << ", "
	<< setw(write_precision+6) << confBoundsUpper[i] << " ]\n";
  }
}


/** Apply weights to least squares residuals */
void LeastSq::
primary_resp_weighter(const Variables& unweighted_vars,
		      const Variables& weighted_vars,
		      const Response& unweighted_response,
		      Response& weighted_response)
{
  if (leastSqInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n--------------------------------------------------------";
    Cout << "\nPost-processing Function Evaluation: Weighting Residuals";
    Cout << "\n--------------------------------------------------------" 
	 << std::endl;
  }

  // TODO: is the DVV management necessary here?

  // apply any weights as necessary (consider code reuse with multi-objective?)
  // for now, recast models don't have the appropriate data for weights
  // TO DO: consider adding to the update from subordinate

  // weighting is the last transformation, so weights are available in
  // the first sub-model
  const RealVector& lsq_weights = leastSqInstance->
    iteratedModel.subordinate_model().primary_response_fn_weights();

  size_t i,j,k;
  SizetMultiArray var_ids;

  const ShortArray& asv = weighted_response.active_set_request_vector();
  const SizetArray& dvv = weighted_response.active_set_derivative_vector();
  const size_t num_deriv_vars = dvv.size(); 

  if (dvv == unweighted_vars.continuous_variable_ids()) {
    var_ids.resize(boost::extents[unweighted_vars.cv()]);
    var_ids = unweighted_vars.continuous_variable_ids();
  }
  else if (dvv == unweighted_vars.inactive_continuous_variable_ids()) {
    var_ids.resize(boost::extents[unweighted_vars.icv()]);
    var_ids = unweighted_vars.inactive_continuous_variable_ids();
  }
  else { // general derivatives
    var_ids.resize(boost::extents[unweighted_vars.acv()]);
    var_ids = unweighted_vars.all_continuous_variable_ids();
  }

  // TODO: review for efficiency (needless copies); can we just update
  // from the unweighted response, then modify?
  const RealVector& fn_vals = unweighted_response.function_values();
  RealVector wt_fn_vals = weighted_response.function_values_view();
  for (size_t i=0; i<leastSqInstance->numLeastSqTerms; i++) {
    // \Sum_i w_i T^2_i => residual scaling as \Sum_i [sqrt(w_i) T_i]^2
    Real wt_i = std::sqrt(lsq_weights[i]);
    // functions
    if (asv[i] & 1)
      wt_fn_vals[i] = wt_i * fn_vals[i];
    // gradients
    if (asv[i] & 2) {
      const RealMatrix& fn_grads = unweighted_response.function_gradients();
      RealMatrix wt_fn_grads = weighted_response.function_gradients_view();
      for (j=0; j<num_deriv_vars; ++j) {
	size_t xj_index = find_index(var_ids, dvv[j]);
	if (xj_index != _NPOS)
	  wt_fn_grads(xj_index,i) = wt_i * fn_grads(xj_index,i);
      }
    }
    // hessians
    if (asv[i] & 4) {
      const RealSymMatrix& fn_hess = unweighted_response.function_hessian(i);
      RealSymMatrix wt_fn_hess = weighted_response.function_hessian_view(i);
      for (j=0; j<num_deriv_vars; ++j) {
	size_t xj_index = find_index(var_ids, dvv[j]);
	if (xj_index != _NPOS) {
	  for (k=0; k<=j; ++k) {
	    size_t xk_index = find_index(var_ids, dvv[k]);
	    if (xk_index != _NPOS)
	      wt_fn_hess(xj_index,xk_index) = wt_i * fn_hess(xj_index,xk_index);
	  }
	}
      }
    }
  } // loop over least squares terms

  if (leastSqInstance->outputLevel > NORMAL_OUTPUT)
    Cout << "Least squares weight-transformed response:\n" << weighted_response 
	 << std::endl;

}


/** This function should be invoked (or reimplemented) by any derived
    implementations of initialize_run() (which would otherwise hide
    it). */
void LeastSq::initialize_run()
{
  Minimizer::initialize_run();

  // pull any late updates into the RecastModel; may need to update
  // from the underlying user model in case of hybrid methods, so
  // should recurse through any local transformations
  if (myModelLayers > 0)
    iteratedModel.update_from_subordinate_model(myModelLayers-1);

  // Track any previous object instance in case of recursion.  Note that
  // leastSqInstance and minimizerInstance must be tracked separately since
  // the previous least squares method and previous minimizer could be
  // different instances (e.g., for UQ of calibrated solutions with NPSOL
  // for MPP search and NL2SOL for NLS, the previous minimizerInstance is
  // NPSOL and the previous leastSqInstance is NULL).  
  prevLSqInstance = leastSqInstance;
  leastSqInstance = this;

  retrievedIterPriFns = false;
  bestIterPriFns.resize(0);
}


/** Implements portions of post_run specific to LeastSq for scaling
    back to native variables and functions.  This function should be
    invoked (or reimplemented) by any derived implementations of
    post_run() (which would otherwise hide it). */
void LeastSq::post_run(std::ostream& s)
{
  // BMA TODO: the lookups can't possibly retrieve config vars properly...
  // DataTransformModel needs to reimplement db_lookup...

  // Moreover, can't store a single best_resp if config vars

  // BMA TODO: print_results review/cleanup now that we're storing
  // native and transformed residuals...

  // Due to all the complicated use cases for residual recovery, we
  // opt to lookup or re-evaluate the necessary models here, performing fn
  // and grad evals separately, in case they are cached in different
  // evals, and expecting duplicates or surrogate evals in most cases.

  if (bestVariablesArray.empty() || bestResponseArray.empty()) {
    Cerr << "\nError: Empty calibration solution variables or response.\n";
    abort_handler(METHOD_ERROR);
  }
  if (bestVariablesArray.size() > 1) {
    Cout << "\nWarning: " << bestVariablesArray.size() << " calibration "
	 << "best parameter sets returned; expected only one." << std::endl;
  }
  if (bestResponseArray.size() > 1) {
    Cout << "\nWarning: " << bestResponseArray.size() << " calibration "
	 << "best residual sets returned; expected only one." << std::endl;
  }

  bool transform_flag = weightFlag || scaleFlag || calibrationDataFlag;

  // On entry:
  //  * best_vars contains the iterator space (transformed) variables
  //  * bestIterPriFns contains the residuals if available
  //  * best_resp contains the iterator space (transformed) constraints

  // BMA REVIEW: best_vars / best_resp must be used judiciously in
  // config var cases...

  Variables& best_vars = bestVariablesArray.front();
  Response& best_resp = bestResponseArray.front();
  RealVector best_fns = best_resp.function_values_view();

  // After recovery:
  //  * iter_resp contains all native fn vals, constraints for return and 
  //    reporting
  //  * iter_resp contains iterator residuals and gradient
  //    for CI calculation and reporting (doesn't need constraints)

  // iterator space variables and response (deep copy only if needed)
  Variables iter_vars(scaleFlag ? best_vars.copy() : best_vars);
  Response iter_resp(transform_flag ? iteratedModel.current_response().copy() :
		     best_resp);
  RealVector iter_fns = iter_resp.function_values_view();

  // Transform variables back to inbound model, before any potential lookup
  if (scaleFlag) {
    ScalingModel* scale_model_rep = 
      static_cast<ScalingModel*>(scalingModel.model_rep());
    best_vars.continuous_variables
      (scale_model_rep->cv_scaled2native(iter_vars.continuous_variables()));
  }

  // also member retrievedIterPriFns (true for NL2SOL, NLSSOL, false for SNLL)
  bool have_native_pri_fns = false;
  bool have_iter_pri_grads = false;

  // If there are no problem transformations, populate (native) best
  // from iterator-space residuals
  if (!transform_flag && retrievedIterPriFns) {
    copy_data_partial(bestIterPriFns, 0, (int)numLeastSqTerms, best_fns, 0);
    have_native_pri_fns = true;
  }

  // -----
  // Retrieve in native/user space for best_resp and reporting
  // -----

  // Always necessary for SNLLLeastSq recovery; other methods if transforms

  // TODO: In the multiple config case, this is going to retrieve
  // whichever config was last evaluated, together with the optimal
  // calibration parameters.  Could skip here since it's done again in
  // DataTransformModel at print_results time.

  if (!have_native_pri_fns) {

    // Only need to retrieve functions; as constraints are always
    // cached by the solver and unscaled if needed below
    // BMA TODO: Don't really need this whole response object
    // Could just cache the evalId here.. via cacheiter.
    Model orig_model = original_model();
    Response found_resp(orig_model.current_response());
    ActiveSet search_set(found_resp.active_set());
    search_set.request_values(0);
    for (size_t i=0; i<numUserPrimaryFns; ++i)
      search_set.request_value(1, i);
    // The receiving response must have the right ASV when using this
    // lookup signature
    found_resp.active_set(search_set);
    have_native_pri_fns |= orig_model.db_lookup(best_vars, search_set,
						found_resp);

    if (have_native_pri_fns)
      copy_data_partial(found_resp.function_values(), 0, (int)numUserPrimaryFns,
			best_fns, 0);
    else
      // This can occur in model calibration under uncertainty using nested
      // models, or surrogate models so make this non-fatal.
      Cout << "Warning: couldn't recover final least squares terms from "
	   << "evaluation database."
	   << std::endl;
  }

  // -----
  // Retrieve in transformed space for CIs and reporting
  // -----
  if (!retrievedIterPriFns) {

    // This lookup should only be necessary for SNLLLeastSq and will
    // fail if there is a data transformation, so will be caught by
    // re-evaluate below.

    // (Rather than transforming the found native function values, keep
    // it simpler and lookup the iterator space responses too.)

    // Only need to retrieve functions; as constraints are always
    // cached by the solver and unscaled if needed below
    // BMA TODO: Don't really need this whole response object
    // Could just cache the evalId here.. via cacheiter.
    Response found_resp(iteratedModel.current_response());
    ActiveSet search_set(found_resp.active_set());
    search_set.request_values(0);
    for (size_t i=0; i<numLeastSqTerms; ++i)
      search_set.request_value(1, i);
    // The receiving response must have the right ASV when using this
    // lookup signature
    found_resp.active_set(search_set);
    retrievedIterPriFns |= iteratedModel.db_lookup(iter_vars, search_set,
						   found_resp);

    if (retrievedIterPriFns)
      copy_data_partial(found_resp.function_values(), 0, (int)numLeastSqTerms,
			iter_fns, 0);
    else
      Cout << "Warning: couldn't recover final (transformed) least squares "
	   << "terms from\n          evaluation database."
	   << std::endl;
  }

  // Confidence intervals calculations are unconditional and no solver
  // stores the gradients.  Try to lookup first.
  // We want the gradient of the transformed residuals
  // w.r.t. the original variables, so use the iteratedModel
  Response found_resp(iteratedModel.current_response());
  ActiveSet search_set(found_resp.active_set());
  search_set.request_values(0);
  for (size_t i=0; i<numLeastSqTerms; ++i)
    search_set.request_value(2, i);
  found_resp.active_set(search_set);
  have_iter_pri_grads |= iteratedModel.db_lookup(iter_vars, search_set,
						 found_resp);

  if (have_iter_pri_grads) {
    RealMatrix found_gradients(Teuchos::View,
			       found_resp.function_gradients(),
			       (int)numContinuousVars, (int)numLeastSqTerms);
    RealMatrix iter_gradients(Teuchos::View, iter_resp.function_gradients(),
			      (int)numContinuousVars, (int)numLeastSqTerms);
    iter_gradients.assign(found_gradients);
  }
  else if ( outputLevel > NORMAL_OUTPUT)
    Cout << "Info: Couldn't recover residual gradient for confidence interval "
	 << "calculation; will attempt re-evaluation." << std::endl;

  // If needed, evaluate the function values separately from the
  // gradients so more likely to hit a duplicate if they're stored in
  // different evals.  These evals will also aid in recovery of
  // surrogate-based LSQ
  if (!retrievedIterPriFns || !have_native_pri_fns) {

    // BMA TODO: Be finer grained and eval original vs. iterated... if
    // only need one or other:
    // if (!iter_fns)
    //    eval iterated model; save iter_fns and conditionally save native_fns
    // else if (!native_fns)
    //    eval original model; save native_fns
    iteratedModel.continuous_variables(iter_vars.continuous_variables());
    activeSet.request_values(0);
    for (size_t i=0; i<numLeastSqTerms; ++i)
      activeSet.request_value(1, i);
    iteratedModel.evaluate(activeSet);

    if (!retrievedIterPriFns) {
      copy_data_partial(iteratedModel.current_response().function_values(),
			0, (int)numLeastSqTerms, iter_fns, 0);
      retrievedIterPriFns = true;
    }

    if (!have_native_pri_fns) {
      copy_data_partial(original_model().current_response().function_values(),
			0, (int)numUserPrimaryFns, best_fns, 0);
      have_native_pri_fns = true;
    }
  }

  // Can't evaluate gradient with vendor-computed gradients (so no CIs)
  if (!have_iter_pri_grads && !vendorNumericalGradFlag) {
    // For now, we populate iter_resp, then partially undo the scaling in
    // get_confidence_intervals.  Could instead get the eval from
    // original_model and transform it up.
    iteratedModel.continuous_variables(iter_vars.continuous_variables());
    activeSet.request_values(0);
    for (size_t i=0; i<numLeastSqTerms; ++i)
      activeSet.request_value(2, i);
    iteratedModel.evaluate(activeSet);

    RealMatrix eval_gradients(Teuchos::View,
			      iteratedModel.current_response().function_gradients(),
			      (int)numContinuousVars, (int)numLeastSqTerms);
    RealMatrix iter_gradients(Teuchos::View, iter_resp.function_gradients(),
			      (int)numContinuousVars, (int)numLeastSqTerms);
    iter_gradients.assign(eval_gradients);

    have_iter_pri_grads = true;

  }

  // All derived solvers return constraints in best_resp; unscale
  // in-place if needed
  // BMA TODO: constrained LSQ with scaling test
  if (scaleFlag && numNonlinearConstraints > 0) {
    ScalingModel* scale_model_rep =
      static_cast<ScalingModel*>(scalingModel.model_rep());
    RealVector best_fns = best_resp.function_values_view();
    // only requesting scaling of constraints, so no need for variable Jacobian
    activeSet.request_values(1);
    scale_model_rep->
      secondary_resp_scaled2native(best_resp.function_values(),
				   activeSet.request_vector(),
				   best_fns);
  }

  // confidence intervals require
  //  - fully transformed residuals
  //  - native vars for calculating intervals
  //  - Jacobian: transformed resid / native vars
  //    (or fully transformed for un-transforming)
  get_confidence_intervals(best_vars, iter_resp);

  Minimizer::post_run(s);
}


/** Calculate individual confidence intervals for each parameter,
    based on a linear approximation of the nonlinear model. native_cv
    are needed for transformations and final reporting.  iter_resp
    must contain the final differenced, scaled, weighted residuals and gradients. */
void LeastSq::get_confidence_intervals(const Variables& native_vars,
				       const Response& iter_resp)
{
  // TODO: Fix CIs for interpolation and multi-experiment cases.  For
  // simple multi-experiment cases, can just use the model derivatives
  // without replication.  Concern is with singular aggregate Jacobian
  // matrix J.

  // Confidence intervals should be based on weighted/scaled iterator
  // residuals (since that was the nonlinear regression problem
  // formulation), but original user parameters, so must use
  // d(scaled_residual) / d(native_vars).
  if (vendorNumericalGradFlag) {
    Cout << "\nWarning: Confidence Interval calculations are not available"
         << "\n         for vendor numerical gradients.\n\n";
    return;
  }
  if (numLeastSqTerms < numContinuousVars) {
    Cout << "\nWarning: Confidence Interval calculations are not available"
         << "\n         when number of residuals is less than number of"
	 << "\n         variables.\n\n";
    return;
  }

  // recover the iterator space residuals, hopefully via duplicate detection
  const RealVector& resid_star = iter_resp.function_values();

  // first calculate the estimate of sigma-squared.
  Real dof = std::max(double(numLeastSqTerms-numContinuousVars), 1.);
  Real sse_total = 0.;
  for (int i=0; i<numLeastSqTerms; i++)
    sse_total += (resid_star[i]*resid_star[i]);
  Real sigma_sq_hat = sse_total/dof;
 
  // We are using a formulation where the standard error of the
  // parameter vector is calculated as the square root of the diagonal
  // elements of sigma_sq_hat*inverse(J'J), where J is the matrix
  // of derivatives of the model with respect to the parameters,
  // and J' is the transpose of J.  Insteaad of calculating J'J and
  // explicitly taking the inverse, we are using a QR decomposition,
  // where J=QR, and inv(J'J)= inv((QR)'QR)=inv(R'Q'QR)=inv(R'R)=
  // inv(R)*inv(R').
  // J must be in column order for the Fortran call
  Teuchos::LAPACK<int, Real> la;
  int info;
  int M = numLeastSqTerms;
  int N = numContinuousVars;
  int LDA = numLeastSqTerms;
  double *tau, *work;
  double* Jmatrix = new double[numLeastSqTerms*numContinuousVars];
  
  // With scaling, the iter_resp will potentially contain
  // d(scaled_resp) / d(scaled_params).
  //
  // When parameters are scaled, have to apply the variable
  // transformation Jacobian to get to
  // d(scaled_resp) / d(native_params) =
  //   d(scaled_resp) / d(scaled_params) * d(scaled_params) / d(native_params)

  // envelope to hold the either unscaled or iterator response
  Response ultimate_resp = scaleFlag ? iter_resp.copy() : iter_resp; 
  if (scaleFlag) {
    ScalingModel* scale_model_rep =
      static_cast<ScalingModel*>(scalingModel.model_rep());
    bool unscale_resp = false;
    scale_model_rep->response_modify_s2n(native_vars, iter_resp,
					 ultimate_resp, 0, numLeastSqTerms,
					 unscale_resp);
  }
  const RealMatrix& fn_grads = ultimate_resp.function_gradients_view();

  // BMA: TODO we don't need to transpose this matrix...
  for (int i=0; i<numLeastSqTerms; i++)
    for (int j=0; j<numContinuousVars; j++)
      Jmatrix[(j*numLeastSqTerms)+i]=fn_grads(j,i);

  // This is the QR decomposition, the results are returned in J
  work = new double[N + std::min(M,N)];
  tau = work + N;

  la.GEQRF(M,N,Jmatrix,LDA,tau,work,N,&info);
  bool error_flag = info;
  delete[] work;

  // if you add these three lines right after DGEQRF, then the upper triangular
  // part of Jmatrix will then contain Rinverse

  char uplo = 'U'; // upper triangular
  char unitdiag = 'N'; // non-unit trangular
  la.TRTRI(uplo, unitdiag, N, Jmatrix, LDA, &info); 
  error_flag &= info;

  if (error_flag) {
    Cout << "\nWarning: LAPACK error computing confidence intervals.\n\n";
    return;
  }

  RealVector standard_error(numContinuousVars);
  RealVector diag(numContinuousVars, true);
  for (int i=0; i<numContinuousVars; i++) {
    for (int j=i; j<numContinuousVars; j++)
      diag(i) += Jmatrix[j*numLeastSqTerms+i]*Jmatrix[j*numLeastSqTerms+i];
    standard_error[i] = std::sqrt(diag(i)*sigma_sq_hat);
  }
  delete[] Jmatrix;

  confBoundsLower.sizeUninitialized(numContinuousVars);
  confBoundsUpper.sizeUninitialized(numContinuousVars); 

  Pecos::students_t_dist t_dist(dof);
  Real tdist =  bmth::quantile(t_dist,0.975);

  const RealVector& native_cv = native_vars.continuous_variables();
  for (int j=0; j<numContinuousVars; j++) {
    confBoundsLower[j] = native_cv[j] - tdist * standard_error[j];
    confBoundsUpper[j] = native_cv[j] + tdist * standard_error[j];
  }
}

} // namespace Dakota
