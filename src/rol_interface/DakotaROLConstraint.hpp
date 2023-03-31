#pragma once
#ifndef DAKOTA_ROL_CONSTRAINT_HPP
#define DAKOTA_ROL_CONSTRAINT_HPP

namespace rol_interface {

class ModelInterface;

class Constraint : public ROL::Constraint<Dakota::Real> {
public:

  class enum Type : std::uint8_t {
    LinearEquality = 0,
    LinearInequality,
    NonlinearEquality,
    NonlinearInequalty,
    Default
  };

  Constraint() = delete;
  Constraint( const ROL::Ptr<ModelInterface>&        model_interface, 
                    Dakota::CONSTRAINT_EQUALITY_TYPE type );

  virtual ~Constraint() = default;

  void update( const ROL::Vector<Dakota::Real>& x,
                     ROL::UpdateType            type,
                     int                        iter = -1 ) override; 

  virtual void update( const Dakota::RealVector& x,
                             ROL::UpdateType     type,
                             int                 iter = -1 ); 

  void value(       ROL::Vector<Dakota::Real>& c,
              const ROL::Vector<Dakota::Real>& x,
                    Dakota::Real&              tol ) override;  

  virtual void value(       Dakota::RealVector& c,
                      const Dakota::RealVector& x,
                            Dakota::Real&       tol );  

  void applyJacobian(       ROL::Vector<Dakota::Real>& jv,
                      const ROL::Vector<Dakota::Real>& v,
                      const ROL::Vector<Dakota::Real>& x,
                            Dakota::Real&              tol ) override;  

  virtual void applyJacobian(       Dakota::RealVector& jv,
                              const Dakota::RealVector& v,
                              const Dakota::RealVector& x,
                                    Dakota::Real&       tol );  

  void applyAdjointJacobian(       ROL::Vector<Dakota::Real>& ajv,
                             const ROL::Vector<Dakota::Real>& v,
                             const ROL::Vector<Dakota::Real>& x,
                                   Dakota::Real&              tol ) override;  

  virtual void applyAdjointJacobian(       Dakota::RealVector& ajv,
                                     const Dakota::RealVector& v,
                                     const Dakota::RealVector& x,
                                           Dakota::Real&       tol );  

  void applyAdjointHessian(        ROL::Vector<Dakota::Real>& ahuv,
                             const ROL::Vector<Dakota::Real>& u,
                             const ROL::Vector<Dakota::Real>& v,
                             const ROL::Vector<Dakota::Real>& x,
                                   Dakota::Real&              tol ) override;  

  virtual void applyAdjointHessian(        Dakota::RealVector& ahuv,
                                     const Dakota::RealVector& u,
                                     const Dakota::RealVector& v,
                                     const Dakota::RealVector& x,
                                           Dakota::Real&       tol );  
      
  
private:

  static constexpr Dakota::Real zero = 0; 
  static constexpr Dakota::Real one = 1;


  Teuchos::BLAS<int,Dakota::Real>  blas;
  ModelVector                      constraintValues;
  ModelMatrix                      constraintJacobian;
  ROL::Ptr<ModelInterface>         modelInterface;
  size_t                           indexOffset;
  bool                             isEquality;


}; // class Constraint

} // namespace rol_interface


#endif // DAKOTA_ROL_CONSTRAINT_HPP
