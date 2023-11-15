/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevBLUESampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"
#include "SharedPolyApproxData.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevBLUESampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevBLUESampling::
NonDMultilevBLUESampling(ProblemDescDB& problem_db, Model& model):
  NonDNonHierarchSampling(problem_db, model)//, multiStartACV(true)
{
  mlmfSubMethod = problem_db.get_ushort("method.sub_method");

  // SDP versus conventional NLP handled by optSubProblemSolver
  //optSubProblemSolver = sub_optimizer_select(
  //  probDescDB.get_ushort("method.nond.opt_subproblem_solver"),SUBMETHOD_SDP);

  if (maxFunctionEvals == SZ_MAX) // accuracy constraint (convTol)
    optSubProblemForm = N_GROUP_LINEAR_OBJECTIVE;
  else                     // budget constraint (maxFunctionEvals)
    optSubProblemForm = N_GROUP_LINEAR_CONSTRAINT;

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "ML BLUE sub-method selection = " << mlmfSubMethod
	 << " sub-method formulation = " << optSubProblemForm
	 << " sub-problem solver = "     << optSubProblemSolver << std::endl;

  // approximation set for ACV includes all approximations
  // defining approxSet allows reuse of functions across ACV and GenACV
  //approxSet.resize(numApprox);
  //for (size_t i=0; i<numApprox; ++i)
  //  approxSet[i] = i;

  // tensor product of order 1 to enumerate approximation groups
  // *** TO DO: need a throttle specification
  UShort2DArray tp;  UShortArray tp_orders(numApprox+1, 1);
  Pecos::SharedPolyApproxData::
    tensor_product_multi_index(tp_orders, tp, true);
  tp.erase(tp.begin()); // discard empty group (all 0's)

  size_t g, m, num_models;
  numGroups = tp.size();  modelGroups.resize(numGroups);
  for (g=0; g<numGroups; ++g) {
    const UShortArray& tp_g = tp[g];
    UShortArray&    group_g = modelGroups[g];
    for (m=0; m<=numApprox; ++m)
      if (tp_g[m]) group_g.push_back(m);
  }

  modelGroupCost.sizeUninitialized(numGroups);
  for (g=0; g<numGroups; ++g) {
    const UShortArray& models = modelGroups[g];
    Real& group_cost = modelGroupCost[g];
    group_cost = 0.; num_models = models.size();
    for (m=0; m<num_models; ++m)
      group_cost += sequenceCost[models[m]];
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "modelGroups:\n" << modelGroups
	 << "modelGroupCost:\n" << modelGroupCost;

  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    numGroups, pilotSamples);

  size_t max_ps = find_max(pilotSamples);
  if (max_ps) maxEvalConcurrency *= max_ps;
}


NonDMultilevBLUESampling::~NonDMultilevBLUESampling()
{ }


