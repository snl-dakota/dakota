/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataFitSurrBasedLocalMinimizer
//- Description: Implementation code for DataFitSurrBasedLocalMinimizer class
//- Owner:       Mike Eldred, Sandia National Laboratories
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DataFitSurrBasedLocalMinimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaGraphics.hpp"
#include "RecastModel.hpp"
#include "DiscrepancyCorrection.hpp"

//#define DEBUG


namespace Dakota {

extern PRPCache data_pairs; // global container


DataFitSurrBasedLocalMinimizer::
DataFitSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedLocalMinimizer(problem_db, model),
  multiLayerBypassFlag(false),
  useDerivsFlag(probDescDB.get_bool("model.surrogate.derivative_usage"))
{
  // check iteratedModel for model form hierarchy and/or discretization levels
  if (iteratedModel.surrogate_type() == "hierarchical") {
    Cerr << "Error: DataFitSurrBasedLocalMinimizer requires a local, multipoint"
	 << ", or global surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  Model& truth_model  = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();

  // If (and only if) the user has requested a surrogate bypass, test sub-models
  // to verify that there there is an additional approx layer to bypass.  The
  // surrogate bypass allows for rigorous evaluation of responseCenterTruth
  // and responseStarTruth (which would otherwise involve an approximation).
  if ( probDescDB.get_bool("method.sbl.truth_surrogate_bypass") == true ) {
    if (truth_model.model_type() == "surrogate")
      multiLayerBypassFlag = true;
    ModelList& ml = truth_model.subordinate_models();
    for (ModelLIter ml_iter = ml.begin(); ml_iter != ml.end(); ml_iter++)
      if (ml_iter->model_type() == "surrogate")
	multiLayerBypassFlag = true;
  }

  // Initialize method/interface dependent settings
  const String& approx_type = probDescDB.get_string("model.surrogate.type");
  short corr_order = (correctionType) ?
    probDescDB.get_short("model.surrogate.correction_order") : -1;
  // approximation types:
  globalApproxFlag   = (strbegins(approx_type, "global_"));
  multiptApproxFlag  = (strbegins(approx_type, "multipoint_"));
  localApproxFlag    = (strbegins(approx_type, "local_"));
  // derivative orders:
  approxSetRequest = truthSetRequest = 1;
  if (corr_order >= 1 || ( globalApproxFlag && useDerivsFlag ) ||
      localApproxFlag || multiptApproxFlag || meritFnType == LAGRANGIAN_MERIT ||
      approxSubProbObj == LAGRANGIAN_OBJECTIVE ) {
    truthSetRequest |= 2;
    if (truth_model.gradient_type() == "none" ) {
      Cerr << "\nError: a gradient calculation method must be specified for "
	   << "the truth model.\n" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  if (corr_order == 2 ||
      ( localApproxFlag && truth_model.hessian_type() != "none" ) ) {
    truthSetRequest |= 4;
    if (truth_model.hessian_type() == "none" ) {
      Cerr << "\nError: a Hessian calculation method must be specified for the "
	   << "truth model.\n" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  if (corr_order >= 1 || approxSubProbCon == LINEARIZED_CONSTRAINTS) {
    approxSetRequest |= 2;
    if (approx_model.gradient_type() == "none" ) {
      Cerr << "\nError: a gradient calculation method must be specified for "
	   << "the surrogate model.\n" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  if (corr_order == 2) {
    approxSetRequest |= 4;
    if (approx_model.hessian_type() == "none" ) {
      Cerr << "\nError: a Hessian calculation method must be specified for the "
	   << "surrogate model.\n" << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  // Instantiate the Model and Minimizer for the approximate sub-problem
  initialize_sub_model();
  initialize_sub_minimizer();
  // Initialize any Lagrange multiplier arrays
  initialize_multipliers();

  // size the trust region bounds to allow individual updates
  trustRegionData.initialize_bounds(numContinuousVars);
  // Initialize variable/response objects (approx/truth and center/star)
  trustRegionData.initialize_data(iteratedModel.current_variables(),
				  approx_model.current_response(),
				  truth_model.current_response(), false);
  // responseCenterTruth is an IntResponsePair: init the eval_id
  trustRegionData.response_center_id(truth_model.evaluation_id(),
				     CORR_TRUTH_RESPONSE);
  // initialize ActiveSets
  trustRegionData.active_set_center(truthSetRequest,   TRUTH_RESPONSE, false);
  trustRegionData.active_set_center(approxSetRequest, APPROX_RESPONSE, false);
  trustRegionData.active_set_star(1,  TRUTH_RESPONSE, false);
  trustRegionData.active_set_star(1, APPROX_RESPONSE, false);

  minTrustRegionFactor = 
    (approx_type == "global_kriging") ? 1.e-3 : minTrustRegionFactor;

  // Set the minimum trust region size.  For kriging, the minimum trust region
  // must be set to O(10^-2 10^-3).  Otherwise, the correlation matrix becomes
  // ill-conditioned and the maximum likelihood operations crash with floating
  // point errors.
  size_t num_factors = origTrustRegionFactor.length();
  Real tr_factor     = (num_factors) ? origTrustRegionFactor[0] : 0.5;
  if (num_factors != 1) origTrustRegionFactor.sizeUninitialized(1);
  origTrustRegionFactor[0]
    = (tr_factor < minTrustRegionFactor) ? minTrustRegionFactor : tr_factor;
}


void DataFitSurrBasedLocalMinimizer::pre_run()
{
  SurrBasedLocalMinimizer::pre_run();

  // reset softConvCount, convergence-related status bits, filter
  trustRegionData.reset();
  // initialize TR center from current Model state (sets newCenterFlag)
  trustRegionData.vars_center(iteratedModel.current_variables());
  // initialize TR factor
  trustRegionData.trust_region_factor(origTrustRegionFactor[0]);

  // Extract subIterator/subModel(s) from the SurrogateModel
  Iterator& dace_iterator = iteratedModel.subordinate_iterator();

  // Update DACE settings for global approximations.  Check that dace_iterator
  // is defined (a dace_iterator specification is not required when the data
  // samples are read in from a file rather than obtained from sampling).
  //daceCenterEvalFlag = false;
  if (globalApproxFlag && !dace_iterator.is_null()) {
    // With correction approaches, the responses specification must provide
    // support for evaluating derivative info.  However, this data is usually
    // not needed for every DACE evaluation in the global surrogate build
    // (exception: "use_derivatives" spec for surrogate builds using derivative
    // data).  Therefore, SBLM overrides the normal DACE behavior of evaluating
    // the full data set at every point in the experimental design.
    short asv_val = (useDerivsFlag) ? 3 : 1;
    ActiveSet dace_set
      = iteratedModel.truth_model().current_response().active_set(); // copy
    dace_set.request_values(asv_val);
    dace_iterator.active_set(dace_set);

    // Extract info on the sampling method type
    //unsigned short sampling_type = dace_iterator.sampling_scheme();
    //if (sampling_type == SUBMETHOD_BOX_BEHNKEN ||
    //    sampling_type == SUBMETHOD_CENTRAL_COMPOSITE)
    //  daceCenterEvalFlag = true;
  }
}


void DataFitSurrBasedLocalMinimizer::post_run(std::ostream& s)
{
  // restore original/global bounds
  //approxSubProbModel.continuous_variables(initialPoint);
  //if (recastSubProb) iteratedModel.continuous_variables(initialPoint);
  approxSubProbModel.continuous_lower_bounds(globalLowerBnds);
  approxSubProbModel.continuous_upper_bounds(globalUpperBnds);
  if (recastSubProb) { // propagate to DFSModel
    iteratedModel.continuous_lower_bounds(globalLowerBnds);
    iteratedModel.continuous_upper_bounds(globalUpperBnds);
  }
  if (trConstraintRelax > NO_RELAX) {
    approxSubProbModel.nonlinear_ineq_constraint_lower_bounds(
      origNonlinIneqLowerBnds);
    approxSubProbModel.nonlinear_ineq_constraint_upper_bounds(
      origNonlinIneqUpperBnds);
    approxSubProbModel.nonlinear_eq_constraint_targets(origNonlinEqTargets);
  }

  bestVariablesArray.front().active_variables(trustRegionData.vars_center());
  bestResponseArray.front().function_values(
    trustRegionData.response_center(CORR_TRUTH_RESPONSE).function_values());

  SurrBasedLocalMinimizer::post_run(s);
}


void DataFitSurrBasedLocalMinimizer::build()
{
  if (!globalApproxFlag && !trustRegionData.status(NEW_CENTER)) {
    Cout << "\n>>>>> Reusing previous approximation.\n";
    return;
  }

  bool embed_correction = (globalApproxFlag) ?
    build_global() : // global rebuild: new center or new TR bounds
    build_local();   // local/multipt/hierarch: rebuild if new center

  // Update graphics for iteration 0 (initial guess).
  if (globalIterCount == 0)
    parallelLib.output_manager().add_datapoint(trustRegionData.vars_center(),
      iteratedModel.truth_model().interface_id(),
      trustRegionData.response_center(CORR_TRUTH_RESPONSE));

  if (!trustRegionData.converged())
    compute_center_correction(embed_correction);
}


bool DataFitSurrBasedLocalMinimizer::build_global()
{
  // global with old or new center

  // Retrieve responseCenterTruth if possible, evaluate it if not
  find_center_truth();

  // Assess hard convergence prior to global surrogate construction
  if (trustRegionData.status(NEW_CENTER))
    hard_convergence_check(trustRegionData, globalLowerBnds, globalUpperBnds);

  bool embed_correction = false;

  // Perform the sampling and the surface fitting
  if (!trustRegionData.converged()) {

    // propagate build bounds to DFSModel
    iteratedModel.continuous_lower_bounds(trustRegionData.tr_lower_bounds());
    iteratedModel.continuous_upper_bounds(trustRegionData.tr_upper_bounds());

    // embed_correction is true if surrogate supports anchor constraints
    embed_correction = iteratedModel.build_approximation(
      trustRegionData.vars_center(),
      trustRegionData.response_center_pair(CORR_TRUTH_RESPONSE));
    // TO DO: problem with CCD/BB duplication!

    /*
    if ( !multiLayerBypassFlag && !daceCenterPtFlag )
      // Can augment the global approximation with new center point data
      iteratedModel.update_approximation(vars_center, resp_center_truth, true);
    */
  }

  return embed_correction;
}


bool DataFitSurrBasedLocalMinimizer::build_local()
{
  // local/multipt/hierarchical with new center

  // propagate build bounds to DFSModel (e.g., for finite difference bounds)
  iteratedModel.continuous_lower_bounds(trustRegionData.tr_lower_bounds());
  iteratedModel.continuous_upper_bounds(trustRegionData.tr_upper_bounds());

  // Evaluate the truth model at the center of the trust region.
  // Local needs values/grads & may need Hessians depending on order of
  // series, multipoint needs values/grads, hierarchical needs values &
  // may need grads/Hessians depending on order of correction.
  iteratedModel.build_approximation();

  // Retrieve responseCenterTruth if possible, evaluate it if not
  find_center_truth();

  // Assess hard convergence following build/retrieve
  hard_convergence_check(trustRegionData, globalLowerBnds, globalUpperBnds);

  // embedded correction:
  return ( localApproxFlag || (multiptApproxFlag && !(approxSetRequest & 4)) );
}


void DataFitSurrBasedLocalMinimizer::
compute_center_correction(bool embed_correction)
{
  // **************************************
  // Evaluate/retrieve responseCenterApprox
  // **************************************
  find_center_approx();

  // ******************************************
  // Compute additive/multiplicative correction
  // ******************************************
  if (correctionType && !embed_correction) {
    // -->> local and up to 1st-order multipt do not need correction
    // -->> hierarchical needs compute_correction if new center
    // -->> global needs compute_correction if new center or new bounds
    DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();

    /* DFSBLM::trustRegionData does not store UNCORR_APPROX_RESPONSE!
    Response resp_center_approx(
      trustRegionData.response_center(UNCORR_APPROX_RESPONSE).copy());
    delta.compute(trustRegionData.vars_center(),
		  trustRegionData.response_center(CORR_TRUTH_RESPONSE),
		  resp_center_approx);
    delta.apply(trustRegionData.vars_center(), resp_center_approx);
    trustRegionData.response_center(resp_center_approx, CORR_APPROX_RESPONSE);
    */

    // correct response in place using shallow copy
    Response resp_center_approx(
      trustRegionData.response_center(CORR_APPROX_RESPONSE));
    delta.compute(trustRegionData.vars_center(),
		  trustRegionData.response_center(CORR_TRUTH_RESPONSE),
		  resp_center_approx);
    delta.apply(trustRegionData.vars_center(), resp_center_approx);
  }
}


void DataFitSurrBasedLocalMinimizer::minimize()
{
  // If hard convergence not achieved in truth values, perform approximate
  // optimization followed by additional (soft) convergence checks.

  update_approx_sub_problem(trustRegionData);

  // *******************************************************
  // Run iterator on approximation (with correction applied)
  // *******************************************************
  SurrBasedLocalMinimizer::minimize();

  // ****************************************
  // Retrieve varsStar and responseStarApprox
  // ****************************************
  trustRegionData.vars_star(approxSubProbMinimizer.variables_results());
  if (recastSubProb) { // Can't back out eval from recast data, can't assume
    // last iteratedModel eval was the final solution, and can't use a DB
    // search for data fits.  Therefore, reevaluate corrected approximation.
    Cout << "\n>>>>> Evaluating approximate optimum outside of subproblem "
	 << "recasting.\n";
    iteratedModel.active_variables(trustRegionData.vars_star());
    // leave iteratedModel in AUTO_CORRECTED_SURROGATE mode
    iteratedModel.evaluate(trustRegionData.active_set_star(APPROX_RESPONSE));
    trustRegionData.response_star(iteratedModel.current_response(),
				  CORR_APPROX_RESPONSE);
  }
  else // Note: fn values only
    trustRegionData.response_star(approxSubProbMinimizer.response_results(),
				  CORR_APPROX_RESPONSE);
}


void DataFitSurrBasedLocalMinimizer::verify()
{
  // ****************************
  // Evaluate responseStarTruth 
  // ****************************
  Cout << "\n>>>>> Evaluating approximate solution with actual model.\n";
  // since we're bypassing iteratedModel, iteratedModel.serve()
  // must be in the correct server mode.
  iteratedModel.component_parallel_mode(TRUTH_MODEL);
  Model& truth_model = iteratedModel.truth_model();
  truth_model.active_variables(trustRegionData.vars_star());
  // In all cases (including gradient mode), we only need the truth fn
  // values to validate the predicted optimum.  For gradient mode, we will
  // compute the gradients below if the predicted optimum is accepted.
  if (multiLayerBypassFlag) {
    short mode = truth_model.surrogate_response_mode();
    truth_model.surrogate_response_mode(BYPASS_SURROGATE);
    truth_model.evaluate(trustRegionData.active_set_star(TRUTH_RESPONSE));
    truth_model.surrogate_response_mode(mode); // restore
  }
  else
    truth_model.evaluate(trustRegionData.active_set_star(TRUTH_RESPONSE));
  const Response& truth_resp = truth_model.current_response();
  trustRegionData.response_star_pair(truth_model.evaluation_id(), truth_resp,
				     CORR_TRUTH_RESPONSE);

  // compute the trust region ratio, update soft convergence counters, and
  // transfer data from star to center (if accepted step)
  compute_trust_region_ratio(trustRegionData, globalApproxFlag);

  // record the iteration results, even if no change in center iterate
  iteratedModel.active_variables(trustRegionData.vars_center());
  OutputManager& output_mgr = parallelLib.output_manager();
  output_mgr.add_datapoint(trustRegionData.vars_center(),
    truth_model.interface_id(),
    trustRegionData.response_center(CORR_TRUTH_RESPONSE));

  // test if max SBLM iterations exceeded
  if (globalIterCount >= maxIterations)
    trustRegionData.set_status_bits(MAX_ITER_CONVERGED);
  // test if trustRegionFactor is less than its minimum value
  if (trustRegionData.trust_region_factor() < minTrustRegionFactor)
    trustRegionData.set_status_bits(MIN_TR_CONVERGED);
  // If the soft convergence criterion is satisfied for a user-specified
  // number of iterations (softConvLimit), then SBLM is deemed converged.
  // Note: this assessment is independent of step acceptance, and "soft
  // convergence" can occur even when a very small improving step is made.
  if (trustRegionData.soft_convergence_count() >= softConvLimit)
    trustRegionData.set_status_bits(SOFT_CONVERGED);
}


void DataFitSurrBasedLocalMinimizer::find_center_truth()
{
  // Single layer:
  // -->> local/multipt/hierarchical: retrieve center truth from build_approx.
  // -->> global CCD/BB: if no TR truncation, retrieve center truth from
  //      all_responses.  If truncated, perform new eval and add to fit.
  // -->> global with other DACE: perform new eval and add to fit.
  // Multiple layer: eval center truth with surrogate bypass.
  // -->> local/multipt/hierarchical: don't replace truth response w/i surrogate
  // -->> global CCD/BB: don't replace data in fit (apples/oranges).
  // -->> global with other DACE: don't add to fit (apples/oranges).

  bool found = false;
  if (globalApproxFlag) { // this fn precedes build_approximation()

    // resp fn vals for center truth updated from star truth in verify() -->
    // compute_trust_region_ratio()
    if (trustRegionData.status(NEW_CENTER))
      found = ( globalIterCount  > 0 &&
	        truthSetRequest == 1 ); // star->center sufficient
    else
      found = ( globalIterCount  > 0 );  // reuse previous center data

    /*
    // This test no longer valid since this fn precedes build_approximation():
    if (!multiLayerBypassFlag && daceCenterPtFlag) {
      // single layer untruncated CCD/BB DOE: retrieve from all_responses
      const Iterator& dace_iterator = iteratedModel.subordinate_iterator();
      const RealVectorArray& all_vars      = dace_iterator.all_variables();
      const IntResponseMap&  all_responses = dace_iterator.all_responses();
      size_t i, j, num_samples = all_vars.length();
      IntRespMCIter r_it = all_responses.begin();
      for (i=0; i<num_samples; ++i, ++r_it) { // center should be first one
        if (all_vars[i].continuous_variables() ==
            trustRegionData.c_vars_center()) {
	  const ShortArray& asv = r_it->active_set_request_vector();
	  bool incomplete = false;
	  for (j=0; j<numFunctions; j++)
	    if ( !(asv[j] & 1) || ( truth_grads           && !(asv[j] & 2) )
	                       || ( (truthSetRequest & 4) && !(asv[j] & 4) ) )
	      incomplete = true;
	  if (!incomplete) {
	    trustRegionData.response_center_pair(*r_it, CORR_TRUTH_RESPONSE);
	    found = true;
	  }
	  break; // out of for loop
        }
      }
    }
    */
  }
  else { // local/multipoint: this fn follows build_approximation()
    if (!multiLayerBypassFlag) { // single layer: retrieve from build_approx.
      Model& truth_model = iteratedModel.truth_model();
      trustRegionData.response_center_pair(truth_model.evaluation_id(),
	truth_model.current_response(), CORR_TRUTH_RESPONSE);
      found = true;
    }
  }

  if (!found) {
    Cout << "\n>>>>> Evaluating actual model at trust region center.\n";
    // since we're bypassing iteratedModel, iteratedModel.serve()
    // must be in the correct server mode.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    Model& truth_model = iteratedModel.truth_model();
    truth_model.active_variables(trustRegionData.vars_center());
    if (multiLayerBypassFlag) {
      short mode = truth_model.surrogate_response_mode();
      truth_model.surrogate_response_mode(BYPASS_SURROGATE);
      truth_model.evaluate(trustRegionData.active_set_center(TRUTH_RESPONSE));
      truth_model.surrogate_response_mode(mode); // restore
    }
    else
      truth_model.evaluate(trustRegionData.active_set_center(TRUTH_RESPONSE));
    trustRegionData.response_center_pair(truth_model.evaluation_id(),
					 truth_model.current_response(),
					 CORR_TRUTH_RESPONSE);
  }
}


void DataFitSurrBasedLocalMinimizer::find_center_approx()
{
  // -->> local/multipoint needs new responseCenterApprox if a new center.
  //      Just copy responseCenterTruth for the local case or for up to
  //      1st-order multipoint case.
  // -->> hierarchical needs new responseCenterApprox if a new center
  //      since responseStarApprox has an old correction applied and,
  //      for gradient mode, lacks gradients.  In non-gradient mode, the
  //      low fidelity uncorrected function values will be returned by
  //      the duplicate detection.
  //  *** Alternate: provide a reverse apply_correction routine and reuse
  //      low fidelity fn values from responseStarApprox. 
  //      In gradient mode, the low fidelity gradients require a new
  //      evaluation (and the low fidelity function values are currently
  //      re-evaluated in this case).
  //  *** May both be duplicate if a gradient-based opt was used on LFM.
  //      Query for the values & gradients separately (DOT/CONMIN) or
  //      revise the data_pairs DB search.
  // -->> global needs new responseCenterApprox every time since
  //      responseStarApprox is wrong for new center or new bounds.
  //      Arguments for hierarchical (old correction, no grads) also apply
  bool found = false;
  if (localApproxFlag) {
    trustRegionData.response_center(
      trustRegionData.response_center(CORR_TRUTH_RESPONSE),
      CORR_APPROX_RESPONSE);
    found = true;
  }
  else if (multiptApproxFlag && !(approxSetRequest & 4)) {
    // Note: current multipoint approximation (TANA) exactly reproduces value
    // and gradient at current expansion point and value at previous expansion
    // point.  It will also normally reproduce the gradient at the previous
    // expansion point, unless numerical safeguarding of p exponents is used.
    trustRegionData.response_center(
      trustRegionData.response_center(CORR_TRUTH_RESPONSE),
      CORR_APPROX_RESPONSE);
    found = true;
  }
  //else if (hierarchApproxFlag && globalIterCount)
  //  found = find_approx_response(iteratedModel.current_variables(),
  //    trustRegionData.response_center(CORR_APPROX_RESPONSE));

  if (found)
    Cout << "\n>>>>> Previous approximate response retrieved at trust "
	 << "region center.\n"; // << responseCenterApprox;
  else { // responseCenterApprox not available
    Cout <<"\n>>>>> Evaluating approximation at trust region center.\n";
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    iteratedModel.evaluate(trustRegionData.active_set_center(APPROX_RESPONSE));
    trustRegionData.response_center(iteratedModel.current_response(),
				    CORR_APPROX_RESPONSE);
  }
}

} // namespace Dakota
