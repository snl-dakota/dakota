/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesGaussianProcess.hpp"
#include "surrogates_tools.hpp"
#include "util_common.hpp"
#include "util_data_types.hpp"

#include <gtest/gtest.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <fstream>

// BMA TODO: Review with team for best practice
using namespace dakota;
using namespace dakota::util;
using namespace dakota::surrogates;

namespace {

void get_1D_gp_test_data(MatrixXd& samples, VectorXd& response,
                         VectorXd& eval_pts) {
  /* num_samples x num_features */
  samples.resize(7, 1);

  /* num_samples (x num_qoi = 1) */
  response.resize(7);

  eval_pts.resize(6);

  samples << 0.05536604, 0.28730518, 0.30391231, 0.40768703, 0.45035059,
      0.52639952, 0.78853488;

  response << -0.15149429, -0.19689361, -0.17323105, -0.02379026, 0.02013445,
      0.05011702, -0.11678312;

  eval_pts << 0.0, 0.2, 0.4, 0.6, 0.8, 1.0;
}

void get_gp_hyperparameter_bounds(const int nvars, VectorXd& sigma_bounds,
                                  MatrixXd& length_scale_bounds) {
  const double lb_val = 1.0e-2;
  const double ub_val = 1.0e2;

  /* sigma bounds (kernel scaling parameter) */
  sigma_bounds.resize(2);
  sigma_bounds(0) = lb_val;
  sigma_bounds(1) = ub_val;

  /* length scale bounds */
  length_scale_bounds.resize(nvars, 2);
  length_scale_bounds.col(0).fill(lb_val);
  length_scale_bounds.col(1).fill(ub_val);
}

ParameterList get_gp_config_options(const VectorXd& sigma_bounds,
                                    const MatrixXd& length_scale_bounds) {
  ParameterList param_list("GP Test Parameters");
  param_list.sublist("Sigma Bounds").set("lower bound", sigma_bounds(0));
  param_list.sublist("Sigma Bounds").set("upper bound", sigma_bounds(1));
  param_list.set("anisotropic length-scale bounds", length_scale_bounds);
  param_list.set("scaler name", "standardization");
  param_list.set("num restarts", 10);
  param_list.sublist("Nugget").set("fixed nugget", 1.0e-12);
  param_list.set("gp seed", 42);
  param_list.set("standardize response", false);

  return param_list;
}

void get_1D_gp_golds(VectorXd& gold_mean, VectorXd& gold_std_dev,
                     MatrixXd& gold_cov, bool gp_has_trend = false) {
  const int num_pts = 6;
  gold_mean.resize(num_pts);
  gold_std_dev.resize(num_pts);
  gold_cov.resize(num_pts, num_pts);

  if (gp_has_trend) {
    gold_mean << -0.05312219, -0.27214983, -0.033252, 0.02527716, -0.12356696,
        -0.18573136;
    gold_std_dev << 0.02028642, 0.00536066, 0.00064835, 0.00459461, 0.00395678,
        0.14659393;
    gold_cov << 0.000411539, -9.13625e-05, 6.17757e-08, -1.9461e-05,
        5.94823e-06, -0.000256977, -9.13625e-05, 2.87366e-05, -5.31468e-07,
        6.01025e-06, -4.53635e-06, -5.15403e-05, 6.17757e-08, -5.31468e-07,
        4.20359e-07, 8.39709e-07, -4.44025e-07, -8.63467e-06, -1.9461e-05,
        6.01025e-06, 8.39709e-07, 2.11104e-05, -1.52399e-05, -0.000388392,
        5.94823e-06, -4.53635e-06, -4.44025e-07, -1.52399e-05, 1.56561e-05,
        0.000499163, -0.000256977, -5.15403e-05, -8.63467e-06, -0.000388392,
        0.000499163, 0.0214898;
  } else {
    gold_mean << -0.046014, -0.278509, -0.0333528, 0.0185393, -0.118491,
        -0.0506785;
    gold_std_dev << 0.0170694, 0.00203616, 1.67821e-05, 0.00317294, 0.00392892,
        0.121505;
    gold_cov << 0.000291366, -3.13983e-05, -2.05299e-07, 2.82111e-05,
        -2.23603e-05, -0.000337062, -3.13983e-05, 4.14595e-06, 3.16567e-08,
        -4.86909e-06, 4.17091e-06, 6.60782e-05, -2.05299e-07, 3.16567e-08,
        2.81647e-10, -4.93076e-08, 4.72685e-08, 8.16402e-07, 2.82111e-05,
        -4.86909e-06, -4.93076e-08, 1.00675e-05, -1.12689e-05, -0.000225718,
        -2.23603e-05, 4.17091e-06, 4.72685e-08, -1.12689e-05, 1.54364e-05,
        0.00039519, -0.000337062, 6.60782e-05, 8.16402e-07, -0.000225718,
        0.00039519, 0.0147636;
  }
}

void get_2D_gp_test_data(MatrixXd& samples, VectorXd& response,
                         MatrixXd& eval_pts) {
  const int num_datasets = 1;
  const int num_vars = 2;
  const int num_samples = 64;
  const int num_qoi = 1;

  samples.resize(num_samples, num_vars);
  response.resize(num_samples);
  eval_pts.resize(4, num_vars);

  std::string samples_fname = "gp_test_data/lhs_data_64.txt";
  std::string responses_fname = "gp_test_data/smooth_herbie_64.txt";

  std::vector<MatrixXd> responses_list;
  std::vector<MatrixXd> samples_list;

  populateMatricesFromFile(samples_fname, samples_list, num_datasets, num_vars,
                           num_samples);
  populateMatricesFromFile(responses_fname, responses_list, num_datasets,
                           num_qoi, num_samples);

  samples = samples_list[0];
  response = responses_list[0];

  eval_pts << 0.2, 0.45, -0.3, -0.7, 0.4, -0.1, -0.25, 0.33;
}

ParameterList get_2D_gp_config_options(const VectorXd& sigma_bounds,
                                       const MatrixXd& length_scale_bounds) {
  ParameterList param_list("GP Test Parameters");
  param_list.set("sigma bounds", sigma_bounds);
  param_list.set("length-scale bounds", length_scale_bounds);
  param_list.set("scaler name", "standardization");
  param_list.set("num restarts", 10);
  param_list.sublist("Nugget").set("fixed nugget", 1.0e-12);
  param_list.set("gp seed", 42);

  return param_list;
}

void get_2D_gp_golds_no_trend(VectorXd& gold_mean, VectorXd& gold_std_dev,
                              MatrixXd& gold_cov, bool gp_has_trend = false) {
  const int num_pts = 4;
  gold_mean.resize(num_pts);
  gold_std_dev.resize(num_pts);
  gold_cov.resize(num_pts, num_pts);

  if (gp_has_trend) {
    gold_mean << 0.779875, 0.84715, 0.744379, 0.746542;
    gold_std_dev << 0.000243319, 0.00178768, 0.000307974, 0.000454846;
    gold_cov << 5.9204e-08, 6.57492e-08, -9.02961e-09, -4.67214e-08,
        6.57492e-08, 3.19579e-06, -3.74954e-07, 2.70074e-07, -9.02961e-09,
        -3.74954e-07, 9.48479e-08, -9.77782e-08, -4.67214e-08, 2.70074e-07,
        -9.77782e-08, 2.06885e-07;
  } else {
    gold_mean << 0.779863, 0.84671, 0.744502, 0.746539;
    gold_std_dev << 0.000202807, 0.00157021, 0.000266543, 0.000399788;
    gold_cov << 4.11307e-08, 5.05967e-08, -6.56123e-09, -3.19852e-08,
        5.05967e-08, 2.46557e-06, -2.8656e-07, 2.18488e-07, -6.56123e-09,
        -2.8656e-07, 7.10453e-08, -7.75076e-08, -3.19852e-08, 2.18488e-07,
        -7.75076e-08, 1.5983e-07;
  }
}

void get_2D_gp_golds_trend_derivs(MatrixXd& gold_grad, MatrixXd& gold_hessian) {
  const int num_vars = 2;

  /* grad and hessian evaluated at a single point */
  gold_grad.resize(1, num_vars);
  gold_hessian.resize(num_vars, num_vars);

  gold_grad << -0.312998265, -0.25777615;
  gold_hessian << 0.86763171, 0.10209617, 0.10209617, -0.84260876;
}

void get_gp_test_arrays(GaussianProcess& gp, const MatrixXd& eval_pts,
                        VectorXd& mean, VectorXd& std_dev, MatrixXd& cov) {
  mean = gp.value(eval_pts);
  std_dev = gp.variance(eval_pts).array().sqrt();
  cov = gp.covariance(eval_pts);
}

void get_1D_gp_golds_matern(VectorXd& gold_mean, VectorXd& gold_std_dev,
                            const std::string& kernel_type) {
  const int num_pts = 6;
  gold_mean.resize(num_pts);
  gold_std_dev.resize(num_pts);

  if (kernel_type == "Matern 3/2") {
    gold_mean << -0.11936208, -0.23318749, -0.03334298, 0.02120401, -0.12067952,
        -0.0974504;
    gold_std_dev << 0.03982259, 0.03521074, 0.00304295, 0.03697039, 0.00904159,
        0.11075583;
  } else if (kernel_type == "Matern 5/2") {
    gold_mean << -0.10214858, -0.25591925, -0.03325482, 0.02288785, -0.12158236,
        -0.10886755;
    gold_std_dev << 0.0333359, 0.01982311, 0.0007937, 0.02341843, 0.0070827,
        0.11714938;
  }
}

void get_2D_gp_golds_matern(VectorXd& gold_mean, VectorXd& gold_std_dev,
                            const std::string& kernel_type) {
  const int num_pts = 4;
  gold_mean.resize(num_pts);
  gold_std_dev.resize(num_pts);

  if (kernel_type == "Matern 3/2") {
    gold_mean << 0.78372109, 0.84579884, 0.74428938, 0.75949557;
    gold_std_dev << 0.02762492, 0.04026672, 0.02225784, 0.02801156;
  } else if (kernel_type == "Matern 5/2") {
    gold_mean << 0.78055266, 0.84403835, 0.74442646, 0.74825591;
    gold_std_dev << 0.01220025, 0.02180271, 0.00996856, 0.01210915;
  }
}

TEST(GaussianProcessTest_tests, test_surrogates_1D_gp_constructor_types) {
  bool print_output = false;

  MatrixXd samples, cov, gold_cov, length_scale_bounds;
  VectorXd response, eval_pts;
  VectorXd sigma_bounds, mean, gold_mean, std_dev, gold_std_dev;

  /* build and eval data*/
  get_1D_gp_test_data(samples, response, eval_pts);

  /* bound constraints */
  get_gp_hyperparameter_bounds(1, sigma_bounds, length_scale_bounds);

  /* configuration options */
  ParameterList param_list =
      get_gp_config_options(sigma_bounds, length_scale_bounds);

  /* gold data for test */
  get_1D_gp_golds(gold_mean, gold_std_dev, gold_cov);

  /* relative tolerance for floating point comparisons */
  const double rel_float_tol = 1.0e-4;

  for (int i = 0; i < 3; i++) {
    if (i == 0) {
      /* 1D GP constructor type 1: Construct GP and build surrogate
       * all at once */
      GaussianProcess gp(samples, response, param_list);
      get_gp_test_arrays(gp, eval_pts, mean, std_dev, cov);
    } else if (i == 1) {
      /* 1D GP constructor type 2:
       * Separate constructor with given options
       * and build steps */
      GaussianProcess gp(param_list);
      gp.build(samples, response);
      get_gp_test_arrays(gp, eval_pts, mean, std_dev, cov);
    } else if (i == 2) {
      /* 1D GP constructor type 3:
       * use defaultConfigOptions and adjust as needed
       * for desired behavior */
      GaussianProcess gp(param_list);
      ParameterList current_opts;
      gp.get_options(current_opts);
      current_opts.set("scaler name", "standardization");
      current_opts.sublist("Nugget").set("fixed nugget", 1.0e-12);
      current_opts.set("gp seed", 42);
      gp.set_options(current_opts);
      gp.build(samples, response);
      get_gp_test_arrays(gp, eval_pts, mean, std_dev, cov);
    }

    if (print_output) {
      std::cout << "\n\n** Surrogate constructor type" << i << " **\n";
      std::cout << "\n1D GP mean:\n";
      std::cout << mean << "\n";
      std::cout << "\n1D GP standard deviation:\n";
      std::cout << std_dev << "\n";
      std::cout << "\n1D GP covariance:\n";
      std::cout << cov << "\n";
    }

    EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
    EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, 100 * rel_float_tol));
    EXPECT_TRUE(relative_allclose(cov, gold_cov, 100 * rel_float_tol));
  }
}

