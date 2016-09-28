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
#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif // HAVE_NPSOL

//#define DEBUG
#define TRUTH_MODEL 2
#define APPROX_MODEL 3


namespace Dakota
{
extern PRPCache data_pairs; // global container

// initialization of statics
//HierarchSurrBasedLocalMinimizer*
//HierarchSurrBasedLocalMinimizer::mlmfInstance(NULL);


HierarchSurrBasedLocalMinimizer::
HierarchSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedLocalMinimizer(problem_db, model), minimizeIndex(0),
  nestedTrustRegions(true)
{
  // check iteratedModel for model form hierarchy and/or discretization levels
  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: HierarchSurrBasedLocalMinimizer requires a hierarchical "
         << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  approxSubProbModel = iteratedModel;

  // Instantiate the approximate sub-problem minimizer
  initialize_sub_minimizer();

  // Get number of model fidelities and number of levels for each fidelity:
  ModelList& models = iteratedModel.subordinate_models(false);
  numFid = models.size();
  numLev.resize(numFid);
  ModelLIter ml_iter; size_t i;
  for (ml_iter=models.begin(), i=0; i<numFid; ++ml_iter, ++i)
    numLev[i] = ml_iter->solution_levels();

  // TODO: This is specific for just multifidelity:
  // TODO: This is hard coded for just multifidelity:
  for (i=0; i<numFid-1; ++i) {
    trustRegions.push_back(HierarchSurrBasedLocalMinimizerHelper(
      iteratedModel.truth_model().current_response(), i, i+1)); // *** MSE: response from each lev?
    trustRegions[i].trust_region_factor(origTrustRegionFactor *
					std::pow(0.5, numFid-2-i));
    trustRegions[i].new_center(true);
  }
}


HierarchSurrBasedLocalMinimizer::~HierarchSurrBasedLocalMinimizer()
{ }


void HierarchSurrBasedLocalMinimizer::pre_run()
{
  SurrBasedLocalMinimizer::pre_run();

  // static pointer to HierarchSurrBasedLocalMinimizer instance
  //mlmfInstance = this;

  for (size_t ii = 0; ii < trustRegions.size(); ii++) {
    trustRegions[ii].vars_center(iteratedModel.current_variables());
    trustRegions[ii].tr_lower_bnds(globalLowerBnds);
    trustRegions[ii].tr_upper_bnds(globalLowerBnds);
  }
}


void HierarchSurrBasedLocalMinimizer::post_run(std::ostream& s)
{
  // restore original/global bounds
  //approxSubProbModel.continuous_variables(initialPoint);
  //if (recastSubProb) iteratedModel.continuous_variables(initialPoint);
  approxSubProbModel.continuous_lower_bounds(globalLowerBnds);
  approxSubProbModel.continuous_upper_bounds(globalUpperBnds);

  bestVariablesArray.front().continuous_variables(
    trustRegions[minimizeIndex].vars_center().continuous_variables());
  bestResponseArray.front().function_values(
    trustRegions[minimizeIndex].response_center(TRUTH_MODEL,
        false).function_values());

  SurrBasedLocalMinimizer::post_run(s);
}


void HierarchSurrBasedLocalMinimizer::reset()
{
  SurrBasedLocalMinimizer::reset();

  for(size_t ii = 0; ii < trustRegions.size(); ii++) {
    trustRegions[ii].trust_region_factor(origTrustRegionFactor);
    trustRegions[ii].new_center(true);
  }
}


void HierarchSurrBasedLocalMinimizer::update_trust_region()
{
  Cout << "Update trust region" << std::endl;

  for (int ind = trustRegions.size()-1; ind >= 0; ind--) {
    // Compute the trust region bounds
    RealVector parent_upper_bounds, parent_lower_bounds;
    if (ind == trustRegions.size()-1 || (!nestedTrustRegions && ind > 0)) {
      parent_upper_bounds = globalUpperBnds;
      parent_lower_bounds = globalLowerBnds;
    } else if (nestedTrustRegions) {
      parent_upper_bounds = trustRegions[ind+1].tr_upper_bnds();
      parent_lower_bounds = trustRegions[ind+1].tr_lower_bnds();
    } else {
      for (int jj = 0; jj < numContinuousVars; jj++) {
        double min_upper_bound = 0.0;
        double max_lower_bound = 0.0;
        for (int ii = 1; ii < trustRegions.size(); ii++) {
          RealVector upper_bounds = trustRegions[ii].tr_upper_bnds();
          RealVector lower_bounds = trustRegions[ii].tr_lower_bnds();
          if (ii == 1 || upper_bounds[jj] < min_upper_bound)
            min_upper_bound = upper_bounds[jj];
          if (ii == 1 || lower_bounds[jj] < max_lower_bound)
            max_lower_bound = lower_bounds[jj];
        }

        parent_upper_bounds[jj] = min_upper_bound;
        parent_lower_bounds[jj] = max_lower_bound;
      }
    }

    size_t i;
    bool c_vars_truncation = false, tr_lower_truncation = false,
         tr_upper_truncation = false;
    RealVector c_vars_center, tr_lower_bnds(numContinuousVars),
               tr_upper_bnds(numContinuousVars);
    copy_data(trustRegions[ind].vars_center().continuous_variables(),
              c_vars_center);
    for (i=0; i<numContinuousVars; i++) {
      // verify that varsCenter is within global bounds
      if ( c_vars_center[i] > parent_upper_bounds[i] ) {
        c_vars_center[i] = parent_upper_bounds[i];
        c_vars_truncation = true;
      }
      if ( c_vars_center[i] < parent_lower_bounds[i] ) {
        c_vars_center[i] = parent_lower_bounds[i];
        c_vars_truncation = true;
      }
      // scalar tr_offset was previously trustRegionOffset[i]
      Real tr_offset = trustRegions[ind].trust_region_factor()/2. *
                       ( parent_upper_bounds[i] - parent_lower_bounds[i] );
      Real up_bound = c_vars_center[i] + tr_offset;
      Real lo_bound = c_vars_center[i] - tr_offset;
      if ( up_bound <= parent_upper_bounds[i] )
        tr_upper_bnds[i] = up_bound;
      else {
        tr_upper_bnds[i] = parent_upper_bounds[i];
        tr_upper_truncation = true;
      }
      if ( lo_bound >= parent_lower_bounds[i] )
        tr_lower_bnds[i] = lo_bound;
      else {
        tr_lower_bnds[i] = parent_lower_bounds[i];
        tr_lower_truncation = true;
      }
    }
    if (c_vars_truncation) {
      trustRegions[ind].vars_center().continuous_variables(c_vars_center);
      trustRegions[ind].new_center(true);
    }

    trustRegions[ind].tr_lower_bnds(tr_lower_bnds);
    trustRegions[ind].tr_upper_bnds(tr_upper_bnds);

    // Output the trust region bounds
    Cout << "\n**************************************************************"
         << "************\nBegin SBLM Iteration Number " << sbIterNum+1
         << "\n\nTrust Region " << ind << "\n";
    Cout << "Truth model form = " << trustRegions[ind].truth_model_form() <<
         std::endl;
    Cout << "Truth model level = " << trustRegions[ind].truth_model_level() <<
         std::endl;
    Cout << "Approx. model form = " << trustRegions[ind].approx_model_form() <<
         std::endl;
    Cout << "Approx. model level = " << trustRegions[ind].approx_model_level() <<
         std::endl;
    Cout << "                 ";
    if (tr_lower_truncation)
      Cout << std::setw(write_precision+9) << "Lower (truncated)";
    else
      Cout << std::setw(write_precision+9) << "Lower";
    Cout   << std::setw(write_precision+9) << "Center";
    if (tr_upper_truncation)
      Cout << std::setw(write_precision+9) << "Upper (truncated)";
    else
      Cout << std::setw(write_precision+9) << "Upper";
    Cout << '\n';
    StringMultiArrayConstView c_vars_labels
      = iteratedModel.continuous_variable_labels();
    for (i=0; i<numContinuousVars; i++)
      Cout << std::setw(16) << c_vars_labels[i] << ':'
           << std::setw(write_precision+9)
           << tr_lower_bnds[i] << std::setw(write_precision+9) << c_vars_center[i]
           << std::setw(write_precision+9) << tr_upper_bnds[i] << '\n';
    Cout << "****************************************************************"
         << "**********\n";
  }

  // Set the trust region center and bounds for approxSubProbOptimizer
  approxSubProbModel.continuous_variables(
    trustRegions[minimizeIndex].vars_center().continuous_variables());
  approxSubProbModel.continuous_lower_bounds(
    trustRegions[minimizeIndex].tr_lower_bnds());
  approxSubProbModel.continuous_upper_bounds(
    trustRegions[minimizeIndex].tr_upper_bnds());
}

void HierarchSurrBasedLocalMinimizer::build()
{
  // Compute new trust region centers:
  for (size_t ii = 0; ii < trustRegions.size(); ii++) {
    // Set the trust region center and bounds
    iteratedModel.continuous_variables(
      trustRegions[ii].vars_center().continuous_variables());
    iteratedModel.continuous_lower_bounds(trustRegions[ii].tr_lower_bnds());
    iteratedModel.continuous_upper_bounds(trustRegions[ii].tr_upper_bnds());

    if (trustRegions[ii].new_center()) {
      set_model_states(ii);

      // This only evaluates the high fidelity model:
      iteratedModel.build_approximation();

      // Extract truth model evaluation and evaluate approx model
      find_center(ii);

      if (!convergenceFlag) {
        // ******************************************
        // Compute additive/multiplicative correction
        // ******************************************

        DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();
        delta.compute(trustRegions[ii].vars_center(),
                      trustRegions[ii].response_center(TRUTH_MODEL, false),
                      trustRegions[ii].response_center(APPROX_MODEL, false));
      }

      trustRegions[ii].new_center(false);
    }
  }

  // TODO: This assumes only model fidelities:
  // Recompute corrected center responses:
  for (size_t ii = 0; ii < trustRegions.size(); ii++) {
    // Compute responseCenterCorrected
    Cout << "\nApplying corrections to trust region center response for:" <<
         std::endl;
    Cout << "Approx. model form = " << trustRegions[ii].approx_model_form() <<
         std::endl;
    Cout << "Approx. model level = " << trustRegions[ii].approx_model_level() <<
         std::endl;
    Response response_center_corrected_temp = trustRegions[ii].response_center(
          APPROX_MODEL, false);
    for (size_t jj = ii; jj < trustRegions.size(); jj++) {
      set_model_states(jj);

      DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();
      delta.apply(trustRegions[ii].vars_center(), response_center_corrected_temp);
    }
    trustRegions[ii].response_center(response_center_corrected_temp, APPROX_MODEL,
                                     true);
  }

  // highest fidelity model doesn't need correcting:
  trustRegions.back().response_center(trustRegions.back().response_center(
                                        TRUTH_MODEL, false),TRUTH_MODEL, true);
}



void HierarchSurrBasedLocalMinimizer::
find_center(size_t tr_index)
{
  bool found = false;

  Model& truth_model = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();

  trustRegions[tr_index].response_center(truth_model.current_response(),
                                         TRUTH_MODEL, false);

  // TODO: this is hard-coded:
  found = true;

  if (!found) {
    Cout << "\n>>>>> Evaluating actual model at trust region center.\n";

    // since we're bypassing iteratedModel, iteratedModel.serve()
    // must be in the correct server mode.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    truth_model.continuous_variables(
      trustRegions[tr_index].vars_center().continuous_variables());
    truth_model.evaluate(trustRegions[tr_index].active_set());

    trustRegions[tr_index].response_center(truth_model.current_response(),
                                           TRUTH_MODEL, false);
  }

