/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "UtilDataScaler.hpp"
#include "util_common.hpp"
#include "util_data_types.hpp"

using namespace dakota;
using namespace dakota::util;

namespace {

MatrixXd create_single_feature_matrix() {
  MatrixXd single_feature_matrix(7, 1);
  single_feature_matrix << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7;
  return single_feature_matrix;
}

MatrixXd create_multiple_features_matrix() {
  MatrixXd multiple_features_matrix(7, 3);
  /*
  multiple_features_matrix << 0.1, 1, 10, 100, 1000, 10000, 100000,
                            0.2, 3, 20, 300, 3000, 30000, 500000,
                            0.5, 6, 50, 700, 8000, 40000, 700000;
  */

  multiple_features_matrix << 0.1, 0.2, 0.5, 1, 3, 6, 10, 20, 50, 100, 300, 700,
      1000, 3000, 8000, 10000, 30000, 40000, 100000, 500000, 700000;
  return multiple_features_matrix;
}

////////////////
// Unit tests //
////////////////

TEST(DataScalarTest_tests,
     util_NormalizationScaler_getScaledFeatures_TestMeanNormalizationTrue) {
  const double norm_factor = 1.0;
  MatrixXd unscaled_features = create_single_feature_matrix();
  NormalizationScaler ns(unscaled_features, true, norm_factor);

  MatrixXd matrix_actual(7, 1);
  MatrixXd matrix_expected(7, 1);

  ns.scale_samples(unscaled_features, matrix_actual);
  matrix_expected << -0.5, -0.333333, -0.166667, 0.0, 0.166667, 0.333333, 0.5;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
}

TEST(DataScalarTest_tests,
     util_NormalizationScaler_getScaledFeatures_TestMeanNormalizationFalse) {
  const double norm_factor = 1.0;
  MatrixXd unscaled_features = create_single_feature_matrix();
  NormalizationScaler ns(unscaled_features, false, norm_factor);

  MatrixXd matrix_actual(7, 1);
  MatrixXd matrix_expected(7, 1);

  ns.scale_samples(unscaled_features, matrix_actual);
  matrix_expected << 0, 0.166667, 0.333333, 0.5, 0.666667, 0.833333, 1;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
}

TEST(
    DataScalarTest_tests,
    util_NormalizationScaler_getScaledFeatures_TestMeanNormalizationFalseWithMultipleSamples) {
  const double norm_factor = 1.0;
  MatrixXd unscaled_features = create_multiple_features_matrix();
  NormalizationScaler ns(unscaled_features, false, norm_factor);

  MatrixXd matrix_actual(7, 3);
  MatrixXd matrix_expected(7, 3);

  ns.scale_samples(unscaled_features, matrix_actual);

  matrix_expected << 0, 0, 0, 9.00001e-06, 5.6e-06, 7.85715e-06, 9.90001e-05,
      3.96e-05, 7.07143e-05, 0.000999001, 0.0005996, 0.000999286, 0.00999901,
      0.0059996, 0.0114279, 0.0999991, 0.0599996, 0.0571422, 1, 1, 1;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
}

TEST(DataScalarTest_tests,
     util_NormalizationScaler_getScaledFeatures_TestNormFactor) {
  const double norm_factor = 2.0;
  MatrixXd unscaled_features = create_single_feature_matrix();
  NormalizationScaler ns(unscaled_features, true, norm_factor);

  MatrixXd matrix_actual(7, 1);
  MatrixXd matrix_expected(7, 1);

  ns.scale_samples(unscaled_features, matrix_actual);
  matrix_expected << -1, -0.666667, -0.333333, 1.85037e-16, 0.333333, 0.666667,
      1;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
}

TEST(DataScalarTest_tests,
     util_StandardizationScaler_getScaledFeatures_TestDefault) {
  MatrixXd unscaled_features = create_single_feature_matrix();
  StandardizationScaler ss(unscaled_features);

  MatrixXd matrix_actual(7, 1);
  MatrixXd matrix_expected(7, 1);

  ss.scale_samples(unscaled_features, matrix_actual);
  matrix_expected << -1.5, -1, -0.5, 2.77556e-16, 0.5, 1, 1.5;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
  // For StandardizationScaler, mean should be effectively zero.
  EXPECT_TRUE((std::abs(matrix_actual.col(0).mean()) < 1.0e-14));
  // For StandardizationScaler, variance should be effectively one.
  const double UNIT_VARIANCE = 1.0;
  EXPECT_TRUE(
      (std::abs(variance(matrix_actual.col(0)) - UNIT_VARIANCE) < 1.0e-14));
}

TEST(DataScalarTest_tests,
     util_StandardizationScaler_getScaledFeatures_TestMultipleSamples) {
  MatrixXd unscaled_features = create_multiple_features_matrix();
  StandardizationScaler ss(unscaled_features);

  MatrixXd matrix_actual(7, 3);
  MatrixXd matrix_expected(7, 3);

  ss.scale_samples(unscaled_features, matrix_actual);

  matrix_expected << -0.45993, -0.439588, -0.441129, -0.459904, -0.439572,
      -0.441107, -0.459643, -0.439474, -0.440925, -0.457035, -0.437858,
      -0.438244, -0.430957, -0.42228, -0.408139, -0.170175, -0.266498,
      -0.276169, 2.43765, 2.44527, 2.44571;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
  // For StandardizationScaler, mean should be effectively zero.
  EXPECT_TRUE((std::abs(matrix_actual.col(0).mean()) < 1.0e-14));
  EXPECT_TRUE((std::abs(matrix_actual.col(1).mean()) < 1.0e-14));
  EXPECT_TRUE((std::abs(matrix_actual.col(2).mean()) < 1.0e-14));
  // For StandardizationScaler, variance should be effectively one.
  const double UNIT_VARIANCE = 1.0;
  EXPECT_TRUE(
      (std::abs(variance(matrix_actual.col(0)) - UNIT_VARIANCE) < 1.0e-14));
  EXPECT_TRUE(
      (std::abs(variance(matrix_actual.col(1)) - UNIT_VARIANCE) < 1.0e-14));
  EXPECT_TRUE(
      (std::abs(variance(matrix_actual.col(2)) - UNIT_VARIANCE) < 1.0e-14));
}

TEST(DataScalarTest_tests, util_StandardizationScaler_scaleSamples) {
  StandardizationScaler ss(create_multiple_features_matrix());

  MatrixXd matrix_actual_unscaled = create_multiple_features_matrix();
  MatrixXd matrix_actual_scaled(7, 3);
  MatrixXd matrix_expected(7, 3);

  ss.scale_samples(matrix_actual_unscaled, matrix_actual_scaled);

  matrix_expected << -0.45993, -0.439588, -0.441129, -0.459904, -0.439572,
      -0.441107, -0.459643, -0.439474, -0.440925, -0.457035, -0.437858,
      -0.438244, -0.430957, -0.42228, -0.408139, -0.170175, -0.266498,
      -0.276169, 2.43765, 2.44527, 2.44571;

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual_scaled << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual_scaled, matrix_expected, 1.0e-4));
  // For StandardizationScaler, mean should be effectively zero.
  EXPECT_TRUE((std::abs(matrix_actual_scaled.col(0).mean()) < 1.0e-14));
  EXPECT_TRUE((std::abs(matrix_actual_scaled.col(1).mean()) < 1.0e-14));
  EXPECT_TRUE((std::abs(matrix_actual_scaled.col(2).mean()) < 1.0e-14));
  // For StandardizationScaler, variance should be effectively one.
  const double UNIT_VARIANCE = 1.0;
  EXPECT_TRUE((std::abs(variance(matrix_actual_scaled.col(0)) - UNIT_VARIANCE) <
               1.0e-14));
  EXPECT_TRUE((std::abs(variance(matrix_actual_scaled.col(1)) - UNIT_VARIANCE) <
               1.0e-14));
  EXPECT_TRUE((std::abs(variance(matrix_actual_scaled.col(2)) - UNIT_VARIANCE) <
               1.0e-14));
}

