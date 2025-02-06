#include "DakotaROLInterface.hpp"

namespace rol_interface { 

NonlinearInequalityConstraint::NonlinearInequalityConstraint( ModelInterface*  model_interface,
                                                              std::string      con_name )
  : Constraint(model_interface, Constraint::Type::NonlinearInequality, con_name ) {
  model_interface->set_dimensions(this);
} // NonlinearInequalityConstraint::NonlinearInequalityConstraint


void NonlinearInequalityConstraint::update_from_model( ModelInterface* model_interface ) {
  model_interface->set_value(this);
  model_interface->set_jacobian(this);
}



} // namespace rol_interface
