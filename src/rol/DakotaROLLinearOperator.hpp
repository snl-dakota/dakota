#pragma once
#ifndef DAKOTA_ROL_LINEAROPERATOR_H
#define DAKOTA_ROL_LINEAROPERATOR_H

#include "ROL_TeuchosVector.hpp"
#include "ROL_LinearOperator.hpp"
#include "ROL_LAPACK.hpp"

namespace Dakota {
namespace ROL {

class LinearOperator : public ROL::LinearOperator<Real> {
public:

  class Type : std::uint8_t {
    Hessian = 0,
    IneqJacobian = 1,
    EqJacobian = 2
  };

  LinearOperator() = delete;
  LinearOperator( const Ptr<Cache>& cache, Type type, bool has_inv_hess = false );

  virtual ~LinearOperator = default;

  void update( const ROL::Vector<Real>& x, 
                     bool               flag = true, 
                     int                iter = -1 ) override;

  void apply(       ROL::Vector<Real>& Hv, 
              const ROL::Vector<Real>& v, 
                    Real&              tol ) const override;

  void applyInverse(       ROL::Vector<Real>& Hv, 
                     const ROL::Vector<Real>& v, 
                           Real&              tol ) const override;

  void applyAdjoint(       ROL::Vector<Real>& Hv, 
                     const ROL::Vector<Real>& v, 
                           Real&              tol ) const override;

  void applyAdjointInverse(       ROL::Vector<Real>& Hv, 
                            const ROL::Vector<Real>& v, 
                                  Real&              tol ) const override;
private:

  Teuchos::LAPACK<int,Real>   lapack;
  Ptr<Cache>                  modelCache;
  std::unique_ptr<RealMatrix> workMat; // LU Storage for Hessian inversion
  RealVector                  workVec;
  std::unique_ptr<int[]>      iPiv;    // LU Pivots for Hessian inversion
  Type                        opType;
  bool                        hasInvHess;

}; // class LinearOperator

} // namespace rol_interface
} // namespace Dakota

#endif // DAKOTA_ROL_LINEAROPERATOR_H
