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
 * \file memUtil.h
 * \author Jonathan Eckstein
 *
 * Little kludges to see how much memory we have used, and how much is left.
 */

#ifndef pebbl_memutil_h
#define pebbl_memutil_h

#include <acro_config.h>
#include <utilib/ParameterSet.h>
#include <pebbl/fundamentals.h>

namespace pebbl {

// Right now, memUtil is defined only for Solaris and Janus (Cougar)

#if defined(SOLARIS) || defined(COUGAR)
#define MEMUTIL_PRESENT
#endif


#ifdef MEMUTIL_PRESENT

class memUtil : public utilib::ParameterSet
{
public:

  ///
  memUtil()
	: memTrack(0)
	{
	create_parameter("memTrack",memTrack,
		"<int>","0",
		"TODO",
		utilib::ParameterLowerBound<int>(0) );
	}

  ///
  static long int inUseBytes();

  ///
  static inline double inUse()     // Amount in use in megabytes.
	{
	return inUseBytes()/(1024.0*1024.0);
	}

#ifdef MEMORY_TRACKING

  ///
  static void baseline() { base = inUseBytes(); };

  ///
  static void trackPrint(const char* string);

#endif

protected:

  ///
  int memTrack;
  
  ///
  static long int base;
  
};

#else

#undef MEMORY_TRACKING

#endif


#ifdef MEMORY_TRACKING
#define MEMORY_IF(i) \
   if ((memUtil::memTrack() > 0) && \
       (i % ((int) memUtil::memTrack()) == 0)) 
#define MEMORY_TRACK(i,s) MEMORY_IF(i) memUtil::trackPrint(s)
#define MEMORY_BASELINE memUtil::baseline()
#else
#define MEMORY_IF(i)       // Nothing
#define MEMORY_TRACK(i,s)  // Nothing
#define MEMORY_BASELINE    // Nothing
#endif



// Class for allocating small pieces of memory in big chunks without much
// overhead.

// Each "chunk" allocated consists of "chunkSize" void*'s, organized
// as follows (with sizes in terms of void*'s):
//
//                 pointer to next chunk  (size = wordLength)
//                 small object #1        (size = datumSize)
//                 small object #2        (size = datumSize)
//                 ...
//                 small object #multiple (size = datumSize)
//
// When a small object isn't in use, the first void* slot in it contains 
// a pointer to the next unused object.


class chunkAllocator 
{
 public:

  // Constructor

  chunkAllocator(int datumSize_, 
		 const char* name="generic",
		 int multiple_=100);

  // Call this to allocate a small piece of memory.
  // It turns out to be most convenient to pass the memory addresses
  // in and out as "void*", but it doesn't really matter.

  void* allocate();

  // Call this to recycle a small piece of memory.

  void putBack(void* datum);

  // Call this to truly free all storage for small objects.  
  // The flag says whether to check for memory leaks.

  void wipe(bool checkForLeaks=true);

  // Destructor.

  ~chunkAllocator() { wipe(); };
  
private:

  const char* name;    // Mainly for printing error messages.

  // The following end up measured in units of (void*)'s, although 
  // they are initially calculated in bytes/chars.

  static int wordSize; // Alignment multiple for the machine 
                       // (typically the length of a double).
  int datumSize;       // Size of small objects to be allocated

  int chunkSize;       // Size of chunks to be malloc'ed.

  // These are simple integers.

  int multiple;        // How may small objects to allocate at a time.

  int allocateCount;   // How many small objects have been made so far.
  int freeCount;       // Of those, how many are unused.

  // Pointers to things

  void** firstChunk;   // Pointer to most-recently-allocated chunk of memory.
  void** firstFreeObj; // Pointer to first unused small object.

  // This function rounds "n" up to a multiple of "divisor".

  static int roundUpToMult(int n,int divisor);

  // Functions for greatest common divisor and least common multiple of
  // two integers

  static int gcd(int a, int b);
  static int lcm(int a, int b) { return a*b/gcd(a,b); };
  
};


// This can be used to initialize the chunk allocator object for a class.

#define CHUNK_ALLOCATOR_DEF(smallObject,mult) \
chunkAllocator smallObject::memory(sizeof(smallObject),#smallObject,mult)

} // namespace pebbl

#endif
