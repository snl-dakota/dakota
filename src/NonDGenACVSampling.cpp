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
}


/* *** Loop around core_run()?
void NonDGenACVSampling::generalized_acv()
{
  for (dag_it=model_dags.begin(); dag_it!=model_dags.end(); ++dag_it) {
    set_active_dag(dag);
    NonDACVSampling::core_run();
  }
}
*/


// *** Loop around individual acv fns?

void NonDGenACVSampling::generalized_acv_online_pilot()
{
  UShortArraySet model_dags;
  generate_dags(model_dags);

  UShortArraySet::const_iterator dag_cit;
  RealMatrix G;  RealVector g, N_vec(numApprox+1), avg_eval_ratios;
  for (int i=0; i<=numApprox; ++i)
    N_vec[i] = (numApprox+1-i)*numSamples;
  copy_data_partial(N_vec, 0, (int)numApprox, avg_eval_ratios);
  avg_eval_ratios.scale(1./N_vec[numApprox]);
  Cout << "N_vec:\n" << N_vec << "r_i:\n" << avg_eval_ratios << std::endl;

  for (dag_cit=model_dags.begin(); dag_cit!=model_dags.end(); ++dag_cit) {
    // for testing:
    const UShortArray& dag = *dag_cit;
    compute_parameterized_G_g(N_vec, dag, G, g);

    //set_active_dag(dag);
    //approximate_control_variate(); // *** must rely on virtual G/g definition
  }

  RealSymMatrix F;
  compute_F_matrix(avg_eval_ratios, F);
  // Differs by a scaling...
  F *= 1./(Real)numSamples;
  Cout << "Scaled F:\n" << F << std::endl;
}


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_offline_pilot()
{ }


/** This function performs control variate MC across two combinations of 
    model form and discretization level. */
void NonDGenACVSampling::generalized_acv_pilot_projection()
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
	if (contains(dag, 0)) // enforce constraints (this is one; also need cyclic check)
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
compute_parameterized_G_g(const RealVector& N_vec, const UShortArray& dag,
			  RealMatrix& G, RealVector& g)
                        //RealSymMatrix& G, RealVector& g) // *** TO DO
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
  if (G.empty()) G.shapeUninitialized(numApprox, numApprox); // *** TO DO
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
      z_i = N_vec[i];  z1_i = N_vec[bi];  z2_i = z_i; // *** CONFIRM z2_i
      g[i] = (std::min(z1_i, z_H) / z1_i - std::min(z2_i, z_H) / z2_i) / z_H;
      for (j=0; j<numApprox; ++j) {
	bj = numApprox - dag[j]; // reverse DAG ordering for sample ordering
	z_j = N_vec[j];  z1_j = N_vec[bj];  z2_j = z_j; // *** CONFIRM z2_j
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

} // namespace Dakota
