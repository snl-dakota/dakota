/*  _________________________________________________________________________
 *
 *  Acro: A Common Repository for Optimizers
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README.txt file in the top Acro directory.
 *  _________________________________________________________________________
 */

/**
 * \file fundamentals.h
 *
 * Common declarations.
 */

#ifndef pebbl_fundamentals_h
#define pebbl_fundamentals_h

#include <acro_config.h>
#include <utilib/_generic.h>
#include <utilib/exception_mngr.h>
#include <utilib/_math.h>

namespace pebbl {

enum optimizeSense {minimize=1, maximize=-1, unknown=0};
//void templates();


//  The ffs function in Solaris seemed to be returning garbage,
//  so I wrote this.

inline int firstOneBit(int x)
{
  if (x < 0)
    x = -x;
  int result = 0;
  while(x > 0)
    {
      x >>= 1;
      result++;
    }
  return result;
}

} // namespace pebbl

#endif
