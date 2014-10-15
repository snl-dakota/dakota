/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       SurrBasedLocalMinimizer
//- Description: A local surrogate-based algorithm which successively invokes
//-              a minimizer on an approximate model within a trust region.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: SurrBasedLocalMinimizer.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef SURR_BASED_LOCAL_MINIMIZER_H
#define SURR_BASED_LOCAL_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"
#include "DakotaModel.hpp"

namespace Dakota {


/// Class for provably-convergent local surrogate-based optimization
/// and nonlinear least squares.

/** This minimizer uses a SurrogateModel to perform minimization based
    on local, global, or hierarchical surrogates. It achieves provable
    convergence through the use of a sequence of trust regions and the
    application of surrogate corrections at the trust region centers. */

class SurrBasedLocalMinimizer: public SurrBasedMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  SurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~SurrBasedLocalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  /// reset convergence controls in case of multiple SBLM executions
  void reset();

private:

  //
  //- Heading: Convenience member functions
  //

  /// Performs local surrogate-based minimization by minimizing local,
  /// global, or hierarchical surrogates over a series of trust regions.
  void minimize_surrogates();

  /// compute current trust region bounds
  bool tr_bounds(const RealVector& global_lower_bnds,
		 const RealVector& global_upper_bnds,
		 RealVector& tr_lower_bnds, RealVector& tr_upper_bnds);

  /// retrieve responseCenterTruth if possible, evaluate it if not
  void find_center_truth(const Iterator& dace_iterator, Model& truth_model);

  /// retrieve responseCenter_approx if possible, evaluate it if not
  void find_center_approx();

  /// check for hard convergence (norm of projected gradient of
  /// merit function near zero)
  void hard_convergence_check(const Response& response_truth,
			      const RealVector& c_vars,
			      const RealVector& lower_bnds,
			      const RealVector& upper_bnds);

  /// compute trust region ratio (for SBLM iterate acceptance and trust
  /// region resizing) and check for soft convergence (diminishing returns)
  void tr_ratio_check(const RealVector& c_vars_star,
		      const RealVector& tr_lower_bounds,
		      const RealVector& tr_upper_bounds);

  /// initialize and update the penaltyParameter
  void update_penalty(const RealVector& fns_center_truth,
		      const RealVector& fns_star_truth);

  /// static function used to define the approximate subproblem objective.
  static void approx_subprob_objective_eval(const Variables& surrogate_vars,
					    const Variables& recast_vars,
					    const Response& surrogate_response,
					    Response& recast_response);

  /// static function used to define the approximate subproblem constraints.
  static void approx_subprob_constraint_eval(const Variables& surrogate_vars,
					     const Variables& recast_vars,
					     const Response& surrogate_response,
					     Response& recast_response);

  /// relax constraints by updating bounds when current iterate is infeasible
  void relax_constraints(const RealVector& lower_bnds, 
			 const RealVector& upper_bnds);

  /// static function used by NPSOL as the objective function in the
  /// homotopy constraint relaxation formulation.
  static void hom_objective_eval(int& mode, int& n, double* tau_and_x,
				 double& f, double* grad_f, int&);

  /// static function used by NPSOL as the constraint function in the
  /// homotopy constraint relaxation formulation.
  static void hom_constraint_eval(int& mode, int& ncnln, int& n, int& nrowj,
				  int* needc, double* tau_and_x, double* c,
				  double* cjac, int& nstate);

  //
  //- Heading: Data members
  //

  // the +/- offsets for each of the variables in the current trust region
  //RealVector trustRegionOffset;
  /// original user specification for trustRegionFactor
  Real origTrustRegionFactor;
  /// the trust region factor is used to compute the total size of the trust
  /// region -- it is a percentage, e.g. for trustRegionFactor = 0.1, the
  /// actual size of the trust region will be 10% of the global bounds (upper
  /// bound - lower bound for each design variable).
  Real trustRegionFactor;
  /// a soft convergence control: stop SBLM when the trust region
  /// factor is reduced below the value of minTrustRegionFactor
  Real minTrustRegionFactor;
  /// trust region ratio min value: contract tr if ratio below this value
  Real trRatioContractValue;
  /// trust region ratio sufficient value: expand tr if ratio above this value
  Real trRatioExpandValue;
  /// trust region contraction factor
  Real gammaContract;
  /// trust region expansion factor
  Real gammaExpand;

