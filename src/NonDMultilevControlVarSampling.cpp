/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevControlVarSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevControlVarSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevControlVarSampling::
NonDMultilevControlVarSampling(ProblemDescDB& problem_db, Model& model):
  NonDMultilevelSampling(problem_db, model),
  //NonDMultifidelitySampling(problem_db, model),
  NonDHierarchSampling(problem_db, model), // top of virtual inheritance
  delegateMethod(MULTILEVEL_MULTIFIDELITY_SAMPLING)
{
  // For now...
  // *** Note: prior to MFMC for num_mf > 2, allow limiting ragged case
  //           with num_hf_lev == 1
  // *** Note: DIFFs since previously this case was delegated to MLMC
  if ( !iteratedModel.multilevel_multifidelity() ) {
    Cerr << "Warning: NonDMultilevControlVarSampling assumes multiple model "
	 << "forms and multiple HF solution levels." << std::endl;
    //abort_handler(METHOD_ERROR);
  }

  // MLCV has two overlapping precedence assignments, one from CV ctor (first)
  // that is then overwritten by the ML ctor (second), such that we retain a
  // ML precedence.  This precedence is not required for core_run() below.
  //iteratedModel.multifidelity_precedence(false); // prefer ML, reassign keys
}


void NonDMultilevControlVarSampling::pre_run()
{
  NonDEnsembleSampling::pre_run();

  // reset sample counters to 0
  size_t i, j, num_mf = NLevActual.size(), num_lev;
  for (i=0; i<num_mf; ++i) {
    Sizet2DArray& Nl_i = NLevActual[i];
    num_lev = Nl_i.size();
    for (j=0; j<num_lev; ++j)
      Nl_i[j].assign(numFunctions, 0);
    NLevAlloc[i].assign(num_lev, 0);
  }
  size_t num_hf_lev = iteratedModel.truth_model().solution_levels();
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within an EnsembleSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDMultilevControlVarSampling::core_run()
{
  // Can trap mis-specification of the MLMF method and delegate to
  // inherited core_run() implementations:
  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  if (model_ensemble.size() <= 1) { // delegate to MLMC
    delegateMethod = MULTILEVEL_SAMPLING;
    NonDMultilevelSampling::core_run();
    return;
  }
  //ModelList::iterator last_m_it = --model_ensemble.end();
  //size_t num_hf_lev = last_m_it->solution_levels();
  //if (num_hf_lev <= 1) { // delegate to MFMC
  //  delegateMethod = MULTIFIDELITY_SAMPLING; // MFMC not inherited as CVMC was
  //  NonDMultifidelitySampling::core_run();
  //  return;
  //}
  // else multiple model forms (currently limited to 2) + multiple soln levels

  sequenceType = Pecos::RESOLUTION_LEVEL_SEQUENCE;

  // For two-model control variate methods, select lowest,highest fidelities
  unsigned short lf_form = 0, hf_form = NLevActual.size() - 1;//ordered low:high
  size_t lev = SZ_MAX; // defer on assigning soln levels
  Pecos::ActiveKey active_key;
  active_key.form_key(0, lf_form, lev, hf_form, lev, Pecos::RAW_DATA);
  iteratedModel.active_model_key(active_key);

  if (pilotProjection) // for algorithm assessment/selection
    multilevel_control_variate_mc_pilot_projection();
  else
    switch (pilotMgmtMode) {
    case ONLINE_PILOT:
      //if (true) // reformulated approach using 1 new QoI correlation per level
      multilevel_control_variate_mc_Qcorr();
      //else      // original approach using 1 discrepancy correlation per level
      //  multilevel_control_variate_mc_Ycorr();
      break;
    case OFFLINE_PILOT:
      multilevel_control_variate_mc_offline_pilot();    break;
    }
  // ML performed on HF + CV applied per level using LF if available:
  // perform MLMC on HF model and bind 1:min(num_hf,num_lf) LF control
  // variates starting at coarsest level
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC si employed
    across two model forms to exploit correlation in the discrepancies
    at each level (Y_l).
void NonDMultilevControlVarSampling::multilevel_control_variate_mc_Ycorr()
{
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();
  size_t qoi, lev, num_mf = NLevActual.size(),
    num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets(num_hf_lev), agg_var_hf(num_hf_lev),
    hf_cost = truth_model.solution_level_costs(),
    lf_cost =  surr_model.solution_level_costs();
  Real eps_sq_div_2, sum_sqrt_var_cost, agg_estvar_iter0 = 0., budget, r_lq,
    lf_lev_cost, hf_lev_cost, hf_ref_cost = hf_cost[num_hf_lev-1];
  if (budget_constrained) budget = (Real)maxFunctionEvals * hf_ref_cost;
  RealVectorArray eval_ratios(num_cv_lev);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[1] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_L_refined, sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH;
  initialize_mlmf_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		       sum_HH, num_hf_lev, num_cv_lev);
  RealMatrix var_YH(numFunctions,  num_hf_lev, false),
             rho2_LH(numFunctions, num_cv_lev, false),
             Lambda(numFunctions,  num_cv_lev, false);
  RealVector avg_rho2_LH(num_cv_lev, false), avg_lambda(num_cv_lev, false);

  // Initialize for pilot sample
  unsigned short group, lf_form = 0, hf_form = num_mf - 1;// 2 models @ extremes
  Sizet2DArray&       N_lf =      NLevActual[lf_form];
  Sizet2DArray&       N_hf =      NLevActual[hf_form];
  Sizet2DArray  delta_N_l;
  load_pilot_sample(pilotSamples, sequenceType, NLevActual, delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_form]; 

  // now converge on sample counts per level (N_hf)
  while (!zeros(delta_N_hf) && mlmfIter <= maxIterations) {

    sum_sqrt_var_cost = 0.;
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      configure_indices(group, hf_form, lev, sequenceType);
      hf_lev_cost = level_cost(hf_cost, lev);

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// assign sequence, get samples, export, evaluate
	evaluate_ml_sample_increment("mlcv_", lev);

	// control variate betwen LF and HF for this discretization level:
	// if unequal number of levels, loop over all HF levels for MLMC and
	// apply CVMC when LF levels are available.  LF levels are assigned as
	// control variates to the leading set of HF levels, since these will
	// tend to have larger variance.      
	if (lev < num_cv_lev) {

	  // store allResponses used for sum_H (and sum_HH)
	  IntResponseMap hf_resp = allResponses; // shallow copy
	  // activate LF response (lev 0) or LF response discrepancy (lev > 0)
	  // within the hierarchical surrogate model.  Level indices & surrogate
	  // response mode are same as HF above, only the model form changes.
	  // However, we must pass the unchanged level index to update the
	  // corresponding variable values for the new model form.
	  configure_indices(group, lf_form, lev, sequenceType);
	  lf_lev_cost = level_cost(lf_cost, lev);
	  // compute allResp w/ LF model form reusing allVars from MLMC step
	  // > Note: this CV sample set is not separately exported (see below).
	  evaluate_parameter_sets(iteratedModel);
	  // process previous and new set of allResponses for CV sums
	  accumulate_mlmf_Ysums(allResponses, hf_resp, sum_L_shared,
				sum_L_refined, sum_H, sum_LL, sum_LH,
				sum_HH, lev, N_lf[lev], N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_shared[1,2], L_refined[1,2], LH[1,2])"
		 << ":\n" << sum_L_shared[1] << sum_L_shared[2]
		 << sum_L_refined[1]<< sum_L_refined[2]<< sum_LH[1]<< sum_LH[2];
	  increment_mlmf_equivalent_cost(numSamples, hf_lev_cost,
					 numSamples, lf_lev_cost,
					 hf_ref_cost, equivHFEvals);

	  // compute the average evaluation ratio and Lambda factor
	  RealVector& eval_ratios_l = eval_ratios[lev];
	  compute_eval_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			      sum_HH[1], hf_lev_cost/lf_lev_cost, lev,
			      N_hf[lev], var_YH, rho2_LH, eval_ratios_l);

	  // retain Lambda per QoI and level, but apply QoI-average where needed
	  for (qoi=0; qoi<numFunctions; ++qoi) {
	    r_lq = eval_ratios_l[qoi];
	    Lambda(qoi,lev) = 1. - rho2_LH(qoi,lev) * (r_lq - 1.) / r_lq;
	  }
	  avg_lambda[lev]  = average(Lambda[lev],  numFunctions);
	  avg_rho2_LH[lev] = average(rho2_LH[lev], numFunctions);
	}
	else { // no LF model for this level; accumulate only multilevel sums
	  RealMatrix& sum_HH1 = sum_HH[1];
	  // accumulate H sums for lev = 0, Y sums for lev > 0
	  accumulate_ml_Ysums(sum_H, sum_HH1, lev, N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (H[1], H[2], HH):\n"
		 << sum_H[1] << sum_H[2] << sum_HH1;
	  increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost,
	                               equivHFEvals);
	  // compute Y variances for this level and aggregate across QoI:
	  variance_Ysum(sum_H[1][lev], sum_HH1[lev], N_hf[lev], var_YH[lev]);
	}
	agg_var_hf_l = sum(var_YH[lev], numFunctions);
      }

      // accumulate sum of sqrt's of estimator var * cost used in N_target
      if (lev < num_cv_lev) {
	Real om_rho2 = 1. - avg_rho2_LH[lev];
	sum_sqrt_var_cost += (budget_constrained) ?
	  std::sqrt(agg_var_hf_l / hf_lev_cost * om_rho2) *
	  (hf_lev_cost + (1. + average(eval_ratios[lev])) * lf_lev_cost) :
	  std::sqrt(agg_var_hf_l * hf_lev_cost / om_rho2) * avg_lambda[lev];
      }
      else
	sum_sqrt_var_cost += std::sqrt(agg_var_hf_l * hf_lev_cost);

      // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
      // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
      // will result in no additional variance reduction beyond MLMC.
      if (mlmfIter == 0 && !budget_constrained)
	agg_estvar_iter0 += aggregate_mse_Yvar(var_YH[lev], N_hf[lev]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0) {
      // MLMC estimator variance for final estvar reporting is not aggregated
      // (reduction from control variate is applied subsequently)
      compute_ml_estimator_variance(var_YH, N_hf, estVarIter0);
      // compute eps^2 / 2 = aggregated estvar0 * rel tol
      if (!budget_constrained) {
	eps_sq_div_2 = agg_estvar_iter0 * convergenceTol;
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
      }
    }

    // update sample targets based on variance estimates
    // Note: sum_sqrt_var_cost is defined differently for the two cases
    Real fact = (budget_constrained) ?
      budget / sum_sqrt_var_cost :      // budget constraint
      sum_sqrt_var_cost / eps_sq_div_2; // error balance constraint
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = level_cost(hf_cost, lev);
      hf_targets[lev] = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = one_sided_delta(N_alloc_hf[lev], hf_targets[lev]);
    }

    ++mlmfIter;
    Cout << "\nMLMF MC iteration " << mlmfIter << " sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // All CV lf_increment() calls now follow convergence of ML iteration:
  for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
    configure_indices(group, lf_form, lev, sequenceType);
    // execute additional LF sample increment
    if (lf_increment(eval_ratios[lev], N_lf[lev], hf_targets[lev],
		     mlmfIter, lev)) {
      accumulate_mlmf_Ysums(sum_L_refined, lev, N_lf[lev]);
      increment_ml_equivalent_cost(numSamples, level_cost(lf_cost, lev),
				   hf_ref_cost, equivHFEvals);
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Accumulated sums (L_refined[1,2]):\n" << sum_L_refined[1]
	     << sum_L_refined[2];
    }
  }

  // Roll up raw moments from MLCVMC and MLMC levels
  RealMatrix Y_mom(numFunctions, 4), Y_cvmc_mom(numFunctions, 4, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf[lev],
		   sum_L_refined, N_lf[lev], //rho2_LH,
		   lev, Y_cvmc_mom);
    Y_mom += Y_cvmc_mom;
  }
  if (num_hf_lev > num_cv_lev)
    ml_raw_moments(sum_H[1], sum_H[2], sum_H[3], sum_H[4], N_hf,
		   num_cv_lev, num_hf_lev, Y_mom);
  convert_moments(Y_mom, momentStats); // raw to final (central or std)

  RealMatrix Y_mlmc_mom(numFunctions, 4), mlmc_stats;
  ml_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
		 0, num_hf_lev, Y_mlmc_mom);
  convert_moments(Y_mlmc_mom, mlmc_stats); // raw to final (central or std)
  recover_variance(mlmc_stats, varH);

  compute_mlmf_estimator_variance(var_YH, N_hf, Lambda, estVar);
  avgEstVar = average(estVar);
}
*/


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC is employed
    across two model forms.  It generalizes the Y_l correlation case
    to separately target correlations for each QoI level embedded
    within the level discrepancies. */
