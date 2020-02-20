/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/*
 * MathTools.cpp
 * author
 */

/////////////
// Imports //
/////////////

#include <cmath>

#include "MathTools.hpp"

///////////////////////
// Using / Namespace //
///////////////////////

namespace dakota {
namespace util {

///////////////
// Utilities //
///////////////

/* Determine the number of possible combinations of k elements that can be
   chosen from n elements. */
int
n_choose_k(int n, int k)
{
  double value = 1.0;
  for ( int i = 0; i < n-k; i++ )
    value *= (double)(n-i) / (double)(n-k-i);

  return (int)round( value );
}

// ------------------------------------------------------------

} // namespace util
} // namespace dakota
