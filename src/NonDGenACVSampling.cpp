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
NonDGenACVSampling(ProblemDescDB& problem_db, ParallelLibrary& parallel_lib, std::shared_ptr<Model> model):
  NonDACVSampling(problem_db, parallel_lib, model),
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
  case MULTILEVEL_SAMPLING: {// MLMC promotion: hierarch search, model selection
    // weighted MLMC = ACV-RD restricted to hierarchical DAG(s)
    // > for model graph search off, ACV-RD defaults to the same hierarchical
    //   DAG(s) as for weighted MLMC
    // > for model graph search on, ACV-RD searches all graphs whereas weighted
    //   MLMC is restricted to the hierarchical subset
    dagWidthLimit = 1;  dagDepthLimit = numApprox; // a hierarchical DAG
    mlmfSubMethod = SUBMETHOD_ACV_RD;
    // check for unsupported allocation targets from MLMC spec
    bool err_flag = false;
    if (problem_db.get_short("method.nond.allocation_target") != TARGET_MEAN) {
      Cerr << "Error: unsupported allocation target specification.\n";
      err_flag = true;
    }
    if (problem_db.get_short("method.nond.qoi_aggregation") !=
	QOI_AGGREGATION_SUM) {
      Cerr << "Error: unsupported qoi aggregation specification.\n";
      err_flag = true;
    }
    if (problem_db.get_short("method.nond.convergence_tolerance_target") !=
	VARIANCE_CONSTRAINT_TARGET) {
      Cerr << "Error: unsupported convergence tol target specification.\n";
      err_flag = true;
    }
    if (err_flag) {
      Cerr << "Some controls not available when promoting weighted MLMC to "
	   << "GenACV." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    break;
  }
  case MULTIFIDELITY_SAMPLING: // MFMC promotion: hierarch search, model select
    // MFMC = ACV-MF restricted to hierarchical DAG(s)
    // > for model graph search off, ACV-MF defaults to peer DAG(s) whereas
    //   MFMC uses hierarchical DAG(s)
    // > for model graph search on, ACV-MF searches all graphs whereas MFMC
    //   is restricted to the hierarchical subset
    // > Note: the SUBMETHOD_MFMC sub-type is not used here
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

  // enable downstream logic to account for graph searches and promotions
  methodName = GEN_APPROX_CONTROL_VARIATE;
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


void NonDGenACVSampling::generate_ensembles_dags()
{
  UShortArray nodes(numApprox);  size_t i;
  for (i=0; i<numApprox; ++i) nodes[i] = i;
  unsigned short root = numApprox;

  /* For verification of consistency with MFMC,ACV:
  UShortArray dag(numApprox);
  for (i=0; i<numApprox; ++i) dag[i] = i+1; // MFMC
  dag.assign(numApprox, numApprox); // ACV
  modelDAGs[nodes].insert(dag);
  return;
  */

  /* For debugging a specific DAG case + specific G_g evaluation
  UShortArray debug_nodes = {0,1,2,3,4,5}, debug_dag = {4,7,3,1,5,2};
  modelDAGs[debug_nodes].insert(debug_dag);
  compGgCntr = 0;
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
  size_t approx_set_size, dag_set_size, total_dag_size = 0;
  for (d_cit=modelDAGs.begin(); d_cit!=modelDAGs.end(); ++d_cit) {
    const UShortArray& approx_set = d_cit->first;
    const UShortArraySet& dag_set = d_cit->second;
    approx_set_size = approx_set.size();
    if (approx_set.size()) { // suppress MC case
      dag_set_size = dag_set.size();  total_dag_size += dag_set_size;
      Cout << "For approximation set:\n" << approx_set
	   << "searching array of DAGs of size " << dag_set_size;
      if (outputLevel >= DEBUG_OUTPUT) Cout << ":\n" << dag_set;//<< '\n';
      else                             Cout << ".\n";
    }
  }
  if (modelSelectType)
    Cout << "Total DAGs across all approximation sets = "<<total_dag_size<<'\n';
  Cout << std::endl;
}


void NonDGenACVSampling::prune_ensembles(const UShortArray& active_approx_set)
{
  UShortArray inactive_approx_set;
  size_t i, cntr = 0, num_active = active_approx_set.size();
  for (i=0; i<numApprox; ++i)
    if (cntr < num_active && active_approx_set[cntr] == i) ++cntr;
    else                         inactive_approx_set.push_back(i);
  if (inactive_approx_set.empty()) return;

  std::map<UShortArray, UShortArraySet>::iterator d_it = modelDAGs.begin();
  while (d_it != modelDAGs.end()) {
    const UShortArray& approx_set = d_it->first;
    if (contains(approx_set, inactive_approx_set, true)) { // ordered
      Cout << "Removing approximation set:\n" << approx_set;
      modelDAGs.erase(d_it++);
    }
    else ++d_it;
  }
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
    // IS,RD = same model groupings, only differ in z1/z2 sample sets
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
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  Sizet2DArray N_L_actual_shared;
  inflate(N_H_actual, N_L_actual_shared, numApprox);
  inflate(N_H_alloc,  NApproxAlloc,      numApprox);
  Real hf_target = 0., avg_N_H;  size_t alloc_incr;
  // start with all approximations included
  std::pair<UShortArray, UShortArray> soln_key(fullApproxSet, UShortArray());

  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    const UShortArray& approx_set = soln_key.first;
    shared_increment("acv_", approx_set); // active approx set + truth
    // leaves inactive approx sums in state inconsistent with sum_H/N_H_actual
    accumulate_genacv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			   sum_LH, sum_HH, N_H_actual, N_L_actual_shared);
    alloc_incr = (backfillFailures && mlmfIter) ?
      one_sided_delta(N_H_alloc, hf_target, relaxFactor) : numSamples;
    N_H_alloc += alloc_incr;
    increment_sample_range(NApproxAlloc, alloc_incr, approx_set);
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (mlmfIter == 0 && costSource != USER_COST_SPEC)
      recover_online_cost(allResponses);
    increment_equivalent_cost(numSamples, sequenceCost, numApprox, approx_set,
			      equivHFEvals);
    // inactive covariance terms are zeroed
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, varH, covLL, covLH, approx_set);

    bool no_solve = precompute_allocations(); // independent of DAG
    if (no_solve) no_solve_solution();
    else {
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
	  if (outputLevel >= QUIET_OUTPUT) {
	    Cout << "Evaluating active DAG:\n";
	    print_dag(active_dag, approx_set);
	  }
	  generate_reverse_dag(approx_set, active_dag);
	  // Use default ordering in root list, prior to final eval ratios soln.
	  // Sufficient for finding best DAG, but not for approx_increments().
	  unroll_reverse_dag_from_root(numApprox, orderedRootList);
	  // compute the LF/HF evaluation ratios from shared samples and compute
	  // ratio of MC and ACV mean sq errors (which incorporates anticipated
	  // variance reduction from application of avg_eval_ratios).
	  MFSolutionData& soln = dagSolns[soln_key];
	  compute_allocations(soln);
	  update_best(soln);// store state for restoration
	  //reset_acv(); // reset state for next ACV execution
	}
      }

      print_best();
      soln_key.first  =  bestModelSetIter->first;
      soln_key.second = *bestDAGIter;
      if (truthFixedByPilot) numSamples = 0;
      else {
	hf_target = dagSolns[soln_key].solution_reference();
	avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
	numSamples = one_sided_delta(avg_N_H, hf_target, relaxFactor);
      }
      ++mlmfIter;  advance_relaxation();
      // prune models discarded by the best soln from the completed iteration.
      // This avoids continuing to invest in covariance refinement for discarded
      // models, which is inconsistent with the budget from the optimal solution
      // over the best model subset (retention results in budget overshoot).
      if (modelSelectType)
	prune_ensembles(soln_key.first);
    }
  }
  restore_best();

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  MFSolutionData& soln = dagSolns[soln_key];
  if (finalStatsType == QOI_STATISTICS) {
    IntRealMatrixMap sum_L_shared, sum_L_refined;
    Sizet2DArray N_L_actual_refined;
    approx_increments(sum_L_baselineH, N_H_actual, N_H_alloc, sum_L_shared,
		      N_L_actual_shared, sum_L_refined, N_L_actual_refined,
		      NApproxAlloc, soln);
    genacv_raw_moments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		       sum_H, N_H_actual, sum_L_shared, N_L_actual_shared,
		       sum_L_refined, N_L_actual_refined, soln);
    finalize_counts(N_L_actual_refined, NApproxAlloc);
  }
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(soln, soln_key.first, N_H_actual, N_H_alloc,
				N_L_actual_shared, NApproxAlloc, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  IntRealVectorMap sum_H_pilot;  IntRealMatrixMap sum_L_pilot, sum_LH_pilot;
  IntRealSymMatrixArrayMap sum_LL_pilot;  RealVector sum_HH_pilot;
  initialize_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot);
  SizetArray N_shared_pilot;
  evaluate_pilot(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		 sum_HH_pilot, N_shared_pilot, false);
  compute_LH_statistics(sum_L_pilot[1], sum_H_pilot[1], sum_LL_pilot[1],
			sum_LH_pilot[1], sum_HH_pilot, N_shared_pilot,
			varH, covLL, covLH);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  inflate(N_H_alloc, NApproxAlloc, numApprox); // used in active_budget()
  std::pair<UShortArray, UShortArray> soln_key;

  // no need for "no_solve" logic in offline case
  precompute_allocations(); // metrics not dependent on DAG
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
      if (outputLevel >= QUIET_OUTPUT) {
	Cout << "Evaluating active DAG:\n";
	print_dag(active_dag, approx_set);
      }
      generate_reverse_dag(approx_set, active_dag);
      // Use default ordering in root list, prior to final eval ratios soln.
      // Sufficient for finding best DAG, but not for approx_increments().
      unroll_reverse_dag_from_root(numApprox, orderedRootList);
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      MFSolutionData& soln = dagSolns[soln_key];
      compute_allocations(soln);
      update_best(soln); // store state for restoration
      //reset_acv(); // reset state for next ACV execution
    }
  }
  print_best();  restore_best();  ++mlmfIter;

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
  increment_sample_range(NApproxAlloc, numSamples, approx_set);
  increment_equivalent_cost(numSamples, sequenceCost, numApprox, approx_set,
			    equivHFEvals);
  // perform LF increments for the online sample profile
  IntRealMatrixMap  sum_L_shared,      sum_L_refined;
  Sizet2DArray N_L_actual_shared, N_L_actual_refined;
  approx_increments(sum_L_baselineH, N_H_actual, N_H_alloc, sum_L_shared,
		    N_L_actual_shared, sum_L_refined, N_L_actual_refined,
		    NApproxAlloc, soln);
  genacv_raw_moments(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		     N_shared_pilot, sum_H, N_H_actual,
		     sum_L_shared, N_L_actual_shared,
		     sum_L_refined, N_L_actual_refined, soln);
  finalize_counts(N_L_actual_refined, NApproxAlloc);
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
  RealVector sum_H, sum_HH;   RealMatrix sum_L, sum_LH;
  RealSymMatrixArray sum_LL;
  if (pilotMgmtMode == OFFLINE_PILOT || // redirected here for ESTIMATOR_PERF
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    SizetArray N_shared_pilot;
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot, false);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared_pilot,
			  varH, covLL, covLH);
    N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  }
  else { // ONLINE_PILOT_PROJECTION
    evaluate_pilot(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual, true);
    compute_LH_statistics(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_H_actual,
			  varH, covLL, covLH);
    N_H_alloc = numSamples;
  } /// TO DO: this code block is exact same as ACV
  inflate(N_H_alloc, NApproxAlloc, numApprox);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  std::pair<UShortArray, UShortArray> soln_key;
  bool no_solve = precompute_allocations(); // independent of DAG
  if (no_solve) no_solve_solution();
  else {
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
	if (outputLevel >= QUIET_OUTPUT) {
	  Cout << "Evaluating active DAG:\n";
	  print_dag(active_dag, approx_set);
	}
	generate_reverse_dag(approx_set, active_dag);
	// Use default ordering in root list, prior to final eval ratios soln.
	// Sufficient for finding best DAG, but not for approx_increments().
	unroll_reverse_dag_from_root(numApprox, orderedRootList);
	// compute the LF/HF evaluation ratios from shared samples and compute
	// ratio of MC and ACV mean sq errors (which incorporates anticipated
	// variance reduction from application of avg_eval_ratios).
	MFSolutionData& soln = dagSolns[soln_key];
	compute_allocations(soln);
	update_best(soln); // store state for restoration
	//reset_acv(); // reset state for next ACV execution
      }
    }
    print_best();  ++mlmfIter;
  }
  restore_best();

  // No LF increments or final moments for pilot projection
  soln_key.first  = activeModelSetIter->first;
  soln_key.second = *activeDAGIter;
  MFSolutionData& soln = dagSolns[soln_key];
  Sizet2DArray N_L_actual; // inflate from N_H_actual for shared pilot
  update_projected_samples(soln, soln_key.first, N_H_actual,
			   N_H_alloc, N_L_actual, NApproxAlloc,
			   deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::ensemble_numerical_solution() recovers N*
  // from the numerical solve and computes projected estVariance{s,Ratios}
}


void NonDGenACVSampling::no_solve_solution()
{
  numSamples = 0; // terminate online iteration

  if (mlmfIter == 0) { // default to peer DAG with all models
    UShortArray peer_dag;  peer_dag.assign(numApprox, numApprox); // peer DAG
    std::pair<UShortArray, UShortArray> soln_key(fullApproxSet, peer_dag);
    MFSolutionData& soln = dagSolns[soln_key]; // create new if necessary

    std::map<UShortArray, UShortArraySet>::iterator m_it;
    UShortArraySet::iterator d_it;

    m_it = modelDAGs.find(fullApproxSet);
    if (m_it == modelDAGs.end()) { // should not happen
      std::pair<UShortArray, UShortArraySet>
	soln_set(fullApproxSet, UShortArraySet());
      m_it = modelDAGs.insert(soln_set).first;
      UShortArraySet& dag_set = m_it->second;
      d_it = dag_set.insert(peer_dag).first;
    }
    else {
      UShortArraySet& dag_set = m_it->second;
      d_it = dag_set.find(peer_dag);
      if (d_it == dag_set.end())
	d_it = dag_set.insert(peer_dag).first;
    }
    bestModelSetIter = m_it;  bestDAGIter = d_it;

    RealVector avg_eval_ratios(numApprox, false);  avg_eval_ratios = 1.;
    // For r_i = 1, C_G,c_g = 0 --> enforce linear constr based on curr DAG
    enforce_augmented_linear_ineq_constraints(avg_eval_ratios, fullApproxSet,
					      orderedRootList);
    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    Real avg_N_H = (backfillFailures) ?
      average(NLevActual[hf_form_index][hf_lev_index]) :
      NLevAlloc[hf_form_index][hf_lev_index];
    soln.anchored_solution_ratios(avg_eval_ratios, avg_N_H);
    no_solve_variances(soln);
  }
  // else leave best soln at previous iters and values
}


void NonDGenACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baseline,
		  const SizetArray& N_H_actual, size_t N_H_alloc,
		  IntRealMatrixMap& sum_L_shared,
		  Sizet2DArray& N_L_actual_shared,
		  IntRealMatrixMap& sum_L_refined,
		  Sizet2DArray& N_L_actual_refined,
		  SizetArray&   N_L_alloc, const MFSolutionData& soln)
{
  // Note: these results do not affect the iteration above and can be
  // performed after N_H has converged

  // The final approx set and DAG have been selected
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t num_approx = approx_set.size(), num_groups = num_approx + 1;
  SizetArray delta_N_G(num_groups);

  // inflate if needed
  inflate_lf_samples(approx_set, N_H_actual, N_H_alloc,
		     N_L_actual_shared, N_L_alloc);

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
  // Map from "horizontal" group incr to "vertical" model incr (see JCP: ACV).
  // > Base class overlay_group_sums() can be used for arbitrary DAG
  //   since modelGroups are defined with each group's root at the end
  //   (see unroll_reverse_dag_from_root(root,group)).  This is necessary
  //   to correctly define the "shared" accumulations.
  sum_L_shared = sum_L_baseline;
  overlay_group_sums(sum_G,         N_G_actual,
		     sum_L_shared,  N_L_actual_shared,
		     sum_L_refined, N_L_actual_refined);
}


void NonDGenACVSampling::
genacv_raw_moments(const IntRealMatrixMap& sum_L_covar,
		   const IntRealVectorMap& sum_H_covar,
		   const IntRealSymMatrixArrayMap& sum_LL_covar,
		   const IntRealMatrixMap& sum_LH_covar,
		   const SizetArray& N_covar,
		   const IntRealVectorMap& sum_H_baseline,
		   const SizetArray& N_baseline,
		   const IntRealMatrixMap& sum_L_shared,
		   const Sizet2DArray& N_L_shared, 
		   const IntRealMatrixMap& sum_L_refined,
		   const Sizet2DArray& N_L_refined, const MFSolutionData& soln)
{
  // no need to check optSubProblemForm since MFSolutionData stores as N_vec
  precompute_genacv_controls(soln.solution_variables());

  const UShortArray& approx_set = activeModelSetIter->first;
  size_t qoi, approx, inflate_approx, num_approx = approx_set.size();
  RealMatrix H_raw_mom(4, numFunctions);  RealVector beta(num_approx);
  for (int mom=1; mom<=4; ++mom) {
    const RealMatrix&          sum_L_m =    sum_L_covar.at(mom);
    const RealVector&          sum_H_m =    sum_H_covar.at(mom);
    const RealSymMatrixArray& sum_LL_m =   sum_LL_covar.at(mom);
    const RealMatrix&         sum_LH_m =   sum_LH_covar.at(mom);
    const RealVector&     sum_H_base_m = sum_H_baseline.at(mom);
    const RealMatrix&       sum_L_sh_m =   sum_L_shared.at(mom);
    const RealMatrix&      sum_L_ref_m =  sum_L_refined.at(mom);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // --------------------------------------
      // Compute beta for control variate terms
      // --------------------------------------
      // > uses either online or offline samples for covariance
      // > TO DO: support shared_approx_increment() --> baselineL
      compute_genacv_control(sum_L_m, sum_H_m[qoi], sum_LL_m[qoi], sum_LH_m,
			     N_covar[qoi], mom, qoi, approx_set, beta);
      // ------------------------------------------------------
      // Evaluate control variate terms to estimate raw moments
      // ------------------------------------------------------
      // > uses online samples for baseline, shared, refined
      Real& H_raw_mq = H_raw_mom(mom-1, qoi);
      H_raw_mq = sum_H_base_m[qoi] / N_baseline[qoi];// 1st term to be augmented
      for (approx=0; approx<num_approx; ++approx) {
	inflate_approx = approx_set[approx];
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << inflate_approx+1
	       << ": control variate beta = " << std::setw(9)
	       << beta[approx] << '\n';
	apply_control(sum_L_sh_m(qoi,inflate_approx),
		      N_L_shared[inflate_approx][qoi],
		      sum_L_ref_m(qoi,inflate_approx),
		      N_L_refined[inflate_approx][qoi],
		      beta[approx], H_raw_mq);
      }
    }
  }
  if (outputLevel >= NORMAL_OUTPUT) Cout << std::endl;

  convert_moments(H_raw_mom, momentStats); // uncentered to final (central|std)
  compute_mean_confidence_intervals(momentStats,
    final_solution_data().estimator_variances(), meanCIs);
}


bool NonDGenACVSampling::precompute_allocations()
{
  // reset for each search over model sets and DAGs; otherwise previous
  // best can hide new best (new iter using more resolved covariances)
  meritFnStar = DBL_MAX;

  bool budget_constrained = (maxFunctionEvals != SZ_MAX), no_solve = false;
  // no_solve for any iteration:
  if (budget_constrained)
    no_solve = (equivHFEvals >= (Real)maxFunctionEvals); // budget exhausted

  if (mlmfIter == 0 && ( pilotMgmtMode == ONLINE_PILOT ||
			 pilotMgmtMode == ONLINE_PILOT_PROJECTION) ) {
    cache_mc_reference();// {estVar,numH}Iter0, estVarMetric0
    // no_solve augmentation for online iter 0:
    if (!budget_constrained) // accuracy controlled by convergenceTol
      no_solve = (convergenceTolType == RELATIVE_CONVERGENCE_TOLERANCE) ?
	(convergenceTol >= 1.) : (estVarMetric0  <= convergenceTol);
  }
  // Offline accuracy-constrained is allowed with absolute tol, but is
  // not available in advance of numerical solve (estVarMetric appears
  // in nln_ineq_con, but pilot estVarMetric0 is not tracked)

  return no_solve;
}


void NonDGenACVSampling::compute_allocations(MFSolutionData& soln)
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

  const UShortArray& approx_set = activeModelSetIter->first;
  if (mlmfIter == 0) {

    size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
    SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
    size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
    Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;

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
      covariance_to_correlation_sq(covLH, covLL, varH, rho2_LH);
      MFSolutionData mf_soln, cv_soln;
      analytic_initialization_from_mfmc(approx_set, rho2_LH, avg_N_H, mf_soln);
      analytic_initialization_from_ensemble_cvmc(approx_set, *activeDAGIter,
	orderedRootList, rho2_LH, avg_N_H, cv_soln);
      competed_initial_guesses(mf_soln, cv_soln, soln);
      break;
    }
    }
  }
  else { // subsequent iterations

    // no scaling needed from prev soln (as in NonDLocalReliability) since
    // updated avg_N_H now includes allocation from previous solution and
    // should be active on constraint bound (excepting sample count rounding)

    // warm start from previous soln for corresponding {approx_set,active_dag}
    // Note: for sequenced minimizers, only the last is used for refinement
    ensemble_numerical_solution(soln);
  }

  process_model_allocations(soln, numSamples);
  if (outputLevel >= NORMAL_OUTPUT)
    print_model_allocations(Cout, soln, approx_set);
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

  Real hf_target;
  if (maxFunctionEvals == SZ_MAX)// HF target from GenACV estvar using MFMC soln
    hf_target = (convergenceTolType == ABSOLUTE_CONVERGENCE_TOLERANCE) ?
      update_hf_target(avg_eval_ratios, avg_N_H, varH) :
      update_hf_target(avg_eval_ratios, avg_N_H, varH, estVarIter0);
  else // allocate_budget(), then manage lower bounds and pilot over-estimation
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, hf_target,
		    approx_set, orderedRootList, activeBudget);
  soln.anchored_solution_ratios(avg_eval_ratios, hf_target);
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

  Real hf_target;
  if (maxFunctionEvals == SZ_MAX) {
    // scale according to accuracy (convergenceTol * estVarIter0)
    enforce_augmented_linear_ineq_constraints(avg_eval_ratios, approx_set,
					      root_list);
    hf_target = (convergenceTolType == ABSOLUTE_CONVERGENCE_TOLERANCE) ?
      update_hf_target(avg_eval_ratios, avg_N_H, varH) :
      update_hf_target(avg_eval_ratios, avg_N_H, varH, estVarIter0);
  }
  else { // scale according to cost
    // incorporates lin ineq enforcement:
    scale_to_target(avg_N_H, sequenceCost, avg_eval_ratios, hf_target,
		    approx_set, root_list, activeBudget);
    //RealVector cd_vars;
    //r_and_N_to_design_vars(avg_eval_ratios, hf_target, cd_vars);
    //soln.estimator_variances(estimator_variances(cd_vars));
  }
  soln.anchored_solution_ratios(avg_eval_ratios, hf_target);

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "GenACV scaled initial guess from ensemble CVMC:\n"
	 << "  average eval ratios:\n" << avg_eval_ratios << "  HF target = "
	 << hf_target << std::endl;
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

  size_t i, num_approx = activeModelSetIter->first.size();
  const UShortArray& dag = *activeDAGIter;
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    num_cdv = num_approx;  num_nln_con = 0;
    num_lin_con = 1;
    for (i=0; i<num_approx; ++i)
      if (dag[i] != numApprox)
	++num_lin_con;
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    num_cdv = num_approx + 1;  num_nln_con = 1;
    num_lin_con = 0;
    for (i=0; i<num_approx; ++i)
      if (dag[i] != numApprox)
	++num_lin_con;
    break;
  case N_MODEL_LINEAR_CONSTRAINT:
    num_cdv = num_lin_con = num_approx + 1;  num_nln_con = 0;
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
  Real offline_N_lwr = 1.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;
  bool offline = (pilotMgmtMode == OFFLINE_PILOT ||
		  pilotMgmtMode == OFFLINE_PILOT_PROJECTION);

  // --------------------------------------
  // Formulate the optimization sub-problem: initial pt, bnds, constraints
  // --------------------------------------
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_cdv = x0.length(), approx, num_approx = approx_set.size();
  Real cost_H = sequenceCost[numApprox];

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
      activeBudget / avg_N_H - 1. : // normal case
      activeBudget - 1.; // bound N_H at 1 (TO DO: need to perform sample)
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
    nln_ineq_ub[0] = activeBudget;
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
      lin_ineq_ub[0] = activeBudget; // remaining ub
      for (approx=0; approx<num_approx; ++approx)
	lin_ineq_coeffs(0, approx) = sequenceCost[approx_set[approx]] / cost_H;
      lin_ineq_coeffs(0, num_approx) = 1.;
    }
    else if (optSubProblemForm == N_MODEL_LINEAR_OBJECTIVE) {
      // nonlinear constraint on estvar
      nln_ineq_lb = -DBL_MAX;  // no lower bnd
      nln_ineq_ub = (convergenceTolType == ABSOLUTE_CONVERGENCE_TOLERANCE)
	? std::log(convergenceTol) : std::log(convergenceTol * estVarMetric0);
    }
    break;
  }
  }

  // use a lb specification that satisfies linear constraints
  // > some optimizers may use or visit corners, violating linear cons,
  //   so make these corners more robust
  // > should not be strictly necessary, but may improve robustness
  // > so far, testing artifacts show mixed effects with no clear trend;
  //   not surprising without more stress tests like weighted MLMC + DAG
  //   search (leaving commented out for now)
  //enforce_augmented_linear_ineq_constraints(x_lb);

  // x0 can undershoot x_lb if an OFFLINE mode, but enforce generally
  enforce_bounds(x0, x_lb, x_ub);
}


void NonDGenACVSampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  // Enforce DAG dependencies (ACV: all point to numApprox)
  // > N for each source model > N for model it targets
  // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)

  const UShortArray& dag = *activeDAGIter;
  const UShortArray& approx_set = activeModelSetIter->first;
  size_t i, num_approx = approx_set.size(), tgt;//, src;
  SizetArray index_map;  inflate_approx_set(approx_set, index_map);

  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_MODEL_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t offset_i, deflate_tgt, lin_ineq_offset
      = (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) ? 1 : 0;
    for (i=0; i<num_approx; ++i) { // N_src > N_tgt
      tgt = dag[i]; //src = approx_set[i];
      deflate_tgt = (tgt == numApprox) ? num_approx : index_map[tgt];
      offset_i = i+lin_ineq_offset;
      lin_ineq_coeffs(offset_i, i) = -1.;
      lin_ineq_coeffs(offset_i, deflate_tgt) = 1. + RATIO_NUDGE;
    }
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT: // active for truthFixedByPilot && !offline
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    size_t cntr = (optSubProblemForm == R_ONLY_LINEAR_CONSTRAINT) ? 1 : 0;
    for (i=0; i<num_approx; ++i) {
      tgt = dag[i]; //src = approx_set[i];
      //offset_i = i+lin_ineq_offset;
      //if (tgt == numApprox) { // r_i > 1. handled by bounds + nudge
      //  lin_ineq_coeffs(offset_i, i) = 1.;
      //  lin_ineq_lb(offset_i) = 1.;
      //  lin_ineq_ub(offset_i) = DBL_MAX;
      //}
      if (tgt != numApprox) { // r_src > r_tgt
	lin_ineq_coeffs(cntr, i) = -1.;
	lin_ineq_coeffs(cntr, index_map[tgt]) = 1. + RATIO_NUDGE;
	++cntr;
      }
    }
    break;
  }
  }
}


void NonDGenACVSampling::
enforce_augmented_linear_ineq_constraints(RealVector& avg_eval_ratios,
					  const UShortArray& approx_set,
					  const UShortList& root_list)
{
  // Enforce DAG dependencies (ACV: all point to numApprox)
  // > N for each source model > N for model it targets
  // > Avoids negative z2 = z - z1 in IS/RD (--> questionable G,g numerics)
  UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
  unsigned short source, target;
  Real r_tgt, r_tgt_nudge, nudge_p1 = RATIO_NUDGE + 1.;
  SizetArray index_map;  inflate_approx_set(approx_set, index_map);
  for (r_cit=root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
    target = *r_cit;  const UShortSet& reverse_dag = reverseActiveDAG[target];
    r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[index_map[target]];
    r_tgt_nudge = r_tgt * nudge_p1;
    for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
      source = *d_cit;
      Real& r_src = avg_eval_ratios[index_map[source]];
      if (r_src < r_tgt_nudge) {
	r_src = r_tgt_nudge;
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "Enforcing source = " << source << " target = " << target
	       << ": r_src = " << r_src << " r_tgt = "<< r_tgt << std::endl;
      }
    }
  }
}


void NonDGenACVSampling::
enforce_augmented_linear_ineq_constraints(RealVector& cd_vars)
{
  // Outer loops: GenACV over DAGs + model subsets
  // > generate_reverse_dag(), unroll_reverse_dag_from_root():
  //   activeDAGIter,activeModelSetIter --> reverseDAG,orderedRootList
  // > Inner loop: compute_allocations --> run_minimizers()
  //   >> each min_ij.run() within global + competed local
  //   >> this fn invoked across sequencing for warm starting from previous best
  // > update_best() --> bestDAGIter,bestModelSetIter 
  // Therefore, map to currently active DAG + model subset (not best)

  const UShortArray& approx_set = activeModelSetIter->first;
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:     // trap target == numApprox
  case R_AND_N_NONLINEAR_CONSTRAINT: // replace N with 1
    enforce_augmented_linear_ineq_constraints(cd_vars, approx_set,
					      orderedRootList);
    break;
  default: {                         // use N directly rather than r
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;
    size_t num_approx = approx_set.size(), deflate_tgt;
    Real N_tgt, N_tgt_nudge, nudge_p1 = RATIO_NUDGE + 1.;
    SizetArray index_map;  inflate_approx_set(approx_set, index_map);
    for (r_cit=orderedRootList.begin(); r_cit!=orderedRootList.end(); ++r_cit) {
      target = *r_cit;
      deflate_tgt = (target == numApprox) ? num_approx : index_map[target];
      N_tgt = cd_vars[deflate_tgt];  N_tgt_nudge = N_tgt * nudge_p1;
      const UShortSet& reverse_dag = reverseActiveDAG[target];
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit;
	Real& N_src = cd_vars[index_map[source]];
	if (N_src < N_tgt_nudge) {
	  N_src = N_tgt_nudge;
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "Enforcing source = " << source << " target = " << target
		 << ": N_src = " << N_src << " N_tgt = "<< N_tgt << std::endl;
	}
      }
    }
    break;
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
    // > Avoids negative z2 = z - z1 in IS/RD (--> failure in G,g numerics)
    UShortList::const_iterator r_cit;  UShortSet::const_iterator d_cit;
    unsigned short source, target;
    Real r_tgt, r_tgt_nudge, cost_r_src, budget_decr, inner_prod_decr,
      nudge_p1 = RATIO_NUDGE + 1.;
    SizetArray index_map;  inflate_approx_set(approx_set, index_map);
    for (r_cit=root_list.begin(); r_cit!=root_list.end(); ++r_cit) {
      target = *r_cit; const UShortSet& reverse_dag = reverseActiveDAG[target];
      r_tgt = (target == numApprox) ? 1. : avg_eval_ratios[index_map[target]];
      r_tgt_nudge = r_tgt * nudge_p1;
      budget_decr = inner_prod_decr = 0.;
      for (d_cit=reverse_dag.begin(); d_cit!=reverse_dag.end(); ++d_cit) {
	source = *d_cit;  Real& r_src = avg_eval_ratios[index_map[source]];
	r_src *= factor;
	if (r_src < r_tgt_nudge) {
	  r_src = r_tgt_nudge;
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
    enforce_augmented_linear_ineq_constraints(avg_eval_ratios, approx_set,
					      root_list);
}


void NonDGenACVSampling::
solve_for_C_G_c_g(RealSymMatrix& C_G, RealVector& c_g, RealVector& lhs,
		  bool copy_C_G, bool copy_c_g)
{
  // The idea behind this approach is to leverage both the solution refinement
  // in solve() and equilibration during factorization (inverting C_G in place
  // can only leverage the latter).

  size_t n = c_g.length();
  if (lhs.length() != n) lhs.size(n); // not sure if initialization matters

  if (hardenNumericSoln) {
    RealMatrix C_G_inv;  Real rcond;
    pseudo_inverse(C_G, C_G_inv, rcond);
    lhs.multiply(Teuchos::NO_TRANS, Teuchos::NO_TRANS, 1., C_G_inv, c_g, 0.);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "GenACV pseudo-inverse solve for LHS:\n" << lhs << "has rcond = "
	   << rcond << std::endl;
  }
  else
    cholesky_solve(C_G, lhs, c_g, copy_C_G, copy_c_g);
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
minimizer_results_to_solution_data(const RealVector& cv_star,
				   const RealVector& fn_star,
				   MFSolutionData& soln)
{
  // Estvar recovery from optimizer provides std::log(estvar_metric).  Notes:
  // > ultimately need a QoI-vector PRIOR to scalar metric reduction, but defer
  //   recomputation until after solution selection
  // > optimal estvar value corresponds to N* (_after_ num_samples applied)
  Real estvar_metric = (optSubProblemForm == N_MODEL_LINEAR_OBJECTIVE) ?
    std::exp(fn_star[1]) : std::exp(fn_star(0));
  soln.estimator_variance_metric(estvar_metric); // QoI-vec evaluated downstream

  // Recover optimizer results for average {eval_ratios,estvar}.  Also compute
  // shared increment from N* or from targeting specified budget || accuracy.
  const UShortArray& approx_set = activeModelSetIter->first;
  int num_approx = (int)approx_set.size();
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    const RealVector& avg_eval_ratios = cv_star; // r*
    Real              hf_target;             // N*
    // N* was not part of the optimization (solver computes r* for fixed N)
    // and has not been updated by the optimizer.  We update it here:

    // Allow for constraint to be inactive at optimum, but generally the
    // opt sub-problem will allocate full budget to increase R^2.
    // Note: this formulation is active for option "truth_fixed_by_pilot"
    if (maxFunctionEvals == SZ_MAX) {
    //if (convergenceTol != -DBL_MAX) { // *** TO DO: detect user spec
      // EstVar target = convTol * estvar_iter0 = estvar_ratio * varH / N_target
      //               = curr_estvar * N_curr / N_target
      //  --> N_target = curr_estvar * N_curr / (convTol * estvar_iter0)
      size_t hf_form_index, hf_lev_index;
      hf_indices(hf_form_index, hf_lev_index);
      SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
      size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
      Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
      hf_target = (convergenceTolType == ABSOLUTE_CONVERGENCE_TOLERANCE) ?
	update_hf_target(avg_eval_ratios, avg_N_H, varH) :
	update_hf_target(avg_eval_ratios, avg_N_H, varH, estVarIter0);
      Cout << "Scaling profile for convergenceTol = " << convergenceTol
	   << ": HF target = " << hf_target << std::endl;
    }
    else {
      // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L
      // > can also under-relax the budget allocation to enable additional N_H
      //   increments + associated shared sample sets to refine shared stats.
      hf_target = allocate_budget(approx_set, avg_eval_ratios, sequenceCost);
      Cout << "Scaling profile for active budget = " << activeBudget
	   << ": HF target = " << hf_target << std::endl;
    }
    //hf_target = std::min(budget_target, ctol_target); // enforce both
    soln.anchored_solution_ratios(avg_eval_ratios, hf_target);
    soln.equivalent_hf_allocation(compute_equivalent_cost(hf_target,
				  avg_eval_ratios, sequenceCost, approx_set));
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


/** Multi-moment map-based version used by GenACV following shared_increment().
    Differs from ACV version in accumulations of N_L_shared, since dependent
    on actve model subset. */
void NonDGenACVSampling::
accumulate_genacv_sums(IntRealMatrixMap& sum_L_shared, IntRealVectorMap& sum_H,
		       IntRealSymMatrixArrayMap& sum_LL,// L w/ itself + other L
		       IntRealMatrixMap&         sum_LH,// each L with H
		       RealVector& sum_HH, SizetArray& N_H_shared,
		       Sizet2DArray& N_L_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  size_t qoi, approx, lf_index, hf_index, num_am1 = numApprox+1;
  IntRespMCIter r_it;

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
	  lf_index = approx * numFunctions + qoi;
	  if (asv[lf_index] & 1) {
	    // for GenACV with some models active, increment N_L separately
	    ++N_L_shared[approx][qoi];
	    accumulate_lf_hf_qoi(fn_vals, asv, qoi, approx, sum_L_shared,
				 sum_LL, sum_LH);
	  }
	}
      }
    }
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

  //Cout << "Call " << ++compGgCntr << " to compute_param_G_g()" << std::endl;
  //if (compGgCntr == 385)
  //  Cout << "Here.\n";

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

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "For dag:\n"; print_dag(active_dag, approx_set);
    Cout << "G matrix:\n" << GMat << "g vector:\n" << gVec << std::endl;
  }
}


