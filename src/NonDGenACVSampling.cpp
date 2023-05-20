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
  modelSelectType(ALL_MODEL_COMBINATIONS),//(NO_MODEL_SELECTION)
  meritFnStar(DBL_MAX)
{
  // Unless the ensemble changes, the set of admissible DAGS is invariant:
  if (dagRecursionType == FULL_GRAPH_RECURSION) dagDepthLimit = numApprox;
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


void NonDGenACVSampling::generate_dags()
{
  UShortArray nodes(numApprox), dag;  size_t i;
  for (i=0; i<numApprox; ++i) nodes[i] = i;
  unsigned short root = numApprox;

  /* For verification of consistency with MFMC,ACV:
  //dag.resize(numApprox); for (i=0; i<numApprox; ++i) dag[i] = i+1; // MFMC
  dag.assign(numApprox, numApprox); // ACV
  modelDAGs[nodes].insert(dag);
  return;
  */

  // zero root directed acyclic graphs
  switch (dagRecursionType) {
  case KL_GRAPH_RECURSION: {
    // ***
    // *** TO DO: add model set enumeration for ACV-KL as well...
    // ***

    size_t K, L, M_minus_K;
    // Dakota low-to-high ordering (enumerate valid KL values and convert)
    UShortArraySet& dag_set = modelDAGs[nodes];  dag_set.clear();
    dag.resize(numApprox);
    for (K=0; K<=numApprox; ++K) {
      M_minus_K = numApprox - K;

      // K highest fidelity approximations target root (numApprox):
      for (i=M_minus_K; i<numApprox; ++i) dag[i] = root;
      // JCP ordering: for (i=0; i<K; ++i) dag[i] = 0; // root = 0

      for (L=0; L<=K; ++L) { // ordered single recursion
	// M-K lowest fidelity approximations target root - L:
	for (i=0; i<M_minus_K; ++i)       dag[i] = root - L;
	// JCP ordering: for (i=K; i<numApprox; ++i) dag[i] = L;

	dag_set.insert(dag);
      }
    }
    break;
  }
  default:
    switch (modelSelectType) {
    case ALL_MODEL_COMBINATIONS: {
      // tensor product of order 1 to enumerate approximation inclusion
      UShort2DArray tp;  UShortArray tp_orders(numApprox, 1);
      Pecos::SharedPolyApproxData::
	tensor_product_multi_index(tp_orders, tp, true);
      //Cout << "tp mi:\n" << tp;
      size_t j, num_tp = tp.size();  unsigned short dag_size;

      // Two enumeration options here:
      // > map<UShortArray,UShortArraySet> to associate pruned node sets to DAGs
      // > keep node set size at numApprox and use USHRT_MAX for omitted models

      // Option 1:
      // Pre-compute generate_sub_trees() for each dag_size (0:numApprox)
      std::vector<UShortArraySet> nominal_dags(numApprox+1); // include size 0
      for (j=0; j<numApprox; ++j) nodes[j] = j;
      for (dag_size=numApprox; dag_size>=1; --dag_size) {
	nodes.resize(dag_size); // discard trailing node
	dag.assign(dag_size, USHRT_MAX);
	generate_sub_trees(dag_size, nodes, std::min(dag_size, dagDepthLimit),
			   dag, nominal_dags[dag_size]);
      }
      // Now map each set of active model nodes through the nominal dags
      UShortArray mapped_dag;
      UShortArraySet::const_iterator cit;  unsigned short nom_dag_j;
      for (i=0; i<num_tp; ++i) { // include first = {0} --> retain MC case
	const UShortArray& tp_i = tp[i];
	nodes.clear();
	for (j=0; j<numApprox; ++j)
	  if (tp_i[j]) nodes.push_back(j);
	dag_size = nodes.size();
	mapped_dag.resize(dag_size);
	const UShortArraySet& nominal_set = nominal_dags[dag_size];
	UShortArraySet& mapped_set = modelDAGs[nodes];	mapped_set.clear();
	for (cit=nominal_set.begin(); cit!=nominal_set.end(); ++cit) {
	  const UShortArray& nominal_dag = *cit;
	  for (j=0; j<dag_size; ++j) {
	    nom_dag_j = nominal_dag[j];
	    mapped_dag[j] = (nom_dag_j < dag_size) ? nodes[nom_dag_j] : root;
	  }
	  //Cout << "nominal_dag =\n" << nominal_dag
	  //     << "mapped_dag  =\n" << mapped_dag << std::endl;
	  mapped_set.insert(mapped_dag);
	}	  
	Cout << "For model set:\n" << nodes
	     << "searching array of DAGs of size " << mapped_set.size();
	if (outputLevel >= DEBUG_OUTPUT) Cout << ":\n" << mapped_set;//<< '\n';
	else                             Cout << ".\n";
      }
      Cout << std::endl;

      /* Option 2:
      nodes.resize(numApprox);
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
      */

      //exit(0); // not yet supported downstream

      break;
    }

    // Rather then enumerating all of the discrete model combinations, this
    // employs one intgrated continuous solve, which approaches the best
    // discrete solution as some sample increments --> 0.  This approach will
    // likely encounter more issues with numerical conditioning
    case NO_MODEL_SELECTION:
      // root node (truth index = numApprox) and set of dependent nodes
      // (approximation model indices 0,numApprox-1) are fixed
      for (i=0; i<numApprox; ++i) nodes[i] = i;
      dag.assign(numApprox, USHRT_MAX);
      // recur for DAG including all approximations:
      UShortArraySet& dag_set = modelDAGs[nodes];  dag_set.clear();
      generate_sub_trees(root, nodes, dagDepthLimit, dag, dag_set);
      Cout << "Searching array of model DAGs of size " << dag_set.size();
      if (outputLevel >= DEBUG_OUTPUT) Cout << ":\n" << dag_set;
      else                             Cout << ".\n";
      Cout << std::endl;
      break;
    }
    break;
  }
}


void NonDGenACVSampling::
generate_sub_trees(unsigned short root, const UShortArray& nodes,
		   unsigned short depth, UShortArray& dag,
		   UShortArraySet& dag_set)
{
  // Modeled after Python DAG generator on stack overflow:
  // https://stackoverflow.com/questions/52061669/how-to-generate-all-
  //   trees-having-n-nodes-and-m-level-depth-the-branching-factor

  //Cout << "\nRecurring into generate_sub_trees(): depth = " << depth
  //     << " root = " << root << " nodes =\n" << nodes << std::endl;

  size_t i, num_nodes = nodes.size();
  if (nodes.empty()) {
    // suppress some duplicates for sub_nodes with empty sub_nodes_per_root
    //dag_set.insert(dag);
    return;
  }
  else if (depth <= 1) {
    for (i=0; i<num_nodes; ++i) dag[nodes[i]] = root;
    //Cout << "Recursion hit bottom.  Inserting:\n" << dag;
    dag_set.insert(dag); // insert completed DAG
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
      dag_set.insert(dag); // insert completed DAG
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
			     dag_set);
      }
    }
  }
}


