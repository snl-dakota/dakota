#include "DakotaROLInterface.hpp"


namespace rol_interface {

Objective::Objective( const ROL::Ptr<ModelInterface>& model_interface, 
                            bool                      has_inverse=false ) 
  : modelInterface(model_interface) {
  if( !model_interface->useDefaultDeriv2 ) {
    hessOp = ROL::makePtr<Hessian>(modelInterface,has_inverse);
  }
}



void Objective::update( const ROL::Vector<Real>&  x,
                              ROL::UpdateType     type,
                              int                 iter ) {
  update( get_vector(x), type, iter );
} // Objective::update

 void Objective::update( const Dakota::RealVector&  x,
                              ROL::UpdateType     type,
                              int                 iter ) {
  modelInterface->update(x,type,iter);
} // Objective::update
         


Real Objective::value( const Dakota::RealVector& x, Real& tol ) {
  auto& model = modelInterface->dakotaModel;
  return model.curent_response().function_value(0);    
} // Objective::value


Real Objective::value( const ROL::Vector<Real>& x, Real& tol ) {
  return value(get_vector(x),tol);
}


void Objective::gradient( Dakota::RealVector& g, const Dakota::RealVector& x, Real& tol ) {
  if( !model_interface->useDefaultDeriv1 ) {
    auto& model = modelInterface->dakotaModel;
    g = model.current_response().function_gradient(0); 
  else 
    ROL::Objective<Real>::gradient(g,x,tol);
} // Objective::gradient

Real Objective::gradient(       ROL::Vector<Real>& g,
                          const ROL::Vector<Real>& x, 
                                Real&              tol ) {
  return gradient(get_vector(g),get_vector(x),tol);
}


void Objective::hessVec(       Dakota::RealVector& hv, 
                         const Dakota::RealVector& v,
                         const Dakota::RealVector& x,
                               Real&               tol ) {
  if( !modelInterface->useDefaultDeriv2 ) {
    hessOp->apply(hv,v,tol);
  else 
    ROL::Objective<Real>::hessVec(hv,v,x,tol);

} // Objective::hessVec

void Objective::hessVec(       ROL::Vector<Real>& hv, 
                         const ROL::Vector<Real>& v,
                         const ROL::Vector<Real>& x,
                               Real&              tol ) {
  hessVec(get_vector(hv),get_vector(v),get_vector(x),tol);
}


void Objective::invHessVec(       Dakota::RealVector& hv, 
                            const Dakota::RealVector& v,
                            const Dakota::RealVector& x,
                                  Real&               tol ) {
  
  if( !modelInterface->useDefaultDeriv2 ) {
    assert(hessOp->has_inverse());
    hessOp->apply(ihv,v,tol);
  else 
    ROL::Objective<Real>::invHessVec(ihv,v,x,tol);

} // Objective::invHessVec

void Objective::invHessVec(       ROL::Vector<Real>& hv, 
                            const ROL::Vector<Real>& v,
                            const ROL::Vector<Real>& x,
                                  Real&              tol ) {
  invHessVec(get_vector(hv),get_vector(v),get_vector(x),tol); 
} // Objective::invHessVec

} // namespace rol_interface

