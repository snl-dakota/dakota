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
  NonDSampling(problem_db, model),
  pilotSamples(probDescDB.get_sza("method.nond.pilot_samples"))
{
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

  /*
  // Check for model forms and solution levels
  if (iteratedModel.subordinate_models(false).size() > 1) {
    hierarchMode = true;
  }
  if (sub_model.model_type() == "simulation" &&
      sub_model.solution_levels() > 1) {
    solnCntlMode = true;
    // TO DO
  }
  */

  if (pilotSamples.empty()) maxEvalConcurrency *= 100;
  else {
    size_t i, num_ps = pilotSamples.size(), max_ps = 0;
    for (i=0; i<num_ps; ++i)
      if (pilotSamples[i] > max_ps)
	max_ps = pilotSamples[i];
    if (max_ps)
      maxEvalConcurrency *= max_ps;
  }
}


NonDMultilevelSampling::~NonDMultilevelSampling()
{ }


void NonDMultilevelSampling::resize()
{
  NonDSampling::resize();

  Cerr << "\nError: Resizing is not yet supported in method "
       << method_enum_to_string(methodName) << "." << std::endl;
  abort_handler(METHOD_ERROR);
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

  size_t model_form = 0, soln_level = 0,
    num_mf = iteratedModel.subordinate_models(false).size();
  // TO DO: hierarchy incl peers (not peers each optionally incl hierarchy)
  //   num_mf     = iteratedModel.model_hierarchy_depth();
  //   num_peer_i = iteratedModel.model_peer_breadth(i);

  if (num_mf > 1) {
    if (iteratedModel.surrogate_model().solution_levels() > 1) {
      // multiple model forms and multiple solutions levels --> ML-MF-MC
    }
    else // multiple model forms (only) --> control variate MC
      control_variate_mc(model_form, model_form+1, soln_level);
  }
  else // multiple solutions levels (only) --> traditional ML-MC
    multilevel_mc(model_form);
}


/** This function performs control variate MC on multiple model forms
    using a single discretization level. */
