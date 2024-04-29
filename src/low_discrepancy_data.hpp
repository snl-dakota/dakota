/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef LOW_DISCREPANCY_DATA_H
#define LOW_DISCREPANCY_DATA_H

#include "dakota_data_types.hpp"

namespace Dakota {

/// Generating vector from Cools, Kuo and Nuyens (2006)
extern UInt32 cools_kuo_nuyens_d250_m20[];

/// Recommended generating vector from Kuo's website
/// https://web.maths.unsw.edu.au/~fkuo/lattice/index.html
extern UInt32 kuo_d3600_m20[];

/// Sobol generating matrices from Joe and Kuo (2008)
/// NOTE: Truncated to 1024 dimensions (to avoid storage issues)
extern UInt64 joe_kuo_d1024_t32_m32[][1024];

/// Order 2 Sobol generating matrices [source?]
extern UInt64 sobol_d250_t64_m32[][250];

} // namespace Dakota

#endif