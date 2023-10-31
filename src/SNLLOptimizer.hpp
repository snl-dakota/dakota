/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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

/**
 * \brief A version of TraitsBase specialized for SNLL optimizers
 *
 */
class SNLLTraits: public TraitsBase
{
  public:

  /// default constructor
  SNLLTraits() { }

  /// destructor
  virtual ~SNLLTraits() { }

  /// A temporary query used in the refactor
  virtual bool is_derived() { return true; }

  // Traits are chosen to be the most common ones across a majority of methods within this TPL.

  /// Return the value of supportsContinuousVariables
  bool supports_continuous_variables() { return true; }

  /// Return the flag indicating whether method supports linear equalities
  bool supports_linear_equality() { return true; }

  /// Return the flag indicating whether method supports linear inequalities
  bool supports_linear_inequality() { return true; }

  /// Return the flag indicating whether method supports nonlinear equalities
  bool supports_nonlinear_equality() { return true; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  bool supports_nonlinear_inequality() { return true; }

  /// Return the format used for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED; }

};


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

  /// alternate constructor for objective/constraint call-backs;
  /// analytic gradient case
  SNLLOptimizer(const RealVector& initial_pt,
    const RealVector& var_l_bnds,      const RealVector& var_u_bnds,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_l_bnds,
    const RealVector& lin_ineq_u_bnds, const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_tgts,     const RealVector& nln_ineq_l_bnds,
    const RealVector& nln_ineq_u_bnds, const RealVector& nln_eq_tgts,
    void (*nlf1_obj_eval) (int mode, int n, const RealVector& x, double& f,
			   RealVector& grad_f, int& result_mode),
    void (*nlf1_con_eval) (int mode, int n, const RealVector& x, RealVector& g,
			   RealMatrix& grad_g, int& result_mode),
    size_t max_iter = 100, size_t max_eval = 1000, Real conv_tol = 1.e-4,
    Real grad_tol = 1.e-4, Real   max_step = 1000.);
  /// alternate constructor for objective/constraint call-backs;
  /// mixed gradient case: numerical objective, analytic constraints
  SNLLOptimizer(const RealVector& initial_pt,
    const RealVector& var_l_bnds,      const RealVector& var_u_bnds,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_l_bnds,
    const RealVector& lin_ineq_u_bnds, const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_tgts,     const RealVector& nln_ineq_l_bnds,
    const RealVector& nln_ineq_u_bnds, const RealVector& nln_eq_tgts,
    void (*nlf0_obj_eval) (int n, const RealVector& x, double& f,
			   int& result_mode),
    void (*nlf1_con_eval) (int mode, int n, const RealVector& x, RealVector& g,
			   RealMatrix& grad_g, int& result_mode),
    const RealVector& fdss, const String& interval_type,
    size_t max_iter = 100, size_t max_eval = 1000, Real conv_tol = 1.e-4,
    Real grad_tol = 1.e-4, Real   max_step = 1000.);
  /// alternate constructor for objective/constraint call-backs;
  /// mixed gradient case: analytic objective, numerical constraints
  SNLLOptimizer(const RealVector& initial_pt,
    const RealVector& var_l_bnds,      const RealVector& var_u_bnds,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_l_bnds,
    const RealVector& lin_ineq_u_bnds, const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_tgts,     const RealVector& nln_ineq_l_bnds,
    const RealVector& nln_ineq_u_bnds, const RealVector& nln_eq_tgts,
    void (*nlf1_obj_eval) (int mode, int n, const RealVector& x, double& f,
			   RealVector& grad_f, int& result_mode),
    void (*nlf0_con_eval) (int n, const RealVector& x, RealVector& g,
			   int& result_mode),
    const RealVector& fdss, const String& interval_type,
    size_t max_iter = 100, size_t max_eval = 1000, Real conv_tol = 1.e-4,
    Real grad_tol = 1.e-4, Real   max_step = 1000.);
  /// alternate constructor for objective/constraint call-backs;
  /// numerical gradient case
  SNLLOptimizer(const RealVector& initial_pt,
    const RealVector& var_l_bnds,      const RealVector& var_u_bnds,
    const RealMatrix& lin_ineq_coeffs, const RealVector& lin_ineq_l_bnds,
    const RealVector& lin_ineq_u_bnds, const RealMatrix& lin_eq_coeffs,
    const RealVector& lin_eq_tgts,     const RealVector& nln_ineq_l_bnds,
    const RealVector& nln_ineq_u_bnds, const RealVector& nln_eq_tgts,
    void (*nlf0_obj_eval) (int n, const RealVector& x, double& f,
			   int& result_mode),
    void (*nlf0_con_eval) (int n, const RealVector& x, RealVector& g,
			   int& result_mode),
    const RealVector& fdss, const String& interval_type,
    size_t max_iter = 100, size_t max_eval = 1000, Real conv_tol = 1.e-4,
    Real grad_tol = 1.e-4, Real   max_step = 1000.);

