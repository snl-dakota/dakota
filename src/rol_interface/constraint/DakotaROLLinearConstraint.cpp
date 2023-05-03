#include "DakotaROLInterface.cpp"

namespace rol_interface {

LinearConstraint::value(       Dakota::RealVector& c,
                         const Dakota::RealVector& x,
                               Dakota::Real&       tol ) {
  applyJacobian(c,x,x,tol);  
}

void LinearConstraint::applyAdjointHessian(        ROL::Vector<Real>& ahuv,
                                             const ROL::Vector<Real>& u,
                                             const ROL::Vector<Real>& v,
                                             const ROL::Vector<Real>& x,
                                                   ROL::Real&         tol ) {
  ahuv.zero();
}

} // namespace rol_inteface
