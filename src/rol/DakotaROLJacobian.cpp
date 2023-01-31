namespace Dakota {
namespace rol_interface {


Jacobian::Jacobian( const ROL::Ptr<Cache>&                 cache,
                          Dakota::CONSTRAINT_EQUALITY_TYPE type) {
  : modelCache(cache), getMatrix(matrix_getter(modelCache->dakotaModel,type)) {
  auto model = modelCache->dakotaModel;
  nCols = model.cv();
  nRows = 
          model.num_linear_ineq_constraints();

    auto A = model.linear_ineq_constraint_coeffs();
    // Performs the matrix-vector operation: y <- alpha*A*x+beta*y or 
    // y <- alpha*A'*x+beta*y where A is a general m by n matrix. 
  }
  else if( opType == Type::EqJacobian ) {
 }


void Jacobian::apply(       ROL::Vector<Real>& Jv,
                      const ROL::Vector<Real>& v,
                            Real&              tol ) const {
  assert(Jv.dimension() == nRows);
  assert(v.dimension() == nCols);
  auto& model = modelCache->dakotaModel;
  auto Jv_values = get_vector_values(Jv);      
  auto v_values  = get_vector_values(v); 
  auto J_values  = getMatrix(model).values();
  blas.GEMV(Teuchos::NO_TRANS, nRows, nCols, one, J_values, nRows, v_values, 1, zero, v, Jv_values, 1);
}

void Jacobian::applyAdjoint(       ROL::Vector<Real>& aJv,
                             const ROL::Vector<Real>& v,
                                   Real&              tol ) const {
  auto& model = modelCache->dakotaModel;
  auto Jv_values = get_vector_values(Jv);      
  auto v_values  = get_vector_values(v); 
  auto J_values  = getMatrix(model).values();
  blas.GEMV(Teuchos::TRANS, nRows, nCols, one, J_values, nRows, v_values, 1, zero, v, Jv_values, 1);
} // Jacobian::applyAdjoint
 

} // namespace rol_interface 
} // namespace Dakota
