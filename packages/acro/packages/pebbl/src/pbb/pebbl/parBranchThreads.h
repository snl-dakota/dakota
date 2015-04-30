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
 * \file parBranchThreads.h
 * \author Jonathan Eckstein
 *
 * Defines thread objects used by the parallel branching class.
 */

#ifndef pebbl_parBranchThreads_h
#define pebbl_parBranchThreads_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI                     // Compile to stub if no MPI.

#include <utilib/mpiUtil.h>
#include <utilib/logEvent.h>
#include <utilib/PackBuf.h>
#include <utilib/_math.h>
#include <pebbl/parPebblBase.h>
#include <pebbl/ThreadObj.h>
#include <pebbl/SelfAdjustThread.h>
#include <pebbl/coTree.h>
#include <pebbl/outBufferQ.h>


#ifdef EVENT_LOGGING_PRESENT
using utilib::logStateObject;
using utilib::logEvent;
#endif

namespace pebbl {


class parallelBranching;    // Forward declaration.


// Base class for simple thread objects with a pointer to a parallel
// branching class.  

class parBranchingThreadObj : virtual public ThreadObj, public fullPebblBase
{
friend class parallelBranching;
friend class parBranchCoTree;
friend class loadBalSurvey;

protected:

  parallelBranching*     global;

  int    messagesReceived;

  IF_LOGGING_COMPILED(logStateObject myLogState;)

public:

  int    logLevel;

  // This is the debug level at which this thread will start printing.
  // This is different from the run-time debug level, which is in "debug"
  // Only if debug >= dbgLevel will you see generic start and stop printouts
  // from this thread.

  int    dbgLevel;

  int    active;

  virtual int messageCount() { return messagesReceived; };

  parBranchingThreadObj(parallelBranching* global_,
			const char* name_,
			const char* shortName,
			const char* logColor,
			int   logLevel_,
			int   dbgLevel_);

  //  This method is called just before the thread is inserted into
  //  the scheduler, and ONLY on processors where the thread will be active.
  //  Default is to do nothing.

  virtual void startup() { };

  //  Standard run method -- make a possible log entry and debug printout,
  //  then run the real method.

  virtual RunStatus run(double* controlParam);

  //  The method below is the one you should define if you want the
  //  thread to run invocations marked in the event log.

  virtual RunStatus runWithinLogging(double* controlParam)
    { 
      EXCEPTION_MNGR(std::runtime_error,"Empty runWithinLogging method invoked.");
      return RunSTOP; 
    };

  // This can be used to get a clean exit by cleaning up any outstanding
  // communication.  It should assume quiesence, though, so it would mostly
  // apply to receive requests.

  virtual void cancelComm() { };

  virtual ~parBranchingThreadObj() { };

  virtual double usefulTime()   { return run_time; };
  virtual double overheadTime() { return 0;        };

  virtual double adjustTime(double adjustment)
    {
      run_time += adjustment;
      return run_time;
    };

  virtual void setDebugLevel(int level)
    {
      setDebug(level);
    };
  
};


// Same, but self-adjusting.

class parBranchSelfAdjThread : 
public parBranchingThreadObj,
public SelfAdjustThread
{
public:

  parBranchSelfAdjThread(parallelBranching* global_,
			 const char* name_,
			 const char* shortName,
			 const char* logColor,
			 int   logLevel_,
			 int   dbgLevel_);

  RunStatus exec() { return SelfAdjustThread::exec(); }

  ThreadState state() { return SelfAdjustThread::state(); }

  void unwaiting()    { SelfAdjustThread::unwaiting(); }

  double usefulTime()   { return overallTimeSum; }
  double overheadTime() { return run_time - overallTimeSum; }

  double adjustTime(double adjustment)
    {
      parBranchingThreadObj::adjustTime(adjustment);
      overallTimeSum += adjustment;
      return timeSum += adjustment;
    }
};



// A thread designed to harbor coTree operations.  Adds methods
// to allow a contained coTree to access the request and status objects,
// and to prepare the thread to enter into a wait state.

class coTreeReadyPBThread : 
public parBranchingThreadObj
{
public:

  coTreeReadyPBThread(parallelBranching* global_,
		      const char* name_,
		      const char* shortName,
		      const char* logColor,
		      int   logLevel_,
		      int   dbgLevel_) :

  parBranchingThreadObj(global_,(char*)name_,(char*)shortName,
			logColor,logLevel_,dbgLevel_)
    { };

  MPI_Request* requestP() { return &request; };
  MPI_Status*  statusP()  { return &status;  };

  parallelBranching* globalP() { return global; };

  void setToWaitFor(MessageID& tag);
};


//  An abstract class that specializes the coTree abstract class
//  to use within PEBBL threads as above.

class parBranchingCoTree : public coTree, public fullPebblBase
{
protected:

