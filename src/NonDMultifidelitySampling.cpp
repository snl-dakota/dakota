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
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultifidelitySampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

static const char rcsId[]="@(#) $Id: NonDMultifidelitySampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultifidelitySampling::
NonDMultifidelitySampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDNonHierarchSampling(problem_db, model),
  numericalSolveMode(problem_db.get_ushort("method.nond.numerical_solve_mode"))
{
  mlmfSubMethod = SUBMETHOD_MFMC; // if needed for numerical solves

  // defining approxSet allows reuse of fns that support model selection
  approxSet.resize(numApprox);
  for (size_t i=0; i<numApprox; ++i)
    approxSet[i] = i;

  // model{Groups,GroupCost} have run-time dependency on approx sequence
  //if (costSource == USER_COST_SPEC) update_model_group_costs(); 

  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    numGroups, pilotSamples);

  size_t max_ps = find_max(pilotSamples);
  if (max_ps) maxEvalConcurrency *= max_ps;
}


NonDMultifidelitySampling::~NonDMultifidelitySampling()
{ }


void NonDMultifidelitySampling::core_run()
{
  //sequence_models(); // enforce correlation condition (*** AFTER PILOT ***)

  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

  switch (pilotMgmtMode) {
  case ONLINE_PILOT:  // iterated MFMC (default)
    // ESTIMATOR_PERFORMANCE case is still iterated for N_H, and therefore
    // differs from ONLINE_PILOT_PROJECTION
    multifidelity_mc_online_pilot();     break;
  case OFFLINE_PILOT: // non-iterated allocation from offline/Oracle correlation
    switch (finalStatsType) {
    // since offline is not iterated, the ESTIMATOR_PERFORMANCE case is the
    // same as OFFLINE_PILOT_PROJECTION --> bypass IntMaps, simplify code
    case ESTIMATOR_PERFORMANCE:  multifidelity_mc_pilot_projection();  break;
    default:                     multifidelity_mc_offline_pilot();     break;
    }
    break;
  case ONLINE_PILOT_PROJECTION:  case OFFLINE_PILOT_PROJECTION:
    multifidelity_mc_pilot_projection(); break;
  }
}


/** This is the standard MFMC version that integrates the pilot alongside
    the sample adaptation and iterates to determine N_H. */
void NonDMultifidelitySampling::multifidelity_mc_online_pilot()
{
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baseline, sum_LL, sum_LH;
  RealVector sum_HH;  RealMatrix var_L;
  initialize_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  while (numSamples && mlmfIter <= maxIterations) {

    // ----------------------------------------------------
    // Evaluate shared increment and increment accumulators
    // ----------------------------------------------------
    shared_increment("mf_"); // spans ALL models, blocking
    accumulate_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH,
		       N_H_actual);
    N_H_alloc += (backfillFailures && mlmfIter) ? one_sided_delta(N_H_alloc,
      mfmcSolnData.solution_reference(), relaxFactor) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the correlation updates.
    if (mlmfIter == 0 && costSource != USER_COST_SPEC)
      recover_online_cost(allResponses);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numGroups,
			      equivHFEvals);

    // -----------------------------------
    // Compute variances and correlations:
    // -----------------------------------
    // First, compute the LF/HF evaluation ratio using shared samples,
    // averaged over QoI.  This includes updating varH and rho2LH.
    compute_LH_correlation(sum_L_baseline[1], sum_H[1], sum_LL[1], sum_LH[1],
			   sum_HH, N_H_actual, var_L, varH, rho2LH);
    // estVarIter0 only uses HF pilot since CV terms (sum_L_shared / N_shared
    // - sum_L_refined / N_refined) cancel out prior to sample refinement.
    if (mlmfIter == 0) cache_mc_reference();

    // ----------------------
    // Solve for allocations:
    // ----------------------
    // compute r*,N* from rho2 and cost, either analytically or numerically
    compute_allocations(rho2LH, varH, N_H_actual, sequenceCost, mfmcSolnData);
    // numSamples is relative to N_H, but the approx_increments() below are
    // computed relative to hf_targets (independent of sunk cost for pilot)
    numSamples = one_sided_delta(N_H_actual, mfmcSolnData.solution_reference(),
				 relaxFactor);
    //numSamples = std::min(num_samp_budget, num_samp_accuracy); // enforce both
    ++mlmfIter;  advance_relaxation();
  }

  // -----------------------------
  // Compute final estvar metrics:
  // -----------------------------
  // Compute estimator variance and variance ratios, which incorporate
  // anticipated variance reduction from upcoming application of r_i ratios.
  // > Note: this could be redundant for tol-based targets with m1* > pilot
  mfmc_estimator_variance(rho2LH, varH, N_H_actual, mfmcSolnData);

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    // N_H_actual is final --> finalize with LF increments + post-processing
    IntRealMatrixMap sum_L_shared, sum_L_refined;
    Sizet2DArray N_L_actual_shared, N_L_actual_refined;
    SizetArray N_L_alloc_refined;  RealVector2DArray beta(4);
    approx_increments(sum_L_baseline, N_H_actual, N_H_alloc, sum_L_shared,
		      N_L_actual_shared, sum_L_refined, N_L_actual_refined,
		      N_L_alloc_refined, mfmcSolnData);
    compute_mf_controls(sum_L_baseline, sum_H, sum_LL, sum_LH, N_H_actual,beta);
    apply_controls(sum_H, N_H_actual, sum_L_shared, N_L_actual_shared,
		   sum_L_refined, N_L_actual_refined, beta);
    finalize_counts(N_L_actual_refined, N_L_alloc_refined);
  }
  else // for consistency with pilot projection
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(mfmcSolnData, N_H_actual, N_H_alloc,
				/*deltaNActualLF,*/ deltaEquivHF);
}


/** This MFMC version treats the pilot sample as a separate offline process. */
void NonDMultifidelitySampling::multifidelity_mc_offline_pilot()
{
  IntRealVectorMap sum_H_pilot;
  IntRealMatrixMap sum_L_pilot, sum_LL_pilot, sum_LH_pilot;
  RealVector sum_HH_pilot(numFunctions);  RealMatrix var_L;
  initialize_mf_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		     sum_HH_pilot);
  SizetArray N_pilot(numFunctions, 0);

  // -------------------------------------------------------------------------
  // Compute final var{L,H},rho2LH from (oracle) pilot treated as offline cost
  // -------------------------------------------------------------------------
  shared_increment("mf_"); // spans ALL models, blocking
  accumulate_mf_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		     sum_HH_pilot, N_pilot);
  if (costSource != USER_COST_SPEC) recover_online_cost(allResponses);
  //increment_equivalent_cost(...); // excluded
  compute_LH_correlation(sum_L_pilot[1], sum_H_pilot[1], sum_LL_pilot[1],
			 sum_LH_pilot[1], sum_HH_pilot, N_pilot,
			 var_L, varH, rho2LH);

  // ---------------------------------
  // Compute online sample increments:
  // ---------------------------------
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  // compute r* from rho2 and cost and update the HF targets
  compute_allocations(rho2LH, varH, N_H_actual, sequenceCost, mfmcSolnData);
  ++mlmfIter;

  // estVarIter0 no longer used for offline mode
  //cache_mc_reference();

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // QOI_STATISTICS case; ESTIMATOR_PERFORMANCE redirects to
  // multifidelity_mc_pilot_projection() to also bypass IntMaps.
  IntRealVectorMap sum_H;  RealVector sum_HH;
  IntRealMatrixMap sum_L_baseline, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH);
  // perform the shared increment for the online sample profile;
  // at least 2 samples reqd for variance (initial N_H_actual = 0)
  numSamples = std::max(one_sided_delta(N_H_actual,
    mfmcSolnData.solution_reference()), (size_t)2); // no relax
  shared_increment("mf_"); // spans ALL models, blocking
  accumulate_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual);
  N_H_alloc += numSamples;
  increment_equivalent_cost(numSamples,sequenceCost,0,numGroups,equivHFEvals);
  // compute the estimator performance metrics
  mfmc_estimator_variance(rho2LH, varH, N_H_actual, mfmcSolnData); // ***

  // finalize with LF increments and post-processing
  IntRealMatrixMap sum_L_shared, sum_L_refined;
  Sizet2DArray N_L_actual_shared, N_L_actual_refined;
  SizetArray N_L_alloc_refined;  RealVector2DArray beta(4);
  approx_increments(sum_L_baseline, N_H_actual, N_H_alloc, sum_L_shared,
		    N_L_actual_shared, sum_L_refined, N_L_actual_refined,
		    N_L_alloc_refined, mfmcSolnData);
  compute_mf_controls(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      N_pilot, beta);
  apply_controls(sum_H, N_H_actual, sum_L_shared, N_L_actual_shared,
		 sum_L_refined, N_L_actual_refined, beta);
  finalize_counts(N_L_actual_refined, N_L_alloc_refined);
}


