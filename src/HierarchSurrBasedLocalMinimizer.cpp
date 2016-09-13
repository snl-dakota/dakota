/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrBasedLocalMinimizer
//- Description: Implementation code for the HierarchSurrBasedLocalMinimizer class
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


namespace Dakota
{
extern PRPCache data_pairs; // global container

// initialization of statics
HierarchSurrBasedLocalMinimizer* HierarchSurrBasedLocalMinimizer::mlmfInstance(NULL);


HierarchSurrBasedLocalMinimizerHelper::
HierarchSurrBasedLocalMinimizerHelper()
{
  // empty
}

HierarchSurrBasedLocalMinimizerHelper::
~HierarchSurrBasedLocalMinimizerHelper()
{
  // empty
}

Real HierarchSurrBasedLocalMinimizerHelper::
trust_region_factor() { return trustRegionFactor; }

void HierarchSurrBasedLocalMinimizerHelper::
trust_region_factor(Real val) { trustRegionFactor = val; }

bool HierarchSurrBasedLocalMinimizerHelper::
new_center() { return newCenterFlag; }

void HierarchSurrBasedLocalMinimizerHelper::
new_center(bool val) { newCenterFlag = val; }

const HierarchSurrBasedLocalMinimizerHelper::
Variables& vars_center() const { return varsCenter; }

void HierarchSurrBasedLocalMinimizerHelper::
vars_center(const Variables& val) { varsCenter = val; }

const Response& HierarchSurrBasedLocalMinimizerHelper::
response_star(bool return_corrected) const
{
  if (return_corrected)
    return responseStarCorrected;
  else
    return responseStarUncorrected;
}

const Response& HierarchSurrBasedLocalMinimizerHelper::
response_center(bool return_corrected) const
{
  if (return_corrected)
    return responseCenterCorrected;
  else
    return responseCenterUncorrected;
}

void HierarchSurrBasedLocalMinimizerHelper::
response_star_corrected(const Response& resp)
{
  responseStarCorrected = resp;
}

void HierarchSurrBasedLocalMinimizerHelper::
response_star_uncorrected(const Response& resp)
{
  responseStarUncorrected = resp;
}

void HierarchSurrBasedLocalMinimizerHelper::
response_center_corrected(const Response& resp)
{
  responseCenterCorrected = resp;
}

void HierarchSurrBasedLocalMinimizerHelper::
response_center_uncorrected(const Response& resp)
{
  responseCenterUncorrected = resp;
}


HierarchSurrBasedLocalMinimizer::
HierarchSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedLocalMinimizer(problem_db, model)
{
  // Verify that iteratedModel is a surrogate model so that
  // approximation-related functions are defined.
  if (iteratedModel.model_type() != "surrogate") {
    Cerr << "Error: HierarchSurrBasedLocalMinimizer::iteratedModel must be a "
         << "surrogate model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // check iteratedModel for model form hierarchy and/or discretization levels;
  if (iteratedModel.surrogate_type() != "hierarchical") {
    Cerr << "Error: HierarchSurrBasedLocalMinimizer requires a hierarchical "
         << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  approxSubProbModel = iteratedModel;

  // Instantiate the approximate sub-problem minimizer
  initialize_sub_minimizer();

  // Extract subIterator/subModel(s) from the SurrogateModel
  Model& truth_model  = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();

  // Get number of model fidelities and number of levels for each fidelity:
  numFid = iteratedModel.subordinate_models(false).size();
  numLev = std::vector<size_t>(numFid);
  for(size_t ii = 0; ii < numFid - 1; ii++) {
    size_t lf_model_form = ii;
    size_t hf_model_form = ii + 1;

    iteratedModel.surrogate_model_indices(lf_model_form);
    iteratedModel.truth_model_indices(hf_model_form);
    numLev[ii]  = truth_model.solution_levels();
  }
  // Get the number of levels for highest fidelity: (in correct state from loop
  // above)
  numLev[numFid - 1]  = truth_model.solution_levels();


  // TODO: This is specific for just multifidelity:
  trustRegions = std::vector<HierarchSurrBasedLocalMinimizerHelper>(numFid);

  for(size_t ii = 0; ii < trustRegions.size(); ii++) {
    trustRegions[ii].trust_region_factor(origTrustRegionFactor);
    trustRegions[ii].new_center(true);
  }
}


HierarchSurrBasedLocalMinimizer::~HierarchSurrBasedLocalMinimizer()
{ }


void HierarchSurrBasedLocalMinimizer::pre_run()
{
  SurrBasedLocalMinimizer::pre_run();

  // static pointer to HierarchSurrBasedLocalMinimizer instance
  mlmfInstance = this;

  // reset convergence controls in case of multiple executions
  if (convergenceFlag)
    reset();

  OutputManager& output_mgr = parallelLib.output_manager();
}


void HierarchSurrBasedLocalMinimizer::reset()
{
  SurrBasedLocalMinimizer::reset();

  for(size_t ii = 0; ii < trustRegions.size(); ii++) {
    trustRegions[ii].trust_region_factor(origTrustRegionFactor);
    trustRegions[ii].new_center(true);
  }
}


//void HierarchSurrBasedLocalMinimizer::multifidelity_opt()
//{

//  // Create arrays for variables and variable bounds
//  Variables vars_star;
//  varsCenter = iteratedModel.current_variables().copy();

//  // need copies of initial point and initial global bounds, since iteratedModel
//  // continuous vars will be reset to the TR center and iteratedModel bounds
//  // will be reset to the TR bounds
//  RealVector initial_pt;
//  copy_data(varsCenter.continuous_variables(), initial_pt);
//  copy_data(iteratedModel.continuous_lower_bounds(), globalLowerBnds);
//  copy_data(iteratedModel.continuous_upper_bounds(), globalUpperBnds);
//  std::vector<RealVector> tr_lower_bnds(numFid), tr_upper_bnds(numFid);

//  for (size_t i = 0; i < numFid; i++) {
//    tr_lower_bnds[i] = RealVector(numContinuousVars);
//    tr_upper_bnds[i] = RealVector(numContinuousVars);
//  }

//  // Create commonly-used ActiveSets
//  ActiveSet val_set, full_approx_set, full_truth_set;

//  for (size_t model_form = 0; model_form < numFid - 1; model_form++) {
//    size_t lf_model_form = model_form;
//    size_t hf_model_form = model_form + 1;

//    iteratedModel.surrogate_model_indices(lf_model_form);
//    iteratedModel.truth_model_indices(hf_model_form);

//    Model& truth_model  = iteratedModel.truth_model();
//    Model& approx_model = iteratedModel.surrogate_model();

//    responseCenterUncorrected[hf_model_form] = truth_model.current_response().copy();
//    responseStarUncorrected[hf_model_form] = responseCenterUncorrected[hf_model_form].copy();

//    responseCenterUncorrected[lf_model_form] = approx_model.current_response().copy();
//    responseStarUncorrected[lf_model_form] = responseCenterUncorrected[lf_model_form].copy();

//    responseCenterCorrected[hf_model_form] = truth_model.current_response().copy();
//    responseStarCorrected[hf_model_form] = responseCenterCorrected[hf_model_form].copy();

//    responseCenterCorrected[lf_model_form] = approx_model.current_response().copy();
//    responseStarCorrected[lf_model_form] = responseCenterCorrected[lf_model_form].copy();

//    val_set = full_approx_set = full_truth_set
//                                = responseCenterCorrected[hf_model_form].active_set();
//    int full_approx_val = 3, full_truth_val = 3;
//    val_set.request_values(1);
//    full_approx_set.request_values(full_approx_val);
//    full_truth_set.request_values(full_truth_val);

//    responseStarUncorrected[hf_model_form].active_set(val_set);
//    responseCenterUncorrected[hf_model_form].active_set(full_truth_set);

//    responseStarUncorrected[lf_model_form].active_set(val_set);
//    responseCenterUncorrected[lf_model_form].active_set(full_truth_set);

//    responseStarCorrected[hf_model_form].active_set(val_set);
//    responseCenterCorrected[hf_model_form].active_set(full_truth_set);

//    responseStarCorrected[lf_model_form].active_set(val_set);
//    responseCenterCorrected[lf_model_form].active_set(full_truth_set);

//    newCenterFlag[lf_model_form] = true;
//  }

//  while ( !convergenceFlag ) {

//    // Compute trust region bounds. Lower fidelity trust regions must be subsets
//    // of higher fidelity trust regions. Truncate the trust region bounds to
//    // enforce this.
//    tr_bounds(tr_lower_bnds, tr_upper_bnds);

//    // Compute new trust region centers:
//    for (size_t model_form = 0; model_form < numFid - 1; model_form++) {
//      size_t lf_model_form = model_form;
//      size_t hf_model_form = model_form + 1;
//    
//      // Set the trust region center and bounds
//      iteratedModel.continuous_variables(varsCenter.continuous_variables());
//      iteratedModel.continuous_lower_bounds(tr_lower_bnds[lf_model_form]);
//      iteratedModel.continuous_upper_bounds(tr_upper_bnds[lf_model_form]);

//      if (newCenterFlag[lf_model_form]) {
//        iteratedModel.surrogate_model_indices(lf_model_form);
//        iteratedModel.truth_model_indices(hf_model_form);

//        // This only evaluates the high fidelity model:
//        iteratedModel.build_approximation();

//        find_center(lf_model_form, hf_model_form);

//        if (!convergenceFlag) {
//          // ******************************************
//          // Compute additive/multiplicative correction
//          // ******************************************

//          DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();
//          delta.compute(varsCenter, responseCenterUncorrected[hf_model_form],
//                        responseCenterUncorrected[lf_model_form]);
//        }
//      }
//    }


//    // Recompute corrected center responses:
//    for (size_t model_form = 0; model_form < numFid - 1; model_form++) {
//      size_t lf_model_form = model_form;
//      size_t hf_model_form = model_form + 1;

//      // Compute responseCenterCorrected
//      Response response_center_corrected_temp = responseCenterUncorrected[lf_model_form].copy();
//      for (int ii = lf_model_form; ii < numFid - 1; ii++) {
//        size_t lf_model_form_temp = ii;
//        size_t hf_model_form_temp = ii + 1;

//        iteratedModel.surrogate_model_indices(lf_model_form_temp);
//        iteratedModel.truth_model_indices(hf_model_form_temp);

//        DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();
//        delta.apply(varsCenter, response_center_corrected_temp);
//      }
//      responseCenterCorrected[lf_model_form].update(response_center_corrected_temp);
//    }
//    // highest fidelity model doesn't need correcting:
//    responseCenterCorrected[numFid-1].update(responseCenterUncorrected[numFid-1]);






//    if (!convergenceFlag) {
//      // *********************************
//      // Optimize at (fully corrected) lowest fidelity only:
//      // *********************************
//      // This part is hard-coded for only two models currently:
//      size_t lf_model_form = 0; // Lowest
//      size_t hf_model_form = 1; // Next to lowest
//      iteratedModel.surrogate_model_indices(lf_model_form);
//      iteratedModel.truth_model_indices(hf_model_form);

//      ((HierarchSurrModel*)(iteratedModel.model_rep()))->
//        correction_mode(FULL_MODEL_FORM_CORRECTION);

//      Cout << "\n>>>>> Starting approximate optimization cycle.\n";
//      iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);
//      ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
//      approxSubProbMinimizer.run(pl_iter); // pl_iter required for hierarchical
//      Cout << "\n<<<<< Approximate optimization cycle completed.\n";
//      sbIterNum++; // full iteration performed: increment the counter

//      // Retrieve vars_star and responseStarCorrected[lf_model_form]
//      vars_star = approxSubProbMinimizer.variables_results();
//      responseStarCorrected[lf_model_form].update(approxSubProbMinimizer.response_results());


//      // ****************************
//      // Validate candidate point
//      // ****************************
//      Cout << "\n>>>>> Evaluating approximate solution with actual model.\n";
//      
//      Model& truth_model = iteratedModel.truth_model();
//      Model& approx_model = iteratedModel.surrogate_model();

//      iteratedModel.component_parallel_mode(TRUTH_MODEL);
//      truth_model.active_variables(vars_star);
//      truth_model.evaluate(val_set);

//      // Do I need to correct this response? -> yes
//      Response truth_temp = truth_model.current_response().copy();
//      Response truth_corrected_temp = truth_temp.copy();

//      for (int model_form = numFid - 2; model_form >= hf_model_form; model_form--) {
//        size_t lf_model_form_temp = model_form;
//        size_t hf_model_form_temp = model_form + 1;

//        iteratedModel.surrogate_model_indices(lf_model_form_temp);
//        iteratedModel.truth_model_indices(hf_model_form_temp);

//        DiscrepancyCorrection& delta = iteratedModel.discrepancy_correction();
//        delta.apply(vars_star, truth_corrected_temp);
//      }
//      iteratedModel.surrogate_model_indices(lf_model_form);
//      iteratedModel.truth_model_indices(hf_model_form);

//      responseStarUncorrected[hf_model_form].update(truth_temp);
//      responseStarCorrected[hf_model_form].update(truth_corrected_temp);

//      // compute the trust region ratio and update soft convergence counters
//      tr_ratio_check();

//      // If the candidate optimum (vars_star) is accepted, then update the
//      // center variables and response data.
//      if (newCenterFlag[lf_model_form]) {
//        const RealVector& c_vars_star = vars_star.continuous_variables();
//        varsCenter.continuous_variables(c_vars_star);
//        responseCenterUncorrected[hf_model_form].update(truth_temp);
//        responseCenterCorrected[hf_model_form].update(truth_corrected_temp);
//      }

//      if (!convergenceFlag) {
//        if (softConvCount >= softConvLimit)
//          convergenceFlag = 3; // soft convergence
//        // terminate SBLM if trustRegionFactor is less than its minimum value
//        else if (trustRegionFactor[lf_model_form] < minTrustRegionFactor)
//          convergenceFlag = 1;
//        // terminate SBLM if the maximum number of iterations has been reached
//        else if (sbIterNum >= maxIterations)
//          convergenceFlag = 2;
//      }

//    } // end of approx opt cycle

//  } // end of outer while() loop
//}



void HierarchSurrBasedLocalMinimizer::
find_center(size_t lf_model_form, size_t hf_model_form)
{
  bool found = false;

  Model& truth_model = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();

  responseCenterUncorrected[hf_model_form].update(truth_model.current_response());
  found = true;

  if (!found) {
    Cout << "\n>>>>> Evaluating actual model at trust region center.\n";

    // since we're bypassing iteratedModel, iteratedModel.serve()
    // must be in the correct server mode.
    iteratedModel.component_parallel_mode(TRUTH_MODEL);
    truth_model.continuous_variables(varsCenter.continuous_variables());
    truth_model.evaluate(responseCenterUncorrected[hf_model_form].active_set());

    responseCenterUncorrected[hf_model_form].update(truth_model.current_response());
  }


  size_t numFid = responseCenterUncorrected.size();        
  if (hf_model_form == numFid - 1) { // I need to remove this if statement
    hard_convergence_check(responseCenterUncorrected[hf_model_form], varsCenter.continuous_variables(), globalLowerBnds, globalUpperBnds);
  }

  if (!convergenceFlag) {
    found = false;

    // search for fn vals, grads, and Hessians separately since they may
    // be different fn evaluations
    ActiveSet search_set = responseCenterUncorrected[lf_model_form].active_set(); // copy
    search_set.request_values(1);
    const Variables& search_vars = iteratedModel.current_variables();
    const String& search_id = iteratedModel.surrogate_model().interface_id();
    PRPCacheHIter cache_it
      = lookup_by_val(data_pairs, search_id, search_vars, search_set);
    if (cache_it != data_pairs.get<hashed>().end()) {
      responseCenterUncorrected[lf_model_form].function_values(
        cache_it->response().function_values());
      search_set.request_values(2);
      cache_it
        = lookup_by_val(data_pairs, search_id, search_vars, search_set);
      if (cache_it != data_pairs.get<hashed>().end()) {
        responseCenterUncorrected[lf_model_form].function_gradients(
          cache_it->response().function_gradients());
        found = true;
      }
    }
    
    if (!found) {
      Cout <<"\n>>>>> Evaluating approximation at trust region center.\n";
      iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
      iteratedModel.evaluate(responseCenterUncorrected[lf_model_form].active_set());
      responseCenterUncorrected[lf_model_form].update(iteratedModel.current_response());
    }
  }
}


/** The hard convergence check computes the gradient of the merit
    function at the trust region center, performs a projection for
    active bound constraints (removing any gradient component directed
    into an active bound), and signals convergence if the 2-norm of
    this projected gradient is less than convergenceTol. */
void HierarchSurrBasedLocalMinimizer::
hard_convergence_check(const Response& response_truth,
                       const RealVector& c_vars,
                       const RealVector& lower_bnds,
                       const RealVector& upper_bnds)
{
  const RealVector& fns_truth   = response_truth.function_values();
  const RealMatrix& grads_truth = response_truth.function_gradients();

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


/** Assess acceptance of SBLM iterate (trust region ratio or filter)
    and compute soft convergence metrics (number of consecutive
    failures, min trust region size, etc.) to assess whether the
    convergence rate has decreased to a point where the process should
    be terminated (diminishing returns). */
void HierarchSurrBasedLocalMinimizer::tr_ratio_check()
{
  // This is hard-coded for only two models:
  size_t lf_model_form = 0; // Lowest
  size_t hf_model_form = 1; // Next to lowest

  const RealVector& fns_center_truth
    = responseCenterCorrected[hf_model_form].function_values();
  const RealVector& fns_star_truth = responseStarCorrected[hf_model_form].function_values();
  const RealVector& fns_center_approx = responseCenterCorrected[lf_model_form].function_values();
  const RealVector& fns_star_approx = responseStarCorrected[lf_model_form].function_values();

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
    newCenterFlag[lf_model_form] = true;

    // Update the trust region size depending on the accuracy of the approximate
    // model. Note: If eta_1 < tr_ratio < eta_2, trustRegionFactor does not
    // change where eta_1 = trRatioContractValue and eta_2 = trRatioExpandValue
    // Recommended values from Conn/Gould/Toint are: eta_1 = 0.05, eta_2 = 0.90
    // For SBLM, the following are working better:   eta_1 = 0.25, eta_2 = 0.75
    // More experimentation is needed.
    Cout << "\n<<<<< Trust Region Ratio = " << tr_ratio << ":\n<<<<< ";
    if (tr_ratio <= trRatioContractValue) { // accept optimum, shrink TR
      trustRegionFactor[lf_model_form] *= gammaContract;
      Cout << "Marginal Accuracy, ACCEPT Step, REDUCE Trust Region Size\n\n";
    } else if (std::fabs(1.-tr_ratio) <= 1.-trRatioExpandValue) { //accept & expand
      // for trRatioExpandValue = 0.75, expand if 0.75 <= tr_ratio <= 1.25
      // This new logic avoids increasing the TR size when a good step is found
      // but the surrogates are inaccurate (e.g., tr_ratio = 2).
      trustRegionFactor[lf_model_form] *= gammaExpand;
      Cout << "Excellent Accuracy, ACCEPT Step, INCREASE Trust Region Size"
           << "\n\n";
    } else // accept optimum, retain current TR
      Cout <<"Satisfactory Accuracy, ACCEPT Step, RETAIN Trust Region Size\n\n";
  } else {
    // If the step is rejected, then retain the current design variables
    // and shrink the TR size.
    newCenterFlag[lf_model_form] = false;
    trustRegionFactor[lf_model_form] *= gammaContract;
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


void HierarchSurrBasedLocalMinimizer::update_trust_region()
{
}

void HierarchSurrBasedLocalMinimizer::verify()
{
}

void HierarchSurrBasedLocalMinimizer::minimize()
{
}

void HierarchSurrBasedLocalMinimizer::build()
{
}


} // namespace Dakota
