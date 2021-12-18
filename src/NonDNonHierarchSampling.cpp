/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
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
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDNonHierarchSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

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
  NonDEnsembleSampling(problem_db, model),
  solutionMode(
    problem_db.get_short("method.nond.ensemble_sampling_solution_mode")),
  optSubProblemForm(0),
  truthFixedByPilot(problem_db.get_bool("method.nond.truth_fixed_by_pilot"))
{
  // default solver to OPT++ NIP based on numerical experience
  optSubProblemSolver = sub_optimizer_select(
    probDescDB.get_ushort("method.nond.opt_subproblem_solver"), SUBMETHOD_NIP);

  // check iteratedModel for model form hi1erarchy and/or discretization levels;
  // set initial response mode for set_communicators() (precedes core_run()).
  bool err_flag = false;
  if (iteratedModel.surrogate_type() == "non_hierarchical")
    aggregated_models_mode(); // truth model + all approx models
  else {
    Cerr << "Error: Non-hierarchical sampling requires a non-hierarchical "
         << "surrogate model specification." << std::endl;
    err_flag = true;
  }

  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  size_t i, num_mf = model_ensemble.size(), num_lev;
  ModelLIter ml_it;
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

  size_t num_steps;
  configure_sequence(num_steps, secondaryIndex, sequenceType);
  numApprox = num_steps - 1;
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  configure_cost(num_steps, multilev, sequenceCost);
  load_pilot_sample(problem_db.get_sza("method.nond.pilot_samples"),
		    num_steps, pilotSamples);

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

  // prefer MF over ML if both available
  iteratedModel.multifidelity_precedence(true);
  // assign an aggregate model key that persists for core_run()
  bool multilev = (sequenceType == Pecos::RESOLUTION_LEVEL_SEQUENCE);
  assign_active_key(numApprox+1, secondaryIndex, multilev);
}


void NonDNonHierarchSampling::
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


