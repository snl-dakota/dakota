/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NPSOLOptimizer
//- Description: Wrapper class for NPSOL
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: NPSOLOptimizer.hpp 6786 2010-05-19 21:39:57Z dmgay $

#ifndef NPSOL_OPTIMIZER_H
#define NPSOL_OPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include "SOLBase.hpp"


namespace Dakota {

/// Wrapper class for the NPSOL optimization library.

/** The NPSOLOptimizer class provides a wrapper for NPSOL, a Fortran
    77 sequential quadratic programming library from Stanford
    University marketed by Stanford Business Associates. It uses a
    function pointer approach for which passed functions must be
    either global functions or static member functions.  Any attribute
    used within static member functions must be either local to that
    function or accessed through a static pointer.

    The user input mappings are as follows: \c
    max_function_evaluations is implemented directly in
    NPSOLOptimizer's evaluator functions since there is no NPSOL
    parameter equivalent, and \c max_iterations, \c
    convergence_tolerance, \c output verbosity, \c verify_level, \c
    function_precision, and \c linesearch_tolerance are mapped into
    NPSOL's "Major Iteration Limit", "Optimality Tolerance", "Major
    Print Level" (\c verbose: Major Print Level = 20; \c quiet: Major
    Print Level = 10), "Verify Level", "Function Precision", and
    "Linesearch Tolerance" parameters, respectively, using NPSOL's
    npoptn() subroutine (as wrapped by npoptn2() from the
    npoptn_wrapper.f file). Refer to [Gill, P.E., Murray, W.,
    Saunders, M.A., and Wright, M.H., 1986] for information on NPSOL's
    optional input parameters and the npoptn() subroutine. */

class NPSOLOptimizer: public Optimizer, public SOLBase
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NPSOLOptimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for Iterator instantiations by name
  NPSOLOptimizer(Model& model);

  /// alternate constructor for instantiations "on the fly"
  NPSOLOptimizer(Model& model, const int& derivative_level,
    const Real& conv_tol);

  /// alternate constructor for instantiations "on the fly"
  NPSOLOptimizer(const RealVector& initial_point,
    const RealVector& var_lower_bnds,  const RealVector& var_upper_bnds,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_lower_bnds,
    const RealVector& lin_ineq_upper_bnds, const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_targets, const RealVector& nonlin_ineq_lower_bnds,
    const RealVector& nonlin_ineq_upper_bnds,
    const RealVector& nonlin_eq_targets, 
    void (*user_obj_eval) (int&, int&, double*, double&, double*, int&),
    void (*user_con_eval) (int&, int&, int&, int&, int*, double*, double*,
			   double*, int&),
    const int& derivative_level, const Real& conv_tol);

  ~NPSOLOptimizer(); ///< destructor
    
  //
  //- Heading: Member functions
  //

  void core_run();

private:

  //
  //- Heading: Convenience member functions
  //

  /// called by core_run for setUpType == "model"
  void find_optimum_on_model();
  /// called by core_run for setUpType == "user_functions"
  void find_optimum_on_user_functions();

  //
  //- Heading: Static member functions passed by pointer to NPSOL
  //

  /// OBJFUN in NPSOL manual: computes the value and first derivatives of the
  /// objective function (passed by function pointer to NPSOL).
  static void objective_eval(int& mode, int& n, double* x, double& f,
			     double* gradf, int& nstate);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NPSOLOptimizer* npsolInstance;

  /// controls iteration mode: "model" (normal usage) or "user_functions"
  /// (user-supplied functions mode for "on the fly" instantiations).
  /// NonDReliability currently uses the user_functions mode.
  String setUpType;
  /// holds initial point passed in for "user_functions" mode.
  RealVector initialPoint;
  /// holds variable lower bounds passed in for "user_functions" mode.
  RealVector lowerBounds;
  /// holds variable upper bounds passed in for "user_functions" mode.
  RealVector upperBounds;
  /// holds function pointer for objective function evaluator passed in for
  /// "user_functions" mode.
  void (*userObjectiveEval)  (int&, int&, double*, double&, double*, int&);
  /// holds function pointer for constraint function evaluator passed in for
  /// "user_functions" mode.
  void (*userConstraintEval) (int&, int&, int&, int&, int*, double*, double*,
			      double*, int&);
};


#ifdef HAVE_DYNLIB_FACTORIES
// ---------------------------------------------------------
// Factory functions for dynamic loading of solver libraries
// ---------------------------------------------------------

NPSOLOptimizer* new_NPSOLOptimizer(ProblemDescDB& problem_db, Model& model);
NPSOLOptimizer* new_NPSOLOptimizer(Model& model);
NPSOLOptimizer* new_NPSOLOptimizer(Model& model, const int&, const Real&);
NPSOLOptimizer* new_NPSOLOptimizer(const RealVector& initial_point,
    const RealVector& var_lower_bnds,
    const RealVector& var_upper_bnds,
    const RealMatrix& lin_ineq_coeffs,
    const RealVector& lin_ineq_lower_bnds,
    const RealVector& lin_ineq_upper_bnds,
    const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_targets,
    const RealVector& nonlin_ineq_lower_bnds,
    const RealVector& nonlin_ineq_upper_bnds,
    const RealVector& nonlin_eq_targets, 
    void (*user_obj_eval) (int&, int&, double*, double&, double*, int&),
    void (*user_con_eval) (int&, int&, int&, int&, int*, double*, double*,
			   double*, int&),
    const int& derivative_level, const Real& conv_tol);

#endif // HAVE_DYNLIB_FACTORIES

} // namespace Dakota

#endif // NPSOL_OPTIMIZER_H
