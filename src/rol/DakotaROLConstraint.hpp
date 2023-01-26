#pragma once
#ifndef DAKOTA_ROL_CONSTRAINT_HPP
#define DAKOTA_ROL_CONSTRAINT_HPP

namespace Dakota {
namespace rol_interface {

class Constraint : public ConstraintType {
public:

  Constraint() = delete;
  Constraint( const Ptr<Cache>&                      cache, 
                    Dakota::CONSTRAINT_EQUALITY_TYPE type );

  virtual ~Constraint() = default;

  void update( const RealVector& x,
                     UpdateType  type,
                     int         iter = -1 ) override; 

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

private:

  Ptr<Cache> modelCache;
  bool hasLinear, hasNonlinear;
}; // class Constraint

} // namespace rol_interface
} // namespace Dakota


#endif // DAKOTA_ROL_CONSTRAINT_HPP
