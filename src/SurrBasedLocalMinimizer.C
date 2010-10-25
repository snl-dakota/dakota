/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedLocalMinimizer
//- Description: Implementation code for the SurrBasedLocalMinimizer class
//- Owner:       Mike Eldred, Sandia National Laboratories
//- Checked by:

#include "system_defs.h"
#include "data_io.h"
#include "SurrBasedLocalMinimizer.H"
#include "ProblemDescDB.H"
#include "ParallelLibrary.H"
#include "ParamResponsePair.H"
#include "PRPMultiIndex.H"
#include "DakotaGraphics.H"
#include "RecastModel.H"
#ifdef DAKOTA_NPSOL
#include "NPSOLOptimizer.H"
#endif // DAKOTA_NPSOL

//#define DEBUG

static const char rcsId[]="@(#) $Id: SurrBasedLocalMinimizer.C 7031 2010-10-22 16:23:52Z mseldre $";


namespace Dakota {

// define special values for componentParallelMode
//#define SURROGATE_MODEL 1
#define TRUTH_MODEL 2

// initialization of statics
SurrBasedLocalMinimizer* SurrBasedLocalMinimizer::sblmInstance(NULL);

SurrBasedLocalMinimizer::SurrBasedLocalMinimizer(Model& model):
  SurrBasedMinimizer(model), 
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
  approxSubProbObj(probDescDB.get_short("method.sbl.subproblem_objective")),
  approxSubProbCon(probDescDB.get_short("method.sbl.subproblem_constraints")),
  meritFnType(probDescDB.get_short("method.sbl.merit_function")),
  acceptLogic(probDescDB.get_short("method.sbl.acceptance_logic")),
  trConstraintRelax(probDescDB.get_short("method.sbl.constraint_relax")),
  //approxSubProbObj(ORIGINAL_PRIMARY),approxSubProbCon(ORIGINAL_CONSTRAINTS),
  //meritFnType(AUGMENTED_LAGRANGIAN_MERIT), acceptLogic(FILTER),
  penaltyIterOffset(-200), convergenceFlag(0), softConvCount(0),
  softConvLimit(probDescDB.get_int("method.sbl.soft_convergence_limit")),
  multiLayerBypassFlag(false)
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

  // Note: There are checks in ProblemDescDB.C to verify that the trust region
  // user-defined values (e.g., gammaExpand, trRationExpandValue, etc.) are 
  // set correctly (i.e., trust region size is not zero, etc.)

  // If the "direct surrogate" formulation is used, then approxSubProbMinimizer
  // is interfaced w/ iteratedModel directly. Otherwise, RecastModel indirection
  // is required to allow recasting of the approximate subproblem.
  if ( ( approxSubProbCon == ORIGINAL_CONSTRAINTS || !numNonlinearConstraints )
    && ( approxSubProbObj == ORIGINAL_PRIMARY || ( optimizationFlag &&
	 approxSubProbObj == SINGLE_OBJECTIVE && numUserPrimaryFns == 1 ) ) ) {
    // Note: for single-objective optimizers (i.e., not MOGA) with
    // approxSubProbObj==ORIGINAL_PRIMARY and numUserPrimaryFns>1, this forces
    // a recast to occur within the approxSubProbMinimizer instantiation.
    // TO DO: consider forcing the necessary recast here.  Also consider using
    // different defaults for approxSubProbObj: SINGLE_OBJECTIVE for opt.,
    // ORIGINAL_PRIMARY for NLS.
    recastSubProb = false;
    approxSubProbModel = iteratedModel; // no recasting: shared representation
  }
  else { // subproblem must be recast
    recastSubProb = true;
    size_t i, num_recast_primary = (approxSubProbObj == ORIGINAL_PRIMARY) ?
      numUserPrimaryFns : 1;
    size_t num_recast_secondary  = (approxSubProbCon == NO_CONSTRAINTS)   ?
      0 : numNonlinearConstraints;
    Sizet2DArray recast_vars_map, recast_primary_resp_map(num_recast_primary),
      recast_secondary_resp_map(num_recast_secondary);
    BoolDequeArray nonlinear_resp_map(num_recast_primary+num_recast_secondary);
    if (approxSubProbObj == ORIGINAL_PRIMARY) {
      for (i=0; i<num_recast_primary; i++) {
	recast_primary_resp_map[i].resize(1);
	recast_primary_resp_map[i][0] = i;
	nonlinear_resp_map[i].resize(1);
	nonlinear_resp_map[i][0] = false;
      }
    }
    else if (approxSubProbObj == SINGLE_OBJECTIVE) {
      recast_primary_resp_map[0].resize(numUserPrimaryFns);
      nonlinear_resp_map[0].resize(numUserPrimaryFns);
      for (i=0; i<numUserPrimaryFns; i++) {
	recast_primary_resp_map[0][i] = i;
	nonlinear_resp_map[0][i] = (optimizationFlag) ? false : true;
      }
    }
    else { // LAGRANGIAN_OBJECTIVE or AUGMENTED_LAGRANGIAN_OBJECTIVE
      recast_primary_resp_map[0].resize(numFunctions);
      nonlinear_resp_map[0].resize(numFunctions);
      for (i=0; i<numFunctions; i++) {
	recast_primary_resp_map[0][i] = i;
	nonlinear_resp_map[0][i]
	  = (approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE &&
	     i >= numUserPrimaryFns) ? true : false;
      }
    }
    if (approxSubProbCon != NO_CONSTRAINTS) {
      for (i=0; i<num_recast_secondary; i++) {
	recast_secondary_resp_map[i].resize(1);
	recast_secondary_resp_map[i][0] = i + numUserPrimaryFns;
	nonlinear_resp_map[i+num_recast_primary].resize(1);
	nonlinear_resp_map[i+num_recast_primary][0] = false;
      }
    }
    // iteratedModel becomes the sub-model of a RecastModel:
    size_t recast_offset
      = (approxSubProbCon == NO_CONSTRAINTS) ? 0 : numNonlinearIneqConstraints;
    approxSubProbModel.assign_rep(new RecastModel(iteratedModel,
      recast_vars_map, false, NULL, NULL, recast_primary_resp_map,
      recast_secondary_resp_map, recast_offset, nonlinear_resp_map,
      approx_subprob_objective_eval, approx_subprob_constraint_eval), false);
  }

