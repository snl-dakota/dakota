/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
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
NonDMultilevControlVarSampling(ProblemDescDB& problem_db, std::shared_ptr<Model> model):
  NonDMultilevelSampling(problem_db, model),
  //NonDMultifidelitySampling(problem_db, model),
  NonDHierarchSampling(problem_db, model), // top of virtual inheritance
  delegateMethod(MULTILEVEL_MULTIFIDELITY_SAMPLING)
{
  // override MULTILEVEL_PRECEDENCE from NonDMultilevel ctor
  iteratedModel->ensemble_precedence(MULTILEVEL_MULTIFIDELITY_PRECEDENCE);
  // Note: only sequenceType is currently used by MLCV
  configure_2d_sequence(numSteps, secondaryIndex, sequenceType);
  numApprox  = numSteps - 1; // numSteps is total = num_cv_lev + num_hf_lev
  costSource = configure_cost(numSteps, sequenceType, sequenceCost,
			      costMetadataIndices);
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
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within an EnsembleSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDMultilevControlVarSampling::core_run()
{
  // Can trap mis-specification of the MLMF method and delegate to
  // inherited core_run() implementations:
  switch (sequenceType) { // 1d fallbacks selected in configure_2d_sequence()
  case Pecos::RESOLUTION_LEVEL_1D_SEQUENCE:
    delegateMethod = MULTILEVEL_SAMPLING;
    NonDMultilevelSampling::core_run(); // reinvokes configure_1d_sequence()
    return;  break;
  // MFMC not inherited as CVMC was, so rely on MLCV with 2 models, num_hf_lev=1
  //case Pecos::MODEL_FORM_1D_SEQUENCE:
    //delegateMethod = MULTIFIDELITY_SAMPLING;
    //NonDMultifidelitySampling::core_run();
    //return;  break;
  }

  // assign an aggregate model key that does not need to be updated
  // (activeSet identifies QoIs for the active model subset)
  assign_active_key();

  switch (pilotMgmtMode) {
  case ONLINE_PILOT:
    //if (true) // reformulated approach using 1 new QoI correlation per level
    //  multilevel_control_variate_mc_Qcorr();
    //else      // original approach using 1 discrepancy correlation per level
    //  multilevel_control_variate_mc_Ycorr();
    multilevel_control_variate_mc_online_pilot(); //_Qcorr();
    break;
  case OFFLINE_PILOT:
    switch (finalStatsType) {
    // since offline is not iterated, the ESTIMATOR_PERFORMANCE case is the
    // same as OFFLINE_PILOT_PROJECTION --> bypass IntMaps, simplify code
    case ESTIMATOR_PERFORMANCE:
      multilevel_control_variate_mc_pilot_projection();  break;
    default:
      multilevel_control_variate_mc_offline_pilot();     break;
    }
    break;
  case ONLINE_PILOT_PROJECTION:  case OFFLINE_PILOT_PROJECTION:
    multilevel_control_variate_mc_pilot_projection(); break;
  }
  // ML performed on HF + CV applied per level using LF if available:
  // perform MLMC on HF model and bind 1:min(num_hf,num_lf) LF control
  // variates starting at coarsest level
}


void NonDMultilevControlVarSampling::assign_active_key()
{
  // For two-model control variate methods, select lowest,highest fidelities
  size_t num_mf = NLevActual.size();
  unsigned short lf_form = 0, hf_form = num_mf - 1;// ordered lo:hi
  size_t num_hf_lev = NLevActual[hf_form].size(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, NLevActual[lf_form].size()) : 0;

  std::vector<Pecos::ActiveKey> form_res_keys(num_cv_lev + num_hf_lev);
  size_t l, cntr = 0;
  for (l=0; l<num_cv_lev; ++l, ++cntr)
    form_res_keys[cntr].form_key(0, lf_form, l);
  for (l=0; l<num_hf_lev; ++l, ++cntr)
    form_res_keys[cntr].form_key(0, hf_form, l);

  Pecos::ActiveKey active_key;
  active_key.aggregate_keys(form_res_keys, Pecos::RAW_DATA);
  iteratedModel->active_model_key(active_key); // data group 0
  resize_active_set();
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC si employed
    across two model forms to exploit correlation in the discrepancies
    at each level (Y_l).
void NonDMultilevControlVarSampling::multilevel_control_variate_mc_Ycorr()
{
  size_t qoi, lev, num_mf = NLevActual.size(),
    num_hf_lev = iteratedModel.truth_model()->solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel.surrogate_model()->solution_levels()) : 0;
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets(num_hf_lev), agg_var_hf(num_hf_lev),
    hf_cost = truth_model.solution_level_costs(),
    lf_cost =  surr_model.solution_level_costs();
  Real eps_sq_div_2, sum_sqrt_var_cost, agg_estvar_iter0 = 0., budget, r_lq,
    lf_lev_cost, hf_lev_cost, hf_ref_cost = hf_cost.back();
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
	  accumulate_ml_Ysums(sum_H, sum_HH1, lev, num_cv_lev, N_hf[lev]);
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
      delta_N_hf[lev]
        = one_sided_delta(N_alloc_hf[lev], hf_targets[lev], relaxFactor);
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
  RealMatrix Y_mom(4, numFunctions), Y_cvmc_mom(4, numFunctions, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf[lev],
		   sum_L_refined, N_lf[lev], //rho2_LH,
		   lev, Y_cvmc_mom);
    Y_mom += Y_cvmc_mom;
  }
  if (num_hf_lev > num_cv_lev)
    ml_Y_raw_moments(sum_H[1], sum_H[2], sum_H[3], sum_H[4], N_hf,
		     num_cv_lev, num_hf_lev, Y_mom);
  convert_moments(Y_mom, momentStats); // raw to final (central or std)

  RealMatrix Y_mlmc_mom(4, numFunctions), mlmc_stats;
  ml_Q_raw_moments(..., N_actual_hf, 0, num_cv_lev, Y_mlmc_mom);
  ml_Y_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
		   num_cv_lev, num_hf_lev, Y_mlmc_mom);
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
void NonDMultilevControlVarSampling::
multilevel_control_variate_mc_online_pilot() //_Qcorr()
{
  size_t qoi, lev, num_mf = NLevActual.size(), N_alloc_l,
    num_hf_lev = iteratedModel->truth_model()->solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel->surrogate_model()->solution_levels()) : 0;
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets(num_hf_lev), lf_targets, agg_var_hf(num_hf_lev);
  Real eps_sq_div_2, sum_sqrt_var_cost, agg_estvar_iter0 = 0., budget, r_lq,
    lf_lev_cost, hf_lev_cost, hf_ref_cost, hf_tgt_l;
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

    // FIRST PASS: launch and synchonize sample batches
    mlmf_increments(delta_N_hf, "mlcv_");
    if (mlmfIter == 0) {
      if (costSource != USER_COST_SPEC)
	recover_online_cost(batchResponsesMap); // define sequenceCost for LF,HF
      hf_ref_cost = sequenceCost[numApprox];
      if (budget_constrained) budget = (Real)maxFunctionEvals * hf_ref_cost;
      // Note: could assign these back if needed elsewhere:
      //if (online_hf_cost) truth_model.solution_level_costs(hf_cost);
      //if (online_lf_cost)  surr_model.solution_level_costs(lf_cost);
    }

    // SECOND PASS: accumulations and cost estimates
    // NOTE: this will also simplify removing non-essential synchronizations
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      numSamples = delta_N_hf[lev];
      if (numSamples) {
	N_alloc_l = (backfillFailures && mlmfIter) ?
	  one_sided_delta(N_alloc_hf[lev], hf_targets[lev], relaxFactor) :
	  numSamples;
	N_alloc_hf[lev] += N_alloc_l;

	// control variate betwen LF and HF for this discretization level:
	// if unequal number of levels, LF levels are assigned as CV to the
	// leading set of HF levels, since these tend to have larger variance.
	IntResponseMap& mlmf_resp_map = batchResponsesMap[lev];
	if (lev < num_cv_lev) {
	  // > Note: this CV sample set is not separately exported using
	  //   export_all_samples() since it is the same as the ML set.
	  // > This is why the preceding set is marked as "mlcv_", indicating
	  //   that the parameter sets should be applied to both ML and CV.
	  accumulate_mlmf_Qsums(mlmf_resp_map, sum_Ll, sum_Llm1, sum_Ll_refined,
				sum_Llm1_refined, sum_Hl, sum_Hlm1, sum_Ll_Ll,
				sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll,
				sum_Hl_Llm1, sum_Hlm1_Ll, sum_Hlm1_Llm1,
				sum_Hl_Hl, sum_Hl_Hlm1,	sum_Hlm1_Hlm1, lev,
				N_actual_lf[lev], N_actual_hf[lev]);
	  N_alloc_lf[lev] += N_alloc_l;
	}
	else // no LF for this level; accumulate only multilevel discrepancies
	  accumulate_ml_Ysums(mlmf_resp_map, sum_Hl, sum_Hl_Hl[1], lev,
			      num_cv_lev, N_actual_hf[lev]);// offset for HF lev
      }
    }
    clear_batches();

    // FINAL PASS: STATS
    sum_sqrt_var_cost = 0.;
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      hf_lev_cost = level_cost(sequenceCost, lev, num_cv_lev);
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      // control variate betwen LF and HF for this discretization level
      if (lev < num_cv_lev) {
	lf_lev_cost = level_cost(sequenceCost, lev);
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
      hf_lev_cost = level_cost(sequenceCost, lev, num_cv_lev);
      hf_tgt_l = hf_targets[lev] = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho_dot2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = (backfillFailures) ?
	one_sided_delta(N_actual_hf[lev], hf_tgt_l, relaxFactor) :
	one_sided_delta(N_alloc_hf[lev],  hf_tgt_l, relaxFactor);
      // Note: N_alloc_{lf,hf} accumulated upstream due to maxIterations exit
    }

    ++mlmfIter;  advance_relaxation();
    Cout << "\nMLMF MC iteration " << mlmfIter << " HF sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS) {

    SizetArray delta_N_lf(num_cv_lev);
    for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) // no relaxation
      if (backfillFailures) { // increment relative to successful samples
	delta_N_lf[lev] = lf_increment(eval_ratios[lev], N_actual_lf[lev],
				       hf_targets[lev], lf_targets);
	N_alloc_lf[lev]
	  += one_sided_delta(N_alloc_lf[lev], average(lf_targets));
      }
      else { // increment relative to allocated samples
	delta_N_lf[lev] = lf_increment(eval_ratios[lev], N_alloc_lf[lev],
				       hf_targets[lev], lf_targets);
	N_alloc_lf[lev] += delta_N_lf[lev];
      }

    // parallel execution for multiple sample batches across multiple models
    lf_increments(delta_N_lf, "cv_");

    // All CV lf_increment() calls now follow convergence of ML iteration:
    // > Avoids early mis-estimation of LF increments
    // > Parallel scheduling benefits from one final large batch of refinements
    for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
      numSamples = delta_N_lf[lev];
      if (numSamples) {
	accumulate_mlmf_Qsums(batchResponsesMap[lev], sum_Ll_refined,
			      sum_Llm1_refined, lev, N_actual_lf[lev]);
	increment_ml_equivalent_cost(numSamples, level_cost(sequenceCost, lev),
				     hf_ref_cost, equivHFEvals);
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Accumulated sums (L_refined[1,2]):\n" << sum_Ll_refined[1]
	       << sum_Ll_refined[2];
      }
    }
    clear_batches();

    // Roll up raw moments from MLCVMC and MLMC levels
    RealMatrix Y_mom(4, numFunctions), Y_cvmc_mom(4, numFunctions, false);
    for (lev=0; lev<num_cv_lev; ++lev) {
      cv_raw_moments(sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
		     sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
		     sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
		     N_actual_hf[lev], sum_Ll_refined, sum_Llm1_refined,
		     N_actual_lf[lev], /*rho_dot2_LH,*/ lev, Y_cvmc_mom);
      Y_mom += Y_cvmc_mom;
    }
    if (num_hf_lev > num_cv_lev)
      ml_Y_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
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
    //RealMatrix Y_mlmc_mom(4, numFunctions), mlmc_stats;
    //ml_Q_raw_moments(..., N_actual_hf, 0, num_cv_lev, Y_mlmc_mom);
    //ml_Y_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
    //		       num_cv_lev, num_hf_lev, Y_mlmc_mom);
    //convert_moments(Y_mlmc_mom, mlmc_stats); // raw to final (central or std)
    //recover_variance(mlmc_stats, varH);
    //
    // Alternate approach 2: for max MFMC/ACV consistency, only use H_L and N_L
    //compute_variance(sum_Hl[1][L], sum_Hl[2][L],, N_actual_hf[L]);
  }
  else { // for consistency with pilot projection
    update_projected_lf_samples(hf_targets, eval_ratios, N_actual_lf,
				N_alloc_lf, deltaEquivHF);
    // roll up ML moments (no CV without LF increments) for using varH in
    // variance reduction metrics (momentStats not reported for projections)
    RealMatrix Y_mom(2, numFunctions);
    RealMatrix& sum_Hl_1 = sum_Hl[1];  RealMatrix& sum_Hl_2 = sum_Hl[2];
    // For levels 0:num_cv_lev, sum_Hl[p] is Ql^p
    ml_Q_raw_moments(sum_Hl_1, sum_Hlm1[1], sum_Hl_2, sum_Hlm1[2], N_actual_hf,
		     0, num_cv_lev, Y_mom);
    // For levels num_cv_lev:num_hf_lev, sum_Hl[p] is level discrepancy in Q^p,
    // i.e. Ql^p - Qlm1^p, and can be summed directly into raw moments:
    ml_Y_raw_moments(sum_Hl_1, sum_Hl_2, N_actual_hf, num_cv_lev, num_hf_lev,
		     Y_mom);
    convert_moments(Y_mom, momentStats); // raw to final (central or std)
    recover_variance(momentStats, varH);
  }

  compute_mlmf_estimator_variance(var_YH, N_actual_hf, Lambda, estVar);
  avgEstVar = average(estVar);
}