void NonDNonHierarchSampling::shared_increment(size_t iter)
{
  if (iter == 0) Cout << "\nNon-hierarchical pilot sample: ";
  else Cout << "\nNon-hierarchical sampling iteration " << iter
	    << ": shared sample increment = ";
  Cout << numSamples << '\n';

  if (numSamples) {
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

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
    //aggregated_models_mode();
    //iteratedModel.active_model_key(agg_key);

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
    export_all_samples("cv_", iteratedModel.truth_model(), iter, step);
    for (size_t i=0; i<numApprox; ++i)
      export_all_samples("cv_", iteratedModel.surrogate_model(i), iter, step);
  }

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


void NonDNonHierarchSampling::
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const SizetArray& approx_sequence,
		   const RealMatrix& eval_ratios, RealVector& estvar_ratios)
{
  // Compute ratio of EstVar for single-fidelity MC and MFMC
  // > Estimator Var for MC = var_H / N_H
  // > Estimator Var for MFMC = (1 - R^2) var_H / N_H
  // > EstVar ratio = EstVar_MFMC / EstVar_MC = (1 - R^2)

  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  // Peherstorfer paper: ratios derived for N_H = m1* = the optimal # of HF
  // samples, not the actual # (when optimal is hidden by pilot):
  // > Estimator Var for MFMC = var_H (1-rho_LH(am1)^2) p / (N_H^2 cost_H)
  //   where budget p = cost^T eval_ratios N_H,  am1 = most-correlated approx
  //   --> EstVar = var_H (1-rho_LH(am1)^2) cost^T eval_ratios / (N_H cost_H)
  //   --> EstVar ratio = EstVar_MFMC / EstVar_MC
  //                    = (1-rho_LH(am1)^2) cost^T eval_ratios / cost_H
  // For this expression, final MFMC estimator variance should use m1*
  // (ignoring pilot) and not the actual N_H (which includes pilot).  This
  // avoids a bug where MFMC est var doesn't change until m1* emerges from
  // pilot.  We can't take credit for N_H > pilot since r* is applied to m1*,
  // not N_H (see update_hf_targets() -> approx_increment() -> lf_targets).
  /*
  Real inner_prod, cost_H = sequenceCost[numApprox];
  size_t qoi, approx, num_am1 = numApprox - 1;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    inner_prod = cost_H; // include cost_H * w_H
    for (approx=0; approx<numApprox; ++approx)
      inner_prod += sequenceCost[approx] * eval_ratios(qoi, approx);
    estvar_ratios[qoi] = (1. - rho2_LH(qoi, num_am1)) * inner_prod / cost_H;
  }
  if (outputLevel >= NORMAL_OUTPUT) {
    for (qoi=0; qoi<numFunctions; ++qoi) {
      for (approx=0; approx<numApprox; ++approx)
	Cout << "  QoI " << qoi+1 << " Approx " << approx+1
	   //<< ": cost_ratio = " << cost_H / cost_L
	     << ": rho2_LH = "    <<     rho2_LH(qoi,approx)
	     << " eval_ratio = "  << eval_ratios(qoi,approx) << '\n';
      Cout << "QoI " << qoi+1 << ": Peherstorfer variance reduction factor = "
	   << estvar_ratios[qoi] << '\n';
    }
    Cout << std::endl;
  }
  */

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq. above.
  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, i, ip1;
  switch (optSubProblemForm) {

  // eval_ratios per qoi,approx with no model re-sequencing
  case ANALYTIC_SOLUTION:
    for (qoi=0; qoi<numFunctions; ++qoi) {
      R_sq = 0.;  r_i = eval_ratios(qoi, 0);
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	r_ip1 = eval_ratios(qoi, ip1);
	R_sq += (r_i - r_ip1) / (r_i * r_ip1) * rho2_LH(qoi, i);
	r_i = r_ip1;
      }
      R_sq += (r_i - 1.) / r_i * rho2_LH(qoi, numApprox-1);
      estvar_ratios[qoi] = (1. - R_sq);
    }
    break;

  // eval_ratios & approx_sequence based on avg_rho2_LH: remain consistent here
  case REORDERED_ANALYTIC_SOLUTION: {
    RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
    bool ordered = approx_sequence.empty();
    approx = (ordered) ? 0 : approx_sequence[0];
    r_i = eval_ratios(0, approx);  R_sq = 0.;
    for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
      approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
      r_ip1 = eval_ratios(0, approx_ip1);
      // Note: monotonicity in reordered r_i is enforced in mfmc_eval_ratios()
      // and in linear constraints for nonhierarch_numerical_solution()
      R_sq += (r_i - r_ip1) / (r_i * r_ip1) * avg_rho2_LH[approx];
      r_i = r_ip1;  approx = approx_ip1;
    }
    R_sq += (r_i - 1.) / r_i * avg_rho2_LH[approx];
    estvar_ratios = (1. - R_sq); // assign scalar to vector components
    break;
  }

  // Note: objective_function() now calls this fn for MFMC numerical solution.
  // ANALYTIC_SOLUTION corresponds to the ordered case of this implementation.
  default: {
    bool ordered = approx_sequence.empty();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      approx = (ordered) ? 0 : approx_sequence[0];
      R_sq = 0.;  r_i = eval_ratios(qoi, approx);
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
	r_ip1 = eval_ratios(qoi, approx_ip1);
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
mfmc_estvar_ratios(const RealMatrix& rho2_LH, const SizetArray& approx_sequence,
		   const RealVector& avg_eval_ratios, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  // Appendix B of JCP paper on ACV:
  // > R^2 = \Sum_i [ (r_i -r_{i-1})/(r_i r_{i-1}) rho2_LH_i ]
  // > Reorder differences since eval ratios/correlations ordered from LF to HF
  //   (opposite of JCP); after this change, reproduces Peherstorfer eq. above.
  Real R_sq, r_i, r_ip1;  size_t qoi, approx, approx_ip1, i, ip1;
  switch (optSubProblemForm) {

  // eval_ratios per qoi,approx with no model re-sequencing
  case ANALYTIC_SOLUTION:
    for (qoi=0; qoi<numFunctions; ++qoi) {
      R_sq = 0.;  r_i = avg_eval_ratios[0];
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	r_ip1 = avg_eval_ratios[ip1];
	R_sq += (r_i - r_ip1) / (r_i * r_ip1) * rho2_LH(qoi, i);
	r_i = r_ip1;
      }
      R_sq += (r_i - 1.) / r_i * rho2_LH(qoi, numApprox-1);
      estvar_ratios[qoi] = (1. - R_sq);
    }
    break;

  // eval_ratios & approx_sequence based on avg_rho2_LH: remain consistent here
  case REORDERED_ANALYTIC_SOLUTION: {
    RealVector avg_rho2_LH;  average(rho2_LH, 0, avg_rho2_LH); // avg over QoI
    bool ordered = approx_sequence.empty();
    approx = (ordered) ? 0 : approx_sequence[0];
    r_i = avg_eval_ratios[approx];  R_sq = 0.;
    for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
      approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
      r_ip1 = avg_eval_ratios[approx_ip1];
      // Note: monotonicity in reordered r_i is enforced in mfmc_eval_ratios()
      // and in linear constraints for nonhierarch_numerical_solution()
      R_sq += (r_i - r_ip1) / (r_i * r_ip1) * avg_rho2_LH[approx];
      r_i = r_ip1;  approx = approx_ip1;
    }
    R_sq += (r_i - 1.) / r_i * avg_rho2_LH[approx];
    estvar_ratios = (1. - R_sq); // assign scalar to vector components
    break;
  }

  // Note: objective_function() now calls this fn for MFMC numerical solution.
  // ANALYTIC_SOLUTION corresponds to the ordered case of this implementation.
  default: {
    bool ordered = approx_sequence.empty();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      approx = (ordered) ? 0 : approx_sequence[0];
      R_sq = 0.;  r_i = avg_eval_ratios[approx];
      for (i=0, ip1=1; ip1<numApprox; ++i, ++ip1) {
	approx_ip1 = (ordered) ? ip1 : approx_sequence[ip1];
	r_ip1 = avg_eval_ratios[approx_ip1];
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
    // > i+1 becomes i-1 and most correlated ref is rho2_LH(qoi, num_am1)
    if (approx)
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L *
	  (rho2_LH_a[qoi] - rho2_LH(qoi, approx-1)));
    else // rho2_LH for approx-1 (non-existent model) is zero
      for (qoi=0; qoi<numFunctions; ++qoi)
	eval_ratios_a[qoi] = std::sqrt(factor[qoi] / cost_L * rho2_LH_a[qoi]);
  }

  // Note: one_sided_delta(numH, hf_targets, 1) enforces monotonicity a bit
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
  size_t qoi, approx;
  Real cost_ratio, rho_sq, cost_H = cost[numApprox];
  for (approx=0; approx<numApprox; ++approx) {
    cost_ratio = cost_H / cost[approx];
    const Real* rho2_LH_a =     rho2_LH[approx];
    Real*   eval_ratios_a = eval_ratios[approx];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      rho_sq = rho2_LH_a[qoi];
      eval_ratios_a[qoi] = (rho_sq < 1.) ? // prevent div by 0, sqrt(negative)
	std::sqrt(cost_ratio * rho_sq / (1. - rho_sq)) :
	(Real)maxFunctionEvals / average(numH);
    }
  }
}


