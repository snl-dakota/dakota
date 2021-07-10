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

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {

// initialization of statics
NonDACVSampling* NonDACVSampling::acvInstance(NULL);


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDEnsembleSampling(problem_db, model),
  acvSubMethod(problem_db.get_ushort("method.sub_method")),
  optSubProblemForm(R_AND_N_NONLINEAR_CONSTRAINT) // *** TO DO: also support option for fixed N_H ("fixed_truth_evaluations" or similar) + equivHF budget
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

  size_t num_steps;
  configure_sequence(num_steps, secondaryIndex, sequenceType);
  numApprox = num_steps - 1;
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  configure_cost(num_steps, multilev, sequenceCost);

  // Use NPSOL/OPT++ with obj/constr callbacks to minmize estimator variance
  // > TO DO: add sqp | nip specification
  if (methodName != MULTIFIDELITY_SAMPLING) { // ACV_*

    // Notes on optimization:
    // > Budget C is fixed and N_H is fixed for this iteration --> design vars
    //   are eval_ratios for 1:numApprox.  eval_ratio lower bounds are set to 1
    //   for now, but could also reflect the pilot sample investment (C and c
    //   can use different pilot sample sizes).
    //   >> if N_H were also a design variable, then lower bounds should be 1.
    //   >> when an optimal ratio is 1, this model drops from apply_control(),
    //      although it may still influence the other model weightings.
    // > a linear inequality is used for the cost constraint and can also be
    //   used for eval_ratio(i) > eval_ratio(i+1), but omit for now (restricts
    //   optimizer search space = most appropriate when sequencing models)
    RealVector lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
               nln_ineq_lb, nln_ineq_ub, nln_eq_tgt;
    RealMatrix lin_ineq_coeffs, lin_eq_coeffs;
    size_t num_cdv;  int deriv_level;
    switch (optSubProblemForm) {
    case R_ONLY_LINEAR_CONSTRAINT:
      num_cdv = numApprox; // r-only: evaluation ratios
      deriv_level = 0; // no user-supplied derivatives
      lin_ineq_lb.sizeUninitialized(1); lin_ineq_lb[0] = -DBL_MAX; // no low bnd
      lin_ineq_ub.sizeUninitialized(1); lin_ineq_ub[0] = (Real)maxFunctionEvals;
      lin_ineq_coeffs.shapeUninitialized(1, numApprox);
      lin_ineq_coeffs = 1.; // updated in compute_ratios()
      break;
    case R_AND_N_NONLINEAR_CONSTRAINT:
      num_cdv = numApprox + 1; // evaluation ratios and N_H
      deriv_level = 2; // user-supplied constraint Jacobian
      nln_ineq_lb.sizeUninitialized(1); nln_ineq_lb[0] = -DBL_MAX; // no low bnd
      nln_ineq_ub.sizeUninitialized(1); nln_ineq_ub[0] = (Real)maxFunctionEvals;
      break;
    }
    RealVector x0(num_cdv), x_lb(num_cdv), x_ub(num_cdv);
    x0   = 1.; // updated in compute_ratios() with warm start || MFMC init guess
    x_lb = 1.; x_ub = DBL_MAX; // no upper bounds

    switch (sub_optimizer_select(
	    probDescDB.get_ushort("method.nond.opt_subproblem_solver"))) {
    case SUBMETHOD_SQP: {
      Real conv_tol = -1.; // use NPSOL default
#ifdef HAVE_NPSOL
      varianceMinimizer.assign_rep(std::make_shared<NPSOLOptimizer>(x0, x_lb,
	x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	npsol_objective_evaluator, npsol_constraint_evaluator,
	deriv_level, conv_tol));
#endif
      break;
    }
    case SUBMETHOD_NIP:
#ifdef HAVE_OPTPP
      varianceMinimizer.assign_rep(std::make_shared<SNLLOptimizer>(x0,x_lb,x_ub,
	lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs, lin_eq_tgt,
	nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, optpp_objective_evaluator,
	optpp_constraint_evaluator));
#endif
      break;
    default: // SUBMETHOD_NONE, ...
      err_flag = true; break;
    }
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
*/


void NonDACVSampling::pre_run()
{
  NonDEnsembleSampling::pre_run();

  // reset sample counters to 0
  acvInstance = this;
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDACVSampling::core_run()
{
  // prefer MF over ML if both available
  iteratedModel.multifidelity_precedence(true);
  // assign an aggregate model key that persists for core_run()
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  assign_active_key(numApprox+1, secondaryIndex, multilev);

  switch (methodName) {
  case MULTIFIDELITY_SAMPLING: // Peherstorfer, Willcox, Gunzburger, 2016
    //sequence_models(); // enforce correlation condition (*** AFTER PILOT ***)
    multifidelity_mc();            break;
  case APPROXIMATE_CONTROL_VARIATE:
    switch (acvSubMethod) {
    case SUBMETHOD_ACV_IS:  case SUBMETHOD_ACV_MF:
      approximate_control_variate(); break;
    //case SUBMETHOD_ACV_KL:
    //for (k) for (l) approximate_control_variate(...); ???
    }
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

  IntRealVectorMap sum_H;
  IntRealMatrixMap sum_L_shared, sum_L_refined, sum_LL, sum_LH;
  RealVector sum_HH, var_H, mse_iter0, mse_ratios, hf_targets;
  RealMatrix rho2_LH, eval_ratios;
  SizetArray N_H, delta_N; Sizet2DArray N_L_shared, N_L_refined, N_LH;
  size_t num_steps = numApprox + 1;
  initialize_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,sum_HH);
  initialize_mf_counts(N_L_shared, N_L_refined, N_H, N_LH);

  // Initialize for pilot sample
  load_pilot_sample(pilotSamples, num_steps, delta_N);
  numSamples = delta_N[numApprox]; // last in array

  while (numSamples && mlmfIter <= maxIterations) {

    // ------------------------------------------------------------------------
    // Compute shared increment targeting specified budget and/or MSE reduction
    // ------------------------------------------------------------------------
    // Scale sample profile based on maxFunctionEvals or convergenceTol,
    // but not both (for now)
    if (mlmfIter)
      update_hf_targets(eval_ratios, sequenceCost, mse_ratios, var_H, N_H,
			mse_iter0, hf_targets);

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,MSE}_ratios
    // --------------------------------------------------------------------
    if (numSamples) {
      shared_increment(mlmfIter); // spans ALL models, blocking
      accumulate_mf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			 sum_HH, N_L_shared, N_L_refined, N_H, N_LH);
      increment_equivalent_cost(numSamples, sequenceCost, 0, num_steps);

      // First, compute the LF/HF evaluation ratio using shared samples,
      // averaged over QoI.  This includes updating var_H and rho2_LH.  Then,
      // compute the ratio of MC and ACV mean squared errors (for convergence).
      // This ratio incorporates the anticipated variance reduction from the
      // upcoming application of eval_ratios.
      compute_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1], sum_HH,
		     sequenceCost, N_L_shared, N_H, N_LH, var_H, rho2_LH,
		     eval_ratios, mse_ratios);
      // mse_iter0 only uses HF pilot since sum_L_shared / N_shared minus
      // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
      // (This differs from MLMC MSE^0 which uses pilot for all levels.)
      if (mlmfIter == 0) compute_mc_estimator_variance(var_H, N_H, mse_iter0);
    }
    //else
    //  Cout << "\nMFMC iteration " << mlmfIter
    //       << ": no shared sample increment" << std::endl;

    ++mlmfIter;
  }

  // ---------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ---------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after N_H has converged, which simplifies maxFnEvals / convTol logic
  // (no need to further interrogate these throttles below)

  // Pyramid/nested sampling: at step i, we sample approximation range
  // [0,numApprox-1-i] using the delta relative to the previous step
  for (size_t approx=numApprox; approx>0; --approx) {
    // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER 1ST PASS LAUNCH
    if (approx_increment(eval_ratios, N_L_refined, hf_targets, mlmfIter,
			 0, approx)) {
      // MFMC   samples on [0, approx) --> sum_L_{shared,refined}
      accumulate_mf_sums(sum_L_shared, sum_L_refined, N_L_shared, N_L_refined,
			 0, approx);
      increment_equivalent_cost(numSamples, sequenceCost, 0, approx);
    }
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  mfmc_raw_moments(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,//rho2_LH,
		   N_L_shared, N_L_refined, N_H, N_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);

  // post final sample counts back to NLev (needed for final eval summary)
  N_L_refined.push_back(N_H); // aggregate into a single Sizet2DArray
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  inflate_final_samples(N_L_refined, multilev, secondaryIndex, NLev);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate()
{
  // Performs pilot + LF increment and then iterates with additional shared
  // increment + LF increment batches until prescribed MSE reduction is obtained

  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;
  IntRealMatrixMap sum_L_shared, sum_L_refined, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, mse_iter0, avg_eval_ratios;
  Real avg_hf_target, mse_ratio;  size_t num_steps = numApprox + 1;
  SizetArray N_H, delta_N;  Sizet2DArray N_L_shared, N_L_refined, N_LH;
  SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL,sum_LH,sum_HH);
  initialize_acv_counts(N_L_shared, N_L_refined, N_H, N_LL, N_LH);
  //initialize_acv_covariances(covLL, covLH, varH);

  // Initialize for pilot sample
  size_t hf_shared_pilot, lf_shared_pilot, start = 0;
  load_pilot_sample(pilotSamples, num_steps, delta_N);
  numSamples = hf_shared_pilot = delta_N[numApprox]; // last in array
  lf_shared_pilot = find_min(delta_N, start, numApprox-1);

  while (numSamples && mlmfIter <= maxIterations) {

    // ------------------------------------------------------------------------
    // Compute shared increment targeting specified budget and/or MSE reduction
    // ------------------------------------------------------------------------
    // Scale sample profile based on maxFunctionEvals or convergenceTol,
    // but not both (for now)
    if (mlmfIter)
      switch (optSubProblemForm) {
      case R_ONLY_LINEAR_CONSTRAINT:
	// Allow for constraint to be inactive at optimum, but generally the
	// opt sub-prob will allocate full budget (--> numSamples = deltaN = 0)
	// since larger eval ratios will increase R^2.
	// Important: r* for fixed N is suboptimal w.r.t. r*,N* --> unlike MFMC,
	// this approach does not converge to the desired sample profile, except
	// for special case where the user specifies a fixed N_H + total budget.
	update_hf_target(avg_eval_ratios, sequenceCost, mse_ratio, varH, N_H,
			 mse_iter0, avg_hf_target);
	break;
      case R_AND_N_NONLINEAR_CONSTRAINT:
	// In this case, the opt-solution for N* for prescribed budget induces
	// a one-sided shared_increment and this continues until conv (shared
	// increment = 0).  Final r*'s are then applied in approx_increment's.
	//numSamples = one_sided_delta(); // performed in compute_ratios()
	break;
      }

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,MSE}_ratios
    // --------------------------------------------------------------------
    if (numSamples) {
      shared_increment(mlmfIter); // spans ALL models, blocking
      accumulate_acv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			  sum_HH, N_L_shared, N_L_refined, N_H, N_LL, N_LH);
      increment_equivalent_cost(numSamples, sequenceCost, 0, num_steps);
      if (lf_shared_pilot > hf_shared_pilot) {// allow pilot to vary for C vs c
	numSamples = lf_shared_pilot - hf_shared_pilot;
	shared_approx_increment(mlmfIter); // spans all approx models
	accumulate_acv_sums(sum_L_shared, sum_L_refined, sum_LL/*_shared, sum_LL_refined*/, N_L_shared, N_L_refined, N_LL/*_shared, N_LL_refined*/); // ***
	increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox);
      }

      // First, compute the LF/HF evaluation ratio using shared samples,
      // averaged over QoI.  This includes updating varH, covLL, covLH.
      // Then, compute ratio of MC and ACV mean sq errors (for convergence),
      // which incorporates the anticipated variance reduction from the
      // upcoming application of avg_eval_ratios.
      compute_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1], sum_HH,
		     sequenceCost, N_L_shared, N_H, N_LL, N_LH, avg_eval_ratios,
		     mse_ratio);
      // mse_iter0 only uses HF pilot since sum_L_shared / N_shared minus
      // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
      // (This differs from MLMC MSE^0 which uses pilot for all levels.)
      // Note: could revisit this for case of lf_shared_pilot > hf_shared_pilot.
      if (mlmfIter == 0) compute_mc_estimator_variance(varH, N_H, mse_iter0);
    }
    //else
    //  Cout << "\nMFMC iteration " << mlmfIter
    //       << ": no shared sample increment" << std::endl;

    ++mlmfIter;
  }

  // ---------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final N_H
  // ---------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after N_H has converged, which simplifies maxFnEvals / convTol logic
  // (no need to further interrogate these throttles below)

  // Pyramid/nested sampling: at step i, we sample approximation range
  // [0,numApprox-1-i] using the delta relative to the previous step
  for (size_t approx=numApprox; approx>0; --approx) {
    // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER 1ST PASS LAUNCH
    start = (acvSubMethod == SUBMETHOD_ACV_IS) ? approx - 1 : 0;
    if (approx_increment(avg_eval_ratios, N_L_refined, avg_hf_target, mlmfIter,
			 start, approx)) {
      // ACV_IS samples on [approx-1,approx) --> sum_L_refined
      // ACV_MF samples on [0, approx)       --> sum_L_refined
      accumulate_acv_sums(sum_L_refined, N_L_refined, start, approx);
      increment_equivalent_cost(numSamples, sequenceCost, start, approx);
    }
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  acv_raw_moments(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		  avg_eval_ratios, N_L_shared, N_L_refined, N_H, N_LL,
		  N_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);

  // post final sample counts back to NLev (needed for final eval summary)
  N_L_refined.push_back(N_H); // aggregate into a single Sizet2DArray
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  inflate_final_samples(N_L_refined, multilev, secondaryIndex, NLev);
}


