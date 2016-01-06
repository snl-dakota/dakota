/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NLPQLPOptimizer
//- Description: Wrapper class for NLPQLP
//- Owner:       Bengt Abel
//- Checked by:  Mike Eldred
//- Version: $Id: NLPQLPOptimizer.hpp 6972 2010-09-17 22:18:50Z briadam $

#ifndef NLPQLP_OPTIMIZER_H
#define NLPQLP_OPTIMIZER_H

#include "DakotaOptimizer.hpp"


namespace Dakota {

/// Wrapper class for the NLPQLP optimization library, Version 2.0.

/** ********************************************************************

          AN IMPLEMENTATION OF A SEQUENTIAL QUADRATIC PROGRAMMING
           METHOD FOR SOLVING NONLINEAR OPTIMIZATION PROBLEMS BY
            DISTRIBUTED COMPUTING AND NON-MONOTONE LINE SEARCH

    This subroutine solves the general nonlinear programming problem

              minimize    F(X)
              subject to  G(J,X)   =  0  ,  J=1,...,ME
                          G(J,X)  >=  0  ,  J=ME+1,...,M
                          XL  <=  X  <=  XU

    and is an extension of the code NLPQLD. NLPQLP is specifically
    tuned to run under distributed systems. A new input parameter L is
    introduced for the number of parallel computers, that is the number of
    function calls to be executed simultaneously. In case of L=1, NLPQLP
    is identical to NLPQLD. Otherwise the line search is modified to allow
    L parallel function calls in advance. Moreover the user has the
    opportunity to used distributed function calls for evaluating gradients.

    The algorithm is a modification of the method of Wilson, Han, and
    Powell. In each iteration step, a linearly constrained quadratic
    programming problem is formulated by approximating the Lagrangian
    function quadratically and by linearizing the constraints.
    Subsequently, a one-dimensional line search is performed with
    respect to an augmented Lagrangian merit function to obtain
    a new iterate. Also the modified line search algorithm guarantees
    convergence under the same assumptions as before.

    For the new version, a non-monotone line search is implemented which 
    allows to increase the merit function in case of instabilities, for 
    example caused by round-off errors, errors in gradient approximations,
    etc.

    The subroutine contains the option to predetermine initial
    guesses for the multipliers or the Hessian of the Lagrangian
    function and is called by reverse communication.
 
    ********************************************************************* */

class NLPQLPOptimizer: public Optimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NLPQLPOptimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NLPQLPOptimizer(Model& model);
  /// destructor
  ~NLPQLPOptimizer();

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

  /// L :       Number of parallel systems, i.e. function calls during
  ///           line search at predetermined iterates.
  ///           HINT: If only less than 10 parallel function evaluations
  ///                 are possible, it is recommended to apply
  ///                 the serial version by setting L=1.
  int L;

  /// numEqConstraints :      Number of equality constraints.
  int numEqConstraints;

  /// MMAX :    Row dimension of array DG containing Jacobian of constraints.
  ///           MMAX must be at least one and greater or equal to M.
  int MMAX;

  /// N :       Number of optimization variables.
  int N;

  /// NMAX :    Row dimension of C. NMAX must be at least two and greater
  ///           than N.
  int NMAX;

  /// MNN2 :    Must be equal to M+N+N+2.
  int MNN2;

  /// X(NMAX,L) : Initially, the first column of X has to contain
  ///           starting values for the optimal solution. On return, X is
  ///           replaced by the current iterate. In the driving program
  ///           the row dimension of X has to be equal to NMAX.
  ///           X is used internally to store L different arguments for which
  ///           function values should be computed simultaneously.
  double *X; 

  /// F(L) :    On return, F(1) contains the final objective function value.
  ///           F is used also to store L different objective function
  ///           values to be computed from L iterates stored in X.
  double *F;

  /// G(MMAX,L) : On return, the first column of G contains the constraint
  ///           function values at the final iterate X. In the driving program
  ///           the row dimension of G has to be equal to MMAX.
  ///           G is used internally to store L different set of constraint
  ///           function values to be computed from L iterates stored in X.
  double *G;

  /// DF(NMAX) : DF contains the current gradient of the objective function.
  ///           In case of numerical differentiation and a distributed
  ///           system (L>1), it is recommended to apply parallel evaluations
  ///           of F to compute DF.
  double *DF;

  /// DG(MMAX,NMAX) : DG contains the gradients of the active constraints
  ///           (ACTIVE(J)=.true.) at a current iterate X.
  ///           The remaining rows are filled with previously computed
  ///           gradients. In the driving program the row dimension of DG
  ///           has to be equal to MMAX.
  double *DG;

