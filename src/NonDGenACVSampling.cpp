/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
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
#include "NonDGenACVSampling.hpp"
#include "ProblemDescDB.hpp"
#include "ActiveKey.hpp"

static const char rcsId[]="@(#) $Id: NonDACVSampling.cpp 7035 2010-10-22 21:45:39Z mseldre $";

namespace Dakota {


/** This constructor is called for a standard letter-envelope iterator 
    instantiation.  In this case, set_db_list_nodes has been called and 
    probDescDB can be queried for settings from the method specification. */
NonDGenACVSampling::
NonDGenACVSampling(ProblemDescDB& problem_db, Model& model):
  NonDACVSampling(problem_db, model), dagRecursionType(KL_GRAPH_RECURSION)
{
  // Unless the ensemble changes, the set of admissible DAGS is invariant:
  //UShortArraySet model_dags;
  generate_dags(modelDAGs);
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


void NonDGenACVSampling::generate_dags(UShortArraySet& model_graphs)
{
  // zero root directed acyclic graphs
  switch (dagRecursionType) {
  case KL_GRAPH_RECURSION: {
    size_t i, K, L;  UShortArray dag(numApprox);
    for (K=0; K<=numApprox; ++K) {
      for (L=0; L<=K; ++L) { // ordered single recursion
	for (i=0; i<K;         ++i)  dag[i] = 0;
	for (i=K; i<numApprox; ++i)  dag[i] = L;
	model_graphs.insert(dag);
      }
    }
    break;
  }
  case SINGLE_GRAPH_RECURSION: {
    size_t i, K, L;  UShortArray dag(numApprox);
    for (K=0; K<=numApprox; ++K) {
      for (L=0; L<=numApprox; ++L) { // unordered single recursion ???
	for (i=0; i<K;         ++i)  dag[i] = 0;
	for (i=K; i<numApprox; ++i)  dag[i] = L;
	if (contains(dag, 0)) // enforce constraints (contains 0, acyclic)
	  model_graphs.insert(dag);
      }
    }
    break;
  }
  case MULTI_GRAPH_RECURSION: {
    break;
  }
  }
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDGenACVSampling::core_run()
{
  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array
  bestAvgEstVar = DBL_MAX;

  switch (pilotMgmtMode) {
  case  ONLINE_PILOT: // iterated ACV (default)
    generalized_acv_online_pilot();     break;
  case OFFLINE_PILOT: // computes perf for offline pilot/Oracle correlation
    generalized_acv_offline_pilot();    break;
  case PILOT_PROJECTION: // for algorithm assessment/selection
    generalized_acv_pilot_projection(); break;
  }

  /*
  Initial DAG-enumeration of inherited implementations was wasteful:
  > relied on duplication detection for reusing pilot
  > allowed/defaulted to online mode for each DAG (wasteful for poor DAG)
  Adopt same approach proposed for AAO hyper-parameter model tuning --> need
  shared increments for online_pilot mode on outer-most loop around DAG/ensemble
  selection, followed by LF increments only after finalization of config.

  for (activeDAGIter=modelDAGs.begin(); activeDAGIter!=modelDAGs.end();
       ++activeDAGIter) {
    Cout << "Generalized ACV evaluating DAG:\n" << *activeDAGIter << std::endl;

    NonDACVSampling::core_run(); // {online,offline,projection} modes

    update_best(); // store reqd state for restoration w/o any recomputation
    reset_acv();   // reset state for next ACV execution
  }

  restore_best();
  // Post-process ...
  */
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_online_pilot()
{
  // retrieve cost estimates across soln levels for a particular model form
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;
  RealVector sum_HH, avg_eval_ratios;  RealMatrix var_L;
  //SizetSymMatrixArray N_LL;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;

  Real avg_hf_target = 0.;
  while (numSamples && mlmfIter <= maxIterations) {

    // --------------------------------------------------------------------
    // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
    // --------------------------------------------------------------------
    shared_increment(mlmfIter); // spans ALL models, blocking
    accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
			sum_LH, sum_HH, N_H_actual);//, N_LL);
    N_H_alloc += (backfillFailures && mlmfIter) ?
      one_sided_delta(N_H_alloc, avg_hf_target) : numSamples;
    // While online cost recovery could be continuously updated, we restrict
    // to the pilot and do not not update after iter 0.  We could potentially
    // update cost for shared samples, mirroring the covariance updates.
    if (onlineCost && mlmfIter == 0) recover_online_cost(sequenceCost);
    increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,
			      equivHFEvals);
    compute_LH_statistics(sum_L_baselineH[1], sum_H[1], sum_LL[1], sum_LH[1],
			  sum_HH, N_H_actual, var_L, varH, covLL, covLH);

    for (activeDAGIter  = modelDAGs.begin();
	 activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
      //N_H_actual.assign(numFunctions, 0); N_H_alloc = 0; avg_hf_target = 0.;// ***

      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		     avgEstVar, avgEstVarRatio);
      update_best(); // store reqd state for restoration w/o any recomputation
      //reset_acv(); // reset state for next ACV execution
    }
    restore_best();
    ++mlmfIter;
  }

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, avg_eval_ratios, avg_hf_target);
  else
    // N_H is final --> do not compute any deltaNActualHF (from maxIter exit)
    update_projected_lf_samples(avg_hf_target, avg_eval_ratios, N_H_actual,
				N_H_alloc, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_offline_pilot()
{
  // ------------------------------------------------------------
  // Compute var L,H & covar LL,LH from (oracle) pilot treated as "offline" cost
  // ------------------------------------------------------------
  RealVector sum_H_pilot, sum_HH_pilot;
  RealMatrix sum_L_pilot, sum_LH_pilot, var_L;
  RealSymMatrixArray sum_LL_pilot;  SizetArray N_shared_pilot;
  evaluate_pilot(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
		 sum_HH_pilot, N_shared_pilot, false);
  compute_LH_statistics(sum_L_pilot, sum_H_pilot, sum_LL_pilot, sum_LH_pilot,
			sum_HH_pilot, N_shared_pilot, var_L, varH, covLL,covLH);

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  IntRealVectorMap sum_H;  IntRealMatrixMap sum_L_baselineH, sum_LH;
  IntRealSymMatrixArrayMap sum_LL;  RealVector sum_HH, avg_eval_ratios;
  initialize_acv_sums(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH);
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];
  Real avg_hf_target;

  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    //N_H_actual.assign(numFunctions, 0); N_H_alloc = 0; avg_hf_target = 0.;// ***

    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);
    update_best(); // store reqd state for restoration w/o any recomputation
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best();

