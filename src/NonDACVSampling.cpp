/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
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
NonDACVSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDNonHierarchSampling(problem_db, model)//, multiStartACV(true)
{
  mlmfSubMethod = problem_db.get_ushort("method.sub_method");

  if (maxFunctionEvals == SZ_MAX) // accuracy constraint (convTol)
    optSubProblemForm = N_MODEL_LINEAR_OBJECTIVE;
  else {                          //   budget constraint (maxFunctionEvals)
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
  // defining fullApproxSet allows reuse of functions across ACV and GenACV
  fullApproxSet.resize(numApprox);
  for (size_t i=0; i<numApprox; ++i)
    fullApproxSet[i] = i;

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
    Cerr << "Error: weighted recursive difference is not supported in ACV "
	 << "using a peer DAG\n       (generalized ACV supports optional DAG "
	 << "search or a default hierarchical DAG)." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // bypass for GenACV:
  update_model_groups();
  if (costSource == USER_COST_SPEC)
    update_model_group_costs();

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
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;
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
    shared_increment("acv_"); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += (backfillFailures && mlmfIter) ? one_sided_delta(N_H_alloc,
      acvSolnData.solution_reference(), relaxFactor) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (mlmfIter == 0 && costSource != USER_COST_SPEC)
      { recover_online_cost(allResponses); update_model_group_costs(); }
    increment_equivalent_cost(numSamples, sequenceCost, 0, numGroups,
			      equivHFEvals);
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, varH, covLL, covLH);

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_allocations(acvSolnData);
    ++mlmfIter;  advance_relaxation();
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  Sizet2DArray N_L_actual_refined;  SizetArray N_L_alloc;
  if (finalStatsType == QOI_STATISTICS) {
    // peer DAG: no need for shared arrays since shared == baseline
    IntRealMatrixMap sum_L_refined;//, sum_L_shared;
    Sizet2DArray N_L_actual_baseline; inflate(N_H_actual, N_L_actual_baseline);
    RealVector2DArray beta(4);
    approx_increments(sum_L_baselineH, N_H_actual, N_H_alloc, sum_L_refined,
		      N_L_actual_refined, N_L_alloc, acvSolnData);
    compute_acv_controls(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
			 acvSolnData, beta);
    apply_controls(sum_H, N_H_actual, sum_L_baselineH, N_L_actual_baseline,
		   sum_L_refined, N_L_actual_refined, beta);
    finalize_counts(N_L_actual_refined, N_L_alloc);
  }
  else // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(acvSolnData, fullApproxSet, N_H_actual,
				N_H_alloc, N_L_actual_refined, N_L_alloc,
				deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  IntRealVectorMap sum_H_pilot;  IntRealMatrixMap sum_L_pilot, sum_LH_pilot;
  IntRealSymMatrixArrayMap sum_LL_pilot;  RealVector sum_HH_pilot;
  SizetArray N_pilot;
  evaluate_pilot(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		 sum_HH_pilot, N_pilot, false);
  if (costSource != USER_COST_SPEC) update_model_group_costs();
  compute_LH_statistics(sum_L_pilot[1], sum_H_pilot[1], sum_LL_pilot[1],
			sum_LH_pilot[1], sum_HH_pilot, N_pilot, varH,
			covLL, covLH);

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
  compute_allocations(acvSolnData);
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // QOI_STATISTICS case; ESTIMATOR_PERFORMANCE redirects to _pilot_projection()

  // perform the shared increment for the online sample profile
  shared_increment("acv_"); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_H_actual);//, N_LL);
  N_H_alloc += numSamples;
  increment_equivalent_cost(numSamples, sequenceCost, 0,numGroups,equivHFEvals);

  // perform LF increments for the online sample profile
  // peer DAG: no need for shared arrays since shared == baseline
  IntRealMatrixMap sum_L_refined;  //, sum_L_shared;
  Sizet2DArray N_L_actual_refined, N_L_actual_baseline;
  SizetArray   N_L_alloc;  RealVector2DArray beta(4);
  inflate(N_H_actual, N_L_actual_baseline);
  approx_increments(sum_L_baselineH, N_H_actual, N_H_alloc, sum_L_refined,
		    N_L_actual_refined, N_L_alloc, acvSolnData);
  compute_acv_controls(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		       N_pilot, acvSolnData, beta);
  apply_controls(sum_H, N_H_actual, sum_L_baselineH, N_L_actual_baseline,
		 sum_L_refined, N_L_actual_refined, beta);
  finalize_counts(N_L_actual_refined, N_L_alloc);
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
  RealVector sum_H, sum_HH;  RealMatrix sum_L, sum_LH;
  RealSymMatrixArray sum_LL;
  switch (pilotMgmtMode) {
  case OFFLINE_PILOT: // redirected from _offline_pilot() in core_run()
  case OFFLINE_PILOT_PROJECTION: {
    SizetArray N_pilot;
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_pilot, false);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_pilot, varH,
			  covLL, covLH);
    N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
    break;
  }
  default: // ONLINE_PILOT_PROJECTION
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual, true);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual,
			  varH, covLL, covLH);
    N_H_alloc = numSamples;
    break;
  }
  if (costSource != USER_COST_SPEC) update_model_group_costs();

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_allocations(acvSolnData);
  ++mlmfIter;

  // No LF increments or final moments for pilot projection
  Sizet2DArray N_L_actual;  SizetArray N_L_alloc; // inflated from N_H
  update_projected_samples(acvSolnData, fullApproxSet, N_H_actual, N_H_alloc,
			   N_L_actual, N_L_alloc, deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected estVariance{s,Ratios}
}


