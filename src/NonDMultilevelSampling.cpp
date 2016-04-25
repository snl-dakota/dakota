/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDMultilevelSampling
//- Description: Implementation code for NonDMultilevelSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevelSampling.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevelSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevelSampling::
NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model)
{
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
  // set initial response mode for {init,set}_communicators.
  if (iteratedModel.surrogate_type() == "hierarchical")
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // init,set LF,HF
  else {
    Cerr << "Error: Multilevel Monte Carlo requires a hierarchical "
	 << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  const SizetArray& pilot_N_l = probDescDB.get_sza("method.nond.pilot_samples");
  size_t pilot_size = pilot_N_l.size(),
    num_mf     = iteratedModel.subordinate_models(false).size(),
    // for now, only SimulationModel supports solution_levels()
    num_hf_lev = iteratedModel.truth_model().solution_levels(),
    num_lf_lev = iteratedModel.surrogate_model().solution_levels();

  NLev.resize(num_mf); deltaNLev.resize(num_mf);
  if (num_mf > 1) {
    //hierarchMode = true;
    NLev[0].assign(num_lf_lev, 0); NLev[1].assign(num_hf_lev, 0); 
    if (pilot_size == 0) {
      deltaNLev[0].assign(num_lf_lev, 100);
      deltaNLev[1].assign(num_hf_lev, 100);
    }
    else if (pilot_size == 1) {
      deltaNLev[0].assign(num_lf_lev, pilot_N_l[0]);
      deltaNLev[1].assign(num_hf_lev, pilot_N_l[0]);
    }
    else if (pilot_size == num_hf_lev && pilot_size == num_lf_lev)
      deltaNLev[0] = deltaNLev[1] = pilot_N_l;
    else if (pilot_size == num_hf_lev + num_lf_lev) {
      copy_data_partial(pilot_N_l, 0, num_lf_lev, deltaNLev[0]);
      copy_data_partial(pilot_N_l, num_lf_lev, num_hf_lev, deltaNLev[1]);
    }
    else {
      Cerr << "Error: bad pilot samples input for NonDMultilevelSampling."
	   << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
  else {
    //solnCntlMode = true;
    NLev[0].assign(num_hf_lev, 0);
    if (pilot_N_l.empty())
      deltaNLev[0].assign(num_hf_lev, 100); // default
    else if (pilot_N_l.size() == 1)
      deltaNLev[0].assign(num_hf_lev, pilot_N_l[0]);
    else {
      deltaNLev[0] = pilot_N_l;
      if (pilot_N_l.size() != num_hf_lev) {
	Cerr << "Error: bad pilot samples input for NonDMultilevelSampling."
	     << std::endl;
	abort_handler(METHOD_ERROR);
      }
    }
  }

  switch (pilot_size) {
  case 0: maxEvalConcurrency *= 100;          break;
  case 1: maxEvalConcurrency *= pilot_N_l[0]; break;
  default: {
    size_t i, max_ps = 0;
    for (i=0; i<pilot_size; ++i)
      if (pilot_N_l[i] > max_ps)
	max_ps = pilot_N_l[i];
    if (max_ps)
      maxEvalConcurrency *= max_ps;
    break;
  }
  }
}


NonDMultilevelSampling::~NonDMultilevelSampling()
{ }


bool NonDMultilevelSampling::resize()
{
  bool parent_reinit_comms = NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);

  return parent_reinit_comms;
}


void NonDMultilevelSampling::pre_run()
{
  Analyzer::pre_run();

  // TO DO
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDMultilevelSampling::core_run()
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
  // > separate method specs that both map to NonDMultilevelSampling ???

  // TO DO: following pilot sample across levels and fidelities in mixed case,
  // could pair models for CVMC based on estimation of rho2_LH.

  size_t model_form = 0, soln_level = 0, num_mf = NLev.size();
  if (num_mf > 1) {
    size_t num_hf_lev = NLev[1].size();
    if (num_hf_lev > 1) { // && NLev[0].size() == num_hf_lev) {
      // multiple model forms + multiple solutions levels --> perform MLMC on
      // HF model and bind 1:min(num_hf,num_lf) LF control variates starting
      // at coarsest level (TO DO: validate case of unequal levels)
      multilevel_control_variate_mc(model_form, model_form+1);
    }
    else { // multiple model forms (only) --> CVMC
      SizetSizetPair lf_form_level(model_form,   soln_level),
	             hf_form_level(model_form+1, soln_level);
      control_variate_mc(lf_form_level, hf_form_level);
    }
  }
  else // multiple solutions levels (only) --> traditional ML-MC
    multilevel_mc(model_form);
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where the discrepancy at
    each level employs CVMC across two model forms. */
void NonDMultilevelSampling::
multilevel_control_variate_mc(size_t lf_model_form, size_t hf_model_form)
{
  iteratedModel.surrogate_model_indices(lf_model_form); // for init levs,cost
  iteratedModel.truth_model_indices(hf_model_form);     // for init levs,cost

  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();
  size_t qoi, iter = 0, samp, new_N_l, lev,
    num_hf_lev = truth_model.solution_levels(),
    num_lf_lev = surr_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, num_lf_lev);
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real avg_eval_ratio, eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0.,
    lf_lev_cost, hf_lev_cost;
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_cost(),
    lf_cost = surr_model.solution_level_cost(), agg_var_hf(num_hf_lev);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[1] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_H; RealMatrix sum_HH(numFunctions, num_hf_lev);
  initialize_ml_sums(sum_H, num_hf_lev);

  IntRealMatrixMap sum_L_refined, sum_L_shared, sum_LL, sum_LH;
  initialize_cv_sums_moments(sum_L_shared, sum_L_refined, //sum_H,
			     sum_LL, /*sum_HH,*/ sum_LH, num_cv_lev);
  RealMatrix var_H(numFunctions, num_cv_lev, false),
           rho2_LH(numFunctions, num_cv_lev, false);
  RealVector Lambda(num_cv_lev, false), avg_rho2_LH(num_cv_lev, false);
  
  // Initialize for pilot sample
  SizetArray&       N_lf =      NLev[lf_model_form];
  SizetArray&       N_hf =      NLev[hf_model_form]; 
  SizetArray& delta_N_lf = deltaNLev[lf_model_form];
  SizetArray& delta_N_hf = deltaNLev[hf_model_form];
  Cout << "\nMLMC pilot sample:\n" << delta_N_hf << std::endl;

  // now converge on sample counts per level (N_hf)
  while (Pecos::l1_norm(delta_N_hf) && iter <= max_iter) {

    // set initial surrogate responseMode and model indices for lev 0
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // surr resp
    iteratedModel.surrogate_model_indices(hf_model_form, 0); // HF level 0

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_hf_lev; ++lev) {

      lf_lev_cost = lf_cost[lev]; hf_lev_cost = hf_cost[lev];
      if (lev) {
	iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // both resp
	iteratedModel.surrogate_model_indices(hf_model_form, lev-1);// HF lev-1
	iteratedModel.truth_model_indices(hf_model_form,     lev);  // HF lev
	lf_lev_cost += lf_cost[lev-1]; hf_lev_cost += hf_cost[lev-1];// 2 levels
      }

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model
	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);
	// update total samples performed for this level
	N_hf[lev] += numSamples;
	// accumulate all orders (only order 2 for sum_HH for
	// computing agg_var_hf_l)
	accumulate_ml_sums(sum_H, sum_HH, lev);
	if (outputLevel == DEBUG_OUTPUT) {
	  Cout << "Accumulated sums (H[1], H[2], HH):\n";
	  write_data(Cout, sum_H[1]); write_data(Cout, sum_H[2]);
	  write_data(Cout, sum_HH);
	}

	// control variate betwen LF and HF for this discretization level:
	// if unequal number of levels, loop over all HF levels for MLMC and
	// apply CVMC when LF levels are available.  LF levels are assigned as
	// control variates to the leading set of HF levels, since these will
	// tend to have larger variance.      
	if (lev < num_lf_lev) {

	  // store allResponses used for sum_H (and sum_HH)
	  IntResponseMap hf_resp = allResponses; // shallow copy
	  // activate LF response (lev 0) or LF response discrepancy (lev > 0)
	  // within the hierarchical surrogate model.  Level indices & surrogate
	  // response mode are same as HF above, only the model form changes.
	  // However, we must pass the unchanged level index to update the
	  // corresponding variable values for the new model form.
	  if (lev) {
	    iteratedModel.surrogate_model_indices(lf_model_form, lev-1);
	    iteratedModel.truth_model_indices(lf_model_form,     lev);
	  }
	  else
	    iteratedModel.surrogate_model_indices(lf_model_form, 0);
	  // compute allResp w/ LF model form reusing allVars from MLMC step
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // update total samples performed for this level
	  N_lf[lev] += numSamples;
	  // process previous and new set of allResponses for CV sums;
	  // sum_H and sum_HH have already been accumulated above
	  accumulate_cv_sums(allResponses, hf_resp, sum_L_shared,
			     sum_L_refined, sum_LL, sum_LH, lev);
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (L_shared[1,2], L_refined[1,2], LH[1,2])"
		 << ":\n";
	    write_data(Cout,sum_L_shared[1]); write_data(Cout,sum_L_shared[2]);
	    write_data(Cout,sum_L_refined[1]);write_data(Cout,sum_L_refined[2]);
	    write_data(Cout,sum_LH[1]);       write_data(Cout,sum_LH[2]);
	  }

	  // compute the average evaluation ratio and Lambda factor
	  avg_eval_ratio
	    = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_HH,
			 sum_LH[1], hf_lev_cost/lf_lev_cost, lev, var_H,
			 rho2_LH, N_hf[lev]);
	  avg_rho2_LH[lev] = average(rho2_LH[lev], numFunctions);
	  Lambda[lev] = 1. - avg_rho2_LH[lev]
	              * (avg_eval_ratio - 1.) / avg_eval_ratio;
	  // now execute additional LF sample increment, if needed
	  if (lf_increment(avg_eval_ratio,  N_hf[lev],
			   delta_N_lf[lev], N_lf[lev])) {
	    accumulate_cv_sums(sum_L_refined, lev); // all incl LF increment
	    if (outputLevel == DEBUG_OUTPUT) {
	      Cout << "Accumulated sums (L_refined[1,2]):\n";
	      write_data(Cout, sum_L_refined[1]);
	      write_data(Cout, sum_L_refined[2]);
	    }
	  }
	}

	// compute estimator mean & variance from current sample accumulation:
	agg_var_hf_l = 0.; size_t N_l = N_hf[lev];
	Real *sum_H1_l = sum_H[1][lev], *sum_HH1_l = sum_HH[lev],
	  bias_corr = 1./(N_l - 1);
	for (qoi=0; qoi<numFunctions; ++qoi) {
	  Real mu_Y = sum_H1_l[qoi] / N_l;
	  // Note: precision loss in variance is difficult to avoid without
	  // storing full sample history; must accumulate Y^2 across iters
	  // instead of (Y-mean)^2 since mean is updated on each iteration.
	  agg_var_hf_l += (sum_HH1_l[qoi] - N_l * mu_Y * mu_Y) * bias_corr;
	}
      }

      // accumulate sum of sqrt's of estimator var * cost used in new_N_l
      sum_sqrt_var_cost += (lev < num_lf_lev) ?
	std::sqrt(agg_var_hf_l * hf_lev_cost * Lambda[lev] /
		  (1. - avg_rho2_LH[lev])) :
	std::sqrt(agg_var_hf_l * hf_lev_cost);
      // mean sq error reference is MC applied to HF:
      if (iter == 0) estimator_var0 += agg_var_hf_l / N_hf[lev];
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (iter == 0) { // eps^2 / 2 = var * relative factor
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // update targets based on variance estimates
    Real fact = sum_sqrt_var_cost / eps_sq_div_2;
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = (lev) ? hf_cost[lev] + hf_cost[lev-1] : hf_cost[lev];
      new_N_l = (lev < num_lf_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = (new_N_l > N_hf[lev]) ? new_N_l - N_hf[lev] : 0;
    }
    ++iter;
    Cout << "\nMLMC iteration " << iter << " sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // Iteration complete.  Now roll up raw moments from combining final
  // CVMC and MLMC estimators.
  RealMatrix Y_mlmc_mom(4, numFunctions), Y_cvmc_mom(4, numFunctions, false);
  for (lev=0; lev<num_lf_lev; ++lev) {
    cv_raw_moments(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH, rho2_LH,
		   //hf_lev_cost/lf_lev_cost,
		   lev, N_hf[lev], N_lf[lev], Y_cvmc_mom);// N_shared,N_refined
    Y_mlmc_mom += Y_cvmc_mom;
  }
  if (num_hf_lev > num_lf_lev) {
    RealMatrix &sum_H1 = sum_H[1], &sum_H2 = sum_H[2],
               &sum_H3 = sum_H[3], &sum_H4 = sum_H[4];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      Real *Y_ml_q = Y_mlmc_mom[qoi];
      for (lev=num_lf_lev; lev<num_hf_lev; ++lev) {
	size_t Nl = N_hf[lev];
	Y_ml_q[0] += sum_H1(qoi,lev) / Nl;  Y_ml_q[1] += sum_H2(qoi,lev) / Nl;
	Y_ml_q[2] += sum_H3(qoi,lev) / Nl;  Y_ml_q[3] += sum_H4(qoi,lev) / Nl;
      }
    }
  }
  // Convert uncentered raw moment estimates to standardized moments
  convert_moments(Y_mlmc_mom, momentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = N_hf[0] * hf_cost[0] + N_lf[0] * lf_cost[0]; // first level
  for (lev=1; lev<num_hf_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += N_hf[lev] * (hf_cost[lev] + hf_cost[lev-1]);
  for (lev=1; lev<num_lf_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += N_lf[lev] * (lf_cost[lev] + lf_cost[lev-1]);
  equivHFEvals /= hf_cost[num_hf_lev-1]; // normalize into equivalent HF evals
}


/** This function performs "geometrical" MLMC on a single model form
    with multiple discretization levels. */
void NonDMultilevelSampling::multilevel_mc(size_t model_form)
{
  // Formulate as a coordinated progression towards convergence, where, e.g.,
  // time step is inferred from the spatial discretization (NOT an additional
  // solution control) based on stability criteria, e.g. CFL condition.  Can
  // we reliably capture runtime estimates as part of pilot run w/i Dakota?
  // Ultimately seems desirable to support either online or offline cost
  // estimates, to allow more accurate resource allocation when possible
  // or necessary (e.g., combustion processes with expense that is highly
  // parameter dependent).
  //   model
  //     id_model = 'LF'
  //     simulation
  //       # point to state vars; ordered based on set values for h, delta-t
  //       solution_level_control = 'dssiv1'
  //       # relative cost estimates in same order as state set values
  //       # --> re-sort into map keyed by increasing cost
  //       solution_level_cost = 10 2 200

  // How to manage the set of MLMC statistics:
  // 1. Simplest: proposal is to use the mean estimator to drive the algorithm,
  //    but carry along other estimates.
  // 2. Later: could consider a refinement for converging the estimator of the
  //    variance after convergence of the mean estimator.

  // How to manage the vector of QoI:
  // 1. Worst case: select N_l based only on QoI w/ highest total variance
  //      from pilot run --> fix for all levels and don't allow switching
  //      across major iterations (possible oscillation?  Or simple overlay
  //      of resolution reqmts?)
  // 2. Better: select N_l based on convergence in aggregated variance.
  
  iteratedModel.surrogate_model_indices(model_form);// soln lev not updated yet
  iteratedModel.truth_model_indices(model_form);    // soln lev not updated yet

  Model& truth_model  = iteratedModel.truth_model();
  size_t lev, num_lev = truth_model.solution_levels(), // single model form
    qoi, iter = 0, samp, new_N_l;
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., lev_cost;
  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost = truth_model.solution_level_cost(), agg_var(num_lev);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[i] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_Y; RealMatrix sum_YY(numFunctions, num_lev);
  initialize_ml_sums(sum_Y, num_lev);
  
  // Initialize for pilot sample
  SizetArray& N_l = NLev[model_form];
  SizetArray& delta_N_l = deltaNLev[model_form];
  Cout << "\nMLMC pilot sample:\n" << delta_N_l << std::endl;

  // now converge on sample counts per level (N_l)
  while (Pecos::l1_norm(delta_N_l) && iter <= max_iter) {

    // set initial surrogate responseMode and model indices for lev 0
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // LF
    iteratedModel.surrogate_model_indices(model_form, 0); // solution level 0

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_lev; ++lev) {

      lev_cost = cost[lev];
      if (lev) {
	if (lev == 1) // update responseMode for levels 1:num_lev-1
	  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // {LF,HF}
	iteratedModel.surrogate_model_indices(model_form, lev-1);
	iteratedModel.truth_model_indices(model_form,     lev);
	lev_cost += cost[lev-1]; // discrepancies incur 2 level costs
      }

      // set the number of current samples from the defined increment
      numSamples = delta_N_l[lev];

      // aggregate variances across QoI for estimating N_l (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_l = agg_var[lev]; // carried over from prev iter if no samp
      if (numSamples) {

	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model
	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);
	// update total samples performed for this level
	N_l[lev] += numSamples;

	// process allResponses: accumulate new samples for each qoi
	accumulate_ml_sums(sum_Y, sum_YY, lev);

	// compute estimator value & variance from current sample accumulation:
	agg_var_l = 0.; size_t N = N_l[lev];
	Real *sum_Y1_l = sum_Y[1][lev],
	    *sum_YY1_l = sum_YY[lev], bias_corr = 1./(N - 1);
	for (qoi=0; qoi<numFunctions; ++qoi) {
	  Real mu_Y = sum_Y1_l[qoi] / N_l[lev];
	  // Note: precision loss in variance is difficult to avoid without
	  // storing full sample history; must accumulate Y^2 across iterations
	  // instead of (Y-mean)^2 since mean is updated on each iteration.
	  agg_var_l += (sum_YY1_l[qoi] - N * mu_Y * mu_Y) * bias_corr;
	}
      }

      sum_sqrt_var_cost += std::sqrt(agg_var_l * lev_cost);
      // MSE reference is MC applied to HF:
      if (iter == 0) estimator_var0 += agg_var_l / N_l[lev];
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (iter == 0) { // eps^2 / 2 = var * relative factor
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // update targets based on variance estimates
    Real fact = sum_sqrt_var_cost / eps_sq_div_2;
    for (lev=0; lev<num_lev; ++lev) {
      // Equation 3.9 in CTR Annual Research Briefs:
      // "A multifidelity control variate approach for the multilevel Monte 
      // Carlo technique," Geraci, Eldred, Iaccarino, 2015.
      new_N_l = std::sqrt(agg_var[lev] / lev_cost) * fact;
      delta_N_l[lev] = (new_N_l > N_l[lev]) ? new_N_l - N_l[lev] : 0;
    }
    ++iter;
    Cout << "\nMLMC iteration " << iter << " sample increments:\n" << delta_N_l
	 << std::endl;
    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "Accumulated sums (Y1, Y2, Y3, Y4, Y1sq):\n";
      write_data(Cout, sum_Y[1]); write_data(Cout, sum_Y[2]);
      write_data(Cout, sum_Y[3]); write_data(Cout, sum_Y[4]);
      write_data(Cout, sum_YY);   Cout << std::endl;
   }
  }

  // aggregate expected value of estimators for Y, Y^2, Y^3, Y^4. Final expected
  // value is sum of expected values from telescopic sum. There is no bias
  // correction for small sample sizes as in NonDSampling::compute_moments().
  RealMatrix Y_raw_mom(4, numFunctions);
  RealMatrix &sum_Y1 = sum_Y[1], &sum_Y2 = sum_Y[2],
	     &sum_Y3 = sum_Y[3], &sum_Y4 = sum_Y[4];
  for (qoi=0; qoi<numFunctions; ++qoi) {
    Real *Y_rm_q = Y_raw_mom[qoi];
    for (lev=0; lev<num_lev; ++lev) {
      size_t Nl = N_l[lev];
      Y_rm_q[0] += sum_Y1(qoi,lev) / Nl;  Y_rm_q[1] += sum_Y2(qoi,lev) / Nl;
      Y_rm_q[2] += sum_Y3(qoi,lev) / Nl;  Y_rm_q[3] += sum_Y4(qoi,lev) / Nl;
    }
  }
  // Convert uncentered raw moment estimates to standardized moments
  convert_moments(Y_raw_mom, momentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = N_l[0] * cost[0]; // first level is single eval
  for (lev=1; lev<num_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += N_l[lev] * (cost[lev] + cost[lev-1]);
  equivHFEvals /= cost[num_lev-1]; // normalize into equivalent HF evals
}


/** This function performs control variate MC on multiple model forms
    using a single discretization level. */
void NonDMultilevelSampling::
control_variate_mc(const SizetSizetPair& lf_form_level,
		   const SizetSizetPair& hf_form_level)
{
  iteratedModel.surrogate_model_indices(lf_form_level);
  iteratedModel.truth_model_indices(hf_form_level);
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  // retrieve cost estimates across model forms for a particular soln level
  Real lf_cost    =  surr_model.solution_level_cost()[lf_form_level.second],
       hf_cost    = truth_model.solution_level_cost()[hf_form_level.second],
       cost_ratio = hf_cost / lf_cost;
  size_t iter = 0;

  IntRealVectorMap sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH;
  initialize_cv_sums_moments(sum_L_shared, sum_L_refined, sum_H, sum_LL,sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false),
            rho2_LH(numFunctions, false);

  size_t& delta_N_lf = deltaNLev[lf_form_level.first][lf_form_level.second];
  size_t& delta_N_hf = deltaNLev[hf_form_level.first][hf_form_level.second];
  size_t&       N_lf =      NLev[lf_form_level.first][lf_form_level.second];
  size_t&       N_hf =      NLev[hf_form_level.first][hf_form_level.second];

  // ---------------------
  // Compute Pilot Samples
  // ---------------------

  // Initialize for pilot sample (shared sample count discarding any excess)
  numSamples = std::min(delta_N_lf, delta_N_hf);
  shared_increment(iter, lf_form_level, hf_form_level);
  accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H,
		     sum_LL, sum_HH, sum_LH);

  // compute the LF/HF evaluation ratio, averaged over the QoI
  // This includes updating var_H and rho2_LH
  Real avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_HH,
				   sum_LH[1], cost_ratio, var_H, rho2_LH, N_hf);
  // compute the ratio of MC and CVMC mean squared errors (controls convergence)
  Real avg_mse_ratio = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, iter, N_hf);

  // -----------------------------------------------------------
  // Compute HF + LF increment targeting specified MSE reduction
  // -----------------------------------------------------------

  // bypass refinement if maxIterations == 0 or convergenceTol already
  // satisfied by pilot sample
  if (maxIterations != 0 && avg_mse_ratio > convergenceTol) {

    // Assuming rho_AB, evaluation_ratio and var_H to be relatively invariant,
    // we seek a relative reduction in MSE using the convergence tol spec:
    //   convTol = CV_mse / MC^0_mse = mse_ratio * N0 / N
    //   delta_N = mse_ratio*N0/convTol - N0 = (mse_ratio/convTol - 1) * N0
    Real incr = (avg_mse_ratio / convergenceTol - 1.) * numSamples;
    numSamples = (size_t)std::floor(incr + .5); // round

    if (numSamples) { // small incr did not round up
      shared_increment(++iter, lf_form_level, hf_form_level);
      accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H,
			 sum_LL, sum_HH, sum_LH);
      // update ratios:
      avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_HH,
				  sum_LH[1], cost_ratio, var_H, rho2_LH, N_hf);
      avg_mse_ratio = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, iter, N_hf);
    }
  }

  // --------------------------------------------------
  // Compute LF increment based on the evaluation ratio
  // --------------------------------------------------
  iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
  if (lf_increment(avg_eval_ratio, N_hf, delta_N_lf, N_lf))
    accumulate_cv_sums(sum_L_refined);

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(4, numFunctions);
  cv_raw_moments(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH, rho2_LH,
		 N_hf, N_lf, H_raw_mom); // N_shared, N_refined
  // Convert uncentered raw moment estimates to standardized moments
  convert_moments(H_raw_mom, momentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = N_hf + (Real)N_lf / cost_ratio;
}


