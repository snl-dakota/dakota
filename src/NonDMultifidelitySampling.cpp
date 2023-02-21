/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultifidelitySampling
//- Description: Implementation code for NonDMultifidelitySampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

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
NonDMultifidelitySampling(ProblemDescDB& problem_db, Model& model):
  NonDNonHierarchSampling(problem_db, model),
  numericalSolveMode(problem_db.get_ushort("method.nond.numerical_solve_mode"))
{
  mlmfSubMethod = SUBMETHOD_MFMC; // if needed for numerical solves
}


NonDMultifidelitySampling::~NonDMultifidelitySampling()
{ }


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDMultifidelitySampling::core_run()
{
  //sequence_models(); // enforce correlation condition (*** AFTER PILOT ***)

  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

  switch (pilotMgmtMode) {
  case  ONLINE_PILOT: // iterated MFMC (default)
    multifidelity_mc();                  break;
  case OFFLINE_PILOT: // computes perf for offline pilot/Oracle correlation
    multifidelity_mc_offline_pilot();    break;
  case PILOT_PROJECTION: // for algorithm assessment/selection
    multifidelity_mc_pilot_projection(); break;
  }

  // Notes on ACV + ensemble model classes:
  // > HierarchSurrModel is limiting (see MFMC) such that we may want to
  //   subsume it with NonHierarchSurrModel --> consider generalizations
  //   that can be deployed across the algorithm set:
  //   >> enhanced parallel usage --> avoid sync points in ACV clients
  //   >> enable sampling over shared + distinct model variable sets
}


/** This is the standard MFMC version that integrates the pilot alongside
    the sample adaptation and iterates to determine N_H. */
void NonDMultifidelitySampling::multifidelity_mc()
{
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baseline, sum_LL, sum_LH;
  RealVector sum_HH, hf_targets;  RealMatrix var_L, eval_ratios;
  initialize_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  while (numSamples && mlmfIter <= maxIterations) {

    // ----------------------------------------------------
    // Evaluate shared increment and increment accumulators
    // ----------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH,
		       N_H_actual);
    N_H_alloc += (backfillFailures && mlmfIter) ?
      one_sided_delta(N_H_alloc, average(hf_targets)) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the correlation updates.
    if (onlineCost && mlmfIter == 0) recover_online_cost(sequenceCost);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,
			      equivHFEvals);

    // -------------------------------------------
    // Compute correlations and evaluation ratios:
    // -------------------------------------------
    // First, compute the LF/HF evaluation ratio using shared samples,
    // averaged over QoI.  This includes updating varH and rho2LH.
    compute_LH_correlation(sum_L_baseline[1], sum_H[1], sum_LL[1], sum_LH[1],
			   sum_HH, N_H_actual, var_L, varH, rho2LH);
    // estVarIter0 only uses HF pilot since CV terms (sum_L_shared / N_shared
    // - sum_L_refined / N_refined) cancel out prior to sample refinement.
    // (This differs from MLMC EstVar^0 which uses pilot for all levels.)
    if (mlmfIter == 0) {
      compute_mc_estimator_variance(varH, N_H_actual, estVarIter0);
      numHIter0 = N_H_actual;
    }
    // compute r* from rho2 and cost, either analytically or numerically
    mfmc_eval_ratios(var_L, rho2LH, sequenceCost, approxSequence, eval_ratios,
		     hf_targets);

    // -----------------------------------
    // Compute estimator variance metrics:
    // -----------------------------------
    // Compute the ratio of MC and MFMC estimator variance, which incorporates
    // anticipated variance reduction from upcoming application of eval_ratios.
    // > Note: this could be redundant for tol-based targets with m1* > pilot
    mfmc_estimator_variance(rho2LH, varH, N_H_actual, hf_targets,approxSequence,
			    eval_ratios, estVarRatios, mfmcSolnData.avgEstVar);

    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    // N_H_actual is final --> finalize with LF increments + post-processing
    approx_increments(sum_L_baseline, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, approxSequence, eval_ratios, hf_targets);
  else // for consistency with pilot projection
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(hf_targets, eval_ratios, N_H_actual, N_H_alloc,
				/*deltaNActualLF,*/ deltaEquivHF);
}


