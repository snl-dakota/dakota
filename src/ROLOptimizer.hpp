/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ROLOptimizer
//- Description: Wrapper class for ROL
//- Owner:       Moe Khalil
//- Checked by:
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

  // CLEAN-UP: Problem type is applicable to other TPLs.  May want to
  // figure out how to generalize and elevate it, much like traits,
  // for broader use.  Maybe evaluation type too.

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

  /// Iterates the ROL solver to determine the optimal solution
  void core_run();

  /// Support re-entrant behavior by allowing problem reset (selected
  /// components for now)
  void reset_problem(const RealVector &,              // initial vals
                     const RealVector &,              // lower bounds
                     const RealVector &,              // upper bounds
                     const Teuchos::ParameterList &); // ROL solver parameters/options

protected:

  //
  //- Heading: constructor convenience member functions
  //

  /// Convenience function to map Dakota input and power-user
  /// parameters to ROL
  void set_rol_parameters();

  /// Helper function called during construction to extract problem
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

  /// Handle to ROL's lower bounds vector 
  Teuchos::RCP<ROL::StdVector<Real> > lowerBounds;

  /// Handle to ROL's upper bounds vector 
  Teuchos::RCP<ROL::StdVector<Real> > upperBounds;

  /// ROL problem type
  unsigned short problemType;

}; // class ROLOptimizer

// -----------------------------------------------------------------
/** ROLTraits defines the types of problems and data formats ROL
    supports by overriding the default traits accessors in
    TraitsBase. */

// CLEAN-UP: Now that we understand how ROL works, we should do a
// sanity check to make sure these still make sense.  We should also
// revisit the Traits base class to see if there's anything that needs
// to be changed or added.

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

  // CHECK-ME
  /// ROL default data type to be used by Dakota data adapters
  typedef std::vector<Real> VecT;

  //
  //- Heading: Virtual member function redefinitions
  //

  // CLEAN-UP: Is this still needed?  Is it used for unit testing?
  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

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
   { return NONLINEAR_EQUALITY_FORMAT::TPL_MANAGED; }

  /// Return flag indicating ROL supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return ROL format for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
   { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

}; // class ROLTraits

// TODO: Add Hessian support.  Per Brian...
// However, ROL behavior will change based on what virtual functions
// are implemented, e.g., if we re-implement the Hessian functions,
// ROL will always call them. This would be a problem if the Dakota
// user hasn.t provided a Hessian specification.  I think we.ll want
// something like the following:
// 1. Specialize each of the ROL evaluators {DakotaROLObjective,
//    DakotaROLIneqConstraints, DakotaROLEqConstraints}, e.g, adding
//    new specializations DakotaROLSecondOrderObjective, or
//    DakotaROLObjectiveHessian something
// 2. In those *SecondOrder* specializations, implement the necessary
//    hessVec, inverseHessVec, applyHessian, etc., virtual functions.
// 3. When a Dakota user has Hessians, instantiate the *SecondOrder*
//    classes; otherwise instantiate the current evaluator classes,
//    and pass them to the ROL Problem.

// -----------------------------------------------------------------
/** DakotaROLObjective is derived from the ROL objective class.
    It overrides the member functions to provide Dakota-specific
    implementations of function and gradient evaluations. */ 

class DakotaROLObjective : public ROL::StdObjective<Real>
{
public:

  //
  //- Heading: Constructor and destructor
  //

  // QUESTION: Do we need a destructor?
  /// Constructor
  DakotaROLObjective(Model & model);

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the objective value (response) to ROL
  Real value(const std::vector<Real> &x,
	     Real &tol) override;

  /// Function to return the response gradient to ROL
  void gradient(std::vector<Real> &g,
		const std::vector<Real> &x,
		Real &tol) override;

  //
  //- Heading: Data
  //

  // CLEAN-UP: Would be nice to ultimately get rid of this via
  // adapters.  May take a little thought and work, so defer for now.

