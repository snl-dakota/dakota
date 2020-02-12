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
  const VectorXd & get_response() const;
  int get_polynomial_order() const;
  bool get_scaling() const;
  const VectorXd & get_polynomial_coeffs() const;
  double get_polynomial_intercept() const;
  const util::LinearSolverBase & get_solver() const;

  // Setters

  void set_samples(const MatrixXd & samples_);
  void set_response(const VectorXd & response_);
  void set_polynomial_order(int polynomial_order_);
  void set_scaling(bool scaling_);
  void set_solver(util::SOLVER_TYPE solver_type_);

  // Surrogate

  void build_surrogate();
  void surrogate_value(const MatrixXd &eval_points, MatrixXd &approx_values);

private:

  // Input fields
  const int num_terms;
  std::shared_ptr<MatrixXd> samples;
  std::shared_ptr<VectorXd> response;
  int polynomial_order;
  bool scaling;
  std::shared_ptr<util::LinearSolverBase> solver;

  // Polynomial surrogate fields
  std::shared_ptr<VectorXd> polynomial_coeffs;
  double polynomial_intercept;
};
} // namespace surrogates
} // namespace dakota

#endif