void NonDACVSampling::
evaluate_pilot(RealMatrix& sum_L_pilot, RealVector& sum_H_pilot,
	       RealSymMatrixArray& sum_LL_pilot, RealMatrix& sum_LH_pilot,
	       RealVector& sum_HH_pilot, SizetArray& N_pilot,
	       bool incr_cost)
{
  initialize_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot);
  N_pilot.assign(numFunctions, 0);

  shared_increment("acv_"); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_pilot,//_baselineH, sum_L_baselineL,
		      sum_H_pilot, sum_LL_pilot, sum_LH_pilot, sum_HH_pilot,
		      N_pilot);//, N_LL_pilot);

  // TO DO: allow pilot to vary for C vs c.  numSamples logic after pilot
  // (mlmfIter >= 1) will require _baseline{L,H}
  //if (lf_pilot > hf_pilot) {
  //  numSamples = lf_pilot - hf_pilot;
  //  shared_approx_increment("acv_"); // spans all approx models
  //  accumulate_acv_sums(sum_L_baselineL, sum_LL,//_baselineL,
  //                      N_L_baselineL);
  //  if (incr_cost)
  //    increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox,
  //			            equivHFEvals);
  //}

  if (costSource != USER_COST_SPEC) // don't update group costs (GenACV)
    recover_online_cost(allResponses); //update_model_group_costs();
  if (incr_cost)
    increment_equivalent_cost(numSamples,sequenceCost,0,numGroups,equivHFEvals);
}


void NonDACVSampling::
evaluate_pilot(IntRealMatrixMap& sum_L_pilot, IntRealVectorMap& sum_H_pilot,
	       IntRealSymMatrixArrayMap& sum_LL_pilot,
	       IntRealMatrixMap& sum_LH_pilot, RealVector& sum_HH_pilot,
	       SizetArray& N_pilot, bool incr_cost)
{
  initialize_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot);
  N_pilot.assign(numFunctions, 0);

  shared_increment("acv_"); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot, N_pilot);

  if (costSource != USER_COST_SPEC) // don't update group costs (GenACV)
    recover_online_cost(allResponses); //update_model_group_costs();
  if (incr_cost)
    increment_equivalent_cost(numSamples,sequenceCost,0,numGroups,equivHFEvals);
}