void NonDMultilevControlVarSampling::multilevel_control_variate_mc_Qcorr()
{
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();
  size_t qoi, lev, num_mf = NLevActual.size(),
    num_hf_lev = truth_model.solution_levels(),
    num_lf_lev =  surr_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, num_lf_lev), N_alloc_l;
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets(num_hf_lev), lf_targets, agg_var_hf(num_hf_lev),
    hf_cost, lf_cost, hf_accum_cost, lf_accum_cost;
  SizetArray hf_num_cost, lf_num_cost;
  bool online_hf_cost = !query_cost(num_hf_lev, truth_model, hf_cost),
       online_lf_cost = !query_cost(num_lf_lev,  surr_model, lf_cost);
  if (online_hf_cost)
    { hf_accum_cost.size(num_hf_lev); hf_num_cost.assign(num_hf_lev, 0); }
  if (online_lf_cost)
    { lf_accum_cost.size(num_cv_lev); lf_num_cost.assign(num_cv_lev, 0); }
  Real eps_sq_div_2, sum_sqrt_var_cost, agg_estvar_iter0 = 0., budget, r_lq,
    lf_lev_cost, hf_lev_cost, hf_ref_cost, hf_tgt_l, lf_tgt_l;
  RealVectorArray eval_ratios(num_cv_lev);

  // CV requires cross-level covariance combinations in Qcorr approach
  IntRealMatrixMap sum_Ll, sum_Llm1,
    sum_Ll_refined, sum_Llm1_refined, sum_Hl, sum_Hlm1,
    sum_Ll_Ll, sum_Ll_Llm1,   // for Var(Q_l^L), Covar(Q_l^L,Q_lm1^L)
    sum_Llm1_Llm1, sum_Hl_Ll, // for Var(Q_lm1^L), Covar(Q_l^H,Q_l^L)
    sum_Hl_Llm1, sum_Hlm1_Ll, // for Covar(Q_l^H,Q_lm1^L), Covar(Q_lm1^H,Q_l^L)
    sum_Hlm1_Llm1,            // for Covar(Q_lm1^H,Q_lm1^L)
    sum_Hl_Hl,                // for Var(Q_l^H)
    sum_Hl_Hlm1,              // for Covar(Q_l^H,Q_lm1^H)
    sum_Hlm1_Hlm1;            // for Var(Q_lm1^H)
  // Initialize accumulators and related arrays/maps, allowing for different
  // number of ML and CV levels (num_hf_lev & num_cv_lev, respectively).
  initialize_mlmf_sums(sum_Ll, sum_Llm1, sum_Ll_refined, sum_Llm1_refined,
		       sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1,
		       sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll, sum_Hlm1_Llm1,
		       sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1, num_hf_lev,
		       num_cv_lev);
  RealMatrix var_YH(numFunctions,      num_hf_lev, false),
             rho_dot2_LH(numFunctions, num_cv_lev, false),
             Lambda(numFunctions,      num_cv_lev, false);
  RealVector avg_rho_dot2_LH(num_cv_lev, false), avg_lambda(num_cv_lev, false);

  // Initialize for pilot sample
  unsigned short group, lf_form = 0, hf_form = num_mf - 1;// 2 models @ extremes
  SizetArray&   N_alloc_lf  = NLevAlloc[lf_form];
  SizetArray&   N_alloc_hf  = NLevAlloc[hf_form];
  Sizet2DArray& N_actual_lf = NLevActual[lf_form];
  Sizet2DArray& N_actual_hf = NLevActual[hf_form];
  Sizet2DArray  delta_N_l;
  load_pilot_sample(pilotSamples, sequenceType, NLevActual, delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_form];

  // now converge on sample counts per level (N_hf)
  while (!zeros(delta_N_hf) && mlmfIter <= maxIterations) {

    // FIRST PASS: evaluations, accumulations, cost estimates
    // NOTE: this will also simplify removing non-essential synchronizations
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      configure_indices(group, hf_form, lev, sequenceType);
      numSamples = delta_N_hf[lev];
      if (numSamples) {
	// assign sequence, get samples, export, evaluate
	evaluate_ml_sample_increment("mlcv_", lev);
	// accumulate online costs for HF model
	if (online_hf_cost && mlmfIter == 0)
	  accumulate_paired_online_cost(hf_accum_cost, hf_num_cost, lev);

	N_alloc_l = (backfillFailures && mlmfIter) ?
	  one_sided_delta(N_alloc_hf[lev], hf_targets[lev]) : numSamples;
	N_alloc_hf[lev] += N_alloc_l;

	// control variate betwen LF and HF for this discretization level:
	// if unequal number of levels, LF levels are assigned as CV to the
	// leading set of HF levels, since these tend to have larger variance.
	if (lev < num_cv_lev) {
	  // store previous allResponses prior to new set of evaluations
	  IntResponseMap hf_resp = allResponses; // shallow copy is sufficient
	  // activate LF response (lev 0) or LF response discrepancy (lev > 0)
	  configure_indices(group, lf_form, lev, sequenceType);
	  // eval allResp w/ LF model reusing allVars from ML step above
	  // > Note: this CV sample set is not separately exported using
	  //   export_all_samples() since it is the same as the ML set.
	  // > This is why the preceding set is marked as "mlcv_", indicating
	  //   that the parameter sets should be applied to both ML and CV.
	  evaluate_parameter_sets(iteratedModel);
	  // process previous and new set of allResponses for MLMF sums;
	  accumulate_mlmf_Qsums(allResponses, hf_resp, sum_Ll, sum_Llm1,
				sum_Ll_refined, sum_Llm1_refined, sum_Hl,
				sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1,
				sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
				sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
				sum_Hlm1_Hlm1, lev, N_actual_lf[lev],
				N_actual_hf[lev]);
	  // accumulate online costs for LF model
	  if (online_lf_cost && mlmfIter == 0)
	    accumulate_paired_online_cost(lf_accum_cost, lf_num_cost, lev);

	  N_alloc_lf[lev] += N_alloc_l;
	}
	else // no LF for this level; accumulate only multilevel discrepancies
	  accumulate_ml_Ysums(sum_Hl, sum_Hl_Hl[1], lev, N_actual_hf[lev]);
      }
    }
    if (mlmfIter == 0) {
      if (online_hf_cost)
	average_online_cost(hf_accum_cost, hf_num_cost, hf_cost);
      if (online_lf_cost)
	average_online_cost(lf_accum_cost, lf_num_cost, lf_cost);
      hf_ref_cost = hf_cost[num_hf_lev-1];
      if (budget_constrained) budget = (Real)maxFunctionEvals * hf_ref_cost;
      // Note: could assign these back if needed elsewhere:
      //if (online_hf_cost) truth_model.solution_level_costs(hf_cost);
      //if (online_lf_cost)  surr_model.solution_level_costs(lf_cost);
    }

    // SECOND PASS: STATS
    sum_sqrt_var_cost = 0.;
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      hf_lev_cost = level_cost(hf_cost, lev);
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      // control variate betwen LF and HF for this discretization level
      if (lev < num_cv_lev) {
	lf_lev_cost = level_cost(lf_cost, lev);
	increment_mlmf_equivalent_cost(numSamples, hf_lev_cost, numSamples,
				       lf_lev_cost, hf_ref_cost, equivHFEvals);

	// compute the average evaluation ratio and Lambda factor
	RealVector& eval_ratios_l = eval_ratios[lev];
	compute_eval_ratios(sum_Ll[1], sum_Llm1[1], sum_Hl[1], sum_Hlm1[1],
			    sum_Ll_Ll[1], sum_Ll_Llm1[1], sum_Llm1_Llm1[1],
			    sum_Hl_Ll[1], sum_Hl_Llm1[1], sum_Hlm1_Ll[1],
			    sum_Hlm1_Llm1[1], sum_Hl_Hl[1], sum_Hl_Hlm1[1],
			    sum_Hlm1_Hlm1[1], hf_lev_cost/lf_lev_cost, lev,
			    N_actual_hf[lev], var_YH, rho_dot2_LH,
			    eval_ratios_l);

	// retain Lambda per QoI and level, but apply QoI-average where needed
	for (qoi=0; qoi<numFunctions; ++qoi) {
	  r_lq = eval_ratios_l[qoi];
	  Lambda(qoi,lev) = 1. - rho_dot2_LH(qoi,lev) * (r_lq - 1.) / r_lq;
	}
	avg_lambda[lev]      = average(Lambda[lev],      numFunctions);
	avg_rho_dot2_LH[lev] = average(rho_dot2_LH[lev], numFunctions);
	agg_var_hf_l         = sum(var_YH[lev],          numFunctions);
	Real om_rho2         = 1. - avg_rho_dot2_LH[lev];
	sum_sqrt_var_cost   += (budget_constrained) ?
	  std::sqrt(agg_var_hf_l / hf_lev_cost * om_rho2) *
	  (hf_lev_cost + (1. + average(eval_ratios[lev])) * lf_lev_cost) :
	  std::sqrt(agg_var_hf_l * hf_lev_cost / om_rho2) * avg_lambda[lev];
      }
      else {
	increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost,
				     equivHFEvals);
	// compute Y variances for this level and aggregate across QoI:
	variance_Ysum(sum_Hl[1][lev], sum_Hl_Hl[1][lev], N_actual_hf[lev],
		      var_YH[lev]);
	agg_var_hf_l       = sum(var_YH[lev], numFunctions);
	sum_sqrt_var_cost += std::sqrt(agg_var_hf_l * hf_lev_cost);
      }

      // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
      // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
      // will result in no additional variance reduction beyond MLMC.
      if (mlmfIter == 0 && !budget_constrained)
	agg_estvar_iter0 += aggregate_mse_Yvar(var_YH[lev], N_actual_hf[lev]);
    }

    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0) {
      // MLMC estimator variance for final estvar reporting is not aggregated
      // (reduction from control variate is applied subsequently)
      compute_ml_estimator_variance(var_YH, N_actual_hf, estVarIter0);
      // compute eps^2 / 2 = aggregated estvar0 * rel tol
      if (!budget_constrained) {// eps^2 / 2 = est var * conv tol
	eps_sq_div_2 = agg_estvar_iter0 * convergenceTol;
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
      }
    }

    // update sample targets based on variance estimates
    // Note: sum_sqrt_var_cost is defined differently for the two cases
    Real fact = (budget_constrained) ?
      budget / sum_sqrt_var_cost :      //        budget constraint
      sum_sqrt_var_cost / eps_sq_div_2; // error balance constraint
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = level_cost(hf_cost, lev);
      hf_tgt_l = hf_targets[lev] = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho_dot2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = (backfillFailures) ?
	one_sided_delta(N_actual_hf[lev], hf_tgt_l, 1) :
	one_sided_delta(N_alloc_hf[lev],  hf_tgt_l);
      // Note: N_alloc_{lf,hf} accumulated upstream due to maxIterations exit
    }

    ++mlmfIter;
    Cout << "\nMLMF MC iteration " << mlmfIter << " HF sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {
    // All CV lf_increment() calls now follow convergence of ML iteration:
    // > Avoids early mis-estimation of LF increments
    // > Parallel scheduling benefits from one final large batch of refinements
    for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
      configure_indices(group, lf_form, lev, sequenceType);
      if (backfillFailures) { // increment relative to successful samples
	lf_increment(eval_ratios[lev], N_actual_lf[lev], hf_targets[lev],
		     lf_targets, mlmfIter, lev);
	lf_tgt_l = average(lf_targets);
	N_alloc_lf[lev] += one_sided_delta(N_alloc_lf[lev], lf_tgt_l);
      }
      else {                  // increment relative to allocated samples
	lf_increment(eval_ratios[lev], N_alloc_lf[lev], hf_targets[lev],
		     lf_tgt_l, mlmfIter, lev);
	N_alloc_lf[lev] += numSamples;
      }
      if (numSamples) {
	accumulate_mlmf_Qsums(sum_Ll_refined, sum_Llm1_refined, lev,
			      N_actual_lf[lev]);
	increment_ml_equivalent_cost(numSamples, level_cost(lf_cost, lev),
				     hf_ref_cost, equivHFEvals);
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Accumulated sums (L_refined[1,2]):\n" << sum_Ll_refined[1]
	       << sum_Ll_refined[2];
      }
    }

    // Roll up raw moments from MLCVMC and MLMC levels
    RealMatrix Y_mom(numFunctions, 4), Y_cvmc_mom(numFunctions, 4, false);
    for (lev=0; lev<num_cv_lev; ++lev) {
      cv_raw_moments(sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
		     sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
		     sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
		     N_actual_hf[lev], sum_Ll_refined, sum_Llm1_refined,
		     N_actual_lf[lev], /*rho_dot2_LH,*/ lev, Y_cvmc_mom);
      Y_mom += Y_cvmc_mom;
    }
    if (num_hf_lev > num_cv_lev)
      ml_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
		     num_cv_lev, num_hf_lev, Y_mom);
    convert_moments(Y_mom, momentStats); // raw to final (central or std)

    // This approach leverages the best available varH for est var reporting,
    // similar to offline pilot mode:
    recover_variance(momentStats, varH);
    // Alternate approach 1: for more consistency with MFMC/ACV, don't take
    // "credit" for the final varH estimates after control variate roll-ups:
    // > MFMC/ACV don't update the varH within the estvar calc after CV roll-up
    //   for moment 2, as this varH would be inconsistent with the iteration
    //   used to generate r*,N*.
    // > Since varH is not accumulated directly, utilize ML-only roll-up
    //   (same for MLCVMC & MLMC) for varH used in final variance reporting.
    // > Counter example: offline pilot cases utilize offline varH...
    //RealMatrix Y_mlmc_mom(numFunctions, 4), mlmc_stats;
    //ml_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
    // 		   0, num_hf_lev, Y_mlmc_mom);
    //convert_moments(Y_mlmc_mom, mlmc_stats); // raw to final (central or std)
    //recover_variance(mlmc_stats, varH);
    //
    // Alternate approach 2: for max MFMC/ACV consistency, only use H_L and N_L
    //compute_variance(sum_Hl[1][L], sum_Hl[2][L],, N_actual_hf[L]);
  }
  else // for consistency with pilot projection
    update_projected_lf_samples(hf_targets, eval_ratios, hf_cost, N_actual_lf,
				N_alloc_lf, lf_cost, deltaEquivHF);

  compute_mlmf_estimator_variance(var_YH, N_actual_hf, Lambda, estVar);
  avgEstVar = average(estVar);
}


