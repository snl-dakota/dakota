/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataFitSurrBasedLocalMinimizer
//- Description: A local surrogate-based algorithm which successively invokes
//-              a minimizer on an approximate model within a trust region.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: DataFitSurrBasedLocalMinimizer.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef DATA_FIT_SURR_BASED_LOCAL_MINIMIZER_H
#define DATA_FIT_SURR_BASED_LOCAL_MINIMIZER_H

#include "SurrBasedLocalMinimizer.hpp"
#include "DakotaModel.hpp"
#include "SurrBasedLevelData.hpp"

namespace Dakota {


/// Class for provably-convergent local surrogate-based optimization
/// and nonlinear least squares.

/** This minimizer uses a SurrogateModel to perform minimization based
    on local, global, or hierarchical surrogates. It achieves provable
    convergence through the use of a sequence of trust regions and the
    application of surrogate corrections at the trust region centers. */

class DataFitSurrBasedLocalMinimizer: public SurrBasedLocalMinimizer
{
public:

  //
  //- Heading: Constructor and destructor
  //

  /// constructor
  DataFitSurrBasedLocalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~DataFitSurrBasedLocalMinimizer();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void pre_run();

  void post_run(std::ostream& s);

  void reset();

  //
  //- Heading: Virtual functions
  //

  /// update the trust region bounds, strictly contained within global bounds
  void update_trust_region();

  void verify();
  void minimize();
  void build();

  bool build_global();
  bool build_local();
  void compute_center_correction(bool embed_correction);

  //
  //- Heading: Convenience member functions
  //

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

  /// container for 
  SurrBasedLevelData trustRegionData;

  /// type of approximate subproblem objective: ORIGINAL_OBJ, LAGRANGIAN_OBJ,
  /// or AUGMENTED_LAGRANGIAN_OBJ
  short approxSubProbObj;
  /// type of approximate subproblem constraints: NO_CON, LINEARIZED_CON, or
  /// ORIGINAL_CON
  short approxSubProbCon;
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

  /// flags the use of a global data fit surrogate (rsm, ann, mars, kriging)
  bool globalApproxFlag;
  /// flags the use of a multipoint data fit surrogate (TANA)
  bool multiptApproxFlag;
  /// flags the use of a local data fit surrogate (Taylor series)
  bool localApproxFlag;
  /// flags the use of a model hierarchy/multifidelity surrogate 
  bool hierarchApproxFlag;

  // flag indicating inclusion of the center point in the DACE
  // evaluations for global approximations (CCD, Box-Behnken)
  //bool daceCenterPtFlag;
  // secondary flag indicating daceCenterPtFlag and no bounds truncation
  //bool daceCenterEvalFlag;

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
  static DataFitSurrBasedLocalMinimizer* sblmInstance;
};


inline DataFitSurrBasedLocalMinimizer::~DataFitSurrBasedLocalMinimizer()
{ }


inline void DataFitSurrBasedLocalMinimizer::update_trust_region()
{
  SurrBasedLocalMinimizer::update_trust_region(trustRegionData);

  // TO DO: will propagate in recast evaluate() but are there direct evaluates?
  //if (recastSubProb)
  //  iteratedModel.continuous_variables(cv_center);
  if (globalApproxFlag) { // propagate build bounds to DFSModel
    iteratedModel.continuous_lower_bounds(trustRegionData.tr_lower_bounds());
    iteratedModel.continuous_upper_bounds(trustRegionData.tr_upper_bounds());
  }
}

} // namespace Dakota

#endif
