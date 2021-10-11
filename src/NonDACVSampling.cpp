/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDACVSampling
//- Description: Implementation code for NonDACVSampling class
//- Owner:       Mike Eldred
//- Checked by:
//- Version:

#include "dakota_system_defs.hpp"
#include "dakota_data_io.hpp"
//#include "dakota_tabular_io.hpp"
#include "DakotaModel.hpp"
#include "DakotaResponse.hpp"
#include "NonDACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"
#include "DakotaIterator.hpp"

#ifdef HAVE_NPSOL
#include "NPSOLOptimizer.hpp"
#endif
#ifdef HAVE_OPTPP
#include "SNLLOptimizer.hpp"
#endif

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {

// initialization of statics
NonDACVSampling* NonDACVSampling::acvInstance(NULL);


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDNonHierarchSampling(problem_db, model),
  acvSubMethod(problem_db.get_ushort("method.sub_method")),
  truthFixedByPilot(problem_db.get_bool("method.nond.truth_fixed_by_pilot"))
{
  // truthFixedByPilot is a user-specified option for fixing the number of HF
  // samples (those from the pilot).  In this case, equivHF budget is allocated
  // by optimizing r* for fixed N.
  optSubProblemForm = (truthFixedByPilot && solutionMode != OFFLINE_PILOT) ?
    R_ONLY_LINEAR_CONSTRAINT : N_VECTOR_LINEAR_CONSTRAINT;
  // default solver to OPT++ NIP based on numerical experience
  unsigned short opt_subprob_solver = sub_optimizer_select(
    probDescDB.get_ushort("method.nond.opt_subproblem_solver"), SUBMETHOD_NIP);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "ACV sub-method selection = " << acvSubMethod
	 << " sub-method formulation = "  << optSubProblemForm
	 << " sub-problem solver = "      << opt_subprob_solver << std::endl;

  // Use NPSOL/OPT++ with obj/constr callbacks to minmize estimator variance
  // > TO DO: add sqp | nip specification

  // Notes on optimization:
  // > Budget C is fixed and numH is fixed for this iteration --> design vars
  //   are eval_ratios for 1:numApprox.  eval_ratio lower bounds are set to 1
  //   for now, but could also reflect the pilot sample investment (C and c
  //   can use different pilot sample sizes).
  //   >> if numH is also a design variable, then lower bounds should be 1.
  //   >> when an optimal ratio is 1, this model drops from apply_control(),
  //      although it may still influence the other model weightings.
  // > a linear inequality is used for the cost constraint and can also be
  //   used for eval_ratio(i) > eval_ratio(i+1), but omit for now (restricts
  //   optimizer search space = most appropriate when sequencing models)
  RealVector lin_ineq_lb, lin_ineq_ub, lin_eq_tgt,
             nln_ineq_lb, nln_ineq_ub, nln_eq_tgt;
  RealMatrix lin_ineq_coeffs, lin_eq_coeffs;
  size_t num_cdv = (optSubProblemForm == R_ONLY_LINEAR_CONSTRAINT) ?
    numApprox : numApprox + 1, max_iter = 100000;
  RealVector x0(num_cdv, false), x_lb(num_cdv, false), x_ub(num_cdv, false);
  x_ub = DBL_MAX; // no upper bounds
  Real conv_tol = 1.e-8; // tight convergence
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    lin_ineq_lb.sizeUninitialized(1); lin_ineq_lb[0] = -DBL_MAX; // no low bnd
    lin_ineq_ub.sizeUninitialized(1); lin_ineq_ub[0] = (Real)maxFunctionEvals;
    lin_ineq_coeffs.shapeUninitialized(1, numApprox);
    lin_ineq_coeffs = 1.; // updated in compute_ratios()
    x0 = x_lb = 1.;
    break;
  case N_VECTOR_LINEAR_CONSTRAINT: {
    size_t num_lin_con = numApprox + 1;
    lin_ineq_lb.sizeUninitialized(num_lin_con);
    lin_ineq_lb = -DBL_MAX; // no lower bnds
    lin_ineq_ub.size(num_lin_con); // init to 0
    lin_ineq_ub[0] = (Real)maxFunctionEvals; // 1st is budget constraint
    lin_ineq_coeffs.shapeUninitialized(num_lin_con, num_cdv);
    lin_ineq_coeffs = 1.; // dummy updated in compute_ratios()
    x0 = x_lb = (solutionMode == OFFLINE_PILOT) ? 1. :
      (Real)pilotSamples[numApprox]; // *** TO DO: copy array
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT:
    nln_ineq_lb.sizeUninitialized(1); nln_ineq_lb[0] = -DBL_MAX; // no low bnd
    nln_ineq_ub.sizeUninitialized(1); nln_ineq_ub[0] = (Real)maxFunctionEvals;
    x0 = x_lb = 1.; // r_i
    x0[numApprox] = x_lb[numApprox] = (solutionMode == OFFLINE_PILOT) ? 1. :
      (Real)pilotSamples[numApprox]; // pilot <= N*
    break;
  }

  switch (opt_subprob_solver) {
  case SUBMETHOD_SQP: {
    int deriv_level = (optSubProblemForm == R_AND_N_NONLINEAR_CONSTRAINT)
      ? 2 : 0; // 0 neither, 1 obj, 2 constr, 3 both
#ifdef HAVE_NPSOL
    varianceMinimizer.assign_rep(std::make_shared<NPSOLOptimizer>(x0, x_lb,
      x_ub, lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs,
      lin_eq_tgt, nln_ineq_lb, nln_ineq_ub, nln_eq_tgt,
      npsol_objective_evaluator, npsol_constraint_evaluator, deriv_level,
      conv_tol, max_iter));
#endif
    break;
  }
  case SUBMETHOD_NIP: {
    size_t max_eval = 500000;  Real max_step = 100000.;
#ifdef HAVE_OPTPP
    varianceMinimizer.assign_rep(std::make_shared<SNLLOptimizer>(x0,x_lb,x_ub,
      lin_ineq_coeffs, lin_ineq_lb, lin_ineq_ub, lin_eq_coeffs, lin_eq_tgt,
      nln_ineq_lb, nln_ineq_ub, nln_eq_tgt, optpp_objective_evaluator,
      optpp_constraint_evaluator, max_iter, max_eval, conv_tol, conv_tol,
      max_step));
#endif
    break;
  }
  default: // SUBMETHOD_NONE, ...
    Cerr << "Error: sub-problem solver undefined in NonDACVSampling."
	 << std::endl;
    abort_handler(METHOD_ERROR);
    break;
  }
}


NonDACVSampling::~NonDACVSampling()
{ }


void NonDACVSampling::pre_run()
{
  NonDNonHierarchSampling::pre_run();

  // reset sample counters to 0
  acvInstance = this;
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDACVSampling::core_run()
{
  /*
  switch (acvSubMethod) {
  case SUBMETHOD_ACV_IS:  case SUBMETHOD_ACV_MF:
    approximate_control_variate(); break;
  //case SUBMETHOD_ACV_KL:
    //for (k) for (l) approximate_control_variate(...); ???
  }
  */
  if (acvSubMethod == SUBMETHOD_ACV_KL) {
    Cerr << "Error: ACV KL not yet implemented." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  switch (solutionMode) {
  case  ONLINE_PILOT: // iterated ACV (default)
    approximate_control_variate();                  break;
  case OFFLINE_PILOT: // computes perf for offline pilot/Oracle correlation
    approximate_control_variate_offline_pilot();    break;
  case PILOT_PROJECTION: // for algorithm assessment/selection
    approximate_control_variate_pilot_projection(); break;
  }
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate()
{
  // Performs pilot + LF increment and then iterates with additional shared
  // increment + LF increment batches until prescribed MSE reduction is obtained

  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, avg_eval_ratios;  RealMatrix var_L;
  Sizet2DArray N_L_baselineH, N_LH;    SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  initialize_acv_counts(N_L_baselineH,  numH,   N_LL,   N_LH);
  //initialize_acv_covariances(covLL, covLH, varH);

  // Initialize for pilot sample
  size_t num_steps = numApprox+1, hf_shared_pilot;//, start=0, lf_shared_pilot;
  numSamples = hf_shared_pilot = pilotSamples[numApprox]; // last in array
  //lf_shared_pilot = find_min(pilotSamples, start, numApprox-1);

  Real avg_hf_target = 0.;
  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,MSE}_ratios
    // --------------------------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_L_baselineH, /*N_L_baselineL,*/
			numH, N_LL, N_LH);
    increment_equivalent_cost(numSamples, sequenceCost, 0, num_steps);
    // allow pilot to vary for C vs c
    // *** TO DO: numSamples logic after pilot (mlmfIter >= 1)
    // *** Will likely require _baselineL and _baselineH
    //if (mlmfIter == 0 && lf_shared_pilot > hf_shared_pilot) {
    //  numSamples = lf_shared_pilot - hf_shared_pilot;
    //  shared_approx_increment(mlmfIter); // spans all approx models
    //  accumulate_acv_sums(sum_L_baselineL, sum_LL,//_baselineL,
    //                      N_L_baselineL, N_LL);//_baselineL);
    //  increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox);
    //}

    const RealMatrix&         sum_L_1  = sum_L_baselineH[1];
    const RealVector&         sum_H_1  = sum_H[1];
    const RealSymMatrixArray& sum_LL_1 = sum_LL[1];
    compute_variance(sum_H_1, sum_HH, numH, varH);
    if (mlmfIter==0) compute_L_variance(sum_L_1, sum_LL_1, N_L_baselineH,var_L);
    compute_LH_covariance(sum_L_1/*baseH*/, sum_H_1, sum_LH[1], N_L_baselineH,
			  numH, N_LH, covLH);
    compute_LL_covariance(sum_L_1/*baseL*/, sum_LL_1, N_L_baselineH/*baseL*/,
			  N_LL, covLL);
    //Cout << "var_H:\n"<< var_H << "cov_LH:\n"<< cov_LH << "cov_LL:\n"<<cov_LL;

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(numH, var_L, varH, covLH, sequenceCost, avg_eval_ratios,
		   avgACVEstVar, avgMSERatio, avg_hf_target);

    ++mlmfIter;
  }

  approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_L_baselineH,
		    N_LL, N_LH, avg_eval_ratios, avg_hf_target);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate_offline_pilot()
{
  RealVector sum_H_pilot(numFunctions), sum_HH_pilot(numFunctions);
  RealMatrix sum_L_pilot(numFunctions, numApprox),
    sum_LH_pilot(numFunctions, numApprox), var_L;
  RealSymMatrixArray sum_LL_pilot(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    sum_LL_pilot[qoi].shape(numApprox);
  SizetArray N_H_pilot;  Sizet2DArray N_L_pilot, N_LH_pilot;
  SizetSymMatrixArray N_LL_pilot;
  initialize_acv_counts(N_L_pilot, N_H_pilot, N_LL_pilot, N_LH_pilot);
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  // Initialize for pilot sample
  size_t num_steps = numApprox + 1, hf_shared_pilot;
  numSamples = hf_shared_pilot = pilotSamples[numApprox]; // last in array
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot, N_L_pilot, N_H_pilot,N_LL_pilot,N_LH_pilot);
  //increment_equivalent_cost(numSamples, sequenceCost, 0, num_steps);
  compute_variance(sum_H_pilot, sum_HH_pilot, N_H_pilot, varH);
  compute_L_variance(sum_L_pilot, sum_LL_pilot, N_L_pilot, var_L);
  compute_LH_covariance(sum_L_pilot, sum_H_pilot, sum_LH_pilot, N_L_pilot,
			N_H_pilot, N_LH_pilot, covLH);
  compute_LL_covariance(sum_L_pilot, sum_LL_pilot, N_L_pilot, N_LL_pilot,covLL);
  //Cout << "var_H:\n"<< var_H << "cov_LH:\n"<< cov_LH << "cov_LL:\n"<<cov_LL;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, avg_eval_ratios;
  Sizet2DArray N_L_baselineH, N_LH;    SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  initialize_acv_counts(N_L_baselineH,  numH,   N_LL,   N_LH);
  //initialize_acv_covariances(covLL, covLH, varH);
  Real avg_hf_target = 0.;

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(numH, var_L, varH, covLH, sequenceCost, avg_eval_ratios,
		 avgACVEstVar, avgMSERatio, avg_hf_target);

  // at least 1 sample (?) reqd for numerics (+ resetting allSamples from pilot)
  if (numSamples < 1) numSamples = 1;
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_L_baselineH, /*N_L_baselineL,*/
		      numH, N_LL, N_LH);
  increment_equivalent_cost(numSamples, sequenceCost, 0, num_steps);
  // allow pilot to vary for C vs c

  approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_L_baselineH,
		    N_LL, N_LH, avg_eval_ratios, avg_hf_target);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDACVSampling::approximate_control_variate_pilot_projection()
{
  RealVector sum_H(numFunctions), sum_HH(numFunctions), avg_eval_ratios;
  RealMatrix sum_L_baselineH(numFunctions, numApprox),
    sum_LH(numFunctions, numApprox), var_L;
  RealSymMatrixArray sum_LL(numFunctions);
  for (size_t qoi=0; qoi<numFunctions; ++qoi)
    sum_LL[qoi].shape(numApprox);
  Sizet2DArray N_L_baselineH, N_LH; SizetSymMatrixArray N_LL;
  initialize_acv_counts(N_L_baselineH,  numH,   N_LL,   N_LH);
  Real avg_hf_target = 0.;

  // Initialize for pilot sample
  size_t num_steps = numApprox+1, hf_shared_pilot;//, start=0, lf_shared_pilot;
  numSamples = hf_shared_pilot = pilotSamples[numApprox]; // last in array
  //lf_shared_pilot = find_min(pilotSamples, start, numApprox-1);

  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,MSE}_ratios
  // --------------------------------------------------------------------
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_L_baselineH, /*N_L_baselineL,*/
		      numH, N_LL, N_LH);
  increment_equivalent_cost(numSamples, sequenceCost, 0, num_steps);
  // allow pilot to vary for C vs c

  compute_variance(sum_H, sum_HH, numH, varH);
  compute_L_variance(sum_L_baselineH, sum_LL, N_L_baselineH, var_L);
  compute_LH_covariance(sum_L_baselineH, sum_H, sum_LH, N_L_baselineH, numH,
			N_LH, covLH);
  compute_LL_covariance(sum_L_baselineH/*baseL*/, sum_LL,
			N_L_baselineH/*baseL*/,	N_LL, covLL);
  //Cout << "var_H:\n"<< var_H << "cov_LH:\n"<< cov_LH << "cov_LL:\n"<<cov_LL;

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(numH, var_L, varH, covLH, sequenceCost, avg_eval_ratios,
		 avgACVEstVar, avgMSERatio, avg_hf_target);

  // No LF increments or final moments for pilot projection

  // update projected numH
  Sizet2DArray N_L_projected = N_L_baselineH;
  update_projected_samples(avg_hf_target, avg_eval_ratios, numH, N_L_projected);
  finalize_counts(N_L_projected);
}


void NonDACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baselineH, IntRealVectorMap& sum_H,
		  IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		  const Sizet2DArray& N_L_baselineH,
		  const SizetSymMatrixArray& N_LL, const Sizet2DArray& N_LH,
		  const RealVector& avg_eval_ratios, Real avg_hf_target)
{
  // ----------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final numH
  // ----------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after numH has converged, which simplifies maxFnEvals / convTol logic
  // (no need to further interrogate these throttles below)

  // maxIterations == 0 is no longer reserved for the pilot only case.
  // See notes in NonDMultifidelitySampling::multifidelity_mc().

  IntRealMatrixMap sum_L_refined = sum_L_baselineH;//baselineL;
  Sizet2DArray       N_L_refined =   N_L_baselineH;//baselineL;
  size_t start, approx;
  for (approx=numApprox; approx>0; --approx) {
    // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER 1ST PASS LAUNCH
    start = (acvSubMethod == SUBMETHOD_ACV_IS) ? approx - 1 : 0;
    if (approx_increment(avg_eval_ratios, N_L_refined, avg_hf_target,
			 mlmfIter, start, approx)) {
      // ACV_IS samples on [approx-1,approx) --> sum_L_refined
      // ACV_MF samples on [0, approx)       --> sum_L_refined
      accumulate_acv_sums(sum_L_refined, N_L_refined, start, approx);
      increment_equivalent_cost(numSamples, sequenceCost, start, approx);
    }
  }

  // -----------------------------------------------------------
  // Compute/apply control variate parameter to estimate moments
  // -----------------------------------------------------------
  RealMatrix H_raw_mom(numFunctions, 4);
  acv_raw_moments(sum_L_baselineH, sum_L_refined, sum_H, sum_LL, sum_LH,
		  avg_eval_ratios, N_L_baselineH, N_L_refined, numH, N_LL,
		  N_LH, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_refined);
}