void NonDMultilevControlVarSampling::
multilevel_control_variate_mc_offline_pilot()
{
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets_pilot, lf_targets, hf_cost, lf_cost;
  RealMatrix Lambda_pilot, var_YH_pilot;
  RealVectorArray eval_ratios_pilot;

  size_t lev, num_hf_lev = iteratedModel.truth_model().solution_levels();
  unsigned short group, lf_form = 0, hf_form = NLevActual.size() - 1;// extremes
  SizetArray&    N_alloc_lf =  NLevAlloc[lf_form];
  SizetArray&    N_alloc_hf =  NLevAlloc[hf_form];
  Sizet2DArray& N_actual_lf = NLevActual[lf_form];
  Sizet2DArray& N_actual_hf = NLevActual[hf_form];
  SizetArray N_alloc_pilot;  Sizet2DArray N_actual_pilot(num_hf_lev);
  N_alloc_pilot.assign(num_hf_lev, 0);
  for (lev=0; lev<num_hf_lev; ++lev)
    N_actual_pilot[lev].assign(numFunctions, 0);

  // -----------------------------------------
  // Initial loop for offline (overkill) pilot
  // -----------------------------------------
  evaluate_pilot(hf_cost, lf_cost, eval_ratios_pilot, Lambda_pilot,var_YH_pilot,
		 N_alloc_pilot, N_actual_pilot, hf_targets_pilot, false, false);

  // Only QOI_STATISTICS requires iteration and final estimation of moments;
  // ESTIMATOR_PERFORMANCE can bypass this expense.
  size_t num_cv_lev = std::min(num_hf_lev, (size_t)lf_cost.length());
  if (finalStatsType == QOI_STATISTICS) {

    // ----------------------------------------------------------
    // Evaluate online sample profile computed from offline pilot
    // ----------------------------------------------------------
    Real lf_lev_cost, hf_lev_cost, hf_ref_cost = hf_cost[num_hf_lev-1];
    IntRealMatrixMap sum_Ll, sum_Llm1, sum_Ll_refined, sum_Llm1_refined, sum_Hl,
      sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1,
      sum_Hlm1_Ll, sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1;
    initialize_mlmf_sums(sum_Ll, sum_Llm1, sum_Ll_refined, sum_Llm1_refined,
			 sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
			 sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
			 sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
			 num_hf_lev, num_cv_lev);
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {
      configure_indices(group, hf_form, lev, sequenceType);
      hf_lev_cost = level_cost(hf_cost, lev);
      // use 0 in place of delta_N_hf[lev]; min of 2 samples reqd for online var
      numSamples
	= std::max(one_sided_delta(0., hf_targets_pilot[lev]), (size_t)2);
      N_alloc_hf[lev] += numSamples;
      evaluate_ml_sample_increment("mlcv_", lev);
      if (lev < num_cv_lev) {
	IntResponseMap hf_resp = allResponses; // shallow copy is sufficient
	configure_indices(group, lf_form, lev, sequenceType);
	lf_lev_cost = level_cost(lf_cost, lev);
	// eval allResp w/ LF model reusing allVars from ML step above
	evaluate_parameter_sets(iteratedModel);
	// process previous and new set of allResponses for MLMF sums;
	accumulate_mlmf_Qsums(allResponses, hf_resp, sum_Ll, sum_Llm1,
			      sum_Ll_refined, sum_Llm1_refined, sum_Hl,
			      sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1,
			      sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
			      sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
			      sum_Hlm1_Hlm1, lev, N_actual_lf[lev],
			      N_actual_hf[lev]);
	increment_mlmf_equivalent_cost(numSamples, hf_lev_cost, numSamples,
				       lf_lev_cost, hf_ref_cost, equivHFEvals);
	N_alloc_lf[lev] += numSamples;
	// leave evaluation ratios and Lambda at values from Oracle pilot
      }
      else {
	// accumulate H sums for lev = 0, Y sums for lev > 0
	accumulate_ml_Ysums(sum_Hl, sum_Hl_Hl[1], lev, N_actual_hf[lev]);
	increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost,
				     equivHFEvals);
      }
    }

    // --------------------------------------------------------
    // LF increments and final stats from online sample profile
    // --------------------------------------------------------
    Real lf_tgt_l;
    for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
      // LF increments from online sample profile
      configure_indices(group, lf_form, lev, sequenceType);
      if (backfillFailures) { // increment relative to successful samples
	lf_increment(eval_ratios_pilot[lev], N_actual_lf[lev],
		     hf_targets_pilot[lev], lf_targets, mlmfIter, lev);
	lf_tgt_l = average(lf_targets);
	N_alloc_lf[lev] += one_sided_delta(N_alloc_lf[lev], lf_tgt_l);
      }
      else {                  // increment relative to allocated samples
	lf_increment(eval_ratios_pilot[lev], N_alloc_lf[lev],
		     hf_targets_pilot[lev], lf_tgt_l, mlmfIter, lev);
	N_alloc_lf[lev] += numSamples;
      }
      if (numSamples) {
	accumulate_mlmf_Qsums(sum_Ll_refined, sum_Llm1_refined, lev,
			      N_actual_lf[lev]);
	increment_ml_equivalent_cost(numSamples, level_cost(lf_cost, lev),
				     hf_ref_cost, equivHFEvals);
      }
    }

    RealMatrix Y_mom(numFunctions, 4), Y_cvmc_mom(numFunctions, 4, false);
    for (lev=0; lev<num_cv_lev; ++lev) {
      cv_raw_moments(sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
		     sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
		     sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
		     N_actual_hf[lev], sum_Ll_refined, sum_Llm1_refined,
		     N_actual_lf[lev], /*rho_dot2_LH,*/ lev, Y_cvmc_mom);
      Y_mom += Y_cvmc_mom;
    }
    if (num_hf_lev > num_cv_lev)
      ml_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
		     num_cv_lev, num_hf_lev, Y_mom);
    // Convert uncentered raw moment estimates to final moments (central or std)
    convert_moments(Y_mom, momentStats);

    recover_variance(momentStats, varH);
    // See explanation of varH recovery options above.
    //RealMatrix Y_mlmc_mom(numFunctions, 4), mlmc_stats;
    //ml_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
    // 		   0, num_hf_lev, Y_mlmc_mom);
    //convert_moments(Y_mlmc_mom, mlmc_stats); // raw to final (central or std)
    //recover_variance(mlmc_stats, varH);

    // Retain var_YH & Lambda from "oracle" pilot for computing final estimator
    // variances.  This results in mixing offline var_YH,Lambda with online
    // varH,N_hf, but is consistent with offline modes for MFMC and ACV.
    compute_mlmf_estimator_variance(var_YH_pilot, N_actual_hf, Lambda_pilot,
				    estVar);
  }
  else { // estimator performance only requires offline pilot
    for (lev=0; lev<num_cv_lev; ++lev)
      { N_actual_lf[lev] = N_actual_hf[lev]; N_alloc_lf[lev] = N_alloc_hf[lev];}
    SizetArray delta_N_hf;  delta_N_hf.assign(num_hf_lev, 0);
    update_projected_samples(hf_targets_pilot, eval_ratios_pilot, N_actual_hf,
			     N_alloc_hf, hf_cost, N_actual_lf, N_alloc_lf,
			     lf_cost, delta_N_hf, deltaEquivHF);
    Sizet2DArray N_proj_hf = N_actual_hf;
    increment_samples(N_proj_hf, delta_N_hf);
    compute_mlmf_estimator_variance(var_YH_pilot, N_proj_hf, Lambda_pilot,
				    estVar);
  }

  avgEstVar = average(estVar);
}