void NonDMultilevelSampling::
shared_increment(size_t iter, const SizetSizetPair& lf_form_level,
		 const SizetSizetPair& hf_form_level)
{
  deltaNLev[lf_form_level.first][lf_form_level.second]
    = deltaNLev[hf_form_level.first][hf_form_level.second] = numSamples;

  if (iter == _NPOS)  Cout << "\nCVMC sample increments: ";
  else if (iter == 0) Cout << "\nCVMC pilot sample: ";
  else Cout << "\nCVMC iteration " << iter << " sample increments: ";
  Cout << "LF = " << numSamples << " HF = " << numSamples << '\n';

  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);

  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model
  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);

  NLev[lf_form_level.first][lf_form_level.second] += numSamples;
  NLev[hf_form_level.first][hf_form_level.second] += numSamples;
}


bool NonDMultilevelSampling::
lf_increment(Real avg_eval_ratio, size_t N_hf, size_t& delta_N_lf, size_t& N_lf)
{
  // ----------------------------------------------
  // Compute Final LF increment for control variate
  // ----------------------------------------------

  // update LF samples based on evaluation ratio
  // r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  // or with inverse r  -> delta = m-n = n/inverse_r - n
  size_t N_lf_target = (size_t)std::floor(N_hf * avg_eval_ratio + .5); // round
  delta_N_lf = (N_lf_target > N_lf) ? N_lf_target - N_lf : 0;
  // reserve HF counts for MLMC usage in hybrid case:
  //deltaNLev[hf_form_level.first][hf_form_level.second] = 0;

  if (delta_N_lf) {
    Cout << "CVMC LF sample increment = " << delta_N_lf << std::endl;

    // set the number of current samples from the defined increment
    numSamples = delta_N_lf;

    // mode for hierarchical surrogate model can be uncorrected surrogate
    // for CV MC, or uncorrected surrogate/aggregated models for ML-CV MC
    // --> set at calling level
    //iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);

    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);// pull dist params from any model
    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);

    N_lf += delta_N_lf;
    return true;
  }
  else
    return false;
}

  
void NonDMultilevelSampling::
initialize_cv_sums_moments(IntRealVectorMap& sum_L_shared,
			   IntRealVectorMap& sum_L_refined,
			   IntRealVectorMap& sum_H, IntRealVectorMap& sum_LL,
                         //IntRealVectorMap& sum_HH,
			   IntRealVectorMap& sum_LH)
{
  // sum_* are running sums across all increments
  std::pair<int, RealVector> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use IntRVMIter to size RealVector in place and init sums to 0
    sum_L_shared.insert(empty_pr).first->second.size(numFunctions);
    sum_L_refined.insert(empty_pr).first->second.size(numFunctions);
    sum_H.insert(empty_pr).first->second.size(numFunctions);
    sum_LL.insert(empty_pr).first->second.size(numFunctions);
  //sum_HH.insert(empty_pr).first->second.size(numFunctions);
    sum_LH.insert(empty_pr).first->second.size(numFunctions);
  }
}


