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
 * \file IntVector.h
 *
 * Defines the \c IntVector typedef
 */

#ifndef utilib_IntVector_h
#define utilib_IntVector_h

#include <utilib_config.h>
#include <utilib/NumArray.h>

namespace utilib {

/**
 * \fn typedef NumArray<int> IntVector
 *
 * Type for NumArray<int> classes.
 */
typedef NumArray<int> IntVector;

} // namespace utilib

#endif