void NonDMultilevControlVarSampling::
multilevel_control_variate_mc_pilot_projection()
{
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets, hf_cost, lf_cost;
  RealMatrix Lambda, var_YH;  RealVectorArray eval_ratios;

  // Initialize for pilot sample
  unsigned short lf_form = 0, hf_form = NLevActual.size() - 1; // 2 @ extremes
  SizetArray&    N_alloc_lf =  NLevAlloc[lf_form];
  SizetArray&    N_alloc_hf =  NLevAlloc[hf_form];
  Sizet2DArray& N_actual_lf = NLevActual[lf_form];
  Sizet2DArray& N_actual_hf = NLevActual[hf_form];

  evaluate_pilot(hf_cost, lf_cost, eval_ratios, Lambda, var_YH, N_alloc_hf,
		 N_actual_hf, hf_targets, true, true);

  // Unlike NonDMultilevelSampling::multilevel_mc_pilot_projection(), here we
  // cannot readily estimate cv_raw_moments().  Rather than reporting only the
  // ml_raw_moments() roll up, seems better to bypass variance recovery.

  size_t lev, num_hf_lev = (size_t)hf_cost.length(),
    num_cv_lev = std::min(num_hf_lev, (size_t)lf_cost.length());
  for (lev=0; lev<num_cv_lev; ++lev)
    { N_actual_lf[lev] = N_actual_hf[lev]; N_alloc_lf[lev] = N_alloc_hf[lev]; }
  SizetArray delta_N_hf;  delta_N_hf.assign(num_hf_lev, 0);
  update_projected_samples(hf_targets, eval_ratios, N_actual_hf, N_alloc_hf,
			   hf_cost, N_actual_lf, N_alloc_lf, lf_cost,
			   delta_N_hf, deltaEquivHF);
  Sizet2DArray N_proj_hf = N_actual_hf;
  increment_samples(N_proj_hf, delta_N_hf);
  compute_mlmf_estimator_variance(var_YH, N_proj_hf, Lambda, estVar);
  avgEstVar = average(estVar);
}


void NonDMultilevControlVarSampling::
evaluate_pilot(RealVector& hf_cost, RealVector& lf_cost,
	       RealVectorArray& eval_ratios, RealMatrix& Lambda,
	       RealMatrix& var_YH, SizetArray& N_alloc, Sizet2DArray& N_actual,
	       RealVector& hf_targets, bool accumulate_cost, bool pilot_estvar)
{
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();
  size_t qoi, lev, num_mf = NLevActual.size(),
    num_hf_lev = truth_model.solution_levels(),
    num_lf_lev =  surr_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, num_lf_lev);
  bool online_hf_cost = !query_cost(num_hf_lev, truth_model, hf_cost),
       online_lf_cost = !query_cost(num_lf_lev,  surr_model, lf_cost);
  RealVector hf_accum_cost, lf_accum_cost; SizetArray hf_num_cost, lf_num_cost;
  if (online_hf_cost)
    { hf_accum_cost.size(num_hf_lev); hf_num_cost.assign(num_hf_lev, 0); }
  if (online_lf_cost)
    { lf_accum_cost.size(num_cv_lev); lf_num_cost.assign(num_cv_lev, 0); }

  eval_ratios.resize(num_cv_lev);
  //N_actual.resize(num_hf_lev);  N_alloc.resize(num_hf_lev);
  hf_targets.sizeUninitialized(num_hf_lev);
  Lambda.shapeUninitialized(numFunctions, num_cv_lev);
  var_YH.shapeUninitialized(numFunctions, num_hf_lev);

  // retrieve cost estimates across solution levels for HF model
  Real eps_sq_div_2, sum_sqrt_var_cost, agg_estvar_iter0 = 0., budget, r_lq,
    lf_lev_cost, hf_lev_cost, hf_ref_cost;
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  RealMatrix sum_Ll(numFunctions, num_cv_lev),
    sum_Llm1(numFunctions, num_cv_lev),sum_Ll_refined(numFunctions, num_cv_lev),
    sum_Llm1_refined(numFunctions, num_cv_lev),
    sum_Ll_Ll(numFunctions, num_cv_lev),  sum_Ll_Llm1(numFunctions, num_cv_lev),
    sum_Llm1_Llm1(numFunctions, num_cv_lev), sum_Hlm1(numFunctions, num_cv_lev),
    sum_Hl_Ll(numFunctions, num_cv_lev),  sum_Hl_Llm1(numFunctions, num_cv_lev),
    sum_Hlm1_Ll(numFunctions, num_cv_lev),
    sum_Hlm1_Llm1(numFunctions, num_cv_lev),  sum_Hl(numFunctions, num_hf_lev),
    sum_Hl_Hl(numFunctions, num_hf_lev), sum_Hl_Hlm1(numFunctions, num_hf_lev),
    sum_Hlm1_Hlm1(numFunctions, num_hf_lev),
    rho_dot2_LH(numFunctions, num_cv_lev, false);
  RealVector agg_var_hf(num_hf_lev), avg_rho_dot2_LH(num_cv_lev, false),
    avg_lambda(num_cv_lev, false);

  // Initialize for pilot sample
  // > Note: N_actual may (pilot projection) or may not (offline pilot)
  //   be the same as N_hf.  We still use N_hf for computing delta_N_hf.
  Sizet2DArray  delta_N_l;
  load_pilot_sample(pilotSamples, sequenceType, NLevActual, delta_N_l);
  unsigned short group, lf_form = 0, hf_form = num_mf - 1;// 2 models @ extremes
  //SizetArray& delta_N_lf = delta_N_l[lf_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_form];

  // FIRST PASS: evaluations, accumulations, cost estimates
  // NOTE: this will also simplify removing non-essential synchronizations
  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

    configure_indices(group, hf_form, lev, sequenceType);
    numSamples = N_alloc[lev] = delta_N_hf[lev];
    //N_actual[lev].assign(numFunctions, 0); // covered in pre_run()

    evaluate_ml_sample_increment("mlcv_", lev);
    if (online_hf_cost) // accumulate online costs for HF model
      accumulate_paired_online_cost(hf_accum_cost, hf_num_cost, lev);

    // control variate betwen LF and HF for this discretization level
    if (lev < num_cv_lev) {
      // store allResponses used for sum_H (and sum_HH)
      IntResponseMap hf_resp = allResponses; // shallow copy is sufficient
      // activate LF response (lev 0) or LF response discrepancy (lev > 0)
      configure_indices(group, lf_form, lev, sequenceType);
      // eval allResp w/ LF model reusing allVars from ML step above
      // > Note: this CV sample set is not separately exported (see above).
      evaluate_parameter_sets(iteratedModel);
      // process previous and new set of allResponses for MLMF sums;
      accumulate_mlmf_Qsums(allResponses, hf_resp, sum_Ll, sum_Llm1,
			    sum_Ll_refined, sum_Llm1_refined, sum_Hl, sum_Hlm1,
			    sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll,
			    sum_Hl_Llm1, sum_Hlm1_Ll, sum_Hlm1_Llm1, sum_Hl_Hl,
			    sum_Hl_Hlm1, sum_Hlm1_Hlm1,lev, N_actual[lev]);
      if (online_lf_cost) // accumulate online costs for LF model
	accumulate_paired_online_cost(lf_accum_cost, lf_num_cost, lev);
    }
    else // no LF for this level; accumulate only multilevel discrepancies
      accumulate_ml_Ysums(sum_Hl, sum_Hl_Hl, lev, N_actual[lev]);
  }
  if (online_hf_cost) average_online_cost(hf_accum_cost, hf_num_cost, hf_cost);
  if (online_lf_cost) average_online_cost(lf_accum_cost, lf_num_cost, lf_cost);
  hf_ref_cost = hf_cost[num_hf_lev-1];
  if (budget_constrained) budget = (Real)maxFunctionEvals * hf_ref_cost;
  // Note: could assign these back if needed elsewhere:
  //if (online_hf_cost) truth_model.solution_level_costs(hf_cost);
  //if (online_lf_cost)  surr_model.solution_level_costs(lf_cost);

  // SECOND PASS: STATS
  sum_sqrt_var_cost = 0.;
  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

    hf_lev_cost = level_cost(hf_cost, lev);
    Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
    numSamples = delta_N_hf[lev];

    if (lev < num_cv_lev) {
      lf_lev_cost = level_cost(lf_cost, lev);
      if (accumulate_cost)
	increment_mlmf_equivalent_cost(numSamples, hf_lev_cost, numSamples,
				       lf_lev_cost, hf_ref_cost, equivHFEvals);

      // compute the average evaluation ratio and Lambda factor
      RealVector& eval_ratios_l = eval_ratios[lev];
      compute_eval_ratios(sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1, sum_Ll_Ll,
			  sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1,
			  sum_Hlm1_Ll, sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
			  sum_Hlm1_Hlm1, hf_lev_cost/lf_lev_cost, lev,
			  N_actual[lev], var_YH, rho_dot2_LH, eval_ratios_l);

      // retain Lambda per QoI and level, but apply QoI-average where needed
      for (qoi=0; qoi<numFunctions; ++qoi) {
	r_lq = eval_ratios_l[qoi];
	Lambda(qoi,lev) = 1. - rho_dot2_LH(qoi,lev) * (r_lq - 1.) / r_lq;
      }
      avg_lambda[lev]      = average(Lambda[lev],      numFunctions);
      avg_rho_dot2_LH[lev] = average(rho_dot2_LH[lev], numFunctions);
      agg_var_hf_l         = sum(var_YH[lev],          numFunctions);
      // accumulate sum of sqrt's of estimator var * cost used in N_target
      Real om_rho2 = 1. - avg_rho_dot2_LH[lev];
      sum_sqrt_var_cost += (budget_constrained) ?
	std::sqrt(agg_var_hf_l / hf_lev_cost * om_rho2) *
	(hf_lev_cost + (1. + average(eval_ratios[lev])) * lf_lev_cost) :
	std::sqrt(agg_var_hf_l * hf_lev_cost / om_rho2) * avg_lambda[lev];
    }
    else { // no LF model for this level; accumulate only multilevel discreps
      if (accumulate_cost)
	increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost,
				     equivHFEvals);
      // compute Y variances for this level and aggregate across QoI:
      variance_Ysum(sum_Hl[lev], sum_Hl_Hl[lev], N_actual[lev], var_YH[lev]);
      agg_var_hf_l = sum(var_YH[lev], numFunctions);
      // accumulate sum of sqrt's of estimator var * cost used in N_target
      sum_sqrt_var_cost += std::sqrt(agg_var_hf_l * hf_lev_cost);
    }

    // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
    // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
    // will result in no additional variance reduction beyond MLMC.
    if (!budget_constrained)
      agg_estvar_iter0 += aggregate_mse_Yvar(var_YH[lev], N_actual[lev]);
  }

  // MLMC estimator variance for final estvar reporting is not aggregated
  // (reduction from control variate is applied subsequently)
  if (pilot_estvar)
    compute_ml_estimator_variance(var_YH, N_actual, estVarIter0);
  // compute eps^2 / 2 = aggregated estvar0 * rel tol
  if (!budget_constrained) {// eps^2 / 2 = est var * conv tol
    eps_sq_div_2 = agg_estvar_iter0 * convergenceTol;
    if (outputLevel == DEBUG_OUTPUT)
      Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
  }

  // update sample targets based on variance estimates
  // Note: sum_sqrt_var_cost is defined differently for the two cases
  Real fact = (budget_constrained) ?
    budget / sum_sqrt_var_cost :      //        budget constraint
    sum_sqrt_var_cost / eps_sq_div_2; // error balance constraint
  for (lev=0; lev<num_hf_lev; ++lev) {
    hf_lev_cost = level_cost(hf_cost, lev);
    hf_targets[lev] = (lev < num_cv_lev) ? fact *
      std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho_dot2_LH[lev])) :
      fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
    //delta_N_hf[lev] = one_sided_delta(N_alloc_hf[lev], hf_targets[lev]);
  }

  ++mlmfIter;
}


void NonDMultilevControlVarSampling::
compute_eval_ratios(RealMatrix& sum_L_shared, RealMatrix& sum_H,
		    RealMatrix& sum_LL, RealMatrix& sum_LH, RealMatrix& sum_HH,
		    Real cost_ratio, size_t lev, const SizetArray& N_shared,
		    RealMatrix& var_H, RealMatrix& rho2_LH,
		    RealVector& eval_ratios)
{
  if (eval_ratios.empty()) eval_ratios.sizeUninitialized(numFunctions);

  //Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH(qoi,lev);
    compute_mf_correlation(sum_L_shared(qoi,lev), sum_H(qoi,lev),
			   sum_LL(qoi,lev), sum_LH(qoi,lev), sum_HH(qoi,lev),
			   N_shared[qoi], var_H(qoi,lev), rho_sq);
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
	   << std::setw(9) << std::sqrt(rho_sq) << '\n';

    eval_ratios[qoi] = (rho_sq < 1.) ? // protect against division by 0
      std::sqrt(cost_ratio * rho_sq / (1. - rho_sq)) :
      std::sqrt(cost_ratio / Pecos::SMALL_NUMBER); // should not happen
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = " << cost_ratio
	   << " rho_sq = " << rho_sq << " eval_ratio = " << eval_ratios[qoi]
	   << std::endl;
  }
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "variance of HF Q[" << lev << "]:\n";
    write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_H);
  }
}


