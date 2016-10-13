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
#include "SurrBasedLevelData.hpp"
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

SurrBasedLocalMinimizer::
SurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedMinimizer(problem_db, model), 
  origTrustRegionFactor(
    probDescDB.get_real("method.sbl.trust_region.initial_size")),
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
  correctionType(probDescDB.get_short("model.surrogate.correction_type"))
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

  // Set method-specific default for softConvLimit
  if (!softConvLimit)
    softConvLimit = 5;
}


SurrBasedLocalMinimizer::~SurrBasedLocalMinimizer()
{ }


void SurrBasedLocalMinimizer::initialize_sub_minimizer()
{
  const String& approx_method_ptr
    = probDescDB.get_string("method.sub_method_pointer");
  const String& approx_method_name
    = probDescDB.get_string("method.sub_method_name");
  if (!approx_method_ptr.empty()) {
    // Approach 1: method spec support for approxSubProbMinimizer
    const String& model_ptr = probDescDB.get_string("method.model_pointer");
    // NOTE: set_db_list_nodes is not used for instantiating a Model for the
    // approxSubProbMinimizer.  Rather, the iteratedModel passed into the SBLM
    // iterator, or a recasting of it, is used.  Thus, the SBLM model_pointer
    // is relevant and any sub-method model_pointer spec is ignored.  
    size_t method_index = probDescDB.get_db_method_node(); // for restoration
    probDescDB.set_db_method_node(approx_method_ptr); // set method only
    approxSubProbMinimizer = probDescDB.get_iterator(approxSubProbModel);
    // suppress DB ctor default and don't output summary info
    approxSubProbMinimizer.summary_output(false);
    // verify approx method's modelPointer is empty or consistent
    const String& am_model_ptr = probDescDB.get_string("method.model_pointer");
    if (!am_model_ptr.empty() && am_model_ptr != model_ptr)
      Cerr << "Warning: SBLM approx_method_pointer specification includes an\n"
	   << "         inconsistent model_pointer that will be ignored."
	   << std::endl;
    // setting SBLM constraintTol is tricky since the DAKOTA default of 0. is a
    // dummy -> NPSOL, DOT, and CONMIN use their internal defaults in this case.
    // It would be preferable to support tolerance rtn in NPSOL/DOT/CONMIN & use
    // constraintTol = approxSubProbMinimizer.constraint_tolerance();
    if (constraintTol <= 0.) { // not specified in SBLM method spec
      Real aspm_constr_tol = probDescDB.get_real("method.constraint_tolerance");
      if (aspm_constr_tol > 0.) // sub-method has spec: enforce SBLM consistency
	constraintTol = aspm_constr_tol;
      else { // neither has spec: assign default and enforce consistency
	constraintTol = 1.e-4; // compromise value among NPSOL/DOT/CONMIN
	Minimizer* aspm = (Minimizer*)approxSubProbMinimizer.iterator_rep();
	aspm->constraint_tolerance(constraintTol);
      }
    }
    else { // SBLM method spec takes precedence over approxSubProbMinimizer spec
      Minimizer* aspm = (Minimizer*)approxSubProbMinimizer.iterator_rep();
      aspm->constraint_tolerance(constraintTol);
    }
    probDescDB.set_db_method_node(method_index); // restore method only
  }
  else if (!approx_method_name.empty()) {
    // Approach 2: instantiate on-the-fly w/o method spec support
    approxSubProbMinimizer
      = probDescDB.get_iterator(approx_method_name, approxSubProbModel);
    if (constraintTol <= 0.) // not specified in SBLM method spec
      constraintTol = 1.e-4; // compromise value among NPSOL/DOT/CONMIN
    Minimizer* aspm = (Minimizer*)approxSubProbMinimizer.iterator_rep();
    aspm->constraint_tolerance(constraintTol);
  }
}


void SurrBasedLocalMinimizer::pre_run()
{
  // reset convergence controls in case of multiple executions
  if (convergenceFlag)
    reset();

  // need copies of initial point and initial global bounds, since iteratedModel
  // continuous vars will be reset to the TR center and iteratedModel bounds
  // will be reset to the TR bounds
  copy_data(iteratedModel.continuous_variables(),    initialPoint);
  copy_data(iteratedModel.continuous_lower_bounds(), globalLowerBnds);
  copy_data(iteratedModel.continuous_upper_bounds(), globalUpperBnds);

  // grad/Hess flags are set in derived ctors prior to calling base pre_run()

  // TO DO: verify utility of these for HSBLM...
  // Create commonly-used ActiveSets
  valSet = fullApproxSet = fullTruthSet
    = iteratedModel.current_response().active_set();
  int full_approx_val = 1, full_truth_val = 1;
  if (approxGradientFlag) full_approx_val += 2;
  if (approxHessianFlag)  full_approx_val += 4;
  if (truthGradientFlag)  full_truth_val  += 2;
  if (truthHessianFlag)   full_truth_val  += 4;
  valSet.request_values(1);
  fullApproxSet.request_values(full_approx_val);
  fullTruthSet.request_values(full_truth_val);
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
    build();

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

  Minimizer::post_run(s);
}