void NonDGenACVSampling::
unroll_z1_z2(const RealVector& N_vec, RealVector& z1, RealVector& z2)
{
  // z1, z2 denote z_m^1 and z_m^2 from ACV paper: Gorodetsky et al., JCP 2020
  // > N_vec,z1,z2 are inflated to full dimension, avoiding the need to map
  //   indices (DAG values to sample count indices) 

  //Real z_H = N_vec[numApprox];
  z1.size(numGroups);  z1[numApprox] = 0;                // JCP ACV Fig 2 "z"
  z2.size(numGroups);  z2[numApprox] = N_vec[numApprox]; // JCP ACV Fig 2 "z"

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_MF: { // not used (special unroll logic not required)
    const UShortArray& approx_set = activeModelSetIter->first;
    const UShortArray& active_dag = *activeDAGIter;
    unsigned short i, source, target;  size_t dag_size = active_dag.size();
    for (i=0; i<dag_size; ++i) {
      source = approx_set[i];  target = active_dag[i];
      //z1[i] = N_vec[target];  z2[i] = N_vec[source];    // COMPACTED z1,z2
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
	z1[source] = N_vec[target] - z1t; // Bomarito Fig 6, JCP ACV Fig 2c
	z2[source] = N_vec[source];
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
	z1s = z2t;                        // Bomarito Fig 5, JCP ACV Fig 2a
	z2[source] = N_vec[source] - z1s;
      }
    }
    break;
  }
  default:
    Cerr << "Error: unsupported mlmfSubMethod in unroll_z1_z2()" << std::endl;
    abort_handler(METHOD_ERROR);  break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "GenACV unroll of N_vec:\n" << N_vec << "into z1:\n" << z1
	 << "and z2:\n" << z2 << std::endl;
}