void NonDACVSampling::
compute_LH_statistics(RealMatrix& sum_L_pilot, RealVector& sum_H_pilot,
		      RealSymMatrixArray& sum_LL_pilot,
		      RealMatrix& sum_LH_pilot, RealVector& sum_HH_pilot,
		      SizetArray& N_pilot, //RealMatrix& var_L,
		      RealVector& var_H, RealSymMatrixArray& cov_LL,
		      RealMatrix& cov_LH, const UShortArray& approx_set)
{
  //if (mlmfIter == 0) // see var_L usage in compute_allocations()
  //  compute_L_variance(sum_L_pilot, sum_LL_pilot, N_pilot, var_L);
  compute_variance(sum_H_pilot, sum_HH_pilot, N_pilot, var_H);
  compute_LL_covariance(sum_L_pilot,//_baselineL,
                        sum_LL_pilot,//N_LL_pilot,
			N_pilot, cov_LL, approx_set);
  compute_LH_covariance(sum_L_pilot,//baselineH,
			sum_H_pilot, sum_LH_pilot, N_pilot, cov_LH, approx_set);
  //Cout << "var_H:\n"<< var_H << "cov_LL:\n"<< cov_LL << "cov_LH:\n"<< cov_LH;
}


void NonDACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baseline,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  IntRealMatrixMap& sum_L_refined,
		  Sizet2DArray& N_L_actual_refined,
		  SizetArray& N_L_alloc, const MFSolutionData& soln)
{
  // Note: these results do not affect the iteration above and can be
  // performed after N_H has converged

  // Perform a sample sequence that reuses sample increments: define an approx
  // sequence in decreasing r_i order (for overlapping sample sequencing)
  // rather than rho2_LH order (for protecting NaN from sqrt(rho2_diff))
  // > directionally consistent with default approx indexing for well-ordered
  //   models: approx 0 is lowest fidelity --> highest r_i
  SizetArray r_approx_sequence;
  if (mlmfSubMethod == SUBMETHOD_ACV_MF)
    ordered_approx_sequence(soln.solution_ratios(), r_approx_sequence, true);
  update_model_groups(r_approx_sequence);  update_model_group_costs();
  // Important: unlike ML BLUE, modelGroups are only used to facilitate shared
  // sample set groupings in group_increments() and these updates to group
  // definitions do not imply changes to the moment roll-up or peer DAG
  // > upstream use of modelGroupCosts in finite_solution_bounds() is complete
  // > downstream processing is agnostic to modelGroups, consuming the overlaid
  //   {sum,num}_L_{sh,ref}.
  // > If modelGroups are used more broadly in the future, then nested sampling
  //   redefinitions may need to employ group defns local to this function

  // --------------------------------------------
  // Perform approximation increments in parallel
  // --------------------------------------------
  SizetArray delta_N_G(numGroups);
  inflate(N_H_actual, N_L_actual_refined);
  inflate(N_H_alloc,  N_L_alloc);
  const RealVector& soln_vars = soln.solution_variables();
  size_t last_g = numGroups - 1;  delta_N_G[last_g] = 0;
  // Pyramid sampling with reuse: base to top, excluding all-model group.
  // Approx sequencing is now embedded within modelGroups (see top of fn).
  for (int g=last_g-1; g>=0; --g)
    delta_N_G[g]
      = group_approx_increment(soln_vars, fullApproxSet, N_L_actual_refined,
			       N_L_alloc, modelGroups[g]);
  group_increments(delta_N_G, "acv_", true); // reverse order for RNG sequence

  // --------------------------
  // Update sums, counts, costs
  // --------------------------
  increment_equivalent_cost(delta_N_G, modelGroupCost, sequenceCost[numApprox],
			    equivHFEvals);
  IntRealMatrixArrayMap sum_G;  initialize_group_sums(sum_G);
  Sizet2DArray     N_G_actual;  initialize_group_counts(N_G_actual);
  accumulate_group_sums(sum_G, N_G_actual, batchResponsesMap);
  clear_batches();
  // Map from "horizontal" group incr to "vertical" model incr (see JCP: ACV)
  // Note: no shared tracking for peer DAG overlay
  sum_L_refined = sum_L_baseline;
  overlay_peer_group_sums(sum_G, N_G_actual, sum_L_refined, N_L_actual_refined);
}


