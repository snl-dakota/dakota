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

extern PRPCache data_pairs;

// initialization of statics
SurrBasedLocalMinimizer* SurrBasedLocalMinimizer::sblmInstance(NULL);


SurrBasedLocalMinimizer::
SurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model):
  SurrBasedMinimizer(problem_db, model), 
  approxSubProbObj(probDescDB.get_short("method.sbl.subproblem_objective")),
  approxSubProbCon(probDescDB.get_short("method.sbl.subproblem_constraints")),
  meritFnType(probDescDB.get_short("method.sbl.merit_function")),
  acceptLogic(probDescDB.get_short("method.sbl.acceptance_logic")),
  trConstraintRelax(probDescDB.get_short("method.sbl.constraint_relax")),
  minimizeCycles(0), penaltyIterOffset(-200), 
  origTrustRegionFactor(
    probDescDB.get_rv("method.trust_region.initial_size")),
  minTrustRegionFactor(
    probDescDB.get_real("method.trust_region.minimum_size")),
  trRatioContractValue(
    probDescDB.get_real("method.trust_region.contract_threshold")),
  trRatioExpandValue(
    probDescDB.get_real("method.trust_region.expand_threshold")),
  gammaContract(
    probDescDB.get_real("method.trust_region.contraction_factor")),
  gammaExpand(probDescDB.get_real("method.trust_region.expansion_factor")),
  softConvLimit(probDescDB.get_ushort("method.soft_convergence_limit")),
  correctionType(probDescDB.get_short("model.surrogate.correction_type"))
{
  // Verify that iteratedModel is a surrogate model so that
  // approximation-related functions are defined.
  if (iteratedModel.model_type() != "surrogate") {
    Cerr << "Error: SurrBasedLocalMinimizer::iteratedModel must be a "
	 << "surrogate model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // alert user to constraint settings
  if (outputLevel >= DEBUG_OUTPUT && numNonlinearConstraints)
    Cout << "\n<<<<< approxSubProbObj  = " << approxSubProbObj
	 << "\n<<<<< approxSubProbCon  = " << approxSubProbCon
	 << "\n<<<<< meritFnType       = " << meritFnType
	 << "\n<<<<< acceptLogic       = " << acceptLogic
	 << "\n<<<<< trConstraintRelax = " << trConstraintRelax << "\n\n";
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
    abort_handler(METHOD_ERROR);
  }
#endif

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


void SurrBasedLocalMinimizer::initialize_sub_model()
{
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
    SizetArray recast_vars_comps_total;  // default: empty; no change in size
    BitArray all_relax_di, all_relax_dr; // default: empty; no discrete relax
    BoolDequeArray nonlinear_resp_map(num_recast_primary+num_recast_secondary);
    if (approxSubProbObj == ORIGINAL_PRIMARY) {
      for (i=0; i<num_recast_primary; ++i) {
	recast_primary_resp_map[i].resize(1);
	recast_primary_resp_map[i][0] = i;
	nonlinear_resp_map[i].resize(1);
	nonlinear_resp_map[i][0] = false;
      }
    }
    else if (approxSubProbObj == SINGLE_OBJECTIVE) {
      recast_primary_resp_map[0].resize(numUserPrimaryFns);
      nonlinear_resp_map[0].resize(numUserPrimaryFns);
      for (i=0; i<numUserPrimaryFns; ++i) {
	recast_primary_resp_map[0][i] = i;
	nonlinear_resp_map[0][i] = !optimizationFlag; // nonlinear if NLS->Opt
      }
    }
    else { // LAGRANGIAN_OBJECTIVE or AUGMENTED_LAGRANGIAN_OBJECTIVE
      recast_primary_resp_map[0].resize(numFunctions);
      nonlinear_resp_map[0].resize(numFunctions);
      for (i=0; i<numFunctions; ++i) {
	recast_primary_resp_map[0][i] = i;
	nonlinear_resp_map[0][i]
	  = (approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE &&
	     i >= numUserPrimaryFns) ? true : false;
      }
    }
    if (approxSubProbCon != NO_CONSTRAINTS) {
      for (i=0; i<num_recast_secondary; ++i) {
	recast_secondary_resp_map[i].resize(1);
	recast_secondary_resp_map[i][0] = i + numUserPrimaryFns;
	nonlinear_resp_map[i+num_recast_primary].resize(1);
	nonlinear_resp_map[i+num_recast_primary][0] = false;
      }
    }
    // iteratedModel becomes the sub-model of a RecastModel:
    size_t recast_offset
      = (approxSubProbCon == NO_CONSTRAINTS) ? 0 : numNonlinearIneqConstraints;
    // no Hessians in approx_subprob evaluators
    short recast_resp_order = 3;
    void (*set_recast) (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
    // = (!optimizationFlag && approxSubProbObj == SINGLE_OBJECTIVE &&
    //    iteratedModel.hessian_type() == "none") ? gnewton_set_recast : NULL;

    approxSubProbModel.assign_rep(new RecastModel(iteratedModel,
      recast_vars_map, recast_vars_comps_total, all_relax_di, all_relax_dr,
      false, NULL, set_recast, recast_primary_resp_map,
      recast_secondary_resp_map, recast_offset, recast_resp_order,
      nonlinear_resp_map, approx_subprob_objective_eval,
      approx_subprob_constraint_eval), false);

    // these formulations have converted multiple objectives or
    // calibration terms to a single objective
    if (approxSubProbObj != ORIGINAL_PRIMARY)
      approxSubProbModel.primary_fn_type(OBJECTIVE_FNS);
  }

  // activate warm starting of accumulated data (i.e., quasi-Newton Hessians).
  // Note: this is best done at the iteratedModel level, to avoid any
  // interaction with penalty and multiplier states in approxSubProbModel.
  approxSubProbModel.warm_start_flag(true);
}


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


void SurrBasedLocalMinimizer::initialize_multipliers()
{
  // initialize Lagrange multipliers
  size_t num_multipliers = numNonlinearEqConstraints;
  for (size_t i=0; i<numNonlinearIneqConstraints; ++i) {
    if (origNonlinIneqLowerBnds[i] > -bigRealBoundSize) // g has a lower bound
      ++num_multipliers;
    if (origNonlinIneqUpperBnds[i] <  bigRealBoundSize) // g has an upper bound
      ++num_multipliers;
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
}


void SurrBasedLocalMinimizer::pre_run()
{
  // reset convergence controls in case of multiple executions
  if (converged())
    reset();

  // need copies of initial point and initial global bounds, since iteratedModel
  // continuous vars will be reset to the TR center and iteratedModel bounds
  // will be reset to the TR bounds
  copy_data(iteratedModel.continuous_variables(),    initialPoint);
  copy_data(iteratedModel.continuous_lower_bounds(), globalLowerBnds);
  copy_data(iteratedModel.continuous_upper_bounds(), globalUpperBnds);
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

  sblmInstance = this;
  while (!converged()) {

    // Compute trust region bounds.  If the trust region extends outside
    // the global bounds, then truncate to the global bounds.
    update_trust_region();

    // Build new approximations and compute corrections for use within
    // approxSubProbMinimizer.run() (unless previous build can be reused)
    build(); // Build the approximation and perform hard convergence check

    if (!converged()) {
      minimize(); // run approxSubProbMinimizer and update responseStarApprox
      verify(); // eval responseStarTruth, update TR, perform other conv checks
    }
  }
}


void SurrBasedLocalMinimizer::post_run(std::ostream& s)
{
  // SBLM is complete: write out the convergence condition
  // (final results output in derived post_run())
  s << "\nSurrogate-Based Optimization Complete:\n";
  print_convergence_code(s, converged());
  s << "Total Number of Trust Region Minimizations Performed = "
    << globalIterCount << std::endl;

  Minimizer::post_run(s);
}


void SurrBasedLocalMinimizer::
print_convergence_code(std::ostream& s, unsigned short code)
{
  // these can be overlaid:
  if (code & MIN_TR_CONVERGED)   s << "Minimum Trust Region Bounds Reached\n";
  if (code & MAX_ITER_CONVERGED) s << "Exceeded Maximum Number of Iterations\n";

  // these two are exclusive:
  if (code & HARD_CONVERGED)
    s << "Hard Convergence: Norm of Projected Lagrangian Gradient <= "
      << "Conv Tol\n";
  else if (code & SOFT_CONVERGED)
    s << "Soft Convergence: Progress Between " << softConvLimit
      << " Successive Iterations <= Conv Tol\n";
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
  for (i=0; i<numContinuousVars; ++i) {
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
    // compute 1-sided trust region offset: scale is always relative to the
    // global bounds, not the parent bounds
    Real tr_offset = tr_data.trust_region_factor() / 2. * 
      ( globalUpperBnds[i] - globalLowerBnds[i] );
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
  //if (cv_truncation) tr_data.set_status_bits(NEW_CENTER);//handled in SBLD set
  tr_data.reset_status_bits(NEW_TR_FACTOR); // TR updates applied; reset bit

  // a flag for global approximations defining the availability of the
  // current iterate in the DOE/DACE evaluations: CCD/BB DOE evaluates the
  // center of the sampled region, whereas LHS/OA/QMC/CVT DACE does not.
  //daceCenterPtFlag
  //  = (daceCenterEvalFlag && !tr_lower_truncation && !tr_upper_truncation);

  // Output the trust region bounds
  size_t wpp9 = write_precision+9;
  Cout << "\n**************************************************************"
       << "************\nBegin SBLM Iteration Number " << globalIterCount+1
       << "\n\nCurrent Trust Region for surrogate model (form "
       << tr_data.approx_model_form() + 1;
  if (tr_data.approx_model_level() != _NPOS)
    Cout << ", level " << tr_data.approx_model_level() + 1;
  Cout << ")\n                 ";
  if (tr_lower_truncation) Cout << std::setw(wpp9) << "Lower (truncated)";
  else                     Cout << std::setw(wpp9) << "Lower";
  if (cv_truncation)       Cout << std::setw(wpp9) << "Center (truncated)";
  else                     Cout << std::setw(wpp9) << "Center";
  if (tr_upper_truncation) Cout << std::setw(wpp9) << "Upper (truncated)";
  else                     Cout << std::setw(wpp9) << "Upper";
  Cout << '\n';
  const RealVector&     cv_center = tr_data.c_vars_center();
  const RealVector& tr_lower_bnds = tr_data.tr_lower_bounds();
  const RealVector& tr_upper_bnds = tr_data.tr_upper_bounds();
  StringMultiArrayConstView c_vars_labels
    = iteratedModel.continuous_variable_labels();
  for (i=0; i<numContinuousVars; ++i)
    Cout << std::setw(16) << c_vars_labels[i] << ':' << std::setw(wpp9)
	 << tr_lower_bnds[i] << std::setw(wpp9) << cv_center[i]
	 << std::setw(wpp9) << tr_upper_bnds[i] << '\n';
  Cout << "****************************************************************"
       << "**********\n";
}


void SurrBasedLocalMinimizer::
update_approx_sub_problem(SurrBasedLevelData& tr_data)
{
  approxSubProbModel.active_variables(tr_data.vars_center());
  approxSubProbModel.continuous_lower_bounds(tr_data.tr_lower_bounds());
  approxSubProbModel.continuous_upper_bounds(tr_data.tr_upper_bounds());

  if ( trConstraintRelax > NO_RELAX ) // relax constraints if requested
    relax_constraints(tr_data);
}


void SurrBasedLocalMinimizer::minimize()
{
  Cout << "\n>>>>> Starting approximate optimization cycle.\n";
  iteratedModel.component_parallel_mode(SURROGATE_MODEL);
  iteratedModel.surrogate_response_mode(AUTO_CORRECTED_SURROGATE);

  ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
  approxSubProbMinimizer.run(pl_iter); // pl_iter required for hierarchical

  Cout << "\n<<<<< Approximate optimization cycle completed.\n";
  ++minimizeCycles;  // number of trust-region minimization cycles
  ++globalIterCount; // total iterations performed
}


/** Assess acceptance of SBLM iterate (trust region ratio or filter)
    and compute soft convergence metrics (number of consecutive
    failures, min trust region size, etc.) to assess whether the
    convergence rate has decreased to a point where the process should
    be terminated (diminishing returns). */
void SurrBasedLocalMinimizer::
compute_trust_region_ratio(SurrBasedLevelData& tr_data, bool check_interior)
{
  /*
  // We need both data sets consistently corrected, but for > 2 levels, do
  // we want both sets corrected all the way to HF, or the approx corrected
  // 1 level only for consistency with uncorrected truth?
  const RealVector& fns_center_truth
    = tr_data.response_center(UNCORR_TRUTH_RESPONSE).function_values();
  const RealVector& fns_star_truth
    = tr_data.response_star(UNCORR_TRUTH_RESPONSE).function_values();
  const RealVector& fns_center_approx
    = tr_data.response_center(CORR_APPROX_RESPONSE).function_values();
  const RealVector& fns_star_approx
    = tr_data.response_star(CORR_APPROX_RESPONSE).function_values();
  */

  // Current approach corrects all the way to the top of the hierarchy:
  const RealVector& fns_center_truth
    = tr_data.response_center(CORR_TRUTH_RESPONSE).function_values();
  const RealVector& fns_star_truth
    = tr_data.response_star(CORR_TRUTH_RESPONSE).function_values();
  const RealVector& fns_center_approx
    = tr_data.response_center(CORR_APPROX_RESPONSE).function_values();
  const RealVector& fns_star_approx
    = tr_data.response_star(CORR_APPROX_RESPONSE).function_values();

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
  if (meritFnType == LAGRANGIAN_MERIT) { // penalty-free (like filter)

    // This approach has been observed to be ineffective since NNLS/BVLS
    // estimates of lagrangeMult are often zero away from a constrained min,
    // resulting in merit fns that omit important constraint violations.

    // evaluate each merit function with the same lagrangeMult estimates
    // (updated from responseCenterTruth in hard_convergence_check()).
    merit_fn_center_truth = lagrangian_merit(fns_center_truth, sense, wts,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_star_truth = lagrangian_merit(fns_star_truth, sense, wts,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_center_approx = lagrangian_merit(fns_center_approx, sense, wts,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_star_approx = lagrangian_merit(fns_star_approx, sense, wts,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);

    //IntRespMCIter d_it;
    //for (i=0, d_it=dace_responses.begin(); i<num_dace_responses; ++i, ++d_it){
    //  Real merit_dace_truth = lagrangian_merit(
    //    d_it->second.function_values(), wts, origNonlinIneqLowerBnds,
    //    origNonlinIneqUpperBnds, origNonlinEqTargets);
    //  if (merit_dace_truth < merit_fn_star_truth) {
    //    Cerr << "Warning: \n";
    //    tr_data.vars_star(...);
    //    merit_fn_star_truth = merit_dace_truth;
    //    tr_data.scale_trust_region(...);
    //  }
    //}
  }
  else if (meritFnType == AUGMENTED_LAGRANGIAN_MERIT) {

    // evaluate each merit function with the same augLagrangeMult estimates
    // (updated from fns_center_truth on initial iteration and from
    // fns_star_truth for accepted steps) and penaltyParameter (updated
    // if no reduction in constraint violation).
    merit_fn_center_truth = augmented_lagrangian_merit(fns_center_truth,
      sense, wts, origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
      origNonlinEqTargets);
    merit_fn_star_truth = augmented_lagrangian_merit(fns_star_truth, sense, wts,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets);
    merit_fn_center_approx = augmented_lagrangian_merit(fns_center_approx,
      sense, wts, origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
      origNonlinEqTargets);
    merit_fn_star_approx = augmented_lagrangian_merit(fns_star_approx,
      sense, wts, origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
      origNonlinEqTargets);
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
    // is ramped exponentially using an iteration counter.
    if (numNonlinearConstraints)
      update_penalty(fns_center_truth, fns_star_truth);

    // evaluate each merit function with updated/adapted penaltyParameter
    merit_fn_center_truth  = penalty_merit(fns_center_truth, sense, wts);
    merit_fn_star_truth    = penalty_merit(fns_star_truth, sense, wts);
    merit_fn_center_approx = penalty_merit(fns_center_approx, sense, wts);
    merit_fn_star_approx   = penalty_merit(fns_star_approx, sense, wts);
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Response truth:\ncenter = "
	 << tr_data.response_center(CORR_TRUTH_RESPONSE) << "star = "
	 << tr_data.response_star(CORR_TRUTH_RESPONSE)
	 << "Response approx:\ncenter = "
	 << tr_data.response_center(CORR_APPROX_RESPONSE) << "star = "
	 << tr_data.response_star(CORR_APPROX_RESPONSE)
	 << "Merit fn truth:  center = " << merit_fn_center_truth << " star = "
	 << merit_fn_star_truth << "\nMerit fn approx: center = "
	 << merit_fn_center_approx << " star = " << merit_fn_star_approx <<'\n';

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
  if (acceptLogic == FILTER) {
    if (tr_data.filter_size() == 0) // initialize if needed
      initialize_filter(tr_data, fns_center_truth);
    accept_step = update_filter(tr_data, fns_star_truth);
  }
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
    // Update the trust region size depending on the accuracy of the approximate
    // model. Note: If eta_1 < tr_ratio < eta_2, trustRegionFactor does not
    // change where eta_1 = trRatioContractValue and eta_2 = trRatioExpandValue
    // Recommended values from Conn/Gould/Toint are: eta_1 = 0.05, eta_2 = 0.90
    // For SBLM, the following are working better:   eta_1 = 0.25, eta_2 = 0.75
    // More experimentation is needed.
    Cout << "\n<<<<< Trust Region Ratio = " << tr_ratio << ":\n<<<<< ";
    if (tr_ratio <= trRatioContractValue) { // accept optimum, shrink TR
      tr_data.scale_trust_region_factor(gammaContract);
      Cout << "Marginal Accuracy, ACCEPT Step, REDUCE Trust Region Size\n\n";
    }
    else if (std::fabs(1.-tr_ratio) <= 1.-trRatioExpandValue) {//accept & expand
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
      if (check_interior) {
	const RealVector& c_vars_star   = tr_data.c_vars_star();
	const RealVector& tr_lower_bnds = tr_data.tr_lower_bounds();
	const RealVector& tr_upper_bnds = tr_data.tr_upper_bounds();
	for (size_t i=0; i<numContinuousVars; ++i)
	  if ( c_vars_star[i] > tr_upper_bnds[i] - constraintTol ||
	       c_vars_star[i] < tr_lower_bnds[i] + constraintTol )
	    boundary_pt_flag = true;
      }

      if (check_interior && !boundary_pt_flag)
	Cout << "Excellent Accuracy, Iterate in Trust Region Interior, "
	     << "ACCEPT Step, RETAIN Trust Region Size\n\n";
      else {
	tr_data.scale_trust_region_factor(gammaExpand);
	Cout << "Excellent Accuracy, ACCEPT Step, INCREASE Trust Region Size"
	     << "\n\n";
      }
    }
    else // accept optimum, retain current TR
      Cout <<"Satisfactory Accuracy, ACCEPT Step, RETAIN Trust Region Size\n\n";

    // update center vars, set NEW_CENTER, unset CANDIDATE_STATE
    tr_data.vars_center(tr_data.vars_star());
    // update response center from star, allowing for inconsistent data sets.
    // Note: we always do this even though build() may supplant with full center
    // data set on the next iteration, in order to simplify conditional logic,
    // e.g., iter results to OutputManager, final results to bestResponseArray
    // at convergence.
    Response& uncorr_resp_star = tr_data.response_star(UNCORR_TRUTH_RESPONSE);
    if (!uncorr_resp_star.is_null()) {
      Response& uncorr_resp_center
	= tr_data.response_center(UNCORR_TRUTH_RESPONSE);
      uncorr_resp_center.reset(); // zero out all data
      uncorr_resp_center.function_values(uncorr_resp_star.function_values());
    }
    IntResponsePair& corr_pair_star
      = tr_data.response_star_pair(CORR_TRUTH_RESPONSE);
    if (!corr_pair_star.second.is_null()) {
      tr_data.response_center_id(corr_pair_star.first, CORR_TRUTH_RESPONSE);
      Response& corr_resp_center = tr_data.response_center(CORR_TRUTH_RESPONSE);
      corr_resp_center.reset(); // zero out all data
      corr_resp_center.function_values(corr_pair_star.second.function_values());
    }
  }
  else {
    // If the step is rejected, then retain the current design variables
    // and shrink the TR size.
    tr_data.reset_status_bits(CENTER_STATE | CANDIDATE_STATE);
    tr_data.scale_trust_region_factor(gammaContract);
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
  //
  // Merit fn case: soft convergence counter is incremented if insufficient
  // relative or absolute improvement in actual or approx.
  if (accept_step) {
    Real rel_numer = ( std::fabs(merit_fn_center_truth) > DBL_MIN ) ?
      std::fabs(numerator / merit_fn_center_truth)    : std::fabs(numerator);
    Real rel_denom = ( std::fabs(merit_fn_center_approx) > DBL_MIN ) ?
      std::fabs(denominator / merit_fn_center_approx) : std::fabs(denominator);
    if ( numerator   <= 0. || rel_numer < convergenceTol ||
	 denominator <= 0. || rel_denom < convergenceTol )
      tr_data.increment_soft_convergence_count(); // ++ soft conv counter
    else
      tr_data.reset_soft_convergence_count();     // reset soft conv cntr to 0
  }
  else
    tr_data.increment_soft_convergence_count();   // ++ soft conv counter
}


/** The hard convergence check computes the gradient of the merit
    function at the trust region center, performs a projection for
    active bound constraints (removing any gradient component directed
    into an active bound), and signals convergence if the 2-norm of
    this projected gradient is less than convergenceTol. */
void SurrBasedLocalMinimizer::
hard_convergence_check(SurrBasedLevelData& tr_data,
		       const RealVector& lower_bnds,
		       const RealVector& upper_bnds)
{
  const Response& response_truth = tr_data.response_center(CORR_TRUTH_RESPONSE);
  const RealVector&    fns_truth = response_truth.function_values();

  // --------------------------------------
  // Initialize/update Lagrange multipliers
  // --------------------------------------
  // These updates are only performed for new iterates from accepted steps
  // (hard_convergence_check() invoked only if trustRegionData.new_center()).

  // initialize augmented Lagrangian multipliers
  if (minimizeCycles == 0 && numNonlinearConstraints &&
      ( meritFnType       == AUGMENTED_LAGRANGIAN_MERIT ||
	approxSubProbObj  == AUGMENTED_LAGRANGIAN_OBJECTIVE ) )
    update_augmented_lagrange_multipliers(fns_truth);

  // hard convergence requires truth gradients and zero constraint violation
  if ( !(truthSetRequest & 2) )
    return;
  // update standard Lagrangian multipliers: solve non-negative/
  // bound-constrained LLS for lagrangeMult
  bool constraint_viol = (constraint_violation(fns_truth, constraintTol) > 0.);
  if (meritFnType      == LAGRANGIAN_MERIT     ||
      approxSubProbObj == LAGRANGIAN_OBJECTIVE || !constraint_viol)
    update_lagrange_multipliers(fns_truth,
				response_truth.function_gradients());
  if (constraint_viol)
    return;

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
  lagrangian_gradient(fns_truth, response_truth.function_gradients(),
		      iteratedModel.primary_response_fn_sense(),
		      iteratedModel.primary_response_fn_weights(),
		      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
		      origNonlinEqTargets, merit_fn_grad);
  //else if (meritFnType == AUGMENTED_LAGRANGIAN_MERIT)
  //  augmented_lagrangian_gradient(fns_truth, grads_truth, sense, wts,
  //    origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
  //    origNonlinEqTargets, merit_fn_grad);
  //else
  //  penalty_gradient(fns_truth, grads_truth, sense, wts,
  //    origNonlinIneqLowerBnds, origNonlinIneqUpperBnds,
  //    origNonlinEqTargets, merit_fn_grad);

  // Compute norm of projected merit function gradient
  Real merit_fn_grad_norm = 0.0;
  const RealVector& c_vars = tr_data.c_vars_center();
  for (size_t i=0; i<numContinuousVars; ++i) {
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
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In hard convergence check: merit_fn_grad_norm =  "
	 << merit_fn_grad_norm << '\n';
  if (merit_fn_grad_norm < convergenceTol)
    tr_data.set_status_bits(HARD_CONVERGED);
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
    penaltyParameter = std::exp( 2.1 + (double)minimizeCycles/10.0 );
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
    const BoolDeque& sense = iteratedModel.primary_response_fn_sense();
    const RealVector&  wts = iteratedModel.primary_response_fn_weights();
    Real obj_delta = objective(fns_star_truth,   sense, wts)
                   - objective(fns_center_truth, sense, wts);
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
      int new_offset = min_iter - minimizeCycles;
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
    // penaltyIterOffset and minimizeCycles are capped at 200 (e^20 =~ 5e8) to
    // reduce problems w/ small infeasibilities and prevent numerical overflow.
    penaltyParameter = (minimizeCycles < 200)
      ? std::exp((double)(minimizeCycles + penaltyIterOffset + 5)/10.)
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
    //penaltyParameter = std::min(penaltyParameter, 1.e+20); // cap max penalty?
    Real mu = 1./2./penaltyParameter; // conversion between r_p and mu penalties
    etaSequence = eta*std::pow(mu, alphaEta);
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Penalty updated: " << penaltyParameter << '\n';
    if (meritFnType      == AUGMENTED_LAGRANGIAN_MERIT ||
	approxSubProbObj == AUGMENTED_LAGRANGIAN_OBJECTIVE)
      Cout << "eta updated: " << etaSequence << '\n';
  }
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
  const RealVector& surrogate_fns = surrogate_response.function_values();
  const ShortArray& recast_asv = recast_response.active_set_request_vector();
  if (sblmInstance->approxSubProbObj == ORIGINAL_PRIMARY) {
    for (size_t i=0; i<sblmInstance->numUserPrimaryFns; ++i) {
      if (recast_asv[i] & 1)
	recast_response.function_value(surrogate_fns[i], i);
      if (recast_asv[i] & 2)
	recast_response.function_gradient(
	  surrogate_response.function_gradient_view(i), i );
      if (recast_asv[i] & 4)
	recast_response.function_hessian(
	  surrogate_response.function_hessian(i), i );
    }
  }
  else {
    // use approxSubProbModel to retrieve bounds/targets in order to capture
    // any relaxation, except for the case where the subproblem has been
    // recast without constraints.  In this case, relaxation is not active,
    // the approxSubProbModel has empty bounds/targets, and the original
    // bounds/targets are used.
    const BoolDeque& sense
      = sblmInstance->iteratedModel.primary_response_fn_sense();
    const RealVector& wts
      = sblmInstance->iteratedModel.primary_response_fn_weights();
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
	recast_fn = sblmInstance->objective(surrogate_fns, sense, wts);
	break;
      case LAGRANGIAN_OBJECTIVE:
	recast_fn = sblmInstance->lagrangian_merit(surrogate_fns, sense, wts,
	  nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts);
	break;
      case AUGMENTED_LAGRANGIAN_OBJECTIVE:
	recast_fn = sblmInstance->augmented_lagrangian_merit(surrogate_fns,
	  sense, wts, nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts);
	break;
      }
      recast_response.function_value(recast_fn, 0);
    }

    if (recast_asv[0] & 2) {
      RealVector recast_grad;
      const RealMatrix& surrogate_grads
	= surrogate_response.function_gradients();
      switch (sblmInstance->approxSubProbObj) {
      case SINGLE_OBJECTIVE:
	sblmInstance->objective_gradient(surrogate_fns, surrogate_grads, sense,
	  wts, recast_grad);
	break;
      case LAGRANGIAN_OBJECTIVE:
	sblmInstance->lagrangian_gradient(surrogate_fns, surrogate_grads, sense,
	  wts, nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts, recast_grad);
	break;
      case AUGMENTED_LAGRANGIAN_OBJECTIVE:
	sblmInstance->augmented_lagrangian_gradient(surrogate_fns,
	  surrogate_grads, sense, wts, nln_ineq_l_bnds, nln_ineq_u_bnds,
	  nln_eq_tgts, recast_grad);
	break;
      }
      recast_response.function_gradient(recast_grad, 0);
    }

    if (recast_asv[0] & 4) {
      RealSymMatrix recast_hess;
      const RealMatrix& surrogate_grads
	= surrogate_response.function_gradients();
      const RealSymMatrixArray& surrogate_hessians
	= surrogate_response.function_hessians();
      switch (sblmInstance->approxSubProbObj) {
      case SINGLE_OBJECTIVE:
	sblmInstance->objective_hessian(surrogate_fns, surrogate_grads,
	  surrogate_hessians, sense, wts, recast_hess);
	break;
      case LAGRANGIAN_OBJECTIVE:
	sblmInstance->lagrangian_hessian(surrogate_fns, surrogate_grads,
	  surrogate_hessians, sense,
	  wts, nln_ineq_l_bnds, nln_ineq_u_bnds, nln_eq_tgts, recast_hess);
	break;
      case AUGMENTED_LAGRANGIAN_OBJECTIVE:
	sblmInstance->augmented_lagrangian_hessian(surrogate_fns,
	  surrogate_grads, surrogate_hessians, sense, wts, nln_ineq_l_bnds,
	  nln_ineq_u_bnds, nln_eq_tgts, recast_hess);
	break;
      }
      recast_response.function_hessian(recast_hess, 0);
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
  const RealVector& surrogate_fns = surrogate_response.function_values();

  switch (sblmInstance->approxSubProbCon) {

  case ORIGINAL_CONSTRAINTS: {
    //
    // Note: constraints are NOT converted to std form as done within merit fns
    //
    for (i=0; i<num_recast_cons; ++i) {
      size_t recast_i = i + num_recast_primary,
	     surr_i   = i + sblmInstance->numUserPrimaryFns;
      if (recast_asv[recast_i] & 1)
	recast_response.function_value(surrogate_fns[surr_i], recast_i);
      if (recast_asv[recast_i] & 2)
	recast_response.function_gradient(
	  surrogate_response.function_gradient_view(surr_i), recast_i );
      if (recast_asv[recast_i] & 4)
	recast_response.function_hessian(
	  surrogate_response.function_hessian(surr_i), recast_i );
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
    SurrBasedLevelData& tr_data = sblmInstance->trust_region();
    const RealVector& center_c_vars = tr_data.c_vars_center();
    const Response&   center_approx_resp
      = tr_data.response_center(CORR_APPROX_RESPONSE);
    const RealVector& center_approx_fns = center_approx_resp.function_values();
    const RealMatrix& center_approx_grads
      = center_approx_resp.function_gradients();
    const RealVector& c_vars = recast_vars.continuous_variables();

    size_t j, num_recast_vars
      = recast_response.active_set_derivative_vector().size();
    for (i=0; i<num_recast_cons; ++i) {
      size_t recast_i = i + num_recast_primary,
	     surr_i   = i + sblmInstance->numUserPrimaryFns;
      if (recast_asv[recast_i] & 1) {
	Real sum = center_approx_fns[surr_i];
	for (j=0; j<num_recast_vars; ++j)
	  sum += center_approx_grads(j,surr_i) * (c_vars[j] - center_c_vars[j]);
	recast_response.function_value(sum, recast_i);
      }
      if (recast_asv[recast_i] & 2)
	recast_response.function_gradient(
	  center_approx_resp.function_gradient_view(surr_i), recast_i);
      if (recast_asv[recast_i] & 4) {
	RealSymMatrix recast_hess
	  = recast_response.function_hessian_view(recast_i);
	recast_hess = 0.; // linearized constraints
      }
    }
#ifdef DEBUG
    Cout << "center_c_vars =\n" << center_c_vars << "c_vars =\n" << c_vars
	 << "center_approx_fns =\n" << center_approx_fns
	 << "center_approx_grads =\n" << center_approx_grads
	 << "recast_response =\n" << recast_response << std::endl;
#endif
    break;
  }

  case NO_CONSTRAINTS:
    break;
  }
}


bool SurrBasedLocalMinimizer::
find_response(const Variables& search_vars, Response& search_resp,
	      const String& search_id, short set_request)
{
  bool found = false;

  // search for fn vals, grads, and Hessians separately since they may
  // be different fn evals
  ActiveSet search_set = search_resp.active_set(); // copy
  search_set.request_values(1);
  PRPCacheHIter cache_it
    = lookup_by_val(data_pairs, search_id, search_vars, search_set);
  if (cache_it != data_pairs.get<hashed>().end()) {
    search_resp.function_values(cache_it->response().function_values());
    if (set_request & 2) {
      search_set.request_values(2);
      cache_it = lookup_by_val(data_pairs, search_id, search_vars, search_set);
      if (cache_it != data_pairs.get<hashed>().end()) {
	search_resp.function_gradients(
	  cache_it->response().function_gradients());
	if (set_request & 4) {
	  search_set.request_values(4);
	  cache_it
	    = lookup_by_val(data_pairs, search_id, search_vars, search_set);
	  if (cache_it != data_pairs.get<hashed>().end()) {
	    search_resp.function_hessians(
	      cache_it->response().function_hessians());
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
  return found;
}


void SurrBasedLocalMinimizer::relax_constraints(SurrBasedLevelData& tr_data)
{
  // NOTE 1: this needs revision in the case where the surrogates could
  // be expensive (e.g., multifidelity).  Resort to linearized surrogates
  // (SQP-like) in this case.

  // NOTE 2: approxSubProbModel (and iteratedModel if no recasting) contains
  // the relaxed constraint bounds and targets, and is used in the approximate
  // subproblem cycles.  The original bounds and targets are catalogued in SBLM
  // class member variables in the ctor.

  // get current function/constraint values
  const RealVector& fns_center_truth
    = tr_data.response_center(CORR_TRUTH_RESPONSE).function_values();

  // initial relaxation data during the first SBLM iteration
  if (minimizeCycles == 0) {

    // initialize inequality constraint slack vectors
    if (numNonlinearIneqConstraints) {
      nonlinIneqLowerBndsSlack.sizeUninitialized(numNonlinearIneqConstraints);
      nonlinIneqLowerBndsSlack = 0.;
      nonlinIneqUpperBndsSlack.sizeUninitialized(numNonlinearIneqConstraints);
      nonlinIneqUpperBndsSlack = 0.;
      for (size_t i=0; i<numNonlinearIneqConstraints; ++i) {
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
      for (size_t i=0; i<numNonlinearEqConstraints; ++i) {
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
    copy_data_partial(tr_data.c_vars_center(),   tau_and_x_initial, 1);
    copy_data_partial(tr_data.tr_lower_bounds(), tau_and_x_lower_bnds, 1);
    copy_data_partial(tr_data.tr_upper_bounds(), tau_and_x_upper_bnds, 1);
    
    // setup optimization problem for updating tau
#ifdef HAVE_NPSOL
    tau_minimizer.assign_rep(new NPSOLOptimizer(tau_and_x_initial,
      tau_and_x_lower_bnds, tau_and_x_upper_bnds, lin_ineq_coeffs,
      lin_ineq_lower_bnds, lin_ineq_lower_bnds, lin_eq_coeffs, lin_eq_targets,
      origNonlinIneqLowerBnds, origNonlinIneqUpperBnds, origNonlinEqTargets,
      hom_objective_eval, hom_constraint_eval, deriv_level, conv_tol), false);
#endif

    // find optimum tau by solving approximate subproblem
    // (pl_iter could be needed for hierarchical surrogate case, in which case
    // {set,free}_communicators must be added)
    //ParLevLIter pl_iter = methodPCIter->mi_parallel_level_iterator(miPLIndex);
    tau_minimizer.run();//(pl_iter);

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
	for(size_t i=0; i<numNonlinearIneqConstraints; ++i) {
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
	for(size_t i=0; i<numNonlinearEqConstraints; ++i)
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
    for (int i=1; i<n; ++i)
      grad_f[i] = 0.;  // d(-tau)/dx = 0 (tau does not depend on other vars)
#ifdef DEBUG
    for (int i=0; i<n; ++i)
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
  for (int i=0; i<ncnln; ++i)
    local_asv[num_obj_fns+i] = (needc[i] > 0) ? asv_request : 0;
  ActiveSet local_set
    = sblmInstance->approxSubProbModel.current_response().active_set();
  local_set.request_vector(local_asv);

  // update model variables x from tau_and_x for use in evaluate()
  //RealVector local_des_vars(n-1);
  // WJB: copy vs. view?? copy_data(&tau_and_x[1], n-1, local_des_vars);
  RealVector local_des_vars(Teuchos::View, &tau_and_x[1], n-1);
  sblmInstance->approxSubProbModel.continuous_variables(local_des_vars);

  // compute response
  sblmInstance->approxSubProbModel.evaluate(local_set);
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
    for (i=0; i<num_nli_constr; ++i)
      c[i] = resp_fn[nli_offset+i]
	- (1.-tau)*(nli_lower_slack[i] + nli_upper_slack[i]);
  
    // equality targets (based on tau parameter)
    for (i=0; i<num_nle_constr; ++i)
      c[num_nli_constr+i] = resp_fn[nle_offset+i]
	- (1.-tau)*nle_targets_slack[i];
#ifdef DEBUG
    for (i=0; i<ncnln; ++i)
      Cout << "c[" << i << "] = " << c[i] << std::endl;
#endif // DEBUG
  } // function value computation

  if (asv_request & 2) {
    const RealMatrix& resp_grad = resp.function_gradients();
  
    // gradients of constraints
    size_t cntr = 0;
    for (j=0; j<n; ++j) {
      // nonlinear inequality constraints
      for(i=0; i<num_nli_constr; ++i)
	cjac[cntr++] = (j == 0) ? nli_lower_slack[i] + nli_upper_slack[i]
	                        : resp_grad(j-1,nli_offset+i);
      // nonlinear equality constraints
      for(i=0; i<num_nle_constr; ++i)
	cjac[cntr++] = (j == 0) ? nle_targets_slack[i]
	                        : resp_grad(j-1,nle_offset+i);
    }
#ifdef DEBUG
    for (i=0; i<cntr; ++i)
      Cout << "cjac[" << i << "] = " << cjac[i] << std::endl;
#endif // DEBUG
  } // gradient computation
}

} // namespace Dakota