void NonDMultilevelSampling::
initialize_cv_sums_moments(IntRealMatrixMap& sum_L_shared,
			   IntRealMatrixMap& sum_L_refined,
			 //IntRealMatrixMap& sum_H,
			   IntRealMatrixMap& sum_LL,
                         //IntRealMatrixMap& sum_HH,
			   IntRealMatrixMap& sum_LH, size_t num_lev)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use IntRVMIter to shape RealMatrix in place and init sums to 0
    sum_L_shared.insert(empty_pr).first->second.shape(numFunctions, num_lev);
    sum_L_refined.insert(empty_pr).first->second.shape(numFunctions, num_lev);
  //sum_H.insert(empty_pr).first->second.shape(numFunctions, num_lev);
    sum_LL.insert(empty_pr).first->second.shape(numFunctions, num_lev);
  //sum_HH.insert(empty_pr).first->second.shape(numFunctions, num_lev);
    sum_LH.insert(empty_pr).first->second.shape(numFunctions, num_lev);
  }
}


void NonDMultilevelSampling::
initialize_ml_sums(IntRealMatrixMap& sum_Y, size_t num_lev)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRMMIter, bool>:
    // use IntRMMIter to shape RealMatrix in place and init sums to 0
    sum_Y.insert(empty_pr).first->second.shape(numFunctions, num_lev);
  }
}