  // Instantiate the approximate sub-problem minimizer
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
      Cerr << "Warning: SBLM approx_method_pointer specification include an\n"
	   << "         inconsistent model_pointer that will be ignored."
	   << std::endl;
    // setting SBLM constraintTol is tricky since the DAKOTA default of 0. is a
    // dummy -> NPSOL, DOT, and CONMIN use their internal defaults in this case.
    // It would be preferable to support tolerance rtn in NPSOL/DOT/CONMIN & use
    // constraintTol = approxSubProbMinimizer.constraint_tolerance();
    if (constraintTol <= 0.) { // not specified in SBLM method spec
      Real aspo_constr_tol = probDescDB.get_real("method.constraint_tolerance");
      constraintTol = (aspo_constr_tol > 0.) ?
	aspo_constr_tol : // sub-method has spec: enforce consistency
	1.e-4;            // compromise value among NPSOL/DOT/CONMIN
    }
    probDescDB.set_db_method_node(method_index); // restore method only
  }
  else if (!approx_method_name.empty()) {
    // Approach 2: instantiate on-the-fly w/o method spec support
    approxSubProbMinimizer = Iterator(approx_method_name, approxSubProbModel);
    if (constraintTol <= 0.) // not specified in SBLM method spec
      constraintTol = 1.e-4; // compromise value among NPSOL/DOT/CONMIN
  }

  // Allocate comms in approxSubProbModel/iteratedModel for parallel SBLM.
  // For DataFitSurrModel, concurrency is from daceIterator evals (global) or
  // numerical derivs (local/multipt) on actualModel.  For HierarchSurrModel,
  // concurrency is from approxSubProbMinimizer on lowFidInterface.
  approxSubProbModel.init_communicators(
    approxSubProbMinimizer.maximum_concurrency());

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

  Model& truth_model  = iteratedModel.truth_model();
  Model& approx_model = iteratedModel.surrogate_model();

  // Initialize response results objects (approx/truth and center/star).  These
  // must be deep copies to avoid representation sharing: initialize with copy()
  // and then use update() within the main loop.
  responseCenterApprox = approx_model.current_response().copy();
  responseStarApprox   = responseCenterApprox.copy();
  responseCenterTruth  = truth_model.current_response().copy();
  responseStarTruth    = responseCenterTruth.copy();

  // Initialize method/interface dependent settings
  const String& approx_type = probDescDB.get_string("model.surrogate.type");
  const String& correction_type
    = probDescDB.get_string("model.surrogate.correction_type");
  short correction_order
    = probDescDB.get_short("model.surrogate.correction_order");
  useDerivsFlag = probDescDB.get_bool("model.surrogate.derivative_usage");
  correctionFlag     = (!correction_type.empty());
  globalApproxFlag   = (approx_type.begins("global_"));
  multiptApproxFlag  = (approx_type.begins("multipoint_"));
  localApproxFlag    = (approx_type.begins("local_"));
  hierarchApproxFlag = (approx_type == "hierarchical");
  truthGradientFlag  = ( localApproxFlag || multiptApproxFlag || 
			 ( globalApproxFlag && useDerivsFlag ) ||
			 ( correctionFlag && correction_order >= 1 ) ||
			 meritFnType      == LAGRANGIAN_MERIT ||
			 approxSubProbObj == LAGRANGIAN_OBJECTIVE );
  approxGradientFlag = ( ( correctionFlag && correction_order >= 1 ) ||
			 approxSubProbCon == LINEARIZED_CONSTRAINTS );
  truthHessianFlag
    = ( ( localApproxFlag && truth_model.hessian_type() != "none" ) ||
	( correctionFlag  && correction_order == 2 ) );
  approxHessianFlag = ( correctionFlag && correction_order == 2 );

  // Sanity check on derivative specifications for first- and second-order SBLM.
  if ( truthGradientFlag && truth_model.gradient_type() == "none" ) {
    Cerr << "\nError: a gradient calculation method must be specified for the "
	 << "truth model.\n" << std::endl;
    abort_handler(-1);
  }
  if ( approxGradientFlag && approx_model.gradient_type() == "none" ) {
    Cerr << "\nError: a gradient calculation method must be specified for the "
	 << "surrogate model.\n" << std::endl;
    abort_handler(-1);
  }
  if ( truthHessianFlag && truth_model.hessian_type() == "none" ) {
    Cerr << "\nError: a Hessian calculation method must be specified for the "
	 << "truth model.\n" << std::endl;
    abort_handler(-1);
  }
  if ( approxHessianFlag && approx_model.hessian_type() == "none" ) {
    Cerr << "\nError: a Hessian calculation method must be specified for the "
	 << "surrogate model.\n" << std::endl;
    abort_handler(-1);
  }

  // initialize Lagrange multipliers
  size_t num_multipliers = numNonlinearEqConstraints;
  for (size_t i=0; i<numNonlinearIneqConstraints; i++) {
    if (origNonlinIneqLowerBnds[i] > -bigRealBoundSize) // g has a lower bound
      num_multipliers++;
    if (origNonlinIneqUpperBnds[i] <  bigRealBoundSize) // g has an upper bound
      num_multipliers++;
  }
  if (truthGradientFlag || meritFnType == LAGRANGIAN_MERIT ||
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
#ifndef DAKOTA_NPSOL
  if (trConstraintRelax > NO_RELAX) {
    Cerr << "Error: this executable not configured with NPSOL.\n       "
	 << "SurrBasedLocalMinimizer cannot perform constraint relaxation."
	 << std::endl;
    abort_handler(-1);
  }
#endif

  // Set the minimum trust region size.  For kriging, the minimum trust region
  // must be set to O(10^-2 10^-3).  Otherwise, the correlation matrix becomes
  // ill-conditioned and the maximum likelihood operations crash with floating
  // point errors.
  minTrustRegionFactor = 
    (approx_type == "global_kriging") ? 1.e-3 : minTrustRegionFactor;
  if (trustRegionFactor < minTrustRegionFactor)
    trustRegionFactor = minTrustRegionFactor;
  //trustRegionOffset.reshape(numContinuousVars);
}


SurrBasedLocalMinimizer::~SurrBasedLocalMinimizer()
{
  // Virtual destructor handles referenceCount at Strategy level.

  // free communicators for approxSubProbModel/iteratedModel
  approxSubProbModel.free_communicators(
    approxSubProbMinimizer.maximum_concurrency());
}


/** Trust region-based strategy to perform surrogate-based optimization
    in subregions (trust regions) of the parameter space.  The minimizer
    operates on approximations in lieu of the more expensive simulation-based
    response functions.  The size of the trust region is varied according to
    the goodness of the agreement between the approximations and the true
    response functions. */
