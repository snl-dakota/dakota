/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedLocalMinimizer
//- Description: Implementation code for the SurrBasedLocalMinimizer class
//- Owner:       Mike Eldred, Sandia National Laboratories
//- Checked by:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "SurrBasedLocalMinimizer.hpp"
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

static const char rcsId[]="@(#) $Id: SurrBasedLocalMinimizer.cpp 7031 2010-10-22 16:23:52Z mseldre $";


namespace Dakota {
  extern PRPCache data_pairs; // global container

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL 2

SurrBasedLocalMinimizer::
SurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedMinimizer(problem_db, model), 
  origTrustRegionFactor(
    probDescDB.get_real("method.sbl.trust_region.initial_size")),
  trustRegionFactor(origTrustRegionFactor),
  minTrustRegionFactor(
    probDescDB.get_real("method.sbl.trust_region.minimum_size")),
  trRatioContractValue(
    probDescDB.get_real("method.sbl.trust_region.contract_threshold")),
  trRatioExpandValue(
    probDescDB.get_real("method.sbl.trust_region.expand_threshold")),
  gammaContract(
    probDescDB.get_real("method.sbl.trust_region.contraction_factor")),
  gammaExpand(probDescDB.get_real("method.sbl.trust_region.expansion_factor")),
  convergenceFlag(0), softConvCount(0),
  softConvLimit(probDescDB.get_ushort("method.soft_convergence_limit")),
  correctionType(probDescDB.get_short("model.surrogate.correction_type")),
  newCenterFlag(true)
{
  // Verify that iteratedModel is a surrogate model so that
  // approximation-related functions are defined.
  if (iteratedModel.model_type() != "surrogate") {
    Cerr << "Error: SurrBasedLocalMinimizer::iteratedModel must be a "
	 << "surrogate model." << std::endl;
    abort_handler(-1);
  }

  bestVariablesArray.push_back(
    iteratedModel.truth_model().current_variables().copy());

  // Note: There are checks in ProblemDescDB.cpp to verify that the trust region
  // user-defined values (e.g., gammaExpand, trRationExpandValue, etc.) are 
  // set correctly (i.e., trust region size is not zero, etc.)

  Model& truth_model  = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();
  // Initialize response results objects (approx/truth and center/star).  These
  // must be deep copies to avoid representation sharing: initialize with copy()
  // and then use update() within the main loop.
  responseCenterApprox       = approx_model.current_response().copy();
  responseStarApprox         = responseCenterApprox.copy();
  responseCenterTruth.first  = truth_model.evaluation_id();
  responseCenterTruth.second = truth_model.current_response().copy();
  responseStarTruth.first    = responseCenterTruth.first;
  responseStarTruth.second   = responseCenterTruth.second.copy();

  // Set method-specific default for softConvLimit
  if (!softConvLimit)
    softConvLimit = 5;
}


SurrBasedLocalMinimizer::~SurrBasedLocalMinimizer()
{ }


void SurrBasedLocalMinimizer::pre_run()
{
  // Create arrays for variables and variable bounds
  varsCenter = iteratedModel.current_variables().copy();
  // need copies of initial point and initial global bounds, since iteratedModel
  // continuous vars will be reset to the TR center and iteratedModel bounds
  // will be reset to the TR bounds
  RealVector initial_pt;
  copy_data(varsCenter.continuous_variables(), initial_pt);
  copy_data(iteratedModel.continuous_lower_bounds(), globalLowerBnds);
  copy_data(iteratedModel.continuous_upper_bounds(), globalUpperBnds);

  // Create commonly-used ActiveSets
  valSet = fullApproxSet = fullTruthSet
    = responseCenterTruth.second.active_set();
  int full_approx_val = 1, full_truth_val = 1;
  if (approxGradientFlag) full_approx_val += 2;
  if (approxHessianFlag)  full_approx_val += 4;
  if (truthGradientFlag)  full_truth_val  += 2;
  if (truthHessianFlag)   full_truth_val  += 4;
  valSet.request_values(1);
  fullApproxSet.request_values(full_approx_val);
  fullTruthSet.request_values(full_truth_val);
  // Set ActiveSets within the response copies
  responseStarApprox.active_set(valSet);
  responseStarTruth.second.active_set(valSet);
  responseCenterApprox.active_set(fullApproxSet);
  responseCenterTruth.second.active_set(fullTruthSet);
}


/** Trust region-based strategy to perform surrogate-based optimization
    in subregions (trust regions) of the parameter space.  The minimizer 
    operates on approximations in lieu of the more expensive 
    simulation-based response functions.  The size of the trust region 
    is adapted according to the agreement between the approximations and 
    the true response functions. */
void SurrBasedLocalMinimizer::core_run()
{
  // TO DO: consider *SurrModel::initialize_mapping() --> initial surr build
  // would simplify detection of auto build and remove some checks in evaluate()
  // --> longer term, lower priority: defer for now

  while (!convergenceFlag) {

    // Compute trust region bounds.  If the trust region extends outside
    // the global bounds, then truncate to the global bounds.
    update_trust_region();

    // Build new approximations and compute corrections for use within
    // approxSubProbMinimizer.run() (unless previous build can be reused)
    // > Build the approximation
    // > Evaluate/retrieve responseCenterTruth
    // > Perform hard convergence check
    if (globalApproxFlag || newCenterFlag) build();
    else Cout << "\n>>>>> Reusing previous approximation.\n";

    if (!convergenceFlag) { // check for hard convergence within build()
      minimize(); // run approxSubProbMinimizer and update responseStarApprox
      verify();   // evaluate responseStarTruth and update trust region
    }
  }
}


void SurrBasedLocalMinimizer::post_run(std::ostream& s)
{
  // SBLM is complete: write out the convergence condition and final results
  // from the center point of the last trust region.
  Cout << "\nSurrogate-Based Optimization Complete - ";
  if ( convergenceFlag == 1 )
    Cout << "Minimum Trust Region Bounds Reached\n";
  else if ( convergenceFlag == 2 )
    Cout << "Exceeded Maximum Number of Iterations\n";
  else if ( convergenceFlag == 3 )  
    Cout << "Soft Convergence Tolerance Reached\nProgress Between "
	 << softConvLimit <<" Successive Iterations <= Convergence Tolerance\n";
  else if ( convergenceFlag == 4 )
    Cout << "Hard Convergence Reached\nNorm of Projected Lagrangian Gradient "
	 << "<= Convergence Tolerance\n";
  else {
    Cout << "\nError: bad convergenceFlag in SurrBasedLocalMinimizer."
	 << std::endl;
    abort_handler(-1);
  }
  Cout << "Total Number of Iterations = " << sbIterNum << '\n';

  bestVariablesArray.front().continuous_variables(
    varsCenter.continuous_variables());
  bestResponseArray.front().function_values(
    responseCenterTruth.second.function_values());

  Minimizer::post_run(s);
}


void SurrBasedLocalMinimizer::reset()
{
  convergenceFlag = softConvCount = sbIterNum = 0;

  newCenterFlag = true;

  trustRegionFactor = origTrustRegionFactor;
}

} // namespace Dakota
