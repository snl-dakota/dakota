/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
    sol_optn_wrapper.f file). Refer to [Gill, P.E., Murray, W.,
    Saunders, M.A., and Wright, M.H., 1986] for information on NPSOL's
    optional input parameters and the npoptn() subroutine. */

/**
 * \brief A version of TraitsBase specialized for NPSOL optimizers
 *
 */

class NPSOLTraits: public TraitsBase
{
  public:

  /// default constructor
  NPSOLTraits() { }

  /// destructor
  virtual ~NPSOLTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

};


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
  NPSOLOptimizer(Model& model, int derivative_level, Real conv_tol);

  /// alternate constructor for instantiations "on the fly"
  NPSOLOptimizer(const RealVector& cv_initial,
		 const RealVector& cv_lower_bnds,
		 const RealVector& cv_upper_bnds,
		 const RealMatrix& lin_ineq_coeffs,
		 const RealVector& lin_ineq_lower_bnds,
		 const RealVector& lin_ineq_upper_bnds,
		 const RealMatrix& lin_eq_coeffs,
		 const RealVector& lin_eq_targets,
		 const RealVector& nonlin_ineq_lower_bnds,
		 const RealVector& nonlin_ineq_upper_bnds,
		 const RealVector& nonlin_eq_targets,
		 void (*user_obj_eval) (int&, int&, double*, double&,
					double*, int&),
		 void (*user_con_eval) (int&, int&, int&, int&, int*,
					double*, double*, double*, int&),
		 int derivative_level, Real conv_tol = 0., size_t max_iter = 0,
		 Real fdss = 0., Real fn_precision = 0., Real feas_tol = 0.,
		 Real lin_feas_tol = 0., Real nonlin_feas_tol = 0.);

  ~NPSOLOptimizer(); ///< destructor
    
  //
  //- Heading: Virtual function redefinitions
  //

  //void pre_run();
  void core_run();

  void declare_sources();

  void check_sub_iterator_conflict();

  // updaters for user-functions mode:

  void initial_point(const RealVector& pt);
  void update_callback_data(const RealVector& cv_initial,
			    const RealVector& cv_lower_bnds,
			    const RealVector& cv_upper_bnds,
			    const RealMatrix& lin_ineq_coeffs,
			    const RealVector& lin_ineq_l_bnds,
			    const RealVector& lin_ineq_u_bnds,
			    const RealMatrix& lin_eq_coeffs,
			    const RealVector& lin_eq_targets,
			    const RealVector& nln_ineq_l_bnds,
			    const RealVector& nln_ineq_u_bnds,
			    const RealVector& nln_eq_targets);
  const RealMatrix& callback_linear_ineq_coefficients() const;
  //const RealVector& callback_linear_ineq_lower_bounds() const;
  //const RealVector& callback_linear_ineq_upper_bounds() const;

protected:

  //
  //- Heading: Member functions
  //

  void send_sol_option(std::string sol_option) override;

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
  /// cache the linear inequality constraint coefficients in
  /// update_callback_data() for use in find_optimum_on_user_functions()
  RealMatrix linIneqCoeffs;
  /// cache the linear equality constraint coefficients in
  /// update_callback_data() for use in find_optimum_on_user_functions()
  RealMatrix linEqCoeffs;
  /// holds function pointer for objective function evaluator passed in for
  /// "user_functions" mode.
  void (*userObjectiveEval)  (int&, int&, double*, double&, double*, int&);
  /// holds function pointer for constraint function evaluator passed in for
  /// "user_functions" mode.
  void (*userConstraintEval) (int&, int&, int&, int&, int*, double*, double*,
			      double*, int&);
};


inline void NPSOLOptimizer::initial_point(const RealVector& pt)
{ copy_data(pt, initialPoint); } // protect from incoming view


inline const RealMatrix& NPSOLOptimizer::
callback_linear_ineq_coefficients() const
{ return linIneqCoeffs; }


/*
inline const RealVector& NPSOLOptimizer::
callback_linear_ineq_lower_bounds() const
{
  return RealVector(Teuchos::View, &lowerBounds[numContinuousVars],
		    numLinearIneqConstraints);
}


inline const RealVector& NPSOLOptimizer::
callback_linear_ineq_upper_bounds() const
{
  return RealVector(Teuchos::View, &upperBounds[numContinuousVars],
		    numLinearIneqConstraints);
}
*/


#ifdef HAVE_DYNLIB_FACTORIES
// ---------------------------------------------------------
// Factory functions for dynamic loading of solver libraries
// ---------------------------------------------------------

NPSOLOptimizer* new_NPSOLOptimizer(ProblemDescDB& problem_db, Model& model);
NPSOLOptimizer* new_NPSOLOptimizer(Model& model);
NPSOLOptimizer* new_NPSOLOptimizer(Model& model, int, Real);
NPSOLOptimizer* new_NPSOLOptimizer(const RealVector& cv_initial,
    const RealVector& cv_lower_bnds,
    const RealVector& cv_upper_bnds,
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
    int derivative_level, Real conv_tol);

#endif // HAVE_DYNLIB_FACTORIES

} // namespace Dakota

#endif // NPSOL_OPTIMIZER_H