void NonDMultilevControlVarSampling::
compute_eval_ratios(RealMatrix& sum_Ll,        RealMatrix& sum_Llm1,
		    RealMatrix& sum_Hl,        RealMatrix& sum_Hlm1,
		    RealMatrix& sum_Ll_Ll,     RealMatrix& sum_Ll_Llm1,
		    RealMatrix& sum_Llm1_Llm1, RealMatrix& sum_Hl_Ll,
		    RealMatrix& sum_Hl_Llm1,   RealMatrix& sum_Hlm1_Ll,
		    RealMatrix& sum_Hlm1_Llm1, RealMatrix& sum_Hl_Hl,
		    RealMatrix& sum_Hl_Hlm1,   RealMatrix& sum_Hlm1_Hlm1,
		    Real cost_ratio, size_t lev, const SizetArray& N_shared,
		    RealMatrix& var_YHl,       RealMatrix& rho_dot2_LH,
		    RealVector& eval_ratios)
{
  if (lev == 0)
    compute_eval_ratios(sum_Ll, sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl,
			cost_ratio, lev, N_shared, var_YHl, rho_dot2_LH,
			eval_ratios);
  else {
    if (eval_ratios.empty()) eval_ratios.sizeUninitialized(numFunctions);
    Real beta_dot, gamma;
    for (size_t qoi=0; qoi<numFunctions; ++qoi) {
      Real& rho_dot_sq = rho_dot2_LH(qoi,lev);
      compute_mlmf_control(sum_Ll(qoi,lev), sum_Llm1(qoi,lev), sum_Hl(qoi,lev),
			   sum_Hlm1(qoi,lev), sum_Ll_Ll(qoi,lev),
			   sum_Ll_Llm1(qoi,lev), sum_Llm1_Llm1(qoi,lev),
			   sum_Hl_Ll(qoi,lev), sum_Hl_Llm1(qoi,lev),
			   sum_Hlm1_Ll(qoi,lev), sum_Hlm1_Llm1(qoi,lev),
			   sum_Hl_Hl(qoi,lev), sum_Hl_Hlm1(qoi,lev),
			   sum_Hlm1_Hlm1(qoi,lev), N_shared[qoi],
			   var_YHl(qoi,lev), rho_dot_sq, beta_dot, gamma);
      if (outputLevel >= NORMAL_OUTPUT)
	Cout << "rho_dot_LH for QoI " << qoi+1 << " = " << std::setw(9)
	     << std::sqrt(rho_dot_sq) << '\n';

      eval_ratios[qoi] = (rho_dot_sq < 1.) ? // protect against division by 0
	std::sqrt(cost_ratio * rho_dot_sq / (1.-rho_dot_sq)) :
	std::sqrt(cost_ratio / Pecos::SMALL_NUMBER); // should not happen
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = "
	     << cost_ratio << " rho_dot_sq = " << rho_dot_sq
	     << " eval_ratio = " << eval_ratios[qoi] << std::endl;
    }
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "variance of HF Y[" << lev << "]:\n";
      write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_YHl);
    }
  }
}


bool NonDMultilevControlVarSampling::
lf_increment(const RealVector& eval_ratios, const SizetArray& N_lf,
	     Real hf_target, RealVector& lf_targets, size_t iter, size_t lev)
{
  // update LF samples based on evaluation ratio
  //   r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  //   or with inverse r  -> delta = m-n = n/inverse_r - n
  if (lf_targets.empty()) lf_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_target;
  // Choose average, RMS, max of difference?
  // Trade-off: Possible overshoot vs. more iteration...
  numSamples = one_sided_delta(N_lf, lf_targets, 1); // average

  if (numSamples)
    Cout << "\nControl variate LF sample increment = " << numSamples;
  else Cout << "\nNo control variate LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from avg LF = " << average(N_lf) << ", HF target = " << hf_target
	 << ", avg eval_ratio = "<< average(eval_ratios);
  Cout << std::endl;

  return (numSamples) ? lf_perform_samples(iter, lev) : false;
}


bool NonDMultilevControlVarSampling::
lf_increment(const RealVector& eval_ratios, size_t N_lf, Real hf_target,
	     Real& lf_target, size_t iter, size_t lev)
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

  return (numSamples) ? lf_perform_samples(iter, lev) : false;
}


bool NonDMultilevControlVarSampling::lf_perform_samples(size_t iter, size_t lev)
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
    evaluate_parameter_sets(iteratedModel);
    return true;
  //}
  //else return false;
}


void NonDMultilevControlVarSampling::
cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_H,
	       IntRealMatrixMap& sum_LL,       IntRealMatrixMap& sum_LH,
	       const SizetArray& N_shared,     IntRealMatrixMap& sum_L_refined,
	       const SizetArray& N_refined,  //const RealMatrix& rho2_LH,
	       size_t lev,                     RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);
  RealVector beta(numFunctions, false);

  // rho2_LH not stored for i > 1
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
  //	   << std::setw(9) << std::sqrt(rho2_LH(qoi,lev)) << '\n';
  //<< ", effectiveness ratio = " << std::setw(9) << rho2_LH(qoi,lev)*cr1;

  for (int i=1; i<=4; ++i) {
    compute_mf_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i],
		       N_shared, lev, beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_mf_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		     N_refined, lev, beta, H_rm_col);
  }
  Cout << '\n'; // for loop over levels
}


void NonDMultilevControlVarSampling::
cv_raw_moments(IntRealMatrixMap& sum_Ll,        IntRealMatrixMap& sum_Llm1,
	       IntRealMatrixMap& sum_Hl,        IntRealMatrixMap& sum_Hlm1,
	       IntRealMatrixMap& sum_Ll_Ll,     IntRealMatrixMap& sum_Ll_Llm1,
	       IntRealMatrixMap& sum_Llm1_Llm1, IntRealMatrixMap& sum_Hl_Ll,
	       IntRealMatrixMap& sum_Hl_Llm1,   IntRealMatrixMap& sum_Hlm1_Ll,
	       IntRealMatrixMap& sum_Hlm1_Llm1, IntRealMatrixMap& sum_Hl_Hl,
	       IntRealMatrixMap& sum_Hl_Hlm1,   IntRealMatrixMap& sum_Hlm1_Hlm1,
	       const SizetArray& N_shared, IntRealMatrixMap& sum_Ll_refined,
	       IntRealMatrixMap& sum_Llm1_refined,
	       const SizetArray& N_refined, //const RealMatrix& rho_dot2_LH,
	       size_t lev, RealMatrix& H_raw_mom)
{
  if (lev == 0)
    cv_raw_moments(sum_Ll, sum_Hl, sum_Ll_Ll, sum_Hl_Ll, N_shared,
		   sum_Ll_refined, N_refined, /*rho_dot2_LH,*/ lev, H_raw_mom);
  else {
    if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);
    RealVector beta_dot(numFunctions, false), gamma(numFunctions, false);

    // rho_dot2_LH not stored for i > 1
    //for (size_t qoi=0; qoi<numFunctions; ++qoi)
    //  Cout << "rho_dot_LH for QoI " << qoi+1 << " = " << std::setw(9)
    //       << std::sqrt(rho_dot2_LH(qoi,lev)) << '\n';
    //<< ", effectiveness ratio = " << rho_dot2_LH(qoi,lev) * cr1;

    // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k
    for (int i=1; i<=4; ++i) {
      compute_mlmf_control(sum_Ll[i], sum_Llm1[i], sum_Hl[i], sum_Hlm1[i],
			   sum_Ll_Ll[i], sum_Ll_Llm1[i], sum_Llm1_Llm1[i],
			   sum_Hl_Ll[i], sum_Hl_Llm1[i], sum_Hlm1_Ll[i],
			   sum_Hlm1_Llm1[i], sum_Hl_Hl[i], sum_Hl_Hlm1[i],
			   sum_Hlm1_Hlm1[i], N_shared, lev, beta_dot, gamma);
      Cout << "Moment " << i << ":\n";
      RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
      apply_mlmf_control(sum_Hl[i], sum_Hlm1[i], sum_Ll[i], sum_Llm1[i],
			 N_shared, sum_Ll_refined[i], sum_Llm1_refined[i],
			 N_refined, lev, beta_dot, gamma, H_rm_col);
    }
    Cout << '\n'; // for loop over levels
  }
}


void NonDMultilevControlVarSampling::
compute_mlmf_control(Real sum_Ll, Real sum_Llm1, Real sum_Hl, Real sum_Hlm1,
		     Real sum_Ll_Ll, Real sum_Ll_Llm1, Real sum_Llm1_Llm1,
		     Real sum_Hl_Ll, Real sum_Hl_Llm1, Real sum_Hlm1_Ll,
		     Real sum_Hlm1_Llm1, Real sum_Hl_Hl, Real sum_Hl_Hlm1,
		     Real sum_Hlm1_Hlm1, size_t N_shared, Real& var_YHl,
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
  var_YHl      = var_Hl - 2. * cov_Hl_Hlm1 + var_Hlm1; // var(H_l - H_lm1)
  Real var_YLl = var_Ll - 2. * cov_Ll_Llm1 + var_Llm1, // var(L_l - L_lm1)
       theta   = cov_YHl_YLldot / cov_YHl_YLl, tau = var_YLldot / var_YLl;
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
    Cout << "compute_mlmf_control(): var reduce ratio = " << ratio
	 << " rho2_LH = " << rho2_LH << " rho_dot2_LH = " << rho_dot2_LH
	 << std::endl;
}


void NonDMultilevControlVarSampling::
update_projected_samples(const RealVector& hf_targets,
			 const RealVectorArray& eval_ratios,
			 const Sizet2DArray& N_actual_hf,
			 SizetArray& N_alloc_hf, const RealVector& hf_cost,
			 const Sizet2DArray& N_actual_lf,
			 SizetArray& N_alloc_lf, const RealVector& lf_cost,
			 SizetArray& delta_N_actual_hf,
			 //SizetArray& delta_N_actual_lf,
			 Real& delta_equiv_hf)
{
  size_t hf_actual_incr, hf_alloc_incr, lf_actual_incr, lf_alloc_incr,
    lev, num_hf_lev = hf_cost.length(),
    num_cv_lev = std::min(num_hf_lev, (size_t)lf_cost.length());
  Real hf_target_l, hf_ref_cost = hf_cost[num_hf_lev-1];
  RealVector lf_targets(numFunctions, false);
  for (lev=0; lev<num_hf_lev; ++lev) {
    hf_target_l      = hf_targets[lev];
    hf_alloc_incr    = one_sided_delta(N_alloc_hf[lev], hf_target_l);
    // Note: not duplicate as evaluate_pilot() does not compute delta_N_hf
    hf_actual_incr   = (backfillFailures) ?
      one_sided_delta(N_actual_hf[lev], hf_target_l, 1) : hf_alloc_incr;
    if (pilotMgmtMode == OFFLINE_PILOT) {
      size_t offline_N_lwr = (finalStatsType == QOI_STATISTICS) ? 2 : 1;
      hf_alloc_incr  = std::max(hf_alloc_incr,  offline_N_lwr);
      hf_actual_incr = std::max(hf_actual_incr, offline_N_lwr);
    }
    N_alloc_hf[lev]        += hf_alloc_incr;
    //increment_samples(N_actual_hf[lev], hf_actual_incr);
    delta_N_actual_hf[lev] += hf_actual_incr;
    if (lev<num_cv_lev) {
      const RealVector& eval_ratios_l = eval_ratios[lev];
      for (size_t qoi=0; qoi<numFunctions; ++qoi)
	lf_targets[qoi] = eval_ratios_l[qoi] * hf_target_l;
      lf_alloc_incr    = one_sided_delta(N_alloc_lf[lev], average(lf_targets));
      N_alloc_lf[lev] += lf_alloc_incr;
      lf_actual_incr   = (backfillFailures) ?
	one_sided_delta(N_actual_lf[lev], lf_targets, 1) : lf_alloc_incr;
      //increment_samples(N_actual_lf[lev], lf_actual_incr);
      //delta_N_actual_lf[lev] += lf_actual_incr;
      increment_mlmf_equivalent_cost(hf_actual_incr, level_cost(hf_cost, lev),
				     lf_actual_incr, level_cost(lf_cost, lev),
				     hf_ref_cost, delta_equiv_hf);
    }
    else
      increment_ml_equivalent_cost(hf_actual_incr, level_cost(hf_cost, lev),
				   hf_ref_cost, delta_equiv_hf);
  }
}


