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

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDACVSampling::
NonDACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDNonHierarchSampling(problem_db, model), multiStartACV(true)
{
  mlmfSubMethod = problem_db.get_ushort("method.sub_method");
  // truthFixedByPilot is a user-specified option for fixing the number of HF
  // samples (those from the pilot).  In this case, equivHF budget is allocated
  // by optimizing r* for fixed N.
  optSubProblemForm = (truthFixedByPilot && pilotMgmtMode != OFFLINE_PILOT) ?
    R_ONLY_LINEAR_CONSTRAINT : N_VECTOR_LINEAR_CONSTRAINT;
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "ACV sub-method selection = " << mlmfSubMethod
	 << " sub-method formulation = "  << optSubProblemForm
	 << " sub-problem solver = "      << optSubProblemSolver << std::endl;

  if (maxFunctionEvals == SZ_MAX) {
    Cerr << "Error: evaluation budget required for ACV (convergence tolerance "
	 << "option not yet supported)." << std::endl;
    abort_handler(METHOD_ERROR);
  }
}


NonDACVSampling::~NonDACVSampling()
{ }


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDACVSampling::core_run()
{
  /*
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS:  case SUBMETHOD_ACV_MF:
    approximate_control_variate(); break;
  //case SUBMETHOD_ACV_KL:
    //for (k) for (l) approximate_control_variate(...); ???
  }
  */
  if (mlmfSubMethod == SUBMETHOD_ACV_KL) {
    Cerr << "Error: ACV KL not yet implemented." << std::endl;
    abort_handler(METHOD_ERROR);
  }

  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

  switch (pilotMgmtMode) {
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
  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, avg_eval_ratios;  RealMatrix var_L;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  numH.assign(numFunctions, 0);//initialize_acv_counts(numH, N_LL);
  //initialize_acv_covariances(covLL, covLH, varH);

  // Initialize for pilot sample
  size_t hf_shared_pilot = numSamples;
  //, start=0, lf_shared_pilot = find_min(pilotSamples, start, numApprox-1);

  Real avg_hf_target = 0.;
  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, numH);//, N_LL);
    // Online cost recovery spans pilot and is not updated after mlmfIter 0
    if (sequenceCost.empty()) recover_online_cost(sequenceCost);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps);
    // allow pilot to vary for C vs c
    // *** TO DO: numSamples logic after pilot (mlmfIter >= 1)
    // *** Will likely require _baselineL and _baselineH
    //if (mlmfIter == 0 && lf_shared_pilot > hf_shared_pilot) {
    //  numSamples = lf_shared_pilot - hf_shared_pilot;
    //  shared_approx_increment(mlmfIter); // spans all approx models
    //  accumulate_acv_sums(sum_L_baselineL, sum_LL,//_baselineL,
    //                      N_L_baselineL);
    //  increment_equivalent_cost(numSamples, sequenceCost, 0, numApprox);
    //}

    const RealMatrix&         sum_L_1  = sum_L_baselineH[1];
    const RealVector&         sum_H_1  = sum_H[1];
    const RealSymMatrixArray& sum_LL_1 = sum_LL[1];
    compute_variance(sum_H_1, sum_HH, numH, varH);
    if (mlmfIter==0) compute_L_variance(sum_L_1, sum_LL_1,      numH, var_L);
    compute_LH_covariance(sum_L_1/*baseH*/, sum_H_1, sum_LH[1], numH, covLH);
    compute_LL_covariance(sum_L_1/*baseL*/, sum_LL_1,   /*N_LL*/numH, covLL);
    //Cout << "var_H:\n"<< var_H << "cov_LH:\n"<< cov_LH << "cov_LL:\n"<<cov_LL;

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);

    ++mlmfIter;
  }

  approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, numH,
		    avg_eval_ratios, avg_hf_target);
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
  SizetArray N_shared_pilot;  //SizetSymMatrixArray N_LL_pilot;
  N_shared_pilot.assign(numFunctions, 0);
  //initialize_acv_counts(N_shared_pilot, N_LL_pilot);
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  // Initialize for pilot sample
  size_t hf_shared_pilot = numSamples;
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		      sum_HH_pilot, N_shared_pilot);//, N_LL_pilot);
  if (sequenceCost.empty()) recover_online_cost(sequenceCost);
  //increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps);
  compute_variance(sum_H_pilot, sum_HH_pilot, N_shared_pilot, varH);
  compute_L_variance(sum_L_pilot, sum_LL_pilot, N_shared_pilot, var_L);
  compute_LH_covariance(sum_L_pilot, sum_H_pilot, sum_LH_pilot,
			N_shared_pilot, covLH);
  compute_LL_covariance(sum_L_pilot, sum_LL_pilot,
			/*N_LL_pilot*/N_shared_pilot, covLL);
  //Cout << "var_H:\n"<< var_H << "cov_LH:\n"<< cov_LH << "cov_LL:\n"<<cov_LL;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, avg_eval_ratios;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  numH.assign(numFunctions, 0);//initialize_acv_counts(numH, N_LL);
  //initialize_acv_covariances(covLL, covLH, varH);
  Real avg_hf_target = 0.;

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		 avgEstVar, avgEstVarRatio);

  // at least 2 samples reqd for variance (+ resetting allSamples from pilot)
  numSamples = std::max(numSamples, (size_t)2);
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, numH);//, N_LL);
  increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps);
  // allow pilot to vary for C vs c

  approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, numH,
		    avg_eval_ratios, avg_hf_target);
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
  //SizetSymMatrixArray N_LL;  initialize_acv_counts(numH, N_LL);
  numH.assign(numFunctions, 0);
  Real avg_hf_target = 0.;

  // Initialize for pilot sample
  size_t hf_shared_pilot = numSamples;
  //, start=0, lf_shared_pilot = find_min(pilotSamples, start, numApprox-1);

  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, numH);//, N_LL);
  if (sequenceCost.empty()) recover_online_cost(sequenceCost);
  increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps);
  // allow pilot to vary for C vs c

  compute_variance(sum_H, sum_HH, numH, varH);
  compute_L_variance(sum_L_baselineH, sum_LL, numH, var_L);
  compute_LH_covariance(sum_L_baselineH, sum_H, sum_LH, numH, covLH);
  compute_LL_covariance(sum_L_baselineH/*baseL*/, sum_LL, /*N_LL*/numH, covLL);
  //Cout << "var_H:\n"<< var_H << "cov_LH:\n"<< cov_LH << "cov_LL:\n"<<cov_LL;

  // compute the LF/HF evaluation ratios from shared samples and compute
  // ratio of MC and ACV mean sq errors (which incorporates anticipated
  // variance reduction from application of avg_eval_ratios).
  compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		 avgEstVar, avgEstVarRatio);

  // No LF increments or final moments for pilot projection

  // overwrite actual incurred numH with projected numH
  //SizetArray N_H_projected = numH; // more fine-grained bookkeeping if needed
  Sizet2DArray N_L_projected;  inflate(numH, N_L_projected);
  update_projected_samples(avg_hf_target, avg_eval_ratios, numH, N_L_projected);
  finalize_counts(N_L_projected);
}


