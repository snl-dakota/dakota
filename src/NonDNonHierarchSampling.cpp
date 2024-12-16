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
#include "DakotaResponse.hpp"
#include "NonDNonHierarchSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "MinimizerAdapterModel.hpp"
#include "DataFitSurrModel.hpp"
#include "DataFitSurrBasedLocalMinimizer.hpp"
#include "EffGlobalMinimizer.hpp"
#include "NonDLHSSampling.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif
#ifdef HAVE_NCSU
#include "NCSUOptimizer.hpp"
#endif

static const char rcsId[]="@(#) $Id: NonDNonHierarchSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {

// initialization of statics
NonDNonHierarchSampling* NonDNonHierarchSampling::nonHierSampInstance(NULL);


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDNonHierarchSampling::
NonDNonHierarchSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDEnsembleSampling(problem_db, model), optSubProblemForm(0),
  truthFixedByPilot(problem_db.get_bool("method.nond.truth_fixed_by_pilot"))
{
  // solver(s) that perform the numerical solution for resource allocations
  optSubProblemSolver = sub_optimizer_select(
    probDescDB.get_ushort("method.nond.opt_subproblem_solver"),
    SUBMETHOD_DIRECT_NPSOL_OPTPP); // default is global + competed local
  if (!optSubProblemSolver) // error messages output by sub_optimizer_select()
    abort_handler(METHOD_ERROR);

  size_t num_forms_resolutions;
  configure_enumeration(num_forms_resolutions, sequenceType);
  numApprox = num_forms_resolutions - 1;
  if (methodName != MULTILEVEL_BLUE) // else deferred until ML BLUE ctor
    numGroups = num_forms_resolutions;

  // Precedence: if solution costs provided, then we use them;
  // else we rely on online cost recovery through response metadata
  costSource = configure_cost(num_forms_resolutions, sequenceType,
			      sequenceCost, costMetadataIndices);
}


NonDNonHierarchSampling::~NonDNonHierarchSampling()
{ }


/*
bool NonDNonHierarchSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}
*/


void NonDNonHierarchSampling::pre_run()
{
  NonDEnsembleSampling::pre_run();

  nonHierSampInstance = this;
  deltaNActualHF = 0;

  /* Numerical solves do not involve use of iteratedModel

  // Prevent nesting of an instance of a Fortran iterator within another
  // instance of the same iterator.  Run-time check since NestedModel::
  // subIterator is constructed in init_communicators().
  if (optSubProblemSolver == SUBMETHOD_...) {
    Iterator sub_iterator = iteratedModel.subordinate_iterator();
    if (!sub_iterator.is_null() && 
	( sub_iterator.method_name() == ... ||
	  sub_iterator.uses_method() == ... ) )
      sub_iterator.method_recourse(methodName);
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	  ( sub_iterator.method_name() == ... ||
	    sub_iterator.uses_method() == ... ) )
	sub_iterator.method_recourse(methodName);
    }
  }
  */

  // assign an aggregate model key that persists for core_run()
  assign_active_key();
}


void NonDNonHierarchSampling::assign_active_key()
{
  Pecos::ActiveKey active_key;
  std::vector<Pecos::ActiveKey> form_res_keys(numApprox+1);

  // case Pecos::FORM_RESOLUTION_ENUMERATION
  ModelList& sub_models = iteratedModel->subordinate_models(false);// incl HF
  size_t m, l, num_lev, cntr = 0;  ModelLIter m_iter;
  for (m=0,m_iter=sub_models.begin(); m_iter!=sub_models.end(); ++m_iter,++m){
    num_lev = (*m_iter)->solution_levels(); // lower bound of 1
    for (l=0; l<num_lev; ++l, ++cntr)
      form_res_keys[cntr].form_key(0, m, l);
  }

  active_key.aggregate_keys(form_res_keys, Pecos::RAW_DATA);
  //iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
  iteratedModel->active_model_key(active_key); // data group 0
  resize_active_set();
}


void NonDNonHierarchSampling::
hf_indices(size_t& hf_form_index, size_t& hf_lev_index)
{
  // case Pecos::FORM_RESOLUTION_ENUMERATION
  hf_form_index = NLevActual.size() - 1;
  hf_lev_index  = NLevActual[hf_form_index].size() - 1;
}


void NonDNonHierarchSampling::shared_increment(String prepend)
{
  if (mlmfIter == 0) Cout << "\nNon-hierarchical pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << mlmfIter
	    << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
    //iteratedModel.active_model_key(agg_key);
    //resize_active_set();

    activeSet.request_values(1);
    ensemble_sample_increment(prepend, numGroups); // block on single batch
  }
}


void NonDNonHierarchSampling::
shared_increment(String prepend, const UShortArray& approx_set)
{
  if (mlmfIter == 0) Cout << "\nNon-hierarchical pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << mlmfIter
	    << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
    //iteratedModel.active_model_key(agg_key);
    //resize_active_set();

    // active approximation subset
    ensemble_active_set(approx_set);
    // truth (not included in approx_set above)
    size_t start = numApprox * numFunctions;
    activeSet.request_values(1, start, start + numFunctions); // augment approx

    ensemble_sample_increment(prepend, numGroups); // block on single batch
  }
}


void NonDNonHierarchSampling::shared_approx_increment(String prepend)
{
  if (mlmfIter == 0) Cout << "\nNon-hierarchical approx pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << mlmfIter
	    << ": shared approx sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
    //iteratedModel.active_model_key(agg_key);
    //resize_active_set();

    size_t approx_qoi = numApprox  * numFunctions,
                  end = approx_qoi + numFunctions;
    activeSet.request_values(1, 0,   approx_qoi); // all approx QoI
    activeSet.request_values(0, approx_qoi, end); //   no truth QoI

    ensemble_sample_increment(prepend, numApprox); // block on single batch
  }
}


bool NonDNonHierarchSampling::
approx_increment(String prepend, const SizetArray& approx_sequence,
		 size_t start, size_t end)
{
  if (numSamples && start < end) {
    Cout << "\nApprox sample increment = " << numSamples << " for approximation"
	 << " sequence [" << start+1 << ", " << end << ']' << std::endl;

    bool ordered = approx_sequence.empty();
    size_t i, approx, start_qoi;

    activeSet.request_values(0);
    for (size_t i=start; i<end; ++i) { // [start,end)
      approx = (ordered) ? i : approx_sequence[i];
      start_qoi = approx * numFunctions;
      activeSet.request_values(1, start_qoi, start_qoi + numFunctions);
    }

    ensemble_sample_increment(prepend, start); // block on single batch
    return true;
  }
  else {
    Cout << "\nNo approx sample increment for approximation sequence ["
	 << start+1 << ", " << end << ']' << std::endl;
    return false;
  }
}


bool NonDNonHierarchSampling::
approx_increment(String prepend, const SizetArray& approx_sequence,
		 size_t start, size_t end, const UShortArray& approx_set)
{
  if (numSamples && start < end) {
    Cout << "\nApprox sample increment = " << numSamples << " for approximation"
	 << " sequence [" << start+1 << ", " << end << ']' << std::endl;

    bool ordered = approx_sequence.empty();
    size_t i, approx, start_qoi;

    activeSet.request_values(0);
    for (size_t i=start; i<end; ++i) { // [start,end)
      approx = (ordered) ? i : approx_sequence[i];    // compact indexing
      start_qoi = approx_set[approx] * numFunctions; // inflated indexing
      activeSet.request_values(1, start_qoi, start_qoi + numFunctions);
    }

    ensemble_sample_increment(prepend, start); // block on single batch
    return true;
  }
  else {
    Cout << "\nNo approx sample increment for approximation sequence ["
	 << start+1 << ", " << end << ']' << std::endl;
    return false;
  }
}


bool NonDNonHierarchSampling::
approx_increment(String prepend, unsigned short root,
		 const UShortSet& reverse_dag)
{
  UShortSet::const_iterator cit;
  if (numSamples) Cout << "\nApprox sample increment = " << numSamples;
  else            Cout << "\nNo approx sample increment";
  Cout << " for root node " << root;
  if (!reverse_dag.empty()) {
    Cout << " and its leaf nodes { ";
    for (cit=reverse_dag.begin(); cit!=reverse_dag.end(); ++cit)
      Cout << *cit << ' ';
    Cout << '}';
  }
  Cout << '.' << std::endl;

  if (numSamples) {
    // Evaluate shared samples across a dependency: each z1[leaf] = z2[root]
    activeSet.request_values(0);
    size_t start_qoi = root * numFunctions;
    activeSet.request_values(1, start_qoi, start_qoi + numFunctions);
    for (cit=reverse_dag.begin(); cit!=reverse_dag.end(); ++cit) {
      start_qoi = *cit * numFunctions;
      activeSet.request_values(1, start_qoi, start_qoi + numFunctions);
    }

    ensemble_sample_increment(prepend, root); // block on single batch
    return true;
  }
  else
    return false;
}


void NonDNonHierarchSampling::
group_increments(SizetArray& delta_N_G, String prepend, bool reverse_order)
{
  if (mlmfIter == 0) Cout << "\nPerforming pilot sample for model groups.\n";
  else Cout << "\nGroup sampling iteration " << mlmfIter
	    << ": sample increment =\n" << delta_N_G << '\n';

  // Ordering does not impact evaluation management, but does impact random
  // number sequencing across calls to get_parameter_sets()
  size_t num_groups = modelGroups.size();
  if (reverse_order) // high to low ordering (e.g., bottom-up pyramid)
    for (int g=num_groups-1; g>=0; --g) {
      numSamples = delta_N_G[g];
      if (numSamples) {
	ensemble_active_set(modelGroups[g]);
	ensemble_sample_batch(prepend, g); // index is group_id, non-blocking
      }
    }
  else // low to high ordering (e.g. combinatorial defn of ML BLUE modelGroups)
    for (size_t g=0; g<num_groups; ++g) {
      numSamples = delta_N_G[g];
      if (numSamples) {
	ensemble_active_set(modelGroups[g]);
	ensemble_sample_batch(prepend, g); // index is group_id, non-blocking
      }
    }

  if (iteratedModel->asynch_flag())
    synchronize_batches(*iteratedModel); // schedule all groups (return ignored)
}


size_t NonDNonHierarchSampling::
group_approx_increment(const RealVector& soln_vars,
		       const UShortArray& approx_set,
		       const Sizet2DArray& N_L_actual, SizetArray& N_L_alloc,
		       const UShortArray& model_group)
{
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment allowing use of
  //   one_sided_delta() with latest counts
  // > No relaxation is used for approx increments since iteration is complete

  size_t num_samp, root = model_group.back(), // root must be last model
    deflate_root = (approx_set.size() == numApprox) ? root :
    find_index(approx_set, root);
  // soln variables sized as {approx_set,truth} with ascending model order
  Real lf_target = soln_vars[deflate_root];
  if (backfillFailures) {
    const SizetArray& lf_curr = N_L_actual[root];
    num_samp = one_sided_delta(lf_curr, lf_target); // delta of average
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << num_samp << " computed from average delta "
	   << "between LF target = " << lf_target << " and current counts:\n"
	   << lf_curr << std::endl;
    size_t N_alloc = one_sided_delta(N_L_alloc[root], lf_target);
    increment_sample_range(N_L_alloc, N_alloc, model_group);
  }
  else {
    size_t lf_curr = N_L_alloc[root];
    num_samp = one_sided_delta((Real)lf_curr, lf_target);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << num_samp << " computed from delta between "
	   << "LF target = " << lf_target << " and current allocation = "
	   << lf_curr << std::endl;
    increment_sample_range(N_L_alloc, num_samp, model_group);
  }
  return num_samp;
}


/*
size_t NonDNonHierarchSampling::
dag_approx_increment(const RealVector& soln_vars, const UShortArray& approx_set,
		     const Sizet2DArray& N_L_actual, SizetArray& N_L_alloc,
		     unsigned short root, const UShortSet& reverse_dag)
{
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment allowing use of
  //   one_sided_delta() with latest counts

  size_t num_samp, deflate_root = (approx_set.size() == numApprox) ? root :
    find_index(approx_set, root);
  Real lf_target = soln_vars[deflate_root]; // soln_vars sized by approx_set
  if (backfillFailures) {
    const SizetArray& lf_curr = N_L_actual[root];
    num_samp = one_sided_delta(lf_curr, lf_target); // delta of average
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << num_samp << " for root node index "
	   << root << " computed from delta between LF target = " << lf_target
	   << " and current counts:\n" << lf_curr << std::endl;
    size_t N_alloc = one_sided_delta(N_L_alloc[root], lf_target);
    increment_sample_range(N_L_alloc, N_alloc, root, reverse_dag);
  }
  else {
    size_t lf_curr = N_L_alloc[root];
    num_samp = one_sided_delta((Real)lf_curr, lf_target);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Approx samples = " << num_samp << " for root node index "
	   << root << " computed from delta between LF target = " << lf_target
	   << " and current allocation = " << lf_curr << std::endl;
    increment_sample_range(N_L_alloc, num_samp, root, reverse_dag);
  }
  return num_samp;
}
*/