  hard_convergence_check(tr_index);

  if (!convergenceFlag) {
    found = false;

    // search for fn vals, grads, and Hessians separately since they may
    // be different fn evaluations
    ActiveSet search_set = trustRegions[tr_index].active_set(); // copy
    search_set.request_values(1);
    const Variables& search_vars = iteratedModel.current_variables();
    const String& search_id = iteratedModel.surrogate_model().interface_id();
    PRPCacheHIter cache_it
      = lookup_by_val(data_pairs, search_id, search_vars, search_set);
    if (cache_it != data_pairs.get<hashed>().end()) {
      trustRegions[tr_index].response_center(APPROX_MODEL,
                                             false).function_values(cache_it->response().function_values());
      search_set.request_values(2);
      cache_it
        = lookup_by_val(data_pairs, search_id, search_vars, search_set);
      if (cache_it != data_pairs.get<hashed>().end()) {
        trustRegions[tr_index].response_center(APPROX_MODEL,
                                               false).function_gradients(cache_it->response().function_gradients());
        found = true;
      }
    }

    if (!found) {
      Cout <<"\n>>>>> Evaluating approximation at trust region center.\n";
      iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
      iteratedModel.evaluate(trustRegions[tr_index].active_set());
      trustRegions[tr_index].response_center(iteratedModel.current_response(),
                                             APPROX_MODEL, false);
    }
  }
}


