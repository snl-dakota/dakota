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
#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDMultilevelSampling.hpp"
#include "ProblemDescDB.hpp"
#include <boost/math/special_functions/fpclassify.hpp>

static const char rcsId[]="@(#) $Id: NonDMultilevelSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevelSampling::
NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  pilotSamples(probDescDB.get_sza("method.nond.pilot_samples")),
  finalCVRefinement(true),
  exportSampleSets(probDescDB.get_bool("method.nond.export_sample_sequence")),
  exportSamplesFormat(
    probDescDB.get_ushort("method.nond.export_samples_format"))
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
  // set initial response mode for set_communicators() (precedes core_run()).
  if (iteratedModel.surrogate_type() == "hierarchical")
    iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // set LF,HF
  else {
    Cerr << "Error: Multilevel Monte Carlo requires a hierarchical "
	 << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& ordered_models = iteratedModel.subordinate_models(false);
  size_t i, j, num_mf = ordered_models.size(), num_lev,
    pilot_size = pilotSamples.size();
  ModelLIter ml_iter;
  NLev.resize(num_mf);
  for (i=0, ml_iter=ordered_models.begin(); i<num_mf; ++i, ++ml_iter) {
    // for now, only SimulationModel supports solution_levels()
    num_lev = ml_iter->solution_levels();
    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer to pre_run()
  }

  switch (pilot_size) {
  case 0: maxEvalConcurrency *= 100;          break;
  case 1: maxEvalConcurrency *= pilotSamples[0]; break;
  default: {
    size_t max_ps = 0;
    for (i=0; i<pilot_size; ++i)
      if (pilotSamples[i] > max_ps)
	max_ps = pilotSamples[i];
    if (max_ps)
      maxEvalConcurrency *= max_ps;
    break;
  }
  }

  // For testing multilevel_mc_Qsum():
  // subIteratorFlag = true;
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
    size_t num_hf_lev = NLev.back().size();
    if (num_hf_lev > 1) { // ML performed on HF with CV using available LF
      // multiple model forms + multiple solutions levels --> perform MLMC on
      // HF model and bind 1:min(num_hf,num_lf) LF control variates starting
      // at coarsest level (TO DO: validate case of unequal levels)
      if (false) // original approach using 1 discrepancy correlation per level
	multilevel_control_variate_mc_Ycorr(model_form, model_form+1);
      else   // reformulated approach using 2 QoI correlations per level
	multilevel_control_variate_mc_Qcorr(model_form, model_form+1);
    }
    else { // multiple model forms (only) --> CVMC
      SizetSizetPair lf_form_level(model_form,   soln_level),
	             hf_form_level(model_form+1, soln_level);
      control_variate_mc(lf_form_level, hf_form_level);
    }
  }
  else { // multiple solutions levels (only) --> traditional ML-MC
    if (subIteratorFlag) multilevel_mc_Qsum(model_form); // includes error est
    else                 multilevel_mc_Ysum(model_form); // lighter weight
  }
}


/** This function performs "geometrical" MLMC on a single model form
    with multiple discretization levels. */