void NonDNonHierarchSampling::
ensemble_sample_increment(const String& prepend, size_t step, bool new_samples)
{
  // Single sample batch case: define and evaluate (includes synchronization)

  if (new_samples) {
    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);
    // export separate output files for each data set
    export_sample_sets(prepend, step);
  }

  // compute allResponses from all{Samples,Variables} using model ensemble
  evaluate_parameter_sets(*iteratedModel); // includes synchronize
}


void NonDNonHierarchSampling::
ensemble_sample_batch(const String& prepend, size_t step, bool new_samples)
{
  // Queue one sample batch among multiple (excludes synchronization)

  if (new_samples) {
    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);
    // export separate output files for each data set
    export_sample_sets(prepend, step);
  }

  // evaluate all{Samples,Variables} using model ensemble and migrate
  // all{Samples,Variables} to batch{Samples,Variables}Map
  evaluate_batch(*iteratedModel, step); // excludes synchronize
}


void NonDNonHierarchSampling::
export_sample_sets(const String& prepend, size_t step)
{
  if (exportSampleSets) { // for HF+LF models, use the HF tags
    if (active_set_for_model(numApprox))
      export_all_samples(prepend, *iteratedModel->active_truth_model(),
			 mlmfIter, step);
    for (size_t i=0; i<numApprox; ++i)
      if (active_set_for_model(i))
	      export_all_samples(prepend, *iteratedModel->active_surrogate_model(i),
			   mlmfIter, step);
  }
}


void NonDNonHierarchSampling::
export_all_samples(const String& root_prepend, const Model& model, size_t iter,
		   size_t step)
{
  String tabular_filename(root_prepend);
  const String& iface_id = model.interface_id();
  size_t i, num_samp = allSamples.numCols();
  if (iface_id.empty()) tabular_filename += "NO_ID_i";
  else                  tabular_filename += iface_id + "_i";
  tabular_filename += std::to_string(iter) +  "_s" + std::to_string(step)
    + '_' + std::to_string(num_samp) + ".dat";

  NonDEnsembleSampling::export_all_samples(model, tabular_filename);
}


void NonDNonHierarchSampling::
mfmc_analytic_solution(const UShortArray& approx_set, const RealMatrix& rho2_LH,
		       const RealVector& cost, RealVector& avg_eval_ratios,
		       bool lower_bounded_r, bool monotonic_r)
{
  int a;  size_t qoi, num_approx = approx_set.size(), num_am1 = num_approx - 1;
  if (avg_eval_ratios.length() != num_approx) avg_eval_ratios.size(num_approx);
  else                                        avg_eval_ratios = 0.;

  unsigned short approx, prev_approx, last_approx = approx_set[num_am1];
  Real cost_L, cost_H = cost[numApprox], nudge_p1 = RATIO_NUDGE + 1.;
  // standard approach for well-ordered approxs
  RealVector factor(numFunctions, false);
  for (qoi=0; qoi<numFunctions; ++qoi)
    factor[qoi] = cost_H / (1. - rho2_LH(qoi, last_approx));
  for (a=0; a<num_approx; ++a) {
    approx = approx_set[a]; // approx_set is ordered but with omissions
    const Real* rho2_LH_m = rho2_LH[approx];  cost_L = cost[approx]; // full
    Real& avg_eval_ratio = avg_eval_ratios[a];               // contracted
    // NOTE: indexing is reversed from Peherstorfer
    // (HF = 1, MF = 2, LF = 3 becomes LF = 0, MF = 1, truth HF = 2)
    if (a)
      for (qoi=0; qoi<numFunctions; ++qoi)
	avg_eval_ratio += std::sqrt(factor[qoi] / cost_L *
	  (rho2_LH_m[qoi] - rho2_LH(qoi, prev_approx)));
    else // rho2_LH for m-1 (non-existent approx) is zero
      for (qoi=0; qoi<numFunctions; ++qoi)
	avg_eval_ratio += std::sqrt(factor[qoi] / cost_L * rho2_LH_m[qoi]);
    avg_eval_ratio /= numFunctions;
    prev_approx = approx;
  }

  // Reverse order for incremental lower bound enforcement: enforce r_i > 1
  // to protect numerics in mfmc_estvar_ratios()
  if (lower_bounded_r)
    for (a=num_am1; a>=0; --a) {
      // approx_set is ordered but with omissions
      Real& avg_eval_ratio = avg_eval_ratios[approx_set[a]];
      if (avg_eval_ratio < nudge_p1)
	{ avg_eval_ratio = nudge_p1; nudge_p1 += RATIO_NUDGE; }
    }

  // If requested (analytic stand-alone), enforce monotonicity in r_i
  if (monotonic_r) {
    Real r, prev_r = 1.;
    for (a=num_am1; a>=0; --a) {
      r = std::max(avg_eval_ratios[a], prev_r);
      prev_r = avg_eval_ratios[a] = r;
    }
  }
}


void NonDNonHierarchSampling::
mfmc_reordered_analytic_solution(const UShortArray& approx_set,
				 const RealMatrix& rho2_LH,
				 const RealVector& cost,
				 SizetArray& corr_approx_sequence,
				 RealVector& avg_eval_ratios,
				 bool lower_bounded_r, bool monotonic_r)
{
  int a;  size_t qoi, num_approx = approx_set.size(), num_am1 = num_approx-1;
  if (avg_eval_ratios.length() != num_approx)
    avg_eval_ratios.sizeUninitialized(num_approx);

  // employ a single approx reordering that is shared across the QoI
  RealVector avg_rho2_LH(num_approx); // init to 0.
  for (a=0; a<num_approx; ++a)
    avg_rho2_LH[a] = average(rho2_LH[approx_set[a]], numFunctions);
  bool ordered = ordered_approx_sequence(avg_rho2_LH, corr_approx_sequence);
  // Note: even if avg_rho2_LH is now ordered, rho2_LH is not for all QoI, so
  // stick with this alternate formulation, at least for this MFMC iteration.
  if (ordered)
    Cout << "MFMC: averaged correlations are well-ordered.\n" << std::endl;
  else
    Cout << "MFMC: average correlation-ordered approximation sequence "
	 << "(low to high):\n" << corr_approx_sequence << std::endl;

  // precompute a factor based on most-correlated approx
  unsigned short approx, inflate_approx;
  size_t most_corr = (ordered) ? num_am1 : corr_approx_sequence[num_am1];
  Real cost_L, cost_H = cost[numApprox], rho2, prev_rho2, rho2_diff,
    nudge_p1 = RATIO_NUDGE + 1.,
    factor = cost_H / (1. - avg_rho2_LH[most_corr]);// most correlated
  // Compute averaged eval ratios using averaged rho2 for corr_approx_sequence
  for (a=0; a<num_approx; ++a) {
    approx = (ordered) ? a : corr_approx_sequence[a];
    inflate_approx = approx_set[approx];  cost_L = cost[inflate_approx];// full
    // NOTE: indexing is inverted from Peherstorfer (i+1 becomes i-1)
    rho2_diff = rho2  = avg_rho2_LH[approx]; // contracted
    if (a) rho2_diff -= prev_rho2;
    avg_eval_ratios[approx] = std::sqrt(factor / cost_L * rho2_diff);
    prev_rho2 = rho2;
  }

  // Reverse order for incremental lower bound enforcement: enforce r_i > 1
  // to protect numerics in mfmc_estvar_ratios()
  if (lower_bounded_r)
    for (a=num_am1; a>=0; --a) {
      approx = (ordered) ? a : corr_approx_sequence[a];
      Real& avg_eval_ratio = avg_eval_ratios[approx];
      if (avg_eval_ratio < nudge_p1)
	{ avg_eval_ratio = nudge_p1; nudge_p1 += RATIO_NUDGE; }
    }

  // If requested (analytic stand-alone), enforce monotonicity in r_i for
  // rho ordering:
  // > max() is applied bottom-up from the base of the pyramid (samples
  //   performed bottom up, so precedence also applied in this direction),
  //   where assigning r_i = prev_ri effectively drops the CV for approx i
  if (monotonic_r) {
    Real r, prev_r = 1.;
    for (a=num_am1; a>=0; --a) {
      approx = (ordered) ? a : corr_approx_sequence[a];
      r = std::max(avg_eval_ratios[approx], prev_r);
      prev_r = avg_eval_ratios[approx] = r;
    }
  }
}


