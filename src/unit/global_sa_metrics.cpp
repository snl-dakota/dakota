/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "dakota_stat_util.hpp"
#include "dakota_data_types.hpp"
#include "SurrogatesPolynomialRegression.hpp"
#include "SensAnalysisGlobal.hpp"
#include "util_common.hpp"
#include "util_metrics.hpp"
#include <string>
#include "Eigen/Dense"

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
#ifdef HAVE_DAKOTA_SURROGATES

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

  Real bad = std::numeric_limits<Real>::infinity();
  MatrixXd gold_srcs(NVARS, 3);
  gold_srcs << bad, 0.996027,  0.996027,  
               bad, 0.122718,  0.122718,  
               bad, 0.0131245, 0.0131245, 
               bad, 0.0010858, 0.0010858;

  /////////////////////  What we want to test - multiple responses
  RealMatrix std_rcoeffs;
  RealVector cods;
  compute_std_regression_coeffs(samples, responses, std_rcoeffs, cods);
  MatrixMap test_srcs(std_rcoeffs.values(), NVARS, 3);
  test_srcs = test_srcs.array().abs();
  BOOST_CHECK(dakota::util::matrix_equals(gold_srcs, test_srcs, 1.0e-5));
  BOOST_CHECK(std::numeric_limits<Real>::infinity() == cods(0));
  BOOST_CHECK_CLOSE(cods(1), 1.0, 1.e-13 /* NB this is a percent-based tol */);
  BOOST_CHECK_CLOSE(cods(2), 1.0, 1.e-13 /* NB this is a percent-based tol */);
  ///////////////////////////  What we want to test ///////////////////////////
}

#endif 

//----------------------------------------------------------------

class SensAnalysisGlobalTest : public SensAnalysisGlobal {

  public:
    void compute_binned_sobol_indices_from_valid_samples( RealMatrix valid_samples, size_t n_bins ){ 
      return SensAnalysisGlobal::compute_binned_sobol_indices_from_valid_samples(valid_samples, n_bins);
      }
    void set_num_vars( size_t num_vars ){ numVars = num_vars; }
    void set_num_fns( size_t num_fns ){ numFns = num_fns; }
    RealVectorArray get_indexSi( ){return indexSi; }
};

class SobolG{
  /*
  Implements the Sobol G function, defined and discussed in detail in [1].

  [1] Saltelli, Andrea, Paola Annoni, Ivano Azzini, Francesca Campolongo, 
  Marco Ratto, and Stefano Tarantola. “Variance Based Sensitivity Analysis 
  of Model Output. Design and Estimator for the Total Sensitivity Index.” 
  Computer Physics Communications 181, no. 2 (February 1, 2010): 259–70. 
  https://doi.org/10.1016/j.cpc.2009.09.018.

  This class implements evaluation of the function, as well as computation of 
  its analytical main effects indices.
  */
  public: 
    SobolG():
    a(10)
    {
      a << 0,0.1,0.2,0.3,0.4,0.8,1,2,3,4;
      compute_analytical_main_effects();
    }

    SobolG( const Eigen::ArrayXd & a_in ):
    a(a_in.size()){
      a = a_in;
      compute_analytical_main_effects();
    }

    Eigen::ArrayXd get_analytical_main_effects(){ return analyticalMainEffects; }

    Eigen::ArrayXXd generate_input_samples( int n_samples ){
      // Eigen's Random method generates samples from U[-1,1], so we add 1 and scale by 0.5
      // to generate samples from U[0,1].
      return 0.5 * ( Eigen::ArrayXXd::Random( a.size(), n_samples ) + 1 );
    }

    Eigen::ArrayXd evaluate( const Eigen::ArrayXXd& input_samples ){
      /*
      Input: [N_inputs x N_samples ] input_samples 
      Implements the Sobol' G function, defined as 
        prod_i=1^k g_i,   g_i = (|4X_i - 2| + a_i ) / ( 1 + a_i )
      */

      // colwise() means the following operation is performed over a column; 
      // that means we'll be taking the product over the elements of each column.
      Eigen::ArrayXXd gi{ (4*input_samples-2).abs() };

      gi.colwise() += a;
      gi.colwise() /= 1+a;

      return gi.colwise().prod();      
    } 

  private:
    void compute_analytical_main_effects(){
      /*
      The partial variance is computed as 
      V_i = 1/3 / (1+a_i)^2
      */

      // Computing and storing partial variances in the main effects
      // array for now; these aren't normalized yet.
      analyticalMainEffects = 1/(1+a).pow(2.0)/3;

      float total_variance = (1+analyticalMainEffects).prod()-1;

      analyticalMainEffects /= total_variance;
    }
    Eigen::ArrayXd a;
    Eigen::ArrayXd analyticalMainEffects;
};

BOOST_AUTO_TEST_CASE(test_binned_sobol_computation)
{


  // Generating input and output samples from our test function.
  Eigen::ArrayXd a(Eigen::ArrayXd::Zero(2));
  SobolG gfunc(a);

  // Setting the seed so we get same samples every time
  std::srand((unsigned int) 20230530); 
  size_t num_samples = 1000000;
  size_t num_bins = std::sqrt(num_samples);
  Eigen::ArrayXXd x = gfunc.generate_input_samples(num_samples);
  Eigen::ArrayXd g = gfunc.evaluate(x);

  SensAnalysisGlobalTest gsa;
  size_t num_vars = x.rows(), num_fns = 1;
  gsa.set_num_vars( num_vars );
  gsa.set_num_fns( num_fns );

  // Converting to a format Dakota can ingest
  MatrixXd samplesXd{ MatrixXd::Zero( num_vars + num_fns, num_samples )};  
  samplesXd.topRows(num_vars) = x;
  samplesXd.row(num_vars) = g;
  RealMatrix samples;
  copy_data(samplesXd, samples);

  gsa.compute_binned_sobol_indices_from_valid_samples( samples, num_bins );  
  RealVectorArray test_sobols_va = gsa.get_indexSi();

  RealMatrix test_sobols{int(num_vars), int(num_fns)};
  for (int i=0; i < num_fns; i++){
    Teuchos::setCol( test_sobols_va[i], i, test_sobols );
  }

  RealMatrix true_sobols;
  copy_data(gfunc.get_analytical_main_effects(), true_sobols);

  // Compute the L2 error between the test and analytical Sobol' indices.
  true_sobols -= test_sobols;
  auto frob_err = true_sobols.normFrobenius();

  BOOST_CHECK(frob_err < 1e-3);
}