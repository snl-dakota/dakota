/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/** The ROLOptimizer class provides a wrapper for the Rapid
    Optimization Library (ROL), a Sandia-developed C++ library for
    large-scale gradient-based optimization.

    This implementation uses the refactored rol_interface components. */

#ifndef DAKOTA_ROL_OPTIMIZER_HPP
#define DAKOTA_ROL_OPTIMIZER_HPP

#ifdef HAVE_ROL

// Dakota headers
#include "DakotaOptimizer.hpp"
#include "DakotaModel.hpp"
#include "DakotaTraitsBase.hpp"

// ROL headers
#include "ROL_Problem.hpp"
#include "ROL_Solver.hpp"

// Teuchos headers
#include "Teuchos_ParameterList.hpp"

namespace Dakota {

// Forward declaration for pIMPL idiom
namespace rol_optimizer_impl {
  class ROLOptimizerImpl;
}

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
  ROLOptimizer(ProblemDescDB& problem_db,
               ParallelLibrary& parallel_lib,
               std::shared_ptr<Model> model);

  /// Alternate constructor for Iterator instantiations by name
  ROLOptimizer(const String& method_name,
               std::shared_ptr<Model> model);

  /// Destructor
  ~ROLOptimizer() override;

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Initializes the ROLOptimizer with values available after the chain of
  /// constructors has finished.
  void initialize_run() override;

  /// Iterates the ROL solver to determine the optimal solution
  void core_run() override;

  /// Finalize ROL results, bypassing generic local-recast recovery when not needed
  void post_run(std::ostream& s) override;

  /// Support resetting ROL solver options
  void reset_solver_options(const Teuchos::ParameterList&); // ROL solver settings

  /// Return the active ROL optimizer instance when ROL callbacks are executing
  static ROLOptimizer* active_instance();

  /// Record the best fully evaluated Dakota point encountered during the run
  void record_evaluated_point();

protected:

  //
  //- Heading: constructor convenience member functions
  //

  /// Helper function called during construction to extract problem
  /// information from the Model and set it for ROL
  void set_problem();

  /// Determine ROL problem type without constructing the full ROL problem
  void determine_problem_type();

  /// Convenience function to map Dakota input and power-user
  /// parameters to ROL
  void set_rol_parameters();

private:

  Real constraint_violation(const Variables& vars, const Response& resp) const;
  bool candidate_is_better(Real objective_value, Real constraint_violation) const;

  //
  //- Heading: Data (using pIMPL idiom to hide ROL types)
  //

  /// Pointer to implementation (hides ROL types from header)
  std::unique_ptr<rol_optimizer_impl::ROLOptimizerImpl> pimpl_;

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
  ~ROLTraits() override { }

  /// ROL default data type to be used by Dakota data adapters
  typedef std::vector<Real> VecT;

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Return flag indicating ROL supports continuous variables
  bool supports_continuous_variables() override { return true; }

  /// Return flag indicating ROL supports linear equalities
  bool supports_linear_equality() override { return true; }

  /// Return flag indicating ROL supports linear inequalities
  bool supports_linear_inequality() override { return true; }

  /// Return flag indicating ROL supports nonlinear equalities
  bool supports_nonlinear_equality() override { return true; }

  /// Return ROL format for nonlinear equality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format() override
   { return NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY; }

  /// Return flag indicating ROL supports nonlinear inequalities
  bool supports_nonlinear_inequality() override { return true; }

  /// Return ROL format for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format() override
   { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

}; // class ROLTraits

} // namespace Dakota

#endif // HAVE_ROL
#endif // DAKOTA_ROL_OPTIMIZER_HPP
