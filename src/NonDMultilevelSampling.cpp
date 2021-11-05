/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevelSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevelSampling::
NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model):
  NonDHierarchSampling(problem_db, model),
  allocationTarget(problem_db.get_short("method.nond.allocation_target")),
  useTargetVarianceOptimizationFlag(
    problem_db.get_bool("method.nond.allocation_target.optimization")),
  qoiAggregation(problem_db.get_short("method.nond.qoi_aggregation")),
  convergenceTolType(
    problem_db.get_short("method.nond.convergence_tolerance_type")),
  convergenceTolTarget(
    problem_db.get_short("method.nond.convergence_tolerance_target"))
{
  // For testing multilevel_mc_Qsum():
  //subIteratorFlag = true;

  if (allocationTarget == TARGET_SCALARIZATION) {
    if (qoiAggregation == QOI_AGGREGATION_SUM) {
      Cerr << "\nError: Scalarization not available with setting qoi_"
	   << "aggregation=sum. Use qoi_aggregation=max instead." << std::endl;
      abort_handler(METHOD_ERROR);
    }
    // Retrieve the variable mapping inputs
    const RealVector& scalarization_resp_vector
      = probDescDB.get_rv("method.nond.scalarization_response_mapping");
    if (scalarization_resp_vector.empty() ||
	scalarization_resp_vector.length() != numFunctions*(2*numFunctions) ) {
      Cerr << "\n Warning: no or incomplete mappings provided for scalarization"
	   << " mapping in multilevel sampling initialization. Checking for "
	   << "nested model." << std::endl;
    }
    else{
      scalarizationCoeffs.reshape(numFunctions, 2*numFunctions);
      size_t vec_ctr = 0;
      for(size_t i = 0; i < numFunctions; ++i){
        for(size_t j = 0; j < numFunctions; ++j){
          scalarizationCoeffs(i, 2*j)   = scalarization_resp_vector[vec_ctr++];
          scalarizationCoeffs(i, 2*j+1) = scalarization_resp_vector[vec_ctr++];
        }
      }
    }
  }
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDMultilevelSampling::core_run()
{
  // prefer ML over MF if both available
  iteratedModel.multifidelity_precedence(false);

  if (true)//(subIteratorFlag)
    multilevel_mc_Qsum(); // w/ error est, unbiased central moments
  else
    multilevel_mc_Ysum(); // lighter weight
}


/** This function performs MLMC on a model sequence, either defined by
    model forms or discretization levels. */
void NonDMultilevelSampling::multilevel_mc_Ysum()
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

  // Allow either model forms or discretization levels, but not both
  size_t num_steps, form, lev, secondary_index; short seq_type;
  configure_sequence(num_steps, secondary_index, seq_type);
  bool multilev = (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE),
    budget_constrained = (maxFunctionEvals != SZ_MAX);
  // either lev varies and form is fixed, or vice versa:
  size_t& step = (multilev) ? lev : form;
  if (multilev) form = secondary_index;
  else          lev  = secondary_index;

  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost, agg_var(num_steps);
  configure_cost(num_steps, multilev, cost);
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., lev_cost, budget,
    ref_cost = cost[num_steps-1]; // HF cost (1 level)
  if (budget_constrained) budget = (Real)maxFunctionEvals * ref_cost;
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (sum_Y[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[i] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_Y; RealMatrix sum_YY(numFunctions, num_steps);
  initialize_ml_Ysums(sum_Y, num_steps);

  // Initialize for pilot sample
  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, num_steps, delta_N_l);

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  //SizetArray raw_N_l(num_steps, 0);
  RealVectorArray mu_hat(num_steps);
  //Sizet2DArray& N_l = NLev[form]; // slice only valid for ML
  // define a new 2D array and then post back to NLev at end
  Sizet2DArray N_l(num_steps);
  for (step=0; step<num_steps; ++step)
    N_l[step].assign(numFunctions, 0);

  // now converge on sample counts per level (N_l)
  while (Pecos::l1_norm(delta_N_l) && mlmfIter <= maxIterations) {

    sum_sqrt_var_cost = 0.;
    for (step=0; step<num_steps; ++step) { // step is reference to lev

      configure_indices(step, form, lev, seq_type); // step,form,lev as size_t
      lev_cost = level_cost(cost, step); // raw cost (not equiv HF)

      // set the number of current samples from the defined increment
      numSamples = delta_N_l[step];

      // aggregate variances across QoI for estimating N_l (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_l = agg_var[step]; // carried over from prev iter if no samp
      if (numSamples) {

	// assign sequence, get samples, export, evaluate
	evaluate_ml_sample_increment(step);

	// process allResponses: accumulate new samples for each qoi and
	// update number of successful samples for each QoI
	//if (mlmfIter == 0) accumulate_offsets(mu_hat[lev]);
	accumulate_ml_Ysums(sum_Y, sum_YY, lev, mu_hat[step], N_l[step]);
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Accumulated sums (Y1, Y2, Y3, Y4, Y1sq):\n" << sum_Y[1]
	       << sum_Y[2] << sum_Y[3] << sum_Y[4] << sum_YY << std::endl;
	// update raw evaluation counts
	//raw_N_l[step] += numSamples;
	increment_ml_equivalent_cost(numSamples, lev_cost, ref_cost);

	// compute estimator variance from current sample accumulation:
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "variance of Y[" << step << "]: ";
	agg_var_l
	  = aggregate_variance_Ysum(sum_Y[1][step], sum_YY[step], N_l[step]);
      }

      sum_sqrt_var_cost += std::sqrt(agg_var_l * lev_cost);
      // MSE reference is MLMC with pilot sample, prior to any N_l adaptation:
      if (mlmfIter == 0 && !budget_constrained)
	estimator_var0
	  += aggregate_mse_Ysum(sum_Y[1][step], sum_YY[step], N_l[step]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance from MLMC
    // on the pilot sample and then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0 && !budget_constrained) { // eps^2 / 2 = est var * rel tol
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // update sample targets based on latest variance estimates
    Real N_target, fact = (budget_constrained) ?
      budget / sum_sqrt_var_cost :      // budget constraint
      sum_sqrt_var_cost / eps_sq_div_2; // error balance constraint
    for (step=0; step<num_steps; ++step) {
      // Equation 3.9 in CTR Annual Research Briefs:
      // "A multifidelity control variate approach for the multilevel Monte 
      // Carlo technique," Geraci, Eldred, Iaccarino, 2015.
      N_target = std::sqrt(agg_var[step] / level_cost(cost, step)) * fact;
      delta_N_l[step] = one_sided_delta(average(N_l[step]), N_target);
    }
    ++mlmfIter;
    Cout << "\nMLMC iteration " << mlmfIter << " sample increments:\n"
	 << delta_N_l << std::endl;
  }
  // post final N_l back to NLev (needed for final eval summary)
  inflate_final_samples(N_l, multilev, secondary_index, NLev);

  // aggregate expected value of estimators for Y, Y^2, Y^3, Y^4. Final expected
  // value is sum of expected values from telescopic sum.  Note: raw moments
  // have no bias correction (no additional variance from an estimated center).
  RealMatrix Q_raw_mom(numFunctions, 4);
  RealMatrix &sum_Y1 = sum_Y[1], &sum_Y2 = sum_Y[2],
	     &sum_Y3 = sum_Y[3], &sum_Y4 = sum_Y[4];
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    for (step=0; step<num_steps; ++step) {
      size_t Nlq = N_l[step][qoi];
      Q_raw_mom(qoi,0) += sum_Y1(qoi,step) / Nlq;
      Q_raw_mom(qoi,1) += sum_Y2(qoi,step) / Nlq;
      Q_raw_mom(qoi,2) += sum_Y3(qoi,step) / Nlq;
      Q_raw_mom(qoi,3) += sum_Y4(qoi,step) / Nlq;
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Q_raw_mom, momentStats);
}


/** This function performs "geometrical" MLMC on a single model form
    with multiple discretization levels. */
void NonDMultilevelSampling::multilevel_mc_Qsum()
{
  if (allocationTarget == TARGET_SCALARIZATION && scalarizationCoeffs.empty()) {
    Cerr << "\n Warning: no or incomplete mappings provided for scalarization "
	 << "mapping\n          in multilevel sampling initialization. Has to "
	 << "be specified\n          via scalarization_response_mapping or "
	 << "nested model." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Allow either model forms or discretization levels, but not both
  size_t num_steps, form, lev, secondary_index; short seq_type;
  configure_sequence(num_steps, secondary_index, seq_type);
  bool multilev = (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE),
    budget_constrained = (maxFunctionEvals != SZ_MAX);
  // either lev varies and form is fixed, or vice versa:
  size_t& step = (multilev) ? lev : form;
  if (multilev) form = secondary_index;
  else          lev  = secondary_index;

  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost;  configure_cost(num_steps, multilev, cost);
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., lev_cost,
    ref_cost = cost[num_steps-1]; // HF cost (1 level)

  // retrieve cost estimates across soln levels for a particular model form
  RealVector agg_var(num_steps), agg_var_of_var(num_steps),
    estimator_var0_qoi(numFunctions), eps_sq_div_2_qoi(numFunctions),
    sum_sqrt_var_cost_qoi(numFunctions),
    sum_sqrt_var_cost_var_qoi(numFunctions),
    sum_sqrt_var_cost_mean_qoi(numFunctions), agg_var_l_qoi(numFunctions),
    level_cost_vec(num_steps);
  RealMatrix agg_var_qoi(numFunctions, num_steps),
    agg_var_mean_qoi(numFunctions, num_steps),
    agg_var_var_qoi(numFunctions, num_steps);

  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[i] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_Ql, sum_Qlm1;
  IntIntPairRealMatrixMap sum_QlQlm1;
  initialize_ml_Qsums(sum_Ql, sum_Qlm1, sum_QlQlm1, num_steps);

  // Initialize for pilot sample
  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, num_steps, delta_N_l);

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  //SizetArray raw_N_l(num_steps, 0);
  RealVectorArray mu_hat(num_steps);
  //Sizet2DArray& N_l = NLev[form]; // *** VALID ONLY FOR ML
  // define a new 2D array and then post back to NLev at end
  Sizet2DArray N_l(num_steps);
  for (step=0; step<num_steps; ++step)
    N_l[step].assign(numFunctions, 0);

  // Useful for future extensions when convergence tolerance can be a vector
  convergenceTolVec.resize(numFunctions);
  for(size_t qoi = 0; qoi < numFunctions; ++qoi)
    convergenceTolVec[qoi] = convergenceTol;

  // now converge on sample counts per level (N_l)
  while (Pecos::l1_norm(delta_N_l) && mlmfIter <= maxIterations) {

    for (step=0; step<num_steps; ++step) {

      configure_indices(step, form, lev, seq_type);
      lev_cost = level_cost_vec[step] = level_cost(cost, step);

      // set the number of current samples from the defined increment
      numSamples = delta_N_l[step];

      // aggregate variances across QoI for estimating N_l (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      //Real &agg_var_l = agg_var[step];//carried over from prev iter if no samp
      if (numSamples) {

	// assign sequence, get samples, export, evaluate
	evaluate_ml_sample_increment(step);

	accumulate_sums(sum_Ql, sum_Qlm1, sum_QlQlm1, step, mu_hat, N_l);

	// update raw evaluation counts
	//raw_N_l[step] += numSamples;
	increment_ml_equivalent_cost(numSamples, lev_cost, ref_cost);

	aggregate_variance_target_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l,
				       step, agg_var_qoi);
	// MSE reference is MC applied to HF
	if (mlmfIter == 0 && !budget_constrained)
	  aggregate_mse_target_Qsum(agg_var_qoi, N_l, step, estimator_var0_qoi);
      }
    }
    if (mlmfIter == 0 && !budget_constrained) // eps^2 / 2 = est var * conv tol
      set_convergence_tol(estimator_var0_qoi, cost, eps_sq_div_2_qoi);

    // update targets based on variance estimates
    if (budget_constrained)
      compute_sample_allocation_target(agg_var_qoi, cost, N_l, delta_N_l);
    else
      compute_sample_allocation_target(sum_Ql, sum_Qlm1, sum_QlQlm1,
				       eps_sq_div_2_qoi, agg_var_qoi,
				       cost, N_l, delta_N_l);

    ++mlmfIter;
    Cout << "\nMLMC iteration " << mlmfIter << " sample increments:\n"
	 << delta_N_l << std::endl;
  }
  // post final N_l back to NLev (needed for final eval summary)
  inflate_final_samples(N_l, multilev, secondary_index, NLev);

  // roll up moment contributions
  compute_moments(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l);
  // populate finalStatErrors
  compute_error_estimates(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l);
}


void NonDMultilevelSampling::
configure_indices(unsigned short group, unsigned short form,
		  size_t lev, short seq_type)
{
  // Notes:
  // > could consolidate with NonDExpansion::configure_indices() with a passed
  //   model and virtual *_mode() assignments.  Leaving separate for now...
  // > group index is assigned based on step in model form/resolution sequence
  // > CVMC does not use this helper; it requires uncorrected_surrogate_mode()

  Pecos::ActiveKey hf_key;  hf_key.form_key(group, form, lev);

  if ( (seq_type == Pecos::MODEL_FORM_SEQUENCE       && form == 0) ||
       (seq_type == Pecos::RESOLUTION_LEVEL_SEQUENCE && lev  == 0)) {
    // step 0 in the sequence
    bypass_surrogate_mode();
    iteratedModel.active_model_key(hf_key);      // one active fidelity
  }
  else {
    aggregated_models_mode();

    Pecos::ActiveKey lf_key(hf_key.copy()), discrep_key;
    lf_key.decrement_key(seq_type); // seq_index defaults to 0
    // For MLMC/MFMC/MLMFMC, we aggregate levels but don't reduce them
    discrep_key.aggregate_keys(hf_key, lf_key, Pecos::RAW_DATA);
    iteratedModel.active_model_key(discrep_key); // two active fidelities
  }
}


void NonDMultilevelSampling::evaluate_ml_sample_increment(unsigned short step)
{
  // advance any sequence specifications (seed_sequence)
  assign_specification_sequence(step);
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set, including a header
  // and variables sets (no responses).
  // *** TO DO: Even though these samples typically involve {truth,surrogate}
  //     aggregation, we currently tag with the truth_model's interface id.
  //     This is correct for bypass_surrogate_mode(), but consider the new
  //     integrated tabular format for aggregated_models_mode().
  if (exportSampleSets)
    export_all_samples("ml_", iteratedModel.truth_model(), mlmfIter, step);

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


void NonDMultilevelSampling::
accumulate_sums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		IntIntPairRealMatrixMap& sum_QlQlm1, const size_t step,
		const RealVectorArray& offset, Sizet2DArray& N_l)
{
  // process allResponses: accumulate new samples for each qoi and
  // update number of successful samples for each QoI
  //if (mlmfIter == 0) accumulate_offsets(mu_hat[step]);
  accumulate_ml_Qsums(sum_Ql, sum_Qlm1, sum_QlQlm1, step,
                      offset[step], N_l[step]);

  if (outputLevel == DEBUG_OUTPUT)
    Cout << "Accumulated sums (Ql[1,2], Qlm1[1,2]):\n" << sum_Ql[1]
         << sum_Ql[2] << sum_Qlm1[1] << sum_Qlm1[2] << std::endl;
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
accumulate_ml_Qsums(IntRealMatrixMap& sum_Q, size_t lev,
		    const RealVector& offset, SizetArray& num_Q)
{
  using std::isfinite;
  Real q_l, q_l_prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRMMIter q_it;
  bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      q_l_prod = q_l = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];

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
    //Cout << r_it->first << ": " << sum_Q[1];
  }
}


void NonDMultilevelSampling::
accumulate_ml_Qsums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
		    IntIntPairRealMatrixMap& sum_QlQlm1, size_t lev,
		    const RealVector& offset, SizetArray& num_Q)
{
  if (lev == 0)
    accumulate_ml_Qsums(sum_Ql, lev, offset, num_Q);
  else {
    using std::isfinite;
    Real q_l, q_lm1, q_l_prod, q_lm1_prod, qq_prod;
    int l1_ord, l2_ord, active_ord; size_t qoi;
    IntRespMCIter r_it; IntRMMIter l1_it, l2_it; IntIntPair pr;
    bool os = !offset.empty();

    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();

      for (qoi=0; qoi<numFunctions; ++qoi) {
	// response mode AGGREGATED_MODELS orders HF (active model key)
	// followed by LF (previous/decremented model key)
	q_l_prod   = q_l   = (os) ?  fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
	q_lm1_prod = q_lm1 = (os) ?
	  fn_vals[qoi+numFunctions] - offset[qoi+numFunctions] :
	  fn_vals[qoi+numFunctions];

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
      //Cout << r_it->first << ": " << sum_Ql[1] << sum_Qlm1[1];
    }
  }
}


