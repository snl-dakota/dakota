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
  mlmfSubMethod = problem_db.get_ushort("method.sub_method");

  if (maxFunctionEvals == SZ_MAX) // accuracy constraint (convTol)
    optSubProblemForm = N_VECTOR_LINEAR_OBJECTIVE;
  else                     // budget constraint (maxFunctionEvals)
    // truthFixedByPilot is a user-specified option for fixing the number of
    // HF samples (to those in the pilot).  In this case, equivHF budget is
    // allocated by optimizing r* for fixed N.
    optSubProblemForm = (truthFixedByPilot && pilotMgmtMode != OFFLINE_PILOT) ?
      R_ONLY_LINEAR_CONSTRAINT : N_VECTOR_LINEAR_CONSTRAINT;

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "ACV sub-method selection = " << mlmfSubMethod
	 << " sub-method formulation = "  << optSubProblemForm
	 << " sub-problem solver = "      << optSubProblemSolver << std::endl;
}


NonDGenACVSampling::~NonDGenACVSampling()
{ }


/** The primary run function manages the general case: a hierarchy of model 
    forms (from the ordered model fidelities within a HierarchSurrModel), 
    each of which may contain multiple discretization levels. */
void NonDGenACVSampling::core_run()
{
  UShortArraySet model_dags;
  generate_dags(model_dags);

  UShortArraySet::const_iterator dag_cit;  bestAvgEstVar = DBL_MAX;
  for (dag_cit=model_dags.begin(); dag_cit!=model_dags.end(); ++dag_cit) {
    activeDAG = *dag_cit;
    Cout << "Generalized ACV evaluating DAG:\n" << activeDAG << std::endl;

    NonDACVSampling::core_run(); // {online,offline,projection} modes

    // store best result:
    if (avgEstVar < bestAvgEstVar)
      { bestAvgEstVar = avgEstVar;  bestDAG = activeDAG; }//***TO DO: REQD STATE
    // reset state for next ACV execution:
    reset_acv();
  }

  // Post-process
  Cout << "Best estimator variance = " << bestAvgEstVar
       << " from DAG:\n" << bestDAG << std::endl;
  // *** TO DO: restore best state, compute/store/print final results
}


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


void NonDGenACVSampling::
estimator_variance_ratios(const RealVector& cd_vars, RealVector& estvar_ratios)
{
  if (estvar_ratios.empty()) estvar_ratios.sizeUninitialized(numFunctions);

  RealMatrix G, C_G_inv;  RealVector g, c_g;  Real R_sq, N_H;
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
    compute_parameterized_G_g(N_vec, activeDAG, G, g);
    break;
  }
  case R_AND_N_NONLINEAR_CONSTRAINT: { // convert r_and_N to N_vec:
    RealVector N_vec;  copy_data(cd_vars, N_vec);
    N_H = N_vec[numApprox];
    for (size_t i=0; i<numApprox; ++i)
      N_vec[i] *= N_H; // N_i = r_i * N
    compute_parameterized_G_g(N_vec, activeDAG, G, g);
    break;
  }
  case N_VECTOR_LINEAR_OBJECTIVE:  case N_VECTOR_LINEAR_CONSTRAINT:
    compute_parameterized_G_g(cd_vars, activeDAG, G, g);
    N_H = cd_vars[numApprox];
    break;
  }

  // N is an opt. variable for
  for (size_t qoi=0; qoi<numFunctions; ++qoi) {
    invert_C_G_matrix(covLL[qoi], G, C_G_inv);
    //Cout << "C-G inverse =\n" << C_G_inv << std::endl;
    compute_c_g_vector(covLH, qoi, g, c_g);
    //Cout << "c-g vector =\n" << c_g << std::endl;
    compute_R_sq(C_G_inv, c_g, varH[qoi], N_H, R_sq);
    //Cout << "varH[" << qoi << "] = " << varH[qoi] << " Rsq[" << qoi << "] =\n"
    //     << R_sq << std::endl;
    estvar_ratios[qoi] = (1. - R_sq);
  }
}


