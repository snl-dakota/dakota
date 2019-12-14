/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_GP_OBJECTIVE_HPP
#define DAKOTA_SURROGATES_GP_OBJECTIVE_HPP

#include <ROL_Objective.hpp>
#include <ROL_StdVector.hpp>
#include <ROL_Types.hpp>
#include "GaussianProcess.hpp"
#include "util_data_types.hpp"

using V = ROL::Vector<double>;
using SV = ROL::StdVector<double>;

namespace dakota {
namespace surrogates {

/**
 *  \brief ROL objective function for the Gaussian Process (GP)
 *  surrogate.
 *
 */
class GP_Objective : public ROL::Objective<double> {


  public:

/**
 *  \brief Constructor for GP_Objective.
 *  \param[in] gp_model Pointer to the GaussianProcess surrogate.
 *
 */
    GP_Objective(GaussianProcess* gp_model);
    ~GP_Objective();

/**
 *  \brief Get the value of the objective function at a point.
 *  \param[in] p ROL vector of parameters.
 *  \param[in] tol Tolerance for inexact evaluation? (not used).
 *
 */
    double value(const V& p, double& tol);
/**
 *  \brief Get the gradient of the objective function at a point.
 *  \param[out] g Gradient of the objective function.
 *  \param[in] p ROL vector of parameters.
 *  \param[in] tol Tolerance for inexact evaluation? (not used).
 *
 */
    void gradient(V& g, const V& p, double&);

  private:

    /// Pointer to the GaussianProcess surrogate.
    GaussianProcess* gp;

    /// Number of optimization variables.
    int nopt;
    /// Previously computed value of the objective function.
    double Jold;
    /// Previously computed gradient of the objective function.
    VectorXd grad_old;
    /// Previous value of the parameter vector.
    VectorXd pold;
    /// Tolerance for l2 difference between parameter vectors. Should
    /// smaller than ROL's stepnorm but bigger than machine epsilon.
    double difftol = 1.0e-15;
/**
 *  \brief Convert a const ROL Vector to a ROL::Ptr<const std::vector>
 *  \param[in] vec const ROL vector
 *
 */
    ROL::Ptr<const std::vector<double> > getVector(const V& vec) {
      return dynamic_cast<const SV&>(vec).getVector();
    }
/**
 *  \brief Convert a ROL Vector to a ROL::Ptr<std::vector>
 *  \param[in] vec ROL vector
 *
 */
    ROL::Ptr<std::vector<double> > getVector(V& vec) {
      return dynamic_cast<SV&>(vec).getVector();
    }

/**
 *  \brief Compute the l2 norm of the difference between new
 *  and old parameter vectors.
 *  \param[in] pnew New value of the parameter vector.
 *
 */
    bool pdiff(const std::vector<double>& pnew);
};

}  // namespace surrogates
}  // namespace dakota


#endif  // include guard