void NonDMultilevelSampling::
accumulate_ml_Ysums(IntRealMatrixMap& sum_Y, RealMatrix& sum_YY, size_t lev,
		    const RealVector& offset, SizetArray& num_Y)
{
  using std::isfinite;
  Real lf_fn, lf_prod;
  int y_ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRMMIter y_it;
  bool os = !offset.empty();

  if (lev == 0) {
    for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
      const RealVector& fn_vals = r_it->second.function_values();
      for (qoi=0; qoi<numFunctions; ++qoi) {

	lf_prod = lf_fn = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
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

	// response mode AGGREGATED_MODELS orders HF (active model key)
	// followed by LF (previous/decremented model key)
	hf_prod = hf_fn = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
	lf_prod = lf_fn = (os) ?
	  fn_vals[qoi+numFunctions] - offset[qoi+numFunctions] :
	  fn_vals[qoi+numFunctions];
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
aggregate_variance_target_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                const Sizet2DArray& N_l, const size_t step, RealMatrix& agg_var_qoi)
{
  // compute estimator variance from current sample accumulation:
  if (outputLevel >= DEBUG_OUTPUT) Cout << "variance of Y[" << step << "]: ";
  for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
    if (allocationTarget == TARGET_MEAN) {
      agg_var_qoi(qoi, step) = aggregate_variance_mean_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi);
    } else if (allocationTarget == TARGET_VARIANCE) {
      agg_var_qoi(qoi, step) = aggregate_variance_variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi); 
    } else if (allocationTarget == TARGET_SIGMA) {
      agg_var_qoi(qoi, step) = aggregate_variance_sigma_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi); 
    } else if (allocationTarget == TARGET_SCALARIZATION){
      agg_var_qoi(qoi, step) = aggregate_variance_scalarization_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, qoi); 
    }else{
        Cout << "NonDMultilevelSampling::aggregate_variance_target_Qsum: allocationTarget is not known.\n";
        abort_handler(INTERFACE_ERROR);
    }
    check_negative(agg_var_qoi(qoi, step));
  }
}

Real NonDMultilevelSampling::aggregate_variance_mean_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const Sizet2DArray& N_l, const size_t step, const size_t qoi)
{
  IntIntPair pr11(1, 1);
  Real agg_var_l = 0.;
  agg_var_l = aggregate_variance_Qsum(sum_Ql.at(1)[step], sum_Qlm1.at(1)[step],
                                                       sum_Ql.at(2)[step], sum_QlQlm1.at(pr11)[step],
                                                       sum_Qlm1.at(2)[step],
                                                           N_l[step], step, qoi);

  return agg_var_l;
}

Real NonDMultilevelSampling::aggregate_variance_variance_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const Sizet2DArray& N_l, const size_t step, const size_t qoi)
{
  Real place_holder;
  Real agg_var_l = 0.;
  agg_var_l = ((step == 0) ? var_of_var_ml_l0(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
                                                           N_l[step][qoi], qoi, false, place_holder)
                                        : var_of_var_ml_l(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
                                                          N_l[step][qoi], qoi, step, false, place_holder)) *
                            N_l[step][qoi]; //As described in the paper by Krumscheid, Pisaroni, Nobile

  return agg_var_l;
}

Real NonDMultilevelSampling::aggregate_variance_sigma_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const Sizet2DArray& N_l, const size_t step, const size_t qoi)
{
  Real place_holder;
  Real agg_var_l = 0.;
  Real var_l = 0;
  IntIntPair pr11(1, 1);

  agg_var_l = ((step == 0) ? var_of_var_ml_l0(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
                                                           N_l[step][qoi], qoi, false, place_holder)
                                        : var_of_var_ml_l(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l[step][qoi],
                                                          N_l[step][qoi], qoi, step, false, place_holder)); 
  var_l = var_lev_l(sum_Ql.at(1)[step][qoi], sum_Qlm1.at(1)[step][qoi], sum_Ql.at(2)[step][qoi], sum_Qlm1.at(2)[step][qoi], N_l[step][qoi]);
  if(var_l <= 0)
    return 0;
  return 1./(4. * var_l) * agg_var_l * N_l[step][qoi]; //Multiplication by N_l as described in the paper by Krumscheid, Pisaroni, Nobile
}

Real NonDMultilevelSampling::aggregate_variance_scalarization_Qsum(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, 
                  const Sizet2DArray& N_l, const size_t step, const size_t qoi)
{
  Real upper_bound_cov_of_mean_sigma = 0;
  Real var_of_mean_l = 0.;
  Real var_of_sigma_l = 0;
  Real var_of_scalarization_cur = 0;
  Real var_of_scalarization_l = 0;
  size_t cur_qoi_offset = 0;

  /// For TARGET_SCALARIZATION we have the special case that we can also combine scalarization over multiple qoi
  /// This is respresented in the scalarization response mapping stored in scalarizationCoeffs
  /// This is for now neglecting cross terms for covariance terms inbetween different qois, e.g.
  /// V[mu_1 + 2 sigma_1 + 3 mu_2] = 
  /// V[mu_1] + V[2 sigma_1] + 2 Cov[mu_1, 2 sigma_1] + V[3 mu_2] + 2 Cov[2 mu_1, 3 mu_2] + 2 Cov[2 sigma_1, 3 mu_2]
  /// \approx V[mu_1] + V[2 sigma_1] + 2 Cov[mu_1, 2 sigma_1] + V[3 mu_2] (What we do)
  Real cov_scaling = 1.0;
  for(size_t cur_qoi = 0; cur_qoi < numFunctions; ++cur_qoi){
    cur_qoi_offset = cur_qoi*2;
    var_of_mean_l = aggregate_variance_mean_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, cur_qoi);
    var_of_sigma_l = aggregate_variance_sigma_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, cur_qoi);
    upper_bound_cov_of_mean_sigma = std::sqrt(var_of_mean_l*var_of_sigma_l); // * N_l[step][cur_qoi];
    var_of_scalarization_cur = scalarizationCoeffs(qoi, cur_qoi_offset) * scalarizationCoeffs(qoi, cur_qoi_offset) * var_of_mean_l 
                            + scalarizationCoeffs(qoi, cur_qoi_offset+1) * scalarizationCoeffs(qoi, cur_qoi_offset+1) * var_of_sigma_l
                            + cov_scaling * 2.0 * std::abs(scalarizationCoeffs(qoi, cur_qoi_offset)) * std::abs(scalarizationCoeffs(qoi, cur_qoi_offset+1)) * upper_bound_cov_of_mean_sigma;
    var_of_scalarization_l += var_of_scalarization_cur;
  }                 
  return var_of_scalarization_l; //Multiplication by N_l as described in the paper by Krumscheid, Pisaroni, Nobile is already done in submethods
}

