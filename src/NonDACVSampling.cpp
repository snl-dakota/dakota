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
#include "NonDACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDNonHierarchSampling(problem_db, model)//, multiStartACV(true)
{
  mlmfSubMethod = problem_db.get_ushort("method.sub_method");

  if (maxFunctionEvals == SZ_MAX) // accuracy constraint (convTol)
    optSubProblemForm = N_MODEL_LINEAR_OBJECTIVE;
  else {                   // budget constraint (maxFunctionEvals)
    // truthFixedByPilot is a user-specified option for fixing the number of
    // HF samples (to those in the pilot).  In this case, equivHF budget is
    // allocated by optimizing r* for fixed N.
    bool offline = (pilotMgmtMode == OFFLINE_PILOT ||
		    pilotMgmtMode == OFFLINE_PILOT_PROJECTION);
    optSubProblemForm = (truthFixedByPilot && !offline) ?
      R_ONLY_LINEAR_CONSTRAINT : N_MODEL_LINEAR_CONSTRAINT;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "ACV sub-method selection = " << mlmfSubMethod
	 << " sub-method formulation = "  << optSubProblemForm
	 << " sub-problem solver = "      << optSubProblemSolver << std::endl;

  // approximation set for ACV includes all approximations
  // defining approxSet allows reuse of functions across ACV and GenACV
  approxSet.resize(numApprox);
  for (size_t i=0; i<numApprox; ++i)
    approxSet[i] = i;

  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    numGroups, pilotSamples);

  size_t max_ps = find_max(pilotSamples);
  if (max_ps) maxEvalConcurrency *= max_ps;
}


NonDACVSampling::~NonDACVSampling()
{ }


void NonDACVSampling::core_run()
{
  if (mlmfSubMethod == SUBMETHOD_ACV_RD) {
    Cerr << "Error: ACV recursive difference not yet implemented." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // bypass for GenACV:
  update_model_groups();
  if (!onlineCost) update_model_group_costs();

  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

  switch (pilotMgmtMode) {
  case  ONLINE_PILOT: // iterated ACV (default)
    // ESTIMATOR_PERFORMANCE case is still iterated for N_H, and therefore
    // differs from ONLINE_PILOT_PROJECTION
    approximate_control_variate_online_pilot();     break;
  case OFFLINE_PILOT: // computes perf for offline/Oracle correlation
    switch (finalStatsType) {
    // since offline is not iterated, the ESTIMATOR_PERFORMANCE case is the
    // same as OFFLINE_PILOT_PROJECTION --> bypass IntMaps, simplify code
    case ESTIMATOR_PERFORMANCE:
      approximate_control_variate_pilot_projection(); break;
    default:
      approximate_control_variate_offline_pilot();    break;
    }
    break;
  case  ONLINE_PILOT_PROJECTION:
  case OFFLINE_PILOT_PROJECTION:
    approximate_control_variate_pilot_projection(); break;
  }
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate_online_pilot()
{
  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;  RealMatrix var_L;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  //initialize_acv_counts(N_H_actual, N_LL);
  //initialize_acv_covariances(covLL, covLH, varH);

  // Initialize for pilot sample
  //size_t hf_shared_pilot = numSamples, start=0,
  //  lf_shared_pilot = find_min(pilotSamples, start, numApprox-1);

  //acvSolnData.avgHFTarget = 0.;
  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += (backfillFailures && mlmfIter) ? one_sided_delta(N_H_alloc,
      acvSolnData.solution_reference(), relaxFactor) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (onlineCost && mlmfIter == 0)
      { recover_online_cost(allResponses); update_model_group_costs(); }
    increment_equivalent_cost(numSamples, sequenceCost, 0, numGroups,
			      equivHFEvals);
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, var_L, varH, covLL, covLH);

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, acvSolnData);
    ++mlmfIter;  advance_relaxation();
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, acvSolnData);
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(acvSolnData, approxSet, N_H_actual,
				N_H_alloc, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  RealVector sum_H_pilot, sum_HH_pilot;
  RealMatrix sum_L_pilot, sum_LH_pilot, var_L;
  RealSymMatrixArray sum_LL_pilot;  SizetArray N_shared_pilot;
  evaluate_pilot(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		 sum_HH_pilot, N_shared_pilot, false);
  if (onlineCost) update_model_group_costs();
  compute_LH_statistics(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
			sum_HH_pilot, N_shared_pilot, var_L, varH, covLL,covLH);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  //initialize_acv_counts(N_H_actual, N_LL);
  //initialize_acv_covariances(covLL, covLH, varH);

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(var_L, acvSolnData);
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // QOI_STATISTICS case; ESTIMATOR_PERFORMANCE redirects to _pilot_projection()

  // perform the shared increment for the online sample profile
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_H_actual);//, N_LL);
  N_H_alloc += numSamples;
  increment_equivalent_cost(numSamples, sequenceCost, 0,numGroups,equivHFEvals);
  // perform LF increments for the online sample profile
  approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		    N_H_alloc, acvSolnData);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate_pilot_projection()
{
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];

  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealVector sum_H, sum_HH;  RealMatrix sum_L, sum_LH, var_L;
  RealSymMatrixArray sum_LL;
  switch (pilotMgmtMode) {
  case OFFLINE_PILOT: // redirected from _offline_pilot() in core_run()
  case OFFLINE_PILOT_PROJECTION: {
    SizetArray N_shared_pilot;
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot, false);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot,
			  var_L, varH, covLL, covLH);
    N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
    break;
  }
  default: // ONLINE_PILOT_PROJECTION
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual, true);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual,
			  var_L, varH, covLL, covLH);
    N_H_alloc = numSamples;
    break;
  }
  if (onlineCost) update_model_group_costs();

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(var_L, acvSolnData);
  ++mlmfIter;

  // No LF increments or final moments for pilot projection
  update_projected_samples(acvSolnData, approxSet, N_H_actual, N_H_alloc,
			   deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDACVSampling::
evaluate_pilot(RealMatrix& sum_L_pilot, RealVector& sum_H_pilot,
	       RealSymMatrixArray& sum_LL_pilot, RealMatrix& sum_LH_pilot,
	       RealVector& sum_HH_pilot, SizetArray& N_shared_pilot,
	       bool incr_cost)
{
  initialize_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot);
  N_shared_pilot.assign(numFunctions, 0);
  //initialize_acv_counts(N_shared_pilot, N_LL_pilot);

  // ----------------------------------------
  // Compute var L,H & covar LL,LH from pilot
  // ----------------------------------------
  // Initialize for pilot sample
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_pilot,//_baselineH, sum_L_baselineL,
		      sum_H_pilot, sum_LL_pilot, sum_LH_pilot, sum_HH_pilot,
		      N_shared_pilot);//, N_LL_pilot);

  // TO DO: allow pilot to vary for C vs c.  numSamples logic after pilot
  // (mlmfIter >= 1) will require _baseline{L,H}
  //if (lf_shared_pilot > hf_shared_pilot) {
  //  numSamples = lf_shared_pilot - hf_shared_pilot;
  //  shared_approx_increment(mlmfIter); // spans all approx models
  //  accumulate_acv_sums(sum_L_baselineL, sum_LL,//_baselineL,
  //                      N_L_baselineL);
  //  if (incr_cost)
  //    increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox,
  //			            equivHFEvals);
  //}

  if (onlineCost) // don't update group costs (GenACV usage)
    recover_online_cost(allResponses); //update_model_group_costs();
  if (incr_cost)
    increment_equivalent_cost(numSamples,sequenceCost,0,numGroups,equivHFEvals);
}