/** This MFMC version is for algorithm selection; it estimates the
    variance reduction from pilot-only sampling. */
void NonDMultifidelitySampling::multifidelity_mc_pilot_projection()
{
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  RealVector sum_H(numFunctions), sum_HH(numFunctions);
  RealMatrix var_L, sum_L(numFunctions, numApprox),
    sum_LL(numFunctions, numApprox), sum_LH(numFunctions, numApprox);
  // ----------------------------------------------------
  // Evaluate shared increment and increment accumulators
  // ----------------------------------------------------
  shared_increment("mf_"); // spans ALL models, blocking
  if (costSource != USER_COST_SPEC) recover_online_cost(allResponses);
  if (pilotMgmtMode == OFFLINE_PILOT || // redirected here for ESTIMATOR_PERF
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    SizetArray N_shared_pilot(numFunctions, 0);
    accumulate_mf_sums(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot);
    //increment_equivalent_cost(...); // excluded
    compute_LH_correlation(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot,
			   var_L, varH, rho2LH);
  }
  else { // ONLINE_PILOT_PROJECTION
    accumulate_mf_sums(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual);
    N_H_alloc += numSamples;
    increment_equivalent_cost(numSamples,sequenceCost,0,numGroups,equivHFEvals);
    // First, compute the LF/HF evaluation ratio using shared samples,
    // averaged over QoI.  This includes updating varH and rho2LH.
    compute_LH_correlation(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual,
			   var_L, varH, rho2LH);
  }

  // --------------------------
  // Compute evaluation ratios:
  // --------------------------
  // estVarIter0 only uses HF pilot since CV terms (sum_L_shared / N_shared
  // - sum_L_refined / N_refined) cancel out prior to sample refinement.
  cache_mc_reference();
  // compute r* from rho2 and cost
  compute_allocations(rho2LH, varH, N_H_actual, sequenceCost, mfmcSolnData);
  ++mlmfIter;

  // ----------------------
  // Compute EstVar ratios:
  // ----------------------
  // No LF increments or final moments for pilot projection
  update_projected_samples(mfmcSolnData, N_H_actual, N_H_alloc, deltaNActualHF,
			   /*deltaNActualLF,*/ deltaEquivHF);
  // Compute the ratio of MC and MFMC mean squared errors, which incorporates
  // anticipated variance reduction from upcoming application of r_i ratios.
  // > Note: this could be redundant for tol-based targets with m1* > pilot
  SizetArray N_H_actual_proj = N_H_actual;
  increment_samples(N_H_actual_proj, deltaNActualHF);
  mfmc_estimator_variance(rho2LH, varH, N_H_actual_proj, mfmcSolnData);
}


/** LF only */
void NonDMultifidelitySampling::
update_projected_lf_samples(const MFSolutionData& soln,
			    const SizetArray& N_H_actual, size_t& N_H_alloc,
			    //SizetArray& delta_N_L_actual,
			    Real& delta_equiv_hf)
{
  Sizet2DArray N_L_actual;  inflate(N_H_actual, N_L_actual);
  SizetArray   N_L_alloc;   inflate(N_H_alloc,  N_L_alloc);
  size_t qoi, approx, alloc_incr, actual_incr;  Real lf_target;
  const RealVector& soln_vars = soln.solution_variables();
  for (approx=0; approx<numApprox; ++approx) {
    lf_target = soln_vars[approx];
    const SizetArray& N_L_actual_a = N_L_actual[approx];
    size_t&           N_L_alloc_a  = N_L_alloc[approx];
    // No relaxation for projections
    alloc_incr  = one_sided_delta(N_L_alloc_a, lf_target);
    actual_incr = (backfillFailures) ?
      one_sided_delta(N_L_actual_a, lf_target) : alloc_incr;
    /*delta_N_L_actual[approx] += actual_incr;*/  N_L_alloc_a += alloc_incr;
    increment_equivalent_cost(actual_incr, sequenceCost, approx,delta_equiv_hf);
  }

  finalize_counts(N_L_actual, N_L_alloc);
}


/** LF and HF */
void NonDMultifidelitySampling::
update_projected_samples(const MFSolutionData& soln,
			 const SizetArray& N_H_actual, size_t& N_H_alloc,
			 size_t& delta_N_H_actual,
			 //SizetArray& delta_N_L_actual,
			 Real& delta_equiv_hf)
{
  // The N_L baseline is the shared set PRIOR to delta_N_H --> important for
  // cost incr even if lf_targets is defined robustly (hf_targets * eval_ratios)
  update_projected_lf_samples(soln, N_H_actual, N_H_alloc, //delta_N_L_actual,
			      delta_equiv_hf);

  Real hf_target = soln.solution_reference();
  // No relaxation for projections
  size_t alloc_incr = one_sided_delta(N_H_alloc, hf_target),
    actual_incr = (backfillFailures) ? one_sided_delta(N_H_actual, hf_target)
                                     : alloc_incr;
  // For analytic solns, mirror CDV lower bnd for numerical solns; see rationale
  // in NonDNonHierarchSampling::numerical_solution_bounds_constraints()
  if ( ( pilotMgmtMode == OFFLINE_PILOT ||
	 pilotMgmtMode == OFFLINE_PILOT_PROJECTION ) &&
       ( optSubProblemForm == ANALYTIC_SOLUTION ||
	 optSubProblemForm == REORDERED_ANALYTIC_SOLUTION ) ) {
    size_t offline_N_lwr = 1; //(finalStatsType == QOI_STATISTICS) ? 2 : 1;
    alloc_incr  = std::max(alloc_incr,  offline_N_lwr);
    actual_incr = std::max(actual_incr, offline_N_lwr);
  }
  delta_N_H_actual += actual_incr;  N_H_alloc += alloc_incr;
  increment_equivalent_cost(actual_incr, sequenceCost, numApprox,
			    delta_equiv_hf);
}


