#pragma once
#ifndef DAKOTA_ROL_LINEAR_EQUALITY_CONSTRAINT_HPP
#define DAKOTA_ROL_LINEAR_EQUALITY_CONSTRAINT_HPP

namespace rol_interface {

class LinearEqualityConstraint : public LinearConstraint {
public:

  LinearEqualityConstraint( const ROL::Ptr<ModelInterface>& model_interface,
                                  std::string               con_name = "Linear Equality" );
  virtual ~LinearEqualityConstraint = default;

  void value(       Dakota::RealVector& c,
              const Dakota::RealVector& x,
                    Dakota::Real&       tol ) override final;

protected:

  void update_from_model( ModelInterface* model_interface ) override final;  
  
}; // class LinearEqualityConstraint

} // namespace rol_interface

#endif // DAKOTA_ROL_LINEAR_EQUALITY_CONSTRAINT_HPP
