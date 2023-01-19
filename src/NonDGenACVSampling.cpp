/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGenACVSampling
//- Description: Implementation code for NonDGenACVSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDGenACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"

static const char rcsId[]="@(#) $Id: NonDGenACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGenACVSampling::
NonDGenACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDACVSampling(problem_db, model), dagRecursionType(KL_GRAPH_RECURSION)
{
  // Unless the ensemble changes, the set of admissible DAGS is invariant:
  //UShortArraySet model_dags;
  generate_dags(modelDAGs);
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


void NonDGenACVSampling::generate_dags(UShortArraySet& model_graphs)
{
  // zero root directed acyclic graphs
  switch (dagRecursionType) {
  case KL_GRAPH_RECURSION: {
    size_t i, K, L, M_minus_K;  UShortArray dag(numApprox);
    // Dakota low-to-high ordering (enumerate valid KL values and convert)
    for (K=0; K<=numApprox; ++K) {
      M_minus_K = numApprox - K;

      // K highest fidelity approximations target root (numApprox):
      for (i=M_minus_K; i<numApprox; ++i) dag[i] = numApprox;

      for (L=0; L<=K; ++L) { // ordered single recursion
	// M-K lowest fidelity approximations target L:
	for (i=0; i<M_minus_K; ++i)       dag[i] = numApprox - L;

	/* ACV/GenACV ordering:
	// K  highest fidelity approximations target root (0):
	for (i=0; i<K;         ++i)  dag[i] = 0;
	// M-K lowest fidelity approximations target L:
	for (i=K; i<numApprox; ++i)  dag[i] = L;
	*/

	model_graphs.insert(dag);
      }
    }
    break;
  }
  case SINGLE_GRAPH_RECURSION: {
    size_t i, K, L;  UShortArray dag(numApprox);
    for (K=0; K<=numApprox; ++K) {
      for (L=0; L<=numApprox; ++L) { // unordered single recursion ???
	for (i=0; i<K;         ++i)  dag[i] = 0;
	for (i=K; i<numApprox; ++i)  dag[i] = L;
	if (contains(dag, 0)) // enforce constraints (contains 0, acyclic)
	  model_graphs.insert(dag);
      }
    }
    break;
  }
  case MULTI_GRAPH_RECURSION: {
    break;
  }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Model DAGs:\n" << model_graphs << std::endl;
}


void NonDGenACVSampling::generate_reverse_dag(const UShortArray& dag)
{
  reverseActiveDAG.clear();
  reverseActiveDAG.resize(numApprox+1);

  size_t i;  unsigned short dag_curr, dag_next;
  for (i=0; i<numApprox; ++i) {
    // walk+store path to root
    dag_curr = i;  dag_next = dag[dag_curr];
    reverseActiveDAG[dag_next].insert(dag_curr);
    while (dag_next != numApprox) {
      dag_curr = dag_next;  dag_next = dag[dag_curr];
      reverseActiveDAG[dag_next].insert(dag_curr);
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Reverse active DAG:\n" << reverseActiveDAG << std::endl;
}


void NonDGenACVSampling::root_list_from_reverse_dag(UShortList& root_list)
{
  // create an ordered list of roots that enable ordered sample increments
  // by ensuring that root sample levels are defined
  root_list.clear(); root_list.push_back(numApprox);
  UShortList::iterator it = root_list.begin(); 
  while (it != root_list.end()) {
    const UShortSet& reverse_dag = reverseActiveDAG[*it];
    root_list.insert(root_list.end(), reverse_dag.begin(), reverse_dag.end());
    ++it;
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Ordered root list in root_list_from_reverse_dag():\n" << root_list
	 << std::endl;
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDGenACVSampling::core_run()
{
  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

  switch (pilotMgmtMode) {
  case  ONLINE_PILOT: // iterated ACV (default)
    generalized_acv_online_pilot();     break;
  case OFFLINE_PILOT: // computes perf for offline pilot/Oracle correlation
    generalized_acv_offline_pilot();    break;
  case PILOT_PROJECTION: // for algorithm assessment/selection
    generalized_acv_pilot_projection(); break;
  }

  /*
  Initial DAG-enumeration of inherited implementations was wasteful:
  > relied on duplication detection for reusing pilot
  > allowed/defaulted to online mode for each DAG (wasteful for poor DAG)
  Adopt same approach proposed for AAO hyper-parameter model tuning --> need
  shared increments for online_pilot mode on outer-most loop around DAG/ensemble
  selection, followed by LF increments only after finalization of config.

  for (activeDAGIter=modelDAGs.begin(); activeDAGIter!=modelDAGs.end();
       ++activeDAGIter) {
    Cout << "Generalized ACV evaluating DAG:\n" << *activeDAGIter << std::endl;

    NonDACVSampling::core_run(); // {online,offline,projection} modes

    update_best(); // store reqd state for restoration w/o any recomputation
    reset_acv();   // reset state for next ACV execution
  }

  restore_best();
  // Post-process ...
  */
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_online_pilot()
{
  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, avg_eval_ratios;  RealMatrix var_L;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  Real avg_hf_target = 0.;
  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += (backfillFailures && mlmfIter) ?
      one_sided_delta(N_H_alloc, avg_hf_target) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (onlineCost && mlmfIter == 0) recover_online_cost(sequenceCost);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,
			      equivHFEvals);
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, var_L, varH, covLL, covLH);

    bestAvgEstVar = DBL_MAX;
    for (activeDAGIter  = modelDAGs.begin();
	 activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
      // sample set definitions are enabled by reversing the DAG direction:
      generate_reverse_dag(*activeDAGIter);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		     avgEstVar, avgEstVarRatio);
      update_best(avg_eval_ratios, avg_hf_target);// store state for restoration
      //reset_acv(); // reset state for next ACV execution
    }
    restore_best(avg_eval_ratios, avg_hf_target);
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, avg_eval_ratios, avg_hf_target);
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(avg_hf_target, avg_eval_ratios, N_H_actual,
				N_H_alloc, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_offline_pilot()
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
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH, avg_eval_ratios;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  Real avg_hf_target = 0.;
  bestAvgEstVar = DBL_MAX;
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // sample set definitions are enabled by reversing the DAG direction:
    generate_reverse_dag(*activeDAGIter);
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);
    update_best(avg_eval_ratios, avg_hf_target); // store state for restoration
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best(avg_eval_ratios, avg_hf_target);

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // at least 2 samples reqd for variance (+ resetting allSamples from pilot)
  numSamples = std::max(numSamples, (size_t)2);  ++mlmfIter;
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_H_actual);//, N_LL);
  N_H_alloc += numSamples;
  increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,equivHFEvals);
  // allow pilot to vary for C vs c

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, avg_eval_ratios, avg_hf_target);
  else
    // N_H is converged from offline pilot --> do not compute deltaNActualHF
    update_projected_lf_samples(avg_hf_target, avg_eval_ratios, N_H_actual,
				N_H_alloc, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_pilot_projection()
{
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];

  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealVector sum_H, sum_HH;   RealMatrix sum_L_baselineH, sum_LH, var_L;
  RealSymMatrixArray sum_LL;
  evaluate_pilot(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH,
		 N_H_actual, true);
  compute_LH_statistics(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH,
			N_H_actual, var_L, varH, covLL, covLH);
  N_H_alloc = numSamples;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  RealVector avg_eval_ratios;  Real avg_hf_target = 0.;
  bestAvgEstVar = DBL_MAX;
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // sample set definitions are enabled by reversing the DAG direction:
    generate_reverse_dag(*activeDAGIter);
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);
    update_best(avg_eval_ratios, avg_hf_target); // store state for restoration
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best(avg_eval_ratios, avg_hf_target);

  ++mlmfIter;
  // No LF increments or final moments for pilot projection
  update_projected_samples(avg_hf_target, avg_eval_ratios, N_H_actual,
			   N_H_alloc, deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::nonhierarch_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDGenACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baselineH, IntRealVectorMap& sum_H,
		  IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  const RealVector& avg_eval_ratios, Real avg_hf_target)
{
  // ---------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ---------------------------------------------------------------
  // Note: for consistency with MFMC/ACV, r* is always defined relative to N_H,
  // even though an oversample may target a different model based on active DAG

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF:
    // Could use reverse_dag_set here as well, but reordering based on
    // avg_eval_ratios is sufficient for overlapping pyramid sample sets
    NonDACVSampling::
      approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
			N_H_alloc, avg_eval_ratios, avg_hf_target);
    break;
  default: { // IS / RD sample management require DAG tracking
    IntRealMatrixMap  sum_L_refined = sum_L_baselineH;//baselineL;
    Sizet2DArray  N_L_actual_shared;  inflate(N_H_actual, N_L_actual_shared);
    Sizet2DArray N_L_actual_refined = N_L_actual_shared;
    SizetArray    N_L_alloc_refined;  inflate(N_H_alloc, N_L_alloc_refined);

    // create an ordered list of roots that enable ordered sample increments
    // by ensuring that root sample levels are defined
    UShortList root_list;  root_list_from_reverse_dag(root_list);

    // Process shared sample increments based on the ordered list of roots, but
    // skip initial root = numApprox as it is out of bounds for r* and LF counts
    UShortList::iterator it;  unsigned short root;
    for (it=++root_list.begin(); it!=root_list.end(); ++it) {
      root = *it;  const UShortSet& reverse_dag_set = reverseActiveDAG[root];
      // *** TO DO NON_BLOCKING: PERFORM PASS 2 ACCUMULATE AFTER PASS 1 LAUNCH
      if (genacv_approx_increment(avg_eval_ratios, N_L_actual_refined,
				  N_L_alloc_refined, avg_hf_target, mlmfIter,
				  root, reverse_dag_set)) {
	// ACV_IS/RD samples on [approx-1,approx) --> sum_L_refined
	// ACV_MF    samples on [0, approx)       --> sum_L_refined
	accumulate_acv_sums(sum_L_refined, N_L_actual_refined, root,
			    reverse_dag_set);
	increment_equivalent_cost(numSamples, sequenceCost, root,
				  reverse_dag_set, equivHFEvals);
      }
    }

    // -----------------------------------------------------------
    // Compute/apply control variate parameter to estimate moments
    // -----------------------------------------------------------
    RealMatrix H_raw_mom(numFunctions, 4);
    acv_raw_moments(sum_L_baselineH, sum_L_refined, sum_H, sum_LL, sum_LH,
		    avg_eval_ratios, N_H_actual, N_L_actual_refined, H_raw_mom);
    // Convert uncentered raw moment estimates to final moments (central or std)
    convert_moments(H_raw_mom, momentStats);
    // post final sample counts into format for final results reporting
    finalize_counts(N_L_actual_refined, N_L_alloc_refined);
    break;
  }
  }
}