void NonDMultilevControlVarSampling::
multilevel_control_variate_mc_offline_pilot()
{
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets_pilot, lf_targets;
  RealMatrix Lambda_pilot, var_YH_pilot, Y_mom;
  RealVectorArray eval_ratios_pilot;

  size_t lev, num_mf = NLevActual.size(),
    num_hf_lev = iteratedModel->truth_model()->solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel->surrogate_model()->solution_levels()) : 0;
  unsigned short group, lf_form = 0, hf_form = num_mf - 1;// extremes
  SizetArray&    N_alloc_lf =  NLevAlloc[lf_form];
  SizetArray&    N_alloc_hf =  NLevAlloc[hf_form];
  Sizet2DArray& N_actual_lf = NLevActual[lf_form];
  Sizet2DArray& N_actual_hf = NLevActual[hf_form];
  SizetArray N_alloc_pilot;  Sizet2DArray N_actual_pilot(num_hf_lev);
  N_alloc_pilot.assign(num_hf_lev, 0);
  for (lev=0; lev<num_hf_lev; ++lev)
    N_actual_pilot[lev].assign(numFunctions, 0);
  SizetArray delta_N_hf(num_hf_lev);

  // -----------------------------------------
  // Initial loop for offline (overkill) pilot
  // -----------------------------------------
  evaluate_pilot(eval_ratios_pilot, Lambda_pilot, var_YH_pilot, N_alloc_pilot,
		 N_actual_pilot, hf_targets_pilot, Y_mom, false, false);

  // ----------------------------------------------------------
  // Evaluate online sample profile computed from offline pilot
  // ----------------------------------------------------------
  // QOI_STATISTICS case; ESTIMATOR_PERFORMANCE redirects to
  // multilevel_control_variate_mc_pilot_projection() to also bypass IntMaps.
  Real lf_lev_cost, hf_lev_cost, hf_ref_cost = sequenceCost[numApprox];
  IntRealMatrixMap sum_Ll, sum_Llm1, sum_Ll_refined, sum_Llm1_refined, sum_Hl,
    sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1,
    sum_Hlm1_Ll, sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1;
  initialize_mlmf_sums(sum_Ll, sum_Llm1, sum_Ll_refined, sum_Llm1_refined,
		       sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
		       sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
		       sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
		       num_hf_lev, num_cv_lev);

  // min of 2 samp reqd for online var; no relaxation since no iteration
  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group)
    delta_N_hf[lev]
      = std::max(one_sided_delta(0., hf_targets_pilot[lev]), (size_t)2);
  mlmf_increments(delta_N_hf, "mlcv_");

  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

    numSamples = delta_N_hf[lev];  N_alloc_hf[lev] += numSamples;
    hf_lev_cost = level_cost(sequenceCost, lev, num_cv_lev);
    IntResponseMap& mlmf_resp_map = batchResponsesMap[lev];

    if (lev < num_cv_lev) {

      lf_lev_cost = level_cost(sequenceCost, lev);
      // process previous and new set of allResponses for MLMF sums;
      accumulate_mlmf_Qsums(mlmf_resp_map, sum_Ll, sum_Llm1, sum_Ll_refined,
			    sum_Llm1_refined, sum_Hl, sum_Hlm1, sum_Ll_Ll,
			    sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1,
			    sum_Hlm1_Ll, sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
			    sum_Hlm1_Hlm1, lev, N_actual_lf[lev],
			    N_actual_hf[lev]);
      increment_mlmf_equivalent_cost(numSamples, hf_lev_cost, numSamples,
				     lf_lev_cost, hf_ref_cost, equivHFEvals);
      N_alloc_lf[lev] += numSamples;
      // leave evaluation ratios and Lambda at values from Oracle pilot
    }
    else {
      // accumulate H sums for lev = 0, Y sums for lev > 0
      accumulate_ml_Ysums(mlmf_resp_map, sum_Hl, sum_Hl_Hl[1], lev, num_cv_lev,
			  N_actual_hf[lev]); // offet for HF lev
      increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost,
				   equivHFEvals);
    }
  }
  clear_batches();

  // -------------
  // LF increments
  // -------------
  SizetArray delta_N_lf(num_cv_lev);
  for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) // no relaxation
    if (backfillFailures) { // increment relative to successful samples
      delta_N_lf[lev] = lf_increment(eval_ratios_pilot[lev], N_actual_lf[lev],
				     hf_targets_pilot[lev], lf_targets);
      N_alloc_lf[lev]
	+= one_sided_delta(N_alloc_lf[lev], average(lf_targets));
    }
    else { // increment relative to allocated samples
      delta_N_lf[lev] = lf_increment(eval_ratios_pilot[lev], N_alloc_lf[lev],
				     hf_targets_pilot[lev], lf_targets);
      N_alloc_lf[lev] += delta_N_lf[lev];
    }

  // parallel execution for multiple sample batches across multiple models
  lf_increments(delta_N_lf, "cv_");

  // All CV lf_increment() calls now follow convergence of ML iteration:
  // > Avoids early mis-estimation of LF increments
  // > Parallel scheduling benefits from one final large batch of refinements
  for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
    numSamples = delta_N_lf[lev];
    if (numSamples) {
      accumulate_mlmf_Qsums(batchResponsesMap[lev], sum_Ll_refined,
			    sum_Llm1_refined, lev, N_actual_lf[lev]);
      increment_ml_equivalent_cost(numSamples, level_cost(sequenceCost, lev),
				   hf_ref_cost, equivHFEvals);
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Accumulated sums (L_refined[1,2]):\n" << sum_Ll_refined[1]
	     << sum_Ll_refined[2];
    }
  }
  clear_batches();

  // --------------------------------------
  // Final stats from online sample profile
  // --------------------------------------
  Y_mom.shape(4, numFunctions);  RealMatrix Y_cvmc_mom(4, numFunctions, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    cv_raw_moments(sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
		   sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
		   sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
		   N_actual_hf[lev], sum_Ll_refined, sum_Llm1_refined,
		   N_actual_lf[lev], /*rho_dot2_LH,*/ lev, Y_cvmc_mom);
    Y_mom += Y_cvmc_mom;
  }
  if (num_hf_lev > num_cv_lev)
    ml_Y_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
		     num_cv_lev, num_hf_lev, Y_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Y_mom, momentStats);

  recover_variance(momentStats, varH);
  // See explanation of varH recovery options above.
  //RealMatrix Y_mlmc_mom(4, numFunctions), mlmc_stats;
  //ml_Q_raw_moments(..., N_actual_hf, 0, num_cv_lev, Y_mlmc_mom);
  //ml_Y_raw_moments(sum_Hl[1], sum_Hl[2], sum_Hl[3], sum_Hl[4], N_actual_hf,
  //	             num_cv_lev, num_hf_lev, Y_mlmc_mom);
  //convert_moments(Y_mlmc_mom, mlmc_stats); // raw to final (central or std)
  //recover_variance(mlmc_stats, varH);

  // Retain var_YH & Lambda from "oracle" pilot for computing final estimator
  // variances.  This results in mixing offline var_YH,Lambda with online
  // varH,N_hf, but is consistent with offline modes for MFMC and ACV.
  compute_mlmf_estimator_variance(var_YH_pilot,N_actual_hf,Lambda_pilot,estVar);
  avgEstVar = average(estVar);
}


