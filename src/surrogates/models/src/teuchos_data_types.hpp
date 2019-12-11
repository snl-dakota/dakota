/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef SURROGATES_UTIL_TEUCHOS_DATA_TYPES_HPP
#define SURROGATES_UTIL_TEUCHOS_DATA_TYPES_HPP

#include <complex>
#include <memory>
#include "Teuchos_SerialDenseVector.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"

namespace Surrogates {

typedef double Real;
typedef std::complex<double> Complex;

typedef Teuchos::SerialDenseMatrix<int,int> IntMatrix;
typedef Teuchos::SerialDenseMatrix<int,Real> RealMatrix;
typedef Teuchos::SerialDenseMatrix<int,Complex> ComplexMatrix;

typedef Teuchos::SerialDenseVector<int,int> IntVector;
typedef Teuchos::SerialDenseVector<int,Real> RealVector;
typedef std::vector<RealVector>  RealVectorArray;
typedef Teuchos::SerialDenseVector<int,Complex> ComplexVector;

typedef std::vector<IntMatrix> IntMatrixList;
typedef std::vector<RealMatrix> RealMatrixList;
typedef std::vector<ComplexMatrix> ComplexMatrixList;

typedef Teuchos::SerialSymDenseMatrix<int, int> IntSymMatrix;
typedef Teuchos::SerialSymDenseMatrix<int, Real> RealSymMatrix;
typedef Teuchos::SerialSymDenseMatrix<int, Complex> ComplexSymMatrix;

}  // namespace Surrogates

#endif  // include guard
