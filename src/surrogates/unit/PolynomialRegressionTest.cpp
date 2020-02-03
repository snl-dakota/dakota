/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include <Teuchos_UnitTestHarness.hpp>
#include "PolynomialRegression.hpp"

using namespace dakota::surrogates;

namespace{

////////////////
// Unit tests //
////////////////

TEUCHOS_UNIT_TEST(PolynomialRegressionSurrogate, build)
{
  PolynomialRegression pr;
  pr.set_polynomial_order(2);
  pr.build_surrogate();
}

} // namespace