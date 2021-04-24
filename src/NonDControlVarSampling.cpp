/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDControlVarSampling
//- Description: Implementation code for NonDControlVarSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDControlVarSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDControlVarSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDControlVarSampling::
NonDControlVarSampling(ProblemDescDB& problem_db, Model& model):
  NonDHierarchSampling(problem_db, model)
{
  // initialize scalars from sequence
  seedSpec = randomSeed = random_seed(0);

  // Support multilevel LHS as a specification override.  The estimator variance
  // is known/correct for MC and an assumption/approximation for LHS.  To get an
  // accurate LHS estimator variance, one would need:
  // (a) assumptions about separability -> analytic variance reduction by a
  //     constant factor
  // (b) similarly, assumptions about the form relative to MC (e.g., a constant
  //     factor largely cancels out within the relative sample allocation.)
  // (c) numerically-generated estimator variance (from, e.g., replicated LHS)
  if (!sampleType) // SUBMETHOD_DEFAULT
    sampleType = SUBMETHOD_RANDOM;

  // check iteratedModel for model form hierarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  if (iteratedModel.surrogate_type() == "hierarchical")
    aggregated_models_mode();
  else {
    Cerr << "Error: ControlVar Monte Carlo requires a hierarchical "
	 << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& ordered_models = iteratedModel.subordinate_models(false);
  size_t i, j, num_mf = ordered_models.size(), num_lev,
    prev_lev = std::numeric_limits<size_t>::max(),
    pilot_size = pilotSamples.size();
  ModelLRevIter ml_rit; bool err_flag = false;
  NLev.resize(num_mf);
  for (i=num_mf-1, ml_rit=ordered_models.rbegin();
       ml_rit!=ordered_models.rend(); --i, ++ml_rit) { // high fid to low fid
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev = ml_rit->solution_levels(); // lower bound is 1 soln level

    if (num_lev > prev_lev) {
      Cerr << "\nWarning: unused solution levels in multilevel sampling for "
	   << "model " << ml_rit->model_id() << ".\n         Ignoring "
	   << num_lev - prev_lev << " of " << num_lev << " levels."<< std::endl;
      num_lev = prev_lev;
    }

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    if (num_lev > ml_rit->solution_levels(false)) { // default is 0 soln costs
      Cerr << "Error: insufficient cost data provided for multilevel sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_rit->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer to pre_run()

    prev_lev = num_lev;
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  if( !std::all_of( std::begin(pilotSamples), std::end(pilotSamples), [](int i){ return i > 0; }) ){
    Cerr << "\nError: Some levels have pilot samples of size 0 in "
       << method_enum_to_string(methodName) << "." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  switch (pilot_size) {
    case 0: maxEvalConcurrency *= 100;             break;
    case 1: maxEvalConcurrency *= pilotSamples[0]; break;
    default: {
      size_t max_ps = 0;
      for (i=0; i<pilot_size; ++i)
        if (pilotSamples[i] > max_ps)
  	max_ps = pilotSamples[i];
      if (max_ps)
        maxEvalConcurrency *= max_ps;
      break;
    }
  }

}


NonDControlVarSampling::~NonDControlVarSampling()
{ }


void NonDControlVarSampling::pre_run()
{
  NonDSampling::pre_run();

  // reset sample counters to 0
  size_t i, j, num_mf = NLev.size(), num_lev;
  for (i=0; i<num_mf; ++i) {
    Sizet2DArray& Nl_i = NLev[i];
    num_lev = Nl_i.size();
    for (j=0; j<num_lev; ++j)
      Nl_i[j].assign(numFunctions, 0);
  }
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDControlVarSampling::core_run()
{
  //model,
  //  surrogate hierarchical
  //    ordered_model_fidelities = 'LF' 'MF' 'HF'
  //
  // Future: include peer alternatives (1D list --> matrix)
  //         For MLMC, could seek adaptive selection of most correlated
  //         alternative (or a convex combination of alternatives).

  // TO DO: hierarchy incl peers (not peers each optionally incl hierarchy)
  //   num_mf     = iteratedModel.model_hierarchy_depth();
  //   num_peer_i = iteratedModel.model_peer_breadth(i);

  // TO DO: this initial logic is limiting:
  // > allow MLMC and CVMC for either model forms or discretization levels
  // > separate method specs that both map to NonDControlVarSampling ???

  // TO DO: following pilot sample across levels and fidelities in mixed case,
  // could pair models for CVMC based on estimation of rho2_LH.

  // For two-model control variate methods, select lowest,highest fidelities
  size_t num_mf = NLev.size();
  unsigned short lf_form = 0, hf_form = num_mf - 1; // ordered_models = low:high
  size_t num_hf_lev = NLev.back().size();
  if (num_hf_lev > 1) { // ML performed on HF with CV using available LF
    // multiple model forms + multiple solutions levels
      
  }
  else { // multiple model forms (only) --> CVMC
    // use nominal value from user input, ignoring solution_level_control
    Pecos::ActiveKey hf_lf_key;
    size_t lev = std::numeric_limits<size_t>::max();
    hf_lf_key.form_key(0, hf_form, lev, lf_form, lev, Pecos::RAW_DATA);
    control_variate_mc(hf_lf_key);
  }
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDControlVarSampling::
control_variate_mc(const Pecos::ActiveKey& active_key)
{
  // Current implementation performs pilot + shared increment + LF increment,
  // where these increments are targeting a prescribed MSE reduction.
  // **********
  // *** TO DO: should CV MC iterate (new shared + LF increments)
  // ***        until MSE target is met?
  // **********

  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  // retrieve active index
  //size_t lf_lev_index =  surr_model.solution_level_cost_index(),
  //       hf_lev_index = truth_model.solution_level_cost_index();
  // retrieve cost estimates across model forms for a particular soln level
  Real lf_cost =  surr_model.solution_level_cost(),
       hf_cost = truth_model.solution_level_cost(),
    cost_ratio = hf_cost / lf_cost, avg_eval_ratio, avg_mse_ratio;

  IntRealVectorMap sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH;
  initialize_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false),
            rho2_LH(numFunctions, false);

  // Initialize for pilot sample
  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, NLev, delta_N_l);

  // NLev allocations currently enforce truncation to #HF levels (1)
  Pecos::ActiveKey hf_key, lf_key;
  active_key.extract_keys(hf_key, lf_key);
  unsigned short hf_form = hf_key.retrieve_model_form(),
                 lf_form = lf_key.retrieve_model_form();
  SizetArray& N_lf = NLev[lf_form][0];//[lf_lev_index];
  SizetArray& N_hf = NLev[hf_form][0];//[hf_lev_index];
  size_t raw_N_lf = 0, raw_N_hf = 0;
  RealVector mu_hat;

  // ---------------------
  // Compute Pilot Samples
  // ---------------------

  mlmfIter = 0;

  // Initialize for pilot sample (shared sample count discarding any excess)
  numSamples = std::min(delta_N_l[lf_form], delta_N_l[hf_form]);
  shared_increment(mlmfIter, 0);
  accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		     sum_HH, mu_hat, N_lf, N_hf);
  raw_N_lf += numSamples; raw_N_hf += numSamples;

  // Compute the LF/HF evaluation ratio, averaged over the QoI.
  // This includes updating var_H and rho2_LH.
  avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			      sum_HH, cost_ratio, N_hf, var_H, rho2_LH);
  // compute the ratio of MC and CVMC mean squared errors (controls convergence)
  avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, mlmfIter, N_hf);

  // ----------------------------------------------------------
  // Compute shared increment targeting specified MSE reduction
  // ----------------------------------------------------------

  // bypass refinement if maxIterations == 0 or convergenceTol already
  // satisfied by pilot sample
  if (maxIterations && avg_mse_ratio > convergenceTol) {

    // Assuming rho_AB, evaluation_ratio and var_H to be relatively invariant,
    // we seek a relative reduction in MSE using the convergence tol spec:
    //   convTol = CV_mse / MC^0_mse = mse_ratio * N0 / N
    //   delta_N = mse_ratio*N0/convTol - N0 = (mse_ratio/convTol - 1) * N0
    Real incr = (avg_mse_ratio / convergenceTol - 1.) * numSamples;
    numSamples = (size_t)std::floor(incr + .5); // round

    if (numSamples) {
      shared_increment(++mlmfIter, 0);
      accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			 sum_HH, mu_hat, N_lf, N_hf);
      raw_N_lf += numSamples; raw_N_hf += numSamples;
      // update ratios:
      avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1],
				  sum_LH[1], sum_HH, cost_ratio, N_hf,
				  var_H, rho2_LH);
      avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH,mlmfIter,N_hf);
    }
  }

  // --------------------------------------------------
  // Compute LF increment based on the evaluation ratio
  // --------------------------------------------------
  uncorrected_surrogate_mode(); // also needed for assignment of lf_key below
  // Group id in lf_key is not currently important, since no SurrogateData
  // (correlations are computed based on the paired LF/HF data group, prior
  // to the augmentation, which could imply a future group segregation)
  iteratedModel.active_model_key(lf_key); // sets activeKey and surrModelKey
  if (lf_increment(avg_eval_ratio, N_lf, N_hf, ++mlmfIter, 0)) { // level 0
    accumulate_cv_sums(sum_L_refined, mu_hat, N_lf);
    raw_N_lf += numSamples;
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf, sum_L_refined, N_lf,
		 rho2_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf + (Real)raw_N_lf / cost_ratio;
}