void SurrBasedLocalMinimizer::minimize_surrogates()
{
  // static pointer to SurrBasedLocalMinimizer instance
  sblmInstance = this;
  // reset convergence controls in case of multiple executions
  if (convergenceFlag)
    reset();

  // Extract subIterator/subModel(s) from the SurrogateModel
  Model&    truth_model   = iteratedModel.truth_model();
  Model&    approx_model  = iteratedModel.surrogate_model();
  Iterator& dace_iterator = iteratedModel.subordinate_iterator();

  // Update DACE settings for global approximations.  Check that dace_iterator
  // is defined (a dace_iterator specification is not required when the data
  // samples are read in from a file rather than obtained from sampling).
  bool dace_center_eval_flag = false;
  if (globalApproxFlag && !dace_iterator.is_null()) {
    // With correction approaches, the responses specification must provide
    // support for evaluating derivative info.  However, this data is usually
    // not needed for every DACE evaluation in the global surrogate build
    // (exception: "use_derivatives" spec for surrogate builds using derivative
    // data).  Therefore, SBLM overrides the normal DACE behavior of evaluating
    // the full data set at every point in the experimental design.
    short asv_val = (useDerivsFlag) ? 3 : 1;
    ActiveSet dace_set = truth_model.current_response().active_set(); // copy
    dace_set.request_values(asv_val);
    dace_iterator.active_set(dace_set);

    // Extract info on the sampling method type
    const String& sampling_type = dace_iterator.sampling_scheme();
    if (sampling_type == "box_behnken" || sampling_type == "central_composite")
      dace_center_eval_flag = true;
  }

  // Create arrays for variables and variable bounds
  Variables vars_star;
  varsCenter = iteratedModel.current_variables().copy();
  // need copies of initial point and initial global bounds, since iteratedModel
  // continuous vars will be reset to the TR center and iteratedModel bounds
  // will be reset to the TR bounds
  RealVector initial_pt, global_lower_bnds, global_upper_bnds;
  copy_data(varsCenter.continuous_variables(), initial_pt);
  copy_data(iteratedModel.continuous_lower_bounds(), global_lower_bnds);
  copy_data(iteratedModel.continuous_upper_bounds(), global_upper_bnds);
  RealVector tr_lower_bnds(numContinuousVars), tr_upper_bnds(numContinuousVars);

  // Create commonly-used ActiveSets
  ActiveSet val_set, full_approx_set, full_truth_set;
  val_set = full_approx_set = full_truth_set = responseCenterTruth.active_set();
  int full_approx_val = 1, full_truth_val = 1;
  if (approxGradientFlag) full_approx_val += 2;
  if (approxHessianFlag)  full_approx_val += 4;
  if (truthGradientFlag)  full_truth_val  += 2;
  if (truthHessianFlag)   full_truth_val  += 4;
  val_set.request_values(1);
  full_approx_set.request_values(full_approx_val);
  full_truth_set.request_values(full_truth_val);
  // Set ActiveSets within the response copies
  responseStarApprox.active_set(val_set);
  responseStarTruth.active_set(val_set);
  responseCenterApprox.active_set(full_approx_set);
  responseCenterTruth.active_set(full_truth_set);

  newCenterFlag = true;
  extern Graphics dakota_graphics;
  while ( !convergenceFlag ) {

    // Compute trust region bounds.  If the trust region extends outside
    // the global bounds, then truncate to the global bounds.
    bool tr_truncation = tr_bounds(global_lower_bnds, global_upper_bnds,
				   tr_lower_bnds,     tr_upper_bnds);

    // a flag for global approximations defining the availability of the
    // current iterate in the DOE/DACE evaluations: CCD/BB DOE evaluates the
    // center of the sampled region, whereas LHS/OA/QMC/CVT DACE does not.
    daceCenterPtFlag = (dace_center_eval_flag && !tr_truncation);

    // Set the trust region center and bounds
    iteratedModel.continuous_variables(varsCenter.continuous_variables());
    iteratedModel.continuous_lower_bounds(tr_lower_bnds);
    iteratedModel.continuous_upper_bounds(tr_upper_bnds);
    if (recastSubProb) { // update recast model
      approxSubProbModel.continuous_variables(
	varsCenter.continuous_variables());
      approxSubProbModel.continuous_lower_bounds(tr_lower_bnds);
      approxSubProbModel.continuous_upper_bounds(tr_upper_bnds);
    }

    // Build new approximations in parallel and compute corrections for use
    // within run_iterator() (unless previous data can be reused).
    // -->> global approx rebuild always needed (new center or new bounds).
    // -->> local/multipt/hierarchical approx rebuild needed only if new center.
    if (globalApproxFlag || newCenterFlag) {

      // ***************************************
      // Build the approximation
      // Evaluate/retrieve responseCenterTruth
      // Perform hard convergence check
      // ***************************************
      bool embed_correction = false;
      if (globalApproxFlag) { // global with old or new center

	// Retrieve responseCenterTruth if possible, evaluate it if not
	find_center_truth(dace_iterator, truth_model);

	// Assess hard convergence prior to global surrogate construction
	if (newCenterFlag)
	  hard_convergence_check(responseCenterTruth,
				 varsCenter.continuous_variables(),
				 global_lower_bnds, global_upper_bnds);

	// Perform the sampling and the surface fitting
	if (!convergenceFlag)
	  // embed_correction is true if constraints accepted by surrogate
	  embed_correction = iteratedModel.build_approximation(varsCenter,
	    responseCenterTruth); // TO DO: problem with CCD/BB duplication!

	/*
	if ( !multiLayerBypassFlag && !daceCenterPtFlag )
	  // Can augment the global approximation with new center point data
	  iteratedModel.update_approximation(varsCenter, responseCenterTruth,
                                             true);
	*/
      }
      else { // local/multipt/hierarchical with new center

	// Evaluate the truth model at the center of the trust region.
	// Local needs values/grads & may need Hessians depending on order of
	// series, multipoint needs values/grads, hierarchical needs values &
	// may need grads/Hessians depending on order of correction.
	iteratedModel.build_approximation();

	// Retrieve responseCenterTruth if possible, evaluate it if not
	find_center_truth(dace_iterator, truth_model);

	// Assess hard convergence following build/retrieve
	hard_convergence_check(responseCenterTruth,
			       varsCenter.continuous_variables(),
			       global_lower_bnds, global_upper_bnds);

	if ( localApproxFlag || (multiptApproxFlag && !approxHessianFlag) )
	  embed_correction = true;
      }

      // Update graphics for iteration 0 (initial guess).
      if (sbIterNum == 0)
	dakota_graphics.add_datapoint(iteratedModel.current_variables(),
				      responseCenterTruth);

      if (!convergenceFlag) {
	// **************************************
	// Evaluate/retrieve responseCenterApprox
	// **************************************
	find_center_approx();
	  
	// ******************************************
	// Compute additive/multiplicative correction
	// ******************************************
	if (correctionFlag && !embed_correction) {
	  // -->> local and up to 1st-order multipt do not need correction
	  // -->> hierarchical needs compute_correction if new center
	  // -->> global needs compute_correction if new center or new bounds
	  iteratedModel.compute_correction(responseCenterTruth,
					   responseCenterApprox,
					   varsCenter.continuous_variables());
	  iteratedModel.apply_correction(responseCenterApprox,
					 varsCenter.continuous_variables());
	}
	else
	  iteratedModel.auto_correction(false); // TO DO: correct usage ???
      }
    } // end of "if (globalApproxFlag || newCenterFlag)" block
    else
      Cout << "\n>>>>> Reusing previous approximation.\n";

    // If hard convergence not achieved in truth values, perform approximate
    // optimization followed by additional (soft) convergence checks.
    if (!convergenceFlag) {

      // *******************************************************
      // Run iterator on approximation (with correction applied)
      // *******************************************************
      Cout << "\n>>>>> Starting approximate optimization cycle.\n";
      iteratedModel.auto_correction(true);
      if ( trConstraintRelax > NO_RELAX ) // relax constraints if requested
      	relax_constraints(tr_lower_bnds, tr_upper_bnds);
      // This iterator constructed from DB, but summary output suppressed:
      approxSubProbMinimizer.run_iterator(Cout);

      Cout << "\n<<<<< Approximate optimization cycle completed.\n";
      sbIterNum++; // full iteration performed: increment the counter

      // *******************************************
      // Retrieve vars_star and responseStarApprox
      // *******************************************
      vars_star = approxSubProbMinimizer.variables_results();
      if (recastSubProb) { // Can't back out eval from recast data, can't assume
	// last iteratedModel eval was the final solution, and can't use a DB
	// search for data fits.  Therefore, reevaluate (and rely on duplicate
	// detection for multifidelity surrogates).
	Cout << "\n>>>>> Evaluating approximate optimum outside of subproblem "
	     << "recasting.\n";
        iteratedModel.active_variables(vars_star);
	iteratedModel.compute_response(val_set);
	responseStarApprox.update(iteratedModel.current_response());
      }
      else // Note: fn values only
	responseStarApprox.update(approxSubProbMinimizer.response_results());
      // ****************************
      // Evaluate responseStarTruth
      // ****************************
      Cout << "\n>>>>> Evaluating approximate solution with actual model.\n";
      // since we're bypassing iteratedModel, iteratedModel.serve()
      // must be in the correct server mode.
      iteratedModel.component_parallel_mode(TRUTH_MODEL);
      truth_model.active_variables(vars_star);
      truth_model.surrogate_bypass(multiLayerBypassFlag);
      // In all cases (including gradient mode), we only need the truth fn
      // values to validate the predicted optimum.  For gradient mode, we will
      // compute the gradients below if the predicted optimum is accepted.
      truth_model.compute_response(val_set); // fn values only
      truth_model.surrogate_bypass(false);
      responseStarTruth.update(truth_model.current_response());

      // compute the trust region ratio and update soft convergence counters
      const RealVector& c_vars_star = vars_star.continuous_variables();
      tr_ratio_check(c_vars_star, tr_lower_bnds, tr_upper_bnds);

      // If the candidate optimum (vars_star) is accepted, then update the
      // center variables and response data.
      if (newCenterFlag) {
	varsCenter.continuous_variables(c_vars_star);
	responseCenterTruth.update(responseStarTruth);
	// update responseCenterApprox in the hierarchical case only if the
	// old correction can be backed out.  Currently relying on a DB search
	// to recover uncorrected low fidelity fn values.
	//if (hierarchApproxFlag)
	//  responseCenterApprox.update(responseStarApprox);
      }

      // record the iteration results (irregardless of new center)
      iteratedModel.continuous_variables(varsCenter.continuous_variables());
      dakota_graphics.add_datapoint(iteratedModel.current_variables(),
				    responseCenterTruth);

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
	else if (trustRegionFactor < minTrustRegionFactor)
	  convergenceFlag = 1;
	// terminate SBLM if the maximum number of iterations has been reached
	else if (sbIterNum >= maxIterations)
	  convergenceFlag = 2;
      }

    } // end of approx opt cycle

  } // end of outer while() loop

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
    responseCenterTruth.function_values());

  // restore original Model data
  iteratedModel.continuous_variables(initial_pt);
  iteratedModel.continuous_lower_bounds(global_lower_bnds);
  iteratedModel.continuous_upper_bounds(global_upper_bnds);
  if (recastSubProb) {
    approxSubProbModel.continuous_variables(initial_pt);
    approxSubProbModel.continuous_lower_bounds(global_lower_bnds);
    approxSubProbModel.continuous_upper_bounds(global_upper_bnds);
  }
  if (trConstraintRelax > NO_RELAX) {
    approxSubProbModel.nonlinear_ineq_constraint_lower_bounds(
      origNonlinIneqLowerBnds);
    approxSubProbModel.nonlinear_ineq_constraint_upper_bounds(
      origNonlinIneqUpperBnds);
    approxSubProbModel.nonlinear_eq_constraint_targets(origNonlinEqTargets);
  }
}


void SurrBasedLocalMinimizer::reset()
{
  convergenceFlag   = 0;
  softConvCount     = 0;
  sbIterNum         = 0;

  penaltyIterOffset = -200;
  penaltyParameter  = 5.;

  trustRegionFactor = origTrustRegionFactor;

  eta               = 1.;
  alphaEta          = 0.1;
  betaEta           = 0.9;
  etaSequence       = eta*std::pow(2.*penaltyParameter, -alphaEta);

  //lagrangeMult    = 0.; // not necessary since redefined each time
  augLagrangeMult   = 0.; // necessary since += used
}


