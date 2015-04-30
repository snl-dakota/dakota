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
// parBranchThreads.cpp
//
// Defines thread objects used by the parallel branching class.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/seconds.h>
#include <utilib/logEvent.h>
#include <pebbl/parBranching.h>
#include <pebbl/parBranchThreads.h>

using namespace std;

namespace pebbl {


parBranchingThreadObj::parBranchingThreadObj(parallelBranching* global_,
					     const char* name_,
					     const char* shortName,
					     const char* logColor,
					     int   logLevel_,
					     int   dbgLevel_) :
  global(global_),
  messagesReceived(0),
  logLevel(logLevel_),
  dbgLevel(dbgLevel_),
  active(false)
  {
    name    = name_;
    request = MPI_REQUEST_NULL;
#ifdef EVENT_LOGGING_PRESENT
    if (global->eventLog && shortName && logColor)
      myLogState.define(shortName,logColor);
#endif
  };


parBranchSelfAdjThread::parBranchSelfAdjThread(parallelBranching* global_,
			const char* name_,
			const char* shortName,
			const char* logColor,
			int logLevel_,
			int dbgLevel_) :
    parBranchingThreadObj(global_,name_,shortName,logColor,logLevel_,dbgLevel_),
    SelfAdjustThread(global_->timeSlice)
{ }


void coTreeReadyPBThread::setToWaitFor(MessageID& tag_)
{
  DEBUGPR(300,ucout << "About to wait on tag " << (int) tag_ << ".\n");
  tag = tag_;
  state_flag = ThreadWaiting;
};


//  Standard run method -- make a possible log entry and debug printout,
//  then run the real method.

ThreadObj::RunStatus parBranchingThreadObj::run(double* controlParam)
{  
  LOG_EVENTX(global,logLevel,start,myLogState);

  global->currentThread = this;

#if defined(ACRO_VALIDATING) || defined(WARNINGS)
  double startTime = CPUSeconds();
#else
  double startTime = 0.0;
#endif      

  DEBUGPR(dbgLevel,ucout << name << " slice, control " 
      << *controlParam << '\n');
  RunStatus sliceStatus = runWithinLogging(controlParam);
  DEBUGPR(dbgLevel,ucout << name << " slice done at " << 
	  CPUSeconds() - startTime << " seconds, control " <<
	  *controlParam << '\n');
  LOG_EVENTX(global,logLevel,end,myLogState);
  return sliceStatus;
};


//  Common code to implement message-triggered threads.

messageTriggeredPBThread::messageTriggeredPBThread(parallelBranching* global_,
						   const char* name_,
						   const char* shortName,
						   const char* logColor,
						   int    logLevel_,
						   int    dbgLevel_,
						   size_t bufferSize_,
						   MessageID tag_,
						   int sender_) :
parBranchingThreadObj(global_,name_,shortName,logColor,logLevel_,dbgLevel_),
bufferSize(uMPI::packSlop(bufferSize_)),
inBuf(new char[bufferSize],bufferSize,1),
sender(sender_)
{
  request_posted=false;
  tag = tag_;
  DEBUGPRX(50,global,"Message-triggered thread for " << name 
	   << ", bufferSize=" << bufferSize << ", tag=" << (int) tag
	   << ", sender=" << sender << '\n');
}


void messageTriggeredPBThread::postRequest()
{
  uMPI::irecv((void *) inBuf.buf(),
	      bufferSize,
	      MPI_PACKED,
	      sender,
	      tag,
	      &request);
  DEBUGPR(100,ucout << name << " thread posted request for tag "
	  << tag << endl);
  request_posted=true;
}


// This code runs when a message-triggered thread is activated.
// Gobble and process as many messages as possible, not just the one
// that caused the thread to be invoked.  There may be many more messages that 
// have already been locally buffered waiting for a matching receive to be 
// posted.  When all the messages seem exhausted, invoke the "preExitAction"
// method.

// Note that the inBuf.reset call resets InBuf so that its read limit 
// is the exact length of the message (as gleaned from the status object).

ThreadObj::RunStatus 
messageTriggeredPBThread::runWithinLogging(double* controlParam)
{
  RunStatus returnStatus;

  do
    {
      DEBUGPR(100,ucout << name << " thread processing message\n");
      messagesReceived++;
      inBuf.reset(&status);
      returnStatus = handleMessage(controlParam);
      postRequest();
    }
  while(uMPI::test(&request,&status));

  DEBUGPR(100,ucout << name << " thread done processing messages\n");

  preExitAction();

  state_flag = ThreadWaiting;
  return returnStatus;
}


//  Code for generic broadcast operations.

//  Constructor just makes the underlying messageTriggeredThread, sets the
//  tree radix, and creates and outBufferQueue.

broadcastPBThread::broadcastPBThread(parallelBranching* global_,
				     const char* name_,
				     const char* shortName,
				     const char* logColor,
				     int    logLevel_,
				     int    dbgLevel_,
				     size_t bufferSize_,
				     MessageID tag_,
				     int   radix_) :
  messageTriggeredPBThread(global_,
			   name_,
			   shortName,
			   logColor,
			   logLevel_,
			   dbgLevel_,
			   bufferSize_,
			   tag_),
  radix(radix_)
{
  outQueue.reset(2*radix,bufferSize);
  DEBUGPRX(50,global,"Broadcast thread created for "
	   << name << ", radix=" << radix << '\n');
}


//  Generic message handling for broadcasts: unpack the buffer, and if
//  it's OK to continue the broadcast, relay the message.

ThreadObj::RunStatus broadcastPBThread::handleMessage(double* controlParam)
{
  global->recordMessageReceived(this);
  int continueFlag = unloadBuffer();
  if (continueFlag)
    relay();
  return RunOK;
}


//  Generic code to either initiate or relay a generic broadcast operation.

void broadcastPBThread::relay(int initiateFlag)
{
  DEBUGPR(dbgLevel, ucout << name 
	  << (initiateFlag ? " initiate broadcast" : " relay") 
	  << " called, time=" << MPI_Wtime() - global->baseTime << '\n');

  if (initiateFlag)
    originator = myOriginatorID();
  
  nAryTree* treeP = makeTreeObject();

  DEBUGPRP(dbgLevel, ucout << name << " data sent to:");

  while(treeP->childrenLeft() > 0)
    {
      DEBUGPRP(dbgLevel, ucout << ' ' << treeP->currentChild());
      // Purify complains that this outBuf is never deleted, but 
      // that is OK, as soon as it is sent, outQueue owns it, and will
      // delete it.
      PackBuffer* outBuf = outQueue.getFree();
      if (initiateFlag)
	initialLoadBuffer(outBuf);
      else
	relayLoadBuffer(outBuf);
      outQueue.send(outBuf,treeP->operator++(0),tag);
			// tree++ doesn't work with SUNOS CC compiler
      global->recordMessageSent(this);
    }

  DEBUGPRX(dbgLevel,global," (done)\n");

  delete treeP;
}

} // namespace pebbl

#endif