  /// type of approximate subproblem objective: ORIGINAL_OBJ, LAGRANGIAN_OBJ,
  /// or AUGMENTED_LAGRANGIAN_OBJ
  short approxSubProbObj;
  /// type of approximate subproblem constraints: NO_CON, LINEARIZED_CON, or
  /// ORIGINAL_CON
  short approxSubProbCon;
  /// the approximate sub-problem formulation solved on each approximate
  /// minimization cycle: may be a shallow copy of iteratedModel, or may
  /// involve a RecastModel recursion applied to iteratedModel
  Model approxSubProbModel;
  /// flag to indicate when approxSubProbModel involves a RecastModel recursion
  bool recastSubProb;
  /// type of trust region constraint relaxation for infeasible starting
  /// points: NO_RELAX or HOMOTOPY
  short trConstraintRelax;
  /// type of merit function used in trust region ratio logic: PENALTY_MERIT,
  /// ADAPTIVE_PENALTY_MERIT, LAGRANGIAN_MERIT, or AUGMENTED_LAGRANGIAN_MERIT
  short meritFnType;
  /// type of iterate acceptance test logic: FILTER or TR_RATIO
  short acceptLogic;

  /// iteration offset used to update the scaling of the penalty parameter
  /// for adaptive_penalty merit functions
  int penaltyIterOffset;

  /// code indicating satisfaction of hard or soft convergence conditions
  short convergenceFlag;
  /// number of consecutive candidate point rejections.  If the
  /// count reaches softConvLimit, stop SBLM.
  unsigned short softConvCount;
  /// the limit on consecutive candidate point rejections.  If
  /// exceeded by softConvCount, stop SBLM.
  unsigned short softConvLimit;

  /// flags the use/availability of truth gradients within the SBLM process
  bool truthGradientFlag;
  /// flags the use/availability of surrogate gradients within the SBLM process
  bool approxGradientFlag;
  /// flags the use/availability of truth Hessians within the SBLM process
  bool truthHessianFlag;
  /// flags the use/availability of surrogate Hessians within the SBLM process
  bool approxHessianFlag;
  /// flags the use of surrogate correction techniques at the center
  /// of each trust region
  short correctionType;
  /// flags the use of a global data fit surrogate (rsm, ann, mars, kriging)
  bool globalApproxFlag;
  /// flags the use of a multipoint data fit surrogate (TANA)
  bool multiptApproxFlag;
  /// flags the use of a local data fit surrogate (Taylor series)
  bool localApproxFlag;
  /// flags the use of a model hierarchy/multifidelity surrogate 
  bool hierarchApproxFlag;
  /// flags the acceptance of a candidate point and the existence of
  /// a new trust region center
  bool newCenterFlag;
  /// flags the availability of the center point in the DACE
  /// evaluations for global approximations (CCD, Box-Behnken)
  bool daceCenterPtFlag;
  /// flags the simultaneous presence of two conditions: (1) additional
  /// layerings w/i actual_model (e.g., surrogateModel = layered/nested/layered
  /// -> actual_model = nested/layered), and (2) a user-specification to bypass
  /// all layerings within actual_model for the evaluation of truth data
  /// (responseCenterTruth and responseStarTruth).
  bool multiLayerBypassFlag;
  /// flag for the "use_derivatives" specification for which derivatives
  /// are to be evaluated at each DACE point in global surrogate builds.
  bool useDerivsFlag;

  // Data needed for computing merit functions
  /// individual violations of nonlinear inequality constraint lower bounds
  RealVector nonlinIneqLowerBndsSlack;
  /// individual violations of nonlinear inequality constraint upper bounds
  RealVector nonlinIneqUpperBndsSlack;
  /// individual violations of nonlinear equality constraint targets
  RealVector nonlinEqTargetsSlack;
  /// constraint relaxation parameter
  Real tau;
  /// constraint relaxation parameter backoff parameter (multiplier)
  Real alpha;

  /// pointer to SBLM instance used in static member functions
  static SurrBasedLocalMinimizer* sblmInstance;

  Variables varsCenter;          ///< variables at the trust region center

  Response responseCenterApprox; ///< approx response at trust region center
  Response responseStarApprox;   ///< approx response at SBLM cycle minimum

  IntResponsePair responseCenterTruth;///< truth response at trust region center
  IntResponsePair responseStarTruth;  ///< truth response at SBLM cycle minimum
};

} // namespace Dakota

#endif
