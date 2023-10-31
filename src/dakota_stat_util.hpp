/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_STAT_UTIL_H
#define DAKOTA_STAT_UTIL_H

#include "dakota_data_types.hpp"
#include "dakota_data_util.hpp"

// Statistics-related utilities

namespace Dakota {

/// clock microseconds-based random seed in [1, 1000000]
int generate_system_seed();

#ifdef HAVE_DAKOTA_SURROGATES
/// Compute (non-standardized) linear regression coefficients and return R^2
void compute_regression_coeffs( const RealMatrix & samples,
                                const RealMatrix & resps,
                                RealMatrix & coeffs,
                                RealVector & cods );

/// Compute standardized linear regression coefficients
void compute_std_regression_coeffs( const RealMatrix & samples,
                                    const RealMatrix & resps,
                                    RealMatrix & coeffs,
                                    RealVector & cods );
#endif

} // namespace Dakota

#endif // DAKOTA_STAT_UTIL_H
