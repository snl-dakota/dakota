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
  dagRecursionType(
    problem_db.get_short("method.nond.search_model_graphs.recursion")),
  dagDepthLimit(problem_db.get_ushort("method.nond.graph_depth_limit")),
  modelSelectType(
    problem_db.get_short("method.nond.search_model_graphs.selection")),
  meritFnStar(DBL_MAX)
{
  // Support constrained DAG ensembles for method promotions (hierarchical
  // for MFMC/MLMC, peer for ACV); recursion + model selection are optional
  switch (methodName) {
  case MULTILEVEL_SAMPLING:    // MLMC promotion for model selection
    // weighted MLMC = ACV-RD for hierarchical DAG(s)
    //dagRecursionType = NO_GRAPH_RECURSION; // DAG recursion now optional
    dagWidthLimit = 1;  dagDepthLimit = numApprox; // a hierarchical DAG
    mlmfSubMethod = SUBMETHOD_ACV_RD;   break;
  case MULTIFIDELITY_SAMPLING: // MFMC promotion for model selection
    // MFMC = ACV-MF for hierarchical DAG(s) (Note: SUBMETHOD_MFMC not used)
    //dagRecursionType = NO_GRAPH_RECURSION; // DAG recursion now optional
    dagWidthLimit = 1;  dagDepthLimit = numApprox; // a hierarchical DAG
    mlmfSubMethod = SUBMETHOD_ACV_MF;   break;
  default: // not a promotion: ACV specification + search options
    // assign appropriate depth/width throttles for recursion options
    switch (dagRecursionType) {
    case NO_GRAPH_RECURSION:
      switch (mlmfSubMethod) {
      case SUBMETHOD_ACV_RD: // default hierarch DAG for ACV_RD
	dagWidthLimit = 1;  dagDepthLimit = numApprox;  break;
      default:               // default peer DAG for ACV_MF,ACV_IS
	dagDepthLimit = 1;  dagWidthLimit = numApprox;  break;
      }
      break;
    case KL_GRAPH_RECURSION:
      dagDepthLimit = 2;  dagWidthLimit = numApprox;  break;
    case PARTIAL_GRAPH_RECURSION:
      // dagDepthLimit as user specified
      dagWidthLimit = numApprox;                      break;
    case FULL_GRAPH_RECURSION:
      dagDepthLimit = dagWidthLimit = numApprox;      break;
    }
    break;
  }
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


void NonDGenACVSampling::generate_ensembles_dags()
{
  UShortArray nodes(numApprox);  size_t i;
  for (i=0; i<numApprox; ++i) nodes[i] = i;
  unsigned short root = numApprox;

  /* For verification of consistency with MFMC,ACV:
  //dag.resize(numApprox); for (i=0; i<numApprox; ++i) dag[i] = i+1; // MFMC
  dag.assign(numApprox, numApprox); // ACV
  modelDAGs[nodes].insert(dag);
  return;
  */

  switch (modelSelectType) {
  case ALL_MODEL_COMBINATIONS: {
    // tensor product of order 1 to enumerate approximation inclusion
    UShort2DArray tp;  UShortArray tp_orders(numApprox, 1);
    Pecos::SharedPolyApproxData::tensor_product_multi_index(tp_orders, tp,true);
    //Cout << "tp mi:\n" << tp;
    size_t j, num_tp = tp.size();  unsigned short dag_size;

    // We use a map<UShortArray,UShortArraySet> to associate pruned node sets
    // to DAG sets.  Another option is to consolidate by keeping keep node set
    // size at numApprox and using USHRT_MAX for omitted models, but this is
    // expected to make the linear solve numerics more difficult.

    // Pre-compute nominal ordered DAGs for each dag_size (0:numApprox)
    std::vector<UShortArraySet> nominal_dags(numGroups); // include size 0
    for (dag_size=numApprox; dag_size>=1; --dag_size) {
      nodes.resize(dag_size); // discard trailing node
      generate_dags(dag_size, nodes, nominal_dags[dag_size]);
    }

    // Now map each set of active model nodes through the nominal dags
    UShortArray mapped_dag;  unsigned short nom_dag_j;
    for (i=0; i<num_tp; ++i) { // include first = {0} --> retain MC case
      const UShortArray& tp_i = tp[i];
      nodes.clear();
      for (j=0; j<numApprox; ++j)
	if (tp_i[j]) nodes.push_back(j);
      //Cout << "tp_i:\n" << tp_i << "nodes:\n" << nodes << std::endl;
      dag_size = nodes.size();
      mapped_dag.resize(dag_size);
      const UShortArraySet& nominal_set = nominal_dags[dag_size];
      UShortArraySet& mapped_set = modelDAGs[nodes];  mapped_set.clear();
      UShortArraySet::const_iterator s_cit;
      for (s_cit=nominal_set.begin(); s_cit!=nominal_set.end(); ++s_cit) {
	const UShortArray& nominal_dag = *s_cit;
	for (j=0; j<dag_size; ++j) {
	  nom_dag_j = nominal_dag[j];
	  mapped_dag[j] = (nom_dag_j < dag_size) ? nodes[nom_dag_j] : root;
	}
	//Cout << "nominal_dag =\n" << nominal_dag
	//     << "mapped_dag  =\n" << mapped_dag << std::endl;
	mapped_set.insert(mapped_dag);
      }
    }

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
    break;
  }
  case NO_MODEL_SELECTION:
    // root node (truth index = numApprox) and set of dependent nodes
    // (approximation model indices 0,numApprox-1) are fixed
    generate_dags(root, nodes, modelDAGs[nodes]);  break;
  }

  std::map<UShortArray, UShortArraySet>::const_iterator d_cit;
  size_t set_size, total_size = 0;
  for (d_cit=modelDAGs.begin(); d_cit!=modelDAGs.end(); ++d_cit) {
    const UShortArraySet& dag_set = d_cit->second;
    set_size = dag_set.size();  total_size += set_size;
    Cout << "For approximation set:\n" << d_cit->first
	 << "searching array of DAGs of size " << set_size;
    if (outputLevel >= DEBUG_OUTPUT) Cout << ":\n" << dag_set;//<< '\n';
    else                             Cout << ".\n";
  }
  if (modelSelectType)
    Cout << "Total DAGs across all approximation sets = " << total_size << '\n';
  Cout << std::endl;
}


void NonDGenACVSampling::
generate_dags(unsigned short root, const UShortArray& nodes,
	      UShortArraySet& dag_set)
{
  // Note: This function does not need to manage gaps within the nodes array
  //       (from compact to inflated approx mappings) since its use is limited
  //       to NO_MODEL_SELECTION and nominal_dags within ALL_MODEL_COMBINATIONS.

  dag_set.clear();
  unsigned short num_approx = nodes.size(),
    d_limit = std::min(num_approx, dagDepthLimit),
    w_limit = std::min(num_approx, dagWidthLimit);
  UShortArray dag;

  if (d_limit <= 1) { // peer DAG (ACV)
    if (d_limit == 1) dag.assign(num_approx, root);
    dag_set.insert(dag); // empty DAG (Monte Carlo) if d_limit=0 (num_approx=0)
    return;
  }
  else if (w_limit == 1) { // hierarchical DAG (MLMC, MFMC)
    // Note: numerical MFMC (promoted here for search_model_graphs options)
    // defines ratioApproxSequence on the fly (see NonDMultifidelitySampling::
    // estimator_variance_ratios()) to ensure estvar validity while avoiding
    // linear constraints needed to enforce a fixed ordering. This essentially
    // updates a hierarchical DAG on the fly, with the downside of potentially
    // introducing discontinuities in the integrated solve --> test against
    // promoting "search_model_graphs full recursion" here, trading (nonsmooth
    // but efficient) integration for (smooth but expensive) enumeration.

    dag.resize(num_approx);
    switch (dagRecursionType) {
    case NO_GRAPH_RECURSION:
      // Omit alternate model orderings --> one ordered hierarchical DAG
      for (unsigned short i=0; i<num_approx; ++i) dag[i] = i+1;
      dag_set.insert(dag);
      break;
    case FULL_GRAPH_RECURSION:
      // Include all hierarchical DAG orderings (depth exactly = numApprox)
      // *** TO DO: verify GenACV DAG dependencies handle misordered hierarch
      // *** TO DO: can we treat the weighted MLMC case the same way?
      generate_hierarchical_sub_trees(root, nodes, d_limit, dag, dag_set);
      break;
    }
    return;
  }

  // zero root directed acyclic graphs
  dag.assign(num_approx, USHRT_MAX);
  switch (dagRecursionType) {
  case KL_GRAPH_RECURSION: {

    unsigned short i, K, L, M_minus_K;
    // Dakota low-to-high ordering (enumerate valid KL values and convert)
    for (K=0; K<=num_approx; ++K) {
      M_minus_K = num_approx - K;

      // K highest fidelity approximations target root (num_approx):
      for (i=M_minus_K; i<num_approx; ++i) dag[i] = root;
      // JCP ordering: for (i=0; i<K; ++i) dag[i] = 0; // root = 0

      for (L=0; L<=K; ++L) { // ordered single recursion
	// M-K lowest fidelity approximations target root - L:
	for (i=0; i<M_minus_K; ++i)        dag[i] = root - L;
	// JCP ordering: for (i=K; i<numApprox; ++i) dag[i] = L;

	dag_set.insert(dag);
      }
    }
    break;
  }
  default:
    generate_sub_trees(root, nodes, d_limit, dag, dag_set);
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
  if (num_nodes == 0) {
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
generate_hierarchical_sub_trees(unsigned short root, const UShortArray& nodes,
				unsigned short depth, UShortArray& dag,
				UShortArraySet& dag_set)
{
  // Simplified version (fewer tp1 options) of recursion above

  size_t i, num_nodes = nodes.size();
  if (num_nodes == 0) {
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

  UShort2DArray tp2, sub_nodes_per_root;
  UShortArray   tp2_orders, sub_roots, sub_nodes;
  size_t n, j, k, num_tp2, num_sub_roots, num_sub_nodes;
  unsigned short node_j, dm1 = depth - 1, nm1 = num_nodes - 1;
  num_sub_roots = 1;  num_sub_nodes = num_nodes - num_sub_roots;
  sub_roots.resize(num_sub_roots); sub_nodes.resize(num_sub_nodes);

  for (i=0; i<num_nodes; ++i) {
    // segregate "on" (1) into roots, "off" (0) into nexts
    for (j=0, n=0; j<num_nodes; ++j) {
      node_j = nodes[j];
      if (j == i) { sub_roots[0] = node_j;  dag[node_j] = root; }
      else sub_nodes[n++] = node_j;
    }
 
    if (sub_nodes.empty()) { // optimize out unnecessary recursion
      //Cout << "Recursion out of nodes.  Inserting:\n" << dag;
      dag_set.insert(dag); // insert completed DAG
    }
    else {
      sub_nodes_per_root.resize(num_sub_roots);
      // 2nd TP defines assignments of sub_nodes per sub_root (differs from 1st
      // TP above as there are multiple roots available, which will be recurred)
      tp2_orders.assign(num_sub_nodes, num_sub_roots);
      Pecos::SharedPolyApproxData::
	tensor_product_multi_index(tp2_orders, tp2, false);
      num_tp2 = tp2.size();
      for (j=0; j<num_tp2; ++j) { // 1st entry (all 0's) is valid for this TP
	const UShortArray& tp2_j = tp2[j];
	for (k=0; k<num_sub_roots; ++k) sub_nodes_per_root[k].clear();
	for (k=0; k<num_sub_nodes; ++k) {
	  unsigned short tp2_jk = tp2_j[k];
	  sub_nodes_per_root[tp2_jk].push_back(sub_nodes[k]);
	}
	for (k=0; k<num_sub_roots; ++k)
	  generate_hierarchical_sub_trees(sub_roots[k], sub_nodes_per_root[k],
					  dm1, dag, dag_set);
      }
    }
  }
}


void NonDGenACVSampling::
generate_reverse_dag(const UShortArray& approx_set, const UShortArray& dag)
{
  // define an array of source nodes that point to a given target
  size_t i, dag_size = dag.size();
  reverseActiveDAG.clear();
  reverseActiveDAG.resize(numGroups);
  SizetArray index_map;  inflate_approx_set(approx_set, index_map);
  unsigned short source, target;
  for (i=0; i<dag_size; ++i) { // walk+store path to root
    source = approx_set[i];  target = dag[i];
    reverseActiveDAG[target].insert(source);
    while (target != numApprox) {
      source = target;  target = dag[index_map[source]];
      reverseActiveDAG[target].insert(source);
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In generate_reverse_dag(), reverse DAG:\n" << reverseActiveDAG
	 << std::endl;
}


void NonDGenACVSampling::
unroll_reverse_dag_from_root(unsigned short root, UShortList& root_list)
{
  // create an ordered list of roots that enable ordered sample increments by
  // ensuring that root sample levels are defined before their dependent nodes
  root_list.clear();  root_list.push_back(root);
  UShortList::iterator it = root_list.begin();
  while (it != root_list.end()) {
    const UShortSet& reverse_dag = reverseActiveDAG[*it];
    // append all dependent nodes (reverse iterator: order from high to low)
    root_list.insert(root_list.end(), reverse_dag.rbegin(), reverse_dag.rend());
    ++it;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In unroll_reverse_dag_from_root(), root list:\n"
	 << root_list << std::endl;
}


void NonDGenACVSampling::
unroll_reverse_dag_from_root(unsigned short root, UShortArray& group)
{
  UShortList root_list;
  unroll_reverse_dag_from_root(root, root_list);

  group.clear();
  group.insert(group.end(), root_list.rbegin(), root_list.rend());
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In unroll_reverse_dag_from_root(), group:\n" << group << std::endl;
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

  // use a std::multimap to order nodes based on increasing r_i
  std::multimap<Real, unsigned short> root_ratios;
  UShortList::iterator l_it = default_root_list.begin();
  std::pair<Real, unsigned short> ratio_root_pr;  unsigned short node;
  const UShortArray& approx_set = activeModelSetIter->first;
  SizetArray index_map;  inflate_approx_set(approx_set, index_map);
  for (; l_it != default_root_list.end(); ++l_it) {
    node = *l_it;
    ratio_root_pr.first = (node==root) ? 1. : avg_eval_ratios[index_map[node]];
    ratio_root_pr.second = node;
    // Note: order of key-value pairs with equiv keys is order of insertion
    root_ratios.insert(ratio_root_pr);
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


void NonDGenACVSampling::update_model_groups()
{
  // Reserve num{Approx,Groups} for total model counts and use active
  // counts here (corresponding to active approx set and active DAG)
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t g, num_approx = approx_set.size();  unsigned short root;
  modelGroups.resize(num_approx + 1);

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF:
    // reverseActiveDAG is not recursive --> need a full unroll
    for (g=0; g<num_approx; ++g)
      unroll_reverse_dag_from_root(approx_set[g], modelGroups[g]);
    unroll_reverse_dag_from_root(numApprox, modelGroups[num_approx]);
    break;
  case SUBMETHOD_ACV_IS:  case SUBMETHOD_ACV_RD: {
    // same model groupings, only differ in z1/z2 sample set definitions
    unsigned short root;
    for (g=0; g<num_approx; ++g) {
      root = approx_set[g];
      root_reverse_dag_to_group(root, reverseActiveDAG[root], modelGroups[g]);
    }
    root_reverse_dag_to_group(numApprox, reverseActiveDAG[numApprox],
			      modelGroups[num_approx]);
    break;
  }
  }

  /* This corresponds to ACV-IS (root DAG), not GenACV-IS (general DAG):
  // singleton groups comprised of each root node
  for (g=0; g<num_approx; ++g)
    singleton_model_group(approx_set[g], modelGroups[g]);
  // shared group
  mfmc_model_group(numApprox, modelGroups[num_approx]);
  break;
  */

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In update_model_groups(), modelGroups:\n" << modelGroups
	 << std::endl;
}


void NonDGenACVSampling::update_model_groups(const UShortList& root_list)
{
  if (root_list.empty())
    update_model_groups();

  // Here the ordering of groups is important to preserve incremental sample
  // constraints (e.g. pyramid sampling in ACV-MF)

  const UShortArray& approx_set = activeModelSetIter->first;
  size_t num_approx = approx_set.size();
  modelGroups.resize(num_approx + 1);
  UShortList::const_iterator r_cit;  int g = num_approx;

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF:
    // reverseActiveDAG is not recursive --> need a full unroll
    for (r_cit =root_list.begin(), g=num_approx;
	 r_cit!=root_list.end() && g >= 0; ++r_cit, --g)
      unroll_reverse_dag_from_root(*r_cit, modelGroups[g]);
    break;
  case SUBMETHOD_ACV_IS:  case SUBMETHOD_ACV_RD: {
    // same model groupings, only differ in z1/z2 sample set definitions
    unsigned short root;
    for (r_cit =root_list.begin(), g=num_approx;
	 r_cit!=root_list.end() && g >= 0; ++r_cit, --g) {
      root = *r_cit;
      root_reverse_dag_to_group(root, reverseActiveDAG[root], modelGroups[g]);
    }
    break;
  }
  }

  /* This corresponds to ACV-IS (root DAG), not GenACV-IS (general DAG):
  // singleton groups comprised of each root node
  for (r_cit =root_list.begin(), g=num_approx;
       r_cit!=root_list.end() && g >= 0; ++r_cit, --g)
    cvmc_model_group(*r_cit, modelGroups[g]);
  break;
  */

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "In update_model_groups(UShortList&), modelGroups:\n" << modelGroups
	 << std::endl;
}


void NonDGenACVSampling::pre_run()
{
  NonDNonHierarchSampling::pre_run();

  // For hyper-parameter/design/state changes, can reuse modelDAGs.  Other
  // changes ({dagRecursion,modelSelect}Type) would induce need to regenerate.
  if (modelDAGs.empty())
    generate_ensembles_dags();

  meritFnStar = DBL_MAX;
  bestModelSetIter = modelDAGs.end();
  dagSolns.clear();
}


void NonDGenACVSampling::core_run()
{
  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

  switch (pilotMgmtMode) {
  case ONLINE_PILOT: // iterated GenACV (default)
    // ESTIMATOR_PERFORMANCE case is still iterated for N_H, and therefore
    // differs from ONLINE_PILOT_PROJECTION
    generalized_acv_online_pilot();  break;
  case OFFLINE_PILOT: // non-iterated allocation from offline/Oracle correlation
    switch (finalStatsType) {
    // since offline is not iterated, the ESTIMATOR_PERFORMANCE case is the
    // same as OFFLINE_PILOT_PROJECTION --> bypass IntMaps, simplify code
    case ESTIMATOR_PERFORMANCE:  generalized_acv_pilot_projection();  break;
    default:                     generalized_acv_offline_pilot();     break;
    }
    break;
  case ONLINE_PILOT_PROJECTION:  case OFFLINE_PILOT_PROJECTION:
    generalized_acv_pilot_projection();  break;
  }
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_online_pilot()
{
  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;  RealMatrix var_L;
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
    shared_increment("acv_"); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);
    N_H_alloc += (backfillFailures && mlmfIter) ?
      one_sided_delta(N_H_alloc, avg_hf_target, relaxFactor) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (mlmfIter == 0 && costSource != USER_COST_SPEC)
      recover_online_cost(allResponses);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numGroups,
			      equivHFEvals);
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, var_L, varH, covLL, covLH);

    if (mlmfIter == 0) precompute_ratios(); // metrics not dependent on DAG
    for (activeModelSetIter  = modelDAGs.begin();
	 activeModelSetIter != modelDAGs.end(); ++activeModelSetIter) {
      const UShortArray& approx_set = activeModelSetIter->first;
      const UShortArraySet& dag_set = activeModelSetIter->second;
      soln_key.first = approx_set;
      for (activeDAGIter  = dag_set.begin();
	   activeDAGIter != dag_set.end(); ++activeDAGIter) {
	// sample set definitions are enabled by reversing the DAG direction:
	const UShortArray& active_dag = *activeDAGIter;
	soln_key.second  = active_dag;
	if (outputLevel >= QUIET_OUTPUT)
	  Cout << "Evaluating active DAG:\n" << active_dag
	       << "for approximation set:\n" << approx_set << std::endl;
	generate_reverse_dag(approx_set, active_dag);
	// Use default ordering in root list, prior to final eval ratios soln.
	// Sufficient for finding best DAG, but not for approx_increments().
	unroll_reverse_dag_from_root(numApprox, orderedRootList);
	// compute the LF/HF evaluation ratios from shared samples and compute
	// ratio of MC and ACV mean sq errors (which incorporates anticipated
	// variance reduction from application of avg_eval_ratios).
	MFSolutionData& soln = dagSolns[soln_key];
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
      avg_hf_target = dagSolns[soln_key].solution_reference();
      avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
      numSamples = one_sided_delta(avg_N_H, avg_hf_target, relaxFactor);
    }
    ++mlmfIter;  advance_relaxation();
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  MFSolutionData& soln = dagSolns[soln_key];
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, soln);
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(soln, soln_key.first, N_H_actual, N_H_alloc,
				deltaEquivHF);
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
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  precompute_ratios(); // compute metrics not dependent on active DAG
  std::pair<UShortArray, UShortArray> soln_key;
  for (activeModelSetIter  = modelDAGs.begin();
       activeModelSetIter != modelDAGs.end(); ++activeModelSetIter) {
    const UShortArray& approx_set = activeModelSetIter->first;
    const UShortArraySet& dag_set = activeModelSetIter->second;
    soln_key.first = approx_set;
    for (activeDAGIter  = dag_set.begin();
	 activeDAGIter != dag_set.end(); ++activeDAGIter) {
      // sample set definitions are enabled by reversing the DAG direction:
      const UShortArray& active_dag = *activeDAGIter;
      soln_key.second  = active_dag;
      if (outputLevel >= QUIET_OUTPUT)
	Cout << "Evaluating active DAG:\n" << active_dag
	     << "for approximation set:\n" << approx_set << std::endl;
      generate_reverse_dag(approx_set, active_dag);
      // Use default ordering in root list, prior to final eval ratios soln.
      // Sufficient for finding best DAG, but not for approx_increments().
      unroll_reverse_dag_from_root(numApprox, orderedRootList);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      MFSolutionData& soln = dagSolns[soln_key];
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
  // QOI_STATISTICS case; ESTIMATOR_PERFORMANCE redirects to
  // generalized_acv_pilot_projection() to also bypass IntMaps.
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  const UShortArray& approx_set = activeModelSetIter->first;
  soln_key.first = approx_set;  soln_key.second = *activeDAGIter;
  MFSolutionData& soln = dagSolns[soln_key];
  if (truthFixedByPilot) numSamples = 0;
  else {
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
    // No relaxation since final allocation
    numSamples = one_sided_delta(avg_N_H, soln.solution_reference());
  }
  // perform the shared increment for the online sample profile
  // > utilize ASV for active approx subset
  shared_increment("acv_", approx_set); // spans ACTIVE models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_H_actual);
  N_H_alloc += numSamples;
  increment_equivalent_cost(numSamples, sequenceCost, numApprox, approx_set,
			    equivHFEvals);
  // perform LF increments for the online sample profile
  approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		    N_H_alloc, soln);
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
  RealVector sum_H, sum_HH;   RealMatrix sum_L, sum_LH, var_L;
  RealSymMatrixArray sum_LL;
  if (pilotMgmtMode == OFFLINE_PILOT || // redirected here for ESTIMATOR_PERF
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    SizetArray N_shared_pilot;
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot, false);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot,
			  var_L, varH, covLL, covLH);
    N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  }
  else { // ONLINE_PILOT_PROJECTION
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual, true);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual,
			  var_L, varH, covLL, covLH);
    N_H_alloc = numSamples;
  } /// *** TO DO: this code block is exact same as ACV
  std::pair<UShortArray, UShortArray> soln_key;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  precompute_ratios(); // compute metrics not dependent on active DAG
  for (activeModelSetIter  = modelDAGs.begin();
       activeModelSetIter != modelDAGs.end(); ++activeModelSetIter) {
    const UShortArray&  approx_set = activeModelSetIter->first;
    const UShortArraySet& dag_set = activeModelSetIter->second;
    soln_key.first = approx_set;
    for (activeDAGIter  = dag_set.begin();
	 activeDAGIter != dag_set.end(); ++activeDAGIter) {
      // sample set definitions are enabled by reversing the DAG direction:
      const UShortArray& active_dag = *activeDAGIter;
      soln_key.second  = active_dag;
      if (outputLevel >= QUIET_OUTPUT)
	Cout << "Evaluating active DAG:\n" << active_dag << std::endl;
      generate_reverse_dag(approx_set, active_dag);
      // Use default ordering in root list, prior to final eval ratios soln.
      // Sufficient for finding best DAG, but not for approx_increments().
      unroll_reverse_dag_from_root(numApprox, orderedRootList);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      MFSolutionData& soln = dagSolns[soln_key];
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
  MFSolutionData& soln = dagSolns[soln_key];
  update_projected_samples(soln, soln_key.first, N_H_actual, N_H_alloc,
			   deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDGenACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		  IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  const MFSolutionData& soln)
{
  // Note: these results do not affect the iteration above and can be
  // performed after N_H has converged

  // The final approx set and DAG have been selected
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t num_approx = approx_set.size(), num_groups = num_approx + 1;

  Sizet2DArray N_L_actual_shared, N_L_actual_refined; // ***
  SizetArray   N_L_alloc_refined, delta_N_G(num_groups);
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    // online shared_increment() only includes best model set after
    // processing of offline covariance data
    inflate(N_H_actual, N_L_actual_shared, approx_set);
    inflate(N_H_alloc,  N_L_alloc_refined, approx_set);
  }
  else { // all models are part of online shared_increment()
    inflate(N_H_actual, N_L_actual_shared);
    inflate(N_H_alloc,  N_L_alloc_refined);
  }

  // For pyramid sampling (ACV-MF), define a sequence that reuses sample
  // increments: approx_sequence uses decreasing r_i order, directionally
  // consistent with default approx indexing for well-ordered models
  // > approx 0 is lowest fidelity --> lowest corr,cost --> highest r_i
  // > approx_sequence corresponds to the current approx_set
  //SizetArray approx_sequence;
  //if (mlmfSubMethod == SUBMETHOD_ACV_MF)
  //  ordered_approx_sequence(soln.solution_ratios(), approx_sequence, true);
  //update_model_groups(approx_sequence);
  update_model_groups(orderedRootList);
  update_model_group_costs();
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
  delta_N_G[num_approx] = 0;
  unsigned short root;  const RealVector& soln_vars = soln.solution_variables();
  for (int g=num_approx-1; g>=0; --g) // base to top, excluding all-model group
    delta_N_G[g]
      = group_approx_increment(soln_vars, approx_set, N_L_actual_refined,
			       N_L_alloc_refined, modelGroups[g]);
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
  IntRealMatrixMap sum_L_shared = sum_L_baseline, sum_L_refined;
  overlay_approx_group_sums(sum_G, N_G_actual, sum_L_shared, sum_L_refined,
			    N_L_actual_shared, N_L_actual_refined);

  // ------------------------------------
  // Compute/apply CV to estimate moments
  // ------------------------------------
  RealMatrix H_raw_mom(4, numFunctions);
  genacv_raw_moments(sum_L_baseline, sum_L_shared, sum_L_refined, sum_H,
		     sum_LL, sum_LH, soln.solution_ratios(), N_L_actual_shared,
		     N_L_actual_refined, N_H_actual, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_actual_refined, N_L_alloc_refined);
}


void NonDGenACVSampling::precompute_ratios()
{
  if (pilotMgmtMode == ONLINE_PILOT ||
      pilotMgmtMode == ONLINE_PILOT_PROJECTION)
    cache_mc_reference();// {estVar,numH}Iter0
}


void NonDGenACVSampling::
compute_ratios(const RealMatrix& var_L, MFSolutionData& soln)
{
  // --------------------------------------
  // Configure the optimization sub-problem
  // --------------------------------------
  // Set initial guess based either on related analytic solutions (iter == 0)
  // or warm started from previous solution (iter >= 1)

  // modelGroupCost used for unified treatment in finite_solution_bounds()
  // > for costSource=USER_COST_SPEC, sequenceCost is estimated once,
  //   but modelGroupCost is dependent on the active DAG/model subset
  update_model_groups(); // no approx sequence at this point
  update_model_group_costs();

  bool budget_constrained = (maxFunctionEvals != SZ_MAX), budget_exhausted
    = (budget_constrained && equivHFEvals >= (Real)maxFunctionEvals),
    no_solve = (budget_exhausted || convergenceTol >= 1.); // bypass opt solve
  const UShortArray& approx_set = activeModelSetIter->first;

  if (mlmfIter == 0) {
    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;

    if (no_solve) { // no need for solve
      size_t num_approx = approx_set.size();
      RealVector avg_eval_ratios(num_approx, false);  avg_eval_ratios = 1.;
      // For r_i = 1, C_G,c_g = 0 --> enforce linear constr based on current DAG
      enforce_linear_ineq_constraints(avg_eval_ratios, approx_set,
				      orderedRootList);
      soln.anchored_solution_ratios(avg_eval_ratios, avg_N_H);
      // For offline pilot, the online EstVar is undefined prior to any online
      // samples, but should not happen (no budget used) unless bad convTol spec
      if (pilotMgmtMode == ONLINE_PILOT ||
	  pilotMgmtMode == ONLINE_PILOT_PROJECTION)
	soln.average_estimator_variance(average(estVarIter0));
      else
	soln.average_estimator_variance(std::numeric_limits<Real>::infinity());
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
      ensemble_numerical_solution(soln);  break;
    default: { // competed initial guesses with (competed) local methods
      RealMatrix rho2_LH;
      covariance_to_correlation_sq(covLH, var_L, varH, rho2_LH);
      MFSolutionData mf_soln, cv_soln;
      analytic_initialization_from_mfmc(approx_set, rho2_LH, avg_N_H, mf_soln);
      analytic_initialization_from_ensemble_cvmc(approx_set, *activeDAGIter,
	orderedRootList, rho2_LH, avg_N_H, cv_soln);
      ensemble_numerical_solution(mf_soln);
      ensemble_numerical_solution(cv_soln);
      pick_mfmc_cvmc_solution(mf_soln, cv_soln, soln);
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

    // warm start from previous soln for corresponding {approx_set,active_dag}
    // Note: for sequenced minimizers, only the last is used for refinement
    ensemble_numerical_solution(soln);
  }

  process_model_solution(soln, numSamples);
  if (outputLevel >= NORMAL_OUTPUT)
    print_model_solution(Cout, soln, approx_set);
}


void NonDGenACVSampling::
analytic_initialization_from_mfmc(const UShortArray& approx_set,
				  const RealMatrix& rho2_LH,
				  Real avg_N_H, MFSolutionData& soln)
{
  // check ordering for each QoI across active set of approx
  RealVector avg_eval_ratios;
  if (ordered_approx_sequence(rho2_LH, approx_set))
    mfmc_analytic_solution(approx_set, rho2_LH, sequenceCost, avg_eval_ratios);
  else {
    // compute reordered MFMC for averaged rho; monotonic r not required
    // > any rho2_LH re-ordering from MFMC initial guess can be ignored (later
    //   gets replaced with r_i ordering for approx_increments() sampling)
    SizetArray approx_sequence;
    mfmc_reordered_analytic_solution(approx_set, rho2_LH, sequenceCost,
				     approx_sequence, avg_eval_ratios);
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initial guess from analytic MFMC (unscaled eval ratios):\n"
	 << avg_eval_ratios << std::endl;

  Real avg_hf_target;
  if (maxFunctionEvals == SZ_MAX)// HF target from GenACV estvar using MFMC soln
    avg_hf_target = update_hf_target(avg_eval_ratios,avg_N_H,varH,estVarIter0);
  else // allocate_budget(), then manage lower bounds and pilot over-estimation
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, avg_hf_target,
		    approx_set, orderedRootList, (Real)maxFunctionEvals);
  soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
}


void NonDGenACVSampling::
analytic_initialization_from_ensemble_cvmc(const UShortArray& approx_set,
					   const UShortArray& dag,
					   const UShortList& root_list,
					   const RealMatrix& rho2_LH,
					   Real avg_N_H, MFSolutionData& soln)
{
  // For general DAG, set initial guess based on pairwise CVMC analytic solns
  // (analytic MFMC soln expected to be less relevant).  Differs from derived
  // ACV approach through use of paired DAG dependencies.
  RealVector avg_eval_ratios;
  cvmc_ensemble_solutions(covLL, covLH, varH, sequenceCost, approx_set, dag,
			  root_list, avg_eval_ratios);

  Real avg_hf_target;
  if (maxFunctionEvals == SZ_MAX) {
    // scale according to accuracy (convergenceTol * estVarIter0)
    enforce_linear_ineq_constraints(avg_eval_ratios, approx_set, root_list);
    avg_hf_target = update_hf_target(avg_eval_ratios,avg_N_H,varH,estVarIter0);
  }
  else { // scale according to cost
    // incorporates lin ineq enforcement:
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, avg_hf_target,
		    approx_set, root_list, (Real)maxFunctionEvals);
    //RealVector cd_vars;
    //r_and_N_to_design_vars(avg_eval_ratios, avg_hf_target, cd_vars);
    //soln.average_estimator_variance(average_estimator_variance(cd_vars));
  }
  soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "GenACV scaled initial guess from ensemble CVMC:\n"
	 << "  average eval ratios:\n" << avg_eval_ratios
	 << "  average HF target = "   << avg_hf_target << std::endl;
}


void NonDGenACVSampling::
cvmc_ensemble_solutions(const RealSymMatrixArray& cov_LL,
			const RealMatrix& cov_LH, const RealVector& var_H,
			const RealVector& cost,   const UShortArray& approx_set,
			const UShortArray& dag,   const UShortList& root_list,
			RealVector& avg_eval_ratios)
{
  size_t qoi, source, target, d, dag_size = dag.size();
  if (avg_eval_ratios.length() != dag_size) avg_eval_ratios.size(dag_size);
  else                                      avg_eval_ratios = 0.;

  // First pass: compute an ensemble of pairwise CVMC solutions.  Note that
  // computed eval ratios are DAG-based and no longer bound to the HF node.
  Real cost_ratio, rho_sq, cost_H = cost[numApprox], var_L_qs, var_L_qt,
    cov_LH_qs, cov_LL_qst;
  for (d=0; d<dag_size; ++d) {
    source = approx_set[d];  target = dag[d];
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
  SizetArray index_map;  inflate_approx_set(approx_set, index_map);
  UShortList::const_iterator r_cit; UShortSet::const_iterator d_cit; Real r_tgt;
  for (r_cit=++root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
    target = *r_cit;  const UShortSet& reverse_dag = reverseActiveDAG[target];
    r_tgt = avg_eval_ratios[index_map[target]];
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit)
      { source = *d_cit;  avg_eval_ratios[index_map[source]] *= r_tgt; }
  }
}


void NonDGenACVSampling::
numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
			  size_t& num_nln_con)
{
  // Note: SUBMETHOD_MFMC not used by GenACV

  size_t num_approx = activeModelSetIter->first.size();
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    num_cdv = num_approx;  num_nln_con = 0;
    num_lin_con = 1;
    //if (mlmfSubMethod == SUBMETHOD_MFMC) num_lin_con += num_approx;
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    num_cdv = num_approx + 1;  num_nln_con = 1;
    num_lin_con = 0; //(mlmfSubMethod == SUBMETHOD_MFMC) ? num_approx : 0;
    break;
  case N_MODEL_LINEAR_CONSTRAINT:
    num_lin_con = num_cdv = num_approx + 1;  num_nln_con = 0;
    break;
  case N_MODEL_LINEAR_OBJECTIVE:
    num_cdv = num_approx + 1;  num_nln_con = 1;  num_lin_con = num_approx;
    break;
  }
}


void NonDGenACVSampling::
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
  // Refer to NonDNonHierarchSampling base implementation for additional notes

  size_t hf_form_index, hf_lev_index;
  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
  // For offline mode, online allocations must be lower bounded for numerics:
  Real offline_N_lwr = 2.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;
  bool offline = (pilotMgmtMode == OFFLINE_PILOT ||
		  pilotMgmtMode == OFFLINE_PILOT_PROJECTION);

  // --------------------------------------
  // Formulate the optimization sub-problem: initial pt, bnds, constraints
  // --------------------------------------
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_cdv = x0.length(), approx, num_approx = approx_set.size();
  Real cost_H = sequenceCost[numApprox], budget = (Real)maxFunctionEvals;

  // minimizer-specific updates performed in finite_solution_bounds()
  x_ub = DBL_MAX; // no upper bounds needed for x
  lin_ineq_lb = -DBL_MAX; // no lower bounds on lin ineq

  // Note: ACV paper suggests additional linear constraints for r_i ordering
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    x_lb = 1.; // r_i
    RealVector avg_eval_ratios = soln.solution_ratios();
    if (avg_eval_ratios.empty()) x0 = 1.;
    else                         x0 = avg_eval_ratios;
    // set linear inequality constraint for fixed N:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   \Sum_i w_i   r_i <= equivHF * w / N - w
    //   \Sum_i w_i/w r_i <= equivHF / N - 1
    lin_ineq_ub[0] = (avg_N_H > 1.) ? // protect N_H==0 for offline pilot
      budget / avg_N_H - 1. : // normal case
      budget - 1.;            // bound N_H at 1 (TO DO: need to perform sample)
    for (approx=0; approx<num_approx; ++approx)
      lin_ineq_coeffs(0,approx) = sequenceCost[approx_set[approx]] / cost_H;
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    // Could allow optimal profile to emerge from pilot by allowing N* less than
    // the incurred cost (e.g., setting N_lb to 1), but instead we bound with
    // the incurred cost by setting x_lb = latest N_H and retaining r_lb = 1.
    x_lb = 1.; // r_i
    x_lb[num_approx] = (offline) ?
      offline_N_lwr : avg_N_H; //std::floor(avg_N_H + .5); // pilot <= N*

    RealVector avg_eval_ratios = soln.solution_ratios();
    if (avg_eval_ratios.empty()) x0 = 1.;
    else copy_data_partial(avg_eval_ratios, x0, 0);                    // r_i
    x0[num_approx] = (mlmfIter) ? avg_N_H : soln.solution_reference(); // N

    nln_ineq_lb[0] = -DBL_MAX; // no low bnd
    nln_ineq_ub[0] = budget;
    break;
  }
  case N_MODEL_LINEAR_CONSTRAINT:  case N_MODEL_LINEAR_OBJECTIVE: {
    x_lb = (offline) ? offline_N_lwr : avg_N_H;
    const RealVector& soln_vars = soln.solution_variables();
    x0 = (soln_vars.empty()) ? x_lb : soln_vars;
    if (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) {
      // linear inequality constraint on budget:
      //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
      //   N w + \Sum_i w_i N_i <= equivHF * w
      //   N + \Sum_i w_i/w N_i <= equivHF
      lin_ineq_ub[0] = budget; // remaining ub initialized to 0
      for (approx=0; approx<num_approx; ++approx)
	lin_ineq_coeffs(0, approx) = sequenceCost[approx_set[approx]] / cost_H;
      lin_ineq_coeffs(0, num_approx) = 1.;
    }
    else if (optSubProblemForm == N_MODEL_LINEAR_OBJECTIVE) {
      // nonlinear constraint on estvar
      nln_ineq_lb = -DBL_MAX;  // no lower bnd
      nln_ineq_ub = std::log(convergenceTol * average(estVarIter0));
    }
    break;
  }
  }
  // x0 can undershoot x_lb if an OFFLINE mode, but enforce generally
  enforce_bounds(x0, x_lb, x_ub);
}


void NonDGenACVSampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_MODEL_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t src, tgt, deflate_tgt, lin_ineq_offset
      = (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) ? 1 : 0;

    // Enforce DAG dependencies (ACV: all point to numApprox)
    // > N for each source model > N for model it targets
    // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
    const UShortArray& dag = *activeDAGIter;
    const UShortArray& approx_set = activeModelSetIter->first;
    size_t i, num_approx = approx_set.size();
    SizetArray index_map;  inflate_approx_set(approx_set, index_map);
    for (i=0; i<num_approx; ++i) {
      src = approx_set[i];  tgt = dag[i];
      deflate_tgt = (tgt == numApprox) ? num_approx : index_map[tgt];
      lin_ineq_coeffs(i+lin_ineq_offset, i) = -1.;
      lin_ineq_coeffs(i+lin_ineq_offset, deflate_tgt) =  1. + RATIO_NUDGE;
      //Cout << "lin ineq: source = " << source
      //     << " target = " << target << std::endl;
    }
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT:
    // *** TO DO ***:
    // This is active for truthFixedByPilot && !offline
    // but r is not appropriate for general DAG including multiple CV targets.
    // --> either need to alter handling or suppress this option...

    Cerr << "Error: R_ONLY_LINEAR_CONSTRAINT not implemented in NonDGenACV"
	 << "Sampling::augment_linear_ineq_constraints()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT: // not used
    Cerr << "Error: R_AND_N_NONLINEAR_CONSTRAINT not supported in NonDGenACV"
	 << "Sampling::augment_linear_ineq_constraints()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}


Real NonDGenACVSampling::
augmented_linear_ineq_violations(const RealVector& cd_vars,
				 const RealMatrix& lin_ineq_coeffs,
				 const RealVector& lin_ineq_lb,
				 const RealVector& lin_ineq_ub)
{
  Real quad_viol = 0.;
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_MODEL_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t src, tgt, deflate_tgt, lin_ineq_offset
      = (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) ? 1 : 0;

    // Enforce DAG dependencies (ACV: all point to numApprox)
    // > N for each source model > N for model it targets
    // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
    const UShortArray& dag = *activeDAGIter;
    const UShortArray& approx_set = activeModelSetIter->first;
    size_t i, num_approx = approx_set.size();
    SizetArray index_map;  inflate_approx_set(approx_set, index_map);
    Real viol, inner_prod, l_bnd, u_bnd;
    for (i=0; i<num_approx; ++i) {
      src = approx_set[i];  tgt = dag[i];
      deflate_tgt = (tgt == numApprox) ? num_approx : index_map[tgt];
      // Don't use any constraint tolerance since lin_ineq_coeffs already
      // has RATIO_NUDGE built in
      inner_prod = lin_ineq_coeffs(i+lin_ineq_offset, i) * cd_vars[i] +
	lin_ineq_coeffs(i+lin_ineq_offset, deflate_tgt)  * cd_vars[deflate_tgt];
      l_bnd = lin_ineq_lb[i+lin_ineq_offset];
      u_bnd = lin_ineq_ub[i+lin_ineq_offset];
      if (inner_prod < l_bnd) {
	viol = (std::abs(l_bnd) > Pecos::SMALL_NUMBER)
	  ? (1. - inner_prod / l_bnd) : l_bnd - inner_prod;
	quad_viol += viol*viol;
      }
      else if (inner_prod > u_bnd) {
	viol = (std::abs(u_bnd) > Pecos::SMALL_NUMBER)
	  ? (inner_prod / u_bnd - 1.) : inner_prod - u_bnd;
	quad_viol += viol*viol;
      }
    }
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT:
    // *** TO DO ***:
    // This is active for truthFixedByPilot && !offline
    // but r is not appropriate for general DAG including multiple CV targets.
    // --> either need to alter handling or suppress this option...

    Cerr << "Error: R_ONLY_LINEAR_CONSTRAINT not implemented in NonDGenACV"
	 << "Sampling::augmented_linear_ineq_violations()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT: // not used
    Cerr << "Error: R_AND_N_NONLINEAR_CONSTRAINT not supported in NonDGenACV"
	 << "Sampling::augmented_linear_ineq_violations()." << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }

  return quad_viol;
}


