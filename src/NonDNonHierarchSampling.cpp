/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDNonHierarchSampling
//- Description: Implementation code for NonDNonHierarchSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

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

static const char rcsId[]="@(#) $Id: NonDNonHierarchSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {

// initialization of statics
NonDNonHierarchSampling* NonDNonHierarchSampling::nonHierSampInstance(NULL);


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDNonHierarchSampling::
NonDNonHierarchSampling(ProblemDescDB& problem_db, Model& model):
  NonDEnsembleSampling(problem_db, model), optSubProblemForm(0),
  truthFixedByPilot(problem_db.get_bool("method.nond.truth_fixed_by_pilot"))
{
  // default solver to OPT++ NIP based on numerical experience
  optSubProblemSolver = sub_optimizer_select(
    probDescDB.get_ushort("method.nond.opt_subproblem_solver"),
    SUBMETHOD_NPSOL_OPTPP); // default: compete SQP vs. NIP if both available

  // check iteratedModel for model form hierarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  if (iteratedModel.surrogate_type() == "ensemble")
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
  else {
    Cerr << "Error: sampling the full range of a model ensemble requires an "
	 << "ensemble surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  iteratedModel.multifidelity_precedence(true); // prefer MF, reassign keys
  configure_sequence(numSteps, secondaryIndex, sequenceType);
  numApprox = numSteps - 1;
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  // Precedence: if solution costs provided, then we use them; else we rely
  /// on online cost recovery through response metadata
  onlineCost = !query_cost(numSteps, multilev, sequenceCost);
  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    numSteps, pilotSamples);

  size_t max_ps = find_max(pilotSamples);
  if (max_ps) maxEvalConcurrency *= max_ps;
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
  if (optSubProblemSolver == SUBMETHOD_NPSOL) {
    Iterator sub_iterator = iteratedModel.subordinate_iterator();
    if (!sub_iterator.is_null() && 
	( sub_iterator.method_name() ==     NPSOL_SQP ||
	  sub_iterator.method_name() ==    NLSSOL_SQP ||
	  sub_iterator.uses_method() == SUBMETHOD_NPSOL ||
	  sub_iterator.uses_method() == SUBMETHOD_NPSOL_OPTPP ) )
      sub_iterator.method_recourse();
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	  ( sub_iterator.method_name() ==     NPSOL_SQP ||
	    sub_iterator.method_name() ==    NLSSOL_SQP ||
	    sub_iterator.uses_method() == SUBMETHOD_NPSOL ||
	    sub_iterator.uses_method() == SUBMETHOD_NPSOL_OPTPP ) )
	sub_iterator.method_recourse();
    }
  }
  */

  // assign an aggregate model key that persists for core_run()
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  assign_active_key(multilev);
}


void NonDNonHierarchSampling::assign_active_key(bool multilev)
{
  // For M-model control variate, select fidelities/resolutions
  Pecos::ActiveKey active_key, truth_key;
  std::vector<Pecos::ActiveKey> approx_keys(numApprox);
  //unsigned short truth_form;  size_t truth_lev;
  if (multilev) {
    unsigned short fixed_form = (secondaryIndex == SZ_MAX) ?
      USHRT_MAX : secondaryIndex;
    truth_key.form_key(0, fixed_form, numApprox);
    for (size_t approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, fixed_form, approx);
    //truth_form = fixed_form;  truth_lev = numApprox;
  }
  else if (secondaryIndex == SZ_MAX) { // MF with default resolution level(s)
    truth_key.form_key(0, numApprox,
      iteratedModel.truth_model().solution_level_cost_index());
    for (unsigned short approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, approx,
	iteratedModel.surrogate_model(approx).solution_level_cost_index());
    //truth_form = numApprox;  truth_lev = secondaryIndex;
  }
  else { // MF with assigned resolution level
    truth_key.form_key(0, numApprox, secondaryIndex);
    for (unsigned short approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, approx, secondaryIndex);
    //truth_form = numApprox;  truth_lev = secondaryIndex;
  }
  active_key.aggregate_keys(approx_keys, truth_key, Pecos::RAW_DATA);
  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
  iteratedModel.active_model_key(active_key); // data group 0
  resize_active_set();
}


void NonDNonHierarchSampling::
hf_indices(size_t& hf_form_index, size_t& hf_lev_index)
{
  if (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE) {// resolution hierarchy
    // traps for completeness (undefined model form should not occur)
    hf_form_index = (secondaryIndex == SZ_MAX) ?
      NLevActual.size() - 1 : secondaryIndex;
    // extremes of range
    hf_lev_index = NLevActual[hf_form_index].size() - 1;
  }
  else { // model form hierarchy: HF model is max of range
    hf_form_index = NLevActual.size() - 1;
    if (secondaryIndex == SZ_MAX) {
      size_t c_index = iteratedModel.truth_model().solution_level_cost_index();
      hf_lev_index = (c_index == SZ_MAX) ? 0 : c_index;
    }
    else
      hf_lev_index = secondaryIndex;
  }
}


void NonDNonHierarchSampling::shared_increment(size_t iter)
{
  if (iter == 0) Cout << "\nNon-hierarchical pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << iter
	    << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
    //iteratedModel.active_model_key(agg_key);
    //resize_active_set();

    activeSet.request_values(1);
    ensemble_sample_increment(iter, numApprox+1); // BLOCK if not shared_approx_increment()  *** TO DO: step value
  }
}