  // -----------------------------------
  // Perform "online" sample increments:
  // -----------------------------------
  // at least 2 samples reqd for variance (+ resetting allSamples from pilot)
  numSamples = std::max(numSamples, (size_t)2);  ++mlmfIter;
  shared_increment(mlmfIter); // spans ALL models, blocking
  accumulate_acv_sums(sum_L_baselineH, /*sum_L_baselineL,*/ sum_H, sum_LL,
		      sum_LH, sum_HH, N_H_actual);//, N_LL);
  N_H_alloc += numSamples;
  increment_equivalent_cost(numSamples, sequenceCost, 0, numSteps,equivHFEvals);
  // allow pilot to vary for C vs c

  // Only QOI_STATISTICS requires application of oversample ratios and
  // estimation of moments; ESTIMATOR_PERFORMANCE can bypass this expense.
  if (finalStatsType == QOI_STATISTICS)
    approx_increments(sum_L_baselineH, sum_H, sum_LL, sum_LH, N_H_actual,
		      N_H_alloc, avg_eval_ratios, avg_hf_target);
  else
    // N_H is converged from offline pilot --> do not compute deltaNActualHF
    update_projected_lf_samples(avg_hf_target, avg_eval_ratios, N_H_actual,
				N_H_alloc, deltaEquivHF);
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_pilot_projection()
{
  size_t hf_form_index, hf_lev_index;  hf_indices(hf_form_index, hf_lev_index);
  SizetArray& N_H_actual = NLevActual[hf_form_index][hf_lev_index];
  size_t&     N_H_alloc  =  NLevAlloc[hf_form_index][hf_lev_index];

  // --------------------------------------------------------------------
  // Evaluate shared increment and update correlations, {eval,EstVar}_ratios
  // --------------------------------------------------------------------
  RealVector sum_H, sum_HH;   RealMatrix sum_L_baselineH, sum_LH, var_L;
  RealSymMatrixArray sum_LL;
  evaluate_pilot(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH,
		 N_H_actual, true);
  compute_LH_statistics(sum_L_baselineH, sum_H, sum_LL, sum_LH, sum_HH,
			N_H_actual, var_L, varH, covLL, covLH);
  N_H_alloc = numSamples;

  // -----------------------------------
  // Compute "online" sample increments:
  // -----------------------------------
  RealVector avg_eval_ratios;  Real avg_hf_target = 0.;
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);
    update_best(); // store reqd state for restoration w/o any recomputation
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best();

