/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       CONMINOptimizer
//- Description: Wrapper class for CONMIN
//- Owner:       Tony Giunta
//- Checked by:
//- Version: $Id: CONMINOptimizer.hpp 6972 2010-09-17 22:18:50Z briadam $

#ifndef CONMIN_OPTIMIZER_H
#define CONMIN_OPTIMIZER_H

#include "DakotaOptimizer.hpp"


namespace Dakota {

/// Wrapper class for the CONMIN optimization library.

/** The CONMINOptimizer class provides a wrapper for CONMIN, a
    Public-domain Fortran 77 optimization library written by Gary
    Vanderplaats under contract to NASA Ames Research Center. The
    CONMIN User's Manual is contained in NASA Technical Memorandum
    X-62282, 1978.  CONMIN uses a reverse communication mode, which
    avoids the static member function issues that arise with function
    pointer designs (see NPSOLOptimizer and SNLLOptimizer).

    The user input mappings are as follows: \c max_iterations is
    mapped into CONMIN's \c ITMAX parameter, \c
    max_function_evaluations is implemented directly in the
    core_run() loop since there is no CONMIN parameter equivalent,
    \c convergence_tolerance is mapped into CONMIN's \c DELFUN and \c
    DABFUN parameters, \c output verbosity is mapped into CONMIN's \c
    IPRINT parameter (verbose: \c IPRINT = 4; quiet: \c IPRINT = 2),
    gradient mode is mapped into CONMIN's \c NFDG parameter, and
    finite difference step size is mapped into CONMIN's \c FDCH and \c
    FDCHM parameters.  Refer to [Vanderplaats, 1978] for additional
    information on CONMIN parameters. */

class CONMINOptimizer: public Optimizer
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  CONMINOptimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor; construct without ProblemDescDB
  CONMINOptimizer(const String& method_string, Model& model);
  /// destructor
  ~CONMINOptimizer();

  //
  //- Heading: Virtual member function redefinitions
  //

  void core_run();

protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  /// performs run-time set up
  void initialize_run();

private:

  //
  //- Heading: Convenience member functions
  //
    
  void initialize();           ///< Shared constructor code

  void allocate_workspace();   ///< Allocates workspace for the optimizer

  void deallocate_workspace(); ///< Releases workspace memory

  void allocate_constraints(); ///< Allocates constraint mappings

  //
  //- Heading: Data members
  //

  /// INFO from CONMIN manual
  /** Information requested by CONMIN: 
      1 = evaluate objective and constraints,
      2 = evaluate gradients of objective and constraints. */
  int conminInfo;

  /// IPRINT from CONMIN manual (controls output verbosity)
  /** Values range from 0 (nothing) to 4 (most output).
      0 = nothing,
      1 = initial and final function information,
      2 = all of #1 plus function value and design vars at each iteration,
      3 = all of #2 plus constraint values and direction vectors,
      4 = all of #3 plus gradients of the objective function and constraints,
      5 = all of #4 plus proposed design vector, plus objective and 
          constraint functions from the 1-D search */
  int printControl;

  /// value of the objective function passed to CONMIN
  Real objFnValue;

  /// array of nonlinear constraint values passed to CONMIN
  /** This array must be of nonzero length and must contain only
      one-sided inequality constraints which are <= 0 (which requires
      a transformation from 2-sided inequalities and equalities). */
  RealVector constraintValues;