void NonDMultilevelSampling::
compute_sample_allocation_target(const RealMatrix& agg_var_qoi_in,
				 const RealVector& cost,
				 const Sizet2DArray& N_l, SizetArray& delta_N_l)
{
  size_t qoi, step, num_steps = N_l.size();
  Real sum_sqrt_var_cost = 0.;
  RealVector agg_var(num_steps); // init to 0
  RealVector lev_cost(num_steps, false);
  for (step = 0; step < num_steps ; ++step) {
    Real& agg_var_l = agg_var[step];  Real& lev_cost_l = lev_cost[step];
    lev_cost_l = level_cost(cost, step);
    for (qoi = 0; qoi < numFunctions ; ++qoi)
      agg_var_l += agg_var_qoi_in(qoi, step);
    sum_sqrt_var_cost += std::sqrt(agg_var_l * lev_cost_l);
  }

  Real budget = (Real)maxFunctionEvals * cost[num_steps-1],
    N_target, fact = budget / sum_sqrt_var_cost;
  for (step=0; step<num_steps; ++step) {
    N_target = std::sqrt(agg_var[step] / lev_cost[step]) * fact;
    delta_N_l[step] = one_sided_delta(average(N_l[step]), N_target);
  }
}


void NonDMultilevelSampling::compute_sample_allocation_target(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, 
                  const IntIntPairRealMatrixMap& sum_QlQlm1, const RealVector& eps_sq_div_2_in, 
                      const RealMatrix& agg_var_qoi_in, const RealVector& cost, 
                      const Sizet2DArray& N_l, SizetArray& delta_N_l)
{
  const size_t num_steps = agg_var_qoi_in.numCols(), max_iter = (maxIterations < 0) ? 25 : maxIterations;
  RealVector level_cost_vec(num_steps);
  RealVector sum_sqrt_var_cost;
  RealMatrix delta_N_l_qoi;
  RealVector eps_sq_div_2;
  RealMatrix agg_var_qoi;

  size_t nb_aggregation_qois = 0;
  double underrelaxation_factor = static_cast<double>(mlmfIter + 1)/static_cast<double>(max_iter + 1);

  if (outputLevel == DEBUG_OUTPUT) Cout << "N_target: " << std::endl;

  if (qoiAggregation==QOI_AGGREGATION_SUM) {
    nb_aggregation_qois = 1;
    eps_sq_div_2.size(nb_aggregation_qois);
    sum_sqrt_var_cost.size(nb_aggregation_qois);
    agg_var_qoi.shape(nb_aggregation_qois, num_steps);
    eps_sq_div_2[0] = 0;
    sum_sqrt_var_cost[0] = 0;

    for (size_t step = 0; step < num_steps ; ++step) {
      agg_var_qoi(0, step) = 0;
      for (size_t qoi = 0; qoi < numFunctions ; ++qoi) {
        agg_var_qoi(0, step) += agg_var_qoi_in(qoi, step);
      }
      sum_sqrt_var_cost[0] += std::sqrt(agg_var_qoi(0, step) * level_cost(cost, step));
    }
    for (size_t qoi = 0; qoi < numFunctions ; ++qoi) {
      eps_sq_div_2[0] += eps_sq_div_2_in[qoi];
    }

  }else if (qoiAggregation==QOI_AGGREGATION_MAX) {
    nb_aggregation_qois = numFunctions;
    eps_sq_div_2.size(nb_aggregation_qois);
    sum_sqrt_var_cost.size(nb_aggregation_qois);
    agg_var_qoi.shape(nb_aggregation_qois, num_steps);

    for (size_t qoi = 0; qoi < nb_aggregation_qois ; ++qoi) {
      eps_sq_div_2[qoi] = eps_sq_div_2_in[qoi];
      sum_sqrt_var_cost[qoi] = 0;
      for (size_t step = 0; step < num_steps ; ++step) {
        sum_sqrt_var_cost[qoi] += std::sqrt(agg_var_qoi_in(qoi, step) * level_cost(cost, step));

        agg_var_qoi(qoi, step) = agg_var_qoi_in(qoi, step);
        if (outputLevel == DEBUG_OUTPUT){
          Cout << "\n\tN_target for Qoi: " << qoi << ", with agg_var_qoi_in: " << agg_var_qoi_in(qoi, step) << std::endl;
          Cout << "\n\tN_target for Qoi: " << qoi << ", with level_cost: " << level_cost(cost, step) << std::endl;
          Cout << "\n\tN_target for Qoi: " << qoi << ", with agg_var_qoi: " << sum_sqrt_var_cost << std::endl;
        }
      }
    }
  }else{
      Cout << "NonDMultilevelSampling::compute_sample_allocation_target: qoiAggregation is not known.\n";
      abort_handler(INTERFACE_ERROR);
  }

  NTargetQoi.shape(nb_aggregation_qois, num_steps);
  NTargetQoiFN.shape(nb_aggregation_qois, num_steps);
  delta_N_l_qoi.shape(nb_aggregation_qois, num_steps);

  for (size_t qoi = 0; qoi < nb_aggregation_qois; ++qoi) {
    Real fact_qoi = (eps_sq_div_2[qoi] <= Pecos::SMALL_NUMBER) ? 0 : sum_sqrt_var_cost[qoi]/eps_sq_div_2[qoi];
    if (outputLevel == DEBUG_OUTPUT){
      Cout << "\n\tN_target for Qoi: " << qoi << ", with sum_sqrt_var_cost: " << sum_sqrt_var_cost[qoi] << std::endl;
      Cout << "\n\tN_target for Qoi: " << qoi << ", with eps_sq_div_2: " << eps_sq_div_2[qoi] << std::endl;
      Cout << "\n\tN_target for Qoi: " << qoi << ", with lagrange: " << fact_qoi << std::endl;
    }
    for (size_t step = 0; step < num_steps; ++step) {
      level_cost_vec[step] = level_cost(cost, step);
      if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
        NTargetQoi(qoi, step) = (fact_qoi <= Pecos::SMALL_NUMBER) ? 0 : std::sqrt(agg_var_qoi(qoi, step) / level_cost_vec[step]) * fact_qoi;
      }else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
        NTargetQoi(qoi, step) = (fact_qoi <= Pecos::SMALL_NUMBER) ? 0 : std::sqrt(agg_var_qoi(qoi, step) / level_cost_vec[step]) * (1./fact_qoi);
      }else{
        Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
        abort_handler(INTERFACE_ERROR);
      }

      NTargetQoi(qoi, step) = NTargetQoi(qoi, step) < 6 ? 6 : NTargetQoi(qoi, step);
      NTargetQoiFN(qoi, step) = NTargetQoi(qoi, step);
      if (outputLevel == DEBUG_OUTPUT) {
        Cout << "\t\tVar of target: " << agg_var_qoi(qoi, step) << std::endl;
        Cout << "\t\tCost: " << level_cost_vec[step] << "\n";
        Cout << "\t\tNTargetQoi: " << NTargetQoi(qoi, step) << "\n";
      }
    }
    bool have_npsol = false, have_optpp = false;
    #ifdef HAVE_NPSOL
        have_npsol = true;
    #endif
    #ifdef HAVE_OPTPP
        have_optpp = true;
    #endif
    if( (allocationTarget == TARGET_VARIANCE || allocationTarget == TARGET_SIGMA || allocationTarget == TARGET_SCALARIZATION) && (have_npsol || have_optpp) && useTargetVarianceOptimizationFlag){
      size_t qoi_copy = qoi;
      size_t qoiAggregation_copy = qoiAggregation;
      size_t numFunctions_copy = numFunctions;
      if (outputLevel == DEBUG_OUTPUT) {
        Cout << "Numerical Optimization for sample allocation targeting " << (allocationTarget == TARGET_VARIANCE ? "variance" : (allocationTarget == TARGET_SIGMA ? "sigma" : "scalarization")) << " using " << (have_npsol ? "NPSOL" : "OPTPP") << std::endl;
        Cout << "\t\t\t Convergence Target " << eps_sq_div_2 << std::endl;
      }
      RealVector initial_point, pilot_samples;
      initial_point.size(num_steps);
      pilot_samples.size(num_steps);
      for (size_t step = 0; step < num_steps; ++step) {
        pilot_samples[step] = N_l[step][qoi];
        initial_point[step] = 8. > NTargetQoi(qoi, step) ? 8 : NTargetQoi(qoi, step); 
      }

      RealVector var_lower_bnds, var_upper_bnds, lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets,
        nonlin_ineq_lower_bnds, nonlin_ineq_upper_bnds, nonlin_eq_targets;
      RealMatrix lin_ineq_coeffs, lin_eq_coeffs;

      //Bound constraints only allowing positive values for Nlq
      var_lower_bnds.size(num_steps); //init to 0
      for (size_t step = 0; step < N_l.size(); ++step) {
        var_lower_bnds[step] = 6.;
      }

      var_upper_bnds.size(num_steps); //init to 0
      var_upper_bnds.putScalar(1e10); //Set to high upper bound

      //Number of linear inequality constraints = 0
      lin_ineq_coeffs.shape(0, 0);
      lin_ineq_lower_bnds.size(0);
      lin_ineq_upper_bnds.size(0);

      //Number of linear equality constraints = 0
      lin_eq_coeffs.shape(0, 0);
      lin_eq_targets.size(0);
      //Number of nonlinear inequality bound constraints = 0
      nonlin_ineq_lower_bnds.size(0);
      nonlin_ineq_upper_bnds.size(0);
      //Number of nonlinear equality constraints = 1, s.t. c_eq: c_1(Nlq) = convergenceTol;
      nonlin_eq_targets.size(1); //init to 0
      nonlin_eq_targets[0] = eps_sq_div_2[qoi]; //convergenceTol;

      assign_static_member(nonlin_eq_targets[0], qoi_copy, qoiAggregation_copy, numFunctions_copy, level_cost_vec, sum_Ql, 
                           sum_Qlm1, sum_QlQlm1, pilot_samples, scalarizationCoeffs);

      std::unique_ptr<Iterator> optimizer;

      void (*objective_function_npsol_ptr) (int&, int&, double*, double&, double*, int&) = nullptr;
      void (*constraint_function_npsol_ptr) (int&, int&, int&, int&, int*, double*, double*, double*, int&) = nullptr;
      void (*objective_function_optpp_ptr) (int, int, const RealVector&, double&, RealVector&, int&) = nullptr;
      void (*constraint_function_optpp_ptr) (int, int, const RealVector&, RealVector&, RealMatrix&, int&) = nullptr;

      #ifdef HAVE_NPSOL
        if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
          objective_function_npsol_ptr = &target_cost_objective_eval_npsol;
          switch(allocationTarget){
            case TARGET_VARIANCE:
              constraint_function_npsol_ptr = &target_var_constraint_eval_npsol;
              break;
            case TARGET_SIGMA:
              constraint_function_npsol_ptr = &target_sigma_constraint_eval_npsol;
              break;
            case TARGET_SCALARIZATION:
              constraint_function_npsol_ptr = &target_scalarization_constraint_eval_npsol;
              break;
            default:
               break;
          }
        }else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
          constraint_function_npsol_ptr = &target_cost_constraint_eval_npsol;
          switch(allocationTarget){
            case TARGET_VARIANCE:
              objective_function_npsol_ptr = &target_var_objective_eval_npsol;
              break;
            case TARGET_SIGMA:
              objective_function_npsol_ptr = &target_sigma_objective_eval_npsol;
              break;
            case TARGET_SCALARIZATION:
              objective_function_npsol_ptr = &target_scalarization_objective_eval_npsol;
              break;
            default:
               break;
          }
        }else{
          Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
          abort_handler(INTERFACE_ERROR);
        }
        optimizer.reset(new NPSOLOptimizer(initial_point,
                                 var_lower_bnds, var_upper_bnds,
                                 lin_ineq_coeffs, lin_ineq_lower_bnds,
                                 lin_ineq_upper_bnds, lin_eq_coeffs,
                                 lin_eq_targets, nonlin_ineq_lower_bnds,
                                 nonlin_ineq_upper_bnds, nonlin_eq_targets,
                                 objective_function_npsol_ptr,
                                 constraint_function_npsol_ptr,
                                 3, 1e-15) //derivative_level = 3 means user_supplied gradients
                                 );
      #elif HAVE_OPTPP
        if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
          objective_function_optpp_ptr = &target_cost_objective_eval_optpp;
          switch(allocationTarget){
            case TARGET_VARIANCE:
              constraint_function_optpp_ptr = &target_var_constraint_eval_optpp;
              break;
            case TARGET_SIGMA:
              constraint_function_optpp_ptr = &target_sigma_constraint_eval_optpp;
              break;
            case TARGET_SCALARIZATION:
              constraint_function_optpp_ptr = &target_scalarization_constraint_eval_optpp;
              break;
            default:
               break;
          }
        }else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
          constraint_function_optpp_ptr = &target_cost_constraint_eval_optpp;
          switch(allocationTarget){
            case TARGET_VARIANCE:
              objective_function_optpp_ptr = &target_var_objective_eval_optpp;
              break;
            case TARGET_SIGMA:
              objective_function_optpp_ptr = &target_sigma_objective_eval_optpp;
              break;
            case TARGET_SCALARIZATION:
              objective_function_optpp_ptr = &target_scalarization_objective_eval_optpp;
              break;
            default:
               break;
          }
        }else{
          Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
          abort_handler(INTERFACE_ERROR);
        }
        optimizer.reset(new SNLLOptimizer(initial_point,
                                var_lower_bnds, var_upper_bnds,
                                lin_ineq_coeffs, lin_ineq_lower_bnds,
                                lin_ineq_upper_bnds, lin_eq_coeffs,
                                lin_eq_targets,nonlin_ineq_lower_bnds,
                                nonlin_ineq_upper_bnds, nonlin_eq_targets,
                                objective_function_optpp_ptr,
                                constraint_function_optpp_ptr,
                                100000, 100000, 1.e-14,
                                1.e-14, 100000)
                                );
      #endif
      optimizer->output_level(DEBUG_OUTPUT);
      optimizer->run();


      //Cout << optimizer->all_variables() << std::endl;
      if (outputLevel == DEBUG_OUTPUT) {
        Cout << "Optimization Run: Initial point: \n";
        for (int i = 0; i < initial_point.length(); ++i) {
          Cout << initial_point[i] << " ";
        }
        Cout << "\nOptimization Run. Best point: \n";
        Cout << optimizer->variables_results().continuous_variables() << std::endl;
        Cout << "Objective: " << optimizer->response_results().function_value(0) << std::endl;
        Cout << "Constraint: " << optimizer->response_results().function_value(1) << std::endl;
        Cout << "Relative Constraint violation: " << std::abs(1 - optimizer->response_results().function_value(1)/nonlin_eq_targets[0]) << std::endl;
        Cout << "\n";
      }

      if(std::abs(1. - optimizer->response_results().function_value(1)/nonlin_eq_targets[0]) > 1.0e-5){
        if (outputLevel == DEBUG_OUTPUT) Cout << "Relative Constraint violation violated: Switching to log scale " << std::endl;
        for (size_t step = 0; step < num_steps; ++step) {
          initial_point[step] = 8. > NTargetQoi(qoi, step) ? 8 : NTargetQoi(qoi, step); //optimizer->variables_results().continuous_variable(step) > pilot_samples[step] ? optimizer->variables_results().continuous_variable(step) : pilot_samples[step];
        }
        nonlin_eq_targets[0] = std::log(eps_sq_div_2[qoi]); //std::log(convergenceTol);
        #ifdef HAVE_NPSOL
          if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
          objective_function_npsol_ptr = &target_cost_objective_eval_npsol;
          switch(allocationTarget){
            case TARGET_VARIANCE:
              constraint_function_npsol_ptr = &target_var_constraint_eval_logscale_npsol;
              break;
            case TARGET_SIGMA:
              constraint_function_npsol_ptr = &target_sigma_constraint_eval_logscale_npsol;
              break;
            case TARGET_SCALARIZATION:
              constraint_function_npsol_ptr = &target_scalarization_constraint_eval_logscale_npsol;
              break;
            default:
               break;
          }
          }else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
            constraint_function_npsol_ptr = &target_cost_constraint_eval_npsol;
            switch(allocationTarget){
              case TARGET_VARIANCE:
                objective_function_npsol_ptr = &target_var_objective_eval_logscale_npsol;
                break;
              case TARGET_SIGMA:
                objective_function_npsol_ptr = &target_sigma_objective_eval_logscale_npsol;
                break;
              case TARGET_SCALARIZATION:
                objective_function_npsol_ptr = &target_scalarization_objective_eval_logscale_npsol;
                break;
              default:
                 break;
            }
          }else{
            Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
            abort_handler(INTERFACE_ERROR);
          }
          optimizer.reset(new NPSOLOptimizer(initial_point,
                                         var_lower_bnds, var_upper_bnds,
                                         lin_ineq_coeffs, lin_ineq_lower_bnds,
                                         lin_ineq_upper_bnds, lin_eq_coeffs,
                                         lin_eq_targets, nonlin_ineq_lower_bnds,
                                         nonlin_ineq_upper_bnds, nonlin_eq_targets,
                                         objective_function_npsol_ptr,
                                         constraint_function_npsol_ptr,
                                         3, 1e-15)
                                         ); //derivative_level = 3 means user_supplied gradients
        #elif HAVE_OPTPP
          if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT){
            objective_function_optpp_ptr = &target_cost_objective_eval_optpp;
            switch(allocationTarget){
              case TARGET_VARIANCE:
                constraint_function_optpp_ptr = &target_var_constraint_eval_logscale_optpp;
                break;
              case TARGET_SIGMA:
                constraint_function_optpp_ptr = &target_sigma_constraint_eval_logscale_optpp;
                break;
              case TARGET_SCALARIZATION:
                constraint_function_optpp_ptr = &target_scalarization_constraint_eval_logscale_optpp;
                break;
              default:
                 break;
            }
          }else if(convergenceTolTarget == CONVERGENCE_TOLERANCE_TARGET_COST_CONSTRAINT){
            constraint_function_optpp_ptr = &target_cost_constraint_eval_optpp;
            switch(allocationTarget){
              case TARGET_VARIANCE:
                objective_function_optpp_ptr = &target_var_objective_eval_logscale_optpp;
                break;
              case TARGET_SIGMA:
                objective_function_optpp_ptr = &target_sigma_objective_eval_logscale_optpp;
                break;
              case TARGET_SCALARIZATION:
                objective_function_optpp_ptr = &target_scalarization_objective_eval_logscale_optpp;
                break;
              default:
                 break;
            }
          }else{
            Cout << "NonDMultilevelSampling::compute_sample_allocation_target: convergenceTolTarget is not known.\n";
            abort_handler(INTERFACE_ERROR);
          }
          optimizer.reset(new SNLLOptimizer(initial_point,
                      var_lower_bnds,      var_upper_bnds,
                      lin_ineq_coeffs, lin_ineq_lower_bnds,
                      lin_ineq_upper_bnds, lin_eq_coeffs,
                      lin_eq_targets,     nonlin_ineq_lower_bnds,
                      nonlin_ineq_upper_bnds, nonlin_eq_targets,
                      objective_function_optpp_ptr,
                      constraint_function_optpp_ptr,
                      100000, 100000, 1.e-14,
                      1.e-14, 100000));
        #endif
        optimizer->run();
        if (outputLevel == DEBUG_OUTPUT) {
          Cout << "Log Optimization Run: Initial point: \n";
          for (int i = 0; i < initial_point.length(); ++i) {
            Cout << initial_point[i] << " ";
          }
          Cout << "\nLog Optimization Run. Best point: \n";
          Cout << optimizer->variables_results().continuous_variables() << std::endl;
          Cout << "Objective: " << optimizer->response_results().function_value(0) << std::endl;
          Cout << "Constraint: " << optimizer->response_results().function_value(1) << std::endl;
          Cout << "Relative Constraint violation: " << std::abs(1 - optimizer->response_results().function_value(1)/nonlin_eq_targets[0]) << std::endl;
          Cout << "\n";
        }
      }

      for (size_t step=0; step<num_steps; ++step) {
        NTargetQoi(qoi, step) = optimizer->variables_results().continuous_variable(step);
      }

      if (outputLevel == DEBUG_OUTPUT) {
        Cout << "Final Optimization results: \n";
        Cout << NTargetQoi << std::endl<< std::endl;
      }
    }
  }

  for (size_t qoi = 0; qoi < nb_aggregation_qois; ++qoi) {
    for (size_t step = 0; step < num_steps; ++step) {
        if(allocationTarget == TARGET_MEAN){
          delta_N_l_qoi(qoi, step) = one_sided_delta(N_l[step][qoi], NTargetQoi(qoi, step));
        }else if (allocationTarget == TARGET_VARIANCE || allocationTarget == TARGET_SIGMA || allocationTarget == TARGET_SCALARIZATION){
          if(max_iter==1){
            delta_N_l_qoi(qoi, step) = one_sided_delta(N_l[step][qoi], NTargetQoi(qoi, step));
          }else{
            delta_N_l_qoi(qoi, step) = std::min(N_l[step][qoi]*3, one_sided_delta(N_l[step][qoi], NTargetQoi(qoi, step)));
            delta_N_l_qoi(qoi, step) = delta_N_l_qoi(qoi, step) > 1 
                                        ?  
                                        delta_N_l_qoi(qoi, step)*underrelaxation_factor > 1 
                                          ?
                                            delta_N_l_qoi(qoi, step)*underrelaxation_factor 
                                          : 
                                            1
                                        :
                                        delta_N_l_qoi(qoi, step);
          }
        }else{
          Cout << "NonDMultilevelSampling::compute_sample_allocation_target: allocationTarget is not implemented.\n";
          abort_handler(INTERFACE_ERROR);
        }
      }
  }
  if (qoiAggregation==QOI_AGGREGATION_SUM) {
    for (size_t step = 0; step < num_steps; ++step) {
      delta_N_l[step] = delta_N_l_qoi(0, step);
    }
  }else if (qoiAggregation==QOI_AGGREGATION_MAX) {
    Real max_qoi_idx = -1, max_cost = -1, cur_cost = 0;
    for (size_t step = 0; step < num_steps; ++step) {
      max_qoi_idx = 0;
      for (size_t qoi = 1; qoi < nb_aggregation_qois; ++qoi) {
        max_qoi_idx = delta_N_l_qoi(qoi, step) > delta_N_l_qoi(max_qoi_idx, step) ? qoi : max_qoi_idx;
      }
      //max_qoi_idx = 1;
      delta_N_l[step] = delta_N_l_qoi(max_qoi_idx, step);
     }
  }else{
      Cout << "NonDMultilevelSampling::compute_sample_allocation_target: qoiAggregation is not known.\n";
      abort_handler(INTERFACE_ERROR);
  }
}