void NonDGenACVSampling::
enforce_linear_ineq_constraints(RealVector& avg_eval_ratios,
				const UShortArray& approx_set,
				const UShortList& root_list)
{
  // Enforce DAG dependencies (ACV: all point to numApprox)
  // > N for each source model > N for model it targets
  // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
  UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
  unsigned short source, target;  Real r_tgt;
  SizetArray index_map;  inflate_approx_set(approx_set, index_map);
  for (r_cit=root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
    target = *r_cit;  const UShortSet& reverse_dag = reverseActiveDAG[target];
    r_tgt = (target == numApprox) ? 1. :
      avg_eval_ratios[index_map[target]];
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
      source = *d_cit;
      Real& r_src = avg_eval_ratios[index_map[source]];
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
		const UShortArray& approx_set, const UShortList& root_list,
		Real budget, Real offline_N_lwr)
{
  // scale to enforce budget constraint.  Since the profile does not emerge
  // from pilot in ACV, don't select an infeasible initial guess:
  // > if N* < N_pilot, scale back r* --> initial = scaled_r*,N_pilot
  // > if N* > N_pilot, use initial = r*,N*
  avg_hf_target = allocate_budget(approx_set, avg_eval_ratios, cost, budget);

  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    if (avg_N_H < offline_N_lwr)
      avg_N_H = offline_N_lwr;
  }
  if (avg_N_H > avg_hf_target) {// replace N* with N_pilot, rescale r* to budget
    avg_hf_target = avg_N_H;

    // Could create another helper if there are additional clients:
    //scale_to_budget_with_pilot(avg_eval_ratios,cost,avg_hf_target,root_list);

    Real approx_inner_prod = 0.;  size_t approx, num_approx = approx_set.size();
    for (approx=0; approx<num_approx; ++approx)
      approx_inner_prod += cost[approx_set[approx]] * avg_eval_ratios[approx];

    // Apply factor: r_scaled = factor r^* which applies to LF (HF r remains 1)
    // > N_pilot (r_scaled^T w + 1) = budget, where w_i = cost_i / cost_H
    // > factor r*^T w = budget / N_pilot - 1
    Real cost_H = cost[numApprox],
      factor = (budget / avg_N_H - 1.) / approx_inner_prod * cost_H;

    // Enforce DAG dependencies (ACV: all point to numApprox)
    // > N for each source model > N for model it targets
    // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;
    Real r_tgt, cost_r_src, budget_decr, inner_prod_decr;
    SizetArray index_map;  inflate_approx_set(approx_set, index_map);
    for (r_cit=root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
      target = *r_cit; const UShortSet& reverse_dag = reverseActiveDAG[target];
      r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[index_map[target]];
      budget_decr = inner_prod_decr = 0.;
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit;  Real& r_src = avg_eval_ratios[index_map[source]];
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
      for (approx=0; approx<num_approx; ++approx)
	inner_prod += cost[approx_set[approx]] * avg_eval_ratios[approx];
      Cout << "Rescale to budget: average evaluation ratios\n"<< avg_eval_ratios
	   << "avg_hf_target = " << avg_hf_target << " budget = "
	   << avg_N_H * inner_prod / cost_H << std::endl;
    }
  }
  else
    enforce_linear_ineq_constraints(avg_eval_ratios, approx_set, root_list);
}


void NonDGenACVSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  // Note: cd_vars is the dimension of the numerical optimization whereas N_vec
  //       is inflated to full dimension for convenience/efficiency --> avoids
  //       the need to map indices (DAG values to sample count indices) as DAG
  //       source/target can be used directly as indices in N_vec/z1/z2)

  const UShortArray& approx_set = activeModelSetIter->first;
  size_t num_approx = approx_set.size();
  RealVector N_vec;  inflate_variables(cd_vars, N_vec, approx_set);
  Real R_sq, N_H = N_vec[numApprox]; // R_ONLY: N_vec inflated w/ avg NLevActual
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:  case R_AND_N_NONLINEAR_CONSTRAINT:
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec);
    break;
  case N_MODEL_LINEAR_OBJECTIVE:  case N_MODEL_LINEAR_CONSTRAINT:
    compute_parameterized_G_g(N_vec);
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

    R_sq = compute_R_sq(covLL[qoi], GMat, covLH, gVec, qoi, approx_set,
			varH[qoi], N_H);
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
recover_results(const RealVector& cv_star, const RealVector& fn_star,
		MFSolutionData& soln)
{
  // Estvar recovery from optimizer provides std::log(average(nh_estvar)) =
  // var_H / N_H (1 - R^2).  Notes:
  // > a QoI-vector prior to averaging would require recomputation from r*,N*)
  // > this value corresponds to N* (_after_ num_samples applied)
  Real avg_estvar = (optSubProblemForm == N_MODEL_LINEAR_OBJECTIVE) ?
    std::exp(fn_star[1]) : std::exp(fn_star(0));
  soln.average_estimator_variance(avg_estvar);

  // Recover optimizer results for average {eval_ratios,estvar}.  Also compute
  // shared increment from N* or from targeting specified budget || accuracy.
  const UShortArray& approx_set = activeModelSetIter->first;
  int num_approx = (int)approx_set.size();
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    const RealVector& avg_eval_ratios = cv_star; // r*
    Real              avg_hf_target;             // N*
    // N* was not part of the optimization (solver computes r* for fixed N)
    // and has not been updated by the optimizer.  We update it here:

    // Allow for constraint to be inactive at optimum, but generally the
    // opt sub-problem will allocate full budget to increase R^2.
    // Note: this formulation is active for option "truth_fixed_by_pilot"
    if (maxFunctionEvals != SZ_MAX) {
      // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L
      // > can also under-relax the budget allocation to enable additional N_H
      //   increments + associated shared sample sets to refine shared stats.
      avg_hf_target = allocate_budget(approx_set, avg_eval_ratios,sequenceCost);
      Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals
	   << ": average HF target = " << avg_hf_target << std::endl;
    }
    else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: detect user spec
      // EstVar target = convTol * estvar_iter0 = estvar_ratio * varH / N_target
      //               = curr_estvar * N_curr / N_target
      //  --> N_target = curr_estvar * N_curr / (convTol * estvar_iter0)
      size_t hf_form_index, hf_lev_index;
      hf_indices(hf_form_index, hf_lev_index);
      SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
      size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
      Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
      avg_hf_target
	= update_hf_target(avg_eval_ratios, avg_N_H, varH, estVarIter0);
      Cout << "Scaling profile for convergenceTol = " << convergenceTol
	   << ": average HF target = " << avg_hf_target << std::endl;
    }
    //avg_hf_target = std::min(budget_target, ctol_target); // enforce both
    soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
    soln.equivalent_hf_allocation(
      compute_equivalent_cost(avg_hf_target, avg_eval_ratios,
			      sequenceCost, approx_set));
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    // R_AND_N:  r*   is leading part of cv_star and N* is trailing entry
    RealVector avg_eval_ratios(Teuchos::View, cv_star.values(), num_approx);
    soln.anchored_solution_ratios(avg_eval_ratios, cv_star[num_approx]);
    soln.equivalent_hf_allocation(fn_star[1]);
    break;
  }
  case N_MODEL_LINEAR_CONSTRAINT:
    // N_VECTOR: N*_i is leading part of cv_star and N* is trailing entry.
    // Note that r_i is always relative to HF N, not its DAG pairing.
    soln.solution_variables(cv_star);
    soln.equivalent_hf_allocation(linear_model_cost(cv_star));
    break;
  case N_MODEL_LINEAR_OBJECTIVE:
    // N_VECTOR: N*_i is leading part of cv_star and N* is trailing entry.
    // Note that r_i is always relative to HF N, not its DAG pairing.
    soln.solution_variables(cv_star);
    soln.equivalent_hf_allocation(fn_star[0]);
    break;
  }
}


