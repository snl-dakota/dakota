/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevBLUESampling
//- Description: Implementation code for NonDMultilevBLUESampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevBLUESampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

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
    optSubProblemForm = BLUE_LINEAR_OBJECTIVE;
  else                     // budget constraint (maxFunctionEvals)
    optSubProblemForm =//(truthFixedByPilot && pilotMgmtMode != OFFLINE_PILOT) ?
      //BLUE_R_ONLY_LINEAR_CONSTRAINT :
      BLUE_LINEAR_CONSTRAINT;

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
  UShort2DArray tp;  UShortArray tp_orders(numApprox, 1);
  Pecos::SharedPolyApproxData::
    tensor_product_multi_index(tp_orders, modelGroups, true);
  modelGroups.erase(modelGroups.begin()); // discard empty group (all 0's)

  size_t group, num_groups = modelGroups.size(), m, num_models;
  modelGroupCost.sizeUninitialized(num_groups);
  for (group=0; group<num_groups; ++group) {
    const UShortArray& models = modelGroups[group];
    Real& group_cost = modelGroupCost[group];
    group_cost = 0.; num_models = models.size();
    for (m=0; m<num_models; ++m)
      group_cost += cost[models[m]];
  }
}


NonDMultilevBLUESampling::~NonDMultilevBLUESampling()
{ }


