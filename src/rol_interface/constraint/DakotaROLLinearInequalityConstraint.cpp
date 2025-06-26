#include "DakotaROLInterface.hpp"

namespace rol_interface {

LinearInequalityConstraint::LinearInequalityConstraint(
    ModelInterface* model_interface, std::string con_name)
    : Constraint(model_interface, Constraint::Type::LinearInequality,
                 con_name) {
  model_interface->set_dimensions(this);
}  // LinearInequalityConstraint()

void LinearInequalityConstraint::update_from_model(
    ModelInterface* model_interface) {
  model_interface->set_jacobian(this);
}

}  // namespace rol_interface