void NonDNonHierarchSampling::
nonhierarch_numerical_solution(const RealVector& cost,
			       const SizetArray& approx_sequence,
			       RealVector& avg_eval_ratios,
			       Real& avg_hf_target, Real& avg_estvar,
			       Real& avg_estvar_ratio)
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

  size_t i, num_cdv, num_lin_con = 0, num_nln_con = 0,
    approx, approx_im1, approx_ip1, max_iter = 100000;
  Real cost_H = cost[numApprox], budget = (Real)maxFunctionEvals,
    avg_N_H = average(numH), conv_tol = 1.e-8; // tight convergence
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
    num_lin_con = num_cdv = numApprox + 1;      break;
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
    if (solutionMode != OFFLINE_PILOT)
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
    x_lb = (solutionMode == OFFLINE_PILOT) ? 1. :
      (Real)pilotSamples[numApprox]; // *** TO DO ***: update to avg_N_H?

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
  }

  if (varianceMinimizer.is_null())
    switch (optSubProblemSolver) {
    case SUBMETHOD_SQP: {
      int deriv_level = (optSubProblemForm == R_AND_N_NONLINEAR_CONSTRAINT) ?
	2 : 0; // 0 neither, 1 obj, 2 constr, 3 both
      Real fdss = 1.e-6;
#ifdef HAVE_NPSOL
      varianceMinimizer.assign_rep(std::make_shared<NPSOLOptimizer>(x0, x_lb,
        x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
        lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
        npsol_objective_evaluator, npsol_constraint_evaluator, deriv_level,
	conv_tol, max_iter, fdss));
#endif
      break;
    }
    case SUBMETHOD_NIP: {
      size_t max_eval = 500000;  Real max_step = 100000.;
#ifdef HAVE_OPTPP
      varianceMinimizer.assign_rep(std::make_shared<SNLLOptimizer>(x0, x_lb,
	x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
	lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
	optpp_objective_evaluator, optpp_constraint_evaluator, max_iter,
	max_eval, conv_tol, conv_tol, max_step));
#endif
      break;
    }
    default: // SUBMETHOD_NONE, ...
      Cerr << "Error: sub-problem solver undefined in NonDNonHierarchSampling."
	   << std::endl;
      abort_handler(METHOD_ERROR);
      break;
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
  const RealVector& fn_star
    = varianceMinimizer.response_results().function_values();
  //Cout << "Minimizer results:\ncv_star =\n"<<cv_star<<"fn_star =\n"<<fn_star;

  // Objective recovery from optimizer provides std::log(average(nh_estvar))
  // (a QoI-vector prior to averaging would require recomputation from r*,N*)
  // Note: this value corresponds to N* (_after_ numSamples applied)
  avg_estvar = std::exp(fn_star(0)); // var_H / N_H (1 - R^2)

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
      Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
      avg_hf_target = allocate_budget(avg_eval_ratios, cost);
    }
    else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: detect user spec
      // EstVar target = convTol * estvar_iter0 = estvar_ratio * varH / N_target
      //               = curr_estvar * N_curr / N_target
      //  --> N_target = curr_estvar * N_curr / (convTol * estvar_iter0)
      // Note: estvar_iter0 is fixed based on pilot
      Cout << "Scaling profile for convergenceTol = " << convergenceTol;
      avg_hf_target = avg_estvar * avg_N_H
	            / (convergenceTol * average(estVarIter0));
    }
    //avg_hf_target = std::min(budget_target, ctol_target); // enforce both
    Cout << ": average HF target = " << avg_hf_target << std::endl;
    break;
  default:
    // R_AND_N:  r*   is leading part of r_and_N and N* is trailing part
    // N_VECTOR: N*_i is leading part of r_and_N and N* is trailing part
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios); // r_i | N_i
    avg_hf_target = cv_star[numApprox];  // N*
    break;
  }
  if (optSubProblemForm == N_VECTOR_LINEAR_CONSTRAINT)
    avg_eval_ratios.scale(1. / avg_hf_target); // r*_i = N*_i / N*

  // compute sample increment for HF from current to target:
  numSamples = (truthFixedByPilot) ? 0 :
    one_sided_delta(avg_N_H, avg_hf_target);

  //if (!numSamples) { // metrics not needed unless print_variance_reduction()

  // All cases employ a projected MC estvar to match the projected ACV estvar
  // from N* (where N* may include a numSamples increment not yet performed)
  RealVector mc_estvar;
  project_mc_estimator_variance(varH, numH, numSamples, mc_estvar);
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