void NonDMultilevelSampling::compute_moments(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1, const IntIntPairRealMatrixMap& sum_QlQlm1, const Sizet2DArray& N_l)
{
  //RealMatrix Q_raw_mom(numFunctions, 4);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q2l = sum_Ql.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q4l = sum_Ql.at(4),
      &sum_Q1lm1 = sum_Qlm1.at(1), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3lm1 = sum_Qlm1.at(3), &sum_Q4lm1 = sum_Qlm1.at(4);
  const IntIntPair pr11(1, 1);
  Real cm1, cm2, cm3, cm4, cm1l, cm2l, cm3l, cm4l;
  const size_t num_steps = sum_Q1l.numCols();
  assert(sum_Q1l.numCols() == sum_Q2l.numCols() && sum_Q2l.numCols() == sum_Q3l.numCols()&& sum_Q3l.numCols() == sum_Q4l.numCols());

  if (momentStats.empty())
    momentStats.shapeUninitialized(4, numFunctions);
  for (size_t qoi = 0; qoi < numFunctions; ++qoi) {
    cm1 = cm2 = cm3 = cm4 = 0.;
    for (size_t step=0; step<num_steps; ++step) {
      size_t Nlq = N_l[step][qoi];
      // roll up unbiased moments centered on level mean
      uncentered_to_centered(sum_Q1l(qoi, step) / Nlq, sum_Q2l(qoi, step) / Nlq,
                             sum_Q3l(qoi, step) / Nlq, sum_Q4l(qoi, step) / Nlq,
                             cm1l, cm2l, cm3l, cm4l, Nlq);

      cm1 += cm1l;
      cm2 += cm2l;
      cm3 += cm3l;
      cm4 += cm4l;

      if (outputLevel == DEBUG_OUTPUT)
      Cout << "CM_l   for level " << step << ": "
           << cm1l << ' ' << cm2l << ' ' << cm3l << ' ' << cm4l << '\n';
      if (step) {
        uncentered_to_centered(sum_Q1lm1(qoi, step) / Nlq, sum_Q2lm1(qoi, step) / Nlq,
                               sum_Q3lm1(qoi, step) / Nlq, sum_Q4lm1(qoi, step) / Nlq,
                               cm1l, cm2l, cm3l, cm4l, Nlq);
        cm1 -= cm1l;
        cm2 -= cm2l; 
        cm3 -= cm3l;
        cm4 -= cm4l;
        if (outputLevel == DEBUG_OUTPUT)
        Cout << "CM_lm1 for level " << step << ": "
             << cm1l << ' ' << cm2l << ' ' << cm3l << ' ' << cm4l << '\n';
      }
    }
    check_negative(cm2);
    check_negative(cm4);
    Real *mom_q = momentStats[qoi];
    if (finalMomentsType == Pecos::CENTRAL_MOMENTS) {
      mom_q[0] = cm1;
      mom_q[1] = cm2;
      mom_q[2] = cm3;
      mom_q[3] = cm4;
    } else
      centered_to_standard(cm1, cm2, cm3, cm4,
                           mom_q[0], mom_q[1], mom_q[2], mom_q[3]);
  }
}

