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
 * \file DoubleMatrix.h
 *
 * Defines the \c DoubleMatrix typedef
 */

#ifndef utilib_DoubleMatrix_h
#define utilib_DoubleMatrix_h

#include <utilib_config.h>
#include <utilib/Num2DArray.h>

namespace utilib {

/**
 * \typedef DoubleMatrix
 *
 * Type for Num2DArray<double> classes.
 */
typedef Num2DArray<double> DoubleMatrix;

} // namespace utilib

#endif
