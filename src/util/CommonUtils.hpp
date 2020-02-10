/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 * CommonUtils.hpp
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
 *  \brief Tests whether two Eigen VectorXd objects are equal, within a given tolerance.
 *  \param[A] The first vector to test.
 *  \param[B] The second vector to test.
 *  \param[tol] The tolerance to use when comparing double values.
 *  \returns Whether the vectors are equal.
*/
bool vector_equals(const VectorXd &A, const VectorXd &B, double tol);

/**
 *  \brief Tests whether two Eigen MatrixXd objects are equal, within a given tolerance.
 *  \param[A] The first matrix to test.
 *  \param[B] The second matrix to test.
 *  \param[tol] The tolerance to use when comparing double values.
 *  \returns Whether the matrices are equal.
*/
bool matrix_equals(const MatrixXd &A, const MatrixXd &B, double tol);

/**
 *  \brief Tests whether two Teuchos RealMatrix objects are equal, within a given tolerance.
 *  \param[A] The first matrix to test.
 *  \param[B] The second matrix to test.
 *  \param[tol] The tolerance to use when comparing double values.
 *  \returns Whether the matrices are equal.
*/
bool matrix_equals(const RealMatrix &A, const RealMatrix &B, double tol);

/**
 *  \brief Populate a collection of vectors read in a from a text file. Assumed data layout is one dataset 
 *  per row.
 *  \param[fileName] The file that contains the data.
 *  \param[R] The collection of vectors to be populated.
 *  \param[num_datasets] The number of datasets to read in.
 *  \param[num_samples] The number of data points (e.g. function values, build points) per dataset.
*/
void populateVectorsFromFile(std::string fileName, std::vector<VectorXd> &R, int num_datasets, int num_samples);

/**
 *  \brief Populate a collection of matrices read in a from a text file. Assumed data layout is a "column-major"
 *  stack of num_samples by num_vars matrices.
 *  \param[fileName] The file that contains the data.
 *  \param[S] The collection of vectors to be populated.
 *  \param[num_datasets] The number of datasets to read in.
 *  \param[num_samples] The number of data points (e.g. function values, build points) per dataset (row dim).
 *  \param[num_vars] The number of variables per dataset (column dim).
*/
void populateMatricesFromFile(std::string fileName, std::vector<MatrixXd> &S, int num_datasets, int num_vars, int num_samples);

/**
 *  \brief Calculates the variance based on an Eigen VectorXd of double values.
 *  \param[vec] The vector.
 *  \returns The calculated variance.
*/
double variance(VectorXd vec);

#endif // DAKOTA_COMMON_TEST_UTILS_HPP
