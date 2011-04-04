/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EffGlobalMinimizer
//- Description: Implementation of Efficient Global Optimization
//- Owner:       Barron J Bichon, Vanderbilt University

#ifndef EGO_MINIMIZER_H
#define EGO_MINIMIZER_H

#include "SurrBasedMinimizer.H"


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

  EffGlobalMinimizer(Model& model); ///< standard constructor

  /// alternate constructor for instantiations "on the fly"
  //EffGlobalMinimizer(Model& model, const int& max_iterations,
  //		     const int& max_fn_evals);

  ~EffGlobalMinimizer(); ///< destructor
    
  //
  //- Heading: Member functions
  //

  void minimize_surrogates();

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

/// Computes relative change between successive c_stars using Euclidean norm
inline Real rel_change_c_star(const RealVector& curr_c_star,
			      const RealVector& prev_c_star)
{
  double norm_prev_c_star = 0, norm_diff_c_star = 0; 
  Real normalized_dc_star;
  for (size_t i=0; i < prev_c_star.length(); i++) {
    norm_diff_c_star
      += std::pow(std::fabs((double)curr_c_star[i]-(double)prev_c_star[i]),2.0);
    norm_prev_c_star += std::pow(std::fabs((double)prev_c_star[i]),2.0);
  }
  normalized_dc_star
    = (Real)std::pow(norm_diff_c_star,0.5)/std::pow(norm_prev_c_star,0.5);
#ifdef DEBUG
  Cout << "(Prev,Curr,dist) " << "(";
  for (size_t i=0; i < curr_c_star.length(); i++)
    Cout << prev_c_star[i] << ", ";
  for (size_t i=0; i < curr_c_star.length(); i++)
    Cout <<  curr_c_star[i] << ", ";
  Cout << normalized_dc_star << ")\n";
#endif
  return normalized_dc_star; 
}

};
		      
} // namespace Dakota

#endif
