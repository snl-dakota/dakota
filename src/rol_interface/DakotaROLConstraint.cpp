#include "DakotaROLInterface.hpp"

namespace rol_interface { 

Constraint::Constraint( const Ptr<ModelInterface>&             model_interface,
                              Dakota::CONSTRAINT_EQUALITY_TYPE type ) 
  : modelInterface(model_interface), 
    constraintValues(get_vector(model_interface->dakotaModel,type)),
    constraintJacobian(get_matrix(model_interface->dakotaModel,type)),
    isEquality( type == Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALITY ) {
  indexOffset = isEquality ? model_interface->dakotaModel.num_nonlinear_ineq_constraints() : 0;
} // Constraint::Constraint


void Constraint::update( const Dakota::RealVector&     x,
                               ROL::UpdateType type,
                               int             iter ) {
  modelInterface->update(c,type,iter);

} // Constraint::update  


void Constraint::value(       Dakota::RealVector& c,
                        const Dakota::RealVector& x,
                              Real&       tol ) {
  auto model = modelInterface->dakotaModel;
  c = constraintValues(model);
  int n = c.size();
  if( isEquality ) {
    auto t = model.nonlinear_eq_constraint_targets();
    blas.AXPY(n,-one,1,c,1);
  }
} // Constraint:::value


void Constraint::applyJacobian(       Dakota::RealVector& jv, 
                                const Dakota::RealVector& v,
                                const Dakota::RealVector& x, 
                                      Real&       tol ) {
  auto model = modelInterface->dakotaModel;
  auto jvdata = jv.values();
  auto vdata = v.values();
  auto jdata = constraintJacobian(model).values();
  int nrows = jv.size();
  int ncols = v.size();
  blas.GEMV(Teuchos::NO_TRANS,nrows,ncols,one,jdata,nrows,vdata,1,zero,jvdata,1);
} // Constraint::applyJacobian

void Constraint::applyJacobian(       Dakota::RealVector& jv, 
                                const Dakota::RealVector& v,
                                const Dakota::RealVector& x, 
                                      Real&       tol ) {
  auto model = modelInterface->dakotaModel;
  auto jvdata = jv.values();
  auto vdata = v.values();
  auto jdata = constraintJacobian(model).values();
  int nrows = jv.size();
  int ncols = v.size();
  blas.GEMV(Teuchos::TRANS,nrows,ncols,one,jdata,nrows,vdata,1,zero,jvdata,1);
} // Constraint::applyJacobian

void Constraint::applyAdjointHessian(       Dakota::RealVector& ahuv, 
                                      const Dakota::RealVector& u,
                                      const Dakota::RealVector& v,
                                      const Dakota::RealVector& x, 
                                            Real&       tol ) {
  auto model = modelInterface->dakotaModel;
  auto resp = model.current_response();
  int nu = u.size();
  int nv = v.size();
  auto vdata = v.values();
  auto ahuvdata = ahuv.values();
  // Repeatedly perform the symmetric matrix AXPY operation 
  // ahuv <- u[i]*H[i]*v + beta[i]*ahuv
  // where beta[i] = (i>0)
  for( int i=0; i<nu; ++i ) { 
    auto j = static_cast<size_t>(1+i+offset);
    Real beta = i>0;
    auto hdata = resp.function_hessian(j).values();
    blas.SYMM(Teuchos::LEFT_SIDE,Teuchos::UPPER_TRI,nv,1,u[i],hdata,nv,vdata,nv,beta,ahuvdata,nv);
  } 
} // Constraint::applyAdjointHessian



void Constraint::update( const ROL::Vector<Real>& x,
                               ROL::UpdateType    type,
                               int                iter ) {
  update(get_vector(x),type,iter);
} // Constraint::update  


void Constraint::value(       ROL::Vector<Real>& c,
                        const ROL::Vector<Real>& x,
                              Real&              tol ) {
  value(get_vector(c), get_vector(x), tol);
} // Constraint:::value


void Constraint::applyJacobian(       ROL::Vector<Real>& jv, 
                                const ROL::Vector<Real>& v,
                                const ROL::Vector<Real>& x, 
                                      Real&       tol ) {
  applyJacobian(get_vector(jv), get_vector(v), get_vector(x),tol);
} // Constraint::applyJacobian

void Constraint::applyJacobian(       ROL::Vector<Real>& jv, 
                                const ROL::Vector<Real>& v,
                                const ROL::Vector<Real>& x, 
                                      Real&       tol ) {
} // Constraint::applyJacobian

void Constraint::applyAdjointHessian(       ROL::Vector<Real>& ahuv, 
                                      const ROL::Vector<Real>& u,
                                      const ROL::Vector<Real>& v,
                                      const ROL::Vector<Real>& x, 
                                            Real&       tol ) {
} // Constraint::applyAdjointHessian

} // namespace rol_interface