  ++mlmfIter;
  // No LF increments or final moments for pilot projection
  update_projected_samples(avg_hf_target, avg_eval_ratios, N_H_actual,
			   N_H_alloc, deltaNActualHF, deltaEquivHF);
  // No need for updating estimator variance given deltaNActualHF since
  // NonDNonHierarchSampling::nonhierarch_numerical_solution() recovers N*
  // from the numerical solve and computes projected avgEstVar{,Ratio}
}


void NonDGenACVSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  RealMatrix C_G_inv;  RealVector c_g;  Real R_sq, N_H;
  switch (optSubProblemForm) {
  case R_ONLY_LINEAR_CONSTRAINT: {
    RealVector N_vec(numApprox+1, false);
    if (cd_vars.length() == numApprox) {
      copy_data_partial(cd_vars, N_vec, 0);
      // N_H not provided so pull from latest counter values
      size_t hf_form_index, hf_lev_index;
      hf_indices(hf_form_index, hf_lev_index);
      // estimator variance uses actual (not alloc) so use same for defining G,g
      // *** TO DO: but avg_hf_target defines delta relative to actual||alloc
      N_vec[numApprox] = N_H = //(backfillFailures) ?
	average(NLevActual[hf_form_index][hf_lev_index]);// :
	//NLevAlloc[hf_form_index][hf_lev_index];
    }
    else { // N_H appended for convenience or rescaling to updated HF target
      copy_data(cd_vars, N_vec);
      N_H = N_vec[numApprox];
    }
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec, *activeDAGIter);
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: { // convert r_and_N to N_vec:
    RealVector N_vec;  copy_data(cd_vars, N_vec);
    N_H = N_vec[numApprox];
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec, *activeDAGIter);
    break;
  }
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
    compute_parameterized_G_g(cd_vars, *activeDAGIter);
    N_H = cd_vars[numApprox];
    break;
  }

  // N is an opt. variable for
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    invert_C_G_matrix(covLL[qoi], GMat, C_G_inv);
    //Cout << "C-G inverse =\n" << C_G_inv << std::endl;
    compute_c_g_vector(covLH, qoi, gVec, c_g);
    //Cout << "c-g vector =\n" << c_g << std::endl;
    compute_R_sq(C_G_inv, c_g, varH[qoi], N_H, R_sq);
    //Cout << "varH[" << qoi << "] = " << varH[qoi] << " Rsq[" << qoi << "] =\n"
    //     << R_sq << std::endl;
    estvar_ratios[qoi] = (1. - R_sq);
  }
}


