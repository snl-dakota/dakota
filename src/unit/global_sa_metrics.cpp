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

using VectorMap = Eigen::Map<Eigen::VectorXd>;
using MatrixMap = Eigen::Map<Eigen::MatrixXd>;

using namespace Dakota;

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_standard_reg_coeffs)
{
  const int NVARS = 4;
  const int NSAMP = 20;
  RealMatrix samples(NSAMP, NVARS);
  samples.random();

  MatrixXd fn_coeffs(NVARS, 1);
  fn_coeffs << 10.0, 1.0, 0.1, 0.01;

  RealMatrix responses(NSAMP, 1);
  for( int i=0; i<NSAMP; ++i ) {
    responses(i,0) = 0.0;
    for( int v=0; v<NVARS; ++v )
      responses(i,0) += fn_coeffs(v,0)*samples(i,v);
  }

  /////////////////////  What we want to test --> Reg. Coeffs. (not standardized)
  RealVector rcoeffs;
  Real r2 = compute_regression_coeffs(samples, responses, rcoeffs);
  MatrixMap test_rcs(rcoeffs.values(), NVARS, 1);
  BOOST_CHECK(dakota::util::matrix_equals(fn_coeffs, test_rcs, 1.0e-10));
  // Coefficient of determination, R^2
  // ... should be 1.0 because we fit the surrogate using exact polynomial objective values
  BOOST_CHECK_CLOSE(r2, 1.0, 1.e-13 /* NB this is a percent-based tol */);
  //double polynomial_intercept = pr.get_polynomial_intercept(); // not used or needed? - RWH
  ///////////////////////////  What we want to test ///////////////////////////

  MatrixXd gold_srcs(NVARS, 1);
  /*
   * The gold Standard Regression Coefficients can be obtained by dumping
   * samples and responses to files "samples.txt" and "responses.txt",
   * respectively, and then running the following script:

     #!/usr/bin/env python3
     import numpy as np
     import statsmodels.api as sm
     from scipy.stats.mstats import zscore
     samp = np.genfromtxt("samples.txt", skip_header=0, unpack=False)
     resp = np.genfromtxt("responses.txt", skip_header=0, unpack=False)
     print(sm.GLS(zscore(resp), zscore(samp)).fit().summary())
     
   * Note that the stddevs used within the zscore values correspond to:

     print(samp.std(axis=0, ddof=1))
     print(resp.std(axis=0, ddof=1))

   */

  // Will we always get the same random values for the samples matrix above? - RWH
  gold_srcs << 0.996027, 0.122718, 0.0131245, 0.0010858;

  /////////////////////  What we want to test --> Reg. Coeffs. (not standardized)
  RealVector std_rcoeffs;
  r2 = compute_std_regression_coeffs(samples, responses, std_rcoeffs);
  MatrixMap test_srcs(std_rcoeffs.values(), NVARS, 1);
  BOOST_CHECK(dakota::util::matrix_equals(gold_srcs, test_srcs, 1.0e-6));
  // Coefficient of determination, R^2 is the same value as the one above
  BOOST_CHECK_CLOSE(r2, 1.0, 1.e-13 /* NB this is a percent-based tol */);
  ///////////////////////////  What we want to test ///////////////////////////
}

//----------------------------------------------------------------