/** The hard convergence check computes the gradient of the merit
    function at the trust region center, performs a projection for
    active bound constraints (removing any gradient component directed
    into an active bound), and signals convergence if the 2-norm of
    this projected gradient is less than convergenceTol. */
void HierarchSurrBasedLocalMinimizer::
hard_convergence_check(size_t tr_index)
{
  Response truth_center_uncorrected = trustRegions[tr_index].response_center(
                                        TRUTH_MODEL, false);
  const RealVector& fns_truth   = truth_center_uncorrected.function_values();
  const RealMatrix& grads_truth = truth_center_uncorrected.function_gradients();

  RealVector fn_grad(numContinuousVars, true);
  const BoolDeque& sense = iteratedModel.primary_response_fn_sense();
  const RealVector&  wts = iteratedModel.primary_response_fn_weights();

  // objective function portion
  objective_gradient(fns_truth, grads_truth, sense, wts, fn_grad);

  Real fn_grad_norm = 0.0;
  for (size_t i=0; i<numContinuousVars; i++) {
    fn_grad_norm += std::pow(fn_grad[i], 2);
  }

  // Terminate SBLM if the norm of the projected merit function gradient
  // at x_c is less than convTol (hard convergence).
  fn_grad_norm = std::sqrt( fn_grad_norm );
  if ( fn_grad_norm < convergenceTol )
    convergenceFlag = 4; // hard convergence

#ifdef DEBUG
  Cout << "In hard convergence check: merit_fn_grad_norm =  "
       << merit_fn_grad_norm << '\n';
#endif
}