void NonDACVSampling::
overlay_peer_group_sums(const IntRealMatrixArrayMap& sum_G,
			const Sizet2DArray& N_G_actual,
			IntRealMatrixMap& sum_L_refined,
			Sizet2DArray& N_L_actual_refined)
{
  // omit the last group (all-models) since (i) there is no HF increment
  // (delta_N_G[numApprox] is assigned 0 in approx_increments()) and
  // (ii) any HF refinement would be out of range for L accumulations.  

  // avoid redundant accumulations by augmenting refined starting from shared
  //N_L_actual_refined = N_L_actual_shared; // not in API -> do upstream
  //sum_L_refined      = sum_L_shared;      // not in API -> do upstream

  size_t m, g, num_L_groups = modelGroups.size() - 1, num_models;
  unsigned short model;
  IntRealMatrixArrayMap::const_iterator g_cit;
  IntRealMatrixMap::iterator s_it;
  for (g=0; g<num_L_groups; ++g) {
    const SizetArray& num_G_g = N_G_actual[g];
    if (zeros(num_G_g)) continue; // all-models group has delta = 0

    const UShortArray& group_g = modelGroups[g];
    num_models = group_g.size(); // this index defines refined set
    for (m=0; m<num_models; ++m) {
      model = group_g[m];
      // counters (span all moments):
      increment_samples(N_L_actual_refined[model], num_G_g);
      // accumulators for each moment:
      for (s_it =sum_L_refined.begin(), g_cit =sum_G.begin();
	   s_it!=sum_L_refined.end() && g_cit!=sum_G.end(); ++g_cit, ++s_it)
	increment_sums(s_it->second[model], g_cit->second[g][m], numFunctions);
    }
  }
}


void NonDACVSampling::
compute_acv_controls(const IntRealMatrixMap& sum_L_covar,
		     const IntRealVectorMap& sum_H_covar,
		     const IntRealSymMatrixArrayMap& sum_LL_covar,
		     const IntRealMatrixMap& sum_LH_covar,
		     const SizetArray& N_covar, const MFSolutionData& soln,
		     RealVector2DArray& beta)
{
  // ------------------------------
  // Compute CV to estimate moments
  // ------------------------------
  precompute_acv_controls(soln.solution_ratios());

  size_t qoi;
  for (int mom=1; mom<=4; ++mom) {
    const RealMatrix&          sum_L_m =  sum_L_covar.at(mom);
    const RealVector&          sum_H_m =  sum_H_covar.at(mom);
    const RealSymMatrixArray& sum_LL_m = sum_LL_covar.at(mom);
    const RealMatrix&         sum_LH_m = sum_LH_covar.at(mom);
    RealVectorArray&            beta_m =            beta[mom-1];
    beta_m.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_acv_control(sum_L_m, sum_H_m[qoi], sum_LL_m[qoi], sum_LH_m,
			  N_covar[qoi], mom, qoi, beta_m[qoi]);
      // *** TO DO: support shared_approx_increment() --> baselineL
  }
}


void NonDACVSampling::
solve_for_C_F_c_f(RealSymMatrix& C_F, RealVector& c_f, RealVector& lhs,
		  bool copy_C_F, bool copy_c_f)
{
  // The idea behind this approach is to leverage both the solution refinement
  // in solve() and equilibration during factorization (inverting C_F in place
  // can only leverage the latter).

  size_t n = c_f.length();
  lhs.size(n); // not sure if initialization matters here...

  if (hardenNumericSoln) {
    RealMatrix C_F_inv;  Real rcond;
    pseudo_inverse(C_F, C_F_inv, rcond);
    lhs.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., C_F_inv, c_f, 0.);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "ACV pseudo-inverse solve for LHS:\n" << lhs << "has rcond = "
	   << rcond << std::endl;
  }
  else
    cholesky_solve(C_F, lhs, c_f, copy_C_F, copy_c_f);
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
    { update_model_groups(); return; }

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