void NonDNonHierarchSampling::
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const RealVector& avg_eval_ratios,
		   SizetArray& approx_sequence, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);
  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, ip1;

  // Analytic cases: sequence fixed upstream based on rho2LH; r_i monotonicity
  //   enforced after analytic r_i computations for rho2LH sequence.
  // Numerical cases: update sequence on every call in NonDMultifidelitySampling
  //   ::estimator_variance_ratios().  Previously, was defined once and retained
  //   via linear constraints on opt solve, but is now reordered on the fly
  //   based on current optimizer design vars to retain validity of estvar.
  bool ordered = approx_sequence.empty();

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq.
  // > Note that R^2 can blow up (resulting in negative estvar_ratios) when
  //   r_i < 1 so we now prevent this in ordered/reordered analytic cases
  //   (as for numerical cases), even though it might only be an initial
  //   guess for something else.

  switch (optSubProblemForm) {

  // eval_ratios based on avg_rho2_LH: remain consistent here
  case REORDERED_ANALYTIC_SOLUTION: {
    RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
    approx = (ordered) ? 0 : approx_sequence[0];
    r_i = avg_eval_ratios[approx];  R_sq = 0.;
    for (ip1=1; ip1<numApprox; ++ip1) {
      approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
      r_ip1 = avg_eval_ratios[approx_ip1];
      R_sq += (r_i - r_ip1) / (r_i * r_ip1) * avg_rho2_LH[approx]; // see below
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
  // Note: ANALYTIC_SOLUTION covered by ordered case below
  default: {
    for (qoi=0; qoi<numFunctions; ++qoi) {
      approx = (ordered) ? 0 : approx_sequence[0];
      R_sq = 0.;  r_i = avg_eval_ratios[approx];
      for (ip1=1; ip1<numApprox; ++ip1) {
	approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
	r_ip1 = avg_eval_ratios[approx_ip1];
	// While eval ratio misordering won't induce numerical exceptions,
	// the derivation of this equation assumes ordered r_i:
	// > either retain a fixed ordering through linear constraints, or
	// > re-order on the fly when the optimization omits these guard rails
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


void NonDNonHierarchSampling::
cvmc_ensemble_solutions(const RealMatrix& rho2_LH, const RealVector& cost,
			RealVector& avg_eval_ratios, bool lower_bounded_r)
{
  if (avg_eval_ratios.empty()) avg_eval_ratios.size(numApprox);
  else                         avg_eval_ratios = 0.;

  // Compute an ensemble of pairwise CVMC solutions, all relative to HF:
  size_t qoi;  int a;  Real cost_ratio, rho_sq, cost_H = cost[numApprox];
  for (a=0; a<numApprox; ++a) {
    cost_ratio = cost_H / cost[a];
    const Real* rho2_LH_a = rho2_LH[a];
    Real&  avg_eval_ratio = avg_eval_ratios[a];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      rho_sq = rho2_LH_a[qoi];
      if (rho_sq < 1.) // prevent div by 0, sqrt(negative)
	avg_eval_ratio += std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      else // should not happen
	avg_eval_ratio += std::sqrt(cost_ratio / Pecos::SMALL_NUMBER);
    }
    avg_eval_ratio /= numFunctions;
  }

  // Reverse order for incremental lower bound enforcement: enforce r_i > 1
  // to protect numerics in mfmc_estvar_ratios()
  if (lower_bounded_r) {
    Real nudge_p1 = 1. + RATIO_NUDGE;
    for (int a=numApprox-1; a>=0; --a) {
      Real& avg_eval_ratio = avg_eval_ratios[a];
      if (avg_eval_ratio < nudge_p1)
	{ avg_eval_ratio = nudge_p1; nudge_p1 += RATIO_NUDGE; }
    }
  }
}


void NonDNonHierarchSampling::
raw_moments(const IntRealVectorMap& sum_H_baseline,
	    const SizetArray& N_baseline,
	    const IntRealMatrixMap& sum_L_shared,
	    const Sizet2DArray& N_L_shared,
	    const IntRealMatrixMap& sum_L_refined,
	    const Sizet2DArray& N_L_refined, const RealVector2DArray& beta)
{
  RealMatrix H_raw_mom(4, numFunctions);
  // ----------------------------
  // Apply CV to estimate moments
  // ----------------------------
  size_t approx, qoi;
  for (int mom=1; mom<=4; ++mom) {
    const RealVector& sum_H_base_m = sum_H_baseline.at(mom);
    const RealMatrix&   sum_L_sh_m =   sum_L_shared.at(mom);
    const RealMatrix&  sum_L_ref_m =  sum_L_refined.at(mom);
    const RealVectorArray&  beta_m =              beta[mom-1];

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      const RealVector& beta_mq = beta_m[qoi];
      Real& H_raw_mq = H_raw_mom(mom-1, qoi);
      H_raw_mq = sum_H_base_m[qoi] / N_baseline[qoi];// 1st term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1 << ": control "
	       << "variate beta = " << std::setw(9) << beta_m[approx] << '\n';
	// For MFMC, shared accumulators and counts telescope pairwise
	apply_control(sum_L_sh_m(qoi,approx),  N_L_shared[approx][qoi],
		      sum_L_ref_m(qoi,approx), N_L_refined[approx][qoi],
		      beta_mq[approx], H_raw_mq);
      }
    }
  }
  if (outputLevel >= NORMAL_OUTPUT) Cout << std::endl;

  convert_moments(H_raw_mom, momentStats); // uncentered to final (central|std)
}


void NonDNonHierarchSampling::update_model_group_costs()
{
  // modelGroupCost used in finite_solution_bounds() for
  // mfmc_numerical_solution().  MFMC numerical preserves approxSequence
  // in augment_linear_ineq_constraints(), so we use it here as well.

  // Note: GenACV can have active num_groups != numGroups
  size_t num_groups = modelGroups.size(); // active groups
  if (modelGroupCost.length() != num_groups)
    modelGroupCost.sizeUninitialized(num_groups);

  // Notes:
  // > Unlike ACV (refer to NonDACVSampling::update_model_group_costs()), MFMC
  //   numerical enforces approx sequence in augment_linear_ineq_constraints().
  //   Therefore, an increment for N_i *does* require corresponding increments
  //   for the more-correlated/higher-fidelity models in the approx sequence,
  //   Similar to the HF group of shared samples, we define these groupings for
  //   all design variables, which can reduce the search domain upper bounds.
  // > Extreme x_ub is N_shared plus one model at r_i = max within budget,
  //   with all sequence-dependent models at same value.
  // > Indexing is aligned with design vars: low to high with no reordering.

  size_t g, m, num_models;
  for (g=0; g<num_groups; ++g) {
    const UShortArray& group_g = modelGroups[g];
    Real& group_cost = modelGroupCost[g];  group_cost = 0.;
    num_models = group_g.size();
    for (m=0; m<num_models; ++m)
      group_cost += sequenceCost[group_g[m]];
  }
}


void NonDNonHierarchSampling::
overlay_group_sums(const IntRealMatrixArrayMap& sum_G,
		   const Sizet2DArray& N_G_actual,
		   IntRealMatrixMap& sum_L_shared,
		   Sizet2DArray& N_L_actual_shared,
		   IntRealMatrixMap& sum_L_refined,
		   Sizet2DArray& N_L_actual_refined)
{
  // This base implementation requires that the last model in the group
  // corresponds to the root of a reverse DAG, which then defines the "shared"
  // model accumulation as well as the basis for "refined" accumulations.
  // GenACV satisfies this requirement via unroll_reverse_dag_from_root(rt,grp)
  // and MFMC satisfies using ordered pyramid groups.
  // > ACV can use a more streamlined implementation since a peer DAG can
  //   bypass sum_L_shared accumulations (first loop below).
  // > We omit the last group (all-models) since (i) there is no HF increment
  //   (delta_N_G[numApprox] is assigned 0 in approx_increments()) and
  //   (ii) any HF refinement would be out of range for L accumulations.

  size_t m, g, num_L_groups = modelGroups.size() - 1, last_m_index;
  unsigned short model, last_model;
  IntRealMatrixArrayMap::const_iterator g_cit;
  IntRealMatrixMap::iterator s_it;
  for (g=0; g<num_L_groups; ++g) {
    const SizetArray&  num_G_g =  N_G_actual[g];
    if (zeros(num_G_g)) continue; // all-models group has delta = 0
    const UShortArray& group_g = modelGroups[g];
    last_m_index = group_g.size() - 1; // this index defines refined set

    for (m=0; m<last_m_index; ++m) {
      model = group_g[m];
      // counters (span all moments):
      increment_samples(N_L_actual_shared[model], num_G_g);
      // accumulators for each moment:
      for (s_it =sum_L_shared.begin(), g_cit =sum_G.begin();
	   s_it!=sum_L_shared.end() && g_cit!=sum_G.end(); ++g_cit, ++s_it)
	increment_sums(s_it->second[model], g_cit->second[g][m], numFunctions);
    }
  }

  // avoid redundant accumulations: copy shared state prior to refinement
  // (includes inflations from HF to L_shared)
  sum_L_refined      = sum_L_shared;
  N_L_actual_refined = N_L_actual_shared;

  for (g=0; g<num_L_groups; ++g) {
    const SizetArray&  num_G_g =  N_G_actual[g];
    if (zeros(num_G_g)) continue; // all-models group has delta = 0
    const UShortArray& group_g = modelGroups[g];
    // Note: HF model index is out of range for N_L --> num_G_g=0 protects this
    last_m_index = group_g.size() - 1;  last_model = group_g[last_m_index];
    // counters (span all moments):
    increment_samples(N_L_actual_refined[last_model], num_G_g);
    // accumulators for each moment:
    for (s_it =sum_L_refined.begin(), g_cit =sum_G.begin();
	 s_it!=sum_L_refined.end() && g_cit!=sum_G.end(); ++s_it, ++g_cit)
      increment_sums(s_it->second[last_model], g_cit->second[g][last_m_index],
		     numFunctions);
  }
}


void NonDNonHierarchSampling::
scale_to_target(Real avg_N_H, const RealVector& cost,
		RealVector& avg_eval_ratios, Real& avg_hf_target,
		Real budget, Real offline_N_lwr)
{
  // Numerical cases: scale to enforce budget constraint.
  // Since the profile does not emerge from pilot in ACV and numerical MFMC,
  // don't select an infeasible initial guess:
  // > if N* < N_pilot, scale back r* --> initial = scaled_r*,N_pilot
  // > if N* > N_pilot, use initial = r*,N*
  avg_hf_target = allocate_budget(avg_eval_ratios, cost, budget); // r* --> N*
  if (pilotMgmtMode == OFFLINE_PILOT ||
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    if (avg_N_H < offline_N_lwr)
      avg_N_H = offline_N_lwr;
  }
  if (avg_N_H > avg_hf_target) {// replace N* with N_pilot, rescale r* to budget
    avg_hf_target = avg_N_H;
    scale_to_budget_with_pilot(avg_eval_ratios, cost, avg_N_H, budget);
  }
}


void NonDNonHierarchSampling::
scale_to_budget_with_pilot(RealVector& avg_eval_ratios, const RealVector& cost,
			   Real avg_N_H, Real budget)
{
  // retain the shape of an r* profile, but scale to budget constrained by
  // incurred pilot cost
  // > for full model set only (not used by GenACV)

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "\nRescale to budget: incoming average evaluation ratios:\n"
	 << avg_eval_ratios;

  Real cost_r_i, approx_inner_prod = 0.;  size_t approx;
  for (approx=0; approx<numApprox; ++approx)
    approx_inner_prod += cost[approx] * avg_eval_ratios[approx];
  // Apply factor: r_scaled = factor r^* which applies to LF (HF r remains 1)
  // > N_pilot (r_scaled^T w + 1) = budget, where w_i = cost_i / cost_H
  // > factor r*^T w = budget / N_pilot - 1
  Real cost_H = cost[numApprox],
    factor = (budget / avg_N_H - 1.) / approx_inner_prod * cost_H;
  //avg_eval_ratios.scale(factor); // can result in infeasible r_i < 1

  for (int i=numApprox-1; i>=0; --i) { // repair r_i < 1 first if possible
    Real& r_i = avg_eval_ratios[i];
    r_i *= factor;
    if (r_i <= 1.) { // fix at 1+NUDGE --> scale remaining r_i to reduced budget
      // > only valid for peer DAG with all CV targets = truth, otherwise
      //   tramples linear ineq for other source-target pairs
      // > DAG-aware overload below preserves source-target ratio
      r_i = 1. + RATIO_NUDGE;  cost_r_i = r_i * cost[i];
      // update factor for next r_i: remove this r_i from budget/inner_prod
      budget -= avg_N_H * cost_r_i / cost_H;  approx_inner_prod -= cost_r_i;
      factor  = (budget / avg_N_H - 1.) / approx_inner_prod * cost_H;
    }
  }
  if (outputLevel >= DEBUG_OUTPUT) {
    Real inner_prod = cost_H;
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * avg_eval_ratios[approx];
    Cout << "Rescale to budget: average evaluation ratios\n" << avg_eval_ratios
	 << "Equiv HF = " << avg_N_H * inner_prod / cost_H << std::endl;
  }
}


void NonDNonHierarchSampling::ensemble_numerical_solution(MFSolutionData& soln)
{
  size_t num_cdv, num_lin_con, num_nln_con;
  numerical_solution_counts(num_cdv, num_lin_con, num_nln_con); // virtual

  RealVector x0(num_cdv, false), x_lb(num_cdv, false), x_ub(num_cdv, false),
    lin_ineq_lb(num_lin_con, false), lin_ineq_ub(num_lin_con), lin_eq_tgt,
    nln_ineq_lb(num_nln_con, false), nln_ineq_ub(num_nln_con,false), nln_eq_tgt;
  RealMatrix lin_ineq_coeffs(num_lin_con, num_cdv), lin_eq_coeffs;
  numerical_solution_bounds_constraints(soln, x0, x_lb, x_ub,
					lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
					nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
					lin_ineq_coeffs, lin_eq_coeffs);
  // virtual augmentation of linear ineq (differs among MFMC, ACV, GenACV)
  augment_linear_ineq_constraints(lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub);

  // Perform the numerical solve(s) and recover the solution:
  configure_minimizers(x0, x_lb, x_ub, lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
		       nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, lin_ineq_coeffs,
		       lin_eq_coeffs);
  run_minimizers(soln);
}


void NonDNonHierarchSampling::
process_model_solution(MFSolutionData& soln, size_t& num_samples)
{
  // compute sample increment for HF from current to target:
  size_t hf_form_index, hf_lev_index;
  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
  size_t soln_incr; // unrelaxed increment from numerical solution
  if (truthFixedByPilot)
    soln_incr = num_samples = 0;
  else {
    soln_incr = one_sided_delta(avg_N_H, soln.solution_reference());
    num_samples = (relaxFactor == 1.) ? soln_incr :
      one_sided_delta(avg_N_H, soln.solution_reference(), relaxFactor);
  }

  // **************************************************************************
  // Introduction of relaxFactor makes the optimal estVar solution inconsistent
  // with any relaxation of the sample increment that follows
  // > inconsistency in final reporting (if final relax factor != 1) as well as
  //   intermediate performance metrics, e.g. estVar ratio below
  // All NonHierarch: this fn, virtual recover_results() after numerical solve,
  // print_variance_reduction()/print_estimator_performance(), no_solve defaults
  // > MFMC analytic: estvar_ratios to estvar using N_H_actual{,_proj}
  // > ACV/GenACV: print_model_solution()
  // > ML BLUE:    {process,print}_group_solution()
  //
  // Approach:
  // > Strictly interpret MFSolutionData as optimal soln, preceding any
  //   enactment of relaxation of the optimal sample increment
  // > For final post-processing and reporting, re-evaluate estvar/estvar_ratio
  //   when final relaxFactor is not 1 (for whatever reason) since
  //   soln.solution_variables() will not be consistent.
  // **************************************************************************

  //bool converged = (num_samples == 0 || mlmfIter > maxIterations ||
  //                  pilotMgmtMode != ONLINE_PILOT);
  //if (converged) { // metrics not needed prior to print_variance_reduction()

  // All cases employ projected MC estvar to match the projected nonhier estvar
  // from N* (where N* includes any soln increment not yet performed)
  // > Since soln.average_estimator_variance() precedes (and ignores subsequent)
  //   sample increment relaxation, don't use relaxed increments here.
  RealVector proj_mc_estvar;
  project_mc_estimator_variance(varH, N_H_actual, soln_incr, proj_mc_estvar);

  // Report ratio of averages rather that average of ratios (see notes in
  // print_variance_reduction())
  soln.average_estimator_variance_ratio(soln.average_estimator_variance() /
					average(proj_mc_estvar)); // (1 - R^2)

  //}
}


void NonDNonHierarchSampling::
numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
			  size_t& num_nln_con)
{
  // Notes:
  // > numerical MFMC now uses N_i > N in base augment_linear_ineq_constraints()
  //   (rather than linear constraints enforcing a hierarchical sequence) since
  //   it reorders on the fly using ratioApproxSequence
  // > R_AND_N_NONLINEAR_CONSTRAINT can likely be retired, although
  //   R_ONLY_LINEAR_CONSTRAINT retains some value for truth_fixed_by_pilot

  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    num_cdv = numApprox;  num_nln_con = 0;  num_lin_con = 1;          break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    num_cdv = numGroups;  num_nln_con = 1;  num_lin_con = 0;          break;
  case N_MODEL_LINEAR_CONSTRAINT:
    num_lin_con = num_cdv = numGroups;      num_nln_con = 0;          break;
  case N_MODEL_LINEAR_OBJECTIVE:
    num_cdv = numGroups;  num_nln_con = 1;  num_lin_con = numApprox;  break;
  }
}


void NonDNonHierarchSampling::
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
  size_t hf_form_index, hf_lev_index;
  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;

  // For offline mode, online allocations must be lower bounded for numerics:
  // > for QOI_STATISTICS, unbiased moments / CV beta require min of 2 samples
  // > for ESTIMATOR_PERF, a lower bnd of 1 sample is allowable (MC reference)
  //   >> 1 line of thinking: an offline oracle should by as optimal as possible
  //      and we will use for apples-to-apples estimator performance comparisons
  //   >> another line of thinking: be consistent at 2 samples for possible
  //      switch from estimator_perf (selection) to qoi_statistics (execution);
  //      moreover, don't select an estimator based on inconsistent formulation
  // Nonzero lower bound ensures replacement of allSamples after offline pilot.
  bool offline = (pilotMgmtMode == OFFLINE_PILOT ||
		  pilotMgmtMode == OFFLINE_PILOT_PROJECTION);
  Real offline_N_lwr = 1.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;

  // --------------------------------------
  // Formulate the optimization sub-problem: initial pt, bnds, constraints
  // --------------------------------------

  // > MFMC analytic requires ordered rho2LH to avoid FPE (approxSequence defn)
  //   followed by ordered r_i for {pyramid sampling, R_sq contribution > 0}
  // > MFMC numerical needs ordered r_i to retain pyramid sampling/recursion
  //   >> estvar objective requires an ordering fixed a priori --> makes sense
  //      to optimize w.r.t. this ordering constraint, similar to std::max()
  //      use in mfmc_reordered_analytic_solution()
  // > ACV-MF use of min() in F_ij supports mis-ordering in that C_ij * F_ij
  //   produces same contribution to R_sq independent of i,j order
  //   >> Rather than constraining N_i > N_{i+1} based on a priori ordering,
  //      retain N_i > N and then compute an approx sequence for sampling
  //   >> No need for a priori model sequence, only for post-proc of opt result
  // > ACV-IS is unconstrained in model order --> retain N_i > N

  size_t i, num_cdv = x0.length(), approx;
  Real cost_H = sequenceCost[numApprox], budget = (Real)maxFunctionEvals;

  // minimizer-specific updates (x bounds) performed in finite_solution_bounds()
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
    for (approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0,approx) = sequenceCost[approx] / cost_H;
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    // Could allow optimal profile to emerge from pilot by allowing N* less than
    // the incurred cost (e.g., setting N_lb to 1), but instead we bound with
    // the incurred cost by setting x_lb = latest N_H and retaining r_lb = 1.
    x_lb = 1.; // r_i
    x_lb[numApprox] = (offline) ? offline_N_lwr : avg_N_H;
    // Allow numerical nudge as lower bound:
    //x_lb[numApprox] = avg_N_H;  enforce_nudge(x_lb);

    RealVector avg_eval_ratios = soln.solution_ratios();
    if (avg_eval_ratios.empty()) x0 = 1.;
    else copy_data_partial(avg_eval_ratios, x0, 0);                   // r_i
    x0[numApprox] = (mlmfIter) ? avg_N_H : soln.solution_reference(); // N

    nln_ineq_lb[0] = -DBL_MAX; // no low bnd
    nln_ineq_ub[0] = budget;
    break;
  }
  case N_MODEL_LINEAR_CONSTRAINT:  case N_MODEL_LINEAR_OBJECTIVE: {
    x_lb = (offline) ? offline_N_lwr : avg_N_H;
    // Allow numerical nudge as lower bound:
    //x_lb = avg_N_H;  enforce_nudge(x_lb);

    const RealVector& soln_vars = soln.solution_variables();
    x0 = (soln_vars.empty()) ? x_lb : soln_vars;
    if (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) {
      // linear inequality constraint on budget:
      //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
      //   N w + \Sum_i w_i N_i <= equivHF * w
      //   N + \Sum_i w_i/w N_i <= equivHF
      lin_ineq_ub[0] = budget; // remaining ub initialized to 0
      for (approx=0; approx<numApprox; ++approx)
	lin_ineq_coeffs(0, approx) = sequenceCost[approx] / cost_H;
      lin_ineq_coeffs(0, numApprox) = 1.;
    }
    else if (optSubProblemForm == N_MODEL_LINEAR_OBJECTIVE) {
      // nonlinear inequality constraint on estvar
      nln_ineq_lb = -DBL_MAX;  // no lower bnd
      nln_ineq_ub = std::log(convergenceTol * average(estVarIter0));
    }
    break;
  }
  }
  // x0 can undershoot x_lb if an OFFLINE mode, but enforce generally
  enforce_bounds(x0, x_lb, x_ub);
}


