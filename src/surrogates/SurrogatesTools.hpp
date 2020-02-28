/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_SURROGATES_TOOLS_HPP
#define DAKOTA_SURROGATES_TOOLS_HPP

#include "util_data_types.hpp"
#include <Eigen/Dense>

namespace dakota {
namespace surrogates {

/**
 *  \brief Utility to compute number of index coefficients for a given level
 *
 */
void size_level_index_vector( int num_dims, int level, Eigen::MatrixXi &indices );

/**
 *  \brief Utility to ...
 *
 */
void compute_hyperbolic_subdim_level_indices( int num_dims, int level,
                                              int num_active_dims, double p,
                                              Eigen::MatrixXi & indices );

/**
 *  \brief Utility to ...
 *
 */
void compute_hyperbolic_level_indices( int num_dims, int level,
                                       double p,
                                       Eigen::MatrixXi & indices );

/**
 *  \brief Utility to ...
 *
 */
void compute_hyperbolic_indices( int num_dims, int level,
                                 double p,
                                 Eigen::MatrixXi & indices );

}  // namespace surrogates
}  // namespace dakota


#endif  // include guard