void HierarchSurrBasedLocalMinimizer::minimize()
{
  // *********************************
  // Optimize at (fully corrected) lowest fidelity only:
  // *********************************

  // Set truth and surrogate models for optimization to be performed on:
  set_model_states(minimizeIndex);

  ((HierarchSurrModel*)(iteratedModel.model_rep()))->
  correction_mode(FULL_MODEL_FORM_CORRECTION);

  Cout << "\n>>>>> Starting approximate optimization cycle.\n";
  iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  approxSubProbMinimizer.run(pl_iter); // pl_iter required for hierarchical
  Cout << "\n<<<<< Approximate optimization cycle completed.\n";
  sbIterNum++; // full iteration performed: increment the counter

  // Retrieve vars_star and responseStarCorrected[lf_model_form]
  trustRegions[minimizeIndex].vars_star(
    approxSubProbMinimizer.variables_results());
  trustRegions[minimizeIndex].response_star(
    approxSubProbMinimizer.response_results(), APPROX_MODEL, true);
}

void HierarchSurrBasedLocalMinimizer::verify()
{
  // ****************************
  // Validate candidate point
  // ****************************

  int tr_index = minimizeIndex;

  Cout << "\n>>>>> Evaluating approximate solution with actual model.\n";

  set_model_states(tr_index);

  Model& truth_model = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();

  iteratedModel.component_parallel_mode(TRUTH_MODEL);
  truth_model.active_variables(trustRegions[minimizeIndex].vars_star());
  truth_model.evaluate(trustRegions[minimizeIndex].active_set());

  trustRegions[minimizeIndex].response_star(truth_model.current_response(),
      TRUTH_MODEL, false);

  tr_ratio_check(tr_index);

  // If the candidate optimum (vars_star) is accepted, then update the
  // center variables and response data.
  if (trustRegions[tr_index].new_center()) {
    trustRegions[tr_index].vars_center(trustRegions[minimizeIndex].vars_star());

    trustRegions[tr_index].response_center(truth_model.current_response(),
                                           TRUTH_MODEL, false);
  }

  // Check for soft convergence:
  if (softConvCount >= softConvLimit)
    convergenceFlag = 3; // soft convergence
  // terminate SBLM if trustRegionFactor is less than its minimum value
  else if (trustRegions[tr_index].trust_region_factor() < minTrustRegionFactor)
    convergenceFlag = 1;
  // terminate SBLM if the maximum number of iterations has been reached
  else if (sbIterNum >= maxIterations)
    convergenceFlag = 2;
}


/** Assess acceptance of SBLM iterate (trust region ratio or filter)
    and compute soft convergence metrics (number of consecutive
    failures, min trust region size, etc.) to assess whether the
    convergence rate has decreased to a point where the process should
    be terminated (diminishing returns). */