void NonDNonHierarchSampling::
augment_linear_ineq_constraints(RealMatrix& lin_ineq_coeffs,
				RealVector& lin_ineq_lb,
				RealVector& lin_ineq_ub)
{
  // linear inequality constraints on sample counts:
  //  N_i >  N (aka r_i > 1) prevents numerical exceptions
  // (N_i >= N becomes N_i > N based on RATIO_NUDGE)

  // numerical MFMC now adopts this base implementation (see above)

  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_MODEL_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t offset = (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) ? 1 : 0;
    for (size_t approx=0; approx<numApprox; ++approx) {
      lin_ineq_coeffs(approx+offset,    approx) = -1.;
      lin_ineq_coeffs(approx+offset, numApprox) =  1. + RATIO_NUDGE; // N_i > N
    }
    break;
  }
  case R_ONLY_LINEAR_CONSTRAINT: case R_AND_N_NONLINEAR_CONSTRAINT:
    break; // none to add (r lower bounds = 1)
  }
}


Real NonDNonHierarchSampling::
augmented_linear_ineq_violations(const RealVector& cd_vars,
				 const RealMatrix& lin_ineq_coeffs,
				 const RealVector& lin_ineq_lb,
				 const RealVector& lin_ineq_ub)
{
  Real quad_viol = 0.;
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_CONSTRAINT:  // lin_ineq #0 is augmented
  case N_MODEL_LINEAR_OBJECTIVE: { // no other lin ineq
    size_t offset = (optSubProblemForm == N_MODEL_LINEAR_CONSTRAINT) ? 1 : 0;
    Real viol, inner_prod, l_bnd, u_bnd, N_H = cd_vars[numApprox];
    for (size_t approx=0; approx<numApprox; ++approx) {
      inner_prod = lin_ineq_coeffs(approx+offset, approx)    * cd_vars[approx]
	         + lin_ineq_coeffs(approx+offset, numApprox) * N_H;
      l_bnd = lin_ineq_lb[approx+offset];
      u_bnd = lin_ineq_ub[approx+offset];
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
  case R_ONLY_LINEAR_CONSTRAINT: case R_AND_N_NONLINEAR_CONSTRAINT:
    break; // none to add (r lower bounds = 1)
  }
  return quad_viol;
}


void NonDNonHierarchSampling::
finite_solution_bounds(const RealVector& x0, RealVector& x_lb, RealVector& x_ub)
{
  // Some optimizers (DIRECT, SBLO, EGO) require finite bounds
  if ( varMinIndices.first == 0 &&
       ( optSubProblemSolver == SUBMETHOD_DIRECT ||
	 optSubProblemSolver == SUBMETHOD_DIRECT_NPSOL ||
	 optSubProblemSolver == SUBMETHOD_DIRECT_OPTPP ||
	 optSubProblemSolver == SUBMETHOD_DIRECT_NPSOL_OPTPP ||
	 optSubProblemSolver == SUBMETHOD_SBGO ||
	 optSubProblemSolver == SUBMETHOD_SBLO ||
	 optSubProblemSolver == SUBMETHOD_EGO ) ) {
    // Prior to approx increments (when numerical solns are performed),
    // equivHFEvals represents the total incurred cost in shared sample sets
    Real budget;
    switch (optSubProblemForm) {
    case N_MODEL_LINEAR_OBJECTIVE: case N_GROUP_LINEAR_OBJECTIVE: {
      // accuracy constrained: infer upper bounds from budget reqd to obtain
      // target accuracy via MC: varH / N = tol * avg_estvar0; for minimizer
      // sequencing, a downstream refinement can omit this approximated bound
      RealVector hf_targets;  apply_mc_reference(hf_targets);
      budget = average(hf_targets);     break;
    }
    default:
      budget = (Real)maxFunctionEvals;  break;
    }

    // "budget_exhausted" logic protects numerical solutions for budget-
    // constrained cases in:
    // > NonD{ACV,GenACV,MultilevBLUE}Sampling::compute_allocations()
    // > NonDMultifidelitySampling::mfmc_numerical_solution()
    // but accuracy-constrained cases estimate hf_targets above, where it is
    // possible for the pilot to overshoot this target, such that we need to
    // protect against x_ub < x_lb.
    if (equivHFEvals < budget) // budget not exhausted
      derived_finite_solution_bounds(x0, x_lb, x_ub, budget);
    else
      x_ub = x0; // Note: x_ub = x_lb could then update x0 w/ enforce_bounds()
  }
  else
    x_ub = DBL_MAX; // no upper bounds needed for x

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Finite bounds (lb, ub):\n" << x_lb << x_ub
	 << "from modelGroupCost:\n" << modelGroupCost << std::endl;
}


void NonDNonHierarchSampling::
derived_finite_solution_bounds(const RealVector& x0, RealVector& x_lb,
			       RealVector& x_ub, Real budget)
{
  size_t i, x_len = x0.length();
  Real cost_H = sequenceCost[numApprox], budget_cost = budget * cost_H;
  // model groups now defined for MFMC,ACV,GenACV, in addition to ML BLUE
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: { // not valid for group allocations
    // for purposes of expended budget, use attempted samples (NLevAlloc)
    // rather than accumulated samples (NLevActual) as fixed soln reference
    size_t hf_form_index, hf_lev_index;
    hf_indices(hf_form_index, hf_lev_index);
    Real N_sh = (Real)NLevAlloc[hf_form_index][hf_lev_index],
      cost_sh = modelGroupCost[x_len], factor = budget_cost / N_sh - cost_sh;
    for (i=0; i<x_len; ++i)
      x_ub[i] = 1. + factor / modelGroupCost[i]; // for ub on r_i
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: { // not valid for group allocations
    // Extreme N_sh is all budget allocated to shared samples:
    size_t hf_index = x_len - 1;  Real cost_sh = modelGroupCost[hf_index];
    x_ub[hf_index] = budget_cost / cost_sh; // ub on N_sh
    // Extreme r_i is all refinement budget allocated to one approx:
    //   (r_i - 1) N_sh cost_i = budget_cost - N_sh cost_sh
    //   r_i = 1 + (budget_cost / N_sh - cost_sh) / cost_i
    // And for variable N_sh, r_i upper bnd corresponds to N_sh lower bnd
    Real N_sh_lb = x_lb[hf_index], factor = budget_cost / N_sh_lb - cost_sh;
    for (i=0; i<hf_index; ++i)
      x_ub[i] = 1. + factor / modelGroupCost[i]; // ub on r_i
    break;
  }
  default: {
    // Extreme N_sh is all budget allocated to shared samples:
    size_t hf_index = x_len - 1;  Real cost_sh = modelGroupCost[hf_index];
    x_ub[hf_index] = budget_cost / cost_sh; // ub on N_sh
    // Extreme N_i is all refinement budget allocated to one approx:
    //   delta_N_i cost_i = budget_cost - N_sh cost_sh
    //   N_i =  (budget_cost - N_sh (cost_sh - cost_i)) / cost_i
    // And for variable N_sh, N_i upper bnd corresponds to N_sh lower bnd
    Real N_sh_lb = x_lb[hf_index], factor = budget_cost - N_sh_lb * cost_sh;
    for (i=0; i<hf_index; ++i)
      x_ub[i] = N_sh_lb + factor / modelGroupCost[i]; // ub on N_i
    break;
  }
  }
}


void NonDNonHierarchSampling::apply_mc_reference(RealVector& mc_targets)
{
  // base implementation for use when varH is available

  if (mc_targets.length() != numFunctions)
    mc_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    mc_targets[qoi] = varH[qoi] / (convergenceTol * estVarIter0[qoi]);
}


void NonDNonHierarchSampling::
configure_minimizers(RealVector& x0, RealVector& x_lb, RealVector& x_ub,
		     const RealVector& lin_ineq_lb,
		     const RealVector& lin_ineq_ub,
		     const RealVector& lin_eq_tgt,
		     const RealVector& nln_ineq_lb,
		     const RealVector& nln_ineq_ub,
		     const RealVector& nln_eq_tgt,
		     const RealMatrix& lin_ineq_coeffs,
		     const RealMatrix& lin_eq_coeffs)
{
  // support sequenced (global-local) and competed (NPSOL_OPTPP) configurations:
  // > EGO vs. DIRECT: the former adds efficiency over the latter when the model
  //   evals are expensive (using the latter for solving the EIF subproblem),
  //   but for this case where we are performing a sequence of linear solves
  //   for fixed covariances, it seems that DIRECT should be preferred.
  bool use_adapter = false, use_dfs = false;
  size_t sequence_len = 1, num_solvers;
  switch (optSubProblemSolver) {
  case SUBMETHOD_SBLO: case SUBMETHOD_SBGO:
    use_adapter = use_dfs = true;  break;
  case SUBMETHOD_EGO:  case SUBMETHOD_EA: //case SUBMETHOD_CPS:
    use_adapter = true;            break;
  case SUBMETHOD_DIRECT_NPSOL: case SUBMETHOD_DIRECT_OPTPP:
  case SUBMETHOD_DIRECT_NPSOL_OPTPP:
    sequence_len = 2;              break;
  }

  // -----------------------------------
  // Configure the variance minimizer(s)
  // -----------------------------------
  if (varianceMinimizers.empty()) {

    // separate optSubProblemSolver into array of single solvers where needed
    Short2DArray solvers(sequence_len);
    ShortArray& solvers_0 = solvers[0];
    switch (optSubProblemSolver) {
    case SUBMETHOD_NPSOL_OPTPP: // competed
      solvers_0.resize(2);
      solvers_0[0] = SUBMETHOD_NPSOL;  solvers_0[1] = SUBMETHOD_OPTPP;  break;
    case SUBMETHOD_DIRECT_NPSOL: { // sequenced global-local
      solvers_0.resize(1);   solvers_0[0] = SUBMETHOD_DIRECT;
      solvers[1].resize(1); solvers[1][0] = SUBMETHOD_NPSOL;  break;
    }
    case SUBMETHOD_DIRECT_OPTPP: { // sequenced global-local
      solvers_0.resize(1);   solvers_0[0] = SUBMETHOD_DIRECT;
      solvers[1].resize(1); solvers[1][0] = SUBMETHOD_OPTPP;  break;
    }
    case SUBMETHOD_DIRECT_NPSOL_OPTPP: {// sequenced global-local+competed local
      solvers_0.resize(1);  solvers_0[0] = SUBMETHOD_DIRECT;
      ShortArray& solvers_1 = solvers[1];  solvers_1.resize(2);
      solvers_1[0] = SUBMETHOD_NPSOL;  solvers_1[1] = SUBMETHOD_OPTPP;  break;
    }
    default:
      solvers_0.resize(1);  solvers_0[0] = optSubProblemSolver;         break;
    }

    std::shared_ptr<Model> adapt_model, sub_prob_model;
    if (use_adapter) {
      // configure the minimization sub-problem
      adapt_model = std::make_shared<MinimizerAdapterModel>(x0, x_lb,
        x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
        lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, response_evaluator);

      //////////////////////////////////////////////////////////////////////////
      // For nested optimization, we emulate EstVar*(hp) at the top level,
      // eliminating all lower level quantities through the lower level solve.
      // > Conceptually simple but repeated optim's required --> top level is
      //   noisy if lower-level is numerical; less efficient than 1 solve
      // > Best option for analytic cases like MLMC, ordered MFMC, et al.
      //   >> numerical opt still relevant for misordered models or
      //      over-estimated pilots
      // > Want to support this in any case, so start with this as lower risk
      //   option --> main needs are recovering cost(hp) from metadata, ...
      //////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////////////
      // For integrated optimization, pilot computed for each model at each hp.
      // Main loop:
      // > shared incr --> counts, sums --> varL, covLL, covLH
      // > Solve r*,N* from min EstVar = varH/N (1-Rsq) where Rsq defined:
      //   >> MFMC: R_sq += (r_i - r_ip1) / (r_i * r_ip1) * rho2_LH(qoi, i)
      //   >> ACV: form F(r), A(F,c), invert CF, triple product --> R_sq
      // > Apply delta-N* and continue
      // Post-process: apply r*, compute LF incr, roll up final stats

      // response_evaluator():
      //   --> objective = min_{N,r_i} Estvar(corr(hp),cost(hp))
      //   --> constraints = lin/nln budget = fn(N,r_i,cost(hp))
      // > change in hyper-parameters must reset sample counts/accumulators
      //   >> track high-level metrics over hp, but don't retain low level
      // > Costs for each model = ensemble average over pilot for each hp

      // Solution of approx sub-problem uses DataFitSurrModel::approxInterface
      // to query either the low-level (cost, corr) or high-level (estvar)
      // surrogates over the hyper-parameters
      // > low-level: minimal emulated set = cost + {corr/covar terms(N,r)} ?
      //   >> simplifications can be made for 3 model case, but not in general
      // > high-level:
      //   >> EstVar = emulated over (hp,N,r_i) --> surrogate emulation extent
      //      can be pointwise(N,r_i) for each hp or combined(hp,N,r_i)
      //   >> EstVar* = emulated only over hp, but requires nested opt
      // DataFitSurrModel::actualModel evaluations compute corr,cost from
      // scratch for each hp (initial surrogate build, validation, refinement)

      // TRMM/EGO must solve multiple approx sub-problems and perform multiple
      // surrogate refinements to converge to EstVar*(hp)

      // Iterated ACV,MFMC would then be outer-loop around this pilot-based
      // solution --> perform N* increment to update correlations/covariances
      // > can we integrate this upstream to eliminate this loop? --> would
      //   require performing N* increment as part of optimization, which would
      //   not be inconsistent with updating N* after a converged sub-problem
      //   (both can overshoot).  Key would be doing this at a major iteration
      //   step (SBLO validation), not during exploration (EGO).
      // > or restrict to offline pilot mode (no iteration)

      // Then r* over-sampling occurs after all iteration is done -> final stats
      //////////////////////////////////////////////////////////////////////////

      if (use_dfs) {
          int samples = 100, seed = 12347;      // TO DO: spec
          unsigned short sample_type = SUBMETHOD_DEFAULT;
          String rng; // empty string: use default
          bool vary_pattern = false, use_derivs = false;
          short corr_type = ADDITIVE_CORRECTION, corr_order = 1, data_order = 1;
        if (use_derivs) { // Would also need to verify surrogate support
          if (adapt_model->gradient_type() != "none") data_order |= 2;
          if (adapt_model->hessian_type()  != "none") data_order |= 4;
        }
        Iterator dace_iterator;
        dace_iterator.assign_rep(std::make_shared<NonDLHSSampling>(adapt_model,
          sample_type, samples, seed, rng, vary_pattern, ACTIVE_UNIFORM));
        dace_iterator.active_set_request_values(data_order);

        String approx_type("global_kriging"), point_reuse("none");// TO DO: spec
        UShortArray approx_order; // empty
        ActiveSet dfs_set = adapt_model->current_response().active_set();// copy
        dfs_set.request_values(1);
        const ShortShortPair& dfs_view = adapt_model->current_variables().view();
        sub_prob_model = std::make_shared<DataFitSurrModel>(
          dace_iterator, adapt_model, dfs_set, dfs_view, approx_type,
          approx_order, corr_type, corr_order, data_order, SILENT_OUTPUT,
          point_reuse);
      }
      else
	      sub_prob_model = adapt_model;
    }

    size_t i, j, max_iter = 50000, max_eval = 250000;
    Real conv_tol = 1.e-8; // tight convergence

    varianceMinimizers.resize(sequence_len);
    for (i=0; i<sequence_len; ++i) {
      ShortArray& solvers_i = solvers[i];
      IteratorArray&  min_i = varianceMinimizers[i];
      varMinIndices.first = i;
      num_solvers = solvers_i.size();
      min_i.resize(num_solvers);
      for (j=0; j<num_solvers; ++j) {
	varMinIndices.second = j;
	// *** TO DO: per-minimizer x_lb,x_ub updates for use_adapter case
	finite_solution_bounds(x0, x_lb, x_ub);
	switch (solvers_i[j]) {
	case SUBMETHOD_NPSOL: {
	  // Keep FDSS smaller than RATIO_NUDGE to avoid FPE
	  // > seems that this can be tight using refined Teuchos solns
	  //   (default is fine for ss_diffusion; leave some gap just in case)
	  Real fdss = 1.e-7; //-1.; (no override of default)
	  int deriv_level;// 0 none, 1 obj, 2 constr, 3 both
	  switch (optSubProblemForm) {
	  case R_AND_N_NONLINEAR_CONSTRAINT: deriv_level = 2;  break;
	  case N_MODEL_LINEAR_OBJECTIVE:
	  case N_GROUP_LINEAR_OBJECTIVE:     deriv_level = 1;  break;
	  default:                           deriv_level = 0;  break;
	  }
#ifdef HAVE_NPSOL
	  min_i[j].assign_rep(std::make_shared<NPSOLOptimizer>(x0, x_lb, x_ub,
	    lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	    lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, npsol_objective,
	    npsol_constraint, deriv_level, conv_tol, max_iter, fdss));
#endif
	  break;
	}
	case SUBMETHOD_OPTPP: {
	  // Keep FDSS smaller than RATIO_NUDGE to avoid FPE
	  // > SNLLBase::snll_post_instantiate() enforces lower bound of
	  //   DBL_EPSILON on pow(fdss,{2,3}) for {forward,central} --> min
	  //   FDSS of ~6e-6 for central, ~1.49e-8 for forward (default FDSS)
	  // > central is not managing bound constr properly (negative offset
	  //   repeats positive); forward seems Ok for current constraints
	  Real max_step = 100000.;    String fd_type = "forward";
	  RealVector fdss(1, false);  fdss[0] = 1.e-7; // ~7x > default
#ifdef HAVE_OPTPP
	  switch (optSubProblemForm) {
	  case N_MODEL_LINEAR_OBJECTIVE: case N_GROUP_LINEAR_OBJECTIVE:
	    min_i[j].assign_rep(std::make_shared<SNLLOptimizer>(x0, x_lb, x_ub,
	      lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	      lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	      optpp_nlf1_objective, optpp_fdnlf1_constraint, fdss, fd_type,
	      max_iter, max_eval, conv_tol, conv_tol, max_step));
	    break;
	  default:
	    min_i[j].assign_rep(std::make_shared<SNLLOptimizer>(x0, x_lb, x_ub,
	      lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	      lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	      optpp_fdnlf1_objective, optpp_nlf1_constraint, fdss, fd_type,
	      max_iter, max_eval, conv_tol, conv_tol, max_step));
	    break;
	  }
#endif
	  break;
	}
	case SUBMETHOD_DIRECT: { // global search + local refinement
	  // > For DIRECT, we have the option of either passing an adapter model
	  //   or passing a fn callback.  Since we don't need parallel or other
	  //   Model features, passing a function callback seems lighter weight.
	  // > We also have the option of forming a merit fn for constraints
	  //   (seems preferred for ordering the search) or passing "infeasible"
	  //   codes intended for implicit constraints.  A merit fn requires
	  //   another wrapper layer, either via model recursion or fn callback.
	  //min_i[j].assign_rep(std::make_shared<NCSUOptimizer>(sub_prob_model,
	  //  max_iter, max_eval, min_box_size, vol_box_size, solution_target));
	  // See NCSUOptimizer::core_run() for default tolerance values
	  Real min_box_size = 1.e-6, //-1. activates NCSU default = 1.e-4
	       vol_box_size = 1.e-9, //-1. activates NCSU default = 1.e-6 
	       soln_target  = -DBL_MAX; // no target, deactivates convergenceTol
#ifdef HAVE_NCSU
	  min_i[j].assign_rep(std::make_shared<NCSUOptimizer>(x_lb, x_ub,
	    lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub,lin_eq_coeffs, lin_eq_tgt,
	    nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, max_iter, max_eval,
	    direct_penalty_merit, min_box_size, vol_box_size, soln_target));
#endif
	  break;
	}
	case SUBMETHOD_SBLO: {
	  short merit_fn = AUGMENTED_LAGRANGIAN_MERIT, accept_logic = FILTER,
	    constr_relax = NO_RELAX;
	  unsigned short soft_conv_limit = 5;
	  Real tr_factor = .5;
	  // TO DO: push updated solution bounds into model(s)
	  min_i[j].assign_rep(std::make_shared<DataFitSurrBasedLocalMinimizer>(
	    sub_prob_model, merit_fn, accept_logic, constr_relax, tr_factor,
	    max_iter, max_eval, conv_tol, soft_conv_limit, false));
	  break;
	}
	case SUBMETHOD_EGO: {
	  // EGO builds its own GP in initialize_sub_problem(), so a DFSModel
	  // does not need to be constructed here as for SBLO/SBGO
	  // > TO DO: pure global opt may need subsequent local refinement
	  int samples = 100, seed = 12347;      // TO DO: spec
	  String approx_type("global_kriging"); // TO DO: spec
	  bool use_derivs = false;
	  // TO DO: push updated solution bounds into model(s)
	  min_i[j].assign_rep(std::make_shared<EffGlobalMinimizer>(
	    sub_prob_model, approx_type, samples, seed, use_derivs, max_iter,
	    max_eval, conv_tol));
	  break;
	}
	/*
        case SUBMETHOD_CPS: { // to be combined with local refinement
          break;
        }
        case SUBMETHOD_SBGO: { // for NonDGlobalInterval, was EAminlp + GP ...
	  // TO DO: push updated solution bounds into model(s)
          min_i[j].assign_rep(std::make_shared<SurrBasedGlobalMinimizer>(
	    sub_prob_model, max_iter, max_eval, conv_tol));
          break;
        }
        case SUBMETHOD_EA: { // to be combined with local refinement
	  // TO DO: push updated solution bounds into model(s)
          min_i[j].assign_rep(std::make_shared<COLINOptimizer>(sub_prob_model,
	    max_iter, max_eval, conv_tol));
          break;
        }
        */
	}
      }
    }
  }
  else {
    // update data for the varianceMinimizers when either:
    // > warm starting for iteration >= 1            (use last in sequence)
    // > trying another initial guess on iteration 0 (use  all in sequence)
    size_t i, j, last_seq_index = sequence_len - 1,
      start = (mlmfIter) ? last_seq_index : 0;
    if (use_adapter) {
      //Model& adapt_model = (use_dfs) ?
      //  subProbModel.subordinate_model() : subProbModel;
      // *** TO DO: (existing active view accessors don't support size change)
      //adapt_model.update_active_constraints(lin_ineq_coeffs, lin_ineq_lb,
      //  lin_ineq_ub, lin_eq_coeffs, lin_eq_tgt, nln_ineq_lb, nln_ineq_ub,
      //  nln_eq_tgt);
      for (i=start; i<sequence_len; ++i) {
	IteratorArray& min_i = varianceMinimizers[i];
	varMinIndices.first = i;
	num_solvers = min_i.size();
	for (j=0; j<num_solvers; ++j) {
	  Iterator& min_ij = min_i[j];
	  //if (!min_ij.is_null()) {
	  //  finite_solution_bounds(x0, x_lb, x_ub);
	  //  adapt_model.update_active_variables(x0, x_lb, x_ub);
	  //  if (use_dfs) subProbModel.update_from_subordinate_model();
	  //  varMinIndices.second = j;
	  //  min_i[j].update_from_model(sub_prob_model);
	  //}
	}
      }
    }
    else
      for (i=start; i<sequence_len; ++i) {
	IteratorArray& min_i = varianceMinimizers[i];
	varMinIndices.first = i;
	num_solvers = min_i.size();
	for (j=0; j<num_solvers; ++j) {
	  Iterator& min_ij = min_i[j];
	  if (!min_ij.is_null()) {
	    varMinIndices.second = j;
	    finite_solution_bounds(x0, x_lb, x_ub);
	    min_ij.update_callback_data(x0, x_lb, x_ub, lin_ineq_coeffs,
	      lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs, lin_eq_tgt, nln_ineq_lb,
	      nln_ineq_ub, nln_eq_tgt);
	  }
	}
      }
  }

  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "Numerical solve (initial, lb, ub):\n" << x0 << x_lb << x_ub;
    if (!lin_ineq_lb.empty() || !lin_ineq_ub.empty())
      Cout << "Numerical solve (lin ineq lb, ub):\n" << lin_ineq_lb
	   << lin_ineq_ub;// << lin_ineq_coeffs << lin_eq_tgt << lin_eq_coeffs;
    if (!nln_ineq_lb.empty() || !nln_ineq_ub.empty())
      Cout << "Numerical solve (nln ineq lb, ub):\n" << nln_ineq_lb
	   << nln_ineq_ub;// << nln_eq_tgt;
    Cout << std::endl;
  }
}