void NonDMultilevControlVarSampling::
multilevel_control_variate_mc_pilot_projection()
{
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_targets;  RealMatrix Lambda, var_YH;
  RealVectorArray eval_ratios;
  size_t lev, num_mf = NLevActual.size(),
    num_hf_lev = iteratedModel->truth_model()->solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel->surrogate_model()->solution_levels()) : 0;

  // Initialize for pilot sample
  unsigned short lf_form = 0, hf_form = NLevActual.size() - 1; // 2 @ extremes
  SizetArray&    N_alloc_lf =  NLevAlloc[lf_form];
  SizetArray&    N_alloc_hf =  NLevAlloc[hf_form];
  Sizet2DArray& N_actual_lf = NLevActual[lf_form];
  Sizet2DArray& N_actual_hf = NLevActual[hf_form];

  RealMatrix pilot_mom(2, numFunctions); // for recovery of varH
  if (pilotMgmtMode == OFFLINE_PILOT || // redirected here for ESTIMATOR_PERF
      pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
    SizetArray N_alloc_pilot;  Sizet2DArray N_actual_pilot(num_hf_lev);
    N_alloc_pilot.assign(num_hf_lev, 0);
    for (lev=0; lev<num_hf_lev; ++lev)
      N_actual_pilot[lev].assign(numFunctions, 0);
    evaluate_pilot(eval_ratios, Lambda, var_YH, N_alloc_pilot, N_actual_pilot,
		   hf_targets, pilot_mom, false, false);
  }
  else // ONLINE_PILOT_PROJECTION
    evaluate_pilot(eval_ratios, Lambda, var_YH, N_alloc_hf, N_actual_hf,
		   hf_targets, pilot_mom, true, true);

  // ML-only moments (no CV without LF increments) are rolled up for using varH
  // in variance reduction metrics (momentStats not reported for projections)
  convert_moments(pilot_mom, momentStats); // raw to final (central or std)
  recover_variance(momentStats, varH);

  for (lev=0; lev<num_cv_lev; ++lev)
    { N_actual_lf[lev] = N_actual_hf[lev]; N_alloc_lf[lev] = N_alloc_hf[lev]; }
  SizetArray delta_N_hf;  delta_N_hf.assign(num_hf_lev, 0);
  update_projected_samples(hf_targets, eval_ratios, N_actual_hf, N_alloc_hf,
			   N_actual_lf, N_alloc_lf, delta_N_hf, deltaEquivHF);
  Sizet2DArray N_proj_hf = N_actual_hf;
  increment_samples(N_proj_hf, delta_N_hf);
  compute_mlmf_estimator_variance(var_YH, N_proj_hf, Lambda, estVar);
  avgEstVar = average(estVar);
}