void NonDGenACVSampling::
compute_parameterized_G_g(const RealVector& N_vec, const UShortArray& dag)
{
  // Invert N_vec ordering
  // > Dakota r_i ordering is low-to-high --> reversed from Peherstorfer
  //   (HF = 1, MF = 2, LF = 3) and Gorodetsky,Bomarito (HF = 0, MF = 1, LF = 2)
  // > incoming N_vec is ordered as for Dakota r_i: LF = 0, MF = 1, HF = 2
  // > incoming DAG is zero root as in Bomarito 2022 --> reverse DAG ordering
  // See also NonDNonHierarchSampling::mfmc_analytic_solution()

  // Here we use ACV notation: z_i is comprised of N_i incoming samples
  // including z^1_i (less resolved "correlated mean estimator") and z^2_i
  // (more resolved "control variate mean").  Mapping from GenACV notation:
  // "z^*_i" --> z^1_i, "z_i" --> z^2_i, "z_{i* U i}" --> z_i

  size_t i, j;
  if (GMat.empty()) GMat.shapeUninitialized(numApprox, numApprox);
  if (gVec.empty()) gVec.sizeUninitialized(numApprox);

  Real bi, bj, z_i, z1_i, z2_i;
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    Real z_j, zi_zj;//, z1_j, z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = numApprox - dag[i]; // reverse DAG ordering for sample ordering
      z_i = N_vec[i];  z1_i = N_vec[bi];  z2_i = z_i - z1_i;
      gVec[i] = (bi == numApprox) ? 1./z1_i - 1./z_i : 0.;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j]; // reverse DAG ordering for sample ordering
	z_j = N_vec[j];  //z_1j = N_vec[bj];  z2_j = z_j - z1_j;
	zi_zj = z_i * z_j;  GMat(i,j)  = 0.;
	if (bi == bj)  GMat(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
	if (bi ==  j)  GMat(i,j) += z1_i/zi_zj - 1./z_j; // false for dag = 0
	if (i  == bj)  GMat(i,j) += z2_i/zi_zj - 1./z_i; // false for dag = 0
	if (i  ==  j)  GMat(i,j) += z2_i/zi_zj;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real z_H = N_vec[numApprox], z_j, z1_j, z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = numApprox - dag[i]; // reverse DAG ordering for sample ordering
      z_i = z2_i = N_vec[i];  z1_i = N_vec[bi];  // *** CONFIRM z2_i
      gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j]; // reverse DAG ordering for sample ordering
	z_j = z2_j = N_vec[j];  z1_j = N_vec[bj]; // *** CONFIRM z2_j
	GMat(i,j)
	  = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
	  + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: // Bomarito Eqs. 19-20
    for (i=0; i<numApprox; ++i) {
      bi = numApprox - dag[i];
      z_i = N_vec[i];  z1_i = N_vec[bi];  z2_i = z_i - z1_i;
      gVec[i] = (bi == numApprox) ? 1./z1_i : 0.;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j];   GMat(i,j)  = 0.;
	if (bi == bj)  GMat(i,j) += 1./z1_i;
	if (bi ==  j)  GMat(i,j) -= 1./z1_i; // always false for dag = 0
	if ( i == bj)  GMat(i,j) -= 1./z2_i; // always false for dag = 0
	if ( i ==  j)  GMat(i,j) += 1./z2_i;
      }
    }
    break;
  default:
    Cerr << "Error: bad sub-method name (" << mlmfSubMethod << ") in NonDGen"
	 << "ACVSampling::compute_parameterized_G_g()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "For dag:\n"  << dag  << "G matrix:\n" << GMat
	 << "g vector:\n" << gVec << std::endl;
}


void NonDGenACVSampling::update_best()
{
  // Store best result: *** TO DO: identify required state
  // > if DAG evaluation mode will be same as final mode, then could go ahead
  //   and post-process and store only best final stats for results reporting
  // > if DAG evalation mode (projection) could differ from final mode
  //   (iteration), then need to store intermediate state to allow additional
  //   iteraton to pick up where it left off
  if (avgEstVar < bestAvgEstVar)
    { bestAvgEstVar = avgEstVar;  bestDAGIter = activeDAGIter; }// ... *** TO DO
}


void NonDGenACVSampling::restore_best()
{
  Cout << "Best estimator variance = " << bestAvgEstVar
       << " from DAG:\n" << *bestDAGIter << std::endl;
  // TO DO: restore best state for compute/archive/print final results
  activeDAGIter = bestDAGIter;  avgEstVar = bestAvgEstVar; //... *** TO DO
}

} // namespace Dakota
