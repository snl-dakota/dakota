/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedMinimizer
//- Description: Implementation code for the SurrBasedMinimizer class
//- Owner:       Mike Eldred, Sandia National Laboratories
//- Checked by:

#include "SurrBasedMinimizer.hpp"
#include "SurrBasedLevelData.hpp"
#include "DakotaGraphics.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "dakota_data_io.hpp"

static const char rcsId[]="@(#) $Id: SurrBasedMinimizer.cpp 4718 2007-11-15 21:44:58Z wjbohnh $";

extern "C" {

#define NNLS_F77 F77_FUNC(nnls,NNLS)
void NNLS_F77( double* a, int& mda, int& m, int& n, double* b, double* x,
	       double& rnorm, double* w, double* zz, int* index, int& mode );

#ifdef DAKOTA_F90
  #if defined(HAVE_CONFIG_H) && !defined(DISABLE_DAKOTA_CONFIG_H)

  // Deprecated; continue to support legacy, clashing macros for ONE RELEASE
  #define BVLS_WRAPPER_FC FC_FUNC_(bvls_wrapper,BVLS_WRAPPER)
  void BVLS_WRAPPER_FC( Dakota::Real* a, int& m, int& n, Dakota::Real* b,
                        Dakota::Real* bnd, Dakota::Real* x, Dakota::Real& rnorm,
                        int& nsetp, Dakota::Real* w, int* index, int& ierr );
  #else

  // Use the CMake-generated fortran name mangling macros (eliminate warnings)
  #include "dak_f90_config.h"
  #define BVLS_WRAPPER_FC DAK_F90_GLOBAL_(bvls_wrapper,BVLS_WRAPPER)
  void BVLS_WRAPPER_FC( Dakota::Real* a, int& m, int& n, Dakota::Real* b,
                        Dakota::Real* bnd, Dakota::Real* x, Dakota::Real& rnorm,
                        int& nsetp, Dakota::Real* w, int* index, int& ierr );
  #endif // HAVE_CONFIG_H and !DISABLE_DAKOTA_CONFIG_H
#endif // DAKOTA_F90

}


using namespace std;

