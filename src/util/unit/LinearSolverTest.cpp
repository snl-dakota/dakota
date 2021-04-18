/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "UtilLinearSolvers.hpp"
#include "util_common.hpp"
#include "util_math_tools.hpp"

#include <Teuchos_UnitTestHarness.hpp>

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

void test_solver(LinearSolverBase& solver, Teuchos::FancyOStream& out,
                 bool& success) {
  MatrixXd A = create_simple_invertible_matrix();
  MatrixXd b = create_uniform_random_double_matrix(A.cols(), 1, 23);
  MatrixXd x(A.cols(), 1);

  solver.solve(A, b, x);

  TEST_ASSERT(matrix_equals(A * x, b, 1.0e-12));
}

// -------------------------------------

void test_solver(SOLVER_TYPE type, Teuchos::FancyOStream& out, bool& success) {
  std::shared_ptr<LinearSolverBase> solver = solver_factory(type);
  return test_solver(*solver, out, success);
}

// -------------------------------------

void test_solver_symmetric(LinearSolverBase& solver, Teuchos::FancyOStream& out,
                           bool& success) {
  MatrixXd A = create_symmetric_matrix();
  MatrixXd b = create_uniform_random_double_matrix(A.cols(), 1, 24);
  MatrixXd x(A.cols(), 1);

  solver.solve(A, b, x);

  TEST_ASSERT(matrix_equals(A * x, b, 1.0e-12));
}

// -------------------------------------

void test_solver_symmetric(SOLVER_TYPE type, Teuchos::FancyOStream& out,
                           bool& success) {
  std::shared_ptr<LinearSolverBase> solver = solver_factory(type);
  return test_solver_symmetric(*solver, out, success);
}

}  // anonymous namespace

////////////////
// Unit tests //
////////////////

// --------------------------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, solver_base) {
  MatrixXd A = create_simple_invertible_matrix();
  MatrixXd b = create_uniform_random_double_matrix(A.cols(), 1, 25);
  MatrixXd x(A.cols(), 1);

  LinearSolverBase base;
  TEST_THROW(base.solve(A, b, x), std::runtime_error);
}

// --------------------------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, solver_lu) {
  LUSolver lu_solver;
  test_solver(lu_solver, out, success);
  test_solver(SOLVER_TYPE::LU, out, success);
}

// --------------------------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, solver_svd) {
  SVDSolver svd_solver;
  test_solver(svd_solver, out, success);
  test_solver(SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION, out, success);
}

// --------------------------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, solver_qr) {
  QRSolver qr_solver;
  test_solver(qr_solver, out, success);
  test_solver(SOLVER_TYPE::QR_LEAST_SQ_REGRESSION, out, success);
}

// --------------------------------------------------------------------------------

TEUCHOS_UNIT_TEST(util, solver_cholesky) {
  CholeskySolver cholesky_solver;
  test_solver_symmetric(cholesky_solver, out, success);
  test_solver_symmetric(SOLVER_TYPE::CHOLESKY, out, success);
}

// --------------------------------------------------------------------------------
