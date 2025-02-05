/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "dakota_linear_algebra.hpp"
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
  NonDNonHierarchSampling(problem_db, model),
  pilotGroupSampling(problem_db.get_short("method.nond.pilot_samples.mode")),
  groupThrottleType(problem_db.get_short("method.nond.group_throttle_type")),
  groupSizeThrottle(problem_db.get_ushort("method.nond.group_size_throttle")),
  rCondBestThrottle(problem_db.get_sizet("method.nond.rcond_best_throttle")),
  rCondTolThrottle(problem_db.get_real("method.nond.rcond_tol_throttle"))
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

  // groupThrottleType is inferred for scalar spec so XML can be flattened
  if (!groupThrottleType) {
    if (groupSizeThrottle != USHRT_MAX)
      groupThrottleType = GROUP_SIZE_THROTTLE;
    else if (rCondBestThrottle != SZ_MAX)
      groupThrottleType = RCOND_BEST_COUNT_THROTTLE;
    else if (rCondTolThrottle != DBL_MAX)
      groupThrottleType = RCOND_TOLERANCE_THROTTLE;
  }

  switch (groupThrottleType) {

  //case DAG_DEPTH_THROTTLE:   // Emulate ACV
  //case DAG_BREADTH_THROTTLE: // Emulate MFMC
    // all approx --> nominal dags --> enforce constraints --> unique groups

  case MFMC_ESTIMATOR_GROUPS:
    // Some avg_eval_ratios from an analytic pairwise CVMC initial_guess will
    // be dropped.  MFMC is of course Ok.
    numGroups = numApprox + 1;
    modelGroups.resize(numGroups);
    for (size_t g=0; g<numGroups; ++g)
      mfmc_model_group(g, modelGroups[g]);
    break;

  case COMMON_ESTIMATOR_GROUPS: {
    // overlay hierarchical and peer groups (linear growth: 3M - duplicates)
    UShortArray group;  UShortArraySet unique_groups;  size_t g;
    for (g=0; g<numApprox; ++g) { // defer all groups to override set ordering
      cvmc_model_group(g, group); // singletons: increments in pair-wise CVMC
      unique_groups.insert(group);
      mfmc_model_group(g, group); // pyramid groups as in MFMC/ACV-MF
      unique_groups.insert(group);
    }
    for (g=0; g<=numApprox; ++g) {
      mlmc_model_group(g, group); // paired  groups as in MLMC/ACV-RD
      unique_groups.insert(group);
    }
    singleton_model_group(numApprox, group); // not the last CVMC group
    unique_groups.insert(group);    
    size_t unique_len = unique_groups.size();  UShortArraySet::iterator it;
    numGroups = unique_len + 1;
    modelGroups.resize(numGroups);
    for (it=unique_groups.begin(), g=0; it!=unique_groups.end(); ++it, ++g)
      modelGroups[g] = *it;
    // for consistency with other cases, force all-model case to be last group
    mfmc_model_group(numApprox, modelGroups[unique_len]);
    break;
  }

  case GROUP_SIZE_THROTTLE: { // simplest throttle
    size_t m, num_models = numApprox+1;
    // tensor product of order 1 to enumerate approximation groups
    UShort2DArray tp;  UShortArray tp_orders(num_models, 1);
    Pecos::SharedPolyApproxData::
      tensor_product_multi_index(tp_orders, tp, true);
    tp.erase(tp.begin()); // discard empty group (all 0's)

    size_t num_tp = tp.size(), g, g_size, g_index;
    UShortArray group_g;
    for (g=0; g<num_tp; ++g) {
      const UShortArray& tp_g = tp[g];
      g_size = std::count(tp_g.begin(), tp_g.end(), 1);
      if (g_size <= groupSizeThrottle) {
	group_g.resize(g_size);
	for (m=0, g_index=0; m<num_models; ++m)
	  if (tp_g[m]) { group_g[g_index] = m; ++g_index; }
	modelGroups.push_back(group_g);
      }
    }
    // augment with all-group where needed:
    // SHARED_PILOT or local/competed_local initial guesses from MFMC/CVMC
    // > since this is subtle / awkward to document / potentially bug-inducing
    //   downstream, we include the all_group w/ all size throttles for now
    // > Note 1: there is only one group with all models, so this renders
    //   size throttle = numApprox the same as no throttle.
    // > Note 2: size throttle < numApprox means some avg_eval_ratios from an
    //   analytic MFMC initial_guess will be dropped.  Pairwise CVMC is Ok.
    if ( groupSizeThrottle < num_models ) {
	 // && ( pilotGroupSampling == SHARED_PILOT ||
	 // varianceMinimizers.size() == 1) ) {// local w/ MFMC/CVMC pre-solve
      group_g.resize(num_models);
      for (m=0; m<num_models; ++m)
	group_g[m] = m;
      modelGroups.push_back(group_g);
    }
    numGroups = modelGroups.size();
    break;
  }

  default: { // NO_GROUP_THROTTLE, RCOND_*_THROTTLE
    // tensor product of order 1 to enumerate approximation groups
    // > modelGroups are not currently ordered by numbers of models,
    //   i.e. all 1-model cases, followed by all 2-model cases, etc.
    UShort2DArray tp;  UShortArray tp_orders(numApprox+1, 1);
    Pecos::SharedPolyApproxData::
      tensor_product_multi_index(tp_orders, tp, true);
    tp.erase(tp.begin()); // discard empty group (all 0's)

    numGroups = tp.size();  modelGroups.resize(numGroups);
    size_t g, m;
    for (g=0; g<numGroups; ++g) {
      const UShortArray& tp_g = tp[g];
      UShortArray&    group_g = modelGroups[g];
      for (m=0; m<=numApprox; ++m)
	if (tp_g[m]) group_g.push_back(m);
    }
    break;
  }
  }

  if (costSource == USER_COST_SPEC) update_model_group_costs();

  // if throttling is complete (not based on observed group covariances),
  // adjust solver according to numGroups if necessary (e.g. dimension
  // limitations for global pre-processing).  If throttling is instead
  // dynamic, perform this update downstream in prune_model_groups().
  if (groupThrottleType != RCOND_TOLERANCE_THROTTLE &&
      groupThrottleType != RCOND_BEST_COUNT_THROTTLE)
    update_search_algorithm();

  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    numGroups, pilotSamples);

  size_t max_ps = find_max(pilotSamples);
  if (max_ps) maxEvalConcurrency *= max_ps;
}


NonDMultilevBLUESampling::~NonDMultilevBLUESampling()
{ }


void NonDMultilevBLUESampling::core_run()
{
  retainedModelGroups.clear();

  switch (pilotMgmtMode) {
  case ONLINE_PILOT: // iterated ML BLUE (default)
    // ESTIMATOR_PERFORMANCE case differs from ONLINE_PILOT_PROJECTION
    ml_blue_online_pilot();     break;
  case OFFLINE_PILOT: // non-iterated allocation from offline/Oracle correlation
    switch (finalStatsType) {
    // since offline is not iterated, the ESTIMATOR_PERFORMANCE case is the
    // same as OFFLINE_PILOT_PROJECTION --> bypass IntMaps, simplify code
    case ESTIMATOR_PERFORMANCE:  ml_blue_pilot_projection();  break;
    default:                     ml_blue_offline_pilot();     break;
    }
    break;
  case ONLINE_PILOT_PROJECTION:  case OFFLINE_PILOT_PROJECTION:
    ml_blue_pilot_projection(); break;
  }
}


/** This function performs ML BLUE using an iterated approach. */
void NonDMultilevBLUESampling::ml_blue_online_pilot()
{
  // Online iteration does not work the same way since complete group increments
  // (as opposed to only incrementing the shared sample set) will exhaust the
  // budget on the first solve (excepting sample roundoff effects). Subsequent
  // solves are over-constrained, becoming budget-exhausted cases.  Thoughts:
  // > Good time to add support for under-relaxation of sample increments
  //   (as for parallel batching, deploy as shared feature set)
  // > SHARED_PILOT case needs logic to switch over from shared covariance to
  //   independent covariance, perhaps when N_actual_g >= pilot_g rather than
  //   mlmfIter > 0 (for which unallocated groups have no data)
  // > For SHARED_PILOT case, can increment the shared group until convergence
  //   (as for MFMC/ACV).  Is there an important interaction between dependence
  //   in group covar samples and the allocation solve?
  //   >> Compare rcond for shared vs. independent Psi??  cov_GG_inv will be Ok,
  //      but Psi_inv may be affected by repeat of pair-wise covariance terms.
  //   If not, first converge on the allocation and *then* evaluate independent
  //   increments for all other groups prior to roll-up of final moments
  //   (QOI_STATISTICS mode).
  //   >> Strict mode: only increment if all_group is allocated --> all new
  //      samples will get used in the final roll-up and we stay in SHARED_PILOT
  //      mode as long as we have all_group increments to perform.  Incurred
  //      expense still undershoots budget such that we could iterate without
  //      under-relaxing, but we also don't want to overshoot all_group samples.
  //   >> Loose mode: increment all_group based on some overlay of group
  //      allocations --> complicates reuse once we shift over to independent
  //      sample sets.  Some but not all model evals could be reused.  Avoid.
  // > Once shifting from shared to independent estimation of covariances,
  //   we need under-relaxation if we are to continue iteration.

  // Implementation:
  // {one-and-done, strict}        SHARED, followed by
  // {one-and-done, under-relaxed} INDEPENDENT
  // > strict SHARED on its own is pretty limited, but if these increments are
  //   present, they seem useful prior to performing samples across all groups.
  //   >> Go ahead and reuse under-relaxation spec to mitigate over-shoot.
  //   >> Counter-argument: is it imbalanced to refine all_group to convergence
  //      w/o refining any other groups?  Not really: all group covariances get
  //      updated --> this is simply refinement w/i the SHARED context.
  // > Using any and all group increments to update their corresponding group
  //   covariances is the strongest motivation, so use under-relaxation here
  //   to mitigate BOTH allocation overshoot and budget exhaustion.
  // > Similar to solution_level_cost, under-relaxation could support:
  //   >> sequence:      .5, .75, 1. for mlmfIter 1,2,3
  //   >> scalar factor: .75 --> .75 fixed or recursive on unity partition, e.g.
  //      .75 recursive = prev .75 + .75 * .25 remaining = .9375, .9844, ...
  //      .5  recursive = .5, .75, .875, ...
  //   >> combination: .5 .8 --> .5, .8, prev + .8 remaining = .96, .992, ...
  //   >> Factors could also be > 1 to mitigate fine deltaN refinement near conv
  //   >> Since factor is increasing while being applied to a shrinking portion
  //      of the allocation (delta->0), recursive compounds these two effects
  //      and is probably more complicated than necessary, e.g. can resort to
  //      factor = 1 (no relaxation) fairly quickly, i.e. after 1 or 2 iters.
  // > under_relax_sequence  = ListOfReal (don't auto-terminate with 1)
  //   under_relax_fixed     = Real (fixed)
  //   under_relax_recursive = Real (recursive per iter on unity partition)

  // retrieve cost estimates across soln levels for a particular model form
  IntRealMatrixArrayMap sum_G;          IntRealSymMatrix2DArrayMap sum_GG;
  initialize_group_sums(sum_G, sum_GG); initialize_group_counts(NGroupActual);
  SizetArray delta_N_G = pilotSamples; // sized by load_pilot_samples()
  NGroupAlloc.assign(numGroups, 0);

  // online iterations for shared covariance estimation:
  if (pilotGroupSampling == SHARED_PILOT) {
    size_t all_group = numGroups - 1; // last group = all models
    NGroupAlloc[all_group] = delta_N_G[all_group];
    shared_covariance_iteration(sum_G, sum_GG, delta_N_G);
    reset_relaxation(); // for initial increments for all other groups
  }
  // online iteration for independent covariance estimation:
  independent_covariance_iteration(sum_G, sum_GG, delta_N_G);

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    blue_raw_moments(sum_G, sum_GG, NGroupActual); // online version

  if (!zeros(delta_N_G)) // exceeded maxIterations
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], deltaEquivHF);
  finalize_counts(NGroupActual, NGroupAlloc);
}