void NonDACVSampling::compute_allocations(MFSolutionData& soln)
{
  // Solve the optimization sub-problem using an initial guess from either
  // related analytic solutions (iter == 0) or warm started from the previous
  // solutions (iter >= 1)

  bool no_solve = (maxFunctionEvals != SZ_MAX &&
		   equivHFEvals >= (Real)maxFunctionEvals); // budget exhausted
  if (mlmfIter == 0) {
    if (pilotMgmtMode == ONLINE_PILOT ||
	pilotMgmtMode == ONLINE_PILOT_PROJECTION) {
      cache_mc_reference(); // {estVar,numH}Iter0, estVarMetric0
      if (convergenceTolType == CONVERGENCE_TOLERANCE_TYPE_RELATIVE)
	no_solve = (no_solve || convergenceTol >= 1.);
      else
	no_solve = (no_solve || estVarMetric0  <= convergenceTol);
    }
    // Note: offline pilot can support absolute conv tol during numeric solve,
    // but not in advance for no_solve since estVarMetric0 would be oracle value

    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;

    if (no_solve) { // no need for solve
      // For r_i = 1, C_F,c_f = 0 --> NUDGE for downstream CV numerics
      RealVector avg_eval_ratios(numApprox, false);
      avg_eval_ratios = 1. + RATIO_NUDGE;
      soln.anchored_solution_ratios(avg_eval_ratios, avg_N_H);
      no_solve_variances(soln);  numSamples = 0;  return;
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
      covariance_to_correlation_sq(covLH, covLL, varH, rho2_LH);
      MFSolutionData mf_soln, cv_soln;
      analytic_initialization_from_mfmc(rho2_LH, avg_N_H, mf_soln);
      analytic_initialization_from_ensemble_cvmc(rho2_LH, avg_N_H, cv_soln);
      competed_initial_guesses(mf_soln, cv_soln, soln);
      break;
    }
    }
  }
  else { // subsequent iterations
    if (no_solve) // leave soln at previous values
      { numSamples = 0; return; }
    // warm start from previous solution (for active or one-and-only DAG)
    // > no scaling needed from prev soln (as in NonDLocalReliability) since
    //   updated avg_N_H now includes allocation from previous solution and
    //   should be active on constraint bound (excepting sample count rounding)
    ensemble_numerical_solution(soln);
  }

  process_model_allocations(soln, numSamples);
  if (outputLevel >= NORMAL_OUTPUT)
    print_model_allocations(Cout, soln, fullApproxSet);
}


void NonDACVSampling::
analytic_initialization_from_mfmc(const RealMatrix& rho2_LH, Real avg_N_H,
				  MFSolutionData& soln)
{
  // > Option 1 is analytic MFMC: differs from ACV due to recursive pairing
  RealVector avg_eval_ratios;
  if (ordered_approx_sequence(rho2_LH)) // for all QoI across all Approx
    mfmc_analytic_solution(fullApproxSet, rho2_LH, sequenceCost,
			   avg_eval_ratios);
  else {
    // compute reordered MFMC for averaged rho; monotonic r not required
    // > any rho2_LH re-ordering from MFMC initial guess can be ignored (later
    //   gets replaced with r_i ordering for approx_increments() sampling)
    SizetArray approx_sequence;
    mfmc_reordered_analytic_solution(fullApproxSet, rho2_LH, sequenceCost,
				     approx_sequence, avg_eval_ratios);
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from analytic MFMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  analytic_ratios_to_solution_variables(avg_eval_ratios, avg_N_H, soln);
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

  analytic_ratios_to_solution_variables(avg_eval_ratios, avg_N_H, soln);
}


void NonDACVSampling::
analytic_ratios_to_solution_variables(RealVector& avg_eval_ratios,
				      Real avg_N_H, MFSolutionData& soln)
{
  Real hf_target;
  if (maxFunctionEvals == SZ_MAX) // HF tgt from ACV estvar using analytic soln
    hf_target = update_hf_target(avg_eval_ratios, avg_N_H, varH, estVarIter0);
  else // allocate_budget(), then manage lower bounds and pilot over-estimation
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, hf_target,
		    activeBudget);

  soln.anchored_solution_ratios(avg_eval_ratios, hf_target);
}


