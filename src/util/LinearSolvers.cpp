/*  _______________________________________________________________________

    PECOS: Parallel Environment for Creation Of Stochastics
    Copyright (c) 2011, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Pecos directory.
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
      return std::make_shared<LUSolver>();

    case SOLVER_TYPE::QR_LEAST_SQ_REGRESSION :
      return std::make_shared<QRSolver>();

    case SOLVER_TYPE::LU_LEAST_SQ_REGRESSION :
      return std::make_shared<LUSolver>();

    default :
      throw(std::runtime_error("Unknown solver type in solver_factory."));
  }
}

}  // namespace util
}  // namespace dakota