void NonDNonHierarchSampling::
shared_increment(size_t iter, const UShortArray& approx_set)
{
  if (iter == 0) Cout << "\nNon-hierarchical pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << iter
	    << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
    //iteratedModel.active_model_key(agg_key);
    //resize_active_set();

    activeSet.request_values(0);
    // truth
    size_t start = numApprox * numFunctions, i, num_approx = approx_set.size();
    activeSet.request_values(1, start, start + numFunctions);
    // active approximations
    for (i=0; i<num_approx; ++i) {
      start = approx_set[i] * numFunctions;
      activeSet.request_values(1, start, start + numFunctions);
    }

    ensemble_sample_increment(iter, numApprox+1); // BLOCK if not shared_approx_increment()  *** TO DO: step value
  }
}


void NonDNonHierarchSampling::shared_approx_increment(size_t iter)
{
  if (iter == 0) Cout << "\nNon-hierarchical approx pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << iter
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

    ensemble_sample_increment(iter, numApprox); // BLOCK  *** TO DO: step value
  }
}


bool NonDNonHierarchSampling::
approx_increment(size_t iter, const SizetArray& approx_sequence,
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

    ensemble_sample_increment(iter, start); // NON-BLOCK
    return true;
  }
  else {
    Cout << "\nNo approx sample increment for approximation sequence ["
	 << start+1 << ", " << end << ']' << std::endl;
    return false;
  }
}


bool NonDNonHierarchSampling::
approx_increment(size_t iter, const SizetArray& approx_sequence,
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

    ensemble_sample_increment(iter, start); // NON-BLOCK
    return true;
  }
  else {
    Cout << "\nNo approx sample increment for approximation sequence ["
	 << start+1 << ", " << end << ']' << std::endl;
    return false;
  }
}


bool NonDNonHierarchSampling::
approx_increment(size_t iter, unsigned short root, const UShortSet& reverse_dag)
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

    ensemble_sample_increment(iter, root); // NON-BLOCK
    return true;
  }
  else
    return false;
}


void NonDNonHierarchSampling::
ensemble_sample_increment(size_t iter, size_t step)
{
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set:
  if (exportSampleSets) { // for HF+LF models, use the HF tags
    export_all_samples("cv_", iteratedModel.active_truth_model(), iter, step);
    for (size_t i=0; i<numApprox; ++i)
      export_all_samples("cv_", iteratedModel.active_surrogate_model(i),
			 iter, step);
  }

  // compute allResponses from allVariables using non-hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


void NonDNonHierarchSampling::recover_online_cost(RealVector& seq_cost)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  size_t cntr, step, num_steps = numApprox+1, num_finite, md_index;
  Real cost, accum;  bool ml = (costMetadataIndices.size() == 1);
  IntRespMCIter r_it;
  using std::isfinite;

  seq_cost.size(num_steps); // init to 0
  for (step=0, cntr=0; step<num_steps; ++step) {
    const SizetSizetPair& cost_mdi = (ml) ? costMetadataIndices[0] :
      costMetadataIndices[step];
    md_index = cntr + cost_mdi.first; // index into aggregated metadata

    accum = 0.;  num_finite = 0;
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      cost = r_it->second.metadata()[md_index]; // offset by index
      if (isfinite(cost))
	{ accum += cost; ++num_finite; }
    }
    seq_cost[step] = accum / num_finite;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Online cost: accum_cost = " << accum << " num_cost = "
	   << num_finite << " seq_cost = " << seq_cost[step] << std::endl;

    cntr += cost_mdi.second; // offset by size of metadata for step
  }
}


void NonDNonHierarchSampling::
mfmc_analytic_solution(const UShortArray& approx_set, const RealMatrix& rho2_LH,
		       const RealVector& cost, DAGSolutionData& soln,
		       bool monotonic_r)
{
  size_t qoi, a, num_approx = approx_set.size(), num_am1 = num_approx - 1;
  RealVector& avg_eval_ratios = soln.avgEvalRatios;
  if (avg_eval_ratios.length() != num_approx) avg_eval_ratios.size(num_approx);
  else                                        avg_eval_ratios = 0.;

  unsigned short approx, prev_approx, last_approx = approx_set[num_am1];
  Real cost_L, cost_H = cost[numApprox]; // HF cost
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

  // Note: one_sided_delta(N_H, hf_targets, 1) enforces monotonicity a bit
  // further downstream.  It averages +/- differences so it's not one-sided
  // per QoI --> to recover the same behavior, we must use avg_eval_ratios.
  // For now, monotonic_r defaults false since it should be redundant.
  if (monotonic_r) {
    Real r_i, prev_ri = 1.;
    for (int i=num_approx-1; i>=0; --i) {
      r_i = std::max(avg_eval_ratios[i], prev_ri);
      prev_ri = avg_eval_ratios[i] = r_i;
    }
  }
}