void NonDMultilevControlVarSampling::
update_projected_lf_samples(const RealVector& hf_targets,
			    const RealVectorArray& eval_ratios,
			    const RealVector& hf_cost,
			    const Sizet2DArray& N_actual_lf,
			    SizetArray& N_alloc_lf, const RealVector& lf_cost,
			    //SizetArray& delta_N_actual_lf,
			    Real& delta_equiv_hf)
{
  size_t lf_actual_incr, lf_alloc_incr, lev, num_hf_lev = hf_cost.length(),
    num_cv_lev = std::min(num_hf_lev, (size_t)lf_cost.length());
  Real hf_target_l, hf_ref_cost = hf_cost[num_hf_lev-1];
  RealVector lf_targets(numFunctions, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    hf_target_l = hf_targets[lev];
    const RealVector& eval_ratios_l = eval_ratios[lev];
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      lf_targets[qoi] = eval_ratios_l[qoi] * hf_target_l;
    lf_alloc_incr    = one_sided_delta(N_alloc_lf[lev], average(lf_targets));
    N_alloc_lf[lev] += lf_alloc_incr;
    lf_actual_incr   = (backfillFailures) ?
      one_sided_delta(N_actual_lf[lev], lf_targets, 1) : lf_alloc_incr;
    //delta_N_actual_lf[lev] += lf_actual_incr;
    increment_ml_equivalent_cost(lf_actual_incr, level_cost(lf_cost, lev),
				 hf_ref_cost, delta_equiv_hf);
  }
}


void NonDMultilevControlVarSampling::
initialize_mlmf_sums(IntRealMatrixMap& sum_L_shared,
		     IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		     IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		     IntRealMatrixMap& sum_HH,        size_t num_ml_lev,
		     size_t num_cv_lev)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to shape RealMatrix in place and init sums to 0

    // num_cv_lev:
    sum_L_shared.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_L_refined.insert(empty_pr).first->second.shape(numFunctions,num_cv_lev);
    sum_LL.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_LH.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);

    // num_ml_lev:
    sum_H.insert(empty_pr).first->second.shape(numFunctions, num_ml_lev);
  }

  // Only need order 1 accumulation for HH
  empty_pr.first = 1;
  sum_HH.insert(empty_pr).first->second.shape(numFunctions, num_ml_lev);
}


