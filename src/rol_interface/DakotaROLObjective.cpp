#include "DakotaROLInterface.hpp"


namespace rol_interface {

Objective::Objective( ModelInterface* model_interface, 
                      bool            has_inverse ) 
  : modelInterface(model_interface) {
  if( model_interface->has_second_derivatives() ) {
    hessOp = std::make_unique<Hessian>(modelInterface,numOpt,has_inverse);
  }
}



void Objective::update( const ROL::Vector<Dakota::Real>& x,
                              ROL::UpdateType            type,
                              int                        iter ) {
  update( get_vector(x), type, iter );
  if(hessOp != nullptr) {

  }
} // Objective::update

 void Objective::update( const Dakota::RealVector& x,
                               ROL::UpdateType     type,
                               int                 iter ) {
  modelInterface->update(x,type,iter);
} // Objective::update
         


Dakota::Real Objective::value( const Dakota::RealVector& x, 
                                     Dakota::Real&       tol ) {
  const Dakota::RealVector& f = modelInterface->get_values();
  return f[0];
} // Objective::value


Real Objective::value( const ROL::Vector<Dakota::Real>& x, 
                             Dakota::Real&              tol ) {
  return value(get_vector(x),tol);
}


void Objective::gradient(       Dakota::RealVector& g, 
                          const Dakota::RealVector& x, 
                                Dakota::Real&       tol ) {
    g = model.current_response().function_gradient(0); 
} // Objective::gradient

void Objective::gradient(       ROL::Vector<Dakota::Real>& g,
                          const ROL::Vector<Dakota::Real>& x, 
                                Dakota::Real&              tol ) {
  if( model_interface->has_first_derivatives()) 
    gradient(get_vector(g),get_vector(x),tol);
  else 
    ROL::Objective<Dakota::Real>::gradient(g,x,tol);
}


void Objective::hessVec(       ROL::Vector<Dakota::Real>& hv, 
                         const ROL::Vector<Dakota::Real>& v,
                         const ROL::Vector<Dakota::Real>& x,
                               Dakota::Real&              tol ) {
  if( modelInterface->has_second_derivatives() ) {
    hessOp->apply(hv,v,tol);
  else 
    ROL::Objective<Dakota::Real>::hessVec(hv,v,x,tol);
}


void Objective::invHessVec(       Dakota::RealVector& hv, 
                            const Dakota::RealVector& v,
                            const Dakota::RealVector& x,
                                  Dakota::Real&       tol ) {
  
  if( modelInterface->has_second_derivatives() ) {
    assert(hessOp->has_inverse());
    hessOp->apply(ihv,v,tol);
  else 
    ROL::Objective<Dakota::Real>::invHessVec(ihv,v,x,tol);

} // Objective::invHessVec

void Objective::invHessVec(       ROL::Vector<Dakota::Real>& hv, 
                            const ROL::Vector<Dakota::Real>& v,
                            const ROL::Vector<Dakota::Real>& x,
                                  Dakota::Real&              tol ) {
  invHessVec(get_vector(hv),get_vector(v),get_vector(x),tol); 
} // Objective::invHessVec

} // namespace rol_interface

