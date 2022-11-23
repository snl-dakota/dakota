/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDControlVariateSampling
//- Description: Implementation code for NonDControlVariateSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDControlVariateSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDControlVariateSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDControlVariateSampling::
NonDControlVariateSampling(ProblemDescDB& problem_db, Model& model):
  NonDHierarchSampling(problem_db, model)//, finalCVRefinement(true)
{
  // Want to define this as construct time for early run-time use in
  // HierarchSurrModel::create_tabular_datastream().  Note that MLCV will have
  // two overlapping assignments, one from this ctor (first) that is then
  // overwritten by the ML ctor (second).  Alternatively we could protect this
  // call with methodName, but the current behavior is sufficient.
  iteratedModel.multifidelity_precedence(true); // prefer MF, reassign keys
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDControlVariateSampling::core_run()
{
  configure_sequence(numSteps, secondaryIndex, sequenceType);
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  onlineCost = !query_cost(numSteps, multilev, sequenceCost);
  if (onlineCost) sequenceCost.size(2); // collapse model recovery down to 2
  deltaNActualHF = 0;

  // For two-model control variate, select extreme fidelities/resolutions
  Pecos::ActiveKey active_key, hf_key, lf_key;
  unsigned short hf_form, lf_form;  size_t hf_lev, lf_lev;
  if (multilev) {
    if (numSteps > 2)
      Cerr << "Warning: NonDControlVariateSampling uses first and last "
	   << "resolution levels and ignores the rest." << std::endl;
    hf_lev  = numSteps - 1;  lf_lev = 0;  // extremes of range
    hf_form = lf_form = (secondaryIndex == SZ_MAX) ? USHRT_MAX : secondaryIndex;
  }
  else {
    if (numSteps > 2)
      Cerr << "Warning: NonDControlVariateSampling uses first and last models "
	   << "in ordered sequence and ignores the rest." << std::endl;
    hf_form = numSteps - 1;  lf_form = 0; // extremes of range
    if (secondaryIndex == SZ_MAX) {
      hf_lev =     iteratedModel.truth_model().solution_level_cost_index();
      lf_lev = iteratedModel.surrogate_model().solution_level_cost_index();
    }
    else
      hf_lev = lf_lev = secondaryIndex;
  }
  lf_key.form_key(0, lf_form, lf_lev);  hf_key.form_key(0, hf_form, hf_lev);
  active_key.aggregate_keys(lf_key, hf_key, Pecos::RAW_DATA);

  iteratedModel.surrogate_response_mode(AGGREGATED_MODEL_PAIR);
  iteratedModel.active_model_key(active_key); // data group 0
  resize_active_set();

  // Two-model control variate approach (Ng and Willcox, 2014) using a
  // hierarchical model (requires model pairing)
  switch (pilotMgmtMode) {
  case ONLINE_PILOT:
    control_variate_mc(active_key);                  break;
  case OFFLINE_PILOT:
    control_variate_mc_offline_pilot(active_key);    break;
  case PILOT_PROJECTION:
    control_variate_mc_pilot_projection(active_key); break;
  }
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDControlVariateSampling::
control_variate_mc(const Pecos::ActiveKey& active_key)
{
  size_t hf_form_index, lf_form_index, hf_lev_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_actual_shared = NLevActual[hf_form_index][hf_lev_index];
  SizetArray& N_actual_lf     = NLevActual[lf_form_index][lf_lev_index];
  N_actual_shared.assign(numFunctions, 0);//N_actual_lf.assign(numFunctions, 0);
  size_t&     N_alloc_shared  = NLevAlloc[hf_form_index][hf_lev_index];
  size_t&     N_alloc_lf      = NLevAlloc[lf_form_index][lf_lev_index];
  N_alloc_shared = 0;

  IntRealVectorMap sum_L_shared, sum_H, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH);
  RealVector eval_ratios, mu_hat, hf_targets, sum_HH(numFunctions),
    rho2_LH(numFunctions, false);
  varH.sizeUninitialized(numFunctions);

  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, 2, delta_N_l); // 2 models only
  numSamples = std::min(delta_N_l[0], delta_N_l[1]);
  Real cost_ratio = 0.;

  while (numSamples && mlmfIter <= maxIterations) {

    // -----------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,estvar}_ratios
    // -----------------------------------------------------------------------
    shared_increment(active_key, mlmfIter, 0);
    accumulate_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH,
		       N_actual_shared);
    N_alloc_shared += (backfillFailures && mlmfIter) ?
      one_sided_delta(N_alloc_shared, average(hf_targets)) : numSamples;
    if (mlmfIter == 0) {
      if (onlineCost) recover_paired_online_cost(sequenceCost, 1);
      cost_ratio  = (onlineCost) ? sequenceCost[1] : sequenceCost[numSteps - 1];
      cost_ratio /= sequenceCost[0]; // HF / LF
    }
    increment_mf_equivalent_cost(numSamples, numSamples, cost_ratio,
				 equivHFEvals);

    // Compute the LF/HF evaluation ratio using shared samples, averaged
    // over QoI.  This includes updating varH and rho2_LH.
    compute_eval_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1], sum_HH,
			cost_ratio, N_actual_shared, varH, rho2_LH,eval_ratios);
    // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
    // sum_L_refined / N_refined is zero for CV prior to sample refinement.
    // (This differs from MLMC estvar^0 which uses pilot for all levels.)
    if (mlmfIter == 0) {
      compute_mc_estimator_variance(varH, N_actual_shared, estVarIter0);
      numHIter0 = N_actual_shared;
    }
    // Compute the ratio of MC and CVMC mean squared errors (for convergence).
    // This ratio incorporates the anticipated variance reduction from the
    // upcoming application of eval_ratios.
    compute_estvar_ratios(eval_ratios, rho2_LH, estVarRatios);

    // -----------------------------------------------------------------------
    // Compute shared increment targeting specified budget || estvar reduction
    // -----------------------------------------------------------------------
    if (maxFunctionEvals != SZ_MAX) {
      Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
      allocate_budget(eval_ratios, cost_ratio, hf_targets);
    }
    else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: support both
      // N_H = estvar_ratio * varH / convTol / estVarIter0
      // Note: don't simplify further since estVarIter0 is fixed based on pilot
      Cout << "Scaling profile for convergenceTol = " << convergenceTol;
      hf_targets = estVarRatios;
      for (size_t qoi=0; qoi<numFunctions; ++qoi)
	hf_targets[qoi] *= varH[qoi] / estVarIter0[qoi] / convergenceTol;
    }
    // numSamples is relative to N_H, but the approx_increments() below are
    // computed relative to hf_targets (independent of sunk cost for pilot)
    // Note: don't backfill failures (unless specific user override?).
    // Reasons: inconsistent w/ budget alloc; inconsistent w/ pilot handling;
    //          would need to carry over to LF increments as well, which are not
    //          iterated; may amplify corr between params and successes/failures
    Cout << ": average HF target = " << average(hf_targets) << std::endl;
    numSamples = (backfillFailures) ? // new option for accuracy tolerance
      one_sided_delta(N_actual_shared,        hf_targets, 1) : // average
      one_sided_delta(N_alloc_shared, average(hf_targets));
    //numSamples = std::min(num_samp_budget, num_samp_ctol);
    // exclude failure backfill from alloc increment, if needed

    //Cout << "\nCVMC iteration " << mlmfIter << " complete." << std::endl;
    ++mlmfIter;
  } // end while

  // -------------------------------------------------------------------
  // Compute new LF increment based on new evaluation ratio for new N_hf
  // -------------------------------------------------------------------
  // shared LF/HF samples to this point, as tracked by N_*_shared
  N_actual_lf = N_actual_shared;  N_alloc_lf = N_alloc_shared;
  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    // Note: these increments do not affect iteration above and can be performed
    // after N_hf has converged, which simplifies maxFnEvals / convTol logic
    // (no need to further interrogate these throttles below)
    IntRealVectorMap sum_L_refined = sum_L_shared;
    Pecos::ActiveKey lf_key;  active_key.extract_key(0, lf_key);
    RealVector lf_targets;
    if (backfillFailures) { // increment relative to successful samples
      lf_increment(lf_key, eval_ratios, N_actual_lf, hf_targets, lf_targets,
		   mlmfIter, 0);
      N_alloc_lf += one_sided_delta(N_alloc_lf, average(lf_targets));
    }
    else {                  // increment relative to allocated samples
      lf_increment(lf_key, eval_ratios, N_alloc_lf,  hf_targets, lf_targets,
		   mlmfIter, 0);
      N_alloc_lf += numSamples;
    }
    if (numSamples) {
      accumulate_mf_sums(sum_L_refined, N_actual_lf);
      increment_mf_equivalent_cost(numSamples, cost_ratio, equivHFEvals);
    }

    // Compute/apply control variate params to estimate uncentered raw moments
    RealMatrix H_raw_mom(numFunctions, 4);
    cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_actual_shared,
		   sum_L_refined, N_actual_lf, H_raw_mom);
    // Convert uncentered raw moment estimates to final moments (central or std)
    convert_moments(H_raw_mom, momentStats);
  }
  else // for consistency with pilot projection
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(hf_targets, eval_ratios, cost_ratio,
				N_actual_lf, N_alloc_lf, deltaEquivHF);

  // estVarRatios is NOT scaled when overshooting hf_target --> since lf_targets
  // are computed from eval_ratios * hf_targets, taking credit for all of
  // N_actual and all of eval/estvar ratios is inaccurate/optimistic.  This
  // deprecated implementation gets fixed in the MFMC 2-model replacement.
  estvar_ratios_to_avg_estvar(estVarRatios, varH, N_actual_shared, avgEstVar);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDControlVariateSampling::