bool NonDACVSampling::
approx_increment(const RealVector& avg_eval_ratios,
		 const Sizet2DArray& N_L_refined, Real hf_target,
		 size_t iter, size_t start, size_t end)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts
  // > eval_ratios have been scaled if needed to satisfy specified budget
  //   (maxFunctionEvals) in
  size_t qoi, approx = end - 1;
  Real lf_target = avg_eval_ratios[approx] * hf_target;
  // Choose avg, RMS, max? (trade-off: possible overshoot vs. more iteration)
  numSamples = one_sided_delta(average(N_L_refined[approx]), lf_target);

  if (numSamples && start < end) {
    Cout << "\nACV sample increment = " << numSamples
	 << " for approximations [" << start+1 << ", " << end << ']';
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << " computed from average delta between target " << lf_target
	   << " and current average count " << average(N_L_refined[approx]);
    Cout << std::endl;
    size_t start_qoi = start * numFunctions, end_qoi = end * numFunctions;
    activeSet.request_values(0);
    //activeSet.request_values(0, 0, start_qoi);
    activeSet.request_values(1, start_qoi, end_qoi);
    //activeSet.request_values(0, end_qoi, iteratedModel.response_size());
    ensemble_sample_increment(iter, start); // NON-BLOCK
    return true;
  }
  else {
    Cout << "\nNo ACV approx sample increment for approximations ["
	 << start+1 << ", " << end << ']' << std::endl;
    return false;
  }
}


