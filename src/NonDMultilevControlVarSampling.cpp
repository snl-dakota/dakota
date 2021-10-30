/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevControlVarSampling
//- Description: Implementation code for NonDMultilevControlVarSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
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
  NonDControlVariateSampling(problem_db, model),
  NonDHierarchSampling(problem_db, model) // top of virtual inheritance
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
}


void NonDMultilevControlVarSampling::pre_run()
{
  NonDEnsembleSampling::pre_run();

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
void NonDMultilevControlVarSampling::core_run()
{
  // Can trap mis-specification of the MLMF method and delegate to
  // inherited core_run() implementations:
  ModelList& model_ensemble = iteratedModel.subordinate_models(false);
  if (model_ensemble.size() <= 1)
    { NonDMultilevelSampling::core_run();    return; } // delegate to MLMC
  ModelList::iterator last_m_it = --model_ensemble.end();
  size_t num_hf_lev = last_m_it->solution_levels();
  if (num_hf_lev <= 1)
    { NonDControlVariateSampling::core_run(); return; } // delegate to CVMC

  // multiple model forms (currently limited to 2) + multiple solutions levels

  // prefer ML over MF if both available
  //iteratedModel.multifidelity_precedence(false);

  // ML performed on HF + CV applied per level using LF if available:
  // perform MLMC on HF model and bind 1:min(num_hf,num_lf) LF control
  // variates starting at coarsest level
  if (true) // reformulated approach using 1 new QoI correlation per level
    multilevel_control_variate_mc_Qcorr();
  else      // original approach using 1 discrepancy correlation per level
    multilevel_control_variate_mc_Ycorr();
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC si employed
    across two model forms to exploit correlation in the discrepancies
    at each level (Y_l). */
void NonDMultilevControlVarSampling::multilevel_control_variate_mc_Ycorr()
{
  // For two-model control variate methods, select lowest,highest fidelities
  size_t num_mf = NLev.size();
  unsigned short group,
    lf_form = 0, hf_form = num_mf - 1; // ordered_models = low:high

  // assign model forms (solution level assignments are deferred until loop)
  Pecos::ActiveKey active_key;
  short seq_type = Pecos::RESOLUTION_LEVEL_SEQUENCE;
  size_t lev = SZ_MAX; // updated in loop below
  active_key.form_key(0, hf_form, lev, lf_form, lev, Pecos::RAW_DATA);
  iteratedModel.active_model_key(active_key);
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  size_t qoi, num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_costs(),
    lf_cost = surr_model.solution_level_costs(), agg_var_hf(num_hf_lev);
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., budget,
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
  RealMatrix var_H(numFunctions, num_cv_lev, false),
           rho2_LH(numFunctions, num_cv_lev, false);
  RealVector Lambda(num_cv_lev, false), avg_rho2_LH(num_cv_lev, false);
  
  // Initialize for pilot sample
  Sizet2DArray&       N_lf =      NLev[lf_form];
  Sizet2DArray&       N_hf =      NLev[hf_form];
  Sizet2DArray  delta_N_l;   load_pilot_sample(pilotSamples, NLev, delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_form]; 

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  //SizetArray raw_N_lf(num_cv_lev, 0), raw_N_hf(num_hf_lev, 0);
  RealVector mu_L_hat, mu_H_hat, lambda_l(numFunctions, false);

  // now converge on sample counts per level (N_hf)
  while (Pecos::l1_norm(delta_N_hf) && mlmfIter <= maxIterations) {

    sum_sqrt_var_cost = 0.;
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      configure_indices(group, hf_form, lev, seq_type);
      hf_lev_cost = level_cost(hf_cost, lev);

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// assign sequence, get samples, export, evaluate
	evaluate_ml_sample_increment(lev);

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
	  configure_indices(group, lf_form, lev, seq_type);
	  lf_lev_cost = level_cost(lf_cost, lev);
	  // compute allResp w/ LF model form reusing allVars from MLMC step
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // process previous and new set of allResponses for CV sums
	  accumulate_mlmf_Ysums(allResponses, hf_resp, sum_L_shared,
				sum_L_refined, sum_H, sum_LL, sum_LH,
				sum_HH, lev, mu_L_hat, mu_H_hat,
				N_lf[lev], N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_shared[1,2], L_refined[1,2], LH[1,2])"
		 << ":\n" << sum_L_shared[1] << sum_L_shared[2]
		 << sum_L_refined[1] << sum_L_refined[2]
		 << sum_LH[1] << sum_LH[2];
	  // update raw evaluation counts
	  //raw_N_hf[lev] += numSamples;  raw_N_lf[lev] += numSamples;
	  increment_mlmf_equivalent_cost(numSamples, hf_lev_cost,
					 numSamples, lf_lev_cost, hf_ref_cost);

	  // compute the average evaluation ratio and Lambda factor
	  RealVector& eval_ratios_l = eval_ratios[lev];
	  compute_eval_ratios(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			      sum_HH[1], hf_lev_cost/lf_lev_cost, lev,
			      N_hf[lev], var_H, rho2_LH, eval_ratios_l);

	  // defer averaging for Lambda
	  for (qoi=0; qoi<numFunctions; ++qoi)
	    lambda_l[qoi] = 1. - rho2_LH(qoi,lev)
	                  * (eval_ratios_l[qoi] - 1.) / eval_ratios_l[qoi];
	  Lambda[lev] = average(lambda_l);
	  avg_rho2_LH[lev] = average(rho2_LH[lev], numFunctions);
	  //Lambda[lev] = 1. - avg_rho2_LH[lev]
	  //            * (avg_eval_ratio - 1.) / avg_eval_ratio;

	  agg_var_hf_l = sum(var_H[lev], numFunctions);
	}
	else { // no LF model for this level; accumulate only multilevel sums
	  RealMatrix& sum_HH1 = sum_HH[1];
	  accumulate_ml_Ysums(sum_H, sum_HH1, lev, mu_H_hat,
			      N_hf[lev]); // sum_Y for lev>0
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (H[1], H[2], HH):\n"
		 << sum_H[1] << sum_H[2] << sum_HH1;
	  //raw_N_hf[lev] += numSamples;
	  increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost);
	  // aggregate Y variances across QoI for this level
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "variance of Y[" << lev << "]: ";
	  agg_var_hf_l
	    = aggregate_variance_Ysum(sum_H[1][lev], sum_HH1[lev], N_hf[lev]);
	}
      }

      // accumulate sum of sqrt's of estimator var * cost used in N_target
      if (lev < num_cv_lev) {
	Real om_rho2 = 1. - avg_rho2_LH[lev];
	sum_sqrt_var_cost += (budget_constrained) ?
	  std::sqrt(agg_var_hf_l / hf_lev_cost * om_rho2) *
	    (hf_lev_cost + average(eval_ratios[lev]) * lf_lev_cost) :
	  std::sqrt(agg_var_hf_l * hf_lev_cost / om_rho2) * Lambda[lev];
      }
      else
	sum_sqrt_var_cost += std::sqrt(agg_var_hf_l * hf_lev_cost);

      // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
      // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
      // will result in no additional variance reduction beyond MLMC.
      if (mlmfIter == 0 && !budget_constrained)
	estimator_var0 += (lev < num_cv_lev) ?
	  aggregate_mse_Yvar(var_H[lev], N_hf[lev]) :
	  aggregate_mse_Ysum(sum_H[1][lev], sum_HH[1][lev], N_hf[lev]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0 && !budget_constrained) {// eps^2 / 2 = est var * conv tol
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // All CV lf_increment() calls now follow all ML level evals:
    for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
      if (delta_N_hf[lev]) {
	configure_indices(group, lf_form, lev, seq_type);//augment LF grp

	// execute additional LF sample increment
	if (lf_increment(eval_ratios[lev], N_lf[lev], N_hf[lev],mlmfIter,lev)) {
	  accumulate_mlmf_Ysums(sum_L_refined, lev, mu_L_hat, N_lf[lev]);
	  //raw_N_lf[lev] += numSamples;
	  increment_ml_equivalent_cost(numSamples, level_cost(lf_cost, lev),
				       hf_ref_cost);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_refined[1,2]):\n"
		 << sum_L_refined[1] << sum_L_refined[2];
	}
      }
    }

    // update sample targets based on variance estimates
    // Note: sum_sqrt_var_cost is defined differently for the two cases
    Real N_target, fact = (budget_constrained) ?
      budget / sum_sqrt_var_cost :      // budget constraint
      sum_sqrt_var_cost / eps_sq_div_2; // error balance constraint
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = (lev) ? hf_cost[lev] + hf_cost[lev-1] : hf_cost[lev];
      N_target = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = one_sided_delta(average(N_hf[lev]), N_target);
    }
    ++mlmfIter;
    Cout << "\nMLMF MC iteration " << mlmfIter << " sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // Iteration complete.  Now roll up raw moments from CVMC and MLMC estimators.
  RealMatrix Y_mlmc_mom(numFunctions, 4), Y_cvmc_mom(numFunctions, 4, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf[lev],
		   sum_L_refined, N_lf[lev], rho2_LH, lev, Y_cvmc_mom);
    Y_mlmc_mom += Y_cvmc_mom;
  }
  if (num_hf_lev > num_cv_lev) {
    RealMatrix &sum_H1 = sum_H[1], &sum_H2 = sum_H[2],
               &sum_H3 = sum_H[3], &sum_H4 = sum_H[4];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      for (lev=num_cv_lev; lev<num_hf_lev; ++lev) {
	size_t Nlq = N_hf[lev][qoi];
	Y_mlmc_mom(qoi,0) += sum_H1(qoi,lev) / Nlq;
	Y_mlmc_mom(qoi,1) += sum_H2(qoi,lev) / Nlq;
	Y_mlmc_mom(qoi,2) += sum_H3(qoi,lev) / Nlq;
	Y_mlmc_mom(qoi,3) += sum_H4(qoi,lev) / Nlq;
      }
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Y_mlmc_mom, momentStats);
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC is employed
    across two model forms.  It generalizes the Y_l correlation case
    to separately target correlations for each QoI level embedded
    within the level discrepancies. */