void NonDMultilevControlVarSampling::
initialize_mlmf_sums(IntRealMatrixMap& sum_Ll, IntRealMatrixMap& sum_Llm1,
		     IntRealMatrixMap& sum_Ll_refined,
		     IntRealMatrixMap& sum_Llm1_refined,
		     IntRealMatrixMap& sum_Hl, IntRealMatrixMap& sum_Hlm1,
		     IntRealMatrixMap& sum_Ll_Ll, IntRealMatrixMap& sum_Ll_Llm1,
		     IntRealMatrixMap& sum_Llm1_Llm1,
		     IntRealMatrixMap& sum_Hl_Ll, IntRealMatrixMap& sum_Hl_Llm1,
		     IntRealMatrixMap& sum_Hlm1_Ll,
		     IntRealMatrixMap& sum_Hlm1_Llm1,
		     IntRealMatrixMap& sum_Hl_Hl, IntRealMatrixMap& sum_Hl_Hlm1,
		     IntRealMatrixMap& sum_Hlm1_Hlm1, size_t num_ml_lev,
		     size_t num_cv_lev)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use iterator to shape RealMatrix in place and init sums to 0

    // num_cv_lev:
    sum_Ll.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Llm1.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Ll_refined.insert(empty_pr).first->
      second.shape(numFunctions, num_cv_lev);
    sum_Llm1_refined.insert(empty_pr).first->
      second.shape(numFunctions, num_cv_lev);
    sum_Ll_Ll.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Ll_Llm1.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Llm1_Llm1.insert(empty_pr).first->second.shape(numFunctions,num_cv_lev);
    sum_Hlm1.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Hl_Ll.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Hl_Llm1.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Hlm1_Ll.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Hlm1_Llm1.insert(empty_pr).first->second.shape(numFunctions,num_cv_lev);
    // num_ml_lev:
    sum_Hl.insert(empty_pr).first->second.shape(numFunctions, num_ml_lev);
    sum_Hl_Hl.insert(empty_pr).first->second.shape(numFunctions, num_ml_lev);
    sum_Hl_Hlm1.insert(empty_pr).first->second.shape(numFunctions, num_ml_lev);
    sum_Hlm1_Hlm1.insert(empty_pr).first->second.shape(numFunctions,num_ml_lev);
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		      size_t lev, SizetArray& num_Q)
{
  if (lev == 0)
    accumulate_ml_Qsums(sum_Ql, lev, num_Q);
  else {
    using std::isfinite;
    Real q_l, q_l_prod, q_lm1_prod, q_lm1;
    int l1_ord, l2_ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter l1_it, l2_it;

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	q_lm1_prod = q_lm1 = fn_vals[qoi];
	q_l_prod   = q_l   = fn_vals[qoi+numFunctions];

	// sync sample counts for Ql and Qlm1
	if (isfinite(q_l) && isfinite(q_lm1)) { // neither NaN nor +/-Inf
	  l1_it  = sum_Ql.begin();
	  l2_it  = sum_Qlm1.begin();
	  l1_ord = (l1_it == sum_Ql.end())   ? 0 : l1_it->first;
	  l2_ord = (l2_it == sum_Qlm1.end()) ? 0 : l2_it->first;

	  active_ord = 1;
	  while (l1_it != sum_Ql.end() || l2_it != sum_Qlm1.end()) {

	    // Low: Ll, Llm1
	    if (l1_ord == active_ord) {
	      l1_it->second(qoi,lev) += q_l_prod; ++l1_it;
	      l1_ord = (l1_it == sum_Ql.end()) ? 0 : l1_it->first;
	    }
	    if (l2_ord == active_ord) {
	      l2_it->second(qoi,lev) += q_lm1_prod; ++l2_it;
	      l2_ord = (l2_it == sum_Qlm1.end()) ? 0 : l2_it->first;
	    }

	    q_l_prod *= q_l; q_lm1_prod *= q_lm1; ++active_ord;
	  }
	  ++num_Q[qoi];
	}
      }
      //Cout << r_it->first << ": " << sum_Ql[1] << sum_Qlm1[1];
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Ysums(IntRealMatrixMap& sum_Y, size_t lev, SizetArray& num_Y)
{
  // uses one set of allResponses in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODEL_PAIR (lev > 0) modes.  IntRealMatrixMap is a multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0)
    accumulate_ml_Qsums(sum_Y, lev, num_Y);
  else { // AGGREGATED_MODEL_PAIR -> 2 sets of qoi per response map
    using std::isfinite;
    Real fn_l, prod_l, fn_lm1, prod_lm1;
    int ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter y_it;

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	prod_lm1 = fn_lm1 = fn_vals[qoi];
	prod_l   = fn_l   = fn_vals[qoi+numFunctions];

	if (isfinite(fn_l) && isfinite(fn_lm1)) { // neither NaN nor +/-Inf
	  y_it = sum_Y.begin(); ord = y_it->first; active_ord = 1;
	  while (y_it!=sum_Y.end()) {
    
	    if (ord == active_ord) {
	      y_it->second(qoi,lev) += prod_l - prod_lm1; ++y_it;
	      ord = (y_it == sum_Y.end()) ? 0 : y_it->first;
	    }

	    prod_l *= fn_l; prod_lm1 *= fn_lm1;
	    ++active_ord;
	  }
	  ++num_Y[qoi];
	}
      }
      //Cout << r_it->first << ": " << sum_Y[1];
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_HH, size_t lev,
		      SizetArray& num_L, SizetArray& num_H)
{
  using std::isfinite;
  Real lf_l, hf_l, lf_l_prod, hf_l_prod;
  IntRespMCIter lf_r_it, hf_r_it;
  IntRMMIter ls_it, lr_it, h_it, ll_it, lh_it, hh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, hh_ord, active_ord;
  size_t qoi;

  for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
       lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
       ++lf_r_it, ++hf_r_it) {
    const RealVector& lf_fn_vals = lf_r_it->second.function_values();
    const RealVector& hf_fn_vals = hf_r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_l_prod = lf_l = lf_fn_vals[qoi];
      hf_l_prod = hf_l = hf_fn_vals[qoi];

      // sync sample counts for all L and H interactions at this level
      if (isfinite(lf_l) && isfinite(hf_l)) { // neither NaN nor +/-Inf
	ls_it = sum_L_shared.begin(); lr_it = sum_L_refined.begin();
	h_it  = sum_H.begin();  ll_it = sum_LL.begin();
	lh_it = sum_LH.begin(); hh_it = sum_HH.begin();
	ls_ord = (ls_it == sum_L_shared.end())  ? 0 : ls_it->first;
	lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	h_ord  = (h_it  == sum_H.end())         ? 0 :  h_it->first;
	ll_ord = (ll_it == sum_LL.end())        ? 0 : ll_it->first;
	lh_ord = (lh_it == sum_LH.end())        ? 0 : lh_it->first;
	hh_ord = (hh_it == sum_HH.end())        ? 0 : hh_it->first;
	active_ord = 1;

	while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
	       h_it !=sum_H.end()  || ll_it!=sum_LL.end() ||
	       lh_it!=sum_LH.end() || hh_it!=sum_HH.end()) {

	  // Low shared
	  if (ls_ord == active_ord) {
	    ls_it->second(qoi,lev) += lf_l_prod; ++ls_it;
	    ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	  }
	  // Low refined
	  if (lr_ord == active_ord) {
	    lr_it->second(qoi,lev) += lf_l_prod; ++lr_it;
	    lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	  }
	  // High
	  if (h_ord == active_ord) {
	    h_it->second(qoi,lev) += hf_l_prod; ++h_it;
	    h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  // Low-Low
	  if (ll_ord == active_ord) {
	    ll_it->second(qoi,lev) += lf_l_prod * lf_l_prod; ++ll_it;
	    ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	  }
	  // Low-High
	  if (lh_ord == active_ord) {
	    lh_it->second(qoi,lev) += lf_l_prod * hf_l_prod; ++lh_it;
	    lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	  }
	  // High-High (no map, only a single matrix for order 1)
	  if (hh_ord == active_ord) {
	    hh_it->second(qoi,lev) += hf_l_prod * hf_l_prod; ++hh_it;
	    hh_ord = (hh_it == sum_HH.end()) ? 0 : hh_it->first;
	  }

	  if (ls_ord || lr_ord || ll_ord || lh_ord) lf_l_prod *= lf_l;
	  if (h_ord  || lh_ord || hh_ord)           hf_l_prod *= hf_l;
	  ++active_ord;
	}
	++num_L[qoi]; ++num_H[qoi];
      }
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      RealMatrix& sum_L_shared, RealMatrix& sum_L_refined,
		      RealMatrix& sum_H, RealMatrix& sum_LL,
		      RealMatrix& sum_LH, RealMatrix& sum_HH, size_t lev,
		      SizetArray& N_shared)
{
  using std::isfinite;
  Real lf_l, hf_l, lf_l_prod, hf_l_prod;
  IntRespMCIter lf_r_it, hf_r_it;
  size_t qoi;

  for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
       lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
       ++lf_r_it, ++hf_r_it) {
    const RealVector& lf_fn_vals = lf_r_it->second.function_values();
    const RealVector& hf_fn_vals = hf_r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_l_prod = lf_l = lf_fn_vals[qoi];
      hf_l_prod = hf_l = hf_fn_vals[qoi];

      // sync sample counts for all L and H interactions at this level
      if (isfinite(lf_l) && isfinite(hf_l)) { // neither NaN nor +/-Inf

	++N_shared[qoi];

	sum_L_shared(qoi,lev)  += lf_l_prod;             // Low shared
	sum_L_refined(qoi,lev) += lf_l_prod;             // Low refined
	sum_H(qoi,lev)         += hf_l_prod;             // High
	sum_LL(qoi,lev)        += lf_l_prod * lf_l_prod; // Low-Low
	sum_LH(qoi,lev)        += lf_l_prod * hf_l_prod; // Low-High
	sum_HH(qoi,lev)        += hf_l_prod * hf_l_prod; // High-High

	lf_l_prod *= lf_l;  hf_l_prod *= hf_l;
      }
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Ysums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_HH, size_t lev,
		      SizetArray& num_L, SizetArray& num_H)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODEL_PAIR (lev > 0) modes.  IntRealMatrixMaps are for
  // multilevel case with discrepancies, indexed by level.

  if (lev == 0) // BYPASS_SURROGATE -> 1 set of qoi per response map
    accumulate_mlmf_Qsums(lf_resp_map, hf_resp_map, sum_L_shared, sum_L_refined,
			  sum_H, sum_LL, sum_LH, sum_HH, lev, num_L, num_H);
  else { // AGGREGATED_MODEL_PAIR -> 2 sets of qoi per response map
    using std::isfinite;
    Real lf_l, lf_l_prod, lf_lm1, lf_lm1_prod,
         hf_l, hf_l_prod, hf_lm1, hf_lm1_prod;
    IntRespMCIter lf_r_it, hf_r_it;
    IntRMMIter ls_it, lr_it, h_it, ll_it, lh_it, hh_it;
    int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, hh_ord, active_ord;
    size_t qoi;

    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	lf_lm1_prod = lf_lm1 = lf_fn_vals[qoi];
	lf_l_prod   = lf_l   = lf_fn_vals[qoi+numFunctions];
	hf_lm1_prod = hf_lm1 = hf_fn_vals[qoi];
	hf_l_prod   = hf_l   = hf_fn_vals[qoi+numFunctions];

	// sync sample counts for all L and H interactions at this level
	if (isfinite(lf_l) && isfinite(lf_lm1) &&
	    isfinite(hf_l) && isfinite(hf_lm1)) { // neither NaN nor +/-Inf
	  ls_it  = sum_L_shared.begin(); lr_it = sum_L_refined.begin();
	  h_it   = sum_H.begin();  ll_it = sum_LL.begin();
	  lh_it  = sum_LH.begin(); hh_it = sum_HH.begin();
	  ls_ord = (ls_it == sum_L_shared.end())  ? 0 : ls_it->first;
	  lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	  h_ord  = (h_it  == sum_H.end())         ? 0 :  h_it->first;
	  ll_ord = (ll_it == sum_LL.end())        ? 0 : ll_it->first;
	  lh_ord = (lh_it == sum_LH.end())        ? 0 : lh_it->first;
	  hh_ord = (hh_it == sum_HH.end())        ? 0 : hh_it->first;
	  active_ord = 1;

	  while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
		 h_it !=sum_H.end()  || ll_it!=sum_LL.end() ||
		 lh_it!=sum_LH.end() || hh_it!=sum_HH.end()) {

	    // Low shared
	    if (ls_ord == active_ord) {
	      ls_it->second(qoi,lev) += lf_l_prod - lf_lm1_prod; ++ls_it;
	      ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	    }
	    // Low refined
	    if (lr_ord == active_ord) {
	      lr_it->second(qoi,lev) += lf_l_prod - lf_lm1_prod; ++lr_it;
	      lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	    }
	    // High
	    if (h_ord == active_ord) {
	      h_it->second(qoi,lev) += hf_l_prod - hf_lm1_prod; ++h_it;
	      h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) {
	      Real delta_prod = lf_l_prod - lf_lm1_prod;
	      ll_it->second(qoi,lev) += delta_prod * delta_prod;
	      ++ll_it; ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High
	    if (lh_ord == active_ord) {
	      lh_it->second(qoi,lev) += (lf_l_prod - lf_lm1_prod)
		*  (hf_l_prod - hf_lm1_prod);
	      ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }
	    // High-High (map only contains order 1 in some contexts)
	    if (hh_ord == active_ord) {
	      Real delta_prod = hf_l_prod - hf_lm1_prod;
	      hh_it->second(qoi,lev) += delta_prod * delta_prod;
	      ++hh_it; hh_ord = (hh_it == sum_HH.end()) ? 0 : hh_it->first;
	    }

	    if (ls_ord || lr_ord || ll_ord || lh_ord)
	      { lf_l_prod *= lf_l; lf_lm1_prod *= lf_lm1; }
	    if (h_ord || lh_ord || hh_ord)
	      { hf_l_prod *= hf_l; hf_lm1_prod *= hf_lm1; }
	    ++active_ord;
	  }
	  ++num_L[qoi]; ++num_H[qoi];
	}
      }
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      IntRealMatrixMap& sum_Ll, IntRealMatrixMap& sum_Llm1,
		      IntRealMatrixMap& sum_Ll_refined,
		      IntRealMatrixMap& sum_Llm1_refined,
		      IntRealMatrixMap& sum_Hl, IntRealMatrixMap& sum_Hlm1,
		      IntRealMatrixMap& sum_Ll_Ll,
		      IntRealMatrixMap& sum_Ll_Llm1,
		      IntRealMatrixMap& sum_Llm1_Llm1,
		      IntRealMatrixMap& sum_Hl_Ll,
		      IntRealMatrixMap& sum_Hl_Llm1,
		      IntRealMatrixMap& sum_Hlm1_Ll,
		      IntRealMatrixMap& sum_Hlm1_Llm1,
		      IntRealMatrixMap& sum_Hl_Hl,
		      IntRealMatrixMap& sum_Hl_Hlm1,
		      IntRealMatrixMap& sum_Hlm1_Hlm1, size_t lev,
		      SizetArray& num_L, SizetArray& num_H)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODEL_PAIR (lev > 0) modes.  IntRealMatrixMaps are for
  // multilevel case with discrepancies, indexed by level.

  if (lev == 0) // level lm1 not available; accumulate only level l
    accumulate_mlmf_Qsums(lf_resp_map, hf_resp_map, sum_Ll, sum_Ll_refined,
			  sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl, lev,
			  num_L, num_H);
  else {
    using std::isfinite;
    Real lf_l_prod, lf_l, lf_lm1_prod, lf_lm1,
      hf_l_prod, hf_l, hf_lm1_prod, hf_lm1;
    IntRespMCIter lf_r_it, hf_r_it;
    IntRMMIter l1_it, l2_it, lr1_it, lr2_it, h1_it, h2_it, ll1_it, ll2_it,
      ll3_it, lh1_it, lh2_it, lh3_it, lh4_it, hh1_it, hh2_it, hh3_it;
    int active_ord, l1_ord, l2_ord, lr1_ord, lr2_ord, h1_ord, h2_ord,
      ll1_ord, ll2_ord, ll3_ord, lh1_ord, lh2_ord, lh3_ord, lh4_ord,
      hh1_ord, hh2_ord, hh3_ord;
    size_t qoi;

    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	lf_lm1_prod = lf_lm1 = lf_fn_vals[qoi];
	lf_l_prod   = lf_l   = lf_fn_vals[qoi+numFunctions];
	hf_lm1_prod = hf_lm1 = hf_fn_vals[qoi];
	hf_l_prod   = hf_l   = hf_fn_vals[qoi+numFunctions];

	// sync sample counts for all L and H interactions at this level
	if (isfinite(lf_l) && isfinite(lf_lm1) &&
	    isfinite(hf_l) && isfinite(hf_lm1)) { // neither NaN nor +/-Inf
	  // Low: Ll, Llm1, Ll_refined, Llm1_refined
	  l1_it   = sum_Ll.begin();         l2_it  = sum_Llm1.begin();
	  lr1_it  = sum_Ll_refined.begin(); lr2_it = sum_Llm1_refined.begin();
	  l1_ord  = (l1_it == sum_Ll.end())            ? 0 : l1_it->first;
	  l2_ord  = (l2_it == sum_Llm1.end())          ? 0 : l2_it->first;
	  lr1_ord = (lr1_it == sum_Ll_refined.end())   ? 0 : lr1_it->first;
	  lr2_ord = (lr2_it == sum_Llm1_refined.end()) ? 0 : lr2_it->first;
	  // High: Hl, Hlm1
	  h1_it  = sum_Hl.begin();
	  h2_it  = sum_Hlm1.begin();
	  h1_ord = (h1_it == sum_Hl.end())   ? 0 : h1_it->first;
	  h2_ord = (h2_it == sum_Hlm1.end()) ? 0 : h2_it->first;
	  // Low-Low: Ll_Ll, Ll_Llm1, Llm1_Llm1
	  ll1_it = sum_Ll_Ll.begin(); ll2_it = sum_Ll_Llm1.begin();
	  ll3_it = sum_Llm1_Llm1.begin();
	  ll1_ord = (ll1_it == sum_Ll_Ll.end())     ? 0 : ll1_it->first;
	  ll2_ord = (ll2_it == sum_Ll_Llm1.end())   ? 0 : ll2_it->first;
	  ll3_ord = (ll3_it == sum_Llm1_Llm1.end()) ? 0 : ll3_it->first;
	  // Low-High: Hl_Ll, Hl_Llm1, Hlm1_Ll, Hlm1_Llm1
	  lh1_it = sum_Hl_Ll.begin();   lh2_it = sum_Hl_Llm1.begin();
	  lh3_it = sum_Hlm1_Ll.begin(); lh4_it = sum_Hlm1_Llm1.begin();
	  lh1_ord = (lh1_it == sum_Hl_Ll.end())     ? 0 : lh1_it->first;
	  lh2_ord = (lh2_it == sum_Hl_Llm1.end())   ? 0 : lh2_it->first;
	  lh3_ord = (lh3_it == sum_Hlm1_Ll.end())   ? 0 : lh3_it->first;
	  lh4_ord = (lh4_it == sum_Hlm1_Llm1.end()) ? 0 : lh4_it->first;
	  // High-High: Hl_Hl, Hl_Hlm1, Hlm1_Hlm1
	  hh1_it = sum_Hl_Hl.begin();   hh2_it = sum_Hl_Hlm1.begin();
	  hh3_it = sum_Hlm1_Hlm1.begin();
	  hh1_ord = (hh1_it == sum_Hl_Hl.end())     ? 0 : hh1_it->first;
	  hh2_ord = (hh2_it == sum_Hl_Hlm1.end())   ? 0 : hh2_it->first;
	  hh3_ord = (hh3_it == sum_Hlm1_Hlm1.end()) ? 0 : hh3_it->first;

	  active_ord = 1;

	  while (l1_it !=sum_Ll.end()         || l2_it !=sum_Llm1.end()      ||
		 lr1_it!=sum_Ll_refined.end() ||
		 lr2_it!=sum_Llm1_refined.end() || h1_it !=sum_Hl.end()      ||
		 h2_it !=sum_Hlm1.end()       || ll1_it!=sum_Ll_Ll.end()     ||
		 ll2_it!=sum_Ll_Llm1.end()    || ll3_it!=sum_Llm1_Llm1.end() ||
		 lh1_it!=sum_Hl_Ll.end()      || lh2_it!=sum_Hl_Llm1.end()   ||
		 lh3_it!=sum_Hlm1_Ll.end()    || lh4_it!=sum_Hlm1_Llm1.end() ||
		 hh1_it!=sum_Hl_Hl.end()      || hh2_it!=sum_Hl_Hlm1.end()   ||
		 hh3_it!=sum_Hlm1_Hlm1.end()) {

	    // Low: Ll, Llm1, Ll_refined, Llm1_refined
	    if (l1_ord == active_ord) {
	      l1_it->second(qoi,lev) += lf_l_prod; ++l1_it;
	      l1_ord = (l1_it == sum_Ll.end()) ? 0 : l1_it->first;
	    }
	    if (l2_ord == active_ord) {
	      l2_it->second(qoi,lev) += lf_lm1_prod; ++l2_it;
	      l2_ord = (l2_it == sum_Llm1.end()) ? 0 : l2_it->first;
	    }
	    if (lr1_ord == active_ord) {
	      lr1_it->second(qoi,lev) += lf_l_prod; ++lr1_it;
	      lr1_ord = (lr1_it == sum_Ll_refined.end()) ? 0 : lr1_it->first;
	    }
	    if (lr2_ord == active_ord) {
	      lr2_it->second(qoi,lev) += lf_lm1_prod; ++lr2_it;
	      lr2_ord = (lr2_it == sum_Llm1_refined.end()) ? 0 : lr2_it->first;
	    }
	    // High: Hl, Hlm1
	    if (h1_ord == active_ord) {
	      h1_it->second(qoi,lev) += hf_l_prod; ++h1_it;
	      h1_ord = (h1_it == sum_Hl.end()) ? 0 : h1_it->first;
	    }
	    if (h2_ord == active_ord) {
	      h2_it->second(qoi,lev) += hf_lm1_prod; ++h2_it;
	      h2_ord = (h2_it == sum_Hlm1.end()) ? 0 : h2_it->first;
	    }
	    // Low-Low: Ll_Ll, Ll_Llm1, Llm1_Llm1
	    if (ll1_ord == active_ord) {
	      ll1_it->second(qoi,lev) += lf_l_prod * lf_l_prod; ++ll1_it;
	      ll1_ord = (ll1_it == sum_Ll_Ll.end()) ? 0 : ll1_it->first;
	    }
	    if (ll2_ord == active_ord) {
	      ll2_it->second(qoi,lev) += lf_l_prod * lf_lm1_prod; ++ll2_it;
	      ll2_ord = (ll2_it == sum_Ll_Llm1.end()) ? 0 : ll2_it->first;
	    }
	    if (ll3_ord == active_ord) {
	      ll3_it->second(qoi,lev) += lf_lm1_prod * lf_lm1_prod; ++ll3_it;
	      ll3_ord = (ll3_it == sum_Llm1_Llm1.end()) ? 0 : ll3_it->first;
	    }
	    // Low-High: Hl_Ll, Hl_Llm1, Hlm1_Ll, Hlm1_Llm1
	    if (lh1_ord == active_ord) {
	      lh1_it->second(qoi,lev) += hf_l_prod * lf_l_prod; ++lh1_it;
	      lh1_ord = (lh1_it == sum_Hl_Ll.end()) ? 0 : lh1_it->first;
	    }
	    if (lh2_ord == active_ord) {
	      lh2_it->second(qoi,lev) += hf_l_prod * lf_lm1_prod; ++lh2_it;
	      lh2_ord = (lh2_it == sum_Hl_Llm1.end()) ? 0 : lh2_it->first;
	    }
	    if (lh3_ord == active_ord) {
	      lh3_it->second(qoi,lev) += hf_lm1_prod * lf_l_prod; ++lh3_it;
	      lh3_ord = (lh3_it == sum_Hlm1_Ll.end()) ? 0 : lh3_it->first;
	    }
	    if (lh4_ord == active_ord) {
	      lh4_it->second(qoi,lev) += hf_lm1_prod * lf_lm1_prod; ++lh4_it;
	      lh4_ord = (lh4_it == sum_Hlm1_Llm1.end()) ? 0 : lh4_it->first;
	    }
	    // High-High: Hl_Hl, Hl_Hlm1, Hlm1_Hlm1
	    if (hh1_ord == active_ord) {
	      hh1_it->second(qoi,lev) += hf_l_prod * hf_l_prod; ++hh1_it;
	      hh1_ord = (hh1_it == sum_Hl_Hl.end()) ? 0 : hh1_it->first;
	    }
	    if (hh2_ord == active_ord) {
	      hh2_it->second(qoi,lev) += hf_l_prod * hf_lm1_prod; ++hh2_it;
	      hh2_ord = (hh2_it == sum_Hl_Hlm1.end()) ? 0 : hh2_it->first;
	    }
	    if (hh3_ord == active_ord) {
	      hh3_it->second(qoi,lev) += hf_lm1_prod * hf_lm1_prod; ++hh3_it;
	      hh3_ord = (hh3_it == sum_Hlm1_Hlm1.end()) ? 0 : hh3_it->first;
	    }

	    if (l1_ord || lr1_ord || ll1_ord || ll2_ord || lh1_ord || lh3_ord)
	      lf_l_prod   *= lf_l;
	    if (l2_ord || lr2_ord || ll2_ord || ll3_ord || lh2_ord || lh4_ord)
	      lf_lm1_prod *= lf_lm1;
	    if (h1_ord || lh1_ord || lh2_ord || hh1_ord || hh2_ord)
	      hf_l_prod   *= hf_l;
	    if (h2_ord || lh3_ord || lh4_ord || hh2_ord || hh3_ord)
	      hf_lm1_prod *= hf_lm1;
	    ++active_ord;
	  }
	  ++num_L[qoi]; ++num_H[qoi];
	}
      }
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      RealMatrix& sum_Ll, RealMatrix& sum_Llm1,
		      RealMatrix& sum_Ll_refined, RealMatrix& sum_Llm1_refined,
		      RealMatrix& sum_Hl, RealMatrix& sum_Hlm1,
		      RealMatrix& sum_Ll_Ll,     RealMatrix& sum_Ll_Llm1,
		      RealMatrix& sum_Llm1_Llm1, RealMatrix& sum_Hl_Ll,
		      RealMatrix& sum_Hl_Llm1,   RealMatrix& sum_Hlm1_Ll,
		      RealMatrix& sum_Hlm1_Llm1, RealMatrix& sum_Hl_Hl,
		      RealMatrix& sum_Hl_Hlm1,   RealMatrix& sum_Hlm1_Hlm1,
		      size_t lev, SizetArray& N_shared)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODEL_PAIR (lev > 0) modes.

  if (lev == 0) // level lm1 not available; accumulate only level l
    accumulate_mlmf_Qsums(lf_resp_map, hf_resp_map, sum_Ll, sum_Ll_refined,
			  sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl, lev,
			  N_shared);
  else {
    using std::isfinite;
    Real lf_l_prod, lf_l, lf_lm1_prod, lf_lm1, hf_l_prod, hf_l,
      hf_lm1_prod, hf_lm1;
    IntRespMCIter lf_r_it, hf_r_it;
    size_t qoi;

    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	lf_lm1_prod = lf_lm1 = lf_fn_vals[qoi];
	lf_l_prod   = lf_l   = lf_fn_vals[qoi+numFunctions];
	hf_lm1_prod = hf_lm1 = hf_fn_vals[qoi];
	hf_l_prod   = hf_l   = hf_fn_vals[qoi+numFunctions];

	// sync sample counts for all L and H interactions at this level
	if (isfinite(lf_l) && isfinite(lf_lm1) &&
	    isfinite(hf_l) && isfinite(hf_lm1)) { // neither NaN nor +/-Inf

	  ++N_shared[qoi];

	  // Low: Ll, Llm1, Ll_refined, Llm1_refined
	  sum_Ll(qoi,lev)           += lf_l_prod;
	  sum_Llm1(qoi,lev)         += lf_lm1_prod;
	  sum_Ll_refined(qoi,lev)   += lf_l_prod;
	  sum_Llm1_refined(qoi,lev) += lf_lm1_prod;
	  // High: Hl, Hlm1
	  sum_Hl(qoi,lev)   += hf_l_prod;
	  sum_Hlm1(qoi,lev) += hf_lm1_prod;
	  // Low-Low: Ll_Ll, Ll_Llm1, Llm1_Llm1
	  sum_Ll_Ll(qoi,lev)     += lf_l_prod * lf_l_prod;
	  sum_Ll_Llm1(qoi,lev)   += lf_l_prod * lf_lm1_prod;
	  sum_Llm1_Llm1(qoi,lev) += lf_lm1_prod * lf_lm1_prod;
	  // Low-High: Hl_Ll, Hl_Llm1, Hlm1_Ll, Hlm1_Llm1
	  sum_Hl_Ll(qoi,lev)     += hf_l_prod * lf_l_prod;
	  sum_Hl_Llm1(qoi,lev)   += hf_l_prod * lf_lm1_prod;
	  sum_Hlm1_Ll(qoi,lev)   += hf_lm1_prod * lf_l_prod;
	  sum_Hlm1_Llm1(qoi,lev) += hf_lm1_prod * lf_lm1_prod;
	  // High-High: Hl_Hl, Hl_Hlm1, Hlm1_Hlm1
	  sum_Hl_Hl(qoi,lev)     += hf_l_prod * hf_l_prod;
	  sum_Hl_Hlm1(qoi,lev)   += hf_l_prod * hf_lm1_prod;
	  sum_Hlm1_Hlm1(qoi,lev) += hf_lm1_prod * hf_lm1_prod;

	  lf_l_prod *= lf_l;  lf_lm1_prod *= lf_lm1;
	  hf_l_prod *= hf_l;  hf_lm1_prod *= hf_lm1;
	}
      }
    }
  }
}