void NonDControlVarSampling::
initialize_cv_sums(IntRealVectorMap& sum_L_shared,
		   IntRealVectorMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealVectorMap& sum_LL,      //IntRealVectorMap& sum_HH,
		   IntRealVectorMap& sum_LH)
{
  // sum_* are running sums across all increments
  std::pair<int, RealVector> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size RealVector in place and init sums to 0
    sum_L_shared.insert(empty_pr).first->second.size(numFunctions);
    sum_L_refined.insert(empty_pr).first->second.size(numFunctions);
    sum_H.insert(empty_pr).first->second.size(numFunctions);
    sum_LL.insert(empty_pr).first->second.size(numFunctions);
  //sum_HH.insert(empty_pr).first->second.size(numFunctions);
    sum_LH.insert(empty_pr).first->second.size(numFunctions);
  }
}


void NonDControlVarSampling::
accumulate_cv_sums(IntRealVectorMap& sum_L, const RealVector& offset,
		   SizetArray& num_L)
{
  // uses one set of allResponses in BYPASS_SURROGATE mode
  // IntRealVectorMap is not a multilevel case --> no discrepancies

  using std::isfinite;
  Real fn_val, prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRVMIter l_it;
  bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      prod = fn_val = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];

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


void NonDControlVarSampling::
accumulate_cv_sums(IntRealVectorMap& sum_L_shared,
		   IntRealVectorMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealVectorMap& sum_LL, IntRealVectorMap& sum_LH,
		   RealVector& sum_HH, const RealVector& offset,
		   SizetArray& num_L, SizetArray& num_H)
{
  // uses one set of allResponses in AGGREGATED_MODELS mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter ls_it, lr_it, h_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord; size_t qoi;
  bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // response mode AGGREGATED_MODELS orders HF (active model key)
      // followed by LF (previous/decremented model key)
      hf_prod = hf_fn = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
      lf_prod = lf_fn = (os) ?
	fn_vals[qoi+numFunctions] - offset[qoi+numFunctions] :
	fn_vals[qoi+numFunctions];

      // sync sample counts for all L and H interactions at this level
      if (isfinite(lf_fn) && isfinite(hf_fn)) { // neither NaN nor +/-Inf

	// High-High
	sum_HH[qoi] += hf_prod * hf_prod;

	ls_it = sum_L_shared.begin(); lr_it = sum_L_refined.begin();
	h_it  = sum_H.begin(); ll_it = sum_LL.begin(); lh_it = sum_LH.begin();
	ls_ord = /*(ls_it == sum_L_shared.end())  ? 0 :*/ ls_it->first;
	lr_ord = /*(lr_it == sum_L_refined.end()) ? 0 :*/ lr_it->first;
	h_ord  = /*(h_it  == sum_H.end())  ? 0 :*/  h_it->first;
	ll_ord = /*(ll_it == sum_LL.end()) ? 0 :*/ ll_it->first;
	lh_ord = /*(lh_it == sum_LH.end()) ? 0 :*/ lh_it->first;
	active_ord = 1;
	while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
	       h_it!=sum_H.end() || ll_it!=sum_LL.end() ||
	       lh_it!=sum_LH.end() || active_ord <= 1) {
    
	  // Low shared
	  if (ls_ord == active_ord) {
	    ls_it->second[qoi] += lf_prod;
	    ++ls_it; ls_ord = (ls_it == sum_L_shared.end())  ? 0 : ls_it->first;
	  }
	  // Low refined
	  if (lr_ord == active_ord) {
	    lr_it->second[qoi] += lf_prod;
	    ++lr_it; lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	  }
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

	  if (ls_ord || lr_ord || ll_ord || lh_ord) lf_prod *= lf_fn;
	  if (h_ord  || lh_ord)                     hf_prod *= hf_fn;
	  ++active_ord;
	}
	++num_L[qoi]; ++num_H[qoi];
      }
    }
  }
}