/** This function performs ML BLUE using an offline pilot approach. */
void NonDMultilevBLUESampling::ml_blue_offline_pilot()
{
  // --------------------------------------------------------------
  // Compute covar GG from (oracle) pilot treated as "offline" cost
  // --------------------------------------------------------------
  IntRealMatrixArrayMap sum_G_pilot; IntRealSymMatrix2DArrayMap sum_GG_pilot;
  Sizet2DArray N_pilot;
  evaluate_pilot(sum_G_pilot, sum_GG_pilot, N_pilot, false);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealMatrixArrayMap sum_G;          IntRealSymMatrix2DArrayMap sum_GG;
  initialize_group_sums(sum_G, sum_GG); initialize_group_counts(NGroupActual);
  SizetArray delta_N_G;  NGroupAlloc.assign(numGroups, 0);

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_allocations(blueSolnData, NGroupActual, NGroupAlloc, delta_N_G);
  increment_allocations(blueSolnData, NGroupAlloc, delta_N_G);
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // QOI_STATISTICS case; ESTIMATOR_PERFORMANCE redirects to
  // ml_blue_pilot_projection() to also bypass IntMaps.

  // perform the shared increment for the online sample profile
  group_increments(delta_N_G, "blue_"); // spans ALL models, blocking
  accumulate_blue_sums(sum_G, sum_GG, NGroupActual, batchResponsesMap);
  increment_equivalent_cost(delta_N_G, modelGroupCost,
			    sequenceCost[numApprox], equivHFEvals);
  clear_batches();
  // extract moments
  blue_raw_moments(sum_G_pilot, sum_GG_pilot, N_pilot, // offline for covar
		   sum_G, sum_GG, NGroupActual);       // online for mu-hat
  // finalize
  finalize_counts(NGroupActual, NGroupAlloc);
}


/** This function performs ML BLUE using a pilot projection approach. */
void NonDMultilevBLUESampling::ml_blue_pilot_projection()
{
  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealMatrixArray sum_G; RealSymMatrix2DArray sum_GG;
  if (pilotMgmtMode == OFFLINE_PILOT || // redirected here for ESTIMATOR_PERF
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    // accumulate offline counts
    Sizet2DArray N_pilot;
    evaluate_pilot(sum_G, sum_GG, N_pilot, false);
    // initialize online counts
    initialize_group_counts(NGroupActual);  NGroupAlloc.assign(numGroups, 0);
  }
  else { // ONLINE_PILOT_PROJECTION
    if (pilotGroupSampling == SHARED_PILOT) {
      NGroupAlloc.assign(numGroups, 0);
      size_t all_group = numGroups - 1; // last group = all models
      NGroupAlloc[all_group] = pilotSamples[all_group];
    }
    else NGroupAlloc = pilotSamples;
    // initialize + accumulate online counts
    evaluate_pilot(sum_G, sum_GG, NGroupActual, true);
  }

  // -------------------------------------
  // Compute sample increment projections:
  // -------------------------------------
  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  SizetArray delta_N_G;
  compute_allocations(blueSolnData, NGroupActual, NGroupAlloc, delta_N_G);
  increment_allocations(blueSolnData, NGroupAlloc, delta_N_G);
  ++mlmfIter;

  // No final moments for pilot projection
  increment_equivalent_cost(delta_N_G, modelGroupCost,
			    sequenceCost[numApprox], deltaEquivHF);
  finalize_counts(NGroupActual, NGroupAlloc);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected estVariance{s,Ratios}
}


void NonDMultilevBLUESampling::
shared_covariance_iteration(IntRealMatrixArrayMap& sum_G,
			    IntRealSymMatrix2DArrayMap& sum_GG,
			    SizetArray& delta_N_G)
{
  size_t all_group = numGroups - 1; // last group = all models
  numSamples = delta_N_G[all_group];

  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> multilevel_blue: online iteration for shared covariance."
	 << std::endl;
  while (numSamples && mlmfIter <= maxIterations) {
    // -----------------------------------------------
    // Evaluate shared increment and update covariance
    // -----------------------------------------------
    // > Note: evaluate_pilot() does not support IntMaps
    shared_increment("blue_");
    // accumulate for one group only and reuse for group covariances
    accumulate_blue_sums(sum_G, sum_GG, NGroupActual, all_group, allResponses);
    // update covariances, recomputing model group pruning if active
    compute_GG_covariance(sum_G[1][all_group], sum_GG[1][all_group],
			  NGroupActual[all_group], covGG, covGGinv);

    if (mlmfIter == 0 && costSource != USER_COST_SPEC)
      { recover_online_cost(allResponses); update_model_group_costs(); }
    increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox+1,
			      equivHFEvals);

    // --------------------
    // Solve for allocation
    // --------------------
    prune_model_groups(); // redefined from scatch each iteration
    // Numerical soln is subject to latest group pruning (rcond throttles), but
    // all_group retention is enforced in prune_model_groups() for this case
    compute_allocations(blueSolnData, NGroupActual, NGroupAlloc, delta_N_G);
    // only increment NGroupAlloc[all_group]
    increment_allocations(blueSolnData, NGroupAlloc, delta_N_G, all_group);
    numSamples = delta_N_G[all_group];
    ++mlmfIter;  advance_relaxation();
  }
}


void NonDMultilevBLUESampling::
independent_covariance_iteration(IntRealMatrixArrayMap& sum_G,
				 IntRealSymMatrix2DArrayMap& sum_GG,
				 SizetArray& delta_N_G)
{
  // resetting counter for independent iteration allows separated throttling,
  // but also resets the numerical solve initial guess process (expensive)
  //mlmfIter = 0;

  // either leftover delta from shared_covariance_iteration() or initial pilot
  increment_allocations(blueSolnData, NGroupAlloc, delta_N_G);

  // if SHARED_PILOT, delta_N_G[all_group] should now be zero (unless maxIter),
  // but other groups will be nonzero prior to full convergence
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "\n>>>>> multilevel_blue: online iteration for independent "
	 << "covariance." << std::endl;

  SizetArray N_G_ref;
  if (pilotGroupSampling == SHARED_PILOT)
    N_G_ref = NGroupActual[numGroups - 1]; // for compute_GG_covariance()

  while (!zeros(delta_N_G) && mlmfIter <= maxIterations) {

    // -----------------------------------------------
    // Evaluate shared increment and update covariance
    // -----------------------------------------------
    group_increments(delta_N_G, "blue_"); // spans ALL model groups, blocking
    accumulate_blue_sums(sum_G, sum_GG, NGroupActual, batchResponsesMap);
    compute_GG_covariance(sum_G[1], sum_GG[1], NGroupActual,
			  covGG, covGGinv, N_G_ref);
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update on subsequent iterations.  We could
    // potentially mirror the covariance updates with cost updates, but seems
    // likely to induce thrash when run times are not robust.
    if (mlmfIter == 0 && costSource != USER_COST_SPEC)
        /* && pilotGroupSampling == INDEPENDENT_PILOT */
      { recover_online_cost(batchResponsesMap); update_model_group_costs(); }
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], equivHFEvals);
    clear_batches();

    // --------------------
    // Solve for allocation
    // --------------------
    prune_model_groups(); // redefined from scatch each iteration
    compute_allocations(blueSolnData, NGroupActual, NGroupAlloc, delta_N_G);
    increment_allocations(blueSolnData, NGroupAlloc, delta_N_G);
    ++mlmfIter;  advance_relaxation();
  }
}


void NonDMultilevBLUESampling::
evaluate_pilot(RealMatrixArray& sum_G_pilot, RealSymMatrix2DArray& sum_GG_pilot,
	       Sizet2DArray& N_shared_pilot, bool incr_cost)
{
  initialize_group_sums(sum_G_pilot, sum_GG_pilot);
  initialize_group_counts(N_shared_pilot);//, N_GG_pilot);

  // ----------------------------------------
  // Compute var L,H & covar LL,LH from pilot
  // ----------------------------------------
  // INDEPENDENT_PILOT: independent pilot samples for each group = expensive
  //   initialization; straightforward to augment all groups with increments,
  //   but likely that not all groups will get allocated with N_g > N_pilot.
  // SHARED_PILOT: shared pilot sample is (much) less expensive initialization
  //   for initial estimates of covGG; subsequent allocations are independent,
  //   but shared pilot only reused once (all-model group).  Saves cost for
  //   groups that are not selected for investment.
  if (pilotGroupSampling == SHARED_PILOT) {
    size_t all_group = numGroups - 1; // last group = all models
    numSamples = pilotSamples[all_group];
    shared_increment("blue_");
    // accumulate for one group only and reuse for group covariances
    RealMatrix&          sum_G_sh =    sum_G_pilot[all_group];
    RealSymMatrixArray& sum_GG_sh =   sum_GG_pilot[all_group];
    SizetArray&              N_sh = N_shared_pilot[all_group];
    accumulate_blue_sums(sum_G_sh, sum_GG_sh, N_sh, all_group, allResponses);
    compute_GG_covariance(sum_G_sh, sum_GG_sh, N_sh, covGG, covGGinv);
    if (costSource != USER_COST_SPEC)
      { recover_online_cost(allResponses); update_model_group_costs(); }
    if (incr_cost)
      increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox+1,
				equivHFEvals);
  }
  else {
    group_increments(pilotSamples, "blue_"); // all groups, independent samples
    accumulate_blue_sums(sum_G_pilot, sum_GG_pilot, N_shared_pilot,
			 batchResponsesMap);
    compute_GG_covariance(sum_G_pilot, sum_GG_pilot, N_shared_pilot,
			  covGG, covGGinv);
    if (costSource != USER_COST_SPEC)
      { recover_online_cost(batchResponsesMap); update_model_group_costs(); }
    if (incr_cost)
      increment_equivalent_cost(pilotSamples, modelGroupCost,
				sequenceCost[numApprox], equivHFEvals);
    clear_batches();
  }
  prune_model_groups();
}


