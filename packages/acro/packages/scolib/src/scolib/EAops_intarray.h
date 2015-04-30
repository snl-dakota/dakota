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

#if 0
/**
 * \file FOO 
 */

//
// EAops_intarray.h
//

#ifndef scolib_EAops_intarray_h
#define scolib_EAops_intarray_h

#include <acro_config.h>
#include <scolib/EAops_array.h>

namespace scolib {

/** Class that defines operations used with NumArray<double> genotypes.
*/
class EAops_intarray : public EAops<EApoint_array<int>, BasicArray<int> >
{
public:

  ///
  EAops_intarray() {}

protected:

};

} // namespace scolib 

#endif
#endif
