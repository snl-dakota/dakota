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
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#elif HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  pilotSamples(problem_db.get_sza("method.nond.pilot_samples")),
  randomSeedSeqSpec(problem_db.get_sza("method.random_seed_sequence")),
  mlmfIter(0),
  //allocationTarget(problem_db.get_short("method.nond.allocation_target")),
  //qoiAggregation(problem_db.get_short("method.nond.qoi_aggregation")),
  exportSampleSets(problem_db.get_bool("method.nond.export_sample_sequence")),
  exportSamplesFormat(
    problem_db.get_ushort("method.nond.export_samples_format"))
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
  if (iteratedModel.surrogate_type() == "non_hierarchical")
    aggregated_models_mode(); // truth model + all approx models
  else {
    Cerr << "Error: Non-hierarchical sampling requires a non-hierarchical "
         << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  size_t i, num_mf = model_ensemble.size(), num_lev;
  ModelLRevIter ml_it; bool err_flag = false;
  NLev.resize(num_mf);
  for (i=0, ml_it=model_ensemble.begin(); ml_it!=model_ensemble.end();
       ++i, ++ml_it) {
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev = ml_rit->solution_levels(); // lower bound is 1 soln level

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    // > For ACV, we will only require 1 solution cost, neglecting resolutions
    //   for now
    //if (num_lev > ml_rit->solution_levels(false)) { // 
    if (ml_rit->solution_levels(false) == 0) { // default is 0 soln costs
      Cerr << "Error: insufficient cost data provided for ACV sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_rit->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  size_t pilot_size = pilotSamples.size();
  switch (pilot_size) {
  case 0: maxEvalConcurrency *= 100;             break;
  //case 1: maxEvalConcurrency *= pilotSamples[0]; break;
  default: {
    size_t max_ps = find_max(pilotSamples);
    if (max_ps)
      maxEvalConcurrency *= max_ps;
    break;
  }
  }
}


NonDACVSampling::~NonDACVSampling()
{ }


bool NonDACVSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


/*
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


void NonDACVSampling::
configure_indices(size_t group, size_t form, size_t lev, short seq_type)
{
  // Notes:
  // > could consolidate with NonDExpansion::configure_indices() with a passed
  //   model and virtual *_mode() assignments.  Leaving separate for now...
  // > group index is assigned based on step in model form/resolution sequence
  // > MFMC does not use this helper; it requires uncorrected_surrogate_mode()

  // preserve special values across type conversions
  unsigned short grp = (group == SZ_MAX) ? USHRT_MAX : (unsigned short)group,
                 frm = (form  == SZ_MAX) ? USHRT_MAX : (unsigned short)form;
  Pecos::ActiveKey hf_key;  hf_key.form_key(grp, frm, lev);

  if ( (seq_type == Pecos::MODEL_FORM_SEQUENCE       && form == 0) ||
       (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE && lev  == 0)) {
    // step 0 in the sequence
    bypass_surrogate_mode();
    iteratedModel.active_model_key(hf_key);      // one active fidelity
  }
  else {
    aggregated_models_mode();

    // TO DO
    //Pecos::ActiveKey lf_key(hf_key.copy()), agg_key;
    //lf_key.decrement_key(seq_type); // seq_index defaults to 0
    // For MLMC/MFMC/MLMFMC, we aggregate levels but don't reduce them
    //agg_key.aggregate_keys(hf_key, lf_key, Pecos::RAW_DATA);
    //iteratedModel.active_model_key(agg_key); // two active fidelities
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
  iteratedModel.multifidelity_precedence(true);//_enforcement(); *** ???

  switch (acvAlgorithmOption) {
  case MULTIFIDELITY_MC:
    //sequence_models(); // enforce correlation condition...
    multifidelity_mc(); break; // Peherstorfer, Willcox, Gunzburger, 2016
  case ACV_MF:
    approx_control_variate_multifidelity(); break;
  case ACV_IS:
  case ACV_KL:
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

  size_t num_steps, secondary_index;  short seq_type;
  configure_sequence(num_steps, secondary_index, seq_type); // MF if #models > 1
  bool multilev = (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE);//(false);
  size_t i, num_approx = num_steps - 1; //, qoi, form, lev;

  // For M-model control variate, select fidelities/resolutions
  Pecos::ActiveKey active_key, truth_key;
  std::vector<Pecos::ActiveKey> approx_keys(num_approx);
  //unsigned short truth_form;  size_t truth_lev;
  if (multilev) {
    fixed_form = (secondary_index == SZ_MAX) ? USHRT_MAX : secondary_index;
    truth_key.form_key(0, fixed_form, num_approx);
    for (i=0; i<num_approx; ++i)
      approx_keys[i].form_key(0, fixed_form, i);
    //truth_form = fixed_form;  truth_lev = num_approx;
  }
  else {
    truth_key.form_key(0, num_steps-1, secondary_index);
    for (i=0; i<num_approx; ++i)
      approx_keys[i].form_key(0, i, secondary_index);
    //truth_form = num_steps-1;  truth_lev = secondary_index;
  }
  active_key.aggregate_keys(truth_key, approx_keys, Pecos::RAW_DATA);
  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0

  // 1D sequence: either lev varies and form is fixed, or vice versa:
  //size_t& step = (multilev) ? lev : form;
  //if (multilev) form = secondary_index;
  //else          lev  = secondary_index;

  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost;  configure_cost(num_steps, multilev, cost);
  SizetArray raw_N; raw_N.assign(num_steps, 0);

  IntRealVectorMap sum_H;
  IntRealMatrixMap sum_L_shared, sum_L_refined, sum_LL, sum_LH;
  initialize_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false), mu_hat;
  RealMatrix rho2_LH(numFunctions, false), eval_ratios, mse_ratios;
  SizetArray N_H, delta_N; Sizet2DArray N_L(num_approx), N_LH(num_approx);
  N_H.assign(numFunctions, 0);
  for (i=0; i<num_approx; ++i)
    { N_L[i].assign(numFunctions, 0); N_LH[i].assign(numFunctions, 0); }

  // Initialize for pilot sample
  load_pilot_sample(pilotSamples, num_steps, delta_N);
  size_t truth_sample_incr = delta_N[num_approx]; // last in array
  numSamples = truth_sample_incr;

  mlmfIter = 0;  equivHFEvals = 0.;
  while (truth_sample_incr && mlmfIter <= maxIterations &&
	 equivHFEvals <= maxFunctionEvals) {

    // ----------------------------------------------------------
    // Compute shared increment targeting specified MSE reduction
    // ----------------------------------------------------------
    if (mlmfIter) {
      // CV MSE target = convTol * mcMSEIter0 = mse_ratio * var_H / N_truth
      // N_truth = mse_ratio * var_H / convTol / mcMSEIter0
      // Note: don't simplify further since mcMSEIter0 is based on pilot
      //       estimate of var_H / N_truth
      RealVector truth_targets = mse_ratios; // 1st portion of expression
      for (qoi=0; qoi<numFunctions; ++qoi)
	truth_targets[qoi] *= var_H[qoi] / mcMSEIter0[qoi] / convergenceTol;
      // Power mean choice: average, max (desire would be to balance overshoot
      // vs. additional iteration)
      truth_sample_incr = numSamples = one_sided_delta(N_truth,truth_targets,1);
    }

    if (numSamples) {
      shared_increment(); // spans ALL models
      accumulate_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			 sum_HH, mu_hat, N_L, N_H, N_LH);
      increment_mf_samples(numSamples, 0, num_steps, raw_N);
      increment_mf_equivalent_cost(numSamples, 0, num_steps, cost);

      // Compute the LF/HF evaluation ratio using shared samples, averaged
      // over QoI.  This includes updating var_H and rho2_LH.
      compute_eval_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, cost_ratio, N_truth, var_H, rho2_LH,
			  eval_ratios);
      // Compute the ratio of MC and CVMC mean squared errors (for convergence).
      // This ratio incorporates the anticipated variance reduction from the
      // upcoming application of eval_ratios.
      if (mlmfIter == 0) compute_MSE(var_H, N_truth, mcMSEIter0);
      compute_MSE_ratios(eval_ratios, var_H, rho2_LH, mlmfIter, N_truth,
			 mse_ratios);

      // -------------------------------------------------------------------
      // Compute N_approx increments based on new eval ratio for new N_truth
      // -------------------------------------------------------------------
      for (i=0; i<num_approx; ++i) {
	// approx_increment() spans models {i, i+1, ..., #approx}
	if (equivHFEvals <= maxFunctionEvals &&
	    approx_increment(i, eval_ratios, N_approx, N_truth, mlmfIter, 0)) {
	  accumulate_mf_sums(sum_L_refined, mu_hat, N_L);
	  increment_mf_samples(numSamples, i, num_approx, raw_N);
	  increment_mf_equivalent_cost(numSamples, i, num_approx, cost);
	}
      }
    }
    //Cout << "\nMFMC iteration " << mlmfIter << " complete." << std::endl;
    ++mlmfIter;
  } // end while

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_truth, sum_L_refined,
		 N_approx, rho2_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
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

  // 1D sequence: either lev varies and form is fixed, or vice versa:
  size_t& step = (multilev) ? lev : form;
  if (multilev) form = secondary_index;
  else          lev  = secondary_index;

  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost; configure_cost(num_steps, multilev, cost);

  // Initialize for pilot sample
  SizetArray delta_N;
  load_pilot_sample(pilotSamples, num_steps, delta_N);
  size_t hf_sample_incr = delta_N[hf_form];
  numSamples = hf_sample_incr;

  mlmfIter = 0;  equivHFEvals = 0.;
  while (hf_sample_incr && mlmfIter <= maxIterations &&
	 equivHFEvals <= maxFunctionEvals) {

    // ??? MAY NOT REQUIRE LEVEL STEPPING ???
    
    shared_increment(); // spans models {i, i+1, ..., M}
    // to support with HierarchSurrModel, might be simplest to sample one model 
    // at a time using a nested sample set...

    ++mlmfIter;
    // compute the equivalent number of HF evaluations
    increment_mf_equivalent_cost(raw_N_l, cost);
  } // end while

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_truth, sum_L_refined,
		 N_approx, rho2_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
}


void NonDACVSampling::
shared_increment(/*const Pecos::ActiveKey& agg_key,*/ size_t iter, size_t lev)
{
  if (iter == _NPOS)  Cout << "\nMFMC sample increments: ";
  else if (iter == 0) Cout << "\nMFMC pilot sample: ";
  else Cout << "\nMFMC iteration " << iter << " sample increments: ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

    UShortArray asrv; asrv.assign(iteratedModel.response_size(), 1);
    ensemble_sample_increment(asrv, iter, lev); // BLOCK
  }
}