void HierarchSurrBasedLocalMinimizer::tr_ratio_check(size_t tr_index)
{
  Response truth_center_uncorrected = trustRegions[tr_index].response_center(
                                        TRUTH_MODEL, false);
  Response truth_star_uncorrected = trustRegions[tr_index].response_star(
                                      TRUTH_MODEL, false);
  Response approx_center_corrected = trustRegions[tr_index].response_center(
                                       APPROX_MODEL,true);
  Response approx_star_corrected = trustRegions[tr_index].response_star(
                                     APPROX_MODEL,true);

  const RealVector& fns_center_truth = truth_center_uncorrected.function_values();
  const RealVector& fns_star_truth = truth_star_uncorrected.function_values();
  const RealVector& fns_center_approx = approx_center_corrected.function_values();
  const RealVector& fns_star_approx = approx_star_corrected.function_values();

  // ---------------------------------------------------
  // Compute trust region ratio based on merit fn values
  // ---------------------------------------------------

  Real merit_fn_center_truth,  merit_fn_star_truth,
       merit_fn_center_approx, merit_fn_star_approx;

  // TO DO: in global case, search dace_iterator.all_responses() for a
  // better point (using merit fn or filter?) than the minimizer found.
  // If this occurs, then accept point but shrink TR.
  //Iterator& dace_iterator = iteratedModel.subordinate_iterator();
  //IntResponseMap dace_responses;
  //if (globalApproxFlag && !dace_iterator.is_null())
  //  dace_responses = dace_iterator.all_responses();

  // Consolidate objective fn values and constraint violations into a
  // single merit fn value for center truth/approx and star truth/approx.
  const BoolDeque& sense = iteratedModel.primary_response_fn_sense();
  const RealVector&  wts = iteratedModel.primary_response_fn_weights();

  merit_fn_center_truth = objective(fns_center_truth, sense, wts);
  merit_fn_star_truth = objective(fns_star_truth, sense, wts);
  merit_fn_center_approx = objective(fns_center_approx, sense, wts);
  merit_fn_star_approx = objective(fns_star_approx, sense, wts);

  // Compute numerator/denominator for the TR ratio using merit fn values.
  // NOTE 1: this formulation generalizes to the case where correction is not
  // applied.  When correction is applied, this is equivalent to the form of
  // center_truth - star_approx published by some authors (which can give the
  // wrong sense without at least 0th-order consistency).
  // NOTE 2: it is possible for the denominator to be < 0.0 due to
  // (1) inconsistencies in merit function definition between the minimizer
  // and SBLM, or (2) minimizer failure on the surrogate models (e.g., early
  // termination if no feasible solution can be found).  For this reason, the
  // logic checks below cannot rely solely on tr_ratio (and the signs of
  // numerator and denominator must be preserved).
  Real numerator   = merit_fn_center_truth  - merit_fn_star_truth;
  Real denominator = merit_fn_center_approx - merit_fn_star_approx;
  Real tr_ratio = (std::fabs(denominator) > DBL_MIN) ? numerator / denominator
                  : numerator;

  // Accept the step based on simple decrease in the truth merit functions.
  // This avoids any issues with denominator < 0 in tr_ratio:
  //   numerator > 0 and denominator < 0: accept step and contract TR
  //   numerator < 0 and denominator < 0: reject step even though tr_ratio > 0
  bool accept_step = (numerator > 0.);

  // ------------------------------------------
  // Trust region shrink/expand/translate logic
  // ------------------------------------------

  if (accept_step) {
    trustRegions[tr_index].new_center(true);

    // Update the trust region size depending on the accuracy of the approximate
    // model. Note: If eta_1 < tr_ratio < eta_2, trustRegionFactor does not
    // change where eta_1 = trRatioContractValue and eta_2 = trRatioExpandValue
    // Recommended values from Conn/Gould/Toint are: eta_1 = 0.05, eta_2 = 0.90
    // For SBLM, the following are working better:   eta_1 = 0.25, eta_2 = 0.75
    // More experimentation is needed.
    Cout << "\n<<<<< Trust Region Ratio = " << tr_ratio << ":\n<<<<< ";
    if (tr_ratio <= trRatioContractValue) { // accept optimum, shrink TR
      trustRegions[tr_index].scale_trust_region_factor(gammaContract);
      Cout << "Marginal Accuracy, ACCEPT Step, REDUCE Trust Region Size\n\n";
    } else if (std::fabs(1.-tr_ratio) <= 1.-trRatioExpandValue) { //accept & expand
      // for trRatioExpandValue = 0.75, expand if 0.75 <= tr_ratio <= 1.25
      // This new logic avoids increasing the TR size when a good step is found
      // but the surrogates are inaccurate (e.g., tr_ratio = 2).
      trustRegions[tr_index].scale_trust_region_factor(gammaExpand);
      Cout << "Excellent Accuracy, ACCEPT Step, INCREASE Trust Region Size"
           << "\n\n";
    } else // accept optimum, retain current TR
      Cout <<"Satisfactory Accuracy, ACCEPT Step, RETAIN Trust Region Size\n\n";
  } else {
    // If the step is rejected, then retain the current design variables
    // and shrink the TR size.
    trustRegions[tr_index].new_center(false);
    trustRegions[tr_index].scale_trust_region_factor(gammaContract);
    if (tr_ratio > 0.) // rare case of denominator<0
      Cout << "\n<<<<< Trust Region Ratio Numerator = " << numerator;
    else
      Cout << "\n<<<<< Trust Region Ratio = " << tr_ratio;

    Cout << ":\n<<<<< Poor accuracy, REJECT Step, REDUCE Trust Region Size\n\n";
  }

  Real rel_numer = ( std::fabs(merit_fn_center_truth) > DBL_MIN ) ?
                   std::fabs( numerator / merit_fn_center_truth ) : std::fabs(numerator);
  Real rel_denom = ( std::fabs(merit_fn_center_approx) > DBL_MIN ) ?
                   std::fabs( denominator / merit_fn_center_approx ) : std::fabs(denominator);
  if ( !accept_step || numerator   <= 0. || rel_numer < convergenceTol ||
       denominator <= 0. || rel_denom < convergenceTol )
    softConvCount++;
  else
    softConvCount = 0; // reset counter to zero
}