void NonDMultilevBLUESampling::
evaluate_pilot(IntRealMatrixArrayMap& sum_G_pilot,
	       IntRealSymMatrix2DArrayMap& sum_GG_pilot,
	       Sizet2DArray& N_shared_pilot, bool incr_cost)
{
  initialize_group_sums(sum_G_pilot, sum_GG_pilot);
  initialize_group_counts(N_shared_pilot);//, N_GG_pilot);

  // ----------------------------------------
  // Compute var L,H & covar LL,LH from pilot
  // ----------------------------------------
  // INDEPENDENT_PILOT: independent pilot samples for each group = expensive
  //   initialization; straightforward to augment all groups with increments,
  //   but likely that not all groups will get allocated with N_g > N_pilot.
  // SHARED_PILOT: shared pilot sample is (much) less expensive initialization
  //   for initial estimates of covGG; subsequent allocations are independent,
  //   but shared pilot only reused once (all-model group).  Saves cost for
  //   groups that are not selected for investment.
  if (pilotGroupSampling == SHARED_PILOT) {
    size_t all_group = numGroups - 1; // last group = all models
    numSamples = pilotSamples[all_group];
    shared_increment("blue_");
    // accumulate for one group only and reuse for group covariances
    accumulate_blue_sums(sum_G_pilot, sum_GG_pilot, N_shared_pilot, all_group,
			 allResponses);
    compute_GG_covariance(sum_G_pilot[1][all_group], sum_GG_pilot[1][all_group],
			  N_shared_pilot[all_group], covGG, covGGinv);
    if (costSource != USER_COST_SPEC)
      { recover_online_cost(allResponses); update_model_group_costs(); }
    if (incr_cost)
      increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox+1,
				equivHFEvals);
  }
  else {
    group_increments(pilotSamples, "blue_"); // all groups, independent samples
    accumulate_blue_sums(sum_G_pilot, sum_GG_pilot, N_shared_pilot,
			 batchResponsesMap);
    compute_GG_covariance(sum_G_pilot[1], sum_GG_pilot[1], N_shared_pilot,
			  covGG, covGGinv);
    if (costSource != USER_COST_SPEC)
      { recover_online_cost(batchResponsesMap); update_model_group_costs(); }
    if (incr_cost)
      increment_equivalent_cost(pilotSamples, modelGroupCost,
				sequenceCost[numApprox], equivHFEvals);
    clear_batches();
  }
  prune_model_groups();
}


void NonDMultilevBLUESampling::
numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
			  size_t& num_nln_con)
{
  num_cdv = num_active_groups();
  bool offline = (pilotMgmtMode == OFFLINE_PILOT ||
		  pilotMgmtMode == OFFLINE_PILOT_PROJECTION);

  //switch (optSubProblemSolver) {
  //case SUBMETHOD_SDP:
    switch (optSubProblemForm) {
    case N_GROUP_LINEAR_CONSTRAINT:
      num_lin_con = (offline) ? 2 : 1;
      num_nln_con = 0;   break;
    case N_GROUP_LINEAR_OBJECTIVE:
      num_lin_con = (offline) ? 1 : 0;
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

  specify_parameter_bounds(x_lb, x_ub);
  enforce_nudge(x_lb); // nudge away from 0 if needed

  specify_initial_parameters(soln, x0, x_lb, x_ub);
  enforce_bounds(x0, x_lb, x_ub);// for example, x0 can undershoot lb if OFFLINE

  specify_linear_constraints(lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
			     lin_ineq_coeffs, lin_eq_coeffs);
  specify_nonlinear_constraints(nln_ineq_lb, nln_ineq_ub, nln_eq_tgt);
}


void NonDMultilevBLUESampling::
derived_finite_solution_bounds(const RealVector& x0, RealVector& x_lb,
			       RealVector& x_ub, Real budget)
{
  // Extreme N_g is all refinement budget allocated to one group:
  //   delta_N_g cost_g = budget_cost - equivHFEvals
  size_t g, v, num_v = x0.length();  Real cost_H = sequenceCost[numApprox];
  if (equivHFEvals > 0.) {
    Real remaining_cost = (budget - equivHFEvals) * cost_H;
    for (v=0; v<num_v; ++v) {
      g = active_to_all_group(v);
      x_ub[v] = x0[v] + remaining_cost / modelGroupCost[g];
    }
  }
  else { // in this case, avoid offline_N_lwr,RATIO_NUDGE within x0
    Real budget_cost = budget * cost_H;
    for (v=0; v<num_v; ++v) {
      g = active_to_all_group(v);
      x_ub[v] = budget_cost / modelGroupCost[g];
    }
  }
}


void NonDMultilevBLUESampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  // Note: could be collapsed within numerical_solution_bounds_constraints()
  // above to reduce function declarations, but we stick with the convention
  // that augment_linear_ineq_constraints() augments the core problem definition
  // with additional sample count relationship constraints.  Instead of managing
  // accuracy/cost metrics, these constraints ensure a well-posed solution and
  // may need to be enforced first by methods w/o explicit constraint handling.

  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    // Ensure that we have at least one sample for one of the groups containing
    // the HF reference model.  This is already satisfied by pilot sampling for
    // current group definitions used by online/projection modes.
    lin_ineq_lb[0] = 1.;  lin_ineq_ub[0] = DBL_MAX;
    size_t v, g, num_v = lin_ineq_coeffs.numCols();
    for (v=0; v<num_v; ++v) {
      g = active_to_all_group(v);
      if (contains(modelGroups[g], numApprox)) // HF model is part of group
	lin_ineq_coeffs(0, v) = 1.;
    }
  }
}


Real NonDMultilevBLUESampling::
augmented_linear_ineq_violations(const RealVector& cd_vars,
				 const RealMatrix& lin_ineq_coeffs,
				 const RealVector& lin_ineq_lb,
				 const RealVector& lin_ineq_ub)
{
  // These are called out separately to avoid NaNs from inadmissible points

  Real quad_viol = 0.;
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    // Ensure that we have at least one sample for one of the HF groups
    Real inner_prod = 0.;
    size_t v, num_v = cd_vars.length();
    for (v=0; v<num_v; ++v)
      inner_prod += lin_ineq_coeffs(0, v) * cd_vars[v]; // avoid contains()
    Real viol, l_bnd = lin_ineq_lb[0];//, u_bnd = lin_ineq_ub[0];
    if (inner_prod < l_bnd) {
      viol =
	//(std::abs(l_bnd) > Pecos::SMALL_NUMBER) ? (1. - inner_prod / l_bnd) :
	l_bnd - inner_prod;
      quad_viol += viol*viol;
    }
    //else if (inner_prod > u_bnd) {
    //	viol = (std::abs(u_bnd) > Pecos::SMALL_NUMBER)
    //	  ? (inner_prod / u_bnd - 1.) : inner_prod - u_bnd;
    //	quad_viol += viol*viol;
    //}
  }
  return quad_viol;
}


void NonDMultilevBLUESampling::
enforce_bounds_linear_constraints(RealVector& soln_vars)
{
  // This function applies ML BLUE requirements to an (analytic) initial guess
  // from another estimator (MFMC, CVMC).  These requirements are focused on
  // having the matrix solution be well-posed, rather than enforcing constraints
  // on accuracy or budget.

  size_t num_v = num_active_groups();
  RealVector x_lb(num_v), x_ub(num_v);
  specify_parameter_bounds(x_lb, x_ub);
  enforce_nudge(x_lb); // nudge away from 0 if needed
  enforce_bounds(soln_vars, x_lb, x_ub);
  enforce_augmented_linear_ineq_constraints(soln_vars);
}


void NonDMultilevBLUESampling::
specify_parameter_bounds(RealVector& x_lb, RealVector& x_ub)
{
  // Solver parameter bounds.  Note: some minimizers require finite
  // bounds --> these updates are performed in finite_solution_bounds()

  x_ub = DBL_MAX; // no upper bounds for groups
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION)
    x_lb = 0.; // no group lower bounds for OFFLINE case (NUDGE enforced below)
  else {
    // Assign sunk cost to full group and optimize w/ this as a constraint.
    // > One could argue for only lower-bounding with actual incurred samples,
    //   but have elected elsewhere to be consistent with backfill logic.
    // > Note: only NGroup*[all_group] is advanced in shared_covariance_iter()
    size_t g, v, num_v = x_lb.length();
    for (v=0; v<num_v; ++v) {
      g = active_to_all_group(v);
      x_lb[v] = (backfillFailures) ?
	average(NGroupActual[g]) : (Real)NGroupAlloc[g];
    }
  }
  //enforce_nudge(x_lb); // nudge away from 0 if needed
}


void NonDMultilevBLUESampling::
specify_initial_parameters(const MFSolutionData& soln, RealVector& x0,
			   const RealVector& x_lb, const RealVector& x_ub)
{
  const RealVector& soln_vars = soln.solution_variables();
  if (soln_vars.empty()) x0 = x_lb;
  else {
    size_t num_v = num_active_groups();
    if (soln_vars.length() != num_v) deflate(soln_vars, retainedModelGroups,x0);
    else                             x0 = soln_vars;
  }
}


void NonDMultilevBLUESampling::
specify_linear_constraints(RealVector& lin_ineq_lb, RealVector& lin_ineq_ub,
			   RealVector& lin_eq_tgt,  RealMatrix& lin_ineq_coeffs,
			   RealMatrix& lin_eq_coeffs)
{
  switch (optSubProblemForm) {
  case N_GROUP_LINEAR_CONSTRAINT: { // linear inequality constraint on budget:
    // \Sum_grp_i w_grp_i        N_grp_i <= equiv_HF * w_HF
    // \Sum_grp_i w_grp_i / w_HF N_grp_i <= equivHF
    size_t g, v, num_v = num_active_groups(), lin_offset
      = (pilotMgmtMode == OFFLINE_PILOT ||
	 pilotMgmtMode == OFFLINE_PILOT_PROJECTION) ? 1 : 0;
    lin_ineq_lb[lin_offset] = -DBL_MAX; // no lb
    lin_ineq_ub[lin_offset] = activeBudget;
    Real cost_H = sequenceCost[numApprox];
    for (v=0; v<num_v; ++v) {
      g = active_to_all_group(v);
      lin_ineq_coeffs(lin_offset, v) = modelGroupCost[g] / cost_H;
    }
    break;
  }
  }
}


void NonDMultilevBLUESampling::
specify_nonlinear_constraints(RealVector& nln_ineq_lb, RealVector& nln_ineq_ub,
			      RealVector& nln_eq_tgt)
{
  switch (optSubProblemForm) {
  case N_GROUP_LINEAR_OBJECTIVE: // nonlinear accuracy constraint: ub on estvar
    nln_ineq_lb = -DBL_MAX;   // no lower bnd
    nln_ineq_ub = std::log(convergenceTol * estVarMetric0);
    break;
  }
}


void NonDMultilevBLUESampling::
enforce_augmented_linear_ineq_constraints(RealVector& soln_vars)
{
  if ( pilotMgmtMode == ONLINE_PILOT ||
       pilotMgmtMode == ONLINE_PILOT_PROJECTION ) // no augmented linear ineq
    return;

  RealMatrix lin_ineq_coeffs(1, soln_vars.length());
  RealVector lin_ineq_lb(1), lin_ineq_ub(1);
  augment_linear_ineq_constraints(lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub);

  size_t g;  int v, num_v = soln_vars.length(), last_v = num_v - 1;
  Real lb = 1.;
  // Assess presence of at least 1 sample in a HF group (any partial
  // contributions are discarded in a way that linear ineq cannot support)
  bool hf_sampled = false;
  for (v=last_v; v>=0; --v)
    if (lin_ineq_coeffs(0, v) == 1. && // active group contains HF
	soln_vars[v] >= lb)            // active group is sampled
      { hf_sampled = true; break; }

  // Ensure that we have at least one sample for one of the groups containing
  // the HF reference model.  This is already satisfied by pilot sampling for
  // current group definitions used by online modes.
  if (!hf_sampled)
    switch (groupThrottleType) {
    case RCOND_TOLERANCE_THROTTLE: case RCOND_BEST_COUNT_THROTTLE: {
      // > prune_model_groups() has already added a HF group if necessary
      // > Note that the best conditioned will usually be the HF-only group
      //   with group covariance rcond = 1
      v = all_to_active_group(best_conditioned_hf_group());
      soln_vars[v] += lb;
      break;
    }
    default:
      // no rcond ranking: work backwards from last v to first
      // (subsumes all_group cases)
      for (v=last_v; v>=0; --v)
	if (lin_ineq_coeffs(0, v) == 1.) // active group contains HF
	  { soln_vars[v] += lb; break; } // preserve any previous nudge
      break;
    }
}