void NonDNonHierarchSampling::run_minimizers(MFSolutionData& soln)
{
  // ----------------------------------
  // Solve the optimization sub-problem: compute optimal r*,N*
  // ----------------------------------
  Real merit_fn, merit_fn_star = DBL_MAX;
  size_t i, j, sequence_len = varianceMinimizers.size(), best_min, num_solvers,
    last_seq_index = sequence_len - 1, start = (mlmfIter) ? last_seq_index : 0;
  for (i=start; i<sequence_len; ++i) {
    IteratorArray& min_i = varianceMinimizers[i];
    varMinIndices.first = i;
    num_solvers = min_i.size();
    for (j=0; j<num_solvers; ++j) {
      Iterator& min_ij = min_i[j];
      if (min_ij.is_null()) continue;
      varMinIndices.second = j;
      min_ij.run();
      const Variables& vars_star = min_ij.variables_results();
      const RealVector&  cv_star = vars_star.continuous_variables();
      const RealVector&  fn_star = min_ij.response_results().function_values();
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "run_minimizers() results for (i,j) = (" << i << "," << j
	     << "):\ncv_star =\n" << cv_star << "fn_vals_star =\n" << fn_star;
      // track best using penalty merit fn comprised of accuracy and cost
      merit_fn = nh_penalty_merit(cv_star, fn_star);
      if (j == 0 || merit_fn < merit_fn_star)
	{ merit_fn_star = merit_fn;  best_min = j; }
    }
    if (outputLevel >= NORMAL_OUTPUT && num_solvers > 1)
      Cout << "run_minimizers() best solver at step " << i << " = "
	   << min_i[best_min].method_string() << std::endl;

    if (i < last_seq_index) { // propagate best final pt to next initial pt(s)
      const Variables& vars_star = min_i[best_min].variables_results();
      IteratorArray& min_ip1 = varianceMinimizers[i+1];
      num_solvers = min_ip1.size();
      for (j=0; j<num_solvers; ++j) {
	Iterator& min_ij = min_ip1[j];
	auto  model_ij = min_ij.iterated_model();
	if (!model_ij)  min_ij.initial_point(vars_star);
	else                model_ij->current_variables().active_variables(vars_star);
      }
    }
  }

  // -------------------------------------
  // Post-process the optimization results
  // -------------------------------------
  // Note: issues with more involved recovery for upstream (global) optimizers
  // that don't support linear/nonlinear constraints can be avoided by always
  // ending with capable (local gradient-based) minimizers at sequence end.
  Iterator& min_last_best = varianceMinimizers[last_seq_index][best_min];
  recover_results(min_last_best.variables_results().continuous_variables(),
		  min_last_best.response_results().function_values(), soln);
}


