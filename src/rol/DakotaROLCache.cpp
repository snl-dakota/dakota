#include "DakotaROLCache.hpp"

namespace Dakota {
namespace rol_interface {

Cache::Cache( Dakota::Model& model ) 
  : dakotaModel(model),
    evalSet(model.current_response().active_set()) { 

  auto grad_type     = dakotaModel.gradient_type();
  auto hess_type     = dakotaModel.hessian_type();
  auto method_src    = dakotaModel.method_source();
  auto interval_type = dakotaModel.interval_type();  

  // initialize ActiveSet request values
  if( grad_type == "numerical" && method_src == "vendor" ) 
    evalSet.request_values(asVal);
  else if( dakotaModel.hessian_type() == "none" ) 
    evalSet.request_values(asValGrad);
  else
    evalSet.request_values(asValGradHess);
  
  if( grad_type == "analytic" || grad_type == "mixed" ||
    ( grad_type == "numerical" && method_src == "dakota" ) ){
    useDefaultDeriv1 = true;
    useDefaultDeriv2 = ( hess_type != "none" ) 
  } 

  useCenteredDifferences = (interval_type == "central");
}
                  

Cache::update( const RealVector& x,
                     UpdateType  type,
                     int         iter ) {
  
//  switch(type) { 
//    case UpdateType::Initial:
//      break;
//
//    case UpdateType::Accept:
//      break;
//
//    case UpdateType::Revert:
//      break;
//
//    case UpdateType::Trial:
//      break;
//
//    case UpdateType::Temp:
//    default:
//      break;
//  }
  evalSet = dakotaModel.current_response().active_set();
  dakotaModel.continuous_variables(x);
  dakotaModel.evaluate(evalSet);
} // Cache::update


} // namespace rol_interface
} // namespace Dakota
