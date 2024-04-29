/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GP_OBJECTIVE_HPP
#define DAKOTA_SURROGATES_GP_OBJECTIVE_HPP

#include "SurrogatesGaussianProcess.hpp"
#include "util_data_types.hpp"

#include <ROL_Objective.hpp>
#include <ROL_StdVector.hpp>
#include <ROL_Types.hpp>

namespace dakota {
namespace surrogates {

/**
 *  \brief ROL objective function for the Gaussian Process (GP)
 *  surrogate.
 *
 */
class GP_Objective : public ROL::Objective<double> {
 public:
  // ------------------------------------------------------------
  // Constructors and destructors

  /**
   *  \brief Constructor for GP_Objective.
   *  \param[in] gp_model Reference to the GaussianProcess surrogate.
   *
   */
  GP_Objective(GaussianProcess& gp_model);
  ~GP_Objective();

  // ------------------------------------------------------------
  // Public utility functions

  /**
   *  \brief Get the value of the objective function at a point.
   *  \param[in] p ROL vector of parameters.
   *  \param[in] tol Tolerance for inexact evaluation (not used here).
   *
   */
  double value(const ROL::Vector<double>& p, double& tol);

  /**
   *  \brief Get the gradient of the objective function at a point.
   *  \param[out] g Gradient of the objective function.
   *  \param[in] p ROL vector of parameters.
   *  \param[in] tol Tolerance for inexact evaluation (not used here).
   *
   */
  void gradient(ROL::Vector<double>& g, const ROL::Vector<double>& p,
                double& tol);

 private:
  // ------------------------------------------------------------
  // Private utility functions

  /**
   *  \brief Compute the l2 norm of the difference between new
   *  and old parameter vectors.
   *  \param[in] pnew New value of the parameter vector.
   *
   */
  bool pdiff(const std::vector<double>& pnew);

  /**
   *  \brief Convert a const ROL Vector to a ROL::Ptr<const std::vector>
   *  \param[in] vec const ROL vector
   *
   */
  ROL::Ptr<const std::vector<double> > getVector(
      const ROL::Vector<double>& vec) {
    return dynamic_cast<const ROL::StdVector<double>&>(vec).getVector();
  }

  /**
   *  \brief Convert a ROL Vector to a ROL::Ptr<std::vector>
   *  \param[in] vec ROL vector
   *
   */
  ROL::Ptr<std::vector<double> > getVector(ROL::Vector<double>& vec) {
    return dynamic_cast<ROL::StdVector<double>&>(vec).getVector();
  }

  // ------------------------------------------------------------
  // Private member variables

  /// Pointer to the GaussianProcess surrogate.
  GaussianProcess& gp;
  /// Number of optimization variables.
  int nopt;
  /// Previously computed value of the objective function.
  double Jold;
  /// Previously computed gradient of the objective function.
  VectorXd grad_old;
  /// Previous value of the parameter vector.
  VectorXd pold;
};

}  // namespace surrogates
}  // namespace dakota

#endif  // include guard