void NonDMultilevControlVarSampling::
evaluate_pilot(RealVectorArray& eval_ratios, RealMatrix& Lambda,
	       RealMatrix& var_YH, SizetArray& N_alloc, Sizet2DArray& N_actual,
	       RealVector& hf_targets, RealMatrix& pilot_mom,
	       bool accumulate_cost, bool pilot_estvar)
{
  size_t qoi, lev, num_mf = NLevActual.size(),
    num_hf_lev = iteratedModel->truth_model()->solution_levels(),
    num_cv_lev = (num_mf > 1) ?
    std::min(num_hf_lev, iteratedModel->surrogate_model()->solution_levels()) : 0;

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
    sum_Llm1_Llm1(numFunctions, num_cv_lev),
    //sum_Hl(numFunctions, num_hf_lev), sum_Hlm1(numFunctions, num_cv_lev),
    sum_Hl_Ll(numFunctions, num_cv_lev),  sum_Hl_Llm1(numFunctions, num_cv_lev),
    sum_Hlm1_Ll(numFunctions, num_cv_lev),
    sum_Hlm1_Llm1(numFunctions, num_cv_lev),
    sum_Hl_Hl(numFunctions, num_hf_lev), sum_Hl_Hlm1(numFunctions, num_hf_lev),
    sum_Hlm1_Hlm1(numFunctions, num_hf_lev),
    rho_dot2_LH(numFunctions, num_cv_lev, false);
  IntRealMatrixMap sum_Hl;   initialize_ml_Ysums(sum_Hl,   num_hf_lev, 2);
  IntRealMatrixMap sum_Hlm1; initialize_ml_Ysums(sum_Hlm1, num_hf_lev, 2);
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

  // FIRST PASS: launch and synchonize sample batches
  mlmf_increments(delta_N_hf, "mlcv_");
  if (costSource != USER_COST_SPEC)
    recover_online_cost(batchResponsesMap); // define sequenceCost for LF,HF
  hf_ref_cost = sequenceCost[numApprox];
  if (budget_constrained) budget = (Real)maxFunctionEvals * hf_ref_cost;
  // Note: could assign these back if needed elsewhere:
  //if (online_hf_cost) truth_model.solution_level_costs(hf_cost);
  //if (online_lf_cost)  surr_model.solution_level_costs(lf_cost);

  // SECOND PASS: accumulations and cost estimates
  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

    numSamples = N_alloc[lev] = delta_N_hf[lev];
    IntResponseMap& mlmf_resp_map = batchResponsesMap[lev];

    // control variate betwen LF and HF for this discretization level
    if (lev < num_cv_lev) // process {LF,HF} responses for MLMF sums
      accumulate_mlmf_Qsums(mlmf_resp_map, sum_Ll, sum_Llm1, sum_Ll_refined,
			    sum_Llm1_refined, sum_Hl, sum_Hlm1, sum_Ll_Ll,
			    sum_Ll_Llm1, sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1,
			    sum_Hlm1_Ll, sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
			    sum_Hlm1_Hlm1, lev, N_actual[lev]);
    else // no LF for this level; accumulate only multilevel discrepancies
      accumulate_ml_Ysums(mlmf_resp_map, sum_Hl, sum_Hl_Hl, lev, num_cv_lev,
			  N_actual[lev]); // offset for HF lev
  }
  clear_batches();

  // FINAL PASS: STATS
  sum_sqrt_var_cost = 0.;
  for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

    hf_lev_cost = level_cost(sequenceCost, lev, num_cv_lev);
    Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
    numSamples = delta_N_hf[lev];

    if (lev < num_cv_lev) {
      lf_lev_cost = level_cost(sequenceCost, lev);
      if (accumulate_cost)
	increment_mlmf_equivalent_cost(numSamples, hf_lev_cost, numSamples,
				       lf_lev_cost, hf_ref_cost, equivHFEvals);

      // compute the average evaluation ratio and Lambda factor
      RealVector& eval_ratios_l = eval_ratios[lev];
      compute_eval_ratios(sum_Ll, sum_Llm1, sum_Hl[1], sum_Hlm1[1], sum_Ll_Ll,
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
      variance_Ysum(sum_Hl[1][lev], sum_Hl_Hl[lev], N_actual[lev], var_YH[lev]);
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
  // roll up moment contributions from pilot sample (used in variance reduction
  // metrics for projections)
  if (!pilot_mom.empty()) {
    // CV contribution is zero for homogeneous pilot, so bypass cv_raw_moments()
    RealMatrix& sum_Hl_1 = sum_Hl[1];  RealMatrix& sum_Hl_2 = sum_Hl[2];
    // For levels 0:num_cv_lev, sum_Hl[p] is Ql^p
    ml_Q_raw_moments(sum_Hl_1, sum_Hlm1[1], sum_Hl_2, sum_Hlm1[2], N_actual,
		     0, num_cv_lev, pilot_mom);
    // For levels num_cv_lev:num_hf_lev, sum_Hl[p] is level discrepancy in Q^p,
    // i.e. Ql^p - Qlm1^p, and can be summed directly into raw moments:
    ml_Y_raw_moments(sum_Hl_1, sum_Hl_2, N_actual, num_cv_lev, num_hf_lev,
		     pilot_mom);
  }
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
    hf_lev_cost = level_cost(sequenceCost, lev, num_cv_lev);
    hf_targets[lev] = (lev < num_cv_lev) ? fact *
      std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho_dot2_LH[lev])) :
      fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
    //delta_N_hf[lev]
    //  = one_sided_delta(N_alloc_hf[lev], hf_targets[lev], relaxFactor);
  }

  ++mlmfIter;
}


