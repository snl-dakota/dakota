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
#include "SharedPolyApproxData.hpp"

static const char rcsId[]="@(#) $Id: NonDGenACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGenACVSampling::
NonDGenACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDACVSampling(problem_db, model),
  dagRecursionType(problem_db.get_short("method.nond.search_model_graphs")),
  dagDepthLimit(problem_db.get_ushort("method.nond.graph_depth_limit")),
  modelSelectType(NO_MODEL_SELECTION) //(ALL_MODEL_COMBINATIONS)
{
  // Unless the ensemble changes, the set of admissible DAGS is invariant:
  if (dagRecursionType == FULL_GRAPH_RECURSION) dagDepthLimit = numApprox;
  generate_dags();
  bestDAGIter = modelDAGs.end();
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


void NonDGenACVSampling::generate_dags()
{
  /* For verification of consistency with MFMC,ACV:
  UShortArray std_dag(numApprox);
  //for (size_t i=0; i<numApprox; ++i)  std_dag[i] = i+1; // MFMC
  std_dag.assign(numApprox, numApprox); // ACV
  modelDAGs.insert(std_dag);
  return;
  */

  // zero root directed acyclic graphs
  switch (dagRecursionType) {
  case KL_GRAPH_RECURSION: {
    size_t i, K, L, M_minus_K;  UShortArray dag(numApprox);
    // Dakota low-to-high ordering (enumerate valid KL values and convert)
    for (K=0; K<=numApprox; ++K) {
      M_minus_K = numApprox - K;

      // K highest fidelity approximations target root (numApprox):
      for (i=M_minus_K; i<numApprox; ++i) dag[i] = numApprox;
      // JCP ordering: for (i=0; i<K; ++i) dag[i] = 0; // root = 0

      for (L=0; L<=K; ++L) { // ordered single recursion
	// M-K lowest fidelity approximations target numApprox - L:
	for (i=0; i<M_minus_K; ++i)       dag[i] = numApprox - L;
	// JCP ordering: for (i=K; i<numApprox; ++i) dag[i] = L;

	modelDAGs.insert(dag);
      }
    }
    break;
  }
  default: {
    UShortArray nodes, dag;  unsigned short root = numApprox;  size_t i;
    switch (modelSelectType) {
    case ALL_MODEL_COMBINATIONS: {
      // tensor product of order 1 to enumerate approximation inclusion
      UShort2DArray tp;  UShortArray tp_orders(numApprox, 1);
      Pecos::SharedPolyApproxData::
	tensor_product_multi_index(tp_orders, tp, true);
      size_t j, num_tp = tp.size();

      // Two discrete enumeration options here:
      // > map<UShortArray,UShortArraySet> to associate pruned DAGs to nodes
      // > keep DAG size at numApprox and use USHRT_MAX for omitted approx

      /* Compacted version requires map from nodes --> DAG set
      unsigned short dag_size;
      for (i=0; i<num_tp; ++i) { // include first = {0} --> retain MC case
	const UShortArray& tp_i = tp[i];
	nodes.clear();
	for (j=0, dag_size=0; j<numApprox; ++j)
	  if (tp_i[j]) { nodes.push_back(j); ++dag_size; }
	dag.assign(dag_size, USHRT_MAX);
	// recur for dag with a subset of approximations:
	Cout << "\ngenerate_dags(): depth = " << dagDepthLimit
	     << " root = " << root << " nodes =\n" << nodes << std::endl;
	generate_sub_trees(root, nodes, dagDepthLimit, dag, dag_set);
	modelDAGS[nodes] = dag_set;
      }
      */

      // 
      for (i=0; i<num_tp; ++i) { // include first = {0} --> retain MC case
	const UShortArray& tp_i = tp[i];
	for (j=0; j<numApprox; ++j)
	  nodes[j] = (tp_i[j]) ? j : USHRT_MAX;
	// recur for dag with a subset of approximations:
	Cout << "\ngenerate_dags(): depth = " << dagDepthLimit
	     << " root = " << root << " nodes =\n" << nodes << std::endl;
	dag.assign(numApprox, USHRT_MAX);
	generate_sub_trees(root, nodes, dagDepthLimit, dag, modelDAGs);
      }

      exit(0);

      break;
    }

    // Rather then enumerating all of the discrete model combinations, this
    // employs one intgrated continuous solve, which approaches the best
    // discrete solution as some sample increments --> 0.  This approach will
    // likely encounter more issues with numerical conditioning
    case NO_MODEL_SELECTION:
      // root node (truth index = numApprox) and set of dependent nodes
      // (approximation model indices 0,numApprox-1) are fixed
      nodes.resize(numApprox);  for (i=0; i<numApprox; ++i) nodes[i] = i;
      dag.assign(numApprox, USHRT_MAX);
      // recur for DAG including all approximations:
      generate_sub_trees(root, nodes, dagDepthLimit, dag, modelDAGs);
      break;
    }
    break;
  }
  }

  Cout << "Searching array of model DAGs of size " << modelDAGs.size();
  if (outputLevel >= DEBUG_OUTPUT) Cout << ":\n" << modelDAGs;
  Cout << std::endl;
}


void NonDGenACVSampling::
generate_sub_trees(unsigned short root, const UShortArray& nodes,
		   unsigned short depth, UShortArray& dag,
		   UShortArraySet& model_dags)
{
  // Modeled after Python DAG generator on stack overflow
  // ("How to generate all trees having n-nodes and m-level depth")

  //Cout << "\nRecurring into generate_sub_trees(): depth = " << depth
  //     << " root = " << root << " nodes =\n" << nodes << std::endl;

  size_t i, num_nodes = nodes.size();
  if (nodes.empty()) {
    // suppress some duplicates for sub_nodes with empty sub_nodes_per_root
    //model_dags.insert(dag);
    return;
  }
  else if (depth <= 1) {
    for (i=0; i<num_nodes; ++i) dag[nodes[i]] = root;
    //Cout << "Recursion hit bottom.  Inserting:\n" << dag;
    model_dags.insert(dag); // insert completed DAG
    return;
  }

  // recursion required
  UShort2DArray tp1, tp2, sub_nodes_per_root;
  UShortArray   tp1_orders, tp2_orders, sub_roots, sub_nodes;
  // 1st TP defines root vs. non-root designation at this recursion level:
  tp1_orders.assign(num_nodes, 1);
  Pecos::SharedPolyApproxData::tensor_product_multi_index(tp1_orders, tp1,true);
  size_t j, k, num_tp1 = tp1.size(), num_tp2, num_sub_roots, num_sub_nodes;
  unsigned short node_j, dm1 = depth - 1;
  for (i=1; i<num_tp1; ++i) { // skip 1st entry ({0} = no sub-root is invalid)
    //if (depth == dagDepthLimit) dag.assign(numApprox, USHRT_MAX); // reset
    const UShortArray& tp1_i = tp1[i];
    //Cout << "Depth " << depth << " tp1[" << i << "]:\n" << tp1_i;
    // segregate "on" (1) into roots, "off" (0) into nexts
    sub_roots.clear(); sub_nodes.clear();
    for (j=0; j<num_nodes; ++j) {
      node_j = nodes[j];
      if (tp1_i[j]) { sub_roots.push_back(node_j);  dag[node_j] = root; }
      else            sub_nodes.push_back(node_j); // "nexts"
    }
    if (sub_nodes.empty()) { // optimize out unnecessary recursion
      //Cout << "Recursion out of nodes.  Inserting:\n" << dag;
      model_dags.insert(dag); // insert completed DAG
    }
    else {
      num_sub_roots = sub_roots.size();  num_sub_nodes = sub_nodes.size();
      sub_nodes_per_root.resize(num_sub_roots);
      // 2nd TP defines assignments of sub_nodes per sub_root (differs from 1st
      // TP above as there are multiple roots available, which will be recurred)
      tp2_orders.assign(num_sub_nodes, num_sub_roots);
      Pecos::SharedPolyApproxData::
	tensor_product_multi_index(tp2_orders, tp2, false);
      num_tp2 = tp2.size();
      for (j=0; j<num_tp2; ++j) { // 1st entry (all 0's) is valid for this TP
	//Cout << "  Depth " << depth << " tp2[" << j << "]:\n" << tp2[j];
	const UShortArray& tp2_j = tp2[j];
	for (k=0; k<num_sub_roots; ++k) sub_nodes_per_root[k].clear();
	for (k=0; k<num_sub_nodes; ++k) {
	  unsigned short tp2_jk = tp2_j[k];
	  sub_nodes_per_root[tp2_jk].push_back(sub_nodes[k]);
	}
	for (k=0; k<num_sub_roots; ++k)
	  generate_sub_trees(sub_roots[k], sub_nodes_per_root[k], dm1, dag,
			     model_dags);
      }
    }
  }
}


void NonDGenACVSampling::generate_reverse_dag(const UShortArray& dag)
{
  // define an array of source nodes that point to a given target
  reverseActiveDAG.clear();
  reverseActiveDAG.resize(numApprox+1);
  size_t i;  unsigned short dag_curr, dag_next;
  for (i=0; i<numApprox; ++i) { // walk+store path to root
    dag_curr = i;  dag_next = dag[dag_curr];
    reverseActiveDAG[dag_next].insert(dag_curr);
    while (dag_next != numApprox) {
      dag_curr = dag_next;  dag_next = dag[dag_curr];
      reverseActiveDAG[dag_next].insert(dag_curr);
    }
  }

  // create an ordered list of roots that enable ordered sample increments
  // by ensuring that root sample levels are defined
  orderedRootList.clear();  orderedRootList.push_back(numApprox);
  UShortList::iterator it = orderedRootList.begin();
  while (it != orderedRootList.end()) {
    const UShortSet& reverse_dag = reverseActiveDAG[*it];
    orderedRootList.insert(orderedRootList.end(),
			   reverse_dag.begin(), reverse_dag.end());
    ++it;
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In generate_reverse_dag(), reverse DAG:\n" << reverseActiveDAG
	 << "Ordered root list:\n" << orderedRootList << std::endl;
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
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_online_pilot()
{
  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH;  RealMatrix var_L;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  Real avg_hf_target = 0., avg_N_H;
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

    if (mlmfIter == 0) precompute_ratios(); // metrics not dependent on DAG
    for (activeDAGIter  = modelDAGs.begin();
	 activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
      // sample set definitions are enabled by reversing the DAG direction:
      const UShortArray& active_dag = *activeDAGIter;
      if (outputLevel >= QUIET_OUTPUT)
	Cout << "Evaluating active DAG:\n" << active_dag << std::endl;
      generate_reverse_dag(active_dag);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      DAGSolutionData& soln = dagSolns[active_dag];
      compute_ratios(var_L, soln);
      update_best(soln);// store state for restoration
      //reset_acv(); // reset state for next ACV execution
    }
    restore_best();
    if (truthFixedByPilot) numSamples = 0;
    else {
      avg_hf_target = dagSolns[*activeDAGIter].avgHFTarget;
      avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
      numSamples = one_sided_delta(avg_N_H, avg_hf_target);
    }
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  DAGSolutionData& soln = dagSolns[*activeDAGIter];
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, soln.avgEvalRatios, soln.avgHFTarget);
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(soln.avgHFTarget, soln.avgEvalRatios,
				N_H_actual, N_H_alloc, deltaEquivHF);
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
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  precompute_ratios(); // compute metrics not dependent on active DAG
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // sample set definitions are enabled by reversing the DAG direction:
    const UShortArray& active_dag = *activeDAGIter;
    if (outputLevel >= QUIET_OUTPUT)
      Cout << "Evaluating active DAG:\n" << active_dag << std::endl;
    generate_reverse_dag(active_dag);
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    DAGSolutionData& soln = dagSolns[active_dag];
    compute_ratios(var_L, soln);
    update_best(soln); // store state for restoration
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best();
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // Only QOI_STATISTICS requires online shared/approx profile evaluation for
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  DAGSolutionData& soln = dagSolns[*activeDAGIter];
  if (finalStatsType == QOI_STATISTICS) {
    if (truthFixedByPilot) numSamples = 0;
    else {
      Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
      numSamples = one_sided_delta(avg_N_H, soln.avgHFTarget);
    }
    // perform the shared increment for the online sample profile
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += numSamples;
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,
			      equivHFEvals);
    // perform LF increments for the online sample profile
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, soln.avgEvalRatios, soln.avgHFTarget);
  }
  else // project online profile including both shared samples and LF increment
    update_projected_samples(soln.avgHFTarget, soln.avgEvalRatios, N_H_actual,
			     N_H_alloc, deltaNActualHF, deltaEquivHF);
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
  precompute_ratios(); // compute metrics not dependent on active DAG
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // sample set definitions are enabled by reversing the DAG direction:
    const UShortArray& active_dag = *activeDAGIter;
    if (outputLevel >= QUIET_OUTPUT)
      Cout << "Evaluating active DAG:\n" << active_dag << std::endl;
    generate_reverse_dag(active_dag);
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    DAGSolutionData& soln = dagSolns[active_dag];
    compute_ratios(var_L, soln);
    update_best(soln); // store state for restoration
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best();
  ++mlmfIter;

  // No LF increments or final moments for pilot projection
  DAGSolutionData& soln = dagSolns[*activeDAGIter];
  update_projected_samples(soln.avgHFTarget, soln.avgEvalRatios, N_H_actual,
			   N_H_alloc, deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
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
    // Here, use of DAG reverse dependencies to define sample sets is overkill:
    // > must track all recursive dependencies to preserve the pyramid sample
    //   reuse --> all branches from a root node share a sample increment, not
    //   just direct connections captured in reverseActiveDAG[root],
    //   complicating the active model set definition for numSamples.
    // > Base class approach of reordering based on avg_eval_ratios is simpler
    //   and only requires virtual implementation of precompute_acv_control()
    //   and compute_acv_control_mq() downstream.
    NonDACVSampling::
      approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
			N_H_alloc, avg_eval_ratios, avg_hf_target);
    break;
  default: {
    // IS/RD sample management utilizes DAG tracking: in this case, sample set
    // dependencies are sufficiently captured by reverseActiveDAG[root]
    IntRealMatrixMap  sum_L_refined = sum_L_baselineH;//baselineL;
    Sizet2DArray  N_L_actual_shared;  inflate(N_H_actual, N_L_actual_shared);
    Sizet2DArray N_L_actual_refined = N_L_actual_shared;
    SizetArray    N_L_alloc_refined;  inflate(N_H_alloc, N_L_alloc_refined);

    // Process shared sample increments based on the ordered list of roots, but
    // skip initial root = numApprox as it is out of bounds for r* and LF counts
    UShortList::iterator it;  unsigned short root;
    for (it=++orderedRootList.begin(); it!=orderedRootList.end(); ++it) {
      root = *it;  const UShortSet& reverse_dag = reverseActiveDAG[root];
      // *** TO DO NON_BLOCKING: PERFORM PASS 2 ACCUMULATE AFTER PASS 1 LAUNCH
      if (genacv_approx_increment(avg_eval_ratios, N_L_actual_refined,
				  N_L_alloc_refined, avg_hf_target, mlmfIter,
				  root, reverse_dag)) {
	// ACV_IS/RD samples on [approx-1,approx) --> sum_L_refined
	// ACV_MF    samples on [0, approx)       --> sum_L_refined
	accumulate_acv_sums(sum_L_refined, N_L_actual_refined, root,
			    reverse_dag);
	increment_equivalent_cost(numSamples, sequenceCost, root,
				  reverse_dag, equivHFEvals);
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
			const UShortSet& reverse_dag)
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
    increment_sample_range(N_L_alloc_refined, N_alloc, root, reverse_dag);
  }
  else {
    size_t lf_curr = N_L_alloc_refined[root];
    numSamples = one_sided_delta((Real)lf_curr, lf_target);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << numSamples << " for root node index "
	   << root << " computed from delta between LF target = " << lf_target
	   << " and current allocation = " << lf_curr << std::endl;
    increment_sample_range(N_L_alloc_refined, numSamples, root, reverse_dag);
  }

  // the approximation sequence can be managed within one set of jobs using
  // a composite ASV with NonHierarchSurrModel
  return approx_increment(iter, root, reverse_dag);
}


void NonDGenACVSampling::precompute_ratios()
{
  cache_mc_reference();   // store {estVar,numH}Iter0
  approxSequence.clear(); // rho2LH re-ordering from MFMC is not relevant here
}


void NonDGenACVSampling::
compute_ratios(const RealMatrix& var_L, DAGSolutionData& soln)
{
  // --------------------------------------
  // Configure the optimization sub-problem
  // --------------------------------------
  // Set initial guess based either on related analytic solutions (iter == 0)
  // or warm started from previous solution (iter >= 1)

  RealVector& avg_eval_ratios = soln.avgEvalRatios;
  bool     budget_constrained = (maxFunctionEvals != SZ_MAX);
  if (mlmfIter == 0) {
    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
    Real budget            = (Real)maxFunctionEvals;
    bool budget_exhausted  = (equivHFEvals >= budget);
    // Modify budget to allow a feasible soln (var lower bnds: r_i > 1, N > N_H)
    // Can happen if shared pilot rolls up to exceed budget spec.
    //if (budget_exhausted) budget = equivHFEvals;

    if (budget_exhausted || convergenceTol >= 1.) { // no need for solve
      if (avg_eval_ratios.empty()) avg_eval_ratios.sizeUninitialized(numApprox);
      avg_eval_ratios = 1.;  soln.avgHFTarget = avg_N_H;
      soln.avgEstVar = average(estVarIter0);  soln.avgEstVarRatio = 1.;
      // For r_i = 1, C_G,c_g = 0 --> enforce constr for downstream CV numerics
      enforce_linear_ineq_constraints(avg_eval_ratios, orderedRootList);
      numSamples = 0;  return;
    }

    // Run a competition among related analytic approaches (MFMC or pairwise
    // CVMC) for best initial guess, where each initial gues may additionally
    // employ multiple varianceMinimizers in ensemble_numerical_solution()
    covariance_to_correlation_sq(covLH, var_L, varH, rho2LH);
    DAGSolutionData mf_soln, cv_soln;  size_t mf_samp, cv_samp;
    analytic_initialization_from_mfmc(avg_N_H, mf_soln);
    analytic_initialization_from_ensemble_cvmc(*activeDAGIter,avg_N_H, cv_soln);
    ensemble_numerical_solution(sequenceCost, approxSequence, mf_soln, mf_samp);
    ensemble_numerical_solution(sequenceCost, approxSequence, cv_soln, cv_samp);
    pick_mfmc_cvmc_solution(mf_soln, mf_samp, cv_soln, cv_samp,soln,numSamples);
  }
  else { // warm start from previous eval_ratios solution

    // no scaling needed from prev soln (as in NonDLocalReliability) since
    // updated avg_N_H now includes allocation from previous solution and
    // should be active on constraint bound (excepting sample count rounding)

    // warm start from previous solns for corresponding DAG
    ensemble_numerical_solution(sequenceCost, approxSequence, soln, numSamples);
  }

  if (outputLevel >= NORMAL_OUTPUT)
    print_computed_solution(Cout, soln);
}


void NonDGenACVSampling::
analytic_initialization_from_ensemble_cvmc(const UShortArray& dag, Real avg_N_H,
					   DAGSolutionData& soln)
{
  // For general DAG, set initial guess based on pairwise CVMC analytic solns
  // (analytic MFMC soln expected to be less relevant).  Differs from derived
  // ACV approach through use of paired DAG dependencies.
  cvmc_ensemble_solutions(covLL, covLH, varH, sequenceCost, dag, soln);

  if (maxFunctionEvals == SZ_MAX) {
    // scale according to accuracy (convergenceTol * estVarIter0)
    enforce_linear_ineq_constraints(soln.avgEvalRatios, orderedRootList);
    soln.avgHFTarget = update_hf_target(soln.avgEvalRatios, varH, estVarIter0);
  }
  else { // scale according to cost
    scale_to_target(avg_N_H, sequenceCost, soln.avgEvalRatios, soln.avgHFTarget,
		    orderedRootList); // incorporates lin ineq enforcement
    RealVector cd_vars;
    r_and_N_to_design_vars(soln.avgEvalRatios, soln.avgHFTarget, cd_vars);
    soln.avgEstVar = average_estimator_variance(cd_vars); // ACV or GenACV
  }
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "GenACV scaled initial guess from ensemble CVMC:\n"
	 << "  average eval ratios:\n" << soln.avgEvalRatios
	 << "  average HF target = " << soln.avgHFTarget << std::endl;
}


void NonDGenACVSampling::
cvmc_ensemble_solutions(const RealSymMatrixArray& cov_LL,
			const RealMatrix& cov_LH, const RealVector& var_H,
			const RealVector& cost,   const UShortArray& dag,
			DAGSolutionData& soln)
{
  RealVector& avg_eval_ratios = soln.avgEvalRatios;
  if (avg_eval_ratios.empty()) avg_eval_ratios.size(numApprox);
  else                         avg_eval_ratios = 0.;

  // First pass: compute an ensemble of pairwise CVMC solutions.  Note that
  // the computed eval ratios are no longer bound to the HF node.
  size_t qoi, source, target;
  Real cost_ratio, rho_sq, cost_H = cost[numApprox], var_L_qs, var_L_qt,
    cov_LH_qs, cov_LL_qst;
  for (source=0; source<numApprox; ++source) {
    target = dag[source];
    cost_ratio = cost[target] / cost[source];
    //Cout << "CVMC source " << source << " target " << target
    //     << " cost ratio = " << cost_ratio << ":\n";
    Real& avg_eval_ratio = avg_eval_ratios[source];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      const RealSymMatrix& cov_LL_q = cov_LL[qoi];
      var_L_qs = cov_LL_q(source,source);
      if (target == numApprox) {
	cov_LH_qs = cov_LH(qoi,source);
	rho_sq    = cov_LH_qs / var_L_qs * cov_LH_qs / var_H[qoi];
	//Cout << "  QoI " << qoi << ": cov_LH " << cov_LH_qs << " var_L "
	//     << var_L_qs << " var_H " << var_H[qoi] << " rho^2 = " << rho_sq
	//     << std::endl;
      }
      else {
	cov_LL_qst = cov_LL_q(source,target);
	var_L_qt   = cov_LL_q(target,target);
	rho_sq     = cov_LL_qst / var_L_qs * cov_LL_qst / var_L_qt;
	//Cout << "  QoI " << qoi << ": cov_LL " << cov_LL_qst << " var_L "
	//     << var_L_qs << " var_L " << var_L_qt << " rho^2 = " << rho_sq
	//     << std::endl;
      }
      if (rho_sq < 1.) // prevent div by 0, sqrt(negative)
	avg_eval_ratio += std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      else // should not happen
	avg_eval_ratio += std::sqrt(cost_ratio / Pecos::SMALL_NUMBER);
    }
    avg_eval_ratio /= numFunctions;
  }

  // Second pass: convert pairwise ratios to root-node ratios by rolling up
  // the oversample factors across the ordered root list.  Skip the first
  // target = numApprox for which r_tgt = 1.
  UShortList::const_iterator r_cit; UShortSet::const_iterator d_cit; Real r_tgt;
  for (r_cit=++orderedRootList.begin(); r_cit!=orderedRootList.end(); ++r_cit) {
    target = *r_cit;  const UShortSet& reverse_dag = reverseActiveDAG[target];
    r_tgt = avg_eval_ratios[target];
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit)
      { source = *d_cit;  avg_eval_ratios[source] *= r_tgt; }
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
  UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
  unsigned short source, target;  Real r_tgt;
  for (r_cit=root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
    target = *r_cit;  const UShortSet& reverse_dag = reverseActiveDAG[target];
    r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[target];
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
      source = *d_cit;  Real& r_src = avg_eval_ratios[source];
      if (r_src <= r_tgt) {
	r_src = r_tgt * (1. + RATIO_NUDGE);
	if (outputLevel >= DEBUG_OUTPUT)
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

  if (pilotMgmtMode == OFFLINE_PILOT) {
    Real offline_N_lwr = 2.;
    if (avg_N_H < offline_N_lwr) avg_N_H = offline_N_lwr;
  }
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
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;
    Real r_tgt, cost_r_src, budget_decr, inner_prod_decr;
    for (r_cit=root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
      target = *r_cit; const UShortSet& reverse_dag = reverseActiveDAG[target];
      r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[target];
      budget_decr = inner_prod_decr = 0.;
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit; Real& r_src = avg_eval_ratios[source];
	r_src *= factor;
	if (r_src <= 1.) {
	  r_src = r_tgt * (1. + RATIO_NUDGE);
	  if (outputLevel >= DEBUG_OUTPUT)
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
	   << "avg_hf_target = " << avg_hf_target << " budget = "
	   << avg_N_H * inner_prod / cost_H << std::endl;
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
    compute_parameterized_G_g(N_vec);
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: { // convert r_and_N to N_vec:
    RealVector N_vec;  copy_data(cd_vars, N_vec);
    N_H = N_vec[numApprox];
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec);
    break;
  }
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
    compute_parameterized_G_g(cd_vars);
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


void NonDGenACVSampling::compute_parameterized_G_g(const RealVector& N_vec)
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
  Real bi, bj, z1_i, z2_i;
  const UShortArray& active_dag = *activeDAGIter;
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    Real z_i, z_j, zi_zj, z1_j;
    for (i=0; i<numApprox; ++i) {
      bi = active_dag[i];  z_i = N_vec[i];  z1_i = z1[i];  z2_i = z2[i];
      gVec[i] = (bi == numApprox) ? 1./z1_i - 1./z_i : 0.;
      for (j=0; j<=i; ++j) {
	bj = active_dag[j];  z_j = N_vec[j];  z1_j = z1[j];  //z2_j = z2[j];
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
    Real z1_j, z2_j, z_H = N_vec[numApprox];
    for (i=0; i<numApprox; ++i) {
      bi = active_dag[i];
      z1_i = /*z2_bi = z_bi =*/ N_vec[bi];  z2_i = /*z_i =*/ N_vec[i];
      gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<=i; ++j) {
	bj = active_dag[j];
	z1_j = /*z2_bj = z_bj =*/ N_vec[bj];  z2_j = /*z_j =*/ N_vec[j];
	GMat(i,j)
	  = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
	  + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: { // Bomarito Eqs. 19-20
    RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    for (i=0; i<numApprox; ++i) {
      bi = active_dag[i];  z1_i = z1[i];  z2_i = z2[i];
      gVec[i] = (bi == numApprox) ? 1./z1_i : 0.;
      for (j=0; j<=i; ++j) {
	bj = active_dag[j];  //z1_j = z1[j];  z2_j = z2[j];
	GMat(i,j) = 0.;
	if (bi == bj)  GMat(i,j) += 1./z1_i;
	if (bi ==  j)  GMat(i,j) -= 1./z1_i;  // always false for dag = M
	if ( i == bj)  GMat(i,j) -= 1./z1[j]; // always false for dag = M
	if ( i ==  j)  GMat(i,j) += 1./z2_i;
      }
    }
    break;
  }
  default:
    Cerr << "Error: bad sub-method name (" << mlmfSubMethod << ") in NonDGen"
	 << "ACVSampling::compute_parameterized_G_g()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "For dag:\n"  << active_dag  << "G matrix:\n" << GMat
	 << "g vector:\n" << gVec << std::endl;
}


void NonDGenACVSampling::
unroll_z1_z2(const RealVector& N_vec, RealVector& z1, RealVector& z2)
{
  //Real z_H = N_vec[numApprox];
  z1.size(numApprox);  z2.size(numApprox+1);  z2[numApprox] = N_vec[numApprox];

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: case SUBMETHOD_ACV_RD: {
    /* Initial approach generated from scratch:
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
	z2[dag_curr] = N_vec[dag_curr] - z1[dag_curr]; // IS/RD
	dag_next = dag_curr;
      }
    }
    */

    // Preferred: leverage reverseActiveDAG and orderedRootList:
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;  Real z2t;
    for (r_cit=orderedRootList.begin(); r_cit!=orderedRootList.end(); ++r_cit) {
      target = *r_cit;  z2t = z2[target];
      const UShortSet& reverse_dag = reverseActiveDAG[target];
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit;  Real& z1s = z1[source];
	z1s = z2t;  z2[source] = N_vec[source] - z1s; // IS/RD (not MF pyramid)
      }
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // not used (special unroll logic not required)
    const UShortArray& active_dag = *activeDAGIter;  unsigned short i, bi;
    for (i=0; i<numApprox; ++i)
      { bi = active_dag[i];  z1[i] = N_vec[bi];  z2[i] = N_vec[i]; }
    break;
  }
  default:
    Cerr << "Error: unsupported mlmfSubMethod in unroll_z1_z2()" << std::endl;
    abort_handler(METHOD_ERROR);  break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "GenACV-IS/RD unroll of N_vec:\n" << N_vec << "into z1:\n" << z1
	 << "and z2:\n" << z2 << std::endl;
}


void NonDGenACVSampling::update_best(DAGSolutionData& soln)
{
  // Update tracking of best result

  bool update = false;
  if (bestDAGIter == modelDAGs.end())
    update = true;
  else {
    DAGSolutionData& best_soln = dagSolns[*bestDAGIter];
    Real avg_est_var = soln.avgEstVar;
    if (!valid_variance(avg_est_var)) // *** TO DO: problems could be hidden due to averaging --> consider a finer-grained badNumericsFlag triggered per QoI
      update = false;
    else if (maxFunctionEvals == SZ_MAX)
      update = (soln.equivHFAlloc < best_soln.equivHFAlloc);
    else
      update = (avg_est_var       < best_soln.avgEstVar);
  }
  if (update) {
    bestDAGIter = activeDAGIter;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Updating best DAG to:\n" << *bestDAGIter << std::endl;
  }
}


void NonDGenACVSampling::restore_best()
{
  if (bestDAGIter == modelDAGs.end()) {
    Cout << "Warning: best DAG has not been updated in restore_best().\n"
	 << "         Last active DAG will be used." << std::endl;
    return;
  }

  const UShortArray& best_dag = *bestDAGIter;
  //if (outputLevel > SILENT_OUTPUT)
    Cout << "\nBest solution from DAG:\n" << best_dag << std::endl;
  if (outputLevel >= DEBUG_OUTPUT) {
    DAGSolutionData& soln = dagSolns[best_dag];
    Cout << "\nwith avg_eval_ratios =\n" << soln.avgEvalRatios
	 << "and avg_hf_target = "       << soln.avgHFTarget << std::endl;
  }

  // restore best state for compute/archive/print final results
  if (activeDAGIter != bestDAGIter) { // best is not most recent
    activeDAGIter = bestDAGIter;
    if (pilotMgmtMode != PILOT_PROJECTION && finalStatsType == QOI_STATISTICS)
      //&& mlmfSubMethod != SUBMETHOD_ACV_MF) // approx_increments() for IS/RD
      generate_reverse_dag(best_dag);
  }
}

} // namespace Dakota
