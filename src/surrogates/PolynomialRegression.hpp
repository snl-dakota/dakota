/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_POLYNOMIAL_REGRESSION_HPP
#define DAKOTA_SURROGATES_POLYNOMIAL_REGRESSION_HPP

#include <iostream>
#include "DataScaler.hpp"

namespace dakota {
namespace surrogates {

class PolynomialRegression {

public:

  // Constructor

  PolynomialRegression();

  // Destructor

  ~PolynomialRegression();

  // Getters

  MatrixXd get_samples();
  MatrixXd get_response();
  int get_polynomial_order();

  // Setters

  void set_samples(const MatrixXd &samples_);
  void set_response(const MatrixXd &response_);
  void set_polynomial_order(const int polynomial_order_);

  // Surrogate

  void build_surrogate();
  void value(const MatrixXd &samples, MatrixXd &approx_values);

private:

  // Fields
  MatrixXd samples;
  MatrixXd response;
  int polynomial_order;

};
} // namespace surrogates
} // namespace dakota

#endif