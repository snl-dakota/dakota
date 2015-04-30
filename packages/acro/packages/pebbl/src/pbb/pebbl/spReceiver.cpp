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
// spReceiver.cpp
//
// This thread receives subproblems from other processors.  
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>
#include <pebbl/packPointer.h>
#include <pebbl/outBufferQ.h>

using namespace std;

namespace pebbl {


//  Constructor.  

spReceiverObj::spReceiverObj(parallelBranching* global_) :
messageTriggeredPBThread(global_,
			 "Subproblem Receiver",
			 "SP Receive",
			 "pink",
			 3,100,
			 computeBufferSize(global_),
			 global_->deliverSPTag)
{ 
  // Override buffer size if specified in run parameters
  // This is done here instead of in computeBufferSize(...) because
  // we don't wanted expanded by "packSlop" in the messageTriggeredPBThread
  // constructor.

  if (global_->parameter_initialized("spReceiveBuf"))
    bufferSize = global_->spReceiveBuf;
  inBuf.resize(bufferSize);
  DEBUGPRX(50,global,name << " buffer size overridden to " 
	   << bufferSize << endl);
}


// Compute buffer size (but don't bother if it'll be overriden)

int spReceiverObj::computeBufferSize(parallelBranching* global_)
{
  if (global_->parameter_initialized("spReceiveBuf"))
    return 0;
  int segmentSize = sizeof(void*) + sizeof(double) + global_->rememberPackSize;
  return sizeof(int) + (segmentSize + sizeof(int))*(global_->maxSPPacking);
}


//  Run method.  Unpack received problem, and if we can't 
//  immediately fathom it,  place it in the worker pool.  
//  We now also check if we got a buffer enlargement warning instead
//  of a set of packed subproblems.  In that case, enlarge the buffer
//  and go back to receiving.

ThreadObj::RunStatus spReceiverObj::handleMessage(double* controlParam)
{
  int byteCount;
  MPI_Get_count(&status,MPI_PACKED,&byteCount);
  DEBUGPR(100,ucout << "Byte count = " << byteCount << ".\n");

  int signal = -1;
  inBuf >> signal;

  if (signal == spBufferWarningSignal)
    {
      int wantBufSize = -1;
      inBuf >> wantBufSize;
      DEBUGPR(100,ucout << "Want buffer of " << bufferSize 
	      << " expanded to " << wantBufSize << endl);
      if ((int) bufferSize < wantBufSize)
	{
	  inBuf.resize(wantBufSize);
	  bufferSize = wantBufSize;
	  DEBUGPR(100,ucout << "Buffer expanded\n");
	}
      return RunOK;
    }

  if (signal != spDeliverSignal)
     EXCEPTION_MNGR(runtime_error, "spReceiver got undecipherable signal");

  do 
    {
      spToken* hubAddress = (spToken*) unpackPointer(inBuf);
      double bound;
      inBuf >> bound;
      DEBUGPRX(100,global,"Bound of arriving problem is " << bound << '\n');
      parallelBranchSub* p = global->blankParallelSub();
      p->unpackProblem(inBuf);
      DEBUGPRXP(20,global,"Received subproblem " << p);
      global->addToWorkerPool(p,bound,hubAddress);
    } while(multiOutBufferQueue::segmentsLeft(inBuf));
  return RunOK;
}

} // namespace pebbl

#endif
