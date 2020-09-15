/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
#include "DiscrepancyCalculator.hpp"
#include "DakotaIterator.hpp"

static const char rcsId[]="@(#) $Id: NonDMultilevelSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";


namespace Dakota {

/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDMultilevelSampling::
NonDMultilevelSampling(ProblemDescDB& problem_db, Model& model):
  NonDSampling(problem_db, model),
  pilotSamples(problem_db.get_sza("method.nond.pilot_samples")),
  randomSeedSeqSpec(problem_db.get_sza("method.random_seed_sequence")),
  mlmfIter(0),
  allocationTarget(problem_db.get_short("method.nond.allocation_target")),
  qoiAggregation(problem_db.get_short("method.nond.qoi_aggregation")),
  convergenceTolType(problem_db.get_short("method.nond.convergence_tolerance_type")),
  useTargetVarianceOptimizationFlag(problem_db.get_bool("method.nond.allocation_target.variance.optimization")),
  finalCVRefinement(true),
  exportSampleSets(problem_db.get_bool("method.nond.export_sample_sequence")),
  exportSamplesFormat(
    problem_db.get_ushort("method.nond.export_samples_format"))
{
  // initialize scalars from sequence
  seedSpec = randomSeed = random_seed(0);

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
    aggregated_models_mode();
  else {
    Cerr << "Error: Multilevel Monte Carlo requires a hierarchical "
	 << "surrogate model specification." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  ModelList& ordered_models = iteratedModel.subordinate_models(false);
  size_t i, j, num_mf = ordered_models.size(), num_lev,
    prev_lev = std::numeric_limits<size_t>::max(),
    pilot_size = pilotSamples.size();
  ModelLRevIter ml_rit; bool err_flag = false;
  NLev.resize(num_mf);
  for (i=num_mf-1, ml_rit=ordered_models.rbegin();
       ml_rit!=ordered_models.rend(); --i, ++ml_rit) { // high fid to low fid
    // for now, only SimulationModel supports solution_{levels,costs}()
    num_lev = ml_rit->solution_levels(); // lower bound is 1 soln level

    if (num_lev > prev_lev) {
      Cerr << "\nWarning: unused solution levels in multilevel sampling for "
	   << "model " << ml_rit->model_id() << ".\n         Ignoring "
	   << num_lev - prev_lev << " of " << num_lev << " levels."<< std::endl;
      num_lev = prev_lev;
    }

    // Ensure there is consistent cost data available as SimulationModel must
    // be allowed to have empty solnCntlCostMap (when optional solution control
    // is not specified).  Passing false bypasses lower bound of 1.
    if (num_lev > ml_rit->solution_levels(false)) { // default is 0 soln costs
      Cerr << "Error: insufficient cost data provided for multilevel sampling."
	   << "\n       Please provide solution_level_cost estimates for model "
	   << ml_rit->model_id() << '.' << std::endl;
      err_flag = true;
    }

    //Sizet2DArray& Nl_i = NLev[i];
    NLev[i].resize(num_lev); //Nl_i.resize(num_lev);
    //for (j=0; j<num_lev; ++j)
    //  Nl_i[j].resize(numFunctions); // defer to pre_run()

    prev_lev = num_lev;
  }
  if (err_flag)
    abort_handler(METHOD_ERROR);

  if( !std::all_of( std::begin(pilotSamples), std::end(pilotSamples), [](int i){ return i > 0; }) ){
    Cerr << "\nError: Some levels have pilot samples of size 0 in "
       << method_enum_to_string(methodName) << "." << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  switch (pilot_size) {
    case 0: maxEvalConcurrency *= 100;             break;
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

  max_iter = (maxIterations < 0) ? 25 : maxIterations; // default = -1
  // For testing multilevel_mc_Qsum():
  //subIteratorFlag = true;
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
  NonDSampling::pre_run();

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

  // For two-model control variate methods, select lowest,highest fidelities
  size_t num_mf = NLev.size();
  unsigned short lf_form = 0, hf_form = num_mf - 1; // ordered_models = low:high
  if (num_mf > 1) {
    size_t num_hf_lev = NLev.back().size();
    if (num_hf_lev > 1) { // ML performed on HF with CV using available LF
      // multiple model forms + multiple solutions levels --> perform MLMC on
      // HF model and bind 1:min(num_hf,num_lf) LF control variates starting
      // at coarsest level (TO DO: validate case of unequal levels)
      if (true) // reformulated approach using 1 new QoI correlation per level
	multilevel_control_variate_mc_Qcorr(lf_form, hf_form);
      else      // original approach using 1 discrepancy correlation per level
	multilevel_control_variate_mc_Ycorr(lf_form, hf_form);
    }
    else { // multiple model forms (only) --> CVMC
      // use nominal value from user input, ignoring solution_level_control
      UShortArray hf_lf_key;  unsigned short lev = USHRT_MAX;
      Pecos::DiscrepancyCalculator::
	form_key(0, hf_form, lev, lf_form, lev, hf_lf_key);
      control_variate_mc(hf_lf_key);
    }
  }
  else { // multiple solutions levels (only) --> traditional ML-MC
    if (true) //(subIteratorFlag)
      multilevel_mc_Qsum(hf_form); // w/ error est, unbiased central moments
    else
      multilevel_mc_Ysum(hf_form); // lighter weight
  }
}


/** This function performs "geometrical" MLMC on a single model form
    with multiple discretization levels. */
void NonDMultilevelSampling::multilevel_mc_Ysum(unsigned short model_form)
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

  // assign truth model form (solution level assignment is deferred until loop)
  UShortArray truth_key;
  unsigned short seq_index = 2, lev = USHRT_MAX; // lev updated in loop below
  Pecos::DiscrepancyCalculator::form_key(0, model_form, lev, truth_key);
  iteratedModel.active_model_key(truth_key);
  Model& truth_model = iteratedModel.truth_model();

  size_t qoi, num_steps = truth_model.solution_levels();// 1 model form
  unsigned short& step = (true) ? lev : model_form; // option not active
  Real eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0., lev_cost;
  // retrieve cost estimates across soln levels for a particular model form
  RealVector cost = truth_model.solution_level_costs(), agg_var(num_steps);
  // For moment estimation, we accumulate telescoping sums for Q^i using
  // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (sum_Y[i] for i=1:4).
  // For computing N_l from estimator variance, we accumulate square of Y1
  // estimator (YY[i] = (Y^i)^2 for i=1).
  IntRealMatrixMap sum_Y; RealMatrix sum_YY(numFunctions, num_steps);
  initialize_ml_Ysums(sum_Y, num_steps);

  // Initialize for pilot sample
  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, NLev, delta_N_l);

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_l(num_steps, 0);
  RealVectorArray mu_hat(num_steps);
  Sizet2DArray& N_l = NLev[model_form];

  // now converge on sample counts per level (N_l)
  mlmfIter = 0;
  while (Pecos::l1_norm(delta_N_l) && mlmfIter <= max_iter) {

    sum_sqrt_var_cost = 0.;
    for (step=0; step<num_steps; ++step) { // step is reference to lev

      configure_indices(step, model_form, lev, seq_index);
      lev_cost = level_cost(cost, step);

      // set the number of current samples from the defined increment
      numSamples = delta_N_l[step];

      // aggregate variances across QoI for estimating N_l (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_l = agg_var[step]; // carried over from prev iter if no samp
      if (numSamples) {

	// advance any sequence specifications (seed_sequence)
	assign_specification_sequence(step);
	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model

	// export separate output files for each data set.  truth_model()
	// has the correct data when in bypass-surrogate mode.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.truth_model(), mlmfIter, step);

	// compute allResponses from allVariables using hierarchical model
	evaluate_parameter_sets(iteratedModel, true, false);

	// process allResponses: accumulate new samples for each qoi and
	// update number of successful samples for each QoI
	//if (mlmfIter == 0) accumulate_offsets(mu_hat[lev]);
	accumulate_ml_Ysums(sum_Y, sum_YY, lev, mu_hat[step], N_l[step]);
	if (outputLevel == DEBUG_OUTPUT)
	  Cout << "Accumulated sums (Y1, Y2, Y3, Y4, Y1sq):\n" << sum_Y[1]
	       << sum_Y[2] << sum_Y[3] << sum_Y[4] << sum_YY << std::endl;
	// update raw evaluation counts
	raw_N_l[step] += numSamples;

	// compute estimator variance from current sample accumulation:
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "variance of Y[" << step << "]: ";
	agg_var_l
	  = aggregate_variance_Ysum(sum_Y[1][step], sum_YY[step], N_l[step]);
      }

      sum_sqrt_var_cost += std::sqrt(agg_var_l * lev_cost);
      // MSE reference is MLMC with pilot sample, prior to any N_l adaptation:
      if (mlmfIter == 0)
	estimator_var0
	  += aggregate_mse_Ysum(sum_Y[1][step], sum_YY[step], N_l[step]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance from MLMC
    // on the pilot sample and then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0) { // eps^2 / 2 = var * relative factor
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // update targets based on variance estimates
    Real fact = sum_sqrt_var_cost / eps_sq_div_2, N_target;
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

  // aggregate expected value of estimators for Y, Y^2, Y^3, Y^4. Final expected
  // value is sum of expected values from telescopic sum.  Note: raw moments
  // have no bias correction (no additional variance from an estimated center).
  RealMatrix Q_raw_mom(numFunctions, 4);
  RealMatrix &sum_Y1 = sum_Y[1], &sum_Y2 = sum_Y[2],
	     &sum_Y3 = sum_Y[3], &sum_Y4 = sum_Y[4];
  for (qoi=0; qoi<numFunctions; ++qoi) {
    for (step=0; step<num_steps; ++step) {
      size_t Nlq = N_l[lev][qoi];
      Q_raw_mom(qoi,0) += sum_Y1(qoi,step) / Nlq;
      Q_raw_mom(qoi,1) += sum_Y2(qoi,step) / Nlq;
      Q_raw_mom(qoi,2) += sum_Y3(qoi,step) / Nlq;
      Q_raw_mom(qoi,3) += sum_Y4(qoi,step) / Nlq;
    }
  }
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(Q_raw_mom, momentStats);

  // compute the equivalent number of HF evaluations (includes any sim faults)
  equivHFEvals = raw_N_l[0] * cost[0]; // first level is single eval
  for (step=1; step<num_steps; ++step) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_l[step] * (cost[step] + cost[step-1]);
  equivHFEvals /= cost[num_steps-1]; // normalize into equivalent HF evals
}

/** This function performs "geometrical" MLMC on a single model form
    with multiple discretization levels. */
void NonDMultilevelSampling::multilevel_mc_Qsum(unsigned short model_form)
{    
    // retrieve cost estimates across soln levels for a particular model form
    RealVector cost,
      estimator_var0_qoi(numFunctions), eps_sq_div_2_qoi(numFunctions);
    size_t num_steps;//1 model form
    initialize_key_cost_steps(model_form, num_steps, cost);
    seq_index = 2;
    unsigned short lev = USHRT_MAX;
    unsigned short& step = (true) ? lev : model_form;
      
    // For moment estimation, we accumulate telescoping sums for Q^i using
    // discrepancies Yi = Q^i_{lev} - Q^i_{lev-1} (Y_diff_Qpow[i] for i=1:4).
    // For computing N_l from estimator variance, we accumulate square of Y1
    // estimator (YY[i] = (Y^i)^2 for i=1).
    IntRealMatrixMap sum_Ql, sum_Qlm1;
    IntIntPairRealMatrixMap sum_QlQlm1;
    initialize_ml_Qsums(sum_Ql, sum_Qlm1, sum_QlQlm1, num_steps);
    RealMatrix agg_var_qoi(numFunctions, num_steps);

    // Initialize for pilot sample
    SizetArray delta_N_l;
    load_pilot_sample(pilotSamples, NLev, delta_N_l);

    // raw eval counts are accumulation of allSamples irrespective of resp faults
    SizetArray raw_N_l(num_steps, 0);
    RealVectorArray mu_hat(num_steps);
    Sizet2DArray &N_l = NLev[model_form];

////TODO
    //// Problem 18
    
    Real N_L_exact, N_H_exact;

    Real x = iteratedModel.current_variables().all_continuous_variables()[1];
    Real Ac = iteratedModel.current_variables().all_discrete_real_variables()[0];
    if(Ac == -1)
      Ac = 0.5/6. * x + 0.4;
    else if(Ac == -2)
      Ac = 0.5/6. * sin(x) + 0.4;
    else if(Ac == -3)
     Ac = 0.5/6. * log(x) + 0.4;
    else if(Ac == -4)
      Ac = 0.69*1./exp(2.*x)+0.3;
    else
      throw INTERFACE_ERROR;
    Real N_MC = 100.;
    Real half_pow_six = std::pow(0.5, 6);
    Real var_H = 1./7. * half_pow_six;
    Real var_L = Ac*Ac * 1./7. * half_pow_six;
    Real var_deltaHL = (1.-Ac)*(1.-Ac) * var_H;
    Real mu_L_four_exact = half_pow_six*half_pow_six/13. * std::pow(Ac, 4);
    Real mu_H_four_exact = half_pow_six*half_pow_six/13.;
    Real C_H = 1.1;
    Real C_L = 0.1;
    {
    
    ////
      //// Cantilever
      /*
      IntRealMatrixMap sum_Ql_ref, sum_Qlm1_ref;
      IntIntPairRealMatrixMap sum_QlQlm1_ref;
      initialize_ml_Qsums(sum_Ql_ref, sum_Qlm1_ref, sum_QlQlm1_ref, num_steps);
      IntIntPair pr11_ref(1, 1);
      RealVectorArray mu_hat_ref(num_steps);
      RealVector agg_var_mean_qoi_ref(numFunctions), agg_var_var_qoi_ref(numFunctions), agg_var_qoi_ref(numFunctions);

      configure_indices(num_steps-1, model_form, lev, seq_index);

      numSamples = 10000;

      // generate new MC parameter sets
      get_parameter_sets(iteratedModel);// pull dist params from any model

      // compute allResponses from allVariables using hierarchical model
      evaluate_parameter_sets(iteratedModel, true, false);

      // process allResponses: accumulate new samples for each qoi and
      // update number of successful samples for each QoI
      //if (iter == 0) accumulate_offsets(mu_hat[lev]);

      SizetArray num_samples_array(numFunctions);
      for (qoi = 0; qoi < numFunctions; ++qoi) {
        num_samples_array[qoi] = numSamples;
      }
      accumulate_ml_Qsums(sum_Ql_ref, sum_Qlm1_ref, sum_QlQlm1_ref, num_steps-1,
                          mu_hat_ref[num_steps-1], num_samples_array);
      Real agg_var_l_ref = 0;
      if (qoiAggregationNorm==QOI_AGGREGATION_SUM) {
        if (allocationTarget == TARGET_MEAN) {
          agg_var_l_ref = aggregate_variance_Qsum(sum_Ql_ref[1][num_steps-1], sum_Qlm1_ref[1][lev],
                                              sum_Ql_ref[2][num_steps-1], sum_QlQlm1_ref[pr11][lev], sum_Qlm1_ref[2][lev],
                                                  num_samples_array, num_steps-1);
        } else if (allocationTarget == TARGET_VARIANCE) {
          for (qoi = 0; qoi < numFunctions; ++qoi) {
            agg_var_l_ref += var_of_var_ml_lmax(sum_Ql_ref, sum_Qlm1_ref, sum_QlQlm1_ref, numSamples,
                                                        100, qoi, false, place_holder)
                                                           * 100; //As described in the paper by Krumscheid, Pisaroni, Nobile
          }
        } else{
          Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: allocationTarget is not implemented.\n";
          abort_handler(INTERFACE_ERROR);
        }
        check_negative(agg_var_l_ref);
      }else if (qoiAggregationNorm==QOI_AGGREGATION_MAX) {
        for (qoi = 0; qoi < numFunctions; ++qoi) {
          agg_var_mean_qoi_ref[qoi] = variance_Ysum(sum_Ql_ref[1][num_steps-1][qoi], sum_Ql_ref[2][num_steps-1][qoi], numSamples);

          agg_var_var_qoi_ref[qoi] = var_of_var_ml_lmax(sum_Ql_ref, sum_Qlm1_ref, sum_QlQlm1_ref, numSamples,
                                                                          100, qoi, false, place_holder) *
                                      100; //As described in the paper by Krumscheid, Pisaroni, Nobile
          agg_var_qoi_ref[qoi] = (allocationTarget == TARGET_MEAN) ? agg_var_mean_qoi_ref[qoi] : agg_var_var_qoi_ref[qoi];
          check_negative(agg_var_qoi_ref[qoi]);
        }
      }else{
        Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: SampleAllocationType is not known.\n";
        abort_handler(INTERFACE_ERROR);
      }
      convergenceTol = agg_var_qoi_ref[1]/100;
      Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: Convergence Tolerance.\n" << convergenceTol << std::endl;
      //convergenceTol = agg_var_qoi_ref[2]/100;
      //Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: Convergence Tolerance.\n" << convergenceTol << std::endl;
      */

      //// Problem 18
      
      if(allocationTarget == TARGET_MEAN) {
        Real lagrange_mult = 1. / convergenceTol * (std::sqrt(var_L * C_L) + std::sqrt(var_deltaHL * C_H));

        N_L_exact = lagrange_mult * std::sqrt(var_L / C_L);
        N_H_exact = lagrange_mult * std::sqrt(var_deltaHL / C_H);
      }else if(allocationTarget == TARGET_VARIANCE) {
        RealVector initial_point;
        initial_point.size(2);

        //Cout << "Qoi: " << qoi << ", Pilot samples: " << std::endl;
        for (step = 0; step < 2; ++step) {
          initial_point[step] = 10.; //pilot_samples[step]; //N_target_mean_qoi[step][qoi]; //pilot_samples[step];//N_target_qoi[qoi][step]; //> pilot_samples[step] ? N_target_qoi[qoi][step] : pilot_samples[step];
        }
        Cout << "\n";
        RealVector var_lower_bnds, var_upper_bnds, lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets,
            nonlin_ineq_lower_bnds, nonlin_ineq_upper_bnds, nonlin_eq_targets;
        RealMatrix lin_ineq_coeffs, lin_eq_coeffs;
        //Bound constraints only allowing positive values for Nlq
        var_lower_bnds.size(num_steps); //init to 0
        for (step = 0; step < 2; ++step) {
          var_lower_bnds[step] = 7.; //pilot_samples[step] > 5. ? pilot_samples[step] : 5.;
        }

        RealVector level_cost_vec(2);
        for (step = 0; step < N_l.size(); ++step) {
          level_cost_vec[step] = level_cost(cost, step);
        }

        //var_lower_bnds.putScalar(3.); //Set to 3 to avoid NaNs
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
        nonlin_eq_targets[0] = std::log(convergenceTol);

        level_cost_vec[0] = C_L;
        level_cost_vec[1] = C_H;
        assign_static_member_problem18(var_L, var_H, mu_L_four_exact, mu_H_four_exact, Ac, level_cost_vec);
        std::unique_ptr <Iterator> optimizer;
        optimizer.reset(new SNLLOptimizer(initial_point,
                                          var_lower_bnds, var_upper_bnds,
                                          lin_ineq_coeffs, lin_ineq_lower_bnds,
                                          lin_ineq_upper_bnds, lin_eq_coeffs,
                                          lin_eq_targets, nonlin_ineq_lower_bnds,
                                          nonlin_ineq_upper_bnds, nonlin_eq_targets,
                                          &target_var_objective_eval_optpp,
                                          &target_var_constraint_eval_optpp_problem18)
        );
        optimizer->output_level(DEBUG_OUTPUT);
        optimizer->run();
        N_L_exact = optimizer->variables_results().continuous_variable(0);
        N_H_exact = optimizer->variables_results().continuous_variable(1);
      }else if(allocationTarget == TARGET_SIGMA) {
        RealVector initial_point;
        initial_point.size(2);

        //Cout << "Qoi: " << qoi << ", Pilot samples: " << std::endl;
        for (step = 0; step < 2; ++step) {
          initial_point[step] = 10.; //pilot_samples[step]; //N_target_mean_qoi[step][qoi]; //pilot_samples[step];//N_target_qoi[qoi][step]; //> pilot_samples[step] ? N_target_qoi[qoi][step] : pilot_samples[step];
        }
        Cout << "\n";
        RealVector var_lower_bnds, var_upper_bnds, lin_ineq_lower_bnds, lin_ineq_upper_bnds, lin_eq_targets,
            nonlin_ineq_lower_bnds, nonlin_ineq_upper_bnds, nonlin_eq_targets;
        RealMatrix lin_ineq_coeffs, lin_eq_coeffs;
        //Bound constraints only allowing positive values for Nlq
        var_lower_bnds.size(num_steps); //init to 0
        for (step = 0; step < 2; ++step) {
          var_lower_bnds[step] = 7.; //pilot_samples[step] > 5. ? pilot_samples[step] : 5.;
        }

        RealVector level_cost_vec(2);
        for (step = 0; step < N_l.size(); ++step) {
          level_cost_vec[step] = level_cost(cost, step);
        }

        //var_lower_bnds.putScalar(3.); //Set to 3 to avoid NaNs
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
        nonlin_eq_targets[0] = std::log(convergenceTol);

        level_cost_vec[0] = C_L;
        level_cost_vec[1] = C_H;
        assign_static_member_problem18(var_L, var_H, mu_L_four_exact, mu_H_four_exact, Ac, level_cost_vec);
        std::unique_ptr <Iterator> optimizer;
        optimizer.reset(new SNLLOptimizer(initial_point,
                                          var_lower_bnds, var_upper_bnds,
                                          lin_ineq_coeffs, lin_ineq_lower_bnds,
                                          lin_ineq_upper_bnds, lin_eq_coeffs,
                                          lin_eq_targets, nonlin_ineq_lower_bnds,
                                          nonlin_ineq_upper_bnds, nonlin_eq_targets,
                                          &target_var_objective_eval_optpp,
                                          &target_sigma_constraint_eval_optpp_problem18)
        );
        optimizer->output_level(DEBUG_OUTPUT);
        optimizer->run();
        N_L_exact = optimizer->variables_results().continuous_variable(0);
        N_H_exact = optimizer->variables_results().continuous_variable(1);
      }else {
        throw IO_ERROR;
      }

    }
    
    ////

    // now converge on sample counts per level (N_l)
    mlmfIter = 0;
    while (Pecos::l1_norm(delta_N_l) && mlmfIter <= max_iter) {
      for (step=0; step<num_steps; ++step) {
        configure_indices(step, model_form, lev, seq_index);

        // set the number of current samples from the defined increment
        numSamples = delta_N_l[step];

        // aggregate variances across QoI for estimating N_l (justification:
        // for independent QoI, sum of QoI variances = variance of QoI sum)
        //Real &agg_var_l = agg_var[step]; // carried over from prev iter if no samp
        if (numSamples) {
          evaluate_sample_increment(step);

          accumulate_sums(sum_Ql, sum_Qlm1, sum_QlQlm1, step, mu_hat, N_l);

          // update raw evaluation counts
          raw_N_l[step] += numSamples;

          aggregate_variance_target_Qsum(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l, step, agg_var_qoi);
          // MSE reference is MC applied to HF
          if (mlmfIter == 0) {
            aggregate_mse_target_Qsum(agg_var_qoi, N_l, step, estimator_var0_qoi);
          }
        }
      }
      if (mlmfIter == 0) { // eps^2 / 2 = var * relative factor
        set_convergence_tol(estimator_var0_qoi, convergenceTol, eps_sq_div_2_qoi);
      }

      // update targets based on variance estimates
      //if(target_mean){
      if (outputLevel == DEBUG_OUTPUT) Cout << "N_target: " << std::endl;

      compute_sample_allocation_target(sum_Ql, sum_Qlm1, sum_QlQlm1, eps_sq_div_2_qoi, agg_var_qoi, cost, N_l, delta_N_l);

      ++mlmfIter;
      Cout << "\nMLMC iteration " << mlmfIter << " sample increments:\n"
	   << delta_N_l << std::endl;
    }

    //// TODO Problem 18
    
    std::ofstream myfile;
    Real thought_convergence_tol;
    Real exact_var_of_moment;
    Real thought_var_of_moment;
    Real exact_opt_var_of_moment;
    Real thought_opt_var_of_moment;
    Real exact_FN_var_of_moment;
    Real thought_FN_var_of_moment;
    if(allocationTarget == TARGET_MEAN){
      Real half_pow_six = std::pow(0.5, 6);
      Real var_H = 1./7. * half_pow_six;
      Real var_L = Ac*Ac * 1./7. * half_pow_six;
      Real var_deltaHL = (1.-Ac)*(1.-Ac) * var_H;
      exact_var_of_moment = var_L/N_l[0][0] + var_deltaHL/N_l[1][0];
      thought_var_of_moment = agg_var_qoi(1, 0) / N_l[0][0] + agg_var_qoi(1, 1) / N_l[1][0];
      exact_opt_var_of_moment = var_L / N_target_qoi(1, 0) + var_L / N_target_qoi(1, 1);
      thought_opt_var_of_moment = agg_var_qoi(1, 0) / N_target_qoi(1, 0) + agg_var_qoi(1, 1) / N_target_qoi(1, 1);
      thought_convergence_tol = agg_var_qoi(1, 0) / N_L_exact + agg_var_qoi(1, 1) / N_H_exact;
      Cout << "MLMC exact sample size (for mean)\n" << std::ceil(N_L_exact) << "\n" << std::ceil(N_H_exact) << std::endl << std::endl;
      Cout << "MLMC numerical var_of_mean size (for mean)\n" << exact_var_of_moment << "\t vs. \t" << convergenceTol << std::endl << std::endl;
      myfile.open("problem18_sampleallocation_mean.txt", std::ofstream::out | std::ofstream::app);
    }else if(allocationTarget == TARGET_VARIANCE) {
      RealVector Nl_tmp;
      RealVector var_of_moment_tmp;
      RealMatrix grad_g_dummy;
      int result_mode_dummy;
      var_of_moment_tmp.size(1);
      RealVector pilot_samples;
      pilot_samples.size(2);
      RealVector level_cost_vec(2);
      for (step = 0; step < N_l.size(); ++step) {
        pilot_samples[step] = N_l[step][1];
        level_cost_vec[step] = level_cost(cost, step);
      }
      size_t qoiAggregation_copy = qoiAggregation;
      size_t numFunctions_copy = numFunctions;
      Nl_tmp.size(2);
      Nl_tmp[0] = N_l[0][0];
      Nl_tmp[1] = N_l[1][0];
      size_t qoi_tmp = 1;
      exact_var_of_moment = exact_var_of_var_problem18(Nl_tmp);
      assign_static_member(convergenceTol, qoi_tmp, qoiAggregation_copy, numFunctions_copy, level_cost_vec, sum_Ql, sum_Qlm1, sum_QlQlm1, pilot_samples);
      target_var_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_var_of_moment = var_of_moment_tmp[0];

      Nl_tmp[0] = N_target_qoi(1, 0);
      Nl_tmp[1] = N_target_qoi(1, 1);
      exact_opt_var_of_moment = exact_var_of_var_problem18(Nl_tmp);
      target_var_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_opt_var_of_moment = var_of_moment_tmp[0];

      Nl_tmp[0] = N_target_qoi_FN(1, 0);
      Nl_tmp[1] = N_target_qoi_FN(1, 1);
      exact_FN_var_of_moment = exact_var_of_var_problem18(Nl_tmp);
      target_var_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_FN_var_of_moment = var_of_moment_tmp[0];

      Nl_tmp[0] = N_L_exact;
      Nl_tmp[1] = N_H_exact;
      target_var_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_convergence_tol = var_of_moment_tmp[0];

      Cout << "MLMC exact sample size (for var)\n" << std::ceil(N_L_exact) << "\n" << std::ceil(N_H_exact) << std::endl << std::endl;
      Cout << "MLMC numerical var_of_var size (for var)\n" << exact_var_of_moment << "\t vs. \t" << convergenceTol << std::endl << std::endl;
      myfile.open("problem18_sampleallocation_var.txt", std::ofstream::out | std::ofstream::app);
    }else if(allocationTarget == TARGET_SIGMA) {
      RealVector Nl_tmp;
      RealVector var_of_moment_tmp;
      RealMatrix grad_g_dummy;
      int result_mode_dummy;
      var_of_moment_tmp.size(1);
      RealVector pilot_samples;
      pilot_samples.size(2);
      RealVector level_cost_vec(2);
      for (step = 0; step < N_l.size(); ++step) {
        pilot_samples[step] = N_l[step][1];
        level_cost_vec[step] = level_cost(cost, step);
      }
      size_t qoiAggregation_copy = qoiAggregation;
      size_t numFunctions_copy = numFunctions;
      Nl_tmp.size(2);
      Nl_tmp[0] = N_l[0][0];
      Nl_tmp[1] = N_l[1][0];
      size_t qoi_tmp = 1;
      exact_var_of_moment = exact_var_of_sigma_problem18(Nl_tmp);
      assign_static_member(convergenceTol, qoi_tmp, qoiAggregation_copy, numFunctions_copy, level_cost_vec, sum_Ql, sum_Qlm1, sum_QlQlm1, pilot_samples);
      target_sigma_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_var_of_moment = var_of_moment_tmp[0];

      Nl_tmp[0] = N_target_qoi(1, 0);
      Nl_tmp[1] = N_target_qoi(1, 1);
      exact_opt_var_of_moment = exact_var_of_sigma_problem18(Nl_tmp);
      target_sigma_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_opt_var_of_moment = var_of_moment_tmp[0];

      Nl_tmp[0] = N_target_qoi_FN(1, 0);
      Nl_tmp[1] = N_target_qoi_FN(1, 1);
      exact_FN_var_of_moment = exact_var_of_sigma_problem18(Nl_tmp);
      target_sigma_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_FN_var_of_moment = var_of_moment_tmp[0];

      Nl_tmp[0] = N_L_exact;
      Nl_tmp[1] = N_H_exact;
      target_sigma_constraint_eval_optpp(OPTPP::NLPFunction, 2, Nl_tmp, var_of_moment_tmp, grad_g_dummy, result_mode_dummy);
      thought_convergence_tol = var_of_moment_tmp[0];

      Cout << "MLMC exact sample size (for var)\n" << std::ceil(N_L_exact) << "\n" << std::ceil(N_H_exact) << std::endl << std::endl;
      Cout << "MLMC numerical var_of_var size (for var)\n" << exact_var_of_moment << "\t vs. \t" << convergenceTol << std::endl << std::endl;
      myfile.open("problem18_sampleallocation_sigma.txt", std::ofstream::out | std::ofstream::app);
    }else{
      throw IO_ERROR;
    }                                    //2                  //3                  //4
    myfile << x << "\t" << Ac << "\t" << N_l[0][0] << "\t" << N_l[1][0] << "\t" << thought_var_of_moment << "\t"  << exact_var_of_moment // Worst case sample allocation and its exact variance
                              << "\t" << N_L_exact << "\t" << (N_H_exact) << "\t"  << thought_convergence_tol << "\t" << convergenceTol  // Exact sample allocation and convergence tolerance
                              << "\t" << N_target_qoi(1, 0) << "\t" << (N_target_qoi(1, 1)) << "\t" << thought_opt_var_of_moment << "\t" << exact_opt_var_of_moment // Actual sample allocation found and its seen tolerance
                              << "\t" << N_target_qoi_FN(1, 0) << "\t" << (N_target_qoi_FN(1, 1)) << "\t" << thought_FN_var_of_moment << "\t" << exact_FN_var_of_moment << "\n"; // FN sample allocation found and its seen tolerance
    myfile.close();
    
    ////
    compute_moments(sum_Ql, sum_Qlm1, N_l);

    // populate finalStatErrors
    compute_error_estimates(sum_Ql, sum_Qlm1, sum_QlQlm1, N_l);

    // compute the equivalent number of HF evaluations (includes any sim faults)
    compute_equiv_HF_evals(raw_N_l, cost);
}

/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDMultilevelSampling::
control_variate_mc(const UShortArray& active_key)
{
  // Current implementation performs pilot + shared increment + LF increment,
  // where these increments are targeting a prescribed MSE reduction.
  // **********
  // *** TO DO: should CV MC iterate (new shared + LF increments)
  // ***        until MSE target is met?
  // **********

  aggregated_models_mode();
  iteratedModel.active_model_key(active_key); // data group 0
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  // retrieve active index
  //unsigned short lf_lev_index =  surr_model.solution_level_index(),
  //               hf_lev_index = truth_model.solution_level_index();
  // retrieve cost estimates across model forms for a particular soln level
  Real lf_cost =  surr_model.solution_level_cost(),
       hf_cost = truth_model.solution_level_cost(),
    cost_ratio = hf_cost / lf_cost, avg_eval_ratio, avg_mse_ratio;

  IntRealVectorMap sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH;
  initialize_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH);
  RealVector sum_HH(numFunctions), var_H(numFunctions, false),
            rho2_LH(numFunctions, false);

  // Initialize for pilot sample
  SizetArray delta_N_l;
  load_pilot_sample(pilotSamples, NLev, delta_N_l);

  // NLev allocations currently enforce truncation to #HF levels (1)
  UShortArray hf_key, lf_key;
  Pecos::DiscrepancyCalculator::extract_keys(active_key, hf_key, lf_key);
  unsigned short hf_model_form = hf_key[1], lf_model_form = lf_key[1];
  SizetArray& N_lf = NLev[lf_model_form][0];//[lf_lev_index];
  SizetArray& N_hf = NLev[hf_model_form][0];//[hf_lev_index];
  size_t raw_N_lf = 0, raw_N_hf = 0;
  RealVector mu_hat;

  // ---------------------
  // Compute Pilot Samples
  // ---------------------

  mlmfIter = 0;

  // Initialize for pilot sample (shared sample count discarding any excess)
  numSamples = std::min(delta_N_l[lf_model_form], delta_N_l[hf_model_form]);
  shared_increment(mlmfIter, 0);
  accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
		     sum_HH, mu_hat, N_lf, N_hf);
  raw_N_lf += numSamples; raw_N_hf += numSamples;

  // Compute the LF/HF evaluation ratio, averaged over the QoI.
  // This includes updating var_H and rho2_LH.
  avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1], sum_LH[1],
			      sum_HH, cost_ratio, N_hf, var_H, rho2_LH);
  // compute the ratio of MC and CVMC mean squared errors (controls convergence)
  avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH, mlmfIter, N_hf);