Real NonDNonHierarchSampling::objective_function(const RealVector& r_and_N)
{
  RealVector estvar_ratios(numFunctions, false);  size_t qoi;
  switch (mlmfSubMethod) {
  // The ACV implementation below also works for MFMC, but since MFMC has
  // diagonal F, it can be evaluated without per-QoI matrix inversion:
  // > R_sq = a^T [ C o F ]^{-1} a = \Sum_i R_sq_i (sum across set of approx_i)
  // > R_sq_i = F_ii^2 \bar{c}_ii^2 / (F_ii C_ii) for i = approximation number
  //          = F_ii CovLH_i^2 / (VarH_i VarL_i) = F_ii rho2LH_i where
  //   F_ii   = (r_i - r_{i+1}) / (r_i r_{i+1}).
  case SUBMETHOD_MFMC:
    switch (optSubProblemForm) {
    case N_VECTOR_LINEAR_CONSTRAINT: {
      RealVector r;  copy_data_partial(r_and_N, 0, (int)numApprox, r); // N_i
      r.scale(1./r_and_N[numApprox]); // r_i = N_i / N
      // Compiler can resolve overload with (inherited) vector type:
      mfmc_estvar_ratios(rho2LH, approxSequence, r,       estvar_ratios);
      break;
    }
    default: // use leading numApprox terms of r_and_N
      // Compiler can resolve overload with (inherited) vector type:
      mfmc_estvar_ratios(rho2LH, approxSequence, r_and_N, estvar_ratios);
      break;
    }
    break;
  // ACV cases have off-diagonal terms in F and use matrix algebra
  default: {
    RealSymMatrix F;
    switch (optSubProblemForm) {
    case N_VECTOR_LINEAR_CONSTRAINT: {
      RealVector r;  copy_data_partial(r_and_N, 0, (int)numApprox, r); // N_i
      r.scale(1./r_and_N[numApprox]); // r_i = N_i / N
      compute_F_matrix(r, F);
      break;
    }
    case R_ONLY_LINEAR_CONSTRAINT: // N is a vector constant for opt sub-problem
    case R_AND_N_NONLINEAR_CONSTRAINT:
      compute_F_matrix(r_and_N, F); // admits r as leading numApprox terms
      break;
    }
    //Cout << "Objective evaluator: F =\n" << F << std::endl;
    acv_estvar_ratios(F, estvar_ratios);
    break;
  }
  }

  // form estimator variances to pick up dependence on N
  RealVector est_var(numFunctions, false);
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // N is a vector constant for opt sub-problem
    for (qoi=0; qoi<numFunctions; ++qoi)
      est_var[qoi] = varH[qoi] / numH[qoi] * estvar_ratios[qoi];
    break;
  case N_VECTOR_LINEAR_CONSTRAINT:
  case R_AND_N_NONLINEAR_CONSTRAINT: {  // N is a scalar optimization variable
    Real N = r_and_N[numApprox];
    for (qoi=0; qoi<numFunctions; ++qoi)
      est_var[qoi] = varH[qoi] / N         * estvar_ratios[qoi];
    break;
  }
  }

  // protect against R_sq blow-up for N_i < N (if not enforced by linear constr)
  Real avg_est_var = average(est_var), obj_fn = (avg_est_var > 0.) ?
    std::log(avg_est_var) :
    std::numeric_limits<Real>::quiet_NaN();//Pecos::LARGE_NUMBER;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "objective_function(): design vars:\n" << r_and_N
	 << "EstVar ratios:\n" << estvar_ratios
	 << "obj = log(average((1. - Rsq) varH / N)) = " << obj_fn << '\n';
  return obj_fn; // maximize R_sq; use log to flatten contours
}