void NonDMultilevelSampling::
compute_error_estimates(const IntRealMatrixMap& sum_Ql, const IntRealMatrixMap& sum_Qlm1,
			const IntIntPairRealMatrixMap& sum_QlQlm1,
			const Sizet2DArray& num_Q) 
{
  if (!finalMomentsType)
    return;

  if (finalStatErrors.empty())
    finalStatErrors.size(finalStatistics.num_functions()); // init to 0.

  Real agg_estim_var, var_Yl, cm1l, cm2l, cm3l, cm4l, cm1lm1, cm2lm1,
      cm3lm1, cm4lm1, cm1l_sq, cm1lm1_sq, cm2l_sq, cm2lm1_sq, var_Ql, var_Qlm1,
      mu_Q2l, mu_Q2lm1, mu_Q2lQ2lm1,
      mu_Q1lm1_mu_Q2lQ1lm1, mu_Q1lm1_mu_Q1lm1_muQ2l, mu_Q1l_mu_Q1lQ2lm1, mu_Q1l_mu_Q1l_mu_Q2lm1,
      mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1, mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1, mu_Q2l_muQ2lm1, mu_Q1lQ1lm1_mu_Q1lQ1lm1,
      mu_P2lP2lm1, var_P2l, var_P2lm1, covar_P2lP2lm1, term, bessel_corr;
  size_t lev, qoi, cntr = 0, Nlq,
      num_lev = iteratedModel.truth_model().solution_levels();
  IntIntPair pr11(1, 1), pr12(1, 2), pr21(2, 1), pr22(2, 2);
  const RealMatrix &sum_Q1l = sum_Ql.at(1), &sum_Q1lm1 = sum_Qlm1.at(1),
      &sum_Q2l = sum_Ql.at(2), &sum_Q2lm1 = sum_Qlm1.at(2),
      &sum_Q3l = sum_Ql.at(3), &sum_Q3lm1 = sum_Qlm1.at(3),
      &sum_Q4l = sum_Ql.at(4), &sum_Q4lm1 = sum_Qlm1.at(4),
      &sum_Q1lQ1lm1 = sum_QlQlm1.at(pr11), &sum_Q1lQ2lm1 = sum_QlQlm1.at(pr12),
      &sum_Q2lQ1lm1 = sum_QlQlm1.at(pr21), &sum_Q2lQ2lm1 = sum_QlQlm1.at(pr22);
  for (qoi = 0; qoi < numFunctions; ++qoi) {

    // std error in mean estimate
    agg_estim_var = 0;
    for (lev = 0; lev < num_lev; ++lev) {
      Nlq = num_Q[lev][qoi];
      agg_estim_var += aggregate_variance_mean_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, num_Q, lev, qoi)/Nlq;
    }
    check_negative(agg_estim_var);

    finalStatErrors[cntr++] = std::sqrt(agg_estim_var); // std error
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Estimator SE for mean = " << finalStatErrors[cntr - 1] << "\n";
    }
    // std error in variance or std deviation estimate
    lev = 0;
    agg_estim_var = 0;
    for (lev = 0; lev < num_lev; ++lev) {
      Nlq = num_Q[lev][qoi];
      agg_estim_var += aggregate_variance_variance_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, num_Q, lev, qoi)/Nlq;
    }

    check_negative(agg_estim_var);
    if (outputLevel >= DEBUG_OUTPUT){
      Cout << "Estimator Var for variance = " << agg_estim_var << "\n";
      Cout << "Estimator SE for variance = " << sqrt(agg_estim_var) << "\n";
    }

    Real mom2 = momentStats(1, qoi);
    if (finalMomentsType == Pecos::STANDARD_MOMENTS && mom2 > 0.) {
      // std error of std deviation estimator

      // An approximation for std error of a fn of another std error estimator
      // = derivative of function * std error of the other estimator -->
      // d/dtheta of sqrt( variance(theta) ) = 1/2 variance^{-1/2} = 1/(2 stdev)
      // Note: this approx. assumes normality in the estimator distribution.
      // Harding et al. 2014 assumes normality in the QoI distribution and has
      // been observed to contain bias in numerical experiments, whereas bias
      // in the derivative approx goes to zero asymptotically.
      finalStatErrors[cntr] = std::sqrt(agg_estim_var) / (2. * mom2);
      ++cntr;
      if (outputLevel >= DEBUG_OUTPUT)
        Cout << "Estimator SE for stddev = " << finalStatErrors[cntr - 1] << "\n\n";
    } else // std error of variance estimator
      finalStatErrors[cntr++] = std::sqrt(agg_estim_var);

    // level mapping errors not implemented at this time
    cntr +=
        requestedRespLevels[qoi].length() + requestedProbLevels[qoi].length() +
        requestedRelLevels[qoi].length() + requestedGenRelLevels[qoi].length();
  }
}


static const RealVector *static_lev_cost_vec(NULL);
static size_t *static_qoi(NULL);
static const Real *static_eps_sq_div_2(NULL);
static const RealVector *static_Nlq_pilot(NULL);
static const size_t *static_numFunctions(NULL);
static const size_t  *static_qoiAggregation(NULL);

static const IntRealMatrixMap *static_sum_Ql(NULL);
static const IntRealMatrixMap *static_sum_Qlm1(NULL);
static const IntIntPairRealMatrixMap *static_sum_QlQlm1(NULL);
static const RealMatrix *static_scalarization_response_mapping(NULL);

void NonDMultilevelSampling::assign_static_member(const Real &conv_tol, size_t &qoi, const size_t &qoi_aggregation, 
              const size_t &num_functions, const RealVector &level_cost_vec,
              const IntRealMatrixMap &sum_Ql, const IntRealMatrixMap &sum_Qlm1,
              const IntIntPairRealMatrixMap &sum_QlQlm1,
              const RealVector &pilot_samples, const RealMatrix &scalarization_response_mapping) const
{
    static_lev_cost_vec= &level_cost_vec;
    static_qoi = &qoi;
    static_qoiAggregation = &qoi_aggregation;
    static_numFunctions = &num_functions;
    static_sum_Ql = &sum_Ql;
    static_sum_Qlm1 = &sum_Qlm1;
    static_sum_QlQlm1 = &sum_QlQlm1;
    static_eps_sq_div_2 = &conv_tol;
    static_Nlq_pilot = &pilot_samples;
    static_scalarization_response_mapping = &scalarization_response_mapping;
}

static const Real *static_mu_four_L(NULL);
static const Real *static_mu_four_H(NULL);
static const Real *static_var_L(NULL);
static const Real *static_var_H(NULL);
static const Real *static_Ax(NULL);

void NonDMultilevelSampling::assign_static_member_problem18(Real &var_L_exact, Real &var_H_exact, 
                                                            Real &mu_four_L_exact, Real &mu_four_H_exact, 
                                                            Real &Ax, RealVector &level_cost_vec) const
{
    static_var_L = &var_L_exact;
    static_var_H = &var_H_exact;
    static_mu_four_L = &mu_four_L_exact;
    static_mu_four_H = &mu_four_H_exact;
    static_Ax = &Ax;
    static_lev_cost_vec= &level_cost_vec;
}

void NonDMultilevelSampling::target_cost_objective_eval_npsol(int &mode, int &n, double *x, double &f, double *gradf,
                                                             int &nstate) 
{
  RealVector optpp_x;
  RealVector optpp_grad_f;
  optpp_x.size(n);
  optpp_grad_f.size(n);

  f = -1; //Dummy value

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }
  target_cost_objective_eval_optpp(mode, n, optpp_x, f, optpp_grad_f, nstate);

  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }

}

void NonDMultilevelSampling::target_cost_constraint_eval_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }
  target_cost_constraint_eval_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }

}

void
NonDMultilevelSampling::target_var_constraint_eval_npsol(int &mode, int &m, int &n, int &ldJ, int *needc, double *x,
                                                         double *g, double *grad_g, int &nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_var_constraint_eval_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }
}

void
NonDMultilevelSampling::target_var_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate)
{
  RealVector optpp_x;
  Real optpp_f;
  RealVector optpp_grad_f(n);
  optpp_x.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_var_objective_eval_optpp(mode, n, optpp_x, optpp_f, optpp_grad_f, nstate);

  f = optpp_f;
  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }
}