void NonDACVSampling::
compute_LH_statistics(RealMatrix& sum_L_pilot, RealVector& sum_H_pilot,
		      RealSymMatrixArray& sum_LL_pilot,
		      RealMatrix& sum_LH_pilot, RealVector& sum_HH_pilot,
		      SizetArray& N_shared_pilot, RealMatrix& var_L,
		      RealVector& var_H, RealSymMatrixArray& cov_LL,
		      RealMatrix& cov_LH)
{
  if (mlmfIter == 0) // see var_L usage in compute_ratios()
    compute_L_variance(sum_L_pilot, sum_LL_pilot, N_shared_pilot, var_L);
  compute_variance(sum_H_pilot, sum_HH_pilot, N_shared_pilot, var_H);
  compute_LL_covariance(sum_L_pilot,//_baselineL,
                        sum_LL_pilot,//N_LL_pilot,
			N_shared_pilot, cov_LL);
  compute_LH_covariance(sum_L_pilot,//baselineH,
			sum_H_pilot, sum_LH_pilot, N_shared_pilot, cov_LH);
  //Cout << "var_H:\n"<< var_H << "cov_LL:\n"<< cov_LL << "cov_LH:\n"<< cov_LH;
}


void NonDACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baselineH, IntRealVectorMap& sum_H,
		  IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  const MFSolutionData& soln)
{
  // ---------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ---------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after N_H has converged

  // Perform a sample sequence that reuses sample increments: define
  // approx_sequence in decreasing r_i order, directionally consistent
  // with default approx indexing for well-ordered models
  // > approx 0 is lowest fidelity --> lowest corr,cost --> highest r_i
  SizetArray approx_sequence;  bool descending = true;
  RealVector avg_eval_ratios = soln.solution_ratios();
  ordered_approx_sequence(avg_eval_ratios, approx_sequence, descending);
  if (!approx_sequence.empty())
    { update_model_groups(approx_sequence);  update_model_group_costs(); }

  /*
  size_t start, end;
  for (end=numApprox; end>0; --end) {
    // pairwise (IS and RD) or pyramid (MF):
    start = (mlmfSubMethod == SUBMETHOD_ACV_MF) ? 0 : end - 1;
    // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER 1ST PASS LAUNCH
    if (acv_approx_increment(soln, N_L_actual_refined, N_L_alloc_refined,
			     mlmfIter, approx_sequence, start, end)) {
      // ACV_IS samples on [approx-1,approx) --> sum_L_refined
      // ACV_MF samples on [0, approx)       --> sum_L_refined
      accumulate_acv_sums(sum_L_refined, N_L_actual_refined, approx_sequence,
			  start, end);
      increment_equivalent_cost(numSamples, sequenceCost, approx_sequence,
				start, end, equivHFEvals);
    }
  }
  */

  Sizet2DArray N_L_actual_shared, N_L_actual_refined;
  SizetArray   N_L_alloc_refined, delta_N_G(numGroups);
  inflate(N_H_actual, N_L_actual_shared); inflate(N_H_alloc, N_L_alloc_refined);
  delta_N_G[numApprox] = 0;
  const RealVector& soln_vars = soln.solution_variables();
  for (int g=numApprox-1; g>=0; --g) // base to top, excluding all-model group
    delta_N_G[g] = acv_approx_increment(soln_vars, N_L_actual_refined,
					N_L_alloc_refined, modelGroups[g]);
  group_increment(delta_N_G, mlmfIter, true); // reverse order for RNG sequence
  // Note: use of this fn requires modelGroupCost to be kept in sync for all
  // cases, not just numerical solves
  increment_equivalent_cost(delta_N_G, modelGroupCost, sequenceCost[numApprox],
			    equivHFEvals);
  IntRealMatrixArrayMap sum_G;  initialize_group_sums(sum_G);
  Sizet2DArray     N_G_actual;  initialize_group_counts(N_G_actual);
  accumulate_group_sums(sum_G, N_G_actual, batchResponsesMap);
  // Map from "horizontal" group incr to "vertical" model incr (see JCP ACV)
  IntRealMatrixMap sum_L_shared = sum_L_baseline, sum_L_refined;
  overlay_approx_group_sums(sum_G, N_G_actual, sum_L_shared, sum_L_refined,
			    N_L_actual_shared, N_L_actual_refined); // *** TO DO ***: defer and consolidate w/ DAG-based GenACV implementation

  // -----------------------------------------------------------
  // Compute/apply control variate parameter to estimate moments
  // -----------------------------------------------------------
  RealMatrix H_raw_mom(4, numFunctions);
  acv_raw_moments(sum_L_baselineH, sum_L_refined, sum_H, sum_LL, sum_LH,
		  avg_eval_ratios, N_H_actual, N_L_actual_refined, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_actual_refined, N_L_alloc_refined);
}