void NonDMultifidelitySampling::
approx_increments(IntRealMatrixMap& sum_L_baseline,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  IntRealMatrixMap& sum_L_shared,
		  Sizet2DArray& N_L_actual_shared,
		  IntRealMatrixMap& sum_L_refined,
		  Sizet2DArray& N_L_actual_refined,
		  SizetArray& N_L_alloc_refined, const MFSolutionData& soln)
{
  // Note: these results do not affect the HF iteration loop and can be
  // performed after N_H has converged, which simplifies maxFnEvals / convTol
  // logic (no need to further interrogate these throttles below)

  // Notes on approximation sequencing for MFMC:
  // > approx must be ordered on increasing rho2_LH to enable r_i calculation
  //   (see mfmc_analytic_solution() and mfmc_reordered_analytic_solution())
  // > unlike ACV, numerical solutions enforce that this ordering is retained
  //   within r_i through linear constraints in ensemble_numerical_solution()
  //   >> ACV can order approx sample increments based on decreasing r_i
  //      _after_ an unordered ensemble_numerical_solution()
  //   >> potential difference in numerical MFMC vs GenACV for hierarchical DAG
  //      --> consider removing this
  // > analytic cases may be misordered in r_i, so compute a local r_i
  //   ordering here for use incremental sample set reuse.
  switch (optSubProblemForm) {
  case ANALYTIC_SOLUTION:  case REORDERED_ANALYTIC_SOLUTION: // rho ordering
    update_model_groups(corrApproxSequence);  break;
  default: // numerical MFMC: on the fly reordering for r_i
    // final numerical soln may not be last eval: update final sequence
    ordered_approx_sequence(soln.solution_ratios(), ratioApproxSequence, true);
    update_model_groups(ratioApproxSequence); break;
  }
  update_model_group_costs();
  // Important: unlike ML BLUE, modelGroups are only used to facilitate shared
  // sample set groupings in group_increments() and these updates to group
  // definitions do not imply changes to the moment roll-up or peer DAG
  // > upstream use of modelGroupCosts in finite_solution_bounds() is complete
  // > downstream processing is agnostic to modelGroups, consuming the overlaid
  //   {sum,num}_L_{sh,ref}.
  // > If modelGroups are used more broadly in the future, then nested sampling
  //   redefinitions may need to employ group defns local to this function

  // There are a couple options to rework the loop above for batch evaluation:
  // > Minimal: rewrite accumulate_mf_sums() only to process batchResponsesMap
  //   (Note: a general ASV-based batch accumulation is insufficient for the
  //   MFMC shared,refined CV pairings)
  // > Intermediate (SELECTED): rewrite accumulation code to process groups and
  //   then overlay for CV pairs=shared,refined.  Seems preferable to ad hoc
  //   shared,refined logic in accumulate_mf_sums() in that it explicitly maps
  //   from ML BLUE group logic to MFMC paired logic (from "horizontal" group
  //   allocation to "vertical" model allocation in JCP:ACV plots)
  // > Maximal: refactor sums/counters/etc. to be group based.  Note that MFMC
  //   is not natively group-based, but rather based on r_i oversample + reuse
  //   --> overkill at this time.

  // --------------------------------------------
  // Perform approximation increments in parallel
  // --------------------------------------------
  SizetArray delta_N_G(numGroups);  delta_N_G[numApprox] = 0;
  inflate(N_H_actual, N_L_actual_refined);
  inflate(N_H_alloc,  N_L_alloc_refined);
  const RealVector& soln_vars = soln.solution_variables();
  // increment_sample_range() updates the count reference prior to computing
  // the next delta_N_G, such that this in a rolling increment, ordered from
  // base to top of pyramid --> consistent w/ group definitions where previous
  // increments correspond to different groupings.
  for (int g=numApprox-1; g>=0; --g) // base to top, excluding all-model group
    delta_N_G[g]
      = group_approx_increment(soln_vars, approxSet, N_L_actual_refined,
			       N_L_alloc_refined, modelGroups[g]);
  group_increments(delta_N_G, "mf_", true); // reverse order for RNG sequence

  // --------------------------
  // Update sums, counts, costs
  // --------------------------
  // Note: use of this fn requires modelGroupCost to be kept in sync for all
  // cases, not just numerical solves
  increment_equivalent_cost(delta_N_G, modelGroupCost, sequenceCost[numApprox],
			    equivHFEvals);
  IntRealMatrixArrayMap sum_G;  initialize_group_sums(sum_G);
  Sizet2DArray     N_G_actual;  initialize_group_counts(N_G_actual);
  accumulate_group_sums(sum_G, N_G_actual, batchResponsesMap);
  clear_batches();
  // Map from "horizontal" group incr to "vertical" model incr (see JCP: ACV)
  sum_L_shared = sum_L_baseline;  inflate(N_H_actual, N_L_actual_shared);
  overlay_group_sums(sum_G,         N_G_actual,
		     sum_L_shared,  N_L_actual_shared,
		     sum_L_refined, N_L_actual_refined);
}


void NonDMultifidelitySampling::
compute_mf_controls(const IntRealMatrixMap& sum_L_covar,
		    const IntRealVectorMap& sum_H_covar,
		    const IntRealMatrixMap& sum_LL_covar,
		    const IntRealMatrixMap& sum_LH_covar,
		    const SizetArray& N_covar, //const MFSolutionData& soln,
		    RealVector2DArray& beta)
{
  // ------------------------------------
  // Compute/apply CV to estimate moments
  // ------------------------------------
  //precompute_mf_control(soln.solution_variables());

  // Note: ACV-like numerical solutions solve all-at-once for beta as a vector
  // > beta = [ C o F ]^{-1} [diag(F) o c] which, for diagonal F in MFMC,
  //   simplifies to beta_i = F_ii c_i / F_ii C_ii = covLH_i / varL_i
  // --> no need to incur matrix inversion overhead for MFMC; stick with same
  //     scalar approach used for analytic solutions.

  Real sum_H_mq;  size_t approx, qoi, N_base_q;
  for (int mom=1; mom<=4; ++mom) {
    const RealMatrix&  sum_L_m =  sum_L_covar.at(mom);
    const RealVector&  sum_H_m =  sum_H_covar.at(mom);
    const RealMatrix& sum_LL_m = sum_LL_covar.at(mom);
    const RealMatrix& sum_LH_m = sum_LH_covar.at(mom);
    RealVectorArray&    beta_m =         beta[mom-1];
    beta_m.resize(numFunctions);

    for (qoi=0; qoi<numFunctions; ++qoi) {
      RealVector& beta_mq = beta_m[qoi];  beta_mq.sizeUninitialized(numApprox);
      sum_H_mq = sum_H_m[qoi];            N_base_q = N_covar[qoi];
      for (approx=0; approx<numApprox; ++approx)
	// Uses a baseline {sum,N}_L consistent with H,LL,LH accumulators:
	compute_mf_control(sum_L_m(qoi,approx), sum_H_mq,
			   sum_LL_m(qoi,approx), sum_LH_m(qoi,approx),
			   N_base_q, beta_mq[approx]);
    }
  }
}


void NonDMultifidelitySampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  // Numerical MFMC

  // While eval ratio misordering won't induce numerical exceptions,
  // the derivation of the JCP ACV equation assumes ordered r_i:
  // > either retain a fixed ordering through linear constraints (old)
  // > re-order on the fly and omit linear constraint sequencing (new)
  //   (essentially supports search over hierarchical DAGs, similar to
  //   GenACV-MF for width limit = 1)

  // Note: ACV implementation based on F also works for MFMC, but since MFMC
  // has diagonal F, it can be evaluated without per-QoI matrix inversion:
  // > R_sq = a^T [ C o F ]^{-1} a = \Sum_i R_sq_i (sum across set of approx_i)
  // > R_sq_i = F_ii^2 \bar{c}_ii^2 / (F_ii C_ii) for i = approximation number
  //          = F_ii CovLH_i^2 / (VarH_i VarL_i) = F_ii rho2LH_i where
  //   F_ii   = (r_i - r_{i+1}) / (r_i r_{i+1}).
  RealVector r;
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:  case N_MODEL_LINEAR_CONSTRAINT:
    copy_data_partial(cd_vars, 0, (int)numApprox, r); // N_i
    r.scale(1./cd_vars[numApprox]); // r_i = N_i / N
    break;
  default: // r_and_N provided: pass leading numApprox terms of cd_vars
    r = RealVector(Teuchos::View, cd_vars.values(), numApprox);
    break;
  }

  // define approx order using high to low on average evaluation ratios
  bool ordered = ordered_approx_sequence(r, ratioApproxSequence, true);
  if (outputLevel >= DEBUG_OUTPUT) {
    if (ordered)
      Cout << "MFMC: evaluation ratios are well-ordered.\n" << std::endl;
    else
      Cout << "MFMC: evaluation ratio-ordered approximation sequence "
	   << "(high to low):\n" << ratioApproxSequence << std::endl;
  }
  mfmc_estvar_ratios(rho2LH, r, ratioApproxSequence, estvar_ratios);
}