void NonDGenACVSampling::
generate_reverse_dag(const UShortArray& model_set, const UShortArray& dag)
{
  // define an array of source nodes that point to a given target
  size_t i, dag_size = dag.size();
  reverseActiveDAG.clear();
  reverseActiveDAG.resize(numApprox + 1);
  unsigned short dag_curr, dag_next;
  for (i=0; i<dag_size; ++i) { // walk+store path to root
    dag_curr = model_set[i];  dag_next = dag[dag_curr];
    reverseActiveDAG[dag_next].insert(dag_curr);
    while (dag_next != numApprox) {
      dag_curr = dag_next;  dag_next = dag[dag_curr];
      reverseActiveDAG[dag_next].insert(dag_curr);
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In generate_reverse_dag(), reverse DAG:\n" << reverseActiveDAG
	 << std::endl;
}


void NonDGenACVSampling::
unroll_reverse_dag_from_root(unsigned short root, UShortList& root_list)
{
  // create an ordered list of roots that enable ordered sample increments
  // by ensuring that root sample levels are defined
  root_list.clear();  root_list.push_back(root);
  UShortList::iterator it = root_list.begin();
  while (it != root_list.end()) {
    const UShortSet& reverse_dag = reverseActiveDAG[*it];
    // append all dependent nodes (by default, use order of decreasing model
    // index = decreasing fidelity)
    root_list.insert(root_list.end(), reverse_dag.rbegin(), reverse_dag.rend());
    ++it;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In unroll_reverse_dag_from_root(), root list:\n"
	 << root_list << std::endl;
}


void NonDGenACVSampling::
unroll_reverse_dag_from_root(unsigned short root,
			     const RealVector& avg_eval_ratios,
			     UShortList& root_list)
{
  // start from default unroll ordering to extract all nodes that connect
  // to the specified root
  UShortList default_root_list;
  unroll_reverse_dag_from_root(root, default_root_list);

  // use a std::map to order nodes based on increasing r_i
  std::map<Real, unsigned short> root_ratios;
  UShortList::iterator l_it = default_root_list.begin();
  unsigned short node;  Real r_i;
  for (; l_it != default_root_list.end(); ++l_it) {
    node = *l_it;
    r_i = (node == root) ? 1. : avg_eval_ratios[node];
    root_ratios[r_i] = node;
  }
  // this ordered root_list intermingles dependency paths for purposes of
  // nesting sample sets, but reverseActiveDAG maintains dependencies
  std::map<Real, unsigned short>::iterator m_it = root_ratios.begin();
  root_list.clear();
  for (; m_it != root_ratios.end(); ++m_it)
    root_list.push_back(m_it->second);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In unroll_reverse_dag_from_root(), root list:\n"
	 << root_list << std::endl;
}


void NonDGenACVSampling::pre_run()
{
  NonDNonHierarchSampling::pre_run();

  // For hyper-parameter/design/state changes, can reuse modelDAGs.  Other
  // changes ({dagRecursion,modelSelect}Type) would induce need to regenerate.
  if (modelDAGs.empty())
    generate_dags();

  meritFnStar = DBL_MAX;
  bestModelSetIter = modelDAGs.end();
  dagSolns.clear();
}


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
  std::pair<UShortArray, UShortArray> soln_key;

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
    for (activeModelSetIter  = modelDAGs.begin();
	 activeModelSetIter != modelDAGs.end(); ++activeModelSetIter) {
      //  TO DO: create views of covLL,covLH or reuse existing indexing?
      const UShortArray&  model_set = activeModelSetIter->first;
      const UShortArraySet& dag_set = activeModelSetIter->second;
      soln_key.first = model_set;
      for (activeDAGIter  = dag_set.begin();
	   activeDAGIter != dag_set.end(); ++activeDAGIter) {
	// sample set definitions are enabled by reversing the DAG direction:
	const UShortArray& active_dag = *activeDAGIter;
	soln_key.second  = active_dag;
	if (outputLevel >= QUIET_OUTPUT)
	  Cout << "Evaluating active DAG:\n" << active_dag
	       << "for model set:\n" << model_set << std::endl;
	generate_reverse_dag(model_set, active_dag);
	// compute the LF/HF evaluation ratios from shared samples and compute
	// ratio of MC and ACV mean sq errors (which incorporates anticipated
	// variance reduction from application of avg_eval_ratios).
	DAGSolutionData& soln = dagSolns[soln_key];
	compute_ratios(var_L, soln);
	update_best(soln);// store state for restoration
	//reset_acv(); // reset state for next ACV execution
      }
    }
    restore_best();
    soln_key.first  = activeModelSetIter->first;
    soln_key.second = *activeDAGIter;
    if (truthFixedByPilot) numSamples = 0;
    else {
      avg_hf_target = dagSolns[soln_key].avgHFTarget;
      avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
      numSamples = one_sided_delta(avg_N_H, avg_hf_target);
    }
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  DAGSolutionData& soln = dagSolns[soln_key];
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, soln);
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
  std::pair<UShortArray, UShortArray> soln_key;

  precompute_ratios(); // compute metrics not dependent on active DAG
  for (activeModelSetIter  = modelDAGs.begin();
       activeModelSetIter != modelDAGs.end(); ++activeModelSetIter) {
    const UShortArray&  model_set = activeModelSetIter->first;
    const UShortArraySet& dag_set = activeModelSetIter->second;
    soln_key.first = model_set;
    for (activeDAGIter  = dag_set.begin();
	 activeDAGIter != dag_set.end(); ++activeDAGIter) {
      // sample set definitions are enabled by reversing the DAG direction:
      const UShortArray& active_dag = *activeDAGIter;
      soln_key.second  = active_dag;
      if (outputLevel >= QUIET_OUTPUT)
	Cout << "Evaluating active DAG:\n" << active_dag << "for node set:\n"
	     << soln_key.first << std::endl;
      generate_reverse_dag(model_set, active_dag);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      DAGSolutionData& soln = dagSolns[soln_key];
      compute_ratios(var_L, soln);
      update_best(soln); // store state for restoration
      //reset_acv(); // reset state for next ACV execution
    }
  }
  restore_best();
  ++mlmfIter;

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // Only QOI_STATISTICS requires online shared/approx profile evaluation for
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  soln_key.first  = activeModelSetIter->first;
  soln_key.second = *activeDAGIter;
  DAGSolutionData& soln = dagSolns[soln_key];
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
		      N_H_alloc, soln);
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
  std::pair<UShortArray, UShortArray> soln_key;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  precompute_ratios(); // compute metrics not dependent on active DAG
  for (activeModelSetIter  = modelDAGs.begin();
       activeModelSetIter != modelDAGs.end(); ++activeModelSetIter) {
    //  TO DO: create views of covLL,covLH or reuse existing indexing?
    const UShortArray&  model_set = activeModelSetIter->first;
    const UShortArraySet& dag_set = activeModelSetIter->second;
    soln_key.first = model_set;
    for (activeDAGIter  = dag_set.begin();
	 activeDAGIter != dag_set.end(); ++activeDAGIter) {
      // sample set definitions are enabled by reversing the DAG direction:
      const UShortArray& active_dag = *activeDAGIter;
      soln_key.second  = active_dag;
      if (outputLevel >= QUIET_OUTPUT)
	Cout << "Evaluating active DAG:\n" << active_dag << std::endl;
      generate_reverse_dag(model_set, active_dag);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      DAGSolutionData& soln = dagSolns[soln_key];
      compute_ratios(var_L, soln);
      update_best(soln); // store state for restoration
      //reset_acv(); // reset state for next ACV execution
    }
  }
  restore_best();
  ++mlmfIter;

  // No LF increments or final moments for pilot projection
  soln_key.first  = activeModelSetIter->first;
  soln_key.second = *activeDAGIter;
  DAGSolutionData& soln = dagSolns[soln_key];
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
		  const DAGSolutionData& soln)
{
  // ---------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ---------------------------------------------------------------
  // Note: for consistency with MFMC/ACV, r* is always defined relative to N_H,
  // even though an oversample may target a different model based on active DAG

  IntRealMatrixMap sum_L_shared  = sum_L_baselineH,//baselineL;
                   sum_L_refined = sum_L_baselineH;//baselineL;
  Sizet2DArray N_L_actual_shared;  inflate(N_H_actual, N_L_actual_shared);
  Sizet2DArray N_L_actual_refined = N_L_actual_shared;
  SizetArray   N_L_alloc_refined;  inflate(N_H_alloc,  N_L_alloc_refined);

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF: { // special handling for nested pyramid sampling
    SizetArray approx_sequence;  bool descending = true;
    const RealVector& avg_eval_ratios = soln.avgEvalRatios;
    ordered_approx_sequence(avg_eval_ratios, approx_sequence, descending);

    size_t start = 0, end;
    for (end=numApprox; end>0; --end) {
      // ACV_IS samples on [approx-1,approx) --> sum_L_refined
      // ACV_MF samples on [0, approx)       --> sum_L_refined
      //start = (mlmfSubMethod == SUBMETHOD_ACV_MF) ? 0 : end - 1;
      // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER PASS 1 LAUNCH
      if (acv_approx_increment(soln, N_L_actual_refined, N_L_alloc_refined,
			       mlmfIter, approx_sequence, start, end)) {
	increment_equivalent_cost(numSamples, sequenceCost, approx_sequence,
				  start, end, equivHFEvals);
	accumulate_acv_sums(sum_L_refined, N_L_actual_refined,
			    approx_sequence, start, end);
	accumulate_genacv_sums(sum_L_shared, N_L_actual_shared,
			       approx_sequence, start, end); // added rel to ACV
      }
    }
    break;
  }
  default: {
    // Process shared sample increments based on the ordered list of roots,
    // skipping the HF root = numApprox (this shared set already processed)
    // > This works for GenACV-MF pyramid sampling although the samples are not
    //   nested --> above, we instead augment the ACV-MF code for N_shared.
    UShortList::iterator it;  unsigned short root;  UShortSet mf_reverse_dag;
    for (it=++orderedRootList.begin(); it!=orderedRootList.end(); ++it) {
      root = *it;
      /*
      // ACV-MF: need full set of subordinate notes for pyramid sampling
      // ACV-IS: only need nodes with edges connected to root (reverse DAG)
      if (mlmfSubMethod == SUBMETHOD_ACV_MF) {
        UShortList partial_list;
        unroll_reverse_dag_from_root(root, soln.avgEvalRatios, partial_list);
        mf_reverse_dag.clear();
        mf_reverse_dag.insert(++partial_list.begin(), partial_list.end());
      }
      */
      const UShortSet& samp_reverse_dag =
	//(mlmfSubMethod == SUBMETHOD_ACV_MF) ? mf_reverse_dag :
	reverseActiveDAG[root];
      // *** TO DO NON_BLOCKING: PERFORM PASS 2 ACCUMULATE AFTER PASS 1 LAUNCH
      if (genacv_approx_increment(soln, N_L_actual_refined, N_L_alloc_refined,
				  mlmfIter, root, samp_reverse_dag)) {
	accumulate_genacv_sums(sum_L_shared, sum_L_refined, N_L_actual_shared,
			       N_L_actual_refined, root, samp_reverse_dag);
	increment_equivalent_cost(numSamples, sequenceCost, root,
				  samp_reverse_dag, equivHFEvals);
      }
    }
    break;
  }
  }

  // -----------------------------------------------------------
  // Compute/apply control variate parameter to estimate moments
  // -----------------------------------------------------------
  RealMatrix H_raw_mom(numFunctions, 4);
  genacv_raw_moments(sum_L_baselineH, sum_L_shared, sum_L_refined, sum_H,
		     sum_LL, sum_LH, soln.avgEvalRatios, N_L_actual_shared,
		     N_L_actual_refined, N_H_actual, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_actual_refined, N_L_alloc_refined);
}


