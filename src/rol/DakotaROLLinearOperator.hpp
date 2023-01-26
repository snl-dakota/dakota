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

  LinearOperator() = delete;
  LinearOperator( const Ptr<Cache>& cache,
                        Dakota::CONSTRAINT_EQUALITY_TYPE type );
  virtual ~LinearOperator = default;
  
    lapack_.GETRF(nrows,ncols,Awork_->values(),nrows,&ipiv_[0],&info);
  }

  void apply( Vector<Real> &Hv, const Vector<Real> &v, Real &tol ) const {
    Ptr<Teuchos::SerialDenseVector<Ordinal,Real>>       Hv_data = getVector(Hv);
    Ptr<const Teuchos::SerialDenseVector<Ordinal,Real>>  v_data = getVector(v);
    Ordinal nrows = A_->numRows();
    Ordinal ncols = A_->numCols();
    for (Ordinal i = 0; i < nrows; ++i) {
      (*Hv_data)(i) = static_cast<Real>(0);
      for (Ordinal j = 0; j < ncols; ++j) {
        (*Hv_data)(i) += (*A_)(i,j)*(*v_data)(j);
      }
    }
  }

  virtual void applyInverse( Vector<Real> &Hv, const Vector<Real> &v, Real &tol ) const {
    Ptr<Teuchos::SerialDenseVector<Ordinal,Real>>       Hv_data = getVector(Hv);
    Ptr<const Teuchos::SerialDenseVector<Ordinal,Real>>  v_data = getVector(v);
    Ordinal nrows = A_->numRows();
    for (Ordinal i = 0; i < nrows; ++i) {
      (*Hv_data)(i) = (*v_data)(i);
    }
    inverse(*Hv_data,'N');
  }

  void applyAdjoint( Vector<Real> &Hv, const Vector<Real> &v, Real &tol ) const {
    Ptr<Teuchos::SerialDenseVector<Ordinal,Real>>       Hv_data = getVector(Hv);
    Ptr<const Teuchos::SerialDenseVector<Ordinal,Real>>  v_data = getVector(v);
    Ordinal nrows = A_->numRows();
    Ordinal ncols = A_->numCols();
    for (Ordinal j = 0; j < ncols; ++j) {
      (*Hv_data)(j) = static_cast<Real>(0);
      for (Ordinal i = 0; i < nrows; ++i) {
        (*Hv_data)(j) += (*A_)(i,j)*(*v_data)(i);
      }
    }
  }

  virtual void applyAdjointInverse( Vector<Real> &Hv, const Vector<Real> &v, Real &tol ) const {
    Ptr<Teuchos::SerialDenseVector<Ordinal,Real>>       Hv_data = getVector(Hv);
    Ptr<const Teuchos::SerialDenseVector<Ordinal,Real>>  v_data = getVector(v);
    Ordinal nrows = A_->numRows();
    for (Ordinal i = 0; i < nrows; ++i) {
      (*Hv_data)(i) = (*v_data)(i);
    }
    inverse(*Hv_data,'T');
  }

private:
  Teuchos::LAPACK<int,Real> lapack;
  RealMatrix workMat;
  std::vector<int> iPiv;
  int numOpt, numCon; /// < Number of optimization variables and constraints 
  int info = 0;

  void inverse(RealVector& x, const char TRANS) const {
    Ordinal  NRHS  = 1;
    Ordinal  LDA   = N;
    Ordinal  LDB   = N;
    Ordinal  INFO  = 0;
    lapack_.GETRS(TRANS,N,NRHS,workMat->values(),LDA,&ipiv_[0],x.values(),LDB,&INFO);
  }

}; // class LinearOperator

} // namespace rol_interface
} // namespace Dakota

#endif