void NonDNonHierarchSampling::
mfmc_reordered_analytic_solution(const UShortArray& approx_set,
				 const RealMatrix& rho2_LH,
				 const RealVector& cost,
				 SizetArray& approx_sequence,
				 DAGSolutionData& soln, bool monotonic_r)
{
  size_t qoi, a, num_approx = approx_set.size(), num_am1 = num_approx-1;
  RealVector& avg_eval_ratios = soln.avgEvalRatios;
  if (avg_eval_ratios.length() != num_approx)
    avg_eval_ratios.sizeUninitialized(num_approx);

  // employ a single approx reordering that is shared across the QoI
  RealVector avg_rho2_LH(num_approx); // init to 0.
  for (a=0; a<num_approx; ++a)
    avg_rho2_LH[a] = average(rho2_LH[approx_set[a]], numFunctions);
  bool ordered = ordered_approx_sequence(avg_rho2_LH, approx_sequence);
  // Note: even if avg_rho2_LH is now ordered, rho2_LH is not for all QoI, so
  // stick with this alternate formulation, at least for this MFMC iteration.
  if (ordered)
    Cout << "MFMC: averaged correlations are well-ordered.\n" << std::endl;
  else
    Cout << "MFMC: reordered approximation model sequence (low to high):\n"
	 << approx_sequence << std::endl;

  // precompute a factor based on most-correlated approx
  unsigned short approx, inflate_approx;
  size_t most_corr = (ordered) ? num_am1 : approx_sequence[num_am1];
  Real cost_L, cost_H = cost[numApprox], rho2, prev_rho2, rho2_diff,
    factor = cost_H / (1. - avg_rho2_LH[most_corr]);// most correlated
  // Compute averaged eval ratios using averaged rho2 for approx_sequence
  for (a=0; a<num_approx; ++a) {
    approx = (ordered) ? a : approx_sequence[a];
    inflate_approx = approx_set[approx];  cost_L = cost[inflate_approx];// full
    // NOTE: indexing is inverted from Peherstorfer (i+1 becomes i-1)
    rho2_diff = rho2  = avg_rho2_LH[approx]; // contracted
    if (a) rho2_diff -= prev_rho2;
    avg_eval_ratios[approx] = std::sqrt(factor / cost_L * rho2_diff);
    prev_rho2 = rho2;
  }

  // Reverse loop order and enforce monotonicity in reordered r_i
  // > max() is applied bottom-up from the base of the pyramid (samples
  //   performed bottom up, so precedence also applied in this direction),
  //   where assigning r_i = prev_ri effectively drops the CV for approx i
  if (monotonic_r) {
    Real r_i, prev_ri = 1.;
    for (int i=num_approx-1; i>=0; --i) {
      approx = (ordered) ? i : approx_sequence[i];
      r_i = std::max(avg_eval_ratios[approx], prev_ri);
      prev_ri = avg_eval_ratios[approx] = r_i;
    }
  }
}


