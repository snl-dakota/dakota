#include "DakotaROLInterface.hpp"


namespace rol_interface {

Objective::Objective( const ROL::Ptr<ModelInterface>& model_interface, 
                            bool                      has_inverse=false ) 
  : modelInterface(model_interface) {
  if( !model_interface->useDefaultDeriv2 ) {
    hessOp = ROL::makePtr<Hessian>(modelInterface,has_inverse);
  }
}

void Objective::update( const RealVector&       x,
                              ROL::UpdateType   type,
                              int               iter ) {
  modelInterface->update(x,type,iter);
} // Objective::update
         

Real Objective::value( const RealVector& x, Real& tol ) {
  auto& model = modelInterface->dakotaModel;
  return model.curent_response().function_value(0);    
} // Objective::value


void Objective::gradient( RealVector& g, const RealVector& x, Real& tol ) {
  if( !model_interface->useDefaultDeriv1 ) {
    auto& model = modelInterface->dakotaModel;
    g = model.current_response().function_gradient(0); 
  else 
    ROL::Objective<Real>::gradient(g,x,tol);
  
} // Objective::gradient


void Objective::hessVec(       RealVector& hv, 
                         const RealVector& v,
                         const RealVector& x,
                               Real&        tol ) {
  if( !modelInterface->useDefaultDeriv2 ) {
    hessOp->apply(hv,v,tol);
  else 
    ROL::Objective<Real>::hessVec(hv,v,x,tol);

} // Objective::hessVec

void Objective::invHessVec(       RealVector& hv, 
                            const RealVector& v,
                            const RealVector& x,
                                  Real&        tol ) {
  
  if( !modelInterface->useDefaultDeriv2 ) {
    assert(hessOp->has_inverse());
    hessOp->apply(ihv,v,tol);
  else 
    ROL::Objective<Real>::invHessVec(ihv,v,x,tol);

} // Objective::invHessVec

} // namespace rol_interface

