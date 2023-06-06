/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Wrapper class for ROL
//- Owner:       Moe Khalil
//- Checked by:  Patty Hough, Russell Hooper, Brian Adams
//- Version: $Id$

/** The ROLOptimizer class provides a wrapper for the Rapid
    Optimization Library (ROL), a Sandia-developed C++ library for
    large-scale gradient-based optimization. */

#ifndef ROL_OPTIMIZER_H
#define ROL_OPTIMIZER_H

// Dakota headers
#include "DakotaOptimizer.hpp"
#include "DakotaModel.hpp"
#include "DakotaTraitsBase.hpp"

// ROL headers
#include "ROL_StdObjective.hpp"
#include "ROL_StdConstraint.hpp"
#include "ROL_OptimizationSolver.hpp"

// Semi-standard headers
#include <boost/iostreams/filter/line.hpp>

namespace Dakota {

  // Global enums for use by all classes in this file
  // Evaluation type: 1=function, 2=gradient, 4=Hessian
  // Problem type: 1=unconstrained, 2=bound constrained,
  //               3=equality constrained,
  //               4=equality and bound constrained

  enum {AS_FUNC=1, AS_GRAD=2, AS_HESS=4};
  enum {TYPE_U=1, TYPE_B=2, TYPE_E=3, TYPE_EB=4};


// -----------------------------------------------------------------
/** ROLOptimizer specializes DakotaOptimizer to construct and run a
    ROL solver appropriate for the type of problem specified by the
    user. */

class ROLOptimizer : public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// Standard constructor
  ROLOptimizer(ProblemDescDB& problem_db, Model& model);

  /// Alternate constructor for Iterator instantiations by name
  ROLOptimizer(const String& method_name, Model& model);
  
  /// Destructor
  ~ROLOptimizer() {}

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Initializes the ROLOptimizer with values available after the chain of
  /// constructors has finished.
  void initialize_run() override;

  /// Iterates the ROL solver to determine the optimal solution
  void core_run() override;

  /// Support resetting ROL solver options
  void reset_solver_options(const Teuchos::ParameterList &); // ROL solver settings

  /// Accessor for the underlying ROL Problem
  ROL::OptimizationProblem<Real> & get_rol_problem()
  { return optProblem; }

protected:

  //
  //- Heading: constructor convenience member functions
  //

  /// Helper function called during construction to extract problem
  /// information from the Model and set it for ROL
  void set_problem();

  /// Convenience function to map Dakota input and power-user
  /// parameters to ROL
  void set_rol_parameters();

  //
  //- Heading: Data
  //

  /// Parameters for the ROL::OptimizationSolver
  Teuchos::ParameterList optSolverParams;

  /// ROL problem type
  unsigned short problemType;

  /// Handle to ROL's solution vector 
  Teuchos::RCP<std::vector<Real> > rolX;

  /// Handle to ROL's lower bounds vector 
  Teuchos::RCP<ROL::StdVector<Real> > lowerBounds;

  /// Handle to ROL's upper bounds vector 
  Teuchos::RCP<ROL::StdVector<Real> > upperBounds;

  /// Handle to ROL::OptimizationProblem, part of ROL's simplified
  /// interface 
  ROL::OptimizationProblem<Real> optProblem;

}; // class ROLOptimizer


// -----------------------------------------------------------------
/** ROLTraits defines the types of problems and data formats ROL
    supports by overriding the default traits accessors in
    TraitsBase. */

class ROLTraits: public TraitsBase
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Default constructor
  ROLTraits() { }

  /// Destructor
  virtual ~ROLTraits() { }

  /// ROL default data type to be used by Dakota data adapters
  typedef std::vector<Real> VecT;

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Return flag indicating ROL supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return flag indicating ROL supports linear equalities
  bool supports_linear_equality() { return true; }

  /// Return flag indicating ROL supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return flag indicating ROL supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return ROL format for nonlinear equality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format()
   { return NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY; }

  /// Return flag indicating ROL supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return ROL format for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
   { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

}; // class ROLTraits


// -----------------------------------------------------------------
/** DakotaROLObjective is derived from the ROL objective class.
    It overrides the member functions to provide Dakota-specific
    implementations of function evaluations. */ 

class DakotaROLObjective : public ROL::StdObjective<Real>
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLObjective(Model & model);

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the objective value (response) to ROL
  Real value(const std::vector<Real> &x,
	     Real &tol) override;

  //
  //- Heading: Data
  //

  /// Dakota problem data provided by user
  Model & dakotaModel;