void NonDMultilevelSampling::multilevel_mc_Ysum(size_t model_form)
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
    qoi, iter = 0;
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., lev_cost;
  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost = truth_model.solution_level_cost(), agg_var(num_lev);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (sum_Y[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[i] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_Y; RealMatrix sum_YY(numFunctions, num_lev);
  initialize_ml_Ysums(sum_Y, num_lev);

  // Initialize for pilot sample
  Sizet2DArray& N_l = NLev[model_form];
  SizetArray delta_N_l; load_pilot_sample(delta_N_l);
  Cout << "\nMLMC pilot sample:\n" << delta_N_l << std::endl;
  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_l(num_lev, 0);

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

	// export separate output files for each data set.  surrogate_model()
	// has the correct model_form index for all levels.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.surrogate_model(), iter, lev);

	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);

	// process allResponses: accumulate new samples for each qoi and
	// update number of successful samples for each QoI
	accumulate_ml_Ysums(sum_Y, sum_YY, lev, N_l[lev]);
	if (outputLevel == DEBUG_OUTPUT) {
	  Cout << "Accumulated sums (Y1, Y2, Y3, Y4, Y1sq):\n";
	  write_data(Cout, sum_Y[1]); write_data(Cout, sum_Y[2]);
	  write_data(Cout, sum_Y[3]); write_data(Cout, sum_Y[4]);
	  write_data(Cout, sum_YY);   Cout << std::endl;
	}
	// update raw evaluation counts
	raw_N_l[lev] += numSamples;

	// compute estimator variance from current sample accumulation:
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "variance of Y[" << lev << "]: ";
	agg_var_l
	  = aggregate_variance_Ysum(sum_Y[1][lev], sum_YY[lev], N_l[lev]);
      }

      sum_sqrt_var_cost += std::sqrt(agg_var_l * lev_cost);
      // MSE reference is MC applied to HF:
      if (iter == 0)
	estimator_var0
	  += aggregate_mse_Ysum(sum_Y[1][lev], sum_YY[lev], N_l[lev]);
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
    Real fact = sum_sqrt_var_cost / eps_sq_div_2, N_target;
    for (lev=0; lev<num_lev; ++lev) {
      // Equation 3.9 in CTR Annual Research Briefs:
      // "A multifidelity control variate approach for the multilevel Monte 
      // Carlo technique," Geraci, Eldred, Iaccarino, 2015.
      N_target = std::sqrt(agg_var[lev] / lev_cost) * fact;
      delta_N_l[lev] = one_sided_delta(average(N_l[lev]), N_target);
    }
    ++iter;
    Cout << "\nMLMC iteration " << iter << " sample increments:\n" << delta_N_l
	 << std::endl;
  }

  // aggregate expected value of estimators for Y, Y^2, Y^3, Y^4. Final expected
  // value is sum of expected values from telescopic sum. There is no bias
  // correction for small sample sizes as in NonDSampling::compute_moments().
  RealMatrix Q_raw_mom(numFunctions, 4);
  RealMatrix &sum_Y1 = sum_Y[1], &sum_Y2 = sum_Y[2],
	     &sum_Y3 = sum_Y[3], &sum_Y4 = sum_Y[4];
  for (qoi=0; qoi<numFunctions; ++qoi) {
    for (lev=0; lev<num_lev; ++lev) {
      size_t Nlq = N_l[lev][qoi];
      Q_raw_mom(qoi,0) += sum_Y1(qoi,lev) / Nlq;
      Q_raw_mom(qoi,1) += sum_Y2(qoi,lev) / Nlq;
      Q_raw_mom(qoi,2) += sum_Y3(qoi,lev) / Nlq;
      Q_raw_mom(qoi,3) += sum_Y4(qoi,lev) / Nlq;
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Q_raw_mom, finalMomentStats);

  // compute the equivalent number of HF evaluations (includes any sim faults)
  equivHFEvals = raw_N_l[0] * cost[0]; // first level is single eval
  for (lev=1; lev<num_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_l[lev] * (cost[lev] + cost[lev-1]);
  equivHFEvals /= cost[num_lev-1]; // normalize into equivalent HF evals
}


/** This function performs "geometrical" MLMC on a single model form
    with multiple discretization levels. */
void NonDMultilevelSampling::multilevel_mc_Qsum(size_t model_form)
{
  iteratedModel.surrogate_model_indices(model_form);// soln lev not updated yet
  iteratedModel.truth_model_indices(model_form);    // soln lev not updated yet

  Model& truth_model  = iteratedModel.truth_model();
  size_t lev, num_lev = truth_model.solution_levels(), // single model form
    qoi, iter = 0;
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., lev_cost;
  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost = truth_model.solution_level_cost(), agg_var(num_lev);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[i] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_Ql, sum_Qlm1; IntIntPairRealMatrixMap sum_QlQlm1;
  initialize_ml_Qsums(sum_Ql, sum_Qlm1, sum_QlQlm1, num_lev);
  IntIntPair pr11(1,1);

  // Initialize for pilot sample
  Sizet2DArray& N_l = NLev[model_form];
  SizetArray delta_N_l; load_pilot_sample(delta_N_l);
  Cout << "\nMLMC pilot sample:\n" << delta_N_l << std::endl;
  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_l(num_lev, 0);

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

	// export separate output files for each data set.  surrogate_model()
	// has the correct model_form index for all levels.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.surrogate_model(), iter, lev);

	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);

	// process allResponses: accumulate new samples for each qoi and
	// update number of successful samples for each QoI
	accumulate_ml_Qsums(sum_Ql, sum_Qlm1, sum_QlQlm1, lev, N_l[lev]);
	if (outputLevel == DEBUG_OUTPUT) {
	  Cout << "Accumulated sums (Ql[1,2], Qlm1[1,2]):\n";
	  write_data(Cout, sum_Ql[1]);   write_data(Cout, sum_Ql[2]);
	  write_data(Cout, sum_Qlm1[1]); write_data(Cout, sum_Qlm1[2]);
	  Cout << std::endl;
	}
	// update raw evaluation counts
	raw_N_l[lev] += numSamples;

	// compute estimator variance from current sample accumulation:
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "variance of Y[" << lev << "]: ";
	agg_var_l = aggregate_variance_Qsum(sum_Ql[1][lev], sum_Qlm1[1][lev],
	  sum_Ql[2][lev], sum_QlQlm1[pr11][lev], sum_Qlm1[2][lev],N_l[lev],lev);
      }

      sum_sqrt_var_cost += std::sqrt(agg_var_l * lev_cost);
      // MSE reference is MC applied to HF:
      if (iter == 0)
	estimator_var0 += aggregate_mse_Qsum(sum_Ql[1][lev], sum_Qlm1[1][lev],
	  sum_Ql[2][lev], sum_QlQlm1[pr11][lev], sum_Qlm1[2][lev],N_l[lev],lev);
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
    Real fact = sum_sqrt_var_cost / eps_sq_div_2, N_target;
    for (lev=0; lev<num_lev; ++lev) {
      // Equation 3.9 in CTR Annual Research Briefs:
      // "A multifidelity control variate approach for the multilevel Monte 
      // Carlo technique," Geraci, Eldred, Iaccarino, 2015.
      N_target = std::sqrt(agg_var[lev] / lev_cost) * fact;
      delta_N_l[lev] = one_sided_delta(average(N_l[lev]), N_target);
    }
    ++iter;
    Cout << "\nMLMC iteration " << iter << " sample increments:\n" << delta_N_l
	 << std::endl;
  }

  // aggregate expected value of estimators for Y, Y^2, Y^3, Y^4. Final expected
  // value is sum of expected values from telescopic sum. There is no bias
  // correction for small sample sizes as in NonDSampling::compute_moments().
  RealMatrix Q_raw_mom(numFunctions, 4);
  RealMatrix &sum_Q1l   = sum_Ql[1],   &sum_Q2l   = sum_Ql[2],
             &sum_Q3l   = sum_Ql[3],   &sum_Q4l   = sum_Ql[4],
             &sum_Q1lm1 = sum_Qlm1[1], &sum_Q2lm1 = sum_Qlm1[2],
             &sum_Q3lm1 = sum_Qlm1[3], &sum_Q4lm1 = sum_Qlm1[4];
  for (qoi=0; qoi<numFunctions; ++qoi) {
    lev = 0;
    size_t Nlq = N_l[lev][qoi];
    Q_raw_mom(qoi,0) += sum_Q1l(qoi,lev) / Nlq;
    Q_raw_mom(qoi,1) += sum_Q2l(qoi,lev) / Nlq;
    Q_raw_mom(qoi,2) += sum_Q3l(qoi,lev) / Nlq;
    Q_raw_mom(qoi,3) += sum_Q4l(qoi,lev) / Nlq;
    for (lev=1; lev<num_lev; ++lev) {
      Nlq = N_l[lev][qoi];
      Q_raw_mom(qoi,0) += (sum_Q1l(qoi,lev) - sum_Q1lm1(qoi,lev)) / Nlq;
      Q_raw_mom(qoi,1) += (sum_Q2l(qoi,lev) - sum_Q2lm1(qoi,lev)) / Nlq;
      Q_raw_mom(qoi,2) += (sum_Q3l(qoi,lev) - sum_Q3lm1(qoi,lev)) / Nlq;
      Q_raw_mom(qoi,3) += (sum_Q4l(qoi,lev) - sum_Q4lm1(qoi,lev)) / Nlq;
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Q_raw_mom, finalMomentStats);

  // populate finalStatErrors
  compute_error_estimates(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l);

  // compute the equivalent number of HF evaluations (includes any sim faults)
  equivHFEvals = raw_N_l[0] * cost[0]; // first level is single eval
  for (lev=1; lev<num_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_l[lev] * (cost[lev] + cost[lev-1]);
  equivHFEvals /= cost[num_lev-1]; // normalize into equivalent HF evals
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
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
    cost_ratio = hf_cost / lf_cost, avg_eval_ratio, avg_mse_ratio;
  size_t iter = 0;

  IntRealVectorMap sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH;
  initialize_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL,sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false),
            rho2_LH(numFunctions, false);

  SizetArray delta_N_l; load_pilot_sample(delta_N_l);
  SizetArray& N_lf = NLev[lf_form_level.first][lf_form_level.second];
  SizetArray& N_hf = NLev[hf_form_level.first][hf_form_level.second];
  size_t raw_N_lf = 0, raw_N_hf = 0;

  // ---------------------
  // Compute Pilot Samples
  // ---------------------

  // Initialize for pilot sample (shared sample count discarding any excess)
  numSamples = std::min(delta_N_l[lf_form_level.first],
			delta_N_l[hf_form_level.first]);
  shared_increment(iter, 0);
  accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		     sum_HH, N_lf, N_hf);
  raw_N_lf += numSamples; raw_N_hf += numSamples;

  // Compute the LF/HF evaluation ratio, averaged over the QoI.
  // This includes updating var_H and rho2_LH.
  avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			      sum_HH, cost_ratio, N_hf, var_H, rho2_LH);
  // compute the ratio of MC and CVMC mean squared errors (controls convergence)
  avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, iter, N_hf);

  // -----------------------------------------------------------
  // Compute HF + LF increment targeting specified MSE reduction
  // -----------------------------------------------------------

  // bypass refinement if maxIterations == 0 or convergenceTol already
  // satisfied by pilot sample
  if (maxIterations && avg_mse_ratio > convergenceTol) {

    // Assuming rho_AB, evaluation_ratio and var_H to be relatively invariant,
    // we seek a relative reduction in MSE using the convergence tol spec:
    //   convTol = CV_mse / MC^0_mse = mse_ratio * N0 / N
    //   delta_N = mse_ratio*N0/convTol - N0 = (mse_ratio/convTol - 1) * N0
    Real incr = (avg_mse_ratio / convergenceTol - 1.) * numSamples;
    numSamples = (size_t)std::floor(incr + .5); // round

    if (numSamples) { // small incr did not round up
      shared_increment(++iter, 0);
      accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			 sum_HH, N_lf, N_hf);
      raw_N_lf += numSamples; raw_N_hf += numSamples;
      // update ratios:
      avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1],
				  sum_LH[1], sum_HH, cost_ratio, N_hf,
				  var_H, rho2_LH);
      avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, iter, N_hf);
    }
  }

  // --------------------------------------------------
  // Compute LF increment based on the evaluation ratio
  // --------------------------------------------------
  iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
  if (lf_increment(avg_eval_ratio, N_lf, N_hf, ++iter, 0)) { // level 0
    accumulate_cv_sums(sum_L_refined, N_lf);
    raw_N_lf += numSamples;
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf, sum_L_refined, N_lf,
		 rho2_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, finalMomentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf + (Real)raw_N_lf / cost_ratio;
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC si employed
    across two model forms to exploit correlation in the discrepancies
    at each level (Y_l). */
void NonDMultilevelSampling::
multilevel_control_variate_mc_Ycorr(size_t lf_model_form, size_t hf_model_form)
{
  iteratedModel.surrogate_model_indices(lf_model_form); // for init levs,cost
  iteratedModel.truth_model_indices(hf_model_form);     // for init levs,cost

  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();
  size_t qoi, iter = 0, lev, num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real avg_eval_ratio, eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0.,
    lf_lev_cost, hf_lev_cost;
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_cost(),
    lf_cost = surr_model.solution_level_cost(), agg_var_hf(num_hf_lev),
    avg_eval_ratios(num_cv_lev);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[1] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_L_refined, sum_L_shared, sum_H, sum_LL, sum_LH, sum_HH;
  initialize_mlcv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		       sum_HH, num_hf_lev, num_cv_lev);
  RealMatrix var_H(numFunctions, num_cv_lev, false),
           rho2_LH(numFunctions, num_cv_lev, false);
  RealVector Lambda(num_cv_lev, false), avg_rho2_LH(num_cv_lev, false);
  
  // Initialize for pilot sample
  Sizet2DArray&       N_lf =      NLev[lf_model_form];
  Sizet2DArray&       N_hf =      NLev[hf_model_form];
  Sizet2DArray  delta_N_l;   load_pilot_sample(delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_model_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_model_form]; 
  Cout << "\nMLMC pilot sample:\n" << delta_N_hf << std::endl;
  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_lf(num_cv_lev, 0), raw_N_hf(num_hf_lev, 0);

  // now converge on sample counts per level (N_hf)
  while (Pecos::l1_norm(delta_N_hf) && iter <= max_iter) {

    // set initial surrogate responseMode and model indices for lev 0
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // surr resp
    iteratedModel.surrogate_model_indices(hf_model_form, 0); // HF level 0

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_hf_lev; ++lev) {

      hf_lev_cost = hf_cost[lev];
      if (lev) {
	iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // both resp
	iteratedModel.surrogate_model_indices(hf_model_form, lev-1);// HF lev-1
	iteratedModel.truth_model_indices(hf_model_form,     lev);  // HF lev
	hf_lev_cost += hf_cost[lev-1]; // 2 levels
      }

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model

	// export separate output files for each data set.  Note that
	// surrogate_model() is indexed with hf_model_form at this stage for
	// all levels.  The exported discretization level (e.g., state variable
	// value) can't capture a level discrepancy for lev>0 and will reflect
	// the most recent evaluation state.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.surrogate_model(), iter, lev);

	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);

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
	  lf_lev_cost = lf_cost[lev]; 
	  if (lev) {
	    iteratedModel.surrogate_model_indices(lf_model_form, lev-1);
	    iteratedModel.truth_model_indices(lf_model_form,     lev);
	    lf_lev_cost += lf_cost[lev-1];
	  }
	  else
	    iteratedModel.surrogate_model_indices(lf_model_form, 0);
	  // compute allResp w/ LF model form reusing allVars from MLMC step
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // process previous and new set of allResponses for CV sums
	  accumulate_mlcv_Ysums(allResponses, hf_resp, sum_L_shared,
				sum_L_refined, sum_H, sum_LL, sum_LH,
				sum_HH, lev, N_lf[lev], N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (L_shared[1,2], L_refined[1,2], LH[1,2])"
		 << ":\n";
	    write_data(Cout,sum_L_shared[1]); write_data(Cout,sum_L_shared[2]);
	    write_data(Cout,sum_L_refined[1]);write_data(Cout,sum_L_refined[2]);
	    write_data(Cout,sum_LH[1]);       write_data(Cout,sum_LH[2]);
	  }
	  // update raw evaluation counts
	  raw_N_lf[lev] += numSamples; raw_N_hf[lev] += numSamples;

	  // compute the average evaluation ratio and Lambda factor
	  avg_eval_ratio = avg_eval_ratios[lev] =
	    eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
		       sum_HH[1], hf_lev_cost/lf_lev_cost, lev, N_hf[lev],
		       var_H, rho2_LH);
	  avg_rho2_LH[lev] = average(rho2_LH[lev], numFunctions);
	  Lambda[lev] = 1. - avg_rho2_LH[lev]
	              * (avg_eval_ratio - 1.) / avg_eval_ratio;
	  agg_var_hf_l = sum(var_H[lev], numFunctions);
	}
	else { // no LF model for this level; accumulate only multilevel sums
	  RealMatrix& sum_HH1 = sum_HH[1];
	  accumulate_ml_Ysums(sum_H, sum_HH1, lev, N_hf[lev]);//sum_Y for lev>0
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (H[1], H[2], HH):\n";
	    write_data(Cout, sum_H[1]); write_data(Cout, sum_H[2]);
	    write_data(Cout, sum_HH1);
	  }
	  raw_N_hf[lev] += numSamples;
	  // aggregate Y variances across QoI for this level
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "variance of Y[" << lev << "]: ";
	  agg_var_hf_l
	    = aggregate_variance_Ysum(sum_H[1][lev], sum_HH1[lev], N_hf[lev]);
	}
      }

      // accumulate sum of sqrt's of estimator var * cost used in N_target
      sum_sqrt_var_cost += (lev < num_cv_lev) ?
	std::sqrt(agg_var_hf_l * hf_lev_cost / (1. - avg_rho2_LH[lev]))
	  * Lambda[lev] : std::sqrt(agg_var_hf_l * hf_lev_cost);
      // MSE reference is MC applied to HF Y aggregated across qoi & levels
      if (iter == 0)
	estimator_var0 += (lev < num_cv_lev) ?
	  aggregate_mse_Yvar(var_H[lev], N_hf[lev]) :
	  aggregate_mse_Ysum(sum_H[1][lev], sum_HH[1][lev], N_hf[lev]);
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

    // All CV lf_increment() calls now follow all ML level evals:
    for (lev=0; lev<num_cv_lev; ++lev) {
      if (delta_N_hf[lev]) {
	if (lev) {
	  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
	  iteratedModel.surrogate_model_indices(lf_model_form, lev-1);
	  iteratedModel.truth_model_indices(lf_model_form,     lev);
	}
	else {
	  iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
	  iteratedModel.surrogate_model_indices(lf_model_form, 0);
	}
	// now execute additional LF sample increment, if needed
	if (lf_increment(avg_eval_ratios[lev], N_lf[lev], N_hf[lev],iter,lev)) {
	  accumulate_mlcv_Ysums(sum_L_refined, lev, N_lf[lev]);
	  raw_N_lf[lev] += numSamples;
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (L_refined[1,2]):\n";
	    write_data(Cout, sum_L_refined[1]);
	    write_data(Cout, sum_L_refined[2]);
	  }
	}
      }
    }

    // update targets based on variance estimates
    Real fact = sum_sqrt_var_cost / eps_sq_div_2, N_target;
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = (lev) ? hf_cost[lev] + hf_cost[lev-1] : hf_cost[lev];
      N_target = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = one_sided_delta(average(N_hf[lev]), N_target);
    }
    ++iter;
    Cout << "\nMLCVMC iteration " << iter << " sample increments:\n"
	 << delta_N_hf << std::endl;
  }

  // Iteration complete.  Now roll up raw moments from combining final
  // CVMC and MLMC estimators.
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
	size_t Nl = N_hf[lev][qoi];
	Y_mlmc_mom(qoi,0) += sum_H1(qoi,lev) / Nl;
	Y_mlmc_mom(qoi,1) += sum_H2(qoi,lev) / Nl;
	Y_mlmc_mom(qoi,2) += sum_H3(qoi,lev) / Nl;
	Y_mlmc_mom(qoi,3) += sum_H4(qoi,lev) / Nl;
      }
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Y_mlmc_mom, finalMomentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf[0] * hf_cost[0] + raw_N_lf[0] * lf_cost[0]; // 1st lev
  for (lev=1; lev<num_hf_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_hf[lev] * (hf_cost[lev] + hf_cost[lev-1]);
  for (lev=1; lev<num_cv_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_lf[lev] * (lf_cost[lev] + lf_cost[lev-1]);
  equivHFEvals /= hf_cost[num_hf_lev-1]; // normalize into equivalent HF evals
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC is employed
    across two model forms.  It generalizes the Y_l correlation case
    to separately target correlations for each QoI level embedded
    within the level discrepancies. */
void NonDMultilevelSampling::
multilevel_control_variate_mc_Qcorr(size_t lf_model_form, size_t hf_model_form)
{
  iteratedModel.surrogate_model_indices(lf_model_form); // for init levs,cost
  iteratedModel.truth_model_indices(hf_model_form);     // for init levs,cost

  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();
  size_t qoi, iter = 0, lev, num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  size_t max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  Real avg_eval_ratio, eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0.,
    lf_lev_cost, hf_lev_cost;
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_cost(),
    lf_cost = surr_model.solution_level_cost(), agg_var_hf(num_hf_lev),
    avg_eval_ratios(num_cv_lev);

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
  initialize_mlcv_sums(sum_Ll, sum_Llm1, sum_Ll_refined, sum_Llm1_refined,
		       sum_Hl, sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1,
		       sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll, sum_Hlm1_Llm1,
		       sum_Hl_Hl, sum_Hl_Hlm1, sum_Hlm1_Hlm1, num_hf_lev,
		       num_cv_lev);
  RealMatrix var_Yl(numFunctions, num_cv_lev, false),
             rho_dot2_LH(numFunctions, num_cv_lev, false);
  RealVector Lambda(num_cv_lev, false), avg_rho_dot2_LH(num_cv_lev, false);
  
  // Initialize for pilot sample
  Sizet2DArray&       N_lf =      NLev[lf_model_form];
  Sizet2DArray&       N_hf =      NLev[hf_model_form]; 
  Sizet2DArray  delta_N_l;   load_pilot_sample(delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_model_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_model_form]; 
  Cout << "\nMLMC pilot sample:\n" << delta_N_hf << std::endl;
  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_lf(num_cv_lev, 0), raw_N_hf(num_hf_lev, 0);

  // now converge on sample counts per level (N_hf)
  while (Pecos::l1_norm(delta_N_hf) && iter <= max_iter) {

    // set initial surrogate responseMode and model indices for lev 0
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // surr resp
    iteratedModel.surrogate_model_indices(hf_model_form, 0); // HF level 0

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_hf_lev; ++lev) {

      hf_lev_cost = hf_cost[lev];
      if (lev) {
	iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // both resp
	iteratedModel.surrogate_model_indices(hf_model_form, lev-1);// HF lev-1
	iteratedModel.truth_model_indices(hf_model_form,     lev);  // HF lev
	hf_lev_cost += hf_cost[lev-1]; // 2 levels
      }

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model

	// export separate output files for each data set.  Note that
	// surrogate_model() is indexed with hf_model_form at this stage for
	// all levels.  The exported discretization level (e.g., state variable
	// value) can't capture a level discrepancy for lev>0 and will reflect
	// the most recent evaluation state.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.surrogate_model(), iter, lev);

	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);

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
	  lf_lev_cost = lf_cost[lev];
	  if (lev) {
	    iteratedModel.surrogate_model_indices(lf_model_form, lev-1);
	    iteratedModel.truth_model_indices(lf_model_form,     lev);
	    lf_lev_cost += lf_cost[lev-1];
	  }
	  else
	    iteratedModel.surrogate_model_indices(lf_model_form, 0);
	  // eval allResp w/ LF model reusing allVars from ML step above
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // process previous and new set of allResponses for MLCV sums;
	  accumulate_mlcv_Qsums(allResponses, hf_resp, sum_Ll, sum_Llm1,
				sum_Ll_refined, sum_Llm1_refined, sum_Hl,
				sum_Hlm1, sum_Ll_Ll, sum_Ll_Llm1, sum_Llm1_Llm1,
				sum_Hl_Ll, sum_Hl_Llm1, sum_Hlm1_Ll,
				sum_Hlm1_Llm1, sum_Hl_Hl, sum_Hl_Hlm1,
				sum_Hlm1_Hlm1, lev, N_lf[lev], N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (Ll[1,2], L_refined[1,2], Hl[1,2]):\n";
	    write_data(Cout, sum_Ll[1]); write_data(Cout, sum_Ll[2]);
	    write_data(Cout, sum_Ll_refined[1]);
	    write_data(Cout, sum_Ll_refined[2]);
	    write_data(Cout, sum_Hl[1]); write_data(Cout, sum_Hl[2]);
	  }
	  // update raw evaluation counts
	  raw_N_lf[lev] += numSamples; raw_N_hf[lev] += numSamples;

	  // compute the average evaluation ratio and Lambda factor
	  avg_eval_ratio = avg_eval_ratios[lev] =
	    eval_ratio(sum_Ll[1], sum_Llm1[1], sum_Hl[1], sum_Hlm1[1],
		       sum_Ll_Ll[1], sum_Ll_Llm1[1], sum_Llm1_Llm1[1],
		       sum_Hl_Ll[1], sum_Hl_Llm1[1], sum_Hlm1_Ll[1],
		       sum_Hlm1_Llm1[1], sum_Hl_Hl[1], sum_Hl_Hlm1[1],
		       sum_Hlm1_Hlm1[1], hf_lev_cost/lf_lev_cost, lev,
		       N_hf[lev], var_Yl, rho_dot2_LH);
	  avg_rho_dot2_LH[lev] = average(rho_dot2_LH[lev], numFunctions);
	  Lambda[lev] = 1. - avg_rho_dot2_LH[lev]
	              * (avg_eval_ratio - 1.) / avg_eval_ratio;
	  agg_var_hf_l = sum(var_Yl[lev], numFunctions);
	}
	else { // no LF model for this level; accumulate only multilevel
	       // discrepancy sums (Hl is Yl) as in standard MLMC
	  RealMatrix& sum_HH1 = sum_Hl_Hl[1];
	  accumulate_ml_Ysums(sum_Hl, sum_HH1, lev, N_hf[lev]);//sum_Y for lev>0
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (H[1], H[2], HH[1]):\n";
	    write_data(Cout, sum_Hl[1]); write_data(Cout, sum_Hl[2]);
	    write_data(Cout, sum_HH1);
	  }
	  raw_N_hf[lev] += numSamples;
	  // aggregate Y variances across QoI for this level
	  if (outputLevel >= DEBUG_OUTPUT)
	    Cout << "variance of Y[" << lev << "]: ";
	  agg_var_hf_l
	    = aggregate_variance_Ysum(sum_Hl[1][lev], sum_HH1[lev], N_hf[lev]);
	}
      }

      // accumulate sum of sqrt's of estimator var * cost used in N_target
      sum_sqrt_var_cost += (lev < num_cv_lev) ?
	std::sqrt(agg_var_hf_l * hf_lev_cost / (1. - avg_rho_dot2_LH[lev]))
	  * Lambda[lev] : std::sqrt(agg_var_hf_l * hf_lev_cost);
      // MSE reference is MC applied to HF Y aggregated across qoi & levels
      if (iter == 0)
	estimator_var0 += (lev < num_cv_lev) ?
	  aggregate_mse_Yvar(var_Yl[lev], N_hf[lev]) :
	  aggregate_mse_Ysum(sum_Hl[1][lev], sum_Hl_Hl[1][lev], N_hf[lev]);
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

    // All CV lf_increment() calls now follow all ML level evals:
    // > Provides separation of pilot sample from refinements (simplifying
    //   offline execution with data importing w/o undesirable seed progression)
    // > Improves application of max_iterations control in general: user
    //   specification results in consistent count for ML and CV refinements
    // > Incurs a bit more overhead: avg_eval_ratios array, mode resetting
    // > Could potentially have parallel scheduling benefits by grouping
    //   similar Model eval sets for aggregated scheduling
    for (lev=0; lev<num_cv_lev; ++lev) {
      if (delta_N_hf[lev]) {
	if (lev) {
	  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
	  iteratedModel.surrogate_model_indices(lf_model_form, lev-1);
	  iteratedModel.truth_model_indices(lf_model_form,     lev);
	}
	else {
	  iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
	  iteratedModel.surrogate_model_indices(lf_model_form, 0);
	}
	// now execute additional LF sample increment, if needed
	if (lf_increment(avg_eval_ratios[lev], N_lf[lev], N_hf[lev],iter,lev)) {
	  accumulate_mlcv_Qsums(sum_Ll_refined, sum_Llm1_refined,lev,N_lf[lev]);
	  raw_N_lf[lev] += numSamples;
	  if (outputLevel == DEBUG_OUTPUT) {
	    Cout << "Accumulated sums (L_refined[1,2]):\n";
	    write_data(Cout, sum_Ll_refined[1]);
	    write_data(Cout, sum_Ll_refined[2]);
	  }
	}
      }
    }

    // update targets based on variance estimates
    Real fact = sum_sqrt_var_cost / eps_sq_div_2, N_target;
    for (lev=0; lev<num_hf_lev; ++lev) {
      hf_lev_cost = (lev) ? hf_cost[lev] + hf_cost[lev-1] : hf_cost[lev];
      N_target = (lev < num_cv_lev) ? fact *
	std::sqrt(agg_var_hf[lev] / hf_lev_cost * (1. - avg_rho_dot2_LH[lev])) :
	fact * std::sqrt(agg_var_hf[lev] / hf_lev_cost);
      delta_N_hf[lev] = one_sided_delta(average(N_hf[lev]), N_target);
    }
    ++iter;
    Cout << "\nMLCVMC iteration " << iter << " sample increments:\n"
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
	size_t Nl = N_hf[lev][qoi];
	Y_mlmc_mom(qoi,0) += sum_H1(qoi,lev) / Nl;
	Y_mlmc_mom(qoi,1) += sum_H2(qoi,lev) / Nl;
	Y_mlmc_mom(qoi,2) += sum_H3(qoi,lev) / Nl;
	Y_mlmc_mom(qoi,3) += sum_H4(qoi,lev) / Nl;
      }
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Y_mlmc_mom, finalMomentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf[0] * hf_cost[0] + raw_N_lf[0] * lf_cost[0]; // 1st lev
  for (lev=1; lev<num_hf_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_hf[lev] * (hf_cost[lev] + hf_cost[lev-1]);
  for (lev=1; lev<num_cv_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_lf[lev] * (lf_cost[lev] + lf_cost[lev-1]);
  equivHFEvals /= hf_cost[num_hf_lev-1]; // normalize into equivalent HF evals
}


void NonDMultilevelSampling::load_pilot_sample(SizetArray& delta_N_l)
{
  size_t num_mf = NLev.size(), pilot_size = pilotSamples.size(), delta_size;

  if (num_mf > 1) { // CV only case
    delta_size = num_mf;
    for (size_t i=0; i<num_mf; ++i)
      if (NLev[i].size() > 1) {
	Cerr << "Error: multidimensional case in load_pilot_sample(SizetArray)"
	     << std::endl;
	abort_handler(METHOD_ERROR);
      }
  }
  else // ML only case
    delta_size = NLev[0].size();

  if (delta_size == pilot_size)
    delta_N_l = pilotSamples;
  else if (pilot_size <= 1) {
    size_t num_samp = (pilot_size) ? pilotSamples[0] : 100;
    delta_N_l.assign(delta_size, num_samp);
  }
  else {
    Cerr << "Error: inconsistent pilot sample size (" << pilot_size
	 << ") in load_pilot_sample(SizetArray).  " << delta_size
	 << " expected." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


void NonDMultilevelSampling::load_pilot_sample(Sizet2DArray& delta_N_l)
{
  size_t i, num_samp, pilot_size = pilotSamples.size(), num_mf = NLev.size();
  delta_N_l.resize(num_mf);

  // allow several different pilot sample specifications
  if (pilot_size <= 1) {
    num_samp = (pilot_size) ? pilotSamples[0] : 100;
    for (i=0; i<num_mf; ++i)
      delta_N_l[i].assign(NLev[i].size(), num_samp);
  }
  else {
    size_t j, num_lev, num_prev_lev, num_total_lev = 0;
    bool same_lev = true;

    for (i=0; i<num_mf; ++i) {
      // for now, only SimulationModel supports solution_levels()
      num_lev = NLev[i].size();
      delta_N_l[i].resize(num_lev);
      if (i && num_lev != num_prev_lev) same_lev = false;
      num_total_lev += num_lev; num_prev_lev = num_lev;
    }

    if (same_lev && pilot_size == num_lev)
      for (j=0; j<num_lev; ++j) {
	num_samp = pilotSamples[j];
	for (i=0; i<num_mf; ++i)
	  delta_N_l[i][j] = num_samp;
      }
    else if (pilot_size == num_total_lev) {
      size_t cntr = 0;
      for (i=0; i<num_mf; ++i) {
	SizetArray& delta_N_li = delta_N_l[i]; num_lev = delta_N_li.size();
	for (j=0; j<num_lev; ++j, ++cntr)
	  delta_N_li[j] = pilotSamples[cntr];
      }
    }
    else {
      Cerr << "Error: inconsistent pilot sample size (" << pilot_size
	   << ") in load_pilot_sample(Sizet2DArray)." << std::endl;
      abort_handler(METHOD_ERROR);
    }
  }
}


void NonDMultilevelSampling::
initialize_ml_Ysums(IntRealMatrixMap& sum_Y, size_t num_lev)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_pr;
  for (int i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRMMIter, bool>:
    // use iterator to shape RealMatrix in place and init sums to 0
    sum_Y.insert(empty_pr).first->second.shape(numFunctions, num_lev);
  }
}

  
void NonDMultilevelSampling::
initialize_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		    IntIntPairRealMatrixMap& sum_QlQlm1, size_t num_lev)
{
  // sum_* are running sums across all increments
  std::pair<int, RealMatrix> empty_irm_pr; int i, j;
  for (i=1; i<=4; ++i) {
    empty_irm_pr.first = i;
    // std::map::insert() returns std::pair<IntRMMIter, bool>:
    // use iterator to shape RealMatrix in place and init sums to 0
    sum_Ql.insert(empty_irm_pr).first->second.shape(numFunctions, num_lev);
    sum_Qlm1.insert(empty_irm_pr).first->second.shape(numFunctions, num_lev);
  }
  std::pair<IntIntPair, RealMatrix> empty_iirm_pr;
  IntIntPair& ii = empty_iirm_pr.first;
  for (i=1; i<=2; ++i)
    for (j=1; j<=2; ++j) {
      ii.first = i; ii.second = j;
      // std::map::insert() returns std::pair<IIPRMMap::iterator, bool>
      sum_QlQlm1.insert(empty_iirm_pr).first->
	second.shape(numFunctions, num_lev);
    }
}

  
void NonDMultilevelSampling::
initialize_cv_sums(IntRealVectorMap& sum_L_shared,
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


void NonDMultilevelSampling::
initialize_mlcv_sums(IntRealMatrixMap& sum_L_shared,
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


void NonDMultilevelSampling::
initialize_mlcv_sums(IntRealMatrixMap& sum_Ll, IntRealMatrixMap& sum_Llm1,
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


void NonDMultilevelSampling::
accumulate_ml_Qsums(IntRealMatrixMap& sum_Q, size_t lev, SizetArray& num_Q)
{
  using boost::math::isfinite;
  Real q_l, q_l_prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRMMIter q_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      q_l_prod = q_l = fn_vals[qoi];

      if (isfinite(q_l)) { // neither NaN nor +/-Inf
	q_it = sum_Q.begin(); ord = q_it->first;
	active_ord = 1;
	while (q_it!=sum_Q.end()) {
    
	  if (ord == active_ord) {
	    q_it->second(qoi,lev) += q_l_prod; ++q_it;
	    ord = (q_it == sum_Q.end()) ? 0 : q_it->first;
	  }

	  q_l_prod *= q_l; ++active_ord;
	}
	++num_Q[qoi];
      }
    }
    //Cout << r_it->first << ": "; write_data(Cout, sum_Q[1]);
  }
}


void NonDMultilevelSampling::
accumulate_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		    IntIntPairRealMatrixMap& sum_QlQlm1, size_t lev,
		    SizetArray& num_Q)
{
  if (lev == 0)
    accumulate_ml_Qsums(sum_Ql, lev, num_Q);
  else {
    using boost::math::isfinite;
    Real q_l, q_lm1, q_l_prod, q_lm1_prod, qq_prod;
    int l1_ord, l2_ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter l1_it, l2_it; IntIntPair pr;

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODELS orders LF followed by HF
	q_l_prod   = q_l   = fn_vals[qoi+numFunctions];
	q_lm1_prod = q_lm1 = fn_vals[qoi];

	// sync sample counts for Ql and Qlm1
	if (isfinite(q_l) && isfinite(q_lm1)) { // neither NaN nor +/-Inf

	  // covariance terms: products of q_l and q_lm1
	  pr.first = pr.second = 1;
	  qq_prod = q_l_prod * q_lm1_prod;
	  sum_QlQlm1[pr](qoi,lev) += qq_prod;
	  pr.second = 2;
	  sum_QlQlm1[pr](qoi,lev) += qq_prod * q_lm1_prod;
	  pr.first = 2; pr.second = 1;
	  qq_prod *= q_l_prod;
	  sum_QlQlm1[pr](qoi,lev) += qq_prod;
	  pr.second = 2;
	  sum_QlQlm1[pr](qoi,lev) += qq_prod * q_lm1_prod;

	  // mean,variance terms: products of q_l or products of q_lm1
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
      //Cout << r_it->first << ": ";
      //write_data(Cout, sum_Ql[1]); write_data(Cout, sum_Qlm1[1]);
    }
  }
}


void NonDMultilevelSampling::
accumulate_ml_Ysums(IntRealMatrixMap& sum_Y, RealMatrix& sum_YY, size_t lev,
		    SizetArray& num_Y)
{
  using boost::math::isfinite;
  Real lf_fn, lf_prod;
  int y_ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRMMIter y_it;
  if (lev == 0) {
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();
      for (qoi=0; qoi<numFunctions; ++qoi) {

	lf_prod = lf_fn = fn_vals[qoi];
	if (isfinite(lf_fn)) { // neither NaN nor +/-Inf
	  // add to sum_YY: running sums across all sample increments
	  sum_YY(qoi,lev) += lf_prod * lf_prod;

	  // add to sum_Y: running sums across all sample increments
	  y_it = sum_Y.begin(); y_ord = y_it->first;
	  active_ord = 1;
	  while (y_it!=sum_Y.end() || active_ord <= 1) {
	    if (y_ord == active_ord) {
	      y_it->second(qoi,lev) += lf_prod; ++y_it;
	      y_ord = (y_it == sum_Y.end()) ? 0 : y_it->first;
	    }
	    lf_prod *= lf_fn; ++active_ord;
	  }
	  ++num_Y[qoi];
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
	if (isfinite(lf_fn) && isfinite(hf_fn)) { // neither NaN nor +/-Inf

	  // add to sum_YY: running sums across all sample increments
	  Real delta_prod = hf_prod - lf_prod;
	  sum_YY(qoi,lev) += delta_prod * delta_prod; // (HF^p-LF^p)^2 for p=1

	  // add to sum_Y: running sums across all sample increments
	  y_it = sum_Y.begin();  y_ord = y_it->first;
	  active_ord = 1;
	  while (y_it!=sum_Y.end() || active_ord <= 1) {
	    if (y_ord == active_ord) {
	      y_it->second(qoi,lev) += hf_prod - lf_prod; // HF^p-LF^p
	      ++y_it; y_ord = (y_it == sum_Y.end()) ? 0 : y_it->first;
	    }
	    hf_prod *= hf_fn; lf_prod *= lf_fn; ++active_ord;
	  }
	  ++num_Y[qoi];
	}
      }
    }
  }
}


void NonDMultilevelSampling::
accumulate_cv_sums(IntRealVectorMap& sum_L, SizetArray& num_L)
{
  // uses one set of allResponses in UNCORRECTED_SURROGATE mode
  // IntRealVectorMap is not a multilevel case --> no discrepancies

  using boost::math::isfinite;
  Real fn_val, prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRVMIter l_it; 
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      prod = fn_val = fn_vals[qoi];

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


void NonDMultilevelSampling::
accumulate_cv_sums(IntRealVectorMap& sum_L_shared,
		   IntRealVectorMap& sum_L_refined, IntRealVectorMap& sum_H,
		   IntRealVectorMap& sum_LL, IntRealVectorMap& sum_LH,
		   RealVector& sum_HH, SizetArray& num_L, SizetArray& num_H)
{
  // uses one set of allResponses in AGGREGATED_MODELS mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using boost::math::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter ls_it, lr_it, h_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord; size_t qoi;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      lf_prod = lf_fn = fn_vals[qoi];
      hf_prod = hf_fn = fn_vals[qoi+numFunctions];

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


void NonDMultilevelSampling::
accumulate_mlcv_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		      size_t lev, SizetArray& num_Q)
{
  if (lev == 0)
    accumulate_ml_Qsums(sum_Ql, lev, num_Q);
  else {
    using boost::math::isfinite;
    Real q_l, q_l_prod, q_lm1_prod, q_lm1;
    int l1_ord, l2_ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter l1_it, l2_it;

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODELS orders LF followed by HF
	q_l_prod   = q_l   = fn_vals[qoi+numFunctions];
	q_lm1_prod = q_lm1 = fn_vals[qoi];

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
      //Cout << r_it->first << ": ";
      //write_data(Cout, sum_Ql[1]); write_data(Cout, sum_Qlm1[1]);
    }
  }
}


void NonDMultilevelSampling::
accumulate_mlcv_Ysums(IntRealMatrixMap& sum_Y, size_t lev, SizetArray& num_Y)
{
  // uses one set of allResponses in UNCORRECTED_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap is a multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0)
    accumulate_ml_Qsums(sum_Y, lev, num_Y);
  else { // AGGREGATED_MODELS -> 2 sets of qoi per response map
    using boost::math::isfinite;
    Real fn_l, prod_l, fn_lm1, prod_lm1;
    int ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter y_it;

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	prod_l   = fn_l   = fn_vals[qoi+numFunctions];
	prod_lm1 = fn_lm1 = fn_vals[qoi];

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
      //Cout << r_it->first << ": "; write_data(Cout, sum_Y[1]);
    }
  }
}