bool NonDGenACVSampling::
genacv_approx_increment(const RealVector& avg_eval_ratios,
			const Sizet2DArray& N_L_actual_refined,
			SizetArray& N_L_alloc_refined, Real hf_target,
			size_t iter, unsigned short root,
			const UShortSet& reverse_dag_set)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts

  Real lf_target = avg_eval_ratios[root] * hf_target;
  if (backfillFailures) {
    Real lf_curr = average(N_L_actual_refined[root]);
    numSamples = one_sided_delta(lf_curr, lf_target); // average
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << numSamples << " for root node index "
	   << root << " computed from delta between LF target = " << lf_target
	   << " and current average count = " << lf_curr << std::endl;
    size_t N_alloc = one_sided_delta(N_L_alloc_refined[root], lf_target);
    increment_sample_range(N_L_alloc_refined, N_alloc, root, reverse_dag_set);
  }
  else {
    size_t lf_curr = N_L_alloc_refined[root];
    numSamples = one_sided_delta((Real)lf_curr, lf_target);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << numSamples << " for root node index "
	   << root << " computed from delta between LF target = " << lf_target
	   << " and current allocation = " << lf_curr << std::endl;
    increment_sample_range(N_L_alloc_refined, numSamples,root,reverse_dag_set);
  }

  // the approximation sequence can be managed within one set of jobs using
  // a composite ASV with NonHierarchSurrModel
  return approx_increment(iter, root, reverse_dag_set);
}


