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
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif // HAVE_NPSOL

//#define DEBUG


namespace Dakota {

extern PRPCache data_pairs; // global container

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL 2


DataFitSurrBasedLocalMinimizer::
DataFitSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedLocalMinimizer(problem_db, model),
  trConstraintRelax(probDescDB.get_short("method.sbl.constraint_relax")),
  multiLayerBypassFlag(false),
  useDerivsFlag(probDescDB.get_bool("model.surrogate.derivative_usage"))
{
  Model& truth_model  = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();
  // Initialize response results objects (approx/truth and center/star).  These
  // must be deep copies to avoid representation sharing: initialize with copy()
  // and then use update() within the main loop.
  trustRegionData.initialize_responses(approx_model.current_response(),
				       truth_model.current_response(), false);
  trustRegionData.response_center_id(truth_model.evaluation_id(),
				     CORR_TRUTH_RESPONSE);

  // Instantiate the Model and Minimizer for the approximate sub-problem
  initialize_sub_model();
  initialize_sub_minimizer();

  // If (and only if) the user has requested a surrogate bypass, test sub-models
  // to verify that there there is an additional approx layer to bypass.  The
  // surrogate bypass allows for rigorous evaluation of responseCenterTruth
  // and responseStarTruth (which would otherwise involve an approximation).
  if ( probDescDB.get_bool("method.sbl.truth_surrogate_bypass") == true ) {
    Model& truth_model = iteratedModel.truth_model();
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
  hierarchApproxFlag = (approx_type == "hierarchical");
  // derivative orders:
  approxSetRequest = truthSetRequest = 1;
  if (corr_order >= 1 || ( globalApproxFlag && useDerivsFlag ) ||
      localApproxFlag || multiptApproxFlag || meritFnType == LAGRANGIAN_MERIT ||
      approxSubProbObj == LAGRANGIAN_OBJECTIVE ) {
    truthSetRequest |= 2;
    if (truth_model.gradient_type() == "none" ) {
      Cerr << "\nError: a gradient calculation method must be specified for "
	   << "the truth model.\n" << std::endl;
      abort_handler(-1);
    }
  }
  if (corr_order == 2 ||
      ( localApproxFlag && truth_model.hessian_type() != "none" ) ) {
    truthSetRequest |= 4;
    if (truth_model.hessian_type() == "none" ) {
      Cerr << "\nError: a Hessian calculation method must be specified for the "
	   << "truth model.\n" << std::endl;
      abort_handler(-1);
    }
  }
  if (corr_order >= 1 || approxSubProbCon == LINEARIZED_CONSTRAINTS) {
    approxSetRequest |= 2;
    if (approx_model.gradient_type() == "none" ) {
      Cerr << "\nError: a gradient calculation method must be specified for "
	   << "the surrogate model.\n" << std::endl;
      abort_handler(-1);
    }
  }
  if (corr_order == 2) {
    approxSetRequest |= 4;
    if (approx_model.hessian_type() == "none" ) {
      Cerr << "\nError: a Hessian calculation method must be specified for the "
	   << "surrogate model.\n" << std::endl;
      abort_handler(-1);
    }
  }

  // initialize ActiveSets
  trustRegionData.active_set_center(truthSetRequest,   TRUTH_RESPONSE, false);
  trustRegionData.active_set_center(approxSetRequest, APPROX_RESPONSE, false);
  trustRegionData.active_set_star(1,  TRUTH_RESPONSE, false);
  trustRegionData.active_set_star(1, APPROX_RESPONSE, false);

  // initialize Lagrange multipliers
  size_t num_multipliers = numNonlinearEqConstraints;
  for (size_t i=0; i<numNonlinearIneqConstraints; i++) {
    if (origNonlinIneqLowerBnds[i] > -bigRealBoundSize) // g has a lower bound
      num_multipliers++;
    if (origNonlinIneqUpperBnds[i] <  bigRealBoundSize) // g has an upper bound
      num_multipliers++;
  }
  if ( (truthSetRequest & 2) || meritFnType == LAGRANGIAN_MERIT ||
      approxSubProbObj == LAGRANGIAN_OBJECTIVE) {
    lagrangeMult.resize(num_multipliers);
    lagrangeMult = 0.;
  }
  if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
      approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE) {
    augLagrangeMult.resize(num_multipliers);
    augLagrangeMult = 0.;
  }

  // alert user to constraint settings
#ifdef DEBUG
  if (numNonlinearConstraints)
    Cout << "\n<<<<< approxSubProbObj  = " << approxSubProbObj
	 << "\n<<<<< approxSubProbCon  = " << approxSubProbCon
	 << "\n<<<<< meritFnType       = " << meritFnType
	 << "\n<<<<< acceptLogic       = " << acceptLogic
	 << "\n<<<<< trConstraintRelax = " << trConstraintRelax << "\n\n";
#endif
  // sanity checks
  if ( (approxSubProbCon == NO_CONSTRAINTS || !numNonlinearConstraints) &&
       trConstraintRelax != NO_RELAX) {
    Cerr << "\nWarning: constraint relaxation is inactive without approximate "
	 << "subproblem constraints.\n";
    trConstraintRelax = NO_RELAX;
  }
  else if (trConstraintRelax == COMPOSITE_STEP) { // planned implementation
    Cerr << "\nWarning: COMPOSITE STEP constraint relaxation not yet "
	 << "implemented.\n               Using HOMOTOPY method instead.\n";
    trConstraintRelax = HOMOTOPY;
  }
#ifndef HAVE_NPSOL
  if (trConstraintRelax > NO_RELAX) {
    Cerr << "Error: this executable not configured with NPSOL.\n       "
	 << "DataFitSurrBasedLocalMinimizer cannot perform constraint "
	 << "relaxation." << std::endl;
    abort_handler(-1);
  }
#endif

  // Set the minimum trust region size.  For kriging, the minimum trust region
  // must be set to O(10^-2 10^-3).  Otherwise, the correlation matrix becomes
  // ill-conditioned and the maximum likelihood operations crash with floating
  // point errors.
  minTrustRegionFactor = 
    (approx_type == "global_kriging") ? 1.e-3 : minTrustRegionFactor;
  if (trustRegionData.trust_region_factor() < minTrustRegionFactor)
    trustRegionData.trust_region_factor(minTrustRegionFactor);
  //trustRegionOffset.reshape(numContinuousVars);
}


void DataFitSurrBasedLocalMinimizer::pre_run()
{
  SurrBasedLocalMinimizer::pre_run();

  // initialize TR center from initial point
  trustRegionData.vars_center(iteratedModel.current_variables());

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
  if (globalApproxFlag) { // propagate to DFSModel
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

  bestVariablesArray.front().continuous_variables(
    trustRegionData.c_vars_center());
  bestResponseArray.front().function_values(
    trustRegionData.response_center(CORR_TRUTH_RESPONSE).function_values());

  SurrBasedLocalMinimizer::post_run(s);
}


void DataFitSurrBasedLocalMinimizer::reset()
{
  SurrBasedLocalMinimizer::reset();

  trustRegionData.trust_region_factor(origTrustRegionFactor);
  trustRegionData.new_center(true);

  //lagrangeMult    = 0.; // not necessary since redefined each time
  augLagrangeMult   = 0.; // necessary since += used
}


void DataFitSurrBasedLocalMinimizer::build()
{
  if (!globalApproxFlag && !trustRegionData.new_center()) {
    Cout << "\n>>>>> Reusing previous approximation.\n";
    return;
  }

  bool embed_correction = (globalApproxFlag) ?
    build_global() : // global rebuild: new center or new TR bounds
    build_local();   // local/multipt/hierarch: rebuild if new center

  // Update graphics for iteration 0 (initial guess).
  if (sbIterNum == 0)
    parallelLib.output_manager().add_datapoint(trustRegionData.vars_center(),
      iteratedModel.truth_model().interface_id(),
      trustRegionData.response_center(CORR_TRUTH_RESPONSE));

  if (!convergenceFlag)
    compute_center_correction(embed_correction);
}


bool DataFitSurrBasedLocalMinimizer::build_global()
{
  // global with old or new center

  // Retrieve responseCenterTruth if possible, evaluate it if not
  find_center_truth(iteratedModel.subordinate_iterator(),
		    iteratedModel.truth_model());

  Variables& vars_center = trustRegionData.vars_center();
  IntResponsePair& resp_center_truth
    = trustRegionData.response_center_pair(CORR_TRUTH_RESPONSE);

  // Assess hard convergence prior to global surrogate construction
  if (trustRegionData.new_center())
    hard_convergence_check(resp_center_truth.second,
			   vars_center.continuous_variables(),
			   globalLowerBnds, globalUpperBnds);

  bool embed_correction = false;

  // Perform the sampling and the surface fitting
  if (!convergenceFlag)
    // embed_correction is true if surrogate supports anchor constraints
    embed_correction
      = iteratedModel.build_approximation(vars_center, resp_center_truth);
    // TO DO: problem with CCD/BB duplication!

  /*
  if ( !multiLayerBypassFlag && !daceCenterPtFlag )
    // Can augment the global approximation with new center point data
    iteratedModel.update_approximation(vars_center, resp_center_truth, true);
  */

  return embed_correction;
}


bool DataFitSurrBasedLocalMinimizer::build_local()
{
  // local/multipt/hierarchical with new center

  // Evaluate the truth model at the center of the trust region.
  // Local needs values/grads & may need Hessians depending on order of
  // series, multipoint needs values/grads, hierarchical needs values &
  // may need grads/Hessians depending on order of correction.
  iteratedModel.build_approximation();

  // Retrieve responseCenterTruth if possible, evaluate it if not
  find_center_truth(iteratedModel.subordinate_iterator(),
		    iteratedModel.truth_model());

  // Assess hard convergence following build/retrieve
  hard_convergence_check(trustRegionData.response_center(CORR_TRUTH_RESPONSE),
			 trustRegionData.c_vars_center(),
			 globalLowerBnds, globalUpperBnds);

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
  //if (convergenceFlag) return;

  // If hard convergence not achieved in truth values, perform approximate
  // optimization followed by additional (soft) convergence checks.

  // *******************************************************
  // Run iterator on approximation (with correction applied)
  // *******************************************************
  Cout << "\n>>>>> Starting approximate optimization cycle.\n";
  iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);
  if ( trConstraintRelax > NO_RELAX ) // relax constraints if requested
    relax_constraints(trustRegionData);
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  approxSubProbMinimizer.run(pl_iter); // pl_iter required for hierarchical
  Cout << "\n<<<<< Approximate optimization cycle completed.\n";
  sbIterNum++; // full iteration performed: increment the counter

  // *******************************************
  // Retrieve varsStar and responseStarApprox
  // *******************************************
  trustRegionData.vars_star(approxSubProbMinimizer.variables_results());
  if (recastSubProb) { // Can't back out eval from recast data, can't assume
    // last iteratedModel eval was the final solution, and can't use a DB
    // search for data fits.  Therefore, reevaluate (and rely on duplicate
    // detection for multifidelity surrogates).
    Cout << "\n>>>>> Evaluating approximate optimum outside of subproblem "
	 << "recasting.\n";
    iteratedModel.active_variables(trustRegionData.vars_star());
    // leave iteratedModel in AUTO_CORRECTED_SURROGATE mode
    iteratedModel.evaluate(); // fn values only
    trustRegionData.response_star(iteratedModel.current_response(),
				  CORR_APPROX_RESPONSE);
  }
  else // Note: fn values only
    trustRegionData.response_star(approxSubProbMinimizer.response_results(),
				  CORR_APPROX_RESPONSE);
}


void DataFitSurrBasedLocalMinimizer::verify()
{
  //if (convergenceFlag) return;

  // ****************************
  // Evaluate responseStarTruth 
  // ****************************
  Cout << "\n>>>>> Evaluating approximate solution with actual model.\n";
  // since we're bypassing iteratedModel, iteratedModel.serve()
  // must be in the correct server mode.
  iteratedModel.component_parallel_mode(TRUTH_MODEL);
  Model& truth_model  = iteratedModel.truth_model();
  truth_model.active_variables(trustRegionData.vars_star());
  // In all cases (including gradient mode), we only need the truth fn
  // values to validate the predicted optimum.  For gradient mode, we will
  // compute the gradients below if the predicted optimum is accepted.
  if (multiLayerBypassFlag) {
    short mode = truth_model.surrogate_response_mode();
    truth_model.surrogate_response_mode(BYPASS_SURROGATE);
    truth_model.evaluate(); // fn values only
    truth_model.surrogate_response_mode(mode); // restore
  }
  else
    truth_model.evaluate(); // fn values only
  const Response& truth_resp = truth_model.current_response();
  trustRegionData.response_star(truth_resp, CORR_TRUTH_RESPONSE);

  // compute the trust region ratio and update soft convergence counters
  compute_trust_region_ratio(trustRegionData, globalApproxFlag);

  // If the candidate optimum (varsStar) is accepted, then update the
  // center variables and response data.
  if (trustRegionData.new_center()) {
    trustRegionData.c_vars_center(trustRegionData.c_vars_star());
    trustRegionData.response_center_pair(truth_model.evaluation_id(),
					 truth_resp, CORR_TRUTH_RESPONSE);
    // update responseCenterApprox in the hierarchical case only if the
    // old correction can be backed out.  Currently relying on a DB search
    // to recover uncorrected low fidelity fn values.
    //if (hierarchApproxFlag)
    //  trustRegionData.response_center(
    //    trustRegionData.response_star(CORR_APPROX_RESPONSE),
    //    CORR_APPROX_RESPONSE);
  }

  // record the iteration results (irregardless of new center)
  iteratedModel.continuous_variables(trustRegionData.c_vars_center());
  OutputManager& output_mgr = parallelLib.output_manager();
  output_mgr.add_datapoint(trustRegionData.vars_center(),
    truth_model.interface_id(),
    trustRegionData.response_center(CORR_TRUTH_RESPONSE));

  // If the soft convergence criterion is satisfied for a user-specified
  // number of iterations (softConvLimit), then SBLM is deemed converged.
  // Note: this assessment is independent of step acceptance, and "soft
  // convergence" can occur even when a very small improving step is made.
  // This part of the algorithm is critical, since it is more common to
  // utilize soft convergence in real-world engineering applications
  // where accurate gradients are unavailable.
  if (!convergenceFlag) {
    if (softConvCount >= softConvLimit)
      convergenceFlag = 3; // soft convergence
    // terminate SBLM if trustRegionFactor is less than its minimum value
    else if (trustRegionData.trust_region_factor() < minTrustRegionFactor)
      convergenceFlag = 1;
    // terminate SBLM if the maximum number of iterations has been reached
    else if (sbIterNum >= maxIterations)
      convergenceFlag = 2;
  }
}


void DataFitSurrBasedLocalMinimizer::
find_center_truth(const Iterator& dace_iterator, Model& truth_model)
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
  bool found = false, truth_grads = (truthSetRequest & 2);
  if (sbIterNum && !truth_grads)
    // responseCenterTruth updated from responseStarTruth
    // (responseStarTruth has surrogate bypass; build_approx() does not)
    found = true;
  else if (!multiLayerBypassFlag && !globalApproxFlag) {
    // single layer local/multipoint/hierarchical: retrieve from build_approx.
    trustRegionData.response_center_pair(truth_model.evaluation_id(),
					 truth_model.current_response(),
					 CORR_TRUTH_RESPONSE);
    found = true;
  }
  /*
  // This test no longer valid now that find_center_truth() precedes
  // build_approximation()
  else if (!multiLayerBypassFlag && daceCenterPtFlag) {
    // single layer untruncated CCD/BB DOE: retrieve from all_responses
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

  if (!found) {
    Cout << "\n>>>>> Evaluating actual model at trust region center.\n";
    // since we're bypassing iteratedModel, iteratedModel.serve()
    // must be in the correct server mode.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    truth_model.continuous_variables(trustRegionData.c_vars_center());
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
  else if (hierarchApproxFlag && sbIterNum)
    found = find_approx_response(iteratedModel.current_variables(),
      trustRegionData.response_center(CORR_APPROX_RESPONSE));

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


/** The hard convergence check computes the gradient of the merit
    function at the trust region center, performs a projection for
    active bound constraints (removing any gradient component directed
    into an active bound), and signals convergence if the 2-norm of
    this projected gradient is less than convergenceTol. */
void DataFitSurrBasedLocalMinimizer::
hard_convergence_check(const Response& response_truth,
		       const RealVector& c_vars,
		       const RealVector& lower_bnds,
		       const RealVector& upper_bnds)
{
  const RealVector& fns_truth   = response_truth.function_values();
  const RealMatrix& grads_truth = response_truth.function_gradients();

  // -------------------------------------------------
  // Initialize/update Lagrange multipliers and filter
  // -------------------------------------------------
  // These updates are only performed for new iterates from accepted steps
  // (hard_convergence_check() invoked only if trustRegionData.new_center()).

  // initialize augmented Lagrangian multipliers and filter
  if (sbIterNum == 0) {
    if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
	approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE)
      update_augmented_lagrange_multipliers(fns_truth);
    if (acceptLogic == FILTER)
      update_filter(fns_truth);
  }