void NonDNonHierarchSampling::
scale_to_budget_with_pilot(RealVector& avg_eval_ratios, const RealVector& cost,
			   Real avg_N_H)
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
  Real budget = (Real)maxFunctionEvals, cost_H = cost[numApprox],
       factor = (budget / avg_N_H - 1.) / approx_inner_prod * cost_H;
  //avg_eval_ratios.scale(factor); // can result in infeasible r_i < 1

  for (int i=numApprox-1; i>=0; --i) { // repair r_i < 1 first if possible
    Real& r_i = avg_eval_ratios[i];
    r_i *= factor;
    if (r_i <= 1.) { // fix at 1+NUDGE --> scale remaining r_i to reduced budget
      // > only valid for default DAG with all CV targets = truth, otherwise
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


void NonDNonHierarchSampling::
ensemble_numerical_solution(const RealVector& cost,
			    const SizetArray& approx_sequence,
			    DAGSolutionData& soln, size_t& num_samples)
{
  size_t i, num_cdv, num_lin_con, num_nln_con, approx, approx_im1, approx_ip1,
    hf_form_index, hf_lev_index;
  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
  bool ordered = approx_sequence.empty();
  numerical_solution_counts(num_cdv, num_lin_con, num_nln_con); // virtual

  RealVector x0(num_cdv, false), x_lb(num_cdv, false), x_ub(num_cdv, false),
    lin_ineq_lb(num_lin_con, false), lin_ineq_ub(num_lin_con), lin_eq_tgt,
    nln_ineq_lb(num_nln_con, false), nln_ineq_ub(num_nln_con, false),nln_eq_tgt;
  RealMatrix lin_ineq_coeffs(num_lin_con, num_cdv), lin_eq_coeffs;
  numerical_solution_bounds_constraints(soln, cost, avg_N_H, x0, x_lb, x_ub,
					lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
					nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
					lin_ineq_coeffs, lin_eq_coeffs);

  // Perform the numerical solve(s) and recover the solution:
  size_t num_solvers;  bool sequenced_minimizers;
  configure_minimizers(x0, x_lb, x_ub, lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
		       nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, lin_ineq_coeffs,
		       lin_eq_coeffs, num_solvers, sequenced_minimizers);
  run_minimizers(soln, num_solvers, sequenced_minimizers);
  // compute sample increment for HF from current to target:
  num_samples = (truthFixedByPilot) ? 0 :
    one_sided_delta(avg_N_H, soln.avgHFTarget);

  //if (!num_samples) { // metrics not needed unless print_variance_reduction()

  // All cases employ a projected MC estvar to match the projected ACV estvar
  // from N* (where N* may include a num_samples increment not yet performed)
  RealVector mc_estvar;
  project_mc_estimator_variance(varH, N_H_actual, num_samples, mc_estvar);

  // Report ratio of averages rather that average of ratios (see notes in
  // print_variance_reduction())
  soln.avgEstVarRatio = soln.avgEstVar / average(mc_estvar); // (1 - R^2)
  //RealVector estvar_ratio(numFunctions, false);
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  estvar_ratio[qoi] = 1. - R_sq[qoi];// compute from CF_inv,A->compute_Rsq()
  //avg_estvar_ratio = average(estvar_ratio);

  //}
}


void NonDNonHierarchSampling::
numerical_solution_counts(size_t& num_cdv, size_t& num_lin_con,
			  size_t& num_nln_con)
{
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    num_cdv = numApprox;  num_nln_con = 0;
    num_lin_con = 1;
    if (mlmfSubMethod == SUBMETHOD_MFMC) num_lin_con += numApprox;
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    num_cdv = numApprox + 1;  num_nln_con = 1;
    num_lin_con = (mlmfSubMethod == SUBMETHOD_MFMC) ? numApprox : 0;
    break;
  case N_VECTOR_LINEAR_CONSTRAINT:
    num_lin_con = num_cdv = numApprox + 1;  num_nln_con = 0;
    break;
  case N_VECTOR_LINEAR_OBJECTIVE:
    num_cdv = numApprox + 1;  num_nln_con = 1;  num_lin_con = numApprox;
    break;
  }
}


void NonDNonHierarchSampling::
numerical_solution_bounds_constraints(const DAGSolutionData& soln,
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
  const RealVector& avg_eval_ratios = soln.avgEvalRatios;
  Real              avg_hf_target   = soln.avgHFTarget;
  //Real                 avg_estvar = soln.avgEstVar;
  //Real           avg_estvar_ratio = soln.avgEstVarRatio;
  //Real              equiv_hf_cost = soln.equivHFAlloc;

  // For offline mode, online allocations must be lower bounded for numerics:
  // > for QOI_STATISTICS, unbiased moments / CV beta require min of 2 samples
  // > for ESTIMATOR_PERF, a lower bnd of 1 sample is allowable (MC reference)
  //   >> 1 line of thinking: an offline oracle should by as optimal as possible
  //      and we will use for apples-to-apples estimator performance comparisons
  //   >> another line of thinking: be consistent at 2 samples for possible
  //      switch from estimator_perf (selection) to qoi_statistics (execution);
  //      moreover, don't select an estimator based on inconsistent formulation
  // Nonzero lower bound ensures replacement of allSamples after offline pilot.
  Real offline_N_lwr = 2.; //(finalStatsType == QOI_STATISTICS) ? 2. : 1.;

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
  Real cost_H = cost[numApprox], budget = (Real)maxFunctionEvals;

  x_ub        =  DBL_MAX; // no upper bounds on x
  lin_ineq_lb = -DBL_MAX; // no lower bounds on lin ineq

  // Note: ACV paper suggests additional linear constraints for r_i ordering
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    x0   = avg_eval_ratios;
    x_lb = 1.; // r_i
    // set linear inequality constraint for fixed N:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   \Sum_i w_i   r_i <= equivHF * w / N - w
    //   \Sum_i w_i/w r_i <= equivHF / N - 1
    lin_ineq_ub[0] = (avg_N_H > 1.) ? // protect N_H==0 for offline pilot
      budget / avg_N_H - 1. : // normal case
      budget - 1.;            // bound N_H at 1 (TO DO: need to perform sample)
    for (approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0,approx) = cost[approx] / cost_H;
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    copy_data_partial(avg_eval_ratios, x0, 0);          // r_i
    x0[numApprox] = (mlmfIter) ? avg_N_H : avg_hf_target; // N
    // Could allow optimal profile to emerge from pilot by allowing N* less than
    // the incurred cost (e.g., setting N_lb to 1), but instead we bound with
    // the incurred cost by setting x_lb = latest N_H and retaining r_lb = 1.
    x_lb = 1.; // r_i
    x_lb[numApprox] = (pilotMgmtMode == OFFLINE_PILOT) ?
      offline_N_lwr : avg_N_H; //std::floor(avg_N_H + .5); // pilot <= N*

    nln_ineq_lb[0] = -DBL_MAX; // no low bnd
    nln_ineq_ub[0] = budget;
    break;
  case N_VECTOR_LINEAR_CONSTRAINT: {
    Real N_mult = (mlmfIter) ? avg_N_H : avg_hf_target;
    r_and_N_to_N_vec(avg_eval_ratios, N_mult, x0); // N_i = [ {r_i}, 1 ] * N
    if (pilotMgmtMode == OFFLINE_PILOT) {
      x_lb = offline_N_lwr;
      for (i=0; i<num_cdv; ++i) // bump x0 to satisfy x_lb if needed
	if (x0[i] < x_lb[i])
	  x0[i]   = x_lb[i];
    }
    else x_lb = avg_N_H;
    
    // linear inequality constraint on budget:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   N w + \Sum_i w_i N_i <= equivHF * w
    //   N + \Sum_i w_i/w N_i <= equivHF
    lin_ineq_ub[0] = budget; // remaining ub initialized to 0
    for (approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0, approx) = cost[approx] / cost_H;
    lin_ineq_coeffs(0, numApprox) = 1.;
    break;
  }
  case N_VECTOR_LINEAR_OBJECTIVE: {
    Real N_mult = (mlmfIter) ? avg_N_H : avg_hf_target;
    r_and_N_to_N_vec(avg_eval_ratios, N_mult, x0); // N_i = [ {r_i}, 1 ] * N
    if (pilotMgmtMode == OFFLINE_PILOT) {
      x_lb = offline_N_lwr;
      for (i=0; i<num_cdv; ++i) // bump x0 to satisfy x_lb if needed
	if (x0[i] < x_lb[i])
	  x0[i]   = x_lb[i];
    }
    else x_lb = avg_N_H;

    // nonlinear constraint on estvar
    nln_ineq_lb = -DBL_MAX;  // no lower bnd
    nln_ineq_ub = std::log(convergenceTol * average(estVarIter0));
    break;
  }
  }
  // virtual augmentation of linear ineq (differs among MFMC, ACV, GenACV)
  augment_linear_ineq_constraints(lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub);
}


