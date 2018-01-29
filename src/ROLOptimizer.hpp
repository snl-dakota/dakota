/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Implementation of the ROLOptimizer class, Only able to
//               call line search optimization for now (i.e. ROLOptimizer
//               is single-method TPL for this iterations)
//- Owner:       Moe Khalil
//- Checked by:
//- Version: $Id$

#ifndef ROL_OPTIMIZER_H
#define ROL_OPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include "DakotaModel.hpp"
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_OptimizationSolver.hpp"

#include "DakotaTraitsBase.hpp"

namespace Dakota {

  enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};

/** ROLTraits specializes some traits accessors by over-riding the default 
accessors in TraitsBase. */

// PDH: Now that we understand how ROL works, we should do a sanity check
// to make sure these still make sense.  We should also revisit the Traits
// base class to see if there's anything that needs to be changed or added.

class ROLTraits: public TraitsBase
{
  public:

  /// default constructor
  ROLTraits() { }

  typedef std::vector<Real> VecT;

  /// destructor
  virtual ~ROLTraits() { }

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

  /// Return the format used for nonlinear equality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format()
   { return NONLINEAR_EQUALITY_FORMAT::TPL_MANAGED; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
   { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

};


// --------------------------------------------------------------

class DakotaROLIneqConstraints : public ROL::StdConstraint<Real>
{

  public:

    /// Constructor
    DakotaROLIneqConstraints(Model & model);

    /// Callback to return the constraint value to ROL
    void value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol) override;

    /// Callback to return the result of applying the constraint gradient on an arbitrary vector to ROL
    void applyJacobian(std::vector<Real> &jv,
        const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) override;

    /// Callback to return the result of applying the constraint adjoint on an arbitrary vector to ROL
    void applyAdjointJacobian(std::vector<Real> &ajv,
        const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) override;

  private:

    Model & dakotaModel;

}; // class DakotaROLIneqConstraints


// --------------------------------------------------------------

class DakotaROLEqConstraints : public ROL::StdConstraint<Real>
{

  public:

    /// Constructor
    DakotaROLEqConstraints(Model & model);

    /// Callback to return the constaint value to ROL
    void value(std::vector<Real> &c, const std::vector<Real> &x, Real &tol) override;

    /// Callback to return the result of applying the constraint gradient on an arbitrary vector to ROL
    void applyJacobian(std::vector<Real> &jv,
        const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) override;

    /// Callback to return the result of applying the constraint adjoint on an arbitrary vector to ROL
    void applyAdjointJacobian(std::vector<Real> &ajv,
        const std::vector<Real> &v, const std::vector<Real> &x, Real &tol) override;

  private:

    Model & dakotaModel;

}; // class DakotaROLEqConstraints


// --------------------------------------------------------------

class DakotaROLObjective : public ROL::StdObjective<Real>
{

  public:

    /// Constructor
    DakotaROLObjective(Model & model);

    /// Callback to return the objective value (Response) to ROL
    Real value(const std::vector<Real> &x, Real &tol) override;

    /// Callback to return the response gradient to ROL
    void gradient( std::vector<Real> &g, const std::vector<Real> &x, Real &tol ) override;

  private:

    Model & dakotaModel;

}; // class DakotaROLObjective

// --------------------------------------------------------------


class ROLOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  ROLOptimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for Iterator instantiations by name
  ROLOptimizer(const String& method_name, Model& model);
  
  /// destructor
  ~ROLOptimizer() {}

  //
  //- Heading: Virtual member function redefinitions
  //

  /// iterates the ROL solver to determine the optimal solution
  void core_run();

protected:

  //
  //- Heading: constructor convenience member functions
  //

  /// Convenience function to map Dakota input and power-user parameters to ROL
  void set_rol_parameters();

  /// Helper function called after default construction to extract problem
  /// information from the Model and set it for ROL.
  void set_problem();

  //
  //- Heading: Data
  //

  /// Parameters for the ROL::OptimizationSolver
  Teuchos::ParameterList optSolverParams;

  /// Handle to ROL::OptimizationProblem, part of ROL's simplified interface 
  ROL::OptimizationProblem<Real> optProblem;

  /// Handle to ROL's solution vector 
  Teuchos::RCP<std::vector<Real> > rolX;
};

} // namespace Dakota

#endif