void NonDACVSampling::
print_model_allocations(std::ostream& s, const MFSolutionData& soln,
			const UShortArray& approx_set)
{
  const RealVector& soln_vars = soln.solution_variables();
  size_t i, num_approx = approx_set.size();
  for (i=0; i<num_approx; ++i)
    s << "Approx " << approx_set[i] + 1 << ": sample allocation = "
      << soln_vars[i] << '\n';
  s << "Truth:   sample allocation = " << soln_vars[num_approx] << '\n';
  if (maxFunctionEvals == SZ_MAX)
    s << "Estimator cost allocation = " << soln.equivalent_hf_allocation()
      << std::endl;
  else
    s << "Estimator variance metric = " << soln.estimator_variance_metric()
      << std::endl;
}


Real NonDACVSampling::
update_hf_target(const RealVector& avg_eval_ratios, Real avg_N_H,
		 const RealVector& var_H, const RealVector& estvar0)
{
  RealVector cd_vars, estvar_ratios, estvar;
  r_and_N_to_design_vars(avg_eval_ratios, avg_N_H, cd_vars);
  estimator_variances_and_ratios(cd_vars, estvar_ratios, estvar);

  Real metric;  size_t metric_index;
  MFSolutionData::update_estimator_variance_metric(estVarMetricType,
    estVarMetricNormOrder, estvar_ratios, estvar, metric, metric_index);

  return NonDNonHierarchSampling::
    update_hf_target(estvar_ratios, metric_index, var_H, estvar0);
}


/** Multi-moment map-based version used by ACV following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    IntRealMatrixMap&         sum_LH, // each L with H
		    RealVector& sum_HH, SizetArray& N_H_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  IntRespMCIter r_it; 
  size_t qoi, approx, lf_index, hf_index, num_am1 = numApprox+1;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      if (!check_finite(fn_vals, asv, qoi, num_am1)) continue;

      hf_index = numApprox * numFunctions + qoi;
      if (asv[hf_index] & 1) {
	++N_H_shared[qoi];
	accumulate_hf_qoi(fn_vals, qoi, sum_H, sum_HH);

	for (approx=0; approx<numApprox; ++approx) {
	  // for ACV with all models active, don't additionally increment N_L
	  // since redundant of N_H
	  lf_index = approx * numFunctions + qoi;
	  if (asv[lf_index] & 1)
	    accumulate_lf_hf_qoi(fn_vals, asv, qoi, approx, sum_L_shared,
				 sum_LL, sum_LH);
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

  Real lf_fn, hf_fn;
  IntRespMCIter r_it;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index,
    num_am1 = numApprox+1;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      if (!check_finite(fn_vals, asv, qoi, num_am1)) continue;
      
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


/*
// This version used by ACV following shared_approx_increment()
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    Sizet2DArray& N_L_shared)
{
  // uses one set of allResponses with QoI aggregation across all approx Models,
  // corresponding to unorderedModels[i-1], i=1:numApprox (omits truthModel)

  IntRespMCIter r_it;  size_t qoi, approx, lf_index;
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      if (!check_finite(fn_vals, asv, qoi, numApprox)) continue;

      // Low accumulations:
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	if (asv[lf_index] & 1) {
	  ++N_L_shared[approx][qoi];
	  accumulate_lf_qoi(fn_vals, asv, qoi, approx, sum_L_shared, sum_LL);
	}
      }
    }
  }
}


// This version used by ACV following approx_increment()
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
*/


