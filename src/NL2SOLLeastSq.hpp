/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        NL2SOLLeastSq
//- Description:  Wrapper class for NL2SOL
//- Owner:        David Gay
//- Version: $Id: NL2SOLLeastSq.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef NL2SOL_LEAST_SQ_H
#define NL2SOL_LEAST_SQ_H

#include "DakotaLeastSq.hpp"


namespace Dakota {

typedef void (*Vf)();


/// Wrapper class for the NL2SOL nonlinear least squares library.

/** The NL2SOLLeastSq class provides a wrapper for NL2SOL (TOMS Algorithm 573),
    in the updated form of Port Library routines dn[fg][b ] from Bell Labs;
    see http://www.netlib.org/port/readme.
    The Fortran from Port has been turned into C by f2c.
    NL2SOL uses a function pointer approach for which passed functions
    must be either global functions or static member functions. */

class NL2SOLLeastSq: public LeastSq
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  NL2SOLLeastSq(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NL2SOLLeastSq(Model& model);
  /// destructor
  ~NL2SOLLeastSq();

  //
  //- Heading: Member functions
  //

  void core_run();

private:

  //
  //- Heading: Static member functions passed by pointer to NL2SOL
  //

  /// evaluator function for residual vector
  static void calcr(int *np, int *pp, Real *x, int *nfp, Real *r, int *ui,
		    void *ur, Vf vf);

  /// evaluator function for residual Jacobian
  static void calcj(int *np, int *pp, Real *x, int *nfp, Real *J, int *ui,
		    void *ur, Vf vf);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static
  /// evaluator functions
  static NL2SOLLeastSq* nl2solInstance;

  // For more details on the following data, see "Usage Summary for Selected
  // Optimization Routines" by David M. Gay, Computing Science Technical Report
  // No. 153, AT&T Bell Laboratories, 1990.
  // http://netlib.bell-labs.com/cm/cs/cstr/153.ps.gz

  // These NL2SOL options are mapped from other existing DAKOTA inputs:

  int  auxprt; ///< auxilary printing bits (see Dakota Ref Manual): sum of
	       ///<	1  = x0prt  (print initial guess)
	       ///<	2  = solprt (print final solution)
	       ///<	4  = statpr (print solution statistics)
	       ///<	8  = parprt (print nondefault parameters)
	       ///<	16 = dradpr (print bound constraint drops/adds)
               ///< debug/verbose/normal use default = 31 (everything),
               ///< quiet uses 3, silent uses 0.
  int  outlev; ///< frequency of output summary lines in number of iterations
               ///< (debug/verbose/normal/quiet use default = 1, silent uses 0)

  Real dltfdj; ///< finite-diff step size for computing Jacobian approximation
               ///< (\c fd_gradient_step_size)
  Real delta0; ///< finite-diff step size for gradient differences for H
               ///< (a component of some covariance approximations, if desired)
               ///< (\c fd_hessian_step_size)
  Real dltfdc; ///< finite-diff step size for function differences for H
               ///< (\c fd_hessian_step_size)

  int  mxfcal; ///< function-evaluation limit (\c max_function_evaluations)
  int  mxiter; ///< iteration limit (\c max_iterations)

  Real rfctol; ///< relative fn convergence tolerance (\c convergence_tolerance)

  // These options are defined from the NL2SOL input specification:

  Real afctol; ///< absolute fn convergence tolerance (\c absolute_conv_tol)
  Real xctol;  ///< x-convergence tolerance (\c x_conv_tol)
  Real sctol;  ///< singular convergence tolerance (\c singular_conv_tol)
  Real lmaxs;  ///< radius for singular-convergence test (\c singular_radius)
  Real xftol;  ///< false-convergence tolerance (\c false_conv_tol)

  int  covreq; ///< kind of covariance required (\c covariance):
	       ///<	1 or -1 ==> sigma^2 H^-1 J^T J H^-1
	       ///<	2 or -2 ==> sigma^2 H^-1
	       ///<	3 or -3 ==> sigma^2 (J^T J)^-1
	       ///<	1 or  2 ==> use gradient diffs to estimate H
	       ///<    -1 or -2 ==> use function diffs to estimate H
	       ///< default = 0     (no covariance)
  int  rdreq;  ///< whether to compute the regression diagnostic vector
               ///< (\c regression_diagnostics)

  Real fprec;  ///< expected response function precision (\c function_precision)

  Real lmax0;  ///< initial trust-region radius (\c initial_trust_radius)
};

} // namespace Dakota

#endif
