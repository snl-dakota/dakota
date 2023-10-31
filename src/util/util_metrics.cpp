/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "util_metrics.hpp"

#include "util_common.hpp"

#include <boost/assign.hpp>
#include <boost/bimap.hpp>

namespace dakota {
namespace util {

/// alias for Boost Bimap metric type <--> string
using BimapMetrictypeStr = boost::bimap<METRIC_TYPE, std::string>;
/// Bimap between metric types and names
static BimapMetrictypeStr type_name_bimap =
    boost::assign::list_of<BimapMetrictypeStr::relation>
    // clang-format off
    (METRIC_TYPE::SUM_SQUARED, "sum_squared")
    (METRIC_TYPE::MEAN_SQUARED, "mean_squared")
    (METRIC_TYPE::ROOT_MEAN_SQUARED, "root_mean_squared")
    (METRIC_TYPE::SUM_ABS, "sum_abs")
    (METRIC_TYPE::MEAN_ABS, "mean_abs")
    (METRIC_TYPE::MAX_ABS, "max_abs")
    (METRIC_TYPE::ABS_PERCENTAGE_ERROR, "ape")
    (METRIC_TYPE::MEAN_ABS_PERCENTAGE_ERROR, "mape")
    (METRIC_TYPE::R_SQUARED, "rsquared");
// clang-format on

METRIC_TYPE metric_type(const std::string& metric_name) {
  BimapMetrictypeStr::right_const_iterator rc_iter =
      type_name_bimap.right.find(metric_name);
  if (rc_iter == type_name_bimap.right.end()) {
    throw std::runtime_error("Invalid metric_name");
  }
  return rc_iter->second;
}

double compute_metric(const VectorXd& p, const VectorXd& d,
                      const std::string& metric_name) {
  double N = p.size();
  if (N != d.size()) error("Mismatch between prediction and data vector sizes");

  METRIC_TYPE mtype = metric_type(metric_name);

  switch (mtype) {
    case METRIC_TYPE::SUM_SQUARED:
      return (p - d).squaredNorm();
    case METRIC_TYPE::MEAN_SQUARED:
      return (p - d).squaredNorm() / N;
    case METRIC_TYPE::ROOT_MEAN_SQUARED:
      return std::sqrt((p - d).squaredNorm() / N);
    case METRIC_TYPE::SUM_ABS:
      return (p - d).lpNorm<1>();
    case METRIC_TYPE::MEAN_ABS:
      return (p - d).lpNorm<1>() / N;
    case METRIC_TYPE::MAX_ABS:
      return (p - d).lpNorm<Eigen::Infinity>();
    case METRIC_TYPE::ABS_PERCENTAGE_ERROR:
      return (p - d).cwiseQuotient(d).lpNorm<1>();
    case METRIC_TYPE::MEAN_ABS_PERCENTAGE_ERROR:
      return (p - d).cwiseQuotient(d).lpNorm<1>() / N;
    /* Warning: This definition of R^2 only has meaning for OLS when
     * d is the training responses */
    case METRIC_TYPE::R_SQUARED:
      double dbar = d.sum() / N;
      return (p.array() - dbar).matrix().squaredNorm() /
             (d.array() - dbar).matrix().squaredNorm();
  }
  /* needed to silence a warning */
  return 0.0;
}

}  // namespace util
}  // namespace dakota