void NonDACVSampling::
compute_ratios(const SizetArray& N_H,   const RealMatrix& var_L,
	       const RealVector& var_H, const RealMatrix& cov_LH,
	       const RealVector& cost,  RealVector& avg_eval_ratios,
	       Real& avg_acv_estvar,    Real& avg_estvar_ratio,
	       Real& avg_hf_target)
{
  // --------------------------------------
  // Configure the optimization sub-problem
  // --------------------------------------
  // Set initial guess based on MFMC eval ratios (iter 0) or warm started from
  // previous solution
  Real avg_N_H = average(N_H);
  // Modify budget to allow a feasible soln (var lower bnds: r_i > 1, N > N_H).
  // Can happen if shared pilot rolls up to exceed budget spec.
  Real     budget = (Real)maxFunctionEvals;
  bool budget_exhausted = (equivHFEvals >= budget);
  if (budget_exhausted) budget = equivHFEvals;
  if (mlmfIter == 0) {
    // mseIter0 only uses HF pilot since sum_L_shared / N_shared minus
    // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
    // (This differs from MLMC MSE^0 which uses pilot for all levels.)
    // Note: could revisit this for case of lf_shared_pilot > hf_shared_pilot.
    compute_mc_estimator_variance(var_H, N_H, mseIter0); // N_H==0 is protected

    if (budget_exhausted) { // there is only 1 feasible pt, no need for solve
      if (avg_eval_ratios.empty()) avg_eval_ratios.sizeUninitialized(numApprox);
      numSamples = 0;  avg_eval_ratios = 1.;  avg_hf_target = avg_N_H;
      avg_acv_estvar = average(mseIter0);     avg_estvar_ratio = 1.;
      return;
    }
    else { // compute initial estimate of r* from MFMC
      RealMatrix rho2_LH, eval_ratios;
      covariance_to_correlation_sq(cov_LH, var_L, var_H, rho2_LH);
      mfmc_eval_ratios(rho2_LH, cost, eval_ratios);
      average(eval_ratios, 0, avg_eval_ratios);// avg over qoi for each approx
      if (outputLevel >= NORMAL_OUTPUT)
        Cout << "Initial guess from MFMC (avg eval ratios):\n"
	     << avg_eval_ratios << std::endl;
      // scale to enforce budget constraint.  Since the profile does not emerge
      // from pilot in ACV, don't select an infeasible initial guess:
      // > if N* < N_pilot, scale back r* for use initial = scaled_r*,N_pilot
      // > if N* > N_pilot, use initial = r*,N*
      avg_hf_target = allocate_budget(avg_eval_ratios, cost);
      if (avg_N_H > avg_hf_target) { // rescale r* for over-estimated pilot
	scale_to_budget_with_pilot(budget, avg_eval_ratios, cost, avg_N_H);
	avg_hf_target = avg_N_H;
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "MFMC initial guess rescaled to budget:\n" << avg_eval_ratios
	       << std::endl;
      }
    }
  }
  // else warm start from previous solution
  // > no scaling needed from prev soln (as in NonDLocalReliability) since
  //   updated avg_N_H now includes allocation from previous solution and is
  //   active on constraint bound (excepting integer sample rounding)

  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    varianceMinimizer.initial_point(avg_eval_ratios);

    // set linear inequality constraint for fixed N:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   \Sum_i w_i   r_i <= equivHF * w / N - w
    //   \Sum_i w_i/w r_i <= equivHF / N - 1
    RealVector lin_ineq_lb(1, false), lin_ineq_ub(1, false), lin_eq_tgt;
    RealMatrix lin_ineq_coeffs(1, numApprox, false), lin_eq_coeffs;
    Real cost_H = cost[numApprox];
    lin_ineq_lb[0] = -DBL_MAX;        // no lower bound
    lin_ineq_ub[0] = (avg_N_H > 1.) ? // protect N_H==0 for offline pilot
      budget / avg_N_H - 1. : // normal case
      budget - 1.;            // bound N_H at 1 (TO DO: need to perform sample)
    for (size_t approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0,approx) = cost[approx] / cost_H;
    // we update the Minimizer for user-functions mode (no iteratedModel)
    varianceMinimizer.linear_constraints(lin_ineq_coeffs, lin_ineq_lb,
					 lin_ineq_ub, lin_eq_coeffs,lin_eq_tgt);
    break;
  }
  case N_VECTOR_LINEAR_CONSTRAINT: {
    size_t num_cdv = numApprox+1, num_lin_con = numApprox+1;
    RealVector cv0(num_cdv);
    copy_data_partial(avg_eval_ratios, cv0, 0);  cv0[numApprox] = 1.;
    if (mlmfIter) cv0.scale(avg_N_H); // {N} = [ {r_i}, 1 ] * N_hf
    else          cv0.scale(avg_hf_target);
    varianceMinimizer.initial_point(cv0);
    //Cout << "Variance minimizer initial guess cv0 =\n" << cv0;

    RealVector lin_ineq_lb(num_lin_con, false), lin_ineq_ub(num_lin_con),
               lin_eq_tgt;
    RealMatrix lin_ineq_coeffs(num_lin_con, num_cdv), lin_eq_coeffs;
    lin_ineq_lb = -DBL_MAX; // no lower bounds
    // linear inequality constraint on budget:
    //   N ( w + \Sum_i w_i r_i ) <= C, where C = equivHF * w
    //   N w + \Sum_i w_i N_i <= equivHF * w
    //   N + \Sum_i w_i/w N_i <= equivHF
    lin_ineq_ub[0] = budget;
    Real cost_H = cost[numApprox];
    for (size_t approx=0; approx<numApprox; ++approx)
      lin_ineq_coeffs(0,approx) = cost[approx] / cost_H;
    lin_ineq_coeffs(0,numApprox) = 1.;
    // linear inequality constraints on N_i > N prevent numerical exceptions:
    // N_i >= N transformed to N_i > N using RATIO_NUDGE
    for (size_t approx=1; approx<=numApprox; ++approx) {
      lin_ineq_coeffs(approx, approx-1) = -1.;
      lin_ineq_coeffs(approx,numApprox) =  1. + RATIO_NUDGE;// N_i > N (r_i > 1)
      //lin_ineq_coeffs(approx,approx)  =  1.;// enforce N_i >= N_{i+1}
    }
    // we update the Minimizer for user-functions mode (no iteratedModel)
    varianceMinimizer.linear_constraints(lin_ineq_coeffs, lin_ineq_lb,
					 lin_ineq_ub, lin_eq_coeffs,lin_eq_tgt);
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: {
    RealVector cv0(numApprox+1);
    copy_data_partial(avg_eval_ratios, cv0, 0);          // r_i
    cv0[numApprox] = (mlmfIter) ? avg_N_H : avg_hf_target; // N
    varianceMinimizer.initial_point(cv0);

    // increase nln ineq upper bnd if accumulated cost has exceeded maxFnEvals
    if (budget_exhausted) {
      RealVector nln_ineq_lb(1), nln_ineq_ub(1), nln_eq_tgt;
      nln_ineq_lb[0] = -DBL_MAX;  nln_ineq_ub[0] = budget;
      varianceMinimizer.nonlinear_constraints(nln_ineq_lb, nln_ineq_ub,
					      nln_eq_tgt);
    }

    /*
    // For this case, could allow the optimal profile to emerge from pilot by
    // allowing N* less than the incurred cost (e.g., setting N_lb to 1), but
    // instead we target r*,N* subject to the incurred cost of shared samples
    // by updating N_lb = latest N_H and retaining r_lb = 1.
    size_t num_cdv = numApprox + 1; // evaluation ratios, N_H
    RealVector  x_lb(num_cdv, false), x_ub(num_cdv, false);
    x_lb = 1.;  x_lb[numApprox] = avg_N_H;//std::floor(avg_N_H + .5);
    x_ub = DBL_MAX; // no upper bounds
    varianceMinimizer.variable_bounds(x_lb, x_ub);
    */

    // Consider adding linear constraints suggested in ACV paper
    break;
  }
  }

  // ----------------------------------
  // Solve the optimization sub-problem
  // ----------------------------------
  // compute optimal r*,N* (or r* for fixed N) that maximizes variance reduction
  varianceMinimizer.run();

  // -------------------------------------
  // Post-process the optimization results
  // -------------------------------------
  // Recover optimizer results for average {eval,mse} ratios.  Also compute
  // shared increment from N* or from targeting specified budget or MSE.
  const RealVector& cv_star
    = varianceMinimizer.variables_results().continuous_variables();
  const RealVector& fn_star
    = varianceMinimizer.response_results().function_values();
  //Cout << "Minimizer results:\ncv_star =\n"<<cv_star<<"fn_star =\n"<<fn_star;

  // Objective recovery from optimizer provides std::log(average(acv_estvar))
  // (a QoI-vector prior to averaging would require recomputation from r*,N*)
  // Note: this value corresponds to N* (_after_ numSamples applied)
  avg_acv_estvar = std::exp(fn_star(0)); // var_H / N_H (1 - R^2)

  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT:
    copy_data(cv_star, avg_eval_ratios); // r*
    // Allow for constraint to be inactive at optimum, but generally the
    // opt sub-prob will allocate full budget (--> numSamples = deltaN = 0)
    // since larger eval ratios will increase R^2.
    // Important: r* for fixed N is suboptimal w.r.t. r*,N* --> unlike MFMC,
    // this approach does not converge to the desired sample profile, except
    // for special case where user specifies a fixed N_H + total budget
    // (see option "truth_fixed_by_pilot").
    if (maxFunctionEvals != SZ_MAX) {
      // Full budget allocation: pilot sample + addtnl N_H; then optimal N_L
      // > can also under-relax the budget allocation to enable additional N_H
      //   increments + associated shared sample sets to refine rho2_LH et al.
      Cout << "Scaling profile for maxFunctionEvals = " << maxFunctionEvals;
      avg_hf_target = allocate_budget(avg_eval_ratios, cost);
    }
    else { //if (convergenceTol != -DBL_MAX) { // *** TO DO: detect user spec
      // MSE target = convTol * mse_iter0 = mse_ratio * varH / N_target
      //            = curr_mse * N_curr / N_target
      // --> N_target = curr_mse * N_curr / (convTol * mse_iter0)
      // Note: mse_iter0 is fixed based on pilot
      Cout << "Scaling profile for convergenceTol = " << convergenceTol;
      avg_hf_target = avg_acv_estvar * avg_N_H
	            / (convergenceTol * average(mseIter0));
    }
    //avg_hf_target = std::min(budget_target, ctol_target); // enforce both
    Cout << ": average HF target = " << avg_hf_target << std::endl;
    break;
  case N_VECTOR_LINEAR_CONSTRAINT:  case R_AND_N_NONLINEAR_CONSTRAINT:
    // N_VECTOR: N*_i is leading part of r_and_N and N* is trailing part
    // R_AND_N:  r*   is leading part of r_and_N and N* is trailing part
    copy_data_partial(cv_star, 0, (int)numApprox, avg_eval_ratios); // r_i | N_i
    avg_hf_target = cv_star[numApprox];                             // N*
    if (optSubProblemForm == N_VECTOR_LINEAR_CONSTRAINT)
      avg_eval_ratios.scale(1./avg_hf_target); // N_i -> r_i
    break;
  }

  // compute sample increment for HF from current to target:
  numSamples = (truthFixedByPilot) ? 0 :
    one_sided_delta(avg_N_H, avg_hf_target);

  //if (finished) { // metrics not needed unless print_variance_reduction()

  // All cases employ a projected MC MSE to match the projected ACV MSE from N*
  // (N* may include a numSamples increment not yet performed)
  RealVector mc_estvar;
  project_mc_estimator_variance(var_H, N_H, numSamples, mc_estvar);
  Real avg_mc_estvar = average(mc_estvar);

  // Report ratio of averages rather that average of ratios (see notes in
  // print_variance_reduction())
  avg_estvar_ratio = avg_acv_estvar / avg_mc_estvar;  // (1 - R^2)
  //RealVector estvar_ratio(numFunctions, false);
  //for (size_t qoi=0; qoi<numFunctions; ++qoi)
  //  estvar_ratio[qoi] = 1. - R_sq[qoi];// compute from CF_inv,A->compute_Rsq()
  //avg_estvar_ratio = average(estvar_ratio);

  //}

  if (outputLevel >= NORMAL_OUTPUT) {
    for (size_t approx=0; approx<numApprox; ++approx)
      Cout << "Approx " << approx+1 << ": average evaluation ratio = "
	   << avg_eval_ratios[approx] << '\n';
    Cout << "Average ACV variance / average MC variance = "
	 << avg_estvar_ratio << std::endl;
  }
}