void HierarchSurrBasedLocalMinimizer::set_model_states(size_t tr_index)
{
  size_t approx_model_form = trustRegions[tr_index].approx_model_form();
  size_t truth_model_form = trustRegions[tr_index].truth_model_form();

  size_t approx_model_level = trustRegions[tr_index].approx_model_level();
  size_t truth_model_level = trustRegions[tr_index].truth_model_level();

  iteratedModel.surrogate_model_indices(approx_model_form, approx_model_level);
  iteratedModel.truth_model_indices(truth_model_form, truth_model_level);
}


void HierarchSurrBasedLocalMinimizer::
MG_Opt_driver(const Variables &x0)
{
  RealVector vars_star = x0.continuous_variables();

  int max_iter = 10;
  int num_iter = 0;
  while (!convergenceFlag && num_iter < max_iter) {
    vars_star = MG_Opt(vars_star, numLev[0] - 1);

    // Check for convergence
    // TODO: add code here

    num_iter++;
  }
}

RealVector HierarchSurrBasedLocalMinimizer::
MG_Opt(const RealVector &xk, int k)
{
  if (k == 0) {
    // Full optimization on corrected lowest level
    int max_iter = 30;
    return optimize(xk, max_iter, k);
  } else {
    // Partial optimization
    int max_iter = 3;
    RealVector xk_1 = optimize(xk, max_iter, k);

    // Build discrepancy correction between levels k and k-1:
    // TODO: add code here to compute discrepancy corrections

    // Recursively call MG_Opt
    RealVector xkm1 = MG_Opt(xk_1, k-1);

    // Compute correction:
    RealVector pk(numContinuousVars);
    for (int ii = 0; ii < pk.length(); ii++)
      pk[ii] = xkm1[ii] - xk_1[ii];

    // Perform line search:
    double alpha0 = 1.0;
    RealVector xk_2 = linesearch(xk_1, pk, alpha0);

    // Partial optimization
    return optimize(xk_2, max_iter, k);
  }
}

RealVector HierarchSurrBasedLocalMinimizer::
linesearch(const RealVector &xk, const RealVector &pk, double alpha0)
{
  // TODO: add a real line search algorithm here. For now, always accept the step:
  return xk;
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

  ((HierarchSurrModel*)(iteratedModel.model_rep()))->
  correction_mode(FULL_SOLUTION_LEVEL_CORRECTION);

  iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);
  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  approxSubProbMinimizer.run(pl_iter); // pl_iter required for hierarchical

  // Return candidate point:
  return approxSubProbMinimizer.variables_results().continuous_variables();
}

} // namespace Dakota