void NonDMultilevBLUESampling::
compute_allocations(MFSolutionData& soln, const Sizet2DArray& N_G_actual,
		    SizetArray& N_G_alloc, SizetArray& delta_N_G)
{
  // Solve the optimization sub-problem using an initial guess from either
  // related analytic solutions (iter == 0) or warm started from the previous
  // solutions (iter >= 1)

  bool budget_constrained = (maxFunctionEvals != SZ_MAX), budget_exhausted
    = (budget_constrained && equivHFEvals >= (Real)maxFunctionEvals),
    no_solve = (budget_exhausted || convergenceTol >= 1.); // bypass opt solve

  if (mlmfIter == 0) {
    if (retainedModelGroups.empty()) soln.solution_variables(pilotSamples);
    else {
      RealVector x0; deflate(pilotSamples, retainedModelGroups, x0);
      soln.solution_variables(x0);
    }
    if (pilotMgmtMode == ONLINE_PILOT ||
	pilotMgmtMode == ONLINE_PILOT_PROJECTION) { // cache ref estVarIter0
      estimator_variances(soln.solution_variables(), estVarIter0);
      MFSolutionData::update_estimator_variance_metric(estVarMetricType,
	estVarMetricNormOrder, estVarIter0, estVarMetric0);
    }

    if (no_solve)
      { no_solve_variances(soln); delta_N_G.assign(numGroups, 0); return; }

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
      MFSolutionData mf_soln, cv_soln;
      analytic_initialization_from_mfmc(rho2_LH, mf_soln);
      analytic_initialization_from_ensemble_cvmc(rho2_LH, cv_soln);
      competed_initial_guesses(mf_soln, cv_soln, soln);
      break;
    }
    }
  }
  else { // subsequent iterations
    if (no_solve) // leave soln at previous values
      { delta_N_G.assign(numGroups, 0); return; }

    // warm start from previous solution (for active or one-and-only DAG)
    ensemble_numerical_solution(soln);
  }

  process_group_allocations(soln, N_G_actual, N_G_alloc, delta_N_G);
  if (outputLevel >= NORMAL_OUTPUT)
    print_group_allocations(Cout, soln);
}


void NonDMultilevBLUESampling::
analytic_initialization_from_mfmc(const RealMatrix& rho2_LH,
				  MFSolutionData& soln)
{
  RealVector avg_eval_ratios; // defined over numApprox, not numGroups
  SizetArray approx_sequence;  UShortArray approx_set(numApprox);
  for (size_t i=0; i<numApprox; ++i) approx_set[i] = i;
  // Allow r<1 since only an initial guess (valid MFMC estvar not needed)
  bool lower_bounded_r = false, monotonic_r = false;
  if (ordered_approx_sequence(rho2_LH)) // for all QoI across all Approx
    mfmc_analytic_solution(approx_set, rho2_LH, sequenceCost, avg_eval_ratios,
			   lower_bounded_r, monotonic_r);
  else // compute reordered MFMC for averaged rho; monotonic r not required
       // > any rho2_LH re-ordering from MFMC init guess can be ignored (later
       //   gets replaced with r_i ordering for approx_increments() sampling)
    mfmc_reordered_analytic_solution(approx_set, rho2_LH, sequenceCost,
				     approx_sequence, avg_eval_ratios,
				     lower_bounded_r, monotonic_r);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from analytic MFMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  // Convert to BLUE solution using MFMC "groups": let profile emerge from
  // pilot on MFMC groups, but deduct pilot cost for non-MFMC groups
  SizetArray ratios_to_groups(numApprox+1); UShortArray group; size_t g_index;
  bool no_retain_throttle = retainedModelGroups.empty();
  for (size_t r=0; r<=numApprox; ++r) {
    mfmc_model_group(r, group); // the r-th MFMC group
    g_index = find_index(modelGroups, group);
    ratios_to_groups[r] = (no_retain_throttle || retainedModelGroups[g_index])
                        ? g_index : _NPOS;
  }
  analytic_ratios_to_solution_variables(avg_eval_ratios,ratios_to_groups,soln);
}


void NonDMultilevBLUESampling::
analytic_initialization_from_ensemble_cvmc(const RealMatrix& rho2_LH,
					   MFSolutionData& soln)
{
  // An ensemble of independent pairwise CVMCs, rescaled to an aggregate budget.
  // This is ACV-like in that it is not recursive, but it neglects covariance C
  // among approximations.  It is also insensitive to model sequencing.

  // Allow r<1 since only an initial guess for unbounded groups
  RealVector avg_eval_ratios;  bool lower_bounded_r = false;
  cvmc_ensemble_solutions(rho2_LH, sequenceCost, avg_eval_ratios,
			  lower_bounded_r);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from ensemble CVMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  // Convert to BLUE solution using MFMC "groups": let profile emerge from
  // pilot on MFMC groups, but deduct pilot cost for non-MFMC groups
  SizetArray ratios_to_groups(numApprox+1);  UShortArray group;  size_t g_index;
  bool no_retain_throttle = retainedModelGroups.empty();
  for (size_t r=0; r<=numApprox; ++r) {
    cvmc_model_group(r, group); // the r-th CVMC group
    g_index = find_index(modelGroups, group);
    ratios_to_groups[r] = (no_retain_throttle || retainedModelGroups[g_index])
                        ? g_index : _NPOS;
  }
  analytic_ratios_to_solution_variables(avg_eval_ratios,ratios_to_groups,soln);
}


void NonDMultilevBLUESampling::
analytic_ratios_to_solution_variables(RealVector& avg_eval_ratios,
				      const SizetArray& ratios_to_groups,
				      MFSolutionData& soln)
{
  // For analytic MFMC/CVMC initial guesses, the best ref for avg_eval_ratios
  // is all_group, which is enforced to be present for all throttle cases.
  size_t all_group = numGroups - 1; // for all throttles

  Real hf_target;
  if (maxFunctionEvals == SZ_MAX) { // accuracy-constrained -> online only
    // As for {ACV,GenACV}, employ ML BLUE's native estvar for accuracy scaling
    RealVector soln_vars, mlblue_ev, mlblue_ev_ratios;//ratios remain empty
    SizetArray N_shared;  Real metric;  size_t metric_index;
    // Rather than estimating hf_target for MFMC/CVMC (based on estvar and
    // estVarIter0 from MFMC/CVMC), we instead employ pilot_samp to provide a
    // baseline estvar for ML BLUE, and then scale toward an hf_target for
    // ML BLUE based on this estvar relative to estVarIter0.
    size_t pilot_samp = pilotSamples[all_group];
    ratios_target_to_solution_variables(avg_eval_ratios, (Real)pilot_samp,
					ratios_to_groups, soln_vars);
    enforce_bounds_linear_constraints(soln_vars); // for valid estvar
    estimator_variances(soln_vars, mlblue_ev); // compute ML BLUE estvar
    MFSolutionData::update_estimator_variance_metric(estVarMetricType,
      estVarMetricNormOrder, mlblue_ev_ratios, mlblue_ev, metric, metric_index);
    // the assumed scaling with N_sh is not generally valid for ML BLUE,
    // but is reasonable for emulation of MFMC
    N_shared.assign(numFunctions, pilot_samp); // online
    hf_target = update_hf_target(mlblue_ev, metric_index, N_shared,estVarIter0);
  }
  else { // budget-constrained -> online or offline
    bool offline = (pilotMgmtMode == OFFLINE_PILOT ||
		    pilotMgmtMode == OFFLINE_PILOT_PROJECTION);
    size_t pilot_all = pilotSamples[all_group];
    Real budget = (Real)maxFunctionEvals, offline_N_lwr = 1.,
      N_shared = (offline) ? offline_N_lwr : (Real)pilot_all;
    if (!offline) {
      BitArray active_g(numGroups); // init to false
      size_t g, r, num_r = ratios_to_groups.size(); // numApprox+1
      for (r=0; r<num_r; ++r) {
	g = ratios_to_groups[r];
	if (g != _NPOS) active_g.set(g); // flip bit to on
      }
      // For analytic initial guesses, mirror the budget deductions used in
      // available_budget() for ensemble_numerical_solution()
      switch (pilotGroupSampling) {
      case SHARED_PILOT: {
	BitArray active_m;  retained_groups_to_models(active_g, active_m);
	deduct_inactive_model_costs(active_m, pilot_all,    budget);  break;
      }
      case INDEPENDENT_PILOT: // budget deductions are group-based
	deduct_inactive_group_costs(active_g, pilotSamples, budget);  break;
      }
    }
    if (budget > 0.)
      // scale_to_target() employs allocate_budget() and rescales for lower bnds
      scale_to_target(N_shared, sequenceCost, avg_eval_ratios, hf_target,
		      budget, offline_N_lwr);
    else // budget exhausted
      { hf_target = N_shared; avg_eval_ratios = 1.; }
  }

  RealVector mlblue_vars;
  ratios_target_to_solution_variables(avg_eval_ratios, hf_target,
				      ratios_to_groups, mlblue_vars);
  enforce_bounds_linear_constraints(mlblue_vars); // enforce again
  soln.solution_variables(mlblue_vars);

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Analytic initialization for local solution in ML BLUE:\n";
    print_group_solution_variables(Cout, soln);
  }
}


void NonDMultilevBLUESampling::
ratios_target_to_solution_variables(const RealVector& avg_eval_ratios,
				    Real hf_target,
				    const SizetArray& ratios_to_groups,
				    RealVector& soln_vars)
{
  // Convert avg_{eval_ratios,hf_target} for MFMC/CVMC to soln_vars for ML BLUE
  // > We assume for now that the MFMC/CVMC groups for which eval ratios are
  //   defined are ordered consistently as a subset of ML BLUE's modelGroups,
  //   allowing use of a subset cntr.  Note that avg_eval_ratios retains model
  //   ordering even when an approx_sequence is defined.
  // > Future use of restricted model grouping sets could require a look-up of
  //   MFMC/CVMC model groupings within modelGroups.

  // Initialize soln_vars
  size_t v, num_v = num_active_groups(), g;
  if (soln_vars.length() != num_v) soln_vars.size(num_v); // init to 0
  else                             soln_vars = 0.;
  if (pilotMgmtMode == ONLINE_PILOT ||
      pilotMgmtMode == ONLINE_PILOT_PROJECTION) {
    if (pilotGroupSampling == SHARED_PILOT) {
      // last group = all models is always present in modelGroups,
      // but may be omitted due to active group throttling
      size_t all_group = numGroups - 1;
      v = all_to_active_group(all_group); // group g -> active v
      if (v != _NPOS)
	soln_vars[v] = (Real)pilotSamples[all_group];
    }
    else // INDEPENDENT_PILOT
      for (v=0; v<num_v; ++v) {
	g = active_to_all_group(v);       // active v -> group g
	if (g != _NPOS)
	  soln_vars[v] = (Real)pilotSamples[g];
      }
  }

  // Define soln_vars for active groups using avg_{eval_ratios,hf_target}
  size_t r, num_r = avg_eval_ratios.length();  Real N_samp;
  for (r=0; r<=num_r; ++r) {
    g = ratios_to_groups[r];      // eval ratio r -> group g
    if (g != _NPOS) {
      v = all_to_active_group(g); // group g -> active v
      N_samp = hf_target;
      if (r < num_r)             N_samp *= avg_eval_ratios[r];
      if (N_samp > soln_vars[v]) soln_vars[v] = N_samp; // emerge from pilot
    }
  }
}