  /// Dakota problem data provided by user
  Model & dakotaModel;

private:

}; // class DakotaROLObjective


  // HESSIAN TODO: finish populating this class if anything else is
  // needed
  // second-order
class DakotaROLObjectiveHess : public DakotaROLObjective
{
public:

  //
  //- Heading: Constructor and destructor
  //

  // QUESTION: Do we need a destructor?
  /// Constructor
  DakotaROLObjectiveHess(Model & model);

  // May want to get rid of inverse information
  /// Destructor
  virtual ~DakotaROLObjectiveHess() { }

  //
  //- Heading: Virtual member function redefinitions
  //

  void hessVec(std::vector<Real> &hv,
	       const std::vector<Real> &v,
	       const std::vector<Real> &x,
	       Real &tol) override;

  /// This callback is not used by ROL algorithms currently supported by Dakota
  void invHessVec(std::vector<Real> &hv,
		  const std::vector<Real> &v,
		  const std::vector<Real> &x,
		  Real &tol) override;

private:

  //
  //- Heading: Data
  //

}; // class DakotaROLObjectiveHess

// -----------------------------------------------------------------
/** DakotaROLIneqConstraints is derived from the ROL constraint
    class.  It overrides the member functions to provide
    Dakota-specific implementations of inequality constraint
    evaluation and the application of the inequality constraint
    Jacobian to a vector. */

class DakotaROLIneqConstraints : public ROL::StdConstraint<Real>
{
public:

  //
  //- Heading: Constructor and destructor
  //

  // QUESTION: Do we need a destructor?
  /// Constructor
  DakotaROLIneqConstraints(Model & model);

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the constraint value to ROL
  void value(std::vector<Real> &c,
	     const std::vector<Real> &x,
	     Real &tol) override;

  /// Function to return the result of applying the constraint
  /// gradient on an arbitrary vector to ROL
  void applyJacobian(std::vector<Real> &jv,
		     const std::vector<Real> &v,
		     const std::vector<Real> &x,
		     Real &tol) override;

  /// Function to return the result of applying the constraint adjoint
  /// on an arbitrary vector to ROL
  void applyAdjointJacobian(std::vector<Real> &ajv,
			    const std::vector<Real> &v,
			    const std::vector<Real> &x,
			    Real &tol) override;

private:

  //
  //- Heading: Data
  //

  // CLEAN-UP: Would be nice to ultimately get rid of these via
  // adapters.  May take a little thought and work, so defer for now.

  /// Dakota problem data provided by user
  Model & dakotaModel;

  /// Whether or not problem has nonlinear inequality constraints
  bool haveNlnConst;

}; // class DakotaROLIneqConstraints

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

  // QUESTION: Do we need a destructor?
  /// Constructor
  DakotaROLEqConstraints(Model & model);

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Function to return the constaint value to ROL
  void value(std::vector<Real> &c,
	     const std::vector<Real> &x,
	     Real &tol) override;

  /// Function to return the result of applying the constraint
  /// gradient on an arbitrary vector to ROL
  void applyJacobian(std::vector<Real> &jv,
		     const std::vector<Real> &v,
		     const std::vector<Real> &x,
		     Real &tol) override;

  /// Function to return the result of applying the constraint adjoint
  /// on an arbitrary vector to ROL
  void applyAdjointJacobian(std::vector<Real> &ajv,
			    const std::vector<Real> &v,
			    const std::vector<Real> &x,
			    Real &tol) override;

private:

  //
  //- Heading: Data
  //

  // CLEAN-UP: Would be nice to ultimately get rid of these via
  // adapters.  May take a little thought and work, so defer for now.

  /// Dakota problem data provided by user
  Model & dakotaModel;

  /// Whether or not problem has nonlinear equality constraints
  bool haveNlnConst;

}; // class DakotaROLEqConstraints

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

  // QUESTION: Do we need a destructor?
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
