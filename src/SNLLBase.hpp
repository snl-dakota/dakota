/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SNLLBase
//- Description: Wrapper class for opt++
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SNLLBase.hpp 6720 2010-04-06 05:08:17Z wjbohnh $

#ifndef SNLL_BASE_H
#define SNLL_BASE_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "globals.h"

namespace OPTPP {
class NLP;
class NLP0;
class FDNLF1;
class OptimizeClass;
template<class T> class OptppArray;
}

namespace Dakota {

class Minimizer;
class ProblemDescDB;
/// enumeration for the type of evaluator function
enum EvalType { NLFEvaluator, CONEvaluator }; // could add 0/1/2/2GN granularity


/// Base class for OPT++ optimization and least squares methods.

/** The SNLLBase class provides a common base class for SNLLOptimizer
    and SNLLLeastSq, both of which are wrappers for OPT++, a C++
    optimization library from the Computational Sciences and Mathematics
    Research (CSMR) department at Sandia's Livermore CA site. */

class SNLLBase
{
public:
  
  //
  //- Heading: Constructors and destructor
  //

  SNLLBase();                          ///< default constructor
  SNLLBase(ProblemDescDB& problem_db); ///< standard constructor
  ~SNLLBase();                         ///< destructor

protected:

  //
  //- Heading: Static member functions required by opt++
  //

  /// An initialization mechanism provided by OPT++ (not currently used).
  static void init_fn(int n, RealVector& x);

  //- Convenience routines

  /// convenience function for copying local_fn_vals to g; used by
  /// constraint evaluator functions
  void copy_con_vals_dak_to_optpp(const RealVector& local_fn_vals,
				  RealVector& g, size_t offset);

  /// convenience function for copying g to local_fn_vals; used in
  /// final solution logging
  void copy_con_vals_optpp_to_dak(const RealVector& g,
				  RealVector& local_fn_vals,
				  size_t offset);

  /// convenience function for copying local_fn_grads to grad_g;
  /// used by constraint evaluator functions
  void copy_con_grad(const RealMatrix& local_fn_grads, RealMatrix& grad_g,
		     size_t offset);

  /// convenience function for copying local_fn_hessians to hess_g;
  /// used by constraint evaluator functions
  void copy_con_hess(const RealSymMatrixArray& local_fn_hessians,
		     OPTPP::OptppArray<RealSymMatrix>& hess_g, 
                     size_t offset);

  /// convenience function for setting OPT++ options prior to the
  /// method instantiation
  void snll_pre_instantiate(bool bound_constr_flag, int num_constr);

  /// convenience function for setting OPT++ options after the
  /// method instantiation
  void snll_post_instantiate(int num_cv, bool vendor_num_grad_flag,
			     const String& finite_diff_type,
			     const RealVector& fdss, int max_iter,
			     int max_fn_evals, Real conv_tol, Real grad_tol,
			     Real max_step, bool bound_constr_flag,
			     int num_constr, short output_lev,
			     OPTPP::OptimizeClass* the_optimizer, 
			     OPTPP::NLP0* nlf_objective,
			     OPTPP::FDNLF1* fd_nlf1,OPTPP::FDNLF1* fd_nlf1_con);

  /// convenience function for OPT++ configuration prior to the
  /// method invocation
  void snll_initialize_run(OPTPP::NLP0* nlf_objective,
			   OPTPP::NLP* nlp_constraint,
			   const RealVector& init_pt, bool bound_constr_flag,
			   const RealVector& lower_bnds,
			   const RealVector& upper_bnds,
			   const RealMatrix& lin_ineq_coeffs,
			   const RealVector& lin_ineq_l_bnds,
			   const RealVector& lin_ineq_u_bnds,
			   const RealMatrix& lin_eq_coeffs,
			   const RealVector& lin_eq_targets,
			   const RealVector& nln_ineq_l_bnds,
			   const RealVector& nln_ineq_u_bnds,
			   const RealVector& nln_eq_targets);
  
  /// convenience function for setting OPT++ options after the
  /// method instantiations
  void snll_post_run(OPTPP::NLP0* nlf_objective);

  //
  //- Heading: Data
  //

  /// pointer to the active base class object instance used within the static
  /// evaluator functions in order to avoid the need for static data
  static Minimizer* optLSqInstance; // static only for consistency

  /// value_based_line_search, gradient_based_line_search,
  /// trust_region, or tr_pds
  String searchMethod;

  /// enum: LineSearch, TrustRegion, or TrustPDS
  OPTPP::SearchStrategy searchStrat;
  /// enum: NormFmu, ArgaezTapia, or VanShanno
  OPTPP::MeritFcn meritFn;

  /// value from \c max_step specification
  Real maxStep;
  /// value from \c steplength_to_boundary specification
  Real stepLenToBndry;
  /// value from \c centering_parameter specification
  Real centeringParam;

  /// flags a user selection of active_set_vector == constant.  By mapping
  /// this into mode override, reliance on duplicate detection can be avoided.
  bool constantASVFlag;

  /// flags OPT++ mode override (for combining value, gradient, and
  /// Hessian requests)
  static bool modeOverrideFlag;

  /// an enum used to track whether an nlf evaluator or a constraint
  /// evaluator was the last location of a function evaluation
  static EvalType lastFnEvalLocn;

  /// copy of mode from constraint evaluators
  static int lastEvalMode;

  /// copy of variables from constraint evaluators
  static RealVector lastEvalVars;
};


inline SNLLBase::SNLLBase():
  // leave searchMethod empty
  meritFn(OPTPP::ArgaezTapia), maxStep(1000.), stepLenToBndry(0.99995),
  centeringParam(0.2),
  // TODO: set this flag in lightweight ctors, by pulling Model's ASV status
  constantASVFlag(false)
{ }


inline SNLLBase::~SNLLBase()
{ }

} // namespace Dakota

#endif