void NonDNonHierarchSampling::
configure_minimizers(RealVector& x0, RealVector& x_lb, RealVector& x_ub,
		     RealVector& lin_ineq_lb, RealVector& lin_ineq_ub,
		     RealVector& lin_eq_tgt,  RealVector& nln_ineq_lb,
		     RealVector& nln_ineq_ub, RealVector& nln_eq_tgt,
		     RealMatrix& lin_ineq_coeffs, RealMatrix& lin_eq_coeffs,
		     size_t& num_solvers, bool& sequenced_minimizers)
{
  bool use_adapter = (optSubProblemSolver != SUBMETHOD_NPSOL &&
		      optSubProblemSolver != SUBMETHOD_OPTPP &&
		      optSubProblemSolver != SUBMETHOD_NPSOL_OPTPP);
  bool use_dfs     = (optSubProblemSolver == SUBMETHOD_SBLO ||
		      optSubProblemSolver == SUBMETHOD_SBGO);
  Model adapt_model, sub_prob_model;
  if (use_adapter) {
    // configure the minimization sub-problem
    adapt_model.assign_rep(std::make_shared<MinimizerAdapterModel>(x0, x_lb,
      x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
      lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, response_evaluator));

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
	if (adapt_model.gradient_type() != "none") data_order |= 2;
	if (adapt_model.hessian_type()  != "none") data_order |= 4;
      }
      Iterator dace_iterator;
      dace_iterator.assign_rep(std::make_shared<NonDLHSSampling>(adapt_model,
	sample_type, samples, seed, rng, vary_pattern, ACTIVE_UNIFORM));
      dace_iterator.active_set_request_values(data_order);

      String approx_type("global_kriging"), point_reuse("none");// TO DO: spec
      UShortArray approx_order; // empty
      ActiveSet dfs_set = adapt_model.current_response().active_set();// copy
      dfs_set.request_values(1);
      const ShortShortPair& dfs_view = adapt_model.current_variables().view();
      sub_prob_model.assign_rep(std::make_shared<DataFitSurrModel>(
	dace_iterator, adapt_model, dfs_set, dfs_view, approx_type,approx_order,
	corr_type, corr_order, data_order, SILENT_OUTPUT, point_reuse));
    }
    else
      sub_prob_model = adapt_model;
  }
      
  // -----------------------------------
  // Configure the variance minimizer(s)
  // -----------------------------------
  // lay groundwork for future global-local hybrids while supporting NPSOL+OPTPP
  num_solvers = 1;  sequenced_minimizers = false;
  switch (optSubProblemSolver) {
  case SUBMETHOD_NPSOL_OPTPP:
    num_solvers = 2; break;
  }
  if (varianceMinimizers.empty()) {
    varianceMinimizers.resize(num_solvers);

    size_t max_iter = 100000, max_eval = 500000;
    Real   conv_tol = 1.e-8; // tight convergence

    // select the sub-problem solver
    switch (optSubProblemSolver) {
    case SUBMETHOD_SBLO: {
      short merit_fn = AUGMENTED_LAGRANGIAN_MERIT, accept_logic = FILTER,
	constr_relax = NO_RELAX;
      unsigned short soft_conv_limit = 5;
      Real tr_factor = .5;
      varianceMinimizers[0].assign_rep(
	std::make_shared<DataFitSurrBasedLocalMinimizer>(sub_prob_model,
	merit_fn, accept_logic, constr_relax, tr_factor, max_iter, max_eval,
	conv_tol, soft_conv_limit, false));
      break;
    }
    case SUBMETHOD_EGO: {
      // EGO builds its own GP in initialize_sub_problem(), so a DFSModel
      // does not need to be constructed here as for SBLO/SBGO
      // > TO DO: pure global opt may need subsequent local refinement
      int samples = 100, seed = 12347;      // TO DO: spec
      String approx_type("global_kriging"); // TO DO: spec
      bool use_derivs = false;
      varianceMinimizers[0].assign_rep(std::make_shared<EffGlobalMinimizer>(
	sub_prob_model, approx_type, samples, seed, use_derivs, max_iter,
	max_eval, conv_tol));
      break;
    }
    /*
    case SUBMETHOD_CPS: { // may need to be combined with local refinement
      break;
    }
    case SUBMETHOD_SBGO: { // for NonDGlobalInterval, was EAminlp + GP ...
      varianceMinimizers[0].assign_rep(
        std::make_shared<SurrBasedGlobalMinimizer>(sub_prob_model, max_iter,
	                                           max_eval, conv_tol));
      break;
    }
    case SUBMETHOD_EA: { // may need to be combined with local refinement
      varianceMinimizers[0].assign_rep(std::make_shared<COLINOptimizer>(
        sub_prob_model, max_iter, max_eval, conv_tol));
      break;
    }
    */
    default: { // existing call-back APIs do not require adapter or surrogate
      ShortArray solvers(num_solvers);
      switch (optSubProblemSolver) {
      case SUBMETHOD_NPSOL: case SUBMETHOD_OPTPP:
	solvers[0] = optSubProblemSolver;  break;
      case SUBMETHOD_NPSOL_OPTPP:
	solvers[0] = SUBMETHOD_NPSOL;
	solvers[1] = SUBMETHOD_OPTPP;  break;
      default: // SUBMETHOD_NONE, ...
	Cerr << "Error: sub-problem solver '"
	     << submethod_enum_to_string(optSubProblemSolver)
	     << "' unavailable in NonDNonHierarchSampling::"
	     << "configure_minimizers()." << std::endl;
	abort_handler(METHOD_ERROR);
	break;
      }
      for (size_t i=0; i<num_solvers; ++i)
	switch (solvers[i]) {
	case SUBMETHOD_NPSOL: {
	  Real fdss = 1.e-6; int deriv_level;// 0 none, 1 obj, 2 constr, 3 both
	  switch (optSubProblemForm) {
	  case R_AND_N_NONLINEAR_CONSTRAINT: deriv_level = 2;  break;
	  case N_VECTOR_LINEAR_OBJECTIVE:    deriv_level = 1;  break;
	  default:                           deriv_level = 0;  break;
	  }
#ifdef HAVE_NPSOL
	  varianceMinimizers[i].assign_rep(std::make_shared<NPSOLOptimizer>(x0,
	    x_lb, x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub,lin_eq_coeffs,
            lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, npsol_objective,
	    npsol_constraint, deriv_level, conv_tol, max_iter, fdss));
#endif
	  break;
	}
	case SUBMETHOD_OPTPP: {
	  Real max_step = 100000.;
#ifdef HAVE_OPTPP
	  switch (optSubProblemForm) {
	  case N_VECTOR_LINEAR_OBJECTIVE:
	    varianceMinimizers[i].assign_rep(std::make_shared<SNLLOptimizer>(x0,
	      x_lb, x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub,
	      lin_eq_coeffs, lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	      optpp_nlf1_objective, optpp_fdnlf1_constraint, max_iter, max_eval,
	      conv_tol, conv_tol, max_step));
	    break;
	  default:
	    varianceMinimizers[i].assign_rep(std::make_shared<SNLLOptimizer>(x0,
	      x_lb, x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub,
	      lin_eq_coeffs, lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	      optpp_fdnlf1_objective, optpp_nlf1_constraint, max_iter, max_eval,
	      conv_tol, conv_tol, max_step));
	    break;
	  }
#endif
	  break;
	}
	}
      break;
    }
    }
  }
  else if (use_adapter) {
    // *** TO DO: (existing active view accessors don't support size change)
    //adapt_model.update_active_variables(x0, x_lb, x_ub);
    //adapt_model.update_active_constraints(lin_ineq_coeffs, lin_ineq_lb,
    //  lin_ineq_ub, lin_eq_coeffs, lin_eq_tgt, nln_ineq_lb, nln_ineq_ub,
    //  nln_eq_tgt);
    if (use_dfs) sub_prob_model.update_from_subordinate_model();
    for (size_t i=0; i<num_solvers; ++i)
      varianceMinimizers[i].update_from_model(sub_prob_model);
  }
  else
    for (size_t i=0; i<num_solvers; ++i)
      varianceMinimizers[i].update_callback_data(x0, x_lb, x_ub,
	lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs, lin_eq_tgt,
	nln_ineq_lb, nln_ineq_ub, nln_eq_tgt);
}