void NonDGenACVSampling::update_best(MFSolutionData& soln)
{
  // Update tracking of best result

  bool update = false;  Real merit_fn;
  if (!valid_variances(soln.estimator_variances()))
    update = false;
  else {
    merit_fn = nh_penalty_merit(soln);
    update = (merit_fn < meritFnStar);
  }
  if (update) {
    bestModelSetIter = activeModelSetIter;
    bestDAGIter      = activeDAGIter;
    meritFnStar      = merit_fn;
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Updating best DAG to:\n";
      print_dag(*bestDAGIter, bestModelSetIter->first);
    }
  }
}


void NonDGenACVSampling::print_best()
{
  //Cout << "\n>>>>> Approx subset and DAG evaluation completed\n" << std::endl;

  if (bestModelSetIter == modelDAGs.end()) {
    Cout << "Warning: best DAG has not been updated in restore_best().\n"
	 << "         Last active DAG will be used." << std::endl;
    return;
  }

  const UShortArray& best_models =  bestModelSetIter->first;
  const UShortArray& best_dag    = *bestDAGIter;
  Cout << "\nBest solution from DAG:\n";
  print_dag(best_dag, best_models);

  if (outputLevel >= DEBUG_OUTPUT) {
    std::pair<UShortArray, UShortArray> soln_key(best_models, best_dag);
    Cout << "\nwith solution variables =\n"
	 << dagSolns[soln_key].solution_variables() << std::endl;
  }
}


void NonDGenACVSampling::restore_best()
{
  // restore best state for compute/archive/print final results
  bool approx_incr = ( finalStatsType == QOI_STATISTICS &&
    ( pilotMgmtMode == ONLINE_PILOT || pilotMgmtMode == OFFLINE_PILOT ) );

  const UShortArray& best_models =  bestModelSetIter->first;
  const UShortArray& best_dag    = *bestDAGIter;
  if (activeModelSetIter != bestModelSetIter ||
      activeDAGIter      != bestDAGIter) { // best is not most recent
    activeModelSetIter = bestModelSetIter;
    activeDAGIter      = bestDAGIter;
    //activeBudget     = active_budget(); // for completeness?
    if (approx_incr) generate_reverse_dag(best_models, best_dag);
  }
  // now we can re-order roots based on final eval ratios solution
  // --> used for pyramid sample set ordering in approx_increments()
  if (approx_incr) {
    std::pair<UShortArray, UShortArray> soln_key(best_models, best_dag);
    MFSolutionData& best_soln = dagSolns[soln_key];
    unroll_reverse_dag_from_root(numApprox, best_soln.solution_ratios(),
				 orderedRootList);
  }
}

} // namespace Dakota