  ~SNLLOptimizer(); ///< destructor
    
  //
  //- Heading: Virtual member function redefinitions
  //

  /// Performs the iterations to determine the optimal solution.
  void core_run();

  void reset();

  void declare_sources();

  void initial_point(const RealVector& pt);
  void update_callback_data(const RealVector& cv_initial,
			    const RealVector& cv_lower_bnds,
			    const RealVector& cv_upper_bnds,
			    const RealMatrix& lin_ineq_coeffs,
			    const RealVector& lin_ineq_l_bnds,
			    const RealVector& lin_ineq_u_bnds,
			    const RealMatrix& lin_eq_coeffs,
			    const RealVector& lin_eq_targets,
			    const RealVector& nln_ineq_l_bnds,
			    const RealVector& nln_ineq_u_bnds,
			    const RealVector& nln_eq_targets);
  const RealMatrix& callback_linear_ineq_coefficients() const;
  const RealVector& callback_linear_ineq_lower_bounds() const;
  const RealVector& callback_linear_ineq_upper_bounds() const;

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

  /// deallocate any pointer allocations
  void deallocate();

  /// instantiate an OPTPP_Q_NEWTON solver using standard settings
  void default_instantiate_q_newton(
    void (*obj_eval) (int mode, int n, const RealVector& x, double& f,
		      RealVector& grad_f, int& result_mode) );
  /// instantiate an OPTPP_Q_NEWTON solver using standard settings
  void default_instantiate_q_newton(
    void (*obj_eval) (int n, const RealVector& x, double& f, int& result_mode));
  /// instantiate constraint objectives using standard settings
  void default_instantiate_constraint(
    void (*con_eval) (int mode, int n, const RealVector& x, RealVector& g,
		      RealMatrix& grad_g, int& result_mode) );
  /// instantiate constraint objectives using standard settings
  void default_instantiate_constraint(
    void (*con_eval) (int n, const RealVector& x, RealVector& g,
		      int& result_mode) );
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

  // cache inputs for user-functions mode for use when reallocating due to
  // size change

  /// initial point used in "user_functions" mode
  RealVector initialPoint;
  /// variable lower bounds used in "user_functions" mode
  RealVector lowerBounds;
  /// variable upper bounds used in "user_functions" mode
  RealVector upperBounds;
  /// linear inequality constraint coefficients used in "user_functions" mode
  RealMatrix linIneqCoeffs;
  /// linear inequality constraint lower bounds used in "user_functions" mode
  RealVector linIneqLowerBnds;
  /// linear inequality constraint upper bounds used in "user_functions" mode
  RealVector linIneqUpperBnds;
  /// linear equality constraint coefficients used in "user_functions" mode
  RealMatrix linEqCoeffs;
  /// linear equality constraint targets used in "user_functions" mode
  RealVector linEqTargets;
  /// nonlinear inequality constraint lower bounds used in "user_functions" mode
  RealVector nlnIneqLowerBnds;
  /// nonlinear inequality constraint upper bounds used in "user_functions" mode
  RealVector nlnIneqUpperBnds;
  /// nonlinear equality constraint targets used in "user_functions" mode
  RealVector nlnEqTargets;
  /// finite difference step sizes, either scalar or one per variable
  RealVector fdStepSize;
  /// type of finite difference interval: forward or central
  String fdIntervalType;

  /// cache zeroth-order objective call-back function
  void (*userObjective0) (int n, const RealVector& x, double& f,
			  int& result_mode);
  /// cache first-order objective call-back function
  void (*userObjective1) (int mode, int n, const RealVector& x, double& f,
			  RealVector& grad_f, int& result_mode);
  //void (*userObjective2) (int mode, int n, const RealVector& x, double& f,
  //                        RealVector& grad_f, RealSymMatrix& hess_f,
  //                        int& result_mode);
  /// cache zeroth-order constraint call-back function
  void (*userConstraint0) (int n, const RealVector& x, RealVector& g,
			   int& result_mode);
  /// cache first-order constraint call-back function
  void (*userConstraint1) (int mode, int n, const RealVector& x, RealVector& g,
			   RealMatrix& grad_g, int& result_mode);
  //void (*userConstraint2) (int mode, int n, const RealVector& x,RealVector& g,
  //                         RealMatrix& grad_g,
  //                         OPTPP::OptppArray<RealSymMatrix >& hess_g,
  //                         int& result_mode);
};


inline void SNLLOptimizer::initial_point(const RealVector& pt)
{ copy_data(pt, initialPoint); } // protect from incoming view


inline const RealMatrix& SNLLOptimizer::
callback_linear_ineq_coefficients() const
{ return linIneqCoeffs; }


inline const RealVector& SNLLOptimizer::
callback_linear_ineq_lower_bounds() const
{ return linIneqLowerBnds; }


inline const RealVector& SNLLOptimizer::
callback_linear_ineq_upper_bounds() const
{ return linIneqUpperBnds; }

} // namespace Dakota

#endif