size_t NonDACVSampling::
acv_approx_increment(const RealVector& soln_vars,
		     const Sizet2DArray& N_L_actual, SizetArray& N_L_alloc,
		     const UShortArray& model_group)
{
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment allowing use of
  //   one_sided_delta() with latest counts

  size_t num_samp, approx = model_group.back();
  Real   lf_target = soln_vars[approx]; // no relaxation for approx increments
  if (backfillFailures) {
    const SizetArray& lf_curr = N_L_actual[approx];
    num_samp = one_sided_delta(lf_curr, lf_target); // delta of average
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << num_samp << " computed from average delta "
	   << "between LF target = " << lf_target << " and current counts:\n"
	   << lf_curr << std::endl;
    size_t N_alloc = one_sided_delta(N_L_alloc[approx], lf_target);
    increment_sample_range(N_L_alloc, N_alloc, model_group);
  }
  else {
    size_t lf_curr = N_L_alloc[approx];
    num_samp = one_sided_delta((Real)lf_curr, lf_target);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << num_samp << " computed from delta between "
	   << "LF target = " << lf_target << " and current allocation = "
	   << lf_curr << std::endl;
    increment_sample_range(N_L_alloc, num_samp, model_group);
  }
  return num_samp;
}


void NonDACVSampling::update_model_groups()
{
  // Note: model selection case handled by NonDDGenACV, so here numGroups
  // is the total number of model instances
  if (modelGroups.size() != numGroups)
    modelGroups.resize(numGroups);
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF:
    for (size_t g=0; g<numGroups; ++g)
      mfmc_model_group(g, modelGroups[g]);
    break;
  case SUBMETHOD_ACV_IS:
    for (size_t g=0; g<numGroups; ++g)
      cvmc_model_group(g, modelGroups[g]);
    break;
  case SUBMETHOD_ACV_RD:
    for (size_t g=0; g<numGroups; ++g)
      mlmc_model_group(g, modelGroups[g]);
    break;
  }
}


void NonDACVSampling::update_model_groups(const SizetArray& approx_sequence)
{
  if (approx_sequence.empty())
    update_model_groups();

  // Note: model selection case handled by NonDDGenACV, so here numGroups
  // is the total number of model instances
  if (modelGroups.size() != numGroups)
    modelGroups.resize(numGroups);
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF:
    for (size_t g=0; g<numGroups; ++g)
      mfmc_model_group(g, approx_sequence, modelGroups[g]);
    break;
  case SUBMETHOD_ACV_IS:
    for (size_t g=0; g<numGroups; ++g)
      cvmc_model_group(g, approx_sequence, modelGroups[g]);
    break;
  case SUBMETHOD_ACV_RD:
    for (size_t g=0; g<numGroups; ++g)
      mlmc_model_group(g, approx_sequence, modelGroups[g]);
    break;
  }
}