void NonDACVSampling::
approx_increments(IntRealMatrixMap& sum_L_baselineH, IntRealVectorMap& sum_H,
		  IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		  const SizetArray& N_shared, const RealVector& avg_eval_ratios,
		  Real avg_hf_target)
{
  // ----------------------------------------------------------------
  // Compute N_L increments based on eval ratio applied to final numH
  // ----------------------------------------------------------------
  // Note: these results do not affect the iteration above and can be performed
  // after numH has converged, which simplifies maxFnEvals / convTol logic
  // (no need to further interrogate these throttles below)

  // maxIterations == 0 is no longer reserved for the pilot only case.
  // See notes in NonDMultifidelitySampling::multifidelity_mc().

  // define approx_sequence in decreasing r_i order, directionally consistent
  // with default approx indexing for well-ordered models
  // > approx 0 is lowest fidelity --> lowest corr,cost --> highest r_i
  SizetArray approx_sequence;  bool descending = true;
  ordered_approx_sequence(avg_eval_ratios, approx_sequence, descending);

  IntRealMatrixMap sum_L_refined = sum_L_baselineH;//baselineL;
  Sizet2DArray       N_L_refined;  inflate(N_shared, N_L_refined);
  size_t start, end;
  for (end=numApprox; end>0; --end) {
    // *** TO DO NON_BLOCKING: PERFORM 2ND PASS ACCUMULATE AFTER 1ST PASS LAUNCH
    start = (mlmfSubMethod == SUBMETHOD_ACV_IS) ? end - 1 : 0;
    if (acv_approx_increment(avg_eval_ratios, N_L_refined, avg_hf_target,
			     mlmfIter, approx_sequence, start, end)) {
      // ACV_IS samples on [approx-1,approx) --> sum_L_refined
      // ACV_MF samples on [0, approx)       --> sum_L_refined
      accumulate_acv_sums(sum_L_refined, N_L_refined, approx_sequence,
			  start, end);
      increment_equivalent_cost(numSamples, sequenceCost, approx_sequence,
				start, end);
    }
  }

  // -----------------------------------------------------------
  // Compute/apply control variate parameter to estimate moments
  // -----------------------------------------------------------
  RealMatrix H_raw_mom(numFunctions, 4);
  acv_raw_moments(sum_L_baselineH, sum_L_refined, sum_H, sum_LL, sum_LH,
		  avg_eval_ratios, numH, N_L_refined, H_raw_mom);
  // Convert uncentered raw moment estimates to final moments (central or std)
  convert_moments(H_raw_mom, momentStats);
  // post final sample counts into format for final results reporting
  finalize_counts(N_L_refined);
}