/*
// ***************************************************************************
// Important note: this implementation is being phased out since the estimator
// variance should not take credit for optimal eval_ratios(qoi,approx) that
// are not realized in practice due to sharing of numSamples across QoI.
// As for numerical optimizer cases, avg_eval_ratios is consistent with the
// downstream utilization of eval_ratios under the assumption of a shared
// allocation (neglecting backfill for actual counts).
// ***************************************************************************

void NonDMultifidelitySampling::
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const RealMatrix& eval_ratios,
                   const SizetArray& approx_sequence, RealVector& estvar_ratios)
{
  // Compute ratio of EstVar for single-fidelity MC and MFMC
  // > Estimator Var for   MC =           var_H / N_H
  // > Estimator Var for MFMC = (1 - R^2) var_H / N_H
  // > EstVar ratio = EstVar_MFMC / EstVar_MC = (1 - R^2)

  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  // Peherstorfer paper: ratios derived for N_H = m1* = the optimal # of HF
  // samples, not the actual # (when optimal is hidden by pilot):
  // > Estimator Var for MFMC = var_H (1-rho_LH(am1)^2) p / (N_H^2 cost_H)
  //   where budget p = cost^T eval_ratios N_H,  am1 = most-correlated approx
  //   --> EstVar = var_H (1-rho_LH(am1)^2) cost^T eval_ratios / (N_H cost_H)
  //   --> EstVar ratio = EstVar_MFMC / EstVar_MC
  //                    = (1-rho_LH(am1)^2) cost^T eval_ratios / cost_H
  // For this expression, final MFMC estimator variance should use m1*
  // (ignoring pilot) and not the actual N_H (which includes pilot).  This
  // avoids a bug where MFMC est var doesn't change until m1* emerges from
  // pilot.  We can't take credit for N_H > pilot since r* is applied to m1*,
  // not N_H (see update_hf_targets() -> approx_increment() -> lf_targets).

  // Real inner_prod, cost_H = sequenceCost[numApprox];
  // size_t qoi, approx, num_am1 = numApprox - 1;
  // for (qoi=0; qoi<numFunctions; ++qoi) {
  //   inner_prod = cost_H; // include cost_H * w_H
  //   for (approx=0; approx<numApprox; ++approx)
  //     inner_prod += sequenceCost[approx] * eval_ratios(qoi, approx);
  //   estvar_ratios[qoi] = (1. - rho2_LH(qoi, num_am1)) * inner_prod / cost_H;
  // }
  // if (outputLevel >= NORMAL_OUTPUT) {
  //   for (qoi=0; qoi<numFunctions; ++qoi) {
  //     for (approx=0; approx<numApprox; ++approx)
  // 	Cout << "  QoI " << qoi+1 << " Approx " << approx+1
  // 	   //<< ": cost_ratio = " << cost_H / cost_L
  // 	     << ": rho2_LH = "    <<     rho2_LH(qoi,approx)
  // 	     << " eval_ratio = "  << eval_ratios(qoi,approx) << '\n';
  //     Cout << "QoI " << qoi+1 <<": Peherstorfer variance reduction factor = "
  // 	   << estvar_ratios[qoi] << '\n';
  //   }
  //   Cout << std::endl;
  // }

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq. above.

  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, i, ip1;
  switch (optSubProblemForm) {

  // eval_ratios per qoi,approx with no model re-sequencing
  case ANALYTIC_SOLUTION:
    for (qoi=0; qoi<numFunctions; ++qoi) {
      R_sq = 0.;  r_i = eval_ratios(qoi, 0);
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	r_ip1 = eval_ratios(qoi, ip1); // *** taking credit for QoI tailoring in estvar
	R_sq += (r_i - r_ip1) / (r_i * r_ip1) * rho2_LH(qoi, i);
	r_i = r_ip1;
      }
      R_sq += (r_i - 1.) / r_i * rho2_LH(qoi, numApprox-1);
      estvar_ratios[qoi] = (1. - R_sq);
    }
    break;

  // eval_ratios & approx_sequence based on avg_rho2_LH: remain consistent here
  case REORDERED_ANALYTIC_SOLUTION: {
    RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
    bool ordered = approx_sequence.empty();
    approx = (ordered) ? 0 : approx_sequence[0];
    r_i = eval_ratios(0, approx);  R_sq = 0.;
    for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
      approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
      r_ip1 = eval_ratios(0, approx_ip1);
      // Note: monotonicity in reordered r_i is enforced in compute_allocations()
      // and in linear constraints for ensemble_numerical_solution()
      R_sq += (r_i - r_ip1) / (r_i * r_ip1) * avg_rho2_LH[approx];
      r_i = r_ip1;  approx = approx_ip1;
    }
    R_sq += (r_i - 1.) / r_i * avg_rho2_LH[approx];
    estvar_ratios = (1. - R_sq); // assign scalar to vector components
    break;
  }

  // Note: ANALYTIC_SOLUTION above corresponds to the ordered case below
  default: {
    bool ordered = approx_sequence.empty();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      approx = (ordered) ? 0 : approx_sequence[0];
      R_sq = 0.;  r_i = eval_ratios(qoi, approx);
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
	r_ip1 = eval_ratios(qoi, approx_ip1); // *** taking credit for QoI tailored solution in estvar
	R_sq += (r_i - r_ip1) / (r_i * r_ip1) * rho2_LH(qoi, approx);
	r_i = r_ip1;  approx = approx_ip1;
      }
      R_sq += (r_i - 1.) / r_i * rho2_LH(qoi, approx);
      estvar_ratios[qoi] = (1. - R_sq);
    }
    break;
  }
  }
}
*/


/** Multi-moment map-based, coarse-grained counter version used by
    MFMC following shared_increment() */
void NonDMultifidelitySampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL, // each L with itself
		   IntRealMatrixMap& sum_LH, // each L with H
		   RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter h_it; IntRMMIter lb_it, ll_it, lh_it;
  int lb_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, lf_index, hf_index;
  bool all_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    //if (outputLevel >= DEBUG_OUTPUT) { // sample dump for MATLAB checking
    //  size_t index = 0;
    //  for (approx=0; approx<=numApprox; ++approx)
    // 	  for (qoi=0; qoi<numFunctions; ++qoi, ++index)
    // 	    Cout << fn_vals[index] << ' ';
    //  Cout << '\n';
    //}

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_finite = true;
      for (approx=0; approx<=numApprox; ++approx)
	if (!isfinite(fn_vals[approx * numFunctions + qoi])) // NaN or +/-Inf
	  { all_finite = false; break; }

      if (all_finite) {
	++N_shared[qoi];

	// High accumulations:
	hf_index = numApprox * numFunctions + qoi;
	hf_fn = fn_vals[hf_index];
	// High-High:
	sum_HH[qoi] += hf_fn * hf_fn; // a single vector for ord 1
	// High:
	h_it = sum_H.begin();  h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	hf_prod = hf_fn;       active_ord = 1;
	while (h_ord) {
	  if (h_ord == active_ord) { // support general key sequence
	    h_it->second[qoi] += hf_prod;
	    ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  hf_prod *= hf_fn;  ++active_ord;
	}

	for (approx=0; approx<numApprox; ++approx) {
	  // Low accumulations:
	  lf_index = approx * numFunctions + qoi;
	  lf_fn = fn_vals[lf_index];

	  lb_it = sum_L_baseline.begin();
	  ll_it = sum_LL.begin();  lh_it = sum_LH.begin();
	  lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	  ll_ord = (ll_it == sum_LL.end())         ? 0 : ll_it->first;
	  lh_ord = (lh_it == sum_LH.end())         ? 0 : lh_it->first;
	  lf_prod = lf_fn;  hf_prod = hf_fn;  active_ord = 1;
	  while (lb_ord || ll_ord || lh_ord) {

	    // Low baseline
	    if (lb_ord == active_ord) { // support general key sequence
	      lb_it->second(qoi,approx) += lf_prod;  ++lb_it;
	      lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      ll_it->second(qoi,approx) += lf_prod * lf_prod;  ++ll_it;
	      ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High
	    if (lh_ord == active_ord) { // support general key sequence
	      lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  hf_prod *= hf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** Single moment, coarse-grained counter version used by offline-pilot
    and pilot-projection MFMC following shared_increment() */
void NonDMultifidelitySampling::
accumulate_mf_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
		   RealMatrix& sum_LL, RealMatrix& sum_LH,
		   RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn;  size_t qoi, approx, lf_index, hf_index;
  IntRespMCIter r_it; bool all_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    /*
    if (outputLevel >= DEBUG_OUTPUT) { // sample dump for MATLAB checking
      size_t index = 0;
      for (approx=0; approx<=numApprox; ++approx)
	for (qoi=0; qoi<numFunctions; ++qoi, ++index)
	  Cout << fn_vals[index] << ' ';
      Cout << '\n';
    }
    */

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_finite = true;
      for (approx=0; approx<=numApprox; ++approx)
	if (!isfinite(fn_vals[approx * numFunctions + qoi])) // NaN or +/-Inf
	  { all_finite = false; break; }

      if (all_finite) {
	++N_shared[qoi];
	// High accumulations:
	hf_index = numApprox * numFunctions + qoi;
	hf_fn = fn_vals[hf_index];
	sum_H[qoi]  += hf_fn;         // High
	sum_HH[qoi] += hf_fn * hf_fn; // High-High

	for (approx=0; approx<numApprox; ++approx) {
	  lf_index = approx * numFunctions + qoi;
	  lf_fn = fn_vals[lf_index];
	  // Low accumulations:
	  sum_L_baseline(qoi,approx) += lf_fn; // Low
	  sum_LL(qoi,approx) += lf_fn * lf_fn; // Low-Low
	  // Low-High accumulation:
	  sum_LH(qoi,approx) += lf_fn * hf_fn;
	}
      }
    }
  }
}




/** Multi-moment map-based, fine-grained counter version used by MFMC
    following shared_increment()
void NonDMultifidelitySampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL, // each L with itself
		   IntRealMatrixMap& sum_LH, // each L with H
		   RealVector& sum_HH, Sizet2DArray& num_L_baseline,
		   SizetArray& num_H,  Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter h_it; IntRMMIter lb_it, ll_it, lh_it;
  int lb_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, lf_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    //if (outputLevel >= DEBUG_OUTPUT) { // sample dump for MATLAB checking
    //  size_t index = 0;
    //  for (approx=0; approx<=numApprox; ++approx)
    // 	for (qoi=0; qoi<numFunctions; ++qoi, ++index)
    // 	  Cout << fn_vals[index] << ' ';
    //  Cout << '\n';
    //}

    hf_index = numApprox * numFunctions;
    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = fn_vals[hf_index];
      hf_is_finite = isfinite(hf_fn);
      // High accumulations:
      if (hf_is_finite) { // neither NaN nor +/-Inf
	++num_H[qoi];
	// High-High:
	sum_HH[qoi] += hf_fn * hf_fn; // a single vector for ord 1
	// High:
	h_it = sum_H.begin();  h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	hf_prod = hf_fn;       active_ord = 1;
	while (h_ord) {
	  if (h_ord == active_ord) { // support general key sequence
	    h_it->second[qoi] += hf_prod;
	    ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  hf_prod *= hf_fn;  ++active_ord;
	}
      }

      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_baseline[approx][qoi];
	  if (hf_is_finite) ++num_LH[approx][qoi];

	  lb_it = sum_L_baseline.begin();
	  ll_it = sum_LL.begin();  lh_it = sum_LH.begin();
	  lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	  ll_ord = (ll_it == sum_LL.end())         ? 0 : ll_it->first;
	  lh_ord = (lh_it == sum_LH.end())         ? 0 : lh_it->first;
	  lf_prod = lf_fn;  hf_prod = hf_fn;  active_ord = 1;
	  while (lb_ord || ll_ord || lh_ord) {

	    // Low baseline
	    if (lb_ord == active_ord) { // support general key sequence
	      lb_it->second(qoi,approx) += lf_prod;  ++lb_it;
	      lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      ll_it->second(qoi,approx) += lf_prod * lf_prod;  ++ll_it;
	      ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High
	    if (lh_ord == active_ord) {
	      if (hf_is_finite)
		lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	    if (hf_is_finite)  hf_prod *= hf_fn;
	  }
	}
      }
    }
  }
}
*/


/** Single moment, fine-grained counter version used by offline-pilot
    and pilot-projection MFMC following shared_increment()
void NonDMultifidelitySampling::
accumulate_mf_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
		   RealMatrix& sum_LL, RealMatrix& sum_LH,
		   RealVector& sum_HH, Sizet2DArray& num_L_baseline,
		   SizetArray& num_H,  Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn;  size_t qoi, approx, lf_index, hf_index;
  IntRespMCIter r_it; bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    //if (outputLevel >= DEBUG_OUTPUT) { // sample dump for MATLAB checking
    //  size_t index = 0;
    //  for (approx=0; approx<=numApprox; ++approx)
    // 	for (qoi=0; qoi<numFunctions; ++qoi, ++index)
    // 	  Cout << fn_vals[index] << ' ';
    //  Cout << '\n';
    //}

    hf_index = numApprox * numFunctions;
    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = fn_vals[hf_index];
      hf_is_finite = isfinite(hf_fn);
      // High accumulations:
      if (hf_is_finite) { // neither NaN nor +/-Inf
	++num_H[qoi];
	sum_H[qoi]  += hf_fn;         // High
	sum_HH[qoi] += hf_fn * hf_fn; // High-High
      }

      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_baseline[approx][qoi];
	  sum_L_baseline(qoi,approx) += lf_fn; // Low
	  sum_LL(qoi,approx) += lf_fn * lf_fn; // Low-Low
	  if (hf_is_finite) {
	    ++num_LH[approx][qoi];
	    sum_LH(qoi,approx) += lf_fn * hf_fn;// Low-High
	  }
	}
      }
    }
  }
}
*/


/** This version used by MFMC following approx_increment() */
void NonDMultifidelitySampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, Sizet2DArray& num_L_shared,
		   Sizet2DArray& num_L_refined, const IntResponseMap& resp_map,
		   const SizetArray& approx_sequence,
		   size_t sequence_start, size_t sequence_end)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  Real fn_val, prod;
  int ls_ord, lr_ord, active_ord;
  size_t s, qoi, fn_index, approx, shared_end = sequence_end - 1;
  bool ordered = approx_sequence.empty();
  IntRespMCIter r_it; IntRMMIter ls_it, lr_it;

  for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    // accumulate for leading set of models (omit trailing truth),
    // but note that resp and asv are full aggregated length
    for (s=sequence_start; s<sequence_end; ++s) {

      approx = (ordered) ? s : approx_sequence[s];
      SizetArray& num_L_sh_a  = num_L_shared[approx];
      SizetArray& num_L_ref_a = num_L_refined[approx];
      fn_index = approx * numFunctions;

      for (qoi=0; qoi<numFunctions; ++qoi, ++fn_index) {
	//if (asv[fn_index] & 1) {
	  prod = fn_val = fn_vals[fn_index];
	  if (isfinite(fn_val)) { // neither NaN nor +/-Inf
	    ++num_L_ref_a[qoi];  active_ord = 1;
	    lr_it = sum_L_refined.begin();
	    lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	    // for pyramid sampling, shared range is one less than refined,
	    // i.e. sum_L_{shared,refined} are both accumulated for all s except
	    // sequence_end-1, which accumulates only sum_L_refined.  See z^1
	    // sets in Fig. 2b of ACV paper.
	    if (s < shared_end) {
	      ++num_L_sh_a[qoi];
	      ls_it = sum_L_shared.begin();
	      ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	    }
	    else
	      ls_ord = 0;
	    while (ls_ord || lr_ord) {
	      // Low shared
	      if (ls_ord == active_ord) { // support general key sequence
		ls_it->second(qoi,approx) += prod;  ++ls_it;
		ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	      }
	      // Low refined
	      if (lr_ord == active_ord) { // support general key sequence
		lr_it->second(qoi,approx) += prod;  ++lr_it;
		lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	      }
	      prod *= fn_val;  ++active_ord;
	    }
	  }
	//}
      }
    }
  }
}