void NonDMultilevelSampling::accumulate_cv_sums(IntRealVectorMap& sum_map)
{
  // uses one set of allResponses in UNCORRECTED_SURROGATE mode
  // IntRealVectorMap is not a multilevel case --> no discrepancies

  Real fn_val, prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRVMIter sum_it; 
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      prod = fn_val = fn_vals[qoi];

      sum_it = sum_map.begin(); ord = sum_it->first; active_ord = 1;
      while (sum_it!=sum_map.end()) {
    
	if (ord == active_ord) {
	  sum_it->second[qoi] += prod; ++sum_it;
	  ord = (sum_it == sum_map.end()) ? 0 : sum_it->first;
	}

	prod *= fn_val;
	++active_ord;
      }
    }
  }
}


void NonDMultilevelSampling::
accumulate_cv_sums(IntRealVectorMap& sum_L_shared,
		   IntRealVectorMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealVectorMap& sum_LL, RealVector& sum_HH,
		   IntRealVectorMap& sum_LH)
{
  // uses one set of allResponses in AGGREGATED_MODELS mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter ls_it, lr_it, h_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord; size_t qoi;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_prod = lf_fn = fn_vals[qoi];
      hf_prod = hf_fn = fn_vals[qoi+numFunctions];

      ls_it = sum_L_shared.begin(); lr_it = sum_L_refined.begin();
      h_it  = sum_H.begin(); ll_it = sum_LL.begin(); lh_it = sum_LH.begin();
      ls_ord = /*(ls_it == sum_L_shared.end())  ? 0 :*/ ls_it->first;
      lr_ord = /*(lr_it == sum_L_refined.end()) ? 0 :*/ lr_it->first;
      h_ord  = /*(h_it  == sum_H.end())  ? 0 :*/  h_it->first;
      ll_ord = /*(ll_it == sum_LL.end()) ? 0 :*/ ll_it->first;
      lh_ord = /*(lh_it == sum_LH.end()) ? 0 :*/ lh_it->first;
      active_ord = 1;

      while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
	     h_it!=sum_H.end() || ll_it!=sum_LL.end() || active_ord <= 1 ||
	     lh_it!=sum_LH.end()) {
    
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
	// High-High
	if (active_ord == 1) sum_HH[qoi] += hf_prod * hf_prod;
	// Low-High
	if (lh_ord == active_ord) {
	  lh_it->second[qoi] += lf_prod * hf_prod;
	  ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	}

	if (ls_ord || lr_ord || ll_ord || lh_ord) lf_prod *= lf_fn;
	if (h_ord  || lh_ord)                     hf_prod *= hf_fn;
	++active_ord;
      }
    }
  }
}