bool SurrBasedLocalMinimizer::
tr_bounds(const RealVector& global_lower_bnds,
	  const RealVector& global_upper_bnds,
	  RealVector& tr_lower_bnds, RealVector& tr_upper_bnds)
{
  // Compute the trust region bounds
  size_t i;
  bool c_vars_truncation = false, tr_lower_truncation = false,
     tr_upper_truncation = false;
  RealVector c_vars_center;
  copy_data(varsCenter.continuous_variables(), c_vars_center);
  for (i=0; i<numContinuousVars; i++) {
    // verify that varsCenter is within global bounds
    if ( c_vars_center[i] > global_upper_bnds[i] ) {
      c_vars_center[i] = global_upper_bnds[i];
      c_vars_truncation = true;
    }
    if ( c_vars_center[i] < global_lower_bnds[i] ) {
      c_vars_center[i] = global_lower_bnds[i];
      c_vars_truncation = true;
    }
    // scalar tr_offset was previously trustRegionOffset[i]
    Real tr_offset = trustRegionFactor/2. * 
      ( global_upper_bnds[i] - global_lower_bnds[i] );
    Real up_bound = c_vars_center[i] + tr_offset;
    Real lo_bound = c_vars_center[i] - tr_offset;
    if ( up_bound <= global_upper_bnds[i] )
      tr_upper_bnds[i] = up_bound;
    else {
      tr_upper_bnds[i] = global_upper_bnds[i];
      tr_upper_truncation = true;
    }
    if ( lo_bound >= global_lower_bnds[i] )
      tr_lower_bnds[i] = lo_bound;
    else {
      tr_lower_bnds[i] = global_lower_bnds[i];
      tr_lower_truncation = true;
    }
  }
  if (c_vars_truncation)
    varsCenter.continuous_variables(c_vars_center);

  // Output the trust region bounds
  Cout << "\n**************************************************************"
       << "************\nBegin SBLM Iteration Number " << sbIterNum+1
       << "\n\nCurrent Trust Region\n                 ";
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

  return (tr_lower_truncation || tr_upper_truncation);
}


void SurrBasedLocalMinimizer::
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
  bool found = false;
  if (sbIterNum && !truthGradientFlag)
    // responseCenterTruth updated from responseStarTruth
    // (responseStarTruth has surrogate bypass; build_approx() does not)
    found = true;
  else if (!multiLayerBypassFlag && !globalApproxFlag) {
    // single layer local/multipoint/hierarchical: retrieve from build_approx.
    responseCenterTruth.update(truth_model.current_response());
    found = true;
  }
  /*
  // This test no longer valid now that find_center_truth() precedes
  // build_approximation()
  else if (!multiLayerBypassFlag && daceCenterPtFlag) {
    // single layer untruncated CCD/BB DOE: retrieve from all_responses
    const RealVectorArray& all_vars = dace_iterator.all_variables();
    const ResponseArray&   all_responses = dace_iterator.all_responses();
    size_t i, j, num_samples = all_vars.length();
    for (i=0; i<num_samples; i++) { // center should be first one
      if (all_vars[i].continuous_variables() ==
          varsCenter.continuous_variables()) {
	const ShortArray& asv = all_responses[i].active_set_request_vector();
	bool incomplete = false;
	for (j=0; j<numFunctions; j++)
	  if ( !(asv[j] & 1) || ( truthGradientFlag && !(asv[j] & 2) )
	                     || ( truthHessianFlag  && !(asv[j] & 4) ) )
	    incomplete = true;
	if (!incomplete) {
	  responseCenterTruth.update(all_responses[i]);
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
    truth_model.continuous_variables(varsCenter.continuous_variables());
    truth_model.surrogate_bypass(multiLayerBypassFlag);
    truth_model.compute_response(responseCenterTruth.active_set());
    truth_model.surrogate_bypass(false);
    responseCenterTruth.update(truth_model.current_response());
  }
}


void SurrBasedLocalMinimizer::find_center_approx()
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
    responseCenterApprox.update(responseCenterTruth);
    found = true;
  }
  else if (multiptApproxFlag && !approxHessianFlag) {
    // Note: current multipoint approximation (TANA) exactly reproduces value
    // and gradient at current expansion point and value at previous expansion
    // point.  It will also normally reproduce the gradient at the previous
    // expansion point, unless numerical safeguarding of p exponents is used.
    responseCenterApprox.update(responseCenterTruth);
    found = true;
  }
  else if (hierarchApproxFlag && sbIterNum) {
    // search for fn vals, grads, and Hessians separately since they may
    // be different fn evaluations
    Response desired_resp;
    ActiveSet search_set = responseCenterApprox.active_set(); // copy
    search_set.request_values(1);
    const Variables& search_vars = iteratedModel.current_variables();
    const String& search_id = iteratedModel.surrogate_model().interface_id();
    extern PRPCache data_pairs; // global container
    if ( lookup_by_val(data_pairs, search_id, search_vars, search_set,
		       desired_resp) ) {
      responseCenterApprox.function_values(desired_resp.function_values());
      if (approxGradientFlag) {
	search_set.request_values(2);
	if ( lookup_by_val(data_pairs, search_id, search_vars, search_set,
			   desired_resp) ) {
	  responseCenterApprox.function_gradients(
	    desired_resp.function_gradients());
	  if (approxHessianFlag) {
	    search_set.request_values(4);
	    if ( lookup_by_val(data_pairs, search_id, search_vars, search_set,
			       desired_resp) ) {
	      responseCenterApprox.function_hessians(
		desired_resp.function_hessians());
	      found = true;
	    }
	  }
	  else
	    found = true;
	}
      }
      else
	found = true;
    }
  }

  if (found)
    Cout << "\n>>>>> Previous approximate response retrieved at trust "
	 << "region center.\n"; // << responseCenterApprox;
  else { // responseCenterApprox not available
    Cout <<"\n>>>>> Evaluating approximation at trust region center.\n";
    iteratedModel.auto_correction(false);
    iteratedModel.compute_response(responseCenterApprox.active_set());
    responseCenterApprox.update(iteratedModel.current_response());
  }
}


/** The hard convergence check computes the gradient of the merit
    function at the trust region center, performs a projection for
    active bound constraints (removing any gradient component directed
    into an active bound), and signals convergence if the 2-norm of
    this projected gradient is less than convergenceTol. */
void SurrBasedLocalMinimizer::
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
  // These updates are only performed for new iterates from accepted
  // steps (hard_convergence_check() invoked only if newCenterFlag).

  // initialize augmented Lagrangian multipliers and filter
  if (sbIterNum == 0) {
    if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
	approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE)
      update_augmented_lagrange_multipliers(fns_truth);
    if (acceptLogic == FILTER)
      update_filter(fns_truth);
  }

  // hard convergence assessment requires gradients
  if (!truthGradientFlag)
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
  //if (meritFnType == LAGRANGIAN_MERIT)
  lagrangian_gradient(fns_truth, grads_truth, origNonlinIneqLowerBnds,
    origNonlinIneqUpperBnds, origNonlinEqTargets, merit_fn_grad);
  //else if (meritFnType == AUGMENTED_LAGRANGIAN_MERIT)
  //  augmented_lagrangian_gradient(fns_truth, grads_truth,
  //    origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets,
  //    merit_fn_grad);
  //else
  //  penalty_gradient(fns_truth, grads_truth, origNonlinIneqLowerBnds,
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


/** Assess acceptance of SBLM iterate (trust region ratio or filter)
    and compute soft convergence metrics (number of consecutive
    failures, min trust region size, etc.) to assess whether the
    convergence rate has decreased to a point where the process should
    be terminated (diminishing returns). */