void NonDMultilevBLUESampling::core_run()
{
  switch (pilotMgmtMode) {
  case  ONLINE_PILOT: // iterated ML BLUE (default)
    ml_blue_online_pilot();     break;
  case OFFLINE_PILOT: // computes perf for offline pilot/Oracle correlation
    ml_blue_offline_pilot();    break;
  case PILOT_PROJECTION: // for algorithm assessment/selection
    ml_blue_pilot_projection(); break;
  }
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_online_pilot()
{
  // retrieve cost estimates across soln levels for a particular model form
  IntRealMatrixArrayMap sum_G;  IntRealSymMatrix2DArrayMap sum_GG;
  SizetMatrixArray N_G_actual;  //SizetSymMatrix2DArray N_GG;
  initialize_blue_sums(sum_G, sum_GG); initialize_blue_counts(N_G_actual);
  SizetArray delta_N_G = pilotSamples, // sized by load_pilot_samples()
             N_G_alloc = delta_N_G;

  // *** TO DO ***
  // Different online pilot strategies:
  // 1. Independent pilot samples for each group = expensive initialization;
  //    straightforward to augment all groups with increments, but it is likely
  //    that not all groups will get allocated with N_g > N_pilot.
  // 2. Shared pilot sample (not independent) --> (much) less expensive
  //    initialization for initial estimates of covGG; subsequent allocations
  //    are independent, but shared pilot only reused once (all-model group).
  //    Saves cost for groups that are not selected for investment.
  // *** TO DO ***
  while (!zeros(delta_N_G) && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    group_increment(delta_N_G, mlmfIter); // spans ALL model groups, blocking
    accumulate_blue_sums(sum_G, sum_GG, N_G_actual);//, num_GG);

    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (onlineCost && mlmfIter == 0) recover_online_cost(sequenceCost);
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], equivHFEvals);
    compute_GG_covariance(sum_G[1], sum_GG[1], N_G_actual, covGG, covGGinv);

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and BLUE mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_allocations(blueSolnData, N_G_actual, N_G_alloc, delta_N_G);
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    RealMatrix H_raw_mom(numFunctions, 4);
    blue_raw_moments(sum_G, sum_GG, N_G_actual, H_raw_mom);
    convert_moments(H_raw_mom, momentStats);
  }

  if (!zeros(delta_N_G)) // exceeded maxIterations
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], deltaEquivHF);
  finalize_counts(N_G_actual, N_G_alloc);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var G & covar GG from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  RealMatrixArray sum_G_pilot; RealSymMatrix2DArray sum_GG_pilot;
  SizetMatrixArray N_pilot;
  evaluate_pilot(sum_G_pilot, sum_GG_pilot, N_pilot, false);
  compute_GG_covariance(sum_G_pilot, sum_GG_pilot, N_pilot, covGG, covGGinv);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealMatrixArrayMap sum_G; IntRealSymMatrix2DArrayMap sum_GG;
  SizetMatrixArray N_G_actual;  //SizetSymMatrix2DArray N_GG;
  initialize_blue_sums(sum_G, sum_GG); initialize_blue_counts(N_G_actual);
  SizetArray N_G_alloc, delta_N_G;  N_G_alloc.assign(numGroups, 0);

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_allocations(blueSolnData, N_G_actual, N_G_alloc, delta_N_G);
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    // perform the shared increment for the online sample profile
    group_increment(delta_N_G, mlmfIter); // spans ALL models, blocking
    accumulate_blue_sums(sum_G, sum_GG, N_G_actual);
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], equivHFEvals);
    // extract moments
    RealMatrix H_raw_mom(numFunctions, 4);
    blue_raw_moments(sum_G, sum_GG, N_G_actual, H_raw_mom);
    convert_moments(H_raw_mom, momentStats);
  }
  else
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], deltaEquivHF);

  finalize_counts(N_G_actual, N_G_alloc);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_pilot_projection()
{
  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealMatrixArray sum_G; RealSymMatrix2DArray sum_GG;
  SizetMatrixArray N_G_actual;  //SizetSymMatrix2DArray N_GG;
  evaluate_pilot(sum_G, sum_GG, N_G_actual, true);
  compute_GG_covariance(sum_G, sum_GG, N_G_actual, covGG, covGGinv);
  SizetArray delta_N_G, N_G_alloc = pilotSamples;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_allocations(blueSolnData, N_G_actual, N_G_alloc, delta_N_G);
  ++mlmfIter;

  // No final moments for pilot projection
  increment_equivalent_cost(delta_N_G, modelGroupCost,
			    sequenceCost[numApprox], deltaEquivHF);
  finalize_counts(N_G_actual, N_G_alloc);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDMultilevBLUESampling::
group_increment(SizetArray& delta_N_G, size_t iter)
{
  if (iter == 0) Cout << "\nPerforming pilot sample for ML BLUE.\n";
  else Cout << "\nML BLUE sampling iteration " << iter
	    << ": group sample increment =\n" << delta_N_G << '\n';

  size_t g, m, num_models, start;
  for (size_t g=0; g<numGroups; ++g) {
    numSamples = delta_N_G[g];
    if (numSamples) {
      ensemble_active_set(modelGroups[g]);
      ensemble_sample_batch(iter, g); // index is group_id; non-blocking
    }
  }

  if (iteratedModel.asynch_flag())
    ensemble_sample_synchronize(); // schedule all groups (return ignored)
}


void NonDMultilevBLUESampling::
evaluate_pilot(RealMatrixArray& sum_G_pilot, RealSymMatrix2DArray& sum_GG_pilot,
	       SizetMatrixArray& N_shared_pilot, bool incr_cost)
{
  initialize_blue_sums(sum_G_pilot, sum_GG_pilot);
  initialize_blue_counts(N_shared_pilot);//, N_GG_pilot);

  // ----------------------------------------
  // Compute var L,H & covar LL,LH from pilot
  // ----------------------------------------
  // Initialize for pilot sample
  group_increment(pilotSamples, mlmfIter); // spans ALL models, blocking
  accumulate_blue_sums(sum_G_pilot, sum_GG_pilot, N_shared_pilot);//, N_GG);

  if (onlineCost) recover_online_cost(sequenceCost);
  if (incr_cost)
    increment_equivalent_cost(pilotSamples, modelGroupCost,
			      sequenceCost[numApprox], equivHFEvals);
}


void NonDMultilevBLUESampling::
numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
			  size_t& num_nln_con)
{
  num_cdv = modelGroups.size();

  //switch (optSubProblemSolver) {
  //case SUBMETHOD_SDP:
    switch (optSubProblemForm) {
    case N_GROUP_LINEAR_CONSTRAINT:
      num_lin_con = 1;//(pilotMgmtMode == OFFLINE_PILOT) ? 2 : 1;
      num_nln_con = 0;   break;
    case N_GROUP_LINEAR_OBJECTIVE:
      num_lin_con = 0;//(pilotMgmtMode == OFFLINE_PILOT) ? 1 : 0;
      num_nln_con = 1;  break;
    }
  /*
    break;
  default:
    switch (optSubProblemForm) {
    case N_GROUP_LINEAR_CONSTRAINT:
      num_lin_con = 1;  num_nln_con = 0;   break;
    case N_GROUP_LINEAR_OBJECTIVE:
      num_nln_con = 1;  num_lin_con = numApprox;  break;
    }
    break;
  }
  */
}


