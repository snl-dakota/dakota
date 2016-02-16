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
  size_t i, qoi, iter = 0, N_lf, N_hf, delta_N_lf, delta_N_hf, total_N;

  // sum_* are running sums across all increments
  IntRealVectorMap sum_L, sum_H, sum_LH, mean_L, mean_H, var_L, covar_LH;
  RealVector var_H(numFunctions, false), rho2_LH(numFunctions, false);
  RealVector empty_rv; std::pair<int, RealVector> empty_pr;
  for (i=1; i<=4; ++i) {
    empty_pr.first = i;
    // std::map::insert() returns std::pair<IntRVMIter, bool>:
    // use IntRVMIter to size empty_rv in place and init sums to 0
    sum_L.insert(empty_pr).first->second.size(numFunctions);
    sum_H.insert(empty_pr).first->second.size(numFunctions);
    sum_LH.insert(empty_pr).first->second.size(numFunctions);
    // for moment maps, size empty_rv in place but don't init to 0
    mean_L.insert(empty_pr).first->second.sizeUninitialized(numFunctions);
    mean_H.insert(empty_pr).first->second.sizeUninitialized(numFunctions);
    var_L.insert(empty_pr).first->second.sizeUninitialized(numFunctions);
    covar_LH.insert(empty_pr).first->second.sizeUninitialized(numFunctions);
  }
  empty_pr.first = 6; sum_L.insert(empty_pr).first->second.size(numFunctions);
  empty_pr.first = 8; sum_L.insert(empty_pr).first->second.size(numFunctions);

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

  // accumulate allResponses contributions to LF, HF, and LF-HF sums 
  accumulate_sums(sum_L, sum_H, sum_LH);
  // compute the LF/HF evaluation ratio, averaged over the QoI
  // This includes updating mean_L, mean_H, var_L, var_H, cov_LH, rho2_LH
  Real avg_eval_ratio
    = eval_ratio(sum_L, sum_H, sum_LH, total_N, cost_ratio, mean_L[1],
		 mean_H[1], var_L[1], var_H, covar_LH[1], rho2_LH);
  // compute the ratio of MC and CVMC mean squared errors (controls convergence)
  Real avg_mse_ratio = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, N_hf, iter);

  // -----------------------------------------------------------
  // Compute HF + LF increment targeting specified MSE reduction
  // -----------------------------------------------------------

  // bypass refinement if maxIterations == 0 or convergenceTol already
  // satisfied by pilot sample
  if (maxIterations && convergenceTol < avg_mse_ratio) {
    // Assuming rho_AB, evaluation_ratio and var_H to be relatively invariant,
    // we seek a relative reduction in MSE using the convergence tol spec:
    //   convTol = CV_mse / MC^0_mse = mse_ratio * N0 / N
    //   delta_N = mse_ratio*N0/convTol - N0 = (mse_ratio/convTol - 1) * N0

    Real incr = (avg_mse_ratio/convergenceTol - 1.) * numSamples;
    numSamples = delta_N_lf = delta_N_hf = (size_t)std::floor(incr + .5);
    N_lf += delta_N_lf; N_hf += delta_N_hf; total_N += numSamples;
    ++iter;
    Cout << "CVMC iteration " << iter << " sample increments: LF = "
	 << delta_N_lf << " HF = " << delta_N_hf << std::endl;

    // generate new MC parameter sets
    get_parameter_sets(iteratedModel);// pull dist params from any model
    // compute allResponses from allVariables using hierarchical model
    evaluate_parameter_sets(iteratedModel, true, false);

    // accumulate allResponses contributions to LF, HF, and LF-HF sums
    accumulate_sums(sum_L, sum_H, sum_LH);
    // compute the LF/HF evaluation ratio, averaged over the QoI; this
    // includes updating mean_L, mean_H, var_L, var_H, cov_LH, rho2_LH
    avg_eval_ratio
      = eval_ratio(sum_L, sum_H, sum_LH, total_N, cost_ratio, mean_L[1],
		   mean_H[1], var_L[1], var_H, covar_LH[1], rho2_LH);
    // compute ratio of MC and CVMC mean squared errors, averaged over the
    // QoI; the relative reduction in MSE is used to control convergence
    avg_mse_ratio = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, N_hf, iter);
  }

  // bookkeeping for higher order stats for matched LF/HF evaluations
  // (first 2 evaluation sets), prior to final LF-only sample increment
  Real mu_Li, mu_Hi, bias_corr = 1./(total_N - 1);
  for (i=2; i<=4; ++i) {
    const RealVector& sum_Li = sum_L[i]; const RealVector& sum_L2i = sum_L[2*i];
    const RealVector& sum_Hi = sum_H[i]; const RealVector& sum_LHi = sum_LH[i];
    RealVector& mean_Li = mean_L[i]; RealVector&   mean_Hi =   mean_H[i];
    RealVector&  var_Li =  var_L[i]; RealVector& covar_LHi = covar_LH[i];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      mu_Li = mean_Li[qoi] = sum_Li[qoi] / total_N;
      mu_Hi = mean_Hi[qoi] = sum_Hi[qoi] / total_N;
      var_Li[qoi]    = (sum_L2i[qoi] - total_N * mu_Li * mu_Li) * bias_corr;
      //var_Hi[qoi]  = (sum_H2i[qoi] - total_N * mu_Hi * mu_Hi) * bias_corr;
      covar_LHi[qoi] = (sum_LHi[qoi] - total_N * mu_Li * mu_Hi) * bias_corr;
    }
  }

  // ----------------------------------------------
  // Compute Final LF increment for control variate
  // ----------------------------------------------

  // update LF samples based on evaluation ratio
  // r = m/n -> m = r*n -> delta = m-n = (r-1)*n
  delta_N_lf = (avg_eval_ratio <= 1.) ? 0 :
    (size_t)std::floor(N_lf * (avg_eval_ratio - 1.) + .5); // round
  delta_N_hf = 0;
  Cout << "CVMC final sample increments: LF = " << delta_N_lf
       << " HF = " << delta_N_hf << std::endl;

  // set the number of current samples from the defined increment
  numSamples = delta_N_lf;
  N_lf += delta_N_lf; //N_hf += delta_N_hf; // update total LF/HF samples

  iteratedModel.surrogate_response_mode(UNCORRECTED_SURROGATE);
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model
  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);

  // process allResponses: accumulate new samples for each qoi
  Real lf_fn, lf_prod;
  RealVector& sum_L1 = sum_L[1]; RealVector& sum_L2 = sum_L[2];
  RealVector& sum_L3 = sum_L[3]; RealVector& sum_L4 = sum_L[4];
  for (IntRespMCIter r_it=allResponses.begin(); r_it!=allResponses.end();
       ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // sum_* are running sums across all sample increments
      lf_prod  = lf_fn = fn_vals[qoi]; sum_L1[qoi] += lf_fn;
      lf_prod *= lf_fn; sum_L2[qoi] += lf_prod;
      lf_prod *= lf_fn; sum_L3[qoi] += lf_prod;
      lf_prod *= lf_fn; sum_L4[qoi] += lf_prod;
    }
  }

  // aggregate expected value of estimators for E[Y] for Y=LF^k or Y=HF^k
  if (momentStats.empty()) momentStats.shapeUninitialized(4, numFunctions);
  Real H_cntl, refined_mu_Li, m1, cm2, cm3, cm4;
  RealMatrix H_raw_mom(4, numFunctions);
  for (i=1; i<=4; ++i) {
    const RealVector&  sum_Li =  sum_L[i]; RealVector& mean_Li = mean_L[i];
    const RealVector& mean_Hi = mean_H[i]; const RealVector& var_Li = var_L[i];
    const RealVector& covar_LHi = covar_LH[i];
    for (qoi=0; qoi<numFunctions; ++qoi) {
      // LF expectations prior to final sample increment:
      mu_Li  = mean_Li[qoi];
      H_cntl = covar_LHi[qoi] / var_Li[qoi];
      Cout << "Control variate beta parameter (moment " << i << ", QoI "
	   << qoi+1 << ") is " << H_cntl << '\n';
      // updated LF expectations following final sample increment:
      refined_mu_Li = mean_Li[qoi] = sum_Li[qoi] / N_lf;
      // apply control for HF uncentered raw moment estimates:
      H_raw_mom(i-1,qoi) = mean_Hi[qoi] - H_cntl * (mu_Li - refined_mu_Li);
    }
    Cout << '\n';
  }
  // Convert uncentered raw moment estimates to standardized moments
  convert_moments(H_raw_mom, momentStats);
}


