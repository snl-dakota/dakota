/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
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
  // For now...
  size_t num_mf = NLev.size();
  if (num_mf > 2)
    Cerr << "Warning: NonDControlVariateSampling currently uses first and last "
	 << "model in ordered sequence and ignores the rest." << std::endl;
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDControlVariateSampling::core_run()
{
  iteratedModel.multifidelity_precedence(true); // prefer MF to ML if both avail
  configure_sequence(numSteps, secondaryIndex, sequenceType);

  // For two-model control variate, select extreme fidelities/resolutions
  Pecos::ActiveKey active_key, hf_key, lf_key;
  unsigned short hf_form, lf_form;  size_t hf_lev, lf_lev;
  if (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE) {
    hf_lev  = numSteps - 1;  lf_lev = 0;  // extremes of range
    hf_form = lf_form = (secondaryIndex == SZ_MAX) ? USHRT_MAX : secondaryIndex;
  }
  else {
    hf_form = numSteps - 1;  lf_form = 0; // extremes of range
    hf_lev = lf_lev = secondaryIndex;
  }
  hf_key.form_key(0, hf_form, hf_lev);  lf_key.form_key(0, lf_form, lf_lev);
  active_key.aggregate_keys(hf_key, lf_key, Pecos::RAW_DATA);

  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0

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
  SizetArray& N_hf = NLev[hf_form_index][hf_lev_index];
  SizetArray& N_lf = NLev[lf_form_index][lf_lev_index];
  N_hf.assign(numFunctions, 0);  N_lf.assign(numFunctions, 0);

  Real lf_cost, hf_cost, cost_ratio;
  initialize_mf_cost(lf_cost, hf_cost, cost_ratio);

  IntRealVectorMap sum_L_shared, sum_H, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH);
  RealVector eval_ratios, mu_hat, hf_targets, sum_HH(numFunctions),
    rho2_LH(numFunctions, false);
  varH.sizeUninitialized(numFunctions);

  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, 2, delta_N_l); // 2 models only
  numSamples = std::min(delta_N_l[0], delta_N_l[1]);

  while (numSamples && mlmfIter <= maxIterations) {

    // -----------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,estvar}_ratios
    // -----------------------------------------------------------------------
    shared_increment(active_key, mlmfIter, 0);
    accumulate_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH, N_hf);
    increment_mf_equivalent_cost(numSamples, numSamples, cost_ratio);

    // Compute the LF/HF evaluation ratio using shared samples, averaged
    // over QoI.  This includes updating varH and rho2_LH.
    compute_eval_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1], sum_HH,
			cost_ratio, N_hf, varH, rho2_LH, eval_ratios);
    // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
    // sum_L_refined / N_refined is zero for CV prior to sample refinement.
    // (This differs from MLMC estvar^0 which uses pilot for all levels.)
    if (mlmfIter == 0)
      { compute_mc_estimator_variance(varH,N_hf,estVarIter0); numHIter0 = N_hf;}
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
      // N_hf = estvar_ratio * varH / convTol / estVarIter0
      // Note: don't simplify further since estVarIter0 is fixed based on pilot
      Cout << "Scaling profile for convergenceTol = " << convergenceTol;
      hf_targets = estVarRatios;
      for (size_t qoi=0; qoi<numFunctions; ++qoi)
	hf_targets[qoi] *= varH[qoi] / estVarIter0[qoi] / convergenceTol;
    }
    // numSamples is relative to N_H, but the approx_increments() below are
    // computed relative to hf_targets (independent of sunk cost for pilot)
    Cout << ": average HF target = " << average(hf_targets) << std::endl;
    numSamples = one_sided_delta(N_hf, hf_targets, 1); // average
    //numSamples = std::min(num_samp_budget, num_samp_ctol);

    //Cout << "\nCVMC iteration " << mlmfIter << " complete." << std::endl;
    ++mlmfIter;
  } // end while

  // -------------------------------------------------------------------
  // Compute new LF increment based on new evaluation ratio for new N_hf
  // -------------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after N_hf has converged, which simplifies maxFnEvals / convTol logic
  // (no need to further interrogate these throttles below)
  IntRealVectorMap sum_L_refined = sum_L_shared;
  N_lf = N_hf; // shared to this point, but only N_hf has been updated
  Pecos::ActiveKey lf_key;  active_key.extract_key(1, lf_key);
  if (lf_increment(lf_key, eval_ratios, N_lf, hf_targets, mlmfIter, 0)) {
    accumulate_mf_sums(sum_L_refined, N_lf);
    increment_mf_equivalent_cost(numSamples, cost_ratio);
  }

  // Compute/apply control variate params to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf, sum_L_refined,
		 N_lf, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDControlVariateSampling::