void NonDMultilevelSampling::
accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL, IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_HH, size_t lev,
		      SizetArray& num_L, SizetArray& num_H)
{
  using boost::math::isfinite;
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


void NonDMultilevelSampling::
accumulate_mlcv_Ysums(const IntResponseMap& lf_resp_map,
		      const IntResponseMap& hf_resp_map,
		      IntRealMatrixMap& sum_L_shared,
		      IntRealMatrixMap& sum_L_refined, IntRealMatrixMap& sum_H,
		      IntRealMatrixMap& sum_LL,        IntRealMatrixMap& sum_LH,
		      IntRealMatrixMap& sum_HH, size_t lev,
		      SizetArray& num_L, SizetArray& num_H)
{
  // uses two sets of responses (LF & HF) in UNCORRECTED_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap are for multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0) // UNCORRECTED_SURROGATE -> 1 set of qoi per response map
    accumulate_mlcv_Qsums(lf_resp_map, hf_resp_map, sum_L_shared, sum_L_refined,
			  sum_H, sum_LL, sum_LH, sum_HH, lev, num_L, num_H);
  else { // AGGREGATED_MODELS -> 2 sets of qoi per response map
    using boost::math::isfinite;
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

	lf_l_prod   = lf_l   = lf_fn_vals[qoi+numFunctions];
	lf_lm1_prod = lf_lm1 = lf_fn_vals[qoi];
	hf_l_prod   = hf_l   = hf_fn_vals[qoi+numFunctions];
	hf_lm1_prod = hf_lm1 = hf_fn_vals[qoi];

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


void NonDMultilevelSampling::
accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
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
  // uses two sets of responses (LF & HF) in UNCORRECTED_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap are for multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0) // level lm1 not available; accumulate only level l
    accumulate_mlcv_Qsums(lf_resp_map, hf_resp_map, sum_Ll, sum_Ll_refined,
			  sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl, lev,
			  num_L, num_H);
  else {
    using boost::math::isfinite;
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

	lf_l_prod   = lf_l   = lf_fn_vals[qoi+numFunctions];
	lf_lm1_prod = lf_lm1 = lf_fn_vals[qoi];
	hf_l_prod   = hf_l   = hf_fn_vals[qoi+numFunctions];
	hf_lm1_prod = hf_lm1 = hf_fn_vals[qoi];

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


void NonDMultilevelSampling::shared_increment(size_t iter, size_t lev)
{
  if (iter == _NPOS)  Cout << "\nCVMC sample increments: ";
  else if (iter == 0) Cout << "\nCVMC pilot sample: ";
  else Cout << "\nCVMC iteration " << iter << " sample increments: ";
  Cout << "LF = " << numSamples << " HF = " << numSamples << '\n';

  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);

  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set:
  if (exportSampleSets) // for HF+LF models, use the HF tags
    export_all_samples("cv_", iteratedModel.truth_model(), iter, lev);

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


bool NonDMultilevelSampling::
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
      // mode for hierarchical surrogate model can be uncorrected surrogate
      // for CV MC, or uncorrected surrogate/aggregated models for ML-CV MC
      // --> set at calling level
      //iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);

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


Real NonDMultilevelSampling::
eval_ratio(const RealVector& sum_L_shared, const RealVector& sum_H,
	   const RealVector& sum_LL, const RealVector& sum_LH,
	   const RealVector& sum_HH, Real cost_ratio,
	   const SizetArray& N_shared, RealVector& var_H, RealVector& rho2_LH)
{
  Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH[qoi];
    compute_control(sum_L_shared[qoi], sum_H[qoi], sum_LL[qoi], sum_LH[qoi],
		    sum_HH[qoi], N_shared[qoi], var_H[qoi], rho_sq);

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


Real NonDMultilevelSampling::
eval_ratio(RealMatrix& sum_L_shared, RealMatrix& sum_H, RealMatrix& sum_LL,
	   RealMatrix& sum_LH, RealMatrix& sum_HH, Real cost_ratio, size_t lev,
	   const SizetArray& N_shared, RealMatrix& var_H, RealMatrix& rho2_LH)
{
  Real eval_ratio, avg_eval_ratio = 0.; size_t num_avg = 0;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {

    Real& rho_sq = rho2_LH(qoi,lev);
    compute_control(sum_L_shared(qoi,lev), sum_H(qoi,lev), sum_LL(qoi,lev),
		    sum_LH(qoi,lev), sum_HH(qoi,lev), N_shared[qoi],
		    var_H(qoi,lev), rho_sq);

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
  if (outputLevel >= DEBUG_OUTPUT) {
    Cout << "variance of HF Q[" << lev << "]:\n";
    write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_H);
  }

  if (num_avg) avg_eval_ratio /= num_avg;
  else // should not happen, but provide a reasonable upper bound
    avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);

  return avg_eval_ratio;
}


Real NonDMultilevelSampling::
eval_ratio(RealMatrix& sum_Ll,   RealMatrix& sum_Llm1,  RealMatrix& sum_Hl,
	   RealMatrix& sum_Hlm1, RealMatrix& sum_Ll_Ll, RealMatrix& sum_Ll_Llm1,
	   RealMatrix& sum_Llm1_Llm1, RealMatrix& sum_Hl_Ll,
	   RealMatrix& sum_Hl_Llm1,   RealMatrix& sum_Hlm1_Ll,
	   RealMatrix& sum_Hlm1_Llm1, RealMatrix& sum_Hl_Hl,
	   RealMatrix& sum_Hl_Hlm1,   RealMatrix& sum_Hlm1_Hlm1,
	   Real cost_ratio, size_t lev, const SizetArray& N_shared,
	   RealMatrix& var_YHl,       RealMatrix& rho_dot2_LH)
{
  if (lev == 0)
    return eval_ratio(sum_Ll, sum_Hl, sum_Ll_Ll, sum_Hl_Ll, sum_Hl_Hl,
		      cost_ratio, lev, N_shared, var_YHl, rho_dot2_LH);
  else {
    Real beta_dot, gamma, eval_ratio, avg_eval_ratio = 0.;
    size_t qoi, num_avg = 0;
    for (qoi=0; qoi<numFunctions; ++qoi) {
      Real& rho_dot_sq = rho_dot2_LH(qoi,lev);
      compute_control(sum_Ll(qoi,lev), sum_Llm1(qoi,lev), sum_Hl(qoi,lev),
		      sum_Hlm1(qoi,lev), sum_Ll_Ll(qoi,lev),
		      sum_Ll_Llm1(qoi,lev), sum_Llm1_Llm1(qoi,lev),
		      sum_Hl_Ll(qoi,lev), sum_Hl_Llm1(qoi,lev),
		      sum_Hlm1_Ll(qoi,lev), sum_Hlm1_Llm1(qoi,lev),
		      sum_Hl_Hl(qoi,lev), sum_Hl_Hlm1(qoi,lev),
		      sum_Hlm1_Hlm1(qoi,lev), N_shared[qoi], var_YHl(qoi,lev),
		      rho_dot_sq, beta_dot, gamma);

      if (rho_dot_sq < 1.) { // protect against division by 0
	eval_ratio = std::sqrt(cost_ratio * rho_dot_sq / (1.-rho_dot_sq));
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "eval_ratio() QoI " << qoi+1 << ": cost_ratio = "
	       << cost_ratio << " rho_dot_sq = " << rho_dot_sq
	       << " eval_ratio = " << eval_ratio << std::endl;
	avg_eval_ratio += eval_ratio;
	++num_avg;
      }
    }
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "variance of HF Y[" << lev << "]:\n";
      write_col_vector_trans(Cout, (int)lev, (int)numFunctions, var_YHl);
    }

    if (num_avg) avg_eval_ratio /= num_avg;
    else // should not happen, but provide a reasonable upper bound
      avg_eval_ratio = (Real)maxFunctionEvals / average(N_shared);

    return avg_eval_ratio;
  }
}


Real NonDMultilevelSampling::
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


void NonDMultilevelSampling::
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
    compute_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i], N_shared,
		    beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		  N_refined, beta, H_rm_col);
  }
}