control_variate_mc_offline_pilot(const Pecos::ActiveKey& active_key)
{
  size_t hf_form_index, lf_form_index, hf_lev_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_actual_shared = NLevActual[hf_form_index][hf_lev_index];
  SizetArray& N_actual_lf     = NLevActual[lf_form_index][lf_lev_index];
  N_actual_shared.assign(numFunctions, 0);//N_actual_lf.assign(numFunctions, 0);
  size_t&     N_alloc_shared  = NLevAlloc[hf_form_index][hf_lev_index];
  size_t&     N_alloc_lf      = NLevAlloc[lf_form_index][lf_lev_index];
  //N_alloc_shared = N_alloc_lf = 0;

  // ---------------------------------------------------------------------
  // Compute final rho2LH, varH, {eval,estvar} ratios from (oracle) pilot
  // treated as "offline" cost
  // ---------------------------------------------------------------------
  RealVector eval_ratios, hf_targets;  Real cost_ratio;
  SizetArray N_offline_shared;
  evaluate_pilot(active_key, cost_ratio, eval_ratios, varH, N_offline_shared,
		 hf_targets, false, false); // no cost, estvar

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealVectorMap sum_L_shared, sum_H, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions);

  // online samples are zero; at least 2 samples reqd for online variance/corr
  N_alloc_shared = numSamples =
    std::max(one_sided_delta(N_actual_shared, hf_targets, 1), (size_t)2);
  shared_increment(active_key, mlmfIter, 0);
  accumulate_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH,
		     N_actual_shared);
  increment_mf_equivalent_cost(numSamples, numSamples, cost_ratio,equivHFEvals);

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  N_actual_lf = N_actual_shared;  N_alloc_lf = N_alloc_shared;
  if (finalStatsType == QOI_STATISTICS) {
    IntRealVectorMap sum_L_refined = sum_L_shared;
    // shared LF/HF samples to this point, as tracked by N_*_shared
    Pecos::ActiveKey lf_key;  active_key.extract_key(0, lf_key);
    RealVector lf_targets;
    if (backfillFailures) { // increment relative to successful samples
      lf_increment(lf_key, eval_ratios, N_actual_lf, hf_targets, lf_targets,
		   mlmfIter, 0);
      N_alloc_lf += one_sided_delta(N_alloc_lf, average(lf_targets));
    }
    else {                  // increment relative to allocated samples
      lf_increment(lf_key, eval_ratios, N_alloc_lf,  hf_targets, lf_targets,
		   mlmfIter, 0);
      N_alloc_lf += numSamples;
    }
    if (numSamples) {
      accumulate_mf_sums(sum_L_refined, N_actual_lf);
      increment_mf_equivalent_cost(numSamples, cost_ratio, equivHFEvals);
    }
    // Compute/apply control variate params to estimate uncentered raw moments
    RealMatrix H_raw_mom(numFunctions, 4);
    cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_actual_shared,
		   sum_L_refined, N_actual_lf, H_raw_mom);
    // Convert uncentered raw moment estimates to final moments (central or std)
    convert_moments(H_raw_mom, momentStats);
  }
  else // for consistency with pilot projection
    // N_H is converged --> deltaNActualHF is 0 --> no change to estVar
    update_projected_lf_samples(hf_targets, eval_ratios, cost_ratio,
				N_actual_lf, N_alloc_lf, deltaEquivHF);

  // For offline, no concern w/ inaccurate estvar from overshooting hf_targets
  estvar_ratios_to_avg_estvar(estVarRatios, varH, N_actual_shared, avgEstVar);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDControlVariateSampling::
