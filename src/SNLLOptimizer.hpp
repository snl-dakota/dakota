/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SNLLOptimizer
//- Description: Wrapper class for opt++
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id

#ifndef SNLL_OPTIMIZER_H
#define SNLL_OPTIMIZER_H

#include "DakotaOptimizer.hpp"
#include "SNLLBase.hpp"

namespace OPTPP {
class NLF0;
class NLF1;
class NLF2;
class OptPDS;
class OptCG;
class OptLBFGS;
class OptNewton;
class OptQNewton;
class OptFDNewton;
class OptBCNewton;
class OptBCQNewton;
class OptBCFDNewton;
class OptNIPS;
class OptQNIPS;
class OptFDNIPS;
}


namespace Dakota {

/// Wrapper class for the OPT++ optimization library.

/** The SNLLOptimizer class provides a wrapper for OPT++, a C++
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

class SNLLOptimizer: public Optimizer, public SNLLBase
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  SNLLOptimizer(ProblemDescDB& problem_db, Model& model);

  /// alternate constructor for instantiations "on the fly"
  SNLLOptimizer(const String& method_string, Model& model);

  /// alternate constructor for instantiations "on the fly"
  SNLLOptimizer(const RealVector& initial_pt,
    const RealVector& var_l_bnds,      const RealVector& var_u_bnds,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_l_bnds,
    const RealVector& lin_ineq_u_bnds, const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_tgts,     const RealVector& nln_ineq_l_bnds,
    const RealVector& nln_ineq_u_bnds, const RealVector& nln_eq_tgts, 
    void (*user_obj_eval) (int mode, int n, const RealVector& x, double& f,
			   RealVector& grad_f, int& result_mode),
    void (*user_con_eval) (int mode, int n, const RealVector& x, RealVector& g,
			   RealMatrix& grad_g, int& result_mode));

  ~SNLLOptimizer(); ///< destructor
    
  //
  //- Heading: Virtual member function redefinitions
  //

  /// Performs the iterations to determine the optimal solution.
  void core_run();

protected:

  //
  //- Heading: Virtual member function redefinitions
  //

  /// invokes Optimizer::initialize_run(),
  /// SNLLBase::snll_initialize_run(), and performs other set-up
  void initialize_run();

  /// performs data recovery and calls Optimizer::post_run()
  void post_run(std::ostream& s);

  /// performs cleanup, restores instances and calls parent finalize
  void finalize_run();

private:

  //
  //- Heading: Helper functions
  //

  /// instantiate an OPTPP_Q_NEWTON solver using standard settings
  void default_instantiate_q_newton(
    void (*obj_eval) (int mode, int n, const RealVector& x, double& f,
		      RealVector& grad_f, int& result_mode),
    void (*con_eval) (int mode, int n, const RealVector& x, RealVector& g,
		      RealMatrix& grad_g, int& result_mode) );
  /// instantiate an OPTPP_NEWTON solver using standard settings
  void default_instantiate_newton(
    void (*obj_eval) (int mode, int n, const RealVector& x, double& f,
		      RealVector& grad_f, RealSymMatrix& hess_f,
		      int& result_mode),
    void (*con_eval) (int mode, int n, const RealVector& x, RealVector& g,
		      RealMatrix& grad_g, 
		      OPTPP::OptppArray<RealSymMatrix >& hess_g,
		      int& result_mode) );

  //
  //- Heading: Static member functions required by opt++
  //

  //- Fn. evaluation routines which invoke Model::compute_response
  //- in order to supply f, df/dx, and d^2f/dx^2 to the OPT++ methods.

  /// objective function evaluator function for OPT++ methods which
  /// require only function values.
  static void nlf0_evaluator(int n, const RealVector& x, double& f,
			     int& result_mode);

  /// objective function evaluator function which provides function
  /// values and gradients to OPT++ methods.
  static void nlf1_evaluator(int mode, int n, const RealVector& x,
			     double& f, RealVector& grad_f, int& result_mode);

  /// objective function evaluator function which provides function
  /// values, gradients, and Hessians to OPT++ methods.
  static void nlf2_evaluator(int mode, int n, const RealVector& x,
			     double& f, RealVector& grad_f,
			     RealSymMatrix& hess_f, int& result_mode);

  //- Constraint evaluation routines that invoke Model::compute_response

  /// constraint evaluator function for OPT++ methods which require
  /// only constraint values.
  static void constraint0_evaluator(int n, const RealVector& x, 
				    RealVector& g, int& result_mode);

  /// constraint evaluator function which provides constraint
  /// values and gradients to OPT++ methods.
  static void constraint1_evaluator(int mode, int n, const RealVector& x, 
				    RealVector& g, RealMatrix& grad_g,
				    int& result_mode);

  /// constraint evaluator function which provides constraint
  /// values, gradients, and Hessians to OPT++ methods.
  static void constraint2_evaluator(int mode, int n, const RealVector& x, 
				    RealVector& g, RealMatrix& grad_g, 
				    OPTPP::OptppArray<RealSymMatrix >& hess_g,
				    int& result_mode);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static SNLLOptimizer* snllOptInstance;
  /// pointer to the previously active object instance used for
  /// restoration in the case of iterator/model recursion
  SNLLOptimizer* prevSnllOptInstance;

  //- Base class pointers to OPT++ objects
  OPTPP::NLP0   *nlfObjective;  ///< objective  NLF base class pointer
  OPTPP::NLP0   *nlfConstraint; ///< constraint NLF base class pointer
  OPTPP::NLP    *nlpConstraint; ///< constraint NLP pointer

  //- additional pointers needed for option specification
  /// pointer to objective NLF for nongradient optimizers
  OPTPP::NLF0   *nlf0;
  /// pointer to objective NLF for (analytic) gradient-based optimizers
  OPTPP::NLF1   *nlf1;
  /// pointer to constraint NLF for (analytic) gradient-based optimizers
  OPTPP::NLF1   *nlf1Con;
  /// pointer to objective NLF for (finite diff) gradient-based optimizers
  OPTPP::FDNLF1 *fdnlf1;
  /// pointer to constraint NLF for (finite diff) gradient-based optimizers
  OPTPP::FDNLF1 *fdnlf1Con;
  /// pointer to objective NLF for full Newton optimizers
  OPTPP::NLF2   *nlf2;
  /// pointer to constraint NLF for full Newton optimizers
  OPTPP::NLF2   *nlf2Con;
    
  OPTPP::OptimizeClass *theOptimizer; ///< optimizer base class pointer
  //- additional pointers needed for option specification
  OPTPP::OptPDS        *optpds;       ///< PDS optimizer pointer
  OPTPP::OptCG         *optcg;        ///< CG optimizer pointer
  OPTPP::OptLBFGS      *optlbfgs;     ///< L-BFGS optimizer pointer
  OPTPP::OptNewton     *optnewton;    ///< Newton optimizer pointer
  OPTPP::OptQNewton    *optqnewton;   ///< Quasi-Newton optimizer pointer
  OPTPP::OptFDNewton   *optfdnewton;  ///< Finite Difference Newton opt pointer
  OPTPP::OptBCNewton   *optbcnewton;  ///< Bound constrained Newton opt pointer
  OPTPP::OptBCQNewton  *optbcqnewton; ///< Bnd constrained Quasi-Newton opt ptr
  OPTPP::OptBCFDNewton *optbcfdnewton;///< Bnd constrained FD-Newton opt ptr
  OPTPP::OptNIPS       *optnips;      ///< NIPS optimizer pointer
  OPTPP::OptQNIPS      *optqnips;     ///< Quasi-Newton NIPS optimizer pointer
  OPTPP::OptFDNIPS     *optfdnips;    ///< Finite Difference NIPS opt pointer

  /// flag for iteration mode: "model" (normal usage) or "user_functions"
  /// (user-supplied functions mode for "on the fly" instantiations).
  /// NonDReliability currently uses the user_functions mode.
  String setUpType;
  /// holds initial point passed in for "user_functions" mode.
  RealVector initialPoint;
  /// holds variable lower bounds passed in for "user_functions" mode.
  RealVector lowerBounds;
  /// holds variable upper bounds passed in for "user_functions" mode.
  RealVector upperBounds;
};

} // namespace Dakota

#endif