void NonDMultilevelSampling::
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
    compute_control(sum_L_shared[i], sum_H[i], sum_LL[i], sum_LH[i], N_shared,
		    lev, beta);
    Cout << "Moment " << i << ":\n";
    RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
    apply_control(sum_H[i], sum_L_shared[i], N_shared, sum_L_refined[i],
		  N_refined, lev, beta, H_rm_col);
  }
  Cout << '\n'; // for loop over levels
}


void NonDMultilevelSampling::
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
      compute_control(sum_Ll[i], sum_Llm1[i], sum_Hl[i], sum_Hlm1[i],
		      sum_Ll_Ll[i], sum_Ll_Llm1[i], sum_Llm1_Llm1[i],
		      sum_Hl_Ll[i], sum_Hl_Llm1[i], sum_Hlm1_Ll[i],
		      sum_Hlm1_Llm1[i], sum_Hl_Hl[i], sum_Hl_Hlm1[i],
		      sum_Hlm1_Hlm1[i], N_shared, lev, beta_dot, gamma);
      Cout << "Moment " << i << ":\n";
      RealVector H_rm_col(Teuchos::View, H_raw_mom[i-1], numFunctions);
      apply_control(sum_Hl[i], sum_Hlm1[i], sum_Ll[i], sum_Llm1[i], N_shared,
		    sum_Ll_refined[i], sum_Llm1_refined[i], N_refined, lev,
		    beta_dot, gamma, H_rm_col);
    }
    Cout << '\n'; // for loop over levels
  }
}