TEST(GaussianProcessTest_tests, test_surrogates_1D_gp_with_trend_values_and_derivs) {
  bool print_output = false;

  MatrixXd samples, cov, gold_cov, length_scale_bounds;
  VectorXd response, eval_pts;
  VectorXd sigma_bounds, mean, gold_mean, std_dev, gold_std_dev;

  /* build and eval data */
  get_1D_gp_test_data(samples, response, eval_pts);

  /* bound constraints */
  get_gp_hyperparameter_bounds(1, sigma_bounds, length_scale_bounds);

  /* configuration options */
  ParameterList param_list =
      get_gp_config_options(sigma_bounds, length_scale_bounds);

  /* gold data for test */
  get_1D_gp_golds(gold_mean, gold_std_dev, gold_cov, true);

  /* relative tolerance for floating point comparisons */
  const double rel_float_tol = 1.0e-4;

  /* 1D GP constructor type 3 (test 2):
   * use defaultConfigOptions and introduce a polynomial trend
   * and nugget estimation */
  GaussianProcess gp(param_list);

  ParameterList current_opts;
  gp.get_options(current_opts);
  current_opts.set("scaler name", "standardization");
  current_opts.sublist("Nugget").set("fixed nugget", 0.0);
  current_opts.set("gp seed", 42);
  current_opts.sublist("Nugget").set("estimate nugget", true);
  current_opts.sublist("Trend").set("estimate trend", true);
  current_opts.sublist("Trend").sublist("Options").set("max degree", 1);
  current_opts.set("num restarts", 20);
  gp.set_options(current_opts);

  gp.build(samples, response);

  get_gp_test_arrays(gp, eval_pts, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n1D GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n1D GP standard deviation:\n";
    std::cout << std_dev << "\n";
    std::cout << "\n1D GP covariance:\n";
    std::cout << cov << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));

  /* compute derivatives of GP with trend and check */
  const int eval_point_index = 0;
  auto eval_point = eval_pts.row(eval_point_index);
  MatrixXd grad, hessian;

  grad = gp.gradient(eval_point);
  hessian = gp.hessian(eval_point);

  MatrixXd grad_fd_error, hessian_fd_error;
  fd_check_gradient(gp, eval_point, grad_fd_error);
  fd_check_hessian(gp, eval_point, hessian_fd_error);

  if (print_output) {
    std::cout << "\nGP with trend gradient:\n";
    std::cout << grad;

    std::cout << "\nGP with trend hessian:\n";
    std::cout << hessian;

    std::cout << "\nGP with trend gradient fd error:\n";
    std::cout << grad_fd_error << "\n";

    std::cout << "\nGP with trend hessian fd error:\n";
    std::cout << hessian_fd_error << "\n";
  }

  double grad_drop, hessian_drop;
  grad_drop = log10(grad_fd_error(0, 0) / grad_fd_error.minCoeff());
  hessian_drop = log10(hessian_fd_error(0, 0) / hessian_fd_error.minCoeff());

  EXPECT_TRUE((grad_drop > 6.0));
  EXPECT_TRUE((hessian_drop > 3.0));
}