void NonDMultilevelSampling::
accumulate_sums(IntRealVectorMap& sum_L, IntRealVectorMap& sum_H,
		IntRealVectorMap& sum_LH)
{
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRVMIter l_it, h_it, lh_it;
  int l_ord, h_ord, lh_ord, active_ord;

  for (IntRespMCIter r_it=allResponses.begin(); r_it!=allResponses.end();
       ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (size_t qoi=0; qoi<numFunctions; ++qoi) {

      lf_prod = lf_fn = fn_vals[qoi];
      hf_prod = hf_fn = fn_vals[qoi+numFunctions];

      l_it = sum_L.begin(); h_it = sum_H.begin(); lh_it = sum_LH.begin();
      l_ord  = /*(l_it  == sum_L.end())  ? 0 :*/  l_it->first;
      h_ord  = /*(h_it  == sum_H.end())  ? 0 :*/  h_it->first;
      lh_ord = /*(lh_it == sum_LH.end()) ? 0 :*/ lh_it->first;
      active_ord = 1;

      while (l_it!=sum_L.end() || h_it!=sum_H.end() || lh_it!=sum_LH.end()) {
    
	// Low
	if (l_ord == active_ord) {
	  l_it->second[qoi] += lf_prod;
	  ++l_it; l_ord = (l_it == sum_L.end()) ? 0 : l_it->first;
	}
	// High
	if (h_ord == active_ord) {
	  h_it->second[qoi] += hf_prod;
	  ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	}
	// Low-High
	if (lh_ord == active_ord) {
	  lh_it->second[qoi] += lf_prod * hf_prod;
	  ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	}

	lf_prod *= lf_fn; hf_prod *= hf_fn;
	++active_ord;
      }
    }
  }
}