void SurrBasedLocalMinimizer::
tr_ratio_check(const RealVector& c_vars_star,
	       const RealVector& tr_lower_bnds,
	       const RealVector& tr_upper_bnds)
{
  const RealVector& fns_center_truth  = responseCenterTruth.function_values();
  const RealVector& fns_star_truth    = responseStarTruth.function_values();
  const RealVector& fns_center_approx = responseCenterApprox.function_values();
  const RealVector& fns_star_approx   = responseStarApprox.function_values();

  // ---------------------------------------------------
  // Compute trust region ratio based on merit fn values
  // ---------------------------------------------------

  Real merit_fn_center_truth,  merit_fn_star_truth,
       merit_fn_center_approx, merit_fn_star_approx;

  // TO DO: in global case, search dace_iterator.all_responses() for a
  // better point (using merit fn or filter?) than the minimizer found.
  // If this occurs, then accept point but shrink TR.
  //Iterator& dace_iterator = iteratedModel.subordinate_iterator();
  //ResponseArray dace_responses;
  //if (globalApproxFlag && !dace_iterator.is_null())
  //  dace_responses = dace_iterator.all_responses();

  // Consolidate objective fn values and constraint violations into a
  // single merit fn value for center truth/approx and star truth/approx.
  if (meritFnType == LAGRANGIAN_MERIT) { // penalty-free (like filter)

    // This approach has been observed to be ineffective since NNLS/BVLS
    // estimates of lagrangeMult are often zero away from a constrained min,
    // resulting in merit fns that omit important constraint violations.

    // evaluate each merit function with the same lagrangeMult estimates
    // (updated from responseCenterTruth in hard_convergence_check()).
    merit_fn_center_truth = lagrangian_merit(fns_center_truth,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_star_truth = lagrangian_merit(fns_star_truth,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_center_approx = lagrangian_merit(fns_center_approx,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_star_approx = lagrangian_merit(fns_star_approx,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);

    //for (i=0; i<num_dace_responses; i++) {
    //  Real merit_dace_truth = lagrangian_merit(
    //    dace_responses[i].function_values(), origNonlinIneqLowerBnds,
    //    origNonlinIneqUpperBnds, origNonlinEqTargets);
    //  if (merit_dace_truth < merit_fn_star_truth) {
    //    Cerr << "Warning: \n";
    //    vars_star = ...;
    //    merit_fn_star_truth = merit_dace_truth;
    //    trustRegionFactor *= ...;
    //  }
    //}
  }
  else if (meritFnType == AUGMENTED_LAGRANGIAN_MERIT) {

    // evaluate each merit function with the same augLagrangeMult estimates
    // (updated from fns_center_truth for sbIterNum == 1 and from
    // fns_star_truth for accepted steps) and penaltyParameter (updated if
    // no reduction in constraint violation).
    merit_fn_center_truth = augmented_lagrangian_merit(fns_center_truth,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_star_truth = augmented_lagrangian_merit(fns_star_truth,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_center_approx = augmented_lagrangian_merit(fns_center_approx,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_star_approx = augmented_lagrangian_merit(fns_star_approx,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
  }
  //else if (meritFnType == FILTER_AREA) {
    // Early concept of filter area
    //numerator   = filter_merit(fns_center_truth,  fns_star_truth);
    //denominator = filter_merit(fns_center_approx, fns_star_approx);
  //}
  else { // penalty method: basic or adaptive

    // An adaptive penalty is computed based on the ratio of the objective fn
    // change to the constraint violation change at each obj/cv trade-off.
    // For basic penalties and between adaptive trade-offs, penaltyParameter
    // is ramped exponentially using the sbIterNum counter.
    update_penalty(fns_center_truth, fns_star_truth);

    // evaluate each merit function with updated/adapted penaltyParameter
    merit_fn_center_truth  = penalty_merit(fns_center_truth);
    merit_fn_star_truth    = penalty_merit(fns_star_truth);
    merit_fn_center_approx = penalty_merit(fns_center_approx);
    merit_fn_star_approx   = penalty_merit(fns_star_approx);
  }

#ifdef DEBUG
  Cout << "Response truth:\ncenter = " << responseCenterTruth << "star = "
       << responseStarTruth << "Response approx:\ncenter = "
       << responseCenterApprox << "star = " << responseStarApprox;
  Cout << "Merit fn truth:  center = " << merit_fn_center_truth << " star = "
       << merit_fn_star_truth << "\nMerit fn approx: center = "
       << merit_fn_center_approx << " star = " << merit_fn_star_approx << '\n';
#endif

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

  bool accept_step;
  if (acceptLogic == FILTER)
    accept_step = update_filter(fns_star_truth);
  else if (acceptLogic == TR_RATIO)
    // Accept the step based on simple decrease in the truth merit functions.
    // This avoids any issues with denominator < 0 in tr_ratio:
    //   numerator > 0 and denominator < 0: accept step and contract TR
    //   numerator < 0 and denominator < 0: reject step even though tr_ratio > 0
    accept_step = (numerator > 0.);

  // ------------------------------------------
  // Trust region shrink/expand/translate logic
  // ------------------------------------------

  if (accept_step) {
    newCenterFlag = true;

    // Update the trust region size depending on the accuracy of the approximate
    // model. Note: If eta_1 < tr_ratio < eta_2, trustRegionFactor does not
    // change where eta_1 = trRatioContractValue and eta_2 = trRatioExpandValue
    // Recommended values from Conn/Gould/Toint are: eta_1 = 0.05, eta_2 = 0.90
    // For SBLM, the following are working better:   eta_1 = 0.25, eta_2 = 0.75
    // More experimentation is needed.
    Cout << "\n<<<<< Trust Region Ratio = " << tr_ratio << ":\n<<<<< ";
    if (tr_ratio <= trRatioContractValue) { // accept optimum, shrink TR
      trustRegionFactor *= gammaContract;
      Cout << "Marginal Accuracy, ACCEPT Step, REDUCE Trust Region Size\n\n";
    }
    else if (std::fabs(1.-tr_ratio) <= 1.-trRatioExpandValue) { // accept & expand TR
      // for trRatioExpandValue = 0.75, expand if 0.75 <= tr_ratio <= 1.25
      // This new logic avoids increasing the TR size when a good step is found
      // but the surrogates are inaccurate (e.g., tr_ratio = 2).
 
      // For global approximations (especially those lacking local consistency),
      // make a distinction between successful candidate points that lie on the
      // boundary versus those that lie inside the trust region. The reasoning
      // is that if the tr_ratio is good but the optimum point is interior to
      // the trust region, then the trust region should not grow since this
      // would decrease the sampling density and harm local accuracy. This
      // approach is followed in the work of Rodriguez, Renaud, and Watson, but
      // not in the work of Conn, Gould, and Toint (not surprisingly, as they
      // don't use global data fits).
      bool boundary_pt_flag = false;
      if (globalApproxFlag)
	for (size_t i=0; i<numContinuousVars; i++)
	  if ( c_vars_star[i] > tr_upper_bnds[i] - constraintTol ||
	       c_vars_star[i] < tr_lower_bnds[i] + constraintTol )
	    boundary_pt_flag = true;

      if (globalApproxFlag && !boundary_pt_flag)
	Cout << "Excellent Accuracy, Iterate in Trust Region Interior, "
	     << "ACCEPT Step, RETAIN Trust Region Size\n\n";
      else {
	trustRegionFactor *= gammaExpand;
	Cout << "Excellent Accuracy, ACCEPT Step, INCREASE Trust Region Size"
	     << "\n\n";
      }
    }
    else // accept optimum, retain current TR
      Cout <<"Satisfactory Accuracy, ACCEPT Step, RETAIN Trust Region Size\n\n";
  }
  else {
    // If the step is rejected, then retain the current design variables
    // and shrink the TR size.
    newCenterFlag = false;
    trustRegionFactor *= gammaContract;
    if (acceptLogic == FILTER)
      Cout << "\n<<<<< Iterate rejected by Filter, Trust Region Ratio = "
	   << tr_ratio;
    else { // acceptLogic == TR_RATIO
      if (tr_ratio > 0.) // rare case of denominator<0
	Cout << "\n<<<<< Trust Region Ratio Numerator = " << numerator;
      else
	Cout << "\n<<<<< Trust Region Ratio = " << tr_ratio;
    }
    Cout << ":\n<<<<< Poor accuracy, REJECT Step, REDUCE Trust Region Size\n\n";
  }

  // ------------------------------
  // Perform updates for next cycle
  // ------------------------------

  // Perform updates for augmented Lagrangian.
  // [For the basic Lagrangian, multiplier updates are not dependent on step
  // acceptance, but require grads_star_truth which have not yet been computed
  // -> this update is performed in hard_convergence_check().]
  if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
      approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE) {
    // Previous logic: multipliers are only updated if the step is accepted
    // (to prevent redundant additions in simple update formulas), and penalty
    // is updated only if the constraint violation did not decrease.
    //if (accept_step) // update multipliers first, then penalty
    //  update_augmented_lagrange_multipliers(fns_star_truth);
    //update_penalty(fns_center_truth, fns_star_truth);

    // New logic follows Conn, Gould, and Toint, section 14.4:
    Real norm_cv_star = std::sqrt(constraint_violation(fns_star_truth, 0.));
    if (norm_cv_star < etaSequence)
      update_augmented_lagrange_multipliers(fns_star_truth);
    else
      update_penalty(fns_center_truth, fns_star_truth);
  }

  // Filter area case: soft convergence counter is incremented if step is
  // rejected or insufficient filter metric improvement in actual or approx.
  //if ( !accept_step || std::fabs(numerator)   < convergenceTol ||
  //	                 std::fabs(denominator) < convergenceTol )
  // Merit fn case: soft convergence counter is incremented if insufficient
  // relative or absolute improvement in actual or approx.
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


/** Scaling of the penalty value is important to avoid rejecting SBLM iterates
    which must increase the objective to achieve a reduction in constraint
    violation.  In the basic penalty case, the penalty is ramped exponentially
    based on the iteration counter.  In the adaptive case, the ratio of
    relative change between center and star points for the objective and
    constraint violation values is used to rescale penalty values. */
void SurrBasedLocalMinimizer::
update_penalty(const RealVector& fns_center_truth,
	       const RealVector& fns_star_truth)
{
  // Note: there is a single hard convergence check prior to the first
  // invocation of this function for which the constructor initialized
  // value of penaltyParameter value is used.

  if (meritFnType == PENALTY_MERIT) {
    // Set the value of the penalty parameter in the penalty function.  An
    // offset of e^2.1 is used; this equates to constraint violation reduction
    // initially being ~8 times as important as objective reduction.
    penaltyParameter = std::exp( 2.1 + (double)sbIterNum/10.0 );
    // this penalty schedule can be inconsistent with the final result of the
    // minimizer applied to the surrogate model.  That is, the minimizer should
    // return a large penalty solution (minimal constraint violation) by the end
    // of its iteration, and the above penalty schedule may cause SBLM to reject
    // this solution as having increased the objective fn too much, even if the
    // surrogate/truth agreement is excellent.  However, too high of a penalty
    // can cause problems once the feasible region is located and constraints
    // are active (i.e., if the verification of an approximate optimum results
    // in good objective fn improvement but a small constraint violation in the
    // truth model, this can be rejected depending on the penalty value).
  }
  else if (meritFnType == ADAPTIVE_PENALTY_MERIT) {
    // If there is a trade-off between obj increase and constr violation
    // reduction, update penaltyIterOffset to rescale the penaltyParameter so as
    // to accept the current iterate if this can be accomplished within the
    // restriction of a monotonic _increase_ in penalty level.  This is less
    // restrictive than the pure penalty fn approach (due to the ability to
    // rescale the penalty parameter to accept steps that reduce the constraint
    // violation) but is more restrictive than a filter method (monotonic
    // reductions in constr violation are sought through a strictly increasing
    // penalty as in the pure penalty fn approach).  The goal is to provide more
    // freedom for the approximate optimization cycles to dictate the penalty
    // schedule, rather than arbitrarily assigning one that will often be out of
    // synch.  Other cases: if there is both objective and constraint violation
    // increase or both objective and constraint violation decrease, then the
    // new point will be rejected or accepted, respectively, independent of
    // penalty value.  If there is objective decrease and constraint violation
    // increase, then this point may be accepted depending on the change ratio
    // and penalty value.  The penalty offset is _not_ updated in this case, and
    // the acceptance of these points becomes less likely as the iteration
    // progresses and the penalty ramps up.
    Real obj_delta = objective(fns_star_truth) - objective(fns_center_truth);
    Real cv_delta  = constraint_violation(fns_star_truth,   constraintTol)
                   - constraint_violation(fns_center_truth, constraintTol);
    //Cout << "obj_delta = " << obj_delta << " cv_delta = " << cv_delta << '\n';
    if (cv_delta < 0. && obj_delta > 0.) {
      // round up such that the current iterate would be accepted
      int min_iter= (int)std::ceil(10.*std::log(std::fabs(obj_delta/cv_delta)));
      // if min penalty > current penalty, then update the penalty offset.  The
      // offset has a max value of 200 and does not truncate to this value if it
      // is exceeded.  This results in the rejection of points for which the
      // relative improvement in constraint violation is extremely small (and
      // avoids corrupting the penalty schedule in that case); however, it could
      // cause problems in making it difficult to achieve an offset near 200.
      int new_offset = min_iter - sbIterNum;
      //int delta_offset = new_offset - penaltyIterOffset;
      if (new_offset > penaltyIterOffset && new_offset < 200)
	penaltyIterOffset = new_offset;
    }

    // A penalty parameter on the edge of step acceptance/rejection for the
    // truth model (resulting from a fresh penaltyIterOffset update) increases
    // the likelihood of a negative TR ratio resulting from an increase in the
    // approx penalty fn.  This can lead to premature soft convergence.
    // Therefore, increase the penalty beyond the minimum by a small amount
    // (5 iters = half an exponential power = 65% additional penalty).  Both
    // penaltyIterOffset and sbIterNum are capped at 200 (e^20 =~ 5e8) to
    // reduce problems w/ small infeasibilities and prevent numerical overflow.
    penaltyParameter = (sbIterNum < 200)
      ? std::exp((double)(sbIterNum + penaltyIterOffset + 5)/10.)
      : std::exp(20.5 + (double)penaltyIterOffset/10.);
    //Cout << "penaltyIterOffset = " << penaltyIterOffset
    //     << " penaltyParameter = " << penaltyParameter << '\n';
  }
  else if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
	   approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE) {
    // Previous logic: update the penalty only if the constraint violation
    // was not reduced
    //Real cv_star   = constraint_violation(fns_star_truth,   0.),
    //     cv_center = constraint_violation(fns_center_truth, 0.),
    //     cv_delta  = cv_star - cv_center;
    //if (cv_star > 0. && cv_delta >= 0.)
    //  penaltyParameter *= 2.;

    // New logic follows Conn, Gould, and Toint, section 14.4, step 3
    //   CGT use mu *= tau with tau = 0.01 ->   r_p *= 50
    //   Rodriguez, Renaud, Watson:             r_p *= 10
    //   Robinson, Willcox, Eldred, and Haimes: r_p *= 5
    penaltyParameter *= 10.;
    //penaltyParameter = std::min(penaltyParameter, 1.e+20); // cap the max penalty?
    Real mu = 1./2./penaltyParameter; // conversion between r_p and mu penalties
    etaSequence = eta*std::pow(mu, alphaEta);
  }

#ifdef DEBUG
  Cout << "Penalty updated: " << penaltyParameter << '\n';
  if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
      approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE)
    Cout << "eta updated: " << etaSequence << '\n';
#endif
}


/** Objective functions evaluator for solution of approximate
    subproblem using a RecastModel. */
void SurrBasedLocalMinimizer::
approx_subprob_objective_eval(const Variables& surrogate_vars,
			      const Variables& recast_vars,
			      const Response& surrogate_response,
			      Response& recast_response)
{
  // RecastModel evaluates its subModel response and then invokes this fn
  const RealVector& surrogate_fns   = surrogate_response.function_values();
  const RealMatrix& surrogate_grads = surrogate_response.function_gradients();
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (sblmInstance->approxSubProbObj == ORIGINAL_PRIMARY) {
    for (size_t i=0; i<sblmInstance->numUserPrimaryFns; i++) {
      if (recast_asv[i] & 1)
	recast_response.function_value(surrogate_fns[i], i);
      if (recast_asv[i] & 2)
	recast_response.function_gradient(
	  surrogate_response.function_gradient(i), i );
    }
  }
  else {
    // use approxSubProbModel to retrieve bounds/targets in order to capture
    // any relaxation, except for the case where the subproblem has been
    // recast without constraints.  In this case, relaxation is not active,
    // the approxSubProbModel has empty bounds/targets, and the original
    // bounds/targets are used.
    bool no_sub_prob_con = (sblmInstance->approxSubProbCon == NO_CONSTRAINTS);
    const RealVector& nln_ineq_l_bnds = (no_sub_prob_con) ? 
      sblmInstance->origNonlinIneqLowerBnds :
      sblmInstance->approxSubProbModel.nonlinear_ineq_constraint_lower_bounds();
    const RealVector& nln_ineq_u_bnds = (no_sub_prob_con) ?
      sblmInstance->origNonlinIneqUpperBnds :
      sblmInstance->approxSubProbModel.nonlinear_ineq_constraint_upper_bounds();
    const RealVector& nln_eq_tgts = (no_sub_prob_con) ?
      sblmInstance->origNonlinEqTargets :
      sblmInstance->approxSubProbModel.nonlinear_eq_constraint_targets();

    if (recast_asv[0] & 1) {
      Real recast_fn;
      switch (sblmInstance->approxSubProbObj) {
      case SINGLE_OBJECTIVE:
	recast_fn = sblmInstance->objective(surrogate_fns);
	break;
      case LAGRANGIAN_OBJECTIVE:
	recast_fn = sblmInstance->lagrangian_merit(surrogate_fns,
	  nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts);
	break;
      case AUGMENTED_LAGRANGIAN_OBJECTIVE:
	recast_fn = sblmInstance->augmented_lagrangian_merit(surrogate_fns,
          nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts);
	break;
      }
      recast_response.function_value(recast_fn, 0);
    }

    if (recast_asv[0] & 2) {
      RealVector recast_grad;
      switch (sblmInstance->approxSubProbObj) {
      case SINGLE_OBJECTIVE:
	sblmInstance->objective_gradient(surrogate_fns, surrogate_grads,
	  recast_grad);
	break;
      case LAGRANGIAN_OBJECTIVE:
	sblmInstance->lagrangian_gradient(surrogate_fns, surrogate_grads,
	  nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts, recast_grad);
	break;
      case AUGMENTED_LAGRANGIAN_OBJECTIVE:
	sblmInstance->augmented_lagrangian_gradient(surrogate_fns,
	  surrogate_grads, nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts,
	  recast_grad);
	break;
      }
      recast_response.function_gradient(recast_grad, 0);
    }
  }
}


/** Constraint functions evaluator for solution of approximate
    subproblem using a RecastModel. */
void SurrBasedLocalMinimizer::
approx_subprob_constraint_eval(const Variables& surrogate_vars,
			       const Variables& recast_vars,
			       const Response& surrogate_response,
			       Response& recast_response)
{
  // RecastModel evaluates its subModel response and then invokes this fn
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  size_t i, num_recast_fns = recast_asv.size();
  size_t num_recast_primary =
    (sblmInstance->approxSubProbObj == ORIGINAL_PRIMARY) ?
    sblmInstance->numUserPrimaryFns : 1;
  if (num_recast_fns <= num_recast_primary)
    return;
  size_t num_recast_cons = num_recast_fns - num_recast_primary;

  const RealVector& surrogate_fns   = surrogate_response.function_values();
  //const RealMatrix& surrogate_grads = surrogate_response.function_gradients();

  switch (sblmInstance->approxSubProbCon) {

  case ORIGINAL_CONSTRAINTS: {
    //
    // Note: constraints are NOT converted to std form as done within merit fns
    //
    for (i=0; i<num_recast_cons; i++) {
      size_t recast_i = i + num_recast_primary,
	     surr_i   = i + sblmInstance->numUserPrimaryFns;
      if (recast_asv[recast_i] & 1)
	recast_response.function_value(surrogate_fns[surr_i], recast_i);
      if (recast_asv[recast_i] & 2)
	recast_response.function_gradient(
	  surrogate_response.function_gradient(surr_i), recast_i );
    }
    break;
  }

  case LINEARIZED_CONSTRAINTS: {
    //
    // Note: constraints are NOT converted to std form as done within merit fns
    //
    // Surrogate values and grads are used for the linearization since we may
    // not have truth model grads.  If we do have truth model grads, then the
    // surrogate model _should_ be first-order consistent at the TR center.
    const RealVector& center_c_vars
      = sblmInstance->varsCenter.continuous_variables();
    const RealVector& center_surrogate_fns
      = sblmInstance->responseCenterApprox.function_values();
    const RealMatrix& center_surrogate_grads
      = sblmInstance->responseCenterApprox.function_gradients();
    const RealVector& c_vars = recast_vars.continuous_variables();

    size_t j, num_recast_vars
      = recast_response.active_set_derivative_vector().size();
    for (i=0; i<num_recast_cons; i++) {
      size_t recast_i = i + num_recast_primary,
	     surr_i   = i + sblmInstance->numUserPrimaryFns;
      if (recast_asv[recast_i] & 1) {
	Real sum = center_surrogate_fns[surr_i];
	for (j=0; j<num_recast_vars; j++)
	  sum += center_surrogate_grads(j,surr_i)
	      *  (c_vars[j] - center_c_vars[j]);
	recast_response.function_value(sum, recast_i);
      }
      if (recast_asv[recast_i] & 2)
	recast_response.function_gradient(
	  sblmInstance->responseCenterApprox.function_gradient(surr_i),
	  recast_i );
    }
#ifdef DEBUG
    Cout << "center_c_vars =\n" << center_c_vars << "c_vars =\n" << c_vars
	 << "center_surrogate_fns =\n" << center_surrogate_fns
	 << "center_surrogate_grads =\n" << center_surrogate_grads
	 << "recast_response =\n" << recast_response << std::endl;
#endif
    break;
  }

  case NO_CONSTRAINTS:
    break;
  }
}


void SurrBasedLocalMinimizer::
relax_constraints(const RealVector& lower_bnds,
		  const RealVector& upper_bnds)
{
  // NOTE 1: this needs revision in the case where the surrogates could
  // be expensive (e.g., multifidelity).  Resort to linearized surrogates
  // (SQP-like) in this case.

  // NOTE 2: approxSubProbModel (and iteratedModel if no recasting) contains
  // the relaxed constraint bounds and targets, and is used in the approximate
  // subproblem cycles.  The original bounds and targets are catalogued in SBLM
  // class member variables in the ctor.

  // get current function/constraint values
  const RealVector& fns_center_truth = responseCenterTruth.function_values();

  // initial relaxation data during the first SBLM iteration
  if (sbIterNum == 0) {

    // initialize inequality constraint slack vectors
    if (numNonlinearIneqConstraints) {
      nonlinIneqLowerBndsSlack.sizeUninitialized(numNonlinearIneqConstraints);
      nonlinIneqLowerBndsSlack = 0.;
      nonlinIneqUpperBndsSlack.sizeUninitialized(numNonlinearIneqConstraints);
      nonlinIneqUpperBndsSlack = 0.;
      for (size_t i=0; i<numNonlinearIneqConstraints; i++) {
	const Real& nln_ineq_con = fns_center_truth[numUserPrimaryFns+i];
	const Real& l_bnd = origNonlinIneqLowerBnds[i];
	const Real& u_bnd = origNonlinIneqUpperBnds[i];
	if (nln_ineq_con < l_bnd)                   // *** constraint tol?
	  nonlinIneqLowerBndsSlack[i] = nln_ineq_con - l_bnd; // *** sign?
	else if (nln_ineq_con > u_bnd)              // *** constraint tol?
	  nonlinIneqUpperBndsSlack[i] = nln_ineq_con - u_bnd;
      }

      // output of slacks required for postprocessing SBLM runs in Matlab
      Cout << "\n<<<<< nonlinIneqLowerBndsSlack =\n";
      write_data(Cout, nonlinIneqLowerBndsSlack);
      Cout << "\n<<<<< nonlinIneqUpperBndsSlack =\n";
      write_data(Cout, nonlinIneqUpperBndsSlack);
    }

    // initialize equality constraint slack vectors
    if (numNonlinearEqConstraints) {
      nonlinEqTargetsSlack.sizeUninitialized(numNonlinearEqConstraints);
      nonlinEqTargetsSlack = 0.;
      for (size_t i=0; i<numNonlinearEqConstraints; i++) {
	const Real& nln_eq_con
	  = fns_center_truth[numUserPrimaryFns+numNonlinearIneqConstraints+i];
	const Real& tgt = origNonlinEqTargets[i];
	if ( std::fabs(tgt - nln_eq_con) > constraintTol)
	  nonlinEqTargetsSlack[i] = nln_eq_con - tgt; // *** constraint tol?
      }
      // output of slacks required for postprocessing SBLM runs in Matlab
      Cout << "\n<<<<< nonlinEqTargetsSlack =\n";
      write_data(Cout, nonlinEqTargetsSlack);
    }

    // initialize constraint relaxation parameters
    tau = 0.;
    alpha = 0.9;
  }

  Real constr_viol = constraint_violation(fns_center_truth, 0.);
  if (constr_viol > constraintTol) {
    Cout << "\n<<<<< Constraint violation = " << constr_viol
	 << "\n<<<<< Adjusting constraints ...\n";
    
    // Use NPSOL/OPT++ in "user_functions" mode to optimize tau
    Iterator tau_minimizer;
    
    // derivative level for NPSOL (1 = supplied grads of objective fn,
    // 2 = supplied grads of constraints, 3 = supplied grads of both)
    int deriv_level = 3;
    Real conv_tol = -1.; // use NPSOL default
    // linear constraints not currently used in constraint relaxation 
    RealMatrix lin_ineq_coeffs, lin_eq_coeffs;
    RealVector lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets;
    // initial (tau,x) point and bounds
    RealVector tau_and_x_initial(numContinuousVars+1),
                    tau_and_x_lower_bnds(numContinuousVars+1),
                    tau_and_x_upper_bnds(numContinuousVars+1);
    
    // tau
    tau_and_x_initial[0]    = tau;
    tau_and_x_lower_bnds[0] = 0.;
    tau_and_x_upper_bnds[0] = 1.;
    
    // x
    copy_data_partial(varsCenter.continuous_variables(), tau_and_x_initial, 1);
    copy_data_partial(lower_bnds, tau_and_x_lower_bnds, 1);
    copy_data_partial(upper_bnds, tau_and_x_upper_bnds, 1);
    
    // setup optimization problem for updating tau
#ifdef DAKOTA_NPSOL
    tau_minimizer.assign_rep(new_NPSOLOptimizer3(tau_and_x_initial,
      tau_and_x_lower_bnds, tau_and_x_upper_bnds, lin_ineq_coeffs,
      lin_ineq_lower_bnds, lin_ineq_lower_bnds, lin_eq_coeffs, lin_eq_targets,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets,
      hom_objective_eval, hom_constraint_eval, deriv_level, conv_tol), false);
#endif

    // find optimum tau
    // no summary output since on-the-fly constructed:
    tau_minimizer.run_iterator(Cout);

    // retrieve tau from current response
    const RealVector& tau_and_x_star
      = tau_minimizer.variables_results().continuous_variables();
#ifdef DEBUG
    Cout << "tau_and_x_star:\n" << tau_and_x_star << std::endl;
#endif // debug

    // update tau
    Real tau_new = tau + alpha*(tau_and_x_star[0] - tau);
    tau = (tau_new < 1.) ? tau_new : 1;

    // relax constraints for SBLM if needed
    if (tau < 1.) {
      
      // nonlinear inequality constraints (based on tau parameter)
      if (numNonlinearIneqConstraints) {
	// create copies of true constraints
	RealVector nln_ineq_l_bnds(origNonlinIneqLowerBnds), 
	                nln_ineq_u_bnds(origNonlinIneqUpperBnds);

	// update constraint bounds to be used with SBLM iteration
	for(size_t i=0; i<numNonlinearIneqConstraints; i++) {
	  nln_ineq_l_bnds[i] += (1.-tau)*nonlinIneqLowerBndsSlack[i];
	  nln_ineq_u_bnds[i] += (1.-tau)*nonlinIneqUpperBndsSlack[i];
	}
	approxSubProbModel.nonlinear_ineq_constraint_lower_bounds(
	  nln_ineq_l_bnds);
	approxSubProbModel.nonlinear_ineq_constraint_upper_bounds(
          nln_ineq_u_bnds);
      }
      
      // nonlinear equality target (based on tau parameter)
      if (numNonlinearEqConstraints) {
	// create copy of true constraints
	RealVector nln_eq_targets(origNonlinEqTargets);
      
	// update constraint bounds to be used with SBLM iteration
	for(size_t i=0; i<numNonlinearEqConstraints; i++)
	  nln_eq_targets[i] += (1.-tau)*nonlinEqTargetsSlack[i];
       	approxSubProbModel.nonlinear_eq_constraint_targets(nln_eq_targets);
      }

    } // tau < 1.
  } // constr_viol > constraintTol
  else {
    // force tau to 1; necessary for first truth center with const_viol > 0
    tau = 1.;
    Cout << "\n<<<<< No constraint violation\n";
  }

  // output of tau required for postprocessing SBLM runs in Matlab
  Cout << "\n<<<<< tau = " << tau << '\n';
}


/** NPSOL objective functions evaluator for solution of homotopy constraint 
    relaxation parameter optimization. This constrained optimization problem
    performs the update of the tau parameter in the homotopy heuristic 
    approach used to relax the constraints in the original problem . */
void SurrBasedLocalMinimizer::
hom_objective_eval(int& mode, int& n, double* tau_and_x, double& f,
		   double* grad_f, int&)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;

  if (asv_request & 1) {
    f = -tau_and_x[0]; // tau is first element
#ifdef DEBUG
    Cout << "f = " << f << std::endl;
#endif // DEBUG
  }
  if (asv_request & 2) {
    grad_f[0] = -1.;   // d(-tau)/dtau
    for (int i=1; i<n; i++)
      grad_f[i] = 0.;  // d(-tau)/dx = 0 (tau does not depend on other vars)
#ifdef DEBUG
    for (int i=0; i<n; i++)
      Cout << "grad_f[" << i << "] = " << grad_f[i] << std::endl;
#endif // DEBUG
  }
}


/** NPSOL constraint functions evaluator for solution of homotopy constraint 
    relaxation parameter optimization. This constrained optimization problem
    performs the update of the tau parameter in the homotopy heuristic 
    approach used to relax the constraints in the original problem. */
void SurrBasedLocalMinimizer::
hom_constraint_eval(int& mode, int& ncnln, int& n, int& nrowj, int* needc,
                    double* tau_and_x, double* c, double* cjac, int& nstate)
{
  // NPSOL mode: 0 = get active constraint values using needc
  //             1 = get active constraint gradients using needc
  //             2 = get active constraint values and gradients using needc
  // Currently, needc is ignored which is OK since SOLBase::constraint_eval() 
  // sets *all* of cjac and inactive entries are successfully ignored.

  short asv_request = mode + 1;

  // The evaluator uses approxSubProbModel instead of iteratedModel in order
  // to account for any constraint recasting within the homotopy sub-problem
  // (in particular, linearized constraints).  The minimizer is, however,
  // passed the original unrelaxed constraint bounds and targets (in the
  // tau_minimizer instantiation in relax_constraints()).

  // set active set vector in approxSubProbModel
  size_t num_fns = sblmInstance->approxSubProbModel.num_functions(),
     num_obj_fns = num_fns - ncnln; // 1 if recast, numUserPrimaryFns if not
  ShortArray local_asv(num_fns, 0);
  for (int i=0; i<ncnln; i++)
    local_asv[num_obj_fns+i] = (needc[i] > 0) ? asv_request : 0;
  ActiveSet local_set
    = sblmInstance->approxSubProbModel.current_response().active_set();
  local_set.request_vector(local_asv);

  // update model variables x from tau_and_x for use in compute_response()
  //RealVector local_des_vars(n-1);
  // WJB: copy vs. view?? copy_data(&tau_and_x[1], n-1, local_des_vars);
  RealVector local_des_vars(Teuchos::View, &tau_and_x[1], n-1);
  sblmInstance->approxSubProbModel.continuous_variables(local_des_vars);

  // compute response
  sblmInstance->approxSubProbModel.compute_response(local_set);
  const Response& resp = sblmInstance->approxSubProbModel.current_response();

  // get constraint relaxation slack vectors
  const RealVector& nli_lower_slack
    = sblmInstance->nonlinIneqLowerBndsSlack;
  const RealVector& nli_upper_slack
    = sblmInstance->nonlinIneqUpperBndsSlack;
  const RealVector& nle_targets_slack = sblmInstance->nonlinEqTargetsSlack;

  // get sizes of vectors
  const size_t& num_nli_constr = sblmInstance->numNonlinearIneqConstraints;
  const size_t& num_nle_constr = sblmInstance->numNonlinearEqConstraints;

  // index offsets for constraints
  size_t i, j, nli_offset = num_obj_fns,
    nle_offset = num_obj_fns + num_nli_constr;

  const double& tau = tau_and_x[0];
  if (asv_request & 1) {
    const RealVector& resp_fn = resp.function_values();

    // nonlinear constraints (based on tau parameter)
    for (i=0; i<num_nli_constr; i++)
      c[i] = resp_fn[nli_offset+i]
	- (1.-tau)*(nli_lower_slack[i] + nli_upper_slack[i]);
  
    // equality targets (based on tau parameter)
    for (i=0; i<num_nle_constr; i++)
      c[num_nli_constr+i] = resp_fn[nle_offset+i]
	- (1.-tau)*nle_targets_slack[i];
#ifdef DEBUG
    for (i=0; i<ncnln; i++)
      Cout << "c[" << i << "] = " << c[i] << std::endl;
#endif // DEBUG
  } // function value computation

  if (asv_request & 2) {
    const RealMatrix& resp_grad = resp.function_gradients();
  
    // gradients of constraints
    size_t cntr = 0;
    for (j=0; j<n; j++) {
      // nonlinear inequality constraints
      for(i=0; i<num_nli_constr; i++)
	cjac[cntr++] = (j == 0) ? nli_lower_slack[i] + nli_upper_slack[i]
	                        : resp_grad(j-1,nli_offset+i);
      // nonlinear equality constraints
      for(i=0; i<num_nle_constr; i++)
	cjac[cntr++] = (j == 0) ? nle_targets_slack[i]
	                        : resp_grad(j-1,nle_offset+i);
    }
#ifdef DEBUG
    for (i=0; i<cntr; i++)
      Cout << "cjac[" << i << "] = " << cjac[i] << std::endl;
#endif // DEBUG
  } // gradient computation
}

} // namespace Dakota
