/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_SURROGATE_HPP
#define DAKOTA_SURROGATES_SURROGATE_HPP

#include <Eigen/Dense>
#include "Teuchos_ParameterList.hpp"

using Eigen::MatrixXd;
using Teuchos::ParameterList;

namespace dakota {
namespace surrogates {

class Surrogate {

public:

  /// Default constructor
  Surrogate();

  /// Default destructor
  virtual ~Surrogate();

  /* samples is num_samples by num_vars */

  /* returns num_samples by num_qoi */
  virtual void value(const MatrixXd &samples, MatrixXd &value) = 0;

  /* returns num_samples by num_vars for a given qoi */
  virtual void gradient(const MatrixXd &samples, MatrixXd &gradient, int qoi);

  /* returns num_vars by num_vars for a single sample and a given qoi */
  virtual void hessian(const MatrixXd &samples, MatrixXd &hessian, int qoi);

  virtual void set_options(const ParameterList &options);

  virtual void get_options(ParameterList &options);

protected:
  int numSamples;
  int numVariables;
  int numQOI;
  /// Default Key/value options to configure the surrogate
  ParameterList defaultConfigOptions;
  /// Key/value options to configure the surrogate - will override default opts
  ParameterList configOptions;

};

} // namespace surrogates
} // namespace dakota
#endif // include guard
