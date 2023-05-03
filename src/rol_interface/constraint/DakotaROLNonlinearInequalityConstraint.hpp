#pragma once
#ifndef DAKOTA_ROL_NONLINEAR_INEQUALITY_CONSTRAINT_HPP
#define DAKOTA_ROL_NONLINEAR_INEQUALITY_CONSTRAINT_HPP

namespace rol_interface {

class NonlinearInequalityConstraint : public Constraint {
public:

  NonlinearInequalityConstraint( ModelInterface* model_interface,
                                 std::string     con_name = "Nonlinear Inequality" );
  virtual ~NonlinearInequalityConstraint() = default;

protected:

  void update_from_model( ModelInterface* model_interface ) override final;

}; // class NonlinearInequalityConstraint       


} // namespace rol_interface


#endif // DAKOTA_ROL_NONLINEAR_INEQUALITY_CONSTRAINT_HPP
