/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <gtest/gtest.h>

#include "SurrogatesPython.hpp"
#include "util_common.hpp"

using namespace dakota;
using namespace dakota::util;
using namespace dakota::surrogates;

namespace {

/* Unit tests */

void PythonRegressionSurrogate_straight_line_fit() {
  VectorXd line_vector = VectorXd::LinSpaced(20, 0, 1);  // size, low, high
  VectorXd response = VectorXd::LinSpaced(20, 0, 1) * 1.23;
  response = (response.array() + 2.0).matrix();  // specify a y-intercept of 2.0

  Teuchos::ParameterList config_options("Python Test Parameters");
  config_options.set("verbose", 1);
  //  TODO: propagate params to python...

  // std::cout << "line_vector: " << line_vector << std::endl;
  // std::cout << "response: " << response << std::endl;
  const std::string module_and_class_name = "polynomial_surrogate.Surrogate";
  Python pr(line_vector, response, module_and_class_name);

  VectorXd eval_pts = VectorXd::LinSpaced(5, 0, 1);
  VectorXd expected_vals = 1.23 * eval_pts;
  expected_vals = (expected_vals.array() + 2.0).matrix();

  VectorXd approx_values = pr.value(eval_pts);
  // std::cout << "approx_values: " << approx_values << std::endl;
  // std::cout << "expected_vals: " << expected_vals << std::endl;

  EXPECT_TRUE(matrix_equals(approx_values, expected_vals, 1.0e-5));
}

}  // namespace

// --------------------------------------------------------------------------------

TEST(PythonSurrogateTest_tests, all_tests) {
  PythonRegressionSurrogate_straight_line_fit();

  SUCCEED();
}

// --------------------------------------------------------------------------------

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
