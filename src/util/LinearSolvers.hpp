/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_LINEAR_SOLVERS_HPP
#define DAKOTA_UTIL_LINEAR_SOLVERS_HPP

#include "Eigen/Dense"
#include <iostream>
#include <memory>

using Eigen::MatrixXd;
using Eigen::VectorXd;

namespace dakota {
namespace util {


class LinearSolverBase
{

  public:

    LinearSolverBase() { }

    ~LinearSolverBase() { }

    virtual bool is_factorized()
    {
      return false;
    };

    virtual void factorize( MatrixXd & )
    {
      std::string msg = "factorize() Has not been implemented for this class.";
      throw( std::runtime_error( msg ) );
    };

    /**
     * \brief Find a solution to Ax = b
     */
    virtual void solve( MatrixXd &, MatrixXd &, MatrixXd & )
    {
      std::string msg = "solve() Has not been implemented for this class.";
      throw( std::runtime_error( msg ) );
    };

    /**
     * \brief Find a solution to Ax = b, is A is already computed.
     */
    virtual void solve( MatrixXd &, MatrixXd & )
    {
      std::string msg = "solve() Has not been implemented for this class.";
      throw( std::runtime_error( msg ) );
    };
};

// --------------------------------------------------------------------------------

enum class SOLVER_TYPE {
                         CHOLESKY,
                         EQ_CONS_LEAST_SQ_REGRESSION,
                         LASSO_REGRESSION,
                         LEAST_ANGLE_REGRESSION,
                         LU,
                         ORTHOG_MATCH_PURSUIT,
                         QR_LEAST_SQ_REGRESSION,
                         SVD_LEAST_SQ_REGRESSION
                       };

std::shared_ptr<LinearSolverBase> solver_factory(SOLVER_TYPE);

// --------------------------------------------------------------------------------

class LUSolver : public LinearSolverBase
{
  public:

    LUSolver() : LinearSolverBase() { };

    ~LUSolver() { };

    bool is_factorized() override
    {
      if(lu_ptr) return true;
      return false;
    };

    void factorize ( MatrixXd & A ) override
    {
      Eigen::FullPivLU<MatrixXd> lu;
      lu_ptr = std::make_shared<Eigen::FullPivLU<MatrixXd>>(lu.compute(A));
    };

    void solve( MatrixXd & A, MatrixXd & b, MatrixXd & x ) override
    {
      factorize(A);
      solve(b, x);
    };

    void solve( MatrixXd & b, MatrixXd & x ) override
    {
      if(lu_ptr) {
        x = lu_ptr->solve(b);
      } else {
        std::string msg = "LU has not been previously computed.";
        throw( std::runtime_error( msg ) );
      }
    };

  private:

    std::shared_ptr<Eigen::FullPivLU<MatrixXd>> lu_ptr;
};

// --------------------------------------------------------------------------------

class SVDSolver : public LinearSolverBase
{
  public:

    SVDSolver() : LinearSolverBase() { };

    ~SVDSolver() { };

    bool is_factorized() override
    {
      if(svd_ptr) return true;
      return false;
    };

    void factorize ( MatrixXd & A ) override
    {
      Eigen::BDCSVD<MatrixXd> bdcsvd;
      svd_ptr = std::make_shared<Eigen::BDCSVD<MatrixXd>>(bdcsvd.compute(A, Eigen::ComputeThinU | Eigen::ComputeThinV));
    };

    void solve( MatrixXd & A, MatrixXd & b, MatrixXd & x ) override
    {
      factorize(A);
      solve(b, x);
    };

    void solve( MatrixXd & b, MatrixXd & x ) override
    {
      if(svd_ptr) {
        x = svd_ptr->solve(b);
      } else {
        std::string msg = "SVD has not been previously computed.";
        throw( std::runtime_error( msg ) );
      }
    };

  private:

    std::shared_ptr<Eigen::BDCSVD<MatrixXd>> svd_ptr;

};

// --------------------------------------------------------------------------------

class QRSolver : public LinearSolverBase
{
  public:

    QRSolver() : LinearSolverBase() { };

    ~QRSolver() { };

    bool is_factorized() override
    {
      if(qr_ptr) return true;
      return false;
    };

    void factorize ( MatrixXd & A ) override
    {
      Eigen::ColPivHouseholderQR<MatrixXd> qr;
      qr_ptr = std::make_shared<Eigen::ColPivHouseholderQR<MatrixXd>>(qr.compute(A));
    };

    void solve( MatrixXd & A, MatrixXd & b, MatrixXd & x ) override
    {
      factorize(A);
      solve(b, x);
    };

    void solve( MatrixXd & b, MatrixXd & x ) override
    {
      if(qr_ptr) {
        x = qr_ptr->solve(b);
      } else {
        std::string msg = "QR has not been previously computed.";
        throw( std::runtime_error( msg ) );
      }
    };

  private:

    std::shared_ptr<Eigen::ColPivHouseholderQR<MatrixXd>> qr_ptr;

};

// --------------------------------------------------------------------------------

/**
 * \brief The CholeskySolver class can be used to solve Ax = b,
 * provided that A is a symmetric matrix.
 */
class CholeskySolver : public LinearSolverBase
{
  public:

    CholeskySolver() : LinearSolverBase() { };

    ~CholeskySolver() { };

    bool is_factorized() override
    {
      if(ldlt_ptr) return true;
      return false;
    };

    void factorize ( MatrixXd & A ) override
    {
      Eigen::LDLT<MatrixXd> ldlt;
      ldlt_ptr = std::make_shared<Eigen::LDLT<MatrixXd>>(ldlt.compute(A));
    };

    void solve( MatrixXd & A, MatrixXd & b, MatrixXd & x ) override
    {
      factorize(A);
      solve(b, x);
    };

    void solve( MatrixXd & b, MatrixXd & x ) override
    {
      if(ldlt_ptr) {
        x = ldlt_ptr->solve(b);
      } else {
        std::string msg = "Cholesky has not been previously computed.";
        throw( std::runtime_error( msg ) );
      }
    };

  private:

    std::shared_ptr<Eigen::LDLT<MatrixXd>> ldlt_ptr;

};

} // namespace util
} // namespace dakota

#endif //LINEAR_SOLVERS_PECOS_SRC_HPP