void NonDMultilevelSampling::
accumulate_cv_sums(IntRealMatrixMap& sum_map, size_t lev)
{
  // uses one set of allResponses in UNCORRECTED_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap is a multilevel
  // case with discrepancies, indexed by level.

  Real fn_l, prod_l;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRMMIter sum_it; 
  if (lev == 0) { // UNCORRECTED_SURROGATE -> 1 set of qoi per response map
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	prod_l = fn_l = fn_vals[qoi];

	sum_it = sum_map.begin(); ord = sum_it->first; active_ord = 1;
	while (sum_it!=sum_map.end()) {
    
	  if (ord == active_ord) {
	    sum_it->second(qoi,lev) += prod_l; ++sum_it;
	    ord = (sum_it == sum_map.end()) ? 0 : sum_it->first;
	  }

	  prod_l *= fn_l;
	  ++active_ord;
	}
      }
      //Cout << r_it->first << ": "; write_data(Cout, sum_map[1]);
    }
  }
  else { // AGGREGATED_MODELS -> 2 sets of qoi per response map
    Real fn_lm1, prod_lm1;
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	prod_l   = fn_l   = fn_vals[qoi+numFunctions];
	prod_lm1 = fn_lm1 = fn_vals[qoi];

	sum_it = sum_map.begin(); ord = sum_it->first; active_ord = 1;
	while (sum_it!=sum_map.end()) {
    
	  if (ord == active_ord) {
	    sum_it->second(qoi,lev) += prod_l - prod_lm1; ++sum_it;
	    ord = (sum_it == sum_map.end()) ? 0 : sum_it->first;
	  }

	  prod_l *= fn_l; prod_lm1 *= fn_lm1;
	  ++active_ord;
	}
      }
      //Cout << r_it->first << ": "; write_data(Cout, sum_map[1]);
    }
  }
}