namespace Dakota {
  extern PRPCache data_pairs; // global container

SurrBasedMinimizer::SurrBasedMinimizer(ProblemDescDB& problem_db, Model& model):
  Minimizer(problem_db, model), globalIterCount(0),
  // See Conn, Gould, and Toint, pp. 598-599
  penaltyParameter(5.), eta(1.), alphaEta(0.1), betaEta(0.9),
  etaSequence(eta*std::pow(2.*penaltyParameter, -alphaEta))
{
  if (model.primary_fn_type() == OBJECTIVE_FNS)
    optimizationFlag  = true;
  else if (model.primary_fn_type() == CALIB_TERMS)
    optimizationFlag  = false;
  else {
    Cerr << "Error: unsupported response type specification in "
	 << "SurrBasedMinimizer constructor." << std::endl;
    abort_handler(-1);
  }

  // initialize attributes for merit function calculations
  origNonlinIneqLowerBnds
    = iteratedModel.nonlinear_ineq_constraint_lower_bounds();
  origNonlinIneqUpperBnds
    = iteratedModel.nonlinear_ineq_constraint_upper_bounds();
  origNonlinEqTargets = iteratedModel.nonlinear_eq_constraint_targets();

  // Verify that global bounds are available (some Constraints types can
  // return empty vectors) and are not set to the +/- infinity defaults (TR
  // size is relative to the global bounded region).
  const RealVector& lower_bnds = iteratedModel.continuous_lower_bounds();
  const RealVector& upper_bnds = iteratedModel.continuous_upper_bounds();
  if (lower_bnds.length() != numContinuousVars ||
      upper_bnds.length() != numContinuousVars) {
    Cerr << "\nError: mismatch in length of variable bounds array in "
	 << "SurrBasedMinimizer." << std::endl;
    abort_handler(-1);
  }
  for (size_t i=0; i<numContinuousVars; i++)
    if (lower_bnds[i] <= -bigRealBoundSize ||
	upper_bnds[i] >=  bigRealBoundSize) {
      Cerr << "\nError: variable bounds are required in SurrBasedMinimizer."
	   << std::endl;
      abort_handler(-1);
    }
}


SurrBasedMinimizer::~SurrBasedMinimizer()
{ }


void SurrBasedMinimizer::derived_init_communicators(ParLevLIter pl_iter)
{
  // iteratedModel is evaluated to add truth data (single evaluate())
  iteratedModel.init_communicators(pl_iter, maxEvalConcurrency);

  // Allocate comms in approxSubProbModel/iteratedModel for parallel SBM.
  // For DataFitSurrModel, concurrency is from daceIterator evals (global) or
  // numerical derivs (local/multipt) on actualModel.  For HierarchSurrModel,
  // concurrency is from approxSubProbMinimizer on lowFidelityModel.
  // As for constructors, we recursively set and restore DB list nodes
  // (initiated from the restored starting point following construction).
  size_t method_index = probDescDB.get_db_method_node(),
         model_index  = probDescDB.get_db_model_node(); // for restoration
  // As in SurrBasedLocalMinimizer::initialize_sub_minimizer(), the SBLM
  // model_pointer is relevant and any sub-method model_pointer is ignored
  probDescDB.set_db_method_node(approxSubProbMinimizer.method_id());
  probDescDB.set_db_model_nodes(iteratedModel.model_id());
  approxSubProbMinimizer.init_communicators(pl_iter);
  probDescDB.set_db_method_node(method_index); // restore method only
  probDescDB.set_db_model_nodes(model_index);  // restore all model nodes
}


void SurrBasedMinimizer::derived_set_communicators(ParLevLIter pl_iter)
{
  // Virtual destructor handles referenceCount at Strategy level.

  miPLIndex = methodPCIter->mi_parallel_level_index(pl_iter);

  // iteratedModel is evaluated to add truth data (single evaluate())
  iteratedModel.set_communicators(pl_iter, maxEvalConcurrency);

  // set communicators for approxSubProbModel/iteratedModel
  approxSubProbMinimizer.set_communicators(pl_iter);
}


void SurrBasedMinimizer::derived_free_communicators(ParLevLIter pl_iter)
{
  // Virtual destructor handles referenceCount at Strategy level.

  // free communicators for approxSubProbModel/iteratedModel
  approxSubProbMinimizer.free_communicators(pl_iter);

  // iteratedModel is evaluated to add truth data (single evaluate())
  iteratedModel.free_communicators(pl_iter, maxEvalConcurrency);
}


void SurrBasedMinimizer::initialize_graphics(int iterator_server_id)
{
  // may want to replace customized graphics w/ std graphics for use in
  // Hybrid & Concurrent Strategies
  //if (!strategyFlag) {

  Model& truth_model = (methodName == SURROGATE_BASED_LOCAL ||
                        methodName == SURROGATE_BASED_GLOBAL) ?
    iteratedModel.truth_model() : iteratedModel;
  OutputManager& mgr = parallelLib.output_manager();
  Graphics& dakota_graphics = mgr.graphics();
  const Variables& vars = truth_model.current_variables();
  const Response&  resp = truth_model.current_response();

  // For graphics, limit (currently) to server id 1, for both ded master
  // (parent partition rank 1) and peer partitions (parent partition rank 0)
  if (mgr.graph2DFlag && iterator_server_id == 1) { // initialize the 2D plots
    mgr.graphics_counter(0); // starting point is iteration 0
    dakota_graphics.create_plots_2d(vars, resp);
    dakota_graphics.set_x_labels2d("Surr-Based Iteration No.");
  }

  // For output/restart/tabular data, all Iterator masters stream output
  if (mgr.tabularDataFlag) { // initialize data tabulation
    mgr.graphics_counter(0); // starting point is iteration 0
    mgr.tabular_counter_label("iter_no");
    mgr.create_tabular_datastream(vars, resp);
  }

  //}
}


/** For the Rockafellar augmented Lagrangian, simple Lagrange multiplier
    updates are available which do not require the active constraint
    gradients.  For the basic Lagrangian, Lagrange multipliers are estimated
    through solution of a nonnegative linear least squares problem. */
void SurrBasedMinimizer::
update_lagrange_multipliers(const RealVector& fn_vals,
			    const RealMatrix& fn_grads)
{
  // solve nonnegative linear least squares [A]{lambda} = -{grad_f} for
  // lambda where A = gradient matrix for active/violated constraints.

  // identify active inequality constraints (equalities always active)
  size_t i, j, cntr = 0;
  IntList active_lag_ineq, lag_index;
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    int ineq_id = i+1; // can't use +/- 0
    Real g = fn_vals[numUserPrimaryFns+i], l_bnd = origNonlinIneqLowerBnds[i],
      u_bnd = origNonlinIneqUpperBnds[i];
    // check for active, not violated --> apply constraintTol on feasible side
    //   g < l_bnd + constraintTol, g > u_bnd - constraintTol
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      if (g < l_bnd + constraintTol) {
	active_lag_ineq.push_back(-ineq_id);
	lag_index.push_back(cntr);
      }
      ++cntr;
    }
    if (u_bnd < bigRealBoundSize) { // g has an upper bound
      if (g > u_bnd - constraintTol) {
	active_lag_ineq.push_back(ineq_id);
	lag_index.push_back(cntr);
      }
      ++cntr;
    }
  }

  // if there are active constraints, estimate the Lagrange multipliers
  size_t num_active_lag_ineq = active_lag_ineq.size(),
    num_active_lag = num_active_lag_ineq + numNonlinearEqConstraints;
  lagrangeMult = 0.;
  if (num_active_lag) {
    // form [A]
    RealVector A(num_active_lag*numContinuousVars);
    ILIter iter;
    for (i=0, iter=active_lag_ineq.begin(); i<num_active_lag_ineq; i++, iter++){
      int ineq_id = *iter;
      size_t index = numUserPrimaryFns + std::abs(ineq_id) - 1;
      const Real* grad_g = fn_grads[index];
      for (j=0; j<numContinuousVars; j++)
	A[i+j*num_active_lag] = (ineq_id > 0) ? grad_g[j] : -grad_g[j];
    }
    for (i=0; i<numNonlinearEqConstraints; i++) {
      const Real* grad_h
	= fn_grads[numUserPrimaryFns+numNonlinearIneqConstraints+i];
      for (j=0; j<numContinuousVars; j++)
	A[i+num_active_lag_ineq+j*num_active_lag] = grad_h[j];
    }

    // form -{grad_f}
    RealVector m_grad_f;
    const BoolDeque& sense = iteratedModel.primary_response_fn_sense();
    const RealVector&  wts = iteratedModel.primary_response_fn_weights();
    objective_gradient(fn_vals, fn_grads, sense, wts, m_grad_f);
    for (j=0; j<numContinuousVars; ++j)
      m_grad_f[j] = -m_grad_f[j];
    //Cout << "[A]:\n" << A << "-{grad_f}:\n" << m_grad_f;

    // solve bound-constrained least squares using Lawson & Hanson routines:
    // > if inequality-constrained, use non-negative least squares (NNLS)
    // > if equality-constrained, use bound-constrained least squares (BVLS)
    RealVector lambda(num_active_lag), w(num_active_lag);
    IntVector index(num_active_lag);
    double res_norm;
    int m = numContinuousVars, n = num_active_lag;
    if (numNonlinearEqConstraints) {
#ifdef DAKOTA_F90
      int nsetp, ierr;
      RealVector bnd(2*num_active_lag); // bounds on lambda
      // lawson_hanson2.f90: BVLS ignore bounds based on huge(), so +/-DBL_MAX
      // is sufficient here
      for (i=0; i<num_active_lag; ++i) {
	bnd[i*2]   = (i<num_active_lag_ineq) ? 0. : -DBL_MAX; // lower bound
	bnd[i*2+1] = DBL_MAX;                                 // upper bound
      }
      BVLS_WRAPPER_FC( A.values(), m, n, m_grad_f.values(), bnd.values(),
                       lambda.values(), res_norm, nsetp, w.values(),
                       index.values(), ierr );
      if (ierr) {
	Cerr << "\nError: BVLS failed in update_lagrange_multipliers()."
	     << std::endl;
	abort_handler(-1);
      }
#endif // DAKOTA_F90
    }
    else {
      int mda = numContinuousVars, mode;
      RealVector zz(numContinuousVars);
      NNLS_F77( A.values(), mda, m, n, m_grad_f.values(), lambda.values(),
                res_norm, w.values(), zz.values(), index.values(), mode );
      if (mode != 1) {
	Cerr << "\nError: NNLS failed in update_lagrange_multipliers()."
	     << std::endl;
	abort_handler(-1);
      }
    }
    //Cout << "{lambda}:\n" << lambda << "res_norm: " << res_norm << '\n';

    // update lagrangeMult from least squares solution
    cntr = 0;
    for (iter=lag_index.begin(); iter!=lag_index.end(); ++iter)
      lagrangeMult[*iter] = lambda[cntr++];
  }