void NonDNonHierarchSampling::
recover_results(const RealVector& cv_star, const RealVector& fn_star,
		MFSolutionData& soln)
{
  // Estvar recovery from optimizer provides std::log(average(nh_estvar)) =
  // var_H / N_H (1 - R^2).  Notes:
  // > a QoI-vector prior to averaging would require recomputation from r*,N*)
  // > this value corresponds to N* (_after_ num_samples applied)
  Real avg_estvar = (optSubProblemForm == N_MODEL_LINEAR_OBJECTIVE ||
		     optSubProblemForm == N_GROUP_LINEAR_OBJECTIVE) ?
    std::exp(fn_star[1]) : std::exp(fn_star(0));
  soln.average_estimator_variance(avg_estvar);

  // Recover optimizer results for average {eval_ratios,estvar}.  Also compute
  // shared increment from N* or from targeting specified budget || accuracy.
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
      avg_hf_target = allocate_budget(avg_eval_ratios, sequenceCost);
      Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals
	   << ": average HF target = " << avg_hf_target << std::endl;
    }
    else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: detect user spec
      // EstVar target = convTol * estvar_iter0 = estvar_ratio * varH / N_target
      //               = curr_estvar * N_curr / N_target
      //  --> N_target = curr_estvar * N_curr / (convTol * estvar_iter0)
      // Note: estvar_iter0 is fixed based on pilot
      size_t hf_form, hf_lev;  hf_indices(hf_form, hf_lev);
      avg_hf_target = (backfillFailures) ?
	update_hf_target(avg_estvar, NLevActual[hf_form][hf_lev], estVarIter0) :
	update_hf_target(avg_estvar,  NLevAlloc[hf_form][hf_lev], estVarIter0);
      Cout << "Scaling profile for convergenceTol = " << convergenceTol
	   << ": average HF target = " << avg_hf_target << std::endl;
    }
    //avg_hf_target = std::min(budget_target, ctol_target); // enforce both
    soln.anchored_solution_ratios(avg_eval_ratios, avg_hf_target);
    soln.equivalent_hf_allocation(
      compute_equivalent_cost(avg_hf_target, avg_eval_ratios, sequenceCost));
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    // R_AND_N:  r*   is leading part of cv_star and N* is trailing entry
    RealVector avg_eval_ratios(Teuchos::View, cv_star.values(), numApprox);
    soln.anchored_solution_ratios(avg_eval_ratios, cv_star[numApprox]);
    soln.equivalent_hf_allocation(fn_star[1]);
    break;
  }
  case N_MODEL_LINEAR_CONSTRAINT:
    // N_VECTOR: N*_i is leading part of cv_star and N* is trailing entry.
    // Note that r_i is always relative to HF N, not its DAG pairing.
    soln.solution_variables(cv_star);
    soln.equivalent_hf_allocation(linear_model_cost(cv_star));
    break;
  case N_GROUP_LINEAR_CONSTRAINT:
    // N_VECTOR: N*_i is leading part of cv_star and N* is trailing entry.
    // Note that r_i is always relative to HF N, not its DAG pairing.
    soln.solution_variables(cv_star);
    soln.equivalent_hf_allocation(linear_group_cost(cv_star));
    break;
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    // N_VECTOR: N*_i is leading part of cv_star and N* is trailing entry.
    // Note that r_i is always relative to HF N, not its DAG pairing.
    soln.solution_variables(cv_star);
    soln.equivalent_hf_allocation(fn_star[0]);
    break;
  }
}


void NonDNonHierarchSampling::method_recourse(unsigned short outer_method)
{
  // NonDNonHierarchSampling numerical solves must protect use of Fortran
  // solvers at this level from conflicting with use at a higher level.
  // However, it is not necessary to check the other direction by defining
  // check_sub_iterator_conflict(), since solver execution does not span
  // any Model evaluations.

  bool sol_conflict = (outer_method == NPSOL_SQP || outer_method == NLSSOL_SQP),
    ncsu_conflict = (outer_method == NCSU_DIRECT), have_npsol = false,
    have_optpp = false, have_ncsu = false, err_flag = false;
#ifdef HAVE_NPSOL
  have_npsol = true;
#endif
#ifdef HAVE_OPTPP
  have_optpp = true;
#endif
#ifdef HAVE_NCSU
  have_ncsu = true;
#endif

  // Note: multiple simultaneous conflicts not currently supported; rather
  // multiple invocations of recourse could further trim the method list
  if (sol_conflict)
    switch (optSubProblemSolver) {
    case SUBMETHOD_NPSOL: case SUBMETHOD_NPSOL_OPTPP:
      if (have_optpp) optSubProblemSolver = SUBMETHOD_OPTPP;
      else            err_flag = true;
      break;
    case SUBMETHOD_DIRECT_NPSOL: // TO DO: check for DIRECT
      if (have_ncsu) {
	if (have_optpp)	optSubProblemSolver = SUBMETHOD_DIRECT_OPTPP;
	else            optSubProblemSolver = SUBMETHOD_DIRECT;
      }
      else {
	if (have_optpp)	optSubProblemSolver = SUBMETHOD_OPTPP;
	else            err_flag = true;
      }
      break;
    case SUBMETHOD_DIRECT_NPSOL_OPTPP: // TO DO: check for OPT++
      if (have_ncsu) {
	if (have_optpp)	optSubProblemSolver = SUBMETHOD_DIRECT_OPTPP;
	else            optSubProblemSolver = SUBMETHOD_DIRECT;
      }
      else {
	if (have_optpp)	optSubProblemSolver = SUBMETHOD_OPTPP;
	else            err_flag = true;
      }
      break;
    }
  else if (ncsu_conflict)
    switch (optSubProblemSolver) {
    case SUBMETHOD_DIRECT:
      err_flag = true; break;
    case SUBMETHOD_DIRECT_NPSOL: // TO DO: check for NPSOL
      if      (have_npsol) optSubProblemSolver = SUBMETHOD_NPSOL;
      else if (have_optpp) optSubProblemSolver = SUBMETHOD_OPTPP;
      else                 err_flag = true;
      break;
    case SUBMETHOD_DIRECT_OPTPP: // TO DO: check for OPT++
      if      (have_optpp) optSubProblemSolver = SUBMETHOD_OPTPP;
      else if (have_npsol) optSubProblemSolver = SUBMETHOD_NPSOL;
      else                 err_flag = true;
      break;
    case SUBMETHOD_DIRECT_NPSOL_OPTPP:// TO DO: check for NPSOL & OPT++
      if (have_optpp && have_npsol) optSubProblemSolver = SUBMETHOD_NPSOL_OPTPP;
      else if (have_npsol) optSubProblemSolver = SUBMETHOD_NPSOL;
      else if (have_optpp) optSubProblemSolver = SUBMETHOD_OPTPP;
      else                 err_flag = true;
      break;
    }

  if (err_flag) {
    Cerr << "\nError: method conflict detected in NonDNonHierarchSampling but "
	 << "no alternate solver available." << std::endl;
    abort_handler(METHOD_ERROR);
  }
  else
    Cerr << "\nWarning: method recourse invoked in NonDNonHierarchSampling due "
	 << "to detected method conflict.\n         New solver = "
	 << optSubProblemSolver << "\n\n";
}