  /// U(MNN2) : U contains the multipliers with respect to the actual iterate
  ///           stored in the first column of X. The first M locations contain
  ///           the multipliers of the M nonlinear constraints, the subsequent
  ///           N locations the multipliers of the lower bounds, and the
  ///           final N locations the multipliers of the upper bounds.
  ///           At an optimal solution, all multipliers with respect to
  ///           inequality constraints should be nonnegative.
  double *U;

  /// C(NMAX,NMAX) : On return, C contains the last computed approximation
  ///           of the Hessian matrix of the Lagrangian function stored in
  ///           form of an LDL decomposition. C contains the lower triangular
  ///           factor of an LDL factorization of the final quasi-Newton matrix
  ///           (without diagonal elements, which are always one).
  ///           In the driving program, the row dimension of C has to be equal
  ///           to NMAX.
  double *C;

  /// D(NMAX) : The elements of the diagonal matrix of the LDL decomposition
  ///           of the quasi-Newton matrix are stored in the one-dimensional
  ///           array D.
  double *D;

  /// ACC :     The user has to specify the desired final accuracy
  ///           (e.g. 1.0D-7). The termination accuracy should not be smaller
  ///           than the accuracy by which gradients are computed.
  double ACC;

  /// ACCQP :   The tolerance is needed for the QP solver to perform several
  ///           tests, for example whether optimality conditions are satisfied
  ///           or whether a number is considered as zero or not. If ACCQP is
  ///           less or equal to zero, then the machine precision is computed
  ///           by NLPQLP and subsequently multiplied by 1.0D+4.
  double ACCQP;

  /// STPMIN :  Minimum steplength in case of L>1. Recommended is any value in
  ///           the order of the accuracy by which functions are computed.
  ///           The value is needed to compute a steplength reduction factor
  ///           by STPMIN**(1/L-1). If STPMIN<=0, then STPMIN=ACC is used.
  double STPMIN;

  /// MAXFUN :  The integer variable defines an upper bound for the number
  ///           of function calls during the line search (e.g. 20).
  ///           MAXFUN is only needed in case of L=1, and must not be greater
  ///           than 50.
  int MAXFUN;

  /// MAXIT :   Maximum number of outer iterations, where one iteration
  ///           corresponds to one formulation and solution of the quadratic
  ///           programming subproblem, or, alternatively, one evaluation of
  ///           gradients (e.g. 100).
  int MAXIT;

  /// MAX_NM :  Stack size for storing merit function values at previous 
  ///           iterations for non-monotone line search (e.g. 10). In case of
  ///           MAX_NM=0, monotone line search is performed.
  int MAX_NM;

  /// TOL_NM :  Relative bound for increase of merit function value, if line
  ///           search is not successful during the very first step. Must be
  ///           non-negative (e.g. 0.1).
  double TOL_NM;

  /// IPRINT :  Specification of the desired output level.
  ///    IPRINT = 0 :  No output of the program.
  ///    IPRINT = 1 :  Only a final convergence analysis is given.
  ///    IPRINT = 2 :  One line of intermediate results is printed in each
  ///                  iteration.
  ///    IPRINT = 3 :  More detailed information is printed in each iteration
  ///                  step, e.g. variable, constraint and multiplier values.
  ///    IPRINT = 4 :  In addition to 'IPRINT=3', merit function and steplength
  ///                  values are displayed during the line search.
  int IPRINT;

  /// MODE :    The parameter specifies the desired version of NLPQLP.
  ///    MODE = 0 :    Normal execution (reverse communication!).
  ///    MODE = 1 :    The user wants to provide an initial guess for the
  ///                  multipliers in U and for the Hessian of the Lagrangian
  ///                  function in C and D in form of an LDL decomposition.
  int MODE;

  /// IOUT :    Integer indicating the desired output unit number, i.e. all
  ///           write-statements start with 'WRITE(IOUT,... '.
  int IOUT;

