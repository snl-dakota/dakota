#pragma once
#ifndef DAKOTA_ROLTRAITS_H
#define DAKOTA_ROLTRAITS_H

#include "DakotaTraitsBase.hpp"

namespace Dakota {
namespace rol_interface {

// -----------------------------------------------------------------
/** ROLTraits defines the types of problems and data formats ROL
    supports by overriding the default traits accessors in
    TraitsBase. */

class Traits: public TraitsBase {
public:

  //
  //- Heading: Constructor and destructor
  //

  /// Default constructor
  ROLTraits() = default;

  /// Destructor
  virtual ~ROLTraits()  = default;

  //
  //- Heading: Virtual member function redefinitions
  //

  /// Return flag indicating ROL supports continuous variables
  bool supports_continuous_variables() override { return true; }

  /// Return flag indicating ROL supports linear equalities
  bool supports_linear_equality() override { return true; }

  /// Return flag indicating ROL supports linear inequalities
  bool supports_linear_inequality() override { return true; }

  /// Return flag indicating ROL supports nonlinear equalities
  bool supports_nonlinear_equality() override { return true; }

  /// Return ROL format for nonlinear equality constraints
  NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format() {
    return NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY;
  }

  /// Return flag indicating ROL supports nonlinear inequalities
  bool supports_nonlinear_inequality() override { return true; }

  /// Return ROL format for nonlinear inequality constraints
  NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format() {
    return NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED;
  }

  inline static std::shared_ptr<TraitsBase> create() {
    return std::make_shared<ROLTraits>();
  }

}; // class ROLTraits

} // namespace rol_interface
} // namespace Dakota

#endif // DAKOTA_ROL_TRAITS_H
