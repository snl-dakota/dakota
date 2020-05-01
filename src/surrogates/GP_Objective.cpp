/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "GP_Objective.hpp"

namespace dakota {
namespace surrogates {

/// Dakota alias for ROL Vector
using RolVec = ROL::Vector<double>;
/// Dakota alias for ROL StdVector
using RolStdVec = ROL::StdVector<double>;

GP_Objective::GP_Objective(GaussianProcess &gp_model) :
  gp(gp_model)
{
  nopt = gp.get_num_opt_variables();
  grad_old.resize(nopt);
  pold.resize(nopt);
  grad_old.setConstant(-5.0e99);
  pold.setConstant(5.0e99);
  Jold = -2.0;
}

GP_Objective::~GP_Objective() {}

double GP_Objective::value(const ROL::Vector<double> &p, double&) {
  ROL::Ptr<const std::vector<double> > xp = getVector(p);
  double obj_val;
  VectorXd grad(nopt);
  if (pdiff(*xp)) {
    gp.set_opt_params(*xp);
    gp.negative_marginal_log_likelihood(obj_val, grad);
    Jold = obj_val;
    grad_old = grad;
  }
  return Jold;
}

void GP_Objective::gradient(ROL::Vector<double> &g, const ROL::Vector<double> &p, double& tol) {
  ROL::Ptr<const std::vector<double> > xp = getVector(p);
  ROL::Ptr<std::vector<double> > gpointer = getVector(g);
  double obj_val;
  VectorXd grad(nopt);
  if (pdiff(*xp)) {
    gp.set_opt_params(*xp);
    gp.negative_marginal_log_likelihood(obj_val, grad);
    Jold = obj_val;
    for (int i = 0; i < grad.size(); ++i) {
      (*gpointer)[i] = grad(i);
    }
  }
  else {
    for (int i = 0; i < grad.size(); ++i) {
      (*gpointer)[i] = grad_old(i);
    }
  }
}

// parameter diffs
bool GP_Objective::pdiff(const std::vector<double> &pnew) {
  double diffnorm = 0.0;
  for (int i = 0; i < nopt; ++i) {
    diffnorm += pow(pnew[i] - pold(i),2.0);
    pold(i) = pnew[i];
  }
  diffnorm = sqrt(diffnorm);
  if (diffnorm < difftol)
    return false;
  else
    return true;
}


}  // namespace surrogates
}  // namespace dakota
