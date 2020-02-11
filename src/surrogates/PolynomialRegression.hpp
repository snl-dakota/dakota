/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_POLYNOMIAL_REGRESSION_HPP
#define DAKOTA_SURROGATES_POLYNOMIAL_REGRESSION_HPP

#include <iostream>
#include <memory>
#include "DataScaler.hpp"

namespace dakota {
namespace surrogates {

class PolynomialRegression {

public:

  // Constructor

  PolynomialRegression(const int num_terms);

  // Destructor

  ~PolynomialRegression();

  // Getters

  const MatrixXd get_samples();
  const MatrixXd get_response();
  int get_polynomial_order();
  bool get_scaling();
  const VectorXd get_polynomial_coeffs();
  double get_polynomial_intercept();

  // Setters

  void set_samples(const MatrixXd samples_);
  void set_response(const MatrixXd response_);
  void set_polynomial_order(const int polynomial_order_);
  void set_scaling(const bool scaling_);

  // Surrogate

  void build_surrogate();
  void surrogate_value(const MatrixXd &eval_points, MatrixXd &approx_values);

private:

  // Input fields
  const int num_terms;
  std::shared_ptr<MatrixXd> samples;
  std::shared_ptr<MatrixXd> response;
  int polynomial_order;
  bool scaling;

  // Polynomial surrogate fields
  std::shared_ptr<VectorXd> polynomial_coeffs;
  double polynomial_intercept;
};
} // namespace surrogates
} // namespace dakota

#endif