/** The version in NonDNonHierarchSampling would be sufficiently general here
    as well, given AGGREGATED_MODELS controlled by the ASV. However, MLMC and
    MLCV MC employ AGGREGATED_MODEL_PAIR without ASV subsetting, so we
    specialize for that case. */
void NonDMultilevControlVarSampling::
mlmf_increments(SizetArray& delta_N_l, String prepend)
{
  if (mlmfIter == 0) Cout << "\nPerforming pilot sample for model groups.\n";
  else Cout << "\nSampling iteration " << mlmfIter << ": sample increment =\n"
	    << delta_N_l << '\n';

  // surr,truth Keys are fixed and ASV is 2 * num_cv_lev * numFunctions
  size_t lev, num_mf = NLevActual.size(), num_hf_lev = NLevActual.back().size(),
    num_cv_lev = (num_mf > 1) ?
      std::min(num_hf_lev, NLevActual.front().size()) : 0;
  size_t hf_offset = num_cv_lev * numFunctions;
  if (num_hf_lev != delta_N_l.size()) {
    Cerr << "Error: inconsistent increment length in NonDMultilevControlVar"
	 << "Sampling::mlmf_increments()" << std::endl;
    abort_handler(METHOD_ERROR);
  }
  for (lev=0; lev<num_hf_lev; ++lev) {
    numSamples = delta_N_l[lev];
    if (numSamples) {
      assign_specification_sequence(lev); // indexed so can skip if no samples
      // AGGREGATED_MODELS using (hf_form, lf_form) each for (lev, lev-1)
      ml_active_set(lev, hf_offset, true);          // reset request vector
      if (lev < num_cv_lev) ml_active_set(lev, 0, false); // don't reset RV
      ensemble_sample_batch(prepend, lev, true); // new samples
    }
  }

  if (iteratedModel->asynch_flag())
    synchronize_batches(*iteratedModel); // schedule all groups (return ignored)

  /*
  UShortArray batch_key(2); // form,resolution
  bool asynch = iteratedModel.asynch_flag();
  for (lev=0; lev<num_lev; ++lev) {
    numSamples = delta_N_l[lev];
    if (numSamples) {
      assign_specification_sequence(lev); // indexed so can skip if no samples
      batch_key[1] = lev;

      // AGGREGATED_MODEL_PAIR using hf_form with lev, lev-1
      if (!asynch) configure_indices(lev, hf_form, lev, sequenceType);
      batch_key[0] = hf_form;
      ensemble_sample_batch(prepend, batch_key, true); // new samples

      if (lev < num_cv_lev) {
	// AGGREGATED_MODEL_PAIR using lf_form with lev, lev-1
	if (!asynch) configure_indices(lev, lf_form, lev, sequenceType);
	batch_key[0] = lf_form;
	ensemble_sample_batch(prepend, batch_key, false); // reuse prev samples
      }
    }
  }
  */
}


/** The version in NonDNonHierarchSampling would be sufficiently general here
    as well, given AGGREGATED_MODELS controlled by the ASV. However, MLMC and
    MLCV MC employ AGGREGATED_MODEL_PAIR without ASV subsetting, so we
    specialize for that case. */
