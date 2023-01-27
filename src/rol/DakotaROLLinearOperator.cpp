namespace Dakota {
namespace rol_interface {



LinearOperator::LinearOperator( const Ptr<Cache>&          cache,
                                      LinearOperator::Type type,
                                      bool                 has_inv_hess  ) 
  : modelCache(cache), opType(type), hasInvHess(has_inv_hess && type == Type::Hessian) {
  if( hasInvHess ) {
    auto& model = modelCache->dakotaModel;
    auto n = model.cv();
    workMat = std::make_unique<RealMatrix>(n,n);
    iPiv = std::make_unique<int[]>(n);
  }
}


void LinearOperator::update( const ROL::Vector<Real>& x,
                                   bool               flag,
                                   int                iter ) {
  auto& model = modelCache->dakotaModel;
  (*workMat) = model.current_response().function_hessian(0);
  if(hasInvHess) {
    int info = 0;
    int n = x.dimension();
    lapack.GETRF(n,n,workMat->values(),n,iPiv.get(),&info);
  }
}

void LinearOperator::apply(       ROL::Vector<Real>& Hv,
                            const ROL::Vector<Real>& v,
                                  Real&              tol ) const {
  auto& model = modelCache->dakotaModel;
  auto eside  = Teuchos::LEFT_SIDE;
  auto Hv_r = get_vector(Hv);      
  auto v_r  = get_vector(v);      
  if( opType == Type::Hessian ) {
    H = model.current_response().function_hessian(0);
    H.multiply(eside,1,H,v_r,0,Hv_r); 
  }
  else if( opType == Type::IneqJacobian ) {
    
  }
  else if( opType == Type::EqJacobian ) {

  }

}

void LinearOperator::applyInverse(       ROL::Vector<Real>& Hv,
                                   const ROL::Vector<Real>& v,
                                         Real&              tol ) const {
  assert(haveInvHessian);
  auto etrans = Teuchos::NO_TRANS;
  auto& model = modelCache->dakotaModel;
  auto n = model.cv();
  auto Hv_r = get_vector(Hv);      
  auto v_r  = get_vector(v);      
  Hv_r = v_r;
  int info = 0;
  lapack_.GETRS(etrans,n,1,workMat->values(),n,iPiv.get(),Hv_r.values(),n,&info);
}

 
                                      

} // namespace rol_interface 
} // namespace Dakota
