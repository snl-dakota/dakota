/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "SurrogatesPython.hpp"
#include "util_common.hpp"
#include <boost/test/minimal.hpp>

using namespace dakota;
using namespace dakota::util;
using namespace dakota::surrogates;

namespace {

/* Unit tests */

void PythonRegressionSurrogate_straight_line_fit(std::string scaler_type) {
  VectorXd line_vector = VectorXd::LinSpaced(20, 0, 1);  // size, low, high
  VectorXd response = VectorXd::LinSpaced(20, 0, 1)*1.23;
  response = (response.array() + 2.0).matrix();  // specify a y-intercept of 2.0

  Teuchos::ParameterList config_options("Python Test Parameters");
  //  TODO: propagate params to python...
  config_options.set("verbose", 1);
  std::cout << "line_vector: " << line_vector << std::endl;
  std::cout << "response: " << response << std::endl;
  Python pr(line_vector, response, config_options);

  VectorXd eval_pts = VectorXd::LinSpaced(5, 0, 1);
  VectorXd expected_vals = 1.23*eval_pts;
  expected_vals = (expected_vals.array() + 2.0).matrix();

  VectorXd approx_values = pr.value(eval_pts);
  std::cout << "approx_values: " << approx_values << std::endl;
  std::cout << "expected_vals: " << expected_vals << std::endl;

  BOOST_CHECK(
      matrix_equals(approx_values, expected_vals, 1.0e-5));
}

}  // namespace

// --------------------------------------------------------------------------------

int test_main(int argc, char* argv[])  // note the name!
{
  PythonRegressionSurrogate_straight_line_fit("none");

  BOOST_CHECK(boost::exit_success == 0);

  return boost::exit_success;
}

// --------------------------------------------------------------------------------