void
NonDMultilevelSampling::target_sigma_constraint_eval_npsol(int &mode, int &m, int &n, int &ldJ, int *needc, double *x,
                                                         double *g, double *grad_g, int &nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_sigma_constraint_eval_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }
}

void
NonDMultilevelSampling::target_sigma_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate)
{
  RealVector optpp_x;
  Real optpp_f;
  RealVector optpp_grad_f(n);
  optpp_x.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_sigma_objective_eval_optpp(mode, n, optpp_x, optpp_f, optpp_grad_f, nstate);

  f = optpp_f;
  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }
}

void
NonDMultilevelSampling::target_scalarization_constraint_eval_npsol(int &mode, int &m, int &n, int &ldJ, int *needc, double *x,
                                                         double *g, double *grad_g, int &nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_scalarization_constraint_eval_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }
}

void
NonDMultilevelSampling::target_scalarization_objective_eval_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate)
{
  RealVector optpp_x;
  Real optpp_f;
  RealVector optpp_grad_f(n);
  optpp_x.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_scalarization_objective_eval_optpp(mode, n, optpp_x, optpp_f, optpp_grad_f, nstate);

  f = optpp_f;
  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }
}

void NonDMultilevelSampling::target_var_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_var_constraint_eval_logscale_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }
}

void NonDMultilevelSampling::target_var_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate)
{
  RealVector optpp_x;
  Real optpp_f;
  RealVector optpp_grad_f(n);
  optpp_x.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_var_objective_eval_logscale_optpp(mode, n, optpp_x, optpp_f, optpp_grad_f, nstate);

  f = optpp_f;
  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }
}

void NonDMultilevelSampling::target_sigma_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_sigma_constraint_eval_logscale_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }
}

void NonDMultilevelSampling::target_sigma_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate)
{
  RealVector optpp_x;
  Real optpp_f;
  RealVector optpp_grad_f(n);
  optpp_x.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_sigma_objective_eval_logscale_optpp(mode, n, optpp_x, optpp_f, optpp_grad_f, nstate);

  f = optpp_f;
  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }
}

void NonDMultilevelSampling::target_scalarization_constraint_eval_logscale_npsol(int& mode, int& m, int& n, int& ldJ, int* needc, double* x, double* g, double* grad_g, int& nstate)
{
  RealVector optpp_x;
  RealVector optpp_g;
  RealMatrix optpp_grad_g(1, n);
  optpp_x.size(n);
  optpp_g.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_scalarization_constraint_eval_logscale_optpp(mode, n, optpp_x, optpp_g, optpp_grad_g, nstate);

  g[0] = optpp_g[0];
  for (size_t i = 0; i < n && mode; ++i) {
    grad_g[i] = optpp_grad_g[0][i];
  }
}

void NonDMultilevelSampling::target_scalarization_objective_eval_logscale_npsol(int& mode, int& n, double* x, double& f, double* gradf, int& nstate)
{
  RealVector optpp_x;
  Real optpp_f;
  RealVector optpp_grad_f(n);
  optpp_x.size(n);

  for (size_t i = 0; i < n; ++i) {
    optpp_x[i] = x[i];
  }

  target_scalarization_objective_eval_logscale_optpp(mode, n, optpp_x, optpp_f, optpp_grad_f, nstate);

  f = optpp_f;
  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
  }
}

void NonDMultilevelSampling::target_cost_objective_eval_optpp(int mode, int n, const RealVector &x, double &f,
                                                             RealVector &grad_f, int &result_mode) 
{
  f = 0;

#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction){
      result_mode = OPTPP::NLPFunction;
#endif
  for (int i = 0; i < n; ++i) {
    f += x[i] * (*static_lev_cost_vec)[i];
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif

#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPGradient){
      result_mode = OPTPP::NLPGradient;
#endif
  for (int i = 0; i < n; ++i) {
    grad_f[i] = (*static_lev_cost_vec)[i];
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif

}

void NonDMultilevelSampling::target_cost_constraint_eval_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                         RealMatrix& grad_g, int& result_mode) 
{
  g[0] = 0;

#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction){
      result_mode = OPTPP::NLPFunction;
#endif
  for (int i = 0; i < n; ++i) {
    g[0] += x[i] * (*static_lev_cost_vec)[i];
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif

#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPGradient){
      result_mode = OPTPP::NLPGradient;
#endif
  for (int i = 0; i < n; ++i) {
    grad_g[0][i] = (*static_lev_cost_vec)[i];
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif

}

void NonDMultilevelSampling::target_var_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                                      RealMatrix& grad_g, int& result_mode)
{
  Real agg_estim_var = 0;
  size_t num_lev = n;
  target_var_constraint_eval_optpp(mode, n, x, g, grad_g, result_mode);
  agg_estim_var = g[0];
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
#endif
  g[0] = std::log(g[0]); // - (*static_eps_sq_div_2);
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
  if(mode & OPTPP::NLPGradient) {
#endif
  for (size_t lev = 0; lev < num_lev; ++lev) {
    grad_g[0][lev] = grad_g[0][lev]/agg_estim_var;
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif
}

void NonDMultilevelSampling::target_var_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode)
{
  Real agg_estim_var = 0;
  size_t num_lev = n;
  target_var_objective_eval_optpp(mode, n, x, f, grad_f, result_mode);
  agg_estim_var = f;
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
#endif
  f = std::log(f);
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
  if(mode & OPTPP::NLPGradient) {
#endif
  for (size_t lev = 0; lev < num_lev; ++lev) {
    grad_f[lev] = grad_f[lev]/agg_estim_var;
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif
}

void NonDMultilevelSampling::target_sigma_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                                      RealMatrix& grad_g, int& result_mode)
{
  Real agg_estim_var = 0;
  size_t num_lev = n;
  target_sigma_constraint_eval_optpp(mode, n, x, g, grad_g, result_mode);
  agg_estim_var = g[0];
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
#endif
  g[0] = std::log(g[0]); 
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
  if(mode & OPTPP::NLPGradient) {
#endif
  for (size_t lev = 0; lev < num_lev; ++lev) {
    grad_g[0][lev] = grad_g[0][lev]/agg_estim_var;
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif
}


void NonDMultilevelSampling::target_sigma_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode)
{
  Real agg_estim_var = 0;
  size_t num_lev = n;
  target_sigma_objective_eval_optpp(mode, n, x, f, grad_f, result_mode);
  agg_estim_var = f;
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
#endif
  f = std::log(f); 
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
  if(mode & OPTPP::NLPGradient) {
#endif
  for (size_t lev = 0; lev < num_lev; ++lev) {
    grad_f[lev] = grad_f[lev]/agg_estim_var;
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif
}

void NonDMultilevelSampling::target_scalarization_constraint_eval_logscale_optpp(int mode, int n, const RealVector& x, RealVector& g,
                                                      RealMatrix& grad_g, int& result_mode)
{
  Real agg_estim_var = 0;
  size_t num_lev = n;
  target_scalarization_constraint_eval_optpp(mode, n, x, g, grad_g, result_mode);
  agg_estim_var = g[0];
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
#endif
  g[0] = std::log(g[0]);
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
  if(mode & OPTPP::NLPGradient) {
#endif
  for (size_t lev = 0; lev < num_lev; ++lev) {
    grad_g[0][lev] = grad_g[0][lev]/agg_estim_var;
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif
}

void NonDMultilevelSampling::target_scalarization_objective_eval_logscale_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode)
{
  Real agg_estim_var = 0;
  size_t num_lev = n;
  target_scalarization_objective_eval_optpp(mode, n, x, f, grad_f, result_mode);
  agg_estim_var = f;
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
#endif
  f = std::log(f);
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
  if(mode & OPTPP::NLPGradient) {
#endif
  for (size_t lev = 0; lev < num_lev; ++lev) {
    grad_f[lev] = grad_f[lev]/agg_estim_var;
  }
#ifdef HAVE_NPSOL
#elif HAVE_OPTPP
  }
#endif
}

void NonDMultilevelSampling::target_var_constraint_eval_optpp(int mode, int n, const RealVector &x, RealVector &g,
                                                              RealMatrix &grad_g, int &result_mode) 
{
  bool compute_gradient = false;
#ifdef HAVE_NPSOL
  compute_gradient = mode; //if mode == 0, NPSOL ignores gradients
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
    result_mode = OPTPP::NLPFunction;
  }
  if(mode & OPTPP::NLPGradient){
    compute_gradient = true;
    result_mode = OPTPP::NLPGradient;
  }
#endif
  double g_to_f = 0;
  RealVector grad_g_to_grad_f;
  grad_g_to_grad_f.resize(n);
  target_var_objective_eval_optpp(mode, n, x, g_to_f, grad_g_to_grad_f, result_mode);
  g[0] = g_to_f;
  if(compute_gradient){
    for (size_t lev = 0; lev < n; ++lev) {
      grad_g[0][lev] = grad_g_to_grad_f[lev];
    }
  }
}

void NonDMultilevelSampling::target_var_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode)
{
  bool compute_gradient = false;
#ifdef HAVE_NPSOL
  compute_gradient = mode; //if mode == 0, NPSOL ignores gradients
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
    result_mode = OPTPP::NLPFunction;
  }
  if(mode & OPTPP::NLPGradient){
    compute_gradient = true;
    result_mode = OPTPP::NLPGradient;
  }
#endif

  // std error in variance or std deviation estimate
  size_t lev = 0;
  Real Nlq = x[lev];
  size_t Nlq_pilot = (*static_Nlq_pilot)[lev];
  size_t qoi = *static_qoi;
  size_t nb_qois = *static_numFunctions;
  short  qoiAggregation = *static_qoiAggregation;
  size_t num_lev = n;
  RealVector agg_estim_var_l;
  agg_estim_var_l.size(num_lev);
  Real agg_estim_var = 0;

  if (qoiAggregation==QOI_AGGREGATION_SUM) {
    agg_estim_var_l[0] = 0;
    for(size_t qoi = 0; qoi < nb_qois; ++qoi){
      agg_estim_var_l[0] = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi,
                                       compute_gradient, grad_f[0]);
      agg_estim_var += agg_estim_var_l[0];
      for (lev = 1; lev < num_lev; ++lev) {
        Nlq = x[lev];
        Nlq_pilot = (*static_Nlq_pilot)[lev];

        agg_estim_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                         compute_gradient, grad_f[lev]);
        agg_estim_var += agg_estim_var_l[lev];
      }
    }
  }
  else if(qoiAggregation==QOI_AGGREGATION_MAX){
    agg_estim_var_l[0] = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi,
                                   compute_gradient, grad_f[0]);
    agg_estim_var += agg_estim_var_l[0];
    for (lev = 1; lev < num_lev; ++lev) {
      Nlq = x[lev];
      Nlq_pilot = (*static_Nlq_pilot)[lev];

      agg_estim_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                       compute_gradient, grad_f[lev]);
      agg_estim_var += agg_estim_var_l[lev];
    }
  }else{
    Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: qoiAggregation is not known.\n";
    abort_handler(INTERFACE_ERROR);
  }

  f = agg_estim_var; 
}

void NonDMultilevelSampling::target_sigma_constraint_eval_optpp(int mode, int n, const RealVector &x, RealVector &g,
                                                              RealMatrix &grad_g, int &result_mode) 
{

  bool compute_gradient = false;
  #ifdef HAVE_NPSOL
    compute_gradient = mode; //if mode == 0, NPSOL ignores gradients
  #elif HAVE_OPTPP
    if(mode & OPTPP::NLPFunction) {
      result_mode = OPTPP::NLPFunction;
    }
    if(mode & OPTPP::NLPGradient){
      compute_gradient = true;
      result_mode = OPTPP::NLPGradient;
    }
  #endif

  double g_to_f = 0;
  RealVector grad_g_to_grad_f;
  grad_g_to_grad_f.resize(n);
  target_sigma_objective_eval_optpp(mode, n, x, g_to_f, grad_g_to_grad_f, result_mode);
  g[0] = g_to_f;
  if(compute_gradient){
    for (size_t lev = 0; lev < n; ++lev) {
      grad_g[0][lev] = grad_g_to_grad_f[lev];
    }
  }
}

void NonDMultilevelSampling::target_sigma_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode)
{

  bool compute_gradient = false;
#ifdef HAVE_NPSOL
  compute_gradient = mode; //if mode == 0, NPSOL ignores gradients
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
    result_mode = OPTPP::NLPFunction;
  }
  if(mode & OPTPP::NLPGradient){
    compute_gradient = true;
    result_mode = OPTPP::NLPGradient;
  }
