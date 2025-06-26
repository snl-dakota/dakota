/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include <string>

#include "dakota_data_io.hpp"
#include "dakota_tabular_io.hpp"

using namespace Dakota;

//----------------------------------------------------------------

TEST(covariance_reader_tests, test_covariance_reader_read_constant_covariance) {
  const std::string base_name = "../expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 1 /* expt number */,
                  cov_values);  // implies CONSTANT

  // Verify equality of config data
  EXPECT_TRUE((cov_values.numRows() == 1));
  EXPECT_TRUE((cov_values.numCols() == 1));
  EXPECT_LT(std::fabs(1. - cov_values[0][0] / 3.67), 1.e-12 / 100.);
}

//----------------------------------------------------------------

TEST(covariance_reader_tests, test_covariance_reader_read_vector_covariance) {
  const std::string base_name = "../expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(base_name, 2,  // expt number
                  Dakota::CovarianceMatrix::VECTOR,
                  9,  // num values in covariance VECTOR
                  cov_values);

  double gold_values[] = {2.34,    8.552, -3.654, 7.332, 0.01,
                          -0.1509, -5.98, 4.74,   9.99};
  // Verify equality of config data
  EXPECT_TRUE((cov_values.numRows() == 1));
  EXPECT_TRUE((cov_values.numCols() == 9));
  for (int i = 0; i < 9; ++i)
    EXPECT_LT(std::fabs(1. - cov_values[i][0] / gold_values[i]), 1.e-12 / 100.);
}

//----------------------------------------------------------------

TEST(covariance_reader_tests, test_covariance_reader_read_matrix_covariance) {
  const std::string base_name = "../expt_data_test_files/voltage";

  RealMatrix cov_values;
  read_covariance(
      base_name, 3,  // expt number
      Dakota::CovarianceMatrix::MATRIX,
      9,  // num values in covariance MATRIX row (total entries = 9*9)
      cov_values);

  // Verify equality of config data
  EXPECT_TRUE((cov_values.numRows() == 9));
  EXPECT_TRUE((cov_values.numCols() == 9));
  for (int i = 0; i < 9; ++i)
    for (int j = 0; j < 9; ++j) {
      if ((i + j) == 0) {
        EXPECT_LT(std::fabs(cov_values[i][j]), 1.e-12 / 100.);
      } else {
        EXPECT_LT(std::fabs(1. - cov_values[i][j] / ((double)(i + j) / 2.0)),
                  1.e-12 / 100.);
      }
    }
}

//----------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
