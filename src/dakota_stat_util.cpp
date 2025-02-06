/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Statistics-related utilities

#include <chrono>

#include "dakota_stat_util.hpp"
#ifdef HAVE_DAKOTA_SURROGATES
#include "SurrogatesPolynomialRegression.hpp"
#endif
#include "util_metrics.hpp"

using MatrixMap = Eigen::Map<Eigen::MatrixXd>;
using VectorMap = Eigen::Map<Eigen::VectorXd>;

namespace Dakota {

//----------------------------------------------------------------

/** Mimics DDACE timeSeed(), which returns the trailing microseconds
    on the time of day clock. Historically, most algorithms opted for
    DDACE, Utilib, std::clock(), in that order. */
int generate_system_seed()
{
  // Could consider using std::random_device by default, with fallback on clock.
  auto mu_sec = std::chrono::duration_cast<std::chrono::microseconds>
    (std::chrono::high_resolution_clock::now().time_since_epoch()) % 1000000;
  int seed = 1 + mu_sec.count();

  return seed;
}

#ifdef HAVE_DAKOTA_SURROGATES
//----------------------------------------------------------------

void compute_regression_coeffs( const RealMatrix & samples, const RealMatrix & resps,
                                      RealMatrix & rcoeffs, RealVector & cods )
{
  if (samples.numRows() == 0)
    return;

  // Copy Teuchos data to Eigen data to work with surrogates
  MatrixXd copy_samples;
  MatrixXd copy_responses;

  copy_data(samples, copy_samples);
  copy_data(resps,   copy_responses);

  int num_vars = samples.numCols();
  int num_resp = resps.numCols();

  Teuchos::ParameterList param_list("Polynomial Test Parameters");
  // Use the defaults for now, cf
  //     PolynomialRegression::default_options()
  // ... which provides regular RCs (from linear model)

  cods.size(num_resp);
  rcoeffs.shape(num_vars, num_resp);

  for (int i=0; i<num_resp; ++i)
  {
    dakota::surrogates::PolynomialRegression pr(copy_samples, copy_responses.col(i), param_list);
    const MatrixXd & coeffs = pr.get_polynomial_coeffs();

    // A workaround to deal with the inability of RealVector to use const double* data
    VectorXd nonconst_coeffs = coeffs.col(0).tail(num_vars); // slice off the constant coeff values
    std::copy(nonconst_coeffs.data(), nonconst_coeffs.data()+num_vars, rcoeffs[i]);

    // Compute coefficient of determination, R^2
    VectorXd sur_vals = pr.value(copy_samples);
    cods[i] = dakota::util::compute_metric(sur_vals, copy_responses.col(i), "rsquared");
  }

  return;
}

//----------------------------------------------------------------

void compute_std_regression_coeffs( const RealMatrix & samples, const RealMatrix & resps,
                                          RealMatrix & std_reg_coeffs, RealVector & cods )
{
  if (samples.numRows() == 0)
    return;

  // Compute the non-standardized RCs and R^2
  // ... we return the R^2 based on the non-standardized linear regression model
  RealMatrix reg_coeffs;
  compute_regression_coeffs(samples, resps, reg_coeffs, cods);

  // Then scale each RC by (stdev_i/stdev_r)
  // where:
  //    stdev_i is std dev of variable i
  //    stdev_r is std dev of response

  // We could avoid copying into MatrixXd and instead use RealMatrix
  // objects if we implement a stddev utility for the RealMatrix columns.
  MatrixXd copy_samples;
  MatrixXd copy_responses;
  copy_data(samples, copy_samples);
  copy_data(resps,   copy_responses);

  int num_vars = samples.numCols();
  int num_resp = resps.numCols();

  // Compute sample stddevs
  VectorXd means(num_vars);
  VectorXd stddevs(num_vars);
  double stddev;
  for (int j = 0; j < num_vars; j++) {
    means(j) = copy_samples.col(j).mean();
    VectorXd tmp = copy_samples.col(j).array() - means(j);
    stddev = tmp.dot(tmp)/(copy_samples.rows()-1);
    stddevs(j) = std::sqrt(stddev);
  }

  std_reg_coeffs = reg_coeffs;

  VectorXd tmp;
  double resp_stddev;
  for (int j=0; j<copy_responses.cols(); ++j)
  {
    // Compute response stddev
    tmp = copy_responses.col(j).array() - copy_responses.col(j).mean();
    resp_stddev = std::sqrt(tmp.dot(tmp)/(copy_responses.rows()-1));

    // Do we need to do anything special here or just return inf/nan's ? - RWH
    //// Handle the case of resp_stddev = 0 (based on Dakota's definition of small)
    //if (resp_stddev < Pecos::SMALL_NUMBER) {
    //  std_reg_coeffs.putScalar(0.0);
    //  cods.putScalar(0.0);
    //  return;
    //}

    // Scale RCs by ratio of std devs, assumes linear regression model
    VectorMap polynomial_coeffs(reg_coeffs.values()+j*num_vars, num_vars);
    VectorXd std_polynomial_coeffs = (polynomial_coeffs.array()*stddevs.array())/resp_stddev;

    std::copy(std_polynomial_coeffs.data(), std_polynomial_coeffs.data()+num_vars, std_reg_coeffs[j]);
  }

  return;
}
#endif

} // namespace Dakota
