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
 * \file IntMatrix.h
 *
 * Defines the \c IntMatrix typedef
 */

#ifndef utilib_IntMatrix_h
#define utilib_IntMatrix_h

#include <utilib_config.h>
#include <utilib/Num2DArray.h>

namespace utilib {

/**
 * \typedef IntMatrix
 *
 * Type for Num2DArray<int> classes.
 */
typedef Num2DArray<int> IntMatrix;

} // namespace utilib

#endif