Real NonDGenACVSampling::linear_model_cost(const RealVector& N_vec)
{
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_approx = approx_set.size();
  Real cost_H = sequenceCost[numApprox], cost_i,
    N_H = N_vec[num_approx], sum = 0.;

  // linear objective: N + Sum(w_i N_i) / w
  for (i=0; i<num_approx; ++i) {
    cost_i = sequenceCost[approx_set[i]];
    sum += cost_i * N_vec[i]; // Sum(w_i N_i)
  }
  Real lin_obj = N_H + sum / cost_H; // N + sum / w
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear cost = " << lin_obj << std::endl;
  return lin_obj;
}


Real NonDGenACVSampling::nonlinear_model_cost(const RealVector& r_and_N)
{
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_approx = approx_set.size();
  Real cost_H = sequenceCost[numApprox], cost_i,
    N_H = r_and_N[num_approx], approx_inner_prod = 0.;

  // nln ineq constraint: N ( w + Sum(w_i r_i) ) <= C, where C = equivHF * w
  // -->  N ( 1 + Sum(w_i r_i) / w ) <= equivHF
  for (i=0; i<num_approx; ++i) {
    cost_i = sequenceCost[approx_set[i]];
    approx_inner_prod += cost_i * r_and_N[i]; // Sum(c_i r_i)
  }
  approx_inner_prod /= cost_H;                // Sum(w_i r_i)

  Real nln_cost = N_H * (1. + approx_inner_prod); // N ( 1 + Sum(w_i r_i) )
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "nonlinear cost: design vars:\n" << r_and_N
	 << "cost = " << nln_cost << std::endl;
  return nln_cost;
}


