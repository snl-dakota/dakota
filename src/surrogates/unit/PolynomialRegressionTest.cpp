/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesPolynomialRegression.hpp"
#include "Teuchos_XMLParameterListCoreHelpers.hpp"
#include "UtilLinearSolvers.hpp"
#include "surrogates_tools.hpp"
#include "util_common.hpp"

#include <boost/filesystem.hpp>
#include <boost/version.hpp>
#if (BOOST_VERSION < 107000) && !defined(BOOST_ALLOW_DEPRECATED_HEADERS)
// could alternately use: #define BOOST_PENDING_INTEGER_LOG2_HPP 1
#define BOOST_ALLOW_DEPRECATED_HEADERS 1
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#undef BOOST_ALLOW_DEPRECATED_HEADERS
#else
#include <boost/random.hpp>
#include <boost/random/uniform_real.hpp>
#endif
#include <boost/test/minimal.hpp>

using namespace dakota;
using namespace dakota::util;
using namespace dakota::surrogates;

namespace {

MatrixXd create_single_feature_matrix() {
  MatrixXd single_feature_matrix(7, 1);
  single_feature_matrix << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7;
  return single_feature_matrix;
}

MatrixXd create_multiple_features_matrix() {
  MatrixXd multiple_features_matrix(7, 3);
  multiple_features_matrix << 0.1, 0.2, 0.5, 1, 3, 6, 10, 20, 50, 100, 300, 700,
      1000, 3000, 8000, 10000, 30000, 40000, 100000, 500000, 700000;
  return multiple_features_matrix;
}

void get_samples(int num_vars, int num_samples, MatrixXd& samples) {
  samples.resize(num_samples, num_vars);

  int seed = 1337;

  // This choice allows consistent comparison with
  // pecos/surrogates/models/src/RegressionBuilder
  typedef boost::uniform_real<> NumberDistribution;
  // Using Boost unif real dist for cross-platform stability
  boost::uniform_real<double> distribution(-1.0, 1.0);
  // Using Boost MT since need it anyway for unif real dist
  boost::mt19937 generator;
  generator.seed(seed);
  boost::variate_generator<boost::mt19937, NumberDistribution> numberGenerator(
      generator, distribution);

  for (int j = 0; j < num_vars; ++j) {
    for (int i = 0; i < num_samples; ++i) {
      samples(i, j) = numberGenerator();
    }
  }
}

void another_additive_quadratic_function(const MatrixXd& samples,
                                         MatrixXd& func_vals) {
  func_vals.resize(samples.rows(),
                   1);  // hard-coded to one response for now - RWH
  const int num_vars = samples.cols();

  for (int i = 0; i < samples.rows(); ++i) {
    func_vals(i, 0) = 1.0;
    for (int j = 0; j < num_vars; ++j) {
      double x = (2 * samples(i, j) - 1);
      func_vals(i, 0) += x * x + 2.0 * x;
    }
  }
}

void cubic_bivariate_function(const MatrixXd& samples, MatrixXd& func_vals,
                              bool include_cross_terms = true) {
  double scale_cross = include_cross_terms ? 1.0 : 0.0;

  func_vals.resize(samples.rows(), 1);
  const int num_vars = samples.cols();

  for (int i = 0; i < samples.rows(); ++i) {
    double x = samples(i, 0);
    double y = samples(i, 1);
    func_vals(i, 0) = 1 + x + y + scale_cross * (x * y) +
      // This term has degree 4, so removing :
      //              scale_cross * (std::pow(x, 2) * std::pow(y, 2)) +
                      scale_cross * (std::pow(x, 2) * y) +
                      scale_cross * (std::pow(y, 2) * x) + std::pow(x, 3) +
                      std::pow(y, 3);
  }
}

// Gradient includes cross-terms
MatrixXd cubic_bivariate_withcross_gradient(const MatrixXd& samples)
{
  MatrixXd gradients(samples.rows(), samples.cols());
  for (int i = 0; i < samples.rows(); ++i) {
    double x = samples(i, 0);
    double y = samples(i, 1);
    gradients(i, 0) = 1 + y + 2*x*y + y*y +3*x*x;
    gradients(i, 1) = 1 + x + 2*x*y + x*x +3*y*y;
  }
  return gradients;
}

// Hessian includes cross-terms
MatrixXd cubic_bivariate_withcross_hessian(const MatrixXd& sample)
{
  double x = sample(0, 0);
  double y = sample(0, 1);

  MatrixXd hessian(2,2);
  hessian <<
    2*y + 6*x,      1 + 2*x + 2*y,
    1 + 2*x + 2*y,  2*x + 6*y;
  return hessian;
}


/* Unit tests */

void PolynomialRegressionSurrogate_straight_line_fit(std::string scaler_type) {
  VectorXd line_vector = VectorXd::LinSpaced(20, 0, 1);  // size, low, high
  VectorXd response = VectorXd::LinSpaced(20, 0, 1);
  response = (response.array() + 2.0)
                 .matrix();  // +2.0 because the line's y-intercept is 2.0

  Teuchos::ParameterList config_options("Polynomial Test Parameters");
  config_options.set("max degree", 1);
  config_options.set("scaler type", scaler_type);
  PolynomialRegression pr(line_vector, response, config_options);

  const MatrixXd& polynomial_coeffs = pr.get_polynomial_coeffs();
  double polynomial_intercept = pr.get_polynomial_intercept();

  double expected_constant_term = 2.0;  // unscaled intercept via coeffs array
  double expected_first_term = 1.0;     // unscaled slope via coeffs array
  double expected_polynomial_intercept = 0.0;

  if (scaler_type == "mean normalization" || scaler_type == "standardization") {
    expected_constant_term = 0.0;
    expected_polynomial_intercept = 2.5;
  }
  if (scaler_type == "standardization")
    expected_first_term = 0.303488;  // scaled

  double actual_constant_term = polynomial_coeffs(0, 0);
  double actual_first_term = polynomial_coeffs(1, 0);

  BOOST_CHECK(std::abs(expected_constant_term - actual_constant_term) < 1.0e-4);
  BOOST_CHECK(std::abs(expected_first_term - actual_first_term) < 1.0e-4);
  BOOST_CHECK(std::abs(expected_polynomial_intercept - polynomial_intercept) <
              1.0e-4);

  VectorXd unscaled_eval_pts = VectorXd::LinSpaced(100, 0, 1);
  VectorXd expected_approx_values(100);
  expected_approx_values << 2.0, 2.0101, 2.0202, 2.0303, 2.0404, 2.05051,
      2.06061, 2.07071, 2.08081, 2.09091, 2.10101, 2.11111, 2.12121, 2.13131,
      2.14141, 2.15152, 2.16162, 2.17172, 2.18182, 2.19192, 2.20202, 2.21212,
      2.22222, 2.23232, 2.24242, 2.25253, 2.26263, 2.27273, 2.28283, 2.29293,
      2.30303, 2.31313, 2.32323, 2.33333, 2.34343, 2.35354, 2.36364, 2.37374,
      2.38384, 2.39394, 2.40404, 2.41414, 2.42424, 2.43434, 2.44444, 2.45455,
      2.46465, 2.47475, 2.48485, 2.49495, 2.50505, 2.51515, 2.52525, 2.53535,
      2.54545, 2.55556, 2.56566, 2.57576, 2.58586, 2.59596, 2.60606, 2.61616,
      2.62626, 2.63636, 2.64646, 2.65657, 2.66667, 2.67677, 2.68687, 2.69697,
      2.70707, 2.71717, 2.72727, 2.73737, 2.74747, 2.75758, 2.76768, 2.77778,
      2.78788, 2.79798, 2.80808, 2.81818, 2.82828, 2.83838, 2.84848, 2.85859,
      2.86869, 2.87879, 2.88889, 2.89899, 2.90909, 2.91919, 2.92929, 2.93939,
      2.94949, 2.9596, 2.9697, 2.9798, 2.9899, 3.0;

  VectorXd actual_approx_values;
  actual_approx_values = pr.value(unscaled_eval_pts);

  BOOST_CHECK(
      matrix_equals(actual_approx_values, expected_approx_values, 1.0e-5));
}

void PolynomialRegressionSurrogate_multivariate_regression_builder() {
  int num_vars = 2, num_samples = 20, degree = 3;

  /* Generate build data */
  MatrixXd samples, responses;
  get_samples(num_vars, num_samples, samples);
  another_additive_quadratic_function(samples, responses);

  /* Now try out some interpolants that should be exact */

  /* Check correctness of computed polynomial coefficients against the one used
   * to construct the model */
  MatrixXd gold_coeffs(10, 1);
  /* Term exponents for f(x,y) = -1 + 4*x*x + 4*y*y
     x:            0    1    0    2    0    1    3    0    2    1
     y:            0    0    1    0    2    1    0    3    1    2 */
  gold_coeffs << -1.0, 0.0, 0.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0;
  MatrixXd eval_points, gold_responses;
  get_samples(num_vars, 7, eval_points);
  another_additive_quadratic_function(eval_points, gold_responses);

  /* Use the Surrogates API with partially user-defined input parameters */
  VectorXd test_responses3;
  Teuchos::ParameterList param_list_partial("Polynomial Test Parameters");
  param_list_partial.set("max degree", degree);
  param_list_partial.set("scaler type", "none");
  /* Default configOptions will take care of below */
  // param_list_partial.set<double>("p-norm", 1.0);
  // param_list_partial.set("regression solver type", "SVD");

  PolynomialRegression pr3(samples, responses, param_list_partial);

  const MatrixXd& polynomial_coeffs3 = pr3.get_polynomial_coeffs();
  double polynomial_intercept3 = pr3.get_polynomial_intercept();
  test_responses3 = pr3.value(eval_points);

  BOOST_CHECK(std::abs(polynomial_intercept3) < 1.0e-10);
  BOOST_CHECK(matrix_equals(gold_coeffs, polynomial_coeffs3, 1.0e-10));
  BOOST_CHECK(matrix_equals(gold_responses, test_responses3, 1.0e-10));

  /* Use the Surrogates API with user-defined and updated configOptions
     and separate constructor and build steps */
  MatrixXd test_responses4;
  Teuchos::ParameterList param_list_full("Polynomial Test Parameters");
  /* wrong degree for this test -- increment by 1 */
  param_list_full.set("max degree", degree + 1);
  param_list_full.set<double>("p-norm", 1.0);
  param_list_full.set("regression solver type", "SVD");
  param_list_full.set("scaler type", "none");

  PolynomialRegression pr4(param_list_full);
  /* update the parameterList, set it, and build the surrogate */
  param_list_full.set("max degree", degree);
  pr4.set_options(param_list_full);
  pr4.build(samples, responses);

  const MatrixXd& polynomial_coeffs4 = pr4.get_polynomial_coeffs();
  double polynomial_intercept4 = pr4.get_polynomial_intercept();
  test_responses4 = pr4.value(eval_points);

  BOOST_CHECK(std::abs(polynomial_intercept4) < 1.0e-10);
  BOOST_CHECK(matrix_equals(gold_coeffs, polynomial_coeffs4, 1.0e-10));
  BOOST_CHECK(matrix_equals(gold_responses, test_responses4, 1.0e-10));

  /* Use the Surrogates API with default parameters, get and
     update those params. Separate constructor and build steps */
  MatrixXd test_responses5;
  PolynomialRegression pr5;

  /* default poly order is 1, change to degree */
  Teuchos::ParameterList current_opts;
  pr5.get_options(current_opts);
  current_opts.set("max degree", degree);
  pr5.set_options(current_opts);
  pr5.build(samples, responses);

  const MatrixXd& polynomial_coeffs5 = pr5.get_polynomial_coeffs();
  double polynomial_intercept5 = pr5.get_polynomial_intercept();
  test_responses5 = pr5.value(eval_points);

  BOOST_CHECK(std::abs(polynomial_intercept5) < 1.0e-10);
  BOOST_CHECK(matrix_equals(gold_coeffs, polynomial_coeffs5, 1.0e-10));
  BOOST_CHECK(matrix_equals(gold_responses, test_responses5, 1.0e-10));

  /* Test 6 -- Use the Surrogates API with reduced basis, get and
     update those params. Separate constructor and build steps */
  MatrixXd test_responses6;
  PolynomialRegression pr6;

  // Generate realizations of function variables
  MatrixXd responses6, gold_responses6;
  cubic_bivariate_function(samples, responses6, /* cross terms */ false);
  // Check correctness of computed polynomial coefficients against the one used
  // to construct the model
  MatrixXd gold_coeffs6(1 + num_vars * degree, 1);
  // Term exponents for f(x,y) = 1 + x + y           + x^3 + y^3
  //// x:                        0   1   0    0    0    1     0
  //// y:                        0   0   1    0    0    0     1
  gold_coeffs6 << 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0;
  cubic_bivariate_function(eval_points, gold_responses6, false);

  pr6.get_options(current_opts);
  current_opts.set("max degree", degree);
  current_opts.set("reduced basis", true);
  pr6.set_options(current_opts);
  pr6.build(samples, responses6);

  const MatrixXd& polynomial_coeffs6 = pr6.get_polynomial_coeffs();
  double polynomial_intercept6 = pr6.get_polynomial_intercept();
  test_responses6 = pr6.value(eval_points);

  BOOST_CHECK(std::abs(polynomial_intercept6) < 1.0e-10);
  BOOST_CHECK(matrix_equals(gold_coeffs6, polynomial_coeffs6, 1.0e-10));
  BOOST_CHECK(matrix_equals(gold_responses6, test_responses6, 1.0e-10));
}

void PolynomialRegressionSurrogate_gradient_and_hessian() {
  int num_vars = 2, num_samples = 20, degree = 3;

  MatrixXd samples, responses;
  get_samples(num_vars, num_samples, samples);
  cubic_bivariate_function(samples, responses);

  Teuchos::ParameterList config_options("Polynomial Test Parameters");
  config_options.set("max degree", degree);

  // This model should be able to recover the exact original function
  PolynomialRegression pr(samples, responses, config_options);

  MatrixXd gradient, hessian;
  gradient = pr.gradient(samples.topRows(2));
  hessian = pr.hessian(samples.topRows(1));

  MatrixXd gold_gradient = cubic_bivariate_withcross_gradient(samples.topRows(2));
  MatrixXd gold_hessian = cubic_bivariate_withcross_hessian(samples.topRows(1));

  BOOST_CHECK(matrix_equals(gold_gradient, gradient, 1.0e-10));
  BOOST_CHECK(matrix_equals(gold_hessian, hessian, 1.0e-10));
}

void PolynomialRegressionSurrogate_parameter_list_import() {
  int num_vars = 2, num_samples = 20, degree = 3;

  MatrixXd samples, responses;
  get_samples(num_vars, num_samples, samples);
  cubic_bivariate_function(samples, responses);

  // This model should be able to recover the exact original function
  PolynomialRegression pr(samples, responses,
                          "pr_test_data/pr_parameter_list.yaml");

  MatrixXd gradient, hessian;
  gradient = pr.gradient(samples.topRows(2));
  hessian = pr.hessian(samples.topRows(1));

  MatrixXd gold_gradient = cubic_bivariate_withcross_gradient(samples.topRows(2));
  MatrixXd gold_hessian = cubic_bivariate_withcross_hessian(samples.topRows(1));

  BOOST_CHECK(matrix_equals(gold_gradient, gradient, 1.0e-9));
  BOOST_CHECK(matrix_equals(gold_hessian, hessian, 1.0e-9));
}

/// Create, evaluate, and save a basic polynomial; load and verify
/// same evals (based on multivariate_regression_builder test)
void PolynomialRegression_SaveLoad() {
  int num_vars = 2, num_samples = 20, degree = 3;

  /* Generate build data */
  MatrixXd samples, responses;
  get_samples(num_vars, num_samples, samples);
  another_additive_quadratic_function(samples, responses);

  /* Now try out some interpolants that should be exact */

  /* Check correctness of computed polynomial coefficients against the one used
   * to construct the model */
  MatrixXd gold_coeffs(10, 1);
  /* Term exponents for f(x,y) = -1 + 4*x*x + 4*y*y
     x:            0    1    0    2    0    1    3    0    2    1
     y:            0    0    1    0    2    1    0    3    1    2 */
  gold_coeffs << -1.0, 0.0, 0.0, 4.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0;
  MatrixXd eval_points, gold_responses, test_responses;
  get_samples(num_vars, 7, eval_points);
  another_additive_quadratic_function(eval_points, gold_responses);

  /* Use the Surrogates API with partially user-defined input parameters */
  Teuchos::ParameterList param_list_partial("Polynomial Test Parameters");
  param_list_partial.set("max degree", degree);
  param_list_partial.set("scaler type", "none");

  // test serialization through pointers
  auto pr3 = std::make_shared<PolynomialRegression>(samples, responses,
                                                    param_list_partial);
  pr3->variable_labels({"poly_x1", "poly_x2"});
  pr3->response_labels({"poly_f"});

  // Initially modelling what save/load functions would do for binary/text
  for (bool binary : {true, false}) {
    std::string filename("poly_test.surr");
    filename += binary ? ".bin" : ".txt";

    boost::filesystem::remove(filename);
    Surrogate::save(std::dynamic_pointer_cast<Surrogate>(pr3), filename,
                    binary);

    std::shared_ptr<Surrogate> surr_in = Surrogate::load(filename, binary);

    auto pr4 = std::dynamic_pointer_cast<PolynomialRegression>(surr_in);

    BOOST_CHECK(pr3->get_num_terms() == pr4->get_num_terms());
    BOOST_CHECK(pr3->get_polynomial_intercept() ==
                pr4->get_polynomial_intercept());
    BOOST_CHECK(pr3->get_polynomial_coeffs() == pr4->get_polynomial_coeffs());

    // tests on the loaded surrogate based on original unit test
    const MatrixXd& loaded_coeffs = pr4->get_polynomial_coeffs();
    double loaded_intercept = pr4->get_polynomial_intercept();
    VectorXd test_responses;
    test_responses = pr4->value(eval_points);

    BOOST_CHECK(std::abs(loaded_intercept) < 1.0e-10);
    BOOST_CHECK(matrix_equals(gold_coeffs, loaded_coeffs, 1.0e-10));
    BOOST_CHECK(matrix_equals(gold_responses, test_responses, 1.0e-10));
  }
}

}  // namespace

// --------------------------------------------------------------------------------

int test_main(int argc, char* argv[])  // note the name!
{
  // Univariate tests
  PolynomialRegressionSurrogate_straight_line_fit("none");
  PolynomialRegressionSurrogate_straight_line_fit("mean normalization");
  PolynomialRegressionSurrogate_straight_line_fit("standardization");

  // Multivariate tests
  PolynomialRegressionSurrogate_multivariate_regression_builder();
  PolynomialRegressionSurrogate_gradient_and_hessian();

  // ParameterList import test
  PolynomialRegressionSurrogate_parameter_list_import();

  // Serialization tests
  PolynomialRegression_SaveLoad();

  BOOST_CHECK(boost::exit_success == 0);

  return boost::exit_success;
}

// --------------------------------------------------------------------------------