/** This version used by ACV following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    IntRealMatrixMap&         sum_LH, // each L with H
		    RealVector& sum_HH, Sizet2DArray& num_L_baseline,
		    SizetArray& num_H,  SizetSymMatrixArray& num_LL,
		    Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, lf2_fn, hf_fn, lf_prod, lf2_prod, hf_prod;
  IntRespMCIter r_it;              IntRVMIter    h_it;
  IntRMMIter lb_it, lr_it, lh_it;  IntRSMAMIter ll_it;
  int lb_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    hf_index = numApprox * numFunctions;

    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = fn_vals[hf_index];
      hf_is_finite = isfinite(hf_fn);
      // High accumulations:
      if (hf_is_finite) { // neither NaN nor +/-Inf
	++num_H[qoi];
	// High-High:
	sum_HH[qoi] += hf_fn * hf_fn; // a single vector for ord 1
	// High:
	h_it = sum_H.begin();  h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	hf_prod = hf_fn;       active_ord = 1;
	while (h_ord) {
	  if (h_ord == active_ord) { // support general key sequence
	    h_it->second[qoi] += hf_prod;
	    ++h_it; h_ord = (h_it == sum_H.end()) ? 0 : h_it->first;
	  }
	  hf_prod *= hf_fn;  ++active_ord;
	}
      }
	
      SizetSymMatrix& num_LL_q = num_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_baseline[approx][qoi];
	  ++num_LL_q(approx,approx); // Diagonal of C matrix
	  if (hf_is_finite) ++num_LH[approx][qoi]; // pull out of moment loop

	  lb_it = sum_L_baseline.begin();
	  ll_it = sum_LL.begin();  lh_it = sum_LH.begin();
	  lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	  ll_ord = (ll_it == sum_LL.end())         ? 0 : ll_it->first;
	  lh_ord = (lh_it == sum_LH.end())         ? 0 : lh_it->first;
	  lf_prod = lf_fn;  active_ord = 1;
	  while (lb_ord || ll_ord || lh_ord) {
    
	    // Low baseline
	    if (lb_ord == active_ord) { // support general key sequence
	      lb_it->second(qoi,approx) += lf_prod;  ++lb_it;
	      lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      ll_it->second[qoi](approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_fn = fn_vals[lf2_index];

		if (isfinite(lf2_fn)) { // both are finite
		  if (active_ord == 1) ++num_LL_q(approx,approx2);
		  lf2_prod = lf2_fn;
		  for (m=1; m<active_ord; ++m)
		    lf2_prod *= lf2_fn;
		  ll_it->second[qoi](approx,approx2) += lf_prod * lf2_prod;
		}
	      }
	      ++ll_it; ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High (c vector for each QoI):
	    if (lh_ord == active_ord) {
	      if (hf_is_finite) {
		hf_prod = hf_fn;
		for (m=1; m<active_ord; ++m)
		  hf_prod *= hf_fn;
		lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      }
	      ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** This version used by ACV following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
		    RealSymMatrixArray& sum_LL, // L w/ itself + other L
		    RealMatrix&         sum_LH, // each L with H
		    RealVector& sum_HH, Sizet2DArray& num_L_baseline,
		    SizetArray& num_H,  SizetSymMatrixArray& num_LL,
		    Sizet2DArray& num_LH)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;
  Real lf_fn, lf2_fn, hf_fn;
  IntRespMCIter r_it;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;
  bool hf_is_finite;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();
    hf_index = numApprox * numFunctions;

    for (qoi=0; qoi<numFunctions; ++qoi, ++hf_index) {
      hf_fn = fn_vals[hf_index];
      hf_is_finite = isfinite(hf_fn);
      // High accumulations:
      if (hf_is_finite) { // neither NaN nor +/-Inf
	++num_H[qoi];
	sum_H[qoi]  += hf_fn;         // High
	sum_HH[qoi] += hf_fn * hf_fn; // High-High
      }
	
      SizetSymMatrix& num_LL_q = num_LL[qoi];
      RealSymMatrix&  sum_LL_q = sum_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_baseline[approx][qoi];
	  sum_L_baseline(qoi,approx) += lf_fn; // Low

	  ++num_LL_q(approx,approx); // Diagonal of C matrix
	  sum_LL_q(approx,approx) += lf_fn * lf_fn; // Low-Low
	  // Off-diagonal of C matrix:
	  // look back (only) for single capture of each combination
	  for (approx2=0; approx2<approx; ++approx2) {
	    lf2_index = approx2 * numFunctions + qoi;
	    lf2_fn = fn_vals[lf2_index];
	    if (isfinite(lf2_fn)) { // both are finite
	       ++num_LL_q(approx,approx2);
	       sum_LL_q(approx,approx2) += lf_fn * lf2_fn;
	    }
	  }

	  if (hf_is_finite) {
	    ++num_LH[approx][qoi];
	    sum_LH(qoi,approx) += lf_fn * hf_fn;// Low-High (c vector)	    
	  }
	}
      }
    }
  }
}


/** This version used by ACV following shared_approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    Sizet2DArray& num_L_shared, SizetSymMatrixArray& num_LL)
{
  // uses one set of allResponses with QoI aggregation across all approx Models,
  // corresponding to unorderedModels[i-1], i=1:numApprox (omits truthModel)

  using std::isfinite;
  Real lf_fn, lf2_fn, lf_prod, lf2_prod;
  IntRespMCIter r_it; IntRMMIter ls_it; IntRSMAMIter ll_it;
  int ls_ord, ll_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      SizetSymMatrix& num_LL_q = num_LL[qoi];
      for (approx=0; approx<numApprox; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_shared[approx][qoi];
	  ++num_LL_q(approx,approx); // Diagonal of C matrix

	  ls_it = sum_L_shared.begin();  ll_it = sum_LL.begin();
	  ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	  ll_ord = (ll_it == sum_LL.end())       ? 0 : ll_it->first;
	  lf_prod = lf_fn;  active_ord = 1;
	  while (ls_ord || ll_ord) {
    
	    // Low shared
	    if (ls_ord == active_ord) { // support general key sequence
	      ls_it->second(qoi,approx) += lf_prod;  ++ls_it;
	      ls_ord = (ls_it == sum_L_shared.end()) ? 0 : ls_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      ll_it->second[qoi](approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_fn = fn_vals[lf2_index];

		if (isfinite(lf2_fn)) { // both are finite
		  if (active_ord == 1) ++num_LL_q(approx,approx2);
		  lf2_prod = lf2_fn;
		  for (m=1; m<active_ord; ++m)
		    lf2_prod *= lf2_fn;
		  ll_it->second[qoi](approx,approx2) += lf_prod * lf2_prod;
		}
	      }
	      ++ll_it;  ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** This version used by ACV following approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_refined,
		    Sizet2DArray& num_L_refined,
		    size_t approx_start, size_t approx_end)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  Real lf_fn, lf_prod;
  int lr_ord, active_ord;
  size_t qoi, lf_index, approx;
  IntRespMCIter r_it; IntRMMIter lr_it;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      for (approx=approx_start; approx<approx_end; ++approx) {
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++num_L_refined[approx][qoi];
	  lr_it = sum_L_refined.begin();
	  lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	  lf_prod = lf_fn;  active_ord = 1;
	  while (lr_ord) {
    
	    // Low refined
	    if (lr_ord == active_ord) { // support general key sequence
	      lr_it->second(qoi,approx) += lf_prod;  ++lr_it;
	      lr_ord = (lr_it == sum_L_refined.end()) ? 0 : lr_it->first;
	    }

	    lf_prod *= lf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


void NonDACVSampling::
compute_LH_covariance(const RealMatrix& sum_L_shared, const RealVector& sum_H,
		      const RealMatrix& sum_LH, const Sizet2DArray& N_L_shared,
		      const SizetArray& N_H,    const Sizet2DArray& N_LH,
		      RealMatrix& cov_LH)
{
  if (cov_LH.empty()) cov_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    const SizetArray&    N_L_a =   N_L_shared[approx];
    const SizetArray&   N_LH_a =         N_LH[approx];
    Real*             cov_LH_a =       cov_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_covariance(sum_L_shared_a[qoi], sum_H[qoi], sum_LH_a[qoi],
			 N_L_a[qoi], N_H[qoi], N_LH_a[qoi], cov_LH_a[qoi]);
  }
}


void NonDACVSampling::
compute_LL_covariance(const RealMatrix& sum_L_shared,
		      const RealSymMatrixArray& sum_LL,
		      const Sizet2DArray& N_L_shared,
		      const SizetSymMatrixArray& N_LL,
		      RealSymMatrixArray& cov_LL)
{
  size_t qoi, approx, approx2, N_L_aq;
  if (cov_LL.empty()) {
    cov_LL.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi].shapeUninitialized(numApprox);
  }

  Real sum_L_aq;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& sum_LL_q = sum_LL[qoi];
    const SizetSymMatrix&  N_LL_q =   N_LL[qoi];
    RealSymMatrix&       cov_LL_q = cov_LL[qoi];
    for (approx=0; approx<numApprox; ++approx) {
      N_L_aq = N_L_shared[approx][qoi];  sum_L_aq = sum_L_shared(qoi,approx);
      for (approx2=0; approx2<=approx; ++approx2)
	compute_covariance(sum_L_aq, sum_L_shared(qoi,approx2),
			   sum_LL_q(approx,approx2), N_L_aq,
			   N_L_shared[approx2][qoi], N_LL_q(approx,approx2),
			   cov_LL_q(approx,approx2));
    }
  }
}


void NonDACVSampling::
acv_raw_moments(IntRealMatrixMap& sum_L_baseline,
		IntRealMatrixMap& sum_L_refined,  IntRealVectorMap& sum_H,
		IntRealSymMatrixArrayMap& sum_LL, IntRealMatrixMap& sum_LH,
		const RealVector& avg_eval_ratios,
		const Sizet2DArray& N_L_baseline,
		const Sizet2DArray& N_L_refined,  const SizetArray&   N_H,
		const SizetSymMatrixArray& N_LL,  const Sizet2DArray& N_LH,
		RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  RealSymMatrix F, CF_inv;
  compute_F_matrix(avg_eval_ratios, F);

  size_t approx, qoi, N_H_q;
  RealVector beta(numApprox);
  for (int mom=1; mom<=4; ++mom) {
    RealMatrix&     sum_L_base_m = sum_L_baseline[mom];
    RealMatrix&      sum_L_ref_m = sum_L_refined[mom];
    RealVector&          sum_H_m =         sum_H[mom];
    RealSymMatrixArray& sum_LL_m =        sum_LL[mom];
    RealMatrix&         sum_LH_m =        sum_LH[mom];
    if (outputLevel >= NORMAL_OUTPUT)
      Cout << "Moment " << mom << " estimator:\n";
    for (qoi=0; qoi<numFunctions; ++qoi) {
      Real sum_H_mq = sum_H_m[qoi];
      if (mom == 1) // variances/covariances already computed for mean estimator
	compute_acv_control(covLL[qoi], F, covLH, qoi, beta);
      else // compute variances/covariances for higher-order moment estimators
	compute_acv_control(sum_L_base_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			    N_L_baseline, N_H[qoi], N_LL[qoi], N_LH, F,
			    qoi, beta); // need to all be based on shared counts
        // *** TO DO: support shared_approx_increment() --> baselineL

      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      N_H_q = N_H[qoi];
      H_raw_mq = sum_H_mq / N_H_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1 << ": control "
	       << "variate beta = " << std::setw(9) << beta[approx] << '\n';
	// For ACV, shared counts are fixed at N_H for all approx
	apply_control(sum_L_base_m(qoi,approx), N_L_baseline[approx][qoi],
		      sum_L_ref_m(qoi,approx),  N_L_refined[approx][qoi],
		      beta[approx], H_raw_mq);
      }
    }
  }
  if (outputLevel >= NORMAL_OUTPUT) Cout << std::endl;
}


void NonDACVSampling::
update_projected_samples(Real avg_hf_target, const RealVector& avg_eval_ratios,
			 SizetArray& N_H_projected, Sizet2DArray& N_L_projected)
{
  increment_samples(N_H_projected,
		    one_sided_delta(average(N_H_projected), avg_hf_target));

  size_t approx;  Real lf_target;
  for (approx=0; approx<numApprox; ++approx) {
    lf_target = avg_eval_ratios[approx] * avg_hf_target;
    SizetArray& N_L_a = N_L_projected[approx];
    increment_samples(N_L_a, one_sided_delta(average(N_L_a), lf_target));
  }
}


void NonDACVSampling::
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


void NonDACVSampling::print_variance_reduction(std::ostream& s)
{
  size_t wpp7 = write_precision + 7;
  s << "<<<<< Variance for mean estimator:";

  if (solutionMode != OFFLINE_PILOT)
    s << "\n      Initial MC (" << std::setw(4) << pilotSamples[numApprox]
      << " pilot samples): " << std::setw(wpp7) << average(mseIter0);

  String type = (solutionMode == PILOT_PROJECTION) ? "Projected" : "    Final";
  // Ordering of averages:
  // > recomputing final MC MSE, rather than dividing the two averages, gives
  //   a result that is consistent with average(mseIter0) when N* = pilot.
  // > The ACV ratio then differs from final ACV / final MC (due to recovering
  //   avgACVEstVar from the optimizer obj fn), but the difference is small and
  //   less immediately evident.
  // > General preference is to delay averaging as long as possible.
  RealVector final_mc_mse;
  compute_mc_estimator_variance(varH, numH, final_mc_mse);
  s << "\n  " << type << "   MC (" << std::setw(4)
    << (size_t)std::floor(average(numH) + .5) << " HF samples):    "
    << std::setw(wpp7) << average(final_mc_mse) //avgACVEstVar / avgMSERatio
    << "\n  " << type << "  ACV (sample profile):     "
    << std::setw(wpp7) << avgACVEstVar
    << "\n  " << type << "  ACV ratio (1 - R^2):      "
    << std::setw(wpp7) << avgMSERatio << '\n';
}


Real NonDACVSampling::objective_function(const RealVector& r_and_N)
{
  RealSymMatrix F, CF_inv;
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

  RealVector A, R_sq(numFunctions, false);  size_t qoi;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    invert_CF(covLL[qoi], F, CF_inv);
    //Cout << "Objective eval: CF inverse =\n" << CF_inv << std::endl;
    compute_A_vector(F, covLH, qoi, A);     // defer c-bar scaling
    //Cout << "Objective eval: A =\n" << A << std::endl;
    compute_Rsq(CF_inv, A, varH[qoi], R_sq[qoi]); // apply scaling^2
    //Cout << "Objective eval: varH[" << qoi << "] = " << varH[qoi]
    //     << " Rsq[" << qoi << "] =\n" << R_sq[qoi] << std::endl;
  }

  // form estimator variances to pick up dependence on N
  RealVector est_var(numFunctions, false);
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: // N is a vector constant for opt sub-problem
    for (qoi=0; qoi<numFunctions; ++qoi)
      est_var[qoi] = varH[qoi] / numH[qoi] * (1. - R_sq[qoi]);
    break;
  case N_VECTOR_LINEAR_CONSTRAINT:
  case R_AND_N_NONLINEAR_CONSTRAINT: {  // N is a scalar optimization variable
    Real N = r_and_N[numApprox];
    for (qoi=0; qoi<numFunctions; ++qoi)
      est_var[qoi] = varH[qoi] / N         * (1. - R_sq[qoi]);
    break;
  }
  }

  // protect against R_sq blow-up for N_i < N (if not enforced by linear constr)
  Real avg_est_var = average(est_var), obj_fn = (avg_est_var > 0.) ?
    std::log(avg_est_var) :
    std::numeric_limits<Real>::quiet_NaN();//Pecos::LARGE_NUMBER;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "objective_function: "
         << "design vars:\n" << r_and_N << "R squared:\n" << R_sq
	 << "obj = log(average((1.-Rsq)varH/N)) = " << obj_fn << '\n';
  return obj_fn; // maximize R_sq; use log to flatten contours
}


/*
void NonDACVSampling::
objective_gradient(const RealVector& r_and_N, RealVector& obj_grad)
{
  // This would still be called for deriv level 0 to identify the set of terms
  // that must be numerically estimated.

  Cerr << "Warning: gradient of the objective not supported." << std::endl;
  abort_handler(METHOD_ERROR);
}
*/