void NonDGenACVSampling::
linear_model_cost_gradient(const RealVector& N_vec, RealVector& grad_c)
{
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_approx = approx_set.size();  unsigned short inflate_i;
  Real cost_H = sequenceCost[numApprox], cost_i;

  // linear objective: N + Sum(w_i N_i) / w
  // > grad w.r.t. N_i = w_i / w
  // > grad w.r.t. N   = w   / w = 1

  for (i=0; i<num_approx; ++i) {
    cost_i    = sequenceCost[approx_set[i]];
    grad_c[i] = cost_i / cost_H;
  }
  grad_c[num_approx] = 1.;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear cost gradient:\n" << grad_c << std::endl;
}


void NonDGenACVSampling::
nonlinear_model_cost_gradient(const RealVector& r_and_N, RealVector& grad_c)
{
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_approx = approx_set.size();

  // nonlinear inequality constraint: N ( 1 + Sum(w_i r_i) / w ) <= equivHF
  // > grad w.r.t. r_i = N w_i / w
  // > grad w.r.t. N   = 1 + Sum(w_i r_i) / w

  Real cost_H = sequenceCost[numApprox], cost_i,
    N_over_w = r_and_N[num_approx] / cost_H, approx_inner_prod = 0.;
  for (i=0; i<num_approx; ++i) {
    cost_i = sequenceCost[approx_set[i]];
    grad_c[i]          = cost_i * N_over_w;
    approx_inner_prod += cost_i * r_and_N[i]; // Sum(c_i r_i)
  }
  grad_c[num_approx] = 1. + approx_inner_prod / cost_H;     // 1 + Sum(w_i r_i)
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "nonlinear cost gradient:\n" << grad_c << std::endl;
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
    const ShortArray& asv     = resp.active_set_request_vector();

    // the  "shared" accumulations define the z_i^1 sample sets
    // the "refined" accumulations define the z_i^2 sample sets

    // refined only at root node:
    accumulate_acv_sums(sum_L_refined, N_L_refined, fn_vals, asv, root);
    // refined + shared at dependent nodes:
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
      node = *d_cit;
      accumulate_acv_sums(sum_L_shared,  N_L_shared,  fn_vals, asv, node);
      accumulate_acv_sums(sum_L_refined, N_L_refined, fn_vals, asv, node);
    }
  }
}