void NonDControlVarSampling::shared_increment(size_t iter, size_t lev)
{
  if (iter == _NPOS)  Cout << "\nCVMC sample increments: ";
  else if (iter == 0) Cout << "\nCVMC pilot sample: ";
  else Cout << "\nCVMC iteration " << iter << " sample increments: ";
  Cout << "LF = " << numSamples << " HF = " << numSamples << '\n';

  //aggregated_models_mode(); // set at calling level for CV

  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set:
  if (exportSampleSets) // for HF+LF models, use the HF tags
    export_all_samples("cv_", iteratedModel.truth_model(), iter, lev);

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


bool NonDControlVarSampling::
lf_increment(Real avg_eval_ratio, const SizetArray& N_lf,
	     const SizetArray& N_hf, size_t iter, size_t lev)
{
  // ----------------------------------------------
  // Compute Final LF increment for control variate
  // ----------------------------------------------

  // update LF samples based on evaluation ratio
  // r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  // or with inverse r  -> delta = m-n = n/inverse_r - n

  numSamples = one_sided_delta(average(N_lf), average(N_hf) * avg_eval_ratio);
  if (numSamples) {
    Cout << "\nCVMC LF sample increment = " << numSamples;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " from avg LF = " << average(N_lf) << ", avg HF = "
	   << average(N_hf) << ", avg eval_ratio = " << avg_eval_ratio;
    Cout << std::endl;

    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);// pull dist params from any model
    // export separate output files for each data set:
    if (exportSampleSets)
      export_all_samples("cv_", iteratedModel.surrogate_model(), iter, lev);

    // Iteration 0 is defined as the pilot sample, and each subsequent iter
    // can be defined as a CV increment followed by an ML increment.  In this
    // case, terminating based on max_iterations results in a final ML increment
    // without a corresponding CV refinement; thus the number of ML and CV
    // refinements is consistent although the final sample profile is not
    // self-consistent -- to override this and finish with a final CV increment
    // corresponding to the final ML increment, the finalCVRefinement flag can
    // be set.  Note: termination based on delta_N_hf=0 has a final ML increment
    // of zero and corresponding final CV increment of zero.  Therefore, this
    // iteration completes on the previous CV increment and is more consistent
    // with finalCVRefinement=true.

    if (iter < max_iter || finalCVRefinement) {
      // hierarchical surrogate mode could be BYPASS_SURROGATE for CV or
      // BYPASS_SURROGATE/AGGREGATED_MODELS for ML-CV
      //bypass_surrogate_mode(); // set at calling level for CV or ML-CV

      // compute allResponses from allVariables using hierarchical model
      evaluate_parameter_sets(iteratedModel, true, false);
      return true;
    }
    else
      return false;
  }
  else {
    Cout << "\nNo CVMC LF sample increment";
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " from avg LF = " << average(N_lf) << ", avg HF = "
	   << average(N_hf) << ", avg eval_ratio = " << avg_eval_ratio;
    Cout << std::endl;
    return false;
  }
}


