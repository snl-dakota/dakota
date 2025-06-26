#include "DakotaROLInterface.hpp"

namespace rol_interface {

LinearEqualityConstraint(ModelInterface* model_interface, std::string con_name)
    : Constraint(model_interface, Constraint::Type::LinearEquality, con_name) {
  model_interface->set_dimensions(this);
  model_interface->set_target(this);
  model_interface->set_jacobian(this);
}

void LinearEqualityConstraint::value(Dakota::RealVector& c,
                                     const Dakota::RealVector& x,
                                     Dakota::Real& tol) {
  applyJacobian(c, x, x, tol);
  for (int i = 0; i < numCon; ++i) c[i] -= targetData[i];
}

}  // namespace rol_interface