void NonDGenACVSampling::
compute_parameterized_G_g(const RealVector& N_vec, const UShortArray& dag,
			  RealMatrix& G, RealVector& g)
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
  if (G.empty()) G.shapeUninitialized(numApprox, numApprox);
  if (g.empty()) g.sizeUninitialized(numApprox);

  Real bi, bj, z_i, z1_i, z2_i;
  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    Real z_j, zi_zj;//, z1_j, z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = numApprox - dag[i]; // reverse DAG ordering for sample ordering
      z_i = N_vec[i];  z1_i = N_vec[bi];  z2_i = z_i - z1_i;
      g[i] = (bi == numApprox) ? 1./z1_i - 1./z_i : 0.;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j]; // reverse DAG ordering for sample ordering
	z_j = N_vec[j];  //z_1j = N_vec[bj];  z2_j = z_j - z1_j;
	zi_zj = z_i * z_j;  G(i,j)  = 0.;
	if (bi == bj)  G(i,j) += 1./z1_i - 1./z_i - 1./z_j + z1_i/zi_zj;
	if (bi ==  j)  G(i,j) += z1_i/zi_zj - 1./z_j; // false for dag = 0
	if (i  == bj)  G(i,j) += z2_i/zi_zj - 1./z_i; // false for dag = 0
	if (i  ==  j)  G(i,j) += z2_i/zi_zj;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real z_H = N_vec[numApprox], z_j, z1_j, z2_j;
    for (i=0; i<numApprox; ++i) {
      bi = numApprox - dag[i]; // reverse DAG ordering for sample ordering
      z_i = z2_i = N_vec[i];  z1_i = N_vec[bi];  // *** CONFIRM z2_i
      g[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j]; // reverse DAG ordering for sample ordering
	z_j = z2_j = N_vec[j];  z1_j = N_vec[bj]; // *** CONFIRM z2_j
	G(i,j) = (std::min(z1_i, z1_j)/z1_j - std::min(z1_i, z2_j)/z2_j)/z1_i +
	         (std::min(z2_i, z2_j)/z2_j - std::min(z2_i, z1_j)/z1_j)/z2_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: // Bomarito Eqs. 19-20
    for (i=0; i<numApprox; ++i) {
      bi = numApprox - dag[i];
      z_i = N_vec[i];  z1_i = N_vec[bi];  z2_i = z_i - z1_i;
      g[i] = (bi == numApprox) ? 1./z1_i : 0.;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j];   G(i,j)  = 0.;
	if (bi == bj)  G(i,j) += 1./z1_i;
	if (bi ==  j)  G(i,j) -= 1./z1_i; // always false for dag = 0
	if ( i == bj)  G(i,j) -= 1./z2_i; // always false for dag = 0
	if ( i ==  j)  G(i,j) += 1./z2_i;
      }
    }
    break;
  default:
    Cerr << "Error: bad sub-method name (" << mlmfSubMethod << ") in NonDGen"
	 << "ACVSampling::compute_parameterized_F_matrix()" << std::endl;
    abort_handler(METHOD_ERROR); break;
  }

  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "For dag:\n" << dag << "G matrix:\n" << G << "g vector:\n" << g
	 << std::endl;
}


// *** TO DO: can this share more code with the base ACV version?
//            e.g., cache G,g,F and then virtual compute_control()
void NonDGenACVSampling::
acv_raw_moments(IntRealMatrixMap& sum_L_baseline,
		IntRealMatrixMap& sum_L_refined,   IntRealVectorMap& sum_H,
		IntRealSymMatrixArrayMap& sum_LL,  IntRealMatrixMap& sum_LH,
		const RealVector& avg_eval_ratios, const SizetArray& N_shared,
		const Sizet2DArray& N_L_refined,   RealMatrix& H_raw_mom)
{
  if (H_raw_mom.empty()) H_raw_mom.shapeUninitialized(numFunctions, 4);

  // Note: while G,g have a more explicit dependence on N_shared[qoi] than F,
  // we mirror the averaged sample allocations and compute G,g once
  RealMatrix G;  RealVector g, N_vec;
  r_and_N_to_N_vec(avg_eval_ratios, average(N_shared), N_vec);
  compute_parameterized_G_g(N_vec, activeDAG, G, g); // *** TO DO: pass bestDAG or set activeDAG = bestDAG before final roll-up?

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
	compute_genacv_control(covLL[qoi], G, covLH, g, qoi, beta);
      else // compute variances/covariances for higher-order moment estimators
	compute_genacv_control(sum_L_base_m, sum_H_mq, sum_LL_m[qoi], sum_LH_m,
			       N_shared_q, G, g, qoi, beta);// shared counts
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

} // namespace Dakota
