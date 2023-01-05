/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
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
    probDescDB.get_ushort("method.nond.opt_subproblem_solver"),SUBMETHOD_OPTPP);

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
	  sub_iterator.uses_method() == SUBMETHOD_NPSOL ) )
      sub_iterator.method_recourse();
    ModelList& sub_models = iteratedModel.subordinate_models();
    for (ModelLIter ml_iter = sub_models.begin();
	 ml_iter != sub_models.end(); ml_iter++) {
      sub_iterator = ml_iter->subordinate_iterator();
      if (!sub_iterator.is_null() && 
	  ( sub_iterator.method_name() ==     NPSOL_SQP ||
	    sub_iterator.method_name() ==    NLSSOL_SQP ||
	    sub_iterator.uses_method() == SUBMETHOD_NPSOL ) )
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
mfmc_analytic_solution(const RealMatrix& rho2_LH, const RealVector& cost,
		       RealMatrix& eval_ratios, bool monotonic_r)
{
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx, num_am1 = numApprox - 1;
  Real cost_L, cost_H = cost[numApprox]; // HF cost
  // standard approach for well-ordered models
  RealVector factor(numFunctions, false);
  for (qoi=0; qoi<numFunctions; ++qoi)
    factor[qoi] = cost_H / (1. - rho2_LH(qoi, num_am1));
  for (approx=0; approx<numApprox; ++approx) {
    Real*   eval_ratios_a = eval_ratios[approx];
    const Real* rho2_LH_a =     rho2_LH[approx];
    cost_L                =        cost[approx];
    // NOTE: indexing is reversed from Peherstorfer (HF = 1, MF = 2, LF = 3)
    // > becomes Approx LF = 0 and MF = 1, Truth HF = 2
    // > i+1 becomes i-1 and most correlated approx is rho2_LH(qoi, num_am1)
    if (approx)
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L *
	  (rho2_LH_a[qoi] - rho2_LH(qoi, approx-1)));
    else // rho2_LH for approx-1 (non-existent model) is zero
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L * rho2_LH_a[qoi]);
  }

  // Note: one_sided_delta(N_H, hf_targets, 1) enforces monotonicity a bit
  // further downstream.  It averages +/- differences so it's not one-sided
  // per QoI --> to recover the same behavior, we must use avg_eval_ratios.
  // For now, monotonic_r defaults false since it should be redundant.
  if (monotonic_r) {
    RealVector avg_eval_ratios;  average(eval_ratios, 0, avg_eval_ratios);
    Real r_i, prev_ri = 1.;
    for (int i=numApprox-1; i>=0; --i) {
      r_i = std::max(avg_eval_ratios[i], prev_ri);
      inflate(r_i, numFunctions, eval_ratios[i]);
      prev_ri = r_i;
    }
  }
}


