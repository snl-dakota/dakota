/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SNLLLeastSq
//- Description: Wrapper class for opt++
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id

#ifndef SNLL_LEAST_SQ_H
#define SNLL_LEAST_SQ_H

#include "DakotaLeastSq.hpp"
#include "SNLLBase.hpp"

namespace OPTPP {
class NLF1;
class NLF2;
class OptNewton;
class OptBCNewton;
class OptDHNIPS;
}


namespace Dakota {

/// Wrapper class for the OPT++ optimization library.

/** The SNLLLeastSq class provides a wrapper for OPT++, a C++
    optimization library of nonlinear programming and pattern search
    techniques from the Computational Sciences and Mathematics
    Research (CSMR) department at Sandia's Livermore CA site.  It uses
    a function pointer approach for which passed functions must be
    either global functions or static member functions.  Any attribute
    used within static member functions must be either local to that
    function, a static member, or accessed by static pointer.

    The user input mappings are as follows: \c max_iterations, \c
    max_function_evaluations, \c convergence_tolerance, \c max_step,
    \c gradient_tolerance, \c search_method, and \c search_scheme_size
    are set using OPT++'s setMaxIter(), setMaxFeval(), setFcnTol(),
    setMaxStep(), setGradTol(), setSearchStrategy(), and setSSS()
    member functions, respectively; \c output verbosity is used to
    toggle OPT++'s debug mode using the setDebug() member function.
    Internal to OPT++, there are 3 search strategies, while the DAKOTA
    \c search_method specification supports 4 (\c
    value_based_line_search, \c gradient_based_line_search, \c
    trust_region, or \c tr_pds).  The difference stems from the
    "is_expensive" flag in OPT++.  If the search strategy is
    LineSearch and "is_expensive" is turned on, then the \c
    value_based_line_search is used.  Otherwise (the "is_expensive"
    default is off), the algorithm will use the \c
    gradient_based_line_search.  Refer to [Meza, J.C., 1994] and to
    the OPT++ source in the Dakota/packages/OPTPP directory for
    information on OPT++ class member functions. */

class SNLLLeastSq: public LeastSq, public SNLLBase
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  SNLLLeastSq(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for instantiations without ProblemDescDB support
  SNLLLeastSq(const String& method_name, Model& model);
  /// destructor
  ~SNLLLeastSq();
    
  //
  //- Heading: Virtual member function redefinitions
  //

  /// compute the least squares solution
  void core_run();

protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  /// invokes LeastSq::initialize_run(),
  /// SNLLBase::snll_initialize_run(), and performs other set-up
  void initialize_run();
  /// invokes snll_post_run and re-implements post_run (does not call parent)
  /// and performs other solution processing
  void post_run(std::ostream& s);
  /// restores instances 
  void finalize_run();

private:

  //
  //- Heading: Static member functions required by opt++
  //

  //- Fn. evaluation routines which invoke Model::compute_response
  //- in order to supply f, df/dx, and d^2f/dx^2 to the OPT++ methods.

  /// objective function evaluator function which obtains values and
  /// gradients for least square terms and computes objective function
  /// value, gradient, and Hessian using the Gauss-Newton approximation.
  static void nlf2_evaluator_gn(int mode, int n, const RealVector& x, double& f,
				RealVector& grad_f, RealSymMatrix& hess_f,
				int& result_mode);

  //- Constraint evaluation routines that invoke Model::compute_response

  /// constraint evaluator function which provides constraint
  /// values and gradients to OPT++ Gauss-Newton methods.
  /** While it does not employ the Gauss-Newton approximation, it is distinct
      from constraint1_evaluator() due to its need to anticipate the required
      modes for the least squares terms.  This constraint evaluator function
      is used with diaggregated Hessian NIPS and is currently active. */
  static void constraint1_evaluator_gn(int mode, int n, const RealVector& x,
				       RealVector& g, RealMatrix& grad_g,
				       int& result_mode);

  /// constraint evaluator function which provides constraint
  /// values, gradients, and Hessians to OPT++ Gauss-Newton methods.
  /** While it does not employ the Gauss-Newton approximation, it is distinct
      from constraint2_evaluator() due to its need to anticipate the required
      modes for the least squares terms.  This constraint evaluator function
      is used with full Newton NIPS and is currently inactive. */
  static void constraint2_evaluator_gn(int mode, int n, const RealVector& x,
				       RealVector& g, RealMatrix& grad_g,
				       OPTPP::OptppArray<RealSymMatrix>& hess_g,
				       int& result_mode);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static SNLLLeastSq* snllLSqInstance;
  /// pointer to the previously active object instance used for
  /// restoration in the case of iterator/model recursion
  SNLLLeastSq* prevSnllLSqInstance;

  // Base class pointers to OPT++ objects
  OPTPP::NLP0 *nlfObjective;  ///< objective  NLF base class pointer
  OPTPP::NLP0 *nlfConstraint; ///< constraint NLF base class pointer
  OPTPP::NLP  *nlpConstraint; ///< constraint NLP pointer

  // additional pointers needed for option specification
  /// pointer to objective NLF for full Newton optimizers
  OPTPP::NLF2 *nlf2;
  /// pointer to constraint NLF for full Newton optimizers
  OPTPP::NLF2 *nlf2Con;
  /// pointer to constraint NLF for Quasi Newton optimizers
  OPTPP::NLF1 *nlf1Con;  

  OPTPP::OptimizeClass *theOptimizer; ///< optimizer base class pointer
  // additional pointers needed for option specification
  OPTPP::OptNewton   *optnewton;   ///< Newton optimizer pointer
  OPTPP::OptBCNewton *optbcnewton; ///< Bound constrained Newton optimizer ptr
  OPTPP::OptDHNIPS   *optdhnips;   ///< Disaggregated Hessian NIPS optimizer ptr
};

} // namespace Dakota

#endif