control_variate_mc_pilot_projection(const Pecos::ActiveKey& active_key)
{
  size_t hf_form_index, lf_form_index, hf_lev_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_actual_shared = NLevActual[hf_form_index][hf_lev_index];
  SizetArray& N_actual_lf     = NLevActual[lf_form_index][lf_lev_index];
  N_actual_shared.assign(numFunctions, 0);//N_actual_lf.assign(numFunctions, 0);
  size_t&     N_alloc_shared  = NLevAlloc[hf_form_index][hf_lev_index];
  size_t&     N_alloc_lf      = NLevAlloc[lf_form_index][lf_lev_index];

  RealVector eval_ratios, hf_targets;  Real cost_ratio;
  evaluate_pilot(active_key, cost_ratio, eval_ratios, varH, N_actual_shared,
		 hf_targets, true, true); // accumulate cost, compute estvar0
  N_alloc_shared = N_alloc_lf = numSamples;
  N_actual_lf = N_actual_shared; // only N_actual_shared updated so far

  update_projected_samples(hf_targets, eval_ratios, cost_ratio,
			   N_actual_shared, N_alloc_shared, N_actual_lf,
			   N_alloc_lf, deltaNActualHF, deltaEquivHF);
  SizetArray N_actual_shared_proj = N_actual_shared;
  increment_samples(N_actual_shared_proj, deltaNActualHF);
  // For pilot proj, see note re inaccurate estvar from overshooting hf_targets
  estvar_ratios_to_avg_estvar(estVarRatios, varH, N_actual_shared_proj,
			      avgEstVar);
}


