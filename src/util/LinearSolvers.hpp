/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_LINEAR_SOLVERS_HPP
#define DAKOTA_UTIL_LINEAR_SOLVERS_HPP

#include "util_data_types.hpp"

#include <memory>

namespace dakota {
namespace util {

// --------------------------------------------------------------------------------

/**
 *  \brief The LinearSolverBase class serves as an API for derived solvers.
 */
class LinearSolverBase
{
  public:

    /// How best to Doxygenate class enums? RWH
    enum class SOLVER_TYPE { CHOLESKY,
                             EQ_CONS_LEAST_SQ_REGRESSION,
                             LASSO_REGRESSION,
                             LEAST_ANGLE_REGRESSION,
                             LU,
                             ORTHOG_MATCH_PURSUIT,
                             QR_LEAST_SQ_REGRESSION,
                             SVD_LEAST_SQ_REGRESSION
                           };

    /// Constructor
    LinearSolverBase();

    /// Destructor
    ~LinearSolverBase();

    /**
     *  \brief Convert solver name to enum type
     *  \param[in] solver_name LinearSolverBase name to map
     *  \returns Corresponding LinearSolverBase enum
     */
    static SOLVER_TYPE solver_type(const std::string& solver_name);

    /**
     * \brief Query to determine if the matrix of the solver has been factored.
     */
    virtual bool is_factorized() const;

    /**
     * \brief Perform the matrix factorization for the linear solver matrix.
     *
     * \param[in] mat The incoming matrix to factorize.
     */
    virtual void factorize(const MatrixXd &mat);

    /**
     * \brief Find a solution to linear problem.
     *
     * \param[in] lhs The linear system left-hand-side matrix.
     * \param[in] rhs The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    virtual void solve(const MatrixXd &lhs, const MatrixXd &rhs, MatrixXd &x);

    /**
     * \brief Find a solution to linear problem where the LHS is already factorized.
     *
     * \param[in] rhs The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    virtual void solve(const MatrixXd &rhs, MatrixXd &x);
};

/**
 * \brief Free function to construct LinearSolverBase
 *
 * \param[in] type  Which solver to construct
 * \returns   Shared pointer to a LinearSolverBase
 */
std::shared_ptr<LinearSolverBase> solver_factory(LinearSolverBase::SOLVER_TYPE type);

// --------------------------------------------------------------------------------

class LUSolver : public LinearSolverBase
{
  public:

    /// Constructor
    LUSolver();

    /// Destructor
    ~LUSolver();

    /**
     * \brief Query to determine if the matrix of the solver has been factored.
     */
    bool is_factorized() const override;

    /**
     * \brief Perform the matrix factorization for the linear solver matrix.
     *
     * \param[in] A The incoming matrix to factorize.
     */
    void factorize(const MatrixXd &A) override;

    /**
     * \brief Find the solution to Ax = b.
     *
     * \param[in] A The linear system left-hand-side matrix.
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &A, const MatrixXd &b, MatrixXd &x) override;

    /**
     * \brief Find the solution to Ax = b when A is already factorized.
     *
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &b, MatrixXd &x) override;

  private:

    std::shared_ptr<Eigen::FullPivLU<MatrixXd>> LU_Ptr;
};

// --------------------------------------------------------------------------------

class SVDSolver : public LinearSolverBase
{
  public:

    /// Constructor
    SVDSolver();

    /// Destructor
    ~SVDSolver();

    /**
     * \brief Query to determine if the matrix of the solver has been factored.
     */
    bool is_factorized() const override;

    /**
     * \brief Perform the matrix factorization for the linear solver matrix.
     *
     * \param[in] A The incoming matrix to factorize.
     */
    void factorize(const MatrixXd &A) override;

    /**
     * \brief Find a solution to Ax = b.
     *
     * \param[in] A The linear system left-hand-side matrix.
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &A, const MatrixXd &b, MatrixXd &x) override;

    /**
     * \brief Find a solution to Ax = b when A is already factorized.
     *
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &b, MatrixXd & x) override;

  private:

    std::shared_ptr<Eigen::BDCSVD<MatrixXd>> SVD_Ptr;
};

// --------------------------------------------------------------------------------

class QRSolver : public LinearSolverBase
{
  public:

    /// Constructor
    QRSolver();

    /// Destructor
    ~QRSolver();

    /**
     * \brief Query to determine if the matrix of the solver has been factored.
     */
    bool is_factorized() const override;

    /**
     * \brief Perform the matrix factorization for the linear solver matrix.
     *
     * \param[in] mat The incoming matrix to factorize.
     */
    void factorize(const MatrixXd &A) override;

    /**
     * \brief Find the solution to (A^T*A)x = A^T*b .
     *
     * \param[in] A The matrix for the QR decomposition.
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &A, const MatrixXd &b, MatrixXd &x) override;

    /**
     * \brief Find a solution to (A^T*A)x = A^T*b when A is already factorized.
     *
     * \param[in] rhs The linear system right-hand-side (multi-)vector.
     * \param[in] x   The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &b, MatrixXd &x) override;

  private:

    std::shared_ptr<Eigen::ColPivHouseholderQR<MatrixXd>> QR_Ptr;

};

// --------------------------------------------------------------------------------

/**
 * \brief The CholeskySolver class can be used to solve Ax = b,
 * provided that A is a symmetric matrix.
 */
class CholeskySolver : public LinearSolverBase
{
  public:

    /// Constructor
    CholeskySolver();

    /// Destructor
    ~CholeskySolver();

    /**
     * \brief Query to determine if the matrix of the solver has been factored.
     */
    bool is_factorized() const override;

    /**
     * \brief Perform the matrix factorization for the linear solver matrix.
     *
     * \param[in] A The incoming matrix to factorize.
     */
    void factorize(const MatrixXd &A) override;

    /**
     * \brief Find a solution to Ax = b.
     *
     * \param[in] A The linear system left-hand-side matrix.
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &A, const MatrixXd &b, MatrixXd &x) override;

    /**
     * \brief Find a solution to Ax = b when A is already factorized.
     *
     * \param[in] b The linear system right-hand-side (multi-)vector.
     * \param[in] x The linear system solution (multi-)vector.
     */
    void solve(const MatrixXd &b, MatrixXd &x) override;

  private:

    /// Cached LDL^T factorization
    std::shared_ptr<Eigen::LDLT<MatrixXd>> LDLT_Ptr;
};

} // namespace util
} // namespace dakota

#endif // include guard