// Minimizer::penalty_merit() uses too many Minimizer attributes, so we
// use local definitions here
Real NonDNonHierarchSampling::
nh_penalty_merit(const RealVector& cd_vars, const RealVector& fn_vals)
{
  // Assume linear constraints are satisfied (for now)
  // Keep accuracy in log space and normalize both cost and log-accuracy

  Real budget = (Real)maxFunctionEvals;
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    return nh_penalty_merit(fn_vals[0], fn_vals[1],
			    std::log(convergenceTol*average(estVarIter0)));
    break;
  case N_MODEL_LINEAR_CONSTRAINT:
    return nh_penalty_merit(fn_vals[0], linear_model_cost(cd_vars), budget);
    break;
  case N_GROUP_LINEAR_CONSTRAINT:
    return nh_penalty_merit(fn_vals[0], linear_group_cost(cd_vars), budget);
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    return nh_penalty_merit(fn_vals[0], fn_vals[1], budget);
    break;
  default: {
    // Note: all cases could use this except for additional exp/log overhead
    MFSolutionData soln;  recover_results(cd_vars, fn_vals, soln);
    return nh_penalty_merit(soln);
    break;
  }
  }
}


// Minimizer::penalty_merit() uses too many Minimizer attributes, so we
// use local definitions here
Real NonDNonHierarchSampling::nh_penalty_merit(const MFSolutionData& soln)
{
  // Assume linear constraints are satisfied (for now)
  // Keep accuracy in log space and normalize both cost and log-accuracy

  Real  avg_estvar = soln.average_estimator_variance(),
    equiv_hf_alloc = soln.equivalent_hf_allocation();
  switch (optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    return nh_penalty_merit(equiv_hf_alloc, std::log(avg_estvar),
			    std::log(convergenceTol*average(estVarIter0)));
    break;
  default:
    return nh_penalty_merit(std::log(avg_estvar), equiv_hf_alloc,
			    (Real)maxFunctionEvals);
    break;
  }
}


Real NonDNonHierarchSampling::
nh_penalty_merit(Real obj, Real nln_con, Real nln_u_bnd)
{
  Real merit_fn = obj, constr_viol = 0., r_p = 1.e+6, c_tol = .01,
    g_tol = nln_con - nln_u_bnd - c_tol;
  if (g_tol > 0.) {
    Real scale = std::abs(nln_u_bnd); 
    constr_viol = (scale > Pecos::SMALL_NUMBER) ? g_tol/scale : g_tol;
    merit_fn += r_p * constr_viol * constr_viol;
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Penalty merit fn = " << merit_fn << " from obj = " << obj
	 << " constraint viol = " << constr_viol << std::endl;
  return merit_fn;
}


void NonDNonHierarchSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  Cerr << "Error: estimator_variance_ratios() not redefined by derived class.\n"
       << std::endl;
  abort_handler(METHOD_ERROR);
}


Real NonDNonHierarchSampling::
average_estimator_variance(const RealVector& cd_vars)
{
  RealVector estvar_ratios(numFunctions, false);
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual: MFMC,ACV,GenACV

  // form estimator variances to pick up dependence on N
  RealVector est_var(numFunctions, false);
  size_t qoi, num_approx = num_approximations();
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // N is a vector constant for opt sub-problem
    if (cd_vars.length() == num_approx) {
      // N_H not provided so pull from latest counter values
      size_t hf_form_index, hf_lev_index;
      hf_indices(hf_form_index, hf_lev_index);
      SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
      for (qoi=0; qoi<numFunctions; ++qoi)
	est_var[qoi] = varH[qoi] / N_H_actual[qoi] * estvar_ratios[qoi];
    }
    else { // r_and_N_to_design_vars(): N_H appended for convenience
      Real N_H = cd_vars[num_approx];
      for (qoi=0; qoi<numFunctions; ++qoi)
	est_var[qoi] = varH[qoi] / N_H * estvar_ratios[qoi];
    }
    break;
  case N_MODEL_LINEAR_OBJECTIVE:  case N_MODEL_LINEAR_CONSTRAINT:
  case R_AND_N_NONLINEAR_CONSTRAINT: {  // N is a scalar optimization variable
    Real N_H = cd_vars[num_approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      est_var[qoi] = varH[qoi] / N_H * estvar_ratios[qoi];
    break;
  }
  }

  Real avg_estvar = average(est_var);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "NonDNonHierarchSampling::average_estimator_variance(): "
	 << "design vars:\n" << cd_vars << "EstVar ratios:\n" << estvar_ratios
	 << "EstVar:\n" << est_var << "average EstVar = " << avg_estvar << '\n';
  return avg_estvar;
}


Real NonDNonHierarchSampling::linear_model_cost(const RealVector& N_m)
{
  // linear objective: N + Sum(w_i N_i) / w
  Real sum = 0., lin_obj;
  for (size_t i=0; i<numApprox; ++i)
    sum += sequenceCost[i] * N_m[i]; // Sum(w_i N_i)
  lin_obj = N_m[numApprox] + sum / sequenceCost[numApprox];// N + Sum / w
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear model cost = " << lin_obj << std::endl;
  return lin_obj;
}


void NonDNonHierarchSampling::
linear_model_cost_gradient(const RealVector& N_vec, RealVector& grad_c)
{
  // linear objective: N + Sum(w_i N_i) / w
  // > grad w.r.t. N_i = w_i / w
  // > grad w.r.t. N   = w   / w = 1
  //size_t i, len = N_vec.length(), r_len = len-1;
  //if (grad_c.length() != len) grad_c.sizeUninitialized(len); // don't own

  Real cost_H = sequenceCost[numApprox];
  for (size_t i=0; i<numApprox; ++i)
    grad_c[i] = sequenceCost[i] / cost_H;
  grad_c[numApprox] = 1.;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear model cost gradient:\n" << grad_c << std::endl;
}


Real NonDNonHierarchSampling::linear_group_cost(const RealVector& N_g)
{
  // default version for case where retainedModelGroups is undefined

  // linear objective: N + Sum(w_i N_i) / w
  Real lin_obj = 0.;
  for (size_t i=0; i<numGroups; ++i)
    lin_obj += modelGroupCost[i] * N_g[i]; // Sum(w_i N_i)
  lin_obj /= sequenceCost[numApprox];// N + Sum / w
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear group cost = " << lin_obj << std::endl;
  return lin_obj;
}


void NonDNonHierarchSampling::
linear_group_cost_gradient(const RealVector& N_vec, RealVector& grad_c)
{
  // default version for case where retainedModelGroups is undefined

  Real cost_H = sequenceCost[numApprox];
  for (size_t i=0; i<numGroups; ++i)
    grad_c[i] = modelGroupCost[i] / cost_H;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear group cost gradient:\n" << grad_c << std::endl;
}


Real NonDNonHierarchSampling::nonlinear_model_cost(const RealVector& r_and_N)
{
  // nln ineq constraint: N ( w + Sum(w_i r_i) ) <= C, where C = equivHF * w
  // -->  N ( 1 + Sum(w_i r_i) / w ) <= equivHF
  Real approx_inner_prod = 0.;
  for (size_t i=0; i<numApprox; ++i)
    approx_inner_prod += sequenceCost[i] * r_and_N[i]; //       Sum(c_i r_i)
  approx_inner_prod /= sequenceCost[numApprox];        //       Sum(w_i r_i)

  Real nln_cost
    = r_and_N[numApprox] * (1. + approx_inner_prod); // N ( 1 + Sum(w_i r_i) )
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "nonlinear cost: design vars:\n" << r_and_N
	 << "cost = " << nln_cost << std::endl;
  return nln_cost;
}


void NonDNonHierarchSampling::
nonlinear_model_cost_gradient(const RealVector& r_and_N, RealVector& grad_c)
{
  // nonlinear inequality constraint: N ( 1 + Sum(w_i r_i) / w ) <= equivHF
  // > grad w.r.t. r_i = N w_i / w
  // > grad w.r.t. N   = 1 + Sum(w_i r_i) / w
  //size_t i, len = r_and_N.length(), r_len = len-1;
  //if (grad_c.length() != len) grad_c.sizeUninitialized(len); // don't own

  Real cost_H = sequenceCost[numApprox], cost_i,
    N_over_w = r_and_N[numApprox] / cost_H, approx_inner_prod = 0.;
  for (size_t i=0; i<numApprox; ++i) {
    cost_i = sequenceCost[i];
    grad_c[i]          = cost_i * N_over_w;
    approx_inner_prod += cost_i * r_and_N[i]; //     Sum(c_i r_i)
  }
  grad_c[numApprox] = 1. + approx_inner_prod / cost_H;     // 1 + Sum(w_i r_i)
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "nonlinear cost gradient:\n" << grad_c << std::endl;
}


void NonDNonHierarchSampling::
npsol_objective(int& mode, int& n, double* x, double& f, double* grad_f,
		int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:
    if (asv_request & 1)
      f = nonHierSampInstance->linear_model_cost(x_rv);
    if (asv_request & 2) {
      RealVector grad_f_rv(Teuchos::View, grad_f, n);
      nonHierSampInstance->linear_model_cost_gradient(x_rv, grad_f_rv);
    }
    break;
  case N_GROUP_LINEAR_OBJECTIVE:
    if (asv_request & 1)
      f = nonHierSampInstance->linear_group_cost(x_rv);
    if (asv_request & 2) {
      RealVector grad_f_rv(Teuchos::View, grad_f, n);
      nonHierSampInstance->linear_group_cost_gradient(x_rv, grad_f_rv);
    }
    break;
  default:
    if (asv_request & 1)
      f = nonHierSampInstance->log_average_estvar(x_rv);
    // NPSOL estimates unspecified components of the obj grad, so ASV grad
    // request is not an error -- just don't specify anything
    //if (asv_request & 2) {
    //  RealVector grad_f_rv(Teuchos::View, grad_f, n);
    //  nonHierSampInstance->log_average_estvar_gradient(x_rv, grad_f_rv);
    //}
    break;
  }
}


void NonDNonHierarchSampling::
npsol_constraint(int& mode, int& ncnln, int& n, int& nrowj, int* needc,
		 double* x, double* c, double* cjac, int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    if (asv_request & 1)
      c[0] = nonHierSampInstance->log_average_estvar(x_rv);
    // NPSOL estimates unspecified components of the constr grad, so ASV grad
    // request is not an error -- just don't specify anything
    //if (asv_request & 2) {
    //  RealVector grad_c_rv(Teuchos::View, cjac, n);
    //  nonHierSampInstance->log_average_estvar_gradient(x_rv, grad_c_rv);
    //}
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    if (asv_request & 1)
      c[0] = nonHierSampInstance->nonlinear_model_cost(x_rv);
    if (asv_request & 2) {
      RealVector grad_c_rv(Teuchos::View, cjac, n);
      nonHierSampInstance->nonlinear_model_cost_gradient(x_rv, grad_c_rv);
    }
    break;
  }
}


/** API for NLF1 objective (see SNLLOptimizer::nlf1_evaluator()) */
void NonDNonHierarchSampling::
optpp_nlf1_objective(int mode, int n, const RealVector& x, double& f, 
		     RealVector& grad_f, int& result_mode)
{
  result_mode = OPTPP::NLPNoOp; // 0
  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:
    if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
      f = nonHierSampInstance->linear_model_cost(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
      nonHierSampInstance->linear_model_cost_gradient(x, grad_f);
      result_mode |= OPTPP::NLPGradient; // adds 2 bit
    }
    break;
  case N_GROUP_LINEAR_OBJECTIVE:
    if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
      f = nonHierSampInstance->linear_group_cost(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
      nonHierSampInstance->linear_group_cost_gradient(x, grad_f);
      result_mode |= OPTPP::NLPGradient; // adds 2 bit
    }
    break;
  default:
    if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
      f = nonHierSampInstance->log_average_estvar(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
      Cerr << "Error: estimator variance gradient not supported in NonHierarch "
	   << "numerical solution." << std::endl;
      abort_handler(METHOD_ERROR);
      //nonHierSampInstance->log_average_estvar_gradient(x, grad_f);
      //result_mode |= OPTPP::NLPGradient; // adds 2 bit
    }
    break;
  }
}


/** API for NLF1 constraint (see SNLLOptimizer::constraint1_evaluator()) */
void NonDNonHierarchSampling::
optpp_nlf1_constraint(int mode, int n, const RealVector& x, RealVector& c,
		      RealMatrix& grad_c, int& result_mode)
{
  result_mode = OPTPP::NLPNoOp; // 0
  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
      c[0] = nonHierSampInstance->log_average_estvar(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
      Cerr << "Error: estimator variance gradient not supported in NonHierarch "
	   << "numerical solution." << std::endl;
      abort_handler(METHOD_ERROR);
      //nonHierSampInstance->log_average_estvar_gradient(x, grad_f);
      //result_mode |= OPTPP::NLPGradient;
    }
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    if (mode & OPTPP::NLPFunction) { // 1 bit is present, mode = 1 or 3
      c[0] = nonHierSampInstance->nonlinear_model_cost(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2 bit is present, mode = 2 or 3
      RealVector grad_c_rv(Teuchos::View, grad_c[0], n); // 0-th col vec
      nonHierSampInstance->nonlinear_model_cost_gradient(x, grad_c_rv);
      result_mode |= OPTPP::NLPGradient; // adds 2 bit
    }
    break;
  }
}


