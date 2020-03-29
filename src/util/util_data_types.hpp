/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_DATA_TYPES_HPP
#define DAKOTA_UTIL_DATA_TYPES_HPP

// TODO: Consider organizing typedefs by purpose / category, including what's needed

#include "Teuchos_ParameterList.hpp"
#include "Teuchos_SerialDenseHelpers.hpp"
#include "Teuchos_SerialDenseVector.hpp"

#include "Eigen/Dense"

#include <complex>
#include <memory>


namespace dakota {

// TODO: May want to have distinct Dakota names such as EigMatrixXd

/// Eigen generic matrix of doubles in Dakota namespace
using MatrixXd = Eigen::MatrixXd;

/// Teuchos ParameterList for options management in Dakota namespace
using ParameterList = Teuchos::ParameterList;


namespace util {

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

}  // namespace util

}  // namespace dakota

#endif  // include guard