void NonDMultilevelSampling::
accumulate_cv_sums(const IntResponseMap& lf_resp_map,
		   const IntResponseMap& hf_resp_map,
		   IntRealMatrixMap& sum_L_shared,
		   IntRealMatrixMap& sum_L_refined, //IntRealMatrixMap& sum_H,
		   IntRealMatrixMap& sum_LL,        //RealMatrix& sum_HH,
		   IntRealMatrixMap& sum_LH, size_t lev)
{
  // uses two sets of responses (LF & HF) in UNCORRECTED_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap are for multilevel
  // case with discrepancies, indexed by level.

  Real lf_l, hf_l, lf_l_prod, hf_l_prod;
  IntRespMCIter lf_r_it, hf_r_it; IntRMMIter ls_it, lr_it, ll_it, lh_it;
  int ls_ord, lr_ord, ll_ord, lh_ord, active_ord; size_t qoi;

  if (lev == 0) { // UNCORRECTED_SURROGATE -> 1 set of qoi per response map
    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	lf_l_prod = lf_l = lf_fn_vals[qoi];
	hf_l_prod = hf_l = hf_fn_vals[qoi];

	ls_it = sum_L_shared.begin(); lr_it = sum_L_refined.begin();
	ll_it = sum_LL.begin();       lh_it = sum_LH.begin();
	ls_ord = (ls_it == sum_L_shared.end())  ? 0 : ls_it->first;
	lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	ll_ord = (ll_it == sum_LL.end())        ? 0 : ll_it->first;
	lh_ord = (lh_it == sum_LH.end())        ? 0 : lh_it->first;
	active_ord = 1;

	while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
	       ll_it!=sum_LL.end()       || lh_it!=sum_LH.end()) {

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

	  if (ls_ord || lr_ord || ll_ord || lh_ord) lf_l_prod *= lf_l;
	  if (lh_ord) hf_l_prod *= hf_l;
	  ++active_ord;
	}
      }
    }
  }
  else { // AGGREGATED_MODELS -> 2 sets of qoi per response map
    Real lf_lm1, hf_lm1, lf_lm1_prod, hf_lm1_prod;
    for (lf_r_it =lf_resp_map.begin(), hf_r_it =hf_resp_map.begin();
	 lf_r_it!=lf_resp_map.end() && hf_r_it!=hf_resp_map.end();
	 ++lf_r_it, ++hf_r_it) {
      const RealVector& lf_fn_vals = lf_r_it->second.function_values();
      const RealVector& hf_fn_vals = hf_r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {

	lf_l_prod   = lf_l   = lf_fn_vals[qoi+numFunctions];
	lf_lm1_prod = lf_lm1 = lf_fn_vals[qoi];
	hf_l_prod   = hf_l   = hf_fn_vals[qoi+numFunctions];
	hf_lm1_prod = hf_lm1 = hf_fn_vals[qoi];

	ls_it  = sum_L_shared.begin(); lr_it = sum_L_refined.begin();
	ll_it  = sum_LL.begin();       lh_it = sum_LH.begin();
	ls_ord = (ls_it == sum_L_shared.end())  ? 0 : ls_it->first;
	lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	ll_ord = (ll_it == sum_LL.end())        ? 0 : ll_it->first;
	lh_ord = (lh_it == sum_LH.end())        ? 0 : lh_it->first;
	active_ord = 1;

	while (ls_it!=sum_L_shared.end() || lr_it!=sum_L_refined.end() ||
	       ll_it!=sum_LL.end()       || lh_it!=sum_LH.end()) {

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

	  if (ls_ord || lr_ord || ll_ord || lh_ord)
	    { lf_l_prod *= lf_l; lf_lm1_prod *= lf_lm1; }
	  if (lh_ord)
	    { hf_l_prod *= hf_l; hf_lm1_prod *= hf_lm1; }
	  ++active_ord;
	}
      }
    }
  }
}


