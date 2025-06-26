/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include "UtilLinearSolvers.hpp"
#include "util_common.hpp"
#include "util_math_tools.hpp"

using namespace dakota;
using namespace dakota::util;

using SOLVER_TYPE = LinearSolverBase::SOLVER_TYPE;

namespace {

// -------------------------------------

MatrixXd create_simple_invertible_matrix() {
  MatrixXd simple_matrix(3, 3);

  simple_matrix << 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0;

  // Need to improve the condition of the matrix (or use a different test case)
  simple_matrix += MatrixXd::Identity(3, 3);

  return simple_matrix;
}

// -------------------------------------

MatrixXd create_symmetric_matrix() {
  MatrixXd simple_matrix(3, 3);

  simple_matrix << 1.0, 7.0, 3.0, 7.0, 4.0, -5.0, 3.0, -5.0, 6.0;

  return simple_matrix;
}

// -------------------------------------

void test_solver(LinearSolverBase& solver) {
  MatrixXd A = create_simple_invertible_matrix();
  MatrixXd b = create_uniform_random_double_matrix(A.cols(), 1, 23);
  MatrixXd x(A.cols(), 1);

  solver.solve(A, b, x);

  EXPECT_TRUE(matrix_equals(A * x, b, 1.0e-12));
}

// -------------------------------------

void test_solver(SOLVER_TYPE type) {
  std::shared_ptr<LinearSolverBase> solver = solver_factory(type);
  return test_solver(*solver);
}

// -------------------------------------

void test_solver_symmetric(LinearSolverBase& solver) {
  MatrixXd A = create_symmetric_matrix();
  MatrixXd b = create_uniform_random_double_matrix(A.cols(), 1, 24);
  MatrixXd x(A.cols(), 1);

  solver.solve(A, b, x);

  EXPECT_TRUE(matrix_equals(A * x, b, 1.0e-12));
}

// -------------------------------------

void test_solver_symmetric(SOLVER_TYPE type) {
  std::shared_ptr<LinearSolverBase> solver = solver_factory(type);
  return test_solver_symmetric(*solver);
}

}  // anonymous namespace

////////////////
// Unit tests //
////////////////

// --------------------------------------------------------------------------------

TEST(LinearSolverTest_tests, util_solver_base) {
  MatrixXd A = create_simple_invertible_matrix();
  MatrixXd b = create_uniform_random_double_matrix(A.cols(), 1, 25);
  MatrixXd x(A.cols(), 1);

  LinearSolverBase base;
  EXPECT_THROW(base.solve(A, b, x), std::runtime_error);
}

// --------------------------------------------------------------------------------

TEST(LinearSolverTest_tests, util_solver_lu) {
  LUSolver lu_solver;
  test_solver(lu_solver);
  test_solver(SOLVER_TYPE::LU);
}

// --------------------------------------------------------------------------------

TEST(LinearSolverTest_tests, util_solver_svd) {
  SVDSolver svd_solver;
  test_solver(svd_solver);
  test_solver(SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION);
}

// --------------------------------------------------------------------------------

TEST(LinearSolverTest_tests, util_solver_qr) {
  QRSolver qr_solver;
  test_solver(qr_solver);
  test_solver(SOLVER_TYPE::QR_LEAST_SQ_REGRESSION);
}

// --------------------------------------------------------------------------------

TEST(LinearSolverTest_tests, util_solver_cholesky) {
  CholeskySolver cholesky_solver;
  test_solver_symmetric(cholesky_solver);
  test_solver_symmetric(SOLVER_TYPE::CHOLESKY);
}

// --------------------------------------------------------------------------------

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