/** This MFMC version treats the pilot sample as a separate offline process. */
void NonDMultifidelitySampling::multifidelity_mc_offline_pilot()
{
  RealVector sum_H_pilot(numFunctions), sum_HH_pilot(numFunctions), hf_targets;
  RealMatrix sum_L_pilot(numFunctions, numApprox),
    sum_LL_pilot(numFunctions, numApprox),
    sum_LH_pilot(numFunctions, numApprox), var_L, eval_ratios;
  SizetArray N_shared_pilot(numFunctions, 0);
  //Sizet2DArray N_L_pilot, N_LH_pilot;
  //initialize_counts(N_L_pilot, N_H_pilot, N_LH_pilot);
  // -------------------------------------------------------------------------
  // Compute final var{L,H},rho2LH from (oracle) pilot treated as offline cost
  // -------------------------------------------------------------------------
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_mf_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		     sum_HH_pilot, N_shared_pilot);
  if (onlineCost) recover_online_cost(sequenceCost);
  //increment_equivalent_cost(...); // excluded
  compute_LH_correlation(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
			 sum_HH_pilot, N_shared_pilot, var_L, varH, rho2LH);

  // ---------------------------------
  // Compute online sample increments:
  // ---------------------------------
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  // compute r* from rho2 and cost and update the HF targets
  mfmc_eval_ratios(var_L, rho2LH, sequenceCost, approxSequence, eval_ratios,
		   hf_targets);
  ++mlmfIter;

  // Don't replace pilot-based varH (retain "oracle" rho2LH, varH) since this
  // introduces noise in the final MC/MFMC estimator variances.  It does
  // however result in mixing offline varH with online N_H for estVarIter0.
  //compute_variance(sum_H[1], sum_HH, N_H_actual, varH); // online varH
  // With changes to print_results(), estVarIter0 no longer used for this mode.
  //compute_mc_estimator_variance(varH, N_H_actual, estVarIter0);
  //numHIter0 = N_H_actual;

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baseline, sum_LL, sum_LH;
    RealVector sum_HH;     //Sizet2DArray N_L_baseline, N_LH;
    initialize_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH);
    // perform the shared increment for the online sample profile;
    // at least 2 samples reqd for variance (initial N_H_actual = 0)
    numSamples = std::max(one_sided_delta(N_H_actual, hf_targets, 1),(size_t)2);
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH,
		       N_H_actual);
    N_H_alloc += numSamples;
    increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox+1,
			      equivHFEvals);
    // compute the estimator performance metrics
    mfmc_estimator_variance(rho2LH, varH, N_H_actual, hf_targets,approxSequence,
			    eval_ratios, estVarRatios, mfmcSolnData.avgEstVar);
    // finalize with LF increments and post-processing
    approx_increments(sum_L_baseline, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, approxSequence, eval_ratios, hf_targets);
  }
  else {
    // project online profile including both shared samples and LF increment
    update_projected_samples(hf_targets, eval_ratios, N_H_actual, N_H_alloc,
			     deltaNActualHF, /*deltaNActualLF,*/ deltaEquivHF);
    // include deltaNActualHF in estimator performance
    SizetArray N_H_actual_proj = N_H_actual;
    increment_samples(N_H_actual_proj, deltaNActualHF);
    mfmc_estimator_variance(rho2LH, varH, N_H_actual_proj, hf_targets,
			    approxSequence, eval_ratios, estVarRatios,
			    mfmcSolnData.avgEstVar);
  }
}


/** This MFMC version is for algorithm selection; it estimates the
    variance reduction from pilot-only sampling. */
void NonDMultifidelitySampling::multifidelity_mc_pilot_projection()
{
  RealVector sum_H(numFunctions), sum_HH(numFunctions), hf_targets;
  RealMatrix var_L, eval_ratios, sum_L_baseline(numFunctions, numApprox),
    sum_LL(numFunctions, numApprox), sum_LH(numFunctions, numApprox);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  // ----------------------------------------------------
  // Evaluate shared increment and increment accumulators
  // ----------------------------------------------------
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_mf_sums(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual);
  N_H_alloc += numSamples;
  if (onlineCost) recover_online_cost(sequenceCost);
  increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox+1,
			    equivHFEvals);

  // -------------------------------------------
  // Compute correlations and evaluation ratios:
  // -------------------------------------------
  // First, compute the LF/HF evaluation ratio using shared samples,
  // averaged over QoI.  This includes updating varH and rho2LH.
  compute_LH_correlation(sum_L_baseline, sum_H, sum_LL, sum_LH, sum_HH,
			 N_H_actual, var_L, varH, rho2LH);
  // estVarIter0 only uses HF pilot since CV terms (sum_L_shared / N_shared
  // - sum_L_refined / N_refined) cancel out prior to sample refinement.
  // (This differs from MLMC EstVar^0 which uses pilot for all levels.)
  // > Note: numHIter0 may differ from pilotSamples[numApprox] for sim faults
  compute_mc_estimator_variance(varH, N_H_actual, estVarIter0);
  numHIter0 = N_H_actual;
  // compute r* from rho2 and cost
  mfmc_eval_ratios(var_L, rho2LH, sequenceCost, approxSequence, eval_ratios,
		   hf_targets);
  ++mlmfIter;

  // ----------------------
  // Compute EstVar ratios:
  // ----------------------
  // No LF increments or final moments for pilot projection
  update_projected_samples(hf_targets, eval_ratios, N_H_actual, N_H_alloc,
			   deltaNActualHF, /*deltaNActualLF,*/ deltaEquivHF);
  // Compute the ratio of MC and MFMC mean squared errors, which incorporates
  // anticipated variance reduction from upcoming application of eval_ratios.
  // > Note: this could be redundant for tol-based targets with m1* > pilot
  SizetArray N_H_actual_proj = N_H_actual;
  increment_samples(N_H_actual_proj, deltaNActualHF);
  mfmc_estimator_variance(rho2LH, varH, N_H_actual_proj, hf_targets,
			  approxSequence, eval_ratios, estVarRatios,
			  mfmcSolnData.avgEstVar);
}


void NonDMultifidelitySampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  // linear inequality constraints on sample counts:
  // N_i increases w/ decreasing fidelity

  bool ordered = approxSequence.empty();
  size_t i, num_am1 = numApprox - 1, approx_ip1,
    approx = (ordered) ? 0 : approxSequence[0],
    lin_ineq_offset = ( optSubProblemForm == N_VECTOR_LINEAR_CONSTRAINT  ||
			optSubProblemForm == R_ONLY_LINEAR_CONSTRAINT ) ? 1 : 0;
  for (i=0; i<num_am1; ++i) { // N_im1 >= N_i
    approx_ip1 = (ordered) ? i+1 : approxSequence[i+1];
    lin_ineq_coeffs(i+lin_ineq_offset, approx)     = -1.;
    lin_ineq_coeffs(i+lin_ineq_offset, approx_ip1) =  1.; // *** TO DO ***: check this --> would RATIO_NUDGE be helpful?
    approx = approx_ip1;
  }
  // N_im1 > N
  lin_ineq_coeffs(num_am1+lin_ineq_offset,   num_am1) = -1.;
  lin_ineq_coeffs(num_am1+lin_ineq_offset, numApprox) =  1. + RATIO_NUDGE;
}