void NonDACVSampling::
compute_ratios(const RealMatrix& var_L, MFSolutionData& soln)
{
  // Solve the optimization sub-problem using an initial guess from either
  // related analytic solutions (iter == 0) or warm started from the previous
  // solutions (iter >= 1)

  bool budget_constrained = (maxFunctionEvals != SZ_MAX), budget_exhausted
    = (budget_constrained && equivHFEvals >= (Real)maxFunctionEvals),
    no_solve = (budget_exhausted || convergenceTol >= 1.); // bypass opt solve

  if (mlmfIter == 0) {
    bool online = (pilotMgmtMode == ONLINE_PILOT ||
		   pilotMgmtMode == ONLINE_PILOT_PROJECTION);
    if (online) // cache reference estVarIter0
      cache_mc_reference();

    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;

    if (no_solve) { // no need for solve
      // For r_i = 1, C_F,c_f = 0 --> NUDGE for downstream CV numerics
      RealVector avg_eval_ratios(numApprox, false);
      avg_eval_ratios = 1. + RATIO_NUDGE;
      soln.anchored_solution_ratios(avg_eval_ratios, avg_N_H);
      // For offline pilot, the online EstVar is undefined prior to any online
      // samples, but should not happen (no budget used) unless bad convTol spec
      if (online)
	soln.average_estimator_variance(average(estVarIter0));
      else
	soln.average_estimator_variance(std::numeric_limits<Real>::infinity());
      soln.average_estimator_variance_ratio(1.);
      numSamples = 0;  return;
    }

    switch (optSubProblemSolver) {
    // global and sequenced global+local methods (no initial guess)
    case SUBMETHOD_DIRECT_NPSOL_OPTPP:  case SUBMETHOD_DIRECT_NPSOL:
    case SUBMETHOD_DIRECT_OPTPP:        case SUBMETHOD_DIRECT:
    case SUBMETHOD_EGO:  case SUBMETHOD_SBGO:  case SUBMETHOD_EA:
      ensemble_numerical_solution(soln);
      break;
    default: {
      // Run a competition among related analytic approaches (MFMC or pairwise
      // CVMC) for best initial guess, where each initial guess may additionally
      // employ multiple varianceMinimizers in ensemble_numerical_solution()
      RealMatrix rho2_LH;
      covariance_to_correlation_sq(covLH, var_L, varH, rho2_LH);
      MFSolutionData mf_soln, cv_soln;
      analytic_initialization_from_mfmc(rho2_LH, avg_N_H, mf_soln);
      analytic_initialization_from_ensemble_cvmc(rho2_LH, avg_N_H, cv_soln);

      //if (multiStartACV) { // Run numerical solns from both starting points
      ensemble_numerical_solution(mf_soln);
      ensemble_numerical_solution(cv_soln);
      pick_mfmc_cvmc_solution(mf_soln, cv_soln, soln);
      //}
      /*
      else { // Run one numerical soln from best of two starting points
        bool mfmc_init;
        if (budget_constrained) { // same cost, compare accuracy
	  RealVector cdv;
	  r_and_N_to_design_vars(mf_soln.avgEvalRatios,mf_soln.avgHFTarget,cdv);
	  mf_soln.avgEstVar = average_estimator_variance(cdv); // ACV or GenACV
	  r_and_N_to_design_vars(cv_soln.avgEvalRatios,cv_soln.avgHFTarget,cdv);
	  cv_soln.avgEstVar = average_estimator_variance(cdv); // ACV or GenACV
	  mfmc_init = (mf_soln.avgEstVar < cv_soln.avgEstVar);
        }
        else { // same accuracy (convergenceTol * estVarIter0), compare cost 
	  mf_soln.equivHFAlloc = compute_equivalent_cost(mf_soln.avgHFTarget,
	    mf_soln.avgEvalRatios, sequenceCost);
	  cv_soln.equivHFAlloc = compute_equivalent_cost(cv_soln.avgHFTarget,
	    cv_soln.avgEvalRatios, sequenceCost);
	  mfmc_init = (mf_soln.equivHFAlloc < cv_soln.equivHFAlloc);
        }
        soln = (mfmc_init) ? mf_soln : cv_soln;
        // Single solve initiated from lowest estvar
        ensemble_numerical_solution(soln);
      }
      */
      break;
    }
    }
  }
  else { // subsequent iterations
    if (no_solve) // leave soln at previous values
      { numSamples = 0; return; }

    // no scaling needed from prev soln (as in NonDLocalReliability) since
    // updated avg_N_H now includes allocation from previous solution and
    // should be active on constraint bound (excepting sample count rounding)

    // warm start from previous solution (for active or one-and-only DAG)
    ensemble_numerical_solution(soln);
  }

  process_model_solution(soln, numSamples);
  if (outputLevel >= NORMAL_OUTPUT)
    print_model_solution(Cout, soln, approxSet);
}


void NonDACVSampling::
analytic_initialization_from_mfmc(const RealMatrix& rho2_LH, Real avg_N_H,
				  MFSolutionData& soln)
{
  // > Option 1 is analytic MFMC: differs from ACV due to recursive pairing
  RealVector avg_eval_ratios;
  if (ordered_approx_sequence(rho2_LH)) // for all QoI across all Approx
    mfmc_analytic_solution(approxSet, rho2_LH, sequenceCost, avg_eval_ratios);
  else {
    // compute reordered MFMC for averaged rho; monotonic r not required
    // > any rho2_LH re-ordering from MFMC initial guess can be ignored (later
    //   gets replaced with r_i ordering for approx_increments() sampling)
    SizetArray approx_sequence;
    mfmc_reordered_analytic_solution(approxSet, rho2_LH, sequenceCost,
				     approx_sequence, avg_eval_ratios);
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from analytic MFMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  Real avg_hf_target;
  if (maxFunctionEvals == SZ_MAX) // HF target from ACV estvar using MFMC soln
    avg_hf_target = update_hf_target(avg_eval_ratios,avg_N_H,varH,estVarIter0);
  else // allocate_budget(), then manage lower bounds and pilot over-estimation
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, avg_hf_target,
		    (Real)maxFunctionEvals);
  soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
}