#endif

  // std error in variance or std deviation estimate
  size_t lev = 0;
  Real Nlq = x[lev];
  size_t Nlq_pilot = (*static_Nlq_pilot)[lev];
  size_t qoi = *static_qoi;
  size_t nb_qois = *static_numFunctions;
  short  qoiAggregation = *static_qoiAggregation;
  size_t num_lev = n;
  RealVector agg_estim_var_of_var_l, agg_estim_var_l, grad_var, grad_var_var;
  agg_estim_var_of_var_l.size(num_lev);
  agg_estim_var_l.size(num_lev);
  grad_var.size(num_lev);
  grad_var_var.size(num_lev);
  RealVector grad_test;
  grad_test.size(num_lev);
  Real agg_estim_var_of_var = 0;
  Real agg_estim_var = 0;
  Real gradient_var = 0;
  Real gradient_var_var = 0;

  IntIntPair pr11(1, 1);

  if (qoiAggregation==QOI_AGGREGATION_SUM) {
    //safe initialization
    for (lev = 0; lev < num_lev; ++lev) {
      agg_estim_var_of_var_l[lev] = 0;
      agg_estim_var_l[lev] = 0;
    }
    for(size_t qoi = 0; qoi < nb_qois; ++qoi){
      agg_estim_var_of_var_l[0] = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi,
                                       compute_gradient, gradient_var_var);
      agg_estim_var_of_var += agg_estim_var_of_var_l[0];

      agg_estim_var_l[0] = variance_Ysum_static((*static_sum_Ql).at(1)[0][qoi], (*static_sum_Ql).at(1)[0][qoi], Nlq_pilot, Nlq,
                                       compute_gradient, gradient_var); 
      agg_estim_var += agg_estim_var_l[0];

      if(compute_gradient){
      grad_f[0] += agg_estim_var_l[0] > 0 ? 1./4. * 
                    ( 
                      -1./(agg_estim_var_l[0] * agg_estim_var_l[0]) * gradient_var * agg_estim_var_of_var_l[0] 
                      +
                      1./agg_estim_var_l[0] * gradient_var_var
                    ) : 0;
      }

      for (lev = 1; lev < num_lev; ++lev) {
        Nlq = x[lev];
        Nlq_pilot = (*static_Nlq_pilot)[lev];

        agg_estim_var_of_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                         compute_gradient, gradient_var_var);
        agg_estim_var_of_var += agg_estim_var_of_var_l[lev];

        agg_estim_var_l[lev] = var_lev_l_static((*static_sum_Ql).at(1)[lev][qoi], (*static_sum_Qlm1).at(1)[lev][qoi], (*static_sum_Ql).at(2)[lev][qoi], 
                    (*static_sum_Qlm1).at(2)[lev][qoi], Nlq_pilot, Nlq, compute_gradient, gradient_var);
        agg_estim_var += agg_estim_var_l[lev];

        if(compute_gradient){
          grad_f[lev] += agg_estim_var_l[0] > 0 ? 1./4. * 
                        ( 
                          -1./(agg_estim_var_l[lev] * agg_estim_var_l[lev]) * gradient_var * agg_estim_var_of_var_l[lev] 
                          +
                          1./agg_estim_var_l[lev] * gradient_var_var
                        ) : 0;
        }
      }
    }
  }else if(qoiAggregation==QOI_AGGREGATION_MAX){
    agg_estim_var_of_var_l[0] = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi,
                                   compute_gradient, gradient_var_var);
    agg_estim_var_of_var += agg_estim_var_of_var_l[0];
    grad_var_var[0] = gradient_var_var;

    agg_estim_var_l[0] = variance_Ysum_static((*static_sum_Ql).at(1)[0][qoi], (*static_sum_Ql).at(2)[0][qoi], Nlq_pilot, Nlq,
                                       compute_gradient, gradient_var); 
    grad_var[0] = gradient_var;

    agg_estim_var += agg_estim_var_l[0];

    for (lev = 1; lev < num_lev; ++lev) {
      Nlq = x[lev];
      Nlq_pilot = (*static_Nlq_pilot)[lev];

      agg_estim_var_of_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                       compute_gradient, gradient_var_var);

      agg_estim_var_of_var += agg_estim_var_of_var_l[lev];

      grad_var_var[lev] = gradient_var_var;

      agg_estim_var_l[lev] = var_lev_l_static((*static_sum_Ql).at(1)[lev][qoi], (*static_sum_Qlm1).at(1)[lev][qoi], (*static_sum_Ql).at(2)[lev][qoi], 
                    (*static_sum_Qlm1).at(2)[lev][qoi], Nlq_pilot, Nlq, compute_gradient, gradient_var);
      agg_estim_var += agg_estim_var_l[lev];
      grad_var[lev] = gradient_var;
    }
    if(compute_gradient){
      for (lev = 0; lev < num_lev; ++lev) {
        grad_f[lev] = agg_estim_var > 0 ? 
                          1./4. * ( (grad_var_var[lev] * agg_estim_var - agg_estim_var_of_var * grad_var[lev])/(agg_estim_var*agg_estim_var) ) 
                          : 0;
      }
    }
  }else{
    Cout << "NonDMultilevelSampling::target_sigma_objective_eval_optpp: qoiAggregation is not known.\n";
    abort_handler(INTERFACE_ERROR);
  }

  f = agg_estim_var > 0 ? 1./4. * agg_estim_var_of_var/agg_estim_var : 0; // - (*static_eps_sq_div_2);
}

void NonDMultilevelSampling::target_scalarization_constraint_eval_optpp(int mode, int n, const RealVector &x, RealVector &g,
                                                              RealMatrix &grad_g, int &result_mode) 
{
  bool compute_gradient = false;
  #ifdef HAVE_NPSOL
    compute_gradient = mode; //if mode == 0, NPSOL ignores gradients
  #elif HAVE_OPTPP
    if(mode & OPTPP::NLPFunction) {
      result_mode = OPTPP::NLPFunction;
    }
    if(mode & OPTPP::NLPGradient){
      compute_gradient = true;
      result_mode = OPTPP::NLPGradient;
    }
  #endif

  double g_to_f = 0;
  RealVector grad_g_to_grad_f;
  grad_g_to_grad_f.resize(n);
  target_scalarization_objective_eval_optpp(mode, n, x, g_to_f, grad_g_to_grad_f, result_mode);
  g[0] = g_to_f;
  if(compute_gradient){
    for (size_t lev = 0; lev < n; ++lev) {
      grad_g[0][lev] = grad_g_to_grad_f[lev];
    }
  }
}

void NonDMultilevelSampling::target_scalarization_objective_eval_optpp(int mode, int n, const RealVector& x, double& f,
                                        RealVector& grad_f, int& result_mode)
{

  bool compute_gradient = false;
#ifdef HAVE_NPSOL
  compute_gradient = mode; //if mode == 0, NPSOL ignores gradients
#elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
    result_mode = OPTPP::NLPFunction;
  }
  if(mode & OPTPP::NLPGradient){
    compute_gradient = true;
    result_mode = OPTPP::NLPGradient;
  }
#endif

  // std error in variance or std deviation estimate
  size_t lev, Nlq_pilot;
  Real Nlq;
  size_t qoi = *static_qoi;
  size_t nb_qois = *static_numFunctions;
  short  qoiAggregation = *static_qoiAggregation;
  size_t num_lev = n;
  IntIntPair pr11(1, 1);
  
  f = 0;
  for (lev = 0; lev < num_lev; ++lev) {
    grad_f[lev] = 0;
  }
  size_t cur_qoi_offset = 0;
  for(size_t cur_qoi = 0; cur_qoi < nb_qois; ++cur_qoi){
    cur_qoi_offset = cur_qoi*2;

    //Mean
    Real f_mean = 0;
    Real f_var = 0;
    Real cur_grad_var = 0;
    RealVector grad_f_mean;
    grad_f_mean.resize(num_lev);
    if (qoiAggregation==QOI_AGGREGATION_SUM) {
      for(size_t sum_qoi = 0; sum_qoi < nb_qois; ++sum_qoi){
        for (lev = 0; lev < num_lev; ++lev) {
          Nlq = x[lev];
          Nlq_pilot = (*static_Nlq_pilot)[lev];

          f_var = (lev == 0) ? variance_Ysum_static((*static_sum_Ql).at(1)[0][sum_qoi], (*static_sum_Ql).at(2)[0][sum_qoi], Nlq_pilot, Nlq,
                                         compute_gradient, cur_grad_var) :
                      variance_Qsum_static((*static_sum_Ql).at(1)[lev][sum_qoi], (*static_sum_Qlm1).at(1)[lev][sum_qoi], (*static_sum_Ql).at(2)[lev][sum_qoi], 
                        (*static_sum_QlQlm1).at(pr11)[lev][sum_qoi], (*static_sum_Qlm1).at(2)[lev][sum_qoi], Nlq_pilot, Nlq, compute_gradient, cur_grad_var);
          
          f_mean += f_var/Nlq;

          if(compute_gradient){
            grad_f_mean[lev] += (Nlq * cur_grad_var - f_var)/(Nlq*Nlq);
          }
        }
      }
    }else if(qoiAggregation==QOI_AGGREGATION_MAX){
      for (lev = 0; lev < num_lev; ++lev) {
        Nlq = x[lev];
        Nlq_pilot = (*static_Nlq_pilot)[lev];
        f_var = (lev == 0) ? variance_Ysum_static((*static_sum_Ql).at(1)[0][cur_qoi], (*static_sum_Ql).at(2)[0][cur_qoi], Nlq_pilot, Nlq,
                                         compute_gradient, cur_grad_var) :
                             variance_Qsum_static((*static_sum_Ql).at(1)[lev][cur_qoi], (*static_sum_Qlm1).at(1)[lev][cur_qoi], (*static_sum_Ql).at(2)[lev][cur_qoi], 
                        (*static_sum_QlQlm1).at(pr11)[lev][cur_qoi], (*static_sum_Qlm1).at(2)[lev][cur_qoi], Nlq_pilot, Nlq, compute_gradient, cur_grad_var);
        f_mean += f_var/Nlq;
        
        if(compute_gradient){
          grad_f_mean[lev] = (Nlq * cur_grad_var - f_var)/(Nlq*Nlq);
        }
      }
    }else{
      Cout << "NonDMultilevelSampling::target_scalarization_objective_eval_optpp: qoiAggregation is not known.\n";
      abort_handler(INTERFACE_ERROR);
    }

    //Sigma
    Real f_sigma = 0;
    RealVector grad_f_sigma;
    grad_f_sigma.resize(num_lev);
    for (lev = 0; lev < num_lev; ++lev) {
      grad_f_sigma[lev] = 0;
    }
    *static_qoi = cur_qoi;
    target_sigma_objective_eval_optpp(mode, n, x, f_sigma, grad_f_sigma, result_mode);
    *static_qoi = qoi; //Reset pointer

    //Cov
    Real f_upper_bound_cov = std::sqrt(f_mean * f_sigma);
    RealVector grad_f_upper_bound_cov;
    grad_f_upper_bound_cov.resize(num_lev);
    if(compute_gradient){
      for (lev = 0; lev < num_lev; ++lev) {
        grad_f_upper_bound_cov[lev] = f_upper_bound_cov > 0. ? 1./(2.*f_upper_bound_cov)*(grad_f_mean[lev]*f_sigma + f_mean*grad_f_sigma[lev]) : 0;
      }
    }

    Real cov_scaling = 1.0;
    Real f_tmp = (*static_scalarization_response_mapping)(qoi, cur_qoi_offset) * (*static_scalarization_response_mapping)(qoi, cur_qoi_offset) * f_mean 
          + (*static_scalarization_response_mapping)(qoi, cur_qoi_offset+1) * (*static_scalarization_response_mapping)(qoi, cur_qoi_offset+1) * f_sigma 
          + cov_scaling * 2.0 * std::abs((*static_scalarization_response_mapping)(qoi, cur_qoi_offset)) * std::abs((*static_scalarization_response_mapping)(qoi, cur_qoi_offset+1)) * f_upper_bound_cov;
    f += f_tmp > 0 ? f_tmp : 0;
    if(compute_gradient){
      for (lev = 0; lev < num_lev; ++lev) {
        grad_f[lev] += (*static_scalarization_response_mapping)(qoi, cur_qoi_offset) * (*static_scalarization_response_mapping)(qoi, cur_qoi_offset) * grad_f_mean[lev] 
          + (*static_scalarization_response_mapping)(qoi, cur_qoi_offset+1) * (*static_scalarization_response_mapping)(qoi, cur_qoi_offset+1) * grad_f_sigma[lev] 
          + cov_scaling * 2.0 * std::abs((*static_scalarization_response_mapping)(qoi, cur_qoi_offset)) * std::abs((*static_scalarization_response_mapping)(qoi, cur_qoi_offset+1)) * grad_f_upper_bound_cov[lev] ;
      }
    }
  }   
}