/* Deactivated (see notes in compute_correlation())
void NonDMultifidelitySampling::
compute_LH_correlation(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		       const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		       const RealVector& sum_HH, const Sizet2DArray& N_L_shared,
		       const SizetArray& N_H,    const Sizet2DArray& N_LH,
		       RealMatrix& var_L,        RealVector& var_H,
		       RealMatrix& rho2_LH)
{
  if (var_L.empty())     var_L.shapeUninitialized(numFunctions, numApprox);
  if (var_H.empty())     var_H.sizeUninitialized(numFunctions);
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LL_a =       sum_LL[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    const SizetArray&    N_L_a =   N_L_shared[approx];
    const SizetArray&   N_LH_a =         N_LH[approx];
    Real*              var_L_a =        var_L[approx];
    Real*            rho2_LH_a =      rho2_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_correlation(sum_L_shared_a[qoi], sum_H[qoi], sum_LL_a[qoi],
			  sum_LH_a[qoi], sum_HH[qoi], N_L_a[qoi], N_H[qoi],
			  N_LH_a[qoi], var_L_a[qoi], var_H[qoi],
			  rho2_LH_a[qoi]);
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "rho2_LH in compute_LH_correlation():\n" << rho2_LH << std::endl;
}
*/


void NonDMultifidelitySampling::
compute_LH_correlation(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		       const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		       const RealVector& sum_HH, const SizetArray& N_shared,
		       RealMatrix& var_L, RealVector& var_H,RealMatrix& rho2_LH)
{
  if (var_L.empty())     var_L.shapeUninitialized(numFunctions, numApprox);
  if (var_H.empty())     var_H.sizeUninitialized(numFunctions);
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LL_a =       sum_LL[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    Real*              var_L_a =        var_L[approx];
    Real*            rho2_LH_a =      rho2_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_correlation(sum_L_shared_a[qoi], sum_H[qoi], sum_LL_a[qoi],
			  sum_LH_a[qoi], sum_HH[qoi], N_shared[qoi],
			  var_L_a[qoi], var_H[qoi], rho2_LH_a[qoi]);
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "rho2_LH in compute_LH_correlation():\n" << rho2_LH << std::endl;
}


void NonDMultifidelitySampling::
compute_allocations(const RealMatrix& rho2_LH, const RealVector& var_H,
		    const SizetArray& N_H,     const RealVector& cost,
		    MFSolutionData& soln)
{
  // -------------------------------------------------------------
  // Based on rho2_LH sequencing, determine best solution approach
  // -------------------------------------------------------------
  // compute a model sequence sorted by Low-High correlation
  // > rho2, N_L, N_H, {eval,estvar}_ratios, etc. are all ordered based on the
  //   user-provided model list ordering
  // > we employ approx_sequence to pair approximations using a different order
  //   for computing rho2_diff, cost --> eval_ratios --> estvar_ratios, but the
  //   results are indexed by the original approx ordering
  // > control variate compute/apply are per approx, so sequencing not required
  // > approx_increment requires model sequence to define the sample pyramid

  // Bomarito & Warner (NASA LaRC): stay within a numerical ACV-like approach
  // by defining F for this graph (hierarchical MFMC rather than peer ACV).
  // Encounter singularity when models are not sequenced for this graph,
  // which is addressed numerically by introducing a (diagonal) nugget.

  // incoming approx_sequence is defined w/i mfmc_reordered_analytic_solution()
  // based on averaged rho --> we do not use it for logic below
  bool ordered_rho = true, budget_constr = (maxFunctionEvals != SZ_MAX);
  switch (numericalSolveMode) {
  case NUMERICAL_OVERRIDE: // specification option
    optSubProblemForm = (budget_constr) ? N_MODEL_LINEAR_CONSTRAINT :
      N_MODEL_LINEAR_OBJECTIVE;
    break;
  case NUMERICAL_FALLBACK: // default
    // Theory for JCP EstVar expression (Appendix B) used in minimization
    // assumes ordered eval ratios [sqrt(rho2_diff) no longer applies to this
    // case, no we can now focus or r_i ordering rather than rho ordering].
    // Ordering in avg_eval_ratios is then used for pyramid sampling downstream
    // > ascending order in rho required all QoI for all Approx
    ordered_rho = ordered_approx_sequence(rho2_LH);
    if (ordered_rho) {
      Cout << "MFMC: model sequence provided is ordered in Low-High "
	   << "correlation for all QoI.\n      No fallback: computing "
	   << "standard analytic solution.\n" << std::endl;
      optSubProblemForm = ANALYTIC_SOLUTION;
    }
    else {
      optSubProblemForm = (budget_constr) ? N_MODEL_LINEAR_CONSTRAINT :
	N_MODEL_LINEAR_OBJECTIVE;
      Cout << "MFMC: model sequence provided is out of order with respect to "
	   << "Low-High\n      correlation for at least one QoI.  Fallback: "
	   << "switching to numerical solution.\n";
    }
    break;
  case REORDERED_FALLBACK: // not currently in XML spec (numerical FB preferred)
    ordered_rho = ordered_approx_sequence(rho2_LH);
    if (ordered_rho) {
      Cout << "MFMC: model sequence provided is ordered in Low-High "
	   << "correlation for all QoI.\n      No fallback: computing "
	   << "standard analytic solution.\n" << std::endl;
      optSubProblemForm = ANALYTIC_SOLUTION;
    }
    else {
      optSubProblemForm = REORDERED_ANALYTIC_SOLUTION;
      Cout << "MFMC: model sequence provided is out of order with respect to "
	   << "Low-High\n      correlation for at least one QoI.  Fallback: "
	   << "switching to reordered analytic solution.\n";
    }
    break;
  }

  // For stand-alone analytic MFMC (not an analytic initial guess), enforce
  // monotonicity in r_i for a valid evaluation of estvar that is consistent
  // with the order used for computing avg_eval_ratios.  Reordering sample set
  // evaluations downstream is Ok for purposes of nested sample reuse, but
  // reordering based on r_i when evaluating estvar (as can be done in the
  // numerical case) would be inconsistent with the sequence used for the 
  // analytic r_i computation --> preserve rho ordering and enforce monotonic
  // r_i for downstream eval of estvar, moment roll-up, et al.
  bool lower_bounded_r = true, monotonic_r = true;
  RealVector avg_eval_ratios;
  switch (optSubProblemForm) {
  case ANALYTIC_SOLUTION:
    corrApproxSequence.clear();  ratioApproxSequence.clear();
    mfmc_analytic_solution(approxSet, rho2_LH, cost, avg_eval_ratios,
			   lower_bounded_r, monotonic_r);
    process_analytic_allocations(rho2_LH, var_H, N_H, cost,
				 avg_eval_ratios, soln);
    //update_model_groups();  update_model_group_costs();
    break;

  case REORDERED_ANALYTIC_SOLUTION: // inactive (see above)
    ratioApproxSequence.clear();
    mfmc_reordered_analytic_solution(approxSet, rho2_LH, cost,
				     corrApproxSequence, avg_eval_ratios,
				     lower_bounded_r, monotonic_r);
    process_analytic_allocations(rho2_LH, var_H, N_H, cost,
				 avg_eval_ratios, soln);
    //update_model_groups();  update_model_group_costs();
    break;

  default: // any of several numerical optimization formulations
    mfmc_numerical_solution(rho2_LH, cost, soln);
    break;
  }
}


void NonDMultifidelitySampling::
mfmc_numerical_solution(const RealMatrix& rho2_LH, const RealVector& cost,
			MFSolutionData& soln)
{
  bool budget_constrained = (maxFunctionEvals != SZ_MAX), budget_exhausted
    = (budget_constrained && equivHFEvals >= (Real)maxFunctionEvals),
    no_solve = (budget_exhausted || convergenceTol >= 1.); // bypass opt solve

  if (mlmfIter == 0) {

    size_t hf_form_index, hf_lev_index;
    hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
    RealVector avg_eval_ratios(numApprox, false);

    if (no_solve) { // only 1 feasible pt, no need for solve
      avg_eval_ratios = 1.;
      soln.anchored_solution_ratios(avg_eval_ratios, avg_N_H);
      no_solve_variances(soln);  numSamples = 0;  return;
    }

    // Compute r* initial guess from analytic MFMC
    // > leave r_i at analytic values unless mfmc_estvar_ratios() is needed
    //   for accuracy constraint
    bool lower_bounded_r = true, monotonic_r = (budget_constrained) ?
      false : true;
    if (ordered_approx_sequence(rho2_LH)) { // can happen w/ NUMERICAL_OVERRIDE
      corrApproxSequence.clear();
      mfmc_analytic_solution(approxSet, rho2_LH, cost, avg_eval_ratios,
			     lower_bounded_r, monotonic_r);
    }
    else
      mfmc_reordered_analytic_solution(approxSet, rho2_LH, cost,
				       corrApproxSequence, avg_eval_ratios,
				       lower_bounded_r, monotonic_r);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Initial guess from analytic MFMC (average eval ratios):\n"
	   << avg_eval_ratios << std::endl;

    Real hf_target;
    if (budget_constrained) // for numerical, re-scale for over-estimated pilot
      scale_to_target(avg_N_H, cost, avg_eval_ratios, hf_target,
		      (Real)maxFunctionEvals);
    else { // accuracy-constrained
      // Computes estvar_ratios* from r*,rho2.  Next, m1* from estvar_ratios*;
      // then these estvar_ratios get replaced for actual profile
      RealVector estvar_ratios, estvar;  Real metric;  size_t metric_index;
      mfmc_estvar_ratios(rho2_LH, avg_eval_ratios, corrApproxSequence,
			 estvar_ratios);
      estvar_ratios_to_estvar(estvar_ratios, varH, N_H_actual, estvar);
      MFSolutionData::update_estimator_variance_metric(estVarMetricType,
	estVarMetricNormOrder, estvar_ratios, estvar, metric, metric_index);
      hf_target = update_hf_target(estvar_ratios, metric_index, varH,
				   estVarIter0);
    }
    // push updates to MFSolutionData
    soln.anchored_solution_ratios(avg_eval_ratios, hf_target);
  }
  else if (no_solve) // subsequent iterations
    { numSamples = 0;  return; } // leave soln at previous values

  // define covLH and covLL from rho2LH, var_L, varH
  //correlation_sq_to_covariance(rho2_LH, var_L, varH, covLH);
  //matrix_to_diagonal_array(var_L, covLL);

  // This definition of modelGroupCost allows a unified treatment in
  // NonDNonHierarchSampling::derived_finite_solution_bounds(), which aligns
  // solution vars N[i] with modelGroupCost[i] --> use ordered groups here.
  // > Downstream, modelGroup{s,Cost} are used for group_increments() -->
  //   groups are redefined for an r_i ordering.
  update_model_groups();  update_model_group_costs();

  // Base class implementation of numerical solve (shared with ACV,GenACV):
  ensemble_numerical_solution(soln);
  process_model_allocations(soln, numSamples);
  //if (outputLevel >= NORMAL_OUTPUT)
  //  print_model_allocations(Cout, soln, approxSet);
}


void NonDMultifidelitySampling::
process_analytic_allocations(const RealMatrix& rho2_LH, const RealVector& var_H,
			     const SizetArray& N_H,     const RealVector& cost,
			     RealVector& avg_eval_ratios, MFSolutionData& soln)
{
  // Called from compute_allocations() for analytic cases, due to ordering:
  // use of soln to communicate state and reqmts for updating anchored soln

  // Print base analytic solution prior to modifications:
  // > only print when primary MFMC solve, not for initial guess elsewhere
  print_analytic_solution(rho2_LH, avg_eval_ratios);

  // Form an initial estimate of N^* from r_i^* based on budget/accuracy
  // > for accuracy-constrained, estvar_ratios is also an initial estimate
  Real hf_target, budget = (Real)maxFunctionEvals;
  if (maxFunctionEvals == SZ_MAX) {
    RealVector estvar_ratios, estvar;  Real metric;  size_t metric_index;
    mfmc_estvar_ratios(rho2_LH, avg_eval_ratios, corrApproxSequence,
		       estvar_ratios);
    estvar_ratios_to_estvar(estvar_ratios, var_H, N_H, estvar);
    MFSolutionData::update_estimator_variance_metric(estVarMetricType,
      estVarMetricNormOrder, estvar_ratios, estvar, metric, metric_index);
    hf_target = update_hf_target(estvar_ratios, metric_index, varH,
				 estVarIter0);
  }
  else
    hf_target = allocate_budget(avg_eval_ratios, cost, budget);

  // Now we update these initial estimates in view of the pilot: analytic cases
  // use the intersection of the analytic profile with the pilot sample,
  // "emerging" from the pilot:
  // > we do not use scale_to_target() to alter r*, rather preserving the
  //   analytic sample profile for N_i allocations that exceed the pilot.
  // > To be accurate in estvar, we must take credit for the pilot and any
  //   other modifications to analytic soln
  emerge_from_pilot(N_H, avg_eval_ratios, hf_target);

  // Now we lock in the modified analytic solution
  // > estvar/estvar_ratios are updated downstream once iteration is complete
  soln.anchored_solution_ratios(avg_eval_ratios, hf_target);
}


void NonDMultifidelitySampling::
mfmc_estimator_variance(const RealMatrix& rho2_LH, const RealVector& var_H,
			const SizetArray& N_H, MFSolutionData& soln)
{
  // These operations are deferred until convergence of any solution iteration
  // > Accuracy-constrained needs estvar_ratios for computing hf_target in
  //   process_allocations()

  switch (optSubProblemForm) {

  case ANALYTIC_SOLUTION:  case REORDERED_ANALYTIC_SOLUTION: {
    // estvar/estvar_ratios not required during online iteration
    //bool converged = (pilotMgmtMode != ONLINE_PILOT || !numSamples ||
    //		      mlmfIter > maxIterations);
    //if (converged) {
    mfmc_estvar_ratios(rho2_LH, soln.solution_ratios(),
		       corrApproxSequence, soln);
    estvar_ratios_to_estvar(var_H, N_H, soln);
    // Note: we are not updating soln.anchored_solution_ratios()
    //       (not necessary for things like one_sided_delta() to follow)
    //}
    if (outputLevel >= NORMAL_OUTPUT) {
      const RealVector& estvar_ratios = soln.estimator_variance_ratios();
      for (size_t qoi=0; qoi<numFunctions; ++qoi)
	Cout << "QoI " << qoi+1 << ": variance reduction factor = "
	     << estvar_ratios[qoi] << '\n';
      Cout << std::endl;
    }
    break;
  }

  // Numerical MFMC: mfmc_numerical_solution() uses process_model_allocations()
  // to store optimized estvar/estvar_ratios in soln.  In this case, there is
  // no emerge_from_pilot() modification so estvar remains consistent,
  // including pilot projections that target this optimized state.
  }

  // update metric for solution comparisons (e.g. model tuning)
  soln.update_estimator_variance_metric(estVarMetricType,estVarMetricNormOrder);
}


void NonDMultifidelitySampling::update_model_groups()
{
  // Note: model selection case handled by NonDDGenACV, so here numGroups
  // is the total number of model instances
  if (modelGroups.size() != numGroups)
    modelGroups.resize(numGroups);
  for (size_t g=0; g<numGroups; ++g)
    mfmc_model_group(g, modelGroups[g]);
}


void NonDMultifidelitySampling::
update_model_groups(const SizetArray& approx_sequence)
{
  if (approx_sequence.empty())
    update_model_groups();

  // Note: model selection case handled by NonDDGenACV, so here numGroups
  // is the total number of model instances
  if (modelGroups.size() != numGroups)
    modelGroups.resize(numGroups);
  for (size_t g=0; g<numGroups; ++g)
    mfmc_model_group(g, approx_sequence, modelGroups[g]);
}


void NonDMultifidelitySampling::
emerge_from_pilot(const SizetArray& N_H, RealVector& avg_eval_ratios,
		  Real& avg_hf_target, Real offline_N_lwr)
{
  // Analytic cases: the profile emerges from the pilot sample.
  // > if N* < N_H --> N = N_H
  // > if r*_i N* <= N_H (shared sample), take credit for pilot
  //   --> increase N_i to N_H with     r_i N_i = (1+nudge) N_H
  // > if r*_i N* >  N_H, keep N_i with r_i N_i = (scaled r_i) N_H
  Real avg_N_H = average(N_H);
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    if (avg_N_H < offline_N_lwr)
      avg_N_H = offline_N_lwr;
  }
  if (avg_N_H > avg_hf_target) { // replace N* and rescale r*
    Real nudge = RATIO_NUDGE, scaling = avg_hf_target / avg_N_H, N_i;
    bool ordered = ratioApproxSequence.empty();  unsigned short approx;
    for (int i=numApprox-1; i>=0; --i) {
      approx = (ordered) ? i : ratioApproxSequence[i];
      Real& r_i = avg_eval_ratios[approx];  N_i = r_i * avg_hf_target;
      // use RATIO_NUDGE, noting that best for peer DAG
      if (N_i <= avg_N_H) // new N_i ~= N_H (new r_i ~= 1)
	{ r_i = 1. + nudge; nudge += RATIO_NUDGE; }
      else // same N_i (new r_i for new N_H)
	r_i *= scaling;
    }
    avg_hf_target = avg_N_H; // replace N* with N_H
  }
}


