#include "DakotaROLInterface.hpp"

namespace rol_interface {

Hessian::Hessian( const ROL::Ptr<ModelInterface>& model_interface,
                        bool                      has_inverse ) 
  : modelInterface(model_interface), hasInverse(has_inverse) {

  auto& model = modelInterface->dakotaModel;
  nRows = model.cv();
    f( hasInverse ) {
    workMat = std::make_unique<Dakota::RealMatrix>(nCols,nCols);
    iPiv = std::make_unique<int[]>(nCols);
  }
} // Hessian::Hessian


void Hessian::update( const ROL::Vector<Dakota::Real>& x,
                            bool               flag,
                            int                iter ) {
  auto& model = modelInterface->dakotaModel;
  (*workMat) = model.current_response().function_hessian(0);
  if(hasInverse) {
    int info = 0;
    int n = x.dimension();
    lapack.GETRF(n,n,workMat->values(),n,iPiv.get(),&info);
  }
}

void Hessian::apply(       ROL::Vector<Dakota::Real>& Hv,
                     const ROL::Vector<Dakota::Real>& v,
                           Dakota::Real&              tol ) const {
  assert(Av.dimension() == nRows);
  assert(v.dimension() == nRows);

  auto& model = modelInterface->dakotaModel;
  auto& Hv_ptr  = get_vector(Hv);      
  auto v_ptr   = get_vector(v); 
  auto hdata = modelInterface->dakotaModel.current_response().function_hessian(0).values(); 
  blas.SYMM(Teuchos::LEFT_SIDE,Teuchos::UPPER_TRI,nRows,1,one,hdata,nRows,v.values(),nRows,zero,Mv.values(),nRows);

} // Hessian::apply

void Hessian::applyInverse(       ROL::Vector<Dakota::Real>& Av,
                            const ROL::Vector<Dakota::Real>& v,
                                  Dakota::Real&              tol ) const {
  assert(haveInvHessian);
  auto etrans = Teuchos::NO_TRANS;
  auto Av_r = get_vector(Av);      
  auto v_r  = get_vector(v);      
  Av_r = v_r;
  int info = 0;
  lapack_.GETRS(Teuchos::NO_TRANS,nRows,1,workMat->values(),nRows,iPiv.get(),Av_r.values(),nRows,&info);
} // Hessian::applyInverse

} // namespace rol_interface 