void NonDMultilevBLUESampling::core_run()
{
  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

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

  //blueSolnData.avgHFTarget = 0.;
  // ***************************************************************************
  // *** TO DO: do we stick with shared_increment based on the complete group?
  //     (I think yes.) --> review initial covariance set...
  // *** Another option is to retire online iteration, but avoid if possible.
  // ***************************************************************************
  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += (backfillFailures && mlmfIter) ? one_sided_delta(N_H_alloc,
      blueSolnData.solution_reference()) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (onlineCost && mlmfIter == 0) recover_online_cost(sequenceCost);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,
			      equivHFEvals);
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, var_L, varH, covLL, covLH);

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and BLUE mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, blueSolnData); // ***
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, blueSolnData); // ***
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_group/*lf*/_samples(blueSolnData, N_H_actual, N_H_alloc,
					 deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  RealVector sum_H_pilot, sum_HH_pilot;
  RealMatrix sum_L_pilot, sum_LH_pilot, var_L;
  RealSymMatrixArray sum_LL_pilot;  SizetArray N_shared_pilot;
  evaluate_pilot(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		 sum_HH_pilot, N_shared_pilot, false);
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
  compute_ratios(var_L, blueSolnData); // ***
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    // perform the shared increment for the online sample profile
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += numSamples;
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,
			      equivHFEvals);
    // perform LF increments for the online sample profile
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, blueSolnData); // ***
  }
  else // project online profile including both shared samples and LF increment
    update_projected_group_samples(blueSolnData, N_H_actual, N_H_alloc,
				   deltaNActualHF, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_pilot_projection()
{
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];

  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealVector sum_H, sum_HH;  RealMatrix sum_L_baselineH, sum_LH, var_L;
  RealSymMatrixArray sum_LL;
  evaluate_pilot(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH,
		 N_H_actual, true);
  compute_LH_statistics(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH,
			N_H_actual, var_L, varH, covLL, covLH);
  N_H_alloc = numSamples;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(var_L, blueSolnData); // ***
  ++mlmfIter;

  // No LF increments or final moments for pilot projection
  update_projected_group_samples(blueSolnData, N_H_actual, N_H_alloc,
				 deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDMultilevBLUESampling::
numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
			  size_t& num_nln_con)
{
  num_cdv = modelGroups.size();

  //switch (optSubProblemSolver) {
  //case SUBMETHOD_SDP:
    switch (optSubProblemForm) {
    case BLUE_LINEAR_CONSTRAINT:
      num_lin_con = (pilotMgmtMode == OFFLINE_PILOT) ? 2 : 1;
      num_nln_con = 0;   break;
    case BLUE_LINEAR_OBJECTIVE:
      num_lin_con = (pilotMgmtMode == OFFLINE_PILOT) ? 1 : 0;
      num_nln_con = 1;  break;
    }
  /*
    break;
  default:
    switch (optSubProblemForm) {
    case BLUE_LINEAR_CONSTRAINT:
      num_lin_con = 1;  num_nln_con = 0;   break;
    case BLUE_LINEAR_OBJECTIVE:
      num_nln_con = 1;  num_lin_con = numApprox;  break;
    }
    break;
  }
  */
}


void NonDMultilevBLUESampling::
numerical_solution_bounds_constraints(const MFSolutionData& soln,
				      const RealVector& cost, Real avg_N_H,
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

  size_t group, num_groups = x0.length(), last_index = num_groups-1, lin_offset;
  const RealVector& soln_vars = soln.solution_variables();
  //Real offline_N_lwr = 2.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;

  // minimizer-specific updates (x bounds) performed in finite_solution_bounds()
  x_lb = 0.; // no lower bound for most groups (*** TO DO: NUDGE if not SDP?)
  x_ub = DBL_MAX; // no upper bounds for groups
  if (pilotMgmtMode == OFFLINE_PILOT) {
    lin_ineq_lb[0] = 1.; // at least 1 sample for any group
    lin_ineq_ub[0] = DBL_MAX;
    for (group=0; group<num_groups; ++group)
      lin_ineq_coeffs(0, group) = 1.;
    lin_offset = 1;
  }
  // assign sunk pilot cost to full group and optimize w/ this as a constraint
  else {
    x_lb[last_index] = avg_N_H;
    lin_offset = 0;
  }

  if (soln_vars.empty()) x0 = x_lb;
  else {
    x0 = soln_vars;
    if (pilotMgmtMode == OFFLINE_PILOT) // x0 could undershoot x_lb
      for (i=0; i<num_groups; ++i) // bump x0 to satisfy x_lb if needed
	if (x0[i] < x_lb[i])
	  x0[i] = x_lb[i];
  }

  switch (optSubProblemForm) {
  case BLUE_LINEAR_CONSTRAINT: { // linear inequality constraint on budget:
    // \Sum_grp_i w_grp_i        N_grp_i <= equiv_HF * w_HF
    // \Sum_grp_i w_grp_i / w_HF N_grp_i <= equivHF
    Real cost_H = cost[numApprox];
    lin_ineq_lb[lin_offset] = -DBL_MAX; // no lb
    lin_ineq_ub[lin_offset] = (Real)maxFunctionEvals;//budget;
    for (group=0; group<num_groups; ++group)
      lin_ineq_coeffs(lin_offset, group) = modelGroupCost[group] / cost_H;
    break;
  }
  case BLUE_LINEAR_OBJECTIVE: // nonlinear accuracy constraint: ub on estvar
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
compute_ratios(const RealMatrix& var_L, MFSolutionData& soln)
{
  // Solve the optimization sub-problem using an initial guess from either
  // related analytic solutions (iter == 0) or warm started from the previous
  // solutions (iter >= 1)

  if (mlmfIter == 0) {
    if (pilotMgmtMode != OFFLINE_PILOT) cache_mc_reference();

    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
    bool budget_constrained = (maxFunctionEvals != SZ_MAX);
    bool budget_exhausted
      = (budget_constrained && equivHFEvals >= (Real)maxFunctionEvals);

    if (budget_exhausted || convergenceTol >= 1.) { // no need for solve
      // For r_i = 1, C_F,c_f = 0 --> NUDGE for downstream CV numerics
      RealVector soln_vars(numApprox+1); // init to 0
      soln_vars[numApprox] = avg_N_H; // last group = all models
      soln.solution_variables(soln_vars);
      // For offline pilot, the online EstVar is undefined prior to any online
      // samples, but should not happen (no budget used) unless bad convTol spec
      if (pilotMgmtMode == OFFLINE_PILOT)
	soln.average_estimator_variance(std::numeric_limits<Real>::infinity());
      else
	soln.average_estimator_variance(average(estVarIter0));
      soln.average_estimator_variance_ratio(1.);
      numSamples = 0;  return;
    }

    // Run a competition among related analytic approaches (MFMC or pairwise
    // CVMC) for best initial guess, where each initial gues may additionally
    // employ multiple varianceMinimizers in ensemble_numerical_solution()
    switch (optSubProblemSolver) { // no initial guess
    // global and sequenced global+local methods:
    case SUBMETHOD_DIRECT_NPSOL_OPTPP:  case SUBMETHOD_DIRECT_NPSOL:
    case SUBMETHOD_DIRECT_OPTPP:        case SUBMETHOD_DIRECT:
    case SUBMETHOD_EGO:  case SUBMETHOD_SBGO:  case SUBMETHOD_EA:
      ensemble_numerical_solution(sequenceCost, soln, numSamples);
      break;
    default: { // competed initial guesses with (competed) local methods
      covariance_to_correlation_sq(covLH, var_L, varH, rho2LH);
      // use pyramid sample definitions for hierarchical groups as init guess,
      // even though they are not independent sample sets in MFMC
      analytic_initialization_from_mfmc(avg_N_H, soln);
      ensemble_numerical_solution(sequenceCost, soln, numSamples);
      break;
    }
    }
  }
  else { // warm start from previous solution (for active or one-and-only DAG)

    // no scaling needed from prev soln (as in NonDLocalReliability) since
    // updated avg_N_H now includes allocation from previous solution and
    // should be active on constraint bound (excepting sample count rounding)

    ensemble_numerical_solution(sequenceCost, soln, numSamples);
  }

  if (outputLevel >= NORMAL_OUTPUT)
    print_computed_solution(Cout, soln); // ***
}


void NonDMultilevBLUESampling::
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

  IntRealMatrixMap sum_L_refined = sum_L_baselineH;//baselineL;
  Sizet2DArray N_L_actual_shared;  inflate(N_H_actual, N_L_actual_shared);
  Sizet2DArray N_L_actual_refined = N_L_actual_shared;
  SizetArray   N_L_alloc_refined;  inflate(N_H_alloc, N_L_alloc_refined);
  size_t start, end;
  for (end=numApprox; end>0; --end) {
    // pairwise (IS and RD) or pyramid (MF):
    start = (mlmfSubMethod == SUBMETHOD_ACV_MF) ? 0 : end - 1;
    // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER 1ST PASS LAUNCH
    if (blue_approx_increment(soln, N_L_actual_refined, N_L_alloc_refined,
			      mlmfIter, approx_sequence, start, end)) {
      // ACV_IS samples on [approx-1,approx) --> sum_L_refined
      // ACV_MF samples on [0, approx)       --> sum_L_refined
      accumulate_acv_sums(sum_L_refined, N_L_actual_refined, approx_sequence,
			  start, end);
      increment_equivalent_cost(numSamples, sequenceCost, approx_sequence,
				start, end, equivHFEvals);
    }
  }

  // -----------------------------------------------------------
  // Compute/apply control variate parameter to estimate moments
  // -----------------------------------------------------------
  RealMatrix H_raw_mom(numFunctions, 4);
  blue_raw_moments(sum_L_baselineH, sum_L_refined, sum_H, sum_LL, sum_LH,
		  avg_eval_ratios, N_H_actual, N_L_actual_refined, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_actual_refined, N_L_alloc_refined);
}


bool NonDMultilevBLUESampling::
blue_approx_increment(const MFSolutionData& soln,
		     const Sizet2DArray& N_L_actual_refined,
		     SizetArray& N_L_alloc_refined,
		     size_t iter, const SizetArray& approx_sequence,
		     size_t start, size_t end)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts

  bool   ordered = approx_sequence.empty();
  size_t  approx = (ordered) ? end-1 : approx_sequence[end-1];
  Real lf_target = soln.solution_variables()[approx];
  if (backfillFailures) {
    Real lf_curr = average(N_L_actual_refined[approx]);
    numSamples = one_sided_delta(lf_curr, lf_target); // average
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << numSamples
	   << " computed from delta between LF target = " << lf_target
	   << " and current average count = " << lf_curr << std::endl;
    size_t N_alloc = one_sided_delta(N_L_alloc_refined[approx], lf_target);
    increment_sample_range(N_L_alloc_refined, N_alloc, approx_sequence,
			   start, end);
  }
  else {
    size_t lf_curr = N_L_alloc_refined[approx];
    numSamples = one_sided_delta((Real)lf_curr, lf_target);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << numSamples
	   << " computed from delta between LF target = " << lf_target
	   << " and current allocation = " << lf_curr << std::endl;
    increment_sample_range(N_L_alloc_refined, numSamples, approx_sequence,
			   start, end);
  }
  // the approximation sequence can be managed within one set of jobs using
  // a composite ASV with EnsembleSurrModel
  return approx_increment(iter, approx_sequence, start, end);
}


void NonDMultilevBLUESampling::
print_computed_solution(std::ostream& s, const MFSolutionData& soln)
{
  const RealVector& soln_vars = soln.solution_variables();
  size_t i, num_v = soln_vars.size();
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

// *** TO HERE ***
// > Need to proliferate MFSolutionData update
// > Need to start defining optimizer callbacks for est_var and linear_cost


Real NonDMultilevBLUESampling::
update_hf_target(const RealVector& avg_eval_ratios, const RealVector& var_H,
		 const RealVector& estvar0)
{
  // Note: there is a circular dependency between estvar_ratios and hf_targets

  // estimator variance uses actual (not alloc) so use same for defining G,g
  // *** TO DO: but avg_hf_target defines delta relative to actual||alloc ***
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  Real N_H = //(backfillFailures) ?
    average(NLevActual[hf_form_index][hf_lev_index]);// :
    //NLevAlloc[hf_form_index][hf_lev_index];
  RealVector cd_vars, estvar_ratios;
  r_and_N_to_design_vars(avg_eval_ratios, N_H, cd_vars);
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual for ACV,GenACV

  RealVector hf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    hf_targets[qoi] = var_H[qoi] * estvar_ratios[qoi]
                    / (convergenceTol * estvar0[qoi]);
  Real avg_hf_target = average(hf_targets);
  return avg_hf_target;
}


/** Multi-moment map-based version used by ACV following shared_increment() */
void NonDMultilevBLUESampling::
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
void NonDMultilevBLUESampling::
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


/** This version used by ACV following shared_approx_increment() */
void NonDMultilevBLUESampling::
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
void NonDMultilevBLUESampling::
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
void NonDMultilevBLUESampling::
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


/* *** TO DO: ELEVATE SHARED CODE OR INHERIT FROM ACV SIMILAR TO GenACV
void NonDMultilevBLUESampling::
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


void NonDMultilevBLUESampling::
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
*/


void NonDMultilevBLUESampling::
blue_raw_moments(IntRealMatrixMap& sum_L_baseline,
		 IntRealMatrixMap& sum_L_refined,   IntRealVectorMap& sum_H,
		 IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		 const RealVector& avg_eval_ratios, const SizetArray& N_shared,
		 const Sizet2DArray& N_L_refined,   RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  precompute_blue_control(avg_eval_ratios, N_shared);

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
      compute_blue_control(sum_L_base_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			   N_shared_q, mom, qoi, beta);
      // *** TO DO: support shared_approx_increment() --> baselineL

      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
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


void NonDMultilevBLUESampling::
update_projected_group_samples(const MFSolutionData& soln,
			       //const SizetArray& N_H_actual,size_t& N_H_alloc,
			       //SizetArray& delta_N_L_actual,
			       Real& delta_equiv_hf)
{
  Sizet2DArray N_L_actual; inflate(N_H_actual, N_L_actual);
  SizetArray   N_L_alloc;  inflate(N_H_alloc,  N_L_alloc);

  size_t group, num_groups = modelGroups.size(), alloc_incr, actual_incr;
  Real group_tgt;
  const RealVector& soln_vars = soln.solution_variables();
  for (group=0; group<num_groups; ++group) {
    group_tgt = soln_vars[group];
    const SizetArray& N_L_actual_g = N_L_actual[group];
    size_t&           N_L_alloc_g  =  N_L_alloc[group];
    alloc_incr  = one_sided_delta(N_L_alloc_g, group_tgt);
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_L_actual_g), group_tgt) : alloc_incr;
    /*delta_N_L_actual[approx] += actual_incr;*/  N_L_alloc_g += alloc_incr;
    increment_equivalent_cost(actual_incr, sequenceCost, //inflate_i, // ***
			      delta_equiv_hf);
  }

  finalize_counts(N_L_actual, N_L_alloc);
}


/**
void NonDMultilevBLUESampling::
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

  size_t alloc_incr = one_sided_delta(N_H_alloc, avg_hf_target),
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_H_actual), avg_hf_target) : alloc_incr;
  delta_N_H_actual += actual_incr;  N_H_alloc += alloc_incr;
  increment_equivalent_cost(actual_incr, sequenceCost, numApprox,
			    delta_equiv_hf);
}
*/

} // namespace Dakota