#ifdef DEBUG
  Cout << "Lagrange multipliers updated:\n" << lagrangeMult << '\n';
#endif
}


/** For the Rockafellar augmented Lagrangian, simple Lagrange multiplier
    updates are available which do not require the active constraint
    gradients.  For the basic Lagrangian, Lagrange multipliers are estimated
    through solution of a nonnegative linear least squares problem. */
void SurrBasedMinimizer::
update_augmented_lagrange_multipliers(const RealVector& fn_vals)
{
  size_t i, j, cntr = 0;
  // The Rockafellar augmented Lagrangian has simple and explicit multiplier
  // updates.  augLagrangeMult is an "extended" multiplier vector in this case
  // and the update formulas are applied even for inactive constraints.
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    Real g = fn_vals[numUserPrimaryFns+i], g0, psi,
      l_bnd = origNonlinIneqLowerBnds[i], u_bnd = origNonlinIneqUpperBnds[i];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      g0 = l_bnd - g; // convert l <= g to l - g <= 0
      psi = std::max(g0, -augLagrangeMult[cntr]/2./penaltyParameter);
      augLagrangeMult[cntr++] += 2.*penaltyParameter*psi;
    }
    if (u_bnd < bigRealBoundSize) { // g has an upper bound
      g0 = g - u_bnd; // convert g <= u to g - u <= 0
      psi = std::max(g0, -augLagrangeMult[cntr]/2./penaltyParameter);
      augLagrangeMult[cntr++] += 2.*penaltyParameter*psi;
    }
  }
  for (i=0; i<numNonlinearEqConstraints; i++) {
    // convert to h0 = 0
    Real h0 = fn_vals[numUserPrimaryFns+numNonlinearIneqConstraints+i]
            - origNonlinEqTargets[i];
    augLagrangeMult[cntr++] += 2.*penaltyParameter*h0;
  }

  // New logic follows Conn, Gould, and Toint, section 14.4, Step 2.
  // penaltyParameter could be increased, but is not (see CGT)
  Real mu = 1./2./penaltyParameter; // conversion between r_p and mu penalties
  etaSequence *= std::pow(mu, betaEta);