void NonDACVSampling::
assign_active_key(size_t num_steps, size_t secondary_index, bool multilev)
{
  // For M-model control variate, select fidelities/resolutions
  Pecos::ActiveKey active_key, truth_key;
  std::vector<Pecos::ActiveKey> approx_keys(numApprox);
  //unsigned short truth_form;  size_t truth_lev;
  if (multilev) {
    unsigned short fixed_form = (secondary_index == SZ_MAX) ?
      USHRT_MAX : secondary_index;
    truth_key.form_key(0, fixed_form, numApprox);
    for (size_t approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, fixed_form, approx);
    //truth_form = fixed_form;  truth_lev = numApprox;
  }
  else {
    truth_key.form_key(0, numApprox, secondary_index);
    for (unsigned short approx=0; approx<numApprox; ++approx)
      approx_keys[approx].form_key(0, approx, secondary_index);
    //truth_form = numApprox;  truth_lev = secondary_index;
  }
  active_key.aggregate_keys(truth_key, approx_keys, Pecos::RAW_DATA);
  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0
}


void NonDACVSampling::
update_hf_target(const RealVector& avg_eval_ratios, const RealVector& cost,
		 Real mse_ratio, const RealVector& var_H, const SizetArray& N_H,
		 const RealVector& mse_iter0, Real& avg_hf_target)
{
  // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L 
  // > could consider under-relaxing the budget allocation to enable
  //   additional N_H increments + associated updates to shared samples
  //   for improving rho2_LH et al.
  if (maxFunctionEvals != SZ_MAX) {
    Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
    allocate_budget(avg_eval_ratios, cost, avg_hf_target);
  }
  // MSE target = convTol * mse_iter0 = mse_ratio * var_H / N_H
  // --> N_H = mse_ratio * var_H / convTol / mse_iter0
  // Note: don't simplify further since mse_iter0 is fixed based on pilot
  else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: to support both, need to retain default special value (-DBL_MAX) to detect a user spec
    Cout << "Scaling profile for convergenceTol = " << convergenceTol;
    Real avg_varH_div_mse0 = 0.;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      avg_varH_div_mse0 += var_H[qoi] / mse_iter0[qoi];
    avg_varH_div_mse0 /= numFunctions;
    avg_hf_target = mse_ratio * avg_varH_div_mse0 / convergenceTol;
  }
  // numSamples is relative to N_H, but the approx_increments() below are
  // computed relative to hf_targets (independent of sunk cost for pilot)
  Cout << ": average HF target = " << avg_hf_target << std::endl;
  numSamples = one_sided_delta(average(N_H), avg_hf_target);
  //numSamples = std::min(num_samp_budget, num_samp_ctol); // enforce both
}