Real NonDACVSampling::nonlinear_constraint(const RealVector& r_and_N)
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


void NonDACVSampling::
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


/** NPSOL objective functions evaluator for solution of homotopy constraint 
    relaxation parameter optimization. This constrained optimization problem
    performs the update of the tau parameter in the homotopy heuristic 
    approach used to relax the constraints in the original problem . */
void NonDACVSampling::
npsol_objective_evaluator(int& mode, int& n, double* x, double& f,
			  double* grad_f, int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  if (asv_request & 1)
    f = acvInstance->objective_function(x_rv);
  // NPSOL estimates unspecified components of the obj grad, so ASV grad
  // request is not an error -- just don't specify anything
  //if (asv_request & 2) {
  //  RealVector grad_f_rv(Teuchos::View, grad_f, n);
  //  acvInstance->objective_gradient(x_rv, grad_f_rv);
  //}
}


void NonDACVSampling::
npsol_constraint_evaluator(int& mode, int& ncnln, int& n, int& nrowj,
			   int* needc, double* x, double* c, double* cjac,
			   int& nstate)
{
  // NPSOL mode: 0 = get f, 1 = get grad_f, 2 = get both
  // promote mode to standard asv request codes
  short asv_request = mode + 1;
  RealVector x_rv(Teuchos::View, x, n);
  if (asv_request & 1)
    c[0] = acvInstance->nonlinear_constraint(x_rv);
  if (asv_request & 2) {
    RealVector grad_c_rv(Teuchos::View, cjac, n);
    acvInstance->nonlinear_constraint_gradient(x_rv, grad_c_rv);
  }
}