void NonDMultifidelitySampling::
update_hf_targets(const RealMatrix& eval_ratios, const RealVector& cost,
		  RealVector& hf_targets)
{
  // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L 
  // > could consider under-relaxing the budget allocation to enable
  //   additional N_H increments + associated updates to shared samples
  //   for improving rho2LH et al.

  if (hf_targets.empty()) hf_targets.sizeUninitialized(numFunctions);
  // Scale this profile based on specified budget (maxFunctionEvals) if needed
  // using N_H = maxFunctionEvals / cost^T eval_ratios
  // > Pilot case iter = 0: can only scale back after shared_increment().
  //   Optimal profile can be hidden by one_sided_delta() with pilot --> optimal
  //   shape emerges from initialization cost as for ML cases controlled by
  //   convTol (allow budget overshoot due to overlap of optimal with pilot,
  //   rather than strictly allocating remaining budget)
  size_t qoi, approx;
  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  switch (optSubProblemForm) {
  // cases which average eval_ratios over QoI: can flatten loops
  case REORDERED_ANALYTIC_SOLUTION:
    inner_prod = cost_H; // raw cost (un-normalized)
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * eval_ratios(0, approx);
    hf_targets = budget / inner_prod * cost_H; // normalized to equivHF
    break;
  default:
    for (qoi=0; qoi<numFunctions; ++qoi) {
      inner_prod = cost_H; // raw cost (un-normalized)
      for (approx=0; approx<numApprox; ++approx)
	inner_prod += cost[approx] * eval_ratios(qoi, approx);
      hf_targets[qoi] = budget / inner_prod * cost_H; // normalized to equivHF
    }
    break;
  }

  /* Alternate Scalar estimation (as in ACV)
  RealVector avg_eval_ratios; // *** change incoming API to avoid repeated avg
  average(eval_ratios, 0, avg_eval_ratios);// avg over qoi for each approx
  inner_prod = cost_H; // raw cost (un-normalized)
  for (approx=0; approx<numApprox; ++approx)
    inner_prod += cost[approx] * avg_eval_ratios[approx];
  hf_target = budget / inner_prod * cost_H; // normalized to equivHF
  */

  Cout << "Scaling profile for budget = " << maxFunctionEvals
       << ": average HF target = " << average(hf_targets) << std::endl;

  // For reference, compute est_var if pilot over-sample is ignored
  //mfmc_estvar_ratios(rho2_LH, approx_sequence, eval_ratios, estVarRatios);
  //RealVector est_var(numFunctions);
  //Real rnd_hf_target = std::floor(average(hf_targets) + .5);
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  est_var[qoi] = estVarRatios[qoi] * varH[qoi] / rnd_hf_target;
  //Cout << "Avg est var neglecting pilot = " << average(est_var) << '\n';
}


void NonDMultifidelitySampling::
update_hf_targets(const RealMatrix& rho2_LH, const SizetArray& approx_sequence,
		  const RealMatrix& eval_ratios,  const RealVector& var_H,
		  const RealVector& estvar_iter0, RealVector& estvar_ratios,
		  RealVector& hf_targets)
{
  // Note: there is a circular dependency between estvar_ratios and hf_targets
  // > 1st compute r*,rho2 --> EstVar*, then compute EstVar*,tol --> m1* target
  // > if pilot > m1*, then done (numSamples = 0) other than reporting _actual_
  //   EstVar including additional pilot (actual EstVar should be < EstVar*,
  //   although estVarRatios may be > estVarRatio* due to decrease in MC EstVar)
  // > if m1* > pilot, then increment numSamples and continue
  mfmc_estvar_ratios(rho2_LH, approx_sequence, eval_ratios, estvar_ratios);
  // This computes estvar_ratios* from r*,rho2.  Next, m1* from estvar_ratios*;
  // then these estvar_ratios get replaced for actual profile

  // EstVar target = convTol * estvar_iter0 = estvar_ratio * var_H / N_H
  // --> N_H = estvar_ratio * var_H / convTol / estvar_iter0
  // Note: don't simplify further since estvar_iter0 is fixed based on pilot
  hf_targets = estvar_ratios;
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    hf_targets[qoi] *= var_H[qoi] / estvar_iter0[qoi] / convergenceTol;

  Cout << "Scaling profile for convergenceTol = " << convergenceTol
       << ": average HF target = " << average(hf_targets) << std::endl;
}


/** LF only */
void NonDMultifidelitySampling::
update_projected_lf_samples(const RealVector& hf_targets,
			    const RealMatrix& eval_ratios,
			    const SizetArray& N_H_actual, size_t& N_H_alloc,
			    //SizetArray& delta_N_L_actual,
			    Real& delta_equiv_hf)
{
  Sizet2DArray N_L_actual;  inflate(N_H_actual, N_L_actual);
  SizetArray   N_L_alloc;   inflate(N_H_alloc,  N_L_alloc);
  size_t qoi, approx, alloc_incr, actual_incr;
  RealVector lf_targets(numFunctions, false);
  for (approx=0; approx<numApprox; ++approx) {
    for (qoi=0; qoi<numFunctions; ++qoi)
      lf_targets[qoi] = eval_ratios(qoi, approx) * hf_targets[qoi];
    const SizetArray& N_L_actual_a = N_L_actual[approx];
    size_t&           N_L_alloc_a  = N_L_alloc[approx];
    alloc_incr  = one_sided_delta(N_L_alloc_a, average(lf_targets));
    actual_incr = (backfillFailures) ?
      one_sided_delta(N_L_actual_a, lf_targets, 1) : alloc_incr;
    /*delta_N_L_actual[approx] += actual_incr;*/  N_L_alloc_a += alloc_incr;
    increment_equivalent_cost(actual_incr, sequenceCost, approx,
			      delta_equiv_hf);
  }

  finalize_counts(N_L_actual, N_L_alloc);
}