TEST(GaussianProcessTest_tests, test_surrogates_2D_gp_no_trend) {
  bool print_output = false;

  MatrixXd samples, cov, gold_cov, length_scale_bounds, eval_pts;
  VectorXd response, sigma_bounds, mean, gold_mean, std_dev, gold_std_dev;

  /* build and eval data */
  get_2D_gp_test_data(samples, response, eval_pts);

  /* bound constraints */
  get_gp_hyperparameter_bounds(2, sigma_bounds, length_scale_bounds);

  /* configuration options */
  ParameterList param_list =
      get_gp_config_options(sigma_bounds, length_scale_bounds);
  param_list.sublist("Nugget").set("fixed nugget", 1.0e-10);
  param_list.set("num restarts", 15);

  /* gold data for test */
  get_2D_gp_golds_no_trend(gold_mean, gold_std_dev, gold_cov);

  /* relative tolerance for floating point comparisons */
  const double rel_float_tol = 1.0e-4;

  /* 2D GP constructor type 1 */
  GaussianProcess gp(param_list);

  gp.build(samples, response);

  get_gp_test_arrays(gp, eval_pts, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n2D GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n2D GP standard deviation:\n";
    std::cout << std_dev << "\n";
    std::cout << "\n2D GP covariance:\n";
    std::cout << cov << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
  EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, 100 * rel_float_tol));
  EXPECT_TRUE(relative_allclose(cov, gold_cov, 100 * rel_float_tol));
}

