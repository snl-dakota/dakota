/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_COMMON_HPP
#define DAKOTA_UTIL_COMMON_HPP

#include "util_data_types.hpp"

namespace dakota {
namespace util {

/**
 *  \brief Throws a std::runtime_error based on the message argument
 *  \param[in] msg The error message to throw
 */
void error(const std::string& msg);

/**
 *  \brief Tests whether two Eigen MatrixXi objects are equal
 *  \param[in] A The first matrix to test
 *  \param[in] B The second matrix to test
 *  \returns Whether the matrices are equal
 */
bool matrix_equals(const MatrixXi& A, const MatrixXi& B);

/**
 *  \brief Tests whether two Eigen MatrixXd objects are equal, within a given
 * tolerance \param[in] A The first matrix to test \param[in] B The second
 * matrix to test \param[in] tol The tolerance to use when comparing double
 * values \returns Whether the matrices are equal to within tolerance
 */
bool matrix_equals(const MatrixXd& A, const MatrixXd& B, double tol);

/**
 *  \brief Tests whether two Teuchos RealMatrix objects are equal, within a
 * given tolerance \param[in] A The first matrix to test \param[in] B The second
 * matrix to test \param[in] tol The tolerance to use when comparing double
 * values \returns Whether the matrices are equal to within tolerance
 */
bool matrix_equals(const RealMatrix& A, const RealMatrix& B, double tol);

/**
 *  \brief Tests whether two Eigen MatrixXd objects relatively equal
 * (element-wise) within a given tolerance \param[in] A The first matrix to test
 *  \param[in] B The second matrix to test
 *  \param[in] tol The relative tolerance to use when comparing double values
 *  \returns Whether the matrices are relatively equal (within the tolerance)
 */
bool relative_allclose(const MatrixXd& A, const MatrixXd& B, const double tol);

/**
 *  \brief Populate a collection of vectors read in a from a text file assuming
 * data layout is one dataset per row \param[in] filename The file that contains
 * the data \param[out] R The collection of vectors to be populated \param[in]
 * num_datasets The number of datasets to read in \param[in] num_samples The
 * number of data points (e.g. function values, build points) per dataset
 */
void populateVectorsFromFile(const std::string& filename,
                             std::vector<VectorXd>& R, int num_datasets,
                             int num_samples);

/**
 *  \brief Populate a collection of matrices read in a from a text file assuming
 * data layout is a "column-major" stack of num_samples by num_vars matrices
 *  \param[in] filename The file that contains the data
 *  \param[out] S The collection of vectors to be populated
 *  \param[in] num_datasets The number of datasets to read in
 *  \param[in] num_samples The number of data points (e.g. function values,
 * build points) per dataset (row dim) \param[in] num_vars The number of
 * variables per dataset (column dim)
 */
void populateMatricesFromFile(const std::string& filename,
                              std::vector<MatrixXd>& S, int num_datasets,
                              int num_vars, int num_samples);

/**
 *  \brief Calculates the variance based on an Eigen VectorXd of double values
 *  \param[in] vec The vector
 *  \returns The calculated variance
 */
double variance(const VectorXd& vec);

}  // namespace util
}  // namespace dakota

#endif  // include guard