/** LF and HF */
void NonDMultifidelitySampling::
update_projected_samples(const RealVector& hf_targets,
			 const RealMatrix& eval_ratios,
			 const SizetArray& N_H_actual, size_t& N_H_alloc,
			 size_t& delta_N_H_actual,
			 /*SizetArray& delta_N_L_actual,*/ Real& delta_equiv_hf)
{
  // The N_L baseline is the shared set PRIOR to delta_N_H --> important for
  // cost incr even if lf_targets is defined robustly (hf_targets * eval_ratios)
  update_projected_lf_samples(hf_targets, eval_ratios, N_H_actual, N_H_alloc,
			      /*delta_N_L_actual,*/ delta_equiv_hf);

  size_t alloc_incr = one_sided_delta(N_H_alloc, average(hf_targets)),
    actual_incr = (backfillFailures) ?
      one_sided_delta(N_H_actual, hf_targets, 1) : alloc_incr;
  // For analytic solns, mirror the CDV lower bound for numerical solutions --
  // see rationale in NonDNonHierarchSampling::nonhierarch_numerical_solution()
  if ( pilotMgmtMode == OFFLINE_PILOT &&
       ( optSubProblemForm == ANALYTIC_SOLUTION ||
	 optSubProblemForm == REORDERED_ANALYTIC_SOLUTION ) ) {
    size_t offline_N_lwr = 2; //(finalStatsType == QOI_STATISTICS) ? 2 : 1;
    alloc_incr  = std::max(alloc_incr,  offline_N_lwr);
    actual_incr = std::max(actual_incr, offline_N_lwr);
  }
  delta_N_H_actual += actual_incr;  N_H_alloc += alloc_incr;
  increment_equivalent_cost(actual_incr, sequenceCost, numApprox,
			    delta_equiv_hf);
}