#ifdef DEBUG
  Cout << "Augmented Lagrange multipliers updated:\n" << augLagrangeMult
       << "\neta updated: " << etaSequence << '\n';
#endif
}


void SurrBasedMinimizer::
initialize_filter(SurrBasedLevelData& tr_data, const RealVector& fn_vals)
{
  Real new_f = objective(fn_vals, iteratedModel.primary_response_fn_sense(),
			 iteratedModel.primary_response_fn_weights());
  Real new_g = (numNonlinearConstraints)
             ? constraint_violation(fn_vals, 0.) : 0.;
  tr_data.initialize_filter(new_f, new_g);
}


/** Update the paretoFilter with fn_vals if new iterate is non-dominated. */
bool SurrBasedMinimizer::
update_filter(SurrBasedLevelData& tr_data, const RealVector& fn_vals)
{
  Real new_f = objective(fn_vals, iteratedModel.primary_response_fn_sense(),
			 iteratedModel.primary_response_fn_weights());
  return (numNonlinearConstraints) ?
    tr_data.update_filter(new_f, constraint_violation(fn_vals, 0.)) :
    tr_data.update_filter(new_f);
}


/*  Return a filter-based merit function.  As a first cut, use the area
    swept out from the two points only.
Real SurrBasedMinimizer::
filter_merit(const RealVector& fns_center, const RealVector& fns_star)
{
  Real obj_delta = objective(fns_star, sense, wts)
                 - objective(fns_center, sense, wts),
        cv_delta = constraint_violation(fns_star,   0.)
                 - constraint_violation(fns_center, 0.);

  // This filter merit can be positive or negative.  The sign is not critical,
  // but the sign of the ratio is.  If one delta is zero, return the other.
  // *** TO DO: approx/truth must be in synch!!
  // *** TO DO: handle unconstrained and feasible cases properly!
  if (fabs(obj_delta) < DBL_MIN)
    return cv_delta;
  else if (std::fabs(cv_delta) < DBL_MIN)
    return obj_delta;
  else
    return obj_delta * cv_delta;
}
*/


/** The Lagrangian function computation sums the objective function
    and the Lagrange multipler terms for inequality/equality
    constraints.  This implementation follows the convention in
    Vanderplaats with g<=0 and h=0.  The bounds/targets passed in may
    reflect the original constraints or the relaxed constraints. */
Real SurrBasedMinimizer::
lagrangian_merit(const RealVector& fn_vals, const BoolDeque& sense,
		 const RealVector& primary_wts,
		 const RealVector& nln_ineq_l_bnds,
		 const RealVector& nln_ineq_u_bnds,
		 const RealVector& nln_eq_tgts)
{
  size_t i, cntr = 0;

  // objective function portion
  Real lag = objective(fn_vals, sense, primary_wts);

  // inequality constraint portion
  Real g0;
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    // check for active, not violated --> apply constraintTol on feasible side
    //   g < l_bnd + constraintTol, g > u_bnd - constraintTol
    // Note: if original bounds/targets, lagrangeMult will be 0 for inactive
    const Real& g = fn_vals[numUserPrimaryFns+i];
    const Real& l_bnd = nln_ineq_l_bnds[i];
    const Real& u_bnd = nln_ineq_u_bnds[i];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      g0 = l_bnd - g;                // convert l <= g to l - g <= 0
      if (g0 + constraintTol > 0.)   // g is active
	lag += lagrangeMult[cntr]*g0;
      ++cntr;
    }
    if (u_bnd < bigRealBoundSize) {  // g has an upper bound
      g0 = g - u_bnd;                // convert g <= u to g - u <= 0
      if (g0 + constraintTol > 0.)   // g is active
	lag += lagrangeMult[cntr]*g0;
      ++cntr;
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    // convert to h0 = 0
    Real h0 = fn_vals[numUserPrimaryFns+numNonlinearIneqConstraints+i]
            - nln_eq_tgts[i];
    lag += lagrangeMult[cntr++]*h0;
  }
  return lag;
}


