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
 * \file nicePrint.cpp
 *
 * Helper functions for printing
 *
 * \author Jonathan Eckstein
 */

#include <utilib/std_headers.h>
#include <utilib/_math.h>
#include <utilib/nicePrint.h>

using namespace std;

namespace utilib {

int digitsNeededFor(double value)
{
  if (fabs(value) > 0.0)
    return std::max((int) floor(log10(fabs(value))) + 1,1);
  else
    return 1;
}


ostream& hyphens(ostream& stream,int n)
{
  stream.width(n);
  stream.fill('-');
  stream << "";
  stream.fill(' ');
  return stream;
}


const char* plural(int count,const char* suffix)
{
  if (count == 1)
    return "";
  else
    return suffix;
}


ostream& printPercent(ostream& stream,
		      double numerator, 
		      double denominator)
{
  stream.setf(ios::fixed,ios::floatfield);
  int oldPrecision = stream.precision(1);
  stream.width(5);
  if (denominator != 0)
    stream << 100*numerator/denominator;
  else if (numerator == 0)
    stream << 0.0;
  else
    stream << "N/A";
  stream.unsetf(ios::floatfield);
  stream.precision(oldPrecision);
  stream << '%';
  return stream;
}

} // namespace utilib