void NonDControlVariateSampling::
evaluate_pilot(const Pecos::ActiveKey& active_key, Real& cost_ratio,
	       RealVector& eval_ratios, RealVector& var_H,
	       SizetArray& N_actual_shared, RealVector& hf_targets,
	       bool accumulate_cost, bool pilot_estvar)
{
  RealVector sum_L(numFunctions), sum_H(numFunctions), sum_LL(numFunctions),
    sum_LH(numFunctions), sum_HH(numFunctions), rho2_LH(numFunctions, false);
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  N_actual_shared.assign(numFunctions, 0);

  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, 2, delta_N_l); // 2 models only
  numSamples = std::min(delta_N_l[0], delta_N_l[1]);

  // -----------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,estvar}_ratios
  // -----------------------------------------------------------------------
  shared_increment(active_key, mlmfIter, 0);
  accumulate_mf_sums(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_actual_shared);
  if (onlineCost) recover_paired_online_cost(sequenceCost, 1);
  cost_ratio  = (onlineCost) ? sequenceCost[1] : sequenceCost[numSteps - 1];
  cost_ratio /= sequenceCost[0]; // HF / LF
  if (accumulate_cost)
    increment_mf_equivalent_cost(numSamples, numSamples, cost_ratio,
				 equivHFEvals);

  // Compute the LF/HF evaluation ratio using shared samples, averaged
  // over QoI.  This includes updating var_H and rho2_LH.
  compute_eval_ratios(sum_L, sum_H, sum_LL, sum_LH, sum_HH, cost_ratio,
		      N_actual_shared, var_H, rho2_LH, eval_ratios);
  // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
  // sum_L_refined / N_refined is zero for CV prior to sample refinement.
  // (This differs from MLMC estvar^0 which uses pilot for all levels.)
  if (pilot_estvar || !budget_constrained) {
    compute_mc_estimator_variance(var_H, N_actual_shared, estVarIter0);
    numHIter0 = N_actual_shared;
  }
  // Compute the ratio of MC and CVMC mean squared errors (for convergence).
  // This ratio incorporates the anticipated variance reduction from the
  // upcoming application of eval_ratios.
  compute_estvar_ratios(eval_ratios, rho2_LH, estVarRatios);

  // -----------------------------------------------------------------------
  // Compute shared increment targeting specified budget || estvar reduction
  // -----------------------------------------------------------------------
  if (budget_constrained) {
    Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
    allocate_budget(eval_ratios, cost_ratio, hf_targets);
  }
  else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: support both
    // N_H = estvar_ratio * var_H / convTol / estVarIter0
    // Note: don't simplify further since estVarIter0 is fixed based on pilot
    Cout << "Scaling profile for convergenceTol = " << convergenceTol;
    hf_targets = estVarRatios;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      hf_targets[qoi] *= var_H[qoi] / estVarIter0[qoi] / convergenceTol;
  }
  // numSamples is relative to N_H, but the approx_increments() below are
  // computed relative to hf_targets (independent of sunk cost for pilot)
  Cout << ": average HF target = " << average(hf_targets) << std::endl;
  //numSamples = one_sided_delta(N_hf, hf_targets, 1); // average
  ++mlmfIter;
}


void NonDControlVariateSampling::
hf_lf_indices(size_t& hf_form_index, size_t& hf_lev_index,
	      size_t& lf_form_index, size_t& lf_lev_index)
{
  if (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE) {// resolution hierarchy
    // traps for completeness (undefined model form should not occur)
    hf_form_index = lf_form_index
      = (secondaryIndex == SZ_MAX) ? NLevActual.size() - 1 : secondaryIndex;
    // extremes of range
    hf_lev_index = NLevActual[hf_form_index].size() - 1;  lf_lev_index = 0;
  }
  else { // model form hierarchy: extremes of range
    hf_form_index = NLevActual.size() - 1;  lf_form_index = 0;
    if (secondaryIndex == SZ_MAX) {
      size_t c_index = iteratedModel.truth_model().solution_level_cost_index();
      hf_lev_index = (c_index == SZ_MAX) ? 0 : c_index;
      c_index    = iteratedModel.surrogate_model().solution_level_cost_index();
      lf_lev_index = (c_index == SZ_MAX) ? 0 : c_index;
    }
    else
      hf_lev_index = lf_lev_index = secondaryIndex;
  }
}


void NonDControlVariateSampling::
initialize_mf_sums(IntRealVectorMap& sum_L_shared,
		   //IntRealVectorMap& sum_L_refined,
		   IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
		   IntRealVectorMap& sum_LH)//, IntRealVectorMap& sum_HH)
{
  // sum_* are running sums across all increments
  std::pair<int, RealVector> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i; // moment number
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size RealVector in place and init sums to 0
    sum_L_shared.insert(empty_pr).first->second.size(numFunctions);
  //sum_L_refined.insert(empty_pr).first->second.size(numFunctions);
    sum_H.insert(empty_pr).first->second.size(numFunctions);
    sum_LL.insert(empty_pr).first->second.size(numFunctions);
    sum_LH.insert(empty_pr).first->second.size(numFunctions);
  //sum_HH.insert(empty_pr).first->second.size(numFunctions);
  }
}


