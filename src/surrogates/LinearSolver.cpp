/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "LinearSolver.hpp"

namespace dakota {
namespace surrogates {

LinearSolver::LinearSolver() {}
LinearSolver::~LinearSolver() {}

const VectorXd LinearSolver::solve(const MatrixXd& basis_matrix, const MatrixXd& responses) {
  return basis_matrix.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(responses);
}

} // namespace surrogates
} // namespace dakota