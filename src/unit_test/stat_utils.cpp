
#include "NonDBayesCalibration.hpp"
#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

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

TEUCHOS_UNIT_TEST(stat_utils, mutual_info)
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
  Real mutualinfo_est = NonDBayesCalibration::knn_mutual_info(Xmatrix, 1, 1); 

  Real gold_mi = -0.0438223;
  TEST_FLOATING_EQUALITY(mutualinfo_est, gold_mi, 1.e-5);
}

//------------------------------------

