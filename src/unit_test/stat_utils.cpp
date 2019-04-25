/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "NonDBayesCalibration.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"
#include "bayes_calibration_utils.hpp"

#include <string>

#include <Teuchos_UnitTestHarness.hpp>

using namespace Dakota;

//------------------------------------

TEUCHOS_UNIT_TEST(stat_utils, kl_divergence)
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

  Real gold_kl = 0.024596;
  TEST_FLOATING_EQUALITY(kl_est, gold_kl, 1.e-5);
}

//------------------------------------

TEUCHOS_UNIT_TEST(stat_utils, mutual_info_ksg1)
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

  Real gold_mi = -0.0218954;
  TEST_FLOATING_EQUALITY(mutualinfo_est, gold_mi, 1.e-5);
}

//------------------------------------

TEUCHOS_UNIT_TEST(stat_utils, mutual_info_ksg2)
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

  Real gold_mi = -0.0561375;
  TEST_FLOATING_EQUALITY(mutualinfo_est, gold_mi, 1.e-5);
}

//------------------------------------

TEUCHOS_UNIT_TEST(stat_utils, batch_means_mean)
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
  batch_means_interval(Xmatrix, interval_matrix, 1, 0.95);

  Real gold_lower_int = -6.3120595090e-02;
  Real gold_upper_int = 8.1516649910e-02;
  TEST_FLOATING_EQUALITY(interval_matrix[0][0], gold_lower_int, 1.e-5);
  TEST_FLOATING_EQUALITY(interval_matrix[0][1], gold_upper_int, 1.e-5);
}

//------------------------------------

TEUCHOS_UNIT_TEST(stat_utils, batch_means_variance)
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
  batch_means_interval(Xmatrix, interval_matrix, 2, 0.95);

  Real gold_lower_int = 9.1432956019e-01;
  Real gold_upper_int = 1.0688302101e+00;
  TEST_FLOATING_EQUALITY(interval_matrix[0][0], gold_lower_int, 1.e-5);
  TEST_FLOATING_EQUALITY(interval_matrix[0][1], gold_upper_int, 1.e-5);
}

//------------------------------------