void NonDGenACVSampling::
compute_ratios(const RealMatrix& var_L,     const RealVector& cost,
	       RealVector& avg_eval_ratios, Real& avg_hf_target,
	       Real& avg_estvar,            Real& avg_estvar_ratio)
{
  // --------------------------------------
  // Configure the optimization sub-problem
  // --------------------------------------
  // Set initial guess based either on related analytic solutions (iter == 0)
  // or warm started from previous solution (iter >= 1)

  approxSequence.clear(); // rho2_LH re-ordering from MFMC is not relevant here
  const UShortArray& active_dag = *activeDAGIter;
  if (mlmfIter == 0) {
    // For general DAG, set initial guess based on pairwise CVMC analytic solns
    // (analytic MFMC soln expected to be less relevant)

    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
    // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
    // (This differs from MLMC EstVar^0 which uses pilot for all levels.)
    // Note: could revisit this for case of lf_shared_pilot > hf_shared_pilot.
    compute_mc_estimator_variance(varH, N_H_actual, estVarIter0);
    numHIter0 = N_H_actual;
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
    // Modify budget to allow a feasible soln (var lower bnds: r_i > 1, N > N_H)
    // Can happen if shared pilot rolls up to exceed budget spec.
    Real budget             = (Real)maxFunctionEvals;
    bool budget_constrained = (maxFunctionEvals != SZ_MAX),
         budget_exhausted   = (equivHFEvals >= budget);
    //if (budget_exhausted) budget = equivHFEvals;

    if (budget_exhausted || convergenceTol >= 1.) { // no need for solve
      if (avg_eval_ratios.empty()) avg_eval_ratios.sizeUninitialized(numApprox);
      numSamples = 0; avg_eval_ratios = 1.; avg_hf_target = avg_N_H;
      avg_estvar = average(estVarIter0);    avg_estvar_ratio = 1.;
      return;
    }

    // Use ensemble of independent 2-model CVMCs, rescaled to aggregate budget.
    // Differs from derived ACV approach through use of paired DAG dependencies.
    RealMatrix eval_ratios;
    cvmc_ensemble_solutions(covLL, covLH, varH, cost, active_dag, eval_ratios);
    Cout << "CVMC eval_ratios:\n" << eval_ratios << std::endl;
    average(eval_ratios, 0, avg_eval_ratios);

    // By using an ordered root list, we can repair sequentially, top down.
    UShortList root_list;  root_list_from_reverse_dag(root_list);
    if (budget_constrained) { // scale according to cost
      // scale_to_target(..., root_list) incorporates linear ineq enforcement:
      scale_to_target(avg_N_H, cost, avg_eval_ratios, avg_hf_target, root_list);
      RealVector cd_vars;
      r_and_N_to_design_vars(avg_eval_ratios, avg_hf_target, cd_vars);
      avg_estvar = average_estimator_variance(cd_vars); // ACV or GenACV
    }
    else { // scale according to accuracy (convergenceTol * estVarIter0)
      enforce_linear_ineq_constraints(avg_eval_ratios, root_list);// prevent NaN
      avg_hf_target = update_hf_target(avg_eval_ratios, varH, estVarIter0);
    }
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "GenACV initial guess from ensemble CVMC:\n"
	   << "  average eval ratios:\n" << avg_eval_ratios
	   << "  average HF target = " << avg_hf_target
	   << "  average estvar = " << avg_estvar << std::endl;

    // Single solve initiated from lowest estvar
    nonhierarch_numerical_solution(cost, approxSequence, avg_eval_ratios,
				   avg_hf_target, numSamples, avg_estvar,
				   avg_estvar_ratio);
    prevSolns[active_dag] // *** TO DO: replace w/ ref to Map upstream
      = std::pair<RealVector, Real>(avg_eval_ratios, avg_hf_target);
  }
  else { // warm start from previous eval_ratios solution
    
    // no scaling needed from prev soln (as in NonDLocalReliability) since
    // updated avg_N_H now includes allocation from previous solution and is
    // active on constraint bound (excepting integer sample rounding)

    // warm start from previous solns for corresponding DAG
    std::pair<RealVector, Real>& soln = prevSolns[active_dag];
    nonhierarch_numerical_solution(cost, approxSequence, soln.first,
				   soln.second, numSamples, avg_estvar,
				   avg_estvar_ratio);
    avg_eval_ratios = soln.first; // *** TO DO: replace w/ ref to Map upstream
    avg_hf_target   = soln.second;
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    for (size_t approx=0; approx<numApprox; ++approx)
      Cout << "Approx " << approx+1 << ": average evaluation ratio = "
	   << avg_eval_ratios[approx] << '\n';
    Cout << "Average estimator variance = " << avg_estvar
	 << "\nAverage GenACV variance / average MC variance = "
	 << avg_estvar_ratio << std::endl;
  }
}


void NonDGenACVSampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  switch (optSubProblemForm) {
  case N_VECTOR_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_VECTOR_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t source, target, lin_ineq_offset
      = (optSubProblemForm == N_VECTOR_LINEAR_CONSTRAINT) ? 1 : 0;

    // Enforce DAG dependencies (ACV: all point to numApprox)
    // > N for each source model > N for model it targets
    // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
    const UShortArray& dag = *activeDAGIter;
    for (source=0; source<numApprox; ++source) {
      target = dag[source];
      lin_ineq_coeffs(source+lin_ineq_offset, source) = -1.;
      lin_ineq_coeffs(source+lin_ineq_offset, target) =  1. + RATIO_NUDGE;
      //Cout << "lin ineq: source = " << source
      //     << " target = " << target << std::endl;
    }
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT:
    // *** TO DO ***:
    // This is active for truthFixedByPilot && pilotMgmtMode != OFFLINE_PILOT
    // but r is not appropriate for general DAG including multiple CV targets.
    // --> either need to alter handling or suppress this option...

    Cerr << "Error: R_ONLY_LINEAR_CONSTRAINT not implemented in "
	 << "NonDGenACVSampling::linear_constraints()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT: // not used
    Cerr << "Error: R_AND_N_NONLINEAR_CONSTRAINT not supported in "
	 << "NonDGenACVSampling::linear_constraints()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}


void NonDGenACVSampling::
enforce_linear_ineq_constraints(RealVector& avg_eval_ratios,
				const UShortList& root_list)
{
  // Enforce DAG dependencies (ACV: all point to numApprox)
  // > N for each source model > N for model it targets
  // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
  UShortList::const_iterator rl_cit;  UShortSet::const_iterator rd_cit;
  unsigned short source, target;  Real r_tgt;
  for (rl_cit=root_list.begin(); rl_cit!=root_list.end(); ++rl_cit) {
    target = *rl_cit;  const UShortSet& reverse_dag = reverseActiveDAG[target];
    r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[target];
    for (rd_cit=reverse_dag.begin(); rd_cit!=reverse_dag.end(); ++rd_cit) {
      source = *rd_cit;  Real& r_src = avg_eval_ratios[source];
      if (r_src <= r_tgt) {
	r_src = r_tgt * (1. + RATIO_NUDGE);
	Cout << "Enforcing source = " << source << " target = " << target
	     << ": r_src = " << r_src << " r_tgt = "<< r_tgt << std::endl;
      }
    }
  }
}


void NonDGenACVSampling::
scale_to_target(Real avg_N_H, const RealVector& cost,
		RealVector& avg_eval_ratios, Real& avg_hf_target,
		const UShortList& root_list)
{
  // scale to enforce budget constraint.  Since the profile does not emerge
  // from pilot in ACV, don't select an infeasible initial guess:
  // > if N* < N_pilot, scale back r* --> initial = scaled_r*,N_pilot
  // > if N* > N_pilot, use initial = r*,N*
  avg_hf_target = allocate_budget(avg_eval_ratios, cost); // r* --> N*
  if (avg_N_H > avg_hf_target) {// replace N* with N_pilot, rescale r* to budget
    avg_hf_target = avg_N_H;

    // Could create another helper if there are additional clients:
    //scale_to_budget_with_pilot(avg_eval_ratios,cost,avg_hf_target,root_list);

    Real approx_inner_prod = 0.;  size_t approx;
    for (approx=0; approx<numApprox; ++approx)
      approx_inner_prod += cost[approx] * avg_eval_ratios[approx];
    // Apply factor: r_scaled = factor r^* which applies to LF (HF r remains 1)
    // > N_pilot (r_scaled^T w + 1) = budget, where w_i = cost_i / cost_H
    // > factor r*^T w = budget / N_pilot - 1
    Real budget = (Real)maxFunctionEvals, cost_H = cost[numApprox],
         factor = (budget / avg_N_H - 1.) / approx_inner_prod * cost_H;

    // Enforce DAG dependencies (ACV: all point to numApprox)
    // > N for each source model > N for model it targets
    // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
    UShortList::const_iterator rl_cit;  UShortSet::const_iterator rd_cit;
    unsigned short source, target;
    Real r_tgt, cost_r_src, budget_decr, inner_prod_decr;
    for (rl_cit=root_list.begin(); rl_cit!=root_list.end(); ++rl_cit) {
      target = *rl_cit; const UShortSet& reverse_dag = reverseActiveDAG[target];
      r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[target];
      budget_decr = inner_prod_decr = 0.;
      for (rd_cit=reverse_dag.begin(); rd_cit!=reverse_dag.end(); ++rd_cit) {
	source = *rd_cit; Real& r_src = avg_eval_ratios[source];
	r_src *= factor;
	if (r_src <= 1.) {
	  r_src = r_tgt * (1. + RATIO_NUDGE);
	  Cout << "Enforcing source = " << source << " target = " << target
	       << ": r_src = " << r_src << " r_tgt = "<< r_tgt << std::endl;
	  // complete the reverse DAG using the same factor:
	  cost_r_src       = r_src * cost[source];
	  budget_decr     += avg_N_H * cost_r_src / cost_H;
	  inner_prod_decr += cost_r_src;
	}
      }
      // now update factor for next root: decrement budget/inner_prod
      budget -= budget_decr;  approx_inner_prod -= inner_prod_decr;
      factor  = (budget / avg_N_H - 1.) / approx_inner_prod * cost_H;
    }
    if (outputLevel >= DEBUG_OUTPUT) {
      Real inner_prod = cost_H;
      for (approx=0; approx<numApprox; ++approx)
	inner_prod += cost[approx] * avg_eval_ratios[approx];
      Cout << "Rescale to budget: average evaluation ratios\n"<< avg_eval_ratios
	   << "budget = " << avg_N_H * inner_prod / cost_H << std::endl;
    }
  }
  else
    enforce_linear_ineq_constraints(avg_eval_ratios, root_list);
}


void NonDGenACVSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  RealMatrix C_G_inv;  RealVector c_g;
  Real R_sq, N_H;
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    RealVector N_vec(numApprox+1, false);
    if (cd_vars.length() == numApprox) {
      copy_data_partial(cd_vars, N_vec, 0);
      // N_H not provided so pull from latest counter values
      size_t hf_form_index, hf_lev_index;
      hf_indices(hf_form_index, hf_lev_index);
      // estimator variance uses actual (not alloc) so use same for defining G,g
      // *** TO DO: but avg_hf_target defines delta relative to actual||alloc
      N_vec[numApprox] = N_H = //(backfillFailures) ?
	average(NLevActual[hf_form_index][hf_lev_index]);// :
	//NLevAlloc[hf_form_index][hf_lev_index];
    }
    else { // N_H appended for convenience or rescaling to updated HF target
      copy_data(cd_vars, N_vec);
      N_H = N_vec[numApprox];
    }
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec, *activeDAGIter);
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: { // convert r_and_N to N_vec:
    RealVector N_vec;  copy_data(cd_vars, N_vec);
    N_H = N_vec[numApprox];
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec, *activeDAGIter);
    break;
  }
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
    compute_parameterized_G_g(cd_vars, *activeDAGIter);
    N_H = cd_vars[numApprox];
    break;
  }

  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    //invert_C_G_matrix(covLL[qoi], GMat, C_G_inv);
    //compute_c_g_vector(covLH, qoi, gVec, c_g);
    //R_sq = compute_R_sq(C_G_inv, c_g, varH[qoi], N_H);
    //if (outputLevel >= DEBUG_OUTPUT)
    //  Cout << "-----------------------------\n"
    // 	     << "GenACV::estimator_variance_ratios(): C-G inverse =\n"
    // 	     << C_G_inv << "c-g vector =\n" << c_g
    // 	     << " Rsq[" << qoi << "] via invert() = " << R_sq
    // 	     << "\n-----------------------------\n" << std::endl;

    R_sq = compute_R_sq(covLL[qoi], GMat, covLH, gVec, qoi, varH[qoi], N_H);
    //if (outputLevel >= DEBUG_OUTPUT)
    //  Cout << "R_sq[" << qoi << "] via solve()  = " << R_sq
    //       << "\n-----------------------------\n" << std::endl;

    if (R_sq >= 1.) { // add nugget to C_G prior to solve()
      Cerr << "Warning: numerical issues in GenACV: R^2 > 1." << std::endl;
      /*
      Real nugget = 1.e-6;
      while (R_sq >= 1. and nugget_cntr <= 10) {
	R_sq = compute_R_sq(covLL[qoi], GMat, covLH, gVec, qoi,
	                    varH[qoi], N_H, nugget);
	nugget *= 10.;  ++nugget_cntr;
      }
      */
    }
    estvar_ratios[qoi] = (1. - R_sq);
  }
}