Real NonDControlVarSampling::
eval_ratio(const RealVector& sum_L_shared, const RealVector& sum_H,
	   const RealVector& sum_LL, const RealVector& sum_LH,
	   const RealVector& sum_HH, Real cost_ratio,
	   const SizetArray& N_shared, RealVector& var_H, RealVector& rho2_LH)
{
  Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH[qoi];
    compute_control(sum_L_shared[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		    sum_HH[qoi], N_shared[qoi], var_H[qoi], rho_sq);

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    // Given use of 1/r in MSE_ratio, one approach would average 1/r, but
    // this does not seem to behave as well in limited numerical experience.
    //if (rho_sq > Pecos::SMALL_NUMBER) {
    //  avg_inv_eval_ratio += std::sqrt((1. - rho_sq)/(cost_ratio * rho_sq));
    if (rho_sq < 1.) { // protect against division by 0
      eval_ratio = std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = " << cost_ratio
	     << " rho_sq = " << rho_sq << " eval_ratio = " << eval_ratio
	     << std::endl;
      avg_eval_ratio += eval_ratio;
      ++num_avg;
    }
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "variance of HF Q:\n" << var_H;

  if (num_avg) avg_eval_ratio /= num_avg;
  else // should not happen, but provide a reasonable upper bound
    avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);

  return avg_eval_ratio;
}


