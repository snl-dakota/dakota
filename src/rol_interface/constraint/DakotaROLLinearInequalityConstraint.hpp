#pragma once
#ifndef DAKOTA_ROL_LINEAR_INEQUALITY_CONSTRAINT_HPP
#define DAKOTA_ROL_LINEAR_INEQUALITY_CONSTRAINT_HPP

namespace rol_interface {

class LinearInequalityConstraint : public Constraint {
 public:
  LinearInequalityConstraint(ModelInterface* model_interface,
                             std::string con_name = "Linear Inequality");

  virtual ~LinearInequalityConstraint() = default;

 protected:
  void update_from_model(ModelInterface* model_interface) override final;
};

}  // namespace rol_interface

#endif  // DAKOTA_ROL_LINEAR_INEQUALITY_CONSTRAINT_HPP
