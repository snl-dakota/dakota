#include "DakotaROLInterface.hpp"


namespace rol_interface { 

NonlinearEqualityConstraint::NonlinearEqualityConstraint( ModelInterface* model_interface,
                                                          std::string     con_name )
  : Constraint(model_interface, Constraint::Type::NonlinearEquality, con_name) {
  model_interface->set_dimensions(this);
  model_interface->set_value(this);
} // NonlinearEqualityConstraint::NonlinearEqualityConstraint


void NonlinerEqualityConstraint::update_from_model( ModelInterface* model_interface ) {
  model_interface->set_jacobian(this);
}

} // namespace rol_interface
