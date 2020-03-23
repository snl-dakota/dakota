/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "Surrogate.hpp"

namespace dakota {
namespace surrogates {

Surrogate::Surrogate() : numQOI(0) {}

Surrogate::~Surrogate(){}

/* returns num_samples by num_vars for a given qoi */
void Surrogate::gradient(const MatrixXd &samples, MatrixXd &gradient, int qoi) {
  throw(std::string("This Surrogate type does not provide gradients"));
}

/* returns num_vars by num_vars for a single sample and a given qoi */
void Surrogate::hessian(const MatrixXd &samples, MatrixXd &hessian, int qoi) {
  throw(std::string("This Surrogate type does not provide Hessians"));
}

void Surrogate::set_options(const ParameterList &options) {
  configOptions = options;
}

void Surrogate::get_options(ParameterList &options) {
  options = configOptions;
}



} // namespace surrogates
} // namespace dakota

