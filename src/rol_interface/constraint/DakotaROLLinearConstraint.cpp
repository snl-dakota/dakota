#include "DakotaROLInterface.hpp"

namespace rol_interface {

void LinearConstraint::value(       Dakota::RealVector& c,
                              const Dakota::RealVector& x,
                                    Dakota::Real&       tol ) {
  applyJacobian(c,x,x,tol);  
}

void LinearConstraint::applyAdjointHessian(        ROL::Vector<Dakota::Real>& ahuv,
                                             const ROL::Vector<Dakota::Real>& u,
                                             const ROL::Vector<Dakota::Real>& v,
                                             const ROL::Vector<Dakota::Real>& x,
                                                   Dakota::Real&              tol ) {
  ahuv.zero();
}

} // namespace rol_inteface