void NonDMultifidelitySampling::
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const RealVector& avg_eval_ratios,
		   SizetArray& approx_sequence, RealVector& estvar_ratios)
{
  if (estvar_ratios.length() != numFunctions)
    estvar_ratios.sizeUninitialized(numFunctions);
  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, ip1;

  // Analytic cases: sequence fixed upstream based on rho2LH; r_i monotonicity
  //   enforced after analytic r_i computations for rho2LH sequence.
  // Numerical cases: update sequence on every call in NonDMultifidelitySampling
  //   ::estimator_variance_ratios().  Previously, was defined once and retained
  //   via linear constraints on opt solve, but is now reordered on the fly
  //   based on current optimizer design vars to retain validity of estvar.
  bool ordered = approx_sequence.empty();

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq.
  // > Note that R^2 can blow up (resulting in negative estvar_ratios) when
  //   r_i < 1 so we now prevent this in ordered/reordered analytic cases
  //   (as for numerical cases), even though it might only be an initial
  //   guess for something else.

  switch (optSubProblemForm) {

  // eval_ratios based on avg_rho2_LH: remain consistent here
  case REORDERED_ANALYTIC_SOLUTION: {
    RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
    approx = (ordered) ? 0 : approx_sequence[0];
    r_i = avg_eval_ratios[approx];  R_sq = 0.;
    for (ip1=1; ip1<numApprox; ++ip1) {
      approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
      r_ip1 = avg_eval_ratios[approx_ip1];
      R_sq += (r_i - r_ip1) / (r_i * r_ip1) * avg_rho2_LH[approx]; // see below
      r_i = r_ip1;  approx = approx_ip1;
    }
    R_sq += (r_i - 1.) / r_i * avg_rho2_LH[approx];
    estvar_ratios = (1. - R_sq); // assign scalar to vector components
    break;
  }

  // Call stack for MFMC numerical solution:
  // > NonDNonHierarchSampling::log_estvar_metric(RealVector&)
  // > NonDNonHierarchSampling::estimator_variance_metric(RealVector&)
  // > NonDNonHierarchSampling::estimator_variances(RealVector&)
  // > NonDMultifidelitySampling::estimator_variance_ratios() [virtual]
  // > This function (vector of avg_eval_ratios from opt design variables)
  // Note: ANALYTIC_SOLUTION covered by ordered case below
  default: {
    for (qoi=0; qoi<numFunctions; ++qoi) {
      approx = (ordered) ? 0 : approx_sequence[0];
      R_sq = 0.;  r_i = avg_eval_ratios[approx];
      for (ip1=1; ip1<numApprox; ++ip1) {
	approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
	r_ip1 = avg_eval_ratios[approx_ip1];
	// While eval ratio misordering won't induce numerical exceptions,
	// the derivation of this equation assumes ordered r_i:
	// > either retain a fixed ordering through linear constraints, or
	// > re-order on the fly when the optimization omits these guard rails
	R_sq += (r_i - r_ip1) / (r_i * r_ip1) * rho2_LH(qoi, approx);
	r_i = r_ip1;  approx = approx_ip1;
      }
      R_sq += (r_i - 1.) / r_i * rho2_LH(qoi, approx);
      estvar_ratios[qoi] = (1. - R_sq);
    }
    break;
  }
  }
}

} // namespace Dakota
