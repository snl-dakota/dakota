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


void SurrBasedLocalMinimizer::reset()
{
  convergenceFlag = softConvCount = sbIterNum = 0;
}

} // namespace Dakota