void SurrBasedLocalMinimizer::
update_trust_region_data(SurrBasedLevelData& tr_data,
			 const RealVector& parent_l_bnds,
			 const RealVector& parent_u_bnds)
{
  // Compute the trust region bounds.  Center is only updated if it violates
  // a bound --> TR may be asymmetric: preferable in terms of continuity of
  // iteration / warm-start efficiency / etc.  Recentering could also cause
  // problems for export/restart workflows (starting point for new cycle moves
  // from final soln from prev cycle).
  size_t i;
  bool cv_truncation = false, tr_lower_truncation = false,
    tr_upper_truncation = false;
  for (i=0; i<numContinuousVars; i++) {
    // verify that varsCenter is within global bounds
    Real cv_center = tr_data.c_var_center(i);
    if ( cv_center > parent_u_bnds[i] ) {
      cv_center = parent_u_bnds[i]; cv_truncation = true;
      tr_data.c_var_center(cv_center, i);
    }
    if ( cv_center < parent_l_bnds[i] ) {
      cv_center = parent_l_bnds[i]; cv_truncation = true;
      tr_data.c_var_center(cv_center, i);
    }
    // compute 1-sided trust region offset
    Real tr_offset = tr_data.trust_region_factor() / 2. * 
      ( parent_u_bnds[i] - parent_l_bnds[i] );
    Real up_bound = cv_center + tr_offset, lo_bound = cv_center - tr_offset;
    if ( up_bound <= parent_u_bnds[i] )
      tr_data.tr_upper_bound(up_bound, i);
    else {
      tr_data.tr_upper_bound(parent_u_bnds[i], i);
      tr_upper_truncation = true;
    }
    if ( lo_bound >= parent_l_bnds[i] )
      tr_data.tr_lower_bound(lo_bound, i);
    else {
      tr_data.tr_lower_bound(parent_l_bnds[i], i);
      tr_lower_truncation = true;
    }
  }
  if (cv_truncation)
    tr_data.new_center(true);

  // a flag for global approximations defining the availability of the
  // current iterate in the DOE/DACE evaluations: CCD/BB DOE evaluates the
  // center of the sampled region, whereas LHS/OA/QMC/CVT DACE does not.
  //daceCenterPtFlag
  //  = (daceCenterEvalFlag && !tr_lower_truncation && !tr_upper_truncation);

  const RealVector&     cv_center = tr_data.c_vars_center();
  const RealVector& tr_lower_bnds = tr_data.tr_lower_bounds();
  const RealVector& tr_upper_bnds = tr_data.tr_upper_bounds();
  // Set the trust region center and bounds for approxSubProbOptimizer
  approxSubProbModel.continuous_variables(cv_center);
  approxSubProbModel.continuous_lower_bounds(tr_lower_bnds);
  approxSubProbModel.continuous_upper_bounds(tr_upper_bnds);

  // Output the trust region bounds
  size_t wpp9 = write_precision+9;
  Cout << "\n**************************************************************"
       << "************\nBegin SBLM Iteration Number " << sbIterNum+1
       << "\n\nCurrent Trust Region for Truth model form "
       << tr_data.truth_model_form() << " level " << tr_data.truth_model_level()
       << ", Approx. model form " << tr_data.approx_model_form() << " level "
       << tr_data.approx_model_level() << "\n                 ";
  if (tr_lower_truncation) Cout << std::setw(wpp9) << "Lower (truncated)";
  else                     Cout << std::setw(wpp9) << "Lower";
  if (cv_truncation)       Cout << std::setw(wpp9) << "Center (truncated)";
  else                     Cout << std::setw(wpp9) << "Center";
  if (tr_upper_truncation) Cout << std::setw(wpp9) << "Upper (truncated)";
  else                     Cout << std::setw(wpp9) << "Upper";
  Cout << '\n';
  StringMultiArrayConstView c_vars_labels
    = iteratedModel.continuous_variable_labels();
  for (i=0; i<numContinuousVars; i++)
    Cout << std::setw(16) << c_vars_labels[i] << ':' << std::setw(wpp9)
	 << tr_lower_bnds[i] << std::setw(wpp9) << cv_center[i]
	 << std::setw(wpp9) << tr_upper_bnds << '\n';
  Cout << "****************************************************************"
       << "**********\n";
}


/* TO DO:
  tr_ratio_check
  find_center
  ...
*/

} // namespace Dakota