TEST(GaussianProcessTest_tests, test_surrogates_2D_gp_with_trend_values_derivs_and_save_load) {
  bool print_output = false;

  MatrixXd samples, cov, gold_cov, length_scale_bounds, eval_pts;
  MatrixXd grad, gold_grad, hessian, gold_hessian;
  VectorXd response, sigma_bounds, mean, gold_mean, std_dev, gold_std_dev;

  /* build and eval data */
  get_2D_gp_test_data(samples, response, eval_pts);

  /* bound constraints */
  get_gp_hyperparameter_bounds(2, sigma_bounds, length_scale_bounds);

  /* configuration options */
  ParameterList param_list =
      get_gp_config_options(sigma_bounds, length_scale_bounds);
  param_list.set("num restarts", 20);
  param_list.set("standardize response", false);
  param_list.sublist("Nugget").set("fixed nugget", 0.0);
  param_list.sublist("Nugget").set("estimate nugget", true);
  param_list.sublist("Trend").set("estimate trend", true);
  param_list.sublist("Trend").sublist("Options").set("max degree", 2);

  VectorXd nugget_bounds(2);
  nugget_bounds << 3.17e-8, 1.0e-2;
  param_list.sublist("Nugget").sublist("Bounds").set("lower bound",
                                                  nugget_bounds(0));
  param_list.sublist("Nugget").sublist("Bounds").set("upper bound",
                                                  nugget_bounds(1));

  /* gold data for test */
  get_2D_gp_golds_no_trend(gold_mean, gold_std_dev, gold_cov, true);
  get_2D_gp_golds_trend_derivs(gold_grad, gold_hessian);

  /* relative tolerance for floating point comparisons */
  const double rel_float_tol = 1.0e-4;

  /* 2D GP constructor type 1 */
  GaussianProcess gp(param_list);

  gp.build(samples, response);

  /* MLE history info */
  if (print_output) {
    std::cout << "\n2D GP MLE objective function values history:\n";
    std::cout << gp.get_objective_function_history() << "\n";
    std::cout << "\n2D GP MLE objective function gradient history:\n";
    std::cout << gp.get_objective_gradient_history() << "\n";
    std::cout << "\n2D GP MLE hyperparameter history:\n";
    std::cout << gp.get_theta_history() << "\n";
    std::cout << "\n";
  }

  get_gp_test_arrays(gp, eval_pts, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n2D GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n2D GP standard deviation:\n";
    std::cout << std_dev << "\n";
    std::cout << "\n2D GP covariance:\n";
    std::cout << cov << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
  EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, 100 * rel_float_tol));
  EXPECT_TRUE(relative_allclose(cov, gold_cov, 100 * rel_float_tol));

  /* compute derivatives of GP with trend and check */
  const int eval_point_index = 1;
  auto eval_point = eval_pts.row(eval_point_index);

  grad = gp.gradient(eval_point);
  hessian = gp.hessian(eval_point);

  if (print_output) {
    std::cout << "\n2D trend gp gradient:\n";
    std::cout << grad;
    std::cout << "\n\n";
    std::cout << "2D trend gp hessian:\n";
    std::cout << hessian << "\n\n";
  }

  EXPECT_TRUE(relative_allclose(grad, gold_grad, rel_float_tol));
  EXPECT_TRUE(relative_allclose(hessian, gold_hessian, rel_float_tol));

  MatrixXd grad_fd_error, hessian_fd_error;
  fd_check_gradient(gp, eval_point, grad_fd_error);
  fd_check_hessian(gp, eval_point, hessian_fd_error);

  if (print_output) {
    std::cout << "\n2D trend gradient fd error:\n";
    std::cout << grad_fd_error << "\n";
    std::cout << "\n2D trend hessian fd error:\n";
    std::cout << hessian_fd_error << "\n";
    std::cout << "\n";
  }

  VectorXd grad_drop(2);     // dx, dy
  VectorXd hessian_drop(3);  // dxx, dxy, dyy

  for (int i = 0; i < 2; i++) {
    grad_drop(i) =
        log10(grad_fd_error.col(i)(0) / grad_fd_error.col(i).minCoeff());

    hessian_drop(i) =
        log10(hessian_fd_error.col(i)(0) / hessian_fd_error.col(i).minCoeff());

    EXPECT_TRUE((grad_drop(i) > 6.0));
    EXPECT_TRUE((hessian_drop(i) > 3.0));

    if (i == 1) {
      i += 1;
      hessian_drop(i) = log10(hessian_fd_error.col(i)(0) /
                              hessian_fd_error.col(i).minCoeff());
      EXPECT_TRUE((hessian_drop(i) > 3.0));
    }
  }

  // Initially modelling what save/load functions would do for binary/text
  VectorXd mean_save, mean_load, std_dev_load;
  MatrixXd cov_load, grad_save, grad_load, hess_save, hess_load;
  std::string filename("gp_test.surr");

  for (bool binary : {true, false}) {
    boost::filesystem::remove(filename);
    Surrogate::save(gp, filename, binary);

    GaussianProcess gp_loaded;
    Surrogate::load(filename, binary, gp_loaded);

    mean_save = gp.value(eval_pts);
    mean_load = gp_loaded.value(eval_pts);

    grad_save = gp.gradient(eval_point);
    grad_load = gp_loaded.gradient(eval_point);

    hess_save = gp.hessian(eval_point);
    hess_load = gp_loaded.hessian(eval_point);

    // Verify saved vs. loaded to tight tolerance
    const double tight_tol = 1.0e-16;
    EXPECT_TRUE(matrix_equals(mean_save, mean_load, tight_tol));
    EXPECT_TRUE(matrix_equals(grad_save, grad_load, tight_tol));
    EXPECT_TRUE(matrix_equals(hess_save, hess_load, tight_tol));

    // Verify vs. original unit test
    get_gp_test_arrays(gp, eval_pts, mean_load, std_dev_load, cov_load);
    EXPECT_TRUE(relative_allclose(mean_load, gold_mean, rel_float_tol));
    EXPECT_TRUE(
        relative_allclose(std_dev_load, gold_std_dev, 100 * rel_float_tol));
    EXPECT_TRUE(relative_allclose(cov_load, gold_cov, 100 * rel_float_tol));
  }
}

