/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_METRICS_HPP
#define DAKOTA_UTIL_METRICS_HPP

#include "util_data_types.hpp"

namespace dakota {
namespace util {

/// Enumeration for supported metric types.
enum class METRIC_TYPE {
  SUM_SQUARED,
  MEAN_SQUARED,
  ROOT_MEAN_SQUARED,
  SUM_ABS,
  MEAN_ABS,
  MAX_ABS,
  ABS_PERCENTAGE_ERROR,
  MEAN_ABS_PERCENTAGE_ERROR,
  R_SQUARED
};

/**
 *  \brief Convert the metric from string to enum.
 *  \param[in] metric_name metric
 *  \returns converted metric
 */
METRIC_TYPE metric_type(const std::string& metric_name);

/**
 *  \brief Computes the difference between prediction and data vectors
 *  \param[in] p prediction vector.
 *  \param[in] d data vector.
 *  \param[in] metric_name metric to compute.
 *  \returns the value of the computed metric.
 */
double compute_metric(const VectorXd& p, const VectorXd& d,
                      const std::string& metric_name);

}  // namespace util
}  // namespace dakota

#endif  // include guard