bool NonDGenACVSampling::
genacv_approx_increment(const DAGSolutionData& soln,
			const Sizet2DArray& N_L_actual_refined,
			SizetArray& N_L_alloc_refined, size_t iter,
			unsigned short root, const UShortSet& reverse_dag)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts

  Real lf_target = soln.avgEvalRatios[root] * soln.avgHFTarget;
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
  // a composite ASV with EnsembleSurrModel
  return approx_increment(iter, root, reverse_dag);
}


void NonDGenACVSampling::precompute_ratios()
{
  if (pilotMgmtMode != OFFLINE_PILOT) cache_mc_reference();// {estVar,numH}Iter0
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

  if (mlmfIter == 0) {
    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
    bool budget_exhausted  = (maxFunctionEvals != SZ_MAX &&
			      equivHFEvals >= (Real)maxFunctionEvals);

    // use default ordering for now, prior to avgEvalRatios soln
    // > sufficient for determination of best DAG, but not for pyramid sample
    //   set ordering in approx_increments()
    unroll_reverse_dag_from_root(numApprox, orderedRootList);

    if (budget_exhausted || convergenceTol >= 1.) { // no need for solve
      RealVector& avg_eval_ratios = soln.avgEvalRatios;
      if (avg_eval_ratios.empty()) avg_eval_ratios.sizeUninitialized(numApprox);
      avg_eval_ratios = 1.;  soln.avgHFTarget = avg_N_H;
      // For offline pilot, the online EstVar is undefined prior to any online
      // samples, but should not happen (no budget used) unless bad convTol spec
      soln.avgEstVar = (pilotMgmtMode == OFFLINE_PILOT) ?
	std::numeric_limits<Real>::infinity() :	average(estVarIter0);
      soln.avgEstVarRatio = 1.;
      // For r_i = 1, C_G,c_g = 0 --> enforce constr for downstream CV numerics
      enforce_linear_ineq_constraints(avg_eval_ratios, orderedRootList);
      numSamples = 0;  return;
    }

    // Run a competition among related analytic approaches (MFMC or pairwise
    // CVMC) for best initial guess, where each initial gues may additionally
    // employ multiple varianceMinimizers in ensemble_numerical_solution()
    covariance_to_correlation_sq(covLH, var_L, varH, rho2LH);
    DAGSolutionData mf_soln, cv_soln;  size_t mf_samp, cv_samp;
    const UShortArray& model_set = activeModelSetIter->first;
    analytic_initialization_from_mfmc(model_set, avg_N_H, mf_soln);//***
    analytic_initialization_from_ensemble_cvmc(model_set, *activeDAGIter,
					       orderedRootList,avg_N_H,cv_soln);
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
analytic_initialization_from_mfmc(const UShortArray& model_set,
				  Real avg_N_H, DAGSolutionData& soln)
{
  /* *** TO DO
  // > Option 1 is analytic MFMC: differs from ACV due to recursive pairing
  if (ordered_approx_sequence(rho2LH)) // for all QoI across all Approx
    mfmc_analytic_solution(rho2LH, sequenceCost, soln);
  else // compute reordered MFMC for averaged rho; monotonic r not reqd
    mfmc_reordered_analytic_solution(rho2LH, sequenceCost, approxSequence,
				     soln, false);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from analytic MFMC (unscaled eval ratios):\n"
	 << soln.avgEvalRatios << std::endl;
  // any rho2_LH re-ordering from MFMC initial guess can be ignored (later
  // gets replaced with r_i ordering for approx_increments() sampling)
  approxSequence.clear();

  if (maxFunctionEvals == SZ_MAX)
    soln.avgHFTarget = update_hf_target(soln.avgEvalRatios, varH, estVarIter0);
  else
    scale_to_target(avg_N_H, sequenceCost, soln.avgEvalRatios,soln.avgHFTarget);
  */
}


void NonDGenACVSampling::
analytic_initialization_from_ensemble_cvmc(const UShortArray& model_set,
					   const UShortArray& dag,
					   const UShortList& root_list,
					   Real avg_N_H, DAGSolutionData& soln)
{
  // For general DAG, set initial guess based on pairwise CVMC analytic solns
  // (analytic MFMC soln expected to be less relevant).  Differs from derived
  // ACV approach through use of paired DAG dependencies.
  cvmc_ensemble_solutions(covLL, covLH, varH, sequenceCost, model_set, dag,
			  root_list, soln);

  if (maxFunctionEvals == SZ_MAX) {
    // scale according to accuracy (convergenceTol * estVarIter0)
    enforce_linear_ineq_constraints(soln.avgEvalRatios, root_list);
    soln.avgHFTarget = update_hf_target(soln.avgEvalRatios, varH, estVarIter0);//***
  }
  else { // scale according to cost
    scale_to_target(avg_N_H, sequenceCost, soln.avgEvalRatios, soln.avgHFTarget,
		    root_list); // incorporates lin ineq enforcement ***
    RealVector cd_vars;
    r_and_N_to_design_vars(soln.avgEvalRatios, soln.avgHFTarget, cd_vars); //***
    soln.avgEstVar = average_estimator_variance(cd_vars); // ACV or GenACV ***
  }
  if (outputLevel >= NORMAL_OUTPUT)
    Cout << "GenACV scaled initial guess from ensemble CVMC:\n"
	 << "  average eval ratios:\n" << soln.avgEvalRatios
	 << "  average HF target = " << soln.avgHFTarget << std::endl;
}


void NonDGenACVSampling::
cvmc_ensemble_solutions(const RealSymMatrixArray& cov_LL,
			const RealMatrix& cov_LH, const RealVector& var_H,
			const RealVector& cost,   const UShortArray& model_set,
			const UShortArray& dag,   const UShortList& root_list,
			DAGSolutionData& soln)
{
  size_t qoi, source, target, d, dag_size = dag.size();
  RealVector& avg_eval_ratios = soln.avgEvalRatios;
  if (avg_eval_ratios.length() != dag_size) avg_eval_ratios.size(dag_size);
  else                                      avg_eval_ratios = 0.;

  // First pass: compute an ensemble of pairwise CVMC solutions.  Note that
  // computed eval ratios are DAG-based and no longer bound to the HF node.
  Real cost_ratio, rho_sq, cost_H = cost[numApprox], var_L_qs, var_L_qt,
    cov_LH_qs, cov_LL_qst;
  for (d=0; d<dag_size; ++d) {
    source = model_set[d];  target = dag[source];
    cost_ratio = cost[target] / cost[source];
    //Cout << "CVMC source " << source << " target " << target
    //     << " cost ratio = " << cost_ratio << ":\n";
    Real& avg_eval_ratio = avg_eval_ratios[d];
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
  for (r_cit=++root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
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

  // *** TO DO: INFLATE cd_vars[dag_size+1] TO N_vec[numApprox+1] FOR CONVENIENCE --> allows (compressed) DAG source/target to be used as indices in N_vec/z1/z2

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


void NonDGenACVSampling::
accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined,
		       Sizet2DArray& N_L_shared, Sizet2DArray& N_L_refined,
		       unsigned short root, const UShortSet& reverse_dag)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  unsigned short node;
  IntRespMCIter r_it;  UShortSet::const_iterator d_cit;
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    // the  "shared" accumulations define the z_i^1 sample sets
    // the "refined" accumulations define the z_i^2 sample sets

    // refined only at root node:
    accumulate_acv_sums(sum_L_refined, N_L_refined, fn_vals, root);
    // refined + shared at dependent nodes:
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
      node = *d_cit;
      accumulate_acv_sums(sum_L_shared,  N_L_shared,  fn_vals, node);
      accumulate_acv_sums(sum_L_refined, N_L_refined, fn_vals, node);
    }
  }
}


void NonDGenACVSampling::
accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared, Sizet2DArray& N_L_shared,
		       const SizetArray& approx_sequence, size_t sequence_start,
		       size_t sequence_end)
{
  // Based on active DAG for [start,end), precompute the set of models for
  // which to update {sum,N}_L_shared with this sample increment
  // > the  "shared" accumulations define the z^1_s sample sets (here)
  // > the "refined" accumulations define the z^2_s sample sets (other fns)
  UShortSet accum_z1_sets;  size_t s;  unsigned short source, target;
  const UShortArray& active_dag = *activeDAGIter;
  if (approx_sequence.empty())
    for (s=sequence_start; s<sequence_end; ++s) {
      source = s; target = active_dag[s];
      // if the CV target for model s is part of the models being sampled,
      // then we will increment its z^1 "shared" accumulators
      if (target >= sequence_start && target < sequence_end)
	accum_z1_sets.insert(source);
    }
  else {
    UShortSet approx_set;
    for (s=sequence_start; s<sequence_end; ++s)
      approx_set.insert(approx_sequence[s]);
    for (s=sequence_start; s<sequence_end; ++s) {
      source = approx_sequence[s]; target = active_dag[source];
      if (approx_set.find(target) != approx_set.end())
	accum_z1_sets.insert(source);
    }
  }
  //Cout << "For sequence end = " << sequence_end
  //     << " accum_z1_sets =\n" << accum_z1_sets << std::endl;

  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest
  IntRespMCIter r_it;  UShortSet::iterator z1_it;
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();
    // shared z^1 at dependent nodes:
    for (z1_it=accum_z1_sets.begin(); z1_it!=accum_z1_sets.end(); ++z1_it)
      accumulate_acv_sums(sum_L_shared, N_L_shared, fn_vals, *z1_it);
    // *** TO DO: these accumulations are redundant --> prefer to accumulate
    // ***        once and then add contribution to multiple roll-ups
  }
}