void SurrBasedMinimizer::
lagrangian_gradient(const RealVector& fn_vals, const RealMatrix& fn_grads,
		    const BoolDeque& sense, const RealVector& primary_wts,
		    const RealVector& nln_ineq_l_bnds,
		    const RealVector& nln_ineq_u_bnds,
		    const RealVector& nln_eq_tgts, RealVector& lag_grad)
{
  size_t i, j, cntr = 0;

  // objective function portion
  objective_gradient(fn_vals, fn_grads, sense, primary_wts, lag_grad);

  // inequality constraint portion
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    // check for active, not violated --> apply constraintTol on feasible side
    //   g < l_bnd + constraintTol, g > u_bnd - constraintTol
    // Note: if original bounds/targets, lagrangeMult will be 0 for inactive
    const Real& g = fn_vals[numUserPrimaryFns+i];
    const Real* grad_g = fn_grads[numUserPrimaryFns+i];
    const Real& l_bnd = nln_ineq_l_bnds[i];
    const Real& u_bnd = nln_ineq_u_bnds[i];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      if (g < l_bnd + constraintTol) // g is active
	for (j=0; j<numContinuousVars; j++) // l - g <= 0  ->  grad g0 = -grad g
	  lag_grad[j] -= lagrangeMult[cntr] * grad_g[j];
      ++cntr;
    }
    if (u_bnd < bigRealBoundSize) {  // g has an upper bound
      if (g > u_bnd - constraintTol) // g is active
	for (j=0; j<numContinuousVars; j++) // g - u <= 0  ->  grad g0 = +grad g
	  lag_grad[j] += lagrangeMult[cntr] * grad_g[j];
      ++cntr;
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    const Real* grad_h
      = fn_grads[numUserPrimaryFns+numNonlinearIneqConstraints+i];
    for (j=0; j<numContinuousVars; j++)
      lag_grad[j] += lagrangeMult[cntr] * grad_h[j];
    ++cntr;
  }
}


void SurrBasedMinimizer::
lagrangian_hessian(const RealVector& fn_vals, const RealMatrix& fn_grads, 
		   const RealSymMatrixArray& fn_hessians,
		   const BoolDeque& sense, const RealVector& primary_wts,
		   const RealVector& nln_ineq_l_bnds,
		   const RealVector& nln_ineq_u_bnds,
		   const RealVector& nln_eq_tgts, RealSymMatrix& lag_hess)
{
  size_t i, j, k, index, cntr = 0;

  // objective function portion
  objective_hessian(fn_vals, fn_grads, fn_hessians, sense, primary_wts,
		    lag_hess);

  // inequality constraint portion
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    // check for active, not violated --> apply constraintTol on feasible side
    //   g < l_bnd + constraintTol, g > u_bnd - constraintTol
    // Note: if original bounds/targets, lagrangeMult will be 0 for inactive
    index = i + numUserPrimaryFns;
    const Real& g = fn_vals[index];
    const RealSymMatrix& hess_g = fn_hessians[index];
    const Real& l_bnd = nln_ineq_l_bnds[i];
    const Real& u_bnd = nln_ineq_u_bnds[i];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      if (g < l_bnd + constraintTol) // g is active
	for (j=0; j<numContinuousVars; j++) // l - g <= 0  ->  hess g0 = -hess g
	  for (k=0; k<=j; ++k)
	    lag_hess(j,k) -= lagrangeMult[cntr] * hess_g(j,k);
      ++cntr;
    }
    if (u_bnd < bigRealBoundSize) {  // g has an upper bound
      if (g > u_bnd - constraintTol) // g is active
	for (j=0; j<numContinuousVars; j++) // g - u <= 0  ->  hess g0 = +hess g
	  for (k=0; k<=j; ++k)
	    lag_hess(j,k) += lagrangeMult[cntr] * hess_g(j,k);
      ++cntr;
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    index = i + numUserPrimaryFns + numNonlinearIneqConstraints;
    const RealSymMatrix& hess_h = fn_hessians[index];
    for (j=0; j<numContinuousVars; j++)
      for (k=0; k<=j; ++k)
	lag_hess(j,k) += lagrangeMult[cntr] * hess_h(j,k);
    ++cntr;
  }
}


/** The Rockafellar augmented Lagrangian function sums the objective
    function, Lagrange multipler terms for inequality/equality
    constraints, and quadratic penalty terms for inequality/equality
    constraints.  This implementation follows the convention in
    Vanderplaats with g<=0 and h=0.  The bounds/targets passed in may
    reflect the original constraints or the relaxed constraints.*/