control_variate_mc_offline_pilot(const Pecos::ActiveKey& active_key)
{
  size_t hf_form_index, lf_form_index, hf_lev_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_hf = NLev[hf_form_index][hf_lev_index];
  SizetArray& N_lf = NLev[lf_form_index][lf_lev_index];
  N_hf.assign(numFunctions, 0);  N_lf.assign(numFunctions, 0);

  Real lf_cost, hf_cost, cost_ratio;
  initialize_mf_cost(lf_cost, hf_cost, cost_ratio);

  // ---------------------------------------------------------------------
  // Compute final rho2LH, varH, {eval,estvar} ratios from (oracle) pilot
  // treated as "offline" cost
  // ---------------------------------------------------------------------
  RealVector eval_ratios, hf_targets;  SizetArray N_shared;
  evaluate_pilot(active_key, cost_ratio, eval_ratios, varH, N_shared,
		 hf_targets, false, false); // no cost, estvar

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealVectorMap sum_L_shared, sum_H, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions);

  numSamples = one_sided_delta(N_hf, hf_targets, 1); // online N_hf is zero
  shared_increment(active_key, mlmfIter, 0);
  accumulate_mf_sums(sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH, N_hf);
  increment_mf_equivalent_cost(numSamples, numSamples, cost_ratio);

  IntRealVectorMap sum_L_refined = sum_L_shared;
  N_lf = N_hf; // shared to this point, but only N_hf has been updated
  Pecos::ActiveKey lf_key;  active_key.extract_key(1, lf_key);
  if (lf_increment(lf_key, eval_ratios, N_lf, hf_targets, mlmfIter, 0)) {
    accumulate_mf_sums(sum_L_refined, N_lf);
    increment_mf_equivalent_cost(numSamples, cost_ratio);
  }

  // Compute/apply control variate params to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf, sum_L_refined,
		 N_lf, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDControlVariateSampling::
control_variate_mc_pilot_projection(const Pecos::ActiveKey& active_key)
{
  size_t hf_form_index, lf_form_index, hf_lev_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_hf = NLev[hf_form_index][hf_lev_index];
  SizetArray& N_lf = NLev[lf_form_index][lf_lev_index];
  N_hf.assign(numFunctions, 0);  N_lf.assign(numFunctions, 0);

  Real lf_cost, hf_cost, cost_ratio;
  initialize_mf_cost(lf_cost, hf_cost, cost_ratio);

  RealVector eval_ratios, hf_targets;
  evaluate_pilot(active_key, cost_ratio, eval_ratios, varH, N_hf, hf_targets,
		 true, true); // accumulate cost, compute estvar0

  N_lf = N_hf; // shared to this point, but only N_hf has been updated
  update_projected_samples(hf_targets, eval_ratios, cost_ratio, N_hf, N_lf);
}


void NonDControlVariateSampling::
evaluate_pilot(const Pecos::ActiveKey& active_key, Real cost_ratio,
	       RealVector& eval_ratios, RealVector& var_H, SizetArray& N_shared,
	       RealVector& hf_targets, bool accumulate_cost, bool pilot_estvar)
{
  RealVector sum_L(numFunctions), sum_H(numFunctions), sum_LL(numFunctions),
    sum_LH(numFunctions), sum_HH(numFunctions), rho2_LH(numFunctions, false);
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  N_shared.assign(numFunctions, 0);

  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, 2, delta_N_l); // 2 models only
  numSamples = std::min(delta_N_l[0], delta_N_l[1]);

  // -----------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,estvar}_ratios
  // -----------------------------------------------------------------------
  shared_increment(active_key, mlmfIter, 0);
  accumulate_mf_sums(sum_L, sum_H, sum_LL, sum_LH, sum_HH, N_shared);
  if (accumulate_cost)
    increment_mf_equivalent_cost(numSamples, numSamples, cost_ratio);

  // Compute the LF/HF evaluation ratio using shared samples, averaged
  // over QoI.  This includes updating var_H and rho2_LH.
  compute_eval_ratios(sum_L, sum_H, sum_LL, sum_LH, sum_HH, cost_ratio,
		      N_shared, var_H, rho2_LH, eval_ratios);
  // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
  // sum_L_refined / N_refined is zero for CV prior to sample refinement.
  // (This differs from MLMC estvar^0 which uses pilot for all levels.)
  if (pilot_estvar || !budget_constrained) {
    compute_mc_estimator_variance(var_H, N_shared, estVarIter0);
    numHIter0 = N_shared;
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
    // N_hf = estvar_ratio * var_H / convTol / estVarIter0
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
}