void NonDACVSampling::
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

  Real avg_hf_target;
  if (maxFunctionEvals == SZ_MAX) // HF target from ACV estvar using CVMC solns
    avg_hf_target = update_hf_target(avg_eval_ratios, avg_N_H,varH,estVarIter0);
  else // allocate_budget(), then manage lower bounds and pilot over-estimation
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, avg_hf_target,
		    (Real)maxFunctionEvals);
  soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
}


void NonDACVSampling::
print_model_solution(std::ostream& s, const MFSolutionData& soln,
		     const UShortArray& approx_set)
{
  const RealVector& soln_vars = soln.solution_variables();
  size_t i, num_approx = approx_set.size();
  for (i=0; i<num_approx; ++i)
    Cout << "Approx " << approx_set[i] + 1 << ": sample allocation = "
	 << soln_vars[i] << '\n';
  Cout << "Truth:   sample allocation = " << soln_vars[num_approx] << '\n';
  if (maxFunctionEvals == SZ_MAX)
    Cout << "Estimator cost allocation = " << soln.equivalent_hf_allocation()
	 << std::endl;
  else
    Cout << "Average estimator variance = " << soln.average_estimator_variance()
	 << "\nAverage ACV variance / average MC variance = "
	 << soln.average_estimator_variance_ratio() << std::endl;
}


void NonDACVSampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  // linear inequality constraints on sample counts:
  //  N_i >  N (aka r_i > 1) prevents numerical exceptions
  // (N_i >= N becomes N_i > N based on RATIO_NUDGE)

  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: case R_AND_N_NONLINEAR_CONSTRAINT:
    break; // none to add (r lower bounds = 1)
  case N_MODEL_LINEAR_CONSTRAINT: // lin_ineq #0 is augmented
    for (size_t approx=1; approx<=numApprox; ++approx) {
      lin_ineq_coeffs(approx,  approx-1) = -1.;
      lin_ineq_coeffs(approx, numApprox) =  1. + RATIO_NUDGE; // N_i > N
    }
    break;
  case N_MODEL_LINEAR_OBJECTIVE: // no other lin ineq
    for (size_t approx=0; approx<numApprox; ++approx) {
      lin_ineq_coeffs(approx,    approx) = -1.;
      lin_ineq_coeffs(approx, numApprox) =  1. + RATIO_NUDGE; // N_i > N
    }
    break;
  }
}


Real NonDACVSampling::
augmented_linear_ineq_violations(const RealVector& cd_vars,
				 const RealMatrix& lin_ineq_coeffs,
				 const RealVector& lin_ineq_lb,
				 const RealVector& lin_ineq_ub)
{
  Real quad_viol = 0.;
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_MODEL_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t lin_ineq_offset
      = (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) ? 1 : 0;
    Real viol, inner_prod, l_bnd, u_bnd, N_H = cd_vars[numApprox];
    for (size_t approx=0; approx<numApprox; ++approx) {
      inner_prod
	= lin_ineq_coeffs(approx+lin_ineq_offset, approx)    * cd_vars[approx]
	+ lin_ineq_coeffs(approx+lin_ineq_offset, numApprox) * N_H;
      l_bnd = lin_ineq_lb[approx+lin_ineq_offset];
      u_bnd = lin_ineq_ub[approx+lin_ineq_offset];
      if (inner_prod < l_bnd)
	{ viol = (1. - inner_prod / l_bnd);  quad_viol += viol*viol; }
      else if (inner_prod > u_bnd)
	{ viol = (inner_prod / u_bnd - 1.);  quad_viol += viol*viol; }
    }
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT: case R_AND_N_NONLINEAR_CONSTRAINT:
    break; // none to add (r lower bounds = 1)
  }
  return quad_viol;
}


Real NonDACVSampling::
update_hf_target(const RealVector& avg_eval_ratios, Real avg_N_H,
		 const RealVector& var_H, const RealVector& estvar0)
{
  // Note: there is a circular dependency between estvar_ratios and hf_targets

  RealVector cd_vars, estvar_ratios;
  r_and_N_to_design_vars(avg_eval_ratios, avg_N_H, cd_vars);
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual for ACV,GenACV

  return NonDNonHierarchSampling::
    update_hf_target(estvar_ratios, var_H, estvar0);
}