void NonDMultilevelSampling::
control_variate_mc(size_t lf_model_form, size_t hf_model_form,
		   size_t soln_level)
{
  iteratedModel.surrogate_model(lf_model_form, soln_level);
  iteratedModel.truth_model(hf_model_form, soln_level);
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  // retrieve cost estimates across model forms for a particular soln level
  Real lf_cost    =  surr_model.solution_level_cost()[soln_level],
       hf_cost    = truth_model.solution_level_cost()[soln_level],
       cost_ratio = hf_cost / lf_cost;
  size_t qoi, iter = 0, N_lf, N_hf, delta_N_lf, delta_N_hf, total_N;
  
  // ---------------------
  // Compute Pilot Samples
  // ---------------------

  // Initialize for pilot sample
  if      (pilotSamples.empty())     delta_N_lf = delta_N_hf = 100; // default
  else if (pilotSamples.size() == 1) delta_N_lf = delta_N_hf = pilotSamples[0];
  else if (pilotSamples.size() == 2)
    { delta_N_lf = pilotSamples[0]; delta_N_hf = pilotSamples[1]; }
  else {
    Cerr << "Error: bad pilot samples input for Control Variate MC."<<std::endl;
    abort_handler(METHOD_ERROR);
  }
  Cout << "\nCVMC pilot sample: LF = " << delta_N_lf << " HF = " << delta_N_hf
       << std::endl;

  // set the number of current samples from the defined increment
  numSamples = std::min(delta_N_lf, delta_N_hf); // ignore any excess
  total_N = N_lf = N_hf = numSamples; // update LF/HF samples
  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);

  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model
  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);

  // process allResponses: accumulate new samples for each qoi
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  RealVector sum_L(numFunctions), sum_H(numFunctions), sum_LH(numFunctions),
    sum_L2(numFunctions), sum_H2(numFunctions), sum_L2H2(numFunctions),
    sum_L3(numFunctions), sum_H3(numFunctions), sum_L3H3(numFunctions),
    sum_L4(numFunctions), sum_H4(numFunctions), sum_L4H4(numFunctions),
    sum_L6(numFunctions), sum_L8(numFunctions);
  IntRespMCIter r_it;
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // sum_* are running sums across all increments
      lf_prod = lf_fn = fn_vals[qoi];
      hf_prod = hf_fn = fn_vals[qoi+numFunctions];
      sum_L[qoi] += lf_fn; sum_H[qoi] += hf_fn; sum_LH[qoi] += lf_fn * hf_fn;

      lf_prod *= lf_fn; hf_prod *= hf_fn;
      sum_L2[qoi] += lf_prod; sum_H2[qoi] += hf_prod;
      sum_L2H2[qoi] += lf_prod * hf_prod;

      lf_prod *= lf_fn; hf_prod *= hf_fn;
      sum_L3[qoi] += lf_prod; sum_H3[qoi] += hf_prod; 
      sum_L3H3[qoi] += lf_prod * hf_prod;

      lf_prod *= lf_fn; hf_prod *= hf_fn;
      sum_L4[qoi] += lf_prod; sum_H4[qoi] += hf_prod;
      sum_L4H4[qoi] += lf_prod * hf_prod;

      lf_prod *= lf_fn * lf_fn; sum_L6[qoi] += lf_prod;
      lf_prod *= lf_fn * lf_fn; sum_L8[qoi] += lf_prod;
    }
  }
  // compute & store subset of stats needed to compute avg_eval_ratio
  RealVector mean_L(numFunctions, false), mean_L2(numFunctions, false),
    mean_L3(numFunctions, false),  mean_L4(numFunctions, false),
    mean_H(numFunctions, false),   mean_H2(numFunctions, false),
    mean_H3(numFunctions, false),  mean_H4(numFunctions, false),
    var_L(numFunctions, false),    var_L2(numFunctions, false),
    var_L3(numFunctions, false),   var_L4(numFunctions, false),
    var_H(numFunctions, false),    cov_LH(numFunctions, false),
    cov_L2H2(numFunctions, false), cov_L3H3(numFunctions, false),
    cov_L4H4(numFunctions, false), rho2_LH(numFunctions, false);
  Real mu_L, mu_H, cov, rho_sq, mc_mse, cvmc_mse, mse_ratio, avg_mse_ratio = 0.,
    avg_eval_ratio = 0., bias_corr = 1./(total_N - 1);
  for (qoi=0; qoi<numFunctions; ++qoi) {
    // unbiased mean estimator X-bar = 1/N * sum
    mu_L = mean_L[qoi] = sum_L[qoi] / total_N;
    mu_H = mean_H[qoi] = sum_H[qoi] / total_N;
    // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
    // = 1/(N-1) ( sum[X^2_i] - N X-bar^2 ) where bias correction = 1/(N-1)
    var_L[qoi] = (sum_L2[qoi] - total_N * mu_L * mu_L) * bias_corr;
    var_H[qoi] = (sum_H2[qoi] - total_N * mu_H * mu_H) * bias_corr;
    cov = cov_LH[qoi] = (sum_LH[qoi] - total_N * mu_L * mu_H) * bias_corr;

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    rho_sq = rho2_LH[qoi] = cov / var_L[qoi] * cov / var_H[qoi]; // bias cancels
    avg_eval_ratio += (rho_sq >= 1.) ? maxFunctionEvals : // should not happen
      std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
  }
  avg_eval_ratio /= numFunctions;

  for (qoi=0; qoi<numFunctions; ++qoi) {
    // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
    // (just a diagnostic prior to iteration)
    mse_ratio = 1. - rho2_LH[qoi] * (1. - 1./avg_eval_ratio);
    mc_mse = var_H[qoi] / N_hf; cvmc_mse = mc_mse * mse_ratio;
    Cout << "Mean square error reduced from " << mc_mse << " to " << cvmc_mse
	 << " (relative factor of " << mse_ratio << ")\n";
    avg_mse_ratio += mse_ratio;
  }
  avg_mse_ratio /= numFunctions;

  // -----------------------------------------------------------
  // Compute HF + LF increment targeting specified MSE reduction
  // -----------------------------------------------------------

  // bypass refinement if maxIterations == 0 or convergenceTol already
  // satisfied by pilot sample.
  if (maxIterations && convergenceTol < avg_mse_ratio) {
    // Assuming rho_AB, evaluation_ratio and var_H to be invariant, we seek
    // a relative reduction in MSE using the convergence tolerance spec:
    //   convTol = CV_mse / MC^0_mse = mse_ratio * N0 / N
    //   delta_N = mse_ratio*N0/convTol - N0 = (mse_ratio/convTol - 1) * N0

    Real incr = (avg_mse_ratio/convergenceTol - 1.) * numSamples;
    numSamples = delta_N_lf = delta_N_hf = (size_t)std::floor(incr + .5);
    N_lf += delta_N_lf; N_hf += delta_N_hf; total_N += numSamples;

    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);// pull dist params from any model
    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();
      for (qoi=0; qoi<numFunctions; ++qoi) {
	// sum_* are running sums across all increments
	lf_prod = lf_fn = fn_vals[qoi];
	hf_prod = hf_fn = fn_vals[qoi+numFunctions];
	sum_L[qoi] += lf_fn; sum_H[qoi] += hf_fn; sum_LH[qoi] += lf_fn * hf_fn;

	lf_prod *= lf_fn; hf_prod *= hf_fn;
	sum_L2[qoi] += lf_prod; sum_H2[qoi] += hf_prod;
	sum_L2H2[qoi] += lf_prod * hf_prod;

	lf_prod *= lf_fn; hf_prod *= hf_fn;
	sum_L3[qoi] += lf_prod; sum_H3[qoi] += hf_prod; 
	sum_L3H3[qoi] += lf_prod * hf_prod;

	lf_prod *= lf_fn; hf_prod *= hf_fn;
	sum_L4[qoi] += lf_prod; sum_H4[qoi] += hf_prod;
	sum_L4H4[qoi] += lf_prod * hf_prod;

	lf_prod *= lf_fn * lf_fn; sum_L6[qoi] += lf_prod;
	lf_prod *= lf_fn * lf_fn; sum_L8[qoi] += lf_prod;
      }
    }

    // Update rho^2, avg_eval_ratio:
    avg_eval_ratio = 0.;
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // unbiased mean estimator X-bar = 1/N * sum
      mu_L = mean_L[qoi] = sum_L[qoi] / total_N;
      mu_H = mean_H[qoi] = sum_H[qoi] / total_N;
      // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
      // = 1/(N-1) (sum[X^2_i] - N X-bar^2) where bias correction = 1/(N-1)
      var_L[qoi] = (sum_L2[qoi] - total_N * mu_L * mu_L) * bias_corr;
      var_H[qoi] = (sum_H2[qoi] - total_N * mu_H * mu_H) * bias_corr;
      cov = cov_LH[qoi] = (sum_LH[qoi] - total_N * mu_L * mu_H) * bias_corr;

      // compute evaluation ratio which determines increment for LF samples
      // > the sample increment optimizes the total computational budget and is
      //   not treated as a worst case accuracy reqmt --> use the QoI average
      // > refinement based only on QoI mean statistics
      rho_sq = rho2_LH[qoi] = cov / var_L[qoi] * cov / var_H[qoi];//bias cancels
      avg_eval_ratio += (rho_sq >= 1.) ? maxFunctionEvals : // should not happen
	std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
    }
    avg_eval_ratio /= numFunctions;

    // Output updated MSE ratios:
    //avg_mse_ratio = 0.;
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
      // (just a diagnostic prior to iteration)
      mse_ratio = 1. - rho2_LH[qoi] * (1. - 1./avg_eval_ratio);
      mc_mse = var_H[qoi] / N_hf; cvmc_mse = mc_mse * mse_ratio;
      Cout << "Mean square error reduced from " << mc_mse << " to " << cvmc_mse
	   << " (relative factor of " << mse_ratio << ")\n";
      //avg_mse_ratio += mse_ratio;
    }
    //avg_mse_ratio /= numFunctions;
  }

  // bookkeeping for initial higher order stats
  for (qoi=0; qoi<numFunctions; ++qoi) {
    mu_L = mean_L2[qoi] = sum_L2[qoi] / total_N;
    mu_H = mean_H2[qoi] = sum_H2[qoi] / total_N;
    var_L2[qoi] = (sum_L4[qoi] - total_N * mu_L * mu_L) * bias_corr;
    //var_H2[qoi] = (sum_H4[qoi] - total_N * mu_H * mu_H) * bias_corr;
    cov_L2H2[qoi] = (sum_L2H2[qoi] - total_N * mu_L * mu_H) * bias_corr;

    mu_L = mean_L3[qoi] = sum_L3[qoi] / total_N;
    mu_H = mean_H3[qoi] = sum_H3[qoi] / total_N;
    var_L3[qoi] = (sum_L6[qoi] - total_N * mu_L * mu_L) * bias_corr;
    //var_H3[qoi] = (sum_H6[qoi] - total_N * mu_H * mu_H) * bias_corr;
    cov_L3H3[qoi] = (sum_L3H3[qoi] - total_N * mu_L * mu_H) * bias_corr;

    mu_L = mean_L4[qoi] = sum_L4[qoi] / total_N;
    mu_H = mean_H4[qoi] = sum_H4[qoi] / total_N;
    var_L4[qoi] = (sum_L8[qoi] - total_N * mu_L * mu_L) * bias_corr;
    //var_H4[qoi] = (sum_H8[qoi] - total_N * mu_H * mu_H) * bias_corr;
    cov_L4H4[qoi] = (sum_L4H4[qoi] - total_N * mu_L * mu_H) * bias_corr;
  }

  // ----------------------------------------------
  // Compute Final LF increment for control variate
  // ----------------------------------------------

  // update LF samples based on evaluation ratio
  // r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  delta_N_lf = (avg_eval_ratio <= 1.) ? 0 :
    (size_t)std::floor(N_lf * (avg_eval_ratio - 1.) + .5); // round
  delta_N_hf = 0; // simple one-pass
  ++iter;
  Cout << "CVMC iteration " << iter << " sample increments: LF = "
       << delta_N_lf << " HF = " << delta_N_hf << std::endl;

  // set the number of current samples from the defined increment
  numSamples = delta_N_lf;
  N_lf += delta_N_lf; //N_hf += delta_N_hf; // update total LF/HF samples

  iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model
  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);

  // process allResponses: accumulate new samples for each qoi
  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // sum_* are running sums across all sample increments
      lf_prod  = lf_fn = fn_vals[qoi]; sum_L[qoi] += lf_fn;
      lf_prod *= lf_fn; sum_L2[qoi] += lf_prod;
      lf_prod *= lf_fn; sum_L3[qoi] += lf_prod;
      lf_prod *= lf_fn; sum_L4[qoi] += lf_prod;
    }
  }

  // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k. These
  // uncentered raw moment estimates are then converted to standardized moments.
  if (momentStats.empty()) momentStats.shape/*Uninitialized*/(4, numFunctions);
  Real mu_L2, mu_L3, mu_L4, refined_mu_L, refined_mu_L2, refined_mu_L3,
    refined_mu_L4, est_H, est_H2, est_H3, est_H4, H_cntl, H2_cntl, H3_cntl,
    H4_cntl, cm2, cm3, cm4;
  bias_corr = 1. / (N_lf - 1);
  for (qoi=0; qoi<numFunctions; ++qoi) {
    // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
    // (just a diagnostic prior to iteration)
    mse_ratio = 1. - rho2_LH[qoi] * (1. - 1./avg_eval_ratio);
    mc_mse = var_H[qoi] / N_hf; cvmc_mse = mc_mse * mse_ratio;
    Cout << "Mean square error reduced from " << mc_mse << " to " << cvmc_mse
	 << " (relative factor of " << mse_ratio << ")\n";

    // from original data set:
    mu_L  = mean_L[qoi];      H_cntl = cov_LH[qoi]   / var_L[qoi];
    mu_L2 = mean_L2[qoi];    H2_cntl = cov_L2H2[qoi] / var_L2[qoi];
    mu_L3 = mean_L3[qoi];    H3_cntl = cov_L3H3[qoi] / var_L3[qoi];
    mu_L4 = mean_L4[qoi];    H4_cntl = cov_L4H4[qoi] / var_L4[qoi];

    // updated:
    refined_mu_L  = mean_L[qoi]  =  sum_L[qoi] / N_lf;
    refined_mu_L2 = mean_L2[qoi] = sum_L2[qoi] / N_lf;
    refined_mu_L3 = mean_L3[qoi] = sum_L3[qoi] / N_lf;
    refined_mu_L4 = mean_L4[qoi] = sum_L4[qoi] / N_lf;
    //var_L2[qoi] = (sum_L4[qoi] - numSamples * mu_L2 * mu_L2) * bias_corr;
    //var_H2[qoi] = (sum_H4[qoi] - numSamples * mu_H2 * mu_H2) * bias_corr;
    //cov_L2H2[qoi] = (sum_L2H2[qoi] - numSamples * mu_L2 * mu_H2) * bias_corr;
    //var_L3 = (sum_L6[qoi] - numSamples * mu_L3 * mu_L3) * bias_corr;
    //var_H3 = (sum_H6[qoi] - numSamples * mu_H3 * mu_H3) * bias_corr;
    //cov_L3H3 = (sum_L3H3[qoi] - numSamples * mu_L3 * mu_H3) * bias_corr;
    //var_L4 = (sum_L8[qoi] - numSamples * mu_L4 * mu_L4) * bias_corr;
    //var_H4 = (sum_H8[qoi] - numSamples * mu_H4 * mu_H4) * bias_corr;
    //cov_L4H4 = (sum_L4H4[qoi] - numSamples * mu_L4 * mu_H4) * bias_corr;

    // apply control for HF estimates (mean, std dev, skewness, kurtosis)
    est_H  = mean_H[qoi]  -  H_cntl * (mu_L  - refined_mu_L);
    est_H2 = mean_H2[qoi] - H2_cntl * (mu_L2 - refined_mu_L2);
    est_H3 = mean_H3[qoi] - H3_cntl * (mu_L3 - refined_mu_L3);
    est_H4 = mean_H4[qoi] - H4_cntl * (mu_L4 - refined_mu_L4);
    // convert from uncentered to centered moments
    cm2 = est_H2 - est_H*est_H; // variance
    cm3 = est_H3 - est_H*(3.*cm2 + est_H*est_H);
    cm4 = est_H4 - est_H*(4.*cm3 + est_H*(6.*cm2 + est_H*est_H));
    // convert from centered to standardized moments
    momentStats(0,qoi) = est_H;                    // mean
    momentStats(1,qoi) = std::sqrt(cm2);           // std deviation
    momentStats(2,qoi) = cm3 / std::pow(cm2, 1.5); // skewness
    momentStats(3,qoi) = cm4 / (cm2 * cm2) - 3.;   // excess kurtosis
  }
  Cout << '\n';

  /*
  //////////////////// REPLACE W/ ITERATIVE LOOP IN TIME //////////////////////

  // converge on sample counts per level
  N_lf = N_hf = 0;
  while ( (delta_N_lf || delta_N_hf) && iter <= maxIterations ) {
      
    // set the number of current samples from the defined increment
    N_lf += delta_N_lf; // update total LF samples
    N_hf += delta_N_hf; // update total HF samples
    if (delta_N_lf && delta_N_hf) {
      //check_same(delta_N_lf, delta_N_hf);
      iteratedModel.surrogate_response_mode(AGGREGATED_MODELS);
    }
    else if (delta_N_lf)
      iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
    else if (delta_N_hf) // should not happen
      iteratedModel.surrogate_response_mode(BYPASS_SURROGATE);

    numSamples = std::max(delta_N_lf, delta_N_hf);
    if (numSamples) {

      // generate new MC parameter sets
      get_parameter_sets(iteratedModel);// pull dist params from any model
      // compute allResponses from allVariables using hierarchical model
      evaluate_parameter_sets(iteratedModel, true, false);

      // process allResponses: accumulate new samples for each qoi
      for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
	const RealVector& fn_vals = r_it->second.function_values();
	for (qoi=0; qoi<numFunctions; ++qoi) {
	  // sum_* are running sums across all increments
	  lf_fn = fn_vals[qoi];
	  sum_L[qoi]  += lf_fn;
	  sum_L2[qoi] += lf_fn * lf_fn;
	  if (delta_N_hf) {
	    hf_fn = fn_vals[qoi+numFunctions];
	    sum_H[qoi]  += hf_fn;
	    sum_H2[qoi] += hf_fn * hf_fn;
	    sum_LH[qoi] += lf_fn * hf_fn;
	  }
	}
      }
      max_evr = 1.; // don't allow m < n
      for (qoi=0; qoi<numFunctions; ++qoi) {
	mu_L = mean_L[qoi] = sum_L[qoi] / N_lf;
	var_L[qoi] = sum_L2[qoi] / N_lf - mu_L * mu_L;
	if (delta_N_hf) {
	  mu_H = mean_H[qoi] = sum_H[qoi] / N_hf;
	  var_H[qoi]  = sum_H2[qoi] / N_hf - mu_H * mu_H;
	  cov  = cov_LH[qoi] = sum_LH[qoi] / N_hf - mu_L * mu_H;//not consistent
	  rho_sq = cov * cov / var_L[qoi] / var_H[qoi];
	  ev_ratio = std::sqrt(cost_ratio * rho2 / (1. - rho2));
	  if (ev_ratio > max_evr) max_evr = ev_ratio; // average expense ratio?
	}
      }
    }

    // 

    // update targets based on variance estimates
    // r = m/n -> m = r*n -> delta = m-n = (r-1)*n
    delta_N_lf = (iter) ? 0 : (size_t)std::floor((max_evr-1.) * N_lf + .5);
    delta_N_hf = 0;
    ++iter;
    Cout << "CVMC iteration " << iter << " sample increments: LF = "
	 << delta_N_lf << " HF = " << delta_N_hf << std::endl;
    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "Accumulated raw moments ():\n"; // TO DO
    }
  }

  // aggregate expected value of estimators for Y, Y^2, Y^3, and Y^4. Final
  // expected value result is sum of expected values from telescopic sum. These
  // uncentered raw moment estimates are then converted to standardized moments.
  if (momentStats.empty()) momentStats.shapeUninitialized(4, numFunctions);
  for (qoi=0; qoi<numFunctions; ++qoi) {
    Real control = cov_LH[qoi] / var_L[qoi];
    momentStats(0,qoi) = mu_H[qoi] - control * (mu_L[qoi] - TODO);
    // TO DO
  }
  */
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
  
  iteratedModel.surrogate_model(model_form);// LF soln_level not updated (yet)
  iteratedModel.truth_model(model_form);    // HF soln_level not updated (yet)

  Model& surr_model = iteratedModel.surrogate_model();
  size_t lev, num_lev = surr_model.solution_levels(), // single model form
    qoi, iter = 0, samp, new_N_l;
  SizetArray N_l, delta_N_l;
  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost = surr_model.solution_level_cost(), agg_var(num_lev);
  // For moment estimation, we accumulate telescoping sums for Q^order,
  // Yi = Q^i_{HF} - Q^i_{LF}.  For computing N_l from (aggregated) estimator
  // variance, we accumulate the square of the mean estimator Y1sq = Y1^2.
  RealMatrix sum_Y1(numFunctions, num_lev), sum_Y2(numFunctions, num_lev),
             sum_Y3(numFunctions, num_lev), sum_Y4(numFunctions, num_lev),
             sum_Y1sq(numFunctions, num_lev);
  Real agg_var_l, eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0.;
  IntRespMCIter r_it;
  
  // Initialize for pilot sample
  N_l.assign(num_lev, 0);
  if      (pilotSamples.empty())     delta_N_l.assign(num_lev, 100); // default
  else if (pilotSamples.size() == 1) delta_N_l.assign(num_lev, pilotSamples[0]);
  else                               delta_N_l = pilotSamples;
  Cout << "\nMLMC pilot sample:\n" << delta_N_l << std::endl;

  // now converge on sample counts per level (N_l)
  while (Pecos::l1_norm(delta_N_l) && iter <= maxIterations) {
      
    // set initial surrogate responseMode and model indices for lev 0
    iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // LF
    iteratedModel.surrogate_model(model_form, 0); // solution level 0

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_lev; ++lev) {

      if (lev) {
	if (lev == 1) // update responseMode for levels 1:num_lev-1
	  iteratedModel.surrogate_response_mode(AGGREGATED_MODELS); // {LF,HF}
	iteratedModel.surrogate_model(model_form, lev-1);
	iteratedModel.truth_model(model_form,     lev);
      }
      
      // set the number of current samples from the defined increment
      numSamples = delta_N_l[lev];
      // update total samples performed for this level
      N_l[lev]  += numSamples;

      // aggregate variances across QoI for estimating N_l (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_l = agg_var[lev]; // carried over from prev iter if no samp
      if (numSamples) {

	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model
	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);

	Real lf_fn, hf_fn, delta_fn, lf_prod, hf_prod, *sum_Y1_l = sum_Y1[lev],
	  *sum_Y2_l = sum_Y2[lev], *sum_Y3_l   = sum_Y3[lev],
	  *sum_Y4_l = sum_Y4[lev], *sum_Y1sq_l = sum_Y1sq[lev];

	// process allResponses: accumulate new samples for each qoi
	for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
	  const RealVector& fn_vals = r_it->second.function_values();
	  for (qoi=0; qoi<numFunctions; ++qoi) {
	    lf_fn = fn_vals[qoi];
	    // sum_Y*_l are running sums across all level increments
	    if (lev) {
	      hf_fn = fn_vals[qoi+numFunctions]; delta_fn = hf_fn - lf_fn;
	      sum_Y1_l[qoi] += delta_fn; sum_Y1sq_l[qoi] += delta_fn * delta_fn;
	      hf_prod = hf_fn*hf_fn; lf_prod = lf_fn*lf_fn;
	      sum_Y2_l[qoi] += hf_prod - lf_prod; // HF^2 - LF^2
	      hf_prod *= hf_fn; lf_prod *= lf_fn;
	      sum_Y3_l[qoi] += hf_prod - lf_prod; // HF^3 - LF^3
	      hf_prod *= hf_fn; lf_prod *= lf_fn;
	      sum_Y4_l[qoi] += hf_prod - lf_prod; // HF^4 - LF^4
	    }
	    else {
	      sum_Y1_l[qoi] += lf_fn; lf_prod = lf_fn*lf_fn;
	      sum_Y2_l[qoi] += lf_prod; sum_Y1sq_l[qoi] += lf_prod; // LF^2
	      lf_prod *= lf_fn; sum_Y3_l[qoi] += lf_prod; // LF^3
	      lf_prod *= lf_fn; sum_Y4_l[qoi] += lf_prod; // LF^4
	    }
	  }
	}

	// compute estimator mean & variance from current sample accumulation:
	agg_var_l = 0.;
	for (qoi=0; qoi<numFunctions; ++qoi) {
	  Real mu_Y = sum_Y1_l[qoi] / N_l[lev];
	  // Note: precision loss in variance is difficult to avoid without
	  // storing full sample history; must accumulate Y^2 across iterations
	  // instead of (Y-mean)^2 since mean is updated on each iteration.
	  agg_var_l += sum_Y1sq_l[qoi] / N_l[lev] - mu_Y * mu_Y;
	}
      }

      sum_sqrt_var_cost += std::sqrt(agg_var_l * cost[lev]);
      if (iter == 0) estimator_var0 += agg_var_l / N_l[lev];
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (iter == 0) // eps^2 / 2 = var * relative factor
      eps_sq_div_2 = estimator_var0 * convergenceTol;

    // update targets based on variance estimates
    Real fact = sum_sqrt_var_cost / eps_sq_div_2;
    for (lev=0; lev<num_lev; ++lev) {
      // Equation 3.9 in CTR Annual Research Briefs:
      // "A multifidelity control variate approach for the multilevel Monte 
      // Carlo technique," Geraci, Eldred, Iaccarino, 2015.
      new_N_l = std::sqrt(agg_var[lev] / cost[lev]) * fact;
      delta_N_l[lev] = (new_N_l > N_l[lev]) ? new_N_l - N_l[lev] : 0;
    }
    ++iter;
    Cout << "MLMC iteration " << iter << " sample increments:\n" << delta_N_l
	 << std::endl;
    if (outputLevel == DEBUG_OUTPUT) {
      Cout << "Accumulated raw moments (Y1, Y2, Y3, Y4):\n";
      write_data(Cout, sum_Y1); write_data(Cout, sum_Y2);
      write_data(Cout, sum_Y3); write_data(Cout, sum_Y4); Cout << std::endl;
      //write_data(Cout, sum_Y1sq);
    }
  }

  // aggregate expected value of estimators for Y, Y^2, Y^3, and Y^4.  Final
  // expected value result is sum of expected values from telescopic sum.
  // These uncentered raw moment estimates are then converted to standardized
  // moments.  There is no bias correction for small sample sizes as in
  // NonDSampling::compute_moments().
  if (momentStats.empty()) momentStats.shapeUninitialized(4, numFunctions);
  for (qoi=0; qoi<numFunctions; ++qoi) {
    Real mu_Y1 = 0., mu_Y2 = 0., mu_Y3 = 0., mu_Y4 = 0.;
    for (lev=0; lev<num_lev; ++lev) {
      size_t Nl = N_l[lev];
      mu_Y1 += sum_Y1(qoi,lev) / Nl; mu_Y2 += sum_Y2(qoi,lev) / Nl;
      mu_Y3 += sum_Y3(qoi,lev) / Nl; mu_Y4 += sum_Y4(qoi,lev) / Nl;
    }
    // convert from uncentered to centered moments
    Real cm2 = mu_Y2 - mu_Y1*mu_Y1, cm3 = mu_Y3 - mu_Y1*(3.*cm2 + mu_Y1*mu_Y1),
         cm4 = mu_Y4 - mu_Y1*(4.*cm3 + mu_Y1*(6.*cm2 + mu_Y1*mu_Y1));
    // convert from centered to standardized moments
    momentStats(0,qoi) = mu_Y1;                    // mean
    momentStats(1,qoi) = std::sqrt(cm2);           // std deviation
    momentStats(2,qoi) = cm3 / std::pow(cm2, 1.5); // skewness
    momentStats(3,qoi) = cm4 / (cm2 * cm2) - 3.;   // excess kurtosis
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
    s << "\nStatistics based on multilevel sample set:\n"; //<< N_l; // TO DO
    print_moments(s);//print_statistics(s);
  }
}

} // namespace Dakota