void NonDNonHierarchSampling::
mfmc_reordered_analytic_solution(const RealMatrix& rho2_LH,
				 const RealVector& cost,
				 SizetArray& approx_sequence,
				 RealMatrix& eval_ratios, bool monotonic_r)
{
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);

  size_t qoi, approx, num_am1 = numApprox - 1;
  Real cost_L, cost_H = cost[numApprox]; // HF cost

  // employ a single model reordering that is shared across the QoI
  RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
  bool ordered = ordered_approx_sequence(avg_rho2_LH, approx_sequence);
  // Note: even if avg_rho2_LH is now ordered, rho2_LH is not for all QoI, so
  // stick with this alternate formulation, at least for this MFMC iteration.
  if (ordered)
    Cout << "MFMC: averaged correlations are well-ordered.\n" << std::endl;
  else
    Cout << "MFMC: reordered approximation model sequence (low to high):\n"
	 << approx_sequence << std::endl;

  // precompute a factor based on most-correlated model
  size_t most_corr = (ordered) ? num_am1 : approx_sequence[num_am1];
  Real rho2, prev_rho2, rho2_diff, r_i, prev_ri,
    factor = cost_H / (1. - avg_rho2_LH[most_corr]);// most correlated
  // Compute averaged eval_ratios using averaged rho2 for approx_sequence
  RealVector r_unconstrained;
  if (monotonic_r) r_unconstrained.sizeUninitialized(numApprox);
  for (int i=0; i<numApprox; ++i) {
    approx = (ordered) ? i : approx_sequence[i];
    cost_L = cost[approx];
    // NOTE: indexing is inverted from Peherstorfer: HF = 1, MF = 2, LF = 3
    // > i+1 becomes i-1 and most correlated is rho2_LH(qoi, most_corr)
    rho2_diff = rho2  = avg_rho2_LH[approx];
    if (i) rho2_diff -= prev_rho2;
    r_i = std::sqrt(factor / cost_L * rho2_diff);
    if (monotonic_r) r_unconstrained[approx] = r_i;
    else             inflate(r_i, numFunctions, eval_ratios[approx]);
    prev_rho2 = rho2;
  }

  // Reverse loop order and enforce monotonicity in reordered r_i
  // > max() is applied bottom-up from the base of the pyramid (samples
  //   performed bottom up, so precedence also applied in this direction),
  //   where assigning r_i = prev_ri effectively drops the CV for model i
  if (monotonic_r) {
    prev_ri = 1.;
    for (int i=numApprox-1; i>=0; --i) {
      approx = (ordered) ? i : approx_sequence[i];
      r_i = std::max(r_unconstrained[approx], prev_ri);
      inflate(r_i, numFunctions, eval_ratios[approx]);
      prev_ri = r_i;
    }
  }
}


void NonDNonHierarchSampling::
cvmc_ensemble_solutions(const RealMatrix& rho2_LH, const RealVector& cost,
			RealMatrix& eval_ratios)
{
  if (eval_ratios.empty())
    eval_ratios.shapeUninitialized(numFunctions, numApprox);

  // Compute an ensemble of two-model CVMC solutions:
    size_t qoi, approx, hf_form_index, hf_lev_index;
  Real cost_ratio, rho_sq, cost_H = cost[numApprox];
  for (approx=0; approx<numApprox; ++approx) {
    cost_ratio = cost_H / cost[approx];
    const Real* rho2_LH_a =     rho2_LH[approx];
    Real*   eval_ratios_a = eval_ratios[approx];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      rho_sq = rho2_LH_a[qoi];
      eval_ratios_a[qoi] = (rho_sq < 1.) ? // prevent div by 0, sqrt(negative)
	std::sqrt(cost_ratio * rho_sq / (1. - rho_sq)) :
	std::sqrt(cost_ratio / Pecos::SMALL_NUMBER); // should not happen
    }
  }
}


