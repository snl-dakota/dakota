/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_common.hpp"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <fstream>

namespace dakota {
namespace util {

// ------------------------------------------------------------

void error(const std::string& msg) { throw(std::runtime_error(msg)); }

// ------------------------------------------------------------

bool matrix_equals(const MatrixXi& A, const MatrixXi& B) { return A == B; }

// ------------------------------------------------------------

bool matrix_equals(const MatrixXd& A, const MatrixXd& B, double tol) {
  if ((A.rows() != B.rows()) || (A.cols() != B.cols())) {
    std::cout << A.rows() << "," << A.cols() << std::endl;
    std::cout << B.rows() << "," << B.cols() << std::endl;
    error("matrix_equals() matrix sizes are inconsistent");
  }
  for (int j = 0; j < A.cols(); j++) {
    for (int i = 0; i < A.rows(); i++) {
      if (std::abs(A(i, j) - B(i, j)) > tol) return false;
    }
  }
  return true;
}

// ------------------------------------------------------------

bool matrix_equals(const RealMatrix& A, const RealMatrix& B, double tol) {
  if ((A.numRows() != B.numRows()) || (A.numCols() != B.numCols())) {
    std::cout << A.numRows() << "," << A.numCols() << std::endl;
    std::cout << B.numRows() << "," << B.numCols() << std::endl;
    error("matrix_equals() matrices sizes are inconsistent");
  }
  for (int j = 0; j < A.numCols(); j++) {
    for (int i = 0; i < A.numRows(); i++) {
      if (std::abs(A(i, j) - B(i, j)) > tol) return false;
    }
  }
  return true;
}

// ------------------------------------------------------------

bool relative_allclose(const MatrixXd& A, const MatrixXd& B, const double tol) {
  MatrixXd rel_diff = (A - B).cwiseQuotient(B).cwiseAbs();
  MatrixXd rel_tol(rel_diff.rows(), rel_diff.cols());
  rel_tol.fill(tol);
  return (rel_diff.array() <= rel_tol.array()).all();
}

// ------------------------------------------------------------

double variance(const VectorXd& vec) {
  boost::accumulators::accumulator_set<
      double, boost::accumulators::features<boost::accumulators::tag::variance>>
      acc;
  for (int i = 0; i < vec.size(); i++) {
    acc(vec(i));
  }
  return boost::accumulators::variance(acc);
}

// ------------------------------------------------------------

void populateVectorsFromFile(const std::string& filename,
                             std::vector<VectorXd>& R, int num_datasets,
                             int num_samples) {
  R.resize(num_datasets);
  std::ifstream in(filename, std::ios::in);

  if (!in.is_open()) {
    throw(std::runtime_error("File does not exist!"));
  }

  for (int k = 0; k < num_datasets; k++) {
    R[k].resize(num_samples);
    for (int i = 0; i < num_samples; i++) {
      in >> R[k](i);
    }
  }
  in.close();
}

// ------------------------------------------------------------

void populateMatricesFromFile(const std::string& filename,
                              std::vector<MatrixXd>& S, int num_datasets,
                              int num_vars, int num_samples) {
  S.resize(num_datasets);
  std::ifstream in(filename, std::ios::in);

  if (!in.is_open()) {
    throw(std::runtime_error("File does not exist!"));
  }

  for (int k = 0; k < num_datasets; k++) {
    S[k].resize(num_samples, num_vars);
    for (int i = 0; i < num_samples; i++) {
      for (int j = 0; j < num_vars; j++) {
        in >> S[k](i, j);
      }
    }
  }

  in.close();
}

// ------------------------------------------------------------

}  // namespace util
}  // namespace dakota