void NonDMultilevBLUESampling::
process_group_allocations(MFSolutionData& soln, const Sizet2DArray& N_G_actual,
			  const SizetArray& N_G_alloc, SizetArray& delta_N_G)
{
  // compute sample increment for HF from current to target:

  // first relax on real values and then round to delta_N_G
  // > don't need logic for different modes: via the XML groupings,
  //   relaxFactor will default to 1 if not ONLINE_PILOT
  RealVector inflated_vars;
  inflate(soln.solution_variables(), retainedModelGroups, inflated_vars);
  if (backfillFailures)
    one_sided_delta(N_G_actual, inflated_vars, delta_N_G, relaxFactor);
  else
    one_sided_delta(N_G_alloc,  inflated_vars, delta_N_G, relaxFactor);

  // Employ projected MC estvar as reference to the projected ML BLUE estvar
  // from N* (where N* may include a num_samples increment not yet performed).
  // Two reference points are used for ML BLUE:
  // 1. For HF-only, employ var_H / projected-N_H --> this is the closest
  //    thing to the estvar ratio (1. - R^2)
  // 2. For equivalent HF, emply var_H / (equivHFEvals + deltaEquivHF)
  size_t ref_group, ref_model_index;
  switch (pilotMgmtMode) {
  case OFFLINE_PILOT:  case OFFLINE_PILOT_PROJECTION:
    // no online samp, so use delta for max projection (covGG assumed accurate)
    find_hf_sample_reference(delta_N_G,  ref_group, ref_model_index);  break;
  default: // define online ref from group with max HF samples (best varH)
    find_hf_sample_reference(N_G_actual, ref_group, ref_model_index);  break;
  }

  // Recompute full estvar vectors for final solution since numerical solves
  // target average over QoI (don't update vectors on every eval since last
  // eval may differ from final optimal soln)
  RealVector estvar;//, cd_vars; solution_to_design_vars(soln, cd_vars);
  estimator_variances(/*cd_vars*/soln.solution_variables(), estvar);
  soln.estimator_variances(estvar);

  if (ref_group == _NPOS) { // no online HF samples
    Real q_nan = std::numeric_limits<Real>::quiet_NaN();
    projEstVarHF = q_nan; // all QoI
    projNActualHF.size(numFunctions); // set to 0
    soln.initialize_estimator_variance_ratios(numFunctions, q_nan); 
  }
  else {
    // Note: estvar is nan for 1 HF sample since bessel corr divides by 0
    project_mc_estimator_variance(covGG[ref_group], ref_model_index,
				  N_G_actual[ref_group], delta_N_G[ref_group],
				  projEstVarHF, projNActualHF);
    // Recompute full estvar vectors for final solution since minimizers
    // target average over QoI (don't store on every eval since last eval
    // may differ from final optimal soln)
    soln.initialize_estimator_variance_ratios(numFunctions);
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      soln.estimator_variance_ratio(estvar[qoi] / projEstVarHF[qoi], qoi); 
  }

  soln.update_estimator_variance_metric(estVarMetricType,estVarMetricNormOrder);
}


void NonDMultilevBLUESampling::
print_group_allocations(std::ostream& s, const MFSolutionData& soln)
{
  print_group_solution_variables(s, soln);

  if (maxFunctionEvals == SZ_MAX)
    s << "Estimator cost allocation = " << soln.equivalent_hf_allocation()
      << "\nequivHFEvals = " << equivHFEvals << " deltaEquivHF = "
      << deltaEquivHF << std::endl;
  else {
    s << "Estimator variance metric = " << soln.estimator_variance_metric()
      << std::endl;
  }
}


void NonDMultilevBLUESampling::
print_group_solution_variables(std::ostream& s, const MFSolutionData& soln)
{
  const RealVector& soln_vars = soln.solution_variables();
  size_t g, num_g = modelGroups.size(), cntr = 0;
  bool no_retain_throttle = retainedModelGroups.empty();
  s << "Numerical solution for samples per model group:\n";
  for (g=0; g<num_g; ++g) {
    if (no_retain_throttle || retainedModelGroups[g]) {
      s << "  Group " << g << " samples = " << soln_vars[cntr++];
      print_group(s, g);
    }
  }
}


void NonDMultilevBLUESampling::
finalize_counts(const Sizet2DArray& N_G_actual, const SizetArray& N_G_alloc)
{
  // post final sample counts back to NLev{Actual,Alloc} (for final summaries)

  // For now, overlay group samples into model-resolution instance samples
  size_t g, m, num_models, q, mf, rl, m_index;
  const Pecos::ActiveKey& active_key = iteratedModel.active_model_key();
  for (g=0; g<numGroups; ++g) {
    const UShortArray& group_g = modelGroups[g];
    num_models = group_g.size();

    const SizetArray& N_G_actual_g = N_G_actual[g];
    size_t            N_G_alloc_g  = N_G_alloc[g];
    for (m=0; m<num_models; ++m) {
      m_index = group_g[m];
      mf = active_key.retrieve_model_form(m_index);
      rl = active_key.retrieve_resolution_level(m_index);

      NLevAlloc[mf][rl] += N_G_alloc_g;
      SizetArray& N_l_actual_fl = NLevActual[mf][rl];
      if (N_l_actual_fl.empty()) N_l_actual_fl = N_G_actual_g;
      else     increment_samples(N_l_actual_fl,  N_G_actual_g);
    }
  }
}


void NonDMultilevBLUESampling::
blue_raw_moments(const IntRealMatrixArrayMap& sum_G_online,
		 const IntRealSymMatrix2DArrayMap& sum_GG_online,
		 const Sizet2DArray& N_G_online)
{
  // For ONLINE_PILOT where covar and sums use latest accumulations

  RealMatrix H_raw_mom(4, numFunctions);  RealVectorArray mu_hat;
  for (int mom=1; mom<=4; ++mom) {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    const RealMatrixArray&       sum_G_online_m  = sum_G_online.at(mom);
    const RealSymMatrix2DArray& sum_GG_online_m = sum_GG_online.at(mom);

    if (mom == 1) // Use online covar + online sum to solve for mean
      compute_mu_hat(covGGinv, sum_G_online_m, N_G_online, mu_hat);
    else { // compute covariances for higher-order moment solves
      RealSymMatrix2DArray cov_GG, cov_GG_inv;
      compute_GG_covariance(sum_G_online_m, sum_GG_online_m, N_G_online,
			    cov_GG, cov_GG_inv);
      //prune_model_groups(); // "Against" selected
      compute_mu_hat(cov_GG_inv, sum_G_online_m, N_G_online, mu_hat);
    }

    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      H_raw_mom(mom-1, qoi) = mu_hat[qoi][numApprox]; // last model
  }

  convert_moments(H_raw_mom, momentStats);
  compute_mean_confidence_intervals(momentStats,
    final_solution_data().estimator_variances(), meanCIs);
}


void NonDMultilevBLUESampling::
blue_raw_moments(const IntRealMatrixArrayMap& sum_G_offline,
		 const IntRealSymMatrix2DArrayMap& sum_GG_offline,
		 const Sizet2DArray& N_G_offline,
		 const IntRealMatrixArrayMap& sum_G_online,
		 const IntRealSymMatrix2DArrayMap& sum_GG_online,
		 const Sizet2DArray& N_G_online)
{
  // For OFFLINE_PILOT where covar is offline and sums are online

  // Ambiguity arises when ML BLUE reuses covGG for moment solves (also occurs
  // with control variate beta in MFMC/ACV/GenACV).
  // > approach above with offline pilot + online-only moment roll-up incurs
  //   issues with sample reqmts for online covar (offline_N_lwr = 2).
  // > This can be avoided in approach below by mixing offline var/covar +
  //   online sample accumulations: var/covar are consistently offline/Oracle,
  //   but mu-hat solves are inconsistent in online/offline data.
  // Prefer to avoid additional padding in linear constraints for online opt.

  RealMatrix H_raw_mom(4, numFunctions);  RealVectorArray mu_hat;
  size_t all_group = numGroups - 1;
  for (int mom=1; mom<=4; ++mom) {
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    const RealMatrixArray&       sum_G_online_m  = sum_G_online.at(mom);
    const RealSymMatrix2DArray& sum_GG_online_m = sum_GG_online.at(mom);

    // This approach has consistency in that it uses the same var/covar data
    // throughout, but inconsistency in mixing offline group covariances with
    // online sums for compute_mu_hat(). Note: could precompute offline covar
    // for all moments or store offline sums for these covariances, deferring
    // computation until needed here.  Using the latter approach for now.
    if (mom == 1) // Use offline covar + online sum to solve for mean
      compute_mu_hat(covGGinv, sum_G_online_m, N_G_online, mu_hat);
    else {
      const RealMatrixArray&       sum_G_offline_m  = sum_G_offline.at(mom);
      const RealSymMatrix2DArray& sum_GG_offline_m = sum_GG_offline.at(mom);
      // compute covariances for higher-order moments using offline sums
      RealSymMatrix2DArray cov_GG, cov_GG_inv;
      if (pilotGroupSampling == SHARED_PILOT)
	compute_GG_covariance( sum_G_offline_m[all_group],
			      sum_GG_offline_m[all_group],
			      N_G_offline[all_group], cov_GG, cov_GG_inv);
      else
	compute_GG_covariance(sum_G_offline_m, sum_GG_offline_m, N_G_offline,
			      cov_GG, cov_GG_inv);
      // Update model group pruning or stick with set from final iteration?
      // (Psi solve still needed although optimization cycles are complete.)
      // For:     online:  condition rankings will change for higher moments
      //          offline: online dataset differs, perhaps significantly
      // Against: sample allocations were based on final iteration's pruning
      //          and we should utilize this final set for the final moments
      //prune_model_groups(); // "Against" selected

      // solve for mu-hat using online sums
      compute_mu_hat(cov_GG_inv, sum_G_online_m, N_G_online, mu_hat);
    }

    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      H_raw_mom(mom-1, qoi) = mu_hat[qoi][numApprox]; // last model
  }

  convert_moments(H_raw_mom, momentStats);
  compute_mean_confidence_intervals(momentStats,
    final_solution_data().estimator_variances(), meanCIs);
}