void NonDGenACVSampling::
genacv_raw_moments(IntRealMatrixMap& sum_L_baseline,
		   IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined,  IntRealVectorMap& sum_H,
		   IntRealSymMatrixArrayMap& sum_LL, IntRealMatrixMap& sum_LH,
		   const RealVector& avg_eval_ratios,
		   const Sizet2DArray& N_L_shared,
		   const Sizet2DArray& N_L_refined,  const SizetArray& N_H,
		   RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  precompute_genacv_control(avg_eval_ratios, N_H);

  size_t approx, qoi, N_H_q;  Real sum_H_mq;
  RealVector beta(numApprox);
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix&     sum_L_base_m = sum_L_baseline[mom];
    RealMatrix&       sum_L_sh_m =   sum_L_shared[mom];
    RealMatrix&      sum_L_ref_m =  sum_L_refined[mom];
    RealVector&          sum_H_m =          sum_H[mom];
    RealSymMatrixArray& sum_LL_m =         sum_LL[mom];
    RealMatrix&         sum_LH_m =         sum_LH[mom];
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      sum_H_mq = sum_H_m[qoi];  N_H_q = N_H[qoi];
      compute_genacv_control(sum_L_base_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			     N_H_q, mom, qoi, beta);
      // *** TO DO: support shared_approx_increment() --> baselineL

      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1 << ": control "
	       << "variate beta = " << std::setw(9) << beta[approx] << '\n';
	// For ACV, shared counts are fixed at N_H for all approx
	apply_control(sum_L_sh_m(qoi,approx),  N_L_shared[approx][qoi],
		      sum_L_ref_m(qoi,approx), N_L_refined[approx][qoi],
		      beta[approx], H_raw_mq);
      }
    }
  }
  if (outputLevel >= NORMAL_OUTPUT) Cout << std::endl;
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

  // *** FOR NOW, ASSUMING N_vec IN FULL DIMENSION IS CONVENIENT ***

  const UShortArray& model_set  = activeModelSetIter->first;
  const UShortArray& active_dag = *activeDAGIter;
  size_t i, j, dag_size = activeDAGIter->size();
  if (GMat.empty()) GMat.shapeUninitialized(dag_size);
  if (gVec.empty()) gVec.sizeUninitialized(dag_size);

  // define sample recursion sets backwards (from z_H down to lowest fid)
  Real src_i, src_j, tgt_i, tgt_j, z1_i, z2_i, z1_j;
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    Real z_i, z_j, zi_zj;
    for (i=0; i<dag_size; ++i) {
      src_i = model_set[i];  tgt_i = active_dag[i];
      z_i = N_vec[src_i];  z1_i = z1[src_i];  z2_i = z2[src_i];
      gVec[i] = (tgt_i == numApprox) ? 1./z1_i - 1./z_i : 0.;
      for (j=0; j<=i; ++j) {
	src_j = model_set[j];  tgt_j = active_dag[j]; //bj = active_dag[j];
	z_j = N_vec[src_j];  z1_j = z1[src_j];  //z2_j = z2[src_j];
	GMat(i,j) = 0.;  zi_zj = z_i * z_j;
	if (tgt_i == tgt_j) GMat(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
	if (tgt_i == src_j) GMat(i,j) += z1_i/zi_zj - 1./z_j; // false for dag=M
	if (src_i == tgt_j) GMat(i,j) += z1_j/zi_zj - 1./z_i; // false for dag=M
	if (src_i == src_j) GMat(i,j) += z2_i/zi_zj;
      }
    }
    //RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    //Real z_i, z_j, zi_zj, z1_j;
    //for (i=0; i<numApprox; ++i) {
    //  bi = active_dag[i];  z_i = N_vec[i];  z1_i = z1[i];  z2_i = z2[i];
    //  gVec[i] = (bi == numApprox) ? 1./z1_i - 1./z_i : 0.;
    //  for (j=0; j<=i; ++j) {
    //    bj = active_dag[j];  z_j = N_vec[j];  z1_j = z1[j];  //z2_j = z2[j];
    // 	  GMat(i,j) = 0.;  zi_zj = z_i * z_j;
    // 	  if (bi == bj)  GMat(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
    // 	  if (bi ==  j)  GMat(i,j) += z1_i/zi_zj - 1./z_j; // false for dag = M
    // 	  if (i  == bj)  GMat(i,j) += z1_j/zi_zj - 1./z_i; // false for dag = M
    // 	  if (i  ==  j)  GMat(i,j) += z2_i/zi_zj;
    //  }
    //}
    break;
  }
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real z2_j, z_H = N_vec[dag_size]; // active LF approx followed by HF
    for (i=0; i<dag_size; ++i) {
      src_i = model_set[i];  tgt_i = active_dag[i]; // *** This indexing would be out of bounds if N_vec was collapsed
      z1_i = /*z2_bi = z_bi =*/ N_vec[tgt_i];  z2_i = /*z_i =*/ N_vec[src_i];
      gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<=i; ++j) {
	src_j = model_set[j];  tgt_j = active_dag[j];
	z1_j = /*z2_bj = z_bj =*/ N_vec[tgt_j];  z2_j = /*z_j =*/ N_vec[src_j];
	GMat(i,j)
	  = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
	  + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    /////////////////////////////////////////
    //Real z1_j, z2_j, z_H = N_vec[numApprox];
    //for (i=0; i<numApprox; ++i) {
    //  bi = active_dag[i];
    //  z1_i = /*z2_bi = z_bi =*/ N_vec[bi];  z2_i = /*z_i =*/ N_vec[i];
    //  gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
    //  for (j=0; j<=i; ++j) {
    //     bj = active_dag[j];
    // 	   z1_j = /*z2_bj = z_bj =*/ N_vec[bj];  z2_j = /*z_j =*/ N_vec[j];
    // 	   GMat(i,j)
    // 	     = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
    // 	     + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
    //  }
    //}
    break;
  }
  case SUBMETHOD_ACV_RD: { // Bomarito Eqs. 19-20
    RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    for (i=0; i<numApprox; ++i) {
      src_i = model_set[i];  tgt_i = active_dag[i];
      z1_i  = z1[src_i];     z2_i  = z2[src_i];
      gVec[i] = (tgt_i == numApprox) ? 1./z1_i : 0.;
      for (j=0; j<=i; ++j) {
	src_j = model_set[j];  tgt_j = active_dag[j];
	z1_j = z1[src_j];      GMat(i,j) = 0.;
	if (tgt_i == tgt_j) GMat(i,j) += 1./z1_i;
	if (tgt_i == src_j) GMat(i,j) -= 1./z1_i; // always false for dag = M
	if (src_i == tgt_j) GMat(i,j) -= 1./z1_j; // always false for dag = M
	if (src_i == src_j) GMat(i,j) += 1./z2_i;
      }
    }

    //RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    //for (i=0; i<numApprox; ++i) {
    //  bi = active_dag[i];  z1_i = z1[i];  z2_i = z2[i];
    //  gVec[i] = (bi == numApprox) ? 1./z1_i : 0.;
    //  for (j=0; j<=i; ++j) {
    //    bj = active_dag[j];  //z1_j = z1[j];  z2_j = z2[j];
    //    GMat(i,j) = 0.;
    //    if (bi == bj)  GMat(i,j) += 1./z1_i;
    //    if (bi ==  j)  GMat(i,j) -= 1./z1_i;  // always false for dag = M
    //    if ( i == bj)  GMat(i,j) -= 1./z1[j]; // always false for dag = M
    //    if ( i ==  j)  GMat(i,j) += 1./z2_i;
    //  }
    //}
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
  // *** FOR NOW, INFLATING N_vec,z1,z2 TO FULL DIMENSION IS CONVENIENT ***

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
    const UShortArray& model_set  = activeModelSetIter->first;
    const UShortArray& active_dag = *activeDAGIter;
    unsigned short i, source, target;  size_t dag_size = active_dag.size();
    for (i=0; i<dag_size; ++i) {
      source = model_set[i];  target = active_dag[i];
      //z1[i] = N_vec[target];  z2[i] = N_vec[source]; // COMPACTED z1,z2
      z1[source] = N_vec[target];  z2[source] = N_vec[source]; // FULL z1,z2
    }
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

  bool update = false;  Real merit_fn, avg_est_var = soln.avgEstVar;
  if (!valid_variance(avg_est_var)) // *** TO DO: problems could be hidden due to averaging --> consider a finer-grained badNumericsFlag triggered per QoI
    update = false;
  else {
    merit_fn = nh_penalty_merit(soln);
    update = (merit_fn < meritFnStar);
  }
  if (update) {
    bestModelSetIter = activeModelSetIter;
    bestDAGIter      = activeDAGIter;
    meritFnStar      = merit_fn;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Updating best DAG to:\n" << *bestDAGIter << " for model set:\n"
	   << activeModelSetIter->first << std::endl;
  }
}


