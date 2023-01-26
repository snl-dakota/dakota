#include "DakotaROLObjective.hpp"

namespace Dakota {
namespace rol_interface {

Objective::Objective( const Ptr<Cache>& cache ) 
  : modelCache(cache) {}

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
    auto& model = cache->dakotaModel;
    auto h_mat  = model.current_response().function_hessian(0); 
    Dakota::apply_matrix_partial(h_mat,v,hv);
  else 
    ROL::Objective<Real>::hessVec(hv,v,x,tol);

} // Objective::hessVec

} // namespace rol_interface
} // namespace Dakota