void NonDGenACVSampling::
compute_parameterized_G_g(const RealVector& N_vec, const UShortArray& dag)
{
  // Invert N_vec ordering
  // > Dakota r_i ordering is low-to-high --> reversed from Peherstorfer
  //   (HF = 1, MF = 2, LF = 3) and Gorodetsky,Bomarito (HF = 0, MF = 1, LF = 2)
  // > incoming N_vec is ordered as for Dakota r_i: LF = 0, MF = 1, HF = 2
  // > incoming DAG is zero root as in Bomarito 2022 --> reverse DAG ordering
  // See also NonDNonHierarchSampling::mfmc_analytic_solution()

  // Here we use ACV notation: z_i is comprised of N_i incoming samples
  // including z^1_i (less resolved "correlated mean estimator") and z^2_i
  // (more resolved "control variate mean").  Mapping from GenACV notation:
  // "z^*_i" --> z^1_i, "z_i" --> z^2_i, "z_{i* U i}" --> z_i

  size_t i, j;
  if (GMat.empty()) GMat.shapeUninitialized(numApprox);
  if (gVec.empty()) gVec.sizeUninitialized(numApprox);

  // define sample recursion sets backwards (from z_H down to lowest fid)
  RealVector z1, z2;  Real bi, bj, z1_i, z1_j, z2_i, z_H = N_vec[numApprox];
  if (mlmfSubMethod == SUBMETHOD_ACV_IS || mlmfSubMethod == SUBMETHOD_ACV_RD) {
    z1.size(numApprox);  z2.size(numApprox+1);  z2[numApprox] = z_H;
    
    // General approach should work for any recursion:
    unsigned short dag_curr, dag_next;
    UShortList path;  UShortList::reverse_iterator rit;
    for (i=0; i<numApprox; ++i) {
      // walk+store path to root
      dag_curr = i;  dag_next = dag[dag_curr];
      path.clear();  path.push_back(dag_curr);  path.push_back(dag_next);
      while (z2[dag_next] == 0) { // dag_next != numApprox &&
	dag_curr = dag_next;  dag_next = dag[dag_curr];
	path.push_back(dag_next);
      }
      // walk path in reverse direction to fill z1,z2
      rit = path.rbegin();  dag_next = *rit;  ++rit;
      for (; rit != path.rend(); ++rit) {
	dag_curr = *rit;
	z1[dag_curr] = z2[dag_next];
	z2[dag_curr] = N_vec[dag_curr] - z1[dag_curr];
	dag_next = dag_curr;
      }
    }

    // Approach leveraging reverseActiveDAG:
    //for (int d_index=numApprox; d_index>=0; --d_index) {
    //  const UShortSet& reverse_dag_set = reverseActiveDAG[d_index];
    //  ...
    //}

    // Single sweep is sufficient for ordered single recursion (ACV-KL)
    //for (int target=numApprox; target>=0; --target)
    //  for (i=0; i<numApprox; ++i)
    //	  if (dag[i] == target)
    //      { z1[i] = z2[target];  z2[i] = N_vec[i] - z1[i]; }

    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "GenACV-IS/RD unroll of N_vec:\n" << N_vec << "into z1:\n" << z1
	   << "and z2:\n" << z2 << std::endl;
  }

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    Real z_i, z_j, zi_zj;//, z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];  z_i = N_vec[i];  z1_i = z1[i];  z2_i = z2[i];
      gVec[i] = (bi == numApprox) ? 1./z1_i - 1./z_i : 0.;
      for (j=0; j<=i; ++j) {
	bj = dag[j];  z_j = N_vec[j];  z1_j = z1[j];  //z2_j = z2[j];
	GMat(i,j) = 0.;  zi_zj = z_i * z_j;
	if (bi == bj)  GMat(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
	if (bi ==  j)  GMat(i,j) += z1_i/zi_zj - 1./z_j; // false for dag = M
	if (i  == bj)  GMat(i,j) += z1_j/zi_zj - 1./z_i; // false for dag = M
	if (i  ==  j)  GMat(i,j) += z2_i/zi_zj;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];
      z1_i = /*z2_bi = z_bi =*/ N_vec[bi];  z2_i = /*z_i =*/ N_vec[i];
      gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<=i; ++j) {
	bj = dag[j];
	z1_j = /*z2_bj = z_bj =*/ N_vec[bj];  z2_j = /*z_j =*/ N_vec[j];
	GMat(i,j)
	  = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
	  + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: // Bomarito Eqs. 19-20
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];  z1_i = z1[i];  z2_i = z2[i];
      gVec[i] = (bi == numApprox) ? 1./z1_i : 0.;
      for (j=0; j<=i; ++j) {
	bj = dag[j];  //z1_j = z1[j];  z2_j = z2[j];
	GMat(i,j) = 0.;
	if (bi == bj)  GMat(i,j) += 1./z1_i;
	if (bi ==  j)  GMat(i,j) -= 1./z1_i;  // always false for dag = M
	if ( i == bj)  GMat(i,j) -= 1./z1[j]; // always false for dag = M
	if ( i ==  j)  GMat(i,j) += 1./z2_i;
      }
    }
    break;
  default:
    Cerr << "Error: bad sub-method name (" << mlmfSubMethod << ") in NonDGen"
	 << "ACVSampling::compute_parameterized_G_g()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "For dag:\n"  << dag  << "G matrix:\n" << GMat
	 << "g vector:\n" << gVec << std::endl;
}