void NonDMultilevelSampling::
accumulate_ml_sums(IntRealMatrixMap& sum_Y, RealMatrix& sum_YY, size_t lev)
{
  Real lf_fn, lf_prod;
  int y_ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRMMIter y_it;
  if (lev == 0) {
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();
      for (qoi=0; qoi<numFunctions; ++qoi) {

	lf_prod = lf_fn = fn_vals[qoi];
	y_it = sum_Y.begin(); y_ord = y_it->first; active_ord = 1;

	while (y_it!=sum_Y.end() || active_ord <= 1) {
	  // add to sum_Y: running sums across all sample increments
	  if (y_ord == active_ord) {
	    y_it->second(qoi,lev) += lf_prod; ++y_it;
	    y_ord = (y_it == sum_Y.end()) ? 0 : y_it->first;
	  }
	  // add to sum_YY: running sums across all sample increments
	  if (active_ord == 1) sum_YY(qoi,lev) += lf_prod * lf_prod;
	  
	  lf_prod *= lf_fn; ++active_ord;
	}
      }
    }
  }
  else {
    Real hf_fn, hf_prod;
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();
      for (qoi=0; qoi<numFunctions; ++qoi) {

	lf_prod = lf_fn = fn_vals[qoi];
	hf_prod = hf_fn = fn_vals[qoi+numFunctions];
	y_it = sum_Y.begin();  y_ord = y_it->first;  active_ord = 1;

	while (y_it!=sum_Y.end() || active_ord <= 1) {
	  // add to sum_Y: running sums across all sample increments
	  if (y_ord == active_ord) {
	    y_it->second(qoi,lev) += hf_prod - lf_prod; // HF^p-LF^p
	    ++y_it; y_ord = (y_it == sum_Y.end()) ? 0 : y_it->first;
	  }
	  // add to sum_YY: running sums across all sample increments
	  if (active_ord == 1) {
	    Real delta_prod = hf_prod - lf_prod;
	    sum_YY(qoi,lev) += delta_prod * delta_prod; // (HF^p-LF^p)^2
	  }
	  hf_prod *= hf_fn; lf_prod *= lf_fn; ++active_ord;
	}
      }
    }
  }
}


Real NonDMultilevelSampling::
eval_ratio(const RealVector& sum_L_shared, const RealVector& sum_H,
	   const RealVector& sum_LL, const RealVector& sum_HH,
	   const RealVector& sum_LH, Real cost_ratio, RealVector& var_H,
	   RealVector& rho2_LH, size_t N_shared)
{
  // Update rho^2, avg_eval_ratio:
  Real mu_L, mu_H, var_L, cov_LH, rho_sq, avg_eval_ratio = 0.,
       bias_corr = 1./(N_shared - 1);
  size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    // unbiased mean estimator X-bar = 1/N * sum
    mu_L =  sum_L_shared[qoi] / N_shared;
    mu_H =         sum_H[qoi] / N_shared;
    // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
    // = 1/(N-1) (sum[X^2_i] - N X-bar^2) where bias correction = 1/(N-1)
    var_L      = (sum_LL[qoi] - N_shared * mu_L * mu_L) * bias_corr;
    var_H[qoi] = (sum_HH[qoi] - N_shared * mu_H * mu_H) * bias_corr;
    cov_LH     = (sum_LH[qoi] - N_shared * mu_L * mu_H) * bias_corr;

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    // Given use of 1/r in MSE_ratio, one approach would average 1/r, but
    // this does not seem to behave as well in limited numerical experience.
    rho_sq = rho2_LH[qoi] = cov_LH / var_L * cov_LH / var_H[qoi];// bias cancels
    //if (rho_sq > Pecos::SMALL_NUMBER) {
    //  avg_inv_eval_ratio += std::sqrt((1. - rho_sq)/(cost_ratio * rho_sq));
    if (rho_sq < 1.) { // protect against division by 0
      avg_eval_ratio += std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
      ++num_avg;
    }
  }
  if (num_avg) avg_eval_ratio /= num_avg;
  else // should not happen, but provide a reasonable upper bound
    avg_eval_ratio = (Real)maxFunctionEvals / (Real)N_shared;

  return avg_eval_ratio;
}


