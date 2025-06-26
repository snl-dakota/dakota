#pragma once
#ifndef DAKOTA_ROL_NONLINEAR_EQUALITY_CONSTRAINT_HPP
#define DAKOTA_ROL_NONLINEAR_EQUALITY_CONSTRAINT_HPP

namespace rol_interface {

class NonlinearEqualityConstraint : public Constraint {
 public:
  NonlinearEqualityConstraint(ModelInterface* model_interface,
                              std::string con_name = "Nonlinear Equality");

  virtual ~NonlinearEqualityConstraint() = default;

 protected:
  void update_from_model(ModelInterface* model_interface) override final;

};  // class NonlinearEqualityConstraint

}  // namespace rol_interface

#endif  // DAKOTA_ROL_NONLINEAR_EQUALITY_CONSTRAINT_HPP
