#pragma once
#ifndef DAKOTA_ROLTRAITS_H
#define DAKOTA_ROLTRAITS_H

namespace rol_interface {

// -----------------------------------------------------------------
/** ROLTraits defines the types of problems and data formats ROL
    supports by overriding the default traits accessors in
    TraitsBase. */

class Traits : public Dakota::TraitsBase {
 public:
  //
  //- Heading: Constructor and destructor
  //

  /// Default constructor
  Traits() = default;

  /// Destructor
  virtual ~Traits() = default;

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
  Dakota::NONLINEAR_EQUALITY_FORMAT nonlinear_equality_format() {
    return Dakota::NONLINEAR_EQUALITY_FORMAT::TRUE_EQUALITY;
  }

  /// Return flag indicating ROL supports nonlinear inequalities
  bool supports_nonlinear_inequality() override { return true; }

  /// Return ROL format for nonlinear inequality constraints
  Dakota::NONLINEAR_INEQUALITY_FORMAT nonlinear_inequality_format() {
    return Dakota::NONLINEAR_INEQUALITY_FORMAT::TWO_SIDED;
  }

  inline static std::shared_ptr<Dakota::TraitsBase> create() {
    return std::make_shared<Traits>();
  }

};  // class Traits

}  // namespace rol_interface

#endif  // DAKOTA_ROL_TRAITS_H