void NonDMultifidelitySampling::
approx_increments(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		  IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  const SizetArray& approx_sequence,
		  const RealMatrix& eval_ratios, const RealVector& hf_targets)
{
  // ----------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ----------------------------------------------------------------
  // Note: these results do not affect the HF iteration loop and can be
  // performed after N_H has converged, which simplifies maxFnEvals / convTol
  // logic (no need to further interrogate these throttles below)

  // maxIterations == 0 was inconsistent for targeting the pilot only case
  // (unlike all other throttle values, it did not follow the converged HF
  // iteration with LF increments).  Other ideas (some previously implemented):
  // > NonDControlVarSampling::finalCVRefinement (can be hard-wired false)
  // > maxFunctionEvals could be used as a second throttle (e.g., set equal to
  //   pilot) with additional checks embedded below
  // > use a special pilot-only mode (now implemented in pilotMgmtMode)

  // Notes on approximation sequencing for MFMC:
  // > approx must be ordered on increasing rho2_LH to enable r_i calculation
  //   (see mfmc_analytic_solution() and mfmc_reordered_analytic_solution())
  // > unlike ACV, we enforce that this ordering is retained within r_i through
  //   linear constraint definitions in nonhierarch_numerical_solution()
  //   >> ACV can order approx sample increments based on decreasing r_i
  //      _after_ an unordered nonhierarch_numerical_solution()

  // Pyramid/nested sampling: at step i, we sample approximation range
  // [0,numApprox-1-i] using the delta relative to the previous step
  IntRealMatrixMap sum_L_shared  = sum_L_baseline,
                   sum_L_refined = sum_L_baseline; // copies
  Sizet2DArray N_L_actual_shared;  inflate(N_H_actual, N_L_actual_shared);
  Sizet2DArray N_L_actual_refined = N_L_actual_shared;
  SizetArray   N_L_alloc_refined;  inflate(N_H_alloc, N_L_alloc_refined);
  for (size_t end=numApprox; end>0; --end) {
    // *** TO DO NON_BLOCKING: 2ND PASS ACCUMULATION AFTER 1ST PASS LAUNCH
    if (mfmc_approx_increment(eval_ratios, N_L_actual_refined,
			      N_L_alloc_refined, hf_targets, mlmfIter,
			      approx_sequence, 0, end)) {
      // MFMC samples on [0, approx) --> sum_L_{shared,refined}
      accumulate_mf_sums(sum_L_shared, sum_L_refined, N_L_actual_shared,
			 N_L_actual_refined, approx_sequence, 0, end);
      increment_equivalent_cost(numSamples, sequenceCost, approx_sequence,
				0, end, equivHFEvals);
    }
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  mf_raw_moments(sum_L_baseline, sum_L_shared, sum_L_refined, sum_H, sum_LL,
		 sum_LH, N_L_actual_shared, N_L_actual_refined, N_H_actual,
		 H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_actual_refined, N_L_alloc_refined);
}


bool NonDMultifidelitySampling::
mfmc_approx_increment(const RealMatrix& eval_ratios,
		      const Sizet2DArray& N_L_actual_refined,
		      SizetArray& N_L_alloc_refined,
		      const RealVector& hf_targets, size_t iter,
		      const SizetArray& approx_sequence,
		      size_t start, size_t end)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts

  // When to apply averaging requires some care.  To properly enforce scalar
  // budget for vector QoI, need to either scalarize to average targets from
  // the beginning (scalar hf_target -> scalar lf_target as in ACV) or retain
  // per-QoI targets until the very end (i.e., at numSamples estimation).
  // > all HF QoI get sampled together using a scalar count (average(N*_H))
  //   such that using N*_L(q) = r*(q) N*_H(q) seems non-optimal in isolation,
  //   but vector estimation in update_hf_targets() (avoiding any premature
  //   consolidation) requires vector estimation here to hit budget target.
  //   These LF targets subsequently get averaged by one_sided_delta().
  // > would be interesting to compare averaging from the start vs. averaging
  //   at the very end.
  //   Probably will be similar, in which case we will prefer the latter since
  //   it leaves the door open for per-QoI optimized sample profiles.
  // > Both MFMC and ACV defer rounding until the end (numSamples estimation).

  RealVector lf_targets(numFunctions, false);
  bool ordered = approx_sequence.empty();
  size_t qoi, approx = (ordered) ? end-1 : approx_sequence[end-1];
  for (qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios(qoi, approx) * hf_targets[qoi];
  // These approaches overshoot when combined with vector update_hf_targets():
  //   lf_targets[qoi] = eval_ratios(qoi, approx) * avg_hf_target;
  //   lf_target = avg_eval_ratios[approx] * avg_hf_target;
  if (backfillFailures) {
    const SizetArray& lf_curr = N_L_actual_refined[approx];
    numSamples = one_sided_delta(lf_curr, lf_targets, 1); // average
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples (" << numSamples << ") computed from average "
	   << "delta between targets:\n" << lf_targets
	   << "and current counts:\n" << lf_curr << std::endl;
    size_t N_alloc = one_sided_delta(N_L_alloc_refined[approx],
				     average(lf_targets));
    increment_sample_range(N_L_alloc_refined, N_alloc, approx_sequence,
			   start, end);
  }
  else {
    size_t lf_curr = N_L_alloc_refined[approx];
    numSamples = one_sided_delta(lf_curr, average(lf_targets));
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples (" << numSamples << ") computed from average "
	   << "delta between targets:\n" << lf_targets
	   << "and current allocation = " << lf_curr << std::endl;
    increment_sample_range(N_L_alloc_refined, numSamples, approx_sequence,
			   start, end);
  }
  // the approximation sequence can be managed within one set of jobs using
  // a composite ASV with NonHierarchSurrModel
  return approx_increment(iter, approx_sequence, start, end);
}


void NonDMultifidelitySampling::
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const SizetArray& approx_sequence,
		   const RealMatrix& eval_ratios, RealVector& estvar_ratios)
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
  /*
  Real inner_prod, cost_H = sequenceCost[numApprox];
  size_t qoi, approx, num_am1 = numApprox - 1;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // include cost_H * w_H
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += sequenceCost[approx] * eval_ratios(qoi, approx);
    estvar_ratios[qoi] = (1. - rho2_LH(qoi, num_am1)) * inner_prod / cost_H;
  }
  if (outputLevel >= NORMAL_OUTPUT) {
    for (qoi=0; qoi<numFunctions; ++qoi) {
      for (approx=0; approx<numApprox; ++approx)
	Cout << "  QoI " << qoi+1 << " Approx " << approx+1
	   //<< ": cost_ratio = " << cost_H / cost_L
	     << ": rho2_LH = "    <<     rho2_LH(qoi,approx)
	     << " eval_ratio = "  << eval_ratios(qoi,approx) << '\n';
      Cout << "QoI " << qoi+1 << ": Peherstorfer variance reduction factor = "
	   << estvar_ratios[qoi] << '\n';
    }
    Cout << std::endl;
  }
  */

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq. above.

  // ***************************************************************************
  // Important note: this implementation is being phased out since the estimator
  // variance should not take credit for optimal eval_ratios(qoi,approx) that
  // are not realized in practice due to integration of numSamples across QoI.
  // As for the numerical optimizer cases, avg_eval_ratios is consistent with
  // the downstream utilization of eval_ratios under the assumption of a scalar
  // allocation (neglecting backfill for actual counts).
  // ***************************************************************************

  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, i, ip1;
  switch (optSubProblemForm) {

  // eval_ratios per qoi,approx with no model re-sequencing
  case ANALYTIC_SOLUTION:
    for (qoi=0; qoi<numFunctions; ++qoi) {
      R_sq = 0.;  r_i = eval_ratios(qoi, 0);
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	r_ip1 = eval_ratios(qoi, ip1);
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
      // Note: monotonicity in reordered r_i is enforced in mfmc_eval_ratios()
      // and in linear constraints for nonhierarch_numerical_solution()
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
	r_ip1 = eval_ratios(qoi, approx_ip1);
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


void NonDMultifidelitySampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  // Note: ACV implementation based on F also works for MFMC, but since MFMC
  // has diagonal F, it can be evaluated without per-QoI matrix inversion:
  // > R_sq = a^T [ C o F ]^{-1} a = \Sum_i R_sq_i (sum across set of approx_i)
  // > R_sq_i = F_ii^2 \bar{c}_ii^2 / (F_ii C_ii) for i = approximation number
  //          = F_ii CovLH_i^2 / (VarH_i VarL_i) = F_ii rho2LH_i where
  //   F_ii   = (r_i - r_{i+1}) / (r_i r_{i+1}).
  switch (optSubProblemForm) {
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT: {
    RealVector r;  copy_data_partial(cd_vars, 0, (int)numApprox, r); // N_i
    r.scale(1./cd_vars[numApprox]); // r_i = N_i / N
    // Compiler can resolve overload with (inherited) vector type:
    mfmc_estvar_ratios(rho2LH, approxSequence, r,       estvar_ratios);
    break;
  }
  default: // r_and_N provided: pass leading numApprox terms of cd_vars
    // Compiler can resolve overload with (inherited) vector type:
    mfmc_estvar_ratios(rho2LH, approxSequence, cd_vars, estvar_ratios);
    break;
  }
}


void NonDMultifidelitySampling::
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const SizetArray& approx_sequence,
		   const RealVector& avg_eval_ratios, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq. above.
  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, i, ip1;
  switch (optSubProblemForm) {

  // eval_ratios per qoi,approx with no model re-sequencing
  case ANALYTIC_SOLUTION:
    for (qoi=0; qoi<numFunctions; ++qoi) {
      R_sq = 0.;  r_i = avg_eval_ratios[0];
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	r_ip1 = avg_eval_ratios[ip1];
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
    r_i = avg_eval_ratios[approx];  R_sq = 0.;
    for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
      approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
      r_ip1 = avg_eval_ratios[approx_ip1];
      // Note: monotonicity in reordered r_i is enforced in mfmc_eval_ratios()
      // and in linear constraints for nonhierarch_numerical_solution()
      R_sq += (r_i - r_ip1) / (r_i * r_ip1) * avg_rho2_LH[approx];
      r_i = r_ip1;  approx = approx_ip1;
    }
    R_sq += (r_i - 1.) / r_i * avg_rho2_LH[approx];
    estvar_ratios = (1. - R_sq); // assign scalar to vector components
    break;
  }

  // Call stack for MFMC numerical solution:
  // > NonDNonHierarchSampling::log_average_estvar()
  // > NonDNonHierarchSampling::average_estimator_variance()
  // > NonDMultifidelitySampling::estimator_variance_ratios() [virtual]
  // > This function (vector of avg_eval_ratios from opt design variables)
  // Note: ANALYTIC_SOLUTION above corresponds to ordered case below
  default: {
    bool ordered = approx_sequence.empty();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      approx = (ordered) ? 0 : approx_sequence[0];
      R_sq = 0.;  r_i = avg_eval_ratios[approx];
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
	r_ip1 = avg_eval_ratios[approx_ip1];
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
		   Sizet2DArray& num_L_refined,
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

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
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
mfmc_eval_ratios(const RealMatrix& var_L, const RealMatrix& rho2_LH,
		 const RealVector& cost,  SizetArray& approx_sequence,
		 RealMatrix& eval_ratios, RealVector& hf_targets)
               //bool for_warm_start)
{
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);
  if (hf_targets.empty()) hf_targets.sizeUninitialized(numFunctions);

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
    optSubProblemForm = (budget_constr) ? N_VECTOR_LINEAR_CONSTRAINT :
      N_VECTOR_LINEAR_OBJECTIVE;
    break;
  case NUMERICAL_FALLBACK: // default
    ordered_rho = ordered_approx_sequence(rho2_LH); // all QoI for all Approx
    if (ordered_rho)
      optSubProblemForm = ANALYTIC_SOLUTION;
    else {
      optSubProblemForm = (budget_constr) ? N_VECTOR_LINEAR_CONSTRAINT :
	N_VECTOR_LINEAR_OBJECTIVE;
      Cout << "MFMC: model sequence provided is out of order with respect to "
	   << "Low-High\n      correlation for at least one QoI.  Switching "
	   << "to numerical solution.\n";
    }
    break;
  case REORDERED_FALLBACK: // not currently in XML spec, but could be added
    ordered_rho = ordered_approx_sequence(rho2_LH); // all QoI for all Approx
    if (ordered_rho)
      optSubProblemForm = ANALYTIC_SOLUTION;
    else {
      optSubProblemForm = REORDERED_ANALYTIC_SOLUTION;
      Cout << "MFMC: model sequence provided is out of order with respect to "
	   << "Low-High\n      correlation for at least one QoI.  Switching "
	   << "to alternate analytic solution.\n";
    }
    break;
  }

  switch (optSubProblemForm) {
  case ANALYTIC_SOLUTION:
    Cout << "MFMC: model sequence provided is ordered in Low-High correlation "
	 << "for all QoI.\n      Computing standard analytic solution.\n"
	 << std::endl;
    approx_sequence.clear();
    mfmc_analytic_solution(rho2_LH, cost, eval_ratios);
    break;
  case REORDERED_ANALYTIC_SOLUTION: // inactive (see above)
    mfmc_reordered_analytic_solution(rho2_LH, cost, approx_sequence,
				     eval_ratios, true); // monotonic r for seq
    break;
  default: // any of several numerical optimization formulations
    mfmc_numerical_solution(var_L, rho2_LH, cost, approx_sequence,
			    eval_ratios, hf_targets);
    break;
  }

  // Numerical solution has updated hf_target from the computed N*.
  // Analytic solutions scale the profile to target budget || accuracy.
  switch (optSubProblemForm) {
  case ANALYTIC_SOLUTION:  case REORDERED_ANALYTIC_SOLUTION:
    if (maxFunctionEvals != SZ_MAX)
      update_hf_targets(eval_ratios, sequenceCost, hf_targets);
    else //if (convergenceTol != -DBL_MAX) *** TO DO: need special default value
      update_hf_targets(rho2_LH, approx_sequence, eval_ratios, varH,
			estVarIter0, estVarRatios, hf_targets);
    break;
  }
}