/** This version used by ACV, GenACV following approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L, Sizet2DArray& N_L_actual,
		    const RealVector& fn_vals, const ShortArray& asv,
		    size_t approx)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  size_t qoi, lf_index;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    lf_index = approx * numFunctions + qoi;
    if ( (asv[lf_index] & 1) && std::isfinite(fn_vals[lf_index]) ) {
      ++N_L_actual[approx][qoi];
      accumulate_lf_qoi(fn_vals, qoi, approx, sum_L);
    }
  }
}


void NonDACVSampling::
accumulate_hf_qoi(const RealVector& fn_vals, size_t qoi,
		  IntRealVectorMap& sum_H, RealVector& sum_HH)
{
  size_t hf_index = numApprox * numFunctions + qoi;
  Real hf_fn = fn_vals[hf_index], hf_prod = hf_fn;

  // High-High:
  sum_HH[qoi] += hf_fn * hf_fn; // a single vector for ord 1
  // High:
  IntRVMIter h_it = sum_H.begin();
  int active_ord = 1, h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
  while (h_ord) {
    if (h_ord == active_ord) { // support general key sequence
      h_it->second[qoi] += hf_prod;
      ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
    }
    hf_prod *= hf_fn;  ++active_ord;
  }
}


void NonDACVSampling::
accumulate_lf_qoi(const RealVector& fn_vals, size_t qoi, size_t approx,
		  IntRealMatrixMap& sum_L)
{
  size_t lf_index = approx * numFunctions + qoi;
  Real lf_fn = fn_vals[lf_index], lf_prod = lf_fn;
  IntRMMIter l_it = sum_L.begin();
  int l_ord = (l_it == sum_L.end()) ? 0 : l_it->first, active_ord = 1;
  while (l_ord) {
    if (l_ord == active_ord) { // support general key sequence
      l_it->second(qoi,approx) += lf_prod;  ++l_it;
      l_ord = (l_it == sum_L.end()) ? 0 : l_it->first;
    }
    lf_prod *= lf_fn;  ++active_ord;
  }
}


void NonDACVSampling::
accumulate_lf_qoi(const RealVector& fn_vals, const ShortArray& asv,
		  size_t qoi, size_t approx, IntRealMatrixMap& sum_L_shared,
		  IntRealSymMatrixArrayMap& sum_LL)
{
  size_t lf_index = approx * numFunctions + qoi, approx2, lf2_index, m;

  IntRMMIter ls_it = sum_L_shared.begin(); IntRSMAMIter ll_it = sum_LL.begin();
  int ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first,
      ll_ord = (ll_it == sum_LL.end())       ? 0 : ll_it->first,
      active_ord = 1;
  Real lf_fn = fn_vals[lf_index], lf_prod = lf_fn, lf2_fn, lf2_prod;
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


void NonDACVSampling::
accumulate_lf_hf_qoi(const RealVector& fn_vals, const ShortArray& asv,
		     size_t qoi, size_t approx, IntRealMatrixMap& sum_L_shared,
		     IntRealSymMatrixArrayMap& sum_LL, IntRealMatrixMap& sum_LH)
{
  size_t hf_index = numApprox * numFunctions + qoi,
         lf_index =    approx * numFunctions + qoi, approx2, lf2_index, m;

  IntRMMIter ls_it = sum_L_shared.begin(), lh_it = sum_LH.begin();
  IntRSMAMIter ll_it = sum_LL.begin();
  int ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first,
      ll_ord = (ll_it == sum_LL.end())       ? 0 : ll_it->first,
      lh_ord = (lh_it == sum_LH.end())       ? 0 : lh_it->first,
      active_ord = 1;
  Real lf_fn = fn_vals[lf_index], lf_prod = lf_fn, lf2_fn, lf2_prod,
       hf_fn = fn_vals[hf_index], hf_prod = hf_fn;
  while (ls_ord || ll_ord || lh_ord) {
    
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


void NonDACVSampling::
compute_LH_covariance(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		      const RealMatrix& sum_LH, const SizetArray& N_shared,
		      RealMatrix& cov_LH, const UShortArray& approx_set)
{
  if (cov_LH.numRows() != numFunctions) cov_LH.shape(numFunctions, numApprox);
  else                                  cov_LH = 0.;

  size_t i, approx, num_approx = approx_set.size(), qoi;
  for (i=0; i<num_approx; ++i) {
    approx = approx_set[i];
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
		      RealSymMatrixArray& cov_LL, const UShortArray& approx_set)
{
  size_t qoi, i, j, approx, approx2;
  if (cov_LL.size() != numFunctions) {
    cov_LL.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi].shape(numApprox); // leave at full size, but inactive = 0
  }
  else
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi] = 0.;

  Real sum_L_aq;  size_t N_sh_q, num_approx = approx_set.size();
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& sum_LL_q = sum_LL[qoi];
    RealSymMatrix&       cov_LL_q = cov_LL[qoi];
    N_sh_q = N_shared[qoi]; //const SizetSymMatrix&  N_LL_q = N_LL[qoi];
    for (i=0; i<num_approx; ++i) {
      approx = approx_set[i];
      sum_L_aq = sum_L_shared(qoi,approx);
      for (j=0; j<=i; ++j) {
	approx2 = approx_set[j];
	compute_covariance(sum_L_aq, sum_L_shared(qoi,approx2),
			   sum_LL_q(approx,approx2), N_sh_q,
			   cov_LL_q(approx,approx2));
      }
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "cov_LL in compute_LL_covariance():\n" << cov_LL << std::endl;
}


/** LF and HF */
void NonDACVSampling::
update_projected_samples(const MFSolutionData& soln,
			 const UShortArray& approx_set,
			 const SizetArray& N_H_actual, size_t& N_H_alloc,
			 Sizet2DArray& N_L_actual, SizetArray& N_L_alloc,
			 size_t& delta_N_H_actual,
			 //SizetArray& delta_N_L_actual,
			 Real& delta_equiv_hf)
{
  // The N_L baseline is the shared set PRIOR to delta_N_H --> important for
  // cost incr even if lf_targets is defined robustly (hf_targets * eval_ratios)
  update_projected_lf_samples(soln, approx_set, N_H_actual, N_H_alloc,
			      N_L_actual, N_L_alloc, //delta_N_L_actual,
			      delta_equiv_hf);

  Real hf_target = soln.solution_reference();
  // No relaxation for projections
  size_t alloc_incr = one_sided_delta(N_H_alloc, hf_target),
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_H_actual), hf_target) : alloc_incr;
  delta_N_H_actual += actual_incr;  N_H_alloc += alloc_incr;
  increment_equivalent_cost(actual_incr, sequenceCost, numApprox,
			    delta_equiv_hf);
}