void NonDMultilevControlVarSampling::print_variance_reduction(std::ostream& s)
{
  switch (delegateMethod) {
  case MULTILEVEL_SAMPLING: // not currently overridden by MLMC
    NonDMultilevelSampling::print_variance_reduction(s);     break;
  //case MULTIFIDELITY_SAMPLING: // MFMC not inherited as CVMC was
  //  NonDMultifidelitySampling::print_variance_reduction(s);  break;
  default: {
    Real avg_mlmc_estvar0, avg_budget_mc_estvar;
    String type = (pilotProjection) ? "Projected":"   Online";
    size_t wpp7 = write_precision + 7;
    s << "<<<<< Variance for mean estimator:\n";
    switch (pilotMgmtMode) {
    case OFFLINE_PILOT:
      s << "  " << type << " MLCVMC (sample profile):   "
	<< std::setw(wpp7) << avgEstVar << '\n';
      break;
    default:
      avg_mlmc_estvar0 = average(estVarIter0);
      s << "      Initial MLMC (pilot samples):    " << std::setw(wpp7)
	<< avg_mlmc_estvar0 << "\n  "
	<< type << " MLCVMC (sample profile):   "
	<< std::setw(wpp7) << avgEstVar	<< "\n  "
	<< type << " MLCVMC / pilot ratio:      "
	// report ratio of averages rather than average of ratios:
	<< std::setw(wpp7) << avgEstVar / avg_mlmc_estvar0 << '\n';
      break;
    }
    if (finalStatsType == QOI_STATISTICS)
      switch (pilotMgmtMode) {
      case ONLINE_PILOT: case OFFLINE_PILOT: {
	Real proj_equiv_hf = equivHFEvals + deltaEquivHF;
	avg_budget_mc_estvar = average(varH) / proj_equiv_hf;
	s << " Equivalent     MC (" << std::setw(5)
	  << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
	  << std::setw(wpp7) << avg_budget_mc_estvar
	  << "\n Equivalent MLCVMC / MC ratio:         " << std::setw(wpp7)
	  << avgEstVar / avg_budget_mc_estvar << '\n';
	break;
      }
      }
    break;
  }
  }
}

} // namespace Dakota