void NonDNonHierarchSampling::
run_minimizers(DAGSolutionData& soln, size_t num_solvers,
	       bool sequenced_minimizers)
{
  // ----------------------------------
  // Solve the optimization sub-problem: compute optimal r*,N*
  // ----------------------------------
  Real merit_fn, merit_fn_star = DBL_MAX;
  size_t i, last_index = num_solvers - 1, best_min = last_index;
  for (i=0; i<num_solvers; ++i) {
    Iterator& min_i = varianceMinimizers[i];
    min_i.run();
    const Variables& vars_star = min_i.variables_results();
    const RealVector&  cv_star = vars_star.continuous_variables();
    const RealVector&  fn_star = min_i.response_results().function_values();
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "ensemble_numerical_solution() results:\ncv_star =\n"
	   << cv_star << "fn_vals_star =\n" << fn_star;

    if (sequenced_minimizers) { // coordinated optimization
      if (i < last_index) // propagate final pt to next initial pt
	varianceMinimizers[i+1].iterated_model().active_variables(vars_star);
    }
    else {                      //    competed optimization
      // track best using penalty merit fn comprised of accuracy and cost
      merit_fn = nh_penalty_merit(cv_star, fn_star);
      if (i == 0 || merit_fn < merit_fn_star)
	{ merit_fn_star = merit_fn;  best_min = i; }
    }
  }

  // -------------------------------------
  // Post-process the optimization results
  // -------------------------------------
  const Iterator& min_s = varianceMinimizers[best_min];
  if (outputLevel >= NORMAL_OUTPUT && num_solvers > 1 && !sequenced_minimizers)
    Cout << "ensemble_numerical_solution() best solver = "
	 << min_s.method_string() << std::endl;
  recover_results(min_s.variables_results().continuous_variables(),
		  min_s.response_results().function_values(), soln.avgEstVar,
		  soln.avgEvalRatios, soln.avgHFTarget, soln.equivHFAlloc);
}