/** LF only */
void NonDACVSampling::
update_projected_lf_samples(const MFSolutionData& soln,
			    const UShortArray& approx_set,
			    const SizetArray& N_H_actual, size_t  N_H_alloc,
			    Sizet2DArray& N_L_actual, SizetArray& N_L_alloc,
			    //SizetArray& delta_N_L_actual,
			    Real& delta_equiv_hf)
{
  // inflate if needed
  inflate_lf_samples(approx_set, N_H_actual, N_H_alloc, N_L_actual, N_L_alloc);

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


void NonDACVSampling::
inflate_lf_samples(const UShortArray& approx_set,
		   const SizetArray& N_H_actual, size_t      N_H_alloc,
		   Sizet2DArray&     N_L_actual, SizetArray& N_L_alloc)
{
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    // after processing of offline covariance data, online shared_increment()
    // which follows only includes best model set 
    if (N_L_actual.empty()) inflate(N_H_actual, N_L_actual, approx_set);
    if (N_L_alloc.empty())  inflate(N_H_alloc,  N_L_alloc,  approx_set);
  }
  else {
    // online: all models are part of initial shared_increment(), which can
    // then be updated when there is GenACV iteration of the active set.
    // > when iteration is not used, N_L counts do not need to be managed
    //   separately and can be inflated here from N_H.
    if (N_L_actual.empty()) inflate(N_H_actual, N_L_actual);
    if (N_L_alloc.empty())  inflate(N_H_alloc,  N_L_alloc);
  }
}

} // namespace Dakota