void NonDControlVariateSampling::
hf_lf_indices(size_t& hf_form_index, size_t& hf_lev_index,
	      size_t& lf_form_index, size_t& lf_lev_index)
{
  if (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE) {// resolution hierarchy
    // traps for completeness (undefined model form should not occur)
    hf_form_index = lf_form_index
      = (secondaryIndex == SZ_MAX) ? 0 : secondaryIndex;
    // extremes of range
    hf_lev_index = NLev[hf_form_index].size() - 1;  lf_lev_index = 0;
  }
  else { // model form hierarchy
    hf_form_index = NLev.size() - 1;  lf_form_index = 0; // extremes of range
    size_t raw_index = iteratedModel.truth_model().solution_level_cost_index();
    hf_lev_index = (raw_index == SZ_MAX) ? 0 : raw_index;
    raw_index    = iteratedModel.surrogate_model().solution_level_cost_index();
    lf_lev_index = (raw_index == SZ_MAX) ? 0 : raw_index;
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
  // uses one set of allResponses in AGGREGATED_MODELS mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter ls_it, h_it, ll_it, lh_it;//, lr_it
  int ls_ord, h_ord, ll_ord, lh_ord, active_ord;//, lr_ord
  size_t qoi;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // response mode AGGREGATED_MODELS orders HF (active model key)
      // followed by LF (previous/decremented model key)
      hf_prod = hf_fn = fn_vals[qoi];
      lf_prod = lf_fn = fn_vals[qoi+numFunctions];

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
  // uses one set of allResponses in AGGREGATED_MODELS mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using std::isfinite;
  Real lf_fn, hf_fn;  size_t qoi;  IntRespMCIter r_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // response mode AGGREGATED_MODELS orders HF (active model key)
      // followed by LF (previous/decremented model key)
      hf_fn = fn_vals[qoi];
      lf_fn = fn_vals[qoi+numFunctions];

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
  if (iter == _NPOS)  Cout << "\nCVMC sample increments: ";
  else if (iter == 0) Cout << "\nCVMC pilot sample: ";
  else Cout << "\nCVMC iteration " << iter << " sample increments: ";
  Cout << "LF = " << numSamples << " HF = " << numSamples << '\n';

  if (numSamples) {
    aggregated_models_mode();
    iteratedModel.active_model_key(agg_key);

    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);// pull dist params from any model

    // export separate output files for each data set:
    if (exportSampleSets) // for HF+LF models, use the HF tags
      export_all_samples("cv_", iteratedModel.truth_model(), iter, lev);

    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);
  }
}


/** version with LF key */
bool NonDControlVariateSampling::
lf_increment(const Pecos::ActiveKey& lf_key, const RealVector& eval_ratios,
	     const SizetArray& N_lf, const RealVector& hf_targets,
	     size_t iter, size_t lev)
{
  // update LF samples based on evaluation ratio
  //   r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  //   or with inverse r  -> delta = m-n = n/inverse_r - n
  RealVector lf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_targets[qoi];
  // Choose average, RMS, max of difference?
  // Trade-off: Possible overshoot vs. more iteration...
  numSamples = one_sided_delta(N_lf, lf_targets, 1); // average

  if (numSamples) Cout << "\nCVMC LF sample increment = " << numSamples;
  else            Cout << "\nNo CVMC LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from avg LF = " << average(N_lf) << ", avg HF targets = "
	 << average(hf_targets) << ", avg eval_ratio = "<< average(eval_ratios);
  Cout << std::endl;

  if (numSamples) {
    uncorrected_surrogate_mode(); // also needed for lf_key assignment below
    iteratedModel.active_model_key(lf_key); // sets activeKey and surrModelKey

    return lf_increment(iter, lev);
  }
  else
    return false;
}


/** version without LF key */
bool NonDControlVariateSampling::
lf_increment(const RealVector& eval_ratios, const SizetArray& N_lf,
	     Real hf_target, size_t iter, size_t lev)
{
  // NonDMLCVSampling applies eval_ratio to hf_target as allocated by ML portion

  // update LF samples based on evaluation ratio
  //   r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  //   or with inverse r  -> delta = m-n = n/inverse_r - n
  RealVector lf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_target;
  // Choose average, RMS, max of difference?
  // Trade-off: Possible overshoot vs. more iteration...
  numSamples = one_sided_delta(N_lf, lf_targets, 1); // average

  if (numSamples) Cout << "\nCVMC LF sample increment = " << numSamples;
  else            Cout << "\nNo CVMC LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from avg LF = " << average(N_lf) << ", HF target = " << hf_target
	 << ", avg eval_ratio = "<< average(eval_ratios);
  Cout << std::endl;

  return (numSamples) ? lf_increment(iter, lev) : false;
}