/*
void NonDNonHierarchSampling::
objective_gradient(const RealVector& r_and_N, RealVector& obj_grad)
{
  // This would still be called for deriv level 0 to identify the set of terms
  // that must be numerically estimated.

  Cerr << "Warning: gradient of the objective not supported." << std::endl;
  abort_handler(METHOD_ERROR);
}
*/


Real NonDNonHierarchSampling::nonlinear_constraint(const RealVector& r_and_N)
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
    Cout << "nonlinear_constraint: design vars:\n" << r_and_N
	 << "budget constr = " << nln_con << std::endl;
  return nln_con;
}


void NonDNonHierarchSampling::
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
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "nonlinear_constraint gradient:\n" << grad_c << std::endl;
}


void NonDNonHierarchSampling::
npsol_objective_evaluator(int& mode, int& n, double* x, double& f,
			  double* grad_f, int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  if (asv_request & 1)
    f = nonHierSampInstance->objective_function(x_rv);
  // NPSOL estimates unspecified components of the obj grad, so ASV grad
  // request is not an error -- just don't specify anything
  //if (asv_request & 2) {
  //  RealVector grad_f_rv(Teuchos::View, grad_f, n);
  //  nonHierSampInstance->objective_gradient(x_rv, grad_f_rv);
  //}
}


void NonDNonHierarchSampling::
npsol_constraint_evaluator(int& mode, int& ncnln, int& n, int& nrowj,
			   int* needc, double* x, double* c, double* cjac,
			   int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  if (asv_request & 1)
    c[0] = nonHierSampInstance->nonlinear_constraint(x_rv);
  if (asv_request & 2) {
    RealVector grad_c_rv(Teuchos::View, cjac, n);
    nonHierSampInstance->nonlinear_constraint_gradient(x_rv, grad_c_rv);
  }
}


