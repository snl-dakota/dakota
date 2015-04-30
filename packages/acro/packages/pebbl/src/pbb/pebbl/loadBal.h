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
 * \file loadBal.h
 * \author Jonathan Eckstein
 *
 * Principle code for PEBBL to have multiple hubs and load balance between
 * them.
 */

#ifndef pebbl_loadBal_h
#define pebbl_loadBal_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <mpi.h>
#include <utilib/PM_LCG.h>
#include <utilib/default_rng.h>
#include <utilib/IntVector.h>
#include <utilib/IntMatrix.h>
#include <pebbl/clusterTree.h>
#include <pebbl/loadBalPair.h>
#include <pebbl/parLoadObject.h>
//#include <pebbl/parBranching.h>

namespace pebbl {


// Forward declaration:

class loadBalObj;


//  This class figures out the global load across all hubs.

class loadBalSurvey : public parBranchingCoTree
{
friend class loadBalObj;
friend class parallelBranching;

public:

 loadBalSurvey(coTreeReadyPBThread* thread_,
	       treeTopology*        treeP_);

  void initAction();
  void upMessageAction();
  void upRelayAction();
  void rootAction();
  void downReceiveAction();

  void* upSendBuf()        { return (void*) outBuf.buf(); };
  void* upRecvBuf()        { return (void*) inBuf.buf();  };
  void* downSendBuf(int i) { return (void*) outBuf.buf(); };
  void* downRecvBuf()      { return (void*) inBuf.buf();  };

  int upSendBufSize()        { return outBuf.size(); };
  int upRecvBufSize()        { return bufSize;       };
  int downSendBufSize(int i) { return outBuf.size(); };
  int downRecvBufSize()      { return bufSize;       };

private:

 int           bufSize;
 parLoadObject load;
 UnPackBuffer  inBuf;
 PackBuffer    outBuf;
 int           lastPrint;
 double        lastPrintTime;

};


//  This is another coTree-based class.  Counts potential donors and
//  receivers of work, and assigns unique numbers to each cluster.

using utilib::IntMatrix;

class loadBalCount : public parBranchingCoTree
{
public:

  loadBalCount(loadBalObj*   balThread_,
	       treeTopology* treeP_);

  ~loadBalCount() { };

  void initAction();
  void upMessageAction();
  void upRelayAction();
  void rootAction();
  void downReceiveAction();
  void downRelayAction();
  void downRelayLoopAction(int i);

  void* upSendBuf()        { return (void*) upBuf.data();   };
  void* upRecvBuf()        { return (void*) upBuf.data();   };
  void* downSendBuf(int i) { return (void*) childBuf[i];    };
  void* downRecvBuf()      { return (void*) downBuf.data(); };

  int upSendBufSize()        { return 2; };
  int upRecvBufSize()        { return 2; };
  int downSendBufSize(int i) { return 4; };
  int downRecvBufSize()      { return 4; };

private:

  loadBalPair             clusterCount;
  BasicArray<loadBalPair> childCount;
  loadBalPair             subTreeCount;
  loadBalObj*             balThread;

  IntVector upBuf;
  IntVector downBuf;
  IntMatrix childBuf;

};


class intSumTree : public parBranchingCoTree
{
public:

  int value;

  intSumTree(coTreeReadyPBThread* thread_,
	     treeTopology*        treeP_) :
    parBranchingCoTree(MPI_INT,thread_,treeP_),
    value(0)
    { };

  void initAction()      { runningSum  = value; };
  void upMessageAction() { runningSum += value; };
  void upRelayAction()   { value = runningSum;  };
  void rootAction()      { value = runningSum;  };

  void* upSendBuf()        { return (void*) &value; };
  void* upRecvBuf()        { return (void*) &value; };
  void* downSendBuf(int i) { return (void*) &value; };
  void* downRecvBuf()      { return (void*) &value; };

  // Buffer sizes left at default, which is 1.

protected:

  int runningSum;
  
};  


//  At last, the thread that actually manages load balancing.

class loadBalObj : public coTreeReadyPBThread
{
friend class loadBalCount;
friend class parallelBranching;

public:

  // Constructor.

  loadBalObj(parallelBranching* global_);

  // Cancel the pending messages

  void cancelComm() { outBufQ.clear(); }

  // Slightly funky computation of state.  This is not a strictly 
  // message-triggered thread because nodes of the cluster tree block
  // until they think the time for load balancing is right.

  ThreadState state();

  // Main method to run thread.

  RunStatus runWithinLogging(double* controlParam);

  // To compute total messages received:

  int messageCount()
    {
      return messagesReceived
	  + surveyObject.messagesReceived
	    + countObject.messagesReceived
	      + termCheckObject.messagesReceived;
    };

  int numRounds()          { return roundNumber;     };
  int numSurveyRestarts()  { return surveyRestarts;  };
  int numQuiescencePolls() { return quiescencePolls; };
  int numTermChecks()      { return termChecks;      };

  void setDebugLevel(int level);

private:

  // State information for main state machine.

  enum loadBalState
    {
      start,
      blockPoint,
      startSurvey,
      surveying,           // This state has secondary state in surveyObject
      counting,            // This state has secondary state in countObject
      receiverInfo,
      rendezvous,
      donorWait,
      knowDonor,
      receiverWait,
      knowReceiver,
      rendezvousDone,
      returnWait,
      donorKnowsReceiver,
      workWait,
      startIdleCheck,
      quiescencePoll,
      quiescencePollWait,
      startTermCheck,
      termCheckCluster,
      termCheckClusterWait,
      termCheckTree,       // This state has secondary state in termCheckObject
      dead
    };
  
  loadBalState myState;

  // Tree information

  nAryClusterTree tree;

  int myCluster;

  // Secondary state machines used for tree-of-clusters operations.

  loadBalSurvey surveyObject;
  loadBalCount  countObject;
  intSumTree    termCheckObject;

  // Working information on matching donors and receivers etc.

  loadBalPair eligible;
  loadBalPair total;
  loadBalPair myID;

  int lbPairs;
  int iAmDonor;
  int iAmReceiver;
  int iAmRVPoint;

  int receiverProc;          // Receiver for this rendezvous pair.
  int donorProc;             // Donor for this rendezvous pair.
  int myReceiver;            // The receiver that applies if I'm a donor.

  parLoadObject receiverLoad;

  enum { unknown = -1 };

  // Random number stuff.

  utilib::PM_LCG lbRandom;

  // Buffer information...

  int bufferSize;

  outBufferQueue outBufQ;
  UnPackBuffer   inBuf;

  // Message tags...

  MessageID donorRVTag;
  MessageID receiverRVTag;
  MessageID returnRVTag;
  MessageID loadBalTag;

  // Time of last load balance, for spacing

  double lastLoadBalTime;

  // Whether doing a survey restart
  bool surveyRestartFlag;

  // Used by final termination check.

  int workersReplied;
  int termCheckTarget;
  int workerCount;
  
  // Functions used within the class.

  int  computeBufferSize();
  bool canStart();
  void waitToReceive(MessageID& tag_);
  int  subproblemsProcessed();
  void stateEntryDebugPrint(const char* stateName);

  // For end-of-run statisics, debugging, and error checking.

  int roundNumber;
  int surveyRestarts;
  int quiescencePolls;
  int termChecks;

};

}  // namespace pebbl

#endif

#endif