void NonDMultilevelSampling::target_var_constraint_eval_optpp_problem18(int mode, int n, const RealVector &x, RealVector &g,
                                                              RealMatrix &grad_g, int &result_mode) 
{
  bool compute_gradient = false;

  #ifdef HAVE_NPSOL
  #elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
    result_mode = OPTPP::NLPFunction;
  }
  if(mode & OPTPP::NLPGradient){
    compute_gradient = true;
    result_mode = OPTPP::NLPGradient;
  }
  #endif

  Real Hl1_deriv, Hl2_deriv;

  Real Ax = *static_Ax;
  Real var_L = *static_var_L;
  Real var_H = *static_var_H;
  Real mu_four_L = *static_mu_four_L;
  Real mu_four_H = *static_mu_four_H;

  //Level 0
  size_t lev = 0;
  Real Nlq = x[lev];
  Real C1 = mu_four_L;
  Real C2 = var_L * var_L;
  Real Hl1 = 1./Nlq;
  Real Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Cost1L = 0;
  Real Cost2L = 0;
  Real var_of_var_l0 = Hl1 * C1 - Hl2 * C2;
  if(compute_gradient){
    Hl1_deriv = -1./(Nlq*Nlq);
    Hl2_deriv = ( (Nlq * (Nlq - 1.)) - (Nlq - 3.)*(2. * Nlq - 1) ) / ( std::pow(Nlq * (Nlq - 1.), 2) );
    grad_g[0][lev] = 1./var_of_var_l0 *
        ( Hl1_deriv * C1 - Hl2_deriv * C2 );
  }

  //Level 1
  lev = 1;
  Nlq = x[lev];
  C1 = mu_four_H + mu_four_L;
  C2 = var_H * var_H + var_L * var_L;
  Hl1 = 1./Nlq;
  Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Ax_squared = Ax * Ax;
  Real var_H_squared = var_H * var_H;
  Real Cost1H = std::pow(0.5, 12)/13. * Ax_squared - Ax_squared * var_H_squared;
  Real Cost2H = Ax_squared * var_H_squared;
  Real var_of_var_l1 = Hl1 * C1 - Hl2 * C2 - 2. * Cost1H / Nlq - 2. * Cost2H / (Nlq * (Nlq - 1.));
  if(compute_gradient){
    Hl1_deriv = -1./(Nlq*Nlq);
    Hl2_deriv = ( (Nlq * (Nlq - 1.)) - (Nlq - 3.)*(2. * Nlq - 1) ) / ( (Nlq * (Nlq - 1.))*(Nlq * (Nlq - 1.)) );
    grad_g[0][lev] = 1./var_of_var_l1 *
                     ( Hl1_deriv * C1 - Hl2_deriv * C2
                        - 2. * Cost1H * (-1/(Nlq*Nlq))
                        - 2. * Cost2H * (1 - 2*Nlq ) / (std::pow(Nlq * (Nlq - 1.), 2)) );
  }

  //Final var
  Real var_of_var_ml = var_of_var_l0 + var_of_var_l1;
  g[0] = std::log(var_of_var_ml);
}

void NonDMultilevelSampling::target_sigma_constraint_eval_optpp_problem18(int mode, int n, const RealVector &x, RealVector &g,
                                                              RealMatrix &grad_g, int &result_mode) 
{
  bool compute_gradient = false;

  #ifdef HAVE_NPSOL
  #elif HAVE_OPTPP
  if(mode & OPTPP::NLPFunction) {
    result_mode = OPTPP::NLPFunction;
  }
  if(mode & OPTPP::NLPGradient){
    compute_gradient = true;
    result_mode = OPTPP::NLPGradient;
  }
  #endif

  Real Hl1_deriv, Hl2_deriv;

  Real Ax = *static_Ax;
  Real var_L = *static_var_L;
  Real var_H = *static_var_H;
  Real mu_four_L = *static_mu_four_L;
  Real mu_four_H = *static_mu_four_H;

  //Level 0
  size_t lev = 0;
  Real Nlq = x[lev];
  Real C1 = mu_four_L;
  Real C2 = var_L * var_L;
  Real Hl1 = 1./Nlq;
  Real Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Cost1L = 0;
  Real Cost2L = 0;
  Real var_of_var_l0 = Hl1 * C1 - Hl2 * C2;
  if(compute_gradient){
    Hl1_deriv = -1./(Nlq*Nlq);
    Hl2_deriv = ( (Nlq * (Nlq - 1.)) - (Nlq - 3.)*(2. * Nlq - 1) ) / ( std::pow(Nlq * (Nlq - 1.), 2) );
    grad_g[0][lev] = 1./(4.*var_H*var_H) * ((Hl1_deriv * C1 - Hl2_deriv * C2 ) * var_H);
  }

  //Level 1
  lev = 1;
  Nlq = x[lev];
  C1 = mu_four_H + mu_four_L;
  C2 = var_H * var_H + var_L * var_L;
  Hl1 = 1./Nlq;
  Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Ax_squared = Ax * Ax;
  Real var_H_squared = var_H * var_H;
  Real Cost1H = std::pow(0.5, 12)/13. * Ax_squared - Ax_squared * var_H_squared;
  Real Cost2H = Ax_squared * var_H_squared;
  Real var_of_var_l1 = Hl1 * C1 - Hl2 * C2 - 2. * Cost1H / Nlq - 2. * Cost2H / (Nlq * (Nlq - 1.));
  if(compute_gradient){
    Hl1_deriv = -1./(Nlq*Nlq);
    Hl2_deriv = ( (Nlq * (Nlq - 1.)) - (Nlq - 3.)*(2. * Nlq - 1) ) / ( (Nlq * (Nlq - 1.))*(Nlq * (Nlq - 1.)) );
    grad_g[0][lev] = 1./(4.*var_H*var_H) * ( Hl1_deriv * C1 - Hl2_deriv * C2
                        - 2. * Cost1H * (-1/(Nlq*Nlq))
                        - 2. * Cost2H * (1 - 2*Nlq ) / (std::pow(Nlq * (Nlq - 1.), 2)) * var_H );   
  }
  //Final var
  Real var_of_var_ml = var_of_var_l0 + var_of_var_l1;
  g[0] = (1./(4. * var_H ) * var_of_var_ml);
}

double NonDMultilevelSampling::exact_var_of_var_problem18(const RealVector &Nl) {

  Real Ax = *static_Ax;
  Real var_L = *static_var_L;
  Real var_H = *static_var_H;
  Real mu_four_L = *static_mu_four_L;
  Real mu_four_H = *static_mu_four_H;

  //Level 0
  size_t lev = 0;
  Real Nlq = Nl[lev];
  Real C1 = mu_four_L;
  Real C2 = var_L * var_L;
  Real Hl1 = 1./Nlq;
  Real Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Cost1L = 0;
  Real Cost2L = 0;
  Real var_of_var_l0 = Hl1 * C1 - Hl2 * C2;

  //Level 1
  lev = 1;
  Nlq = Nl[lev];
  C1 = mu_four_H + mu_four_L;
  C2 = var_H * var_H + var_L * var_L;
  Hl1 = 1./Nlq;
  Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Ax_squared = Ax * Ax;
  Real var_H_squared = var_H * var_H;
  Real Cost1H = std::pow(0.5, 12)/13. * Ax_squared - Ax_squared * var_H_squared;
  Real Cost2H = Ax_squared * var_H_squared;
  Real var_of_var_l1 = Hl1 * C1 - Hl2 * C2 - 2. * Cost1H / Nlq - 2. * Cost2H / (Nlq * (Nlq - 1.));

  //Final var
  Real var_of_var_ml = var_of_var_l0 + var_of_var_l1;
  return var_of_var_ml;
 }

 double NonDMultilevelSampling::exact_var_of_sigma_problem18(const RealVector &Nl) {
  Real Ax = *static_Ax;
  Real var_L = *static_var_L;
  Real var_H = *static_var_H;
  Real mu_four_L = *static_mu_four_L;
  Real mu_four_H = *static_mu_four_H;

  //Level 0
  size_t lev = 0;
  Real Nlq = Nl[lev];
  Real C1 = mu_four_L;
  Real C2 = var_L * var_L;
  Real Hl1 = 1./Nlq;
  Real Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Cost1L = 0;
  Real Cost2L = 0;
  Real var_of_var_l0 = Hl1 * C1 - Hl2 * C2;

  //Level 1
  lev = 1;
  Nlq = Nl[lev];
  C1 = mu_four_H + mu_four_L;
  C2 = var_H * var_H + var_L * var_L;
  Hl1 = 1./Nlq;
  Hl2 = (Nlq - 3.)/(Nlq * (Nlq - 1.));
  Real Ax_squared = Ax * Ax;
  Real var_H_squared = var_H * var_H;
  Real Cost1H = std::pow(0.5, 12)/13. * Ax_squared - Ax_squared * var_H_squared;
  Real Cost2H = Ax_squared * var_H_squared;
  Real var_of_var_l1 = Hl1 * C1 - Hl2 * C2 - 2. * Cost1H / Nlq - 2. * Cost2H / (Nlq * (Nlq - 1.));

  //Final var
  Real var_of_var_ml = var_of_var_l0 + var_of_var_l1;
  return (1./(4. * var_H ) * var_of_var_ml);
 }

} // namespace Dakota
