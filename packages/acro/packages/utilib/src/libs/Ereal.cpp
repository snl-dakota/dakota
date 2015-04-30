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

//
// Ereal.cpp
//

#include <utilib_config.h>
#include <utilib/Ereal.h>
#include <utilib/_math.h>

namespace utilib {

#ifndef _MSC_VER
// For everything other than MSVS, we (correctly) define specific static
// member definitions here in a single location (compilation unit).
template<>
const long double Ereal<long double>::positive_infinity_val = MAXLONGDOUBLE;
template<>
const long double Ereal<long double>::negative_infinity_val = -MAXLONGDOUBLE;

template<>
const double Ereal<double>::positive_infinity_val = MAXDOUBLE;
template<>
const double Ereal<double>::negative_infinity_val = -MAXDOUBLE;

#endif

} // namespace utilib
