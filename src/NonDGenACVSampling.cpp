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
    size_t i, K, L, M_minus_K;  UShortArray dag(numApprox);
    // Dakota low-to-high ordering (enumerate valid KL values and convert)
    for (K=0; K<=numApprox; ++K) {
      M_minus_K = numApprox - K;

      // K highest fidelity approximations target root (numApprox):
      for (i=M_minus_K; i<numApprox; ++i) dag[i] = numApprox;

      for (L=0; L<=K; ++L) { // ordered single recursion
	// M-K lowest fidelity approximations target L:
	for (i=0; i<M_minus_K; ++i)       dag[i] = numApprox - L;

	/* ACV/GenACV ordering:
	// K  highest fidelity approximations target root (0):
	for (i=0; i<K;         ++i)  dag[i] = 0;
	// M-K lowest fidelity approximations target L:
	for (i=K; i<numApprox; ++i)  dag[i] = L;
	*/

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

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Model DAGs:\n" << model_graphs << std::endl;
}


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDGenACVSampling::core_run()
{
  // Initialize for pilot sample
  numSamples = pilotSamples[numApprox]; // last in pilot array

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

    bestAvgEstVar = DBL_MAX;
    for (activeDAGIter  = modelDAGs.begin();
	 activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
      // compute the LF/HF evaluation ratios from shared samples and compute
      // ratio of MC and ACV mean sq errors (which incorporates anticipated
      // variance reduction from application of avg_eval_ratios).
      compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		     avgEstVar, avgEstVarRatio);
      update_best(avg_eval_ratios, avg_hf_target);// store state for restoration
      //reset_acv(); // reset state for next ACV execution
    }
    restore_best(avg_eval_ratios, avg_hf_target);
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
  N_H_actual.assign(numFunctions, 0);  N_H_alloc = 0;
  Real avg_hf_target = 0.;

  bestAvgEstVar = DBL_MAX;
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);
    update_best(avg_eval_ratios, avg_hf_target); // store state for restoration
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best(avg_eval_ratios, avg_hf_target);

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
  bestAvgEstVar = DBL_MAX;
  for (activeDAGIter  = modelDAGs.begin();
       activeDAGIter != modelDAGs.end(); ++activeDAGIter) {
    // compute the LF/HF evaluation ratios from shared samples and compute
    // ratio of MC and ACV mean sq errors (which incorporates anticipated
    // variance reduction from application of avg_eval_ratios).
    compute_ratios(var_L, sequenceCost, avg_eval_ratios, avg_hf_target,
		   avgEstVar, avgEstVarRatio);
    update_best(avg_eval_ratios, avg_hf_target); // store state for restoration
    //reset_acv(); // reset state for next ACV execution
  }
  restore_best(avg_eval_ratios, avg_hf_target);

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

  RealMatrix C_G_inv;  RealVector c_g;
  Real R_sq, N_H;
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

  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    //invert_C_G_matrix(covLL[qoi], GMat, C_G_inv);
    //compute_c_g_vector(covLH, qoi, gVec, c_g);
    //R_sq = compute_R_sq(C_G_inv, c_g, varH[qoi], N_H);
    //if (outputLevel >= DEBUG_OUTPUT)
    //  Cout << "-----------------------------\n"
    // 	     << "GenACV::estimator_variance_ratios(): C-G inverse =\n"
    // 	     << C_G_inv << "c-g vector =\n" << c_g
    // 	     << " Rsq[" << qoi << "] via invert() = " << R_sq
    // 	     << "\n-----------------------------\n" << std::endl;

    R_sq = compute_R_sq(covLL[qoi], GMat, covLH, gVec, qoi, varH[qoi], N_H);
    //if (outputLevel >= DEBUG_OUTPUT)
    //  Cout << "R_sq[" << qoi << "] via solve()  = " << R_sq
    //       << "\n-----------------------------\n" << std::endl;

    if (R_sq >= 1.) { // add nugget to C_G prior to solve()
      Cerr << "Warning: numerical issues in GenACV: R^2 > 1." << std::endl;
      /*
      Real nugget = 1.e-6;
      while (R_sq >= 1. and nugget_cntr <= 10) {
	R_sq = compute_R_sq(covLL[qoi], GMat, covLH, gVec, qoi,
	                    varH[qoi], N_H, nugget);
	nugget *= 10.;  ++nugget_cntr;
      }
      */
    }
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
  if (GMat.empty()) GMat.shapeUninitialized(numApprox);
  if (gVec.empty()) gVec.sizeUninitialized(numApprox);

  // define sample recursion sets backwards (from z_H down to lowest fid)
  RealVector z1, z2;  Real bi, bj, z1_i, z1_j, z2_i, z_H = N_vec[numApprox];
  if (mlmfSubMethod == SUBMETHOD_ACV_IS || mlmfSubMethod == SUBMETHOD_ACV_RD) {
    z1.size(numApprox);  z2.size(numApprox+1);  z2[numApprox] = z_H;
    unsigned short dag_curr, dag_next;
    UShortList path;  UShortList::reverse_iterator rit;
    for (i=0; i<numApprox; ++i) {
      // walk+store path to root
      dag_curr = i;  dag_next = dag[dag_curr];
      path.clear();  path.push_back(dag_curr);  path.push_back(dag_next);
      while (/*dag_next != numApprox &&*/ z2[dag_next] == 0) {
	dag_curr = dag_next;  dag_next = dag[dag_curr];
	path.push_back(dag_next);
      }
      // walk path in reverse direction to fill z1,z2
      rit = path.rbegin();  dag_next = *rit;  ++rit;
      for (; rit != path.rend(); ++rit) {
	dag_curr = *rit;
	z1[dag_curr] = z2[dag_next];
	z2[dag_curr] = N_vec[dag_curr] - z1[dag_curr];
	dag_next = dag_curr;
      }
    }
    /*
    // Ok for ordered single recusion (ACV-KL)
    for (int target=numApprox; target>=0; --target)
      for (i=0; i<numApprox; ++i)
	if (dag[i] == target)
	  { z1[i] = z2[target];  z2[i] = N_vec[i] - z1[i]; }
    */
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "GenACV unroll:\nz1:\n" << z1 << "z2:\n" << z2 << std::endl;
  }

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    Real z_i, z_j, zi_zj;//, z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];  z_i = N_vec[i];  z1_i = z1[i];  z2_i = z2[i];
      gVec[i] = (bi == numApprox) ? 1./z1_i - 1./z_i : 0.;
      for (j=0; j<=i; ++j) {
	bj = dag[j];  z_j = N_vec[j];  z1_j = z1[j];  //z2_j = z2[j];
	GMat(i,j) = 0.;  zi_zj = z_i * z_j;
	if (bi == bj)  GMat(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
	if (bi ==  j)  GMat(i,j) += z1_i/zi_zj - 1./z_j; // false for dag = M
	if (i  == bj)  GMat(i,j) += z1_j/zi_zj - 1./z_i; // false for dag = M
	if (i  ==  j)  GMat(i,j) += z2_i/zi_zj;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];
      z1_i = /*z2_bi = z_bi =*/ N_vec[bi];  z2_i = /*z_i =*/ N_vec[i];
      gVec[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<=i; ++j) {
	bj = dag[j];
	z1_j = /*z2_bj = z_bj =*/ N_vec[bj];  z2_j = /*z_j =*/ N_vec[j];
	GMat(i,j)
	  = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i
	  + (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: // Bomarito Eqs. 19-20
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];  z1_i = z1[i];  z2_i = z2[i];
      gVec[i] = (bi == numApprox) ? 1./z1_i : 0.;
      for (j=0; j<=i; ++j) {
	bj = dag[j];  z1_j = z1[j];  //z2_j = z2[j];
	GMat(i,j) = 0.;
	if (bi == bj)  GMat(i,j) += 1./z1_i;
	if (bi ==  j)  GMat(i,j) -= 1./z1_i; // always false for dag = M
	if ( i == bj)  GMat(i,j) -= 1./z1_j; // always false for dag = M
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


void NonDGenACVSampling::
update_best(const RealVector& avg_eval_ratios, Real avg_hf_target)
{
  // Store best result:
  // > could potentially prune some of this tracking for final_statistics mode
  //   = estimator_performance, but suppress this additional complexity for now
  if (valid_variance(avgEstVar) && // *** TO DO: insufficient due to averaging --> use something like a badNumericsFlag to prevent adoption of bogus solve
      avgEstVar < bestAvgEstVar) {
    bestDAGIter        = activeDAGIter;
    bestAvgEvalRatios  = avg_eval_ratios;
    bestAvgHFTarget    = avg_hf_target;
    // could recompute these to reduce tracking
    bestAvgEstVar      = avgEstVar;
    bestAvgEstVarRatio = avgEstVarRatio;
    // reference points for output
    if (pilotMgmtMode != OFFLINE_PILOT) {
      bestEstVarIter0 = estVarIter0;
      bestNumHIter0   = numHIter0;
    }
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Updating best DAG to:\n" << *bestDAGIter << std::endl;
  }
}


void NonDGenACVSampling::
restore_best(RealVector& avg_eval_ratios, Real& avg_hf_target)
{
  Cout << "\nBest estimator variance = " << bestAvgEstVar
       << " from DAG:\n" << *bestDAGIter << std::endl;
  // restore best state for compute/archive/print final results
  if (activeDAGIter != bestDAGIter) { // best is not most recent
    activeDAGIter   = bestDAGIter;
    avg_eval_ratios = bestAvgEvalRatios;
    avg_hf_target   = bestAvgHFTarget;
    avgEstVar       = bestAvgEstVar;
    avgEstVarRatio  = bestAvgEstVarRatio;
    if (pilotMgmtMode != OFFLINE_PILOT) {
      estVarIter0 = bestEstVarIter0;
      numHIter0   = bestNumHIter0;
    }
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Restoring best DAG to:\n" << *activeDAGIter
	   << "with avg_hf_target = " << avg_hf_target
	   << "\nand avg_eval_ratios =\n" << avg_eval_ratios << std::endl;
  }
}

} // namespace Dakota