TEST(GaussianProcessTest_tests, test_surrogates_matern_32_gp) {
  bool print_output = false;

  const std::string kernel_type = "Matern 3/2";

  MatrixXd samples, cov, length_scale_bounds;
  VectorXd response, eval_pts;
  VectorXd sigma_bounds, mean, gold_mean, std_dev, gold_std_dev;

  /* 1D GP tests */

  /* build and eval data */
  get_1D_gp_test_data(samples, response, eval_pts);

  /* bound constraints */
  get_gp_hyperparameter_bounds(1, sigma_bounds, length_scale_bounds);

  /* configuration options */
  ParameterList param_list =
      get_gp_config_options(sigma_bounds, length_scale_bounds);

  /* gold data for test */
  get_1D_gp_golds_matern(gold_mean, gold_std_dev, kernel_type);

  /* relative tolerance for floating point comparisons */
  const double rel_float_tol = 1.0e-4;

  GaussianProcess gp_1D(param_list);

  ParameterList current_opts;
  gp_1D.get_options(current_opts);
  current_opts.set("kernel type", kernel_type);
  current_opts.sublist("Nugget").set("fixed nugget", 0.0);
  param_list.set("num restarts", 10);
  gp_1D.set_options(current_opts);

  gp_1D.build(samples, response);

  get_gp_test_arrays(gp_1D, eval_pts, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n1D Matern 3/2 GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n1D Matern 3/2 GP standard deviation:\n";
    std::cout << std_dev << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
  EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, rel_float_tol));

  /* compute gradient of GP and check */
  /* The Matern 3/2 GP produces C^1-smooth functions and therefore
   * does not have a Hessian */
  const int eval_point_index = 0;
  auto eval_point = eval_pts.row(eval_point_index);
  MatrixXd grad;

  grad = gp_1D.gradient(eval_point);

  MatrixXd grad_fd_error;
  fd_check_gradient(gp_1D, eval_point, grad_fd_error);

  if (print_output) {
    std::cout << "\n1D Matern 3/2 GP gradient:\n";
    std::cout << grad;

    std::cout << "\n1D Matern 3/2 GP fd error:\n";
    std::cout << grad_fd_error << "\n";
  }

  double grad_drop;
  grad_drop = log10(grad_fd_error(0, 0) / grad_fd_error.minCoeff());
  EXPECT_TRUE((grad_drop > 6.0));

  /* 2D GP tests */

  /* build and eval data */
  MatrixXd eval_pts_2D;
  get_2D_gp_test_data(samples, response, eval_pts_2D);

  /* bound constraints */
  get_gp_hyperparameter_bounds(2, sigma_bounds, length_scale_bounds);

  /* configuration options */
  param_list = get_gp_config_options(sigma_bounds, length_scale_bounds);

  /* gold data for test */
  get_2D_gp_golds_matern(gold_mean, gold_std_dev, kernel_type);

  GaussianProcess gp_2D(param_list);

  gp_2D.get_options(current_opts);
  current_opts.set("kernel type", kernel_type);
  current_opts.sublist("Nugget").set("fixed nugget", 0.0);
  param_list.set("num restarts", 20);
  gp_2D.set_options(current_opts);

  gp_2D.build(samples, response);

  get_gp_test_arrays(gp_2D, eval_pts_2D, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n2D Matern 3/2 GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n2D Matern 3/2 GP standard deviation:\n";
    std::cout << std_dev << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
  EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, 100 * rel_float_tol));

  /* compute derivatives of GP and check */
  const int eval_point_index_2D = 1;
  auto eval_point_2D = eval_pts_2D.row(eval_point_index_2D);

  fd_check_gradient(gp_2D, eval_point_2D, grad_fd_error);

  if (print_output) {
    std::cout << "\n2D Matern 3/2 GP gradient fd error:\n";
    std::cout << grad_fd_error << "\n";
  }

  VectorXd grad_drop_2D(2);  // dx, dy

  for (int i = 0; i < 2; i++) {
    grad_drop_2D(i) =
        log10(grad_fd_error.col(i)(0) / grad_fd_error.col(i).minCoeff());
    EXPECT_TRUE((grad_drop_2D(i) > 6.0));
  }
}

