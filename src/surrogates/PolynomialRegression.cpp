/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PolynomialRegression.hpp"
#include <cmath>

namespace dakota {
namespace surrogates {

// Constructor

PolynomialRegression::PolynomialRegression() {}

// Destructor

PolynomialRegression::~PolynomialRegression() {}

// Getters

MatrixXd& PolynomialRegression::get_samples() { return samples; }

MatrixXd& PolynomialRegression::get_response() { return response; }

int PolynomialRegression::get_polynomial_order() { return polynomial_order; }

bool PolynomialRegression::get_scaling() { return scaling; }

VectorXd& PolynomialRegression::get_polynomial_coeffs() { return polynomial_coeffs; }

double PolynomialRegression::get_polynomial_intercept() { return polynomial_intercept; }

// Setters

void PolynomialRegression::set_samples(const MatrixXd &samples_) { samples = samples_; }

void PolynomialRegression::set_response(const MatrixXd &response_) { response = response_; }

void PolynomialRegression::set_polynomial_order(const int polynomial_order_) { polynomial_order = polynomial_order_; }

void PolynomialRegression::set_scaling(const bool scaling_) { scaling = scaling_; }

// Surrogate

void PolynomialRegression::build_surrogate() {
  const int NUM_SAMPLES = samples.size();
  const int NUM_RESPONSES = response.size();
  const int NUM_VARS = 1;
  const int NUM_TERMS = 2;

  if(NUM_SAMPLES == 0 || NUM_RESPONSES == 0) {
  	return;
  }

  // Generate the basis matrix
  MatrixXd unscaled_basis_matrix = MatrixXd::Zero(NUM_SAMPLES, NUM_TERMS);
  for(int i = 0; i < NUM_SAMPLES; i++) {
    for(int j = 0; j < NUM_TERMS; j++) {
      unscaled_basis_matrix(i, j) = std::pow(samples(i), (double)j);
    }
  }
  //std::cout << "unscaled_basis_matrix:" << std::endl;
  //std::cout << unscaled_basis_matrix << std::endl;

  // Standardize the basis matrix
  MatrixXd scaled_basis_matrix;
  if(scaling) {
    //TODO:
    // scaler = StandardScaler()
    // scaled_basis_matrix = scaler.fit_transform(unscaled_basis_matrix)
  } else {
  	scaled_basis_matrix = unscaled_basis_matrix;
  }

  // Compute the singular value decomposition of the basis matrix using SVD.
  polynomial_coeffs = scaled_basis_matrix.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(response);

  // Compute the intercept
  polynomial_intercept = 0.0;
  if(scaling) {
  	polynomial_intercept = response.mean() - (scaled_basis_matrix*polynomial_coeffs).mean();
  }
}

void PolynomialRegression::surrogate_value(const MatrixXd &eval_points, MatrixXd &approx_values) {
  const int NUM_EVAL_POINTS = eval_points.size();
  const int NUM_TERMS = 2;

  // Generate the basis matrix for the eval points
  MatrixXd unscaled_basis_matrix = MatrixXd::Zero(NUM_EVAL_POINTS, NUM_TERMS);
  for(int i = 0; i < NUM_EVAL_POINTS; i++) {
    for(int j = 0; j < NUM_TERMS; j++) {
      unscaled_basis_matrix(i, j) = std::pow(eval_points(i), (double)j);
    }
  }

  // Scale sample points if scaling was used.
  //TODO
  MatrixXd scaled_basis_matrix = unscaled_basis_matrix;

  // Find the polynomial regression values.
  approx_values = scaled_basis_matrix * polynomial_coeffs;
  approx_values = (approx_values.array() + polynomial_intercept).matrix();
}

} // namespace surrogates
} // namespace dakota