void NonDACVSampling::
update_hf_targets(const RealMatrix& eval_ratios, const RealVector& cost,
		  const RealVector& mse_ratios,  const RealVector& var_H,
		  const SizetArray& N_H, const RealVector& mse_iter0,
		  RealVector& hf_targets)
{
  // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L 
  // > could consider under-relaxing the budget allocation to enable
  //   additional N_H increments + associated updates to shared samples
  //   for improving rho2_LH et al.
  if (maxFunctionEvals != SZ_MAX) {
    Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
    allocate_budget(eval_ratios, cost, hf_targets);
  }
  // MSE target = convTol * mse_iter0 = mse_ratio * var_H / N_H
  // --> N_H = mse_ratio * var_H / convTol / mse_iter0
  // Note: don't simplify further since mse_iter0 is fixed based on pilot
  else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: to support both, need to retain default special value (-DBL_MAX) to detect a user spec
    Cout << "Scaling profile for convergenceTol = " << convergenceTol;
    hf_targets = mse_ratios;
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      hf_targets[qoi] *= var_H[qoi] / mse_iter0[qoi] / convergenceTol;
  }
  // numSamples is relative to N_H, but the approx_increments() below are
  // computed relative to hf_targets (independent of sunk cost for pilot)
  Cout << ": average HF target = " << average(hf_targets) << std::endl;
  numSamples = one_sided_delta(N_H, hf_targets, 1);
  //numSamples = std::min(num_samp_budget, num_samp_ctol); // enforce both
}


void NonDACVSampling::shared_increment(size_t iter)
{
  if (iter == 0) Cout << "\nMFMC pilot sample: ";
  else Cout << "\nMFMC iteration " << iter << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

    activeSet.request_values(1);
    ensemble_sample_increment(iter, numApprox+1); // BLOCK if not shared_approx_increment()  *** TO DO: step value
  }
}


