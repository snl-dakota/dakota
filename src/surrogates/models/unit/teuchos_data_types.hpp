/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
    _______________________________________________________________________ */

#ifndef SURROGATES_UTIL_TEUCHOS_DATA_TYPES_HPP
#define SURROGATES_UTIL_TEUCHOS_DATA_TYPES_HPP

#include <complex>
#include "Teuchos_SerialDenseVector.hpp"

namespace Surrogates{

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