void NonDNonHierarchSampling::
recover_results(const RealVector& cv_star, const RealVector& fn_star,
		Real& avg_estvar, RealVector& avg_eval_ratios,
		Real& avg_hf_target, Real& equiv_hf_cost)
{
  // Estvar recovery from optimizer provides std::log(average(nh_estvar)) =
  // var_H / N_H (1 - R^2).  Notes:
  // > a QoI-vector prior to averaging would require recomputation from r*,N*)
  // > this value corresponds to N* (_after_ num_samples applied)
  avg_estvar = (optSubProblemForm == N_VECTOR_LINEAR_OBJECTIVE) ?
    std::exp(fn_star[1]) : std::exp(fn_star(0));

  // Recover optimizer results for average {eval_ratios,estvar}.  Also compute
  // shared increment from N* or from targeting specified budget || accuracy.
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    copy_data(cv_star, avg_eval_ratios); // r*
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
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    // R_AND_N:  r*   is leading part of cv_star and N* is trailing entry
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios); // r*
    avg_hf_target = cv_star[numApprox];                             // N*
    break;
  case N_VECTOR_LINEAR_OBJECTIVE: case N_VECTOR_LINEAR_CONSTRAINT:
    // N_VECTOR: N*_i is leading part of cv_star and N* is trailing entry.
    // Note that r_i is always relative to HF N, not its DAG pairing.
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios); // N*_i
    avg_hf_target = cv_star[numApprox];                             // N*
    avg_eval_ratios.scale(1. / avg_hf_target);        // r*_i = N*_i / N*
    break;
  }

  // Cost recovery:
  switch (optSubProblemForm) {
  case N_VECTOR_LINEAR_OBJECTIVE:
    equiv_hf_cost = fn_star[0];  break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    equiv_hf_cost = fn_star[1];  break;
  default:
    equiv_hf_cost
      = compute_equivalent_cost(avg_hf_target, avg_eval_ratios, sequenceCost);
    break;
  }
}


// Minimizer::penalty_merit() uses too many Minimizer attributes, so we
// use local definitions here
Real NonDNonHierarchSampling::
nh_penalty_merit(const RealVector& c_vars, const RealVector& fn_vals)
{
  // Assume linear constraints are satisfied (for now)
  // Keep accuracy in log space and normalize both cost and log-accuracy

  Real budget = (Real)maxFunctionEvals;
  switch (optSubProblemForm) {
  case N_VECTOR_LINEAR_OBJECTIVE:
    return nh_penalty_merit(fn_vals[0], fn_vals[1],
			    std::log(convergenceTol*average(estVarIter0)));
    break;

  case R_AND_N_NONLINEAR_CONSTRAINT:
    return nh_penalty_merit(fn_vals[0], fn_vals[1], budget);  break;

  //default: return fn_vals[0]; break; // no nln con, assume linear are enforced
  default:  { // more involved recovery of equiv cost from cv_star
    Real avg_estvar, avg_hf_target, equiv_hf_cost;  RealVector avg_eval_ratios;
    recover_results(c_vars, fn_vals, avg_estvar, avg_eval_ratios,
		    avg_hf_target, equiv_hf_cost);
    return nh_penalty_merit(std::log(avg_estvar), equiv_hf_cost, budget);
    break;
  }
  }
}


// Minimizer::penalty_merit() uses too many Minimizer attributes, so we
// use local definitions here
Real NonDNonHierarchSampling::nh_penalty_merit(const DAGSolutionData& soln)
{
  // Assume linear constraints are satisfied (for now)
  // Keep accuracy in log space and normalize both cost and log-accuracy

  switch (optSubProblemForm) {
  case N_VECTOR_LINEAR_OBJECTIVE:
    return nh_penalty_merit(soln.equivHFAlloc, std::log(soln.avgEstVar),
			    std::log(convergenceTol*average(estVarIter0)));
    break;
  default: //case R_AND_N_NONLINEAR_CONSTRAINT:
    return nh_penalty_merit(std::log(soln.avgEstVar), soln.equivHFAlloc,
			    (Real)maxFunctionEvals);
    break;
  //default: // no nonlinear constraint, log scaling not necessary for estvar
  //  return soln.avgEstVar;  break;
  }
}


Real NonDNonHierarchSampling::
nh_penalty_merit(Real obj, Real nln_con, Real nln_u_bnd)
{
  Real merit_fn = obj, constr = nln_con / nln_u_bnd, // normalize
    r_p = 1.e+6, c_tol = .01, c_tol_p1 = 1. + c_tol, constr_viol = 0.;
  if (constr > c_tol_p1) {
    constr_viol = constr - c_tol_p1;
    merit_fn += r_p * constr_viol * constr_viol;
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Penalty merit fn = " << merit_fn << " from obj = " << obj
	 << " constraint viol = " << constr_viol << std::endl;
  return merit_fn;
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
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
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
	 << "average((1. - Rsq) varH / N) = " << avg_estvar << '\n';
  return avg_estvar;
}


Real NonDNonHierarchSampling::log_average_estvar(const RealVector& cd_vars)
{
  Real avg_est_var = average_estimator_variance(cd_vars);
  if (avg_est_var > 0.)
    return std::log(avg_est_var); // use log to flatten contours
  else
    return std::numeric_limits<Real>::quiet_NaN();//Pecos::LARGE_NUMBER;
}


Real NonDNonHierarchSampling::linear_cost(const RealVector& N_vec)
{
  // linear objective: N + Sum(w_i N_i) / w
  Real sum = 0., lin_obj;
  for (size_t i=0; i<numApprox; ++i)
    sum += sequenceCost[i] * N_vec[i]; // Sum(w_i N_i)
  lin_obj = N_vec[numApprox] + sum / sequenceCost[numApprox];// N + Sum / w
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear cost = " << lin_obj << std::endl;
  return lin_obj;
}


Real NonDNonHierarchSampling::nonlinear_cost(const RealVector& r_and_N)
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
linear_cost_gradient(const RealVector& N_vec, RealVector& grad_c)
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
    Cout << "linear cost gradient:\n" << grad_c << std::endl;
}