bool NonDACVSampling::
acv_approx_increment(const RealVector& avg_eval_ratios,
		     const Sizet2DArray& N_L_refined, Real hf_target,
		     size_t iter, const SizetArray& approx_sequence,
		     size_t start, size_t end)
{
  // Update LF samples based on evaluation ratio
  //   r = N_L/N_H -> N_L = r * N_H -> delta = N_L - N_H = (r-1) * N_H
  // Notes:
  // > the sample increment for the approx range is determined by approx[end-1]
  //   (helpful to refer to Figure 2(b) in ACV paper, noting index differences)
  // > N_L is updated prior to each call to approx_increment (*** if BLOCKING),
  //   allowing use of one_sided_delta() with latest counts

  bool ordered = approx_sequence.empty();
  size_t approx = (ordered) ? end-1 : approx_sequence[end-1];
  Real lf_target = avg_eval_ratios[approx] * hf_target,
       lf_curr   = average(N_L_refined[approx]);
  // Choose avg, RMS, max? (trade-off: possible overshoot vs. more iteration)
  numSamples = one_sided_delta(lf_curr, lf_target);
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Approx samples computed from delta between target " << lf_target
	 << " and current average count " << lf_curr << std::endl;

  return approx_increment(iter, approx_sequence, start, end);
}


