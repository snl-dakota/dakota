/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "LinearSolvers.hpp"
#include <boost/assign.hpp>
#include <boost/bimap.hpp>


namespace dakota {
namespace util {

using BimapSolvertypeStr = boost::bimap<SOLVER_TYPE, std::string>;

static BimapSolvertypeStr type_name_bimap =
  boost::assign::list_of< BimapSolvertypeStr::relation >
  (SOLVER_TYPE::CHOLESKY, "cholesky")
  (SOLVER_TYPE::EQ_CONS_LEAST_SQ_REGRESSION, "equality-constrained lsq regression")
  (SOLVER_TYPE::LASSO_REGRESSION, "lasso regression")
  (SOLVER_TYPE::LEAST_ANGLE_REGRESSION, "least angle regression")
  (SOLVER_TYPE::LU, "LU")
  (SOLVER_TYPE::ORTHOG_MATCH_PURSUIT, "orthogonal matching pursuit")
  (SOLVER_TYPE::QR_LEAST_SQ_REGRESSION, "QR lsq regression")
  (SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION, "SVD")
  ;


SOLVER_TYPE LinearSolverBase::solver_type(const std::string& solver_name)
{
  BimapSolvertypeStr::right_const_iterator rc_iter
    = type_name_bimap.right.find(solver_name);
  if (rc_iter == type_name_bimap.right.end()) {
    throw std::runtime_error("Invalid solver_name");
  }
  return rc_iter->second;
}

std::shared_ptr<LinearSolverBase>
solver_factory(SOLVER_TYPE type)
{
  switch (type)
  {
    case SOLVER_TYPE::CHOLESKY :
      return std::make_shared<CholeskySolver>();

    case SOLVER_TYPE::EQ_CONS_LEAST_SQ_REGRESSION :
      throw(std::runtime_error("EQ_CONS_LEAST_SQ_REGRESSION not yet implemented."));

    case SOLVER_TYPE::LASSO_REGRESSION :
      throw(std::runtime_error("LASSO_REGRESSION not yet implemented."));

    case SOLVER_TYPE::LEAST_ANGLE_REGRESSION :
      throw(std::runtime_error("LEAST_ANGLE_REGRESSION not yet implemented."));

    case SOLVER_TYPE::LU :
      return std::make_shared<LUSolver>();      

    case SOLVER_TYPE::ORTHOG_MATCH_PURSUIT :
      throw(std::runtime_error("ORTHOG_MATCH_PURSUIT not yet implemented."));      

    case SOLVER_TYPE::QR_LEAST_SQ_REGRESSION :
      return std::make_shared<QRSolver>();

    case SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION :
      return std::make_shared<SVDSolver>();

    default :
      throw(std::runtime_error("Unknown solver type in solver_factory."));
  }
}

}  // namespace util
}  // namespace dakota