TEST(DataScalarTest_tests, util_StandardizationScaler_scaleSamples_wrongSize) {
  MatrixXd unscaled_features = create_multiple_features_matrix();
  StandardizationScaler ss(unscaled_features);

  MatrixXd wrong_size_matrix(3, 7);
  MatrixXd scaled_features;

  wrong_size_matrix << 0.1, 0.2, 0.5, 1, 3, 6, 10, 20, 50, 100, 300, 700, 1000,
      3000, 8000, 10000, 30000, 40000, 100000, 500000, 700000;

  EXPECT_THROW(ss.scale_samples(wrong_size_matrix, scaled_features),
               std::runtime_error);
}

TEST(DataScalarTest_tests, util_NoScaler_scale_samples_TestDefault) {
  MatrixXd unscaled_features = create_single_feature_matrix();
  NoScaler ns(unscaled_features);

  MatrixXd matrix_actual(7, 1);
  MatrixXd matrix_expected(7, 1);

  ns.scale_samples(unscaled_features, matrix_actual);
  matrix_expected = create_single_feature_matrix();

  // std::cout << matrix_expected << std::endl;
  // std::cout << matrix_actual << std::endl;

  EXPECT_TRUE(matrix_equals(matrix_actual, matrix_expected, 1.0e-4));
}

TEST(DataScalarTest_tests, util_StandardizationScaler_serialize) {
  // BMA TODO: This doesn't test serialization detection of the
  // derived type and loading into pointer to DataScaler base

  StandardizationScaler ss(create_multiple_features_matrix());
  std::ostringstream scaler_osstream;
  boost::archive::text_oarchive output_archive(scaler_osstream);
  output_archive << ss;

  StandardizationScaler loaded_ss;
  std::istringstream scaler_istream(scaler_osstream.str());
  boost::archive::text_iarchive input_archive(scaler_istream);
  input_archive >> loaded_ss;

  // These should be exact, so using a tight tol
  EXPECT_TRUE(matrix_equals(ss.get_scaler_features_offsets(),
                            loaded_ss.get_scaler_features_offsets(), 1.0e-12));
  EXPECT_TRUE(matrix_equals(ss.get_scaler_features_scale_factors(),
                            loaded_ss.get_scaler_features_scale_factors(),
                            1.0e-12));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

}  // namespace
