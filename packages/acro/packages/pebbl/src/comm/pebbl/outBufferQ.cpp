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
// outBufferQ.cpp
//
// Manages buffer space for threads that send multiple messages.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/exception_mngr.h>
#include <utilib/mpiUtil.h>
#include <pebbl/outBufferQ.h>

using namespace utilib;
using namespace std;

namespace pebbl {


outBufferQElt::~outBufferQElt()
{
  DEBUGPR(200,ucout << "Deleting obqElt with request " << request  << 
	  ", tag " << tag << '\n');
  uMPI::killSendRequest(&request);
  if (buffer)
    delete buffer;                         // Now it's OK to recycle the buffer
};


void outBufferQueue::send(PackBuffer* buffer,int dest,int tag)
{
  outBufferQElt* sendObj = new outBufferQElt(buffer,tag,this);

  uMPI::isend((void *) buffer->buf(),
	      buffer->size(),
	      MPI_PACKED,
	      dest,
	      tag,
	      &(sendObj->request));
  DEBUGPR(170,ucout << "Sent " << buffer->size() << " bytes to "
	  << dest << ", tag " << tag << ", request handle " 
	  << sendObj->request << '\n');

  queue.add(sendObj);     
}


//  To get a free buffer.  If the number of buffers in use is >= scavengeSize,
//  we try to scavenge a buffer that has already been sent and is now 
//  now available.  If fewer than that many buffers are in use, or none are 
//  free, we return a new PackBuffer of size startingBufferSize.

PackBuffer* outBufferQueue::getFree()
{
  if (queue.size() >= static_cast<size_type>(scavengeSize))
    {
      ListItem<outBufferQElt*>* item = queue.head();

      while (item)
	{
	  if (uMPI::testSend(&(item->data()->request)))
	    {
	      // Now the caller owns the buffer.
	      PackBuffer* toReturn = item->data()->buffer;  
	      item->data()->buffer = 0;
	      queue.remove(item);     
	      DEBUGPR(170,ucout << "Recycling buffer.\n");
	      toReturn->reset();                      // Reset and return the
	      return toReturn;                        // buffer.
	    }
	  item = queue.next(item);
	}
    }
  DEBUGPR(100,ucout << "Making new buffer.\n");
  return new PackBuffer(startingBufferSize);
};


void outBufferQueue::completeAll(int tag)
{
  ListItem<outBufferQElt*> *item = queue.head(), *item2;

  while(item)
    {
      item2 = queue.next(item);

      if ((tag == MPI_ANY_TAG) || (item->data()->tag == tag))
	{
	  MPI_Status status;
	  uMPI::wait(&(item->data()->request),&status,true/*kill request*/);
	  queue.remove(item);
	  
	}
      item = item2;
    }
};


outBufferQueue::~outBufferQueue()
{ clear(); }

void outBufferQueue::clear()
{
  outBufferQElt* toKill;

  while (!queue.empty()) 
    {
      queue.remove(queue.tail(),toKill);
      delete toKill;
    }

};



void multiOutBufferQueue::reset(int maxSegments_,
					 int tag_,
					 int scavengeSize_,
					 int startingBufferSize_)
{
  bufferQ.reset(scavengeSize_,startingBufferSize_);
  maxSegments=maxSegments_;
  tag=tag_;

  int n = uMPI::size;
  buffer = new PackBuffer*[n];
  segmentsUsed.resize(n);
  nextProc.resize(n);
  prevProc.resize(n);
  firstProc = noProcessor;
  for(int i=0; i<n; i++)
    {
      buffer[i]       = NULL;
      segmentsUsed[i] = 0;
      nextProc[i]     = noProcessor;
      prevProc[i]     = noProcessor;
    }
}


void multiOutBufferQueue::clear()
{ bufferQ.clear(); }


multiOutBufferQueue::~multiOutBufferQueue()
{
  if (firstProc != noProcessor)
    EXCEPTION_MNGR(runtime_error, "multiOutBufferQueue deleted with pending sends.");

  clear();
  if (buffer)
     delete[] buffer;
}


PackBuffer* multiOutBufferQueue::startSegmentBuffer(int dest)
{
  if (segmentsUsed[dest] == 0)
    {
      // No active buffer for this destination.  Get a new buffer
      // and push it onto the linked list of buffers.

      DEBUGPR(100,ucout << "Making new buffer for destination " << dest
	      << ".\n");
      buffer[dest] = bufferQ.getFree();
      nextProc[dest] = firstProc;
      if (firstProc != noProcessor)
	prevProc[firstProc] = dest;
      firstProc = dest;
      prevProc[dest] = noProcessor;
    }
  else
    {
      // Otherwise write a separator and return the currently active buffer.

      DEBUGPR(100,ucout << "Continue filling buffer for " << dest 
	      << ", containing " << segmentsUsed[dest] << " segments.\n");
      *(buffer[dest]) << (int) segmentSeparator;
    }
  
  segmentsUsed[dest]++;
  DEBUGPR(100,ucout << "Returning buffer " << buffer[dest] << ", now with "
	  << buffer[dest]->size() << " bytes in " <<
	  segmentsUsed[dest] << " segments.\n");
  DEBUGPRP(100,ucout << "Buffer list:");
  DEBUGPRP(100,for(int i=firstProc; 
		   i!=noProcessor; 
		   i=nextProc[i]) ucout << " " << i);
  DEBUGPR(100,ucout << " end.\n");
  return buffer[dest];
}


void multiOutBufferQueue::segmentDone(int dest)
{
  DEBUGPR(100,ucout << "Segment complete in buffer for " << dest 
	  << ", now containing " << buffer[dest]->size() << " bytes.\n");
  if (segmentsUsed[dest] == maxSegments)     // Is this buffer now full?
    {
      // Yes.  Send it.
      DEBUGPR(100,ucout << "Buffer segment limit reached.\n");
      sendOnly(dest);
    }
}


void multiOutBufferQueue::sendOnly(int dest)
{
  DEBUGPR(100,ucout << "SendOnly called for " << dest << endl);

  DEBUGPR(1000,
  if (buffer[dest] == NULL)
    {
      ucerr << "multiOutBufferQueue::sendOnly called for empty buffer." << endl;
      return;
    }
          );

  // Send the buffer.

  sendBuffer(dest);

  // Unlink it from the list of partially full buffers.

  if (dest == firstProc)
    firstProc = nextProc[dest];
  else
    nextProc[prevProc[dest]] = nextProc[dest];

  if (nextProc[dest] != noProcessor)
    prevProc[nextProc[dest]] = prevProc[dest];

  nextProc[dest] = noProcessor;
  prevProc[dest] = noProcessor;

  DEBUGPRP(100,ucout << "Buffer list:");
  DEBUGPRP(100,for(int i=firstProc; 
		   i!=noProcessor; 
		   i=nextProc[i]) ucout << " " << i);
  DEBUGPR(100,ucout << " end.\n");
}


void multiOutBufferQueue::sendBuffer(int dest)
{
  // Write a last-segment marker.

  (*buffer[dest]) << (int) lastSegmentMarker;

  // Send the buffer.

  DEBUGPR(100,ucout << "Sending buffer at " << buffer[dest] << 
	  ", containing " << buffer[dest]->size() << " bytes to "
	  << dest << ".\n");
  bufferQ.send(buffer[dest],dest,tag);

  // Mark it empty.

  buffer[dest] = NULL;
  segmentsUsed[dest] = 0;
}


void multiOutBufferQueue::flush()
{
  // Go down the list of partially full buffers and send them all.
  // Wipe out the linked list as you go.

  DEBUGPR(100,ucout << "Writing partial buffers.\n");

  for(int dest=firstProc; dest!=noProcessor; )
    {
      sendBuffer(dest);
      DEBUGPR(100,ucout << "Buffer sent to " << dest << ".\n");
      prevProc[dest] = noProcessor;
      int temp = nextProc[dest];
      DEBUGPR(100,ucout << "Next destination is " << temp << ".\n");
      nextProc[dest] = noProcessor;
      dest = temp;
    }

  firstProc = noProcessor;
}


bool multiOutBufferQueue::segmentsLeft(UnPackBuffer& inBuf)
{
  int temp;
  inBuf >> temp;
  if (temp == segmentSeparator)
    return true;
  if (temp != (int) lastSegmentMarker)
    EXCEPTION_MNGR(runtime_error,
		   "Segment length mismatch reading multi-segment message.");
  return false;
}

} // namespace pebbl

#endif
