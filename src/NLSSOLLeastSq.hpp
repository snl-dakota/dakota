/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NLSSOLLeastSq
//- Description: Wrapper class for NLSSOL
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: NLSSOLLeastSq.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef NLSSOL_LEAST_SQ_H
#define NLSSOL_LEAST_SQ_H

#include "DakotaLeastSq.hpp"
#include "SOLBase.hpp"


namespace Dakota {

/// Wrapper class for the NLSSOL nonlinear least squares library.

/** The NLSSOLLeastSq class provides a wrapper for NLSSOL, a Fortran
    77 sequential quadratic programming library from Stanford
    University marketed by Stanford Business Associates. It uses a
    function pointer approach for which passed functions must be
    either global functions or static member functions.  Any nonstatic
    attribute used within static member functions must be either local
    to that function or accessed through a static pointer.

    The user input mappings are as follows: \c
    max_function_evaluations is implemented directly in
    NLSSOLLeastSq's evaluator functions since there is no NLSSOL
    parameter equivalent, and \c max_iterations, \c
    convergence_tolerance, \c output verbosity, \c verify_level, \c
    function_precision, and \c linesearch_tolerance are mapped into
    NLSSOL's "Major Iteration Limit", "Optimality Tolerance", "Major
    Print Level" (\c verbose: Major Print Level = 20; \c quiet: Major
    Print Level = 10), "Verify Level", "Function Precision", and
    "Linesearch Tolerance" parameters, respectively, using NLSSOL's
    npoptn() subroutine (as wrapped by npoptn2() from the
    npoptn_wrapper.f file). Refer to [Gill, P.E., Murray, W.,
    Saunders, M.A., and Wright, M.H., 1986] for information on NLSSOL's
    optional input parameters and the npoptn() subroutine. */

class NLSSOLLeastSq: public LeastSq, public SOLBase
{
public:

  //
  //- Heading: Constructors and destructor
  //
 
  /// standard constructor
  NLSSOLLeastSq(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor
  NLSSOLLeastSq(Model& model);
  /// destructor
  ~NLSSOLLeastSq();

  //
  //- Heading: Member functions
  //

  void core_run();

private:

  //
  //- Heading: Convenience member functions
  //

  //
  //- Heading: Static member functions passed by pointer to NLSSOL
  //

  /// Evaluator for NLSSOL: computes the values and first derivatives of the
  /// least squares terms (passed by function pointer to NLSSOL).
  static void least_sq_eval(int& mode, int& m, int& n, int& nrowfj, double* x,
			    double* f, double* gradf, int& nstate);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NLSSOLLeastSq* nlssolInstance;
};

} // namespace Dakota

#endif