/** shared helper */
bool NonDControlVariateSampling::lf_increment(size_t iter, size_t lev)
{
  // ----------------------------------------
  // Compute LF increment for control variate
  // ----------------------------------------

  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model
  // export separate output files for each data set:
  if (exportSampleSets)
    export_all_samples("cv_", iteratedModel.surrogate_model(), iter, lev);

  // Iteration 0 is defined as the pilot sample + initial CV increment, and
  // each subsequent iter can be defined as a pair of ML + CV increments.  If
  // it is desired to stop between the ML and CV components, finalCVRefinement
  // can be hardwired to false (not currently part of input spec).
  // Note: termination based on delta_N_hf=0 has final ML and CV increments
  //       of zero, which is consistent with finalCVRefinement=true.
  if (iter < maxIterations) { //|| finalCVRefinement) {
    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);
    return true;
  }
  else
    return false;
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

  //Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH[qoi];
    compute_mf_correlation(sum_L_shared[qoi], sum_H[qoi], sum_LL[qoi],
			   sum_LH[qoi], sum_HH[qoi], N_shared[qoi],
			   var_H[qoi], rho_sq);

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    // Given use of 1/r in estvar_ratio, one approach would average 1/r, but
    // this does not seem to behave as well in limited numerical experience.
    //if (rho_sq > Pecos::SMALL_NUMBER) {
    //  avg_inv_eval_ratio += std::sqrt((1. - rho_sq)/(cost_ratio * rho_sq));
    if (rho_sq < 1.) { // protect against division by 0, sqrt(negative)
      eval_ratios[qoi] = std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "evaluation_ratios() QoI " << qoi+1 << ": cost_ratio = "
	     << cost_ratio << " rho_sq = " << rho_sq << " eval_ratio = "
	     << eval_ratios[qoi] << std::endl;
      //avg_eval_ratio += eval_ratios[qoi];
      //++num_avg;
    }
    else // should not happen, but provide a reasonable upper bound
      eval_ratios[qoi] = (Real)maxFunctionEvals / average(N_shared);

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
	   << std::setw(9) << std::sqrt(rho_sq) << '\n';
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "variance of HF Q:\n" << var_H;

  //if (num_avg) avg_eval_ratio /= num_avg;
  //else         avg_eval_ratio  = (Real)maxFunctionEvals / average(N_shared);
  //return avg_eval_ratio;
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
update_projected_samples(const RealVector& hf_targets,
			 const RealVector& eval_ratios, Real cost_ratio,
			 SizetArray& N_hf, SizetArray& N_lf)
{
  RealVector lf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_targets[qoi];

  size_t hf_incr = one_sided_delta(N_hf, hf_targets, 1),
         lf_incr = one_sided_delta(N_lf, lf_targets, 1);
  increment_samples(N_hf, hf_incr);
  increment_samples(N_lf, lf_incr);
  increment_mf_equivalent_cost(hf_incr, lf_incr, cost_ratio);
}


void NonDControlVariateSampling::print_variance_reduction(std::ostream& s)
{
  size_t hf_form_index, hf_lev_index, lf_form_index, lf_lev_index;
  hf_lf_indices(hf_form_index, hf_lev_index, lf_form_index, lf_lev_index);
  SizetArray& N_hf = NLev[hf_form_index][hf_lev_index];

  RealVector mc_est_var(numFunctions, false),
           cvmc_est_var(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    cvmc_est_var[qoi]  = mc_est_var[qoi] = varH[qoi] / N_hf[qoi];
    cvmc_est_var[qoi] *= estVarRatios[qoi];
  }
  Real avg_cvmc_est_var      = average(cvmc_est_var),
       avg_mc_est_var        = average(mc_est_var),
       avg_budget_mc_est_var = average(varH) / equivHFEvals;

  String type = (pilotMgmtMode == PILOT_PROJECTION) ? "Projected":"    Final";
  s << "<<<<< Variance for mean estimator:\n";
  size_t wpp7 = write_precision + 7;

  if (pilotMgmtMode != OFFLINE_PILOT)
    s << "      Initial MC (" << std::setw(5)
      << (size_t)std::floor(average(numHIter0) + .5) << " HF samples): "
      << std::setw(wpp7) << average(estVarIter0);

  s << "\n  " << type << "   MC (" << std::setw(5)
    << (size_t)std::floor(average(N_hf) + .5) << " HF samples): "
    << std::setw(wpp7) << avg_mc_est_var
    << "\n  " << type << " CVMC (sample profile):   "
    << std::setw(wpp7) << avg_cvmc_est_var
    << "\n  " << type << " CVMC ratio (1 - R^2):    "
    << std::setw(wpp7) << avg_cvmc_est_var / avg_mc_est_var
      << "\n Equivalent   MC (" << std::setw(5)
    << (size_t)std::floor(equivHFEvals + .5) << " HF samples): "
    << std::setw(wpp7) << avg_budget_mc_est_var
    << "\n Equivalent CVMC ratio:              " << std::setw(wpp7)
    << avg_cvmc_est_var / avg_budget_mc_est_var << '\n';
}

} // namespace Dakota
