/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DOTOptimizer
//- Description: Wrapper class for DOT
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: DOTOptimizer.hpp 6972 2010-09-17 22:18:50Z briadam $

#ifndef DOT_OPTIMIZER_H
#define DOT_OPTIMIZER_H

#include "DakotaOptimizer.hpp"


namespace Dakota {

/// Wrapper class for the DOT optimization library.

/** The DOTOptimizer class provides a wrapper for DOT, a commercial
    Fortran 77 optimization library from Vanderplaats Research and
    Development. It uses a reverse communication mode, which avoids
    the static member function issues that arise with function pointer
    designs (see NPSOLOptimizer and SNLLOptimizer).

    The user input mappings are as follows: \c max_iterations is
    mapped into DOT's \c ITMAX parameter within its \c IPRM array, \c
    max_function_evaluations is implemented directly in the
    core_run() loop since there is no DOT parameter equivalent, \c
    convergence_tolerance is mapped into DOT's \c DELOBJ parameter
    (the relative convergence tolerance) within its \c RPRM array, \c
    output verbosity is mapped into DOT's \c IPRINT parameter within
    its function call parameter list (verbose: \c IPRINT = 7; quiet:
    \c IPRINT = 3), and \c optimization_type is mapped into DOT's \c
    MINMAX parameter within its function call parameter list. Refer to
    [Vanderplaats Research and Development, 1995] for information on
    \c IPRM, \c RPRM, and the DOT function call parameter list. */

/**
 * \brief A version of TraitsBase specialized for DOT optimizers
 *
 */

class DOTTraits: public TraitsBase
{
  public:

  /// default constructor
  DOTTraits() { }

  /// destructor
  virtual ~DOTTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  /// Return the flag indicating whether method supports continuous variables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return the format used for linear inequality constraints
  LINEAR_INEQUALITY_FORMAT linear_inequality_format()
    { return LINEAR_INEQUALITY_FORMAT::ONE_SIDED_UPPER; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::ONE_SIDED_UPPER; }

};


class DOTOptimizer: public Optimizer
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  DOTOptimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor; construct without ProblemDescDB
  DOTOptimizer(const String& method_string, Model& model);
  /// destructor
  ~DOTOptimizer();
    
  //
  //- Heading: Virtual function redefinitions
  //

  void core_run();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void initialize_run();
  void check_sub_iterator_conflict();

private:

  //
  //- Heading: Convenience functions
  //
    
  void initialize();           ///< Shared constructor code

  void allocate_workspace();   ///< Allocates workspace for the optimizer

  void allocate_constraints(); ///< Allocates constraint mappings

  //
  //- Heading: Data members
  //

  /// INFO from DOT manual
  /** Information requested by DOT: 0=optimization complete, 1=get
      values, 2=get gradients */
  int dotInfo;

  /// internal DOT parameter NGOTOZ
  /** the DOT parameter list has been modified to pass NGOTOZ, which signals
      whether DOT is finite-differencing (nonzero value) or performing the
      line search (zero value). */
  int dotFDSinfo;

  /// METHOD from DOT manual
  /** For nonlinear constraints: 0/1 = dot_mmfd, 2 = dot_slp, 3 = dot_sqp.  
      For unconstrained:         0/1 = dot_bfgs, 2 = dot_frcg. */
  int dotMethod;

  /// IPRINT from DOT manual (controls output verbosity)
  /** Values range from 0 (least output) to 7 (most output). */
  int printControl;

  /// RPRM from DOT manual
  /** Array of real control parameters. */
  RealArray realCntlParmArray;
  /// IPRM from DOT manual
  /** Array of integer control parameters. */
  IntArray  intCntlParmArray;

  /// array of design variable values passed to DOT
  RealVector designVars;

  /// value of the objective function passed to DOT
  Real objFnValue;

  /// array of nonlinear constraint values passed to DOT
  /** This array must be of nonzero length and must contain only
      one-sided inequality constraints which are <= 0 (which requires
      a transformation from 2-sided inequalities and equalities). */
  RealVector constraintValues;

  int       realWorkSpaceSize; ///< size of realWorkSpace
  int       intWorkSpaceSize;  ///< size of intWorkSpace
  RealArray realWorkSpace;     ///< real work space for DOT
  IntArray  intWorkSpace;      ///< int work space for DOT

  /// total number of nonlinear constraints seen by DOT
  int numDotNlnConstr;
  /// total number of linear constraints seen by DOT
  int numDotLinConstr;
  /// total number of linear and nonlinear constraints seen by DOT
  int numDotConstr;
};


#ifdef HAVE_DYNLIB_FACTORIES
// ---------------------------------------------------------
// Factory functions for dynamic loading of solver libraries
// ---------------------------------------------------------

DOTOptimizer* new_DOTOptimizer(ProblemDescDB& problem_db, Model& model);
DOTOptimizer* new_DOTOptimizer(Model& model);
#endif // HAVE_DYNLIB_FACTORIES

} // namespace Dakota

#endif // DOT_OPTIMIZER_H
