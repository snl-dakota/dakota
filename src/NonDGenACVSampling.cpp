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
  RealMatrix G;  RealVector g, N_vec(numApprox+1);
  N_vec[numApprox] = numSamples;
  for (int i=0; i<numApprox; ++i)
    N_vec[i] = (numApprox+1-i)*numSamples;
  for (dag_cit=model_dags.begin(); dag_cit!=model_dags.end(); ++dag_cit) {
    // for testing:
    const UShortArray& dag = *dag_cit;
    compute_parameterized_G_g(N_vec, dag, G, g);

    //set_active_dag(dag);
    //approximate_control_variate(); // *** must rely on virtual G/g definition
  }
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
  // ************************************
  // *** TO DO: invert N_vec ordering ***
  // ************************************

  size_t i, j;
  if (G.empty()) G.shapeUninitialized(numApprox, numApprox); // *** TO DO
  if (g.empty()) g.sizeUninitialized(numApprox);

  switch (mlmfSubMethod) {
  case SUBMETHOD_ACV_IS: { // Bomarito Eqs. 21-22
    Real bi, bj, N_i, N_bi, NpN_i, NpN_j, NpN_NpN;
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];  N_i = N_vec[i];  N_bi = N_vec[bi];  NpN_i = N_bi + N_i;
      g[i] = (bi == 0) ? 1./N_bi - 1./NpN_i : 0.;
      for (j=0; j<numApprox; ++j) { // *** TO DO: VERIFY SYMMETRY --> j<=i
	bj = dag[j];  NpN_j = N_vec[bj] + N_vec[j];  NpN_NpN = NpN_i * NpN_j;
	G(i,j)  = 0.;
	if (bi == bj)  G(i,j) += 1./N_bi - 1./NpN_i - 1./NpN_j + N_bi/NpN_NpN;
	if (bi ==  j)  G(i,j) += N_bi/NpN_NpN - 1./NpN_j;
	if ( i == bj)  G(i,j) += N_i /NpN_NpN - 1./NpN_i;
	if ( i ==  j)  G(i,j) += N_i /NpN_NpN;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_MF: { // Bomarito Eqs. 16-17
    Real N_0 = N_vec[0], N_i, N_bi, N_j, N_bj;
    for (i=0; i<numApprox; ++i) {
      N_i = N_vec[i];  N_bi = N_vec[dag[i]];
      g[i] = (std::min(N_bi, N_0) / N_bi - std::min(N_i, N_0) / N_i) / N_0;
      for (j=0; j<numApprox; ++j) { // *** TO DO: VERIFY SYMMETRY --> j<=i
	N_j = N_vec[j];  N_bj = N_vec[dag[j]];
	G(i,j) =
	  (std::min(N_bi, N_bj) / N_bj - std::min(N_bi, N_j) / N_j)  / N_bi +
	  (std::min(N_i,  N_j)  / N_j  - std::min(N_i, N_bj) / N_bj) / N_i;
      }
    }
    break;
  }
  case SUBMETHOD_ACV_RD: { // Bomarito Eqs. 19-20
    Real bi, bj, N_i, N_bi;
    for (i=0; i<numApprox; ++i) {
      bi = dag[i];  N_i = N_vec[i];  N_bi = N_vec[bi];
      g[i] = (bi == 0) ? 1./N_bi : 0.;
      for (j=0; j<numApprox; ++j) { // *** TO DO: VERIFY SYMMETRY --> j<=i
	bj = dag[j];   G(i,j)  = 0.;
	if (bi == bj)  G(i,j) += 1./N_bi;
	if (bi ==  j)  G(i,j) -= 1./N_bi;
	if ( i == bj)  G(i,j) -= 1./N_i;
	if ( i ==  j)  G(i,j) += 1./N_i;
      }
    }
    break;
  }
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
