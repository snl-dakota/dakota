/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


// Includes

#include <boost/test/minimal.hpp> // Boost.Test
#include "../../util/CommonUtils.hpp"
#include "../SurrogatesTools.hpp"
#include "PolynomialRegression.hpp"
#include "LinearSolvers.hpp"

#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>

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
  PolynomialRegression pr(1, 0);
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

  PolynomialRegression pr(/* degree */ 1, /* num_vars */ 1);
  pr.set_samples(line_vector);
  pr.set_response(response);
  pr.set_scaler_type(scaler_type);
  pr.set_solver(dakota::util::SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION);
  pr.build_surrogate();

  const MatrixXd& polynomial_coeffs = pr.get_polynomial_coeffs();
  double polynomial_intercept = pr.get_polynomial_intercept();

  double expected_constant_term = 2.0;        // unscaled intercept via coeffs array
  double expected_first_term =    1.0;        // unscaled slope via coeffs array
  double expected_polynomial_intercept = 0.0;
  if( scaler_type == dakota::util::SCALER_TYPE::MEAN_NORMALIZATION ||
      scaler_type == dakota::util::SCALER_TYPE::STANDARDIZATION )
  {
    expected_constant_term = 0.0;
    expected_polynomial_intercept = 2.5;
  }
  if( scaler_type == dakota::util::SCALER_TYPE::STANDARDIZATION )
    expected_first_term = 0.303488; // scaled

  double actual_constant_term = polynomial_coeffs(0, 0);
  double actual_first_term = polynomial_coeffs(1, 0);

  BOOST_CHECK(std::abs(expected_constant_term        - actual_constant_term) < 1.0e-4);
  BOOST_CHECK(std::abs(expected_first_term           - actual_first_term)    < 1.0e-4);
  BOOST_CHECK(std::abs(expected_polynomial_intercept - polynomial_intercept) < 1.0e-4);

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

// --------------------------------------------------------------------------------

// More utilities to aid unit testing and which migh tlater get parceled into
// othe util classes - RWH
void get_samples(int num_vars, int num_samples, MatrixXd & samples)
{
  samples.resize(num_samples, num_vars);

  //std::random_device rd;
  //std::mt19937 gen(rd());

  int seed = 1337;
  //std::mt19937 gen(seed);
  //std::uniform_real_distribution<double> dis(-1.0, 1.0);

  // This choice allows consistent comparison with pecos/surrogates/models/src/RegressionBuilder
  typedef boost::uniform_real<> NumberDistribution; 
  typedef boost::mt19937 RNG; 
  typedef boost::variate_generator<RNG&,NumberDistribution> Generator; 

  boost::uniform_real<double> distribution( -1.0, 1.0 ); 
  boost::mt19937 generator; 
  generator.seed( seed );
  boost::variate_generator<boost::mt19937, NumberDistribution> numberGenerator( generator, distribution ); 

  for( int j=0; j<num_vars; ++j )
    for( int i=0; i<num_samples; ++i )
      //samples(i,j) = dis(gen);
      samples(i,j) = numberGenerator();
}

  // ------------------------------------------------------------

void
another_additive_quadratic_function(const MatrixXd & samples, MatrixXd & func_vals)
{
  func_vals.resize(samples.rows(),1); // hard-coded to one response for now - RWH
  const int num_vars = samples.cols();

  for( int i=0; i<samples.rows(); ++i )
  {
    func_vals(i,0) = 1.0;
    for (int j=0; j<num_vars; ++j)
    {
      double x = (2*samples(i,j)-1);
      func_vals(i,0) += x*x+2.0*x;
    }
  }
}

 // ------------------------------------------------------------

void
cubic_bivariate_function(const MatrixXd & samples, MatrixXd & func_vals)
{
  func_vals.resize(samples.rows(),1);
  const int num_vars = samples.cols();

  for( int i=0; i<samples.rows(); ++i )
  {
    double x = samples(i,0);
    double y = samples(i,1);
    func_vals(i,0) =
      1 + x + y + (x*y) +
      (std::pow(x,2) * std::pow(y,2)) +
      (std::pow(x,2) * y) +
      (std::pow(y,2) * x) +
      std::pow(x,3) + std::pow(y,3);
  }
}


  // ------------------------------------------------------------

