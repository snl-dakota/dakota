/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_STAT_UTIL_H
#define DAKOTA_STAT_UTIL_H

// Statistics-related utilities

namespace Dakota {

/// clock microseconds-based random seed in [1, 1000000]
int generate_system_seed();

} // namespace Dakota

#endif // DAKOTA_STAT_UTIL_H
