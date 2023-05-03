#include "DakotaROLInterface.hpp"

namespace rol_interface {

Jacobian::Jacobian( const Dakota::Real* const data, int num_rows, int num_cols ) : 
  jacValues(data), nRows(num_rows), nCols(num_cols) {
} // Jacobian::Jacobian

void Jacobian::apply(       ROL::Vector<Dakota::Real>& jv,
                      const ROL::Vector<Dakota::Real>& v,
                            Dakota::Real&              tol ) const {
  apply(get_vector(jv),get_vector(v),tol);
} // Jacobian::apply

void Jacobian::apply(       Dakota::RealVector& jv,
                      const Dakota::RealVector& v,
                            Dakota::Real&       tol ) const {
  blas.GEMV(Teuchos::NO_TRANS, nRows, nCols, 1, jacValues, nRows, v.values(), 1, 0, jv.values(), 1);
}


void Jacobian::applyAdjoint(       ROL::Vector<Dakota::Real>& ajv,
                             const ROL::Vector<Dakota::Real>& v,
                                   Dakota::Real&              tol ) const {
  applyAdjoint(get_vector(ajv), get_vector(v), tol);
} 

void Jacobian::applyAdjoint(       Dakota::RealVector& ajv,
                             const Dakota::RealVector& v,
                                   Dakota::Real&       tol ) const {
  blas.GEMV(Teuchos::TRANS, nRows, nCols, 1, jacValues, nRows, v.values(), 1, 0, v, ajv.values(), 1);
} // Jacobian::applyAdjoint
 

} // namespace rol_interface 
