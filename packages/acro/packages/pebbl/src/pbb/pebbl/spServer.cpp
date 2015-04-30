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
// spServer.cpp
//
// This thread receives tokens from other processors, and forwards the
// corresponding subproblem data.  
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


//  Figures out buffer size.

int spServerObj::computeBufferSize()  
{
  int segmentSize = spToken::packSize() + 2*sizeof(int) + sizeof(void*);
  return (segmentSize + sizeof(int))*(parallelPebblParams::maxTokenQueuing);
};


//  Constructor.  The global pointer is set by the base class constructor.
//  Size and allocated buffer, then post a request to receive.

spServerObj::spServerObj(parallelBranching* global_) :
messageTriggeredPBThread(global_,
			 "Subproblem Server",
			 "SP Serve",
			 "tan",
			 3,100,
			 computeBufferSize(),
			 global_->forwardSPTag)
{ }


//  Run method.  Unpack token, destination processor, and child code.
//  Call parallelBranching method to deliver the data 
//  (also does error checks).
//  Now checks for multiple message segments.

ThreadObj::RunStatus spServerObj::handleMessage(double* controlParam)
{
  do
    {
      double bound;
      inBuf >> bound;
      branchSubId spid;
      inBuf >> spid;
      DEBUGPR(100,ucout << "Token is " << spid << bound << '\n');
      int whichChild;
      inBuf >> whichChild;
      parallelBranchSub* p = (parallelBranchSub*) unpackPointer(inBuf);
      int destProcessor;
      inBuf >> destProcessor;
      spToken* hubAddress = (spToken*) unpackPointer(inBuf);
      DEBUGPR(100,ucout << "Want delivery to [" << destProcessor << "].\n");
      if (global->canFathom(bound))
	global->abortDeliverSP(status.MPI_SOURCE);
      else
	global->deliverSP(spid,
			  whichChild,
			  bound,
			  p,
			  destProcessor,
			  hubAddress);
    }
  while(multiOutBufferQueue::segmentsLeft(inBuf));
  
  return RunOK;
}


// This guy runs after we've gobbled all the messages.  It sends out
// any partially full buffers.

void spServerObj::preExitAction() 
{ 
  global->deliverSPBuffers.flush(); 
};

} // namespace pebbl

#endif