Real SurrBasedMinimizer::
augmented_lagrangian_merit(const RealVector& fn_vals, const BoolDeque& sense,
			   const RealVector& primary_wts,
			   const RealVector& nln_ineq_l_bnds,
			   const RealVector& nln_ineq_u_bnds,
			   const RealVector& nln_eq_tgts)
{
  size_t i, cntr = 0;

  // objective function portion
  Real aug_lag = objective(fn_vals, sense, primary_wts);

  // inequality constraint portion
  Real g0, psi;
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    // For the Rockafellar augmented Lagrangian, augLagrangeMult is an
    // "extended" multiplier vector and includes inactive constraints.
    const Real& g = fn_vals[numUserPrimaryFns+i];
    const Real& l_bnd = nln_ineq_l_bnds[i];
    const Real& u_bnd = nln_ineq_u_bnds[i];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      g0 = l_bnd - g; // convert l <= g to l - g <= 0
      psi = std::max(g0, -augLagrangeMult[cntr]/2./penaltyParameter);
      aug_lag += (augLagrangeMult[cntr++] + penaltyParameter*psi)*psi;
    }
    if (u_bnd < bigRealBoundSize) { // g has an upper bound
      g0 = g - u_bnd; // convert g <= u to g - u <= 0
      psi = std::max(g0, -augLagrangeMult[cntr]/2./penaltyParameter);
      aug_lag += (augLagrangeMult[cntr++] + penaltyParameter*psi)*psi;
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    // convert to h0 = 0
    Real h0 = fn_vals[numUserPrimaryFns+numNonlinearIneqConstraints+i]
            - nln_eq_tgts[i];
    aug_lag += (augLagrangeMult[cntr++] + penaltyParameter*h0)*h0;
  }
  return aug_lag;
}


void SurrBasedMinimizer::
augmented_lagrangian_gradient(const RealVector& fn_vals, 
			      const RealMatrix& fn_grads,
			      const BoolDeque&  sense,
			      const RealVector& primary_wts,
			      const RealVector& nln_ineq_l_bnds,
			      const RealVector& nln_ineq_u_bnds,
			      const RealVector& nln_eq_tgts,
			      RealVector& alag_grad)
{
  size_t i, j, index, cntr = 0;

  // objective function portion
  objective_gradient(fn_vals, fn_grads, sense, primary_wts, alag_grad);

  // inequality constraint portion
  Real g0;
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    // For the Rockafellar augmented Lagrangian, augLagrangeMult is an
    // "extended" multiplier vector and includes inactive constraints.
    index = i + numUserPrimaryFns;
    const Real& g = fn_vals[index];
    const Real& l_bnd = nln_ineq_l_bnds[i];
    const Real& u_bnd = nln_ineq_u_bnds[i];
    const Real* grad_g = fn_grads[index];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      g0 = l_bnd - g; // convert l <= g to l - g <= 0
      // grad psi = grad g0 if "active", 0 if "inactive"
      if (g0 >= -augLagrangeMult[cntr]/2./penaltyParameter)
	for (j=0; j<numContinuousVars; j++)
	  alag_grad[j] -= (augLagrangeMult[cntr] + 2.*penaltyParameter*g0)
                       *  grad_g[j]; // l - g <= 0  -->  grad g0 = -grad g
      ++cntr;
    }
    if (u_bnd < bigRealBoundSize) { // g has an upper bound
      g0 = g - u_bnd; // convert g <= u to g - u <= 0
      // grad psi = grad g0 if "active", 0 if "inactive"
      if (g0 >= -augLagrangeMult[cntr]/2./penaltyParameter)
	for (j=0; j<numContinuousVars; j++)
	  alag_grad[j] += (augLagrangeMult[cntr] + 2.*penaltyParameter*g0)
                       *  grad_g[j]; // g - u <= 0  -->  grad g0 = +grad g
      ++cntr;
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    index = i + numUserPrimaryFns + numNonlinearIneqConstraints;
    Real h0 = fn_vals[index] - nln_eq_tgts[i]; // convert to h0 = 0
    const Real* grad_h = fn_grads[index];
    for (j=0; j<numContinuousVars; j++)
      alag_grad[j] += (augLagrangeMult[cntr] + 2.*penaltyParameter*h0)
                   *  grad_h[j];
    ++cntr;
  }
}