void NonDMultifidelitySampling::
mfmc_numerical_solution(const RealMatrix& var_L, const RealMatrix& rho2_LH,
			const RealVector& cost,  SizetArray& approx_sequence,
			RealMatrix& eval_ratios, RealVector& hf_targets)
{
  size_t qoi, approx, num_am1 = numApprox - 1, hf_form_index, hf_lev_index;
  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real cost_L, cost_H = cost[numApprox], budget = (Real)maxFunctionEvals, r_i,
    avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
  bool budget_constrained = (maxFunctionEvals != SZ_MAX),
       budget_exhausted   = (equivHFEvals >= budget);
  //if (budget_exhausted) budget = equivHFEvals;

  RealVector& avg_eval_ratios = mfmcSolnData.avgEvalRatios;
  Real&       avg_hf_target   = mfmcSolnData.avgHFTarget;
  if (mlmfIter == 0) {

    if (budget_exhausted) { // only 1 feasible pt, no need for solve
      avg_eval_ratios.sizeUninitialized(numApprox);
      eval_ratios = 1.;       avg_eval_ratios = 1.;
      hf_targets  = avg_N_H;  avg_hf_target   = avg_N_H;
      return;
    }

    // Compute approx_sequence and r* initial guess from analytic MFMC
    if (ordered_approx_sequence(rho2_LH)) {// can happen w/ NUMERICAL_OVERRIDE
      approx_sequence.clear();
      mfmc_analytic_solution(rho2_LH, cost, eval_ratios);
    }
    else // If misordered rho, enforce that r increases monotonically across
         // approx_sequence for consistency w/ linear constr in numerical soln
      mfmc_reordered_analytic_solution(rho2_LH, cost, approx_sequence,
				       eval_ratios, true);// monotonic r

    average(eval_ratios, 0, avg_eval_ratios);// avg over qoi for each approx
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Initial guess from analytic MFMC (average eval ratios):\n"
	   << avg_eval_ratios << std::endl;

    RealVector hf_targets;
    if (budget_constrained) {
      update_hf_targets(eval_ratios, cost, hf_targets); // from maxFunctionEvals
      avg_hf_target = average(hf_targets);
      // scale to enforce budget constraint.  Since the profile does not emerge
      // (make numerical MFMC more resilient to pilot over-estimation like ACV),
      // don't select an infeasible initial guess:
      // > if N* < N_pilot, scale back r* for use initial = scaled_r*,N_pilot
      // > if N* > N_pilot, use initial = r*,N*
      if (avg_N_H > avg_hf_target) { // rescale r* for over-estimated pilot
	scale_to_budget_with_pilot(avg_eval_ratios, cost, avg_N_H);
	avg_hf_target = avg_N_H;
      }
    }
    else { // accuracy-constrained
      update_hf_targets(rho2_LH, approx_sequence, eval_ratios, varH,
			estVarIter0, estVarRatios, hf_targets);
      avg_hf_target = average(hf_targets);
    }
  }
  // This step no longer needed since mfmcSolnData.avgEvalRatios persists:
  //else // warm start from previous solution
  //  average(eval_ratios, 0, avg_eval_ratios);// avg over qoi

  // define covLH and covLL from rho2LH, var_L, varH
  correlation_sq_to_covariance(rho2_LH, var_L, varH, covLH);
  matrix_to_diagonal_array(var_L, covLL);

  // Base class implementation of numerical solve (shared with ACV,GenACV):
  nonhierarch_numerical_solution(cost,approx_sequence,mfmcSolnData,numSamples);

  // inflate averages from numerical opt into eval_ratios and hf_targets
  inflate(avg_eval_ratios, eval_ratios);
  hf_targets = avg_hf_target; // assign scalar to vector components
}