void NonDACVSampling::
compute_ratios(const RealMatrix& var_L,     const RealVector& cost,
	       RealVector& avg_eval_ratios, Real& avg_hf_target,
	       Real& avg_estvar,            Real& avg_estvar_ratio)
{
  // --------------------------------------
  // Configure the optimization sub-problem
  // --------------------------------------

  // Modify budget to allow a feasible soln (var lower bnds: r_i > 1, N > numH).
  // Can happen if shared pilot rolls up to exceed budget spec.
  Real budget           = (Real)maxFunctionEvals;
  bool budget_exhausted = (equivHFEvals >= budget);
  if  (budget_exhausted) budget = equivHFEvals;

  // Set initial guess based either on MFMC analytic solution (iter == 0)
  // or warm started from previous solution (iter >= 1)
  if (mlmfIter == 0) {
    // estVarIter0 only uses HF pilot since sum_L_shared / N_shared minus
    // sum_L_refined / N_refined are zero for CVs prior to sample refinement.
    // (This differs from MLMC EstVar^0 which uses pilot for all levels.)
    // Note: could revisit this for case of lf_shared_pilot > hf_shared_pilot.
    compute_mc_estimator_variance(varH, numH, estVarIter0);  numHIter0 = numH;

    if (budget_exhausted) { // there is only 1 feasible pt, no need for solve
      if (avg_eval_ratios.empty()) avg_eval_ratios.sizeUninitialized(numApprox);
      numSamples = 0;  avg_eval_ratios = 1.;  avg_hf_target = average(numH);
      avg_estvar = average(estVarIter0);      avg_estvar_ratio = 1.;
      return;
    }
    else { // compute initial estimate of r* from MFMC
      covariance_to_correlation_sq(covLH, var_L, varH, rho2LH);
      Real avg_N_H = average(numH);

      // Run a competition among analytic approaches for best initial guess:
      // > Option 1 is analytic MFMC: differs from ACV due to recursive pairing
      Real avg_estvar1, avg_estvar2, avg_hf_target1, avg_hf_target2;
      RealMatrix     eval_ratios1,     eval_ratios2;
      RealVector avg_eval_ratios1, avg_eval_ratios2;
      if (ordered_approx_sequence(rho2LH)) // for all QoI across all Approx
	mfmc_analytic_solution(rho2LH, cost, eval_ratios1);
      else // compute reordered MFMC for averaged rho; monotonic r not reqd
	mfmc_reordered_analytic_solution(rho2LH, cost, approxSequence,
					 eval_ratios1, false);

      // > Option 2 is ensemble of independent two-model CVMCs, rescaled to an
      //   aggregate budget.  This is more ACV-like in the sense that it is not
      //   recursive, but it neglects the covariance C among approximations.
      //   It is also insensitive to model sequencing.
      cvmc_ensemble_solutions(rho2LH, cost, eval_ratios2);

      // any rho2_LH re-ordering from MFMC initial guess can be ignored (later
      // gets replaced with r_i ordering for approx_increments() sampling)
      approxSequence.clear();
      if (multiStartACV) {
	// Run numerical solns from both starting points (first mlmfIter only)
	average(eval_ratios1, 0, avg_eval_ratios1);
	average(eval_ratios2, 0, avg_eval_ratios2);
	scale_to_target(avg_N_H, cost, avg_eval_ratios1, avg_hf_target1);
	scale_to_target(avg_N_H, cost, avg_eval_ratios2, avg_hf_target2);
	Real avg_estvar_ratio1, avg_estvar_ratio2;
	size_t num_samp1, num_samp2;
	nonhierarch_numerical_solution(cost, approxSequence, avg_eval_ratios1,
				       avg_hf_target1, num_samp1, avg_estvar1,
				       avg_estvar_ratio1);
	nonhierarch_numerical_solution(cost, approxSequence, avg_eval_ratios2,
				       avg_hf_target2, num_samp2, avg_estvar2,
				       avg_estvar_ratio2);
	bool mfmc_init = (avg_estvar1 <= avg_estvar2);
	Cout << "\nACV best solution from ";
	if (mfmc_init) {
	  Cout << "analytic MFMC." << std::endl;
	  avg_eval_ratios  = avg_eval_ratios1;  avg_hf_target = avg_hf_target1;
	  numSamples       = num_samp1;         avg_estvar    = avg_estvar1;
	  avg_estvar_ratio = avg_estvar_ratio1;
	}
	else {
	  Cout << "ensemble of two-model CVMC." << std::endl;
	  avg_eval_ratios  = avg_eval_ratios2;  avg_hf_target = avg_hf_target2;
	  numSamples       = num_samp2;         avg_estvar    = avg_estvar2;
	  avg_estvar_ratio = avg_estvar_ratio2;
	}
      }
      else {
	// Run one numerical soln from best of two starting points
	avg_estvar1 = acv_estimator_variance(eval_ratios1, avg_N_H, cost,
					     avg_eval_ratios1, avg_hf_target1);
	avg_estvar2 = acv_estimator_variance(eval_ratios2, avg_N_H, cost,
					     avg_eval_ratios2, avg_hf_target2);
	bool mfmc_init = (avg_estvar1 <= avg_estvar2);
	if (mfmc_init)
	  { avg_eval_ratios = avg_eval_ratios1; avg_hf_target = avg_hf_target1;}
	else
	  { avg_eval_ratios = avg_eval_ratios2; avg_hf_target = avg_hf_target2;}
	if (outputLevel >= NORMAL_OUTPUT) {
	  Cout << "ACV initial guess candidates:\n  analytic MFMC estvar = "
	       << avg_estvar1 << "\n  ensemble CVMC estvar = " << avg_estvar2
	       << "\nACV initial guess from ";
	  if (mfmc_init) Cout << "analytic MFMC ";
	  else           Cout << "ensemble of two-model CVMC ";
	  Cout << "(average eval ratios):\n" << avg_eval_ratios << std::endl;
	}
	// Single solve initiated from lowest estvar
	nonhierarch_numerical_solution(cost, approxSequence, avg_eval_ratios,
				       avg_hf_target, numSamples, avg_estvar,
				       avg_estvar_ratio);
      }
    }
  }
  else { // update approx_sequence after shared sample increment
    //covariance_to_correlation_sq(covLH, var_L, varH, rho2LH);
    //RealVector avg_rho2_LH;  average(rho2LH, 0, avg_rho2_LH);
    //ordered_approx_sequence(avg_rho2_LH, approxSequence);

    // warm start from previous eval_ratios solution
    // > no scaling needed from prev soln (as in NonDLocalReliability) since
    //   updated avg_N_H now includes allocation from previous solution and is
    //   active on constraint bound (excepting integer sample rounding)
    approxSequence.clear();

    // Should not be required so long as previous solution is feasible:
    //Real avg_N_H = average(numH);
    //Cout << "Before: avg_eval_ratios =:\n" << avg_eval_ratios
    // 	   << "avg_hf_target = " << avg_hf_target << std::endl;
    //scale_to_target(avg_N_H, cost, avg_eval_ratios, avg_hf_target);
    //Cout << "After:  avg_eval_ratios =:\n" << avg_eval_ratios
    // 	   << "avg_hf_target = " << avg_hf_target << std::endl;

    nonhierarch_numerical_solution(cost, approxSequence, avg_eval_ratios,
				   avg_hf_target, numSamples, avg_estvar,
				   avg_estvar_ratio);
  }

  if (outputLevel >= NORMAL_OUTPUT) {
    for (size_t approx=0; approx<numApprox; ++approx)
      Cout << "Approx " << approx+1 << ": average evaluation ratio = "
	   << avg_eval_ratios[approx] << '\n';
    Cout << "Average estimator variance = " << avg_estvar
	 << "\nAverage ACV variance / average MC variance = "
	 << avg_estvar_ratio << std::endl;
  }
}


