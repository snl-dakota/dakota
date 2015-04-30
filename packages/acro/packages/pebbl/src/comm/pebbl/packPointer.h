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
 * \file packPointer.h
 * \author Jonathan Eckstein
 * 
 * Generic code for putting pointer in and out of (Un)PackBuffers.
 */

#ifndef pebbl_packPointer_h
#define pebbl_packPointer_h

#include <acro_config.h>
#include <utilib/PackBuf.h>

namespace pebbl {


#define unpackAs(buf,what,kind,castType) \
   { \
       castType unpackTemp_; \
       buf >> unpackTemp_; \
       kind* tp = (kind*) &unpackTemp_; \
       what = *tp; \
   }


inline void packPointer(PackBuffer& outBuffer,void* ptr)
{
  if (sizeof(void*) == sizeof(int))
    outBuffer << *((int*) &ptr);
  else if (sizeof(void*) == sizeof(long))
    outBuffer << *((long*) &ptr);
  else if (sizeof(void*) == sizeof(double))
    outBuffer << *((double*) &ptr);
  else
    EXCEPTION_MNGR(std::runtime_error, "Can't figure out how to pack " << sizeof(void*) << "-byte pointers");
}


inline void* unpackPointer(UnPackBuffer& inBuffer)
{
  void* temp = 0;
  if (sizeof(void*) == sizeof(int))
    unpackAs(inBuffer,temp,void*,int)
  else if (sizeof(void*) == sizeof(long))
    unpackAs(inBuffer,temp,void*,long)
  else if (sizeof(void*) == sizeof(double))
    unpackAs(inBuffer,temp,void*,double)
  else
    EXCEPTION_MNGR(std::runtime_error, "Can't figure out how to unpack " << sizeof(void*) << "-byte pointers");
  return (void*) temp;
}
  
} // namespace pebbl

#endif