void NonDMultilevBLUESampling::
numerical_solution_bounds_constraints(const MFSolutionData& soln,
				      RealVector& x0, RealVector& x_lb,
				      RealVector& x_ub, RealVector& lin_ineq_lb,
				      RealVector& lin_ineq_ub,
				      RealVector& lin_eq_tgt,
				      RealVector& nln_ineq_lb,
				      RealVector& nln_ineq_ub,
				      RealVector& nln_eq_tgt,
				      RealMatrix& lin_ineq_coeffs,
				      RealMatrix& lin_eq_coeffs)
{
  // --------------------------------------
  // Formulate the optimization sub-problem: initial pt, bnds, constraints
  // --------------------------------------

  size_t group, last_index = numGroups-1, lin_offset = 0;
  const RealVector& soln_vars = soln.solution_variables();
  //Real offline_N_lwr = 2.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;

  // Initial point and parameter bounds.  Note: some minimizers require finite
  // bounds --> these updates are performed in finite_solution_bounds()

  x_ub = DBL_MAX; // no upper bounds for groups
  if (pilotMgmtMode == OFFLINE_PILOT) {
    x_lb = 0.; // no lower bound for most groups (*** TO DO: NUDGE if not SDP?)
    //lin_ineq_lb[0] = 1.; // at least 1 sample for any group
    //lin_ineq_ub[0] = DBL_MAX;
    //for (group=0; group<numGroups; ++group)
    //  lin_ineq_coeffs(0, group) = 1.;
    //lin_offset = 1;
  }
  // assign sunk cost to full group and optimize w/ this as a constraint
  else if (mlmfIter)
    x_lb = soln_vars;
  else
    for (group=0; group<numGroups; ++group)
      x_lb[group] = (Real)pilotSamples[group];

  if (soln_vars.empty()) x0 = x_lb;
  else {
    x0 = soln_vars;
    //if (pilotMgmtMode == OFFLINE_PILOT) // x0 could undershoot x_lb
    //  for (group=0; group<numGroups; ++group)// bump x0 to satisfy x_lb if nec
    //	  if (x0[group] < x_lb[group])
    //	    x0[group] = x_lb[group];
  }

  // Linear and nonlinear constraints:

  switch (optSubProblemForm) {
  case N_GROUP_LINEAR_CONSTRAINT: { // linear inequality constraint on budget:
    // \Sum_grp_i w_grp_i        N_grp_i <= equiv_HF * w_HF
    // \Sum_grp_i w_grp_i / w_HF N_grp_i <= equivHF
    Real cost_H = sequenceCost[numApprox];
    lin_ineq_lb[lin_offset] = -DBL_MAX; // no lb
    lin_ineq_ub[lin_offset] = (Real)maxFunctionEvals;//budget;
    for (group=0; group<numGroups; ++group)
      lin_ineq_coeffs(lin_offset, group) = modelGroupCost[group] / cost_H;
    break;
  }
  case N_GROUP_LINEAR_OBJECTIVE: // nonlinear accuracy constraint: ub on estvar
    nln_ineq_lb = -DBL_MAX;   // no lower bnd
    nln_ineq_ub = std::log(convergenceTol * average(estVarIter0));
    break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Numerical solve (initial, lb, ub):\n" << x0 << x_lb << x_ub
	 << "Numerical solve (lin ineq lb, ub):\n" << lin_ineq_lb << lin_ineq_ub
       //<< lin_eq_tgt
	 << "Numerical solve (nln ineq lb, ub):\n" << nln_ineq_lb << nln_ineq_ub
       //<< nln_eq_tgt << lin_ineq_coeffs << lin_eq_coeffs
	 << std::endl;
}


void NonDMultilevBLUESampling::
compute_allocations(MFSolutionData& soln, const SizetMatrixArray& N_G_actual,
		    SizetArray& N_G_alloc, SizetArray& delta_N_G)
{
  // Solve the optimization sub-problem using an initial guess from either
  // related analytic solutions (iter == 0) or warm started from the previous
  // solutions (iter >= 1)

  if (mlmfIter == 0) {
    RealVector soln_vars(numGroups, false);
    for (size_t group=0; group<numGroups; ++group)
      soln_vars[group] = (Real)pilotSamples[group];
    soln.solution_variables(soln_vars);

    if (pilotMgmtMode != OFFLINE_PILOT) // cache reference estVarIter0
      estimator_variance(soln_vars, estVarIter0);

    bool budget_constrained = (maxFunctionEvals != SZ_MAX);
    bool budget_exhausted
      = (budget_constrained && equivHFEvals >= (Real)maxFunctionEvals);
    if (budget_exhausted || convergenceTol >= 1.) { // no need for solve
      // For offline pilot, the online EstVar is undefined prior to any online
      // samples, but should not happen (no budget used) unless bad convTol spec
      if (pilotMgmtMode == OFFLINE_PILOT)
	soln.average_estimator_variance(std::numeric_limits<Real>::infinity());
      else
	soln.average_estimator_variance(average(estVarIter0));
      soln.average_estimator_variance_ratio(1.);
      delta_N_G.assign(numGroups, 0);  return;
    }

    // Run a competition among related analytic approaches (MFMC or pairwise
    // CVMC) for best initial guess, where each initial gues may additionally
    // employ multiple varianceMinimizers in ensemble_numerical_solution()
    switch (optSubProblemSolver) { // no initial guess
    // global and sequenced global+local methods:
    case SUBMETHOD_DIRECT_NPSOL_OPTPP:  case SUBMETHOD_DIRECT_NPSOL:
    case SUBMETHOD_DIRECT_OPTPP:        case SUBMETHOD_DIRECT:
    case SUBMETHOD_EGO:  case SUBMETHOD_SBGO:  case SUBMETHOD_EA:
      ensemble_numerical_solution(soln);
      break;
    default: { // competed initial guesses with (competed) local methods
      // use pyramid sample definitions for hierarchical groups as init guess,
      // even though they are not independent sample sets in MFMC
      size_t g = numGroups - 1;// don't mix sample sets: use group w/ all models
      RealMatrix rho2_LH;  covariance_to_correlation_sq(covGG[g], rho2_LH);
      Real avg_N_H = (backfillFailures) ? average(N_G_actual[g]) : N_G_alloc[g];
      MFSolutionData mf_soln, cv_soln;
      analytic_initialization_from_mfmc(rho2_LH, avg_N_H, mf_soln);
      analytic_initialization_from_ensemble_cvmc(rho2_LH, avg_N_H, cv_soln);

      //if (multiStartACV) { // Run numerical solns from both starting points
      ensemble_numerical_solution(mf_soln);
      ensemble_numerical_solution(cv_soln);
      pick_mfmc_cvmc_solution(mf_soln, cv_soln, soln);
      break;
    }
    }
  }
  else { // warm start from previous solution (for active or one-and-only DAG)

    // no scaling needed from prev soln (as in NonDLocalReliability) since
    // updated avg_N_H now includes allocation from previous solution and
    // should be active on constraint bound (excepting sample count rounding)

    ensemble_numerical_solution(soln);
  }

  process_group_solution(soln, N_G_actual, N_G_alloc, delta_N_G);
  if (outputLevel >= NORMAL_OUTPUT)
    print_group_solution(Cout, soln);

  if (backfillFailures) // delta_N_G may include backfill
    one_sided_update(N_G_alloc, soln.solution_variables());
  else // delta_N_G is the allocation increment
    increment_samples(N_G_alloc, delta_N_G);
}


void NonDMultilevBLUESampling::
analytic_initialization_from_mfmc(const RealMatrix& rho2_LH, Real avg_N_H,
				  MFSolutionData& soln)
{
  RealVector avg_eval_ratios; // defined over numApprox, not numGroups
  UShortArray approx_set(numApprox);
  for (size_t i=0; i<numApprox; ++i) approx_set[i] = i;
  if (ordered_approx_sequence(rho2_LH)) // for all QoI across all Approx
    mfmc_analytic_solution(approx_set, rho2_LH, sequenceCost, avg_eval_ratios);
  else {
    // compute reordered MFMC for averaged rho; monotonic r not required
    // > any rho2_LH re-ordering from MFMC initial guess can be ignored (later
    //   gets replaced with r_i ordering for approx_increments() sampling)
    SizetArray approx_sequence;
    mfmc_reordered_analytic_solution(approx_set, rho2_LH, sequenceCost,
				     approx_sequence, avg_eval_ratios);
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from analytic MFMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  Real avg_hf_target;  RealVector soln_vars(numGroups, false);
  if (maxFunctionEvals == SZ_MAX) { // *** TO DO
    //avg_hf_target = update_hf_target(avg_eval_ratios, varH, estVarIter0);
    //soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
  }
  else {
    //scale_to_target(pilotSamples/*avg_N_H*/, modelGroupCost, avg_eval_ratios, sequenceCost, avg_hf_target); // redo for group-based pilot sample

    // let profile emerge from pilot on MFMC groups, but deduct pilot cost
    // for non-MFMC groups
    BitArray active_groups(numGroups); // init to off
    Real remaining = (Real)maxFunctionEvals;  size_t g, cntr = 0;
    for (g=0; g<numGroups; ++g) {
      if (mfmc_model_grouping(modelGroups[g]))  active_groups.set(g);
      else          remaining -= pilotSamples[g] * modelGroupCost[g];
    }
    // allocate remaining budget among MFMC groups
    avg_hf_target = allocate_budget(avg_eval_ratios, sequenceCost, remaining);
    // Convert to BLUE solution using MFMC "groups":
    for (g=0; g<numGroups; ++g)
      soln_vars[g] = (active_groups[g]) ?
	avg_eval_ratios[cntr++] * avg_hf_target : pilotSamples[g]; // *** TO DO: verify cntr ordering
  }
  soln.solution_variables(soln_vars);
}


void NonDMultilevBLUESampling::
analytic_initialization_from_ensemble_cvmc(const RealMatrix& rho2_LH,
					   Real avg_N_H, MFSolutionData& soln)
{
  // > Option 2 is ensemble of independent pairwise CVMCs, rescaled to an
  //   aggregate budget.  This is more ACV-like in the sense that it is not
  //   recursive, but it neglects the covariance C among approximations.
  //   It is also insensitive to model sequencing.
  RealVector avg_eval_ratios;
  cvmc_ensemble_solutions(rho2_LH, sequenceCost, avg_eval_ratios);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from ensemble CVMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  Real avg_hf_target;  RealVector soln_vars(numGroups, false);
  if (maxFunctionEvals == SZ_MAX) { // *** TO DO
    //avg_hf_target = update_hf_target(avg_eval_ratios, varH, estVarIter0);
    //soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
  }
  else {
    //scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, avg_hf_target);

    // let profile emerge from pilot on CVMC groups, but deduct pilot cost
    // for non-CVMC groups
    BitArray active_groups(numGroups); // init to off
    Real remaining = (Real)maxFunctionEvals;  size_t g, cntr = 0;
    for (g=0; g<numGroups; ++g) {
      if (cvmc_model_grouping(modelGroups[g]))  active_groups.set(g);
      else          remaining -= pilotSamples[g] * modelGroupCost[g];
    }
    // allocate remaining budget among MFMC groups
    avg_hf_target = allocate_budget(avg_eval_ratios, sequenceCost, remaining);
    // Convert to BLUE solution using MFMC "groups":
    for (g=0; g<numGroups; ++g)
      soln_vars[g] = (active_groups[g]) ?
	avg_eval_ratios[cntr++] * avg_hf_target : pilotSamples[g]; // *** TO DO: verify cntr ordering
  }
  soln.solution_variables(soln_vars);
}


void NonDMultilevBLUESampling::
process_group_solution(MFSolutionData& soln, const SizetMatrixArray& N_G_actual,
		       const SizetArray& N_G_alloc, SizetArray& delta_N_G)
{
  // compute sample increment for HF from current to target:
  if (backfillFailures)
    one_sided_delta(N_G_actual, soln.solution_variables(), delta_N_G);
  else
    one_sided_delta(N_G_alloc,  soln.solution_variables(), delta_N_G);

  // Employ projected MC estvar as reference to the projected ML BLUE estvar
  // from N* (where N* may include a num_samples increment not yet performed).
  // Two reference points are used for ML BLUE:
  // 1. For HF-only, employ var_H / projected-N_g for g = the HF-only group
  //    --> this is the closest thing to the estvar ratio (1. - R^2)
  // 2. For equivalent HF, emply var_H / (equivHFEvals + deltaEquivHF)
  UShortArray hf_only_group(1);  hf_only_group[0] = numApprox;
  unsigned short hf_index = find_index(modelGroups, hf_only_group);
  project_mc_estimator_variance(covGG[hf_index], N_G_actual[hf_index],
				delta_N_G[hf_index], projEstVarHF,
				projNActualHF);
  // Report ratio of averages rather that average of ratios (see notes in
  // print_variance_reduction())
  soln.average_estimator_variance_ratio(soln.average_estimator_variance() /
					average(projEstVarHF)); // (1 - R^2)
}


void NonDMultilevBLUESampling::
print_group_solution(std::ostream& s, const MFSolutionData& soln)
{
  const RealVector& soln_vars = soln.solution_variables();
  size_t i, num_v = soln_vars.length();
  for (i=0; i<num_v; ++i)
    Cout << "Group " << i+1 << " samples = " << soln_vars[i] << '\n';
  if (maxFunctionEvals == SZ_MAX)
    Cout << "Estimator cost allocation = " << soln.equivalent_hf_allocation()
	 << std::endl;
  else
    Cout << "Average estimator variance = " << soln.average_estimator_variance()
	 << "\nAverage ACV variance / average MC variance = "
	 << soln.average_estimator_variance_ratio() << std::endl;
}


inline void NonDMultilevBLUESampling::
finalize_counts(const SizetMatrixArray& N_G_actual, const SizetArray& N_G_alloc)
{
  // post final sample counts back to NLev{Actual,Alloc} (for final summaries)

  //bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  //inflate_approx_samples(N_G_actual, multilev, secondaryIndex, NLevActual);
  //inflate_approx_samples(N_G_alloc,  multilev, secondaryIndex, NLevAlloc);

  // For now, overlay group samples into model-resolution instance samples
  size_t g, m, num_models, q, mf, rl, m_index;
  const Pecos::ActiveKey& active_key = iteratedModel.active_model_key();
  for (g=0; g<numGroups; ++g) {
    const UShortArray& group_g = modelGroups[g];
    num_models = group_g.size();

    const SizetMatrix& N_G_actual_g = N_G_actual[g];
    size_t             N_G_alloc_g =  N_G_alloc[g];
    for (m=0; m<num_models; ++m) {
      m_index = group_g[m];
      mf = active_key.retrieve_model_form(m_index);
      rl = active_key.retrieve_resolution_level(m_index);

      NLevAlloc[mf][rl] += N_G_alloc_g;
      SizetArray& N_l_actual_fl = NLevActual[mf][rl];
      if (N_l_actual_fl.empty()) N_l_actual_fl.assign(numFunctions, 0);
      for (q=0; q<numFunctions; ++q)
	N_l_actual_fl[q] += N_G_actual_g(q,m);
    }
  }
}


void NonDMultilevBLUESampling::print_variance_reduction(std::ostream& s)
{
  //print_estimator_performance(s, blueSolnData);

  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:\n";

  if (pilotMgmtMode != OFFLINE_PILOT)
    s << "    Initial pilot (" << std::setw(5)
      << (size_t)std::floor(average(pilotSamples) + .5) << " ML samples):  "
      << std::setw(wpp7) << average(estVarIter0) << '\n';

  String method = " ML BLUE",
    type = (pilotMgmtMode == PILOT_PROJECTION) ? "Projected" : "   Online";
  // Ordering of averages:
  // > recomputing final MC estvar, rather than dividing the two averages, gives
  //   a result that is consistent with average(estVarIter0) when N* = pilot.
  // > The ACV ratio then differs from final ACV / final MC (due to recovering
  //   avgEstVar from the optimizer obj fn), but difference is usually small.
  UShortArray hf_only_group(1);  hf_only_group[0] = numApprox;
  unsigned short hf_index = find_index(modelGroups, hf_only_group);
  RealVector proj_equiv_estvar;
  project_mc_estimator_variance(covGG[hf_index], equivHFEvals, deltaEquivHF,
				proj_equiv_estvar);
  Real avg_estvar = blueSolnData.average_estimator_variance(),
    avg_proj_equiv_estvar = average(proj_equiv_estvar);
  // As described in process_group_solution(), we have two MC references:
  // projected HF-only samples and projected equivalent HF samples.
  s << "  " << type << " MC    (" << std::setw(5)
    << (size_t)std::floor(average(projNActualHF) + .5) << " HF samples):  "
    << std::setw(wpp7) << average(projEstVarHF)
    << "\n  " << type << method << " (sample profile):  "
    << std::setw(wpp7) << avg_estvar
    << "\n  " << type << method << " ratio  (1 - R^2):  "
    << std::setw(wpp7) << blueSolnData.average_estimator_variance_ratio()
    << "\n Equivalent MC    (" << std::setw(5)
    << (size_t)std::floor(equivHFEvals + deltaEquivHF + .5) << " HF samples):  "
    << std::setw(wpp7) << avg_proj_equiv_estvar
    << "\n Equivalent" << method << " ratio:             "
    << std::setw(wpp7) << avg_estvar / avg_proj_equiv_estvar << '\n';
}


void NonDMultilevBLUESampling::
project_mc_estimator_variance(const RealSymMatrixArray& var_H,
			      const SizetMatrix& N_H_actual, size_t delta_N_H,
			      RealVector& proj_est_var, SizetVector& proj_N_H)
{
  // Defines projected estvar for use as a consistent reference
  proj_est_var.sizeUninitialized(numFunctions);
  proj_N_H.sizeUninitialized(numFunctions);
  size_t qoi, N_l_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    N_l_q = proj_N_H[qoi] = N_H_actual(qoi,0) + delta_N_H;
    proj_est_var[qoi] = (N_l_q) ? var_H[qoi](0,0) / N_l_q : DBL_MAX;
  }
}


void NonDMultilevBLUESampling::
project_mc_estimator_variance(const RealSymMatrixArray& var_H, Real N_H_actual,
			      Real delta_N_H, RealVector& proj_est_var)
{
  // Defines projected estvar for use as a consistent reference
  proj_est_var.sizeUninitialized(numFunctions);
  size_t qoi; Real N_l_q = N_H_actual + delta_N_H;
  for (qoi=0; qoi<numFunctions; ++qoi)
    proj_est_var[qoi] = (N_l_q > 0.) ? var_H[qoi](0,0) / N_l_q : DBL_MAX;
}


/** Multi-moment map-based version used by online pilot */
void NonDMultilevBLUESampling::
accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
		     IntRealSymMatrix2DArrayMap& sum_GG,
		     SizetMatrixArray& num_G)//,SizetSymMatrix2DArray& num_GG)
{
  using std::isfinite;  bool all_finite;
  Real g1_fn, g2_fn, g1_prod, g2_prod;
  IntRespMCIter r_it;  IntRMAMIter g_it;  IntRSM2AMIter gg_it;
  int g_ord, gg_ord, active_ord, ord;
  size_t g, num_groups = modelGroups.size(), qoi, m, m2, num_models,
    g1_index, g2_index;

  for (g=0; g<num_groups; ++g) {
    SizetMatrix&             num_G_g =  num_G[g]; // index is group_id
    //SizetSymMatrixArray&  num_GG_g = num_GG[g];
    const UShortArray&       group_g = modelGroups[g];
    num_models                       = group_g.size();

    const IntResponseMap& resp_map_g = batchResponsesMap[g];// index is group_id
    for (r_it=resp_map_g.begin(); r_it!=resp_map_g.end(); ++r_it) {
      const Response&   resp    = r_it->second;
      const RealVector& fn_vals = resp.function_values();
      const ShortArray& asv     = resp.active_set_request_vector();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// see Bessel correction notes in NonDNonHierarchSampling::
	// compute_correlation(): population mean and variance should be
	// computed from the same sample set
	all_finite = true;
	for (m=0; m<num_models; ++m) {
	  g1_index = group_g[m] * numFunctions + qoi;
	  if ( (asv[g1_index] & 1) && !isfinite(fn_vals[g1_index]) )
	    { all_finite = false; break; }
	}
	if (!all_finite) continue;

	for (m=0; m<num_models; ++m) {
	  g1_index = group_g[m] * numFunctions + qoi;
	  if (asv[g1_index] & 1) {
	    ++num_G_g(qoi,m); // shared due to fault tol logic
	    g1_fn = fn_vals[g1_index];

	    g_it    = sum_G.begin();  gg_it = sum_GG.begin();
	    g_ord   = (g_it  ==  sum_G.end()) ? 0 :  g_it->first;
	    gg_ord  = (gg_it == sum_GG.end()) ? 0 : gg_it->first;
	    g1_prod = g1_fn;  active_ord = 1;
	    while (g_ord || gg_ord) {
    
	      if (g_ord == active_ord) { // support general key sequence
		g_it->second[g](qoi,m) += g1_prod;
		++g_it;  g_ord = (g_it == sum_G.end()) ? 0 : g_it->first;
	      }
	      if (gg_ord == active_ord) { // support general key sequence
		RealSymMatrix& sum_GG_gq = gg_it->second[g][qoi];
		sum_GG_gq(m,m) += g1_prod * g1_prod;
		// Off-diagonal of C matrix:
		for (m2=0; m2<m; ++m2) {
		  g2_index = group_g[m2] * numFunctions + qoi;
		  if (asv[g2_index] & 1) {
		    // regenerate g2_prod i/o storing off-diagonal combinations
		    g2_prod = g2_fn = fn_vals[g2_index];
		    for (ord=1; ord<active_ord; ++ord)
		      g2_prod *= g2_fn;
		    sum_GG_gq(m,m2) += g1_prod * g2_prod;
		  }
		}
		++gg_it; gg_ord = (gg_it == sum_GG.end()) ? 0 : gg_it->first;
	      }

	      g1_prod *= g1_fn;  ++active_ord;
	    }
	  }
	}
      }
    }
  }
}