void NonDMultilevControlVarSampling::
lf_increments(SizetArray& delta_N_lf, String prepend)
{
  if (mlmfIter == 0) Cout << "\nPerforming pilot sample for model groups.\n";
  else Cout << "\nSampling iteration " << mlmfIter << ": sample increment =\n"
	    << delta_N_lf << '\n';

  size_t lev, num_cv_lev = delta_N_lf.size(), lf_form = 0;
  for (lev=0; lev<num_cv_lev; ++lev) {
    numSamples = delta_N_lf[lev];
    if (numSamples) {
      // No assign_specification_sequence() for LF increments
      //configure_indices(lev, lf_form, lev, sequenceType);
      // AGGREGATED_MODEL_PAIR using lf_form with lev, lev-1
      ml_active_set(lev, 0, true);
      ensemble_sample_batch(prepend, lev, true); // new samples
    }
  }

  if (iteratedModel->asynch_flag())
    synchronize_batches(*iteratedModel); // schedule all groups (return ignored)
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


size_t NonDMultilevControlVarSampling::
lf_increment(const RealVector& eval_ratios, const SizetArray& N_lf,
	     Real hf_target, RealVector& lf_targets)
{
  // update LF samples based on evaluation ratio
  //   r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  //   or with inverse r  -> delta = m-n = n/inverse_r - n
  if (lf_targets.empty()) lf_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_target;
  // Choose average, RMS, max of difference?
  // Trade-off: Possible overshoot vs. more iteration...
  size_t num_samp = one_sided_delta(N_lf, lf_targets);// averaged, no relaxation

  if (num_samp)
    Cout << "\nControl variate LF sample increment = " << num_samp;
  else Cout << "\nNo control variate LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from avg LF = " << average(N_lf) << ", HF target = " << hf_target
	 << ", relaxation = " << relaxFactor << ", avg eval_ratio = "
	 << average(eval_ratios);
  Cout << std::endl;

  return num_samp;
}


size_t NonDMultilevControlVarSampling::
lf_increment(const RealVector& eval_ratios, size_t N_lf, Real hf_target,
	     RealVector& lf_targets)
{
  if (lf_targets.empty()) lf_targets.sizeUninitialized(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    lf_targets[qoi] = eval_ratios[qoi] * hf_target;
  size_t num_samp = one_sided_delta((Real)N_lf, average(lf_targets));// no relax

  if (num_samp)
    Cout << "\nControl variate LF sample increment = " << num_samp;
  else Cout << "\nNo control variate LF sample increment";
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << " from LF = " << N_lf << ", HF target = " << hf_target
	 << ", avg eval_ratio = "<< average(eval_ratios);
  Cout << std::endl;

  return num_samp;
}


void NonDMultilevControlVarSampling::
cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_H,
	       IntRealMatrixMap& sum_LL,       IntRealMatrixMap& sum_LH,
	       const SizetArray& N_shared,     IntRealMatrixMap& sum_L_refined,
	       const SizetArray& N_refined,  //const RealMatrix& rho2_LH,
	       size_t lev,                     RealMatrix& H_raw_mom)
{
  size_t num_mom = sum_H.size();
  if (H_raw_mom.numRows() != num_mom)
    H_raw_mom.shapeUninitialized(num_mom, numFunctions);
  RealVector beta(numFunctions, false);

  // rho2_LH not stored for i > 1
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  Cout << "rho_LH (Pearson correlation) for QoI " << qoi+1 << " = "
  //	   << std::setw(9) << std::sqrt(rho2_LH(qoi,lev)) << '\n';
  //<< ", effectiveness ratio = " << std::setw(9) << rho2_LH(qoi,lev)*cr1;

  for (int m=1; m<=num_mom; ++m) {
    compute_mf_control(sum_L_shared[m], sum_H[m], sum_LL[m], sum_LH[m],
		       N_shared, lev, beta);
    Cout << "Moment " << m << ":\n";
    apply_mf_control(sum_H[m], sum_L_shared[m], N_shared, sum_L_refined[m],
		     N_refined, lev, beta, H_raw_mom, m-1);
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
    size_t num_mom = sum_Hl.size();
    if (H_raw_mom.numRows() != num_mom)
      H_raw_mom.shapeUninitialized(num_mom, numFunctions);
    RealVector beta_dot(numFunctions, false), gamma(numFunctions, false);

    // rho_dot2_LH not stored for i > 1
    //for (size_t qoi=0; qoi<numFunctions; ++qoi)
    //  Cout << "rho_dot_LH for QoI " << qoi+1 << " = " << std::setw(9)
    //       << std::sqrt(rho_dot2_LH(qoi,lev)) << '\n';
    //<< ", effectiveness ratio = " << rho_dot2_LH(qoi,lev) * cr1;

    // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k
    for (int m=1; m<=num_mom; ++m) {
      compute_mlmf_control(sum_Ll[m], sum_Llm1[m], sum_Hl[m], sum_Hlm1[m],
			   sum_Ll_Ll[m], sum_Ll_Llm1[m], sum_Llm1_Llm1[m],
			   sum_Hl_Ll[m], sum_Hl_Llm1[m], sum_Hlm1_Ll[m],
			   sum_Hlm1_Llm1[m], sum_Hl_Hl[m], sum_Hl_Hlm1[m],
			   sum_Hlm1_Hlm1[m], N_shared, lev, beta_dot, gamma);
      Cout << "Moment " << m << ":\n";
      apply_mlmf_control(sum_Hl[m], sum_Hlm1[m], sum_Ll[m], sum_Llm1[m],
			 N_shared, sum_Ll_refined[m], sum_Llm1_refined[m],
			 N_refined, lev, beta_dot, gamma, H_raw_mom, m-1);
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
			 SizetArray& N_alloc_hf,
			 const Sizet2DArray& N_actual_lf,
			 SizetArray& N_alloc_lf, SizetArray& delta_N_actual_hf,
			 //SizetArray& delta_N_actual_lf,
			 Real& delta_equiv_hf)
{
  size_t hf_actual_incr, hf_alloc_incr, lf_actual_incr, lf_alloc_incr, lev,
    num_hf_lev = iteratedModel->truth_model()->solution_levels(),
    num_cv_lev = std::min(num_hf_lev,
			  iteratedModel->surrogate_model()->solution_levels());
  Real hf_target_l, hf_ref_cost = sequenceCost[numApprox];
  RealVector lf_targets(numFunctions, false);
  for (lev=0; lev<num_hf_lev; ++lev) {
    hf_target_l      = hf_targets[lev];
    hf_alloc_incr    = one_sided_delta(N_alloc_hf[lev], hf_target_l);// no relax
    // Note: not duplicate as evaluate_pilot() does not compute delta_N_hf
    hf_actual_incr   = (backfillFailures) ?
      one_sided_delta(N_actual_hf[lev], hf_target_l) : hf_alloc_incr;// no relax
    if (pilotMgmtMode == OFFLINE_PILOT ||
	pilotMgmtMode == OFFLINE_PILOT_PROJECTION) {
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
      // no relaxation for projections
      lf_alloc_incr    = one_sided_delta(N_alloc_lf[lev], average(lf_targets));
      N_alloc_lf[lev] += lf_alloc_incr;
      lf_actual_incr   = (backfillFailures) ?
	one_sided_delta(N_actual_lf[lev], lf_targets) : lf_alloc_incr;
      //increment_samples(N_actual_lf[lev], lf_actual_incr);
      //delta_N_actual_lf[lev] += lf_actual_incr;
      increment_mlmf_equivalent_cost(hf_actual_incr,
	level_cost(sequenceCost, lev, num_cv_lev), lf_actual_incr,
	level_cost(sequenceCost, lev), hf_ref_cost, delta_equiv_hf);
    }
    else
      increment_ml_equivalent_cost(hf_actual_incr,
	level_cost(sequenceCost, lev, num_cv_lev), hf_ref_cost, delta_equiv_hf);
  }
}


void NonDMultilevControlVarSampling::
update_projected_lf_samples(const RealVector& hf_targets,
			    const RealVectorArray& eval_ratios,
			    const Sizet2DArray& N_actual_lf,
			    SizetArray& N_alloc_lf,
			    //SizetArray& delta_N_actual_lf,
			    Real& delta_equiv_hf)
{
  size_t lf_actual_incr, lf_alloc_incr, lev, num_cv_lev
    = std::min(iteratedModel->truth_model()->solution_levels(),
	       iteratedModel->surrogate_model()->solution_levels());
  Real hf_target_l, hf_ref_cost = sequenceCost[numApprox];
  RealVector lf_targets(numFunctions, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    hf_target_l = hf_targets[lev];
    const RealVector& eval_ratios_l = eval_ratios[lev];
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      lf_targets[qoi] = eval_ratios_l[qoi] * hf_target_l;
    // no relaxation for projections
    lf_alloc_incr    = one_sided_delta(N_alloc_lf[lev], average(lf_targets));
    N_alloc_lf[lev] += lf_alloc_incr;
    lf_actual_incr   = (backfillFailures) ?
      one_sided_delta(N_actual_lf[lev], lf_targets) : lf_alloc_incr;
    //delta_N_actual_lf[lev] += lf_actual_incr;
    increment_ml_equivalent_cost(lf_actual_incr, level_cost(sequenceCost, lev),
				 hf_ref_cost, delta_equiv_hf);
  }
}


void NonDMultilevControlVarSampling::
initialize_mlmf_sums(IntRealMatrixMap& sum_L_shared,
		     IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		     IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		     IntRealMatrixMap& sum_HH,
		     size_t num_ml_lev, size_t num_cv_lev, size_t num_mom)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=num_mom; ++i) {
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
		     size_t num_cv_lev, size_t num_mom)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=num_mom; ++i) {
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


/** Used in CV lf_increment() for leading (LF) function values in resp_map */
void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(const IntResponseMap& resp_map, IntRealMatrixMap& sum_Ql,
		      IntRealMatrixMap& sum_Qlm1, size_t lev, SizetArray& num_Q)
{
  if (lev == 0)
    accumulate_ml_Qsums(resp_map, sum_Ql, lev, num_Q);
  else {
    using std::isfinite;
    Real q_l, q_l_prod, q_lm1_prod, q_lm1;
    int l1_ord, l2_ord, active_ord;
    size_t qoi, offset_lm1 = (lev-1)*numFunctions, offset_l = lev*numFunctions;
    IntRespMCIter r_it; IntRMMIter l1_it, l2_it;

    for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	q_lm1_prod = q_lm1 = fn_vals[qoi+offset_lm1];
	q_l_prod   = q_l   = fn_vals[qoi+offset_l];

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
accumulate_mlmf_Qsums(const IntResponseMap& resp_map,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_HH, size_t lev,
		      SizetArray& num_L, SizetArray& num_H)
{
  using std::isfinite;
  Real lf_l, hf_l, lf_l_prod, hf_l_prod;
  IntRespMCIter r_it;  IntRMMIter ls_it, lr_it, h_it, ll_it, lh_it, hh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, hh_ord, active_ord;
  size_t qoi, offset_lf_lm1 = (lev-1) * numFunctions,
    offset_lf_l   = lev * numFunctions,
    offset_hf     = sum_L_shared.begin()->second.numCols() * numFunctions,
    offset_hf_lm1 = offset_hf + offset_lf_lm1,
    offset_hf_l   = offset_hf + offset_lf_l;

  for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_l_prod = lf_l = fn_vals[qoi+offset_lf_l];
      hf_l_prod = hf_l = fn_vals[qoi+offset_hf_l];

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
accumulate_mlmf_Qsums(const IntResponseMap& resp_map,
		      RealMatrix& sum_L_shared, RealMatrix& sum_L_refined,
		      IntRealMatrixMap& sum_H, RealMatrix& sum_LL,
		      RealMatrix& sum_LH, RealMatrix& sum_HH, size_t lev,
		      SizetArray& N_shared)
{
  using std::isfinite;
  Real lf_l, hf_l, hf_prod;  int h_ord, active_ord;
  size_t qoi, offset_lf_l = lev * numFunctions,
      offset_hf_l = sum_L_shared.numCols() * numFunctions + offset_lf_l;
  IntRespMCIter r_it;  IntRMMIter h_it;

  for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_l = fn_vals[qoi+offset_lf_l];
      hf_l = fn_vals[qoi+offset_hf_l];

      // sync sample counts for all L and H interactions at this level
      if (isfinite(lf_l) && isfinite(hf_l)) { // neither NaN nor +/-Inf

	++N_shared[qoi];

	sum_L_shared(qoi,lev)  += lf_l;        // Low shared
	sum_L_refined(qoi,lev) += lf_l;        // Low refined
	sum_LL(qoi,lev)        += lf_l * lf_l; // Low-Low
	sum_LH(qoi,lev)        += lf_l * hf_l; // Low-High
	sum_HH(qoi,lev)        += hf_l * hf_l; // High-High

	h_it = sum_H.begin();  h_ord = h_it->first;
	active_ord = 1;        hf_prod = hf_l;
	while (h_it!=sum_H.end()) {
	  if (h_ord == active_ord) {
	    h_it->second(qoi,lev) += hf_prod;  ++h_it; // High
	    h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  hf_prod *= hf_l;  ++active_ord;
	}
      }
    }
  }
}


void NonDMultilevControlVarSampling::
accumulate_mlmf_Qsums(const IntResponseMap& resp_map,
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
    accumulate_mlmf_Qsums(resp_map, sum_Ll, sum_Ll_refined, sum_Hl,
			  sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl, lev, num_L, num_H);
  else {
    using std::isfinite;
    Real lf_l_prod, lf_l, lf_lm1_prod, lf_lm1,
      hf_l_prod, hf_l, hf_lm1_prod, hf_lm1;
    IntRespMCIter r_it;
    IntRMMIter l1_it, l2_it, lr1_it, lr2_it, h1_it, h2_it, ll1_it, ll2_it,
      ll3_it, lh1_it, lh2_it, lh3_it, lh4_it, hh1_it, hh2_it, hh3_it;
    int active_ord, l1_ord, l2_ord, lr1_ord, lr2_ord, h1_ord, h2_ord,
      ll1_ord, ll2_ord, ll3_ord, lh1_ord, lh2_ord, lh3_ord, lh4_ord,
      hh1_ord, hh2_ord, hh3_ord;
    size_t qoi, offset_lf_lm1 = (lev-1) * numFunctions,
      offset_lf_l   = lev * numFunctions,
      offset_hf     = sum_Ll.begin()->second.numCols() * numFunctions,
      offset_hf_lm1 = offset_hf + offset_lf_lm1,
      offset_hf_l   = offset_hf + offset_lf_l;

    for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	lf_lm1_prod = lf_lm1 = fn_vals[qoi+offset_lf_lm1];
	lf_l_prod   = lf_l   = fn_vals[qoi+offset_lf_l];
	hf_lm1_prod = hf_lm1 = fn_vals[qoi+offset_hf_lm1];
	hf_l_prod   = hf_l   = fn_vals[qoi+offset_hf_l];

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
accumulate_mlmf_Qsums(const IntResponseMap& resp_map, RealMatrix& sum_Ll,
		      RealMatrix& sum_Llm1,         RealMatrix& sum_Ll_refined,
		      RealMatrix& sum_Llm1_refined, IntRealMatrixMap& sum_Hl,
		      IntRealMatrixMap& sum_Hlm1,   RealMatrix& sum_Ll_Ll,
		      RealMatrix& sum_Ll_Llm1,      RealMatrix& sum_Llm1_Llm1,
		      RealMatrix& sum_Hl_Ll,        RealMatrix& sum_Hl_Llm1,
		      RealMatrix& sum_Hlm1_Ll,      RealMatrix& sum_Hlm1_Llm1,
		      RealMatrix& sum_Hl_Hl,        RealMatrix& sum_Hl_Hlm1,
		      RealMatrix& sum_Hlm1_Hlm1,    size_t lev,
		      SizetArray& N_shared)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODEL_PAIR (lev > 0) modes.

  if (lev == 0) // level lm1 not available; accumulate only level l
    accumulate_mlmf_Qsums(resp_map, sum_Ll, sum_Ll_refined, sum_Hl, sum_Ll_Ll,
			  sum_Hl_Ll, sum_Hl_Hl, lev, N_shared);
  else {
    using std::isfinite;
    Real lf_l, lf_lm1, hf_l, hf_lm1, hf_l_prod, hf_lm1_prod;
    IntRespMCIter r_it;  IntRMMIter h1_it, h2_it;
    int h1_ord, h2_ord, active_ord;
    size_t qoi, offset_lf_lm1 = (lev-1) * numFunctions,
      offset_lf_l   = lev * numFunctions,
      offset_hf     = sum_Ll.numCols() * numFunctions,
      offset_hf_lm1 = offset_hf + offset_lf_lm1,
      offset_hf_l   = offset_hf + offset_lf_l;

    for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	lf_lm1 = fn_vals[qoi+offset_lf_lm1];
	lf_l   = fn_vals[qoi+offset_lf_l];
	hf_lm1 = fn_vals[qoi+offset_hf_lm1];
	hf_l   = fn_vals[qoi+offset_hf_l];

	// sync sample counts for all L and H interactions at this level
	if (isfinite(lf_l) && isfinite(lf_lm1) &&
	    isfinite(hf_l) && isfinite(hf_lm1)) { // neither NaN nor +/-Inf

	  ++N_shared[qoi];

	  // Low: Ll, Llm1, Ll_refined, Llm1_refined
	  sum_Ll(qoi,lev)           += lf_l;
	  sum_Llm1(qoi,lev)         += lf_lm1;
	  sum_Ll_refined(qoi,lev)   += lf_l;
	  sum_Llm1_refined(qoi,lev) += lf_lm1;
	  // Low-Low: Ll_Ll, Ll_Llm1, Llm1_Llm1
	  sum_Ll_Ll(qoi,lev)     += lf_l * lf_l;
	  sum_Ll_Llm1(qoi,lev)   += lf_l * lf_lm1;
	  sum_Llm1_Llm1(qoi,lev) += lf_lm1 * lf_lm1;
	  // Low-High: Hl_Ll, Hl_Llm1, Hlm1_Ll, Hlm1_Llm1
	  sum_Hl_Ll(qoi,lev)     += hf_l * lf_l;
	  sum_Hl_Llm1(qoi,lev)   += hf_l * lf_lm1;
	  sum_Hlm1_Ll(qoi,lev)   += hf_lm1 * lf_l;
	  sum_Hlm1_Llm1(qoi,lev) += hf_lm1 * lf_lm1;
	  // High-High: Hl_Hl, Hl_Hlm1, Hlm1_Hlm1
	  sum_Hl_Hl(qoi,lev)     += hf_l * hf_l;
	  sum_Hl_Hlm1(qoi,lev)   += hf_l * hf_lm1;
	  sum_Hlm1_Hlm1(qoi,lev) += hf_lm1 * hf_lm1;

	  // High: Hl, Hlm1
	  h1_it  = sum_Hl.begin();
	  h2_it  = sum_Hlm1.begin();
	  h1_ord = (h1_it == sum_Hl.end())   ? 0 : h1_it->first;
	  h2_ord = (h2_it == sum_Hlm1.end()) ? 0 : h2_it->first;
	  active_ord = 1;  hf_l_prod = hf_l;  hf_lm1_prod = hf_lm1;
	  while (h1_it != sum_Hl.end() || h2_it != sum_Hlm1.end()) {
	    if (h1_ord == active_ord) {
	      h1_it->second(qoi,lev) += hf_l_prod;  ++h1_it;
	      h1_ord = (h1_it == sum_Hl.end()) ? 0 : h1_it->first;
	    }
	    if (h2_ord == active_ord) {
	      h2_it->second(qoi,lev) += hf_lm1_prod;  ++h2_it;
	      h2_ord = (h2_it == sum_Hlm1.end()) ? 0 : h2_it->first;
	    }
	    hf_l_prod *= hf_l;  hf_lm1_prod *= hf_lm1;  ++active_ord;
	  }
	}
      }
    }
  }
}


/*
void NonDMultilevControlVarSampling::
accumulate_mlmf_Ysums(const IntResponseMap& resp_map, IntRealMatrixMap& sum_Y,
		      size_t lev, SizetArray& num_Y)
{
  // uses one set of allResponses in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODEL_PAIR (lev > 0) modes.  IntRealMatrixMap is a multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0)
    accumulate_ml_Qsums(resp_map, sum_Y, lev, num_Y);
  else { // AGGREGATED_MODEL_PAIR -> 2 sets of qoi per response map
    using std::isfinite;
    Real fn_l, prod_l, fn_lm1, prod_lm1;
    int ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter y_it;

    for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	prod_lm1 = fn_lm1 = fn_vals[qoi+offset_lm1];
	prod_l   = fn_l   = fn_vals[qoi+offset_l];

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
accumulate_mlmf_Ysums(const IntResponseMap& resp_map,
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
    accumulate_mlmf_Qsums(resp_map, sum_L_shared, sum_L_refined, sum_H,
			  sum_LL, sum_LH, sum_HH, lev, num_L, num_H);
  else { // AGGREGATED_MODEL_PAIR -> 2 sets of qoi per response map
    using std::isfinite;
    Real lf_l, lf_l_prod, lf_lm1, lf_lm1_prod,
         hf_l, hf_l_prod, hf_lm1, hf_lm1_prod;
    IntRespMCIter r_it;
    IntRMMIter ls_it, lr_it, h_it, ll_it, lh_it, hh_it;
    int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, hh_ord, active_ord;
    size_t qoi;

    for (r_it=resp_map.begin(); r_it!=resp_map.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODEL_PAIR orders low to high fidelity
	lf_lm1_prod = lf_lm1 = fn_vals[qoi+offset_lf_lm1];
	lf_l_prod   = lf_l   = fn_vals[qoi+offset_lf_l];
	hf_lm1_prod = hf_lm1 = fn_vals[qoi+offset_hf_lm1];
	hf_l_prod   = hf_l   = fn_vals[qoi+offset_hf_l];

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
*/


void NonDMultilevControlVarSampling::print_variance_reduction(std::ostream& s)
{
  if (delegateMethod == MULTILEVEL_SAMPLING) // not currently overridden by MLMC
    { NonDMultilevelSampling::print_variance_reduction(s);  return; }
  //else if (delegateMethod == MULTIFIDELITY_SAMPLING) //  MFMC not inherited
  //  { NonDMultifidelitySampling::print_variance_reduction(s);  return; }

  bool projection = (pilotMgmtMode ==  ONLINE_PILOT_PROJECTION ||
		     pilotMgmtMode == OFFLINE_PILOT_PROJECTION);
  String type = (projection) ? "Projected" : "   Online";
  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:\n";
  switch (pilotMgmtMode) {
  case OFFLINE_PILOT:  case OFFLINE_PILOT_PROJECTION:
    s << "  " << type << " MLCVMC (sample profile):   "
      << std::setw(wpp7) << avgEstVar << '\n';
    break;
  default: {
    Real avg_mlmc_estvar0 = average(estVarIter0);
    s << "      Initial MLMC (pilot samples):    " << std::setw(wpp7)
      << avg_mlmc_estvar0 << "\n  "
      << type << " MLCVMC (sample profile):   "
      << std::setw(wpp7) << avgEstVar	<< "\n  "
      << type << " MLCVMC / pilot ratio:      "
      // report ratio of averages rather than average of ratios:
      << std::setw(wpp7) << avgEstVar / avg_mlmc_estvar0 << '\n';
    break;
  }
  }

  // MC estvar uses varH from recover_variance()
  Real     proj_equiv_hf = equivHFEvals + deltaEquivHF,
    avg_budget_mc_estvar = average(varH) / proj_equiv_hf;
  s << " Equivalent     MC (" << std::setw(5)
    << (size_t)std::floor(proj_equiv_hf + .5) << " HF samples): "
    << std::setw(wpp7) << avg_budget_mc_estvar
    << "\n Equivalent MLCVMC / MC ratio:         " << std::setw(wpp7)
    << avgEstVar / avg_budget_mc_estvar << '\n';
}

} // namespace Dakota