void NonDGenACVSampling::
update_best(const RealVector& avg_eval_ratios, Real avg_hf_target)
{
  // Store best result:
  // > could potentially prune some of this tracking for final_statistics mode
  //   = estimator_performance, but suppress this additional complexity for now
  if (valid_variance(avgEstVar) && // *** TO DO: insufficient due to averaging --> use something like a badNumericsFlag to prevent adoption of bogus solve
      avgEstVar < bestAvgEstVar) {
    bestDAGIter        = activeDAGIter;
    bestAvgEvalRatios  = avg_eval_ratios;
    bestAvgHFTarget    = avg_hf_target;
    // could recompute these to reduce tracking
    bestAvgEstVar      = avgEstVar;
    bestAvgEstVarRatio = avgEstVarRatio;
    // reference points for output
    if (pilotMgmtMode != OFFLINE_PILOT) {
      bestEstVarIter0 = estVarIter0;
      bestNumHIter0   = numHIter0;
    }
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Updating best DAG to:\n" << *bestDAGIter << std::endl;
  }
}


void NonDGenACVSampling::
restore_best(RealVector& avg_eval_ratios, Real& avg_hf_target)
{
  Cout << "\nBest estimator variance = " << bestAvgEstVar
       << " from DAG:\n" << *bestDAGIter << std::endl;
  // restore best state for compute/archive/print final results
  if (activeDAGIter != bestDAGIter) { // best is not most recent
    activeDAGIter    = bestDAGIter;
    avg_eval_ratios  = bestAvgEvalRatios;
    avg_hf_target    = bestAvgHFTarget;
    avgEstVar        = bestAvgEstVar;
    avgEstVarRatio   = bestAvgEstVarRatio;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Restoring best DAG to:\n" << *activeDAGIter
	   << "with avg_hf_target = " << avg_hf_target
	   << "\nand avg_eval_ratios =\n" << avg_eval_ratios << std::endl;
    if (finalStatsType == QOI_STATISTICS &&
	pilotMgmtMode  != PILOT_PROJECTION &&
	mlmfSubMethod  != SUBMETHOD_ACV_MF) // approx_increments() for IS/RD
      generate_reverse_dag(*activeDAGIter);
    if (pilotMgmtMode != OFFLINE_PILOT)
      { estVarIter0 = bestEstVarIter0;  numHIter0 = bestNumHIter0; }
  }
}

} // namespace Dakota