void
PolynomialRegressionSurrogate_multivariate_regression_builder()
{
  int num_vars    = 2,
      num_samples = 20,
      degree      = 3;

  // Generate realizations of function variables
  MatrixXd samples, responses;
  get_samples(num_vars, num_samples, samples);
  another_additive_quadratic_function(samples, responses);

  PolynomialRegression pr(degree, num_vars);
  pr.set_samples(samples);
  pr.set_response(responses);
  pr.set_scaler_type(dakota::util::SCALER_TYPE::NONE);
  pr.set_solver(dakota::util::SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION);
  pr.build_surrogate();

  const MatrixXd& polynomial_coeffs = pr.get_polynomial_coeffs();
  // Will need to see how this interacts with the constant term and scaling/transforms - RWH
  double polynomial_intercept = pr.get_polynomial_intercept();
  BOOST_CHECK( std::abs(polynomial_intercept) < 1.0e-10 );

  // Check correctness of computed polynomial coefficients against the one used to construct the model
  MatrixXd gold_coeffs(10,1);
  // Term exponents for f(x,y) = -1 + 4*x*x + 4*y*y
  // x:            0    1    0    2    0    1    3    0    2    1
  // y:            0    0    1    0    2    1    0    3    1    2
  gold_coeffs << -1.0, 0.0, 0.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0;
  BOOST_CHECK(matrix_equals(gold_coeffs, polynomial_coeffs, 1.0e-10));

  // Now try out some interpolants that should be exact
  MatrixXd eval_points, gold_responses, test_responses;
  get_samples(num_vars, 7, eval_points);
  another_additive_quadratic_function(eval_points, gold_responses);

  pr.surrogate_value(eval_points, test_responses);
  BOOST_CHECK(matrix_equals(gold_responses, test_responses, 1.0e-10));

  // Now use the options-based API
  auto options = std::make_shared<Teuchos::ParameterList>();
  options->set("Num Vars", num_vars);
  options->set("Max Degree", degree);
  PolynomialRegression pr2(options);
  pr2.set_samples(samples);
  pr2.set_response(responses);
  pr2.set_scaler_type(dakota::util::SCALER_TYPE::NONE);
  pr2.set_solver(dakota::util::SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION);
  pr2.build_surrogate();

  const MatrixXd& polynomial_coeffs2 = pr2.get_polynomial_coeffs();
  double polynomial_intercept2 = pr2.get_polynomial_intercept();
  BOOST_CHECK( std::abs(polynomial_intercept2) < 1.0e-10 );

  // Check correctness of computed polynomial coefficients against the one used to construct the model
  BOOST_CHECK(matrix_equals(gold_coeffs, polynomial_coeffs2, 1.0e-10));
}

void
PolynomialRegressionSurrogate_gradient_and_hessian()
{
  int num_vars    = 2,
      num_samples = 20,
      degree      = 3;

  MatrixXd samples, responses;
  get_samples(num_vars, num_samples, samples);
  cubic_bivariate_function(samples, responses);

  PolynomialRegression pr(degree, num_vars);
  pr.set_samples(samples);
  pr.set_polynomial_order(degree);
  pr.set_response(responses);
  pr.set_scaler_type(dakota::util::SCALER_TYPE::NONE);
  pr.set_solver(dakota::util::SOLVER_TYPE::SVD_LEAST_SQ_REGRESSION);
  pr.build_surrogate();

  MatrixXd gradient, hessian;
  pr.gradient(samples.topRows(2), gradient);
  pr.hessian(samples.topRows(1), hessian);

  /*
  std::cout << "samples" << std::endl;
  std::cout << samples.topRows(2) << std::endl;
  std::cout << "\n";
  */

  MatrixXd gold_gradient(2,2);
  gold_gradient << 1.59711,  1.06684, 
                   0.691654, 2.90352;

  MatrixXd gold_hessian(2,2);
  gold_hessian << -2.75852, 0.04462,
                   0.04462, -1.83287;

  BOOST_CHECK( matrix_equals(gold_hessian, hessian, 1.0e-4) );
}


} // namespace

// --------------------------------------------------------------------------------

int test_main( int argc, char* argv[] ) // note the name!
{
  // Univariate tests
  PolynomialRegressionSurrogate_getters_and_setters();
  PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE::NONE);
  PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE::MEAN_NORMALIZATION);
  PolynomialRegressionSurrogate_straight_line_fit(dakota::util::SCALER_TYPE::STANDARDIZATION);

  // Multivariate tests
  PolynomialRegressionSurrogate_multivariate_regression_builder();
  PolynomialRegressionSurrogate_gradient_and_hessian();

  int run_result = 0;
  BOOST_CHECK( run_result == 0 || run_result == boost::exit_success );

  return boost::exit_success;
}

// --------------------------------------------------------------------------------
