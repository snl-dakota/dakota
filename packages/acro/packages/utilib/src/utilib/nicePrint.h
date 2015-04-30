/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file nicePrint.h
 *
 * Defines helper functions for printing.
 *
 * \author Jonathan Eckstein
 */

#ifndef utilib_nicePrint_h
#define utilib_nicePrint_h

#include <utilib_config.h>
#include <utilib/std_headers.h>

namespace utilib {

/**
 * Computes the number of digits needed to print a number.
 */
int digitsNeededFor(double value);

/**
 * Uses hyphens to fill a width-delimited string.
 */
std::ostream& hyphens(std::ostream& stream,int n);

/**
 * Returns a string that can be used to pluralize a value.
 */
const char* plural(int count,const char* suffix="s");

/**
 * Print a percentage value.
 */
std::ostream& printPercent(std::ostream& stream,
		      double numerator, 
		      double denominator = 1);

} // namespace utilib

#endif
