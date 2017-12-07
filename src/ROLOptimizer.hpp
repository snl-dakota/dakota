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
#include "ROL_OptimizationSolver.hpp"

#include "DakotaTraitsBase.hpp"

namespace Dakota {

  typedef double RealT;

/** ROLTraits specializes some traits accessors by over-riding the default 
accessors in TraitsBase. */

class ROLTraits: public TraitsBase
{
  public:

  /// default constructor
  ROLTraits();

  /// destructor
  virtual ~ROLTraits();

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  ///// Return the flag indicating whether method supports discrete variables
  //bool supports_discrete_variables() { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() { return true; }

  ///// Return the flag indicating whether method supports linear inequalities
  //bool supports_linear_inequality() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  ///// Return the format used for nonlinear equality constraints
  //NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format()
  //  { return NONLINEAR_EQUALITY_FORMAT::TPL_MANAGED; }

  ///// Return the flag indicating whether method supports nonlinear inequalities
  //bool supports_nonlinear_inequality() { return true; }

  ///// Return the format used for nonlinear inequality constraints
  //NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
  //  { return NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_LOWER; }

};

inline ROLTraits::ROLTraits()
{ }

inline ROLTraits::~ROLTraits()
{ }


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

  /// define the ROL objectives, constraints, and optimization problem
  void initialize_run();

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

  // BMA: commented out as is a base class member
  /// Shallow copy of the model on which ROL will iterate.
  // Model iteratedModel;

  /// Parameters for the ROL::OptimizationSolver
  Teuchos::ParameterList optSolverParams;

  /// Handle to ROL::OptimizationProblem, part of ROL's simplified interface 
  ROL::OptimizationProblem<RealT> problem;

  /// Handle to ROL's solution vector 
  Teuchos::RCP<std::vector<RealT> > x_rcp;
};

} // namespace Dakota

#endif
