/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedMinimizer
//- Description: Base class for local and global surrogate-based optimization
//-              and nonlinear least squares.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SurrBasedMinimizer.H 4461 2007-08-28 17:40:08Z mseldre $

#ifndef SURR_BASED_MINIMIZER_H
#define SURR_BASED_MINIMIZER_H

#include "DakotaMinimizer.H"
#include "DakotaModel.H"

namespace Dakota {


/// Base class for local/global surrogate-based optimization/least squares.

/** These minimizers use a SurrogateModel to perform optimization based
    either on local trust region methods or global updating methods. */

class SurrBasedMinimizer: public Minimizer
{
protected:

  //
  //- Heading: Constructor and destructor
  //

  SurrBasedMinimizer(Model& model); ///< constructor
  ~SurrBasedMinimizer();            ///< destructor
    
  //
  //- Heading: Virtual function redefinitions
  //

  /// initialize graphics customized for surrogate-based iteration
  void initialize_graphics(bool graph_2d, bool tabular_data,
			   const String& tabular_file);
  void run();
  void print_results(std::ostream& s);

  //
  //- Heading: New virtual member functions
  //

  /// Used for computing the optimal solution using a surrogate-based
  /// approach.  Redefines the Iterator::run() virtual function.
  virtual void minimize_surrogates() = 0;

  //
  //- Heading: Utility member functions
  //

  /// initialize and update Lagrange multipliers for basic Lagrangian
  void update_lagrange_multipliers(const RealVector& fn_vals,
				   const RealMatrix& fn_grads);

  /// initialize and update the Lagrange multipliers for augmented Lagrangian
  void update_augmented_lagrange_multipliers(const RealVector& fn_vals);

  /// update a filter from a set of function values
  bool update_filter(const RealVector& fn_vals);

  // compute a filter merit function from a set of function values
  //Real filter_merit(const RealVector& fns_center, const RealVector& fns_star);

  /// compute a Lagrangian function from a set of function values
  Real lagrangian_merit(const RealVector& fn_vals,
			const RealVector& nln_ineq_l_bnds,
			const RealVector& nln_ineq_u_bnds,
			const RealVector& nln_eq_tgts);

  /// compute the gradient of the Lagrangian function
  void lagrangian_gradient(const RealVector& fn_vals,
			   const RealMatrix& fn_grads,
			   const RealVector& nln_ineq_l_bnds,
			   const RealVector& nln_ineq_u_bnds,
			   const RealVector& nln_eq_tgts,
			   RealVector& lag_grad);

  /// compute an augmented Lagrangian function from a set of function values
  Real augmented_lagrangian_merit(const RealVector& fn_vals,
				  const RealVector& nln_ineq_l_bnds,
				  const RealVector& nln_ineq_u_bnds,
				  const RealVector& nln_eq_tgts);

  /// compute the gradient of the augmented Lagrangian function
  void augmented_lagrangian_gradient(const RealVector& fn_vals,
				     const RealMatrix& fn_grads,
				     const RealVector& nln_ineq_l_bnds,
				     const RealVector& nln_ineq_u_bnds,
				     const RealVector& nln_eq_tgts,
				     RealVector& alag_grad);

  /// compute a penalty function from a set of function values
  Real penalty_merit(const RealVector& fn_vals);

  /// compute the gradient of the penalty function
  void penalty_gradient(const RealVector& fn_vals, const RealMatrix& fn_grads,
			RealVector& pen_grad);

  /// compute a composite objective value from one or more objective functions
  Real objective(const RealVector& fn_vals);

  /// compute the gradient of the composite objective function
  void objective_gradient(const RealVector& fn_vals, const RealMatrix& fn_grads,
			  RealVector& obj_grad);

  /// compute the constraint violation from a set of function values
  Real constraint_violation(const RealVector& fn_vals,
			    const Real& constraint_tol);

  //
  //- Heading: Data members
  //

  /// the minimizer used on the surrogate model to solve the
  /// approximate subproblem on each surrogate-based iteration
  Iterator approxSubProbMinimizer;

  /// surrogate-based minimization iteration number
  int sbIterNum;

  /// flag for use where optimization and NLS must be distinguished
  bool optimizationFlag;

  /// Set of response function vectors defining a filter (objective vs.
  /// constraint violation) for iterate selection/rejection
  // RealVectorList sbFilter;
  RealVectorArray sbFilter;
  /// Lagrange multipliers for basic Lagrangian calculations
  RealVector lagrangeMult;
  /// Lagrange multipliers for augmented Lagrangian calculations
  RealVector augLagrangeMult;
  /// the penalization factor for violated constraints used in quadratic
  /// penalty calculations; increased in update_penalty()
  Real penaltyParameter;

  /// original nonlinear inequality constraint lower bounds (no relaxation)
  RealVector origNonlinIneqLowerBnds;
  /// original nonlinear inequality constraint upper bounds (no relaxation)
  RealVector origNonlinIneqUpperBnds;
  /// original nonlinear equality constraint targets (no relaxation)
  RealVector origNonlinEqTargets;

  /// constant used in etaSequence updates
  Real eta;
  /// power for etaSequence updates when updating penalty
  Real alphaEta;
  /// power for etaSequence updates when updating multipliers
  Real betaEta;
  /// decreasing sequence of allowable constraint violation used in augmented
  /// Lagrangian updates (refer to Conn, Gould, and Toint, section 14.4)
  Real etaSequence;
};


inline void SurrBasedMinimizer::run()
{ minimize_surrogates(); }

} // namespace Dakota

#endif
