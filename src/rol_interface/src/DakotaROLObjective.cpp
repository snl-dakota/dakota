#include "DakotaROLInterface.hpp"

namespace rol_interface {

Objective::Objective( BoolDispatch   HasGradient,
                      BoolDispatch   HasHessian,
                      Dakota::Model& model )  
  : numOpt{Dakota::ModelUtils::cv(model)}, 
    hasGradient{HasGradient},
    hasHessian{HasHessian},
    dakotaModel{dakotaModel} { 
      
}

void Objective::update( const ROL::Vector<Dakota::Real>& x,
                              ROL::UpdateType            type,
                              int                        iter ) {
} // Objective::update


Dakota::Real Objective::value( const ROL::Vector<Dakota::Real>& x, 
                                     Dakota::Real&              tol ) {
  const auto& resp = dakotaModel.current_response();
  return resp.function_value(0);      
}



void Objective::gradient(       ROL::Vector<Dakota::Real>& g,
                          const ROL::Vector<Dakota::Real>& x, 
                                Dakota::Real&              tol ) {
  
}


void Objective::hessVec(       ROL::Vector<Dakota::Real>& hv, 
                         const ROL::Vector<Dakota::Real>& v,
                         const ROL::Vector<Dakota::Real>& x,
                               Dakota::Real&              tol ) {

}


} // namespace rol_interface