void NonDMultilevBLUESampling::
print_multigroup_summary(std::ostream& s, const String& summary_type,
			 bool projections) const
{
  size_t wpp7 = write_precision + 7, g, num_groups = NGroupAlloc.size(),
    m, num_models;

  s << "<<<<< " << summary_type << "allocation of samples per model group:\n";
  for (g=0; g<num_groups; ++g) {
    s << "                     " << std::setw(wpp7) << NGroupAlloc[g]
      << "  QoI_group" << g;
    print_group(s, g);
  }

  if (projections || differ(NLevAlloc, NLevActual)) {
    s << "<<<<< Online accumulated samples per model group:\n";
    for (g=0; g<num_groups; ++g) {
      const SizetArray& N_G_g = NGroupActual[g];
      if (!N_G_g.empty()) {
	s << "                     " << std::setw(wpp7) << N_G_g[0];
	if (!homogeneous(N_G_g)) { // print all counts in this 1D array
	  size_t q, num_q = N_G_g.size();
	  for (size_t q=1; q<num_q; ++q)
	    s << ' ' << N_G_g[q];
	}
	s << "  QoI_group" << g;
	print_group(s, g);
      }
    }
  }
}


void NonDMultilevBLUESampling::print_variance_reduction(std::ostream& s) const
{
  const RealVector&  mlblue_est_var = blueSolnData.estimator_variances();
  const RealVector&  mlblue_ratios  = blueSolnData.estimator_variance_ratios();
  const StringArray& labels = iteratedModel.truth_model().response_labels();
  Real mlblue_est_var_q, mlblue_ratio_q, proj_equiv_estvar_q;
  size_t qoi, wpp7 = write_precision+7,
    proj_equiv_hf = (size_t)std::floor(equivHFEvals + deltaEquivHF + .5);
  String method = " ML BLUE",
     pilot_type = (pilotGroupSampling == SHARED_PILOT) ? "share" : "indep",
           type = (pilotMgmtMode ==  ONLINE_PILOT_PROJECTION ||
		   pilotMgmtMode == OFFLINE_PILOT_PROJECTION)
                ? "Projected" : "   Online";
  bool online = (pilotMgmtMode == ONLINE_PILOT ||
		 pilotMgmtMode == ONLINE_PILOT_PROJECTION),
    mc_only_ref = !zeros(projNActualHF);

  // search for the most refined covGG[g][qoi](H,H)
  size_t ref_group, ref_model_index, all_group = numGroups - 1;
  switch (pilotMgmtMode) {
  case OFFLINE_PILOT:  case OFFLINE_PILOT_PROJECTION:
    ref_group = numGroups - 1;  ref_model_index = numApprox;             break;
  default: // define online ref from group with max HF samples (best varH)
    find_hf_sample_reference(NGroupActual, ref_group, ref_model_index);  break;
  }
  // As described in process_group_allocations(), we have two MC references:
  // projected HF-only samples and projected equivalent HF samples.
  RealVector proj_equiv_estvar;
  project_mc_estimator_variance(covGG[ref_group], ref_model_index, equivHFEvals,
                                deltaEquivHF, proj_equiv_estvar);

  s << "<<<<< Variance for mean estimator:\n";
  for (qoi=0; qoi<numFunctions; ++qoi) {
    s << std::setw(14) << labels[qoi] << ":\n"; // mirror print_moments()

    mlblue_est_var_q    = mlblue_est_var[qoi];
    mlblue_ratio_q      = mlblue_ratios[qoi];
    proj_equiv_estvar_q = proj_equiv_estvar[qoi];

    if (online)
      s << "    Initial pilot (" << std::setw(3) << pilotSamples[all_group]
	<< " " << pilot_type << " samples):  " << std::setw(wpp7)
	<< estVarIter0[qoi] << '\n';
    if (mc_only_ref)
      s << "  " << type << " MC    (" << std::setw(6) << projNActualHF[qoi]
	<< " HF samples):  " << std::setw(wpp7) << projEstVarHF[qoi] << '\n';
    s << "  " << type << method << "  (sample profile):  "
      << std::setw(wpp7) << mlblue_est_var_q << '\n';
    if (mc_only_ref && valid_variance(mlblue_ratio_q))
      s << "  " << type << method << " ratio   (1 - R^2):  "
	<< std::setw(wpp7) << mlblue_ratio_q << '\n';
    s << " Equivalent MC    (" << std::setw(6) << proj_equiv_hf
      << " HF samples):  " << std::setw(wpp7) << proj_equiv_estvar_q << '\n';
    if (valid_variance(mlblue_est_var_q) && valid_variance(proj_equiv_estvar_q))
      s << " Equivalent" << method << " ratio:              " << std::setw(wpp7)
	<< mlblue_est_var_q / proj_equiv_estvar_q << '\n';
  }
}


void NonDMultilevBLUESampling::
project_mc_estimator_variance(const RealSymMatrixArray& cov_GG_g,
			      size_t H_index, const SizetArray& N_H_actual,
			      size_t delta_N_H, RealVector& proj_est_var,
			      SizetVector& proj_N_H) const
{
  // Defines projected estvar for use as a consistent reference
  proj_est_var.sizeUninitialized(numFunctions);
  proj_N_H.sizeUninitialized(numFunctions);
  size_t qoi, N_l_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    N_l_q = proj_N_H[qoi] = N_H_actual[qoi] + delta_N_H;
    proj_est_var[qoi] = (N_l_q) ? cov_GG_g[qoi](H_index,H_index) / N_l_q :
      std::numeric_limits<Real>::quiet_NaN(); // 0 / 0
  }
}


void NonDMultilevBLUESampling::
project_mc_estimator_variance(const RealSymMatrixArray& cov_GG_g,
			      size_t H_index, Real N_H_actual, Real delta_N_H,
			      RealVector& proj_est_var) const
{
  // Defines projected estvar for use as a consistent reference
  proj_est_var.sizeUninitialized(numFunctions);
  size_t qoi; Real N_l_q = N_H_actual + delta_N_H;
  for (qoi=0; qoi<numFunctions; ++qoi)
    proj_est_var[qoi] = (N_l_q > 0.) ? cov_GG_g[qoi](H_index,H_index) / N_l_q :
      std::numeric_limits<Real>::quiet_NaN(); // 0 / 0
}


/** Multi-moment map-based version used by online pilot */
void NonDMultilevBLUESampling::
accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
		     IntRealSymMatrix2DArrayMap& sum_GG, Sizet2DArray& num_G,
		     const IntIntResponse2DMap& batch_resp_map)
{
  IntIntResponse2DMap::const_iterator b_it;
  size_t g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    b_it = batch_resp_map.find(g); // index g corresponds to group_id key
    if (b_it != batch_resp_map.end()) // else no new evals for this group
      accumulate_blue_sums(sum_G, sum_GG, num_G, g, b_it->second);
  }
}


/** Multi-moment map-based version used by online pilot */
void NonDMultilevBLUESampling::
accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
		     IntRealSymMatrix2DArrayMap& sum_GG, Sizet2DArray& num_G,
		     size_t group, const IntResponseMap& resp_map)
{
  using std::isfinite;  bool all_finite;
  Real g1_fn, g2_fn, g1_prod, g2_prod;
  IntRespMCIter r_cit;  IntRMAMIter g_it;  IntRSM2AMIter gg_it;
  int g_ord, gg_ord, active_ord, ord;
  size_t qoi, m, m2, g1_index, g2_index;

  SizetArray&        num_G_g =       num_G[group];
  const UShortArray& group_g = modelGroups[group];
  size_t          num_models = group_g.size();

  for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit) {
    const Response&   resp    = r_cit->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see Bessel correction notes in NonDNonHierarchSampling::
      // compute_correlation(): population mean and variance should be
      // computed from the same sample set
      all_finite = true;
      for (m=0; m<num_models; ++m) {
	g1_index = group_g[m] * numFunctions + qoi;
	if ( (asv[g1_index] & 1) == 0 ) {
	  Cerr << "Error: missing data for group " << group+1 << " model "
	       << group_g[m]+1 << '.' << std::endl;
	  abort_handler(METHOD_ERROR);
	}
	if ( !isfinite(fn_vals[g1_index]) )
	  all_finite = false; //break;
      }
      if (!all_finite) continue;

      ++num_G_g[qoi]; // shared due to fault tol logic
      for (m=0; m<num_models; ++m) {
	g1_index = group_g[m] * numFunctions + qoi;
	g1_fn = fn_vals[g1_index];

	g_it    = sum_G.begin();  gg_it = sum_GG.begin();
	g_ord   = (g_it  ==  sum_G.end()) ? 0 :  g_it->first;
	gg_ord  = (gg_it == sum_GG.end()) ? 0 : gg_it->first;
	g1_prod = g1_fn;  active_ord = 1;
	while (g_ord || gg_ord) {
    
	  if (g_ord == active_ord) { // support general key sequence
	    g_it->second[group](qoi,m) += g1_prod;
	    ++g_it;  g_ord = (g_it == sum_G.end()) ? 0 : g_it->first;
	  }
	  if (gg_ord == active_ord) { // support general key sequence
	    RealSymMatrix& sum_GG_gq = gg_it->second[group][qoi];
	    sum_GG_gq(m,m) += g1_prod * g1_prod;
	    // Off-diagonal of C matrix:
	    for (m2=0; m2<m; ++m2) {
	      g2_index = group_g[m2] * numFunctions + qoi;
	      // regenerate g2_prod i/o storing off-diagonal combinations
	      g2_prod = g2_fn = fn_vals[g2_index];
	      for (ord=1; ord<active_ord; ++ord)
		g2_prod *= g2_fn;
	      sum_GG_gq(m,m2) += g1_prod * g2_prod;
	    }
	    ++gg_it; gg_ord = (gg_it == sum_GG.end()) ? 0 : gg_it->first;
	  }

	  g1_prod *= g1_fn;  ++active_ord;
	}
      }
    }
  }
}


/** Single moment version used by offline-pilot and pilot-projection */
void NonDMultilevBLUESampling::
accumulate_blue_sums(RealMatrixArray& sum_G, RealSymMatrix2DArray& sum_GG,
		     Sizet2DArray& num_G,
		     const IntIntResponse2DMap& batch_resp_map)
{
  IntIntResponse2DMap::const_iterator b_it;
  size_t g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    b_it = batch_resp_map.find(g); // index g corresponds to group_id key
    if (b_it != batch_resp_map.end())
      accumulate_blue_sums(sum_G[g], sum_GG[g], num_G[g], g, b_it->second);
  }
}


/** Single-moment, single-group version for accumulations following a
    shared pilot sample */
void NonDMultilevBLUESampling::
accumulate_blue_sums(RealMatrix& sum_G_g, RealSymMatrixArray& sum_GG_g,
		     SizetArray& num_G_g, size_t group,
		     const IntResponseMap& resp_map)
{
  using std::isfinite;  bool all_finite;
  IntRespMCIter r_cit;  Real g1_fn;
  const UShortArray& group_g = modelGroups[group];
  size_t qoi, m, m2, g1_index, g2_index, num_models = group_g.size();

  // here we process resp_map from shared_increment()
  for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit) {
    const Response&   resp    = r_cit->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation():
      // population mean and variance should be computed from same sample set
      all_finite = true;
      for (m=0; m<num_models; ++m) {
	g1_index = group_g[m] * numFunctions + qoi;
	if ( (asv[g1_index] & 1) == 0 ) {
	  Cerr << "Error: missing data for group " << group+1 << " model "
	       << group_g[m]+1 << '.' << std::endl;
	  abort_handler(METHOD_ERROR);
	}
	if ( !isfinite(fn_vals[g1_index]) )
	  all_finite = false; //break;
      }
      if (!all_finite) continue;

      ++num_G_g[qoi]; // shared due to fault tol logic
      RealSymMatrix& sum_GG_gq = sum_GG_g[qoi];
      for (m=0; m<num_models; ++m) {
	g1_index = group_g[m] * numFunctions + qoi;
	g1_fn = fn_vals[g1_index];

	sum_G_g(qoi,m) += g1_fn;
	sum_GG_gq(m,m) += g1_fn * g1_fn;

	// Off-diagonal of covariance matrix:
	for (m2=0; m2<m; ++m2) {
	  g2_index = group_g[m2] * numFunctions + qoi;
	  sum_GG_gq(m,m2) += g1_fn * fn_vals[g2_index];
	}
      }
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In accumulate_blue_sums(), sum_G[" << group << "]:\n" << sum_G_g
	 << "sum_GG[" << group << "]:\n" << sum_GG_g
	 << "num_G["  << group << "]:\n" << num_G_g << std::endl;
}


