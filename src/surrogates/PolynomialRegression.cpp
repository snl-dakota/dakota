/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "PolynomialRegression.hpp"

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

// Setters

void PolynomialRegression::set_samples(const MatrixXd &samples_) { samples = samples_; }

void PolynomialRegression::set_response(const MatrixXd &response_) { response = response_; }

void PolynomialRegression::set_polynomial_order(const int polynomial_order_) { polynomial_order = polynomial_order_; }

// Surrogate

void PolynomialRegression::build_surrogate() {
	std::cout << "Not yet implemented." << std::endl;
}

void PolynomialRegression::value(const MatrixXd &samples, MatrixXd &approx_values) {
	std::cout << "Not yet implemented." << std::endl;
}

} // namespace surrogates
} // namespace dakota