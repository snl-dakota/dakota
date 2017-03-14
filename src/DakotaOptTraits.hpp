/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        OptTraits
//- Description:  Traits class used in registering the various functionalities
//                supported by individual Dakota optimization TPLs.
//- Owner:        Mike Eldred
//- Version: $Id: DakotaOptTraits.hpp 0001 2017-02-21 10:35:14Z mkhalil $

#ifndef DAKOTA_OPT_TRAITS_H
#define DAKOTA_OPT_TRAITS_H

namespace Dakota {


/// Base class for the optimizer branch of the iterator hierarchy.

/** The Optimizer class provides common data and functionality for
    DOTOptimizer, CONMINOptimizer, NPSOLOptimizer, SNLLOptimizer,
    NLPQLPOptimizer, COLINOptimizer, and JEGAOptimizer. */

class OptTraits
{
public:

protected:

  //
  //- Heading: Constructors and destructor
  //

  /// default constructor
  OptTraits();

  /// destructor
  ~OptTraits();

  //
  //- Heading: Convenience/Helper functions
  //

  /// Sets requiresBounds to true
  void set_requires_bounds(bool requires_bounds);

  /// Return the value of requiresBounds
  bool get_requires_bounds();

  /// Sets supportsLinearEquality to true
  void set_supports_linear_equality(bool supports_linear_equality);

  /// Return the value of supportsLinearEquality
  bool get_supports_linear_equality();

  /// Sets supportsLinearInequality to true
  void set_supports_linear_inequality(bool supports_linear_inequality);

  /// Return the value of supportsLinearInequality
  bool get_supports_linear_inequality();

  /// Sets supportsNonlinearEquality to true
  void set_supports_nonlinear_equality(bool supports_nonlinear_equality);

  /// Return the value of supportsNonlinearEquality
  bool get_supports_nonlinear_equality();

  /// Sets supportsNonlinearInequality to true
  void set_supports_nonlinear_inequality(bool supports_nonlinear_inequality);

  /// Return the value of supportsNonlinearInequality
  bool get_supports_nonlinear_inequality();

  /// Sets supportsScaling to true
  void set_supports_scaling(bool supports_scaling);

  /// Return the value of supportsScaling
  bool get_supports_scaling();

  /// Sets supportsLeastSquares to true
  void set_supports_least_squares(bool supports_least_squares);

  /// Return the value of supportsLeastSquares
  bool get_supports_least_squares();

  /// Sets supportsMultiobjectives to true
  void set_supports_multiobjectives(bool supports_multiobjectives);

  /// Return the value of supportsMultiobjectives
  bool get_supports_multiobjectives();

  /// Sets the type of linear inequality supported
  void set_linear_inequality_type(const std::string& linear_inequality_type);

  /// Return the type of linear inequality supported
  void get_linear_inequality_type(std::string& linear_inequality_type);

  /// Sets the type of nonlinear inequality supported
  void set_nonlinear_inequality_type(const std::string& nonlinear_inequality_type);

  /// Return the type of nonlinear inequality supported
  void get_nonlinear_inequality_type(std::string& nonlinear_inequality_type);

  /// Sets the ordered list of accepted responses
  void set_response_order(const std::vector<std::string>& response_order);

  /// Return the ordered list of accepted responses
  void get_response_order(std::vector<std::string>& response_order);

  /// Sets supportsContinuousVariables to true
  void set_supports_continuous_variables(bool supports_continuous_variables);

  /// Return the value of supportsContinuousVariables
  bool get_supports_continuous_variables();

  /// Sets supportsIntegerVariables to true
  void set_supports_integer_variables(bool supports_integer_variables);

  /// Return the value of supportsIntegerVariables
  bool get_upports_integer_variables();

  /// Sets supportsRelaxableDiscreteVariables to true
  void set_supports_relaxable_discrete_variables(bool supports_relaxable_discrete_variables);

  /// Return the value of supportsRelaxableDiscreteVariables
  bool get_supports_relaxable_discrete_variables();

  /// Sets supportsCategoricalVariables to true
  void set_supports_categorical_variables(bool supports_categorical_variables);

  /// Return the value of supportsCategoricalVariables
  bool get_supports_categorical_variables();

  /// Sets providesBestObjective to true
  void set_provides_best_objective(bool provides_best_objective);

  /// Return the value of providesBestObjective
  bool get_provides_best_objective();

  /// Sets providesBestParameters to true
  void set_provides_best_parameters(bool provides_best_parameters);

  /// Return the value of providesBestParameters
  bool get_provides_best_parameters();

  /// Sets providesBestConstraint to true
  void set_provides_best_constraint(bool provides_best_constraint);