/** overload version for independent group samples */
void NonDMultilevBLUESampling::
compute_GG_covariance(const RealMatrixArray& sum_G,
		      const RealSymMatrix2DArray& sum_GG,
		      const Sizet2DArray& num_G, RealSymMatrix2DArray& cov_GG,
		      RealSymMatrix2DArray& cov_GG_inv,
		      const SizetArray& N_G_ref)
{
  initialize_rsm2a(cov_GG);  initialize_rsm2a(cov_GG_inv); // bypass if sized

  size_t g, m, m2, num_models, qoi, num_G_gq;
  Real sum_G_gqm;  RealVector rcond(numFunctions);
  bool rcond_throttle  = (groupThrottleType == RCOND_TOLERANCE_THROTTLE ||
			  groupThrottleType == RCOND_BEST_COUNT_THROTTLE),
       relative_update = !N_G_ref.empty();
  if (rcond_throttle) groupCovCondMap.clear();

  for (g=0; g<numGroups; ++g) {
    num_models = modelGroups[g].size();
    const SizetArray&        num_G_g =      num_G[g];
    const RealMatrix&        sum_G_g =      sum_G[g];
    RealSymMatrixArray&     cov_GG_g =     cov_GG[g];
    RealSymMatrixArray& cov_GG_inv_g = cov_GG_inv[g];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      num_G_gq = num_G_g[qoi];
      // ONLINE_PILOT + SHARED_PILOT: only update to independent covar if equal
      // or greater samples than shared.  This sticks with the shared covariance
      // approximation during iterative allocations, until superceded.  Final
      // moment estimation instead relies on final online covariances.
      if ( ( relative_update && num_G_gq >= N_G_ref[qoi]) ||
	   (!relative_update && num_G_gq >  1) ) { // sufficient to define new: *** group covariance omitted for active lb of HF group at 1 online sample ***
	const RealSymMatrix& sum_GG_gq = sum_GG[g][qoi];
	RealSymMatrix&       cov_GG_gq = cov_GG_g[qoi];
	if (cov_GG_gq.empty()) cov_GG_gq.shape(num_models);
	for (m=0; m<num_models; ++m) {
	  sum_G_gqm = sum_G_g(qoi,m);
	  for (m2=0; m2<=m; ++m2)
	    compute_covariance(sum_G_gqm, sum_G_g(qoi,m2), sum_GG_gq(m,m2),
			       num_G_gq, cov_GG_gq(m,m2));
	}
	compute_C_inverse(cov_GG_gq, cov_GG_inv_g[qoi], g, qoi, rcond[qoi]);
      }
      else if (!relative_update) // inadequate samples to define covar
	{ cov_GG_g[qoi].shape(0); cov_GG_inv_g[qoi].shape(0); rcond[qoi] = 0.; }
      //else: leave as previous shared covariance and covariance-inverse
    }
    if (rcond_throttle)
      groupCovCondMap.insert(std::pair<Real,size_t>(average(rcond), g));
  }
}


/** overload version for shared group samples */
void NonDMultilevBLUESampling::
compute_GG_covariance(const RealMatrix& sum_G_g,
		      const RealSymMatrixArray& sum_GG_g,
		      const SizetArray& num_G_g,
		      RealSymMatrix2DArray& cov_GG,
		      RealSymMatrix2DArray& cov_GG_inv)
{
  initialize_rsm2a(cov_GG);

  size_t qoi, m1, m2, num_models = numApprox + 1, num_mapped, g, g1,
    all_group = numGroups - 1;
  Real sum_G_gqm;  size_t num_G_gq;
  RealSymMatrixArray&     cov_GG_g = cov_GG[all_group];// last group, all models
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& sum_GG_gq = sum_GG_g[qoi];
    RealSymMatrix&       cov_GG_gq = cov_GG_g[qoi];
    num_G_gq                       =  num_G_g[qoi];
    if (num_G_gq > 1) {
      if (cov_GG_gq.numRows() != num_models) cov_GG_gq.shape(num_models);
      for (m1=0; m1<num_models; ++m1) {
	sum_G_gqm = sum_G_g(qoi,m1);
	for (m2=0; m2<=m1; ++m2)
	  compute_covariance(sum_G_gqm, sum_G_g(qoi,m2), sum_GG_gq(m1,m2),
			     num_G_gq, cov_GG_gq(m1,m2));
      }
      // map shared covariance into partial groups
      for (g=0; g<all_group; ++g) {
	RealSymMatrix& cov_GG_mapped = cov_GG[g][qoi];
	const UShortArray& group_g = modelGroups[g];
	num_mapped = group_g.size();
	if (cov_GG_mapped.numRows() != num_mapped)
	  cov_GG_mapped.shape(num_mapped);
	for (m1=0; m1<num_mapped; ++m1) {
	  g1 = group_g[m1];
	  for (m2=0; m2<=m1; ++m2)
	    cov_GG_mapped(m1,m2) = cov_GG_gq(g1, group_g[m2]);
	}
      }
    }
    else {
      cov_GG_gq.shape(0);
      for (g=0; g<all_group; ++g)
	cov_GG[g][qoi].shape(0);
    }
  }

  // precompute 2D array of C_k inverses for numerical solver use
  // (Phi-inverse is dependent on N_G, but C-inverse is not)
  compute_C_inverse(cov_GG, cov_GG_inv);
}


/*
void NonDMultilevBLUESampling::
compute_G_variance(const RealMatrixArray& sum_G,
		   const RealSymMatrix2DArray& sum_GG,
		   const Sizet2DArray& num_G, RealMatrixArray& var_G)
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
    const SizetArray&          num_G_g =  num_G[g];
    RealMatrix&                var_G_g =  var_G[g];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      const RealSymMatrix&   sum_GG_gq = sum_GG_g[qoi];
      for (m=0; m<num_models; ++m)
	compute_variance(sum_G_g(qoi,m), sum_GG_gq(m,m), num_G_g[qoi],
			 var_G_g(qoi,m));
    }
  }
}
*/


void NonDMultilevBLUESampling::
compute_C_inverse(const RealSymMatrix& cov_GG_gq, RealSymMatrix& cov_GG_inv_gq,
		  size_t group, size_t qoi, Real& rcond)
{
  if (cov_GG_gq.empty()) // insufficient samples to define cov_GG
    { cov_GG_inv_gq.shape(0); rcond = 0.; return; }

  /* This approach has not been effective for ill-conditioned cov_GG:
  int r, nr = cov_GG_gq.numRows();
  cov_GG_inv_gq.shape(nr);
  RealSymMatrix A(cov_GG_gq);  RealMatrix X(nr, nr), B(nr, nr);
  for (r=0; r<nr; ++r) B(r,r) = 1.; // identity
  // Leverage both the soln refinement in solve() and equilibration during
  // factorization (inverting C in place can only leverage the latter).
  RealSpdSolver spd_solver;
  spd_solver.setMatrix( Teuchos::rcp(&A,false));
  spd_solver.setVectors(Teuchos::rcp(&X, false), Teuchos::rcp(&B, false));
  if (spd_solver.shouldEquilibrate())
    spd_solver.factorWithEquilibration(true);
  spd_solver.solveToRefinedSolution(true);
  int code = spd_solver.solve();
  copy_data(X, cov_GG_inv_gq); // Dense to SymDense
  */

  /* Detection of ill-conditioning in Cholesky factorization has been
     observed to be insufficient to prevent blow-up within invert() -->
     use SVD at all times rather than as a fallback.
  cov_GG_inv_gq = cov_GG_gq; // copy for inversion in place
  RealSpdSolver spd_solver;
  spd_solver.setMatrix(Teuchos::rcp(&cov_GG_inv_gq, false));
  // Equilibration scales the system to improve solution conditioning; it
  // involves equilibrateMatrix() and equilibrateRHS() prior to solve,
  // followed by unequilibrateLHS() after solve.  Here, we factor/invert C
  // without equilibration as we assemble C-inverse into Psi without any
  // solve(); otherwise C-inverse would be the inverse of the equilibrated
  // matrix and there is no corresponding unequilibrate to use at that point.
  // Downstream, solves using the assembled Psi are equilibrated as needed.

  // factor() is embedded within both reciprocalConditionEstimate() and
  // invert(), so this return code is the furthest upstream
  int fact_code = spd_solver.factor(); // Real rcond;
  //int rcond_code = spd_solver.reciprocalConditionEstimate(rcond);
  //int   inv_code = spd_solver.invert(); // in place
  if (fact_code) { // only traps extreme cases
    Cerr << "Warning: failure in group covariance factorization in ML BLUE::"
         << "compute_C_inverse()\n         for group " << group << " QoI "
	 << qoi //<< " with C:\n" << cov_GG_gq
	 << " (LAPACK error: leading minor of order " << fact_code
	 << " is not positive definite,\n       and the factorization could "
	 << "not be completed).  Resorting to pseudo-inverse via SVD."
	 << std::endl;

    // This drops the group contribution to Psi but probably also need
    // to drop the group design var from the numerical soln to prevent
    // unconstrained behavior there.
    cov_GG_inv_gq.shape(0);
  }
  else { // bad inverses can still occur
    spd_solver.invert();
    //if (outputLevel >= DEBUG_OUTPUT)
      Cout << "LL^T inverse for group " << group << " QoI " << qoi << ":\n"
           << cov_GG_inv_gq << std::endl;
  }
  */

  /*
  // SYEV not consistent with Cholesky-based inverse for well conditioned
  // Need to review Petra2014
  RealVector eigen_vals;  RealMatrix eigen_vecs;
  symmetric_eigenvalue_decomposition( cov_GG_gq, eigen_vals, eigen_vecs );
  //if (outputLevel >= DEBUG_OUTPUT)
  //  Cout << "SYEV eigenvalues for " << group << " QoI " << qoi << ":\n"
  // 	     << eigen_vals << std::endl;
  // Form V and D
  Real ev_tol = std::sqrt(DBL_EPSILON); // Petra2014 suggests tol=1 in Fig 5.2
  int n, r, num_rows = eigen_vals.length(), num_neglect = 0;
  for (n=0; n<num_rows; ++n) // returned in increasing order
    if ( eigen_vals[n] <= ev_tol ) ++num_neglect;
    else                           break;
  int num_low_rank = num_rows - num_neglect, offset_r;
  RealSymMatrix D(num_low_rank); // init to 0;    r x r diagonal matrix
  RealMatrix V(num_rows, num_low_rank, false); // n x r matrix for r retained
  for (r=0; r<num_low_rank; ++r) {
    offset_r = r + num_neglect;
    Real lambda = eigen_vals[offset_r];
    D(r,r) = lambda / (lambda + 1.); // Sherman-Morrison-Woodbury
    for (n=0; n<num_rows; ++n)
      V(n,r) = eigen_vecs(n,offset_r); // copy column
  }
  // Form inverse = I - V D V^T
  // inv(hessian) of posterior = L (I - V D V^T) L^T for prior Cholesky L
  cov_GG_inv_gq.shapeUninitialized(num_rows);
  Teuchos::symMatTripleProduct(Teuchos::NO_TRANS, -1., D, V, cov_GG_inv_gq);
  for (n=0; n<num_rows; ++n)
    cov_GG_inv_gq(n,n) += 1.;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Pseudo-inverse by SYEV for group " << group << " QoI " << qoi
	 << ":\n" << cov_GG_inv_gq << std::endl;
  */

  // Rely on SVD (full or pseudo-inverse as dictated by singular vals)
  RealMatrix A, A_inv;
  copy_data(cov_GG_gq, A);         // RealSymMatrix to RealMatrix
  pseudo_inverse(A, A_inv, rcond);
  copy_data(A_inv, cov_GG_inv_gq); // RealMatrix to RealSymMatrix

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In compute_C_inverse() for group " << group << " QoI " << qoi
	 << ", covariance =\n" << cov_GG_gq << "Pseudo-inverse by truncated "
	 << "SVD: rcond = " << rcond << ", inverse covariance =\n"
	 << cov_GG_inv_gq << "\n--------------\n" << std::endl;
 
  // Alternatives:
  // > Pseudo-inverse for covariances: is symmetric_eigenvalue_decomp()
  //   preferred to SVD for case of symmetric matrices?
  // > SVD for both C and Psi? or just for Psi while dropping C_k groups that
  //   are ill-conditioned? --> factor() has inadequate detection so would
  //   still access to eigen/singular values).
  // > Schaden and Ullmann suggest + \delta I nugget offset, but again this
  //   seems best combined with a detection scheme
  // > SDP solvers (helps only with Psi solve --> issues with C-inverse must
  //   be addressed separately)
}