Real NonDControlVarSampling::
eval_ratio(RealMatrix& sum_L_shared, RealMatrix& sum_H, RealMatrix& sum_LL,
	   RealMatrix& sum_LH, RealMatrix& sum_HH, Real cost_ratio, size_t lev,
	   const SizetArray& N_shared, RealMatrix& var_H, RealMatrix& rho2_LH)
{
  Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH(qoi,lev);
    compute_control(sum_L_shared(qoi,lev), sum_H(qoi,lev), sum_LL(qoi,lev),
		    sum_LH(qoi,lev), sum_HH(qoi,lev), N_shared[qoi],
		    var_H(qoi,lev), rho_sq);

    if (rho_sq < 1.) { // protect against division by 0
      eval_ratio = std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = " << cost_ratio
	     << " rho_sq = " << rho_sq << " eval_ratio = " << eval_ratio
	     << std::endl;
      avg_eval_ratio += eval_ratio;
      ++num_avg;
    }
  }
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "variance of HF Q[" << lev << "]:\n";
    write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_H);
  }

  if (num_avg) avg_eval_ratio /= num_avg;
  else // should not happen, but provide a reasonable upper bound
    avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);

  return avg_eval_ratio;
}


Real NonDControlVarSampling::
eval_ratio(RealMatrix& sum_Ll,   RealMatrix& sum_Llm1,  RealMatrix& sum_Hl,
	   RealMatrix& sum_Hlm1, RealMatrix& sum_Ll_Ll, RealMatrix& sum_Ll_Llm1,
	   RealMatrix& sum_Llm1_Llm1, RealMatrix& sum_Hl_Ll,
	   RealMatrix& sum_Hl_Llm1,   RealMatrix& sum_Hlm1_Ll,
	   RealMatrix& sum_Hlm1_Llm1, RealMatrix& sum_Hl_Hl,
	   RealMatrix& sum_Hl_Hlm1,   RealMatrix& sum_Hlm1_Hlm1,
	   Real cost_ratio, size_t lev, const SizetArray& N_shared,
	   RealMatrix& var_YHl,       RealMatrix& rho_dot2_LH)
{
  if (lev == 0)
    return eval_ratio(sum_Ll, sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl,
		      cost_ratio, lev, N_shared, var_YHl, rho_dot2_LH);
  else {
    Real beta_dot, gamma, eval_ratio, avg_eval_ratio = 0.;
    size_t qoi, num_avg = 0;
    for (qoi=0; qoi<numFunctions; ++qoi) {
      Real& rho_dot_sq = rho_dot2_LH(qoi,lev);
      compute_control(sum_Ll(qoi,lev), sum_Llm1(qoi,lev), sum_Hl(qoi,lev),
		      sum_Hlm1(qoi,lev), sum_Ll_Ll(qoi,lev),
		      sum_Ll_Llm1(qoi,lev), sum_Llm1_Llm1(qoi,lev),
		      sum_Hl_Ll(qoi,lev), sum_Hl_Llm1(qoi,lev),
		      sum_Hlm1_Ll(qoi,lev), sum_Hlm1_Llm1(qoi,lev),
		      sum_Hl_Hl(qoi,lev), sum_Hl_Hlm1(qoi,lev),
		      sum_Hlm1_Hlm1(qoi,lev), N_shared[qoi], var_YHl(qoi,lev),
		      rho_dot_sq, beta_dot, gamma);

      if (rho_dot_sq < 1.) { // protect against division by 0
	eval_ratio = std::sqrt(cost_ratio * rho_dot_sq / (1.-rho_dot_sq));
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = "
	       << cost_ratio << " rho_dot_sq = " << rho_dot_sq
	       << " eval_ratio = " << eval_ratio << std::endl;
	avg_eval_ratio += eval_ratio;
	++num_avg;
      }
    }
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "variance of HF Y[" << lev << "]:\n";
      write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_YHl);
    }

    if (num_avg) avg_eval_ratio /= num_avg;
    else // should not happen, but provide a reasonable upper bound
      avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);

    return avg_eval_ratio;
  }
}