void SurrBasedMinimizer::
augmented_lagrangian_hessian(const RealVector& fn_vals, 
			     const RealMatrix& fn_grads, 
			     const RealSymMatrixArray& fn_hessians,
			     const BoolDeque&  sense,
			     const RealVector& primary_wts,
			     const RealVector& nln_ineq_l_bnds,
			     const RealVector& nln_ineq_u_bnds,
			     const RealVector& nln_eq_tgts,
			     RealSymMatrix& alag_hess)
{
  size_t i, j, k, index, cntr = 0;

  // objective function portion
  objective_hessian(fn_vals, fn_grads, fn_hessians, sense, primary_wts,
		    alag_hess);

  // inequality constraint portion
  Real g0;
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    // For the Rockafellar augmented Lagrangian, augLagrangeMult is an
    // "extended" multiplier vector and includes inactive constraints.
    index = i + numUserPrimaryFns;
    const Real& g = fn_vals[index];
    const Real& l_bnd = nln_ineq_l_bnds[i];
    const Real& u_bnd = nln_ineq_u_bnds[i];
    const RealSymMatrix& hess_g = fn_hessians[index];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      g0 = l_bnd - g; // convert l <= g to l - g <= 0
      // grad psi = grad g0 if "active", 0 if "inactive"
      if (g0 >= -augLagrangeMult[cntr]/2./penaltyParameter) {
	Real term = augLagrangeMult[cntr] + 2.*penaltyParameter*g0;
	for (j=0; j<numContinuousVars; j++)
	  for (k=0; k<=j; ++k) // l - g <= 0  -->  hess g0 = -hess g
	    alag_hess(j,k) -= term * hess_g(j,k);
      }
      ++cntr;
    }
    if (u_bnd < bigRealBoundSize) { // g has an upper bound
      g0 = g - u_bnd; // convert g <= u to g - u <= 0
      // grad psi = grad g0 if "active", 0 if "inactive"
      if (g0 >= -augLagrangeMult[cntr]/2./penaltyParameter) {
	Real term = augLagrangeMult[cntr] + 2.*penaltyParameter*g0;
	for (j=0; j<numContinuousVars; j++)
	  for (k=0; k<=j; ++k) // g - u <= 0  -->  hess g0 = +hess g
	    alag_hess(j,k) += term * hess_g(j,k);
      }
      ++cntr;
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    index = i + numUserPrimaryFns + numNonlinearIneqConstraints;
    const RealSymMatrix& hess_h = fn_hessians[index];
    Real h0 = fn_vals[index] - nln_eq_tgts[i], // convert to h0 = 0
      term  = augLagrangeMult[cntr] + 2.*penaltyParameter*h0;
    for (j=0; j<numContinuousVars; j++)
      for (k=0; k<=j; ++k) // g - u <= 0  -->  hess g0 = +hess g
	alag_hess(j,k) += term * hess_h(j,k);
    ++cntr;
  }
}


/** The penalty function computation applies a quadratic penalty to
    any constraint violations and adds this to the objective function(s)
    p = f + r_p cv. */
Real SurrBasedMinimizer::
penalty_merit(const RealVector& fn_vals, const BoolDeque& sense,
	      const RealVector& primary_wts)
{
  return objective(fn_vals, sense, primary_wts)
    + penaltyParameter * constraint_violation(fn_vals, constraintTol);
}


void SurrBasedMinimizer::
penalty_gradient(const RealVector& fn_vals, const RealMatrix& fn_grads,
		 const BoolDeque& sense, const RealVector& primary_wts,
		 RealVector& pen_grad)
{
  size_t i, j, index;

  // objective function portion
  objective_gradient(fn_vals, fn_grads, sense, primary_wts, pen_grad);

  // inequality constraint portion
  for (i=0; i<numNonlinearIneqConstraints; i++) {
    index = i + numUserPrimaryFns;
    const Real& g = fn_vals[index];
    const Real& l_bnd = origNonlinIneqLowerBnds[i];
    const Real& u_bnd = origNonlinIneqUpperBnds[i];
    const Real* grad_g = fn_grads[index];
    // Define violation as g0 > c_tol:
    //   g_l - g > c_tol  -->  grad g0 = -grad g
    //   g - g_u > c_tol  -->  grad g0 = +grad g
    if (l_bnd > -bigRealBoundSize) {
      Real g0_viol = l_bnd - g - constraintTol;
      if (g0_viol > 0.)
	for (j=0; j<numContinuousVars; j++)
	  pen_grad[j] -= 2.*penaltyParameter*g0_viol*grad_g[j];
    }
    if (u_bnd < bigRealBoundSize) {
      Real g_viol = g - u_bnd - constraintTol;
      if (g_viol > 0.)
	for (j=0; j<numContinuousVars; j++)
	  pen_grad[j] += 2.*penaltyParameter*g_viol*grad_g[j];
    }
  }

  // equality constraint portion
  for (i=0; i<numNonlinearEqConstraints; i++) {
    index = i + numUserPrimaryFns + numNonlinearIneqConstraints;
    Real h0 = fn_vals[index] - origNonlinEqTargets[i];
    const Real* grad_h = fn_grads[index];
    // Define violation as fabs(h0) > c_tol:
    //   h0 >  c_tol  -->  grad h0 = +grad h
    //   h0 < -c_tol  -->  grad h0 = +grad h
    if (h0 > constraintTol) {
      Real h_viol = h0 - constraintTol;
      for (j=0; j<numContinuousVars; j++)
	pen_grad[j] += 2.*penaltyParameter*h_viol*grad_h[j];
    }
    else if (h0 < -constraintTol) {
      Real h_viol = h0 + constraintTol;
      for (j=0; j<numContinuousVars; j++)
	pen_grad[j] += 2.*penaltyParameter*h_viol*grad_h[j];
    }
  }
}


