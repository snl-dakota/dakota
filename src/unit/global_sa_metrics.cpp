/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_data_util.hpp"
#include "SurrogatesPolynomialRegression.hpp"
#include "util_common.hpp"
#include "util_metrics.hpp"
#include <string>

#define BOOST_TEST_MODULE dakota_global_sa_metrics
#include <boost/test/included/unit_test.hpp>

using namespace Dakota;

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_standard_reg_coeffs)
{
  const int NVARS = 4;
  const int NSAMP = 20;
  RealMatrix samples(NSAMP, NVARS);
  samples.random();

  MatrixXd fn_coeffs(5, 1);
  fn_coeffs << 0.0, 10.0, 1.0, 0.1, 0.01;

  RealMatrix responses(NSAMP, 1);
  for( int i=0; i<NSAMP; ++i ) {
    responses(i,0) = 0.0;
    for( int v=0; v<NVARS; ++v )
      responses(i,0) += fn_coeffs(v+1,0)*samples(i,v);
  }

  MatrixXd copy_samples;
  MatrixXd copy_responses;

  copy_data(samples,   copy_samples);
  copy_data(responses, copy_responses);

  //Cout << "Samples: \n" << copy_samples << std::endl;
  //Cout << "Responses: \n" << copy_responses << std::endl;

  Teuchos::ParameterList param_list("Polynomial Test Parameters");
  // This should produce regular RCs
  param_list.set("scaler type", "none");
  // Does this produce SRCs ?
  //param_list.set("scaler type", "standardization");

  /////////////////  What we want to test
  dakota::surrogates::PolynomialRegression pr(copy_samples, copy_responses, param_list);
  const MatrixXd & polynomial_coeffs = pr.get_polynomial_coeffs();
  //double polynomial_intercept = pr.get_polynomial_intercept();
  /////////////////  What we want to test

  //Cout << "SRCs: \n" << polynomial_coeffs/polynomial_coeffs.sum() << std::endl;
  BOOST_CHECK(dakota::util::matrix_equals(fn_coeffs, polynomial_coeffs, 1.0e-10));

  // Compute coefficient of determination, R^2
  VectorXd sur_vals = pr.value(copy_samples);
  double cod = dakota::util::compute_metric(sur_vals, copy_responses.col(0), "rsquared");
  // R^2 should be 1.0 because we fit the surrogate using exact polynomial objective values
  BOOST_CHECK_CLOSE(cod, 1.0, 1.e-13 /* NB this is a percent-based tol */);
}

//----------------------------------------------------------------