  /// IFAIL :   The parameter shows the reason for terminating a solution
  ///           process. Initially IFAIL must be set to zero. On return IFAIL
  ///           could contain the following values:
  ///    IFAIL =-2 :   Compute gradient values w.r.t. the variables stored in
  ///                  first column of X, and store them in DF and DG. Only
  ///                  derivatives for active constraints ACTIVE(J)=.TRUE. need
  ///                  to be computed. Then call NLPQLP again, see below.
  ///    IFAIL =-1 :   Compute objective fn and all constraint values subject
  ///                  the variables found in the first L columns of X, and
  ///                  store them in F and G. Then call NLPQLP again, see below.
  ///    IFAIL = 0 :   The optimality conditions are satisfied.
  ///    IFAIL = 1 :   The algorithm has been stopped after MAXIT iterations.
  ///    IFAIL = 2 :   The algorithm computed an uphill search direction.
  ///    IFAIL = 3 :   Underflow occurred when determining a new approxi-
  ///                  mation matrix for the Hessian of the Lagrangian.
  ///    IFAIL = 4 :   The line search could not be terminated successfully.
  ///    IFAIL = 5 :   Length of a working array is too short. More
  ///                  detailed error information is obtained with
  ///                  'IPRINT>0'.
  ///    IFAIL = 6 :   There are false dimensions, for example M>MMAX,
  ///                  N>=NMAX, or MNN2<>M+N+N+2.
  ///    IFAIL = 7 :   The search direction is close to zero, but the
  ///                  current iterate is still infeasible.
  ///    IFAIL = 8 :   The starting point violates a lower or upper bound.
  ///    IFAIL = 9 :   Wrong input parameter, i.e., MODE, LDL decomposition in D
  ///                  and C (in case of MODE=1), IPRINT, IOUT
  ///    IFAIL = 10 :  Internal inconsistency of the quadratic subproblem,
  ///                  division by zero.
  ///    IFAIL > 100 : The solution of the quadratic programming
  ///                  subproblem has been terminated with
  ///                  an error message and IFAIL is set to IFQL+100,
  ///                  where IFQL denotes the index of an inconsistent 
  ///                  constraint.
  int IFAIL;

  /// WA(LWA) : WA is a real working array of length LWA.
  double *WA;
  /// LWA :     LWA value extracted from NLPQLP20.f.
  int LWA;

  /// KWA(LKWA) : The user has to provide working space for an integer array.
  int *KWA;
  /// LKWA :      LKWA should be at least N+10.
  int LKWA;

  /// ACTIVE(LACTIV) : The logical array shows a user the constraints, which
  ///                  NLPQLP considers to be active at the last computed
  ///                  iterate, i.e. G(J,X) is active, if and only if
  ///                  ACTIVE(J)=.TRUE., J=1,...,M.
  int *ACTIVE;
  /// LACTIV :         The length LACTIV of the logical array should be at
  ///                  least 2*M+10.
  int LACTIVE;
  /// LQL :            If LQL = .TRUE., the quadratic programming subproblem
  ///                  is to be solved with a full positive definite quasi-
  ///                  Newton matrix.  Otherwise, a Cholesky decomposition is
  ///                  performed and updated, so that the subproblem matrix
  ///                  contains only an upper triangular factor.
  int LQL;

  /// total number of constraints seen by NLPQL
  int numNlpqlConstr;
  /// a list of indices for referencing the DAKOTA nonlinear inequality
  /// constraints used in computing the corresponding NLPQL constraints.
  SizetList nonlinIneqConMappingIndices;
  /// a list of multipliers for mapping the DAKOTA nonlinear inequality
  /// constraints to the corresponding NLPQL constraints.
  RealList nonlinIneqConMappingMultipliers;
  /// a list of offsets for mapping the DAKOTA nonlinear inequality
  /// constraints to the corresponding NLPQL constraints.
  RealList nonlinIneqConMappingOffsets;

  /// a list of indices for referencing the DAKOTA linear inequality
  /// constraints used in computing the corresponding NLPQL constraints.
  SizetList linIneqConMappingIndices;
  /// a list of multipliers for mapping the DAKOTA linear inequality
  /// constraints to the corresponding NLPQL constraints.
  RealList linIneqConMappingMultipliers;
  /// a list of offsets for mapping the DAKOTA linear inequality
  /// constraints to the corresponding NLPQL constraints.
  RealList linIneqConMappingOffsets;
};


#ifdef HAVE_DYNLIB_FACTORIES
// ---------------------------------------------------------
// Factory functions for dynamic loading of solver libraries
// ---------------------------------------------------------

NLPQLPOptimizer* new_NLPQLPOptimizer(ProblemDescDB& problem_db, Model& model);
NLPQLPOptimizer* new_NLPQLPOptimizer(Model& model);
#endif // HAVE_DYNLIB_FACTORIES

} // namespace Dakota

#endif // NLPQLP_OPTIMIZER_H