/** Single moment version used by offline-pilot and pilot-projection */
void NonDMultilevBLUESampling::
accumulate_blue_sums(RealMatrixArray&  sum_G, RealSymMatrix2DArray& sum_GG,
		     SizetMatrixArray& num_G)//, SizetSymMatrix2DArray& num_GG)
{
  using std::isfinite;  bool all_finite;
  Real g1_fn;  IntRespMCIter r_it;
  size_t g, num_groups = modelGroups.size(), m, m2, num_models, qoi,
    g1_index, g2_index;

  for (g=0; g<num_groups; ++g) {
    RealMatrix&              sum_G_g =  sum_G[g]; // index is group_id
    RealSymMatrixArray&     sum_GG_g = sum_GG[g]; // index is group_id
    SizetMatrix&             num_G_g =  num_G[g]; // index is group_id
    //SizetSymMatrixArray&  num_GG_g = num_GG[g];
    const UShortArray&       group_g = modelGroups[g];
    num_models                       = group_g.size();

    const IntResponseMap& resp_map_g = batchResponsesMap[g];//index g = group_id
    for (r_it=resp_map_g.begin(); r_it!=resp_map_g.end(); ++r_it) {
      const Response&   resp    = r_it->second;
      const RealVector& fn_vals = resp.function_values();
      const ShortArray& asv     = resp.active_set_request_vector();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// see Bessel correction notes in NonDNonHierarchSampling::
	// compute_correlation(): population mean and variance should be
	// computed from the same sample set
	all_finite = true;
	for (m=0; m<num_models; ++m) {
	  g1_index = group_g[m] * numFunctions + qoi;
	  if ( (asv[g1_index] & 1) && !isfinite(fn_vals[g1_index]) )
	    { all_finite = false; break; }
	}
	if (!all_finite) continue;
      
	RealSymMatrix& sum_GG_gq = sum_GG_g[qoi];
	//SizetSymMatrix& num_GG_gq = num_GG_g[qoi];
	for (m=0; m<num_models; ++m) {
	  g1_index = group_g[m] * numFunctions + qoi;
	  if (asv[g1_index] & 1) {
	    ++num_G_g(qoi,m); // shared due to fault tol logic

	    g1_fn = fn_vals[g1_index];
	    sum_G_g(qoi,m) += g1_fn;
	    sum_GG_gq(m,m) += g1_fn * g1_fn;

	    // Off-diagonal of covariance matrix:
	    // look back (only) for single capture of each combination
	    for (m2=0; m2<m; ++m2) {
	      g2_index = group_g[m2] * numFunctions + qoi;
	      if (asv[g2_index] & 1) {
		//++num_GG_gq(m,m2);
		sum_GG_gq(m,m2) += g1_fn * fn_vals[g2_index];
	      }
	    }
	  }
	}
      }
    }

    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "In accumulate_blue_sums(), sum_G[" << g << "]:\n" << sum_G_g
	   << "sum_GG[" << g << "]:\n" << sum_GG_g
	   << "num_G["  << g << "]:\n" << num_G_g << std::endl;
  }
}


