/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDACVSampling
//- Description: Implementation code for NonDACVSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

//#ifdef HAVE_NPSOL
//#include "NPSOLOptimizer.hpp"
//#elif HAVE_OPTPP
//#include "SNLLOptimizer.hpp"
//#endif

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDEnsembleSampling(problem_db, model)
{
  // check iteratedModel for model form hi1erarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  if (iteratedModel.surrogate_type() == "non_hierarchical")
    aggregated_models_mode(); // truth model + all approx models
  else {
    Cerr << "Error: Non-hierarchical sampling requires a non-hierarchical "
         << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  size_t i, num_mf = model_ensemble.size(), num_lev;
  ModelLIter ml_it; bool err_flag = false;
  NLev.resize(num_mf);
  for (i=0, ml_it=model_ensemble.begin(); ml_it!=model_ensemble.end();
       ++i, ++ml_it) {
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev = ml_it->solution_levels(); // lower bound is 1 soln level

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    // > For ACV, only require 1 solution cost, neglecting resolutions for now
    //if (num_lev > ml_it->solution_levels(false)) { // 
    if (ml_it->solution_levels(false) == 0) { // default is 0 soln costs
      Cerr << "Error: insufficient cost data provided for ACV sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_it->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);
}


NonDACVSampling::~NonDACVSampling()
{ }


/*
bool NonDACVSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDACVSampling::pre_run()
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
*/


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDACVSampling::core_run()
{
  // remove default key (empty activeKey) since this interferes with approx
  // combination in MF surrogates.  Also useful for ML/MF re-entrancy.
  iteratedModel.clear_model_keys();
  // prefer MF over ML if both available
  iteratedModel.multifidelity_precedence(true);

  switch (methodName) {
  case MULTIFIDELITY_SAMPLING:
    //sequence_models(); // enforce correlation condition...
    multifidelity_mc(); break; // Peherstorfer, Willcox, Gunzburger, 2016
  case ACV_MULTIFIDELITY_SAMPLING:
    approx_control_variate_multifidelity(); break;
  case ACV_INDEPENDENT_SAMPLING:
    break;
  case ACV_KL_SAMPLING:
    break;
  }
  // Notes on ACV + ensemble model classes:
  // > HierarchSurrModel is limiting (see MFMC) such that we may want to
  //   subsume it with NonHierarchSurrModel --> consider generalizations
  //   that can be deployed across the algorithm set:
  //   >> enhanced parallel usage --> avoid sync points in ACV clients
  //   >> enable sampling over shared + distinct model variable sets
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::multifidelity_mc()
{
  // Performs pilot + LF increment and then iterates with additional shared
  // increment + LF increment batches until prescribed MSE reduction is obtained

  // *** TO DO: test for #Models = 2 and then retire DakotaIterator switch...

  short seq_type;  size_t num_steps, secondary_index, qoi, approx;
  configure_sequence(num_steps, secondary_index, seq_type); // MF if #models > 1
  bool multilev = (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE);//(false);
  numApprox = num_steps - 1;

  // For M-model control variate, select fidelities/resolutions
  Pecos::ActiveKey active_key, truth_key;
  std::vector<Pecos::ActiveKey> approx_keys(numApprox);
  //unsigned short truth_form;  size_t truth_lev;
  if (multilev) {
    unsigned short fixed_form = (secondary_index == SZ_MAX) ?
      USHRT_MAX : secondary_index;
    truth_key.form_key(0, fixed_form, numApprox);
    for (approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, fixed_form, approx);
    //truth_form = fixed_form;  truth_lev = numApprox;
  }
  else {
    truth_key.form_key(0, numApprox, secondary_index);
    for (approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, approx, secondary_index);
    //truth_form = numApprox;  truth_lev = secondary_index;
  }
  active_key.aggregate_keys(truth_key, approx_keys, Pecos::RAW_DATA);
  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0

  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost;  configure_cost(num_steps, multilev, cost);
  //SizetArray raw_N; raw_N.assign(num_steps, 0);

  IntRealVectorMap sum_H;
  IntRealMatrixMap sum_L_shared, sum_L_refined, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false),
    mse_iter0, mse_ratios, hf_targets;
  RealMatrix rho2_LH(numFunctions, false), eval_ratios;
  SizetArray N_H, delta_N; Sizet2DArray N_L(numApprox), N_LH(numApprox);
  N_H.assign(numFunctions, 0);
  for (approx=0; approx<numApprox; ++approx)
    { N_L[approx].assign(numFunctions,0); N_LH[approx].assign(numFunctions,0); }

  // Initialize for pilot sample
  load_pilot_sample(pilotSamples, num_steps, delta_N);
  numSamples = delta_N[numApprox]; // last in array

  mlmfIter = 0;  bool converged = false;
  while (!converged) {

    // ------------------------------------------------------------------------
    // Compute shared increment targeting specified budget and/or MSE reduction
    // ------------------------------------------------------------------------
    // Scale sample profile based on maxFunctionEvals or convergenceTol,
    // but not both (for now)
    if (mlmfIter) {
      // ------------------------------------------------------
      // Option 1: assign complete budget based on pilot sample
      // ------------------------------------------------------
      // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L 
      // > could consider under-relaxing the budget allocation to enable
      //   additional N_H increments + associated updates to shared samples
      //   for improving rho2_LH et al.
      if (maxFunctionEvals != SZ_MAX) {
	Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
	allocate_budget(eval_ratios, cost, hf_targets);
      }
      // ---------------------------------------------------
      // Option 2: iterate until relative tolerance achieved
      // ---------------------------------------------------
      // MFMC MSE target = convTol * mse_iter0 = mse_ratio * var_H / N_H
      //           N_H = mse_ratio * var_H / convTol / mse_iter0
      // Note: don't simplify further since mse_iter0 is fixed based on pilot
      else {
	Cout << "Scaling profile for convergenceTol = " << convergenceTol;
	hf_targets = mse_ratios;
	for (qoi=0; qoi<numFunctions; ++qoi)
	  hf_targets[qoi] *= var_H[qoi] / mse_iter0[qoi] / convergenceTol;
	// Power mean choice: average, max (desire would be to balance
	// overshoot vs. additional iteration)
      }
      // numSamples is relative to N_H, but the approx_increments() below are
      // computed relative to hf_targets (independent of sunk cost for pilot)
      Cout << ": average HF target = " << average(hf_targets) << std::endl;
      numSamples = one_sided_delta(N_H, hf_targets, 1);
    }

    if (numSamples) {
      shared_increment(mlmfIter, numApprox); // spans ALL models, blocking
      accumulate_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			 sum_HH, N_L, N_H, N_LH);
      //increment_mf_samples(numSamples, 0, num_steps, raw_N);
      increment_mf_equivalent_cost(numSamples, 0, num_steps, cost);

      // Compute the LF/HF evaluation ratio using shared samples, averaged
      // over QoI.  This includes updating var_H and rho2_LH.
      compute_eval_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, cost, N_L, N_H, N_LH, var_H, rho2_LH,
			  eval_ratios);
      // mse_iter0 only uses HF pilot since sum_L_shared / N_shared minus
      // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
      // (This differs from MLMC MSE^0 which uses pilot for all levels.)
      if (mlmfIter == 0) compute_mc_estimator_variance(var_H, N_H, mse_iter0);
      // Compute the ratio of MC and CVMC mean squared errors (for convergence).
      // This ratio incorporates the anticipated variance reduction from the
      // upcoming application of eval_ratios.
      compute_MSE_ratios(eval_ratios, rho2_LH, cost, mse_ratios);
    }
    else {
      converged = true;
      //Cout << "\nMFMC iteration " << mlmfIter
      //     << ": no shared sample increment" << std::endl;
    }

    ++mlmfIter;
    if (mlmfIter > maxIterations) converged = true;
  }

  // ---------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ---------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after N_H has converged, which simplifies maxFnEvals / convTol logic
  // (no need to further interrogate these throttles below)

  // Pyramid/nested sampling: at step i, we sample approximation range
  // [0,numApprox-1-i] using the delta relative to the previous step
  for (approx=numApprox; approx>0; --approx) {
    // pyramid sampling increments span models {0, ..., approx}
    if (approx_increment(eval_ratios, N_L, hf_targets, mlmfIter, 0, approx)) { // *** TO DO: NON_BLOCKING: PERFORM A 2ND PASS ACCUMULATION AFTER 1ST PASS LAUNCH
      accumulate_mf_sums(sum_L_shared, sum_L_refined, approx, N_L);
      //increment_mf_samples(numSamples,       0, approx, raw_N);
      increment_mf_equivalent_cost(numSamples, 0, approx, cost);
    }
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, sum_L_refined, rho2_LH,
		 N_L, N_H, N_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);

  // post final sample counts back to NLev (needed for final eval summary)
  N_L.push_back(N_H); // aggregate into a single Sizet2DArray
  inflate_final_samples(N_L, multilev, secondary_index, NLev);
  /*
  // Alternate print_results():
  print_multilevel_evaluation_summary(s, N_L);
  print_multilevel_evaluation_summary(s, N_H);
  s << "<<<<< Equivalent number of high fidelity evaluations: "
    << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";
  */

}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approx_control_variate_multifidelity()
{
  // Performs pilot + LF increment and then iterates with additional shared
  // increment + LF increment batches until prescribed MSE reduction is obtained

  size_t qoi, num_steps, form, lev, fixed_index;  short seq_type;
  configure_sequence(num_steps, fixed_index, seq_type);
  bool multilev = (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE); // false


  // *** TO DO ***


  // Convert uncentered raw moment estimates to final moments (central or std)
  RealMatrix H_raw_mom(numFunctions, 4);
  //cv_raw_moments(..., H_raw_mom);
  convert_moments(H_raw_mom, momentStats);
}


void NonDACVSampling::
shared_increment(/*const Pecos::ActiveKey& agg_key,*/ size_t iter, size_t step)
{
  if (iter == 0) Cout << "\nMFMC pilot sample: ";
  else Cout << "\nMFMC iteration " << iter << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

    activeSet.request_values(1);
    ensemble_sample_increment(iter, step); // BLOCK
  }
}


