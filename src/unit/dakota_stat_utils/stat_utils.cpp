/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "NonDBayesCalibration.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "bayes_calibration_utils.hpp"
#include "dakota_stat_util.hpp"
#include <random>
#include <thread>

#include <string>

#include <gtest/gtest.h>

using namespace Dakota;

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_system_seed)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(1, 1999); // avg sleep = 1 ms

  unsigned int num_out_of_bounds_seeds = 0;
  // 1000 trials will, on average, span 1 sec = 1000000 microseconds
  for (unsigned int i=0; i<1000; ++i) {
    int seed = generate_system_seed();
    if (seed < 1 || seed > 1000000)
      ++num_out_of_bounds_seeds;
    std::this_thread::sleep_for(std::chrono::microseconds(dis(gen)));
  }
  EXPECT_TRUE((num_out_of_bounds_seeds == 0));
}

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_kl_divergence)
{
  // Read in matrices 
  std::ifstream infile1("stat_util_test_files/Matrix1.txt");
  RealMatrix dist1;
  dist1.shapeUninitialized(1,1000);
  for (int i = 0; i < 1000 && (infile1 >> dist1[0][i]); ++i){  }
  std::ifstream infile2("stat_util_test_files/Matrix2.txt");
  RealMatrix dist2;
  dist2.shapeUninitialized(1,1000);
  for (int i = 0; i < 1000 && (infile2 >> dist2[0][i]); ++i){  }

  // Function being tested
  Real kl_est = NonDBayesCalibration::knn_kl_div(dist1, dist2, 1);

  Real gold_kl = 0.02459600677;
  EXPECT_LT(std::fabs(1. - kl_est / gold_kl), 1.e-3/100. );
}

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_mutual_info_ksg1)
{
  // Read in matrices 
  std::ifstream infile1("stat_util_test_files/Matrix1.txt");
  std::ifstream infile2("stat_util_test_files/Matrix2.txt");
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(2,1000);
  for (int i = 0; i < 1000; ++i){
    infile1 >> Xmatrix[i][0];
    infile2 >> Xmatrix[i][1];
  }

  // Function being tested
  Real mutualinfo_est = NonDBayesCalibration::knn_mutual_info(Xmatrix, 1, 1, 0);

  Real gold_mi = -0.02189544513;
  EXPECT_LT(std::fabs(1. - mutualinfo_est / gold_mi), 1.e-3/100. );
}

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_mutual_info_ksg2)
{
  // Read in matrices 
  std::ifstream infile1("stat_util_test_files/Matrix1.txt");
  std::ifstream infile2("stat_util_test_files/Matrix2.txt");
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(2,1000);
  for (int i = 0; i < 1000; ++i){
    infile1 >> Xmatrix[i][0];
    infile2 >> Xmatrix[i][1];
  }

  // Function being tested
  Real mutualinfo_est = NonDBayesCalibration::knn_mutual_info(Xmatrix, 1, 1, 1);

  Real gold_mi = -0.0561375052;
  EXPECT_LT(std::fabs(1. - mutualinfo_est / gold_mi), 5.e-1/100. );
}

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_batch_means_mean)
{
  // Read in matrices 
  std::ifstream infile1("stat_util_test_files/Matrix1.txt");
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(1,1000);
  for (int i = 0; i < 1000; ++i){
    infile1 >> Xmatrix[i][0];
  }
  RealMatrix interval_matrix;
  interval_matrix.shapeUninitialized(2,1);
  RealMatrix means_matrix;
  batch_means_interval(Xmatrix, interval_matrix, means_matrix, 1, 0.95);

  Real gold_lower_int = -6.3120595090e-02;
  Real gold_upper_int = 8.1516649910e-02;
  EXPECT_LT(std::fabs(1. - interval_matrix[0][0] / gold_lower_int), 1.e-3/100. );
  EXPECT_LT(std::fabs(1. - interval_matrix[0][1] / gold_upper_int), 1.e-3/100. );
}

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_batch_means_variance)
{
  // Read in matrices 
  std::ifstream infile1("stat_util_test_files/Matrix1.txt");
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(1,1000);
  for (int i = 0; i < 1000; ++i){
    infile1 >> Xmatrix[i][0];
  }
  RealMatrix interval_matrix;
  interval_matrix.shapeUninitialized(2,1);
  RealMatrix means_matrix;
  batch_means_interval(Xmatrix, interval_matrix, means_matrix, 2, 0.95);

  Real gold_lower_int = 9.1432956019e-01;
  Real gold_upper_int = 1.0688302101e+00;
  EXPECT_LT(std::fabs(1. - interval_matrix[0][0] / gold_lower_int), 1.e-3/100. );
  EXPECT_LT(std::fabs(1. - interval_matrix[0][1] / gold_upper_int), 1.e-3/100. );
}

//------------------------------------

TEST(stat_utils_tests, test_stat_utils_batch_means_percentile)
{
  // Read in matrices 
  std::ifstream infile1("stat_util_test_files/Matrix1.txt");
  RealMatrix Xmatrix;
  Xmatrix.shapeUninitialized(1,1000);
  for (int i = 0; i < 1000; ++i){
    infile1 >> Xmatrix[i][0];
  }
  //std::cout << "M = " << Xmatrix << std::endl;
  RealMatrix interval_matrix;
  interval_matrix.shapeUninitialized(2,1);
  RealMatrix means_matrix;
  batch_means_percentile(Xmatrix, interval_matrix, means_matrix, 95, 0.95);

  Real gold_lower_int = 1.5047654078e+00;
  Real gold_upper_int = 1.7926345922e+00;
  EXPECT_LT(std::fabs(1. - interval_matrix[0][0] / gold_lower_int), 1.e-3/100. );
  EXPECT_LT(std::fabs(1. - interval_matrix[0][1] / gold_upper_int), 1.e-3/100. );
}

//------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
