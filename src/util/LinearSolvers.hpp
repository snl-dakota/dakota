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

    /**
     * \brief Find a solution to Ax = b
     */
    virtual void solve( MatrixXd &, VectorXd &, VectorXd & )
    {
      std::string msg = "solve() Has not been implemented for this class.";
      throw( std::runtime_error( msg ) );
    };
};

// --------------------------------------------------------------------------------

enum class SOLVER_TYPE {
                         SVD_LEAST_SQ_REGRESSION ,
                         QR_LEAST_SQ_REGRESSION  ,
                         LU_LEAST_SQ_REGRESSION
                       };

std::shared_ptr<LinearSolverBase> solver_factory(SOLVER_TYPE);

// --------------------------------------------------------------------------------

class LUSolver : public LinearSolverBase
{
  public:

    LUSolver() : LinearSolverBase() { };

    ~LUSolver() { };

    void solve( MatrixXd & A, VectorXd & b, VectorXd & x ) override
    {
      Eigen::FullPivLU<MatrixXd> lu(A);
      x = lu.solve(b);
      //std::cout << "Here's LU: " << lu.matrixLU() << std::endl;
    };
};

// --------------------------------------------------------------------------------

class SVDSolver : public LinearSolverBase
{
  public:

    SVDSolver() : LinearSolverBase() { };

    ~SVDSolver() { };

    void solve( MatrixXd & A, VectorXd & b, VectorXd & x ) override
    {
      x = A.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(b);
    };

};

// --------------------------------------------------------------------------------

class QRSolver : public LinearSolverBase
{
  public:

    QRSolver() : LinearSolverBase() { };

    ~QRSolver() { };

    void solve( MatrixXd & A, VectorXd & b, VectorXd & x ) override
    {
      x = A.colPivHouseholderQr().solve(b);
    };

};


} // namespace util
} // namespace dakota

#endif //LINEAR_SOLVERS_PECOS_SRC_HPP