void NonDMultifidelitySampling::
mfmc_estimator_variance(const RealMatrix& rho2_LH, const RealVector& var_H,
			const SizetArray& N_H, const RealVector& hf_targets,
			const SizetArray& approx_sequence,
			const RealMatrix& eval_ratios,
			RealVector& estvar_ratios, Real& avg_est_var)
{
  switch (optSubProblemForm) {
  // For these cases, it is convenient to compute estimator variance ratios
  // using the expression for R^2
  case ANALYTIC_SOLUTION:  case REORDERED_ANALYTIC_SOLUTION: {

    if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

    bool scale_to_N_H = false;
    Real avg_hf_target = average(hf_targets);
    size_t qoi, approx, rnd_hf_target = (size_t)std::floor(avg_hf_target + .5);
    for (qoi=0; qoi<numFunctions; ++qoi)
      if (N_H[qoi] > rnd_hf_target)// over-shoot of tgt from pilot | iteration
	scale_to_N_H = true;

    if (scale_to_N_H) {
      // R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
      // --> take credit for N_H > N* by using r_actual < r* for N_H > m1*
      // --> N_L is kept fixed at r* m1* (see lf_targets in approx_increment()),
      //     but r_actual = N_L / N_H = r* m1* / N_H
      RealMatrix scaled_eval_ratios = eval_ratios; // copy
      Real star_to_actual;
      for (qoi=0; qoi<numFunctions; ++qoi) {
	star_to_actual = avg_hf_target / (Real)N_H[qoi];
	for (approx=0; approx<numApprox; ++approx) // no need to sequence
	  scaled_eval_ratios(qoi, approx) *= star_to_actual; // r* -> r_actual
      }
      mfmc_estvar_ratios(rho2_LH, approx_sequence, scaled_eval_ratios,
			 estvar_ratios);

      // verify correct N_L is preserved after star_to_actual:
      //for (approx=0; approx<numApprox; ++approx) 
      //  Cout << "avg N_L[" <<approx<< "] = "<< N_L[approx]/numFunctions<<'\n';
    }
    else
      mfmc_estvar_ratios(rho2_LH, approx_sequence, eval_ratios, estvar_ratios);

    // update avg_est_var for final variance report and finalStats
    estvar_ratios_to_avg_estvar(estvar_ratios, var_H, N_H, avg_est_var);

    if (outputLevel >= NORMAL_OUTPUT) {
      bool ordered = approx_sequence.empty();
      for (qoi=0; qoi<numFunctions; ++qoi) {
	for (size_t i=0; i<numApprox; ++i) {
	  approx = (ordered) ? i : approx_sequence[i];
	  Cout << "  QoI " << qoi+1 << " Approx " << approx+1
	    //<< ": cost_ratio = " << cost_H / cost_L
	       << ": rho2_LH = "    <<     rho2_LH(qoi,approx)
	       << " eval_ratio = "  << eval_ratios(qoi,approx) << '\n';
	}
	Cout << "QoI " << qoi+1 << ": variance reduction factor = "
	     << estvar_ratios[qoi] << '\n';
      }
      Cout << std::endl;
    }
    // numSamples is relative to N_H, but the approx_increments() below are
    // computed relative to hf_targets (independent of sunk cost for pilot)
    numSamples = one_sided_delta(N_H, hf_targets, 1);
    //numSamples = std::min(num_samp_budget, num_samp_ctol); // enforce both
    break;
  }
  // For numerical cases, mfmc_numerical_solution() must incorporate varH/N_H
  // in the objective and returns avg estvar as the final objective.  So estVar
  // is more direct here than estVarRatios.
  //default:
  //  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);
  //  for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //    estvar_ratios[qoi] = avg_est_var / var_H[qoi] * N_H[qoi]; // (1-R^2)
  //  break;
  }
}