/** Multi-moment map-based version used by ACV following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_baseline, IntRealVectorMap& sum_H,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    IntRealMatrixMap&         sum_LH, // each L with H
		    RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;  bool all_finite;
  Real lf_fn, lf2_fn, hf_fn, lf_prod, lf2_prod, hf_prod;
  IntRespMCIter r_it;              IntRVMIter    h_it;
  IntRMMIter lb_it, lr_it, lh_it;  IntRSMAMIter ll_it;
  int lb_ord, lr_ord, h_ord, ll_ord, lh_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_finite = true;
      for (approx=0; approx<=numApprox; ++approx)
	if (!isfinite(fn_vals[approx * numFunctions + qoi])) // NaN or +/-Inf
	  { all_finite = false; break; }

      if (all_finite) {
	++N_shared[qoi];
	// High accumulations:
	hf_index = numApprox * numFunctions + qoi;
	hf_fn = fn_vals[hf_index];
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

	for (approx=0; approx<numApprox; ++approx) {
	  // Low accumulations:
	  lf_index = approx * numFunctions + qoi;
	  lf_fn = fn_vals[lf_index];

	  lb_it = sum_L_baseline.begin();
	  ll_it = sum_LL.begin();  lh_it = sum_LH.begin();
	  lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	  ll_ord = (ll_it == sum_LL.end())         ? 0 : ll_it->first;
	  lh_ord = (lh_it == sum_LH.end())         ? 0 : lh_it->first;
	  lf_prod = lf_fn;  hf_prod = hf_fn;  active_ord = 1;
	  while (lb_ord || ll_ord || lh_ord) {
    
	    // Low baseline
	    if (lb_ord == active_ord) { // support general key sequence
	      lb_it->second(qoi,approx) += lf_prod;  ++lb_it;
	      lb_ord = (lb_it == sum_L_baseline.end()) ? 0 : lb_it->first;
	    }
	    // Low-Low
	    if (ll_ord == active_ord) { // support general key sequence
	      RealSymMatrix& sum_LL_q = ll_it->second[qoi];
	      sum_LL_q(approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_prod = lf2_fn = fn_vals[lf2_index];
		for (m=1; m<active_ord; ++m)
		  lf2_prod *= lf2_fn;
		sum_LL_q(approx,approx2) += lf_prod * lf2_prod;
	      }
	      ++ll_it; ll_ord = (ll_it == sum_LL.end()) ? 0 : ll_it->first;
	    }
	    // Low-High (c vector for each QoI):
	    if (lh_ord == active_ord) {
	      lh_it->second(qoi,approx) += lf_prod * hf_prod;
	      ++lh_it; lh_ord = (lh_it == sum_LH.end()) ? 0 : lh_it->first;
	    }

	    lf_prod *= lf_fn;  hf_prod *= hf_fn;  ++active_ord;
	  }
	}
      }
    }
  }
}


/** Single moment version used by offline-pilot and pilot-projection ACV
    following shared_increment() */
