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
    optSubProblemForm =//(truthFixedByPilot && pilotMgmtMode != OFFLINE_PILOT) ?
      //N_GROUP_R_ONLY_LINEAR_CONSTRAINT :
      N_GROUP_LINEAR_CONSTRAINT;

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
    tensor_product_multi_index(tp_orders, modelGroups, true);
  modelGroups.erase(modelGroups.begin()); // discard empty group (all 0's)

  numGroups = modelGroups.size();  size_t g, m, num_models;
  modelGroupCost.sizeUninitialized(numGroups);
  for (g=0; g<numGroups; ++g) {
    const UShortArray& models = modelGroups[g];
    Real& group_cost = modelGroupCost[g];
    group_cost = 0.; num_models = models.size();
    for (m=0; m<num_models; ++m)
      group_cost += sequenceCost[models[m]];
  }
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
  SizetArray N_G_alloc;  N_G_alloc.assign(numGroups, 0);
  RealMatrixArray var_G;
  //initialize_blue_counts(N_G_actual, N_GG);
  //initialize_blue_covariances(covGG, var_G);
  SizetArray delta_N_G = pilotSamples; // sized by load_pilot_samples()

  while (Pecos::l1_norm(delta_N_G) && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    group_increment(delta_N_G, mlmfIter); // spans ALL model groups, blocking
    accumulate_blue_sums(sum_G, sum_GG, N_G_actual);//, num_GG);
    //N_G_alloc += (backfillFailures && mlmfIter) ? one_sided_delta(N_G_alloc,
    //  blueSolnData.solution_reference()) : numSamples; // *** TO DO

    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (onlineCost && mlmfIter == 0) recover_online_cost(sequenceCost);
    increment_equivalent_cost(delta_N_G, modelGroupCost,
			      sequenceCost[numApprox], equivHFEvals);
    compute_GG_statistics(sum_G[1], sum_GG[1], N_G_actual, var_G, covGG); // *** TO HERE ***

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and BLUE mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_allocations(var_G, blueSolnData); // *** TO DO: update delta_N_G
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    RealMatrix H_raw_mom(numFunctions, 4);
    //blue_raw_moments(sum_G, sum_GG, H_raw_mom); // *** TO DO ***
    convert_moments(H_raw_mom, momentStats);
  }
  //finalize_counts(N_G_actual, N_G_alloc); // *** TO DO ***
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var G & covar GG from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  RealMatrixArray sum_G_pilot, var_G; RealSymMatrix2DArray sum_GG_pilot;
  SizetMatrixArray N_shared_pilot;
  evaluate_pilot(sum_G_pilot, sum_GG_pilot, N_shared_pilot, false);
  compute_GG_statistics(sum_G_pilot, sum_GG_pilot, N_shared_pilot,
			var_G, covGG);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealMatrixArrayMap sum_G; IntRealSymMatrix2DArrayMap sum_GG;
  SizetMatrixArray N_G_actual;  //SizetSymMatrix2DArray N_GG;
  initialize_blue_sums(sum_G, sum_GG); initialize_blue_counts(N_G_actual);
  SizetArray N_G_alloc;  N_G_alloc.assign(numGroups, 0);
  //initialize_blue_counts(N_G_actual, N_GG);
  //initialize_blue_covariances(covGG, covLH, varG);

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_allocations(var_G, blueSolnData); // *** TO DO: update N_G_alloc
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    // perform the shared increment for the online sample profile
    group_increment(N_G_alloc, mlmfIter); // spans ALL models, blocking
    accumulate_blue_sums(sum_G, sum_GG, N_G_actual);
    //N_G_alloc += numSamples; // *** TO DO ***
    increment_equivalent_cost(N_G_alloc, modelGroupCost,
			      sequenceCost[numApprox], equivHFEvals);
    // extract moments
    RealMatrix H_raw_mom(numFunctions, 4);
    //blue_raw_moments(sum_G, sum_GG, H_raw_mom); // *** TO DO ***
    convert_moments(H_raw_mom, momentStats);
  }
  //finalize_counts(N_G_actual, N_G_alloc); // *** TO DO ***
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevBLUESampling::ml_blue_pilot_projection()
{
  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealMatrixArray sum_G, var_G; RealSymMatrix2DArray sum_GG;
  SizetMatrixArray N_G_actual;  //SizetSymMatrix2DArray N_GG;
  evaluate_pilot(sum_G, sum_GG, N_G_actual, true);
  compute_GG_statistics(sum_G, sum_GG, N_G_actual, var_G, covGG);
  SizetArray N_G_alloc = pilotSamples;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_allocations(var_G, blueSolnData); // *** TO DO: update N_G_alloc
  ++mlmfIter;

  // No LF increments or final moments for pilot projection
  //finalize_counts(N_G_actual, N_G_alloc); // *** TO DO ***
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDMultilevBLUESampling::
group_increment(SizetArray& delta_N_G, size_t iter)
{
  if (iter == 0) Cout << "\nML BLUE pilot sample:\n";
  else Cout << "\nML BLUE sampling iteration " << iter
	    << ": group sample increment =\n";
  Cout << delta_N_G << '\n';

  size_t g, m, num_models, start;
  for (size_t g=0; g<numGroups; ++g) {
    numSamples = delta_N_G[g];
    if (numSamples) {
      ensemble_active_set(modelGroups[g]);
      ensemble_sample_batch(iter, g); // index is group_id; non-blocking
    }
  }

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
      num_lin_con = (pilotMgmtMode == OFFLINE_PILOT) ? 2 : 1;
      num_nln_con = 0;   break;
    case N_GROUP_LINEAR_OBJECTIVE:
      num_lin_con = (pilotMgmtMode == OFFLINE_PILOT) ? 1 : 0;
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

  size_t group, last_index = numGroups-1, lin_offset;
  const RealVector& soln_vars = soln.solution_variables();
  //Real offline_N_lwr = 2.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;

  // minimizer-specific updates (x bounds) performed in finite_solution_bounds()
  x_lb = 0.; // no lower bound for most groups (*** TO DO: NUDGE if not SDP?)
  x_ub = DBL_MAX; // no upper bounds for groups
  if (pilotMgmtMode == OFFLINE_PILOT) {
    lin_ineq_lb[0] = 1.; // at least 1 sample for any group
    lin_ineq_ub[0] = DBL_MAX;
    for (group=0; group<numGroups; ++group)
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
      for (group=0; group<numGroups; ++group) // bump x0 to satisfy x_lb if nec
	if (x0[group] < x_lb[group])
	  x0[group] = x_lb[group];
  }

  switch (optSubProblemForm) {
  case N_GROUP_LINEAR_CONSTRAINT: { // linear inequality constraint on budget:
    // \Sum_grp_i w_grp_i        N_grp_i <= equiv_HF * w_HF
    // \Sum_grp_i w_grp_i / w_HF N_grp_i <= equivHF
    Real cost_H = cost[numApprox];
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
compute_allocations(const RealMatrixArray& var_L, MFSolutionData& soln)
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
      // use pyramid sample definitions for hierarchical groups as init guess,
      // even though they are not independent sample sets in MFMC
      //covariance_to_correlation_sq(covLH, var_L, varH, rho2LH);
      //analytic_initialization_from_mfmc(avg_N_H, soln);
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
print_computed_solution(std::ostream& s, const MFSolutionData& soln)
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

/*
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
*/


/** Multi-moment map-based version used by online pilot */
void NonDMultilevBLUESampling::
accumulate_blue_sums(IntRealMatrixArrayMap& sum_G,
		     IntRealSymMatrix2DArrayMap& sum_GG,
		     SizetMatrixArray& num_G)//,SizetSymMatrix2DArray& num_GG)
{
  using std::isfinite;  bool all_finite;
  Real g1_fn, g2_fn, g1_prod, g2_prod;
  IntRespMCIter r_it;  IntRMAMIter g_it;  IntRSM2AMIter gg_it;
  int g_ord, gg_ord, active_ord;
  size_t g, num_groups = modelGroups.size(), qoi, m, m2, num_models,
    g1_index, g2_index;

  for (g=0; g<num_groups; ++g) {
    num_models                       = modelGroups[g].size();
    SizetMatrix&             num_G_g =  num_G[g]; // index is group_id
    //SizetSymMatrixArray&  num_GG_g = num_GG[g];
    num_models                       = modelGroups[g].size();
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
	  g1_index = m * numFunctions + qoi;
	  if ((asv[g1_index] & 1) && !isfinite(fn_vals[g1_index]))
	    { all_finite = false; break; }
	}
	if (!all_finite) continue;

	for (m=0; m<num_models; ++m) {
	  g1_index = m * numFunctions + qoi;
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
		  g2_index = m2 * numFunctions + qoi;
		  if (asv[g2_index] & 1) {
		    // regenerate g2_prod i/o storing off-diagonal combinations
		    g2_prod = g2_fn = fn_vals[g2_index];
		    for (m=1; m<active_ord; ++m)
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
    num_models                       = modelGroups[g].size();
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
	  g1_index = m * numFunctions + qoi;
	  if ((asv[g1_index] & 1) && !isfinite(fn_vals[g1_index]))
	    { all_finite = false; break; }
	}
	if (!all_finite) continue;
      
	RealSymMatrix& sum_GG_gq = sum_GG_g[qoi];
	//SizetSymMatrix& num_GG_gq = num_GG_g[qoi];
	for (m=0; m<num_models; ++m) {
	  g1_index = m * numFunctions + qoi;
	  if (asv[g1_index] & 1) {
	    ++num_G_g(qoi,m); // shared due to fault tol logic

	    g1_fn = fn_vals[g1_index];
	    sum_G_g(qoi,m) += g1_fn;
	    sum_GG_gq(m,m) += g1_fn * g1_fn;

	    // Off-diagonal of covariance matrix:
	    // look back (only) for single capture of each combination
	    for (m2=0; m2<m; ++m2) {
	      g2_index = m2 * numFunctions + qoi;
	      if (asv[g2_index] & 1) {
		//++num_GG_gq(m,m2);
		sum_GG_gq(m,m2) += g1_fn * fn_vals[g2_index];
	      }
	    }
	  }
	}
      }
    }
  }
}


void NonDMultilevBLUESampling::
compute_GG_statistics(RealMatrixArray& sum_G,
		      RealSymMatrix2DArray& sum_GG,
		      SizetMatrixArray& N_shared, RealMatrixArray& var_G,
		      RealSymMatrix2DArray& cov_GG)
{
  if (mlmfIter == 0) // see var_G usage in compute_allocations()
    compute_G_variance(sum_G,  sum_GG, N_shared, var_G); // *** Review uses; just extract once from cov_GG ?
  compute_GG_covariance(sum_G, sum_GG, N_shared, cov_GG);
  //Cout << "var_G:\n" << var_G << "cov_GG:\n" << cov_GG;
}


void NonDMultilevBLUESampling::
compute_GG_covariance(const RealMatrixArray& sum_G,
		      const RealSymMatrix2DArray& sum_GG,
		      const SizetMatrixArray& num_G,
		    //SizetSymMatrix2DArray& num_GG,
		      RealSymMatrix2DArray& cov_GG)
{
  size_t g, m, m2, num_models, qoi;
  if (cov_GG.size() != numGroups) {
    cov_GG.resize(numGroups);
    for (g=0; g<numGroups; ++g) {
      num_models = modelGroups[g].size();
      RealSymMatrixArray& cov_GG_g = cov_GG[g];
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
	for (m2=0; m2<=num_models; ++m2)
	  compute_covariance(sum_G_gqm, sum_G_g(qoi,m2), sum_GG_gq(m,m2),
			     N_sh_gqm, cov_GG_gq(m,m2));
      }
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "cov_GG in compute_GG_covariance():\n" << cov_GG << std::endl;
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
			       //const SizetArray& N_H_actual,size_t& N_H_alloc,
			       SizetArray& delta_N_L_actual,
			       Real& delta_equiv_hf)
{
  Sizet2DArray N_L_actual; //inflate(N_H_actual, N_L_actual);
  SizetArray   N_L_alloc;  //inflate(N_H_alloc,  N_L_alloc);

  size_t group, alloc_incr, actual_incr;
  Real group_tgt;
  const RealVector& soln_vars = soln.solution_variables();
  for (group=0; group<numGroups; ++group) {
    group_tgt = soln_vars[group];
    /*
    const SizetArray& N_L_actual_g = N_L_actual[group];
    size_t&           N_L_alloc_g  =  N_L_alloc[group];
    alloc_incr  = one_sided_delta(N_L_alloc_g, group_tgt);
    actual_incr = (backfillFailures) ?
      one_sided_delta(average(N_L_actual_g), group_tgt) : alloc_incr;
    //delta_N_L_actual[approx] += actual_incr;
    N_L_alloc_g += alloc_incr;
    */
  }

  increment_equivalent_cost(delta_N_L_actual, modelGroupCost,
			    sequenceCost[numApprox], delta_equiv_hf);
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


void NonDMultilevBLUESampling::
blue_raw_moments(IntRealMatrixMap& sum_L_baseline,
		 IntRealMatrixMap& sum_L_refined,
		 IntRealSymMatrixArrayMap& sum_LL,
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
*/

} // namespace Dakota