void NonDControlVariateSampling::
accumulate_mf_sums(IntRealVectorMap& sum_L, SizetArray& num_L)
{
  // uses one set of allResponses in BYPASS_SURROGATE mode
  // IntRealVectorMap is not a multilevel case --> no discrepancies

  using std::isfinite;
  Real fn_val, prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRVMIter l_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    //const Response& resp    = r_it->second;
    const RealVector& fn_vals = r_it->second.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      prod = fn_val = fn_vals[qoi];

      if (isfinite(fn_val)) { // neither NaN nor +/-Inf
	l_it = sum_L.begin(); ord = l_it->first; active_ord = 1;
	while (l_it!=sum_L.end()) {
    
	  if (ord == active_ord) {
	    l_it->second[qoi] += prod; ++l_it;
	    ord = (l_it == sum_L.end()) ? 0 : l_it->first;
	  }

	  prod *= fn_val; ++active_ord;
	}
	++num_L[qoi];
      }
    }
  }
}


void NonDControlVariateSampling::
accumulate_mf_sums(IntRealVectorMap& sum_L_shared,
		   //IntRealVectorMap& sum_L_refined,
		   IntRealVectorMap& sum_H,  IntRealVectorMap& sum_LL,
		   IntRealVectorMap& sum_LH, RealVector& sum_HH,
		   SizetArray& N_shared)
{
  // uses one set of allResponses in AGGREGATED_MODEL_PAIR mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter ls_it, h_it, ll_it, lh_it;//, lr_it
  int ls_ord, h_ord, ll_ord, lh_ord, active_ord;//, lr_ord
  size_t qoi;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
      lf_prod = lf_fn = fn_vals[qoi];
      hf_prod = hf_fn = fn_vals[qoi+numFunctions];

      // sync sample counts for all L and H interactions at this level
      if (isfinite(lf_fn) && isfinite(hf_fn)) { // neither NaN nor +/-Inf
	++N_shared[qoi];

	// High-High
	sum_HH[qoi] += hf_prod * hf_prod;

	ls_it = sum_L_shared.begin(); //lr_it = sum_L_refined.begin();
	h_it  = sum_H.begin(); ll_it = sum_LL.begin(); lh_it = sum_LH.begin();
	ls_ord = /*(ls_it == sum_L_shared.end())  ? 0 :*/ ls_it->first;
	//lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	h_ord  = /*(h_it  == sum_H.end())  ? 0 :*/  h_it->first;
	ll_ord = /*(ll_it == sum_LL.end()) ? 0 :*/ ll_it->first;
	lh_ord = /*(lh_it == sum_LH.end()) ? 0 :*/ lh_it->first;
	active_ord = 1;
	while (ls_it!=sum_L_shared.end() || //lr_it!=sum_L_refined.end() ||
	       h_it!=sum_H.end() || ll_it!=sum_LL.end() ||
	       lh_it!=sum_LH.end() || active_ord <= 1) {
    
	  // Low shared
	  if (ls_ord == active_ord) {
	    ls_it->second[qoi] += lf_prod;
	    ++ls_it; ls_ord = (ls_it == sum_L_shared.end())  ? 0 : ls_it->first;
	  }
	  // Low refined
	  //if (lr_ord == active_ord) {
	  //  lr_it->second[qoi] += lf_prod;
	  //  ++lr_it; lr_ord = (lr_it == sum_L_refined.end()) ? 0:lr_it->first;
	  //}
	  // High
	  if (h_ord == active_ord) {
	    h_it->second[qoi] += hf_prod;
	    ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  // Low-Low
	  if (ll_ord == active_ord) {
	    ll_it->second[qoi] += lf_prod * lf_prod;
	    ++ll_it; ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	  }
	  // Low-High
	  if (lh_ord == active_ord) {
	    lh_it->second[qoi] += lf_prod * hf_prod;
	    ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	  }

	  if (ls_ord || ll_ord || lh_ord) lf_prod *= lf_fn; //|| lr_ord 
	  if (h_ord  || lh_ord)           hf_prod *= hf_fn;
	  ++active_ord;
	}
      }
    }
  }
}