private:

}; // class DakotaROLObjective


// -----------------------------------------------------------------
/** DakotaROLObjectiveGrad is derived from DakotaROLObjective.  It
    implements overrides of ROL member functions to provide a
    Dakota-specific Gradient support for the objective function. This
    separate class is needed to allow for the option of utilizing
    ROL's finite-differenced gradients */ 

class DakotaROLObjectiveGrad : public DakotaROLObjective
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLObjectiveGrad(Model & model);

  /// Destructor
  virtual ~DakotaROLObjectiveGrad() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the response gradient to ROL
  void gradient(std::vector<Real> &g,
    const std::vector<Real> &x,
    Real &tol) override;

private:

}; // class DakotaROLObjectiveGrad


// -----------------------------------------------------------------
/** DakotaROLObjectiveHess is derived from DakotaROLObjectiveGrad. It
    implements overrides of ROL member functions to provide a
    Dakota-specific implementation of a Hessian-vector product.  This
    separate class is needed (rather than putting the product into
    DakotaROLObjective) because logic in ROL does not always protect
    against calling the Hessian-vector product in cases where there
    is not actually a Hessian provided. */ 

class DakotaROLObjectiveHess : public DakotaROLObjectiveGrad
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLObjectiveHess(Model & model);

  /// Destructor
  virtual ~DakotaROLObjectiveHess() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return Hessian-vector product needed by ROL when
  /// using user/Dakota-supplied Hessians
  void hessVec(std::vector<Real> &hv,
	       const std::vector<Real> &v,
	       const std::vector<Real> &x,
	       Real &tol) override;

  /// This function is not used by ROL algorithms currently supported
  /// by Dakota but is included to protect against unexpected behavior
  void invHessVec(std::vector<Real> &hv,
		  const std::vector<Real> &v,
		  const std::vector<Real> &x,
		  Real &tol) override;

private:

}; // class DakotaROLObjectiveHess


// -----------------------------------------------------------------
/** DakotaROLIneqConstraints is derived from the ROL constraint
    class.  It overrides the member functions to provide
    Dakota-specific implementations of inequality constraint
    evaluation. */

class DakotaROLIneqConstraints : public ROL::StdConstraint<Real>
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLIneqConstraints(Model & model);

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the constraint value to ROL
  void value(std::vector<Real> &c,
       const std::vector<Real> &x,
       Real &tol) override;

protected:

  //
  //- Heading: Data
  //

  /// Dakota problem data provided by user
  Model & dakotaModel;

  /// Whether or not problem has nonlinear inequality constraints
  bool haveNlnConst;

}; // class DakotaROLIneqConstraints


// -----------------------------------------------------------------
/** DakotaROLIneqConstraintsGrad is derived from
    DakotaROLIneqConstraints.  It implements overrides of ROL member
    functions to provide a Dakota-specific application of the
    inequality constraint Jacobian to a vector.  This
    separate class is needed to allow for the option of utilizing
    ROL's finite-differenced gradients */

class DakotaROLIneqConstraintsGrad : public DakotaROLIneqConstraints
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLIneqConstraintsGrad(Model & model);

  /// Destructor
  virtual ~DakotaROLIneqConstraintsGrad() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the result of applying the constraint
  /// gradient on an arbitrary vector to ROL
  void applyJacobian(std::vector<Real> &jv,
         const std::vector<Real> &v,
         const std::vector<Real> &x,
         Real &tol) override;

  /// Function to return the result of applying the constraint adjoint
  /// to an arbitrary vector to ROL
  void applyAdjointJacobian(std::vector<Real> &ajv,
          const std::vector<Real> &v,
          const std::vector<Real> &x,
          Real &tol) override;

private:

}; // class DakotaROLIneqConstraintsGrad


// -----------------------------------------------------------------
/** DakotaROLIneqConstraintsHess is derived from
    DakotaROLIneqConstraintsGrad.  It implements overrides of ROL member
    functions to provide a Dakota-specific implementation of a adjoint
    Hessian-vector product for inequality constraints.  This separate
    class is needed (rather than putting the product into
    DakotaROLIneqConstraints) because logic in ROL does not always
    protect against calling the adjoint Hessian-vector product in
    cases where there is not actually a Hessian provided. */

