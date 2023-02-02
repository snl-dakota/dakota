#include "DakotaROLObjective.hpp"

namespace Dakota {
namespace rol_interface {

Objective::Objective( const Ptr<Cache>& cache, bool has_inverse=false ) 
  : modelCache(cache) {
  if( !cache->useDefaultDeriv2 ) {
    hessOp = ROL::makePtr<Hessian>(cache,has_inverse);
  }
}

void Objective::update( const RealVector& x,
                              UpdateType   type,
                              int          iter ) {
  modelCache->update(x,type,iter);
} // Objective::update
         

Real Objective::value( const RealVector& x, Real& tol ) {
  auto& model = cache->dakotaModel;
  return model.curent_response().function_value(0);    
} // Objective::value


void Objective::gradient( RealVector& g, const RealVector& x, Real& tol ) {
  if( !cache->useDefaultDeriv1 ) {
    auto& model = cache->dakotaModel;
    g = model.current_response().function_gradient(0); 
  else 
    ROL::Objective<Real>::gradient(g,x,tol);
  
} // Objective::gradient


void Objective::hessVec(       RealVector& hv, 
                         const RealVector& v,
                         const RealVector& x,
                               Real&        tol ) {
  if( !cache->useDefaultDeriv2 ) {
    hessOp->apply(hv,v,tol);
  else 
    ROL::Objective<Real>::hessVec(hv,v,x,tol);

} // Objective::hessVec

void Objective::invHessVec(       RealVector& hv, 
                            const RealVector& v,
                            const RealVector& x,
                                  Real&        tol ) {
  
  if( !cache->useDefaultDeriv2 ) {
    assert(hessOp->has_inverse());
    hessOp->apply(ihv,v,tol);
  else 
    ROL::Objective<Real>::invHessVec(ihv,v,x,tol);

} // Objective::invHessVec

} // namespace rol_interface
} // namespace Dakota

