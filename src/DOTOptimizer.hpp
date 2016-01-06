/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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

  /// performs run-time set up
  void initialize_run();

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
  /// a container of indices for referencing the corresponding
  /// Response constraints used in computing the DOT constraints.
  /** The length of the container corresponds to the number of DOT
      constraints, and each entry in the container points to the
      corresponding DAKOTA constraint. */
  SizetArray constraintMappingIndices;
  /// a container of multipliers for mapping the Response constraints to
  /// the DOT constraints.
  /** The length of the container corresponds to the number of DOT
      constraints, and each entry in the container stores a multiplier
      for the DAKOTA constraint identified with constraintMappingIndices.
      These multipliers are currently +1 or -1. */
  RealArray constraintMappingMultipliers;
  /// a container of offsets for mapping the Response constraints to the
  /// DOT constraints.
  /** The length of the container corresponds to the number of DOT
      constraints, and each entry in the container stores an offset for
      the DAKOTA constraint identified with constraintMappingIndices.
      These offsets involve inequality bounds or equality targets,
      since DOT assumes constraint allowables = 0. */
  RealArray constraintMappingOffsets;
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
