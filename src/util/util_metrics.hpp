/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_UTIL_METRICS_HPP
#define DAKOTA_UTIL_METRICS_HPP

#include "util_data_types.hpp"

namespace dakota {
namespace util {

enum class METRIC_TYPE { SUM_SQUARED,
                         MEAN_SQUARED,
                         ROOT_MEAN_SQUARED,
                         SUM_ABS,
                         MEAN_ABS,
                         MAX_ABS,
                         ABS_PERCENTAGE_ERROR,
                         MEAN_ABS_PERCENTAGE_ERROR,
                         R_SQUARED };

METRIC_TYPE metric_type(const std::string &metric_name);

/**
 *  \brief Computes the difference between prediction and data vectors
 *  \param[in] VectorXd The prediction vector.
 *  \param[in] VectorXd The data vector.
 *  \param[out] VectorXd The difference between the prediction and data vector.
*/
double compute_metric(const VectorXd &p, const VectorXd &d, const std::string &metric_name);

} // namespace util
} // namespace dakota

#endif // include guard