void NonDACVSampling::
accumulate_acv_sums(RealMatrix& sum_L_baseline, RealVector& sum_H,
		    RealSymMatrixArray& sum_LL, // L w/ itself + other L
		    RealMatrix&         sum_LH, // each L with H
		    RealVector& sum_HH, SizetArray& N_shared)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // ordered by unorderedModels[i-1], i=1:numApprox --> truthModel

  using std::isfinite;  bool all_finite;
  Real lf_fn, lf2_fn, hf_fn;
  IntRespMCIter r_it;
  size_t qoi, approx, approx2, lf_index, lf2_index, hf_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_finite = true;
      for (approx=0; approx<=numApprox; ++approx)
	if (!isfinite(fn_vals[approx * numFunctions + qoi])) // NaN or +/-Inf
	  { all_finite = false; break; }

      if (all_finite) {
	++N_shared[qoi];
	// High accumulations:
	hf_index = numApprox * numFunctions + qoi;
	hf_fn = fn_vals[hf_index];
	sum_H[qoi]  += hf_fn;         // High
	sum_HH[qoi] += hf_fn * hf_fn; // High-High

	RealSymMatrix& sum_LL_q = sum_LL[qoi];
	for (approx=0; approx<numApprox; ++approx) {
	  lf_index = approx * numFunctions + qoi;
	  lf_fn = fn_vals[lf_index];

	  // Low accumulations:
	  sum_L_baseline(qoi,approx) += lf_fn; // Low
	  sum_LL_q(approx,approx)    += lf_fn * lf_fn; // Low-Low
	  // Off-diagonal of C matrix:
	  // look back (only) for single capture of each combination
	  for (approx2=0; approx2<approx; ++approx2) {
	    lf2_index = approx2 * numFunctions + qoi;
	    lf2_fn = fn_vals[lf2_index];
	    sum_LL_q(approx,approx2) += lf_fn * lf2_fn;
	  }
	  // Low-High (c vector)
	  sum_LH(qoi,approx) += lf_fn * hf_fn;
	}
      }
    }
  }
}