/* API for NLF1 objective (see SNLLOptimizer::nlf1_evaluator())
void NonDNonHierarchSampling::
optpp_objective_evaluator(int mode, int n, const RealVector& x, double& f, 
			  RealVector& grad_f, int& result_mode)
{
  if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
    f = nonHierSampInstance->objective_function(x);
    result_mode = OPTPP::NLPFunction;
  }
  if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
    nonHierSampInstance->objective_gradient(x, grad_f);
    result_mode |= OPTPP::NLPGradient;
  }
}
*/


/** API for FDNLF1 objective (see SNLLOptimizer::nlf0_evaluator()) */
void NonDNonHierarchSampling::
optpp_objective_evaluator(int n, const RealVector& x, double& f,
			  int& result_mode)
{
  f = nonHierSampInstance->objective_function(x);
  result_mode = OPTPP::NLPFunction; // 1 bit
}


/** API for NLF1 constraint (see SNLLOptimizer::constraint1_evaluator()) */
void NonDNonHierarchSampling::
optpp_constraint_evaluator(int mode, int n, const RealVector& x, RealVector& c,
			   RealMatrix& grad_c, int& result_mode)
{
  result_mode = OPTPP::NLPNoOp; // 0
  if (mode & OPTPP::NLPFunction) { // 1 bit is present, mode = 1 or 3
    c[0] = nonHierSampInstance->nonlinear_constraint(x);
    result_mode |= OPTPP::NLPFunction; // adds 1 bit
  }
  if (mode & OPTPP::NLPGradient) { // 2 bit is present, mode = 2 or 3
    RealVector grad_c_rv(Teuchos::View, grad_c[0], n); // 0-th col vec
    nonHierSampInstance->nonlinear_constraint_gradient(x, grad_c_rv);
    result_mode |= OPTPP::NLPGradient; // adds 2 bit
  }
}


void NonDNonHierarchSampling::print_variance_reduction(std::ostream& s)
{
  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:";

  if (solutionMode != OFFLINE_PILOT)
    s << "\n      Initial MC (" << std::setw(4) << pilotSamples[numApprox]
      << " pilot samples): " << std::setw(wpp7) << average(estVarIter0);

  String type = (solutionMode == PILOT_PROJECTION) ? "Projected" : "    Final";
  //String method = method_enum_to_string(methodName); // string too verbose
  String method = (methodName == MULTIFIDELITY_SAMPLING) ? " MFMC" : "  ACV";
  // Ordering of averages:
  // > recomputing final MC estvar, rather than dividing the two averages, gives
  //   a result that is consistent with average(estVarIter0) when N* = pilot.
  // > The ACV ratio then differs from final ACV / final MC (due to recovering
  //   avgEstVar from the optimizer obj fn), but difference is usually small.
  RealVector final_mc_estvar;
  compute_mc_estimator_variance(varH, numH, final_mc_estvar);
  s << "\n  " << type << "   MC (" << std::setw(4)
    << (size_t)std::floor(average(numH) + .5) << " HF samples):    "
    << std::setw(wpp7) << average(final_mc_estvar) // avgEstVar / avgEstVarRatio
    << "\n  " << type << method << " (sample profile):     "
    << std::setw(wpp7) << avgEstVar
    << "\n  " << type << method << " ratio (1 - R^2):      "
    << std::setw(wpp7) << avgEstVarRatio << '\n';
}


void NonDNonHierarchSampling::
print_results(std::ostream& s, short results_state)
{
  switch (solutionMode) {
  case PILOT_PROJECTION:
    print_multilevel_evaluation_summary(s, NLev, "Projected");
    //s << "<<<<< Equivalent number of high fidelity evaluations: "
    //  << equivHFEvals << '\n';
    print_variance_reduction(s);

    //s << "\nStatistics based on multilevel sample set:\n";
    //print_moments(s, "response function",
    //		  iteratedModel.truth_model().response_labels());
    //archive_moments();
    //archive_equiv_hf_evals(equivHFEvals);
    break;
  default:
    NonDEnsembleSampling::print_results(s, results_state); break;
  }
}

} // namespace Dakota