void NonDMultilevBLUESampling::
compute_GG_covariance(const RealMatrixArray& sum_G,
		      const RealSymMatrix2DArray& sum_GG,
		      const SizetMatrixArray& num_G,
		    //SizetSymMatrix2DArray& num_GG,
		      RealSymMatrix2DArray& cov_GG,
		      RealSymMatrix2DArray& cov_GG_inv)
{
  size_t g, m, m2, num_models, qoi;
  if (cov_GG.size() != numGroups) {
    cov_GG.resize(numGroups);
    for (g=0; g<numGroups; ++g) {
      num_models = modelGroups[g].size();
      RealSymMatrixArray& cov_GG_g = cov_GG[g];
      cov_GG_g.resize(numFunctions);
      for (qoi=0; qoi<numFunctions; ++qoi)
	cov_GG_g[qoi].shapeUninitialized(num_models);
    }
  }

  Real sum_G_gqm;  size_t N_sh_gqm;
  for (g=0; g<numGroups; ++g) {
    num_models = modelGroups[g].size();
    const RealMatrix&          sum_G_g = sum_G[g];
    const RealSymMatrixArray& sum_GG_g = sum_GG[g];
    const SizetMatrix&         num_G_g = num_G[g];
    RealSymMatrixArray&       cov_GG_g = cov_GG[g];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      const RealSymMatrix& sum_GG_gq = sum_GG_g[qoi];
      RealSymMatrix&       cov_GG_gq = cov_GG_g[qoi];
      for (m=0; m<num_models; ++m) {
	sum_G_gqm = sum_G_g(qoi,m);  N_sh_gqm = num_G_g(qoi,m);
	for (m2=0; m2<=m; ++m2)
	  compute_covariance(sum_G_gqm, sum_G_g(qoi,m2), sum_GG_gq(m,m2),
			     N_sh_gqm, cov_GG_gq(m,m2));
      }
    }
  }

  // precompute 2D array of C_k inverses for numerical solver use
  // (Phi-inverse is dependent on N_G, but C-inverse is not)
  compute_C_inverse(cov_GG, cov_GG_inv);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In compute_GG_covariance(), cov_GG:\n" << cov_GG
	 << "cov_GG inverse:\n" << cov_GG_inv << std::endl;
}