bool NonDACVSampling::
approx_increment(const RealVector& eval_ratios, const SizetArray& N_lf,
		 const SizetArray& N_hf, size_t iter, size_t lev)
{
  // update LF samples based on evaluation ratio
  //   r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  //   or with inverse r  -> delta = m-n = n/inverse_r - n
  RealVector lf_targets(numFunctions, false);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * N_hf[qoi];
  // Choose average, RMS, max of difference?
  // Trade-off: Possible overshoot vs. more iteration...
  numSamples = one_sided_delta(N_lf, lf_targets, 1); // average

  if (numSamples) Cout << "\nMFMC approx sample increment = " << numSamples;
  else            Cout << "\nNo MFMC approx sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from avg LF = " << average(N_lf) << ", avg HF = "
	 << average(N_hf) << ", avg eval_ratio = " << average(eval_ratios);
  Cout << std::endl;

  return (numSamples) ? ensemble_sample_increment(asrv, iter, lev) // NON-BLOCK
    : false;
}


void NonDACVSampling::
ensemble_sample_increment(const UShortarray& asrv, size_t iter, size_t lev)
{
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set:
  if (exportSampleSets) { // for HF+LF models, use the HF tags
    export_all_samples("cv_", iteratedModel.truth_model(), iter, lev);
    for ()
      export_all_samples("cv_", iteratedModel.surrogate_model(i), iter, lev);
  }

  // compute allResponses from allVariables using hierarchical model
  iteratedModel.active_set_request_vector(asrv); // composite ASV
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
    empty_pr.first = i; // moment number
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to size Real{Vector,Matrix} in place and init sums to 0
    sum_L_shared.insert(mat_pr).first->second.shape(numFunctions, num_approx);
    sum_L_refined.insert(mat_pr).first->second.shape(numFunctions, num_approx);
    sum_H.insert(vec_pr).first->second.size(numFunctions);
    sum_LL.insert(mat_pr).first->second.shape(numFunctions, num_approx);
  //sum_HH.insert(vec_pr).first->second.size(numFunctions);
    sum_LH.insert(mat_pr).first->second.shape(numFunctions, num_approx);
  }
}


void NonDACVSampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L, const RealVector& offset,
		   Sizet2DArray& num_L)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  Real fn_val, prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRVMIter l_it;
  bool os = !offset.empty();
  size_t fn_index, approx, num_approx = unorderedModels.size();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();
    fn_index = 0;

    // accumulate for leading set of models (omit trailing truth),
    // but note that resp and asv are full aggregated length
    for (approx=0; approx<num_approx; ++approx) {

      SizetArray& num_L_a = num_L[approx];
      for (qoi=0; qoi<numFunctions; ++qoi, ++fn_index) {
	if (asv[fn_index] & 1) {
	  prod = fn_val = (os) ? fn_vals[fn_index] - offset[fn_index]
	                       : fn_vals[fn_index];
	  if (isfinite(fn_val)) { // neither NaN nor +/-Inf
	    ++num_L_a[qoi]; // number of recovered response observations

	    l_it = sum_L.begin(); ord = l_it->first; active_ord = 1;
	    while (l_it!=sum_L.end()) {
    
	      if (ord == active_ord) { // support general key sequence
		l_it->second(qoi,approx) += prod; ++l_it;
		ord = (l_it == sum_L.end()) ? 0 : l_it->first;
	      }

	      prod *= fn_val; ++active_ord;
	    }
	  }
	}
      }
    }
  }
}


void NonDACVSampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		   RealVector& sum_HH, const RealVector& offset,
		   Sizet2DArray& num_L, SizetArray& num_H, Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:num_approx --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter h_it; IntRMMIter ls_it, lr_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord;
  size_t qoi, lf_index, hf_index, num_approx = unorderedModels.size();
  bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    const ShortArray& asv     = resp.active_set_request_vector();
    hf_index = num_approx * numFunctions;

    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = (os) ? fn_vals[hf_index] - offset[hf_index] : fn_vals[hf_index];
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
	
      for (approx=0; approx<num_approx; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = (os) ? fn_vals[lf_index] - offset[lf_index] : fn_vals[lf_index];

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
		    const RealVector& sum_HH, Real cost_ratio,
		    const Sizet2DArray& N_L, const SizetArray& N_H,
		    const Sizet2DArray& N_LH, RealVector& var_H,
		    RealMatrix& rho2_LH, RealMatrix& eval_ratios)
{
  size_t approx, num_approx = unorderedModels.size(), qoi;
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, num_approx);
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, num_approx);

  //Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (approx=0; approx<num_approx; ++approx) {

    SizetArray& N_L_a         =         N_L[approx];
    SizetArray& N_LH_a        =        N_LH[approx];
    const Real* rho2_LH_a     =     rho2_LH[approx];
    const Real* eval_ratios_a = eval_ratios[approx];
    for (qoi=0; qoi<numFunctions; ++qoi) {

      Real& rho_sq = rho2_LH_a[qoi];
      compute_mf_correlation(sum_L_shared(qoi,approx), sum_H[qoi],
			     sum_LL(qoi,approx), sum_LH(qoi,approx),
			     sum_HH[qoi], N_L_a[qoi], N_H[qoi], N_LH_a[qoi],
			     var_H[qoi], rho_sq);

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    // Given use of 1/r in MSE_ratio, one approach would average 1/r, but
    // this does not seem to behave as well in limited numerical experience.
    //if (rho_sq > Pecos::SMALL_NUMBER) {
    //  avg_inv_eval_ratio += std::sqrt((1. - rho_sq)/(cost_ratio * rho_sq));
    if (rho_sq < 1.) { // protect against division by 0, sqrt(negative)
      eval_ratios_a[qoi] = std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "evaluation_ratios() QoI " << qoi+1 << ": cost_ratio = "
	     << cost_ratio << " rho_sq = " << rho_sq << " eval_ratio = "
	     << eval_ratios_a[qoi] << std::endl;
      //avg_eval_ratio += eval_ratios[qoi];
      //++num_avg;
    }
    else // should not happen, but provide a reasonable upper bound
      eval_ratios_a[qoi] = (Real)maxFunctionEvals / average(N_shared);
  }
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "variance of HF Q:\n" << var_H;

  //if (num_avg) avg_eval_ratio /= num_avg;
  //else         avg_eval_ratio  = (Real)maxFunctionEvals / average(N_shared);
  //return avg_eval_ratio;
}


