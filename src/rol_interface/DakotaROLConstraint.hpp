#pragma once
#ifndef DAKOTA_ROL_CONSTRAINT_HPP
#define DAKOTA_ROL_CONSTRAINT_HPP

namespace rol_interface {

class Constraint : public ROL::Constraint<Real> {
public:

  Constraint() = delete;
  Constraint( const ROL::Ptr<ModelInterface>&        model_interface, 
                    Dakota::CONSTRAINT_EQUALITY_TYPE type );

  virtual ~Constraint() = default;

  void update( const RealVector&      x,
                     ROL::UpdateType  type,
                     int              iter = -1 ) override; 

  void value(       RealVector& c,
              const RealVector& x,
                    Real&       tol ) override;  

  void applyJacobian(       RealVector& jv,
                      const RealVector& v,
                      const RealVector& x,
                            Real&       tol ) override;  

  void applyAdjointJacobian(       RealVector& ajv,
                             const RealVector& v,
                             const RealVector& x,
                                   Real&       tol ) override;  

  void applyAdjointHessian(        RealVector& ahuv,
                             const RealVector& u,
                             const RealVector& v,
                             const RealVector& x,
                                   Real&       tol ) override;  
  
  /// Utility function to create a Lagrange multiplier vector in the dual constraint space
  inline ROL::Ptr<ROL::Vector<Real>> make_multiplier() const noexcept {
    return isEquality ? make_vector(modelInterface->dakotaModel.num_nonlinear_eq_constraints())->dual().clone()
                      : make_vector(modelInterface->dakotaModel.num_nonlinear_ineq_constraints())->dual().clone() 
  }

  /// Utility function to dynamically allocate a (nonlinear) inequality constraint
  inline static ROL::Ptr<Constraint> make_inequality( const ROL::Ptr<ModelInterface>& model_interface ) {
    return ROL::makePtr<Constraint>(model_interface, Dakota::CONSTRAINT_EQUALITY_TYPE::INEQUALTY);
  }

  /// Utility function to dynamically allocate a (nonlinear) equality constraint
  inline static ROL::Ptr<Constraint> make_equality( const ROL::Ptr<ModelInterface>& model_interface ) {
    return ROL::makePtr<Constraint>(model_interface,Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALTY);
  }

private:

  inline static ModelVector
  vector_getter( const Dakota::Model&                   model
                       Dakota::CONSTRAINT_EQUALITY_TYPE type ) noexcept {
    return ( type == Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                    &Dakota::Model::nonlinear_eq_constraints,
                    &Dakota::Model::nonlinear_ineq_constraints );
  }

  inline static ModelMatrix
  matrix_getter( const Dakota::Model&                   model
                       Dakota::CONSTRAINT_EQUALITY_TYPE type ) noexcept {
    return ( type == Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALITY ) ?
                    &Dakota::Model::nonlinear_eq_constraints,
                    &Dakota::Model::nonlinear_ineq_constraints );
  }

  static constexpr Real zero(0), one(1);


  Teuchos::BLAS<int,Real>  blas;
  ModelVector              constraintValues;
  ModelMatrix              constraintJacobian;
  ROL::Ptr<ModelInterface> modelInterface;
  size_t                   indexOffset;
  bool                     isEquality;


}; // class Constraint

} // namespace rol_interface


#endif // DAKOTA_ROL_CONSTRAINT_HPP