/*
void NonDMultilevBLUESampling::
compute_GG_statistics(RealMatrixArray& sum_G,
		      RealSymMatrix2DArray& sum_GG,
		      SizetMatrixArray& N_shared, //RealMatrixArray& var_G,
		      RealSymMatrix2DArray& cov_GG)
{
  if (mlmfIter == 0) // see var_G usage in compute_allocations()
    compute_G_variance(sum_G,  sum_GG, N_shared, var_G); // just extract from cov_GG ?
  compute_GG_covariance(sum_G, sum_GG, N_shared, cov_GG);

  //Cout << "var_G:\n" << var_G << "cov_GG:\n" << cov_GG;
}


void NonDMultilevBLUESampling::
compute_G_variance(const RealMatrixArray& sum_G,
		   const RealSymMatrix2DArray& sum_GG,
		   const SizetMatrixArray& num_G, RealMatrixArray& var_G)
{
  size_t qoi, g, m, num_models;
  if (var_G.size() != numGroups) {
    var_G.resize(numGroups);
    for (g=0; g<numGroups; ++g)
      var_G[g].shapeUninitialized(numFunctions, modelGroups[g].size());
  }

  for (g=0; g<numGroups; ++g) {
    num_models = modelGroups[g].size();
    const RealMatrix&          sum_G_g =  sum_G[g];
    const RealSymMatrixArray& sum_GG_g = sum_GG[g];
    const SizetMatrix&         num_G_g =  num_G[g];
    RealMatrix&                var_G_g =  var_G[g];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      const RealSymMatrix&   sum_GG_gq = sum_GG_g[qoi];
      for (m=0; m<num_models; ++m)
	compute_variance(sum_G_g(qoi,m), sum_GG_gq(m,m), num_G_g(qoi,m),
			 var_G_g(qoi,m));
    }
  }
}


void NonDMultilevBLUESampling::
update_projected_group_samples(const MFSolutionData& soln,
			       const SizetArray& N_G_actual, size_t& N_G_alloc,
			       SizetArray& delta_N_G, Real& delta_equiv_hf)
{
  Sizet2DArray N_L_actual; //inflate(N_H_actual, N_L_actual);
  SizetArray   N_L_alloc;  //inflate(N_H_alloc,  N_L_alloc);

  size_t group, alloc_incr, actual_incr;
  Real group_tgt;
  const RealVector& soln_vars = soln.solution_variables();
  for (group=0; group<numGroups; ++group) {
    group_tgt = soln_vars[group];

    const SizetArray& N_G_actual_g = N_G_actual[group];
    size_t&           N_G_alloc_g  =  N_G_alloc[group];
    alloc_incr  = one_sided_delta(N_L_alloc_g, group_tgt);
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_L_actual_g), group_tgt) : alloc_incr;
    //delta_N_L_actual[approx] += actual_incr;
    N_L_alloc_g += alloc_incr;
  }

  increment_equivalent_cost(delta_N_L_actual, modelGroupCost,
			    sequenceCost[numApprox], delta_equiv_hf);
  finalize_counts(N_L_actual, N_L_alloc);
}
*/

} // namespace Dakota
