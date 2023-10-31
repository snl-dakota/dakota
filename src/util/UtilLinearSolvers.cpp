/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "UtilLinearSolvers.hpp"

#include <boost/assign.hpp>
#include <boost/bimap.hpp>

namespace dakota {
namespace util {

/// alias for LinearSolverBase's SOLVER_TYPE
using SOLVER_TYPE = LinearSolverBase::SOLVER_TYPE;
/// alias for Boost Bimap solver type <--> string
using BimapSolvertypeStr = boost::bimap<SOLVER_TYPE, std::string>;

// ------------------------------------------------------------

/// Bimap between solver types and names
static BimapSolvertypeStr type_name_bimap =
    boost::assign::list_of<BimapSolvertypeStr::relation>
    // clang-format off
    (SOLVER_TYPE::CHOLESKY, "cholesky")
    (SOLVER_TYPE::EQ_CONS_LEAST_SQ_REGRESSION, "equality-constrained lsq regression")
    (SOLVER_TYPE::LASSO_REGRESSION, "lasso regression")
    (SOLVER_TYPE::LEAST_ANGLE_REGRESSION, "least angle regression")
    (SOLVER_TYPE::LU, "LU")
    (SOLVER_TYPE::ORTHOG_MATCH_PURSUIT, "orthogonal matching pursuit")
    (SOLVER_TYPE::QR_LEAST_SQ_REGRESSION, "QR lsq regression")
    (SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION, "SVD");
// clang-format on

// ------------------------------------------------------------

SOLVER_TYPE LinearSolverBase::solver_type(const std::string& solver_name) {
  BimapSolvertypeStr::right_const_iterator rc_iter =
      type_name_bimap.right.find(solver_name);
  if (rc_iter == type_name_bimap.right.end()) {
    throw std::runtime_error("Invalid solver_name");
  }
  return rc_iter->second;
}

// ------------------------------------------------------------

std::shared_ptr<LinearSolverBase> solver_factory(SOLVER_TYPE type) {
  switch (type) {
    case SOLVER_TYPE::CHOLESKY:
      return std::make_shared<CholeskySolver>();

    case SOLVER_TYPE::EQ_CONS_LEAST_SQ_REGRESSION:
      throw(std::runtime_error(
          "EQ_CONS_LEAST_SQ_REGRESSION not yet implemented."));

    case SOLVER_TYPE::LASSO_REGRESSION:
      throw(std::runtime_error("LASSO_REGRESSION not yet implemented."));

    case SOLVER_TYPE::LEAST_ANGLE_REGRESSION:
      throw(std::runtime_error("LEAST_ANGLE_REGRESSION not yet implemented."));

    case SOLVER_TYPE::LU:
      return std::make_shared<LUSolver>();

    case SOLVER_TYPE::ORTHOG_MATCH_PURSUIT:
      throw(std::runtime_error("ORTHOG_MATCH_PURSUIT not yet implemented."));

    case SOLVER_TYPE::QR_LEAST_SQ_REGRESSION:
      return std::make_shared<QRSolver>();

    case SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION:
      return std::make_shared<SVDSolver>();

    default:
      throw(std::runtime_error("Unknown solver type in solver_factory."));
  }
}

// ------------------------------------------------------------

LinearSolverBase::LinearSolverBase() {}

LinearSolverBase::~LinearSolverBase() {}

bool LinearSolverBase::is_factorized() const { return false; }

void LinearSolverBase::factorize(const MatrixXd& mat) {
  silence_unused_args(mat);
  std::string msg = "factorize() Has not been implemented for this class.";
  throw(std::runtime_error(msg));
}

void LinearSolverBase::solve(const MatrixXd& lhs, const MatrixXd& rhs,
                             MatrixXd& x) {
  silence_unused_args(lhs, rhs, x);
  std::string msg = "solve() Has not been implemented for this class.";
  throw(std::runtime_error(msg));
}

void LinearSolverBase::solve(const MatrixXd& rhs, MatrixXd& x) {
  silence_unused_args(rhs, x);
  std::string msg = "solve() Has not been implemented for this class.";
  throw(std::runtime_error(msg));
}

// ------------------------------------------------------------

LUSolver::LUSolver() : LinearSolverBase() {}

LUSolver::~LUSolver() {}

bool LUSolver::is_factorized() const {
  if (LU_Ptr) return true;
  return false;
}

void LUSolver::factorize(const MatrixXd& A) {
  Eigen::FullPivLU<MatrixXd> lu;
  LU_Ptr = std::make_shared<Eigen::FullPivLU<MatrixXd>>(lu.compute(A));
}

void LUSolver::solve(const MatrixXd& A, const MatrixXd& b, MatrixXd& x) {
  factorize(A);
  solve(b, x);
}

void LUSolver::solve(const MatrixXd& b, MatrixXd& x) {
  if (LU_Ptr) {
    x = LU_Ptr->solve(b);
  } else {
    std::string msg = "LU decomposition has not been previously computed.";
    throw(std::runtime_error(msg));
  }
}

// ------------------------------------------------------------

SVDSolver::SVDSolver() : LinearSolverBase() {}

SVDSolver::~SVDSolver() {}

bool SVDSolver::is_factorized() const {
  if (SVD_Ptr) return true;
  return false;
}

void SVDSolver::factorize(const MatrixXd& A) {
  Eigen::BDCSVD<MatrixXd> bdcsvd;
  SVD_Ptr = std::make_shared<Eigen::BDCSVD<MatrixXd>>(
      bdcsvd.compute(A, Eigen::ComputeThinU | Eigen::ComputeThinV));
}

void SVDSolver::solve(const MatrixXd& A, const MatrixXd& b, MatrixXd& x) {
  factorize(A);
  solve(b, x);
}

void SVDSolver::solve(const MatrixXd& b, MatrixXd& x) {
  if (SVD_Ptr) {
    x = SVD_Ptr->solve(b);
  } else {
    std::string msg = "SVD has not been previously computed.";
    throw(std::runtime_error(msg));
  }
}

// ------------------------------------------------------------

QRSolver::QRSolver() : LinearSolverBase() {}

QRSolver::~QRSolver() {}

bool QRSolver::is_factorized() const {
  if (QR_Ptr) return true;
  return false;
}

void QRSolver::factorize(const MatrixXd& A) {
  Eigen::ColPivHouseholderQR<MatrixXd> qr;
  QR_Ptr =
      std::make_shared<Eigen::ColPivHouseholderQR<MatrixXd>>(qr.compute(A));
}

void QRSolver::solve(const MatrixXd& A, const MatrixXd& b, MatrixXd& x) {
  factorize(A);
  solve(b, x);
}

void QRSolver::solve(const MatrixXd& b, MatrixXd& x) {
  if (QR_Ptr) {
    x = QR_Ptr->solve(b);
  } else {
    std::string msg = "QR decomposition has not been previously computed.";
    throw(std::runtime_error(msg));
  }
}

// ------------------------------------------------------------

CholeskySolver::CholeskySolver() : LinearSolverBase() {}

CholeskySolver::~CholeskySolver() {}

bool CholeskySolver::is_factorized() const {
  if (LDLT_Ptr) return true;
  return false;
}

void CholeskySolver::factorize(const MatrixXd& A) {
  Eigen::LDLT<MatrixXd> ldlt;
  LDLT_Ptr = std::make_shared<Eigen::LDLT<MatrixXd>>(ldlt.compute(A));
}

void CholeskySolver::solve(const MatrixXd& A, const MatrixXd& b, MatrixXd& x) {
  factorize(A);
  solve(b, x);
}

void CholeskySolver::solve(const MatrixXd& b, MatrixXd& x) {
  if (LDLT_Ptr) {
    x = LDLT_Ptr->solve(b);
  } else {
    std::string msg =
        "Cholesky decomposition has not been previously computed.";
    throw(std::runtime_error(msg));
  }
}

// ------------------------------------------------------------

}  // namespace util
}  // namespace dakota