void NonDControlVariateSampling::
accumulate_mf_sums(RealVector& sum_L, RealVector& sum_H, RealVector& sum_LL,
		   RealVector& sum_LH, RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses in AGGREGATED_MODEL_PAIR mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using std::isfinite;
  Real lf_fn, hf_fn;  size_t qoi;  IntRespMCIter r_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
      lf_fn = fn_vals[qoi];
      hf_fn = fn_vals[qoi+numFunctions];

      // sync sample counts for all L and H interactions at this level
      if (isfinite(lf_fn) && isfinite(hf_fn)) { // neither NaN nor +/-Inf
	++N_shared[qoi];
	sum_H[qoi]  += hf_fn;         // High
	sum_HH[qoi] += hf_fn * hf_fn; // High-High
	sum_L[qoi]  += lf_fn;         // Low
	sum_LL[qoi] += lf_fn * lf_fn; // Low-Low
	sum_LH[qoi] += lf_fn * hf_fn; // Low-High
      }
    }
  }
}


void NonDControlVariateSampling::
shared_increment(const Pecos::ActiveKey& agg_key, size_t iter, size_t lev)
{
  if (iter == SZ_MAX) Cout << "\nCVMC sample increments: ";
  else if (iter == 0) Cout << "\nCVMC pilot sample: ";
  else Cout << "\nCVMC iteration " << iter << " sample increments: ";
  Cout << "LF = " << numSamples << " HF = " << numSamples << '\n';

  if (numSamples) {
    iteratedModel.surrogate_response_mode(AGGREGATED_MODEL_PAIR);
    iteratedModel.active_model_key(agg_key);
    resize_active_set();
 
    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);// pull dist params from any model

    // export separate output files for each data set:
    if (exportSampleSets) // for HF+LF models, use the HF tags
      export_all_samples("cv_", iteratedModel.truth_model(), iter, lev);

    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);
  }
}


bool NonDControlVariateSampling::
lf_increment(const Pecos::ActiveKey& lf_key, const RealVector& eval_ratios,
	     const SizetArray& N_lf, const RealVector& hf_targets,
	     RealVector& lf_targets, size_t iter, size_t lev)
{
  lf_allocate_samples(eval_ratios, N_lf, hf_targets, lf_targets);

  if (numSamples) {
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    iteratedModel.active_model_key(lf_key); // sets activeKey and surrModelKey
    resize_active_set();

    return lf_perform_samples(iter, lev);
  }
  else
    return false;
}


bool NonDControlVariateSampling::
lf_increment(const Pecos::ActiveKey& lf_key, const RealVector& eval_ratios,
	     size_t N_lf, const RealVector& hf_targets, RealVector& lf_targets,
	     size_t iter, size_t lev)
{
  SizetArray N_lf_sa;  N_lf_sa.assign(eval_ratios.length(), N_lf);
  lf_allocate_samples(eval_ratios, N_lf_sa, hf_targets, lf_targets);

  if (numSamples) {
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    iteratedModel.active_model_key(lf_key); // sets activeKey and surrModelKey
    resize_active_set();

    return lf_perform_samples(iter, lev);
  }
  else
    return false;
}


/** shared helper */
void NonDControlVariateSampling::
lf_allocate_samples(const RealVector& eval_ratios, const SizetArray& N_lf,
		    const RealVector& hf_targets, RealVector& lf_targets)
{
  // update LF samples based on evaluation ratio
  //   r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  //   or with inverse r  -> delta = m-n = n/inverse_r - n
  if (lf_targets.empty()) lf_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_targets[qoi];
  // Choose average, RMS, max of difference?
  // Trade-off: Possible overshoot vs. more iteration...
  numSamples = one_sided_delta(N_lf, lf_targets, 1); // average

  if (numSamples)
    Cout << "\nControl variate LF sample increment = " << numSamples;
  else Cout << "\nNo control variate LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from avg LF = " << average(N_lf) << ", avg HF targets = "
	 << average(hf_targets) << ", avg eval_ratio = "<< average(eval_ratios);
  Cout << std::endl;
}


/** shared helper */
void NonDControlVariateSampling::
lf_allocate_samples(const RealVector& eval_ratios, size_t N_lf,
		    Real hf_target, Real& lf_target)
{
  lf_target = 0.;
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_target += eval_ratios[qoi] * hf_target;
  lf_target /= numFunctions; // average
  numSamples = one_sided_delta((Real)N_lf, lf_target);

  if (numSamples)
    Cout << "\nControl variate LF sample increment = " << numSamples;
  else Cout << "\nNo control variate LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from LF = " << N_lf << ", HF target = " << hf_target
	 << ", avg eval_ratio = "<< average(eval_ratios);
  Cout << std::endl;
}


/** shared helper */
bool NonDControlVariateSampling::lf_perform_samples(size_t iter, size_t lev)
{
  // ----------------------------------------
  // Compute LF increment for control variate
  // ----------------------------------------

  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model
  // export separate output files for each data set:
  if (exportSampleSets)
    export_all_samples("cv_", iteratedModel.active_surrogate_model(0),iter,lev);

  // Iteration 0 is defined as the pilot sample + initial CV increment, and
  // each subsequent iter can be defined as a pair of ML + CV increments.  If
  // it is desired to stop between the ML and CV components, finalCVRefinement
  // can be hardwired to false (not currently part of input spec).
  // Note: termination based on delta_N_hf=0 has final ML and CV increments
  //       of zero, which is consistent with finalCVRefinement=true.
  //if (iter < maxIterations || finalCVRefinement) {
    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);
    return true;
  //}
  //else return false;
}