void NonDGenACVSampling::
accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared,
		       IntRealMatrixMap& sum_L_refined,
		       Sizet2DArray& N_L_shared, Sizet2DArray& N_L_refined,
		       const SizetArray& approx_sequence, size_t sequence_start,
		       size_t sequence_end)
{
  // Based on active DAG for [start,end), precompute the set of models for
  // which to update {sum,N}_L_shared with this sample increment
  // > the  "shared" accumulations define the z^1_s sample sets (here)
  // > the "refined" accumulations define the z^2_s sample sets (other fns)
  UShortSet accum_z1_sets;  size_t s;  unsigned short source, target;
  const UShortArray& active_dag = *activeDAGIter;
  const UShortArray& approx_set = activeModelSetIter->first;
  bool ordered = approx_sequence.empty();
  if (ordered)
    for (s=sequence_start; s<sequence_end; ++s) {      // compact
      source = approx_set[s];  target = active_dag[s]; // inflated
      // if the CV target for model s is part of the models being sampled,
      // then we will increment its z^1 "shared" accumulators
      if (target >= approx_set[sequence_start] &&
	  target <= approx_set[sequence_end-1])
	accum_z1_sets.insert(source);
    }
  else {
    UShortSet seq_approx_set;  size_t seq_s;
    for (s=sequence_start; s<sequence_end; ++s) {
      seq_s = approx_sequence[s];               // compact
      seq_approx_set.insert(approx_set[seq_s]); // inflated
    }
    for (s=sequence_start; s<sequence_end; ++s) {
      seq_s  = approx_sequence[s];
      source = approx_set[seq_s];  target = active_dag[seq_s];
      if (seq_approx_set.find(target) != seq_approx_set.end())
	accum_z1_sets.insert(source);
    }
  }
  //Cout << "For sequence end = " << sequence_end
  //     << " accum_z1_sets =\n" << accum_z1_sets << std::endl;

  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest
  IntRespMCIter r_it; UShortSet::iterator z1_it; size_t approx, inflate_approx;
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    // "shared" z^1 at target nodes:
    for (z1_it=accum_z1_sets.begin(); z1_it!=accum_z1_sets.end(); ++z1_it)
      accumulate_acv_sums(sum_L_shared, N_L_shared, fn_vals, asv, *z1_it);

    // *** TO DO: z1 & z2 accumulations overlap --> prefer to accumulate
    // ***        once and then add contribution to multiple roll-ups

    // "refined" z^2 at source nodes:
    // Adapted from accumulate_acv_sums({sum_L,N_L}_refined, seq, start, end)
    for (s=sequence_start; s<sequence_end; ++s) {
      approx = (ordered) ? s : approx_sequence[s];
      inflate_approx = approx_set[approx];
      accumulate_acv_sums(sum_L_refined, N_L_refined, fn_vals, asv,
			  inflate_approx);
    }
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
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(4, numFunctions);

  precompute_genacv_controls(avg_eval_ratios, N_H);

  const UShortArray& approx_set = activeModelSetIter->first;
  size_t approx, inflate_approx, num_approx = approx_set.size(), qoi, N_H_q;
  Real sum_H_mq;  RealVector beta(num_approx);
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
			     N_H_q, mom, qoi, approx_set, beta);
      // *** TO DO: support shared_approx_increment() --> baselineL

      Real& H_raw_mq = H_raw_mom(mom-1, qoi);
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<num_approx; ++approx) {
	inflate_approx = approx_set[approx];
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << inflate_approx+1
	       << ": control variate beta = " << std::setw(9) << beta[approx]
	       << '\n';
	// For ACV, shared counts are fixed at N_H for all approx
	apply_control(sum_L_sh_m(qoi,inflate_approx),
		      N_L_shared[inflate_approx][qoi],
		      sum_L_ref_m(qoi,inflate_approx),
		      N_L_refined[inflate_approx][qoi], beta[approx], H_raw_mq);
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

  // Note: N_vec,z1,z2 are inflated to full dimension, avoiding the need to
  //       map indices (DAG values to sample count indices)

  const UShortArray& approx_set = activeModelSetIter->first;
  const UShortArray& active_dag = *activeDAGIter;
  size_t i, j, dag_size = active_dag.size();
  if (GMat.numRows() != dag_size) GMat.shapeUninitialized(dag_size);
  if (gVec.length()  != dag_size) gVec.sizeUninitialized(dag_size);

  //RealMatrix FGMat(dag_size, dag_size, false); // to verify matrix symmetry

  // define sample recursion sets backwards (from z_H down to lowest fid)
  Real src_i, src_j, tgt_i, tgt_j, z1_i;
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real z2_i, z1_j, z2_j, z_H = N_vec[numApprox];
    for (i=0; i<dag_size; ++i) {
      src_i = approx_set[i];  tgt_i = active_dag[i];
      z1_i = /*z2_bi = z_bi =*/ N_vec[tgt_i];  z2_i = /*z_i =*/ N_vec[src_i];
      gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<=i; ++j) {
	src_j = approx_set[j];  tgt_j = active_dag[j];
	z1_j  = /*z2_bj = z_bj =*/ N_vec[tgt_j];  z2_j = /*z_j =*/ N_vec[src_j];
	GMat(i,j)
	  = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
	  + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_IS: {
    // Bomarito Eqs. 21-22.  Notation conversion (Bomarito in quotes):
    // > "N_{beta_i}" denotes z1_i
    // > "N_i" denotes "zprime_i" which is NOT z2_i, but rather z2_i - z1_i
    //   (where z2_i = N_vec[i])
    // > z1_src is matched to "N_tgt", again "zprime_tgt" = z2_tgt - z1_tgt
    RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    Real z_i, z_j, zi_zj, zprime_i;
    for (i=0; i<dag_size; ++i) {
      src_i = approx_set[i];   tgt_i = active_dag[i];
      z_i   = N_vec[src_i];    z1_i  = z1[src_i];
      zprime_i = z2[src_i] - z1_i;
      gVec[i] = (tgt_i == numApprox) ? 1./z1_i - 1./z_i : 0.; // N_0 -> N_beta_i
      for (j=0; j<=i; ++j) {
	src_j = approx_set[j];  tgt_j = active_dag[j];
	z_j   = N_vec[src_j];   zi_zj = z_i * z_j;
	GMat(i,j) = 0.;     
	if (tgt_i == tgt_j) GMat(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
	if (tgt_i == src_j) GMat(i,j) += z1_i/zi_zj - 1./z_j;
	if (src_i == tgt_j) GMat(i,j) += zprime_i/zi_zj - 1./z_i;
	if (src_i == src_j) GMat(i,j) += zprime_i/zi_zj;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: {
    // Bomarito Eqs. 19-20.  Notation conversion (Bomarito in quotes):
    // > "N_{beta_i}" denotes z1_i and "N_i" denotes z2_i
    // > z1_src is matched to "N_tgt", again z2_tgt
    RealVector z1, z2;  unroll_z1_z2(N_vec, z1, z2);
    Real z2_i;
    for (i=0; i<dag_size; ++i) {
      src_i = approx_set[i];  tgt_i = active_dag[i];
      z1_i  = z1[src_i];      z2_i  = z2[src_i];
      gVec[i] = (tgt_i == numApprox) ? 1./z1_i : 0.; // N_0 -> N_beta_i
      for (j=0; j<=i; ++j) {
	src_j = approx_set[j];  tgt_j = active_dag[j];
	GMat(i,j) = 0.;
	if (tgt_i == tgt_j) GMat(i,j) += 1./z1_i;
	if (tgt_i == src_j) GMat(i,j) -= 1./z1_i; // always false for root dag
	if (src_i == tgt_j) GMat(i,j) -= 1./z2_i; // always false for root dag
	if (src_i == src_j) GMat(i,j) += 1./z2_i;
      }
    }
    break;
  }
  default:
    Cerr << "Error: bad sub-method name (" << mlmfSubMethod << ") in NonDGen"
	 << "ACVSampling::compute_parameterized_G_g()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  //Cout << "Full G matrix:\n" << FGMat << "g vector:\n" << gVec << std::endl;
  //exit(0);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "For dag:\n"  << active_dag  << "G matrix:\n" << GMat
	 << "g vector:\n" << gVec << std::endl;
}


void NonDGenACVSampling::
unroll_z1_z2(const RealVector& N_vec, RealVector& z1, RealVector& z2)
{
  // Note: N_vec,z1,z2 are inflated to full dimension, avoiding the need to
  //       map indices (DAG values to sample count indices) 

  //Real z_H = N_vec[numApprox];
  z1.size(numGroups);  z1[numApprox] = 0;
  z2.size(numGroups);  z2[numApprox] = N_vec[numApprox];

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF: { // not used (special unroll logic not required)
    const UShortArray& approx_set = activeModelSetIter->first;
    const UShortArray& active_dag = *activeDAGIter;
    unsigned short i, source, target;  size_t dag_size = active_dag.size();
    for (i=0; i<dag_size; ++i) {
      source = approx_set[i];  target = active_dag[i];
      //z1[i] = N_vec[target];  z2[i] = N_vec[source]; // COMPACTED z1,z2
      z1[source] = N_vec[target];  z2[source] = N_vec[source]; // FULL z1,z2
    }
    break;
  }
  case SUBMETHOD_ACV_IS: {
    // leverage reverseActiveDAG and orderedRootList:
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;  Real z1t;
    for (r_cit=orderedRootList.begin(); r_cit!=orderedRootList.end(); ++r_cit) {
      target = *r_cit;  z1t = z1[target];
      const UShortSet& reverse_dag = reverseActiveDAG[target];
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit;
	z1[source] = N_vec[target] - z1t; // Bomarito Fig. 6
	z2[source] = N_vec[source];       // Bomarito Fig. 6
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: {
    // leverage reverseActiveDAG and orderedRootList:
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;  Real z2t;
    for (r_cit=orderedRootList.begin(); r_cit!=orderedRootList.end(); ++r_cit) {
      target = *r_cit;  z2t = z2[target];
      const UShortSet& reverse_dag = reverseActiveDAG[target];
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit;  Real& z1s = z1[source];
	z1s = z2t;                        // Bomarito Fig. 5
	z2[source] = N_vec[source] - z1s; // Bomarito Fig. 5
      }
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


void NonDGenACVSampling::update_best(MFSolutionData& soln)
{
  // Update tracking of best result

  bool update = false;  Real merit_fn;
  if (!valid_variance(soln.average_estimator_variance())) // *** TO DO: problems could be hidden due to averaging --> consider a finer-grained badNumericsFlag triggered per QoI
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
      Cout << "Updating best DAG to:\n" << *bestDAGIter << "for approximation "
	   << "set:\n" << activeModelSetIter->first << std::endl;
  }
}


void NonDGenACVSampling::restore_best()
{
  //Cout << "\n>>>>> Approx subset and DAG evaluation completed\n" << std::endl;

  if (bestModelSetIter == modelDAGs.end()) {
    Cout << "Warning: best DAG has not been updated in restore_best().\n"
	 << "         Last active DAG will be used." << std::endl;
    return;
  }

  const UShortArray& best_models = bestModelSetIter->first;
  const UShortArray& best_dag    = *bestDAGIter;
  Cout << "\nBest solution from DAG:\n" << best_dag
       << "for approximation set:\n" << best_models << std::endl;
  std::pair<UShortArray, UShortArray> soln_key(best_models, best_dag);
  MFSolutionData& best_soln = dagSolns[soln_key];
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nwith solution variables =\n" << best_soln.solution_variables()
	 << std::endl;

  // restore best state for compute/archive/print final results
  bool approx_incr = ( finalStatsType == QOI_STATISTICS &&
    ( pilotMgmtMode == ONLINE_PILOT || pilotMgmtMode == OFFLINE_PILOT ) );
  if (activeModelSetIter != bestModelSetIter ||
      activeDAGIter      != bestDAGIter) { // best is not most recent
    activeModelSetIter = bestModelSetIter;
    activeDAGIter      = bestDAGIter;
    if (approx_incr)
      generate_reverse_dag(best_models, best_dag);
  }
  // now we can re-order roots based on final eval ratios solution
  // --> used for pyramid sample set ordering in approx_increments()
  if (approx_incr)
    unroll_reverse_dag_from_root(numApprox, best_soln.solution_ratios(),
				 orderedRootList);
}

} // namespace Dakota