TEST(GaussianProcessTest_tests, test_surrogates_matern_52_gp) {
  bool print_output = false;

  const std::string kernel_type = "Matern 5/2";

  MatrixXd samples, cov, length_scale_bounds;
  VectorXd response, eval_pts;
  VectorXd sigma_bounds, mean, gold_mean, std_dev, gold_std_dev;

  /* 1D GP tests */

  /* build and eval data */
  get_1D_gp_test_data(samples, response, eval_pts);

  /* bound constraints */
  get_gp_hyperparameter_bounds(1, sigma_bounds, length_scale_bounds);

  /* configuration options */
  ParameterList param_list =
      get_gp_config_options(sigma_bounds, length_scale_bounds);

  /* gold data for test */
  get_1D_gp_golds_matern(gold_mean, gold_std_dev, kernel_type);

  /* relative tolerance for floating point comparisons */
  const double rel_float_tol = 1.0e-4;

  GaussianProcess gp_1D(param_list);

  ParameterList current_opts;
  gp_1D.get_options(current_opts);
  current_opts.set("kernel type", kernel_type);
  current_opts.sublist("Nugget").set("fixed nugget", 0.0);
  param_list.set("num restarts", 10);
  gp_1D.set_options(current_opts);

  gp_1D.build(samples, response);

  get_gp_test_arrays(gp_1D, eval_pts, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n1D Matern 5/2 GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n1D Matern 5/2 GP standard deviation:\n";
    std::cout << std_dev << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
  EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, rel_float_tol));

  /* compute derivatives of GP with trend and check */
  const int eval_point_index = 0;
  auto eval_point = eval_pts.row(eval_point_index);
  MatrixXd grad, hessian;

  grad = gp_1D.gradient(eval_point);
  hessian = gp_1D.hessian(eval_point);

  MatrixXd grad_fd_error, hessian_fd_error;
  fd_check_gradient(gp_1D, eval_point, grad_fd_error);
  fd_check_hessian(gp_1D, eval_point, hessian_fd_error);

  if (print_output) {
    std::cout << "\n1D Matern 5/2 GP gradient:\n";
    std::cout << grad;

    std::cout << "\n1D Matern 5/2 GP hessian:\n";
    std::cout << hessian;

    std::cout << "\n1D Matern 5/2 GP gradient fd error:\n";
    std::cout << grad_fd_error << "\n";

    std::cout << "\n1D Matern 5/2 GP hessian fd error:\n";
    std::cout << hessian_fd_error << "\n";
  }

  double grad_drop, hessian_drop;
  grad_drop = log10(grad_fd_error(0, 0) / grad_fd_error.minCoeff());
  hessian_drop = log10(hessian_fd_error(0, 0) / hessian_fd_error.minCoeff());

  EXPECT_TRUE((grad_drop > 6.0));
  EXPECT_TRUE((hessian_drop > 3.0));

  /* 2D GP tests */

  /* build and eval data */
  MatrixXd eval_pts_2D;
  get_2D_gp_test_data(samples, response, eval_pts_2D);

  /* bound constraints */
  get_gp_hyperparameter_bounds(2, sigma_bounds, length_scale_bounds);

  /* configuration options */
  param_list = get_gp_config_options(sigma_bounds, length_scale_bounds);

  /* gold data for test */
  get_2D_gp_golds_matern(gold_mean, gold_std_dev, kernel_type);

  GaussianProcess gp_2D(param_list);

  gp_2D.get_options(current_opts);
  current_opts.set("kernel type", kernel_type);
  current_opts.sublist("Nugget").set("fixed nugget", 0.0);
  current_opts.set("num restarts", 20);
  gp_2D.set_options(current_opts);

  gp_2D.build(samples, response);

  get_gp_test_arrays(gp_2D, eval_pts_2D, mean, std_dev, cov);

  if (print_output) {
    std::cout << "\n\n2D Matern 5/2 GP mean:\n";
    std::cout << mean << "\n";
    std::cout << "\n2D Matern 5/2 GP standard deviation:\n";
    std::cout << std_dev << "\n";
  }

  EXPECT_TRUE(relative_allclose(mean, gold_mean, rel_float_tol));
  EXPECT_TRUE(relative_allclose(std_dev, gold_std_dev, 100 * rel_float_tol));

  /* compute derivatives of GP and check */
  const int eval_point_index_2D = 1;
  auto eval_point_2D = eval_pts_2D.row(eval_point_index_2D);

  fd_check_gradient(gp_2D, eval_point_2D, grad_fd_error);
  fd_check_hessian(gp_2D, eval_point_2D, hessian_fd_error);

  if (print_output) {
    std::cout << "\n2D Matern 5/2 GP gradient fd error:\n";
    std::cout << grad_fd_error << "\n";
    std::cout << "\n2D Matern 5/2 GP hessian fd error:\n";
    std::cout << hessian_fd_error << "\n";
  }

  VectorXd grad_drop_2D(2);     // dx, dy
  VectorXd hessian_drop_2D(3);  // dxx, dxy, dyy

  for (int i = 0; i < 2; i++) {
    grad_drop_2D(i) =
        log10(grad_fd_error.col(i)(0) / grad_fd_error.col(i).minCoeff());

    hessian_drop_2D(i) =
        log10(hessian_fd_error.col(i)(0) / hessian_fd_error.col(i).minCoeff());

    EXPECT_TRUE((grad_drop_2D(i) > 6.0));
    EXPECT_TRUE((hessian_drop_2D(i) > 3.0));

    if (i == 1) {
      i += 1;
      hessian_drop_2D(i) = log10(hessian_fd_error.col(i)(0) /
                                 hessian_fd_error.col(i).minCoeff());
      EXPECT_TRUE((hessian_drop_2D(i) > 3.0));
    }
  }
}