/** Multi-moment map-based version with fine-grained fault tolerance, 
    used by ACV following shared_increment()
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


// Single moment version with fine-grained fault tolerance, used by 
// offline-pilot and pilot-projection ACV following shared_increment()
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
*/


/** This version used by ACV following shared_approx_increment() */
void NonDACVSampling::
accumulate_acv_sums(IntRealMatrixMap& sum_L_shared,
		    IntRealSymMatrixArrayMap& sum_LL, // L w/ itself + other L
		    Sizet2DArray& N_L_shared)
{
  // uses one set of allResponses with QoI aggregation across all approx Models,
  // corresponding to unorderedModels[i-1], i=1:numApprox (omits truthModel)

  using std::isfinite;  bool all_lf_finite;
  Real lf_fn, lf2_fn, lf_prod, lf2_prod;
  IntRespMCIter r_it; IntRMMIter ls_it; IntRSMAMIter ll_it;
  int ls_ord, ll_ord, active_ord, m;
  size_t qoi, approx, approx2, lf_index, lf2_index;

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      // see fault tol notes in NonDNonHierarchSampling::compute_correlation()
      all_lf_finite = true;
      for (approx=0; approx<numApprox; ++approx)
	if (!isfinite(fn_vals[approx * numFunctions + qoi])) // NaN or +/-Inf
	  { all_lf_finite = false; break; }

      if (all_lf_finite) {
	++N_L_shared[approx][qoi];

	for (approx=0; approx<numApprox; ++approx) {
	  // Low accumulations:
	  lf_index = approx * numFunctions + qoi;
	  lf_fn = fn_vals[lf_index];

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
	      RealSymMatrix& sum_LL_q = ll_it->second[qoi];
	      sum_LL_q(approx,approx) += lf_prod * lf_prod;
	      // Off-diagonal of C matrix:
	      // look back (only) for single capture of each combination
	      for (approx2=0; approx2<approx; ++approx2) {
		lf2_index = approx2 * numFunctions + qoi;
		lf2_prod = lf2_fn = fn_vals[lf2_index];
		for (m=1; m<active_ord; ++m)
		  lf2_prod *= lf2_fn;
		sum_LL_q(approx,approx2) += lf_prod * lf2_prod;
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
		    Sizet2DArray& N_L_refined,
		    const SizetArray& approx_sequence,
		    size_t sequence_start, size_t sequence_end)
{
  // uses one set of allResponses with QoI aggregation across all Models,
  // led by the approx Model responses of interest

  using std::isfinite;
  int lr_ord, active_ord;  size_t s, qoi, lf_index, approx;
  Real lf_fn, lf_prod;  IntRespMCIter r_it;  IntRMMIter lr_it;
  bool ordered = approx_sequence.empty();

  for (r_it=allResponses.begin(); r_it!=allResponses.end(); ++r_it) {
    const Response&   resp    = r_it->second;
    const RealVector& fn_vals = resp.function_values();
    //const ShortArray& asv   = resp.active_set_request_vector();

    for (qoi=0; qoi<numFunctions; ++qoi) {

      for (s=sequence_start; s<sequence_end; ++s) {
	approx = (ordered) ? s : approx_sequence[s];
	lf_index = approx * numFunctions + qoi;
	lf_fn = fn_vals[lf_index];

	// Low accumulations:
	if (isfinite(lf_fn)) {
	  ++N_L_refined[approx][qoi];
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
		      const RealMatrix& sum_LH, const SizetArray& N_shared,
		      RealMatrix& cov_LH)
{
  if (cov_LH.empty()) cov_LH.shapeUninitialized(numFunctions, numApprox);

  size_t approx, qoi;
  for (approx=0; approx<numApprox; ++approx) {
    const Real* sum_L_shared_a = sum_L_shared[approx];
    const Real*       sum_LH_a =       sum_LH[approx];
    Real*             cov_LH_a =       cov_LH[approx];
    for (qoi=0; qoi<numFunctions; ++qoi)
      compute_covariance(sum_L_shared_a[qoi], sum_H[qoi], sum_LH_a[qoi],
			 N_shared[qoi], cov_LH_a[qoi]);
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "cov_LH in compute_LH_covariance():\n" << cov_LH << std::endl;
}


void NonDACVSampling::
compute_LL_covariance(const RealMatrix& sum_L_shared,
		      const RealSymMatrixArray& sum_LL,
		      const SizetArray& N_shared,//SizetSymMatrixArray& N_LL,
		      RealSymMatrixArray& cov_LL)
{
  size_t qoi, approx, approx2;
  if (cov_LL.empty()) {
    cov_LL.resize(numFunctions);
    for (qoi=0; qoi<numFunctions; ++qoi)
      cov_LL[qoi].shapeUninitialized(numApprox);
  }

  Real sum_L_aq;  size_t N_sh_q;
  for (qoi=0; qoi<numFunctions; ++qoi) {
    const RealSymMatrix& sum_LL_q = sum_LL[qoi];
    RealSymMatrix&       cov_LL_q = cov_LL[qoi];
    N_sh_q = N_shared[qoi]; //const SizetSymMatrix&  N_LL_q = N_LL[qoi];
    for (approx=0; approx<numApprox; ++approx) {
      sum_L_aq = sum_L_shared(qoi,approx);
      for (approx2=0; approx2<=approx; ++approx2)
	compute_covariance(sum_L_aq, sum_L_shared(qoi,approx2),
			   sum_LL_q(approx,approx2), N_sh_q,
			   cov_LL_q(approx,approx2));
    }
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "cov_LL in compute_LL_covariance():\n" << cov_LL << std::endl;
}


void NonDACVSampling::
acv_raw_moments(IntRealMatrixMap& sum_L_baseline,
		IntRealMatrixMap& sum_L_refined,   IntRealVectorMap& sum_H,
		IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		const RealVector& avg_eval_ratios, const SizetArray& N_shared,
		const Sizet2DArray& N_L_refined,   RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  RealSymMatrix F, CF_inv;
  compute_F_matrix(avg_eval_ratios, F);

  size_t approx, qoi, N_shared_q;  Real sum_H_mq;
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
      sum_H_mq = sum_H_m[qoi];  N_shared_q = N_shared[qoi];
      if (mom == 1) // variances/covariances already computed for mean estimator
	compute_acv_control(covLL[qoi], F, covLH, qoi, beta);
      else // compute variances/covariances for higher-order moment estimators
	compute_acv_control(sum_L_base_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			    N_shared_q, F, qoi, beta); // all use shared counts
        // *** TO DO: support shared_approx_increment() --> baselineL

      Real& H_raw_mq = H_raw_mom(qoi, mom-1);
      H_raw_mq = sum_H_mq / N_shared_q; // first term to be augmented
      for (approx=0; approx<numApprox; ++approx) {
	if (outputLevel >= NORMAL_OUTPUT)
	  Cout << "   QoI " << qoi+1 << " Approx " << approx+1 << ": control "
	       << "variate beta = " << std::setw(9) << beta[approx] << '\n';
	// For ACV, shared counts are fixed at N_H for all approx
	apply_control(sum_L_base_m(qoi,approx), N_shared_q,
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
  size_t incr = one_sided_delta(average(N_H_projected), avg_hf_target);
  increment_samples(N_H_projected, incr);
  increment_equivalent_cost(incr, sequenceCost, numApprox);

  Real lf_target;
  for (size_t approx=0; approx<numApprox; ++approx) {
    lf_target = avg_eval_ratios[approx] * avg_hf_target;
    SizetArray& N_L_a = N_L_projected[approx];
    incr = one_sided_delta(average(N_L_a), lf_target);
    increment_samples(N_L_a, incr);
    increment_equivalent_cost(incr, sequenceCost, approx);
  }
}

} // namespace Dakota