Real NonDControlVarSampling::
MSE_ratio(Real avg_eval_ratio, const RealVector& var_H,
	  const RealVector& rho2_LH, size_t iter, const SizetArray& N_hf)
{
  if (iter == 0) mcMSEIter0.sizeUninitialized(numFunctions);

  Real mc_mse, cvmc_mse, mse_ratio, avg_mse_ratio = 0.;//,avg_mse_iter_ratio=0.;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
    mse_ratio = 1. - rho2_LH[qoi] * (1. - 1. / avg_eval_ratio); // Ng 2014
    mc_mse = var_H[qoi] / N_hf[qoi]; cvmc_mse = mc_mse * mse_ratio;
    Cout << "Mean square error for QoI " << qoi+1 << " reduced from " << mc_mse
	 << " (MC) to " << cvmc_mse << " (CV); factor = " << mse_ratio << '\n';

    if (iter == 0)
      { mcMSEIter0[qoi] = mc_mse; avg_mse_ratio += mse_ratio; }
    else
      avg_mse_ratio += cvmc_mse / mcMSEIter0[qoi];
  }
  //avg_mse_iter_ratio /= numFunctions;
  avg_mse_ratio /= numFunctions;
  Cout //<< "Average MSE reduction factor from CV for iteration "
       //<< std::setw(4) << iter << " = " << avg_mse_iter_ratio << '\n'
       << "Average MSE reduction factor since pilot MC = " << avg_mse_ratio
       << " targeting convergence tol = " << convergenceTol << "\n\n";
  return avg_mse_ratio;
}


void NonDControlVarSampling::
cv_raw_moments(IntRealVectorMap& sum_L_shared, IntRealVectorMap& sum_H,
	       IntRealVectorMap& sum_LL,       IntRealVectorMap& sum_LH,
	       const SizetArray& N_shared,     IntRealVectorMap& sum_L_refined,
	       const SizetArray& N_refined,    const RealVector& rho2_LH,
	       RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);
  RealVector beta(numFunctions, false);

  // rho2_LH not stored for i > 1
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
	 << std::setw(9) << std::sqrt(rho2_LH[qoi]) << '\n';
       //<< ", effectiveness ratio = " << std::setw(9) << rho2_LH[qoi] * cr1;

  for (int i=1; i<=4; ++i) {
    compute_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i], N_shared,
		    beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		  N_refined, beta, H_rm_col);
  }
}


void NonDControlVarSampling::
cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_H,
	       IntRealMatrixMap& sum_LL,       IntRealMatrixMap& sum_LH,
	       const SizetArray& N_shared,     IntRealMatrixMap& sum_L_refined,
	       const SizetArray& N_refined,    const RealMatrix& rho2_LH,
	       size_t lev,                     RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);
  RealVector beta(numFunctions, false);

  // rho2_LH not stored for i > 1
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
	 << std::setw(9) << std::sqrt(rho2_LH(qoi,lev)) << '\n';
       //<< ", effectiveness ratio = " << std::setw(9) << rho2_LH(qoi,lev)*cr1;

  for (int i=1; i<=4; ++i) {
    compute_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i], N_shared,
		    lev, beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		  N_refined, lev, beta, H_rm_col);
  }
  Cout << '\n'; // for loop over levels
}


void NonDControlVarSampling::
cv_raw_moments(IntRealMatrixMap& sum_Ll,        IntRealMatrixMap& sum_Llm1,
	       IntRealMatrixMap& sum_Hl,        IntRealMatrixMap& sum_Hlm1,
	       IntRealMatrixMap& sum_Ll_Ll,     IntRealMatrixMap& sum_Ll_Llm1,
	       IntRealMatrixMap& sum_Llm1_Llm1, IntRealMatrixMap& sum_Hl_Ll,
	       IntRealMatrixMap& sum_Hl_Llm1,   IntRealMatrixMap& sum_Hlm1_Ll,
	       IntRealMatrixMap& sum_Hlm1_Llm1, IntRealMatrixMap& sum_Hl_Hl,
	       IntRealMatrixMap& sum_Hl_Hlm1,   IntRealMatrixMap& sum_Hlm1_Hlm1,
	       const SizetArray& N_shared, IntRealMatrixMap& sum_Ll_refined,
	       IntRealMatrixMap& sum_Llm1_refined, const SizetArray& N_refined,
	       const RealMatrix& rho_dot2_LH, size_t lev, RealMatrix& H_raw_mom)
{
  if (lev == 0)
    cv_raw_moments(sum_Ll, sum_Hl, sum_Ll_Ll, sum_Hl_Ll, N_shared,
		   sum_Ll_refined, N_refined, rho_dot2_LH, lev, H_raw_mom);
  else {
    if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);
    RealVector beta_dot(numFunctions, false), gamma(numFunctions, false);

    // rho_dot2_LH not stored for i > 1
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      Cout << "rho_dot_LH (Pearson correlation) for QoI " << qoi+1 << " = "
	   << std::setw(9) << std::sqrt(rho_dot2_LH(qoi,lev)) << '\n';
         //<< ", effectiveness ratio = " << rho_dot2_LH(qoi,lev) * cr1;

    // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k
    for (int i=1; i<=4; ++i) {
      compute_control(sum_Ll[i], sum_Llm1[i], sum_Hl[i], sum_Hlm1[i],
		      sum_Ll_Ll[i], sum_Ll_Llm1[i], sum_Llm1_Llm1[i],
		      sum_Hl_Ll[i], sum_Hl_Llm1[i], sum_Hlm1_Ll[i],
		      sum_Hlm1_Llm1[i], sum_Hl_Hl[i], sum_Hl_Hlm1[i],
		      sum_Hlm1_Hlm1[i], N_shared, lev, beta_dot, gamma);
      Cout << "Moment " << i << ":\n";
      RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
      apply_control(sum_Hl[i], sum_Hlm1[i], sum_Ll[i], sum_Llm1[i], N_shared,
		    sum_Ll_refined[i], sum_Llm1_refined[i], N_refined, lev,
		    beta_dot, gamma, H_rm_col);
    }
    Cout << '\n'; // for loop over levels
  }
}