  // hard convergence assessment requires gradients
  if ( !(truthSetRequest & 2) )
    return;

  // update standard Lagrangian multipliers
  if (numNonlinearConstraints) {
    Real constraint_viol = constraint_violation(fns_truth, constraintTol);
    // solve non-negative/bound-constrained LLS for lagrangeMult
    if (meritFnType      == LAGRANGIAN_MERIT     ||
	approxSubProbObj == LAGRANGIAN_OBJECTIVE || constraint_viol < DBL_MIN)
      update_lagrange_multipliers(fns_truth, grads_truth);
    // avoid merit fn gradient calculations if constraints are violated
    if (constraint_viol > 0.)
      return;
  }

  // -----------------------------------
  // Compute the merit function gradient
  // -----------------------------------

  // This will not generally be effective in the augmented Lagrangian or penalty
  // cases unless the merit fn is directly minimized in the approx subproblem
  // cycles (since the penalty and augmented Lagrange multiplier updates will
  // not be in synch with the minimizer results for f,g,h treated separately).
  // However, the basic Lagrangian case can be effective, since the multiplier
  // updates are computed directly from the objective/constraint gradients at
  // the current iterate.
  RealVector merit_fn_grad(numContinuousVars, true);
  const BoolDeque& sense = iteratedModel.primary_response_fn_sense();
  const RealVector&  wts = iteratedModel.primary_response_fn_weights();
  //if (meritFnType == LAGRANGIAN_MERIT)
  lagrangian_gradient(fns_truth, grads_truth, sense, wts,
    origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets,
    merit_fn_grad);
  //else if (meritFnType == AUGMENTED_LAGRANGIAN_MERIT)
  //  augmented_lagrangian_gradient(fns_truth, grads_truth, wts,
  //    origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets,
  //    merit_fn_grad);
  //else
  //  penalty_gradient(fns_truth, grads_truth, wts, origNonlinIneqLowerBnds,
  //    origNonlinIneqUpperBnds, origNonlinEqTargets, merit_fn_grad);

