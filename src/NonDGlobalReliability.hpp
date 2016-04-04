/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:	 NonDGlobalReliability
//- Description: Class for local reliability methods within DAKOTA/UQ
//- Owner:	 Mike Eldred
//- Checked by:
//- Version:

#ifndef NOND_GLOBAL_RELIABILITY_H
#define NOND_GLOBAL_RELIABILITY_H

#include "NonDReliability.hpp"

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

  void core_run();
  void print_results(std::ostream& s);

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

} // namespace Dakota

#endif