void NonDACVSampling::
allocate_budget(const RealMatrix& eval_ratios, const RealVector& cost,
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
  size_t qoi, approx;
  Real cost_H = cost[numApprox], inner_prod, budget = (Real)maxFunctionEvals;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // raw cost (un-normalized)
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * eval_ratios(qoi, approx);
    hf_targets[qoi] = budget / inner_prod * cost_H; // normalized to equivHF
  }
}


bool NonDACVSampling::
approx_increment(const RealMatrix& eval_ratios, const Sizet2DArray& N_L,
		 const RealVector& hf_targets, size_t iter,
		 size_t start, size_t end)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts
  // > eval_ratios have been scaled if needed to satisfy specified budget
  //   (maxFunctionEvals) in
  size_t qoi, approx = end - 1;
  RealVector lf_targets(numFunctions, false);
  for (qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios(qoi, approx) * hf_targets[qoi];

  // Choose avg, RMS, max? (trade-off: possible overshoot vs. more iteration)
  numSamples = one_sided_delta(N_L[approx], lf_targets, 1); // average

  if (numSamples && start < end) {
    Cout << "\nMFMC sample increment = " << numSamples
	 << " for approximations [" << start+1 << ", " << end << ']';
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " computed from average delta between target:\n" << lf_targets
	   << "and current counts:\n" << N_L[approx];
    Cout << std::endl;
    size_t start_qoi = start * numFunctions, end_qoi = end * numFunctions;
    activeSet.request_values(0);
    //activeSet.request_values(0, 0, start_qoi);
    activeSet.request_values(1, start_qoi, end_qoi);
    //activeSet.request_values(0, end_qoi, iteratedModel.response_size());
    ensemble_sample_increment(iter, start); // NON-BLOCK
    return true;
  }
  else {
    Cout << "\nNo MFMC approx sample increment for approximations ["
	 << start+1 << ", " << end << ']' << std::endl;
    return false;
  }
}


