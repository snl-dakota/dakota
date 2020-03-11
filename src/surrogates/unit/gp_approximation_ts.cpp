/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <fstream>
#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_ParameterList.hpp>
#include "CommonUtils.hpp"
#include "GaussianProcess.hpp"

// BMA TODO: Review with team for best practice
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

  Teuchos::ParameterList param_list("GP Test Parameters");
  param_list.set("sigma_bounds", sigma_bounds);
  param_list.set("length_scale_bounds", length_scale_bounds);
  param_list.set("scaler_type",
		 std::string("standardization")
		 //std::string("none")
		 );
  param_list.set("num_restarts", int(10));
  param_list.set("nugget", double(1.0e-12));
  param_list.set("gp_seed", int(42));

  /* 1D GP test */
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
  param_list.set("sigma_bounds", sigma_bounds);
  param_list.set("length_scale_bounds", length_scale_bounds);
  param_list.set("nugget", double(1.0e-10));

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
  gp_2D.gradient(eval_point,gp_grad);

  
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
  gp_2D.hessian(eval_point,gp_hessian);

  if (print_output) {
    std::cout << "\nGP Hessian value at evaluation point:\n" << gp_hessian << std::endl;
  }

  if (!matrix_equals(gp_hessian,gold_gp_hessian,atol)){
    std::cout << "8\n";
    return 8;
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