void NonDNonHierarchSampling::
nonlinear_cost_gradient(const RealVector& r_and_N, RealVector& grad_c)
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
  case N_VECTOR_LINEAR_OBJECTIVE:
    if (asv_request & 1)
      f = nonHierSampInstance->linear_cost(x_rv);
    if (asv_request & 2) {
      RealVector grad_f_rv(Teuchos::View, grad_f, n);
      nonHierSampInstance->linear_cost_gradient(x_rv, grad_f_rv);
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
  case N_VECTOR_LINEAR_OBJECTIVE:
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
      c[0] = nonHierSampInstance->nonlinear_cost(x_rv);
    if (asv_request & 2) {
      RealVector grad_c_rv(Teuchos::View, cjac, n);
      nonHierSampInstance->nonlinear_cost_gradient(x_rv, grad_c_rv);
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
  case N_VECTOR_LINEAR_OBJECTIVE:
    if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
      f = nonHierSampInstance->linear_cost(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
      nonHierSampInstance->linear_cost_gradient(x, grad_f);
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
  case N_VECTOR_LINEAR_OBJECTIVE:
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
      c[0] = nonHierSampInstance->nonlinear_cost(x);
      result_mode |= OPTPP::NLPFunction; // adds 1 bit
    }
    if (mode & OPTPP::NLPGradient) { // 2 bit is present, mode = 2 or 3
      RealVector grad_c_rv(Teuchos::View, grad_c[0], n); // 0-th col vec
      nonHierSampInstance->nonlinear_cost_gradient(x, grad_c_rv);
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
  case N_VECTOR_LINEAR_OBJECTIVE:
    if (asv[0] & 1)
      response.function_value(nonHierSampInstance->linear_cost(c_vars), 0);
    if (asv[0] & 2) {
      RealVector grad_c = response.function_gradient_view(0);
      nonHierSampInstance->linear_cost_gradient(c_vars, grad_c);
    }

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
    if (asv[0] & 1)
      response.function_value(nonHierSampInstance->
			      log_average_estvar(c_vars), 0);
    if (asv[0] & 2) {
      Cerr << "Error: estimator variance gradient not supported in NonHierarch "
	   << "numerical solution." << std::endl;
      abort_handler(METHOD_ERROR);
    }

    // R_ONLY_LINEAR_CONSTRAINT has nln_con = 0, so c_vars ends in N_H
    if (nln_con && (asv[1] & 1))
      response.function_value(nonHierSampInstance->nonlinear_cost(c_vars), 1);
    if (nln_con && (asv[1] & 2)) {
      RealVector grad_c = response.function_gradient_view(1);
      nonHierSampInstance->nonlinear_cost_gradient(c_vars, grad_c);
    }
    break;
  }
}


void NonDNonHierarchSampling::
print_estimator_performance(std::ostream& s, const DAGSolutionData& soln)
{
  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:\n";

  if (pilotMgmtMode != OFFLINE_PILOT) {
    // > reporting estVarIter0 best shows the reference for convTol
    // > recomputing with latest varH is more consistent with metrics to follow 
    //RealVector initial_mc_estvar;
    //compute_mc_estimator_variance(varH, numHIter0, initial_mc_estvar);
    s << "    Initial   MC (" << std::setw(5)
      << (size_t)std::floor(average(numHIter0) + .5) << " HF samples): "
      << std::setw(wpp7) << average(estVarIter0) << '\n';
    //<< std::setw(wpp7) << average(initial_mc_estvar) << '\n';
  }

  String type = (pilotMgmtMode == PILOT_PROJECTION) ? "Projected" : "   Online";
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
    avg_budget_mc_estvar = average(varH) / proj_equiv_hf;
  s << "  " << type << "   MC (" << std::setw(5)
    << (size_t)std::floor(average(N_H_actual) + deltaNActualHF + .5)
    << " HF samples): " << std::setw(wpp7) << average(final_mc_estvar)
    << "\n  " << type << method << " (sample profile):   "
    << std::setw(wpp7) << soln.avgEstVar
    << "\n  " << type << method << " ratio (1 - R^2):    "
    << std::setw(wpp7) << soln.avgEstVarRatio
    << "\n Equivalent   MC (" << std::setw(5)
    << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
    << std::setw(wpp7) << avg_budget_mc_estvar
    << "\n Equivalent" << method << " ratio:              "
    << std::setw(wpp7) << soln.avgEstVar / avg_budget_mc_estvar << '\n';
}

} // namespace Dakota