/** Compute the quadratic constraint violation defined as cv = g+^T g+
    + h+^T h+.  This implementation supports equality constraints and
    2-sided inequalities.  The constraint_tol allows for a small
    constraint infeasibility (used for penalty methods, but not
    Lagrangian methods). */
Real SurrBasedMinimizer::
constraint_violation(const RealVector& fn_vals, const Real& constraint_tol)
{
  size_t i;
  Real constr_viol = 0.0;
  for (i=0; i<numNonlinearIneqConstraints; i++) { // ineq constraint violations
    const Real& g = fn_vals[numUserPrimaryFns+i];
    const Real& l_bnd = origNonlinIneqLowerBnds[i];
    const Real& u_bnd = origNonlinIneqUpperBnds[i];
    if (l_bnd > -bigRealBoundSize) { // g has a lower bound
      Real g0_tol = l_bnd - g - constraint_tol; // l - g <= constraint_tol
      if (g0_tol > 0.)
	constr_viol += g0_tol*g0_tol;
    }
    if (u_bnd < bigRealBoundSize) { // g has an upper bound
      Real g0_tol = g - u_bnd - constraint_tol; // g - u <= constraint_tol
      if (g0_tol > 0.)
	constr_viol += g0_tol*g0_tol;
    }
  }
  for (i=0; i<numNonlinearEqConstraints; i++) { // eq constraint violations
    Real abs_h0_tol
      = std::fabs(fn_vals[numUserPrimaryFns+numNonlinearIneqConstraints+i] -
	     origNonlinEqTargets[i]) - constraint_tol;
    if (abs_h0_tol > 0.)
      constr_viol += abs_h0_tol*abs_h0_tol;
  }
  return constr_viol;
}


/** Redefines default iterator results printing to include
    optimization results (objective functions and constraints). */
void SurrBasedMinimizer::print_results(std::ostream& s)
{
  size_t i, num_best = bestVariablesArray.size();
  if (num_best != bestResponseArray.size()) {
    Cerr << "\nError: mismatch in lengths of bestVariables and bestResponses."
         << std::endl;
    abort_handler(-1); 
  } 

  // initialize the results archive for this dataset
  archive_allocate_best(num_best);

  const String& interface_id = (methodName == SURROGATE_BASED_LOCAL ||
				methodName == SURROGATE_BASED_GLOBAL) ?
    iteratedModel.truth_model().interface_id() : iteratedModel.interface_id();
  int eval_id;
  activeSet.request_values(1);

  // -------------------------------------
  // Single and Multipoint results summary
  // -------------------------------------
  for (i=0; i<num_best; ++i) {
    // output best variables
    s << "<<<<< Best parameters          ";
    if (num_best > 1) s << "(set " << i+1 << ") ";
    s << "=\n" << bestVariablesArray[i];
    // output best response
    const RealVector& best_fns = bestResponseArray[i].function_values();
    if (optimizationFlag) {
      if (numUserPrimaryFns > 1) s << "<<<<< Best objective functions ";
      else                       s << "<<<<< Best objective function  ";
    }
    else
      s << "<<<<< Best residual terms      ";
    if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n";
    write_data_partial(s, (size_t)0, numUserPrimaryFns, best_fns);
    size_t num_cons = numFunctions - numUserPrimaryFns;
    if (num_cons) {
      s << "<<<<< Best constraint values   ";
      if (num_best > 1) s << "(set " << i+1 << ") "; s << "=\n";
      write_data_partial(s, numUserPrimaryFns, num_cons, best_fns);
    }
    // lookup evaluation id where best occurred.  This cannot be catalogued 
    // directly because the optimizers track the best iterate internally and 
    // return the best results after iteration completion.  Therfore, perform a
    // search in data_pairs to extract the evalId for the best fn eval.
    PRPCacheHIter cache_it = lookup_by_val(data_pairs, interface_id,
					   bestVariablesArray[i], activeSet);
    if (cache_it == data_pairs.get<hashed>().end())
      s << "<<<<< Best data not found in evaluation cache\n\n";
    else {
      eval_id = cache_it->eval_id();
      if (eval_id > 0)
	s << "<<<<< Best data captured at function evaluation " << eval_id
	  << "\n\n";
      else // should not occur
	s << "<<<<< Best data not found in evaluations from current execution,"
	  << "\n      but retrieved from restart archive with evaluation id "
	  << -eval_id << "\n\n";
    }

    // pass data to the results archive
    archive_best(i, bestVariablesArray[i], bestResponseArray[i]);

  }
}

} // namespace Dakota