/** API for FDNLF1 objective (see SNLLOptimizer::nlf0_evaluator()) */
void NonDNonHierarchSampling::
optpp_fdnlf1_objective(int n, const RealVector& x, double& f, int& result_mode)
{
  f = nonHierSampInstance->log_average_estvar(x);
  result_mode = OPTPP::NLPFunction; // 1 bit
}


/** API for FDNLF1 constraint (see SNLLOptimizer::constraint0_evaluator()) */
void NonDNonHierarchSampling::
optpp_fdnlf1_constraint(int n, const RealVector& x, RealVector& c,
			int& result_mode)
{
  c[0] = nonHierSampInstance->log_average_estvar(x);
  result_mode = OPTPP::NLPFunction; // 1 bit
}


Real NonDNonHierarchSampling::direct_penalty_merit(const RealVector& cd_vars)
{
  // In addition to accuracy + cost in numerical_solution_bounds_constraints(),
  // there are linear ineq augmentations in augment_linear_ineq_constraints()
  // that are required to protect against inf/nan during the numerical solves.
  // Since DIRECT does not enforce these implicitly during box generation, we
  // penalize any violation of these constraints upstream of estvar solutions.
  // > use a variable numerical penalty that provides trend towards feasibility
  //   --> orders infeasible boxes for further subdivision
  //   --> avoid FPE by using surrogate estvar (iter0 reference)
  // Other notes:
  // > estimator_variance_ratios() converts cd_vars as needed
  // > linear_model_cost() requires N_MODEL_*
  const SizetSizetPair& vm_ind = nonHierSampInstance->varMinIndices;
  const Iterator& direct_min
    = nonHierSampInstance->varianceMinimizers[vm_ind.first][vm_ind.second];
  Real lin_ineq_viol
    = nonHierSampInstance->augmented_linear_ineq_violations(cd_vars,
        direct_min.callback_linear_ineq_coefficients(),
        direct_min.callback_linear_ineq_lower_bounds(),
        direct_min.callback_linear_ineq_upper_bounds());
  bool protect_numerics = (lin_ineq_viol > 0.); // RATIO_NUDGE reflected in viol
  Real obj, constr, constr_u_bnd,
    budget = (Real)nonHierSampInstance->maxFunctionEvals, log_avg_estvar;
  if (protect_numerics) {
    //const RealVector& estvar0 = nonHierSampInstance->estVarIter0;
    log_avg_estvar = //(estvar0.empty()) ? // offline modes
      std::log(Pecos::LARGE_NUMBER);// : std::log(average(estvar0));
    if (nonHierSampInstance->outputLevel >= DEBUG_OUTPUT)
      Cout << "Protect numerics: bypass EstVar computation due to linear ineq "
	   << "constraint violation." << std::endl;
  }
  else
    log_avg_estvar = nonHierSampInstance->log_average_estvar(cd_vars);

  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:
    obj = nonHierSampInstance->linear_model_cost(cd_vars);  break;
  case N_GROUP_LINEAR_OBJECTIVE:
    obj = nonHierSampInstance->linear_group_cost(cd_vars);  break;
  default:
    obj = log_avg_estvar;                                   break;
  }

  switch (nonHierSampInstance->optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    // emulate NonDGenACVSampling::inflate_variables():
    size_t hf_form_index, hf_lev_index;
    nonHierSampInstance->hf_indices(hf_form_index, hf_lev_index);
    Real avg_N_H
      = average(nonHierSampInstance->NLevActual[hf_form_index][hf_lev_index]);
    RealVector N_vec;
    nonHierSampInstance->r_and_N_to_N_vec(cd_vars, avg_N_H, N_vec);

    constr       = nonHierSampInstance->linear_model_cost(N_vec);
    constr_u_bnd = budget;
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT:
    constr       = nonHierSampInstance->nonlinear_model_cost(cd_vars);
    constr_u_bnd = budget;
    break;
  case N_MODEL_LINEAR_CONSTRAINT:
    constr       = nonHierSampInstance->linear_model_cost(cd_vars);
    constr_u_bnd = budget;
    break;
  case N_GROUP_LINEAR_CONSTRAINT:
    constr       = nonHierSampInstance->linear_group_cost(cd_vars);
    constr_u_bnd = budget;
    break;
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    constr       = log_avg_estvar;
    constr_u_bnd = std::log(nonHierSampInstance->convergenceTol*
			    average(nonHierSampInstance->estVarIter0));
    break;
  }

  Real merit = nonHierSampInstance->nh_penalty_merit(obj, constr, constr_u_bnd);
  if (protect_numerics) {
    Real r_p_sq = 1.e+12; // square of r_p from accuracy/cost constraint
    merit += r_p_sq  * lin_ineq_viol * lin_ineq_viol;
  }
  return merit;
}


/** API for MinimizerAdapterModel */
void NonDNonHierarchSampling::
response_evaluator(const Variables& vars, const ActiveSet& set,
		   Response& response)
{
  const ShortArray& asv = set.request_vector();
  size_t i, num_fns = asv.size();//, num_deriv_vars = dvv.size();
  //bool grad_flag = false, hess_flag = false;
  //for (i=0; i<num_fns; ++i) {
  //  if (asv[i] & 2) grad_flag = true;
  //  if (asv[i] & 4) hess_flag = true;
  //}
  bool nln_con = (num_fns > 1);

  const RealVector& c_vars = vars.continuous_variables();
  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:
    if (asv[0] & 1)
      response.function_value(nonHierSampInstance->linear_model_cost(c_vars),0);
    if (asv[0] & 2) {
      RealVector grad_c = response.function_gradient_view(0);
      nonHierSampInstance->linear_model_cost_gradient(c_vars, grad_c);
    }
    break;
  case N_GROUP_LINEAR_OBJECTIVE:
    if (asv[0] & 1)
      response.function_value(nonHierSampInstance->linear_group_cost(c_vars),0);
    if (asv[0] & 2) {
      RealVector grad_c = response.function_gradient_view(0);
      nonHierSampInstance->linear_group_cost_gradient(c_vars, grad_c);
    }
    break;
  default:
    if (asv[0] & 1)
      response.function_value(nonHierSampInstance->
			      log_average_estvar(c_vars), 0);
    if (asv[0] & 2) {
      Cerr << "Error: estimator variance gradient not supported in NonHierarch "
	   << "numerical solution." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }

  switch (nonHierSampInstance->optSubProblemForm) {
  case N_MODEL_LINEAR_OBJECTIVE:  case N_GROUP_LINEAR_OBJECTIVE:
    if (nln_con && (asv[1] & 1))
      response.function_value(nonHierSampInstance->
			      log_average_estvar(c_vars), 1);
    if (nln_con && (asv[1] & 2)) {
      Cerr << "Error: estimator variance gradient not supported in NonHierarch "
	   << "numerical solution." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    break;
  default:
    // R_ONLY_LINEAR_CONSTRAINT has nln_con = 0, so c_vars ends in N_H
    if (nln_con && (asv[1] & 1))
      response.function_value(nonHierSampInstance->
			      nonlinear_model_cost(c_vars), 1);
    if (nln_con && (asv[1] & 2)) {
      RealVector grad_c = response.function_gradient_view(1);
      nonHierSampInstance->nonlinear_model_cost_gradient(c_vars, grad_c);
    }
    break;
  }
}


/** Multi-moment map-based version used for approximation increments */
void NonDNonHierarchSampling::
accumulate_group_sums(IntRealMatrixArrayMap& sum_G, Sizet2DArray& num_G,
		     const IntIntResponse2DMap& batch_resp_map)
{
  IntIntResponse2DMap::const_iterator b_it;
  size_t g, num_groups = modelGroups.size();
  for (g=0; g<num_groups; ++g) {
    b_it = batch_resp_map.find(g); // index g corresponds to group_id key
    if (b_it != batch_resp_map.end()) // else no new evals for this group
      accumulate_group_sums(sum_G, num_G, g, b_it->second);
  }
}


/** Multi-moment map-based version for approximation increments */
void NonDNonHierarchSampling::
accumulate_group_sums(IntRealMatrixArrayMap& sum_G, Sizet2DArray& num_G,
		     size_t group, const IntResponseMap& resp_map)
{
  using std::isfinite;  bool all_finite;
  IntRespMCIter r_cit;  IntRMAMIter g_it;
  Real g_fn, g_prod;  int g_ord, active_ord, ord;

  SizetArray&        num_G_g =       num_G[group];
  const UShortArray& group_g = modelGroups[group];
  size_t qoi, m, g_index, num_models = group_g.size();

  for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit) {
    const Response&   resp    = r_cit->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation():
      // population mean and variance should be computed from same sample set
      all_finite = true;
      for (m=0; m<num_models; ++m) {
	g_index = group_g[m] * numFunctions + qoi;
	if ( (asv[g_index] & 1) == 0 ) {
	  Cerr << "Error: missing data for group " << group+1 << " model "
	       << group_g[m]+1 << '.' << std::endl;
	  abort_handler(METHOD_ERROR);
	}
	if ( !isfinite(fn_vals[g_index]) )
	  all_finite = false; //break;
      }
      if (!all_finite) continue;

      ++num_G_g[qoi]; // shared due to fault tol logic
      for (m=0; m<num_models; ++m) {
	g_index = group_g[m] * numFunctions + qoi;
	g_fn = fn_vals[g_index];

	g_it   = sum_G.begin();
	g_ord  = (g_it == sum_G.end()) ? 0 :  g_it->first;
	g_prod = g_fn;  active_ord = 1;
	while (g_ord) {

	  if (g_ord == active_ord) { // support general key sequence
	    g_it->second[group](qoi,m) += g_prod;
	    ++g_it;  g_ord = (g_it == sum_G.end()) ? 0 : g_it->first;
	  }

	  g_prod *= g_fn;  ++active_ord;
	}
      }
    }
  }
}


void NonDNonHierarchSampling::
print_estimator_performance(std::ostream& s, const MFSolutionData& soln)
{
  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:\n";

  if (pilotMgmtMode == ONLINE_PILOT ||
      pilotMgmtMode == ONLINE_PILOT_PROJECTION) {
    // > reporting estVarIter0 best shows the reference for convTol
    // > recomputing with latest varH is more consistent with metrics to follow 
    //RealVector initial_mc_estvar;
    //compute_mc_estimator_variance(varH, numHIter0, initial_mc_estvar);
    s << "    Initial   MC (" << std::setw(5)
      << (size_t)std::floor(average(numHIter0) + .5) << " HF samples): "
      << std::setw(wpp7) << average(estVarIter0) << '\n';
    //<< std::setw(wpp7) << average(initial_mc_estvar) << '\n';
  }

  String type = (pilotMgmtMode ==  ONLINE_PILOT_PROJECTION ||
		 pilotMgmtMode == OFFLINE_PILOT_PROJECTION)
              ? "Projected" : "   Online";
  //String method = method_enum_to_string(methodName); // string too verbose
  String method = (methodName == MULTIFIDELITY_SAMPLING) ? " MFMC" : "  ACV";
  // Ordering of averages:
  // > recomputing final MC estvar, rather than dividing the two averages, gives
  //   a result that is consistent with average(estVarIter0) when N* = pilot.
  // > The ACV ratio then differs from final ACV / final MC (due to recovering
  //   avgEstVar from the optimizer obj fn), but difference is usually small.
  size_t hf_form_index, hf_lev_index; hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  // est_var is projected for cases that are not fully iterated/incremented
  RealVector final_mc_estvar(numFunctions, false);
  //compute_mc_estimator_variance(varH, N_H_actual, final_mc_estvar);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    final_mc_estvar[qoi] = varH[qoi] / (N_H_actual[qoi] + deltaNActualHF);
  Real proj_equiv_hf = equivHFEvals + deltaEquivHF,
    avg_budget_mc_estvar = average(varH) / proj_equiv_hf,
    avg_estvar = soln.average_estimator_variance();
  s << "  " << type << "   MC (" << std::setw(5)
    << (size_t)std::floor(average(N_H_actual) + deltaNActualHF + .5)
    << " HF samples): " << std::setw(wpp7) << average(final_mc_estvar)
    << "\n  " << type << method << " (sample profile):   "
    << std::setw(wpp7) << avg_estvar
    << "\n  " << type << method << " ratio (1 - R^2):    "
    << std::setw(wpp7) << soln.average_estimator_variance_ratio()
    << "\n Equivalent   MC (" << std::setw(5)
    << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
    << std::setw(wpp7) << avg_budget_mc_estvar
    << "\n Equivalent" << method << " ratio:              "
    << std::setw(wpp7) << avg_estvar / avg_budget_mc_estvar << '\n';
}

} // namespace Dakota