void NonDMultilevControlVarSampling::multilevel_control_variate_mc_Qcorr()
{
  // For two-model control variate methods, select lowest,highest fidelities
  size_t num_mf = NLev.size();
  unsigned short group,
    lf_form = 0, hf_form = num_mf - 1; // ordered_models = low:high

  // assign model forms (solution level assignments are deferred until loop)
  Pecos::ActiveKey active_key;
  short seq_type = Pecos::RESOLUTION_LEVEL_SEQUENCE;
  size_t lev = SZ_MAX; // updated in loop below
  active_key.form_key(0, hf_form, lev, lf_form, lev, Pecos::RAW_DATA);
  iteratedModel.active_model_key(active_key);
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  size_t qoi, num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  bool budget_constrained = (maxFunctionEvals != SZ_MAX);

  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_costs(),
    lf_cost = surr_model.solution_level_costs(), agg_var_hf(num_hf_lev);
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., budget,
    lf_lev_cost, hf_lev_cost, hf_ref_cost = hf_cost[num_hf_lev-1];
  if (budget_constrained) budget = (Real)maxFunctionEvals * hf_ref_cost;
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
  RealMatrix var_Yl(numFunctions, num_cv_lev, false),
             rho_dot2_LH(numFunctions, num_cv_lev, false);
  RealVector Lambda(num_cv_lev, false), avg_rho_dot2_LH(num_cv_lev, false);

  // Initialize for pilot sample
  Sizet2DArray&       N_lf =      NLev[lf_form];
  Sizet2DArray&       N_hf =      NLev[hf_form]; 
  Sizet2DArray  delta_N_l;   load_pilot_sample(pilotSamples, NLev, delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_form]; 

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  //SizetArray raw_N_lf(num_cv_lev, 0), raw_N_hf(num_hf_lev, 0);
  RealVector mu_L_hat, mu_H_hat, lambda_l(numFunctions, false);

  // now converge on sample counts per level (N_hf)
  while (Pecos::l1_norm(delta_N_hf) && mlmfIter <= maxIterations) {

    sum_sqrt_var_cost = 0.;
    for (lev=0, group=0; lev<num_hf_lev; ++lev, ++group) {

      configure_indices(group, hf_form, lev, seq_type);
      hf_lev_cost = level_cost(hf_cost, lev);

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// assign sequence, get samples, export, evaluate
	evaluate_ml_sample_increment(lev);

	// control variate betwen LF and HF for this discretization level:
	// if unequal number of levels, loop over all HF levels for MLMC and
	// apply CVMC when LF levels are available.  LF levels are assigned as
	// control variates to the leading set of HF levels, since these will
	// tend to have larger variance.      
	if (lev < num_cv_lev) {

	  // store allResponses used for sum_H (and sum_HH)
	  IntResponseMap hf_resp = allResponses; // shallow copy is sufficient
	  // activate LF response (lev 0) or LF response discrepancy (lev > 0)
	  // within the hierarchical surrogate model.  Level indices & surrogate
	  // response mode are same as HF above, only the model form changes.
	  // However, we must pass the unchanged level index to update the
	  // corresponding variable values for the new model form.
	  configure_indices(group, lf_form, lev, seq_type);
	  lf_lev_cost = level_cost(lf_cost, lev);
	  // eval allResp w/ LF model reusing allVars from ML step above
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // process previous and new set of allResponses for MLMF sums;
	  accumulate_mlmf_Qsums(allResponses, hf_resp, sum_Ll, sum_Llm1,
				sum_Ll_refined, sum_Llm1_refined, sum_Hl,
				sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1,
				sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
				sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
				sum_Hlm1_Hlm1, lev, mu_L_hat, mu_H_hat,
				N_lf[lev], N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (Ll[1,2], L_refined[1,2], Hl[1,2]):\n"
		 << sum_Ll[1] << sum_Ll[2] << sum_Ll_refined[1]
		 << sum_Ll_refined[2] << sum_Hl[1] << sum_Hl[2];
	  // update raw evaluation counts
	  //raw_N_lf[lev] += numSamples; raw_N_hf[lev] += numSamples;
	  increment_mlmf_equivalent_cost(numSamples, hf_lev_cost,
					 numSamples, lf_lev_cost, hf_ref_cost);

	  // compute the average evaluation ratio and Lambda factor
	  RealVector& eval_ratios_l = eval_ratios[lev];
	  compute_eval_ratios(sum_Ll[1], sum_Llm1[1], sum_Hl[1], sum_Hlm1[1],
			      sum_Ll_Ll[1], sum_Ll_Llm1[1], sum_Llm1_Llm1[1],
			      sum_Hl_Ll[1], sum_Hl_Llm1[1], sum_Hlm1_Ll[1],
			      sum_Hlm1_Llm1[1], sum_Hl_Hl[1], sum_Hl_Hlm1[1],
			      sum_Hlm1_Hlm1[1], hf_lev_cost/lf_lev_cost, lev,
			      N_hf[lev], var_Yl, rho_dot2_LH, eval_ratios_l);

	  // defer averaging for Lambda
	  for (qoi=0; qoi<numFunctions; ++qoi)
	    lambda_l[qoi] = 1. - rho_dot2_LH(qoi,lev)
	                  * (eval_ratios_l[qoi] - 1.) / eval_ratios_l[qoi];
	  Lambda[lev] = average(lambda_l);
	  avg_rho_dot2_LH[lev] = average(rho_dot2_LH[lev], numFunctions);
	  //Lambda[lev] = 1. - avg_rho_dot2_LH[lev]
	  //            * (avg_eval_ratio - 1.) / avg_eval_ratio;

	  agg_var_hf_l = sum(var_Yl[lev], numFunctions);
	}
	else { // no LF model for this level; accumulate only multilevel
	       // discrepancy sums (Hl is Yl) as in standard MLMC
	  RealMatrix& sum_HH1 = sum_Hl_Hl[1];
	  accumulate_ml_Ysums(sum_Hl, sum_HH1, lev, mu_H_hat,
			      N_hf[lev]); // sum_Y for lev>0
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (H[1], H[2], HH[1]):\n"
		 << sum_Hl[1] << sum_Hl[2] << sum_HH1;
	  //raw_N_hf[lev] += numSamples;
	  increment_ml_equivalent_cost(numSamples, hf_lev_cost, hf_ref_cost);
	  // aggregate Y variances across QoI for this level
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "variance of Y[" << lev << "]: ";
	  agg_var_hf_l
	    = aggregate_variance_Ysum(sum_Hl[1][lev], sum_HH1[lev], N_hf[lev]);
	}
      }

      // accumulate sum of sqrt's of estimator var * cost used in N_target
      if (lev < num_cv_lev) {
	Real om_rho2 = 1. - avg_rho_dot2_LH[lev];
	sum_sqrt_var_cost += (budget_constrained) ?
	  std::sqrt(agg_var_hf_l / hf_lev_cost * om_rho2) *
	    (hf_lev_cost + average(eval_ratios[lev]) * lf_lev_cost) :
	  std::sqrt(agg_var_hf_l * hf_lev_cost / om_rho2) * Lambda[lev];
      }
      else
	sum_sqrt_var_cost += std::sqrt(agg_var_hf_l * hf_lev_cost);
	
      // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
      // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
      // will result in no additional variance reduction beyond MLMC.
      if (mlmfIter == 0 && !budget_constrained)
	estimator_var0 += (lev < num_cv_lev) ?
	  aggregate_mse_Yvar(var_Yl[lev], N_hf[lev]) :
	  aggregate_mse_Ysum(sum_Hl[1][lev], sum_Hl_Hl[1][lev], N_hf[lev]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0 && !budget_constrained) {// eps^2 / 2 = est var * conv tol
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	     Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // All CV lf_increment() calls now follow all ML level evals:
    // > Provides separation of pilot sample from refinements (simplifying
    //   offline execution with data importing w/o undesirable seed progression)
    // > Improves application of maxIterations control in general: user
    //   specification results in consistent count for ML and CV refinements
    // > Incurs a bit more overhead: eval_ratios array, mode resetting
    // > Could potentially have parallel scheduling benefits by grouping
    //   similar Model eval sets for aggregated scheduling
    for (lev=0, group=0; lev<num_cv_lev; ++lev, ++group) {
      if (delta_N_hf[lev]) {
	configure_indices(group, lf_form, lev, seq_type);//augment LF grp

	// now execute additional LF sample increment
	if (lf_increment(eval_ratios[lev], N_lf[lev], N_hf[lev],mlmfIter,lev)) {
	  accumulate_mlmf_Qsums(sum_Ll_refined, sum_Llm1_refined, lev, mu_L_hat,
				N_lf[lev]);
	  //raw_N_lf[lev] += numSamples;
	  increment_ml_equivalent_cost(numSamples, level_cost(lf_cost, lev),
				       hf_ref_cost);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_refined[1,2]):\n"
		 << sum_Ll_refined[1] << sum_Ll_refined[2];
	}
      }
    }

    // update sample targets based on variance estimates
    // Note: sum_sqrt_var_cost is defined differently for the two cases
    Real N_target, fact = (budget_constrained) ?
      budget / sum_sqrt_var_cost :      // budget constraint
      sum_sqrt_var_cost / eps_sq_div_2; // error balance constraint
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = (lev) ? hf_cost[lev] + hf_cost[lev-1] : hf_cost[lev];
      N_target = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho_dot2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = one_sided_delta(average(N_hf[lev]), N_target);
    }
    ++mlmfIter;
    Cout << "\nMLMF MC iteration " << mlmfIter << " sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // Iteration complete. Now roll up raw moments from CVMC and MLMC estimators.
  RealMatrix Y_mlmc_mom(numFunctions, 4), Y_cvmc_mom(numFunctions, 4, false);
  for (lev=0; lev<num_cv_lev; ++lev) {
    cv_raw_moments(sum_Ll, sum_Llm1, sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1,
		   sum_Llm1_Llm1, sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
		   sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1,
		   N_hf[lev], sum_Ll_refined, sum_Llm1_refined, N_lf[lev],
		   rho_dot2_LH, lev, Y_cvmc_mom);
    Y_mlmc_mom += Y_cvmc_mom;
  }
  if (num_hf_lev > num_cv_lev) {
    // MLMC without CV: sum_H = HF Q sums for lev = 0 and HF Y sums for lev > 0
    RealMatrix &sum_H1 = sum_Hl[1], &sum_H2 = sum_Hl[2],
               &sum_H3 = sum_Hl[3], &sum_H4 = sum_Hl[4];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      for (lev=num_cv_lev; lev<num_hf_lev; ++lev) {
	size_t Nlq = N_hf[lev][qoi];
	Y_mlmc_mom(qoi,0) += sum_H1(qoi,lev) / Nlq;
	Y_mlmc_mom(qoi,1) += sum_H2(qoi,lev) / Nlq;
	Y_mlmc_mom(qoi,2) += sum_H3(qoi,lev) / Nlq;
	Y_mlmc_mom(qoi,3) += sum_H4(qoi,lev) / Nlq;
      }
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Y_mlmc_mom, momentStats);
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

    if (rho_sq < 1.) { // protect against division by 0
      eval_ratios[qoi] = std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = " << cost_ratio
	     << " rho_sq = " << rho_sq << " eval_ratio = " << eval_ratios[qoi]
	     << std::endl;
      //avg_eval_ratio += eval_ratio;
      //++num_avg;
    }
    else // should not happen, but provide a reasonable upper bound
      eval_ratios[qoi] = (Real)maxFunctionEvals / average(N_shared);
  }
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "variance of HF Q[" << lev << "]:\n";
    write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_H);
  }

  //if (num_avg) avg_eval_ratio /= num_avg;
  //else         avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);
  //return avg_eval_ratio;
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
    //Real eval_ratio, avg_eval_ratio = 0.;  size_t num_avg = 0;
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

      if (rho_dot_sq < 1.) { // protect against division by 0
	eval_ratios[qoi] = std::sqrt(cost_ratio * rho_dot_sq / (1.-rho_dot_sq));
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = "
	       << cost_ratio << " rho_dot_sq = " << rho_dot_sq
	       << " eval_ratio = " << eval_ratios[qoi] << std::endl;
	//avg_eval_ratio += eval_ratio;
	//++num_avg;
      }
      else // should not happen, but provide a reasonable upper bound
	eval_ratios[qoi] = (Real)maxFunctionEvals / average(N_shared);
    }
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "variance of HF Y[" << lev << "]:\n";
      write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_YHl);
    }

    //if (num_avg) avg_eval_ratio /= num_avg;
    //else         avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);
    //return avg_eval_ratio;
  }
}


