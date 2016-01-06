/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonlinearCGOptimizer
//- Description: Simple nonlinear conjugate gradient solver w/ line search
//- Owner:       Brian Adams
//- Checked by:  
//- Version: $Id$

#ifndef NONLINEARCGOPTIMIZER_H
#define NONLINEARCGOPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include "Teuchos_SerialDenseVector.hpp"

namespace Dakota {

/// NonlinearCG update options
enum CG_UPDATETYPE {CG_STEEPEST, CG_FLETCHER_REEVES, CG_POLAK_RIBIERE, 
		    CG_POLAK_RIBIERE_PLUS, CG_HESTENES_STIEFEL};

/// NonlinearCG linesearch options
enum CG_LINESEARCHTYPE {CG_FIXED_STEP, CG_LS_SIMPLE, CG_LS_BRENT,
			CG_LS_WOLFE};


/** Experimental implementation of nonlinear CG optimization */
class NonlinearCGOptimizer: public Optimizer
{

public:

  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  NonlinearCGOptimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor      
  ~NonlinearCGOptimizer();

  /// evaluate the objective function given a particular step size
  /// (public for use in boost_ls_eval functor; could use friend)
  Real linesearch_eval(const Real& trial_step, short req_val = 1); 

protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  void core_run();

private:

  /// constructor helper function to parse misc_options from ProblemDescDB
  void parse_options();

  /// compute next direction via choice of method
  void compute_direction();
  
  /// compute step: fixed, simple decrease, sufficient decrease
  bool compute_step();

  /// bracket the 1-D minimum in the linesearch
  void bracket_min(Real& xa, Real& xb, Real& xc,
		   Real& fa, Real& fb, Real& fc); 

  /// Perform 1-D minimization for the stepLength using Brent's method
  Real brent_minimize(Real a, Real b, Real tol);
  
  // method controls (TODO: dynamic initial step -- based on gradient)

  Real initialStep;            ///< initial step length
  Real linesearchTolerance;  ///< approximate accuracy of absissca in LS
  unsigned linesearchType;     ///< type of line search (if any)
  unsigned maxLinesearchIters; ///< maximum evaluations in line search
  Real relFunctionTol;         ///< stopping criterion for rel change in fn
  Real relGradientTol;         ///< stopping criterion for rel reduction in g
  bool resetStep;              ///< whether to reset step with each linesearch
  unsigned restartIter;        ///< iter at which to reset to steepest descent
  unsigned updateType;         ///< type of CG direction update

  // method state

  /// current iteration number
  unsigned iterCurr;
  
  /// current decision variables in the major iteration
  RealVector designVars;

  /// decision variables in the linesearch
  RealVector trialVars;

  /// current function value
  Real functionCurr;

  /// previous function value
  Real functionPrev;

  /// current gradient
  RealVector gradCurr;

  /// previous gradient
  RealVector gradPrev;

  /// temporary for gradient difference (gradCurr - gradPrev)
  RealVector gradDiff;
  
  /// current aggregate search direction
  RealVector searchDirection;

  /// current step length parameter alpha
  Real stepLength;

  /// initial gradient norm squared
  Real gradDotGrad_init;

  /// gradCurr dot gradCurr
  Real gradDotGrad_curr;
  
  /// gradPrev dot gradPrev
  Real gradDotGrad_prev;


}; // class NonlinearCGOptimizer

} // namespace Dakota

#endif // NONLINEARCGOPTIMIZER_H
