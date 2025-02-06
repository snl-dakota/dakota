#include "DakotaROLInterface.hpp"

namespace rol_interface { 

Constraint::Constraint( ModelInterface*  model_interface,
                        Constraint::Type con_type, 
                        std::string      con_name )
  : targetData(nullptr), valueData(nullptr), jacobianData(nullptr), 
  modelInterface(model_interface), conType(con_type), conName(con_name) {
} // Constraint::Constraint


void Constraint::set_value( const Dakota::Real* const value ) {
  valueData = value;
}

void Constraint::set_jacobian( const Dakota::Real* const jacobian ) {
  jacobianData = jacobian;
}

void Constraint::set_dimensions( int num_opt, int num_con ) { 
  numOpt = num_opt;
  numCon = num_con;
}

void Constraint::update( const Dakota::RealVector& x,
                               ROL::UpdateType     type,
                               int                 iter ) {
  modelInterface->update(x,type,iter);
  update_from_model(modelInterface);

} // Constraint::update  

void Constraint::value(       ROL::Vector<Dakota::Real>& c,
                        const ROL::Vector<Dakota::Real>& x,
                              Dakota::Real&              tol ) {
  value(get_vector(c), get_vector(x), tol);
} // Constraint:::value


void Constraint::value(       Dakota::RealVector& c,
                        const Dakota::RealVector& x,
                              Dakota::Real&       tol ) {

  for(int i=0; i<numCon; ++i) c[i] = valueData[i];

  if( is_equality(conType) ) 
      for(int i=0; i<numCon; ++i) c[i] -= targetData[i];

} // Constraint:::value


void Constraint::applyJacobian(       ROL::Vector<Dakota::Real>& jv, 
                                const ROL::Vector<Dakota::Real>& v,
                                const ROL::Vector<Dakota::Real>& x, 
                                      Dakota::Real&              tol ) {
  if( modelInterface->has_first_derivatives() ) {
    applyJacobian(get_vector(jv), get_vector(v), get_vector(x),tol);
  }
  else {
    ROL::Constraint<Dakota::Real>::applyJacobian(jv,v,x,tol);
  }
} // Constraint::applyJacobian


void Constraint::applyJacobian(       Dakota::RealVector& jv, 
                                const Dakota::RealVector& v,
                                const Dakota::RealVector& x, 
                                      Dakota::Real&       tol ) {
  auto& blas = modelInterface->get_blas();
  auto jvdata = jv.values();
  auto vdata = v.values();
  blas.GEMV(Teuchos::NO_TRANS,numCon,numOpt,1,jacobianData,numCon,vdata,1,0,jvdata,1);
} // Constraint::applyJacobian


void Constraint::applyAdjointJacobian(       ROL::Vector<Dakota::Real>& ajv, 
                                       const ROL::Vector<Dakota::Real>& v,
                                       const ROL::Vector<Dakota::Real>& x, 
                                             Dakota::Real&              tol ) {
  if( modelInterface->has_first_derivatives() ) {
    applyAdjointJacobian(get_vector(ajv), get_vector(v), get_vector(x),tol);
  }
  else {
    ROL::Constraint<Dakota::Real>::applyAdjointJacobian(ajv,v,x,tol);
  }
} // Constraint::applyAdjointJacobian


void Constraint::applyAdjointJacobian(       Dakota::RealVector& ajv, 
                                       const Dakota::RealVector& v,
                                       const Dakota::RealVector& x, 
                                             Dakota::Real&       tol ) {
  auto& blas = modelInterface->get_blas();
  auto ajvdata = ajv.values();
  auto vdata = v.values();
  blas.GEMV(Teuchos::TRANS,numCon,numOpt,1,jacobianData,numCon,vdata,1,0,ajvdata,1);
} // Constraint::applyJacobian


void Constraint::applyAdjointHessian(       ROL::Vector<Dakota::Real>& ahuv, 
                                      const ROL::Vector<Dakota::Real>& u,
                                      const ROL::Vector<Dakota::Real>& v,
                                      const ROL::Vector<Dakota::Real>& x, 
                                            Dakota::Real&              tol ) {
  if( modelInterface->has_second_derivatives() ) {
    applyAdjointHessian(get_vector(ahuv), get_vector(u), get_vector(v), get_vector(x), tol);
  }
  else {
    ROL::Constraint<Dakota::Real>::applyAdjointHessian(ahuv,u,v,x,tol);
  }
} // Constraint::applyAdjointHessian


void Constraint::applyAdjointHessian(       Dakota::RealVector& ahuv, 
                                      const Dakota::RealVector& u,
                                      const Dakota::RealVector& v,
                                      const Dakota::RealVector& x, 
                                            Dakota::Real&       tol ) {
  auto& blas = modelInterface->get_blas();
  auto ahuvdata = ahuv.values();
  auto vdata = v.values();

  const Dakota::RealSymMatrixArray& hess = modelInterface->get_hessians();

  // Repeatedly perform ahuv <- beta*ahuv + u[i]*H[i]*v for i=0,...,numCon-1
  for( int i=0; i<numCon; ++i ) {
    Dakota::Real beta = i>0;
    auto hdata = hess[i].values();
    blas.SYMM(Teuchos::LEFT_SIDE,Teuchos::UPPER_TRI,numOpt,1,u[i],hdata,vdata,numOpt,beta,ahuvdata,numOpt);
  } 

} // Constraint::applyAdjointHessian

ROL::Ptr<ROL::Vector<Dakota::Real>> Constraint::make_lagrange_multiplier() const {
  return ROL::makePtr<Vector>(numCon);
}



} // namespace rol_interface
