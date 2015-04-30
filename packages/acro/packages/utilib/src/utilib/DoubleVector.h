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
 * \file DoubleVector.h
 *
 * Defines the DoubleVector typedef
 */

#ifndef utilib_DoubleVector_h
#define utilib_DoubleVector_h

#include <utilib_config.h>
#include <utilib/NumArray.h>

namespace utilib {

/**
 * \typedef DoubleVector
 *
 * Type for NumArray<double> classes.
 */
typedef NumArray<double> DoubleVector;

} // namespace utilib

#endif