/** Multi-moment map-based version used by ACV following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    IntRealMatrixMap&         sum_LH, // each L with H
		    RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;  bool all_finite;
  Real lf_fn, lf2_fn, hf_fn, lf_prod, lf2_prod, hf_prod;
  IntRespMCIter r_it;              IntRVMIter    h_it;
  IntRMMIter lb_it, lr_it, lh_it;  IntRSMAMIter ll_it;
  int lb_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_finite = true;
      for (approx=0; approx<=numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	if ((asv[lf_index] & 1) && !isfinite(fn_vals[lf_index]))//active NaN/Inf
	  { all_finite = false; break; }
      }
      if (!all_finite) continue;

      hf_index = numApprox * numFunctions + qoi;
      if (asv[hf_index] & 1) {
	++N_shared[qoi];
	// High accumulations:
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
	  if (asv[lf_index] & 1) {
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
		RealSymMatrix& sum_LL_q = ll_it->second[qoi];
		sum_LL_q(approx,approx) += lf_prod * lf_prod;
		// Off-diagonal of C matrix:
		// look back (only) for single capture of each combination
		for (approx2=0; approx2<approx; ++approx2) {
		  lf2_index = approx2 * numFunctions + qoi;
		  if (asv[lf2_index] & 1) {
		    lf2_prod = lf2_fn = fn_vals[lf2_index];
		    for (m=1; m<active_ord; ++m)
		      lf2_prod *= lf2_fn;
		    sum_LL_q(approx,approx2) += lf_prod * lf2_prod;
		  }
		}
		++ll_it; ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	      }
	      // Low-High (c vector for each QoI):
	      if (lh_ord == active_ord) {
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
}


/** Single moment version used by offline-pilot and pilot-projection ACV
    following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
		    RealSymMatrixArray& sum_LL, // L w/ itself + other L
		    RealMatrix&         sum_LH, // each L with H
		    RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;  bool all_finite;
  Real lf_fn, hf_fn;
  IntRespMCIter r_it;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_finite = true;
      for (approx=0; approx<=numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	if ((asv[lf_index] & 1) && !isfinite(fn_vals[lf_index]))//active NaN/Inf
	  { all_finite = false; break; }
      }
      if (!all_finite) continue;
      
      // High accumulations:
      hf_index = numApprox * numFunctions + qoi;
      if (asv[hf_index] & 1) {
	++N_shared[qoi];
	hf_fn = fn_vals[hf_index];
	sum_H[qoi]  += hf_fn;         // High
	sum_HH[qoi] += hf_fn * hf_fn; // High-High

	RealSymMatrix& sum_LL_q = sum_LL[qoi];
	for (approx=0; approx<numApprox; ++approx) {
	  lf_index = approx * numFunctions + qoi;
	  if (asv[lf_index] & 1) {
	    lf_fn = fn_vals[lf_index];

	    // Low accumulations:
	    sum_L_baseline(qoi,approx) += lf_fn; // Low
	    sum_LL_q(approx,approx)    += lf_fn * lf_fn; // Low-Low
	    // Off-diagonal of C matrix:
	    // look back (only) for single capture of each combination
	    for (approx2=0; approx2<approx; ++approx2) {
	      lf2_index = approx2 * numFunctions + qoi;
	      if (asv[lf2_index] & 1)
		sum_LL_q(approx,approx2) += lf_fn * fn_vals[lf2_index];
	    }
	    // Low-High (c vector)
	    sum_LH(qoi,approx) += lf_fn * hf_fn;
	  }
	}
      }
    }
  }
}


/** Multi-moment map-based version with fine-grained fault tolerance, 
    used by ACV following shared_increment()
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    IntRealMatrixMap&         sum_LH, // each L with H
		    RealVector& sum_HH, Sizet2DArray& num_L_baseline,
		    SizetArray& num_H,  SizetSymMatrixArray& num_LL,
		    Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, lf2_fn, hf_fn, lf_prod, lf2_prod, hf_prod;
  IntRespMCIter r_it;              IntRVMIter    h_it;
  IntRMMIter lb_it, lr_it, lh_it;  IntRSMAMIter ll_it;
  int lb_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
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
	
      SizetSymMatrix& num_LL_q = num_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_baseline[approx][qoi];
	  ++num_LL_q(approx,approx); // Diagonal of C matrix
	  if (hf_is_finite) ++num_LH[approx][qoi]; // pull out of moment loop

	  lb_it = sum_L_baseline.begin();
	  ll_it = sum_LL.begin();  lh_it = sum_LH.begin();
	  lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	  ll_ord = (ll_it == sum_LL.end())         ? 0 : ll_it->first;
	  lh_ord = (lh_it == sum_LH.end())         ? 0 : lh_it->first;
	  lf_prod = lf_fn;  active_ord = 1;
	  while (lb_ord || ll_ord || lh_ord) {
    
	    // Low baseline
	    if (lb_ord == active_ord) { // support general key sequence
	      lb_it->second(qoi,approx) += lf_prod;  ++lb_it;
	      lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      ll_it->second[qoi](approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_fn = fn_vals[lf2_index];

		if (isfinite(lf2_fn)) { // both are finite
		  if (active_ord == 1) ++num_LL_q(approx,approx2);
		  lf2_prod = lf2_fn;
		  for (m=1; m<active_ord; ++m)
		    lf2_prod *= lf2_fn;
		  ll_it->second[qoi](approx,approx2) += lf_prod * lf2_prod;
		}
	      }
	      ++ll_it; ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High (c vector for each QoI):
	    if (lh_ord == active_ord) {
	      if (hf_is_finite) {
		hf_prod = hf_fn;
		for (m=1; m<active_ord; ++m)
		  hf_prod *= hf_fn;
		lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      }
	      ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


// Single moment version with fine-grained fault tolerance, used by 
// offline-pilot and pilot-projection ACV following shared_increment()
void NonDACVSampling::
accumulate_acv_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
		    RealSymMatrixArray& sum_LL, // L w/ itself + other L
		    RealMatrix&         sum_LH, // each L with H
		    RealVector& sum_HH, Sizet2DArray& num_L_baseline,
		    SizetArray& num_H,  SizetSymMatrixArray& num_LL,
		    Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, lf2_fn, hf_fn;
  IntRespMCIter r_it;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
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
	
      SizetSymMatrix& num_LL_q = num_LL[qoi];
      RealSymMatrix&  sum_LL_q = sum_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_baseline[approx][qoi];
	  sum_L_baseline(qoi,approx) += lf_fn; // Low

	  ++num_LL_q(approx,approx); // Diagonal of C matrix
	  sum_LL_q(approx,approx) += lf_fn * lf_fn; // Low-Low
	  // Off-diagonal of C matrix:
	  // look back (only) for single capture of each combination
	  for (approx2=0; approx2<approx; ++approx2) {
	    lf2_index = approx2 * numFunctions + qoi;
	    lf2_fn = fn_vals[lf2_index];
	    if (isfinite(lf2_fn)) { // both are finite
	       ++num_LL_q(approx,approx2);
	       sum_LL_q(approx,approx2) += lf_fn * lf2_fn;
	    }
	  }

	  if (hf_is_finite) {
	    ++num_LH[approx][qoi];
	    sum_LH(qoi,approx) += lf_fn * hf_fn;// Low-High (c vector)	    
	  }
	}
      }
    }
  }
}
*/


