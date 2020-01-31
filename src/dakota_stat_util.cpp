/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// Statistics-related utilities

#include <chrono>

namespace Dakota {

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

} // namespace Dakota