void NonDMultilevelSampling::
compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH,
		size_t N_shared, Real& beta)
{
  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) (sum[X^2_i] - N X-bar^2) where bias correction = 1/(N-1)
  //Real var_L = (sum_LL - N_shared * mu_L * mu_L) * bias_corr,
  //    cov_LH = (sum_LH - N_shared * mu_L * mu_H) * bias_corr;
  beta = (sum_LH - N_shared * mu_L * mu_H) / (sum_LL - N_shared * mu_L * mu_L);
}


void NonDMultilevelSampling::
compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH, Real sum_HH,
		size_t N_shared, Real& var_H, Real& rho2_LH)
{
  Real bias_corr = 1./(N_shared - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) (sum[X^2_i] - N X-bar^2) where bias correction = 1/(N-1)
  Real var_L = (sum_LL - N_shared * mu_L * mu_L) * bias_corr,
      cov_LH = (sum_LH - N_shared * mu_L * mu_H) * bias_corr;
  var_H      = (sum_HH - N_shared * mu_H * mu_H) * bias_corr;

  //beta  = cov_LH / var_L;
  rho2_LH = cov_LH / var_L * cov_LH / var_H;
}


void NonDMultilevelSampling::
compute_control(Real sum_Ll, Real sum_Llm1, Real sum_Hl, Real sum_Hlm1,
		Real sum_Ll_Ll, Real sum_Ll_Llm1, Real sum_Llm1_Llm1,
		Real sum_Hl_Ll, Real sum_Hl_Llm1, Real sum_Hlm1_Ll,
		Real sum_Hlm1_Llm1, Real sum_Hl_Hl, Real sum_Hl_Hlm1,
		Real sum_Hlm1_Hlm1, size_t N_shared, Real& var_YH,
		Real& rho_dot2_LH, Real& beta_dot, Real& gamma)
{
  Real bias_corr = 1./(N_shared - 1);

  // means, variances, covariances for Q
  // Note: sum_*[i][lm1] is not the same as sum_*lm1[i][lev] due to
  //       discrepancy evaluations with different sample sets!
  Real mu_Ll   = sum_Ll   / N_shared,  mu_Llm1 = sum_Llm1 / N_shared;
  Real mu_Hl   = sum_Hl   / N_shared,  mu_Hlm1 = sum_Hlm1 / N_shared;

  Real var_Ll   = (sum_Ll_Ll - N_shared * mu_Ll * mu_Ll) * bias_corr;
  Real var_Llm1 = (sum_Llm1_Llm1 - N_shared * mu_Llm1 * mu_Llm1) * bias_corr;
  Real var_Hl   = (sum_Hl_Hl - N_shared * mu_Hl * mu_Hl) * bias_corr;
  Real var_Hlm1 = (sum_Hlm1_Hlm1 - N_shared * mu_Hlm1 * mu_Hlm1) * bias_corr;

  Real cov_Hl_Ll = (sum_Hl_Ll - N_shared * mu_Hl * mu_Ll) * bias_corr;
  Real cov_Hl_Llm1 = (sum_Hl_Llm1 - N_shared * mu_Hl * mu_Llm1) * bias_corr;
  Real cov_Hlm1_Ll = (sum_Hlm1_Ll - N_shared * mu_Hlm1 * mu_Ll) * bias_corr;
  Real cov_Hlm1_Llm1
    = (sum_Hlm1_Llm1 - N_shared * mu_Hlm1 * mu_Llm1) * bias_corr;

  Real cov_Ll_Llm1 = (sum_Ll_Llm1 - N_shared * mu_Ll * mu_Llm1) * bias_corr;
  Real cov_Hl_Hlm1 = (sum_Hl_Hlm1 - N_shared * mu_Hl * mu_Hlm1) * bias_corr;

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
    Cout << "compute_control(): var reduce ratio = " << ratio << " rho2_LH = "
	 << rho2_LH << " rho_dot2_LH = " << rho_dot2_LH << std::endl;
}


