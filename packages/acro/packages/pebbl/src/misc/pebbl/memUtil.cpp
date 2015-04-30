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
//
// memUtil.cpp
//
// Little kludges to see how much memory we have used, and how much is left.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#include <utilib/_math.h>
#include <utilib/CommonIO.h>
#include <pebbl/memUtil.h>

#ifdef COUGAR

// removed extern after talking w/ WEH
// results in undefined reference when linking DAKOTA
extern  int profile_total_bytes_used;
#endif


namespace pebbl {


// This code sees how much free memory there is

#ifdef MEMUTIL_PRESENT

// Implementation for Janus

#ifdef COUGAR

long int memUtil::inUseBytes() 
{
  return (long int) profile_total_bytes_used;
}


// Implementation using malloc

#elif defined(CPP_USES_MALLOC)

#include <malloc.h>


long int memUtil::inUseBytes() 
{
  struct mallinfo temp;
  temp = mallinfo();
  return temp.usmblks + temp.uordblks;
}

// Default implementation

#else

#include <unistd.h>

long int memUtil::inUseBytes() 
{
  return (long int) sbrk(0);
}

#endif



// Memory tracking code...

#ifdef MEMORY_TRACKING

long int memUtil::base = 0;

void memUtil::trackPrint(const char* string)
{
  long int mem = inUseBytes();
  cout << string << ": ";
  if (mem > base)
    cout << '+';
  cout << mem - base << " bytes.\n";
  base = mem;
}

#endif

#endif


//
// Here is the code for the small chunk allocator.
//


int chunkAllocator::wordSize = 0;


// Round up code.

int chunkAllocator::roundUpToMult(int n, int divisor)
{
  int remainder = n % divisor;
  if (remainder > 0)
    n += (divisor - remainder);
  return n;
}


// The following is code to for the memory small chunk allocator


// Canned algorithm for GCD of two numbers (which also gives the LCM).

int chunkAllocator::gcd(int a, int b)
{
  if ( a < 0 ) return gcd( -a, b );
  if ( a == 0 ) return b;
  if ( b < a ) return gcd( b, a );
  return gcd( b%a, a );
}


// Code for the constructor.

chunkAllocator::chunkAllocator(int datumSize_, 
			       const char* name_,
			       int multiple_) :
  name(name_),
  multiple(multiple_),
  allocateCount(0),
  freeCount(0),
  firstChunk(NULL),
  firstFreeObj(NULL)
{
  // First calculate stuff in bytes.

  // The word size is the supposed to be a multiple of both the size of a 
  // double and a void* -- this should be 8 on most machines.  Check if
  // it's been initialized, and if not, do so.

  if (wordSize == 0)
    wordSize = lcm(sizeof(void*),sizeof(double));

  // Round up the size of the small objects to the word size.

  datumSize = roundUpToMult(datumSize_,wordSize);

  // Now change size to units of void*'s

  datumSize /= sizeof(void*);

  // Number of void*'s to allocate at a time:

  chunkSize = wordSize/sizeof(void*) + multiple*datumSize;
}


// Code to allocate a small object.

void* chunkAllocator::allocate()
{
  // First see if we need to really get more memory.

  if (!firstFreeObj)
    {
      // Yes, grab a chunk of memory.
      
      void** nextChunk = firstChunk;     // First, remember old first chunk
      firstChunk = new void*[chunkSize]; // Grab memory.

      // Now, the first slot stores the pointer to the previous chunk.
      // After that, start making small objects.  We skip the first
      // "wordSize" chunks to make sure we stay double-aligned.
      // For each small object, store a pointer to the next free object
      // in its first slot.

      *firstChunk = (void*) nextChunk;
      void** cursor = firstChunk + wordSize/sizeof(void*);
      for(int i=0; i<multiple; i++)
	{
	  *cursor      = (void*) firstFreeObj;  // = NULL on first iteration.
	  firstFreeObj = cursor;
	  cursor      += datumSize;
	}
      allocateCount += multiple;    // Record the stuff we allocated.
      freeCount     += multiple;

      // At this point, we've made "multiple" small objects composed of
      // datumSize void*'s each.  The first void* in each one points to 
      // another free small object (except for the first one, where it's 
      // NULL to indicate "no more").
      
    }

  // OK, now we know that firstFreeObj points to an unused small object.
  // We'll return that.  But its first slot contains a pointer to 
  // another free small object, or NULL.  Set firstFreeObj to point to that.

  void* toReturn = (void*) firstFreeObj;
  firstFreeObj = (void**) *firstFreeObj;
  freeCount--;                            // Record the allocation.
  return toReturn;
}


// Code to recycle a small object.  Just push it on the list of free objects.

void chunkAllocator::putBack(void* datum)
{
  freeCount++;                     // Record putting back of object.
  void** pointer = (void**) datum; // First slot in datum gets a link to 
  *pointer = (void*) firstFreeObj; // rest of free list.
  firstFreeObj = pointer;          // Datum now becomes new top of free list.
}


// Code to actually free all memory.  Should be called after all small
// objects have been "put back".

void chunkAllocator::wipe(bool checkForLeaks)
{
  if (checkForLeaks && (freeCount < allocateCount))
    EXCEPTION_MNGR(std::runtime_error, "Memory leak for " << name 
		   << " objects: " << (allocateCount-freeCount) 
		   << " still in use at wipe");

  // Each chunk contains a pointer to the next chunk in its first slot.
  // Chain through the chunks, deleting them, until you hit a NULL pointer.

  while(firstChunk)
    {
      void** toDelete = firstChunk;
      firstChunk = (void**) *firstChunk;
      delete[] toDelete;
    }

  firstFreeObj  = NULL;
  freeCount     = 0;
  allocateCount = 0;
  
}

} // namespace pebbl