void NonDACVSampling::
ensemble_sample_increment(size_t iter, size_t step)
{
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set:
  if (exportSampleSets) { // for HF+LF models, use the HF tags
    export_all_samples("cv_", iteratedModel.truth_model(), iter, step);
    for (size_t i=0; i<numApprox; ++i)
      export_all_samples("cv_", iteratedModel.surrogate_model(i), iter, step);
  }

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


void NonDACVSampling::
initialize_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL,      //IntRealVectorMap& sum_HH,
		   IntRealMatrixMap& sum_LH)
{
  // sum_* are running sums across all increments
  std::pair<int, RealVector> vec_pr; std::pair<int, RealMatrix> mat_pr;
  for (int i=1; i<=4; ++i) {
    vec_pr.first = mat_pr.first = i; // moment number
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size Real{Vector,Matrix} in place and init sums to 0
    sum_L_shared.insert(mat_pr).first->second.shape(numFunctions, numApprox);
    sum_L_refined.insert(mat_pr).first->second.shape(numFunctions, numApprox);
    sum_H.insert(vec_pr).first->second.size(numFunctions);
    sum_LL.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  //sum_HH.insert(vec_pr).first->second.size(numFunctions);
    sum_LH.insert(mat_pr).first->second.shape(numFunctions, numApprox);
  }
}


void NonDACVSampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, size_t approx_end,
		   //const RealVector& offset,
		   Sizet2DArray& num_L)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  Real fn_val, prod;
  int ls_ord, lr_ord, active_ord;
  size_t qoi, fn_index, approx, shared_end = approx_end - 1;
  IntRespMCIter r_it; IntRMMIter ls_it, lr_it;  //bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    fn_index = 0;

    // accumulate for leading set of models (omit trailing truth),
    // but note that resp and asv are full aggregated length
    for (approx=0; approx<approx_end; ++approx) {

      SizetArray& num_L_a = num_L[approx];
      for (qoi=0; qoi<numFunctions; ++qoi, ++fn_index) {
	//if (asv[fn_index] & 1) {
	  prod = fn_val = //(os) ? fn_vals[fn_index] - offset[fn_index] :
	                           fn_vals[fn_index];
	  if (isfinite(fn_val)) { // neither NaN nor +/-Inf
	    ++num_L_a[qoi]; // number of recovered response observations

	    // for pyramid sampling, shared accumulation lags refined by 1
	    if (approx < shared_end) {
	      ls_it = sum_L_shared.begin(); ls_ord = ls_it->first;
	      active_ord = 1;
	      while (ls_it!=sum_L_shared.end()) { // Low shared
		if (ls_ord == active_ord) { // support general key sequence
		  ls_it->second(qoi,approx) += prod;  ++ls_it;
		  ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
		}
		prod *= fn_val;  ++active_ord;
	      }
	    }

	    // index for refined accumulation is 1 more than last shared
	    lr_it = sum_L_refined.begin(); lr_ord = lr_it->first;
	    active_ord = 1;
	    while (lr_it!=sum_L_refined.end()) { // Low refined
	      if (lr_ord == active_ord) { // support general key sequence
		lr_it->second(qoi,approx) += prod;  ++lr_it;
		lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	      }
	      prod *= fn_val;  ++active_ord;
	    }
	  }
	//}
      }
    }
  }
}


void NonDACVSampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		   RealVector& sum_HH, //const RealVector& offset,
		   Sizet2DArray& num_L, SizetArray& num_H, Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter h_it; IntRMMIter ls_it, lr_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord;
  size_t qoi, approx, lf_index, hf_index;  //bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    hf_index = numApprox * numFunctions;

    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = //(os) ? fn_vals[hf_index] - offset[hf_index] :
	fn_vals[hf_index];
      // High accumulations:
      if (isfinite(hf_fn)) { // neither NaN nor +/-Inf
	++num_H[qoi];
	// High-High:
	sum_HH[qoi] += hf_fn * hf_fn; // a single vector for ord 1
	// High:
	h_it = sum_H.begin();  h_ord = h_it->first;
	hf_prod = hf_fn;       active_ord = 1;
	while (h_it!=sum_H.end()) {
	  if (h_ord == active_ord) { // support general key sequence
	    h_it->second[qoi] += hf_prod;
	    ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  hf_prod *= hf_fn;  ++active_ord;
	}
      }
	
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = //(os) ? fn_vals[lf_index] - offset[lf_index] :
	  fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L[approx][qoi];
	  ls_it = sum_L_shared.begin();	  ls_ord = ls_it->first;
	  lr_it = sum_L_refined.begin();  lr_ord = lr_it->first;
	  ll_it = sum_LL.begin();         ll_ord = ll_it->first;
	  lf_prod = lf_fn;	          active_ord = 1;
	  while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
		 ll_it!=sum_LL.end()       || active_ord <= 1) {
    
	    // Low shared
	    if (ls_ord == active_ord) { // support general key sequence
	      ls_it->second(qoi,approx) += lf_prod;  ++ls_it;
	      ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	    }
	    // Low refined
	    if (lr_ord == active_ord) { // support general key sequence
	      lr_it->second(qoi,approx) += lf_prod;  ++lr_it;
	      lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      ll_it->second(qoi,approx) += lf_prod * lf_prod;  ++ll_it;
	      ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }

	  // Low-High accumulations:
	  if (isfinite(hf_fn)) { // both are finite
	    ++num_LH[approx][qoi];
	    lh_it = sum_LH.begin(); lh_ord = lh_it->first;
	    lf_prod = lf_fn;  hf_prod = hf_fn;  active_ord = 1;
	    while (lh_it!=sum_LH.end() || active_ord <= 1) {
	      // Low-High
	      if (lh_ord == active_ord) { // support general key sequence
		lh_it->second(qoi,approx) += lf_prod * hf_prod;
		++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	      }
	      lf_prod *= lf_fn;  hf_prod *= hf_fn;  ++active_ord;
	    }
	  }
	}
      }
    }
  }
}


void NonDACVSampling::
compute_eval_ratios(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		    const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		    const RealVector& sum_HH, const RealVector& cost,
		    const Sizet2DArray& N_L, const SizetArray& N_H,
		    const Sizet2DArray& N_LH, RealVector& var_H,
		    RealMatrix& rho2_LH, RealMatrix& eval_ratios)
{
  size_t approx, qoi;
  if (var_H.empty())     var_H.sizeUninitialized(numFunctions);
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);
  Real cost_ratio, cost_H = cost[numApprox]; // HF cost
  //Real avg_eval_ratio = 0.;  size_t num_avg = 0;

  // first sweep to compute all of rho2_LH
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LL_a =       sum_LL[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    const SizetArray&    N_L_a =          N_L[approx];
    const SizetArray&   N_LH_a =         N_LH[approx];
    Real*            rho2_LH_a =      rho2_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_mf_correlation(sum_L_shared_a[qoi], sum_H[qoi], sum_LL_a[qoi],
			     sum_LH_a[qoi], sum_HH[qoi], N_L_a[qoi], N_H[qoi],
			     N_LH_a[qoi], var_H[qoi], rho2_LH_a[qoi]);
  }

  // precompute a factor based on most-correlated model
  RealVector factor(numFunctions, false);
  size_t num_am1 = numApprox - 1;
  for (qoi=0; qoi<numFunctions; ++qoi)
    factor[qoi] = cost_H / (1. - rho2_LH(qoi, num_am1));

  // second sweep to compute eval_ratios including rho2 look-{ahead,back}
  for (approx=0; approx<numApprox; ++approx) {
    Real* eval_ratios_a = eval_ratios[approx];
    Real*     rho2_LH_a =     rho2_LH[approx];
    Real         cost_L =        cost[approx];
    // TO DO: MUST PROTECT AGAINST sqrt(negative) BY SEQUENCING CORRELATIONS
    // NOTE: indexing is inverted from Peherstorfer: HF = 1, MF = 2, LF = 3
    // > i+1 becomes i-1 and most correlated ref is rho2_LH(qoi, num_am1)
    if (approx)
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L *
	  (rho2_LH_a[qoi] - rho2_LH(qoi, approx-1)));
    else // rho2_LH for approx-1 (non-existent model) is zero
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L * rho2_LH_a[qoi]);

    if (outputLevel >= DEBUG_OUTPUT)
      for (qoi=0; qoi<numFunctions; ++qoi)
    	Cout << "Approx " << approx+1 << " QoI " << qoi+1 << ": cost_ratio = "
	     << cost_H / cost_L << " rho_sq = " << rho2_LH_a[qoi]
	     << " eval_ratio = " << eval_ratios_a[qoi] << std::endl;
  }
}


