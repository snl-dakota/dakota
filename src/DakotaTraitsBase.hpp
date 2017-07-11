/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        TraitsBase
//- Description:  Traits class used in registering the various functionalities
//                supported by individual Dakota optimization TPLs.
//- Owner:        Moe Khalil
//- Version: $Id: DakotaTraitsBase.hpp 0001 2017-02-21 10:35:14Z mkhalil $


#ifndef DAKOTA_TRAITS_BASE_H
#define DAKOTA_TRAITS_BASE_H

namespace Dakota {

/// Base class for traits.

/** TraitsBase provides default traits through various accessors . */

class TraitsBase
{
public:

  /// default constructor
  TraitsBase();

  /// destructor
  virtual ~TraitsBase();

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
  virtual bool supports_integer_variables() { return false; }

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
};

inline TraitsBase::~TraitsBase()
{ }

} // namespace Dakota

#endif
