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
 * \file chunkAlloc.h
 *
 * This is some simple code that can be #include'd into a class to make
 * it use the chunk allocator.
 */

#ifndef pebbl_chunkAlloc_h
#define pebbl_chunkAlloc_h

#include <acro_config.h>
#define INSERT_CHUNKALLOC \
\
public:\
  void*  operator new(size_t /*size*/)       { return memory.allocate(); };\
  void   operator delete(void* p)        { memory.putBack(p); };\
  static void memClear(bool check=false) { memory.wipe(check); };\
private:\
  static chunkAllocator memory;\

#endif
