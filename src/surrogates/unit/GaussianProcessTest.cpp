/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "common_utils.hpp"
#include "GaussianProcess.hpp"
#include "surrogates_tools.hpp"
#include "util_data_types.hpp"

#include <Teuchos_UnitTestHarness.hpp>

#include <fstream>

// BMA TODO: Review with team for best practice
using namespace dakota;
using namespace dakota::surrogates;
using namespace dakota::util;

namespace {

int test_gp(double atol){

  // print output
  bool print_output = true;

  // gp parameters
  int num_qoi = 1; // only using 1 for now

  // num_samples x num_features
  MatrixXd xs_u(7,1);
  // num_samples x num_qoi
  MatrixXd response(7,1);
  // num_eval_pts x num_features
  MatrixXd eval_pts(6,1);
  // num_eval_pts x num_qoi
  MatrixXd pred(6,1);

  /* samples */
  xs_u << 0.05536604, 0.28730518, 0.30391231, 0.40768703,
          0.45035059, 0.52639952, 0.78853488;

  response << -0.15149429, -0.19689361, -0.17323105, -0.02379026,
               0.02013445, 0.05011702, -0.11678312;

  eval_pts << 0.0, 0.2, 0.4, 0.6, 0.8, 1.0;
                             
  // bound constraints -- will be converted to log-scale internally
  // sigma bounds - lower and upper
  VectorXd sigma_bounds(2);
  sigma_bounds(0) = 1.0e-2;
  sigma_bounds(1) = 1.0e2;
  // length scale bounds - num_vars x 2
  MatrixXd length_scale_bounds(1,2);
  length_scale_bounds(0,0) = 1.0e-2;
  length_scale_bounds(0,1) = 1.0e2;

  ParameterList param_list("GP Test Parameters");
  param_list.set("sigma bounds", sigma_bounds);
  param_list.set("length-scale bounds", length_scale_bounds);
  param_list.set("scaler name","standardization");
  param_list.set("num restarts", 10);
  param_list.sublist("Nugget").set("fixed nugget", 1.0e-12);
  param_list.set("gp seed", 42);

  /* 1D GP test #1: Construct GP and build surrogate all
   * at once */
  GaussianProcess gp(xs_u, response, param_list);

  gp.value(eval_pts, pred);
  VectorXd std_dev = gp.get_posterior_std_dev();
  MatrixXd cov = gp.get_posterior_covariance();

  // gold values
  MatrixXd gold_value(6,1);
  VectorXd gold_std(6);
  MatrixXd gold_cov(6,6);

  gold_value << -0.046014, -0.278509, -0.0333528, 0.0185393, -0.118491, -0.0506785;
  gold_std << 0.0170695, 0.00203616, 1.67823e-05, 0.00317294, 0.00392892, 0.121506;
  gold_cov << 0.000291366, -3.13983e-05, -2.05299e-07, 2.82111e-05, -2.23603e-05, -0.000337062,
             -3.13983e-05, 4.14595e-06, 3.16567e-08, -4.86909e-06, 4.17091e-06, 6.60782e-05,
             -2.05299e-07, 3.16567e-08, 2.81647e-10, -4.93076e-08, 4.72685e-08, 8.16402e-07,
              2.82111e-05, -4.86909e-06, -4.93076e-08, 1.00675e-05, -1.12689e-05, -0.000225718,
             -2.23603e-05, 4.17091e-06, 4.72685e-08, -1.12689e-05, 1.54364e-05, 0.00039519,
             -0.000337062, 6.60782e-05, 8.16402e-07, -0.000225718, 0.00039519, 0.0147636;

  if (print_output) {
    std::cout << "\n\n1D GP mean:" << std::endl;
    std::cout << pred << std::endl;
    std::cout << "\n1D GP standard deviation:" << std::endl;
    std::cout << std_dev << std::endl;
    std::cout << "\n1D GP covariance:" << std::endl;
    std::cout << cov << std::endl;
  }

  if (!matrix_equals(pred,gold_value,atol)){
    std::cout << "1\n";
    return 1;
  }

  if (!matrix_equals(std_dev,gold_std,atol)){
    std::cout << "2\n";
    return 2;
  }

  if (!matrix_equals(cov,gold_cov,atol)){
    std::cout << "3\n";
    return 3;
  }
  
  /* 1D GP test #2:
   * Separate constructor with given options
   * and build steps */
  GaussianProcess gp2(param_list);
  gp2.build(xs_u, response);
  gp2.value(eval_pts, pred);
  std_dev = gp2.get_posterior_std_dev();
  cov = gp2.get_posterior_covariance();

  if (!matrix_equals(pred,gold_value,atol)){
    std::cout << "7\n";
    return 7;
  }

  if (!matrix_equals(std_dev,gold_std,atol)){
    std::cout << "8\n";
    return 8;
  }

  if (!matrix_equals(cov,gold_cov,atol)){
    std::cout << "9\n";
    return 9;
  }

  /* 1D GP test #3:
   * use defaultConfigOptions and adjust as needed
   * for proper behavior */
  GaussianProcess gp3;
  ParameterList current_opts;
  gp3.get_options(current_opts);
  current_opts.set("scaler name", "standardization");
  current_opts.sublist("Nugget").set("fixed nugget", 1.0e-12);
  current_opts.set("gp seed", 42);
  gp3.set_options(current_opts);
  gp3.build(xs_u, response);

  gp3.value(eval_pts, pred);
  std_dev = gp3.get_posterior_std_dev();
  cov = gp3.get_posterior_covariance();

  if (!matrix_equals(pred,gold_value,atol)){
    std::cout << "10\n";
    return 10;
  }

  if (!matrix_equals(std_dev,gold_std,atol)){
    std::cout << "11\n";
    return 11;
  }

  if (!matrix_equals(cov,gold_cov,atol)){
    std::cout << "12\n";
    return 12;
  }

  std::cout << "old gp value:" << std::endl;
  std::cout << pred << std::endl;
  std::cout << "\n";

  /* 1D GP test #4:
   * use defaultConfigOptions and introduce a polynomial trend
   * and nugget estimation */
  // gold values
  MatrixXd gold_value4(6,1);
  VectorXd gold_std4(6);
  MatrixXd gold_cov4(6,6);

  gold_value4 << -0.05312219, -0.27214983, -0.033252, 0.02527716, -0.12356696, -0.18573136;
  gold_std4 << 0.02028642, 0.00536066, 0.00064835, 0.00459461, 0.00395678, 0.14659393;
  gold_cov4 << 0.000411539, -9.13625e-05,  6.17757e-08,  -1.9461e-05,  5.94823e-06, -0.000256977,
             -9.13625e-05,  2.87366e-05, -5.31468e-07,  6.01025e-06, -4.53635e-06, -5.15403e-05,
              6.17757e-08, -5.31468e-07,  4.20359e-07,  8.39709e-07, -4.44025e-07, -8.63467e-06,
             -1.9461e-05,  6.01025e-06,  8.39709e-07,  2.11104e-05, -1.52399e-05, -0.000388392,
              5.94823e-06, -4.53635e-06, -4.44025e-07, -1.52399e-05,  1.56561e-05,  0.000499163,
             -0.000256977, -5.15403e-05, -8.63467e-06, -0.000388392,  0.000499163,    0.0214898;


  GaussianProcess gp4;
  ParameterList current_opts4;
  gp4.get_options(current_opts4);
  current_opts4.set("scaler name", "standardization");
  current_opts4.sublist("Nugget").set("fixed nugget", 0.0);
  current_opts4.set("gp seed", 42);
  current_opts4.sublist("Nugget").set("estimate nugget", true);
  current_opts4.sublist("Trend").set("estimate trend", true);
  current_opts4.sublist("Trend").sublist("Options").set("max degree", 1);
  /* debugging */
  current_opts4.set("num restarts", 20);
  gp4.set_options(current_opts4);
  gp4.build(xs_u, response);

  gp4.value(eval_pts, pred);
  std_dev = gp4.get_posterior_std_dev();
  cov = gp4.get_posterior_covariance();

  if (print_output) {
    std::cout << "gp4 value:" << std::endl;
    std::cout << pred << std::endl;
    std::cout << "\n";

    std::cout << "gp4 std_dev:" << std::endl;
    std::cout << std_dev << std::endl;
    std::cout << "\n";

    std::cout << "gp4 cov:" << std::endl;
    std::cout << cov << std::endl;
    std::cout << "\n";
  }

  if (!matrix_equals(pred,gold_value4,atol)){
    std::cout << "13\n";
    return 13;
  }

  if (!matrix_equals(std_dev,gold_std4,atol)){
    std::cout << "14\n";
    return 14;
  }

  if (!matrix_equals(cov,gold_cov4,atol)){
    std::cout << "15\n";
    return 15;
  }

  /* compute derivatives of GP with trend and check */
  MatrixXd gp4_gradient, gp4_hessian;

  gp4.gradient(eval_pts.row(0), gp4_gradient);
  gp4.hessian(eval_pts.row(0), gp4_hessian);

  std::cout << "GP with trend gradient:" << std::endl;
  std::cout << gp4_gradient;
  std::cout << "\n";

  std::cout << "GP with trend hessian:" << std::endl;
  std::cout << gp4_hessian;
  std::cout << "\n";

  MatrixXd grad_fd_error_trend;
  fd_check_gradient(gp4, eval_pts.row(0), grad_fd_error_trend); 
  std::cout << "\nGP with trend gradient fd error:" << std::endl;
  std::cout << grad_fd_error_trend << std::endl;
  std::cout << "\n";

  MatrixXd hessian_fd_error_trend;
  fd_check_hessian(gp4, eval_pts.row(0), hessian_fd_error_trend); 
  std::cout << "\nGP with trend hessian fd error:" << std::endl;
  std::cout << hessian_fd_error_trend << std::endl;
  std::cout << "\n";

  /* 2D GP test */
  int num_datasets = 1;
  int num_vars = 2;
  int num_samples = 64;

  /* bounds */
  /* 64 pts - herbie and smooth herbie */
  sigma_bounds(0) = 1.0e-2;
  sigma_bounds(1) = 1.0e+2;
  length_scale_bounds.resize(2,2);
  length_scale_bounds(0,0) = 1.0e-2;
  length_scale_bounds(1,0) = 1.0e-2;
  length_scale_bounds(0,1) = 1.0e+2;
  length_scale_bounds(1,1) = 1.0e+2;

  /* 64 pts - rosenbrock */
  /*
  sigma_bounds(0) = 1.0e3;
  sigma_bounds(1) = 1.0e4;
  length_scale_bounds.reshape(2,2);
  length_scale_bounds(0,0) = 1.0e-1;
  length_scale_bounds(1,0) = 1.0e-1;
  length_scale_bounds(0,1) = 1.0e1;
  length_scale_bounds(1,1) = 1.0e1;
  */

  /* 64 pts - shubert */
  /*
  sigma_bounds(0) = 1.0e-2;
  sigma_bounds(1) = 1.0e+2;
  length_scale_bounds.reshape(2,2);
  length_scale_bounds(0,0) = 1.0e-2;
  length_scale_bounds(1,0) = 1.0e-2;
  length_scale_bounds(0,1) = 1.0e+1;
  length_scale_bounds(1,1) = 1.0e+1;
  */

  // Update ParameterList for this test
  param_list.set("sigma bounds", sigma_bounds);
  param_list.set("length-scale bounds", length_scale_bounds);
  param_list.sublist("Nugget").set("fixed nugget", 1.0e-10);

  std::string samples_fname = "gp_test_data/lhs_data_64.txt";
  std::string responses_fname = "gp_test_data/smooth_herbie_64.txt";
  //std::string responses_fname = "gp_test_data/herbie_64.txt";
  //std::string responses_fname = "gp_test_data/rosenbrock_64.txt";
  //std::string responses_fname = "gp_test_data/shubert_64.txt";

  std::vector<MatrixXd> responses_list;
  std::vector<MatrixXd> samples_list;

  populateMatricesFromFile(samples_fname,samples_list,num_datasets,num_vars,num_samples);
  populateMatricesFromFile(responses_fname,responses_list,num_datasets,num_qoi,num_samples);

  if( 0 ) // used to dump data for use in the corresponding unit test in src/unit_test/reduced_basis.cpp
  {
    std::cout << "Samples :\n" << samples_list[0] << std::endl;
    std::cout << "\n\nResponses :\n" << responses_list[0] << std::endl;
  }

  /*four evaluation points for the test */
  MatrixXd eval_pts_2D(4,2);
  MatrixXd pred_2D(4,1);

  eval_pts_2D << 0.2, 0.45,
                -0.3, -0.7,
                 0.4, -0.1,
                -0.25, 0.33;

  MatrixXd gold_value_2D(4,1);
  VectorXd gold_std_2D(4);
  MatrixXd gold_cov_2D(4,4);

  gold_value_2D << 0.779863, 0.84671, 0.744502, 0.746539;
  gold_std_2D << 0.000202807, 0.00157021, 0.000266543, 0.000399788;
  gold_cov_2D << 4.11307e-08, 5.05967e-08, -6.56123e-09, -3.19852e-08,
                 5.05967e-08, 2.46557e-06, -2.8656e-07, 2.18488e-07,
                -6.56123e-09, -2.8656e-07, 7.10453e-08, -7.75076e-08,
                -3.19852e-08, 2.18488e-07, -7.75076e-08, 1.5983e-07;

  // not used for testing ...
  //for (int k = 0; k < num_datasets; k++) {
    //std::cout  << "On dataset " << k << std::endl;
  //}

  std::cout << "\n";
  GaussianProcess gp_2D(samples_list[0],responses_list[0], param_list);
  gp_2D.value(eval_pts_2D, pred_2D);
  VectorXd std_dev_2D = gp_2D.get_posterior_std_dev();
  MatrixXd cov_2D = gp_2D.get_posterior_covariance();

  if (!matrix_equals(pred_2D,gold_value_2D,atol)){
    std::cout << "4\n";
    return 4;
  }

  if (!matrix_equals(std_dev_2D,gold_std_2D,atol)){
    std::cout << "5\n";
    return 5;
  }

  if (!matrix_equals(cov_2D,gold_cov_2D,atol)){
    std::cout << "6\n";
    return 6;
  }

  if (print_output) {
    std::cout << "\n\n2D GP mean:" << std::endl;
    std::cout << pred_2D << std::endl;
    std::cout << "\n2D GP standard deviation:" << std::endl;
    std::cout << std_dev_2D << std::endl;
    std::cout << "\n2D GP covariance:" << std::endl;
    std::cout << cov_2D << std::endl;
  }

  /* try to compute the derivative at a point */
  int eval_index = 1;
  auto eval_point = eval_pts_2D.row(eval_index);

  MatrixXd gp_grad;
  MatrixXd gold_gp_grad(1,2);
  gold_gp_grad << -0.31280824, -0.25430975;
  gp_2D.gradient(eval_point, gp_grad);

  
  if (print_output) {
    std::cout << "\nGP grad value at evaluation point:\n" << gp_grad << std::endl;
  }

  if (!matrix_equals(gp_grad,gold_gp_grad,atol)){
    std::cout << "7\n";
    return 7;
  }

  MatrixXd gp_hessian;
  MatrixXd gold_gp_hessian(2,2);
  gold_gp_hessian << 0.87452373, 0.1014484, 0.1014484, -0.84271328;
  gp_2D.hessian(eval_point, gp_hessian);

  if (print_output) {
    std::cout << "\nGP Hessian value at evaluation point:\n" << gp_hessian << std::endl;
  }

  if (!matrix_equals(gp_hessian,gold_gp_hessian,atol)){
    std::cout << "8\n";
    return 8;
  }

  /* Now build 2D function with trend and check gradient/hessian */

  MatrixXd grad_fd_error;
  fd_check_gradient(gp_2D, eval_point, grad_fd_error);
  if (print_output) {
    std::cout << "\ngradient fd error:" << std::endl;
    std::cout << grad_fd_error << std::endl;
    std::cout << "\n";
  }

  MatrixXd hessian_fd_error;
  fd_check_hessian(gp_2D, eval_point, hessian_fd_error);
  if (print_output) {
    std::cout << "\nhessian fd error:" << std::endl;
    std::cout << hessian_fd_error << std::endl;
    std::cout << "\n";
  }
  
  /* test a 2D gp with a quadratic trend and nugget estimation */
  ParameterList pl_2D_quad("2D Quadratic GP with Nugget Estimation Test Parameters");
  pl_2D_quad.set("scaler name","standardization");
  pl_2D_quad.set("num restarts", 20);
  pl_2D_quad.sublist("Nugget").set("fixed nugget", 0.0);
  pl_2D_quad.set("gp seed", 42);
  pl_2D_quad.set("sigma bounds", sigma_bounds);
  pl_2D_quad.set("length-scale bounds", length_scale_bounds);
  pl_2D_quad.sublist("Nugget").set("estimate nugget", true);
  pl_2D_quad.sublist("Trend").set("estimate trend", true);
  pl_2D_quad.sublist("Trend").sublist("Options").set("max degree", 2);

  GaussianProcess gp_2D_quad(samples_list[0], responses_list[0], pl_2D_quad);
  gp_2D_quad.value(eval_pts_2D, pred_2D);
  gp_2D.gradient(eval_point, gp_grad);
  gp_2D.hessian(eval_point, gp_hessian);

  if (print_output) {
    std::cout << "2D trend gp gradient:\n";
    std::cout << gp_grad;
    std::cout << "\n\n";
    std::cout << "2D trend gp hessian:\n";
    std::cout << gp_hessian << "\n\n";
  }

  fd_check_gradient(gp_2D_quad, eval_point, grad_fd_error);
  if (print_output) {
    std::cout << "\n2D trend gradient fd error:" << std::endl;
    std::cout << grad_fd_error << std::endl;
    std::cout << "\n";
  }

  fd_check_hessian(gp_2D_quad, eval_point, hessian_fd_error);
  if (print_output) {
    std::cout << "\n2D trend hessian fd error:" << std::endl;
    std::cout << hessian_fd_error << std::endl;
    std::cout << "\n";
  }

  gold_value_2D << 0.77987534, 0.84715045, 0.74437935, 0.74654155;
  gold_gp_grad << -0.312808, -0.25431;
  gold_gp_hessian << 0.874524,  0.101448, 0.101448, -0.842713;

  if (!matrix_equals(pred_2D,gold_value_2D,atol)){
    std::cout << "16\n";
    return 16;
  }

  if (!matrix_equals(gp_grad,gold_gp_grad,atol)){
    std::cout << "8\n";
    return 17;
  }

  if (!matrix_equals(gp_hessian,gold_gp_hessian,atol)){
    std::cout << "8\n";
    return 18;
  }

  std::cout << "\n\n";

  return 0;
}

//----------------------------------------------------------------
// Unit tests

/**
\brief Create a 1D GP and 2D from sample data and compare their means, 
       standard deviations, and covariance matrices
*/
TEUCHOS_UNIT_TEST(surrogates, gaussian_process)
{
  TEST_ASSERT(!test_gp(5e-7));
}

}
