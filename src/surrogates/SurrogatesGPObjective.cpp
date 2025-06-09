/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesGPObjective.hpp"

namespace dakota {
namespace surrogates {

GP_Objective::GP_Objective(GaussianProcess& gp_model) : gp(gp_model) {
  nopt = gp.get_num_opt_variables();
  grad_old.resize(nopt);
  pold.resize(nopt);
  grad_old.setConstant(-5.0e99);
  pold.setConstant(5.0e99);
  Jold = -2.0;
}

GP_Objective::~GP_Objective() {}

double GP_Objective::value(const ROL::Vector<double>& p, double& tol) {
  silence_unused_args(tol);
  const auto& x = as_VectorXd(p);
  double obj_val;
  VectorXd grad(nopt);
  gp.set_opt_params(x);
  gp.negative_marginal_log_likelihood(false, pdiff(x), obj_val, grad);
  return obj_val;
}

void GP_Objective::gradient(ROL::Vector<double>& g,
                            const ROL::Vector<double>& p, double& tol) {
  silence_unused_args(tol);
  
  const auto& x = as_VectorXd(p);
  auto& gvec = as_VectorXd(g);
  
  double obj_val;
  gp.set_opt_params(x);
  gp.negative_marginal_log_likelihood(true, pdiff(x), obj_val, gvec);
}

bool GP_Objective::pdiff(const Eigen::VectorXd& pnew) {
  double diffnorm = (pnew - pold).norm();
  pold = pnew;
  return diffnorm >= near_zero;
}

}  // namespace surrogates
}  // namespace dakota