/* API for NLF1 objective (see SNLLOptimizer::nlf1_evaluator())
void NonDACVSampling::
optpp_objective_evaluator(int mode, int n, const RealVector& x, double& f, 
			  RealVector& grad_f, int& result_mode)
{
  if (mode & OPTPP::NLPFunction) { // 1st bit is present, mode = 1 or 3
    f = acvInstance->objective_function(x);
    result_mode = OPTPP::NLPFunction;
  }
  if (mode & OPTPP::NLPGradient) { // 2nd bit is present, mode = 2 or 3
    acvInstance->objective_gradient(x, grad_f);
    result_mode |= OPTPP::NLPGradient;
  }
}
*/


/** API for FDNLF1 objective (see SNLLOptimizer::nlf0_evaluator()) */
void NonDACVSampling::
optpp_objective_evaluator(int n, const RealVector& x, double& f,
			  int& result_mode)
{
  f = acvInstance->objective_function(x);
  result_mode = OPTPP::NLPFunction; // 1 bit
}


/** API for NLF1 constraint (see SNLLOptimizer::constraint1_evaluator()) */
void NonDACVSampling::
optpp_constraint_evaluator(int mode, int n, const RealVector& x, RealVector& c,
			   RealMatrix& grad_c, int& result_mode)
{
  result_mode = OPTPP::NLPNoOp; // 0
  if (mode & OPTPP::NLPFunction) { // 1 bit is present, mode = 1 or 3
    c[0] = acvInstance->nonlinear_constraint(x);
    result_mode |= OPTPP::NLPFunction; // adds 1 bit
  }
  if (mode & OPTPP::NLPGradient) { // 2 bit is present, mode = 2 or 3
    RealVector grad_c_rv(Teuchos::View, grad_c[0], n); // 0-th col vec
    acvInstance->nonlinear_constraint_gradient(x, grad_c_rv);
    result_mode |= OPTPP::NLPGradient; // adds 2 bit
  }
}

} // namespace Dakota
