/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
  silence_unused_args(samples, gradient, qoi);
  throw(std::runtime_error("Surrogate does not implement gradient(...)"));
}

/* returns num_vars by num_vars for a single sample and a given qoi */
void Surrogate::hessian(const MatrixXd &samples, MatrixXd &hessian, int qoi) {
  silence_unused_args(samples, hessian, qoi);
  throw(std::runtime_error("Surrogate does not implement hessian(...)"));
}

void Surrogate::set_options(const ParameterList &options) {
  configOptions = options;
}

void Surrogate::get_options(ParameterList &options) {
  options = configOptions;
}



} // namespace surrogates
} // namespace dakota