Real NonDMultilevelSampling::
eval_ratio(const IntRealVectorMap& sum_L, const IntRealVectorMap& sum_H,
	   const IntRealVectorMap& sum_LH, size_t total_N, Real cost_ratio,
	   RealVector& mean_L, RealVector& mean_H,   RealVector& var_L,
	   RealVector& var_H,  RealVector& covar_LH, RealVector& rho2_LH)
{
  // Update rho^2, avg_eval_ratio:
  Real mu_L, mu_H, cov, rho_sq, avg_eval_ratio = 0.,
    bias_corr = 1./(total_N - 1);
  IntRVMCIter l1_cit = sum_L.find(1); const RealVector& sum_L1 = l1_cit->second;
  IntRVMCIter l2_cit = sum_L.find(2); const RealVector& sum_L2 = l2_cit->second;
  IntRVMCIter h1_cit = sum_H.find(1); const RealVector& sum_H1 = h1_cit->second;
  IntRVMCIter h2_cit = sum_H.find(2); const RealVector& sum_H2 = h2_cit->second;
  IntRVMCIter lh1_cit = sum_LH.find(1);
  const RealVector& sum_L1H1 = lh1_cit->second;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    // unbiased mean estimator X-bar = 1/N * sum
    mu_L = mean_L[qoi] = sum_L1[qoi] / total_N;
    mu_H = mean_H[qoi] = sum_H1[qoi] / total_N;
    // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
    // = 1/(N-1) (sum[X^2_i] - N X-bar^2) where bias correction = 1/(N-1)
    var_L[qoi] = (sum_L2[qoi] - total_N * mu_L * mu_L) * bias_corr;
    var_H[qoi] = (sum_H2[qoi] - total_N * mu_H * mu_H) * bias_corr;
    cov = covar_LH[qoi] = (sum_L1H1[qoi] - total_N * mu_L * mu_H) * bias_corr;

    // compute evaluation ratio which determines increment for LF samples
    // > the sample increment optimizes the total computational budget and is
    //   not treated as a worst case accuracy reqmt --> use the QoI average
    // > refinement based only on QoI mean statistics
    rho_sq = rho2_LH[qoi] = cov / var_L[qoi] * cov / var_H[qoi];//bias cancels
    avg_eval_ratio += (rho_sq >= 1.) ? maxFunctionEvals : // should not happen
      std::sqrt(cost_ratio * rho_sq / (1. - rho_sq));
  }
  avg_eval_ratio /= numFunctions;
  return avg_eval_ratio;
}


