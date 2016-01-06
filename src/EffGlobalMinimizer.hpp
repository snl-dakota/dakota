/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EffGlobalMinimizer
//- Description: Implementation of Efficient Global Optimization
//- Owner:       Barron J Bichon, Vanderbilt University

#ifndef EGO_MINIMIZER_H
#define EGO_MINIMIZER_H

#include "SurrBasedMinimizer.hpp"


namespace Dakota {


/// Implementation of Efficient Global Optimization/Least Squares algorithms

/** The EffGlobalMinimizer class provides an implementation of the
    Efficient Global Optimization algorithm developed by Jones, Schonlau,
    & Welch as well as adaptation of the concept to nonlinear least squares. */

class EffGlobalMinimizer: public SurrBasedMinimizer
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// standard constructor
  EffGlobalMinimizer(ProblemDescDB& problem_db, Model& model);
  /// alternate constructor for instantiations "on the fly"
  //EffGlobalMinimizer(Model& model, int max_iterations, int max_fn_evals);
  ~EffGlobalMinimizer(); ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  //void derived_init_communicators(ParLevLIter pl_iter);
  //void derived_set_communicators(ParLevLIter pl_iter);
  //void derived_free_communicators(ParLevLIter pl_iter);

  void core_run();

  const Model& algorithm_space_model() const;

private:

  //
  //- Heading: Convenience member functions
  //

  /// called by minimize_surrogates for setUpType == "model"
  void minimize_surrogates_on_model();
  /// called by minimize_surrogates for setUpType == "user_functions"
  //void minimize_surrogates_on_user_functions();

  /// determine best solution from among sample data for expected
  ///   imporovement function 
  void get_best_sample();

  /// expected improvement function for the GP
  Real expected_improvement(const RealVector& means,
			    const RealVector& variances);

  /// expected violation function for the constraint functions
  RealVector expected_violation(const RealVector& means,
				     const RealVector& variances);

  /// initialize and update the penaltyParameter
  void update_penalty();

  //
  //- Heading: Objective/constraint evaluators passed to RecastModel
  //

  /// static function used as the objective function in the
  /// Expected Improvement (EIF) problem formulation for PMA
  static void EIF_objective_eval(const Variables& sub_model_vars,
				 const Variables& recast_vars,
				 const Response& sub_model_response,
				 Response& recast_response);

  //
  //- Heading: Data
  //

  /// pointer to the active object instance used within the static evaluator
  /// functions in order to avoid the need for static data
  static EffGlobalMinimizer* effGlobalInstance;

  /// controls iteration mode: "model" (normal usage) or "user_functions"
  /// (user-supplied functions mode for "on the fly" instantiations).
  String setUpType;

  /// GP model of response, one approximation per response function
  Model fHatModel;
  /// recast model which assimilates mean and variance to solve the
  /// max(EIF) sub-problem
  Model eifModel;

  /// minimum penalized response from among true function evaluations
  Real meritFnStar;
  /// true function values corresponding to the minimum penalized response
  RealVector truthFnStar;
  /// point that corresponds to the optimal value meritFnStar
  RealVector varStar;

  /// order of the data used for surrogate construction, in ActiveSet
  /// request vector 3-bit format; user may override responses spec
  short dataOrder;
};


inline const Model& EffGlobalMinimizer::algorithm_space_model() const
{ return fHatModel; }
		      
} // namespace Dakota

#endif
