/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


// Includes

#include <boost/test/minimal.hpp> // Boost.Test
#include "../../util/CommonUtils.hpp"
#include "PolynomialRegression.hpp"
#include "LinearSolvers.hpp"

// Namespace

using namespace dakota::surrogates;
using namespace Eigen;

namespace {

// Utilities

MatrixXd create_single_feature_matrix()
{
  MatrixXd single_feature_matrix(7, 1);
  single_feature_matrix << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7;
  return single_feature_matrix;
}

MatrixXd create_multiple_features_matrix()
{
  MatrixXd multiple_features_matrix(7, 3);
  multiple_features_matrix << 0.1, 0.2, 0.5,
                              1, 3, 6,
                              10, 20, 50,
                              100, 300, 700,
                              1000, 3000, 8000,
                              10000, 30000, 40000,
                              100000, 500000, 700000;
  return multiple_features_matrix;
}

// Unit tests

// --------------------------------------------------------------------------------

void PolynomialRegressionSurrogate_getters_and_setters()
{
  PolynomialRegression pr(1);
  MatrixXd samples  = create_multiple_features_matrix();
  MatrixXd response = create_single_feature_matrix();

  pr.set_samples(samples);
  pr.set_response(response);
  pr.set_polynomial_order(2);

  BOOST_CHECK(matrix_equals(pr.get_samples(),  samples,  1.0e-4));
  BOOST_CHECK(matrix_equals(pr.get_response(), response, 1.0e-4));
  BOOST_CHECK(pr.get_polynomial_order() == 2);
}

// --------------------------------------------------------------------------------

void PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE scaler_type)
{
  VectorXd line_vector = VectorXd::LinSpaced(20,0,1); // size, low, high
  VectorXd response    = VectorXd::LinSpaced(20,0,1);
  response = (response.array() + 2.0).matrix(); // +2.0 because the line's y-intercept is 2.0

  PolynomialRegression pr(2); //2 terms for straight line
  pr.set_samples(line_vector);
  pr.set_response(response);
  pr.set_scaler_type(scaler_type);
  pr.set_solver(dakota::util::SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION);
  pr.build_surrogate();

  const VectorXd& polynomial_coeffs = pr.get_polynomial_coeffs();
  const double polynomial_intercept = pr.get_polynomial_intercept();

  double expected_constant_term = 2.0; // unscaled
  double expected_first_term = 1.0; // unscaled
  if( scaler_type == dakota::util::SCALER_TYPE::NORMALIZATION ||
      scaler_type == dakota::util::SCALER_TYPE::STANDARDIZATION )
    expected_constant_term = 2.5; // scaled
  if( scaler_type == dakota::util::SCALER_TYPE::STANDARDIZATION )
    expected_first_term = 0.303488; // scaled

  double actual_constant_term = polynomial_coeffs(0);
  double actual_first_term = polynomial_coeffs(1);

  BOOST_CHECK(std::abs(expected_constant_term - actual_constant_term) < 1.0e-4);
  BOOST_CHECK(std::abs(expected_first_term    - actual_first_term) < 1.0e-4);
  BOOST_CHECK(polynomial_intercept < 1.0e-4); // test for zero

  VectorXd unscaled_eval_pts = VectorXd::LinSpaced(100,0,1);
  MatrixXd expected_approx_values(100, 1);
  expected_approx_values <<
    2.0,     2.0101,  2.0202,  2.0303,  2.0404,  2.05051, 2.06061, 2.07071, 2.08081, 2.09091,
    2.10101, 2.11111, 2.12121, 2.13131, 2.14141, 2.15152, 2.16162, 2.17172, 2.18182, 2.19192,
    2.20202, 2.21212, 2.22222, 2.23232, 2.24242, 2.25253, 2.26263, 2.27273, 2.28283, 2.29293,
    2.30303, 2.31313, 2.32323, 2.33333, 2.34343, 2.35354, 2.36364, 2.37374, 2.38384, 2.39394,
    2.40404, 2.41414, 2.42424, 2.43434, 2.44444, 2.45455, 2.46465, 2.47475, 2.48485, 2.49495,
    2.50505, 2.51515, 2.52525, 2.53535, 2.54545, 2.55556, 2.56566, 2.57576, 2.58586, 2.59596,
    2.60606, 2.61616, 2.62626, 2.63636, 2.64646, 2.65657, 2.66667, 2.67677, 2.68687, 2.69697,
    2.70707, 2.71717, 2.72727, 2.73737, 2.74747, 2.75758, 2.76768, 2.77778, 2.78788, 2.79798,
    2.80808, 2.81818, 2.82828, 2.83838, 2.84848, 2.85859, 2.86869, 2.87879, 2.88889, 2.89899,
    2.90909, 2.91919, 2.92929, 2.93939, 2.94949, 2.9596,  2.9697,  2.9798,  2.9899,  3.0;

  MatrixXd actual_approx_values;
  pr.surrogate_value(unscaled_eval_pts, actual_approx_values); 

  BOOST_CHECK(matrix_equals(actual_approx_values, expected_approx_values, 1.0e-5));
}

} // namespace

// --------------------------------------------------------------------------------

int test_main( int argc, char* argv[] ) // note the name!
{
  PolynomialRegressionSurrogate_getters_and_setters();
  PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE::NONE);
  PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE::NORMALIZATION);
  PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE::STANDARDIZATION);

  int run_result = 0;
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}

// --------------------------------------------------------------------------------