  // Compute norm of projected merit function gradient
  Real merit_fn_grad_norm = 0.0;
  for (size_t i=0; i<numContinuousVars; i++) {
    Real c_var = c_vars[i], l_bnd = lower_bnds[i], u_bnd = upper_bnds[i];
    // Determine if the calculated gradient component dp/dx_i is directed into
    // an active bound constraint.  If not, include it in the gradient norm.
    bool active_lower_bnd = ( (l_bnd == 0.0 && std::fabs(c_var) < 1.e-10) ||
      (l_bnd != 0.0 && std::fabs(1.0 - c_var/l_bnd) < 1.e-10) );
    bool active_upper_bnd = ( (u_bnd == 0.0 && std::fabs(c_var) < 1.e-10) ||
      (u_bnd != 0.0 && std::fabs(1.0 - c_var/u_bnd) < 1.e-10) );
    if ( !( (active_lower_bnd && merit_fn_grad[i] > 0.0) ||
	    (active_upper_bnd && merit_fn_grad[i] < 0.0) ) )
      merit_fn_grad_norm += std::pow(merit_fn_grad[i], 2);
  }

  // Terminate SBLM if the norm of the projected merit function gradient
  // at x_c is less than convTol (hard convergence).
  merit_fn_grad_norm = std::sqrt( merit_fn_grad_norm );
  if ( merit_fn_grad_norm < convergenceTol ) 
    convergenceFlag = 4; // hard convergence

#ifdef DEBUG
  Cout << "In hard convergence check: merit_fn_grad_norm =  "
       << merit_fn_grad_norm << '\n';
#endif
}

} // namespace Dakota