  /// total number of nonlinear constraints seen by CONMIN
  int numConminNlnConstr;
  /// total number of linear constraints seen by CONMIN
  int numConminLinConstr;
  /// total number of linear and nonlinear constraints seen by CONMIN
  int numConminConstr;
  /// a container of indices for referencing the corresponding
  /// Response constraints used in computing the CONMIN constraints.
  /** The length of the container corresponds to the number of CONMIN
      constraints, and each entry in the container points to the
      corresponding DAKOTA constraint. */
  SizetArray constraintMappingIndices;
  /// a container of multipliers for mapping the Response constraints to
  /// the CONMIN constraints.
  /** The length of the container corresponds to the number of CONMIN
      constraints, and each entry in the container stores a multiplier
      for the DAKOTA constraint identified with constraintMappingIndices.
      These multipliers are currently +1 or -1. */
  RealArray constraintMappingMultipliers;
  /// a container of offsets for mapping the Response constraints to the
  /// CONMIN constraints.
  /** The length of the container corresponds to the number of CONMIN
      constraints, and each entry in the container stores an offset
      for the DAKOTA constraint identified with
      constraintMappingIndices.  These offsets involve inequality
      bounds or equality targets, since CONMIN assumes constraint
      allowables = 0. */
  RealArray constraintMappingOffsets;

  // These are variables and arrays that must be declared here prior
  // to calling the F77 CONMIN code.
  //
  /// Size variable for CONMIN arrays. See CONMIN manual.
  /** N1 = number of variables + 2 */
  int N1;
  /// Size variable for CONMIN arrays. See CONMIN manual.
  /** N2 = number of constraints + 2*(number of variables) */
  int N2;
  /// Size variable for CONMIN arrays. See CONMIN manual.
  /** N3 = Maximum possible number of active constraints.*/
  int N3;
  /// Size variable for CONMIN arrays. See CONMIN manual.
  /** N4 = Maximum(N3,number of variables) */
  int N4;
  /// Size variable for CONMIN arrays. See CONMIN manual.
  /** N5 = 2*(N4) */
  int N5;
  //
  // user-controlled variables that affect CONMIN performance
  //
  /// Finite difference flag.
  int NFDG;
  /// Flag to control amount of output data.
  int IPRINT;  
  /// Flag to specify the maximum number of iterations.
  int ITMAX;  
  /// Relative finite difference step size.
  double FDCH; 
  /// Absolute finite difference step size.
  double FDCHM;
  /// Constraint thickness parameter 
  /** The value of CT decreases in magnitude during optimization.*/
  double CT;
  /// Minimum absolute value of CT used during optimization.
  double CTMIN;
  /// Constraint thickness parameter for linear and side constraints.
  double CTL;
  /// Minimum value of CTL used during optimization.
  double CTLMIN;
  /// Relative convergence criterion threshold.
  /*** Threshold for the minimum relative change in the objective function. */
  double DELFUN; 
  /// Absolute convergence criterion threshold.
  /*** Threshold for the minimum relative change in the objective function. */
  double DABFUN;

  /// Array of design variables used by CONMIN (length N1 = numdv+2)
  double *conminDesVars;
  /// Array of lower bounds used by CONMIN (length N1 = numdv+2)
  double *conminLowerBnds;
  /// Array of upper bounds used by CONMIN (length N1 = numdv+2)
  double *conminUpperBnds;

  ///Internal CONMIN array.
  /** Move direction in N-dimensional space.*/
  double *S;
  ///Internal CONMIN array.
  /** Temporary storage of constraint values.*/
  double *G1;
  ///Internal CONMIN array.
  /** Temporary storage of constraint values.*/
  double *G2;
  ///Internal CONMIN array.
  /** Temporary storage for computations involving array S.*/
  double *B;
  ///Internal CONMIN array.
  /** Temporary storage for use with arrays B and S.*/
  double *C;
  ///Internal CONMIN array.
  /** Temporary storage for use with arrays B and S.*/
  int *MS1;
  ///Internal CONMIN array.
  /** Vector of scaling parameters for design parameter values.*/
  double *SCAL;
  ///Internal CONMIN array.
  /** Temporary storage for analytic gradient data.*/
  double *DF;
  ///Internal CONMIN array.
  /** Temporary 2-D array for storage of constraint gradients.*/
  double *A;
  ///Internal CONMIN array.
  /** Array of flags to identify linear constraints. (not used in this 
      implementation of CONMIN) */
  int *ISC;
  ///Internal CONMIN array.
  /** Array of flags to identify active and violated constraints */
  int *IC;
};

} // namespace Dakota

#endif