void NonDGenACVSampling::restore_best()
{
  if (bestModelSetIter == modelDAGs.end()) {
    Cout << "Warning: best DAG has not been updated in restore_best().\n"
	 << "         Last active DAG will be used." << std::endl;
    return;
  }

  const UShortArray& best_models = bestModelSetIter->first;
  const UShortArray& best_dag    = *bestDAGIter;
  Cout << "\nBest solution from DAG:\n" << best_dag << " for model set:\n"
       << best_models << std::endl;
  std::pair<UShortArray, UShortArray> soln_key(best_models, best_dag);
  DAGSolutionData& best_soln = dagSolns[soln_key];
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nwith avg_eval_ratios =\n" << best_soln.avgEvalRatios
	 << "and avg_hf_target = "       << best_soln.avgHFTarget << std::endl;

  // restore best state for compute/archive/print final results
  if (activeModelSetIter != bestModelSetIter ||
      activeDAGIter      != bestDAGIter) { // best is not most recent
    activeModelSetIter = bestModelSetIter;
    activeDAGIter      = bestDAGIter;
    if (pilotMgmtMode != PILOT_PROJECTION && finalStatsType == QOI_STATISTICS) {
      //&& mlmfSubMethod != SUBMETHOD_ACV_MF) // approx_increments() for IS/RD
      generate_reverse_dag(best_models, best_dag);
      // now we can re-order roots based on final eval ratios solution for
      // evaluation of approx increments
      unroll_reverse_dag_from_root(numApprox, best_soln.avgEvalRatios,
				   orderedRootList);
    }
  }
}

} // namespace Dakota