Real NonDMultilevelSampling::
MSE_ratio(Real avg_eval_ratio, const RealVector& var_H,
	  const RealVector& rho2_LH, size_t iter, size_t N_hf)
{
  if (iter == 0) mcMSEIter0.sizeUninitialized(numFunctions);

  Real mc_mse, cvmc_mse, mse_ratio, avg_mse_ratio = 0.;//,avg_mse_iter_ratio=0.;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
    mse_ratio = 1. - rho2_LH[qoi] * (1. - 1. / avg_eval_ratio); // Ng 2014
    mc_mse = var_H[qoi] / N_hf; cvmc_mse = mc_mse * mse_ratio;
    Cout << "Mean square error for QoI " << qoi+1 << " reduced from " << mc_mse
	 << " (MC) to " << cvmc_mse << " (CV); factor = " << mse_ratio << '\n';
    //avg_mse_iter_ratio += mse_ratio;
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


void NonDMultilevelSampling::
cv_raw_moments(IntRealVectorMap& sum_L_shared, IntRealVectorMap& sum_L_refined,
	       IntRealVectorMap& sum_H,  IntRealVectorMap& sum_LL,
	       IntRealVectorMap& sum_LH, const RealVector& rho2_LH,
	       //Real cost_ratio,
	       size_t N_shared, size_t N_refined, RealMatrix& H_raw_mom)
{
  // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k
  Real beta, mu_L, mu_H, var_L, cov_LH, refined_mu_L,
       bias_corr = 1./(N_shared - 1);//, cr1 = cost_ratio + 1.;
  size_t qoi;
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(4, numFunctions);
  for (int i=1; i<=4; ++i) {
    const RealVector& sum_Ls_i = sum_L_shared[i];
    const RealVector& sum_Lr_i = sum_L_refined[i];
    const RealVector& sum_Hi  = sum_H[i];
    const RealVector& sum_LLi = sum_LL[i];
  //const RealVector& sum_HHi = sum_HH[i];
    const RealVector& sum_LHi = sum_LH[i];

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // expectations based on shared samples (prior to LF sample increment)
      mu_L   = sum_Ls_i[qoi] / N_shared;
      mu_H   = sum_Hi[qoi]   / N_shared;
      var_L  = (sum_LLi[qoi] - N_shared * mu_L * mu_L) * bias_corr;
    //var_H  = (sum_HHi[qoi] - N_shared * mu_H * mu_H) * bias_corr;
      cov_LH = (sum_LHi[qoi] - N_shared * mu_L * mu_H) * bias_corr;

      beta   = cov_LH / var_L;
      Cout << "Moment " << i << ", QoI " << qoi+1
	   << ": control variate beta = " << std::setw(9) << beta;
      if (i == 1) // rho2_LH not stored for i > 1
	Cout << ", rho_LH (Pearson correlation) = " << std::setw(9)
	     << std::sqrt(rho2_LH[qoi]);//<< ", effectiveness ratio = "
           //<< std::setw(9) << rho2_LH[qoi] * cr1;
      Cout << '\n';

      // updated LF expectations following final sample increment:
      refined_mu_L = sum_Lr_i[qoi] / N_refined;
      // apply control for HF uncentered raw moment estimates:
      H_raw_mom(i-1,qoi) = mu_H - beta * (mu_L - refined_mu_L);
    }
    if (numFunctions > 1) Cout << '\n';
  }
}


void NonDMultilevelSampling::
cv_raw_moments(IntRealMatrixMap& sum_L_shared, IntRealMatrixMap& sum_L_refined,
	       IntRealMatrixMap& sum_H,  IntRealMatrixMap& sum_LL,
	       IntRealMatrixMap& sum_LH, const RealMatrix& rho2_LH,
	       //Real cost_ratio,
	       size_t lev, size_t N_shared, size_t N_refined,
	       RealMatrix& H_raw_mom)
{
  // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k
  Real beta, mu_L, mu_H, var_L, cov_LH, refined_mu_L,
       bias_corr = 1./(N_shared - 1);//, cr1 = cost_ratio + 1.;
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(4, numFunctions);
  for (int i=1; i<=4; ++i) {
    const Real *sum_Ls_i = sum_L_shared[i][lev],
        *sum_Lr_i = sum_L_refined[i][lev], *sum_Hi  = sum_H[i][lev],
        *sum_LLi = sum_LL[i][lev],       //*sum_HHi = sum_HH[lev],
        *sum_LHi = sum_LH[i][lev],        *rho2_LHi = rho2_LH[lev];

    for (size_t qoi=0; qoi<numFunctions; ++qoi) {

      // expectations based on shared samples (prior to LF sample increment)
      mu_L   = sum_Ls_i[qoi] / N_shared;
      mu_H   = sum_Hi[qoi] / N_shared;
      var_L  = (sum_LLi[qoi] - N_shared * mu_L * mu_L) * bias_corr;
    //var_H  = (sum_HHi[qoi] - N_shared * mu_H * mu_H) * bias_corr;
      cov_LH = (sum_LHi[qoi] - N_shared * mu_L * mu_H) * bias_corr;

      beta = cov_LH / var_L;
      Cout << "Moment " << i << ", QoI " << qoi+1 << ", lev " << lev
	   << ": control variate beta = " << std::setw(9) << beta;
      if (i == 1) // rho2_LH not stored for i > 1
	Cout << ", rho_LH (Pearson correlation) = " << std::setw(9)
	     << std::sqrt(rho2_LHi[qoi]); //<< " Effectiveness ratio = "
           //<< std::setw(9) << rho2_LHi[qoi]*cr1;
      Cout << '\n';

      // updated LF expectations following final sample increment:
      refined_mu_L = sum_Lr_i[qoi] / N_refined;
      // apply control for HF uncentered raw moment estimates:
      H_raw_mom(i-1,qoi) = mu_H - beta * (mu_L - refined_mu_L);
    }
    if (numFunctions > 1) Cout << '\n';
  }
  Cout << '\n'; // for loop over levels
}


void NonDMultilevelSampling::
convert_moments(const RealMatrix& raw_mom, RealMatrix& standard_mom)
{
  if (standard_mom.empty())
    standard_mom.shapeUninitialized(4, numFunctions);
  Real m1, cm2, cm3, cm4;
  // Convert uncentered raw moment estimates to standardized moments
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    m1  = standard_mom(0,qoi) = raw_mom(0,qoi);   // mean
    // convert from uncentered to centered moments
    cm2 = raw_mom(1,qoi) - m1 * m1; // variance
    cm3 = raw_mom(2,qoi) - m1 * (3. * cm2 + m1 * m1);
    cm4 = raw_mom(3,qoi) - m1 * (4. * cm3 + m1 * (6. * cm2 + m1 * m1));
    // convert from centered to standardized moments
    Real stdev = std::sqrt(cm2);
    standard_mom(1,qoi) = stdev;                  // std deviation
    standard_mom(2,qoi) = cm3 / (cm2 * stdev);    // skewness
    standard_mom(3,qoi) = cm4 / (cm2 * cm2) - 3.; // excess kurtosis
  }
}


void NonDMultilevelSampling::post_run(std::ostream& s)
{
  // Statistics are generated here and output in print_results() below
  //if (statsFlag) // calculate statistics on allResponses
  //  compute_statistics(allSamples, allResponses);

  Analyzer::post_run(s);
}


void NonDMultilevelSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    size_t num_mf = NLev.size();
    if (num_mf == 1) s << "<<<<< Final samples per level:\n" << NLev[0];
    else {
      s << "<<<<< Final samples per model form:\n";
      for (size_t i=0; i<num_mf; ++i)
	s << "      Model Form " << i+1 << ":\n" << NLev[i];
    }
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";
  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
  }
}

} // namespace Dakota