void NonDMultilevControlVarSampling::
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
    Cout << "compute_mlmf_control(): var reduce ratio = " << ratio
	 << " rho2_LH = " << rho2_LH << " rho_dot2_LH = " << rho_dot2_LH
	 << std::endl;
}


void NonDMultilevControlVarSampling::
apply_mlmf_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll, Real sum_Llm1,
		   size_t N_shared,  Real sum_Ll_refined,
		   Real sum_Llm1_refined, size_t N_refined, Real beta_dot,
		   Real gamma, Real& H_raw_mom)
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
    sum_Hlm1.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Ll_Ll.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Ll_Llm1.insert(empty_pr).first->second.shape(numFunctions, num_cv_lev);
    sum_Llm1_Llm1.insert(empty_pr).first->second.shape(numFunctions,num_cv_lev);
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
		      size_t lev, const RealVector& offset, SizetArray& num_Q)
{
  if (lev == 0)
    accumulate_ml_Qsums(sum_Ql, lev, offset, num_Q);
  else {
    using std::isfinite;
    Real q_l, q_l_prod, q_lm1_prod, q_lm1;
    int l1_ord, l2_ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter l1_it, l2_it;
    bool os = !offset.empty();

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODELS orders HF (active model key)
	// followed by LF (previous/decremented model key)
	q_l_prod   = q_l   = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
	q_lm1_prod = q_lm1 = (os) ?
	  fn_vals[qoi+numFunctions] - offset[qoi+numFunctions] :
	  fn_vals[qoi+numFunctions];

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
accumulate_mlmf_Ysums(IntRealMatrixMap& sum_Y, size_t lev,
		      const RealVector& offset, SizetArray& num_Y)
{
  // uses one set of allResponses in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap is a multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0)
    accumulate_ml_Qsums(sum_Y, lev, offset, num_Y);
  else { // AGGREGATED_MODELS -> 2 sets of qoi per response map
    using std::isfinite;
    Real fn_l, prod_l, fn_lm1, prod_lm1;
    int ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter y_it;
    bool os = !offset.empty();

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODELS orders HF (active model key)
	// followed by LF (previous/decremented model key)
	prod_l   = fn_l   = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
	prod_lm1 = fn_lm1 = (os) ?
	  fn_vals[qoi+numFunctions] - offset[qoi+numFunctions] :
	  fn_vals[qoi+numFunctions];

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
		      const RealVector& lf_offset, const RealVector& hf_offset,
		      SizetArray& num_L, SizetArray& num_H)
{
  using std::isfinite;
  Real lf_l, hf_l, lf_l_prod, hf_l_prod;
  IntRespMCIter lf_r_it, hf_r_it;
  IntRMMIter ls_it, lr_it, h_it, ll_it, lh_it, hh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, hh_ord, active_ord;
  size_t qoi;
  bool lfos = !lf_offset.empty(), hfos = !hf_offset.empty();

  for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
       lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
       ++lf_r_it, ++hf_r_it) {
    const RealVector& lf_fn_vals = lf_r_it->second.function_values();
    const RealVector& hf_fn_vals = hf_r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_l_prod = lf_l = (lfos) ?
	lf_fn_vals[qoi] - lf_offset[qoi] : lf_fn_vals[qoi];
      hf_l_prod = hf_l = (hfos) ?
	hf_fn_vals[qoi] - hf_offset[qoi] : hf_fn_vals[qoi];

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
accumulate_mlmf_Ysums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_HH, size_t lev,
		      const RealVector& lf_offset, const RealVector& hf_offset,
		      SizetArray& num_L, SizetArray& num_H)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap are for multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0) // BYPASS_SURROGATE -> 1 set of qoi per response map
    accumulate_mlmf_Qsums(lf_resp_map, hf_resp_map, sum_L_shared,
			  sum_L_refined, sum_H, sum_LL, sum_LH, sum_HH,
			  lev, lf_offset, hf_offset, num_L, num_H);
  else { // AGGREGATED_MODELS -> 2 sets of qoi per response map
    using std::isfinite;
    Real lf_l, lf_l_prod, lf_lm1, lf_lm1_prod,
         hf_l, hf_l_prod, hf_lm1, hf_lm1_prod;
    IntRespMCIter lf_r_it, hf_r_it;
    IntRMMIter ls_it, lr_it, h_it, ll_it, lh_it, hh_it;
    int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, hh_ord, active_ord;
    size_t qoi;
    bool lfos = !lf_offset.empty(), hfos = !hf_offset.empty();

    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODELS orders level l (active model key)
	// followed by level l-1 (previous/decremented model key)
	lf_l_prod   = lf_l   = (lfos) ?
	  lf_fn_vals[qoi] - lf_offset[qoi] : lf_fn_vals[qoi];
	lf_lm1_prod = lf_lm1 = (lfos) ?
	  lf_fn_vals[qoi+numFunctions] - lf_offset[qoi+numFunctions] :
	  lf_fn_vals[qoi+numFunctions];
	hf_l_prod   = hf_l   = (hfos) ?
	  hf_fn_vals[qoi] - hf_offset[qoi] : hf_fn_vals[qoi];
	hf_lm1_prod = hf_lm1 = (hfos) ?
	  hf_fn_vals[qoi+numFunctions] - hf_offset[qoi+numFunctions] :
	  hf_fn_vals[qoi+numFunctions];

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
		      const RealVector& lf_offset, const RealVector& hf_offset,
		      SizetArray& num_L, SizetArray& num_H)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap are for multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0) // level lm1 not available; accumulate only level l
    accumulate_mlmf_Qsums(lf_resp_map, hf_resp_map, sum_Ll, sum_Ll_refined,
			  sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl, lev,
			  lf_offset, hf_offset, num_L, num_H);
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
    bool lfos = !lf_offset.empty(), hfos = !hf_offset.empty();

    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	// response mode AGGREGATED_MODELS orders level l (active model key)
	// followed by level l-1 (previous/decremented model key)
	lf_l_prod   = lf_l   = (lfos) ?
	  lf_fn_vals[qoi] - lf_offset[qoi] : lf_fn_vals[qoi];
	lf_lm1_prod = lf_lm1 = (lfos) ?
	  lf_fn_vals[qoi+numFunctions] - lf_offset[qoi+numFunctions] :
	  lf_fn_vals[qoi+numFunctions];
	hf_l_prod   = hf_l   = (hfos) ?
	  hf_fn_vals[qoi] - hf_offset[qoi] : hf_fn_vals[qoi];
	hf_lm1_prod = hf_lm1 = (hfos) ?
	  hf_fn_vals[qoi+numFunctions] - hf_offset[qoi+numFunctions] :
	  hf_fn_vals[qoi+numFunctions];

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

} // namespace Dakota