void NonDNonHierarchSampling::
nonhierarch_numerical_solution(const RealVector& cost,
			       const SizetArray& approx_sequence,
			       RealVector& avg_eval_ratios,
			       Real& avg_hf_target, size_t& num_samples,
			       Real& avg_estvar, Real& avg_estvar_ratio)
{
  // --------------------------------------
  // Formulate the optimization sub-problem
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

  size_t i, num_cdv, num_lin_con = 0, num_nln_con = 0, approx, approx_im1,
    approx_ip1, max_iter=100000, max_eval=500000, hf_form_index, hf_lev_index;
  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real cost_H = cost[numApprox], budget = (Real)maxFunctionEvals,
    conv_tol = 1.e-8, // tight convergence
    avg_N_H = (backfillFailures) ? average(N_H_actual) : N_H_alloc;
  bool ordered = approx_sequence.empty();
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    num_cdv = numApprox;
    num_lin_con = (mlmfSubMethod == SUBMETHOD_MFMC) ? numApprox + 1 : 1;
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    num_cdv = numApprox + 1;  num_nln_con = 1;
    if (mlmfSubMethod == SUBMETHOD_MFMC) num_lin_con = numApprox;
    break;
  case N_VECTOR_LINEAR_CONSTRAINT:
    num_lin_con = num_cdv = numApprox + 1;
    break;
  case N_VECTOR_LINEAR_OBJECTIVE:
    num_cdv = numApprox + 1;  num_nln_con = 1;  num_lin_con = numApprox;
    break;
  }
  RealVector x0(num_cdv, false), x_lb(num_cdv, false), x_ub(num_cdv, false);
  RealVector lin_ineq_lb(num_lin_con, false), lin_ineq_ub(num_lin_con),
    lin_eq_tgt, nln_ineq_lb(num_nln_con, false),
    nln_ineq_ub(num_nln_con, false), nln_eq_tgt;
  RealMatrix lin_ineq_coeffs(num_lin_con, num_cdv), lin_eq_coeffs;
  x_ub = DBL_MAX; // no upper bounds

  // Note: ACV paper suggests additional linear constraints for r_i ordering
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    x0   = avg_eval_ratios;
    x_lb = 1.;
    // set linear inequality constraint for fixed N:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   \Sum_i w_i   r_i <= equivHF * w / N - w
    //   \Sum_i w_i/w r_i <= equivHF / N - 1
    lin_ineq_lb    = -DBL_MAX;        // no lower bounds
    lin_ineq_ub[0] = (avg_N_H > 1.) ? // protect N_H==0 for offline pilot
      budget / avg_N_H - 1. : // normal case
      budget - 1.;            // bound N_H at 1 (TO DO: need to perform sample)
    for (approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0,approx) = cost[approx] / cost_H;
    if (mlmfSubMethod == SUBMETHOD_MFMC)// N_i increasing w/ decreasing fidelity
      for (i=1; i<=numApprox; ++i) {
	approx     = (ordered) ? i   : approx_sequence[i];
	approx_im1 = (ordered) ? i-1 : approx_sequence[i-1];
	lin_ineq_coeffs(i, approx_im1) = -1.;
	lin_ineq_coeffs(i, approx)     =  1.;
      }
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    copy_data_partial(avg_eval_ratios, x0, 0);          // r_i
    x0[numApprox] = (mlmfIter) ? avg_N_H : avg_hf_target; // N
    // Could allow optimal profile to emerge from pilot by allowing N* less than
    // the incurred cost (e.g., setting N_lb to 1), but instead we bound with
    // the incurred cost by setting x_lb = latest N_H and retaining r_lb = 1.
    x_lb = 1.; // r_i
    if (pilotMgmtMode != OFFLINE_PILOT)
      x_lb[numApprox] = avg_N_H;//std::floor(avg_N_H + .5); // pilot <= N*

    nln_ineq_lb[0] = -DBL_MAX; // no low bnd
    nln_ineq_ub[0] = budget;
    if (mlmfSubMethod == SUBMETHOD_MFMC) {// N_i increasing w/ decreasing fidel
      lin_ineq_lb = -DBL_MAX; // no lower bnds
      for (i=0; i<numApprox; ++i) { // N_approx >= N_{approx+1}
	approx     = (ordered) ? i   : approx_sequence[i];
	approx_ip1 = (ordered) ? i+1 : approx_sequence[i+1];
	lin_ineq_coeffs(i, approx)     = -1.;
	lin_ineq_coeffs(i, approx_ip1) =  1.;
      }
    }
    break;
  case N_VECTOR_LINEAR_CONSTRAINT: {
    copy_data_partial(avg_eval_ratios, x0, 0);  x0[numApprox] = 1.;
    if (mlmfIter) x0.scale(avg_N_H); // {N} = [ {r_i}, 1 ] * N_hf
    else          x0.scale(avg_hf_target);
    x_lb = (pilotMgmtMode == OFFLINE_PILOT) ? 1. : avg_N_H;

    // linear inequality constraint on budget:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   N w + \Sum_i w_i N_i <= equivHF * w
    //   N + \Sum_i w_i/w N_i <= equivHF
    lin_ineq_lb = -DBL_MAX;  // no lower bnds
    lin_ineq_ub[0] = budget; // remaining ub initialized to 0
    for (approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0, approx) = cost[approx] / cost_H;
    lin_ineq_coeffs(0, numApprox) = 1.;
    // linear inequality constraints on sample counts:
    if (mlmfSubMethod == SUBMETHOD_MFMC) {//N_i increases w/ decreasing fidelity
      approx_im1 = (ordered) ? 0 : approx_sequence[0];
      for (i=1; i<numApprox; ++i) {
	approx = (ordered) ? i : approx_sequence[i];
	lin_ineq_coeffs(i, approx_im1) = -1.;
	lin_ineq_coeffs(i, approx)     =  1.;
	approx_im1 = approx;
      }
      lin_ineq_coeffs(numApprox,    approx) = -1.;
      lin_ineq_coeffs(numApprox, numApprox) =  1. + RATIO_NUDGE; // N_im1 > N
    }
    else //  N_i >  N (aka r_i > 1) prevents numerical exceptions
         // (N_i >= N becomes N_i > N based on RATIO_NUDGE)
      for (approx=1; approx<=numApprox; ++approx) {
	lin_ineq_coeffs(approx,  approx-1) = -1.;
	lin_ineq_coeffs(approx, numApprox) =  1. + RATIO_NUDGE; // N_i > N
      }
    break;
  }
  case N_VECTOR_LINEAR_OBJECTIVE: {
    copy_data_partial(avg_eval_ratios, x0, 0);  x0[numApprox] = 1.;
    if (mlmfIter) x0.scale(avg_N_H); // {N} = [ {r_i}, 1 ] * N_hf
    else          x0.scale(avg_hf_target);
    x_lb = (pilotMgmtMode == OFFLINE_PILOT) ? 1. : avg_N_H;

    // nonlinear constraint on estvar
    nln_ineq_lb = -DBL_MAX;  // no lower bnd
    nln_ineq_ub = std::log(convergenceTol * average(estVarIter0));

    // linear inequality constraints on sample counts:
    lin_ineq_lb = -DBL_MAX;  // no lower bnds, default-init upper bounds (0)
    if (mlmfSubMethod == SUBMETHOD_MFMC) {//N_i increases w/ decreasing fidelity
      approx = (ordered) ? 0 : approx_sequence[0];
      size_t num_am1 = numApprox - 1;
      for (i=0; i<num_am1; ++i) {
	approx_ip1 = (ordered) ? i+1 : approx_sequence[i+1];
	lin_ineq_coeffs(i, approx)     = -1.;
	lin_ineq_coeffs(i, approx_ip1) =  1.;
	approx = approx_ip1;
      }
      lin_ineq_coeffs(num_am1,    approx) = -1.;
      lin_ineq_coeffs(num_am1, numApprox) =  1. + RATIO_NUDGE; // N_im1 > N
    }
    else //  N_i >  N (aka r_i > 1) prevents numerical exceptions
         // (N_i >= N becomes N_i > N based on RATIO_NUDGE)
      for (approx=0; approx<numApprox; ++approx) {
	lin_ineq_coeffs(approx,    approx) = -1.;
	lin_ineq_coeffs(approx, numApprox) =  1. + RATIO_NUDGE; // N_i > N
      }
    break;
  }
  }

  if (varianceMinimizer.is_null()) {

    bool use_adapter   = (optSubProblemSolver != SUBMETHOD_NPSOL &&
			  optSubProblemSolver != SUBMETHOD_OPTPP);
    bool construct_dfs = (optSubProblemSolver == SUBMETHOD_SBLO ||
			  optSubProblemSolver == SUBMETHOD_SBGO);
    if (use_adapter) {
      // configure the minimization sub-problem
      MinimizerAdapterModel adapt_model(x0, x_lb, x_ub, lin_ineq_coeffs,
					lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
					lin_eq_tgt, nln_ineq_lb, nln_ineq_ub,
					nln_eq_tgt, response_evaluator);

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
      
      Model sub_prob_model;
      if (construct_dfs) {
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
	sub_prob_model = DataFitSurrModel(dace_iterator, adapt_model, dfs_set,
					  approx_type, approx_order, corr_type,
					  corr_order, data_order, SILENT_OUTPUT,
					  point_reuse);
      }
      else
	sub_prob_model = adapt_model;

      // select the sub-problem solver
      switch (optSubProblemSolver) {
      case SUBMETHOD_SBLO: {
	short merit_fn = AUGMENTED_LAGRANGIAN_MERIT, accept_logic = FILTER,
	  constr_relax = NO_RELAX;
	unsigned short soft_conv_limit = 5;
	Real tr_factor = .5;
	varianceMinimizer.assign_rep(
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
	varianceMinimizer.assign_rep(std::make_shared<EffGlobalMinimizer>(
	  sub_prob_model, approx_type, samples, seed, use_derivs, max_iter,
	  max_eval, conv_tol));
	break;
      }
      /*
      case SUBMETHOD_CPS: { // may need to be combined with local refinement
        break;
      }
      case SUBMETHOD_SBGO: { // for NonDGlobalInterval, was EAminlp + GP ...
        varianceMinimizer.assign_rep(std::make_shared<SurrBasedGlobalMinimizer>(
	  sub_prob_model, max_iter, max_eval, conv_tol));
        break;
      }
      case SUBMETHOD_EA: { // may need to be combined with local refinement
        varianceMinimizer.assign_rep(std::make_shared<COLINOptimizer>(
	  sub_prob_model, max_iter, max_eval, conv_tol));
        break;
      }
      */
      default: // SUBMETHOD_NONE, ...
	Cerr << "Error: sub-problem solver undefined in NonDNonHierarchSampling"
	     << std::endl;
	abort_handler(METHOD_ERROR);
	break;
      }
    }
    else { // existing call-back APIs do not require adapter or surrogate
      switch (optSubProblemSolver) {
      case SUBMETHOD_NPSOL: {
	Real fdss = 1.e-6; int deriv_level;// 0 neither, 1 obj, 2 constr, 3 both
	switch (optSubProblemForm) {
	case R_AND_N_NONLINEAR_CONSTRAINT: deriv_level = 2;  break;
	case N_VECTOR_LINEAR_OBJECTIVE:    deriv_level = 1;  break;
	default:                           deriv_level = 0;  break;
	}
#ifdef HAVE_NPSOL
	varianceMinimizer.assign_rep(std::make_shared<NPSOLOptimizer>(x0, x_lb,
          x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
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
	  varianceMinimizer.assign_rep(std::make_shared<SNLLOptimizer>(x0, x_lb,
	    x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	    lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	    optpp_nlf1_objective, optpp_fdnlf1_constraint, max_iter, max_eval,
	    conv_tol, conv_tol, max_step));
	  break;
	default:
	  varianceMinimizer.assign_rep(std::make_shared<SNLLOptimizer>(x0, x_lb,
	    x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	    lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	    optpp_fdnlf1_objective, optpp_nlf1_constraint, max_iter, max_eval,
	    conv_tol, conv_tol, max_step));
	  break;
	}
#endif
	break;
      }
      }
    }
  }
  else {
    varianceMinimizer.initial_point(x0);
    //if (x_bounds_update)
      varianceMinimizer.variable_bounds(x_lb, x_ub);
    if (num_lin_con)
      varianceMinimizer.linear_constraints(lin_ineq_coeffs, lin_ineq_lb,
					   lin_ineq_ub, lin_eq_coeffs,
					   lin_eq_tgt);
    if (num_nln_con)
      varianceMinimizer.nonlinear_constraints(nln_ineq_lb, nln_ineq_ub,
					      nln_eq_tgt);
  }

  // ----------------------------------
  // Solve the optimization sub-problem
  // ----------------------------------
  // compute optimal r*,N* (or r* for fixed N) that maximizes variance reduction
  varianceMinimizer.run();

  // -------------------------------------
  // Post-process the optimization results
  // -------------------------------------
  // Recover optimizer results for average {eval_ratios,estvar}.  Also compute
  // shared increment from N* or from targeting specified budget || accuracy.
  const RealVector& cv_star
    = varianceMinimizer.variables_results().continuous_variables();
  const RealVector& fn_vals_star
    = varianceMinimizer.response_results().function_values();
  //Cout << "Minimizer results:\ncv_star =\n" << cv_star
  //     << "fn_vals_star =\n" << fn_vals_star;

  // Objective recovery from optimizer provides std::log(average(nh_estvar))
  // = var_H / N_H (1 - R^2).  Notes:
  // > a QoI-vector prior to averaging would require recomputation from r*,N*)
  // > this value corresponds to N* (_after_ num_samples applied)
  avg_estvar = (optSubProblemForm == N_VECTOR_LINEAR_OBJECTIVE) ?
    std::exp(fn_vals_star(1)) : std::exp(fn_vals_star(0));

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
      avg_hf_target = allocate_budget(avg_eval_ratios, cost);
      Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals
	   << ": average HF target = " << avg_hf_target << std::endl;
    }
    else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: detect user spec
      // EstVar target = convTol * estvar_iter0 = estvar_ratio * varH / N_target
      //               = curr_estvar * N_curr / N_target
      //  --> N_target = curr_estvar * N_curr / (convTol * estvar_iter0)
      // Note: estvar_iter0 is fixed based on pilot
      avg_hf_target = (backfillFailures) ?
	update_hf_target(avg_estvar, N_H_actual, estVarIter0) :
	update_hf_target(avg_estvar, N_H_alloc,  estVarIter0);
      Cout << "Scaling profile for convergenceTol = " << convergenceTol
	   << ": average HF target = " << avg_hf_target << std::endl;
    }
    //avg_hf_target = std::min(budget_target, ctol_target); // enforce both
    break;
  case R_AND_N_NONLINEAR_CONSTRAINT:
    // R_AND_N:  r*   is leading part of r_and_N and N* is trailing part
    // N_VECTOR: N*_i is leading part of r_and_N and N* is trailing part
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios); // r_i | N_i
    avg_hf_target = cv_star[numApprox];  // N*, bounded by linear ineq constr
    break;
  case N_VECTOR_LINEAR_OBJECTIVE: case N_VECTOR_LINEAR_CONSTRAINT:
    // R_AND_N:  r*   is leading part of r_and_N and N* is trailing part
    // N_VECTOR: N*_i is leading part of r_and_N and N* is trailing part
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios); // r_i | N_i
    avg_hf_target = cv_star[numApprox];  // N*, bounded by linear ineq constr
    avg_eval_ratios.scale(1. / avg_hf_target); // r*_i = N*_i / N*
    break;
  }

  // compute sample increment for HF from current to target:
  num_samples = (truthFixedByPilot) ? 0 :
    one_sided_delta(avg_N_H, avg_hf_target);

  //if (!num_samples) { // metrics not needed unless print_variance_reduction()

  // All cases employ a projected MC estvar to match the projected ACV estvar
  // from N* (where N* may include a num_samples increment not yet performed)
  RealVector mc_estvar;
  project_mc_estimator_variance(varH, N_H_actual, num_samples, mc_estvar);
  Real avg_mc_estvar = average(mc_estvar);

  // Report ratio of averages rather that average of ratios (see notes in
  // print_variance_reduction())
  avg_estvar_ratio = avg_estvar / avg_mc_estvar;  // (1 - R^2)
  //RealVector estvar_ratio(numFunctions, false);
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  estvar_ratio[qoi] = 1. - R_sq[qoi];// compute from CF_inv,A->compute_Rsq()
  //avg_estvar_ratio = average(estvar_ratio);

  //}
}


