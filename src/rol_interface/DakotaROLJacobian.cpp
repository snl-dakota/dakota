#include "DakotaROLInterface.hpp"

namespace rol_interface {

Jacobian::Jacobian( const Dakota::Real* const data, int num_rows, int num_cols ) : 
  jacValues(data), nRows(num_rows), nCols(num_cols) {
} // Jacobian::Jacobian


void Jacobian::apply(       ROL::Vector<Dakota::Real>& jv,
                      const ROL::Vector<Dakota::Real>& v,
                            Dakota::Real&              tol ) const {
  assert(jv.dimension() == nRows);
  assert(v.dimension() == nCols);
  auto& model = modelInterface->dakotaModel;
  auto jv_values = get_vector_values(jv);      
  auto v_values  = get_vector_values(v); 
  auto J_values  = getMatrix(model).values();
  blas.GEMV(Teuchos::NO_TRANS, nRows, nCols, 1, jacValues, nRows, v_values, 1, 0, jv_values, 1);
} // Jacobian::apply

void Jacobian::applyAdjoint(       ROL::Vector<Dakota::Real>& ajv,
                             const ROL::Vector<Dakota::Real>& v,
                                   Dakota::Real&              tol ) const {
  auto& model = modelInterface->dakotaModel;
  auto jv_values = get_vector_values(jv);      
  auto v_values  = get_vector_values(v); 
  blas.GEMV(Teuchos::TRANS, nRows, nCols, 1, jacValues, nRows, v_values, 1, 0, v, jv_values, 1);
} // Jacobian::applyAdjoint
 

} // namespace rol_interface 