Real NonDMultilevelSampling::
MSE_ratio(Real avg_eval_ratio, const RealVector& var_H,
	  const RealVector& rho2_LH, size_t N_hf, size_t iter)
{
  if (iter == 0) mcMSEIter0.sizeUninitialized(numFunctions);
  Real mc_mse, cvmc_mse, mse_ratio, avg_mse_ratio = 0.;//,avg_mse_iter_ratio=0.;
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    // Compute ratio of MSE for high fidelity MC and multifidelity CVMC
    mse_ratio = 1. - rho2_LH[qoi] * (1. - 1./avg_eval_ratio); // Ng 2014
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

  // aggregate expected value of estimators for Y, Y^2, Y^3, Y^4. Final expected
  // value is sum of expected values from telescopic sum. There is no bias
  // correction for small sample sizes as in NonDSampling::compute_moments().
  RealMatrix Y_raw_mom(4, numFunctions);
  for (qoi=0; qoi<numFunctions; ++qoi) {
    Real *Y_rm_q = Y_raw_mom[qoi];
    for (lev=0; lev<num_lev; ++lev) {
      size_t Nl = N_l[lev];
      Y_rm_q[0] += sum_Y1(qoi,lev) / Nl;   Y_rm_q[1] += sum_Y2(qoi,lev) / Nl;
      Y_rm_q[2] += sum_Y3(qoi,lev) / Nl;   Y_rm_q[3] += sum_Y4(qoi,lev) / Nl;
    }
  }
  // Convert uncentered raw moment estimates to standardized moments
  convert_moments(Y_raw_mom, momentStats);
}


void NonDMultilevelSampling::
convert_moments(const RealMatrix& raw_moments, RealMatrix& standard_moments)
{
  if (standard_moments.empty())
    standard_moments.shapeUninitialized(4, numFunctions);
  Real m1, cm2, cm3, cm4;
  // Convert uncentered raw moment estimates to standardized moments
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    m1  = standard_moments(0,qoi) = raw_moments(0,qoi);   // mean
    // convert from uncentered to centered moments
    cm2 = raw_moments(1,qoi) - m1 * m1; // variance
    cm3 = raw_moments(2,qoi) - m1 * (3. * cm2 + m1 * m1);
    cm4 = raw_moments(3,qoi) - m1 * (4. * cm3 + m1 * (6. * cm2 + m1 * m1));
    // convert from centered to standardized moments
    standard_moments(1,qoi) = std::sqrt(cm2);           // std deviation
    standard_moments(2,qoi) = cm3 / std::pow(cm2, 1.5); // skewness
    standard_moments(3,qoi) = cm4 / (cm2 * cm2) - 3.;   // excess kurtosis
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