void NonDControlVariateSampling::
allocate_budget(const RealVector& eval_ratios, Real cost_ratio,
		RealVector& hf_targets)
{
  // Scale this profile based on specified budget (maxFunctionEvals) if needed
  // using N_H = maxFunctionEvals / cost^T eval_ratios
  // > Pilot case iter = 0: can only scale back after shared_increment().
  //   Optimal profile can be hidden by one_sided_delta() with pilot --> optimal
  //   shape emerges from initialization cost as for ML cases controlled by
  //   convTol (allow budget overshoot due to overlap of optimal with pilot,
  //   rather than strictly allocating remaining budget)

  if (hf_targets.empty()) hf_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    hf_targets[qoi] = (Real)maxFunctionEvals
                    / (1. + eval_ratios[qoi] / cost_ratio);
}


void NonDControlVariateSampling::
compute_eval_ratios(const RealVector& sum_L_shared, const RealVector& sum_H,
		    const RealVector& sum_LL, const RealVector& sum_LH,
		    const RealVector& sum_HH, Real cost_ratio,
		    const SizetArray& N_shared, RealVector& var_H,
		    RealVector& rho2_LH, RealVector& eval_ratios)
{
  if (eval_ratios.empty()) eval_ratios.sizeUninitialized(numFunctions);
  if (rho2_LH.empty())         rho2_LH.sizeUninitialized(numFunctions);
  if (var_H.empty())             var_H.sizeUninitialized(numFunctions);

  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH[qoi];
    compute_mf_correlation(sum_L_shared[qoi], sum_H[qoi], sum_LL[qoi],
			   sum_LH[qoi], sum_HH[qoi], N_shared[qoi],
			   var_H[qoi], rho_sq);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
	   << std::setw(9) << std::sqrt(rho_sq) << '\n';

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    // Given use of 1/r in estvar_ratio, one approach would average 1/r, but
    // this does not seem to behave as well in limited numerical experience.
    //if (rho_sq > Pecos::SMALL_NUMBER_SQ) {
    //  avg_inv_eval_ratio += std::sqrt((1. - rho_sq)/(cost_ratio * rho_sq));
    eval_ratios[qoi] = (rho_sq < 1.) ? // prevent division by 0, sqrt(negative)
      std::sqrt(cost_ratio * rho_sq / (1. - rho_sq)) :
      std::sqrt(cost_ratio / Pecos::SMALL_NUMBER); // should not happen
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "evaluation_ratios() QoI " << qoi+1 << ": cost_ratio = "
	   << cost_ratio << " rho_sq = " << rho_sq << " eval_ratio = "
	   << eval_ratios[qoi] << std::endl;
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "variance of HF Q:\n" << var_H;
}


void NonDControlVariateSampling::
compute_estvar_ratios(const RealVector& eval_ratios,
		    //const RealVector& var_H,
		      const RealVector& rho2_LH,
		    //size_t iter, const SizetArray& N_hf,
		      RealVector& estvar_ratios)
{
  //Real curr_mc_estvar,curr_cvmc_estvar,curr_estvar_ratio,avg_estvar_ratio=0.;
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    // Compute ratio of estvar for high fidelity MC and multifidelity CVMC
    // > Estimator Var for MC = sigma_hf^2 / N_hf
    // > Estimator Var for CV = (1+r/w) [1-rho^2(1-1/r)] sigma_hf^2 / p
    //   where p = (1+r/w)N_hf -> Var = [1-rho^2(1-1/r)] sigma_hf^2 / N_hf
    // estvar ratio = Var_CV / Var_MC = [1-rho^2(1-1/r)]
    estvar_ratios[qoi]
      = 1. - rho2_LH[qoi] * (1. - 1. / eval_ratios[qoi]); // Ng 2014
    Cout << "QoI " << qoi+1 << ": CV variance reduction factor = "
	 << estvar_ratios[qoi] << " for eval ratio " << eval_ratios[qoi] <<'\n';
    //curr_mc_estvar   = var_H[qoi] / N_hf[qoi];
    //curr_cvmc_estvar = curr_mc_estvar * curr_estvar_ratio;
    //Cout << "QoI " << qoi+1 << ": Mean square error estimated to reduce from "
    //     << curr_mc_estvar << " (MC) to " << curr_cvmc_estvar
    //     << " (CV) following upcoming LF increment\n";

    //if (iter == 0) { // initialize reference based on pilot
    //  mcEstVarIter0[qoi] = curr_mc_estvar;
    //  avg_estvar_ratio += curr_estvar_ratio;
    //}
    //else           // measure convergence to target based on pilot reference
    //  avg_estvar_ratio += curr_cvmc_estvar / mcEstVarIter0[qoi];
  }

  //avg_estvar_ratio /= numFunctions;
  //Cout << "Average reduction factor since pilot MC = " << avg_estvar_ratio
  //     << " targeting convergence tol = " << convergenceTol << '\n';
  //return avg_estvar_ratio;
}


