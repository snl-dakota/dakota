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

  enum class LINEAR_INEQUALITY_FORMAT    { NONE,
                                           TWO_SIDED,
                                           ONE_SIDED_LOWER,
                                           ONE_SIDED_UPPER };

  enum class NONLINEAR_EQUALITY_FORMAT   { NONE,
                                           TPL_MANAGED,
                                           DAKOTA_MANAGED };

  enum class NONLINEAR_INEQUALITY_FORMAT { NONE,
                                           TWO_SIDED,
                                           ONE_SIDED_LOWER,
                                           ONE_SIDED_UPPER };

/// Base class for traits.

/** TraitsBase provides default traits through various accessors . */

class TraitsBase
{
public:

  /// default constructor
  TraitsBase();

  /// destructor
  virtual ~TraitsBase();

  /// A temporary query used in the refactor
  virtual bool is_derived() { return false; }

  /// Return the flag indicating whether method requires bounds
  virtual bool requires_bounds() { return false; }

  /// Return the flag indicating whether method supports linear equalities
  virtual bool supports_linear_equality() { return false; }

  /// Return the flag indicating whether method supports linear inequalities
  virtual bool supports_linear_inequality() { return false; }

  /// Return the format used for linear inequality constraints
  virtual LINEAR_INEQUALITY_FORMAT linear_inequality_format()
    { return LINEAR_INEQUALITY_FORMAT::NONE; }

  /// Return the flag indicating whether method supports nonlinear equalities
  virtual bool supports_nonlinear_equality() { return false; }

  /// Return the format used for nonlinear equality constraints
  virtual NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format()
    { return NONLINEAR_EQUALITY_FORMAT::NONE; }

  /// Return the flag indicating whether method supports nonlinear inequalities
  virtual bool supports_nonlinear_inequality() { return false; }

  /// Return the format used for nonlinear inequality constraints
  virtual NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format()
    { return NONLINEAR_INEQUALITY_FORMAT::NONE; }

  /// Return the flag indicating whether method expects nonlinear inequality
  /// constraints followed by nonlinear equality constraints
  virtual bool expects_nonlinear_inequalities_first() { return true; }

  /// Return the flag indicating whether method supports parameter scaling
  virtual bool supports_scaling() { return false; }

  /// Return the flag indicating whether method supports least squares
  virtual bool supports_least_squares() { return false; }

  /// Return flag indicating whether method supports multiobjective optimization
  virtual bool supports_multiobjectives() { return false; }

  /// Return the flag indicating whether method supports continuous variables
  virtual bool supports_continuous_variables() { return false; }

  /// Return the flag indicating whether method supports continuous variables
  virtual bool supports_discrete_variables() { return false; }

  /// Return the flag indicating whether method provides best objective result
  virtual bool provides_best_objective() { return false; }

  /// Return the flag indicating whether method provides best parameters result
  virtual bool provides_best_parameters() { return false; }

  /// Return the flag indicating whether method provides best constraint result
  virtual bool provides_best_constraint() { return false; }

  /// Return the flag indicating whether method provides final gradient result
  virtual bool provides_final_gradient() { return false; }

  /// Return the flag indicating whether method provides final hessian result
  virtual bool provides_final_hessian() { return false; }
};

inline TraitsBase::~TraitsBase()
{ }

} // namespace Dakota

#endif
