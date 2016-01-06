/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDLocalReliability
//- Description: Class for local reliability methods within DAKOTA/UQ
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_LOCAL_RELIABILITY_H
#define NOND_LOCAL_RELIABILITY_H

#include "NonDReliability.hpp"
#include "DakotaApproximation.hpp"
#include "pecos_stat_util.hpp"

#ifdef HAVE_OPTPP
#include "globals.h"
#endif

namespace Dakota {


/// Class for the reliability methods within DAKOTA/UQ

/** The NonDLocalReliability class implements the following
    reliability methods through the support of different limit state
    approximation and integration options: mean value (MVFOSM/MVSOSM),
    advanced mean value method (AMV, AMV^2) in x- or u-space, iterated
    advanced mean value method (AMV+, AMV^2+) in x- or u-space,
    two-point adaptive nonlinearity approximation (TANA) in x- or
    u-space, first order reliability method (FORM), and second order
    reliability method (SORM).  All options except mean value employ
    an optimizer (currently NPSOL SQP or OPT++ NIP) to solve an
    equality-constrained optimization problem for the most probable
    point (MPP).  The MPP search may be formulated as the reliability
    index approach (RIA) for mapping response levels to
    reliabilities/probabilities or as the performance measure approach
    (PMA) for performing the inverse mapping of reliability/probability 
    levels to response levels. */

class NonDLocalReliability: public NonDReliability
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDLocalReliability(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDLocalReliability();

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  /// performs an uncertainty propagation using analytical reliability 
  /// methods which solve constrained optimization problems to obtain
  /// approximations of the cumulative distribution function of response 
  void core_run();

  /// print the approximate mean, standard deviation, and importance factors
  /// when using the mean value method or the CDF/CCDF information when using
  /// MPP-search-based reliability methods
  void print_results(std::ostream& s);

  /// return name of active MPP optimizer
  unsigned short uses_method() const;
  /// perform an MPP optimizer method switch due to a detected conflict
  void method_recourse();

private:

  //
  //- Heading: Objective/constraint/set mappings passed to RecastModel
  //

  /// static function used as the objective function in the Reliability
  /// Index Approach (RIA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the objective function of (norm u)^2.
  static void RIA_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  /// static function used as the constraint function in the Reliability
  /// Index Approach (RIA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the constraint of G(u) = response level.
  static void RIA_constraint_eval(const Variables& sub_model_vars,
				  const Variables& recast_vars,
				  const Response& sub_model_response,
				  Response& recast_response);

  /// static function used as the objective function in the Performance
  /// Measure Approach (PMA) problem formulation.  This equality-constrained
  /// optimization problem performs the search for the most probable point
  /// (MPP) with the objective function of G(u).
  static void PMA_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  /// static function used as the constraint function in the first-order
  /// Performance Measure Approach (PMA) problem formulation.  This
  /// optimization problem performs the search for the most probable
  /// point (MPP) with the equality constraint of (norm u)^2 = (beta-bar)^2.
  static void PMA_constraint_eval(const Variables& sub_model_vars,
				  const Variables& recast_vars,
				  const Response& sub_model_response,
				  Response& recast_response);
  /// static function used as the constraint function in the second-order
  /// Performance Measure Approach (PMA) problem formulation.  This
  /// optimization problem performs the search for the most probable
  /// point (MPP) with the equality constraint of beta* = beta*-bar.
  static void PMA2_constraint_eval(const Variables& sub_model_vars,
				   const Variables& recast_vars,
				   const Response& sub_model_response,
				   Response& recast_response);

  /// static function used to augment the sub-model ASV requests for
  /// second-order PMA
  static void PMA2_set_mapping(const Variables& recast_vars,
			       const ActiveSet& recast_set,
			       ActiveSet& sub_model_set);

  //
  //- Heading: Convenience functions
  //

  /// convenience function for performing the initial limit state
  /// Taylor-series approximation
  void initial_taylor_series();

  /// convenience function for encapsulating the simple Mean Value
  /// computation of approximate statistics and importance factors
  void mean_value();

  /// convenience function for encapsulating the reliability methods that
  /// employ a search for the most probable point (AMV, AMV+, FORM, SORM)
  void mpp_search();

  /// convenience function for initializing class scope arrays
  void initialize_class_data();

  /// convenience function for initializing/warm starting MPP search
  /// data for each response function prior to level 0
  void initialize_level_data();

  /// convenience function for initializing/warm starting MPP search
  /// data for each z/p/beta level for each response function
  void initialize_mpp_search_data();

  /// convenience function for updating MPP search data for each
  /// z/p/beta level for each response function
  void update_mpp_search_data(const Variables& vars_star,
			      const Response& resp_star);

  /// convenience function for updating z/p/beta level data and final
  /// statistics following MPP convergence
  void update_level_data();

  /// update pmaMaximizeG from prescribed probabilities or prescribed
  /// generalized reliabilities by inverting second-order integrations
  void update_pma_maximize(const RealVector& mpp_u, const RealVector& fn_grad_u,
			   const RealSymMatrix& fn_hess_u);

  /// convenience function for passing the latest variables/response data
  /// to the data fit embedded within uSpaceModel
  void update_limit_state_surrogate();

  /// update mostProbPointX/U, computedRespLevel, fnGradX/U, and fnHessX/U
  /// from ranVarMeansX/U, fnValsMeanX, fnGradsMeanX, and fnHessiansMeanX
  void assign_mean_data();

  // convenience function for evaluating fnVal(u), fnGradU(u), and fnHessU(u)
  // as required by RIA_constraint_eval() and PMA_objective_eval()
  //void g_eval(int& mode, const RealVector& u);

  /// convenience function for evaluating dg/ds
  void dg_ds_eval(const RealVector& x_vars,
		  const RealVector& fn_grad_x,
		  RealVector& final_stat_grad);

  /// compute factor for derivative of second-order probability with respect to
  /// reliability index (from differentiating BREITUNG or HOHENRACK expressions)
  Real dp2_dbeta_factor(Real beta, bool cdf_flag);

  //
  //- Heading: Utility routines
  //

  /// convert norm of mpp_u (u-space solution) to a signed reliability index
  Real signed_norm(const RealVector& mpp_u, const RealVector& fn_grad_u,
		   bool cdf_flag);
  /// convert norm of u-space vector to a signed reliability index
  Real signed_norm(Real norm_mpp_u);
  /// shared helper function
  Real signed_norm(Real norm_mpp_u, const RealVector& mpp_u,
		   const RealVector& fn_grad_u, bool cdf_flag);

  /// Convert reliability to probability using a first-order integration
  Real probability(Real beta);
  /// Convert computed reliability to probability using either a
  /// first-order or second-order integration
  Real probability(bool cdf_flag, const RealVector& mpp_u,
		   const RealVector& fn_grad_u, const RealSymMatrix& fn_hess_u);
  /// Convert provided reliability to probability using either a
  /// first-order or second-order integration
  Real probability(Real beta, bool cdf_flag, const RealVector& mpp_u,
		   const RealVector& fn_grad_u, const RealSymMatrix& fn_hess_u);

  /// Convert probability to reliability using the inverse of a
  /// first-order integration
  Real reliability(Real p);
  /// Convert probability to reliability using the inverse of a
  /// first-order or second-order integration
  Real reliability(Real p, bool cdf_flag, const RealVector& mpp_u,
		   const RealVector& fn_grad_u, const RealSymMatrix& fn_hess_u);
  /// compute the residual for inversion of second-order probability
  /// corrections using Newton's method (called by reliability(p))
  bool reliability_residual(const Real& p, const Real& beta,
			    const RealVector& kappa, Real& res);
  /// compute the residual derivative for inversion of second-order
  /// probability corrections using Newton's method (called by reliability(p))
  Real reliability_residual_derivative(const Real& p, const Real& beta,
				       const RealVector& kappa);

  /// Compute the kappaU vector of principal curvatures from fnHessU
  void principal_curvatures(const RealVector& mpp_u,
			    const RealVector& fn_grad_u,
			    const RealSymMatrix& fn_hess_u,
			    RealVector& kappa_u);

  /// scale copy of principal curvatures by -1 if needed; else take a view
  void scale_curvature(Real beta, bool cdf_flag, const RealVector& kappa,
		       RealVector& scaled_kappa);

  //
  //- Heading: Data members
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDLocalReliability* nondLocRelInstance;

  // Approximation instance used for TANA-3 and Taylor series limit
  // state approximations
  //Approximation limitStateSurrogate;

  /// output response level calculated
  Real computedRespLevel;
  /// output reliability level calculated for RIA and 1st-order PMA
  Real computedRelLevel;
  /// output generalized reliability level calculated for 2nd-order PMA
  Real computedGenRelLevel;

  /// actual x-space gradient for current function from most recent response
  /// evaluation
  RealVector fnGradX;
  /// u-space gradient for current function updated from fnGradX and
  /// Jacobian dx/du
  RealVector fnGradU;
  /// actual x-space Hessian for current function from most recent response
  /// evaluation
  RealSymMatrix fnHessX;
  /// u-space Hessian for current function updated from fnHessX and
  /// Jacobian dx/du
  RealSymMatrix fnHessU;
  /// principal curvatures derived from eigenvalues of orthonormal
  /// transformation of fnHessU
  RealVector kappaU;

  /// response function values evaluated at mean x
  RealVector fnValsMeanX;
  /// response function gradients evaluated at mean x
  RealMatrix fnGradsMeanX;
  /// response function Hessians evaluated at mean x
  RealSymMatrixArray fnHessiansMeanX;
  // response function values evaluated at u=0 (for first-order integration,
  // p=0.5 -> median function values).  Used to determine the sign of beta.
  //RealVector medianFnVals;

  /// vector of means for all uncertain random variables in x-space
  RealVector ranVarMeansX;
  /// vector of std deviations for all uncertain random variables in x-space
  RealVector ranVarStdDevsX;
  /// vector of means for all uncertain random variables in u-space
  RealVector ranVarMeansU;
  /// flag indicating user specification of (any portion of) initialPtU
  bool initialPtUserSpec;
  /// user specification or default initial guess for local optimization
  RealVector initialPtUSpec;
  /// current starting point for MPP searches in u-space
  RealVector initialPtU;
  /// location of MPP in x-space
  RealVector mostProbPointX;
  /// location of MPP in u-space
  RealVector mostProbPointU;

  /// array of converged MPP's in u-space for level 0.  Used for warm-starting
  /// initialPtU within RBDO.
  RealVectorArray prevMPPULev0;
  /// matrix of limit state sensitivities w.r.t. inactive/design variables
  /// for level 0.  Used for warm-starting initialPtU within RBDO.
  RealMatrix prevFnGradDLev0;
  /// matrix of limit state sensitivities w.r.t. active/uncertain variables
  /// for level 0.  Used for warm-starting initialPtU within RBDO.
  RealMatrix prevFnGradULev0;
  /// previous design vector.  Used for warm-starting initialPtU within RBDO.
  RealVector prevICVars;
  /// accumulation (using |=) of all previous design ASV's from requested
  /// finalStatistics.  Used to detect availability of prevFnGradDLev0 data
  /// for warm-starting initialPtU within RBDO.
  ShortArray prevCumASVLev0;

  /// flag representing the optimization MPP search algorithm
  /// selection (NPSOL SQP or OPT++ NIP)
  bool npsolFlag;
  /// flag indicating the use of warm starts
  bool warmStartFlag;
  /// flag indicating the use of move overrides within OPT++ NIP
  bool nipModeOverrideFlag;
  /// flag indicating that sufficient data (i.e., fnGradU, fnHessU,
  /// mostProbPointU) is available for computing principal curvatures
  bool curvatureDataAvailable;
  /// track when kappaU requires updating via principal_curvatures()
  bool kappaUpdated;
  /// integration order (1 or 2) provided by \c integration specification
  short integrationOrder;
  /// type of second-order integration: Breitung, Hohenbichler-Rackwitz, or Hong
  short secondOrderIntType;
  /// cut-off value for 1/sqrt() term in second-order probability corrections.
  Real curvatureThresh;
  /// order of Taylor series approximations (1 or 2) in MV/AMV/AMV+
  /// derived from hessian type
  short taylorOrder;
  /// importance factors predicted by MV
  RealMatrix impFactor;
  /// derivative level for NPSOL executions (1 = analytic grads of objective
  /// fn, 2 = analytic grads of constraints, 3 = analytic grads of both).
  int npsolDerivLevel;

  /// set of warnings accumulated during execution
  unsigned short warningBits;
};


inline unsigned short NonDLocalReliability::uses_method() const
{
  if (mppSearchType) return (npsolFlag) ? NPSOL_SQP : OPTPP_Q_NEWTON;
  else               return DEFAULT_METHOD;
}


inline Real NonDLocalReliability::
signed_norm(const RealVector& mpp_u, const RealVector& fn_grad_u, bool cdf_flag)
{ return signed_norm(mpp_u.normFrobenius(), mpp_u, fn_grad_u, cdf_flag); }


inline Real NonDLocalReliability::signed_norm(Real norm_mpp_u)
{ return signed_norm(norm_mpp_u, mostProbPointU, fnGradU, cdfFlag); }


// generalized reliability --> probability
// or     FORM reliability --> probability
inline Real NonDLocalReliability::probability(Real beta)
{ return Pecos::NormalRandomVariable::std_cdf(-beta); }


inline Real NonDLocalReliability::
probability(bool cdf_flag, const RealVector& mpp_u, const RealVector& fn_grad_u,
	    const RealSymMatrix& fn_hess_u)
{
  Real beta = signed_norm(mpp_u, fn_grad_u, cdf_flag);
  return probability(beta, cdf_flag, mpp_u, fn_grad_u, fn_hess_u);
}


//    probability --> generalized reliability
// or probability --> FORM reliability
inline Real NonDLocalReliability::reliability(Real p)
{
  if (p <= 0. || p >= 1.) { // warning or error
    if (p == 0.) {      // handle numerical exception
      Cerr << "\nWarning: zero probability passed in NonDLocalReliability::"
	   << "reliability().\n"; return  Pecos::LARGE_NUMBER;// DBL_MAX;
    }
    else if (p == 1.) { // handle numerical exception
      Cerr << "\nWarning: unit probability passed in NonDLocalReliability::"
	   << "reliability().\n"; return -Pecos::LARGE_NUMBER;//-DBL_MAX;
    }
    else {              // trap error for p < 0 or p > 1
      Cerr << "\nError: invalid probability value in NonDLocalReliability::"
	   << "reliability()." << std::endl; abort_handler(-1); return  0.;
    }
  }
  else return -Pecos::NormalRandomVariable::inverse_std_cdf(p);
}


inline void NonDLocalReliability::
scale_curvature(Real beta, bool cdf_flag, const RealVector& kappa,
		RealVector& scaled_kappa)
{
  if ( (cdf_flag && beta < 0.) || (!cdf_flag && beta >= 0.) ) { // copy
    scaled_kappa = RealVector(Teuchos::Copy, kappa.values(), kappa.length());
    scaled_kappa.scale(-1.);
  }
  else                                                          // view
    scaled_kappa = RealVector(Teuchos::View, kappa.values(), kappa.length());
}

} // namespace Dakota

#endif
