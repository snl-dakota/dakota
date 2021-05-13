/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultifidelitySampling
//- Description: Implementation code for NonDMultifidelitySampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultifidelitySampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDMultifidelitySampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultifidelitySampling::
NonDMultifidelitySampling(ProblemDescDB& problem_db, Model& model):
  NonDHierarchSampling(problem_db, model), finalCVRefinement(true)
{
  // For now...
  size_t num_mf = NLev.size();
  if (num_mf > 2) {
    Cerr << "Warning: NonDMultifidelitySampling currently uses first and last "
	 << "model in ordered sequence and ignores the rest." << std::endl;
    //abort_handler(METHOD_ERROR);
  }
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDMultifidelitySampling::core_run()
{
  // remove default key (empty activeKey) since this interferes with approx
  // combination in MF surrogates.  Also useful for ML/MF re-entrancy.
  iteratedModel.clear_model_keys();
  // prefer ML over MF if both available
  iteratedModel.multifidelity_precedence(true);

  ////////////////////////////////////

  // For two-model control variate methods, select lowest,highest fidelities
  // infer the (two) model hierarchy and perform MFMC
  //control_variate_mc();

  ////////////////////////////////////

  // For two-model control variate methods, select lowest,highest fidelities
  Pecos::ActiveKey hf_lf_key;
  size_t num_mf = NLev.size();
  if (num_mf > 1) {
    unsigned short lf_form = 0, hf_form = num_mf - 1;// ordered_models=low:high
    // use nominal value from user input, ignoring solution_level_control
    size_t lev = std::numeric_limits<size_t>::max();
    hf_lf_key.form_key(0, hf_form, lev, lf_form, lev, Pecos::RAW_DATA);
  }
  else {
    size_t num_hf_lev = NLev.back().size();
    if (num_hf_lev > 1) // ML performed on HF with CV using available LF
      hf_lf_key.form_key(0, 0, num_hf_lev-1, 0, 0, Pecos::RAW_DATA);
    else {
      PCerr << "Error: " << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  control_variate_mc(hf_lf_key);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultifidelitySampling::
control_variate_mc(const Pecos::ActiveKey& active_key)
{
  // Current implementation performs pilot + shared increment + LF increment,
  // where these increments are targeting a prescribed MSE reduction.
  // **********
  // *** TO DO: should CV MC iterate (new shared + LF increments)
  // ***        until MSE target is met?
  // **********

  /*
  configure_sequence(num_steps, secondary_index, seq_type);
  bool multilev = (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  // either lev varies and form is fixed, or vice versa:
  size_t& step = (multilev) ? lev : form;
  if (multilev) form = secondary_index;
  else          lev  = secondary_index;
  */

  //////////////////////////////////////

  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0
  Model& truth_model = iteratedModel.truth_model();     // ***
  Model& surr_model  = iteratedModel.surrogate_model(); // ***

  // retrieve active index
  //size_t lf_lev_index =  surr_model.solution_level_cost_index(),
  //       hf_lev_index = truth_model.solution_level_cost_index();
  // retrieve cost estimates across model forms for a particular soln level
  Real lf_cost =  surr_model.solution_level_cost(),
       hf_cost = truth_model.solution_level_cost(),
    cost_ratio = hf_cost / lf_cost, avg_eval_ratio, avg_mse_ratio;

  IntRealVectorMap sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false),
            rho2_LH(numFunctions, false);

  // Initialize for pilot sample
  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, 2, delta_N_l); // 2 models only for now

  // NLev allocations currently enforce truncation to #HF levels (1)
  Pecos::ActiveKey hf_key, lf_key;
  active_key.extract_keys(hf_key, lf_key);
  unsigned short hf_form = hf_key.retrieve_model_form(),
                 lf_form = lf_key.retrieve_model_form();
  SizetArray& N_lf = NLev[lf_form][0];//[lf_lev_index]; // ***
  SizetArray& N_hf = NLev[hf_form][0];//[hf_lev_index]; // ***
  size_t raw_N_lf = 0, raw_N_hf = 0;
  RealVector mu_hat;

  // ---------------------
  // Compute Pilot Samples
  // ---------------------

  mlmfIter = 0;

  // Initialize for pilot sample (shared sample count discarding any excess)
  numSamples = std::min(delta_N_l[lf_form], delta_N_l[hf_form]);
  shared_increment(mlmfIter, 0);
  accumulate_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		     sum_HH, mu_hat, N_lf, N_hf);
  raw_N_lf += numSamples; raw_N_hf += numSamples;

  // Compute the LF/HF evaluation ratio, averaged over the QoI.
  // This includes updating var_H and rho2_LH.
  avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			      sum_HH, cost_ratio, N_hf, var_H, rho2_LH);
  // compute the ratio of MC and CVMC mean squared errors (controls convergence)
  avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, mlmfIter, N_hf);

  //while (Pecos::l1_norm(delta_N_l) && mlmfIter <= max_iter) {

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
      accumulate_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
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
    accumulate_mf_sums(sum_L_refined, mu_hat, N_lf);
    raw_N_lf += numSamples;
  }

  //  ++mlmfIter;
  //  Cout << "\nCVMC iteration " << mlmfIter << " sample increments:\n"
  //       << delta_N_l << std::endl;
  //} // end while

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf, sum_L_refined, N_lf,
		 rho2_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf + (Real)raw_N_lf / cost_ratio;
}


void NonDMultifidelitySampling::
initialize_mf_sums(IntRealVectorMap& sum_L_shared,
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


void NonDMultifidelitySampling::
accumulate_mf_sums(IntRealVectorMap& sum_L, const RealVector& offset,
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


void NonDMultifidelitySampling::
accumulate_mf_sums(IntRealVectorMap& sum_L_shared,
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


void NonDMultifidelitySampling::shared_increment(size_t iter, size_t lev)
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


bool NonDMultifidelitySampling::
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
    size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
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


Real NonDMultifidelitySampling::
eval_ratio(const RealVector& sum_L_shared, const RealVector& sum_H,
	   const RealVector& sum_LL, const RealVector& sum_LH,
	   const RealVector& sum_HH, Real cost_ratio,
	   const SizetArray& N_shared, RealVector& var_H, RealVector& rho2_LH)
{
  Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH[qoi];
    compute_mf_correlation(sum_L_shared[qoi], sum_H[qoi], sum_LL[qoi],
			   sum_LH[qoi], sum_HH[qoi], N_shared[qoi],
			   var_H[qoi], rho_sq);

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


Real NonDMultifidelitySampling::
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


void NonDMultifidelitySampling::
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
    compute_mf_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i],
		       N_shared, beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_mf_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		     N_refined, beta, H_rm_col);
  }
}

} // namespace Dakota
