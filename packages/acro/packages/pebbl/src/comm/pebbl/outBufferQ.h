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
 * \file outBufferQ.h
 * \author Jonathan Eckstein
 *
 * Defines the pebbl::outBufferQueue class.
 */

#ifndef pebbl_outBufferQueue_h
#define pebbl_outBufferQueue_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#include <utilib/mpiUtil.h>

#include <utilib/CommonIO.h>
#include <utilib/PackBuf.h>
#include <utilib/IntVector.h>
#include <utilib/LinkedList.h>
#include <pebbl/fundamentals.h>

namespace pebbl {

using utilib::CommonIO;
using utilib::PackBuffer;
using utilib::UnPackBuffer;
using utilib::LinkedList;
using utilib::uMPI;
using utilib::IntVector;

class outBufferQElt;     // Forward ref to auxiliary class.


/// Manages buffer space for threads that send multiple messages.
class outBufferQueue : public CommonIO
{
public:
  
  outBufferQueue() {}
  
  virtual ~outBufferQueue();

  void clear();

  void reset(int scavengeSize_ = 1, int startingBufferSize_ = 400)
    {  
      startingBufferSize = uMPI::packSlop(startingBufferSize_);
      scavengeSize=scavengeSize_;
      queue.queue_mode();
      clear();
    }

  // To get an empty buffer.  The buffer is now owned by the caller.

  PackBuffer* getFree();

  // To send a buffer and put it on queue.
  // After this call, the outBufferQueue "owns" the buffer.

  void send(PackBuffer* buffer_,
	    int dest,
	    int tag);

  //  Make sure that all sends with a specific tag (or MPI_ANY_TAG for
  //  all sends) have completed.  Their buffers are deleted.

  void completeAll(int tag);

  //  Adjust starting sizes of newly-created buffers.

  void setStartingBufferSize(int size_)
    {
      startingBufferSize = uMPI::packSlop(size_);
    };

private:

  size_t startingBufferSize;
  size_t scavengeSize;

  LinkedList<outBufferQElt*> queue;
  
};



class outBufferQElt 
{
public:

  outBufferQueue* master;
  REFER_DEBUG(master)

  PackBuffer*    buffer;
  MPI_Request    request;
  int            tag;

  outBufferQElt(PackBuffer* buffer_,int tag_,outBufferQueue* master_) :
  	buffer(buffer_), 
  	request(MPI_REQUEST_NULL), 
        tag(tag_)
    {
      master = master_;
    };

  virtual ~outBufferQElt();

};


// This class is for combining multiple similar messages.  Each piece of a 
// a message is called a "segment".  Segments are packed into buffers.  If
// a buffer is full, that is, it contains maxSegments segments, it is sent 
// immediately.  Otherwise, sending is deferred until the flush method is 
// called.

class multiOutBufferQueue : public CommonIO
{
 public:

  multiOutBufferQueue() : buffer(0), firstProc(noProcessor) {}

  void clear();

  void reset(int maxSegments_,
		      int tag_,
		      int scavengeSize_ = 1,
		      int startingBufferSize_ = 400);

  ~multiOutBufferQueue();
  
  // Call this to get a pointer to a buffer to use to send a segment to
  // processor number 'dest'.
  PackBuffer* startSegmentBuffer(int dest);

  // Call this when you are done writing the segment.  If the buffer is 
  // now full it will be sent immediately.
  void segmentDone(int dest);

  // Call when you are done writing a segment, and you want *just*
  // this buffer to be sent immediately even if it is not full.
  void sendOnly(int dest);

  // Call this when you want *all* (non-empty) buffers sent regardless of
  // whether they are full or not.
  void flush();

  // Tells you how many segments you have used in a buffer.
  int segCount(int dest) { return segmentsUsed[dest]; };

  // Used when reading information -- check if we just read the last segment
  // in the buffer.
  static bool segmentsLeft(UnPackBuffer& inBuf);

  outBufferQueue bufferQ;
  
 private:

  void sendBuffer(int dest);

  // These are essentially random medium-sized integers to help catch errors.
  enum { segmentSeparator = 5187, lastSegmentMarker = 21505 };

  enum { noProcessor = -1 };

  int maxSegments;
  int tag;

  PackBuffer** buffer;            // Array of pointers to buffers.

  IntVector segmentsUsed;         // Number of segments used for each
                                  // possible dest processor.
  int firstProc;                  // Organize partially-filled segments
  IntVector nextProc;             // into a simple linked list.
  IntVector prevProc;

};

} // namespace pebbl

#endif

#endif