void NonDMultifidelitySampling::
mf_raw_moments(IntRealMatrixMap& sum_L_baseline, IntRealMatrixMap& sum_L_shared,
	       IntRealMatrixMap& sum_L_refined,  IntRealVectorMap& sum_H,
	       IntRealMatrixMap& sum_LL,         IntRealMatrixMap& sum_LH,
	       const Sizet2DArray& N_L_shared, const Sizet2DArray& N_L_refined,
	       const SizetArray&   N_H,        RealMatrix& H_raw_mom)
{
  // Note: ACV-like numerical solutions solve all-at-once for beta as a vector
  // > beta = [ C o F ]^{-1} [diag(F) o c] which, for diagonal F in MFMC,
  //   simplifies to beta_i = F_ii c_i / F_ii C_ii = covLH_i / varL_i
  // --> no need to incur matrix inversion overhead for MFMC; stick with same
  //     scalar approach used for analytic solutions.

  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  Real beta, sum_H_mq;
  size_t approx, qoi, N_H_q;//, N_shared;
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix& sum_L_base_m = sum_L_baseline[mom];
    RealMatrix& sum_L_sh_m   = sum_L_shared[mom];
    RealMatrix& sum_L_ref_m  = sum_L_refined[mom];
    RealVector& sum_H_m      = sum_H[mom];
    RealMatrix& sum_LL_m     = sum_LL[mom];
    RealMatrix& sum_LH_m     = sum_LH[mom];

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      sum_H_mq = sum_H_m[qoi];  N_H_q = N_H[qoi];
      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	// Uses a baseline {sum,N}_L consistent with H,LL,LH accumulators:
	compute_mf_control(sum_L_base_m(qoi,approx), sum_H_mq,
			   sum_LL_m(qoi,approx), sum_LH_m(qoi,approx),
			   N_H_q, beta); // shared HF baseline
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1
	       << ": control variate beta = " << std::setw(9) << beta << '\n';
	// For MFMC, shared accumulators and counts telescope pairwise
	apply_control(sum_L_sh_m(qoi,approx),  N_L_shared[approx][qoi],
		      sum_L_ref_m(qoi,approx), N_L_refined[approx][qoi],
		      beta, H_raw_mq);
      }
    }
  }
  if (outputLevel >= NORMAL_OUTPUT) Cout << std::endl;
}


void NonDMultifidelitySampling::print_variance_reduction(std::ostream& s)
{
  switch (optSubProblemForm) {

  case ANALYTIC_SOLUTION:  case REORDERED_ANALYTIC_SOLUTION: {
    size_t wpp7 = write_precision + 7;
    s << "<<<<< Variance for mean estimator:\n";

    if (pilotMgmtMode != OFFLINE_PILOT)
      s << "      Initial MC (" << std::setw(5)
	<< (size_t)std::floor(average(numHIter0) + .5) << " HF samples): "
	<< std::setw(wpp7) << average(estVarIter0) << '\n';

    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    // est_var is projected for cases that are not fully iterated/incremented
    Real proj_equiv_hf = equivHFEvals + deltaEquivHF;
    RealVector mc_est_var(numFunctions, false);
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      mc_est_var[qoi] = varH[qoi] / (N_H_actual[qoi] + deltaNActualHF);
    Real avg_mc_est_var        = average(mc_est_var),
         avg_budget_mc_est_var = average(varH) / proj_equiv_hf;
    String type = (pilotMgmtMode == PILOT_PROJECTION) ? "Projected":"   Online";
    s << "  " << type << "   MC (" << std::setw(5)
      << (size_t)std::floor(average(N_H_actual) + deltaNActualHF + .5)
      << " HF samples): " << std::setw(wpp7) << avg_mc_est_var << "\n  "
      << type << " MFMC (sample profile):   " << std::setw(wpp7)
      << mfmcSolnData.avgEstVar << "\n  " << type <<" MFMC ratio (1 - R^2):    "
      // report ratio of averages rather than average of ratios (consistent
      // with ACV definition which would have to recompute the latter)
      << std::setw(wpp7) << mfmcSolnData.avgEstVar / avg_mc_est_var
      << "\n Equivalent   MC (" << std::setw(5)
      << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
      << std::setw(wpp7) << avg_budget_mc_est_var
      << "\n Equivalent MFMC ratio:              " << std::setw(wpp7)
      << mfmcSolnData.avgEstVar / avg_budget_mc_est_var << '\n';
    break;
  }

  // For numerical cases, mfmc_numerical_solution() must incorporate varH / N_H
  // in the objective and returns avg estvar as the final objective.  So estVar
  // is more direct here than estVarRatios, as for NonDACVSampling.
  default: // numerical solution
    print_estimator_performance(s, mfmcSolnData); break;
  }
}

} // namespace Dakota