void NonDControlVarSampling::
compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		size_t N_shared, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  //Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  //Real var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
  //    cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;

  // Cancel repeated N_shared and bessel_corr within beta = cov_LH / var_L:
  beta = (sum_LH - sum_L * sum_H / N_shared)
       / (sum_LL - sum_L * sum_L / N_shared);
}


void NonDControlVarSampling::
compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH, Real sum_HH,
		size_t N_shared, Real& var_H, Real& rho2_LH)
{
  Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
      cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;
  var_H      = (sum_HH / N_shared - mu_H * mu_H) * bessel_corr;

  //beta  = cov_LH / var_L;
  rho2_LH = cov_LH / var_L * cov_LH / var_H;
}


void NonDControlVarSampling::
compute_control(Real sum_Ll, Real sum_Llm1, Real sum_Hl, Real sum_Hlm1,
		Real sum_Ll_Ll, Real sum_Ll_Llm1, Real sum_Llm1_Llm1,
		Real sum_Hl_Ll, Real sum_Hl_Llm1, Real sum_Hlm1_Ll,
		Real sum_Hlm1_Llm1, Real sum_Hl_Hl, Real sum_Hl_Hlm1,
		Real sum_Hlm1_Hlm1, size_t N_shared, Real& var_YH,
		Real& rho_dot2_LH, Real& beta_dot, Real& gamma)
{
  Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1);

  // means, variances, covariances for Q
  // Note: sum_*[i][lm1] is not the same as sum_*lm1[i][lev] due to
  //       discrepancy evaluations with different sample sets!
  Real mu_Ll = sum_Ll / N_shared,  mu_Llm1 = sum_Llm1 / N_shared;
  Real mu_Hl = sum_Hl / N_shared,  mu_Hlm1 = sum_Hlm1 / N_shared;

  Real var_Ll   = (sum_Ll_Ll     / N_shared - mu_Ll   * mu_Ll)   * bessel_corr;
  Real var_Llm1 = (sum_Llm1_Llm1 / N_shared - mu_Llm1 * mu_Llm1) * bessel_corr;
  Real var_Hl   = (sum_Hl_Hl     / N_shared - mu_Hl   * mu_Hl)   * bessel_corr;
  Real var_Hlm1 = (sum_Hlm1_Hlm1 / N_shared - mu_Hlm1 * mu_Hlm1) * bessel_corr;

  Real cov_Hl_Ll   = (sum_Hl_Ll   / N_shared - mu_Hl   * mu_Ll)   * bessel_corr;
  Real cov_Hl_Llm1 = (sum_Hl_Llm1 / N_shared - mu_Hl   * mu_Llm1) * bessel_corr;
  Real cov_Hlm1_Ll = (sum_Hlm1_Ll / N_shared - mu_Hlm1 * mu_Ll)   * bessel_corr;
  Real cov_Hlm1_Llm1
    = (sum_Hlm1_Llm1 / N_shared - mu_Hlm1 * mu_Llm1) * bessel_corr;

  Real cov_Ll_Llm1 = (sum_Ll_Llm1 / N_shared - mu_Ll * mu_Llm1) * bessel_corr;
  Real cov_Hl_Hlm1 = (sum_Hl_Hlm1 / N_shared - mu_Hl * mu_Hlm1) * bessel_corr;

  // quantities derived from Q moments
  // gamma:
  Real cov_YHl_Ll   = cov_Hl_Ll   - cov_Hlm1_Ll;
  Real cov_YHl_Llm1 = cov_Hl_Llm1 - cov_Hlm1_Llm1;
  gamma = (cov_YHl_Llm1 * cov_Ll_Llm1 - var_Llm1 * cov_YHl_Ll)
        / (var_Ll * cov_YHl_Llm1 - cov_YHl_Ll * cov_Ll_Llm1);
  // theta, tau, beta:
  Real cov_YHl_YLldot = gamma * (cov_Hl_Ll - cov_Hlm1_Ll)
                      - cov_Hl_Llm1 + cov_Hlm1_Llm1;
  Real cov_YHl_YLl = cov_Hl_Ll - cov_Hlm1_Ll - cov_Hl_Llm1 + cov_Hlm1_Llm1;
  Real var_YLldot  = gamma * (gamma * var_Ll - 2. * cov_Ll_Llm1) + var_Llm1;
  Real var_YHl = var_Hl - 2. * cov_Hl_Hlm1 + var_Hlm1,
       var_YLl = var_Ll - 2. * cov_Ll_Llm1 + var_Llm1,
       theta   = cov_YHl_YLldot / cov_YHl_YLl, tau = var_YLldot / var_YLl;
  // carry forwards:
  var_YH   = var_Hl - 2. * cov_Hl_Hlm1 + var_Hlm1; // var(H_l - H_lm1)
  beta_dot = cov_YHl_YLldot / var_YLldot;

  // compute evaluation ratio which determines increment for LF samples
  // > the sample increment optimizes the total computational budget and is
  //   not treated as a worst case accuracy reqmt --> use the QoI average
  // > refinement based only on QoI mean statistics
  // Given use of 1/r in MSE_ratio, one approach would average 1/r, but
  // this does not seem to behave as well in limited numerical experience.
  Real rho2_LH = cov_YHl_YLl / var_YHl * cov_YHl_YLl / var_YLl,
       ratio   = theta * theta / tau;
  // variance reduction test
  //if (ratio < 1.) ...switch to Ycorr-based control...
  rho_dot2_LH  = rho2_LH * ratio;
  //rho_dot2_LH = cov_YHl_YLldot / var_YHl * cov_YHl_YLldot / var_YLldot;

  if (outputLevel == DEBUG_OUTPUT)
    Cout << "compute_control(): var reduce ratio = " << ratio << " rho2_LH = "
	 << rho2_LH << " rho_dot2_LH = " << rho_dot2_LH << std::endl;
}


