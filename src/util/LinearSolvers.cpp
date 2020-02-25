/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "LinearSolvers.hpp"

namespace dakota {
namespace util {

std::shared_ptr<LinearSolverBase>
solver_factory(SOLVER_TYPE type)
{
  switch (type)
  {
    case SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION :
      return std::make_shared<SVDSolver>();

    case SOLVER_TYPE::QR_LEAST_SQ_REGRESSION :
      return std::make_shared<QRSolver>();

    case SOLVER_TYPE::LU :
      return std::make_shared<LUSolver>();

    case SOLVER_TYPE::CHOLESKY :
      return std::make_shared<CholeskySolver>();

    default :
      throw(std::runtime_error("Unknown solver type in solver_factory."));
  }
}

}  // namespace util
}  // namespace dakota
