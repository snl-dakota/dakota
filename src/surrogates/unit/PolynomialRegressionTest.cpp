/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


//////////////
// Includes //
//////////////

#include <Teuchos_UnitTestHarness.hpp>
#include "../../util/unit/CommonTestUtils.hpp"
#include "PolynomialRegression.hpp"

///////////////
// Namespace //
///////////////

using namespace dakota::surrogates;
using namespace Eigen;

namespace {

///////////////
// Utilities //
///////////////

MatrixXd create_single_feature_matrix()
{
  MatrixXd single_feature_matrix(7, 1);
  single_feature_matrix << 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7;
  return single_feature_matrix;
}

MatrixXd create_multiple_features_matrix()
{
  MatrixXd multiple_features_matrix(7, 3);
  /*
  multiple_features_matrix << 0.1, 1, 10, 100, 1000, 10000, 100000,
                            0.2, 3, 20, 300, 3000, 30000, 500000,
                            0.5, 6, 50, 700, 8000, 40000, 700000;
  */

  multiple_features_matrix << 0.1, 0.2, 0.5,
                              1, 3, 6,
                              10, 20, 50,
                              100, 300, 700,
                              1000, 3000, 8000,
                              10000, 30000, 40000,
                              100000, 500000, 700000;
  return multiple_features_matrix;
}

////////////////
// Unit tests //
////////////////

TEUCHOS_UNIT_TEST(PolynomialRegressionSurrogate, getters_and_setters)
{
  PolynomialRegression pr;
  MatrixXd samples  = create_multiple_features_matrix();
  MatrixXd response = create_single_feature_matrix();

  pr.set_samples(samples);
  pr.set_response(response);
  pr.set_polynomial_order(2);

  TEST_ASSERT(matrix_equals(pr.get_samples(),  samples,  1.0e-4));
  TEST_ASSERT(matrix_equals(pr.get_response(), response, 1.0e-4));
  TEST_ASSERT(pr.get_polynomial_order() == 2);
}

} // namespace