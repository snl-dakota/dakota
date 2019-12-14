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

namespace dakota {
namespace surrogates {

class GP_Objective : public ROL::Objective<double> {

  typedef ROL::Vector<double> V;
  typedef ROL::StdVector<double> SV;

  public:

    GP_Objective(GaussianProcess* gp_model);
    ~GP_Objective();

    double value(const V& p, double& tol);
    void gradient(V& g, const V& p, double&);

  private:

    GaussianProcess* gp;

    int nopt;
    double Jold;
    VectorXd grad_old;
    VectorXd pold;
    double difftol = 1.0e-15; /* should be smaller than stepnorm,
    but bigger than machine epsilon */

    ROL::Ptr<const std::vector<double> > getVector(const V& vec) {
      return dynamic_cast<const SV&>(vec).getVector();
    }
    ROL::Ptr<std::vector<double> > getVector(V& vec) {
      return dynamic_cast<SV&>(vec).getVector();
    }

    bool pdiff(const std::vector<double>&);
};

}  // namespace surrogates
}  // namespace dakota


#endif  // include guard
