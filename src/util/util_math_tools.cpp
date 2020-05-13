/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */


#include "util_math_tools.hpp"

namespace dakota {
namespace util {

// ------------------------------------------------------------

int n_choose_k(int n, int k)
{
  double value = 1.0;
  for (int i = 0; i < n-k; i++)
    value *= (double)(n - i) / (double)(n - k - i);

  return (int) round(value);
}

// ------------------------------------------------------------

} // namespace util
} // namespace dakota