  coTreeReadyPBThread* thread;
  parallelBranching*   global;
  
public:

  void receiveOperation(void* buffer,int size,MessageID& tag_,int src)
    {
      coTree::receiveOperation(buffer,size,tag_,src);
      thread->setToWaitFor(tag_);
    };

  bool requestMet()    // This is always true because we exit for receives
    {                 // and tell the scheduler the thread is waiting.
      return(true);   // The scheduler will restart the thread only if
    };                // the message has arrived.

  // Constructor...

  parBranchingCoTree(MPI_Datatype         datatype_,
		     coTreeReadyPBThread* thread_,
		     treeTopology*        treeP_) :
  coTree(datatype_,
	 thread_->requestP(),    // MPI request is same as calling thread's.
	 thread_->statusP(),     // MPI status is same as calling thread's.
	 treeP_,
	 true),                  // exit for receive operations.
  thread(thread_),
  global(thread_->globalP())
    { };

};



//  This class implements common code for all threads that simply sit
//  and wait for messages, and do not run under any other circumstances.
//  The code used to be duplicated all over the place prior to January 1999.
//  Enhanced to try to devour multiple messages at a time.

class messageTriggeredPBThread : public parBranchingThreadObj
{
public:

  messageTriggeredPBThread(parallelBranching* global_,
			   const char* name_,
			   const char* shortName,
			   const char* logColor,
			   int    logLevel_,
			   int    dbgLevel_,
			   size_t bufferSize_,
			   MessageID tag_,
			   int sender_ = MPI_ANY_SOURCE
		  	   );

  void startup() 
    { 
      postRequest();
      state_flag = ThreadWaiting;
    };

  virtual void cancelComm()
	{
	if (request_posted) {
           uMPI::killRecvRequest(&request);
	   request_posted = false;
	   }
	};

  virtual ~messageTriggeredPBThread() { cancelComm(); };

  size_t sizeOfBuffer() { return bufferSize; };

protected:

  RunStatus runWithinLogging(double* controlParam);

  virtual RunStatus handleMessage(double* controlParam) = 0;

  virtual void preExitAction() { };

  void postRequest();

  size_t       bufferSize;
  UnPackBuffer inBuf;
  int          sender;
  bool         request_posted;
};  



//  This class specializes a messageTriggeredPBThread to do broadcasts.
//  It is used as the basis for the incumbent broadcaster, but can be used 
//  for other things too.

class broadcastPBThread : public messageTriggeredPBThread
{
public:

  broadcastPBThread(parallelBranching* global_,
		    const char* name_,
		    const char* shortName,
		    const char* logColor,
		    int    logLevel_,
		    int    dbgLevel_,
		    size_t bufferSize_,
		    MessageID tag_,
		    int   radix_);
  
  virtual ~broadcastPBThread() { };
  
  virtual void cancelComm()
	{
	if (request_posted) {
           uMPI::killRecvRequest(&request);
  	   outQueue.clear();
	   request_posted = false;
	   }
	};

  RunStatus handleMessage(double* controlParam);

  // Should unpack data from inBuf, including the "originator" of the 
  // broadcast.  Should return true if the broadcast should continue.

  virtual bool unloadBuffer() = 0;

  // These should load a buffer to be sent to another processor.  They
  // apply when initiating or relaying a broadcast, respectively.

  virtual void initialLoadBuffer(PackBuffer* buf) = 0;
  virtual void relayLoadBuffer(PackBuffer* buf) = 0;

  // Methods that do the broadcasting

  void relay(int initiateFlag);  // initiateFlag=true to start, false to relay

  void relay()             { relay(false); };
  void initiateBroadcast() { relay(true);  };

  // This method makes the object which is used to figure out who
  // (if anybody) gets relayed information.  The default is to define
  // a tree over all MPI nodes.

  virtual nAryTree* makeTreeObject()
    {
      return new nAryTree(radix,originator);
    };

  // The originating processor specifies how it wants to
  // be identified.  Threads derived from this may want to
  // use an ID local to a subset of processors

  virtual int myOriginatorID()
    {
      return(uMPI::rank);
    };
  
protected:

  int radix;
  int originator;
  outBufferQueue outQueue;

};

} // namespace pebbl
 

#endif  

#endif