#ifndef DISABLE_YAML_SURROGATES_CONFIG
TEST(GaussianProcessTest_tests, test_surrogates_gp_read_from_parameterlist) {
  std::string test_parameterlist_file =
      "gp_test_data/GP_test_parameterlist.yaml";
  //"gp_test_data/GP_test_parameterlist.xml";
  GaussianProcess gp(test_parameterlist_file);

  ParameterList plist;
  gp.get_options(plist);

  const double rel_float_tol = 1.0e-12;

  EXPECT_TRUE((plist.get<std::string>("scaler name") == "standardization"));
  EXPECT_TRUE((plist.get<int>("num restarts") == 10));
  EXPECT_TRUE((plist.get<int>("gp seed") == 42));

  const ParameterList plist_nugget = plist.get<ParameterList>("Nugget");
  EXPECT_LT(std::fabs(1. - plist_nugget.get<double>("fixed nugget") / 1.0e-14),
                         100.0*rel_float_tol/100. );
  EXPECT_TRUE((plist_nugget.get<bool>("estimate nugget") == false));

  const ParameterList plist_trend = plist.get<ParameterList>("Trend");
  EXPECT_TRUE((plist_trend.get<bool>("estimate trend") == false));

  const ParameterList plist_options = plist_trend.get<ParameterList>("Options");
  EXPECT_TRUE((plist_options.get<int>("max degree") == 2));
  EXPECT_LT(std::fabs(1. - plist_options.get<double>("p-norm") / 1.0),
                         100.0*rel_float_tol/100. );
  EXPECT_TRUE((plist_options.get<std::string>("scaler type") == "none"));
  EXPECT_TRUE((plist_options.get<std::string>("regression solver type") ==
                "SVD"));
}
#endif

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

}  // namespace