void NonDMultilevBLUESampling::
compute_mu_hat(const RealSymMatrix2DArray& cov_GG_inv,
	       const RealMatrixArray& sum_G, const Sizet2DArray& N_G,
	       RealVectorArray& mu_hat)
{
  // accumulate Psi but don't invert in place
  RealSymMatrixArray Psi;
  compute_Psi(cov_GG_inv, N_G, Psi);

  // Only need to form y when solving for mu-hat:
  RealVectorArray y;
  compute_y(cov_GG_inv, sum_G, y);

  initialize_rva(mu_hat, false);
  size_t q, r, c, g, num_groups = modelGroups.size();
  /*
  RealSpdSolver spd_solver;
  for (q=0; q<numFunctions; ++q) {
    // Leverage both the soln refinement in solve() and equilibration during
    // factorization (inverting Psi in place can only leverage the latter).
    spd_solver.setMatrix(Teuchos::rcp(&Psi[q], false)); // resets solver state
    spd_solver.setVectors(Teuchos::rcp(&mu_hat[q], false),
			  Teuchos::rcp(&y[q], false));
    if (spd_solver.shouldEquilibrate())
      spd_solver.factorWithEquilibration(true);
    spd_solver.solveToRefinedSolution(true);
    int code = spd_solver.solve();
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in ML BLUE compute_mu_hat()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Cholesky solve for mu_hat for QoI " << q << ":\n" << mu_hat[q]
	   << std::endl;
  }
  */

  RealMatrix A, A_inv;  Real rcond;
  for (q=0; q<numFunctions; ++q) {
    copy_data(Psi[q], A); // RealSymMatrix to RealMatrix
    pseudo_inverse(A, A_inv, rcond);
    mu_hat[q].multiply(Teuchos::NO_TRANS,Teuchos::NO_TRANS,1., A_inv, y[q], 0.);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Pseudo-inverse solve for mu_hat for QoI " << q << ":\n"
	   << mu_hat[q] << std::endl;
  }
}


void NonDMultilevBLUESampling::
estimator_variances(const RealVector& cd_vars, RealVector& est_var)
{
  if (est_var.empty()) est_var.sizeUninitialized(numFunctions);

  // This approach leverages both the solution refinement in solve() and
  // equilibration during factorization (inverting Psi in place can only
  // leverage the latter).  It seems to work much more reliably.
  RealSymMatrixArray Psi;
  compute_Psi(covGGinv, cd_vars, Psi);

  size_t q, all_models = numApprox + 1;

  /*
  RealSpdSolver spd_solver;
  RealVector e_last(all_models, false), estvar_q(all_models, false);
  for (q=0; q<numFunctions; ++q) {
    // e_last is equilbrated in place, so must be reset
    e_last.putScalar(0.); e_last[numApprox] = 1.;

    spd_solver.setMatrix( Teuchos::rcp(&Psi[q],   false));// resets solver state
    spd_solver.setVectors(Teuchos::rcp(&estvar_q, false),
			  Teuchos::rcp(&e_last,   false));
    if (spd_solver.shouldEquilibrate())
      spd_solver.factorWithEquilibration(true);
    spd_solver.solveToRefinedSolution(true);
    int code = spd_solver.solve();
    if (code) {
      Cerr << "Error: serial dense solver failure (LAPACK error code " << code
	   << ") in ML BLUE estimator_variances()." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    estvar[q] = estvar_q[numApprox];
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Cholesky       solve for estvar for QoI " << q << " = "
	   << estvar[q] << std::endl;
  }
  */

  // Psi and e_last need to be reset if Cholesky also active above
  //compute_Psi(covGGinv, cd_vars, Psi);
  RealMatrix A, A_inv;  Real rcond;
  for (q=0; q<numFunctions; ++q) {
    copy_data(Psi[q], A); // RealSymMatrix to RealMatrix
    pseudo_inverse(A, A_inv, rcond);
    est_var[q] = A_inv(numApprox,numApprox);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Pseudo-inverse solve for est_var for QoI " << q << " = "
	   << est_var[q] << std::endl;
  }

  // Revisit this flow:
  // should be able to equilibrate and factor each Psi once within a SpdSolver
  // that persists, then solve to refined solution for each est_var and mu-hat.

  /* This approach suffers from poor performance, either from conditioning
     issues or misunderstood Teuchos solver behavior.
  RealSymMatrixArray Psi_inv;
  compute_Psi_inverse(covGGinv, cd_vars, Psi_inv);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    est_var[qoi] = Psi_inv[qoi](numApprox,numApprox); // e_l^T Psi-inverse e_l
  */
}


void NonDMultilevBLUESampling::prune_model_groups()
{
  if (groupThrottleType != RCOND_BEST_COUNT_THROTTLE &&
      groupThrottleType != RCOND_TOLERANCE_THROTTLE )
    { retainedModelGroups.clear(); retainedModels.clear(); return; }

  if (retainedModelGroups.size() != numGroups)
    retainedModelGroups.resize(numGroups);
  retainedModelGroups.reset();
  std::multimap<Real, size_t>::iterator rc_it = groupCovCondMap.begin();

  switch (groupThrottleType) {
  case RCOND_BEST_COUNT_THROTTLE: {
    if (numGroups <= rCondBestThrottle) return;
    Cout << "Pruning model groups from " << numGroups << " to best "
	 << rCondBestThrottle << " based on group covariance conditioning.\n";

    size_t skip_front = numGroups - rCondBestThrottle;
    std::advance(rc_it, skip_front);

    if (outputLevel >= DEBUG_OUTPUT)
      for (std::multimap<Real, size_t>::iterator it=groupCovCondMap.begin();
	   it != rc_it; ++it)
	Cout << "Discard: rcond = "<< it->first<< " group = "<<it->second<<'\n';

    for (; rc_it!=groupCovCondMap.end(); ++rc_it) {
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Retain: rcond = " << rc_it->first << " group = "
	     << rc_it->second << '\n';
      retainedModelGroups.set(rc_it->second);
    }
    break;
  }
  case RCOND_TOLERANCE_THROTTLE:
    Cout << "Pruning model groups based on rcond tolerance = "
	 << rCondTolThrottle << " for group covariances.\n";

    for (; rc_it!=groupCovCondMap.end(); ++rc_it)
      if (rc_it->first < rCondTolThrottle) {
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "Discard: rcond = " << rc_it->first << " group = "
	       << rc_it->second << '\n';
      }
      else break; // out of for loop

    for (; rc_it!=groupCovCondMap.end(); ++rc_it) {
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Retain: rcond = " << rc_it->first << " group = "
	     << rc_it->second << '\n';
      retainedModelGroups.set(rc_it->second);
    }
    break;
  }

  // Need at least 1 group that contains the HF reference model
  //
  // APPROACH 1: always include all_group (as with other throttles)
  // > this group always has shared/indep pilot samples for online, but
  //   covariance may be poorly conditioned
  bool online = (pilotMgmtMode == ONLINE_PILOT ||
		 pilotMgmtMode == ONLINE_PILOT_PROJECTION);
  if (online && pilotGroupSampling == SHARED_PILOT) { // most common cases
    // logic for both online modes is to leverage disproportionate investment
    // in all_group, irregardless of its conditioning.  Further, ONLINE_PILOT
    // mode iterates based on increments to the shared all_group in
    // shared_covariance_iteration().
    size_t all_group = numGroups - 1;
    if (!retainedModelGroups[all_group]) {
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Augment: add HF group = " << all_group << '\n';
      retainedModelGroups.set(all_group);
    }
    // *** TO DO: could adopt APPROACH 2 below and reassign the shared sample
    //            investment to the most expensive retained group
    // *** this enhances group conditioning but discards shared samples for
    //     models omitted from the reassigned group (also increases complexity)
  }
  // APPROACH 2: iff no HF group, add the best-conditioned discard with HF
  // > unlike online all_group, this group may have zero samples
  //   --> enforce_{nudge,bounds}()
  else {//if (!hf_group_retained()) {
    size_t hf_group = best_conditioned_hf_group();
    // if retainedModelGroups[hf_group] then at least 1 HF group is above
    // rcond cutoff, else augment with best HF discard
    if (!retainedModelGroups[hf_group]) {
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Augment: add HF group = " << hf_group << '\n';
      retainedModelGroups.set(hf_group); // Augment
    }
  }

  // Define retainedModels from retainedModelGroups
  retained_groups_to_models(retainedModelGroups, retainedModels);

  if (outputLevel >= DEBUG_OUTPUT) {
    if (retainedModelGroups.empty() || retainedModelGroups.count() == numGroups)
      Cout << "All groups retained\n";
    else {
      Cout << "Retained group count = " << retainedModelGroups.count() << '\n';
      for (size_t g=0; g<numGroups; ++g)
	if (retainedModelGroups[g])
	  Cout << "Retained group " << g << ":\n" << modelGroups[g];
    }
    if (retainedModels.empty() || retainedModels.count() == numApprox+1)
      Cout << "All models retained\n";
    else {
      Cout << "Retained model count = " << retainedModels.count() << '\n';
      for (size_t m=0; m<=numApprox; ++m)
	if (retainedModels[m])
	  Cout << "Retained model " << m << '\n';
    }
  }

  // leave numGroups synchronized with modelGroups and retrieve active count
  // using num_active_groups()
  //numGroups = retainedModelGroups.count();

  // this update performed in ctor for static group allocations
  update_search_algorithm();
}

} // namespace Dakota
