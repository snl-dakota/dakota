#include "DakotaROLInterface.hpp"

namespace rol_interface {

Jacobian::Jacobian( const ROL::Ptr<ModelInterface>&        model_interface,
                          Dakota::CONSTRAINT_EQUALITY_TYPE type) {
  : modelInterface(model_interface), getMatrix(matrix_getter(modelInterface->dakotaModel,type)) {
  auto model = modelInterface->dakotaModel;
  nCols = model.cv();
  nRows = type == Dakota::CONSTRAINT_EQUALITY_TYPE::EQUALITY ? 
          model.num_linear_eq_constraints() : model.num_linear_ineq_constraints();

} // Jacobian::Jacobian


void Jacobian::apply(       ROL::Vector<Real>& jv,
                      const ROL::Vector<Real>& v,
                            Real&              tol ) const {
  assert(jv.dimension() == nRows);
  assert(v.dimension() == nCols);
  auto& model = modelInterface->dakotaModel;
  auto jv_values = get_vector_values(jv);      
  auto v_values  = get_vector_values(v); 
  auto J_values  = getMatrix(model).values();
  blas.GEMV(Teuchos::NO_TRANS, nRows, nCols, one, J_values, nRows, v_values, 1, zero, jv_values, 1);
} // Jacobian::apply

void Jacobian::applyAdjoint(       ROL::Vector<Real>& ajv,
                             const ROL::Vector<Real>& v,
                                   Real&              tol ) const {
  auto& model = modelInterface->dakotaModel;
  auto jv_values = get_vector_values(jv);      
  auto v_values  = get_vector_values(v); 
  auto J_values  = getMatrix(model).values();
  blas.GEMV(Teuchos::TRANS, nRows, nCols, one, J_values, nRows, v_values, 1, zero, v, jv_values, 1);
} // Jacobian::applyAdjoint
 

} // namespace rol_interface 