class DakotaROLIneqConstraintsHess : public DakotaROLIneqConstraintsGrad
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLIneqConstraintsHess(Model & model);

  /// Destructor
  virtual ~DakotaROLIneqConstraintsHess() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the result of applying the constraint adjoint
  /// Hessian to an arbitrary vector to ROL
  void applyAdjointHessian( std::vector<Real>       & ahuv,
                            const std::vector<Real> & u, 
                            const std::vector<Real> & v,
                            const std::vector<Real> & x,
                            Real &tol ) override;

private:

}; // class DakotaROLIneqConstraintsHess


// -----------------------------------------------------------------
/** DakotaROLEqConstraints is derived from the ROL constraint class.
    It overrides the member functions to provide Dakota-specific
    implementations of equality constraint evaluation and the
    application of the equality constraint Jacobian to a vector. */ 

class DakotaROLEqConstraints : public ROL::StdConstraint<Real>
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLEqConstraints(Model & model);

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the constaint value to ROL
  void value(std::vector<Real> &c,
	     const std::vector<Real> &x,
	     Real &tol) override;

protected:

  //
  //- Heading: Data
  //

  /// Dakota problem data provided by user
  Model & dakotaModel;

  /// Whether or not problem has nonlinear equality constraints
  bool haveNlnConst;

}; // class DakotaROLEqConstraints


// -----------------------------------------------------------------
/** DakotaROLEqConstraintsGrad is derived from
    DakotaROLEqConstraints.  It implements overrides of ROL member
    functions to provide a Dakota-specific application of the
    inequality constraint Jacobian to a vector.  This
    separate class is needed to allow for the option of utilizing
    ROL's finite-differenced gradients */

class DakotaROLEqConstraintsGrad : public DakotaROLEqConstraints
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLEqConstraintsGrad(Model & model);

  /// Destructor
  virtual ~DakotaROLEqConstraintsGrad() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the result of applying the constraint
  /// gradient to an arbitrary vector to ROL
  void applyJacobian(std::vector<Real> &jv,
         const std::vector<Real> &v,
         const std::vector<Real> &x,
         Real &tol) override;

  /// Function to return the result of applying the constraint adjoint
  /// to an arbitrary vector to ROL
  void applyAdjointJacobian(std::vector<Real> &ajv,
          const std::vector<Real> &v,
          const std::vector<Real> &x,
          Real &tol) override;

private:

  //
  //- Heading: Data
  //

}; // class DakotaROLIneqConstraintsGrad


// -----------------------------------------------------------------
/** DakotaROLEqConstraintsHess is derived from
    DakotaROLEqConstraintsGrad.  It implements overrides of ROL member
    functions to provide a Dakota-specific implementation of a adjoint
    Hessian-vector product for equality constraints.  This separate
    class is needed (rather than putting the product into
    DakotaROLEqConstraints) because logic in ROL does not always
    protect against calling the adjoint Hessian-vector product in
    cases where there is not actually a Hessian provided. */

class DakotaROLEqConstraintsHess : public DakotaROLEqConstraintsGrad
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  DakotaROLEqConstraintsHess(Model & model);

  /// Destructor
  virtual ~DakotaROLEqConstraintsHess() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the result of applying the constraint adjoint
  /// Hessian to an arbitrary vector to ROL
  void applyAdjointHessian( std::vector<Real>       & ahuv,
                            const std::vector<Real> & u, 
                            const std::vector<Real> & v,
                            const std::vector<Real> & x,
                            Real &tol ) override;

private:

  //
  //- Heading: Data
  //

}; // class DakotaROLIneqConstraintsHess


// -----------------------------------------------------------------
/** PrefixingLineFilter is dervied from a Boost stream filter class in
    order to preface output with specified text.  In this case, the
    intent is to distinguish ROL output. */ 

class PrefixingLineFilter : public boost::iostreams::line_filter
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Constructor
  explicit PrefixingLineFilter(const std::string& prefix_in):
    linePrefix(prefix_in)
  { /* empty ctor */ }

private:

  //
  //- Heading: Virtual member function redefinitions
  //

  /// "Filter" the line by adding the prefix
  std::string do_filter(const std::string& line)
  { return linePrefix + line; }

  //
  //- Heading: Data
  //

  /// Prefix for each line
  std::string linePrefix;

}; // class PrefixingLineFilter

} // namespace Dakota

#endif
