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
#include "../util/LinearSolvers.hpp"

namespace dakota {
namespace surrogates {

class PolynomialRegression {

public:

  // Constructor

  PolynomialRegression(const int num_terms);

  // Destructor

  ~PolynomialRegression();

  // Getters

  const MatrixXd & get_samples() const;
  const MatrixXd & get_response() const;
  int get_polynomial_order() const;
  util::SCALER_TYPE get_scaler_type() const;
  const MatrixXd & get_polynomial_coeffs() const;
  double get_polynomial_intercept() const;
  const util::LinearSolverBase & get_solver() const;

  // Setters

  void set_samples(const MatrixXd & samples_);
  void set_response(const MatrixXd & response_);
  void set_polynomial_order(int polynomial_order_);
  void set_scaler_type(const util::SCALER_TYPE scaler_type);
  void set_solver(const util::SOLVER_TYPE solver_type_);

  // Surrogate

  void build_surrogate();
  void surrogate_value(const MatrixXd &eval_points, MatrixXd &approx_values);

private:

  // Input fields
  const int num_terms;
  std::shared_ptr<MatrixXd> samples;
  std::shared_ptr<MatrixXd> response;
  int polynomial_order;
  util::SCALER_TYPE scaler_type;
  std::shared_ptr<util::DataScaler> scaler;
  std::shared_ptr<util::LinearSolverBase> solver;

  // Polynomial surrogate fields
  std::shared_ptr<MatrixXd> polynomial_coeffs;
  double polynomial_intercept;
};
} // namespace surrogates
} // namespace dakota

#endif
