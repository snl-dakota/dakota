#pragma once
#ifndef DAKOTA_ROL_LINEAR_CONSTRAINT_HPP
#define DAKOTA_ROL_LINEAR_CONSTRAINT_HPP

namespace rol_interface {

//- Class:       Linear Constraint
//- Description: Generic interface for any linear constraint function
//- Owner:       Greg von Winckel

/** LinearConstraint implements the common evaluation components that are
 *  used by both LinearEqualityConstraint and LinearInequalityConstraint */

class LinearConstraint : public Constraint {
 public:
  virtual ~LinearConstraint() = default;

  virtual void value(Dakota::RealVector& c, const Dakota::RealVector& x,
                     Dakota::Real& tol) override;

  void applyAdjointHessian(ROL::Vector<Dakota::Real>& ahuv,
                           const ROL::Vector<Dakota::Real>& u,
                           const ROL::Vector<Dakota::Real>& v,
                           const ROL::Vector<Dakota::Real>& x,
                           Dakota::Real& tol) override final;

};  // LinearConstraint

}  // namespace rol_interface

#endif  // DAKOTA_ROL_LINEAR_CONSTRAINT_HPP