  // ----------------------------------------------------------
  // Compute shared increment targeting specified MSE reduction
  // ----------------------------------------------------------

  // bypass refinement if maxIterations == 0 or convergenceTol already
  // satisfied by pilot sample
  if (maxIterations && avg_mse_ratio > convergenceTol) {

    // Assuming rho_AB, evaluation_ratio and var_H to be relatively invariant,
    // we seek a relative reduction in MSE using the convergence tol spec:
    //   convTol = CV_mse / MC^0_mse = mse_ratio * N0 / N
    //   delta_N = mse_ratio*N0/convTol - N0 = (mse_ratio/convTol - 1) * N0
    Real incr = (avg_mse_ratio / convergenceTol - 1.) * numSamples;
    numSamples = (size_t)std::floor(incr + .5); // round

    if (numSamples) {
      shared_increment(++mlmfIter, 0);
      accumulate_cv_sums(sum_L_shared, sum_L_refined, sum_H, sum_LL, sum_LH,
			 sum_HH, mu_hat, N_lf, N_hf);
      raw_N_lf += numSamples; raw_N_hf += numSamples;
      // update ratios:
      avg_eval_ratio = eval_ratio(sum_L_shared[1], sum_H[1], sum_LL[1],
				  sum_LH[1], sum_HH, cost_ratio, N_hf,
				  var_H, rho2_LH);
      avg_mse_ratio  = MSE_ratio(avg_eval_ratio, var_H, rho2_LH,mlmfIter,N_hf);
    }
  }