void NonDControlVarSampling::
apply_control(Real sum_H, Real sum_L_shared, size_t N_shared,
	      Real sum_L_refined, size_t N_refined, Real beta, Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom = sum_H / N_shared                    // mu_H from shared samples
            - beta * (sum_L_shared  / N_shared -  // mu_L from shared samples
		      sum_L_refined / N_refined); // refined_mu_L incl increment
}


void NonDControlVarSampling::
apply_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll, Real sum_Llm1,
	      size_t N_shared,  Real sum_Ll_refined, Real sum_Llm1_refined,
	      size_t N_refined, Real beta_dot, Real gamma, Real& H_raw_mom)
{
  // updated LF expectations following final sample increment:
  Real mu_Hl = sum_Hl / N_shared,  mu_Hlm1 = sum_Hlm1 / N_shared,
       mu_Ll = sum_Ll / N_shared,  mu_Llm1 = sum_Llm1 / N_shared;
  Real refined_mu_Ll   =   sum_Ll_refined / N_refined;
  Real refined_mu_Llm1 = sum_Llm1_refined / N_refined;

  // apply control for HF uncentered raw moment estimates:
  Real mu_YH            = mu_Hl - mu_Hlm1;
  Real mu_YLdot         = gamma *         mu_Ll -         mu_Llm1;
  Real refined_mu_YLdot = gamma * refined_mu_Ll - refined_mu_Llm1;
  H_raw_mom             = mu_YH - beta_dot * (mu_YLdot - refined_mu_YLdot);
}

} // namespace Dakota
