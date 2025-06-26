/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_windows.hpp"
#include "util_common.hpp"
#include "util_data_types.hpp"
#include "util_math_tools.hpp"
#include "util_metrics.hpp"

#include <gtest/gtest.h>

using namespace dakota;
using namespace dakota::util;

namespace {

TEST(MetricsTest_tests, util_metrics_verification) {
  const int N = 10;
  VectorXd p = create_uniform_random_double_matrix(N, 1, 44);
  VectorXd d = create_uniform_random_double_matrix(N, 1, 15);
  MatrixXd diff = p - d;
  double metric, mval;
  const double atol = 1.0e-14;

  /* SUM_SQUARED */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::pow(diff(i), 2);

  metric = compute_metric(p, d, "sum_squared");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* MEAN_SQUARED */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::pow(diff(i), 2);
  mval /= N;

  metric = compute_metric(p, d, "mean_squared");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* RMS */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::pow(diff(i), 2);
  mval /= N;
  mval = std::sqrt(mval);

  metric = compute_metric(p, d, "root_mean_squared");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* SUM_ABS */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::abs(diff(i));

  metric = compute_metric(p, d, "sum_abs");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* MEAN_ABS */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::abs(diff(i));
  mval /= N;

  metric = compute_metric(p, d, "mean_abs");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* MAX_ABS */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval = std::max(std::abs(diff(i)), mval);

  metric = compute_metric(p, d, "max_abs");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* ABS_PERCENTAGE_ERROR */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::abs(diff(i) / d(i));

  metric = compute_metric(p, d, "ape");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* MEAN_ABS_PERCENTAGE_ERROR */
  mval = 0.0;
  for (int i = 0; i < N; i++) mval += std::abs(diff(i) / d(i));
  mval /= N;

  metric = compute_metric(p, d, "mape");

  EXPECT_TRUE((std::abs(mval - metric) < atol));

  /* R_SQUARED */
  double dbar = 0.0, numer = 0.0, denom = 0.0;
  mval = 0.0;
  for (int i = 0; i < N; i++) dbar += d(i);
  dbar /= N;
  for (int i = 0; i < N; i++) {
    numer += std::pow(p(i) - dbar, 2);
    denom += std::pow(d(i) - dbar, 2);
  }
  mval = numer / denom;

  metric = compute_metric(p, d, "rsquared");

  EXPECT_TRUE((std::abs(mval - metric) < atol));
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

}  // namespace