  // --------------------------------------------------
  // Compute LF increment based on the evaluation ratio
  // --------------------------------------------------
  uncorrected_surrogate_mode(); // also needed for assignment of lf_key below
  // Group id in lf_key is not currently important, since no SurrogateData
  // (correlations are computed based on the paired LF/HF data group, prior
  // to the augmentation, which could imply a future group segregation)
  iteratedModel.active_model_key(lf_key); // sets activeKey and surrModelKey
  if (lf_increment(avg_eval_ratio, N_lf, N_hf, ++mlmfIter, 0)) { // level 0
    accumulate_cv_sums(sum_L_refined, mu_hat, N_lf);
    raw_N_lf += numSamples;
  }

  // Compute/apply control variate parameter to estimate uncentered raw moments
  RealMatrix H_raw_mom(numFunctions, 4);
  cv_raw_moments(sum_L_shared, sum_H, sum_LL, sum_LH, N_hf, sum_L_refined, N_lf,
		 rho2_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf + (Real)raw_N_lf / cost_ratio;
}


/** This function performs "geometrical" MLMC across discretization
    levels for the high fidelity model form where CVMC si employed
    across two model forms to exploit correlation in the discrepancies
    at each level (Y_l). */
void NonDMultilevelSampling::
multilevel_control_variate_mc_Ycorr(unsigned short lf_model_form,
				    unsigned short hf_model_form)
{
  // assign model forms (solution level assignments are deferred until loop)
  UShortArray active_key;
  unsigned short seq_index = 2, lev = USHRT_MAX; // lev updated in loop below
  Pecos::DiscrepancyCalculator::
    form_key(0, hf_model_form, lev, lf_model_form, lev, active_key);
  iteratedModel.active_model_key(active_key);
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  size_t qoi, num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  unsigned short& group = lev; // no alias switch for this algorithm

  Real avg_eval_ratio, eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0.,
    lf_lev_cost, hf_lev_cost;
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_costs(),
    lf_cost = surr_model.solution_level_costs(), agg_var_hf(num_hf_lev),
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
  Sizet2DArray  delta_N_l;   load_pilot_sample(pilotSamples, NLev, delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_model_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_model_form]; 

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_lf(num_cv_lev, 0), raw_N_hf(num_hf_lev, 0);
  RealVector mu_L_hat, mu_H_hat;

  // now converge on sample counts per level (N_hf)
  mlmfIter = 0;
  while (Pecos::l1_norm(delta_N_hf) && mlmfIter <= max_iter) {

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_hf_lev; ++lev) {

      configure_indices(group, hf_model_form, lev, seq_index);
      hf_lev_cost = level_cost(hf_cost, lev);

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// advance any sequence specifications (seed_sequence)
	assign_specification_sequence(lev);
	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model

	// export separate output files for each data set.  Note that
	// truth_model() is indexed with hf_model_form at this stage for
	// all levels.  The exported discretization level (e.g., state variable
	// value) can't capture a level discrepancy for lev>0 and will reflect
	// the most recent evaluation state.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.truth_model(), mlmfIter, lev);

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
	  configure_indices(group, lf_model_form, lev, seq_index);
	  lf_lev_cost = level_cost(lf_cost, lev);
	  // compute allResp w/ LF model form reusing allVars from MLMC step
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // process previous and new set of allResponses for CV sums
	  accumulate_mlcv_Ysums(allResponses, hf_resp, sum_L_shared,
				sum_L_refined, sum_H, sum_LL, sum_LH,
				sum_HH, lev, mu_L_hat, mu_H_hat,
				N_lf[lev], N_hf[lev]);
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_shared[1,2], L_refined[1,2], LH[1,2])"
		 << ":\n" << sum_L_shared[1] << sum_L_shared[2]
		 << sum_L_refined[1] << sum_L_refined[2]
		 << sum_LH[1] << sum_LH[2];
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
	  accumulate_ml_Ysums(sum_H, sum_HH1, lev, mu_H_hat,
			      N_hf[lev]); // sum_Y for lev>0
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (H[1], H[2], HH):\n"
		 << sum_H[1] << sum_H[2] << sum_HH1;
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
      // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
      // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
      // will result in no additional variance reduction beyond MLMC.
      if (mlmfIter == 0)
	estimator_var0 += (lev < num_cv_lev) ?
	  aggregate_mse_Yvar(var_H[lev], N_hf[lev]) :
	  aggregate_mse_Ysum(sum_H[1][lev], sum_HH[1][lev], N_hf[lev]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0) { // eps^2 / 2 = var * relative factor
      eps_sq_div_2 = estimator_var0 * convergenceTol;
      if (outputLevel == DEBUG_OUTPUT)
	Cout << "Epsilon squared target = " << eps_sq_div_2 << std::endl;
    }

    // All CV lf_increment() calls now follow all ML level evals:
    for (lev=0; lev<num_cv_lev; ++lev) {
      if (delta_N_hf[lev]) {
	configure_indices(group, lf_model_form, lev, seq_index);//augment LF grp

	// execute additional LF sample increment, if needed
	if (lf_increment(avg_eval_ratios[lev], N_lf[lev], N_hf[lev],
			 mlmfIter, lev)) {
	  accumulate_mlcv_Ysums(sum_L_refined, lev, mu_L_hat, N_lf[lev]);
	  raw_N_lf[lev] += numSamples;
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_refined[1,2]):\n"
		 << sum_L_refined[1] << sum_L_refined[2];
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
    ++mlmfIter;
    Cout << "\nMLCVMC iteration " << mlmfIter << " sample increments:\n"
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
multilevel_control_variate_mc_Qcorr(unsigned short lf_model_form,
				    unsigned short hf_model_form)
{
  // assign model forms (solution level assignments are deferred until loop)
  UShortArray active_key;
  unsigned short seq_index = 2, lev = USHRT_MAX; // lev updated in loop below
  Pecos::DiscrepancyCalculator::
    form_key(0, hf_model_form, lev, lf_model_form, lev, active_key);
  iteratedModel.active_model_key(active_key);
  Model& truth_model = iteratedModel.truth_model();
  Model& surr_model  = iteratedModel.surrogate_model();

  size_t qoi, num_hf_lev = truth_model.solution_levels(),
    num_cv_lev = std::min(num_hf_lev, surr_model.solution_levels());
  unsigned short& group = lev; // no alias switch for this algorithm

  Real avg_eval_ratio, eps_sq_div_2, sum_sqrt_var_cost, estimator_var0 = 0.,
    lf_lev_cost, hf_lev_cost;
  // retrieve cost estimates across solution levels for HF model
  RealVector hf_cost = truth_model.solution_level_costs(),
    lf_cost = surr_model.solution_level_costs(), agg_var_hf(num_hf_lev),
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
  Sizet2DArray  delta_N_l; load_pilot_sample(pilotSamples, NLev, delta_N_l);
  //SizetArray& delta_N_lf = delta_N_l[lf_model_form];
  SizetArray&   delta_N_hf = delta_N_l[hf_model_form]; 

  // raw eval counts are accumulation of allSamples irrespective of resp faults
  SizetArray raw_N_lf(num_cv_lev, 0), raw_N_hf(num_hf_lev, 0);
  RealVector mu_L_hat, mu_H_hat;

  // now converge on sample counts per level (N_hf)
  mlmfIter = 0;
  while (Pecos::l1_norm(delta_N_hf) && mlmfIter <= max_iter) {

    sum_sqrt_var_cost = 0.;
    for (lev=0; lev<num_hf_lev; ++lev) {

      configure_indices(group, hf_model_form, lev, seq_index);
      hf_lev_cost = level_cost(hf_cost, lev);

      // set the number of current samples from the defined increment
      numSamples = delta_N_hf[lev];

      // aggregate variances across QoI for estimating N_hf (justification:
      // for independent QoI, sum of QoI variances = variance of QoI sum)
      Real& agg_var_hf_l = agg_var_hf[lev];//carried over from prev iter if!samp
      if (numSamples) {

	// advance any sequence specifications (seed_sequence)
	assign_specification_sequence(lev);
	// generate new MC parameter sets
	get_parameter_sets(iteratedModel);// pull dist params from any model

	// export separate output files for each data set.  Note that
	// truth_model() is indexed with hf_model_form at this stage for
	// all levels.  The exported discretization level (e.g., state variable
	// value) can't capture a level discrepancy for lev>0 and will reflect
	// the most recent evaluation state.
	if (exportSampleSets)
	  export_all_samples("ml_", iteratedModel.truth_model(), mlmfIter, lev);

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
	  configure_indices(group, lf_model_form, lev, seq_index);
	  lf_lev_cost = level_cost(lf_cost, lev);
	  // eval allResp w/ LF model reusing allVars from ML step above
	  evaluate_parameter_sets(iteratedModel, true, false);
	  // process previous and new set of allResponses for MLCV sums;
	  accumulate_mlcv_Qsums(allResponses, hf_resp, sum_Ll, sum_Llm1,
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
	  accumulate_ml_Ysums(sum_Hl, sum_HH1, lev, mu_H_hat,
			      N_hf[lev]); // sum_Y for lev>0
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (H[1], H[2], HH[1]):\n"
		 << sum_Hl[1] << sum_Hl[2] << sum_HH1;
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
      // MSE reference is MLMF MC applied to {HF,LF} pilot sample aggregated
      // across qoi.  Note: if the pilot sample for LF is not shaped, then r=1
      // will result in no additional variance reduction beyond MLMC.
      if (mlmfIter == 0)
	estimator_var0 += (lev < num_cv_lev) ?
	  aggregate_mse_Yvar(var_Yl[lev], N_hf[lev]) :
	  aggregate_mse_Ysum(sum_Hl[1][lev], sum_Hl_Hl[1][lev], N_hf[lev]);
    }
    // compute epsilon target based on relative tolerance: total MSE = eps^2
    // which is equally apportioned (eps^2 / 2) among discretization MSE and
    // estimator variance (\Sum var_Y_l / N_l).  Since we do not know the
    // discretization error, we compute an initial estimator variance and
    // then seek to reduce it by a relative_factor <= 1.
    if (mlmfIter == 0) { // eps^2 / 2 = var * relative factor
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
	configure_indices(group, lf_model_form, lev, seq_index);//augment LF grp

	// now execute additional LF sample increment, if needed
	if (lf_increment(avg_eval_ratios[lev], N_lf[lev], N_hf[lev],
			 mlmfIter, lev)) {
	  accumulate_mlcv_Qsums(sum_Ll_refined, sum_Llm1_refined, lev, mu_L_hat,
				N_lf[lev]);
	  raw_N_lf[lev] += numSamples;
	  if (outputLevel == DEBUG_OUTPUT)
	    Cout << "Accumulated sums (L_refined[1,2]):\n"
		 << sum_Ll_refined[1] << sum_Ll_refined[2];
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
    ++mlmfIter;
    Cout << "\nMLCVMC iteration " << mlmfIter << " sample increments:\n"
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

  // compute the equivalent number of HF evaluations
  equivHFEvals = raw_N_hf[0] * hf_cost[0] + raw_N_lf[0] * lf_cost[0]; // 1st lev
  for (lev=1; lev<num_hf_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_hf[lev] * (hf_cost[lev] + hf_cost[lev-1]);
  for (lev=1; lev<num_cv_lev; ++lev) // subsequent levels incur 2 model costs
    equivHFEvals += raw_N_lf[lev] * (lf_cost[lev] + lf_cost[lev-1]);
  equivHFEvals /= hf_cost[num_hf_lev-1]; // normalize into equivalent HF evals
}


void NonDMultilevelSampling::initialize_key_cost_steps(const unsigned short& model_form, size_t& num_steps, RealVector& cost)
{
  // assign truth model form (solution level assignment is deferred until loop)
  UShortArray truth_key;
  unsigned short lev = USHRT_MAX; // lev updated in loop below
  Pecos::DiscrepancyCalculator::form_key(0, model_form, lev, truth_key);
  iteratedModel.active_model_key(truth_key);
  Model& truth_model = iteratedModel.truth_model();
  num_steps = truth_model.solution_levels();
  cost = truth_model.solution_level_costs();
}

void NonDMultilevelSampling::
configure_indices(unsigned short group, unsigned short form,
		  unsigned short lev,   unsigned short s_index)
{
  // Notes:
  // > could consolidate with NonDExpansion::configure_indices() with a passed
  //   model and virtual *_mode() assignments.  Leaving separate for now...
  // > group index is assigned based on step in model form/resolution sequence
  // > CVMC does not use this helper; it requires uncorrected_surrogate_mode()

  UShortArray hf_key;
  Pecos::DiscrepancyCalculator::form_key(group, form, lev, hf_key);

  if (hf_key[s_index] == 0) { // step 0 in the sequence
    bypass_surrogate_mode();
    iteratedModel.active_model_key(hf_key);          // one active fidelity
  }
  else { //if (multilevDiscrepEmulation == DISTINCT_EMULATION) {
    aggregated_models_mode();

    UShortArray lf_key(hf_key), aggregate_key;
    Pecos::DiscrepancyCalculator::decrement_key(lf_key, s_index);    
    Pecos::DiscrepancyCalculator::aggregate_keys(hf_key, lf_key, aggregate_key);
    iteratedModel.active_model_key(aggregate_key); // two active fidelities
  }
}

void NonDMultilevelSampling::evaluate_sample_increment(const unsigned short& step)
{
  // advance any sequence specifications (seed_sequence)
  assign_specification_sequence(step);
  // generate new MC parameter sets
  get_parameter_sets(iteratedModel);// pull dist params from any model

  // export separate output files for each data set.  truth_model()
  // has the correct data when in bypass-surrogate mode.
  if (exportSampleSets)
  export_all_samples("ml_", iteratedModel.truth_model(),
       mlmfIter, step);

  // compute allResponses from allVariables using hierarchical model
  evaluate_parameter_sets(iteratedModel, true, false);
}


void NonDMultilevelSampling::assign_specification_sequence(size_t index)
{
  // Note: seedSpec/randomSeed initialized from randomSeedSeqSpec in ctor

  // advance any sequence specifications, as admissible
  // Note: no colloc pts sequence as load_pilot_sample() handles this separately
  int seed_i = random_seed(index);
  if (seed_i) randomSeed = seed_i;// propagate to NonDSampling::initialize_lhs()
  // else previous value will allow existing RNG to continue for varyPattern
}

void NonDMultilevelSampling::accumulate_sums(IntRealMatrixMap& sum_Ql, IntRealMatrixMap& sum_Qlm1,
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
accumulate_cv_sums(IntRealVectorMap& sum_L, const RealVector& offset,
		   SizetArray& num_L)
{
  // uses one set of allResponses in BYPASS_SURROGATE mode
  // IntRealVectorMap is not a multilevel case --> no discrepancies

  using std::isfinite;
  Real fn_val, prod;
  int ord, active_ord; size_t qoi;
  IntRespMCIter r_it; IntRVMIter l_it;
  bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {
      prod = fn_val = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];

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
		   RealVector& sum_HH, const RealVector& offset,
		   SizetArray& num_L, SizetArray& num_H)
{
  // uses one set of allResponses in AGGREGATED_MODELS mode
  // IntRealVectorMap is not a multilevel case so no discrepancies

  using std::isfinite;
  Real lf_fn, hf_fn, lf_prod, hf_prod;
  IntRespMCIter r_it; IntRVMIter ls_it, lr_it, h_it, ll_it, lh_it;
  int ls_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord; size_t qoi;
  bool os = !offset.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const RealVector& fn_vals = r_it->second.function_values();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // response mode AGGREGATED_MODELS orders HF (active model key)
      // followed by LF (previous/decremented model key)
      hf_prod = hf_fn = (os) ? fn_vals[qoi] - offset[qoi] : fn_vals[qoi];
      lf_prod = lf_fn = (os) ?
	fn_vals[qoi+numFunctions] - offset[qoi+numFunctions] :
	fn_vals[qoi+numFunctions];

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


void NonDMultilevelSampling::
accumulate_mlcv_Ysums(IntRealMatrixMap& sum_Y, size_t lev,
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


void NonDMultilevelSampling::
accumulate_mlcv_Qsums(const IntResponseMap& lf_resp_map,
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


void NonDMultilevelSampling::
accumulate_mlcv_Ysums(const IntResponseMap& lf_resp_map,
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
    accumulate_mlcv_Qsums(lf_resp_map, hf_resp_map, sum_L_shared,
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
		      const RealVector& lf_offset, const RealVector& hf_offset,
		      SizetArray& num_L, SizetArray& num_H)
{
  // uses two sets of responses (LF & HF) in BYPASS_SURROGATE (level 0) or
  // AGGREGATED_MODELS (lev > 0) modes.  IntRealMatrixMap are for multilevel
  // case with discrepancies, indexed by level.

  if (lev == 0) // level lm1 not available; accumulate only level l
    accumulate_mlcv_Qsums(lf_resp_map, hf_resp_map, sum_Ll, sum_Ll_refined,
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


void NonDMultilevelSampling::shared_increment(size_t iter, size_t lev)
{
  if (iter == _NPOS)  Cout << "\nCVMC sample increments: ";
  else if (iter == 0) Cout << "\nCVMC pilot sample: ";
  else Cout << "\nCVMC iteration " << iter << " sample increments: ";
  Cout << "LF = " << numSamples << " HF = " << numSamples << '\n';

  //aggregated_models_mode(); // set at calling level for CV

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

    if (iter < max_iter || finalCVRefinement) {
      // hierarchical surrogate mode could be BYPASS_SURROGATE for CV or
      // BYPASS_SURROGATE/AGGREGATED_MODELS for ML-CV
      //bypass_surrogate_mode(); // set at calling level for CV or ML-CV

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
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  //Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  //Real var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
  //    cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;

  // Cancel repeated N_shared and bessel_corr within beta = cov_LH / var_L:
  beta = (sum_LH - sum_L * sum_H / N_shared)
       / (sum_LL - sum_L * sum_L / N_shared);
}


void NonDMultilevelSampling::
compute_control(Real sum_L, Real sum_H, Real sum_LL, Real sum_LH, Real sum_HH,
		size_t N_shared, Real& var_H, Real& rho2_LH)
{
  Real bessel_corr = (Real)N_shared / (Real)(N_shared - 1);

  // unbiased mean estimator X-bar = 1/N * sum
  Real mu_L = sum_L / N_shared, mu_H = sum_H / N_shared;
  // unbiased sample variance estimator = 1/(N-1) sum[(X_i - X-bar)^2]
  // = 1/(N-1) [ N Raw_X - N X-bar^2 ] = bessel * [Raw_X - X-bar^2]
  Real var_L = (sum_LL / N_shared - mu_L * mu_L) * bessel_corr,
      cov_LH = (sum_LH / N_shared - mu_L * mu_H) * bessel_corr;
  var_H      = (sum_HH / N_shared - mu_H * mu_H) * bessel_corr;

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
convert_moments(const RealMatrix& raw_mom, RealMatrix& final_mom)
{
  // Note: raw_mom is numFunctions x 4 and final_mom is the transpose
  if (final_mom.empty())
    final_mom.shapeUninitialized(4, numFunctions);

  // Convert uncentered raw moment estimates to central moments
  if (finalMomentsType == CENTRAL_MOMENTS) {
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      uncentered_to_centered(raw_mom(qoi,0), raw_mom(qoi,1), raw_mom(qoi,2),
			     raw_mom(qoi,3), final_mom(0,qoi), final_mom(1,qoi),
			     final_mom(2,qoi), final_mom(3,qoi));
  }
  // Convert uncentered raw moment estimates to standardized moments
  else { //if (finalMomentsType == STANDARD_MOMENTS) {
    Real cm1, cm2, cm3, cm4;
    for (size_t qoi=0; qoi<numFunctions; ++qoi) {
      uncentered_to_centered(raw_mom(qoi,0), raw_mom(qoi,1), raw_mom(qoi,2),
			     raw_mom(qoi,3), cm1, cm2, cm3, cm4);
      centered_to_standard(cm1, cm2, cm3, cm4, final_mom(0,qoi),
			   final_mom(1,qoi), final_mom(2,qoi),
			   final_mom(3,qoi));
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    for (size_t qoi=0; qoi<numFunctions; ++qoi)
      Cout <<  "raw mom 1 = "   << raw_mom(qoi,0)
	   << " final mom 1 = " << final_mom(0,qoi) << '\n'
	   <<  "raw mom 2 = "   << raw_mom(qoi,1)
	   << " final mom 2 = " << final_mom(1,qoi) << '\n'
	   <<  "raw mom 3 = "   << raw_mom(qoi,2)
	   << " final mom 3 = " << final_mom(2,qoi) << '\n'
	   <<  "raw mom 4 = "   << raw_mom(qoi,3)
	   << " final mom 4 = " << final_mom(3,qoi) << "\n\n";
}


void NonDMultilevelSampling::
  compute_error_estimates(IntRealMatrixMap &sum_Ql, IntRealMatrixMap &sum_Qlm1,
                          IntIntPairRealMatrixMap &sum_QlQlm1,
                          Sizet2DArray &num_Q) 
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
  RealMatrix &sum_Q1l = sum_Ql[1], &sum_Q1lm1 = sum_Qlm1[1],
      &sum_Q2l = sum_Ql[2], &sum_Q2lm1 = sum_Qlm1[2],
      &sum_Q3l = sum_Ql[3], &sum_Q3lm1 = sum_Qlm1[3],
      &sum_Q4l = sum_Ql[4], &sum_Q4lm1 = sum_Qlm1[4],
      &sum_Q1lQ1lm1 = sum_QlQlm1[pr11], &sum_Q1lQ2lm1 = sum_QlQlm1[pr12],
      &sum_Q2lQ1lm1 = sum_QlQlm1[pr21], &sum_Q2lQ2lm1 = sum_QlQlm1[pr22];
  for (qoi = 0; qoi < numFunctions; ++qoi) {

    // std error in mean estimate
    lev = 0;
    Nlq = num_Q[lev][qoi];

    bessel_corr = (Real) Nlq / (Real) (Nlq - 1);
    cm1l = sum_Q1l(qoi, lev) / Nlq;
    var_Yl = (sum_Q2l(qoi, lev) / Nlq - cm1l * cm1l) * bessel_corr; // var_Ql
    agg_estim_var = var_Yl / Nlq;
    for (lev = 1; lev < num_lev; ++lev) {
      Nlq = num_Q[lev][qoi];
      cm1l = sum_Q1l(qoi, lev) / Nlq;
      cm1lm1 = sum_Q1lm1(qoi, lev) / Nlq;
      //var_Yl = var_Ql - 2.* covar_QlQlm1 + var_Qlm1;
      var_Yl = (sum_Q2l(qoi, lev) / Nlq - cm1l * cm1l
                - 2. * (sum_Q1lQ1lm1(qoi, lev) / Nlq - cm1l * cm1lm1) +
                sum_Q2lm1(qoi, lev) / Nlq - cm1lm1 * cm1lm1) * bessel_corr;
      agg_estim_var += var_Yl / Nlq;
    }
    check_negative(agg_estim_var);

    finalStatErrors[cntr++] = std::sqrt(agg_estim_var); // std error
    if (outputLevel >= DEBUG_OUTPUT) {
      Cout << "Estimator variance for mean = " << agg_estim_var << "\n";
      Cout << "Estimator SE for mean = " << finalStatErrors[cntr - 1] << "\n";
    }
    // std error in variance or std deviation estimate
    lev = 0;
    Nlq = num_Q[lev][qoi];
    uncentered_to_centered(sum_Q1l(qoi, lev) / Nlq, sum_Q2l(qoi, lev) / Nlq,
                           sum_Q3l(qoi, lev) / Nlq, sum_Q4l(qoi, lev) / Nlq,
                           cm1l, cm2l, cm3l, cm4l, Nlq);
    cm2l_sq = cm2l * cm2l;

    //[fm] bias correction for var_P2l
    var_P2l = Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);
    agg_estim_var = var_P2l / Nlq;
    for (lev = 1; lev < num_lev; ++lev) {
      Nlq = num_Q[lev][qoi];
      mu_Q2l = sum_Q2l(qoi, lev) / Nlq;
      uncentered_to_centered(sum_Q1l(qoi, lev) / Nlq, mu_Q2l,
                             sum_Q3l(qoi, lev) / Nlq, sum_Q4l(qoi, lev) / Nlq,
                             cm1l, cm2l, cm3l, cm4l, Nlq);
      mu_Q2lm1 = sum_Q2lm1(qoi, lev) / Nlq;
      uncentered_to_centered(sum_Q1lm1(qoi, lev) / Nlq, mu_Q2lm1,
                             sum_Q3lm1(qoi, lev) / Nlq, sum_Q4lm1(qoi, lev) / Nlq,
                             cm1lm1, cm2lm1, cm3lm1, cm4lm1, Nlq);
      cm2l_sq = cm2l * cm2l;
      cm2lm1_sq = cm2lm1 * cm2lm1;

      //[fm] unbiased products of mean
      mu_Q2lQ2lm1 = sum_Q2lQ2lm1(qoi, lev) / Nlq;
      mu_Q1lm1_mu_Q2lQ1lm1 = unbiased_mean_product_pair(sum_Q1lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                        sum_Q2lQ2lm1(qoi, lev), Nlq);
      mu_Q1lm1_mu_Q1lm1_muQ2l = unbiased_mean_product_triplet(sum_Q1lm1(qoi, lev), sum_Q1lm1(qoi, lev),
                                                              sum_Q2l(qoi, lev),
                                                              sum_Q2lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                              sum_Q2lQ1lm1(qoi, lev),
                                                              sum_Q2lQ2lm1(qoi, lev), Nlq);
      mu_Q1l_mu_Q1lQ2lm1 = unbiased_mean_product_pair(sum_Q1l(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                      sum_Q2lQ2lm1(qoi, lev), Nlq);
      mu_Q1l_mu_Q1l_mu_Q2lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1l(qoi, lev),
                                                             sum_Q2lm1(qoi, lev),
                                                             sum_Q2l(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                             sum_Q1lQ2lm1(qoi, lev),
                                                             sum_Q2lQ2lm1(qoi, lev), Nlq);
      mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 = unbiased_mean_product_triplet(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                                 sum_Q1lQ1lm1(qoi, lev),
                                                                 sum_Q1lQ1lm1(qoi, lev), sum_Q2lQ1lm1(qoi, lev),
                                                                 sum_Q1lQ2lm1(qoi, lev),
                                                                 sum_Q2lQ2lm1(qoi, lev), Nlq);
      mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1 = unbiased_mean_product_pairpair(sum_Q1l(qoi, lev), sum_Q1lm1(qoi, lev),
                                                                      sum_Q1lQ1lm1(qoi, lev),
                                                                      sum_Q2l(qoi, lev), sum_Q2lm1(qoi, lev),
                                                                      sum_Q2lQ1lm1(qoi, lev), sum_Q1lQ2lm1(qoi, lev),
                                                                      sum_Q2lQ2lm1(qoi, lev), Nlq);
      mu_Q2l_muQ2lm1 = unbiased_mean_product_pair(sum_Q2l(qoi, lev), sum_Q2lm1(qoi, lev), sum_Q2lQ2lm1(qoi, lev),
                                                  Nlq);
      mu_P2lP2lm1 = mu_Q2lQ2lm1 //E[QL2 Ql2]
                    - 2. * mu_Q1lm1_mu_Q2lQ1lm1 //E[Ql] E[QL2Ql]
                    + 2. * mu_Q1lm1_mu_Q1lm1_muQ2l //E[Ql]2 E[QL2]
                    - 2. * mu_Q1l_mu_Q1lQ2lm1 //E[QL] E[QLQl2]
                    + 2. * mu_Q1l_mu_Q1l_mu_Q2lm1 //E[QL]2 E[Ql2]
                    + 4. * mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 //E[QL] E[Ql] E[QLQl]
                    - 4. * mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1 //E[QL]2 E[Ql]2
                    - mu_Q2l_muQ2lm1; //E[QL2] E[Ql2]

      // [fm] bias correction for var_P2l and var_P2lm1
      var_P2l = Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4l - (Nlq - 3.) / (Nlq - 1.) * cm2l_sq);
      var_P2lm1 =
          Nlq * (Nlq - 1.) / (Nlq * Nlq - 2. * Nlq + 3.) * (cm4lm1 - (Nlq - 3.) / (Nlq - 1.) * cm2lm1_sq);

      // [fm] unbiased by opening up the square and compute three different term
      mu_Q1lQ1lm1_mu_Q1lQ1lm1 = unbiased_mean_product_pair(sum_Q1lQ1lm1(qoi, lev), sum_Q1lQ1lm1(qoi, lev),
                                                           sum_Q2lQ2lm1(qoi, lev), Nlq);
      term = mu_Q1lQ1lm1_mu_Q1lQ1lm1 - 2. * mu_Q1l_mu_Qlm1_mu_Q1lQ1lm1 + mu_Q1l_mu_Q1l_mu_Q1lm1_muQ1lm1;

      //[fm] Using an unbiased estimator we include the var_Ql * var_Qlm1 term in mu_P2lP2lm1
      //     and term is already squared out
      covar_P2lP2lm1
          = mu_P2lP2lm1 + term / (Nlq - 1.);
      agg_estim_var += (var_P2l + var_P2lm1 - 2. * covar_P2lP2lm1) / Nlq;

      if (outputLevel >= DEBUG_OUTPUT) {
        Cout << "Estimator for covariance for variance = " << covar_P2lP2lm1 << "\n";
        Cout << "Estimator for covariance for variance first term = " << mu_P2lP2lm1 << "\n";
        Cout << "Estimator for covariance for variance second  term = " << term << "\n";
      }
    }
    check_negative(agg_estim_var);
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Estimator SE for variance = " << sqrt(agg_estim_var) << "\n";

    Real mom2 = momentStats(1, qoi);
    if (finalMomentsType == STANDARD_MOMENTS && mom2 > 0.) {
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

  // NonD::update_aleatory_final_statistics() pushes momentStats into
  // finalStatistics
  update_final_statistics();

  Analyzer::post_run(s);
}


void NonDMultilevelSampling::print_results(std::ostream& s, short results_state)
{
  if (statsFlag) {
    print_multilevel_evaluation_summary(s, NLev);
    s << "<<<<< Equivalent number of high fidelity evaluations: "
      << equivHFEvals << "\n\nStatistics based on multilevel sample set:\n";

  //print_statistics(s);
    print_moments(s, "response function",
		  iteratedModel.truth_model().response_labels());
    archive_moments();
    archive_equiv_hf_evals(equivHFEvals); 
  }
}


static RealVector *static_lev_cost_vec(NULL);
static size_t *static_qoi(NULL);
static Real *static_eps_sq_div_2(NULL);
static RealVector *static_Nlq_pilot(NULL);
static size_t *static_numFunctions(NULL);
static size_t  *static_qoiAggregation(NULL);

static IntRealMatrixMap *static_sum_Ql(NULL);
static IntRealMatrixMap *static_sum_Qlm1(NULL);
static IntIntPairRealMatrixMap *static_sum_QlQlm1(NULL);

void NonDMultilevelSampling::assign_static_member(Real &conv_tol, size_t &qoi, size_t &qoi_aggregation, 
              size_t &num_functions, RealVector &level_cost_vec,
              IntRealMatrixMap &sum_Ql, IntRealMatrixMap &sum_Qlm1,
              IntIntPairRealMatrixMap &sum_QlQlm1,
              RealVector &pilot_samples) const
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
}

static Real *static_mu_four_L(NULL);
static Real *static_mu_four_H(NULL);
static Real *static_var_L(NULL);
static Real *static_var_H(NULL);
static Real *static_Ax(NULL);

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

void NonDMultilevelSampling::target_var_objective_eval_npsol(int &mode, int &n, double *x, double &f, double *gradf,
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
  target_var_objective_eval_optpp(mode, n, optpp_x, f, optpp_grad_f, nstate);

  for (size_t i = 0; i < n && mode; ++i) {
    gradf[i] = optpp_grad_f[i];
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

void NonDMultilevelSampling::target_var_objective_eval_optpp(int mode, int n, const RealVector &x, double &f,
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
                                       compute_gradient, grad_g[0][0]);
      agg_estim_var += agg_estim_var_l[0];
      for (lev = 1; lev < num_lev; ++lev) {
        Nlq = x[lev];
        Nlq_pilot = (*static_Nlq_pilot)[lev];

        agg_estim_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                         compute_gradient, grad_g[0][lev]);
        agg_estim_var += agg_estim_var_l[lev];
      }
    }
  }
  else if(qoiAggregation==QOI_AGGREGATION_MAX){
    agg_estim_var_l[0] = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi,
                                   compute_gradient, grad_g[0][0]);
    agg_estim_var += agg_estim_var_l[0];
    for (lev = 1; lev < num_lev; ++lev) {
      Nlq = x[lev];
      Nlq_pilot = (*static_Nlq_pilot)[lev];

      agg_estim_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                       compute_gradient, grad_g[0][lev]);
      agg_estim_var += agg_estim_var_l[lev];
    }
  }else{
    Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: qoiAggregation is not known.\n";
    abort_handler(INTERFACE_ERROR);
  }

  g[0] = agg_estim_var; // - (*static_eps_sq_div_2);
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

      agg_estim_var_l[0] = variance_Qsum_static((*static_sum_Ql)[1][0][qoi], (*static_sum_Qlm1)[1][0][qoi], (*static_sum_Ql)[2][0][qoi], 
                    (*static_sum_QlQlm1)[pr11][0][qoi], (*static_sum_Qlm1)[2][0][qoi], Nlq_pilot, Nlq, compute_gradient, gradient_var);
      agg_estim_var += agg_estim_var_l[0];

      if(compute_gradient){
      grad_g[0][0] = agg_estim_var_l[0] > 0 ? 1./4. * 
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

        agg_estim_var_l[lev] = variance_Qsum_static((*static_sum_Ql)[1][lev][qoi], (*static_sum_Qlm1)[1][lev][qoi], (*static_sum_Ql)[2][lev][qoi], 
                    (*static_sum_QlQlm1)[pr11][lev][qoi], (*static_sum_Qlm1)[2][lev][qoi], Nlq_pilot, Nlq, compute_gradient, gradient_var);
        agg_estim_var += agg_estim_var_l[lev];

        if(compute_gradient){
          grad_g[0][lev] = agg_estim_var_l[0] > 0 ? 1./4. * 
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

    agg_estim_var_l[0] = variance_Qsum_static((*static_sum_Ql)[1][0][qoi], (*static_sum_Qlm1)[1][0][qoi], (*static_sum_Ql)[2][0][qoi], 
                    (*static_sum_QlQlm1)[pr11][0][qoi], (*static_sum_Qlm1)[2][0][qoi], Nlq_pilot, Nlq, compute_gradient, gradient_var);
    grad_var[0] = gradient_var;
    //Cout << "agg_estim_var_l[0]: " << agg_estim_var_l[0] << "\n";
    //if(compute_gradient){
    //  Cout << "grad_agg_estim_var_l[0]: " << gradient_var << "\n";
    //}

    agg_estim_var += agg_estim_var_l[0];

    for (lev = 1; lev < num_lev; ++lev) {
      Nlq = x[lev];
      Nlq_pilot = (*static_Nlq_pilot)[lev];

      agg_estim_var_of_var_l[lev] = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq, qoi, lev,
                                       compute_gradient, gradient_var_var);
      agg_estim_var_of_var += agg_estim_var_of_var_l[lev];
      grad_var_var[lev] = gradient_var_var;

      agg_estim_var_l[lev] = variance_Qsum_static((*static_sum_Ql)[1][lev][qoi], (*static_sum_Qlm1)[1][lev][qoi], (*static_sum_Ql)[2][lev][qoi], 
                    (*static_sum_QlQlm1)[pr11][lev][qoi], (*static_sum_Qlm1)[2][lev][qoi], Nlq_pilot, Nlq, compute_gradient, gradient_var);
      agg_estim_var += agg_estim_var_l[lev];
      grad_var[lev] = gradient_var;
    }
    if(compute_gradient){
      for (lev = 0; lev < num_lev; ++lev) {
        grad_g[0][lev] = agg_estim_var > 0 ? 
                          1./4. * ( (grad_var_var[lev] * agg_estim_var - agg_estim_var_of_var * grad_var[lev])/(agg_estim_var*agg_estim_var) ) 
                          : 0;
        /*
        //FD test
        Real dh = 0.0000000001;
        Real Nlq_plus = x[lev] + dh;
        Real Nlq_minus = x[lev] - dh;
        size_t Nlq_pilot = (*static_Nlq_pilot)[lev];
        Real agg_estim_var_of_var_l_plus, agg_estim_var_of_var_l_minus;
        if(lev==0){
          agg_estim_var_of_var_l_plus = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq_plus, qoi,
                                         false, gradient_var_var);
          agg_estim_var_of_var_l_minus = var_of_var_ml_l0(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq_minus, qoi,
                                         false, gradient_var_var);
        }else{
          agg_estim_var_of_var_l_plus = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq_plus, qoi, lev,
                                         false, gradient_var_var);
          agg_estim_var_of_var_l_minus = var_of_var_ml_l(*static_sum_Ql, *static_sum_Qlm1, *static_sum_QlQlm1, Nlq_pilot, Nlq_minus, qoi, lev,
                                         false, gradient_var_var);
        }
        Real agg_estim_var_l_plus = variance_Qsum_static((*static_sum_Ql)[1][lev][qoi], (*static_sum_Qlm1)[1][lev][qoi], (*static_sum_Ql)[2][lev][qoi], 
                    (*static_sum_QlQlm1)[pr11][lev][qoi], (*static_sum_Qlm1)[2][lev][qoi], Nlq_pilot, Nlq_plus, false, gradient_var);
        Real agg_estim_var_l_minus = variance_Qsum_static((*static_sum_Ql)[1][lev][qoi], (*static_sum_Qlm1)[1][lev][qoi], (*static_sum_Ql)[2][lev][qoi], 
                    (*static_sum_QlQlm1)[pr11][lev][qoi], (*static_sum_Qlm1)[2][lev][qoi], Nlq_pilot, Nlq_minus, false, gradient_var);

        Real agg_estim_var_of_var_plus = 0;
        Real agg_estim_var_plus = 0;
        Real agg_estim_var_of_var_minus = 0;
        Real agg_estim_var_minus = 0;
        for (size_t lev_in = 0; lev_in < num_lev; ++lev_in) {
          if(lev_in == lev){
            agg_estim_var_of_var_plus += agg_estim_var_of_var_l_plus;
            agg_estim_var_plus += agg_estim_var_l_plus;
            agg_estim_var_of_var_minus += agg_estim_var_of_var_l_minus;
            agg_estim_var_minus += agg_estim_var_l_minus;
          }else{
            agg_estim_var_of_var_plus += agg_estim_var_of_var_l[lev_in];
            agg_estim_var_plus += agg_estim_var_l[lev_in];
            agg_estim_var_of_var_minus += agg_estim_var_of_var_l[lev_in];
            agg_estim_var_minus += agg_estim_var_l[lev_in];
          }
        }
        Real deriv_fd = 1./(2.*dh) * 0.25 * (agg_estim_var_of_var_plus/agg_estim_var_plus - agg_estim_var_of_var_minus/agg_estim_var_minus);
        Real deriv_fwd_fd = 1./(dh) * 0.25 * (agg_estim_var_of_var_plus/agg_estim_var_plus - agg_estim_var_of_var/agg_estim_var);
        Real deriv_bw_fd = 1./(dh) * 0.25 * (agg_estim_var_of_var/agg_estim_var - agg_estim_var_of_var_minus/agg_estim_var_minus);

        Cout << "Lev: " << lev << " Orig: " << grad_g[0][lev] << " FD: " << deriv_fd << " , " << deriv_fwd_fd << " , " << deriv_bw_fd << " Old: " << grad_test[lev] << "\n";
      */
      }
    }
  }else{
    Cout << "NonDMultilevelSampling::multilevel_mc_Qsum: qoiAggregation is not known.\n";
    abort_handler(INTERFACE_ERROR);
  }

  g[0] = agg_estim_var > 0 ? 1./4. * agg_estim_var_of_var/agg_estim_var : 0; // - (*static_eps_sq_div_2);
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
    //grad_g[0][lev] = Hl1_deriv * C1 - Hl2_deriv * C2;
  }
  Cout << "\n#######\n";
  Cout << "x: " << x << "\n";
  Cout << "Nlq: " << Nlq << "\n";
  Cout << "C1: " << C1 << "\n";
  Cout << "C2: " << C2 << "\n";
  Cout << "Hl1: " << Hl1 << "\n";
  Cout << "Hl2: " << Hl2 << "\n";
  Cout << "Cost1L: " << Cost1L << "\n";
  Cout << "Cost2L: " << Cost2L << "\n";
  Cout << "var_of_var_l0: " << var_of_var_l0 << "\n";
  Cout << "###\n";
  Cout << "###\n";
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
    //grad_g[0][lev] = ( Hl1_deriv * C1 - Hl2_deriv * C2
    //                   - 2. * Cost1H * (-1/(Nlq*Nlq))
    //                   - 2. * Cost2H * (1 - 2*Nlq ) / (std::pow(Nlq * (Nlq - 1.), 2)) );
  }
  Cout << "Nlq: " << Nlq << "\n";
  Cout << "Ax_squared: " << Ax_squared << "\n";
  Cout << "var_H_squared: " << var_H_squared << "\n";
  Cout << "C1: " << C1 << "\n";
  Cout << "C2: " << C2 << "\n";
  Cout << "Hl1: " << Hl1 << "\n";
  Cout << "Hl2: " << Hl2 << "\n";
  Cout << "Cost1H: " << Cost1H << "\n";
  Cout << "Cost2H: " << Cost2H << "\n";
  Cout << "var_of_var_l1: " << var_of_var_l1 << "\n";
  Cout << "#######\n\n";
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
  if(compute_gradient && false){
    Hl1_deriv = -1./(Nlq*Nlq);
    Hl2_deriv = ( (Nlq * (Nlq - 1.)) - (Nlq - 3.)*(2. * Nlq - 1) ) / ( std::pow(Nlq * (Nlq - 1.), 2) );
    grad_g[0][lev] = 1./var_of_var_l0 *
        ( Hl1_deriv * C1 - Hl2_deriv * C2 );
    //grad_g[0][lev] = 1./4. * ( (1./var_of_var_l0 * ( Hl1_deriv * C1 - Hl2_deriv * C2 ) * var_H) - () )
    //grad_g[0][lev] = Hl1_deriv * C1 - Hl2_deriv * C2;
  }
  Cout << "\n#######\n";
  Cout << "x: " << x << "\n";
  Cout << "Nlq: " << Nlq << "\n";
  Cout << "C1: " << C1 << "\n";
  Cout << "C2: " << C2 << "\n";
  Cout << "Hl1: " << Hl1 << "\n";
  Cout << "Hl2: " << Hl2 << "\n";
  Cout << "Cost1L: " << Cost1L << "\n";
  Cout << "Cost2L: " << Cost2L << "\n";
  Cout << "var_of_var_l0: " << var_of_var_l0 << "\n";
  Cout << "###\n";
  Cout << "###\n";
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
  if(compute_gradient && false){
    Hl1_deriv = -1./(Nlq*Nlq);
    Hl2_deriv = ( (Nlq * (Nlq - 1.)) - (Nlq - 3.)*(2. * Nlq - 1) ) / ( (Nlq * (Nlq - 1.))*(Nlq * (Nlq - 1.)) );
    grad_g[0][lev] = 1./var_of_var_l1 *
                     ( Hl1_deriv * C1 - Hl2_deriv * C2
                        - 2. * Cost1H * (-1/(Nlq*Nlq))
                        - 2. * Cost2H * (1 - 2*Nlq ) / (std::pow(Nlq * (Nlq - 1.), 2)) );       
  }
  Cout << "Nlq: " << Nlq << "\n";
  Cout << "Ax_squared: " << Ax_squared << "\n";
  Cout << "var_H_squared: " << var_H_squared << "\n";
  Cout << "C1: " << C1 << "\n";
  Cout << "C2: " << C2 << "\n";
  Cout << "Hl1: " << Hl1 << "\n";
  Cout << "Hl2: " << Hl2 << "\n";
  Cout << "Cost1H: " << Cost1H << "\n";
  Cout << "Cost2H: " << Cost2H << "\n";
  Cout << "var_of_var_l1: " << var_of_var_l1 << "\n";
  Cout << "#######\n\n";
  //Final var
  Real var_of_var_ml = var_of_var_l0 + var_of_var_l1;
  g[0] = std::log(1./(4. * var_H ) * var_of_var_ml);
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