void NonDACVSampling::
compute_MSE_ratios(const RealMatrix& eval_ratios, const RealMatrix& rho2_LH,
		   const RealVector& cost, RealVector& mse_ratios)
{
  size_t qoi, approx;
  if (mse_ratios.empty()) mse_ratios.sizeUninitialized(numFunctions);

  // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
  // > Estimator Var for MC = var_H / N_H = MSE (neglect HF bias)
  // > Estimator Var for MFMC = var_H (1-rho_LH(am1)^2) p / N_H^2 cost_H
  //   where budget p = cost^T eval_ratios N_H,  am1 = most-correlated approx
  //   --> EstVar = var_H (1-rho_LH(am1)^2) cost^T eval_ratios / N_H cost_H
  // > MSE ratio = EstVar_MFMC / EstVar_MC
  //   = (1-rho_LH(am1)^2) cost^T eval_ratios / cost_H

  Real cost_H = cost[numApprox], inner_prod;  size_t num_am1 = numApprox - 1;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // include cost_H * w_H
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * eval_ratios(qoi, approx); // cost_i * w_i
    mse_ratios[qoi] = (1. - rho2_LH(qoi, num_am1)) * inner_prod / cost_H;
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "QoI " << qoi+1 << ": MFMC variance reduction factor = "
	   << mse_ratios[qoi] << '\n';
  }
}


void NonDACVSampling::
cv_raw_moments(IntRealMatrixMap& sum_L_shared,  IntRealVectorMap& sum_H,
	       IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
	       IntRealMatrixMap& sum_L_refined, const RealMatrix& rho2_LH,
	       const Sizet2DArray& N_L,         const SizetArray& N_H,
	       const Sizet2DArray& N_LH,        RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  Real beta, sum_H_mq;
  size_t approx, qoi, N_L_aq, N_H_q, N_LH_aq, N_shared;
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix& sum_L_sh_m  = sum_L_shared[mom];
    RealVector& sum_H_m     = sum_H[mom];
    RealMatrix& sum_LL_m    = sum_LL[mom];
    RealMatrix& sum_LH_m    = sum_LH[mom];
    RealMatrix& sum_L_ref_m = sum_L_refined[mom];

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << ":\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      sum_H_mq = sum_H_m[qoi];  N_H_q = N_H[qoi];
      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	N_L_aq = N_L[approx][qoi];  N_LH_aq = N_LH[approx][qoi];
	compute_mf_control(sum_L_sh_m(qoi,approx), sum_H_mq,
			   sum_LL_m(qoi,approx), sum_LH_m(qoi,approx),
			   /*N_L_sh_aq*/N_H_q, N_H_q, N_LH_aq, beta); //shared
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1
	       << ": control variate beta = " << std::setw(9) << beta << '\n';
	// shared accumulators and counts must telescope
	N_shared = (approx == numApprox-1) ? N_H_q : N_L[approx+1][qoi];
	apply_mf_control(sum_L_sh_m(qoi,approx),  N_shared, // shared
			 sum_L_ref_m(qoi,approx), N_L_aq,  // refined
			 beta, H_raw_mq);
      }
    }
  }
}

} // namespace Dakota
