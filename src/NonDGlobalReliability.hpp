/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef NOND_GLOBAL_RELIABILITY_H
#define NOND_GLOBAL_RELIABILITY_H

#include "NonDReliability.hpp"
#include "ProbabilityTransformation.hpp"

namespace Dakota {


/// Class for global reliability methods within DAKOTA/UQ

/** The NonDGlobalReliability class implements EGO/SKO for global MPP
    search, which maximizes an expected improvement function derived
    from Gaussian process models.  Once the limit state has been
    characterized, a multimodal importance sampling approach is used
    to compute probabilities. */

class NonDGlobalReliability: public NonDReliability
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  NonDGlobalReliability(ProblemDescDB& problem_db, Model& model);
  /// destructor
  ~NonDGlobalReliability();

  //
  //- Heading: Virtual function redefinitions
  //

  bool resize();
  void derived_init_communicators(ParLevLIter pl_iter);
  void derived_set_communicators(ParLevLIter pl_iter);
  void derived_free_communicators(ParLevLIter pl_iter);

  void pre_run();
  void core_run();
  void print_results(std::ostream& s, short results_state = FINAL_RESULTS);

private:

  //
  //- Heading: Convenience functions
  //

  /// construct the GP using EGO/SKO
  void optimize_gaussian_process();

  /// perform multimodal adaptive importance sampling on the GP
  void importance_sampling();

  /// determine current best solution from among sample data for expected
  /// imporovement function in Performance Measure Approach (PMA)
  void get_best_sample();

  /// calculate the penalty to be applied to the PMA constraint value
  Real constraint_penalty(const Real& constraint, 
			  const RealVector& c_variables);

  /// expected improvement function for the GP
  Real expected_improvement(const RealVector& expected_values,
			    const Variables& recast_vars);

  /// expected feasibility function for the GP
  Real expected_feasibility(const RealVector& expected_values,
			    const Variables& recast_vars);

  /// evaluate iteratedModel at current point to collect x-space truth data
  void x_truth_evaluation(short mode);
  /// evaluate iteratedModel at specified point to collect x-space truth data
  void x_truth_evaluation(const RealVector& c_vars_u, short mode);
  /// evaluate uSpaceModel in BYPASS_SURROGATE mode to collect u-space
  /// truth data at specified point
  void u_truth_evaluation(const RealVector& c_vars_u, short mode);
  /// evaluate uSpaceModel to collect u-space surrogate data at specified point
  void u_evaluation(const RealVector& c_vars_u, short mode);

  //
  //- Heading: Objective/constraint evaluators passed to RecastModel
  //

  /// static function used as the objective function in the
  /// Expected Improvement (EIF) problem formulation for PMA
  static void EIF_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  /// static function used as the objective function in the
  /// Expected Feasibility (EFF) problem formulation for RIA
  static void EFF_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  //
  //- Heading: Data members
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static NonDGlobalReliability* nondGlobRelInstance;

  /// minimum penalized response from among true function evaluations
  Real fnStar;

  /// type of merit function used to penalize sample data
  short meritFunctionType;

  /// Lagrange multiplier for standard Lagrangian merit function
  Real lagrangeMult;
  
  /// Lagrange multiplier for augmented Lagrangian merit function
  Real augLagrangeMult;
  
  /// penalty parameter for augmented Lagrangian merit funciton
  Real penaltyParameter;

  /// constraint violation at last iteration, used to determine if the 
  /// current iterate should be accepted (must reduce violation)
  Real lastConstraintViolation;

  /// flag to determine if last iterate was accepted
  /// this controls update of parameters for augmented Lagrangian merit fn
  bool lastIterateAccepted;

  /// order of the data used for surrogate construction, in ActiveSet
  /// request vector 3-bit format; user may override responses spec
  short dataOrder;

};


inline void NonDGlobalReliability::x_truth_evaluation(short mode)
{
  uSpaceModel.component_parallel_mode(TRUTH_MODEL_MODE); // Recast forwards

  ActiveSet set = iteratedModel.current_response().active_set();
  set.request_values(0); set.request_value(mode, respFnCount);
  iteratedModel.evaluate(set);

  // Not currently necessary as surrogate mode does not employ parallelism:
  //uSpaceModel.component_parallel_mode(SURROGATE_MODEL_MODE); // restore
}


inline void NonDGlobalReliability::
x_truth_evaluation(const RealVector& c_vars_u, short mode)
{
  RealVector c_vars_x;
  SizetMultiArrayConstView x_cv_ids = iteratedModel.continuous_variable_ids(),
    u_cv_ids = (mppSearchType == SUBMETHOD_EGRA_X) ?
    uSpaceModel.continuous_variable_ids() :
    uSpaceModel.subordinate_model().continuous_variable_ids();
  uSpaceModel.probability_transformation().trans_U_to_X(c_vars_u, u_cv_ids,
							c_vars_x, x_cv_ids);
  iteratedModel.continuous_variables(c_vars_x);

  x_truth_evaluation(mode);
}


inline void NonDGlobalReliability::
u_truth_evaluation(const RealVector& c_vars_u, short mode)
{
  uSpaceModel.component_parallel_mode(TRUTH_MODEL_MODE); // Recast forwards
  uSpaceModel.surrogate_response_mode(BYPASS_SURROGATE); // Recast forwards

  uSpaceModel.continuous_variables(c_vars_u);
  ActiveSet set = uSpaceModel.current_response().active_set();
  set.request_values(0); set.request_value(mode, respFnCount);
  uSpaceModel.evaluate(set);

  uSpaceModel.surrogate_response_mode(UNCORRECTED_SURROGATE); // restore
  // Not currently necessary as surrogate mode does not employ parallelism:
  //uSpaceModel.component_parallel_mode(SURROGATE_MODEL_MODE); // restore
}


inline void NonDGlobalReliability::
u_evaluation(const RealVector& c_vars_u, short mode)
{
  uSpaceModel.continuous_variables(c_vars_u);
  ActiveSet set = uSpaceModel.current_response().active_set();
  set.request_values(0); set.request_value(mode, respFnCount);
  uSpaceModel.evaluate(set);
}

} // namespace Dakota

#endif