void NonDControlVariateSampling::
cv_raw_moments(IntRealVectorMap& sum_L_shared, IntRealVectorMap& sum_H,
	       IntRealVectorMap& sum_LL,       IntRealVectorMap& sum_LH,
	       const SizetArray& N_shared,     IntRealVectorMap& sum_L_refined,
	       const SizetArray& N_refined,  //const RealVector& rho2_LH,
	       RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);
  RealVector beta(numFunctions, false);

  // rho2_LH not stored for i > 1
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
  //       << std::setw(9) << std::sqrt(rho2_LH[qoi]) << '\n';
  //<< ", effectiveness ratio = " << std::setw(9) << rho2_LH[qoi] * cr1;

  for (int i=1; i<=4; ++i) {
    compute_mf_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i],
		       N_shared, beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_mf_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		     N_refined, beta, H_rm_col);
  }
}


void NonDControlVariateSampling::
update_projected_lf_samples(const RealVector& hf_targets,
			    const RealVector& eval_ratios, Real cost_ratio,
			    const SizetArray& N_L_actual, size_t& N_L_alloc,
			    /*size_t& delta_N_L_actual,*/ Real& delta_equiv_hf)
{
  RealVector lf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_targets[qoi];

  size_t lf_actual_incr,
    lf_alloc_incr = one_sided_delta(N_L_alloc, average(lf_targets));
  // essentially assigns targets to allocations:
  N_L_alloc += lf_alloc_incr;

  lf_actual_incr = (backfillFailures) ?
    one_sided_delta(N_L_actual, lf_targets, 1) : lf_alloc_incr;
  //delta_N_L_actual += lf_actual_incr;
  increment_mf_equivalent_cost(lf_actual_incr, cost_ratio, delta_equiv_hf);
}


void NonDControlVariateSampling::
update_projected_samples(const RealVector& hf_targets,
			 const RealVector& eval_ratios, Real cost_ratio,
			 const SizetArray& N_H_actual, size_t& N_H_alloc,
			 const SizetArray& N_L_actual, size_t& N_L_alloc,
			 size_t& delta_N_H_actual, //size_t& delta_N_L_actual,
			 Real& delta_equiv_hf)
{
  size_t hf_actual_incr,
    hf_alloc_incr = one_sided_delta(N_H_alloc, average(hf_targets));
  // essentially assigns targets to allocations:
  N_H_alloc += hf_alloc_incr;

  hf_actual_incr = (backfillFailures) ?
    one_sided_delta(N_H_actual, hf_targets, 1) : hf_alloc_incr;
  delta_N_H_actual += hf_actual_incr;
  delta_equiv_hf   += hf_actual_incr;

  update_projected_lf_samples(hf_targets, eval_ratios, cost_ratio,
			      N_L_actual, N_L_alloc, //delta_N_L_actual,
			      delta_equiv_hf);
}


void NonDControlVariateSampling::print_variance_reduction(std::ostream& s)
{
  size_t hf_form_index, hf_lev_index, lf_form_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_hf = NLevActual[hf_form_index][hf_lev_index];

  RealVector mc_est_var(numFunctions, false);
  Real proj_equiv_hf = equivHFEvals + deltaEquivHF;
  // est_var is projected for cases that are not fully iterated/incremented
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    mc_est_var[qoi] = varH[qoi] / (N_hf[qoi] + deltaNActualHF);
  Real     avg_mc_est_var = average(mc_est_var),
    avg_budget_mc_est_var = average(varH) / proj_equiv_hf;

  String type = (pilotMgmtMode == PILOT_PROJECTION) ? "Projected" : "   Online";
  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:\n";

  if (pilotMgmtMode != OFFLINE_PILOT)
    s << "      Initial MC (" << std::setw(5)
      << (size_t)std::floor(average(numHIter0) + .5) << " HF samples): "
      << std::setw(wpp7) << average(estVarIter0) << '\n';

  s << "  " << type << "   MC (" << std::setw(5)
    << (size_t)std::floor(average(N_hf) + deltaNActualHF + .5)
    << " HF samples): " << std::setw(wpp7) << avg_mc_est_var
    << "\n  " << type << " CVMC (sample profile):   "
    << std::setw(wpp7) << avgEstVar
    << "\n  " << type << " CVMC ratio (1 - R^2):    "
    << std::setw(wpp7) << avgEstVar / avg_mc_est_var
    << "\n Equivalent   MC (" << std::setw(5)
    << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
    << std::setw(wpp7) << avg_budget_mc_est_var
    << "\n Equivalent CVMC ratio:              " << std::setw(wpp7)
    << avgEstVar / avg_budget_mc_est_var << '\n';
}

} // namespace Dakota
