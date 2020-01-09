/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 * CommonTestUtils.hpp
 * author Elliott Ridgway
 */

/////////////
// Defines //
/////////////

#ifndef DAKOTA_COMMON_TEST_UTILS_HPP
#define DAKOTA_COMMON_TEST_UTILS_HPP

/////////////
// Imports //
/////////////

#include "Eigen/Dense"
#include "Teuchos_SerialDenseHelpers.hpp"

///////////////////////
// Using / Namespace //
///////////////////////

using Eigen::MatrixXd;
using Eigen::VectorXd;
using RealMatrix = Teuchos::SerialDenseMatrix<int,double>;

using namespace Eigen;

/////////////
// Headers //
/////////////

/**
 *  \brief Throws a std::runtime_error based on the message argument.
 *  \param[msg] The error message to throw.
*/
void error(const std::string msg);

/**
 *  \brief Tests whether two Eigen MatrixXd objects are equal, within a given tolerance.
 *  \param[A] The first matrix to test.
 *  \param[B] The second matrix to test.
 *  \param[tol] The tolerance to use when comparing double values.
*/
bool matrix_equals(const MatrixXd &A, const MatrixXd &B, double tol);

/**
 *  \brief Tests whether two Eigen RealMatrix objects are equal, within a given tolerance.
 *  \param[A] The first matrix to test.
 *  \param[B] The second matrix to test.
 *  \param[tol] The tolerance to use when comparing double values.
*/
bool matrix_equals(const RealMatrix &A, const RealMatrix &B, double tol);

/**
 *  \brief Calculates the variance based on an Eigen VectorXd of double values.
 *  \param[vec] The vector.
 *  \returns The calculated variance.
*/
double variance(VectorXd vec);

#endif // DAKOTA_COMMON_TEST_UTILS_HPP