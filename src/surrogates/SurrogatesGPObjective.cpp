/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesGPObjective.hpp"

namespace dakota {
namespace surrogates {

/// Dakota alias for ROL Vector
using RolVec = ROL::Vector<double>;
/// Dakota alias for ROL StdVector
using RolStdVec = ROL::StdVector<double>;

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
  ROL::Ptr<const std::vector<double> > xp = getVector(p);
  double obj_val;
  VectorXd grad(nopt);
  gp.set_opt_params(*xp);
  gp.negative_marginal_log_likelihood(false, pdiff(*xp), obj_val, grad);
  return obj_val;
}

void GP_Objective::gradient(ROL::Vector<double>& g,
                            const ROL::Vector<double>& p, double& tol) {
  silence_unused_args(tol);
  ROL::Ptr<const std::vector<double> > xp = getVector(p);
  ROL::Ptr<std::vector<double> > gpointer = getVector(g);
  double obj_val;
  VectorXd grad(nopt);
  gp.set_opt_params(*xp);
  gp.negative_marginal_log_likelihood(true, pdiff(*xp), obj_val, grad);
  for (int i = 0; i < grad.size(); ++i) {
    (*gpointer)[i] = grad(i);
  }
}

bool GP_Objective::pdiff(const std::vector<double>& pnew) {
  double diffnorm = 0.0;
  for (int i = 0; i < nopt; ++i) {
    diffnorm += pow(pnew[i] - pold(i), 2.0);
    pold(i) = pnew[i];
  }
  diffnorm = sqrt(diffnorm);
  if (diffnorm < near_zero)
    return false;
  else
    return true;
}

}  // namespace surrogates
}  // namespace dakota
