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

namespace {

  MatrixXd create_samples()
  {
    const int NVARS = 4;
    const int NSAMP = 20;
    MatrixXd samplesXd(NSAMP, NVARS);
    samplesXd << 0.680375,    -0.967399,  0.05349,     -0.52344,
                 -0.211234,   -0.514226,  0.539828,    0.941268,
                 0.566198,    -0.725537,  -0.199543,   0.804416,
                 0.59688,     0.608354,   0.783059,    0.70184,
                 0.823295,    -0.686642,  -0.433371,   -0.466669,
                 -0.604897,   -0.198111,  -0.295083,   0.0795207,
                 -0.329554,   -0.740419,  0.615449,    -0.249586,
                 0.536459,    -0.782382,  0.838053,    0.520497,
                 -0.444451,   0.997849,   -0.860489,   0.0250707,
                 0.10794,     -0.563486,  0.898654,    0.335448,
                 -0.0452059,  0.0258648,  0.0519907,   0.0632129,
                 0.257742,    0.678224,   -0.827888,   -0.921439,
                 -0.270431,   0.22528,    -0.615572,   -0.124725,
                 0.0268018,   -0.407937,  0.326454,    0.86367,
                 0.904459,    0.275105,   0.780465,    0.86162,
                 0.83239,     0.0485744,  -0.302214,   0.441905,
                 0.271423,    -0.012834,  -0.871657,   -0.431413,
                 0.434594,    0.94555,    -0.959954,   0.477069,
                 -0.716795,   -0.414966,  -0.0845965,  0.279958,
                 0.213938,    0.542715,   -0.873808,   -0.291903;

    return samplesXd;
  }

}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_standard_reg_coeffs)
{
  MatrixXd samplesXd = create_samples();
  const int NVARS = samplesXd.cols();
  const int NSAMP = samplesXd.rows();

  RealMatrix samples;
  copy_data(samplesXd, samples);

  MatrixXd fn_coeffs(NVARS, 1);
  fn_coeffs << 10.0, 1.0, 0.1, 0.01;

  RealMatrix responses(NSAMP, 1);
  for( int i=0; i<NSAMP; ++i ) {
    responses(i,0) = 0.0;
    for( int v=0; v<NVARS; ++v )
      responses(i,0) += fn_coeffs(v,0)*samples(i,v);
  }

  /////////////////////  What we want to test --> Reg. Coeffs. (not standardized)
  RealMatrix rcoeffs;
  RealVector cods;
  compute_regression_coeffs(samples, responses, rcoeffs, cods);
  MatrixMap test_rcs(rcoeffs.values(), NVARS, 1);
  BOOST_CHECK(dakota::util::matrix_equals(fn_coeffs, test_rcs, 1.0e-10));
  // Coefficient of determination, R^2
  // ... should be 1.0 because we fit the surrogate using exact polynomial objective values
  BOOST_CHECK_CLOSE(cods(0), 1.0, 1.e-13 /* NB this is a percent-based tol */);
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
     print(sm.OLS(zscore(resp), zscore(samp)).fit().summary())
     
   * Note that the stddevs used within the zscore values correspond to:

     print(samp.std(axis=0, ddof=1))
     print(resp.std(axis=0, ddof=1))

   */
  gold_srcs << 0.996027, 0.122718, 0.0131245, 0.0010858;

  /////////////////////  What we want to test --> Reg. Coeffs. (not standardized)
  RealMatrix std_rcoeffs;
  compute_std_regression_coeffs(samples, responses, std_rcoeffs, cods);
  MatrixMap test_srcs(std_rcoeffs.values(), NVARS, 1);
  BOOST_CHECK(dakota::util::matrix_equals(gold_srcs, test_srcs, 1.0e-6));
  // Coefficient of determination, R^2 is the same value as the one above
  BOOST_CHECK_CLOSE(cods(0), 1.0, 1.e-13 /* NB this is a percent-based tol */);
  ///////////////////////////  What we want to test ///////////////////////////
}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_reg_coeffs_edge_cases)
{
  // Test for no samples
  RealMatrix samples;
  RealMatrix responses;

  /////////////////////  What we want to test --> No Responses
  RealMatrix rcoeffs;
  RealVector cods;
  compute_regression_coeffs(samples, responses, rcoeffs, cods);
  BOOST_CHECK(0 == cods.length());
  compute_std_regression_coeffs(samples, responses, rcoeffs, cods);
  BOOST_CHECK(0 == cods.length());
  ///////////////////////////  What we want to test ///////////////////////////


  // Test for 0 stddev
  MatrixXd samplesXd = create_samples();
  const int NVARS = samplesXd.cols();
  const int NSAMP = samplesXd.rows();

  copy_data(samplesXd, samples);

  responses.reshape(NSAMP, 1);
  for( int i=0; i<NSAMP; ++i ) {
    responses(i,0) = 1.0;
  }

  /////////////////////  What we want to test --> Response stddev = 0
  compute_regression_coeffs(samples, responses, rcoeffs, cods);
  BOOST_CHECK(std::numeric_limits<Real>::infinity() == cods(0));
  compute_std_regression_coeffs(samples, responses, rcoeffs, cods);
  BOOST_CHECK(std::numeric_limits<Real>::infinity() == cods(0));
  ///////////////////////////  What we want to test ///////////////////////////

}

//----------------------------------------------------------------

BOOST_AUTO_TEST_CASE(test_standard_reg_coeffs_multi_resp)
{
  MatrixXd samplesXd = create_samples();
  const int NVARS = samplesXd.cols();
  const int NSAMP = samplesXd.rows();

  RealMatrix samples;
  copy_data(samplesXd, samples);

  MatrixXd fn_coeffs(NVARS, 1);
  fn_coeffs << 10.0, 1.0, 0.1, 0.01;

  RealMatrix responses(NSAMP, 3);
  for( int i=0; i<NSAMP; ++i ) {
    responses(i,0) = 2.5;
    responses(i,1) = 0.0;
    responses(i,2) = 0.0;
    for( int v=0; v<NVARS; ++v ) {
      responses(i,1) += 2.5*fn_coeffs(v  ,0)*samples(i,v);
      responses(i,2) +=     fn_coeffs(v  ,0)*samples(i,v);
    }
  }

  Real bad = 1.0/0.0;
  MatrixXd gold_srcs(NVARS, 3);
  gold_srcs <<  bad, 0.996027,  0.996027,  
                bad, 0.122718,  0.122718,  
               -bad, 0.0131245, 0.0131245, 
                bad, 0.0010858, 0.0010858;

  /////////////////////  What we want to test - multiple responses
  RealMatrix std_rcoeffs;
  RealVector cods;
  compute_std_regression_coeffs(samples, responses, std_rcoeffs, cods);
  MatrixMap test_srcs(std_rcoeffs.values(), NVARS, 3);
  BOOST_CHECK(dakota::util::matrix_equals(gold_srcs, test_srcs, 1.0e-6));
  BOOST_CHECK(std::numeric_limits<Real>::infinity() == cods(0));
  BOOST_CHECK_CLOSE(cods(1), 1.0, 1.e-13 /* NB this is a percent-based tol */);
  BOOST_CHECK_CLOSE(cods(2), 1.0, 1.e-13 /* NB this is a percent-based tol */);
  ///////////////////////////  What we want to test ///////////////////////////
}

//----------------------------------------------------------------