/** This version used by ACV following shared_approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    Sizet2DArray& N_L_shared)
{
  // uses one set of allResponses with QoI aggregation across all approx Models,
  // corresponding to unorderedModels[i-1], i=1:numApprox (omits truthModel)

  using std::isfinite;  bool all_lf_finite;
  Real lf_fn, lf2_fn, lf_prod, lf2_prod;
  IntRespMCIter r_it; IntRMMIter ls_it; IntRSMAMIter ll_it;
  int ls_ord, ll_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_lf_finite = true;
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	if ((asv[lf_index] & 1) && !isfinite(fn_vals[lf_index]))//active NaN/Inf
	  { all_lf_finite = false; break; }
      }
      if (!all_lf_finite) continue;

      // Low accumulations:
      ++N_L_shared[approx][qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	if (asv[lf_index] & 1) {
	  lf_fn = fn_vals[lf_index];

	  ls_it = sum_L_shared.begin();  ll_it = sum_LL.begin();
	  ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	  ll_ord = (ll_it == sum_LL.end())       ? 0 : ll_it->first;
	  lf_prod = lf_fn;  active_ord = 1;
	  while (ls_ord || ll_ord) {
    
	    // Low shared
	    if (ls_ord == active_ord) { // support general key sequence
	      ls_it->second(qoi,approx) += lf_prod;  ++ls_it;
	      ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      RealSymMatrix& sum_LL_q = ll_it->second[qoi];
	      sum_LL_q(approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		if (asv[lf2_index] & 1) {
		  lf2_prod = lf2_fn = fn_vals[lf2_index];
		  for (m=1; m<active_ord; ++m)
		    lf2_prod *= lf2_fn;
		  sum_LL_q(approx,approx2) += lf_prod * lf2_prod;
		}
	      }
	      ++ll_it;  ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** This version used by ACV, GenACV following approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L, Sizet2DArray& N_L_actual,
		    const RealVector& fn_vals, const ShortArray& asv,
		    size_t approx)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  size_t qoi, lf_index;  Real lf_fn;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    lf_index = approx * numFunctions + qoi;
    lf_fn    = fn_vals[lf_index];

    // Low accumulations:
    if ( (asv[lf_index] & 1) && isfinite(lf_fn) ) {
      ++N_L_actual[approx][qoi];
      IntRMMIter lr_it = sum_L.begin();
      int  lr_ord  = (lr_it == sum_L.end()) ? 0 : lr_it->first;
      Real lf_prod = lf_fn;  int active_ord = 1;
      while (lr_ord) {

	// Low refined
	if (lr_ord == active_ord) { // support general key sequence
	  lr_it->second(qoi,approx) += lf_prod;  ++lr_it;
	  lr_ord = (lr_it == sum_L.end()) ? 0 : lr_it->first;
	}

	lf_prod *= lf_fn;  ++active_ord;
      }
    }
  }
}


/** This version used by ACV following approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_refined, Sizet2DArray& N_L_refined,
		    const SizetArray& approx_sequence, size_t sequence_start,
		    size_t sequence_end)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  size_t s, approx;  IntRespMCIter r_it;
  bool ordered = approx_sequence.empty();
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (s=sequence_start; s<sequence_end; ++s) {
      approx = (ordered) ? s : approx_sequence[s];
      accumulate_acv_sums(sum_L_refined, N_L_refined, fn_vals, asv, approx);
    }
  }
}


void NonDACVSampling::
compute_LH_covariance(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		      const RealMatrix& sum_LH, const SizetArray& N_shared,
		      RealMatrix& cov_LH)
{
  if (cov_LH.empty()) cov_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    Real*             cov_LH_a =       cov_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_covariance(sum_L_shared_a[qoi], sum_H[qoi], sum_LH_a[qoi],
			 N_shared[qoi], cov_LH_a[qoi]);
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "cov_LH in compute_LH_covariance():\n" << cov_LH << std::endl;
}


void NonDACVSampling::
compute_LL_covariance(const RealMatrix& sum_L_shared,
		      const RealSymMatrixArray& sum_LL,
		      const SizetArray& N_shared,//SizetSymMatrixArray& N_LL,
		      RealSymMatrixArray& cov_LL)
{
  size_t qoi, approx, approx2;
  if (cov_LL.empty()) {
    cov_LL.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi].shapeUninitialized(numApprox);
  }

  Real sum_L_aq;  size_t N_sh_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& sum_LL_q = sum_LL[qoi];
    RealSymMatrix&       cov_LL_q = cov_LL[qoi];
    N_sh_q = N_shared[qoi]; //const SizetSymMatrix&  N_LL_q = N_LL[qoi];
    for (approx=0; approx<numApprox; ++approx) {
      sum_L_aq = sum_L_shared(qoi,approx);
      for (approx2=0; approx2<=approx; ++approx2)
	compute_covariance(sum_L_aq, sum_L_shared(qoi,approx2),
			   sum_LL_q(approx,approx2), N_sh_q,
			   cov_LL_q(approx,approx2));
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "cov_LL in compute_LL_covariance():\n" << cov_LL << std::endl;
}


void NonDACVSampling::
acv_raw_moments(IntRealMatrixMap& sum_L_baseline,
		IntRealMatrixMap& sum_L_refined,   IntRealVectorMap& sum_H,
		IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		const RealVector& avg_eval_ratios, const SizetArray& N_shared,
		const Sizet2DArray& N_L_refined,   RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(4, numFunctions);

  precompute_acv_control(avg_eval_ratios, N_shared);

  size_t approx, qoi, N_shared_q;  Real sum_H_mq;
  RealVector beta(numApprox);
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix&     sum_L_base_m = sum_L_baseline[mom];
    RealMatrix&      sum_L_ref_m = sum_L_refined[mom];
    RealVector&          sum_H_m =         sum_H[mom];
    RealSymMatrixArray& sum_LL_m =        sum_LL[mom];
    RealMatrix&         sum_LH_m =        sum_LH[mom];
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      sum_H_mq = sum_H_m[qoi];  N_shared_q = N_shared[qoi];
      compute_acv_control(sum_L_base_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			  N_shared_q, mom, qoi, beta);
      // *** TO DO: support shared_approx_increment() --> baselineL

      Real& H_raw_mq = H_raw_mom(mom-1, qoi);
      H_raw_mq = sum_H_mq / N_shared_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1 << ": control "
	       << "variate beta = " << std::setw(9) << beta[approx] << '\n';
	// For ACV, shared counts are fixed at N_H for all approx
	apply_control(sum_L_base_m(qoi,approx), N_shared_q,
		      sum_L_ref_m(qoi,approx),  N_L_refined[approx][qoi],
		      beta[approx], H_raw_mq);
      }
    }
  }
  if (outputLevel >= NORMAL_OUTPUT) Cout << std::endl;
}


/** LF only */
void NonDACVSampling::
update_projected_lf_samples(const MFSolutionData& soln,
			    const UShortArray& approx_set,
			    const SizetArray& N_H_actual, size_t& N_H_alloc,
			    //SizetArray& delta_N_L_actual,
			    Real& delta_equiv_hf)
{
  // pilot+iterated samples shared by all approx, not just final best set
  Sizet2DArray N_L_actual;  SizetArray N_L_alloc;
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    // shared online sampling spans active model set after processing of
    // covariance data assembled offline
    inflate(N_H_actual, N_L_actual, approx_set);
    inflate(N_H_alloc,  N_L_alloc,  approx_set);
  }
  else // shared sampling spans all models as covariances are assembled online
    { inflate(N_H_actual, N_L_actual);  inflate(N_H_alloc, N_L_alloc); }

  size_t i, num_approx = approx_set.size(), alloc_incr, actual_incr;
  unsigned short inflate_i;  Real lf_target;
  const RealVector& soln_vars = soln.solution_variables();
  for (i=0; i<num_approx; ++i) {
    lf_target = soln_vars[i];  inflate_i = approx_set[i];
    const SizetArray& N_L_actual_a = N_L_actual[inflate_i];
    size_t&           N_L_alloc_a  =  N_L_alloc[inflate_i];
    // No relaxation for projections
    alloc_incr  = one_sided_delta(N_L_alloc_a, lf_target);
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_L_actual_a), lf_target) : alloc_incr;
    /*delta_N_L_actual[approx] += actual_incr;*/  N_L_alloc_a += alloc_incr;
    increment_equivalent_cost(actual_incr, sequenceCost, inflate_i,
			      delta_equiv_hf);
  }

  finalize_counts(N_L_actual, N_L_alloc);
}


/** LF and HF */
void NonDACVSampling::
update_projected_samples(const MFSolutionData& soln,
			 const UShortArray& approx_set,
			 const SizetArray& N_H_actual, size_t& N_H_alloc,
			 size_t& delta_N_H_actual,
			 /*SizetArray& delta_N_L_actual,*/ Real& delta_equiv_hf)
{
  // The N_L baseline is the shared set PRIOR to delta_N_H --> important for
  // cost incr even if lf_targets is defined robustly (hf_targets * eval_ratios)
  update_projected_lf_samples(soln, approx_set, N_H_actual, N_H_alloc,
			      /*delta_N_L_actual,*/ delta_equiv_hf);

  Real hf_target = soln.solution_reference();
  // No relaxation for projections
  size_t alloc_incr = one_sided_delta(N_H_alloc, hf_target),
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_H_actual), hf_target) : alloc_incr;
  delta_N_H_actual += actual_incr;  N_H_alloc += alloc_incr;
  increment_equivalent_cost(actual_incr, sequenceCost, numApprox,
			    delta_equiv_hf);
}

} // namespace Dakota