  /// Return the value of providesBestConstraint
  bool get_provides_best_constraint();

  /// Sets providesFinalGradient to true
  void set_provides_final_gradient(bool provides_final_gradient);

  /// Return the value of providesFinalGradient
  bool get_provides_final_gradient();

  /// Sets providesFinalHessian to true
  void set_provides_final_hessian(bool provides_final_hessian);

  /// Return the value of providesFinalHessian
  bool get_provides_final_hessian();

  //
  //- Heading: Data
  //
  
  /// flag indicating whether method requires bounds
  bool requiresBounds;
  
  /// flag indicating whether method supports linear equalities
  bool supportsLinearEquality;

  /// flag indicating whether method supports nonlinear equalities
  bool supportsNonlinearEquality;

  /// flag indicating whether method supports parameter scaling
  bool supportsScaling;

  /// flag indicating whether method supports least squares
  bool supportsLeastSquares;

  /// flag indicating whether method supports multiobjective optimization
  bool supportsMultiobjectives;

  /// structure conatining supported constraint forms
  boost::bimap < std::string, int > validConstraints;

  /// type of linear inequality supported
  int linearInequalityType;

  /// type of nonlinear inequality supported
  int nonlinearInequalityType;

  /// structure conatining supported response forms
  boost::bimap < std::string, int > validResponses;

  /// ordered list of accepted responses
  std::vector<int> responseOrder;

  /// flag indicating whether method supports continuous variables
  bool supportsContinuousVariables;

  /// flag indicating whether method supports integer variables
  bool supportsIntegerVariables;

  /// flag indicating whether method supports relaxable discrete variables
  bool supportsRelaxableDiscreteVariables;

  /// flag indicating whether method supports categorical variables
  bool supportsCategoricalVariables;

  /// flag indicating whether method provides best objective result
  bool providesBestObjective;

  /// flag indicating whether method provides best parameters result
  bool providesBestParameters;

  /// flag indicating whether method provides best constraint result
  bool providesBestConstraint;

  /// flag indicating whether method provides final gradient result
  bool providesFinalGradient;

  /// flag indicating whether method provides final hessian result
  bool providesFinalHessian;


private:

  //
  //- Heading: Convenience/Helper functions
  //

  //
  //- Heading: Data
  //
};


inline OptTraits::OptTraits()
{ }


inline OptTraits::~OptTraits()
{ }


//----------------------------------------------------------------

class InheritableSingleton_OptTraits
{
public:

  /// default constructor
  InheritableSingleton_OptTraits() { }

  /// destructor
  virtual ~InheritableSingleton_OptTraits() { }

protected:
  /// We could disallow instantiation of this base class by using a pure virtual function, eg
  //virtual bool is_derived() = 0;

public:
  //
  //- Heading: Convenience/Helper functions
  //

  /// Return the value of requiresBounds
  virtual bool requires_bounds() { return false; }

  /// Return the value of supportsLinearEquality
  virtual bool supports_linear_equality() { return false; }

  /// Return the value of supportsLinearInequality
  virtual bool supports_linear_inequality() { return false; }

  /// Return the value of supportsNonlinearEquality
  virtual bool supports_nonlinear_equality() { return false; }

  /// Return the value of supportsNonlinearInequality
  virtual bool supports_nonlinear_inequality() { return false; }

  /// Return the value of supportsScaling
  virtual bool supports_scaling() { return false; }

  /// Return the value of supportsLeastSquares
  virtual bool supports_least_squares() { return false; }

  /// Return the value of supportsMultiobjectives
  virtual bool supports_multiobjectives() { return false; }

  /// Return the value of supportsContinuousVariables
  virtual bool supports_continuous_variables() { return false; }

  /// Return the value of supportsIntegerVariables
  virtual bool upports_integer_variables() { return false; }

  /// Return the value of supportsRelaxableDiscreteVariables
  virtual bool supports_relaxable_discrete_variables() { return false; }

  /// Return the value of supportsCategoricalVariables
  virtual bool supports_categorical_variables() { return false; }

  /// Return the value of providesBestObjective
  virtual bool provides_best_objective() { return false; }

  /// Return the value of providesBestParameters
  virtual bool provides_best_parameters() { return false; }

  /// Return the value of providesBestConstraint
  virtual bool provides_best_constraint() { return false; }

  /// Return the value of providesFinalGradient
  virtual bool provides_final_gradient() { return false; }

  /// Return the value of providesFinalHessian
  virtual bool provides_final_hessian() { return false; }


private:

  //
  //- Heading: Convenience/Helper functions
  //

  //
  //- Heading: Data
  //
};



} // namespace Dakota

#endif
