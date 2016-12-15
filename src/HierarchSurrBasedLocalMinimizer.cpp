/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrBasedLocalMinimizer
//- Description: Implementation code for HierarchSurrBasedLocalMinimizer class
//- Owner:       Mike Eldred, Sandia National Laboratories
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "HierarchSurrBasedLocalMinimizer.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaGraphics.hpp"
#include "DiscrepancyCorrection.hpp"

namespace Dakota {

// initialization of statics
//HierarchSurrBasedLocalMinimizer*
//HierarchSurrBasedLocalMinimizer::mlmfInstance(NULL);


HierarchSurrBasedLocalMinimizer::
HierarchSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedLocalMinimizer(problem_db, model), minimizeIndex(0),
  nestedTrustRegions(true), multiLev(false)
{
  // check iteratedModel for model form hierarchy and/or discretization levels
  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: HierarchSurrBasedLocalMinimizer requires a hierarchical "
         << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Get number of model fidelities and number of levels for each fidelity:
  ModelList& models = iteratedModel.subordinate_models(false);
  numFid = models.size(); numLev.resize(numFid);
  ModelLIter ml_iter; size_t i;
  for (ml_iter=models.begin(), i=0; i<numFid; ++ml_iter, ++i) {
    numLev[i] = ml_iter->solution_levels();
    if (numLev[i] > 1) multiLev = true;
  }

  // TODO: Only 1D for multifidelity -- need to support ML & MLMF
  size_t num_tr = numFid - 1; // no TR for truth model (valid for global bnds)
  trustRegions.resize(num_tr);
  for (ml_iter=models.begin(), i=0; i<numFid-1; ++i) {
    // size the trust region bounds to allow individual updates
    trustRegions[i].initialize_bounds(numContinuousVars);
    // assign approx and truth for this level
    trustRegions[i].initialize_responses(ml_iter->current_response(),
					 (++ml_iter)->current_response());
    // assign the approx / truth model forms
    trustRegions[i].initialize_indices(i, i+1);
  }

  // Simpler case than DFSBLM:
  short corr_order = (correctionType) ?
    probDescDB.get_short("model.surrogate.correction_order") : -1;
  approxSetRequest = truthSetRequest = 1;
  if (corr_order >= 1 )// || meritFnType      == LAGRANGIAN_MERIT
                       // || approxSubProbObj == LAGRANGIAN_OBJECTIVE )
    truthSetRequest |= 2;
  if (corr_order >= 1 )// || approxSubProbCon == LINEARIZED_CONSTRAINTS )
    approxSetRequest |= 2;
  if (corr_order == 2)
    { truthSetRequest |= 4; approxSetRequest |= 4; }

  // if needed, reshape origTrustRegionFactor and assign defaults
  // Note: user should specify ordered profile consistent with ordered models
  // from LF to HF-1, e.g. initial_size = .125 .25 .5
  size_t num_factors = origTrustRegionFactor.length();
  if (num_factors != num_tr) {
    if (num_factors <= 1) {
      // apply tr_0 factor recursively, from largest at HF-1 to smallest at LF:
      Real tr_0 = (num_factors) ? origTrustRegionFactor[0] : 0.5;
      Real tr_factor = tr_0;
      origTrustRegionFactor.sizeUninitialized(num_tr);
      for (int i=num_tr-1; i>=0; --i) {
	origTrustRegionFactor[i] = tr_factor;
	tr_factor *= tr_0;
      }
    }
    else {
      Cerr << "Error: wrong length for trust region initial_size ("
	   << num_factors << " specified, " << num_tr << " expected)"
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  // Instantiate the Model and Minimizer for the approximate sub-problem
  initialize_sub_model();
  initialize_sub_minimizer();
  // Initialize any Lagrange multiplier arrays
  initialize_multipliers();
}


HierarchSurrBasedLocalMinimizer::~HierarchSurrBasedLocalMinimizer()
{ }


void HierarchSurrBasedLocalMinimizer::pre_run()
{
  // TODO; then this step if active sets will be passed below...
  SurrBasedLocalMinimizer::pre_run();

  // static pointer to HierarchSurrBasedLocalMinimizer instance
  //mlmfInstance = this;

  // initialize the trust region factors top-down with HF at origTRFactor
  // and lower fidelities nested and reduced by 2x each level.
  size_t i, num_tr = numFid - 1;
  for (i=0; i<num_tr; ++i) {
    SurrBasedLevelData& tr_data = trustRegions[i];

    tr_data.vars_center(iteratedModel.current_variables());// sets NEW_CENTER

    // set TR factor (TR bounds defined during update_trust_region())
    tr_data.trust_region_factor(origTrustRegionFactor[i]); // sets NEW_TR_FACTOR

    tr_data.reset_soft_convergence_count();

    tr_data.active_set_star(1, APPROX_RESPONSE);
    tr_data.active_set_star(1,  TRUTH_RESPONSE);
    tr_data.active_set_center(approxSetRequest, APPROX_RESPONSE);
    tr_data.active_set_center(truthSetRequest,   TRUTH_RESPONSE);
  }
}


void HierarchSurrBasedLocalMinimizer::post_run(std::ostream& s)
{
  // restore original/global bounds
  //approxSubProbModel.continuous_variables(initialPoint);
  //if (recastSubProb) iteratedModel.continuous_variables(initialPoint);
  approxSubProbModel.continuous_lower_bounds(globalLowerBnds);
  approxSubProbModel.continuous_upper_bounds(globalUpperBnds);

  bestVariablesArray.front().active_variables(
    trustRegions[minimizeIndex].vars_center());
  bestResponseArray.front().function_values(
    trustRegions[minimizeIndex].response_center(CORR_TRUTH_RESPONSE).
    function_values());

  SurrBasedLocalMinimizer::post_run(s);
}


/** Step 1 in SurrBasedLocalMinimizer::core_run(). */
void HierarchSurrBasedLocalMinimizer::update_trust_region()
{
  // recur top down to enforce strict bound inter-relationships:
  // > Nested case: all levels are strict subsets of previous
  // > Non-nested case: only the bottom (LF) level is nested; other levels
  //   constrained by global bounds; intermediate levels can evolve
  //   independently based on the accuracy of their individual discrepancies.

  int index, j, k, num_tr_m1 = trustRegions.size() - 1;
  bool new_tr_factor = trustRegions[num_tr_m1].status(NEW_TR_FACTOR),
       parent_update = new_tr_factor;
  // Highest fidelity valid over global bnds: parent bnds for top TR = global
  if (new_tr_factor)
    update_trust_region_data(trustRegions[num_tr_m1], globalLowerBnds,
			     globalUpperBnds);
  // Loop over all subordinate levels
  for (index=num_tr_m1-1; index>=minimizeIndex; --index) {

    new_tr_factor = trustRegions[index].status(NEW_TR_FACTOR);
    if (new_tr_factor)// nested levels at / below this level must update TR bnds
      parent_update = true;

    // if nested at all levels, only need to constraint from one level above:
    if (nestedTrustRegions) {
      if (parent_update) // update if any TR factors at/above level have changed
	update_trust_region_data(trustRegions[index],
				 trustRegions[index+1].tr_lower_bounds(),
				 trustRegions[index+1].tr_upper_bounds());
    }
    // if !nested and !minimizeIndex, then no parent constraints, only global
    else if (index != minimizeIndex) {
      if (new_tr_factor) // update only if this level's TR factor has changed
	update_trust_region_data(trustRegions[index], globalLowerBnds,
				 globalUpperBnds);
    }
    // in this case (!nested and index == minimizeIndex), there was no recursive
    // enforcement for previous levels, but the minimizeIndex level must recur
    // because it is the level where optimization over LF + \Sum_j delta_j,j+1
    // is performed --> enforce all trust regions for all individual deltas.
    else if (parent_update) {
      RealVector parent_upper_bnds(numContinuousVars, false),
	         parent_lower_bnds(numContinuousVars, false);
      for (j=0; j<numContinuousVars; ++j) {
        Real min_up_bnd = globalUpperBnds[j], max_lo_bnd = globalLowerBnds[j];
        for (k=index+1; k<num_tr_m1; ++k) {
          Real up_bnd = trustRegions[k].tr_upper_bound(j),
	       lo_bnd = trustRegions[k].tr_lower_bound(j);
          if (up_bnd < min_up_bnd) min_up_bnd = up_bnd;
          if (lo_bnd > max_lo_bnd) max_lo_bnd = lo_bnd;
        }
	if (max_lo_bnd > min_up_bnd) {
	  Cerr << "Error: inconsistent parent bounds in HierarchSurrBasedLocal"
	       << "Minimizer::update_trust_region()." << std::endl;
	  abort_handler(METHOD_ERROR);
	  // TODO Recourse: target levels that are causing an undesirable
	  // truncation and recenter w/ new evals w/i hierarchy
	}
	else {
	  parent_upper_bnds[j] = min_up_bnd;
	  parent_lower_bnds[j] = max_lo_bnd;
	}
      }
      // Compute new trust region and assign to approxSubProbModel
      // (for the lowest level with opt over LF + \Sum_j delta_j,j+1)
      // As enforced above, the bounds for the lowest level are a strict
      // subset of all level bounds --> candidate iterate that gets
      // generated by the sub-problem solver will lie within all higher
      // level TRs and not require any recentering.
      update_trust_region_data(trustRegions[index], parent_lower_bnds,
			       parent_upper_bnds);
    }
  }
}


/** Step 2 in SurrBasedLocalMinimizer::core_run(). */
void HierarchSurrBasedLocalMinimizer::build()
{
  // Recursively build new approximations according to tr[i].new_center():
  // > if new center at current level, build new approximation
  // > if new center at or above current level, update corrected responses.

  size_t j, num_tr = trustRegions.size(); int i;
  for (i=minimizeIndex; i<num_tr; ++i) {
    SurrBasedLevelData& tr_data = trustRegions[i];
    int ip1 = i + 1; bool last_tr = (ip1 == num_tr);

    // If new candidate indicated for a level, then:
    // > compute response_star_truth (values only)
    // > evaluate TR ratio for improvement for this level at new point
    // > accept/reject step and update TR factor / bounds
    if (tr_data.status(NEW_CANDIDATE)) { // verification needed

      // Evaluate truth_model for TR_i (vals only, +derivs later in build),
      // computes TR ratio, accepts/rejects step, scales TR, and updates
      // vars/resp center if accepted
      verify(i); // updates center vars

      if (tr_data.status(NEW_CENTER)) {
	if (last_tr || !nestedTrustRegions)
	  update_trust_region_data(tr_data, globalLowerBnds, globalUpperBnds);
	else
	  update_trust_region_data(tr_data, trustRegions[ip1].tr_lower_bounds(),
				   trustRegions[ip1].tr_upper_bounds());
      }
    }

    // If new center accepted for a level, then promote star to center,
    // build new approx w/ derivs (response_center_truth)
    if (tr_data.status(NEW_CENTER)) {
      // build approximation at level i and retrieve center truth response

      set_model_states(i); // only HF model is evaluated
      Variables& center_vars = tr_data.vars_center();
      iteratedModel.active_variables(center_vars);
      //iteratedModel.continuous_lower_bounds(tr_data.tr_lower_bounds());
      //iteratedModel.continuous_upper_bounds(tr_data.tr_upper_bounds());
      iteratedModel.build_approximation();

      // Extract truth model evaluation.
      // Note: code from DFSBLM case does lookup, which makes sense if last HF
      // eval was a rejected validation, but if find_center_truth() always
      // follows build_approximation(), then this lookup is not necessary.
      find_center_truth(i); // find/eval *uncorrected* center truth
      // apply recursive correction to top level truth, if needed
      correct_center_truth(i);
      
      // Recursive assessment of hard convergence is bottom up (TR bounds and 
      // corrections managed top-down).  When one level has hard converged,
      // must update and recenter level above.
      // TODO: need to manage soft convergence as well...
      if (last_tr)
	hard_convergence_check(tr_data, globalLowerBnds, globalUpperBnds);
      else
	hard_convergence_check(tr_data, trustRegions[ip1].tr_lower_bounds(),
			       trustRegions[ip1].tr_upper_bounds());
      if (tr_data.converged()) {
	if (last_tr) convergenceCode = 4; // TODO
	else {
	  trustRegions[ip1].vars_star(center_vars); // trigger build for next TR
	  tr_data.reset_status_bits(CONVERGED);
	}
      }
    }
    // TO DO: anything additional for reject? (see Monschke diagram)
    //else {
    //}
  }

  if (convergenceCode)
    return;

  // Loop TRs top-down so that correction logic detects new centers at/above
  bool update_corr = false; 
  for (i=num_tr-1; i>=minimizeIndex; --i) {

    SurrBasedLevelData& tr_data = trustRegions[i];
    bool new_level_center  = tr_data.status(NEW_CENTER);
    Variables& center_vars = tr_data.vars_center();

    if (new_level_center) {
      // all levels at or below this level must update corrected responses
      update_corr = true;

      // Find approx response.  If not found, evaluate approx model.
      find_center_approx(i); // find/eval *uncorrected* center approx

      // Compute additive/multiplicative correction
      DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();
      delta.compute(center_vars,
		    tr_data.response_center(UNCORR_TRUTH_RESPONSE),
		    tr_data.response_center(UNCORR_APPROX_RESPONSE));
    }

    if (update_corr) {
      // Recursively correct truth response (all levels above, excepting
      // current level) and store in tr_data
      if (i+1 < num_tr) {
	Cout << "\nRecursively correcting truth model response (form "
	     << tr_data.truth_model_form();
	if (tr_data.truth_model_level() != _NPOS)
	  Cout << ", level " << tr_data.truth_model_level();
	Cout << ") for trust region center.\n";
	Response corrected_resp
	  = tr_data.response_center(UNCORR_TRUTH_RESPONSE).copy();
	for (j=i+1; j<num_tr; ++j)
	  iteratedModel.single_apply(center_vars, corrected_resp,
				     trustRegions[j].indices());
	tr_data.response_center(corrected_resp, CORR_TRUTH_RESPONSE);
      }
      else
	tr_data.response_center(tr_data.response_center(UNCORR_TRUTH_RESPONSE),
				CORR_TRUTH_RESPONSE);

    //}
    //if (update_corr) {

      // Recursively correct approx response and store in tr_data
      Cout << "\nRecursively correcting surrogate model response (form "
	   << tr_data.approx_model_form();
      if (tr_data.approx_model_level() != _NPOS)
	Cout << ", level " << tr_data.approx_model_level();
      Cout << ") for trust region center.\n";
      // correct approximation across all levels above i
      Response corrected_resp
	= tr_data.response_center(UNCORR_APPROX_RESPONSE).copy();
      for (j=i; j<num_tr; ++j)
	iteratedModel.single_apply(center_vars, corrected_resp,
				   trustRegions[j].indices());
      tr_data.response_center(corrected_resp, CORR_APPROX_RESPONSE);
    }

    // new center now computed, deactivate flag
    if (new_level_center) tr_data.reset_status_bits(NEW_CENTER);
  }
}


/** Step 3 in SurrBasedLocalMinimizer::core_run(). */
void HierarchSurrBasedLocalMinimizer::minimize()
{
  // *********************************
  // Optimize at (fully corrected) lowest fidelity only:
  // *********************************

  // Set truth and surrogate models for optimization to be performed on:
  set_model_states(minimizeIndex);

  // set up recursive corrections across all model forms
  ((HierarchSurrModel*)(iteratedModel.model_rep()))->
    correction_mode(FULL_MODEL_FORM_CORRECTION);

  // Set the trust region center and bounds for approxSubProbOptimizer
  SurrBasedLevelData& tr_min = trustRegions[minimizeIndex];
  update_approx_sub_problem(tr_min);

  // solve the approximate optimization sub-problem:
  SurrBasedLocalMinimizer::minimize();

  // Retrieve vars_star and responseStarCorrected[lf_model_form]
  // Corrections are applied recursively during the minimization, so this
  // response is corrected to the highest fidelity level.
  const Variables& v_star = approxSubProbMinimizer.variables_results();
  tr_min.vars_star(v_star);
  if (recastSubProb) {
    // Can't back out eval from recast data and can't assume last iteratedModel
    // eval was the final solution, but can use a DB search for hierarchical
    // (with fallback to new eval if not found).
    find_star_approx(minimizeIndex); // -> uncorrected resp_star_approx
    // apply correction and store
    Response corr_resp(tr_min.response_star(UNCORR_APPROX_RESPONSE).copy());
    iteratedModel.recursive_apply(v_star, corr_resp);
    tr_min.response_star(corr_resp, CORR_APPROX_RESPONSE);
  }
  else // retrieve corrected final results
    tr_min.response_star(approxSubProbMinimizer.response_results(),
			 CORR_APPROX_RESPONSE); // Note: fn values only
}


/** Step 4 in SurrBasedLocalMinimizer::core_run(). */
void HierarchSurrBasedLocalMinimizer::verify(size_t tr_index)
{
  // ****************************
  // Validate candidate point
  // ****************************

  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  Variables& vars_star = tr_data.vars_star(); // candidate iterate

  set_model_states(tr_index);
  Model& truth_model = iteratedModel.truth_model();

  Cout << "\n>>>>> Evaluating approximate solution with actual model.\n";
  iteratedModel.component_parallel_mode(TRUTH_MODEL);
  truth_model.active_variables(vars_star);
  truth_model.evaluate(tr_data.active_set_star(TRUTH_RESPONSE));

  // Apply correction recursively so that this truth response is consistent
  // with the highest fidelity level:
  tr_data.response_star(truth_model.current_response(), UNCORR_TRUTH_RESPONSE);
  correct_star_truth(tr_index);

  // For accept/reject of opt subproblem step within verify(), we only
  // require a single TR ratio:
  compute_trust_region_ratio(tr_data); // no check_interior
  // Recursive TR updating requires (hard) convergence assessment to indicate
  // when iterations at a lower level are complete and the next higher level
  // TR needs to be recentered --> this occurs within build().
   
  // Check for convergence globally (max SBLM iterations)
  if (sbIterNum >= maxIterations)
    convergenceCode = 2;
  // Check for convergence metrics for this TR:
  if (tr_data.trust_region_factor() < minTrustRegionFactor)
    tr_data.set_status_bits(MIN_TR_CONVERGED);
  if (tr_data.soft_convergence_count() >= softConvLimit)
    tr_data.set_status_bits(SOFT_CONVERGED);
}

// Note: find() implies a DB lookup and DB entries are uncorrected, so employ
// this convention consistently and correct after find() when needed.

void HierarchSurrBasedLocalMinimizer::find_center_truth(size_t tr_index)
{
  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  Model& truth_model = iteratedModel.truth_model();

  // TODO: hard-coded for now, see note in build()
  tr_data.response_center(truth_model.current_response(),UNCORR_TRUTH_RESPONSE);
  bool truth_found = true;

  if (!truth_found) {
    Cout << "\n>>>>> Evaluating actual model at trust region center.\n";

    // since we're bypassing iteratedModel, iteratedModel.serve()
    // must be in the correct server mode.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    truth_model.active_variables(tr_data.vars_center());
    truth_model.evaluate(tr_data.active_set_center(TRUTH_RESPONSE));

    tr_data.response_center(truth_model.current_response(),
                            UNCORR_TRUTH_RESPONSE);
  }
}


void HierarchSurrBasedLocalMinimizer::correct_center_truth(size_t tr_index)
{
  // Must perform hard conv assessment on corrected truth, as consistent with
  // subproblem optimization.  However, this correction is dependent on the
  // hierarchical state of truth resp above, to be updated later in this loop.
  // We therefore require a final pass (bottom of build()) to update all
  // corrected responses once all recursive builds are completed.
  // > Hard convergence is assessed based on new UNCORR_TRUTH_RESPONSE
  //   combined with previous corrections for all levels above.
  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  size_t j, ip1 = tr_index + 1, num_tr = trustRegions.size();
  if (ip1 == num_tr) // last trust region
    tr_data.response_center(tr_data.response_center(UNCORR_TRUTH_RESPONSE),
			    CORR_TRUTH_RESPONSE);
  else {
    Cout << "\nRecursively correcting truth model response (form "
	 << tr_data.truth_model_form();
    if (tr_data.truth_model_level() != _NPOS)
      Cout << ", level " << tr_data.truth_model_level();
    Cout << ") for trust region center.\n";
    Variables& center_vars = tr_data.vars_center();
    Response corrected_resp
      = tr_data.response_center(UNCORR_TRUTH_RESPONSE).copy();
    for (j=ip1; j<num_tr; ++j)
      iteratedModel.single_apply(center_vars, corrected_resp,
				 trustRegions[j].indices());
    tr_data.response_center(corrected_resp, CORR_TRUTH_RESPONSE);
  }
}


void HierarchSurrBasedLocalMinimizer::find_star_truth(size_t tr_index)
{
  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  Model& truth_model = iteratedModel.truth_model();

  // TODO: hard-coded for now, see note in build()
  tr_data.response_star(truth_model.current_response(), UNCORR_TRUTH_RESPONSE);
  bool truth_found = true;

  if (!truth_found) {
    Cout << "\n>>>>> Verifying actual model at trust region candidate.\n";

    // since we're bypassing iteratedModel, iteratedModel.serve()
    // must be in the correct server mode.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    truth_model.active_variables(tr_data.vars_star());
    truth_model.evaluate(tr_data.active_set_star(TRUTH_RESPONSE)); // *** verifyset, not build set

    tr_data.response_star(truth_model.current_response(),
			  UNCORR_TRUTH_RESPONSE);
  }
}


void HierarchSurrBasedLocalMinimizer::correct_star_truth(size_t tr_index)
{
  // Must perform hard conv assessment on corrected truth, as consistent with
  // subproblem optimization.  However, this correction is dependent on the
  // hierarchical state of truth resp above, to be updated later in this loop.
  // We therefore require a final pass (bottom of build()) to update all
  // corrected responses once all recursive builds are completed.
  // > Hard convergence is assessed based on new UNCORR_TRUTH_RESPONSE
  //   combined with previous corrections for all levels above.
  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  size_t j, ip1 = tr_index + 1, num_tr = trustRegions.size();
  if (ip1 == num_tr) // last trust region
    tr_data.response_star(tr_data.response_star(UNCORR_TRUTH_RESPONSE),
			    CORR_TRUTH_RESPONSE);
  else {
    Cout << "\nRecursively correcting truth model response (form "
	 << tr_data.truth_model_form();
    if (tr_data.truth_model_level() != _NPOS)
      Cout << ", level " << tr_data.truth_model_level();
    Cout << ") for trust region candidate.\n";
    Variables& star_vars = tr_data.vars_star();
    Response corrected_resp
      = tr_data.response_star(UNCORR_TRUTH_RESPONSE).copy();
    for (j=ip1; j<num_tr; ++j)
      iteratedModel.single_apply(star_vars, corrected_resp,
				 trustRegions[j].indices());
    tr_data.response_star(corrected_resp, CORR_TRUTH_RESPONSE);
  }
}


void HierarchSurrBasedLocalMinimizer::find_center_approx(size_t tr_index)
{
  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  const Variables&   v_center = tr_data.vars_center();

  bool approx_found
    = find_approx_response(v_center,
			   tr_data.response_center(UNCORR_APPROX_RESPONSE));

  if (!approx_found) {
    Cout <<"\n>>>>> Evaluating approximation at trust region center.\n";
    iteratedModel.component_parallel_mode(SURROGATE_MODEL);
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    iteratedModel.active_variables(v_center);
    iteratedModel.evaluate(tr_data.active_set_center(APPROX_RESPONSE));
    tr_data.response_center(iteratedModel.current_response(),
			    UNCORR_APPROX_RESPONSE);
  }
}


void HierarchSurrBasedLocalMinimizer::find_star_approx(size_t tr_index)
{
  SurrBasedLevelData& tr_data = trustRegions[tr_index];
  const Variables&     v_star = tr_data.vars_star();

  bool approx_found
    = find_approx_response(v_star,
			   tr_data.response_star(UNCORR_APPROX_RESPONSE));

  if (!approx_found) {
    Cout <<"\n>>>>> Evaluating approximation at candidate optimum.\n";
    iteratedModel.component_parallel_mode(SURROGATE_MODEL);
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    iteratedModel.active_variables(v_star);
    iteratedModel.evaluate(); // fn values only
    tr_data.response_star(iteratedModel.current_response(),
			  UNCORR_APPROX_RESPONSE);
  }
}


// top level: not bound to core_run() yet...
void HierarchSurrBasedLocalMinimizer::multigrid_driver(const Variables &x0)
{
  RealVector vars_star = x0.continuous_variables();

  int max_iter = 10, iter = 0;
  while (!convergenceCode && iter < max_iter) {
    // Perform one complete V cycle:
    // recursively applied MG/Opt to all levels w/ line search
    // (no prolongation/restriction at this pt)
    //
    // vars_star returned has already been validated at the HF level
    // (from final line search or final opt post-smoothing)
    vars_star = multigrid_recursion(vars_star, numLev[0] - 1);

    // Only need is to check for hard + soft convergence at top level (no TR
    // updates).  If not converged, then continue V cycles until max iter.
    // On exit of V cycle, we have performed a line search at HF level, but 
    // no gradient evaluation assuming value-based line search.

    // TODO: add code here

    ++iter;
  }

  // TODO: warm starting of Hessian approximations were critical to render this
  // algorithm competitive in Jason's MATLAB prototype
  // > 1st-order MG/Opt was ~2x better than 1st-order single-level opt.
  // > Quasi-2nd-order MG/Opt required warm starts to achieve ~2x again,
  //   relative to 2nd-order single-level opt.  W/o warm starts, 2nd-order
  //   MG/Opt was similar to 1st-order MG/Opt since secant updates did not 
  //   have sufficient iters to accumulate.

  // Note: if remove pre- and post-optimization and replace line search with
  // TR logic, then basically recover TRMM.
  // Jason's feeling is that TR usage is the most fruitful direction for MG/Opt:
  // > pre-optimization w/o max_iters limited by TR
  // > instead of line search, compute TR ratios, accept/reject, adapt
  // An important question is whether pre-optimization adds anything relative
  // to TRMM without it.
}


RealVector HierarchSurrBasedLocalMinimizer::
multigrid_recursion(const RealVector& x0_k, int k)
// TODO: pass full Vars object i/o continuous only
{
  // TODO: flatten and iterate across V cycle to replace fn recursion at ***

  // V cycle:
  // > comes in with k = n-1 and recurs top down with partial opt (max_iter = 3)
  //   until k = 0 --> full/more extensive opt (max_iter = 30)
  // > then begins to unroll nested call stack, bottom up, from k=0 to k=n-1

  if (k == 0) { // Full optimization on corrected lowest level
    // Some authors: run until convergence; others: tune as well.
    // Care is needed due to embedded use of local corrections.
    // **********************************************************************
    // Either need to add trust region control or continue poor-man's control
    // with max iterations to avoid leaving region of correction accuracy.
    // **********************************************************************
    int max_iter = 30; // until convergence...  (tune or expose?)
    return optimize(x0_k, max_iter, k); // Steps 3,4
  }
  else { // Partial optimization

    // Step 6: pre-optimization
    int max_iter = 3;
    RealVector x1_k = optimize(x0_k, max_iter, k); // pre-opt / step 1, level k

    // Step 7: Restriction: x1_km1 = R[x1_k]

    // Step 8: Build discrepancy correction between levels k and k-1:
    // TODO: add code here to compute discrepancy corrections
    //   reuse same trustRegions arrays (but support multidimensional MLMF case)

    // Step 9: Recursively call multigrid_recursion
    RealVector x2_km1
      = multigrid_recursion(x1_k, k-1); // use x1_km1 if restriction

    // Step 10a: prolongation x2_k = P[x2_km1]

    // Step 10b: compute search direction from difference in iterates
    RealVector p_k = x2_km1; // use x2_k if prolongation
    p_k -= x1_k;

    // Step 11: perform line search:
    Real alpha0 = 1.;
    RealVector x2_k = linesearch(x1_k, p_k, alpha0);

    // Optional additional step 12: "post smoothing" (partial opt)
    // This step not represented in Jason's graphic; but does appear in
    // some papers (Lewis & Nash?, Borzi?)
    //return optimize(x2_k, max_iter, k);
    // ...OR...
    return x2_k;
  }
}


// alpha0 is initial step size;
// pk is search direction defined from change in x;
// xk comes in as x^{k-1}, return as x^k
RealVector HierarchSurrBasedLocalMinimizer::
linesearch(const RealVector &xk, const RealVector &pk, double alpha0)
{
  // TODO: add a real line search algorithm here...

  // For now, always accept full step defined from predicted change in x:
  RealVector new_xk = xk;
  // Perform alpha step.  Note: s = -grad f (take care with sign of pk)
  size_t i, len = xk.length();
  for (i=0; i<len; ++i)
    new_xk[i] += alpha0 * pk[i];
  return new_xk;
}


RealVector HierarchSurrBasedLocalMinimizer::
optimize(const RealVector &x, int max_iter, int index)
{
  // Update starting point for optimization:
  approxSubProbModel.continuous_variables(x);

  // Set the max iterations for this level:
  approxSubProbMinimizer.maximum_iterations(max_iter);

  // Set truth and surrogate models for optimization to be performed on:
  set_model_states(index);

  // set up recursive corrections across all solution levels
  ((HierarchSurrModel*)(iteratedModel.model_rep()))->
    correction_mode(FULL_SOLUTION_LEVEL_CORRECTION);

  iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  approxSubProbMinimizer.run(pl_iter); // pl_iter required for hierarchical

  // Return candidate point:
  return approxSubProbMinimizer.variables_results().continuous_variables();
}

} // namespace Dakota