void NonDACVSampling::shared_approx_increment(size_t iter)
{
  if (iter == 0) Cout << "\nMFMC approx pilot sample: ";
  else Cout << "\nMFMC iteration " << iter
	    << ": shared approx sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

    size_t approx_qoi = numApprox  * numFunctions,
                  end = approx_qoi + numFunctions;
    activeSet.request_values(1, 0,   approx_qoi); // all approx QoI
    activeSet.request_values(0, approx_qoi, end); //   no truth QoI

    ensemble_sample_increment(iter, numApprox); // BLOCK  *** TO DO: step value
  }
}


bool NonDACVSampling::
approx_increment(const RealMatrix& eval_ratios, const Sizet2DArray& N_L_refined,
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
  numSamples = one_sided_delta(N_L_refined[approx], lf_targets, 1); // average

  if (numSamples && start < end) {
    Cout << "\nMFMC sample increment = " << numSamples
	 << " for approximations [" << start+1 << ", " << end << ']';
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " computed from average delta between target:\n" << lf_targets
	   << "and current counts:\n" << N_L_refined[approx];
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


bool NonDACVSampling::
approx_increment(const RealVector& avg_eval_ratios,
		 const Sizet2DArray& N_L_refined, Real hf_target,
		 size_t iter, size_t start, size_t end)
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
  Real lf_target = avg_eval_ratios[approx] * hf_target;
  // Choose avg, RMS, max? (trade-off: possible overshoot vs. more iteration)
  numSamples = one_sided_delta(average(N_L_refined[approx]), lf_target);

  if (numSamples && start < end) {
    Cout << "\nACV sample increment = " << numSamples
	 << " for approximations [" << start+1 << ", " << end << ']';
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " computed from average delta between target " << lf_target
	   << " and current average count " << average(N_L_refined[approx]);
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
    Cout << "\nNo ACV approx sample increment for approximations ["
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


/** This version used by MFMC following shared_increment() */
void NonDACVSampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealMatrixMap& sum_LL, // each L with itself
		   IntRealMatrixMap& sum_LH, // each L with H
		   RealVector& sum_HH, Sizet2DArray& num_L_shared,
		   Sizet2DArray& num_L_refined, SizetArray& num_H,
		   Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter h_it; IntRMMIter ls_it, lr_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, lf_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    hf_index = numApprox * numFunctions;

    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = fn_vals[hf_index];
      hf_is_finite = isfinite(hf_fn);
      // High accumulations:
      if (hf_is_finite) { // neither NaN nor +/-Inf
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
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_shared[approx][qoi];  ++num_L_refined[approx][qoi];
	  if (hf_is_finite) ++num_LH[approx][qoi];
	  ls_it = sum_L_shared.begin();	  ls_ord = ls_it->first;
	  lr_it = sum_L_refined.begin();  lr_ord = lr_it->first;
	  ll_it = sum_LL.begin();         ll_ord = ll_it->first;
	  lh_it = sum_LH.begin();         lh_ord = lh_it->first;
	  lf_prod = lf_fn;	          active_ord = 1;
	  while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
		 ll_it!=sum_LL.end() || lh_it!=sum_LH.end() || active_ord <= 1){
    
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
	    // Low-High
	    if (lh_ord == active_ord && hf_is_finite) {
	      hf_prod = hf_fn;
	      for (m=1; m<active_ord; ++m)
		hf_prod *= hf_fn;
	      lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      ++lh_it;  lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** This version used by ACV following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    IntRealMatrixMap&         sum_LH, // each L with H
		    RealVector& sum_HH, Sizet2DArray& num_L_shared,
		    Sizet2DArray& num_L_refined, SizetArray& num_H,
		    SizetSymMatrixArray& num_LL, Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, lf2_fn, hf_fn, lf_prod, lf2_prod, hf_prod;
  IntRespMCIter r_it;              IntRVMIter    h_it;
  IntRMMIter ls_it, lr_it, lh_it;  IntRSMAMIter ll_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    hf_index = numApprox * numFunctions;

    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = fn_vals[hf_index];
      hf_is_finite = isfinite(hf_fn);
      // High accumulations:
      if (hf_is_finite) { // neither NaN nor +/-Inf
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
	
      SizetSymMatrix& num_LL_q = num_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_shared[approx][qoi];  ++num_L_refined[approx][qoi];
	  ++num_LL_q(approx,approx); // Diagonal of C matrix
	  if (hf_is_finite) ++num_LH[approx][qoi]; // pull out of moment loop

	  ls_it = sum_L_shared.begin();	  ls_ord = ls_it->first;
	  lr_it = sum_L_refined.begin();  lr_ord = lr_it->first;
	  ll_it = sum_LL.begin();         ll_ord = ll_it->first;
	  lh_it = sum_LH.begin();         lh_ord = lh_it->first;
	  lf_prod = lf_fn;	          active_ord = 1;
	  while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
		 ll_it!=sum_LL.end() || lh_it!=sum_LH.end() || active_ord <= 1){
    
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
	      ll_it->second[qoi](approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_fn = fn_vals[lf2_index];

		if (isfinite(lf2_fn)) { // both are finite
		  if (active_ord == 1) ++num_LL_q(approx,approx2);
		  lf2_prod = lf2_fn;
		  for (m=1; m<active_ord; ++m)
		    lf2_prod *= lf2_fn;
		  ll_it->second[qoi](approx,approx2) += lf_prod * lf2_prod;
		}
	      }
	      ++ll_it;  ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High (c vector for each QoI):
	    if (lh_ord == active_ord && hf_is_finite) {
	      hf_prod = hf_fn;
	      for (m=1; m<active_ord; ++m)
		hf_prod *= hf_fn;
	      lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      ++lh_it;  lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** This version used by ACV following shared_approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealMatrixMap& sum_L_refined,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    Sizet2DArray& num_L_shared, Sizet2DArray& num_L_refined,
		    SizetSymMatrixArray& num_LL)
{
  // uses one set of allResponses with QoI aggregation across all approx Models,
  // corresponding to unorderedModels[i-1], i=1:numApprox (omits truthModel)

  using std::isfinite;
  Real lf_fn, lf2_fn, lf_prod, lf2_prod;
  IntRespMCIter r_it; IntRMMIter ls_it, lr_it; IntRSMAMIter ll_it;
  int ls_ord, lr_ord, ll_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      SizetSymMatrix& num_LL_q = num_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_shared[approx][qoi];  ++num_L_refined[approx][qoi];
	  ++num_LL_q(approx,approx); // Diagonal of C matrix

	  ls_it = sum_L_shared.begin();	  ls_ord = ls_it->first;
	  lr_it = sum_L_refined.begin();  lr_ord = lr_it->first;
	  ll_it = sum_LL.begin();         ll_ord = ll_it->first;
	  lf_prod = lf_fn;	          active_ord = 1;
	  while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
		 ll_it!=sum_LL.end()       || active_ord <= 1){
    
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
	      ll_it->second[qoi](approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_fn = fn_vals[lf2_index];

		if (isfinite(lf2_fn)) { // both are finite
		  if (active_ord == 1) ++num_LL_q(approx,approx2);
		  lf2_prod = lf2_fn;
		  for (m=1; m<active_ord; ++m)
		    lf2_prod *= lf2_fn;
		  ll_it->second[qoi](approx,approx2) += lf_prod * lf2_prod;
		}
	      }
	      ++ll_it;  ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** This version used by MFMC following approx_increment() */
void NonDACVSampling::
accumulate_mf_sums(IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, Sizet2DArray& num_L_shared,
		   Sizet2DArray& num_L_refined,
		   size_t approx_start, size_t approx_end)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  Real fn_val, prod;
  int ls_ord, lr_ord, active_ord;
  size_t qoi, fn_index, approx, shared_end = approx_end - 1;
  IntRespMCIter r_it; IntRMMIter ls_it, lr_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    fn_index = 0;

    // accumulate for leading set of models (omit trailing truth),
    // but note that resp and asv are full aggregated length
    for (approx=approx_start; approx<approx_end; ++approx) {

      SizetArray& num_L_sh_a  = num_L_shared[approx];
      SizetArray& num_L_ref_a = num_L_refined[approx];
      for (qoi=0; qoi<numFunctions; ++qoi, ++fn_index) {
	//if (asv[fn_index] & 1) {
	  prod = fn_val = fn_vals[fn_index];
	  if (isfinite(fn_val)) { // neither NaN nor +/-Inf

	    // for pyramid sampling, shared range is one less than refined, i.e.
	    // sum_L_{shared,refined} are both accumulated for all approx except
	    // approx_end-1, which accumulates only sum_L_refined.  See z^1 sets
	    // in Fig. 2b of ACV paper.
	    if (approx < shared_end) {
	      ++num_L_sh_a[qoi];
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
	    ++num_L_ref_a[qoi];
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


/** This version used by ACV following approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_refined,
		    Sizet2DArray& num_L_refined,
		    size_t approx_start, size_t approx_end)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  Real lf_fn, lf_prod;
  int lr_ord, active_ord;
  size_t qoi, lf_index, approx;
  IntRespMCIter r_it; IntRMMIter lr_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      for (approx=approx_start; approx<approx_end; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_refined[approx][qoi];
	  lr_it = sum_L_refined.begin();  lr_ord = lr_it->first;
	  lf_prod = lf_fn;	          active_ord = 1;
	  while (lr_it!=sum_L_refined.end() || active_ord <= 1) {
    
	    // Low refined
	    if (lr_ord == active_ord) { // support general key sequence
	      lr_it->second(qoi,approx) += lf_prod;  ++lr_it;
	      lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


void NonDACVSampling::
compute_LH_correlation(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		       const RealMatrix& sum_LL, const RealMatrix& sum_LH,
		       const RealVector& sum_HH, const Sizet2DArray& N_L_shared,
		       const SizetArray& N_H,    const Sizet2DArray& N_LH,
		       RealVector& var_H,        RealMatrix& rho2_LH)
{
  if (var_H.empty())     var_H.sizeUninitialized(numFunctions);
  if (rho2_LH.empty()) rho2_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LL_a =       sum_LL[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    const SizetArray&    N_L_a =   N_L_shared[approx];
    const SizetArray&   N_LH_a =         N_LH[approx];
    Real*            rho2_LH_a =      rho2_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_correlation(sum_L_shared_a[qoi], sum_H[qoi], sum_LL_a[qoi],
			  sum_LH_a[qoi], sum_HH[qoi], N_L_a[qoi], N_H[qoi],
			  N_LH_a[qoi], var_H[qoi], rho2_LH_a[qoi]);
  }
}


void NonDACVSampling::
compute_LH_covariance(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		      const RealMatrix& sum_LH, const Sizet2DArray& N_L_shared,
		      const SizetArray& N_H,    const Sizet2DArray& N_LH,
		      RealMatrix& cov_LH)
{
  if (cov_LH.empty()) cov_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    const SizetArray&    N_L_a =   N_L_shared[approx];
    const SizetArray&   N_LH_a =         N_LH[approx];
    Real*             cov_LH_a =       cov_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_covariance(sum_L_shared_a[qoi], sum_H[qoi], sum_LH_a[qoi],
			 N_L_a[qoi], N_H[qoi], N_LH_a[qoi], cov_LH_a[qoi]);
  }
}


void NonDACVSampling::
compute_LL_covariance(const RealMatrix& sum_L_shared,
		      const RealSymMatrixArray& sum_LL,
		      const Sizet2DArray& N_L_shared,
		      const SizetSymMatrixArray& N_LL,
		      RealSymMatrixArray& cov_LL)
{
  size_t qoi, approx, approx2, N_L_aq;
  if (cov_LL.empty()) {
    cov_LL.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi].shapeUninitialized(numApprox);
  }

  Real sum_L_aq;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& sum_LL_q = sum_LL[qoi];
    const SizetSymMatrix&  N_LL_q =   N_LL[qoi];
    RealSymMatrix&       cov_LL_q = cov_LL[qoi];
    for (approx=0; approx<numApprox; ++approx) {
      N_L_aq = N_L_shared[approx][qoi];  sum_L_aq = sum_L_shared(qoi,approx);
      for (approx2=0; approx2<=approx; ++approx2)
	compute_covariance(sum_L_aq, sum_L_shared(qoi,approx2),
			   sum_LL_q(approx,approx2), N_L_aq,
			   N_L_shared[approx2][qoi], N_LL_q(approx,approx2),
			   cov_LL_q(approx,approx2));
    }
  }
}


void NonDACVSampling::
compute_ratios(const RealMatrix& sum_L_shared, const RealVector& sum_H,
	       const RealMatrix& sum_LL, const RealMatrix& sum_LH,
	       const RealVector& sum_HH, const RealVector& cost,
	       const Sizet2DArray& N_L_shared, const SizetArray& N_H,
	       const Sizet2DArray& N_LH, RealVector& var_H, RealMatrix& rho2_LH,
	       RealMatrix& eval_ratios,  RealVector& mse_ratios)
{
  compute_LH_correlation(sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH,
			 N_L_shared, N_H, N_LH, var_H, rho2_LH);

  mfmc_eval_ratios(rho2_LH, cost, eval_ratios);
  //RealVector avg_eval_ratios;
  //average(eval_ratios, 0, avg_eval_ratios);// average over qoi for each approx
  //Cout << "Average eval ratios from MFMC:\n" << avg_eval_ratios << std::endl;

  // Compute ratio of MSE for single-fidelity MC and MFMC
  // > Estimator Var for MC = var_H / N_H = MSE (neglect HF bias)
  // > Estimator Var for MFMC = var_H (1-rho_LH(am1)^2) p / N_H^2 cost_H
  //   where budget p = cost^T eval_ratios N_H,  am1 = most-correlated approx
  //   --> EstVar = var_H (1-rho_LH(am1)^2) cost^T eval_ratios / N_H cost_H
  // > MSE ratio = EstVar_MFMC / EstVar_MC
  //   = (1-rho_LH(am1)^2) cost^T eval_ratios / cost_H
  if (mse_ratios.empty()) mse_ratios.sizeUninitialized(numFunctions);
  Real inner_prod, cost_H = cost[numApprox];
  size_t qoi, approx, num_am1 = numApprox - 1;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // include cost_H * w_H
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += cost[approx] * eval_ratios(qoi, approx); // cost_i * w_i
    mse_ratios[qoi] = (1. - rho2_LH(qoi, num_am1)) * inner_prod / cost_H;
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    for (qoi=0; qoi<numFunctions; ++qoi) {
      for (approx=0; approx<numApprox; ++approx)
	Cout << "  QoI " << qoi+1 << " Approx " << approx+1
	   //<< ": cost_ratio = " << cost_H / cost_L
	     << ": rho_sq = "     << rho2_LH(qoi,approx)
	     << " eval_ratio = "  << eval_ratios(qoi,approx) << '\n';
      Cout << "QoI " << qoi+1 << ": variance reduction factor = "
	   << mse_ratios[qoi] << '\n';
    }
    Cout << std::endl;
  }
}


void NonDACVSampling::
compute_ratios(const RealMatrix& sum_L_shared, const RealVector& sum_H,
	       const RealSymMatrixArray& sum_LL, const RealMatrix& sum_LH,
	       const RealVector& sum_HH, const RealVector& cost,
	       const Sizet2DArray& N_L_shared, const SizetArray& N_H,
	       const SizetSymMatrixArray& N_LL, const Sizet2DArray& N_LH,
	       RealVector& avg_eval_ratios, Real& mse_ratio)
{
  compute_variance(sum_H, sum_HH, N_H, varH);
  //Cout << "varH:\n" << varH;
  compute_LH_covariance(sum_L_shared, sum_H, sum_LH,
			N_L_shared, N_H, N_LH, covLH);
  //Cout << "covLH:\n" << covLH;
  compute_LL_covariance(sum_L_shared, sum_LL, N_L_shared, N_LL, covLL);
  //Cout << "covLL:\n" << covLL;

  // Set initial guess based on MFMC eval ratios (iter 0) or warm started from
  // previous solution
  if (mlmfIter == 0) {
    RealMatrix rho2_LH, eval_ratios;  RealMatrix var_L;
    compute_variance(sum_L_shared, sum_LL, N_L_shared, var_L);
    covariance_to_correlation_sq(covLH, var_L, varH, rho2_LH);
    mfmc_eval_ratios(rho2_LH, cost, eval_ratios);
    average(eval_ratios, 0, avg_eval_ratios);// average over qoi for each approx
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Evaluation ratios initial guess from MFMC:\n" << avg_eval_ratios
	   << std::endl;
  }
  //else {
    // consider scaling prev soln to feasibility with updated N_H
    // (similar to NonDLocalRel)
  //}
  varianceMinimizer.initial_point(avg_eval_ratios);

  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    // set linear inequality constraint:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   \Sum_i w_i   r_i <= equivHF * w / N - w
    //   \Sum_i w_i/w r_i <= equivHF / N - 1
    RealVector lin_ineq_lb(1), lin_ineq_ub(1), lin_eq_tgt;
    RealMatrix lin_ineq_coeffs(1, numApprox), lin_eq_coeffs;
    Real cost_H = cost[numApprox];
    lin_ineq_lb[0] = -DBL_MAX; // no lower bound
    lin_ineq_ub[0] = (Real)maxFunctionEvals / average(N_H) - 1.;
    for (size_t approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0,approx) = cost[approx] / cost_H;
    // rather than update an iteratedModel, we must update the Minimizer for
    // use by user-functions mode
    varianceMinimizer.linear_constraints(lin_ineq_coeffs, lin_ineq_lb,
					 lin_ineq_ub, lin_eq_coeffs,lin_eq_tgt);
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT:
    // update x_lb[numApprox] = N_H pilot ?
    // or allow optimal profile to emerge from pilot?
    break;
  }

  // solve the sub-problem to compute the optimal r* that maximizes
  // the variance reduction for fixed N_H
  varianceMinimizer.run();

  // Recover optimizer results for average {eval,mse} ratios
  const RealVector& cv_star
    = varianceMinimizer.variables_results().continuous_variables();
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    copy_data(cv_star, avg_eval_ratios); break;
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios);
    Real avg_hf_target = cv_star[numApprox]; // N*
    numSamples = one_sided_delta(average(N_H), avg_hf_target);
    break;
  }
  }
  // Recovery from optimizer provides average R^2 for individual QoI
  // (NonDACVSampling::objective_evaluator() maximizes std::log(average(R^2)))
  Real log_avg_Rsq = -varianceMinimizer.response_results().function_value(0);
  Cout << "log_avg_Rsq = " << log_avg_Rsq << std::endl;
  mse_ratio = 1. - std::exp(log_avg_Rsq); // ratio = 1 - R^2
  // Alt approach: recompute scalar mse_ratio from avg_eval_ratios[approx]
  //for (size_t approx=0; approx<numApprox; ++approx)
  //  mse_ratio = ... avg_eval_ratios[approx] ...;

  //if (outputLevel >= NORMAL_OUTPUT) {
    for (size_t approx=0; approx<numApprox; ++approx)
      Cout << "Approx " << approx+1 << ": average evaluation ratio = "
	   << avg_eval_ratios[approx] << '\n';
    //for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    //  Cout << "QoI " << qoi+1 << ": variance reduction factor = "
    //	     << mse_ratios[qoi] << '\n';
    Cout << "Variance reduction factor = " << mse_ratio << std::endl;
  //}
}


void NonDACVSampling::
mfmc_eval_ratios(const RealMatrix& rho2_LH, const RealVector& cost,
		 RealMatrix& eval_ratios)
{
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);

  // precompute a factor based on most-correlated model
  RealVector factor(numFunctions, false);
  Real cost_ratio, cost_H = cost[numApprox]; // HF cost
  size_t qoi, approx, num_am1 = numApprox - 1;
  for (qoi=0; qoi<numFunctions; ++qoi)
    factor[qoi] = cost_H / (1. - rho2_LH(qoi, num_am1));
  // second sweep to compute eval_ratios including rho2 look-{ahead,back}
  for (approx=0; approx<numApprox; ++approx) {
    Real*   eval_ratios_a = eval_ratios[approx];
    const Real* rho2_LH_a =     rho2_LH[approx];
    Real           cost_L =        cost[approx];

    // *** TO DO: MUST PROTECT AGAINST sqrt(< 0) BY SEQUENCING CORRELATIONS
    // *** At least screen for condition each iter and abort prior to FPE

    // NOTE: indexing is inverted from Peherstorfer: HF = 1, MF = 2, LF = 3
    // > i+1 becomes i-1 and most correlated ref is rho2_LH(qoi, num_am1)
    if (approx)
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L *
	  (rho2_LH_a[qoi] - rho2_LH(qoi, approx-1)));
    else // rho2_LH for approx-1 (non-existent model) is zero
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L * rho2_LH_a[qoi]);
  }
}


void NonDACVSampling::
mfmc_raw_moments(IntRealMatrixMap& sum_L_shared,
		 IntRealMatrixMap& sum_L_refined, IntRealVectorMap& sum_H,
		 IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		 //const RealMatrix& rho2_LH,
		 const Sizet2DArray& N_L_shared,
		 const Sizet2DArray& N_L_refined, const SizetArray& N_H,
		 const Sizet2DArray& N_LH,        RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  Real beta, sum_H_mq;
  size_t approx, qoi, N_L_sh_aq, N_H_q, N_LH_aq;//, N_shared;
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix& sum_L_sh_m  = sum_L_shared[mom];
    RealMatrix& sum_L_ref_m = sum_L_refined[mom];
    RealVector& sum_H_m     = sum_H[mom];
    RealMatrix& sum_LL_m    = sum_LL[mom];
    RealMatrix& sum_LH_m    = sum_LH[mom];

    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << ":\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      sum_H_mq = sum_H_m[qoi];  N_H_q = N_H[qoi];
      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	N_L_sh_aq = N_L_shared[approx][qoi];  N_LH_aq = N_LH[approx][qoi];
	compute_mfmc_control(sum_L_sh_m(qoi,approx), sum_H_mq,
			     sum_LL_m(qoi,approx), sum_LH_m(qoi,approx),
			     N_L_sh_aq, N_H_q, N_LH_aq, beta); //shared
	//if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1
	       << ": control variate beta = " << std::setw(9) << beta << '\n';
	// For MFMC, shared accumulators and counts telescope
	//N_shared = (approx == numApprox-1) ? N_H_q : N_L[approx+1][qoi];
	apply_control(sum_L_sh_m(qoi,approx),  N_L_sh_aq/*N_shared*/, // shared
		      sum_L_ref_m(qoi,approx), N_L_refined[approx][qoi], // ref
		      beta, H_raw_mq);
      }
    }
  }
}


void NonDACVSampling::
acv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_L_refined,
		IntRealVectorMap& sum_H, IntRealSymMatrixArrayMap& sum_LL,
		IntRealMatrixMap& sum_LH, const RealVector& avg_eval_ratios,
		const Sizet2DArray& N_L_shared, const Sizet2DArray& N_L_refined,
		const SizetArray& N_H,          const SizetSymMatrixArray& N_LL,
		const Sizet2DArray& N_LH,       RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  RealSymMatrix F, CF_inv;
  compute_F_matrix(avg_eval_ratios, F);

  size_t approx, qoi, N_H_q;
  RealVector beta(numApprox);
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix&       sum_L_sh_m =  sum_L_shared[mom];
    RealMatrix&      sum_L_ref_m = sum_L_refined[mom];
    RealVector&          sum_H_m =         sum_H[mom];
    RealSymMatrixArray& sum_LL_m =        sum_LL[mom];
    RealMatrix&         sum_LH_m =        sum_LH[mom];
    if (outputLevel >= NORMAL_OUTPUT) Cout << "Moment " << mom << ":\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      Real sum_H_mq = sum_H_m[qoi];
      if (mom == 1) // variances/covariances already computed for mean estimator
	compute_acv_control(covLL[qoi], F, covLH, qoi, beta);
      else // compute variances/covariances for higher-order moment estimators
	compute_acv_control(sum_L_sh_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			    N_L_shared, N_H[qoi], N_LL[qoi], N_LH, F, qoi, beta); // need to all be based on shared counts *** TO DO: special care with shared_approx_increment()

      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      N_H_q = N_H[qoi];
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1 << ": control "
	       << "variate beta = " << std::setw(9) << beta[approx] << '\n';
	// For ACV, shared counts are fixed at N_H for all approx
	apply_control(sum_L_sh_m(qoi,approx),  N_L_shared[approx][qoi],// shared
		      sum_L_ref_m(qoi,approx), N_L_refined[approx][qoi],  // ref
		      beta[approx], H_raw_mq);
      }
    }
  }
}


/*
void NonDACVSampling::print_results(std::ostream& s, short results_state)
{
  if (statsFlag) {
    // Alternate print_results() to eliminate inflate_final_samples()
    print_multilevel_evaluation_summary(s, N_L);
    print_multilevel_evaluation_summary(s, N_H);
    //print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";

  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
    archive_moments();
    archive_equiv_hf_evals(equivHFEvals);
  }
}
*/


Real NonDACVSampling::objective_function(const RealVector& r_and_N)
{
  RealSymMatrix F, CF_inv;
  compute_F_matrix(r_and_N, F); // admits r || r_and_N sub-problems
  //Cout << "Objective evaluator: F =\n" << F << std::endl;

  RealVector A, R_sq(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    invert_CF(covLL[qoi], F, CF_inv);
    //Cout << "Objective eval: CF inverse =\n" << CF_inv << std::endl;
    compute_A_vector(F, covLH, qoi, A);     // defer c-bar scaling
    //Cout << "Objective eval: A =\n" << A << std::endl;
    compute_Rsq(CF_inv, A, varH[qoi], R_sq[qoi]); // apply scaling^2
    //Cout << "Objective eval: varH[" << qoi << "] = " << varH[qoi]
    //     << " Rsq[" << qoi << "] =\n" << R_sq[qoi] << std::endl;
  }

  Real obj_fn = -std::log(average(R_sq));
  //if (outputLevel >= DEBUG_OUTPUT)
    Cout << "objective_evaluator: design vars:\n" << r_and_N << "R squared:\n"
	 << R_sq << "-log(avg(Rsq)) = " << obj_fn << std::endl;
  return obj_fn; // maximize R_sq; use log to flatten contours
}


/*
void NonDACVSampling::
objective_gradient(const RealVector& r_and_N, RealVector& obj_grad)
{
  // This would still be called for deriv level 0 to identify the set of terms
  // that must be numerically estimated.

  //Cerr << "Warning: gradient of the objective not supported." << std::endl;
  //abort_handler(METHOD_ERROR);
}
*/


Real NonDACVSampling::nonlinear_constraint(const RealVector& r_and_N)
{
  // nln ineq constraint: N ( w + Sum(w_i r_i) ) <= C, where C = equivHF * w
  // -->  N ( 1 + Sum(w_i r_i) / w ) <= equivHF
  Real inner_prod = 0.;
  for (size_t i=0; i<numApprox; ++i)
    inner_prod += sequenceCost[i] * r_and_N[i];  //         Sum(w_i r_i)
  inner_prod /= sequenceCost[numApprox];         //         Sum(w_i r_i) / w
  return r_and_N[numApprox] * (1. + inner_prod); // N ( 1 + Sum(w_i r_i) / w )
}


void NonDACVSampling::
nonlinear_constraint_gradient(const RealVector& r_and_N, RealVector& grad_c)
{
  // inequality constraint: N ( 1 + Sum(w_i r_i) / w ) <= equivHF
  // > grad w.r.t. r_i = N w_i / w
  // > grad w.r.t. N   = 1 + Sum(w_i r_i) / w
  size_t i, len = r_and_N.length(), r_len = len-1;
  //if (grad_c.length() != len) grad_c.sizeUninitialized(len); // don't own

  Real cost_H = sequenceCost[r_len], N_over_w = r_and_N[r_len] / cost_H;
  for (i=0; i<r_len; ++i)
    grad_c[i] = N_over_w * sequenceCost[i];

  Real inner_prod = 0.;
  for (i=0; i<numApprox; ++i)
    inner_prod += sequenceCost[i] * r_and_N[i]; //     Sum(w_i r_i)
  grad_c[r_len] = 1. + inner_prod / cost_H;     // 1 + Sum(w_i r_i) / w
}


/** NPSOL objective functions evaluator for solution of homotopy constraint 
    relaxation parameter optimization. This constrained optimization problem
    performs the update of the tau parameter in the homotopy heuristic 
    approach used to relax the constraints in the original problem . */
void NonDACVSampling::
npsol_objective_evaluator(int& mode, int& n, double* x, double& f,
			  double* grad_f, int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  if (asv_request & 1)
    f = acvInstance->objective_function(x_rv);
  //if (asv_request & 2) {
  //  RealVector grad_f_rv(Teuchos::View, grad_f, n);
  //  acvInstance->objective_gradient(x_rv, grad_f_rv);
  //}
}


void NonDACVSampling::
npsol_constraint_evaluator(int& mode, int& ncnln, int& n, int& nrowj,
			   int* needc, double* x, double* c, double* cjac,
			   int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  if (asv_request & 1)
    c[0] = acvInstance->nonlinear_constraint(x_rv);
  if (asv_request & 2) {
    RealVector grad_c_rv(Teuchos::View, cjac, n);
    acvInstance->nonlinear_constraint_gradient(x_rv, grad_c_rv);
  }
}


void NonDACVSampling::
optpp_objective_evaluator(int mode, int n, const RealVector& x, double& f, 
			  RealVector& grad_f, int& result_mode)
{
  if (mode & 1) { // 1st bit is present, mode = 1 or 3
    f = acvInstance->objective_function(x);
    result_mode = OPTPP::NLPFunction;
  }
  //if (mode & 2) { // 2nd bit is present, mode = 2 or 3
  //  acvInstance->objective_gradient(x, grad_f);
  //  result_mode |= OPTPP::NLPGradient;
  //}
}


void NonDACVSampling::
optpp_constraint_evaluator(int mode, int n, const RealVector& x, RealVector& c,
			   RealMatrix& grad_c, int& result_mode)
{
  if (mode & 1) {
    c[0] = acvInstance->nonlinear_constraint(x);
    result_mode = OPTPP::NLPFunction;
  }
  if (mode & 2) { // 2nd bit is present, mode = 2 or 3
    RealVector grad_c_rv(Teuchos::View, grad_c[0], n); // 0-th col vec
    acvInstance->nonlinear_constraint_gradient(x, grad_c_rv);
    result_mode |= OPTPP::NLPGradient;
  }
}

} // namespace Dakota
