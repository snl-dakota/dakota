#include "DakotaROLInterface.hpp"

namespace rol_interface {

Hessian::Hessian(ModelInterface* model_interface, int num_rows,
                 bool has_inverse)
    : nRows(num_rows), hasInverse(has_inverse) {
  if (hasInverse) {
    workMat = std::make_unique<Dakota::RealMatrix>(nRows, nRows);
    iPiv = std::make_unique<int[]>(nCols);
  }
}  // Hessian::Hessian

// void Hessian::update( const ROL::Vector<Dakota::Real>& x,
//                             bool                       flag,
//                             int                        iter ) {
//   (*workMat) = model.current_response().function_hessian(0);
//   if(hasInverse) {
//     int info = 0;
//     int n = x.dimension();
//     lapack.GETRF(n,n,workMat->values(),n,iPiv.get(),&info);
//   }
// }

void Hessian::apply(ROL::Vector<Dakota::Real>& Hv,
                    const ROL::Vector<Dakota::Real>& v,
                    Dakota::Real& tol) const {
  auto hv_values = get_vector_values(Hv);
  auto v_values = get_vector_values(v);
  auto hdata = modelInterface->function_hessians().values();
  blas.SYMM(Teuchos::LEFT_SIDE, Teuchos::UPPER_TRI, nRows, 1, 1, hdata, nRows,
            v_values, nRows, 0, hv_values, nRows);

}  // Hessian::apply

void Hessian::applyInverse(ROL::Vector<Dakota::Real>& Hv,
                           const ROL::Vector<Dakota::Real>& v,
                           Dakota::Real& tol) const {
  assert(hasInverse);
  auto etrans = Teuchos::NO_TRANS;
  auto hv_values = get_vector_values(Hv);
  auto v_values = get_vector_values(v);

  Hv.set(v);
  int info = 0;
  lapack.GETRS(Teuchos::NO_TRANS, nRows, 1, workMat->values(), nRows,
               iPiv.get(), Av_r.values(), nRows, &info);
}  // Hessian::applyInverse

}  // namespace rol_interface