void NonDMultilevelSampling::
apply_control(Real sum_H, Real sum_L_shared, size_t N_shared,
	      Real sum_L_refined, size_t N_refined, Real beta, Real& H_raw_mom)
{
  // apply control for HF uncentered raw moment estimates:
  H_raw_mom = sum_H / N_shared                    // mu_H from shared samples
            - beta * (sum_L_shared  / N_shared -  // mu_L from shared samples
		      sum_L_refined / N_refined); // refined_mu_L incl increment
}


void NonDMultilevelSampling::
apply_control(Real sum_Hl, Real sum_Hlm1, Real sum_Ll, Real sum_Llm1,
	      size_t N_shared,  Real sum_Ll_refined, Real sum_Llm1_refined,
	      size_t N_refined, Real beta_dot, Real gamma, Real& H_raw_mom)
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


void NonDMultilevelSampling::
convert_moments(const RealMatrix& raw_mom, RealMatrix& final_stat_mom)
{
  // Note: raw_mom is numFunctions x 4 and final_stat_mom is the transpose
  if (final_stat_mom.empty())
    final_stat_mom.shapeUninitialized(4, numFunctions);

  // Convert uncentered raw moment estimates to central moments
  if (finalMomentsType == CENTRAL_MOMENTS) {
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      uncentered_to_centered(raw_mom(qoi,0), raw_mom(qoi,1), raw_mom(qoi,2),
			     raw_mom(qoi,3), final_stat_mom(0,qoi),
			     final_stat_mom(1,qoi), final_stat_mom(2,qoi),
			     final_stat_mom(3,qoi));
  }
  // Convert uncentered raw moment estimates to standardized moments
  else { //if (finalMomentsType == STANDARD_MOMENTS) {
    Real cm1, cm2, cm3, cm4;
    for (size_t qoi=0; qoi<numFunctions; ++qoi) {
      uncentered_to_centered(raw_mom(qoi,0), raw_mom(qoi,1), raw_mom(qoi,2),
			     raw_mom(qoi,3), cm1, cm2, cm3, cm4);
      centered_to_standard(cm1, cm2, cm3, cm4, final_stat_mom(0,qoi),
			   final_stat_mom(1,qoi), final_stat_mom(2,qoi),
			   final_stat_mom(3,qoi));
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      Cout <<  "raw mom 1 = "   << raw_mom(qoi,0)
	   << " final mom 1 = " << final_stat_mom(0,qoi) << '\n'
	   <<  "raw mom 2 = "   << raw_mom(qoi,1)
	   << " final mom 2 = " << final_stat_mom(1,qoi) << '\n'
	   <<  "raw mom 3 = "   << raw_mom(qoi,2)
	   << " final mom 3 = " << final_stat_mom(2,qoi) << '\n'
	   <<  "raw mom 4 = "   << raw_mom(qoi,3)
	   << " final mom 4 = " << final_stat_mom(3,qoi) << "\n\n";
}


void NonDMultilevelSampling::
compute_error_estimates(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
			IntIntPairRealMatrixMap& sum_QlQlm1,
			Sizet2DArray& num_Q)
{
  if (!finalMomentsType)
    return;

  if (finalStatErrors.empty())
    finalStatErrors.size(finalStatistics.num_functions()); // init to 0.

  Real agg_estim_var, var_Yl, cm1l, cm2l, cm3l, cm4l, cm1lm1, cm2lm1,
    cm3lm1, cm4lm1, cm1l_sq, cm1lm1_sq, cm2l_sq, cm2lm1_sq, var_Ql, var_Qlm1,
    mu_Q2l, mu_Q2lm1, mu_Q1lQ1lm1, mu_Q2lQ1lm1, mu_Q1lQ2lm1, mu_Q2lQ2lm1,
    mu_P2lP2lm1, var_P2l, var_P2lm1, covar_P2lP2lm1;
  size_t lev, qoi, cntr = 0, Nlq,
    num_lev = iteratedModel.truth_model().solution_levels();
  IntIntPair pr11(1,1), pr12(1,2), pr21(2,1), pr22(2,2);
  RealMatrix &sum_Q1l = sum_Ql[1],           &sum_Q1lm1 = sum_Qlm1[1],
             &sum_Q2l = sum_Ql[2],           &sum_Q2lm1 = sum_Qlm1[2],
             &sum_Q3l = sum_Ql[3],           &sum_Q3lm1 = sum_Qlm1[3],
             &sum_Q4l = sum_Ql[4],           &sum_Q4lm1 = sum_Qlm1[4],
        &sum_Q1lQ1lm1 = sum_QlQlm1[pr11], &sum_Q1lQ2lm1 = sum_QlQlm1[pr12],
        &sum_Q2lQ1lm1 = sum_QlQlm1[pr21], &sum_Q2lQ2lm1 = sum_QlQlm1[pr22];
  for (qoi=0; qoi<numFunctions; ++qoi) {

    // std error in mean estimate
    lev = 0; Nlq = num_Q[lev][qoi];
    cm1l   =  sum_Q1l(qoi,lev) / Nlq;
    var_Yl = (sum_Q2l(qoi,lev) - Nlq * cm1l * cm1l) / (Nlq - 1); // var_Ql
    agg_estim_var = var_Yl / Nlq;
    for (lev=1; lev<num_lev; ++lev) {
      Nlq  = num_Q[lev][qoi];
      cm1l = sum_Q1l(qoi,lev) / Nlq; cm1lm1 = sum_Q1lm1(qoi,lev) / Nlq;
      //var_Yl = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
      var_Yl = ( sum_Q2l(qoi,lev)              - Nlq * cm1l   * cm1l
		 - 2.* ( sum_Q1lQ1lm1(qoi,lev) - Nlq * cm1l   * cm1lm1 ) +
		 sum_Q2lm1(qoi,lev)            - Nlq * cm1lm1 * cm1lm1 )
	     / ( Nlq - 1 ); // bias corr
      agg_estim_var += var_Yl / Nlq;
    }
    finalStatErrors[cntr++] = std::sqrt(agg_estim_var); // std error
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Estimator variance for mean = " << agg_estim_var;

    // std err in std dev estimate (*** TO DO ***)
    if (finalMomentsType == STANDARD_MOMENTS) {
      Cerr << "Warning: std error not currently supported for standardized "
	   << "final moments.\n         Setting estimate to zero." << std::endl;
      finalStatErrors[cntr++] = 0.;
      continue;
    }

    // std error in variance estimate
    lev = 0; Nlq = num_Q[lev][qoi];
    uncentered_to_centered(sum_Q1l(qoi,lev) / Nlq, sum_Q2l(qoi,lev) / Nlq,
			   sum_Q3l(qoi,lev) / Nlq, sum_Q4l(qoi,lev) / Nlq,
			   cm1l, cm2l, cm3l, cm4l);
    cm2l_sq = cm2l * cm2l;
    var_P2l = cm4l - cm2l_sq + 2./(Nlq - 1.) * cm2l_sq;
    agg_estim_var = var_P2l / Nlq;
    for (lev=1; lev<num_lev; ++lev) {
      Nlq = num_Q[lev][qoi];
      mu_Q2l = sum_Q2l(qoi,lev) / Nlq;   mu_Q2lm1 = sum_Q2lm1(qoi,lev) / Nlq;
      uncentered_to_centered(sum_Q1l(qoi,lev) / Nlq, mu_Q2l,
			     sum_Q3l(qoi,lev) / Nlq, sum_Q4l(qoi,lev) / Nlq,
			     cm1l, cm2l, cm3l, cm4l);
      uncentered_to_centered(sum_Q1lm1(qoi,lev) / Nlq, mu_Q2lm1,
			     sum_Q3lm1(qoi,lev) / Nlq, sum_Q4lm1(qoi,lev) / Nlq,
			     cm1lm1, cm2lm1, cm3lm1, cm4lm1);
      cm1l_sq = cm1l * cm1l; cm1lm1_sq = cm1lm1 * cm1lm1;
      cm2l_sq = cm2l * cm2l; cm2lm1_sq = cm2lm1 * cm2lm1;
      var_Ql   = ( sum_Q2l(qoi,lev)   - Nlq * cm1l   * cm1l)    / ( Nlq - 1 );
      var_Qlm1 = ( sum_Q2lm1(qoi,lev) - Nlq * cm1lm1 * cm1lm1 ) / ( Nlq - 1 );
      mu_Q1lQ1lm1 = sum_Q1lQ1lm1(qoi,lev) / Nlq;
      mu_Q1lQ2lm1 = sum_Q1lQ2lm1(qoi,lev) / Nlq;
      mu_Q2lQ1lm1 = sum_Q2lQ1lm1(qoi,lev) / Nlq;
      mu_Q2lQ2lm1 = sum_Q2lQ2lm1(qoi,lev) / Nlq;
      mu_P2lP2lm1 = mu_Q2lQ2lm1 - 2. * cm1lm1 * mu_Q2lQ1lm1
	+ cm1lm1_sq * mu_Q2l + cm1l_sq * mu_Q2lm1
	- 2. * cm1l * mu_Q1lQ2lm1 + 4. * cm1l * cm1lm1 * mu_Q1lQ1lm1
	- 3. * cm1l_sq * cm1lm1_sq;
      var_P2l        = cm4l   - cm2l_sq   + 2./(Nlq - 1.) * cm2l_sq;
      var_P2lm1      = cm4lm1 - cm2lm1_sq + 2./(Nlq - 1.) * cm2lm1_sq;
      // [gg] modified to cope with negative variance      
      covar_P2lP2lm1 = ( mu_P2lP2lm1 - var_Ql * var_Qlm1 +
		         ( mu_Q1lQ1lm1 - cm1l * cm1lm1 )
		         *( mu_Q1lQ1lm1 - cm1l * cm1lm1 ) / (Nlq - 1.) ); 
      agg_estim_var += (var_P2l + var_P2lm1 - 2. * covar_P2lP2lm1) / Nlq;
    }
    finalStatErrors[cntr++] = std::sqrt(agg_estim_var); // std error
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " and for variance = " << agg_estim_var << "\n\n";

    // level mapping errors not implemented at this time
    cntr +=
      requestedRespLevels[qoi].length() +   requestedProbLevels[qoi].length() +
      requestedRelLevels[qoi].length()  + requestedGenRelLevels[qoi].length();
  }
}


void NonDMultilevelSampling::
export_all_samples(String root_prepend, const Model& model, size_t iter,
		   size_t lev)
{
  String tabular_filename(root_prepend);
  const String& iface_id = model.interface_id();
  size_t i, num_samp = allSamples.numCols();
  if (iface_id.empty()) tabular_filename += "NO_ID_i";
  else                  tabular_filename += iface_id + "_i";
  tabular_filename += boost::lexical_cast<std::string>(iter)     +  "_l"
                   +  boost::lexical_cast<std::string>(lev)      +  '_'
                   +  boost::lexical_cast<std::string>(num_samp) + ".dat";
  Variables vars(model.current_variables().copy());

  String context_message("NonDMultilevelSampling::export_all_samples");
  StringArray no_resp_labels; String cntr_label("sample_id");

  // Rather than hard override, rely on output_precision user spec
  //int save_wp = write_precision;
  //write_precision = 16; // override
  std::ofstream tabular_stream;
  TabularIO::open_file(tabular_stream, tabular_filename, context_message);
  TabularIO::write_header_tabular(tabular_stream, vars, no_resp_labels,
				  cntr_label, exportSamplesFormat);
  for (i=0; i<num_samp; ++i) {
    sample_to_variables(allSamples[i], vars); // NonDSampling version
    TabularIO::write_data_tabular(tabular_stream, vars, iface_id, i+1,
				  exportSamplesFormat);
  }

  TabularIO::close_file(tabular_stream, tabular_filename, context_message);
  //write_precision = save_wp; // restore
}


void NonDMultilevelSampling::post_run(std::ostream& s)
{
  // Final moments are generated within core_run() by convert_moments().
  // No addtional stats are currently supported.
  //if (statsFlag) // calculate statistics on allResponses
  //  compute_statistics(allSamples, allResponses);

  // NonD::update_aleatory_final_statistics() pushes finalMomentStats
  // into finalStatistics
  update_final_statistics();

  Analyzer::post_run(s);
}


void NonDMultilevelSampling::print_results(std::ostream& s)
{
  if (statsFlag) {
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";

  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
  }
}

} // namespace Dakota