void NonDACVSampling::
compute_MSE_ratios(const RealMatrix& eval_ratios, const RealVector& var_H,
		   const RealMatrix& rho2_LH, size_t iter,
		   const SizetArray& N_hf, RealMatrix& mse_ratios)
{
  size_t qoi, approx, num_approx = ;
  //Real curr_mc_mse, curr_cvmc_mse, curr_mse_ratio, avg_mse_ratio = 0.;
  if (mse_ratios.empty())
    mse_ratios.shapeUninitialized(numFunctions, num_approx);
  for (approx=0; approx<num_approx; ++approx) {

    const Real* rho2_LH_a     =     rho2_LH[approx];
    const Real* eval_ratios_a = eval_ratios[approx];
    Real*        mse_ratios_a =  mse_ratios[approx];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
      // > Estimator Var for MC = sigma_hf^2 / N_hf = MSE (neglect HF bias)
      // > Estimator Var for CV = (1+r/w) [1-rho^2(1-1/r)] sigma_hf^2 / p
      //   where p = (1+r/w)N_hf -> Var = [1-rho^2(1-1/r)] sigma_hf^2 / N_hf
      // MSE ratio = Var_CV / Var_MC = [1-rho^2(1-1/r)]
      mse_ratios_a[qoi] = 1. - rho2_LH_a[qoi] * (1. - 1. / eval_ratios_a[qoi]);
      Cout << "QoI " << qoi+1 << ": CV variance reduction factor = "
	   << mse_ratios_a[qoi] << " for eval ratio " << eval_ratios_a[qoi]
	   << '\n';
    }
  }
}


void NonDACVSampling::assign_specification_sequence(size_t index)
{
  // Note: seedSpec/randomSeed initialized from randomSeedSeqSpec in ctor

  // advance any sequence specifications, as admissible
  // Note: no colloc pts sequence as load_pilot_sample() handles this separately
  int seed_i = random_seed(index);
  if (seed_i) randomSeed = seed_i;// propagate to NonDSampling::initialize_lhs()
  // else previous value will allow existing RNG to continue for varyPattern
}


void NonDACVSampling::
export_all_samples(String root_prepend, const Model& model, size_t iter,
		   size_t lev)
{
  String tabular_filename(root_prepend);
  const String& iface_id = model.interface_id();
  size_t i, num_samp = allSamples.numCols();
  if (iface_id.empty()) tabular_filename += "NO_ID_i";
  else                  tabular_filename += iface_id + "_i";
  tabular_filename += std::to_string(iter)     +  "_l"
                   +  std::to_string(lev)      +  '_'
                   +  std::to_string(num_samp) + ".dat";
  Variables vars(model.current_variables().copy());

  String context_message("NonDACVSampling::export_all_samples");
  StringArray no_resp_labels;
  String cntr_label("sample_id"), interf_label("interface");

  // Rather than hard override, rely on output_precision user spec
  //int save_wp = write_precision;
  //write_precision = 16; // override
  std::ofstream tabular_stream;
  TabularIO::open_file(tabular_stream, tabular_filename, context_message);
  TabularIO::write_header_tabular(tabular_stream, vars, no_resp_labels,
				  cntr_label, interf_label,exportSamplesFormat);
  for (i=0; i<num_samp; ++i) {
    sample_to_variables(allSamples[i], vars); // NonDSampling version
    TabularIO::write_data_tabular(tabular_stream, vars, iface_id, i+1,
				  exportSamplesFormat);
  }

  TabularIO::close_file(tabular_stream, tabular_filename, context_message);
  //write_precision = save_wp; // restore
}


void NonDACVSampling::post_run(std::ostream& s)
{
  // Final moments are generated within core_run() by convert_moments().
  // No addtional stats are currently supported.
  //if (statsFlag) // calculate statistics on allResponses
  //  compute_statistics(allSamples, allResponses);

  // NonD::update_aleatory_final_statistics() pushes momentStats into
  // finalStatistics
  update_final_statistics();

  Analyzer::post_run(s);
}


void NonDACVSampling::print_results(std::ostream& s, short results_state)
{
  if (statsFlag) {
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";

  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
    archive_moments();
    archive_equiv_hf_evals(equivHFEvals); 
  }
}

} // namespace Dakota