Real NonDNonHierarchSampling::
average_estimator_variance(const RealVector& cd_vars)
{
  RealVector estvar_ratios(numFunctions, false);
  estimator_variance_ratios(cd_vars, estvar_ratios); // virtual: MFMC,ACV,GenACV

  // form estimator variances to pick up dependence on N
  RealVector est_var(numFunctions, false);  size_t qoi;
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // N is a vector constant for opt sub-problem
    if (cd_vars.length() == numApprox) {
      // N_H not provided so pull from latest counter values
      size_t hf_form_index, hf_lev_index;
      hf_indices(hf_form_index, hf_lev_index);
      SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
      for (qoi=0; qoi<numFunctions; ++qoi)
	est_var[qoi] = varH[qoi] / N_H_actual[qoi] * estvar_ratios[qoi];
    }
    else { // N_H appended for convenience or rescaling to updated HF target
      Real N_H = cd_vars[numApprox];
      for (qoi=0; qoi<numFunctions; ++qoi)
	est_var[qoi] = varH[qoi] / N_H * estvar_ratios[qoi];
    }
    break;
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
  case R_AND_N_NONLINEAR_CONSTRAINT: {  // N is a scalar optimization variable
    Real N_H = cd_vars[numApprox];
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
  Real inner_prod = 0.;
  for (size_t i=0; i<numApprox; ++i)
    inner_prod += sequenceCost[i] * r_and_N[i];  //         Sum(w_i r_i)
  inner_prod /= sequenceCost[numApprox];         //         Sum(w_i r_i) / w

  Real nln_con
    = r_and_N[numApprox] * (1. + inner_prod);    // N ( 1 + Sum(w_i r_i) / w )
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "nonlinear cost: design vars:\n" << r_and_N
	 << "cost = " << nln_con << std::endl;
  return nln_con;
}


void NonDNonHierarchSampling::
linear_cost_gradient(const RealVector& N_vec, RealVector& grad_c)
{
  // linear objective: N + Sum(w_i N_i) / w
  // > grad w.r.t. N_i = w_i / w
  // > grad w.r.t. N   = w   / w = 1
  size_t i, len = N_vec.length(), r_len = len-1;
  //if (grad_c.length() != len) grad_c.sizeUninitialized(len); // don't own

  Real cost_H = sequenceCost[r_len];
  for (i=0; i<r_len; ++i)
    grad_c[i] = sequenceCost[i] / cost_H;
  grad_c[r_len] = 1.;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "linear cost gradient:\n" << grad_c << std::endl;
}


void NonDNonHierarchSampling::
nonlinear_cost_gradient(const RealVector& r_and_N, RealVector& grad_c)
{
  // nonlinear inequality constraint: N ( 1 + Sum(w_i r_i) / w ) <= equivHF
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
  default:
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
  default:
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

    if (nln_con && (asv[1] & 1))
      response.function_value(nonHierSampInstance->nonlinear_cost(c_vars), 1);
    if (nln_con && (asv[1] & 2)) {
      RealVector grad_c = response.function_gradient_view(1);
      nonHierSampInstance->nonlinear_cost_gradient(c_vars, grad_c);
    }
    break;
  }
}


void NonDNonHierarchSampling::print_variance_reduction(std::ostream& s)
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
    << std::setw(wpp7) << avgEstVar
    << "\n  " << type << method << " ratio (1 - R^2):    "
    << std::setw(wpp7) << avgEstVarRatio
    << "\n Equivalent   MC (" << std::setw(5)
    << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
    << std::setw(wpp7) << avg_budget_mc_estvar
    << "\n Equivalent" << method << " ratio:              "
    << std::setw(wpp7) << avgEstVar / avg_budget_mc_estvar << '\n';
}

} // namespace Dakota
