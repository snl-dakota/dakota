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
 * \file parBranching.h
 * \author Jonathan Eckstein
 *
 * Parallel branching class definition for PEBBL.
 */

#ifndef pebbl_parBranching_h
#define pebbl_parBranching_h

#include <acro_config.h>
#include <pebbl/branching.h>

#ifdef ACRO_HAVE_MPI

#include <utilib/IntVector.h>
#include <utilib/PackBuf.h>
#include <utilib/seconds.h>
#include <utilib/logEvent.h>
#include <pebbl/parPebblBase.h>
#include <pebbl/parPebblParams.h>
#include <pebbl/clustering.h>
#include <pebbl/outBufferQ.h>
#include <pebbl/parLoadObject.h>
#include <pebbl/Scheduler.h>
#include <pebbl/workerThread.h>
#include <pebbl/workerAuxThread.h>
#include <pebbl/earlyOutputThread.h>
#include <pebbl/hubThread.h>
#include <pebbl/incumbSearchThread.h>
#include <pebbl/incumbCast.h>
#include <pebbl/spReceiver.h>
#include <pebbl/spServer.h>
#include <pebbl/loadBal.h>
#include <pebbl/reposThreads.h>
#include <pebbl/llChainer.h>
#include <pebbl/scatterObj.h>
#include <pebbl/workerInHeap.h>
#include <pebbl/reposArrayInHeap.h>
#include <pebbl/packedSolution.h>
#include <pebbl/chunkAlloc.h>


// John S's magic so we don't need an operator= for GenericHeaps

namespace utilib {
template<typename T>
struct BasicArray_registration<GenericHeap<T> > {
   static bool registrar() { return false; }
};
}


namespace pebbl {

using utilib::uMPI;
using utilib::LocatedGenericHeap;
#ifdef EVENT_LOGGING_PRESENT
using utilib::logEvent;
#endif

class parallelBranchSub;  // Forward declarations.
class spToken;
class parSPHandler;


// Extra class for parallel load logging

class parLoadLogRecord : public loadLogRecord
{
 public:

  int hubPool;
  int serverPool;
  int gLoad;
  int cLoad;
  int releases;
  int dispatch;
  int reception;
  int rebal;
  int lbRounds;

  double globalBound;
  double clusterBound;
  double hubBound;
  double serverBound;

  // Simple constructor that makes an emtpy record
  parLoadLogRecord(int sense);

  // Place-holder destructor
  ~parLoadLogRecord() { };

  // Overload of write-to-stream operation to write more info than in serial
  void writeToStream(std::ostream& os, 
		     int           sense, 
		     double        baseTime, 
		     int           proc = 0);

  // Pack
  void pack(PackBuffer& pb);

  // Unpack
  void unpack(UnPackBuffer& upb);

  // Utility to compute size of a packed object
  static size_type packSize();
};


// The main event...

class parallelBranching : 
virtual public branching, 
#ifdef EVENT_LOGGING_PRESENT
  virtual public logEvent,
#endif
virtual public parallelPebblBase,
virtual public parallelPebblParams
{

  friend class parLoadObject;
  friend class parSPHandler;
  friend class workerInHeap;
  friend class workerInQHeap;
  friend class incumbSearchObj;
  friend class parallelBranchSub;
  friend void  loadReduceOp(void* invec,void* inoutvec,
			    int* len,MPI_Datatype* dt);

public:

  // After defining a serial class derived from 'branching',
  // you have to define a way to pack and unpack problem-specific 
  // data into and out of communication buffers.  You must also
  // provide a function that allows sizing of receive buffers
  // for subproblems (application-specific data only).

  virtual void pack(PackBuffer& outBuffer) = 0;
  virtual void unpack(UnPackBuffer& inBuffer) = 0;

  virtual int spPackSize() = 0;

  // This broadcasts a problem read on the I/O processor to all the
  // other processors.

  virtual void broadcastProblem();

  // Standard version of setup for parallel branching classes

  virtual bool setup(int& argc, char**& argv);

  // These are used to write application-dependent state when
  // checkpointing

  virtual void appCheckpointWrite(PackBuffer& outBuf)  { };
  virtual void appCheckpointRead(UnPackBuffer& inBuf)  { };
  virtual void appMergeGlobalData(UnPackBuffer& inBuf) { };

  // Printout methods

  void printConfiguration(std::ostream& stream = ucout);

  void printSPStatistics(std::ostream& stream = ucout);
  void printTimings(std::ostream& stream = ucout);

  // To do a parallel search.
  
  virtual double
    parallelSearchFramework(parSPHandler* handler_ = NULL);

  double search() { return parallelSearchFramework(NULL); };

  // To resolve ambiguities in diamond inheritance structure.

  void solve();

  void printSolValue(std::ostream& stream = std::cout);

  solution* getSolution(int whichProcessor = allProcessors);

  void getAllSolutions(BasicArray<solution*>& solArray,
		       int whichProcessor = allProcessors);

  int startRepositoryScan();

  solution* nextRepositoryMember(int whichProcessor = allProcessors);


  // Managing solutions

  BasicArray<solution*> refSolArray;
  size_type             numRefSols;
  int                   solBufSize;

  int       computeSolBufferSize();
  size_type registerFirstSolution(solution* referenceSolution);
  size_type registerSolution(solution* referenceSolution);
  void      clearRegisteredSolutions();

  solution* unpackSolution(UnPackBuffer& inBuf);

  // To set up the scheduler.

  virtual void initializeScheduler();
  virtual void placeTasks();
  void placeTask(parBranchingThreadObj* thread,int cond,int group);

  // To abort cleanly without hangs or error messages

  virtual void cleanAbort();

  /// To cancel communication
  virtual void cancelComm();

  // To do with setting the root problem

  virtual parallelBranchSub* makeParRoot();

  // Loading and unloading problems

  virtual void loadCurrentParSP(parallelBranchSub* p);
  void unloadCurrentSP();
  void eraseCurrentSP();

  void unloadCurrentSPtoPool()
    {
      workerPool->insert(currentParSP);
      unloadCurrentSP();
    }

  // Counting subproblems

  inline int spCount()
    { 
      return workerPool->load().count() + haveCurrentSP(); 
    };

  // Override the base relGap() function that gives the overall gap at
  // this point in the search -- this version takes used the
  // globalLoad object.  Must overload the versions that takes an
  // objective value or load object as an argument too, because of C++
  // funkiness.

  double relGap() 
    { 
      if (rampingUp())
	{
	  loadObject lo = updatedPLoad();
	  return branching::relGap(lo);
	}
      return branching::relGap(globalLoad); 
    };

  double relGap(loadObject& l)     { return branching::relGap(l); };
  double relGap(double boundValue) { return branching::relGap(boundValue); };

  // Outputs the solution, but only from one processor.

  void printSolution(const char* header = "",
		     const char* footer = "",
		     std::ostream& outStream = ucout);

  // Puts solution in a file, but only on one processor
  // Replaces the version in branching::

  void solutionToFile();

  // Parallel incumbent heuristic stuff.  By default, there is no
  // heuristic, but a virtual slot is provided for one.
  // The method incumbentThreadBias() dynamically calculates the 
  // scheduler bias for the incumbent thread.  This allows the 
  // time devoted to that thread to fall as the gap closes.

  void signalIncumbent();

  virtual bool hasParallelIncumbentHeuristic() { return false; };

  virtual void parallelIncumbentHeuristic(double* controlParam) 
    {
      *controlParam = 0;
    };

  virtual ThreadObj::ThreadState incumbentHeuristicState()
    {
      return ThreadObj::ThreadBlocked;
    };

  virtual double incumbentThreadBias();

  // If derived classes have their own classes of MPI messages and want
  // them to be incorporated into the general message tracking and 
  // termination procedure, they should record transmission and receipt
  // with these methods.

  void recordMessageSent(parBranchingThreadObj* callingThread)
    {
      messages.general.sent++;
      // preventIdleDeadlock(callingThread);
    };

  void recordMessageReceived(parBranchingThreadObj* callingThread)
    {
      messages.general.received++;
      // preventIdleDeadlock(callingThread);
    };

  // JE commented this out (along with calls to it above), as we're
  // now using an active poll procedure to check for balance of
  // general messages.

  // // This method is used by both of the above (and conceivably by other
  // // derived classes) to make sure that termination doesn't get hung up
  // // simply because no messages arrive to wake up the threads.

  // void preventIdleDeadlock(parBranchingThreadObj* callingThread)
  //   {
  //     forceWorkerToRun = true;
  //     if (searchInProgress && 
  // 	  iAmHub() && 
  // 	  (suspending() || !(clusterLoad.busy())))
  // 	activateHub(callingThread);
  //   };

  // Construction and destruction.

  // This doesn't initialize the virtual base class branching via a
  // call to branchingInit. parallelBranching is usually a base class
  // itself for a class that will initialize branching (e.g. this
  // class has pure virtual functions).

  parallelBranching();

  virtual parallelBranchSub* blankParallelSub() = 0;

  virtual ~parallelBranching();

  void reset(bool VBflag=true);

  // Helper to set the debugging level

  virtual void setDebugLevel(int level);
  virtual void setDebugLevelWithThreads(int level);

  // Ramp-up feature support

  int    rampUpFlag;
  int    rampUpMessages;
  int    rampUpPool;
  int    rampUpBounds;
  double rampUpTime; // CPU time
  double rampUpWCTime; // wallclock time

  int    crossoverFlag;
  
  int rampingUp()    { return rampUpFlag;    };
  int crossingOver() { return crossoverFlag; };

  // Does the ramp up; returns total subproblems that result

  void rampUpSearch();  

  // Makes sure all processors agree on value and location of incumbent.

  void rampUpIncumbentSync();

  // Decides whether the ramp-up phase should continue

  bool keepRampingUp()
    {
    if (forceContinueRampUp())
      return(true);
    return(continueRampUp());
    }

  virtual bool continueRampUp() 
    { 
      if (spCount() <= rampUpPoolLimit)
	return true;
      return spCount() <= rampUpPoolLimitFac*uMPI::size;
    };

  // Force a longer ramp up.  This is useful when we need more careful
  // branching at the start of the computation.  Keep the branching
  // synchronized even if the parallelism isn't being used well.

  virtual bool forceContinueRampUp()
    {
    return (probCounter < minRampUpSubprobsCreated);
    }

  // Clean up any data structures, etc, used only for ramp up

  virtual void rampUpCleanUp() {};

  // Clean abort and print some statistics after ramp-up.

  virtual void postRampUpAbort(double aggBound);

  // Miscelleaneous stuff

  int veryFirstWorker()
    {
      return (iAmWorker() && 
	      (clusterNumber() == 0) && 
	      (workerPosInCluster() == 0));
    };

  int preprocessMessages;

protected:

  void packAll(PackBuffer& outBuffer)
    {
      packGeneric(outBuffer);
      pack(outBuffer);
    };

  void packGeneric(PackBuffer& outBuffer);

  void unpackAll(UnPackBuffer& inBuffer)
    {
      unpackGeneric(inBuffer);
      unpack(inBuffer);
    };

  void unpackGeneric(UnPackBuffer& inBuffer);

  int spGenericPackSize() { return 6*sizeof(int) + 2*sizeof(double); };
  int spAllPackSize()     { return spGenericPackSize() + spPackSize(); };

  int hubMessageSize(int tokens, int acks);

  // parallelBranchSub* pointer to current subproblem

  parallelBranchSub* currentParSP;

  // Objects needed by the scheduler.

  Scheduler sched;                      // Scheduler object.

  int highPriorityGroup;                // Groups within the scheduler.
  int baseGroup;

  // Linked list of all threads

  LinkedList<parBranchingThreadObj*> threadsList;

  // Which thread is current active

  parBranchingThreadObj* currentThread;

  // The threads themselves

  workerThreadObj* worker;              // Thread to bound subproblems, and
  workerAuxObj*    workerAux;           // ... auxiliary worker thread
  incumbCastObj*   incumbentCaster;     // ... to broadcast incumbent data
  incumbSearchObj* incumbentSearcher;   // ... to look around for better ones
  hubObj*          hub;                 // ... to manage work assignment
  loadBalObj*      loadBalancer;        // ... to balance work between hubs
  spServerObj*     spServer;            // ... to deliver subproblem data
  spReceiverObj*   spReceiver;          // ... to receive subproblem data
  earlyOutputObj*  earlyOutputter;      // ... to help with early output
  reposRecvObj*    reposReceiver;       // ... to manage enumeration
  reposMergeObj*   reposMerger;
  llChainObj*      llChainer;           // ... manage continuous load log writes

  virtual incumbSearchObj* createIncumbentSearchThread()
	{ return new incumbSearchObj(this); }

  // Friend class declarations needed for the threads to function.

  friend class parBranchingThreadObj;
  friend class broadcastPBThread;
  friend class workerThreadObj;
  friend class earlyOutputObj;
  friend class workerAuxObj;
  friend class hubObj;
  friend class incumbCastObj;
  friend class spReceiverObj;
  friend class spServerObj;
  friend class loadBalObj;
  friend class loadBalSurvey;
  friend class reposRecvObj;
  friend class reposMergeObj;
  friend class llChainObj;

  // Handler to be used when doing the search.  This should hide the
  // serial object with the same name in the branching class.

  parSPHandler* handler;

  // Message tags

  MessageID forwardSPTag;      // Sent by hub, received by spServer.
  MessageID deliverSPTag;      // Sent by spServer, received by spReceiver.
  MessageID hubTag;            // Anything sent to a hub.
  MessageID workerTag;         // Messages from hub to worker.
  MessageID quiescencePollTag; // Check for general message balance.
  MessageID termCheckTag;      // worker->hub messages for termination confirm.
  MessageID earlyOutputTag;    // Early output signals.
  MessageID incumbCastTag;     // Incumbent broadcast.
  MessageID printSolutionTag;  // For solution printout
  MessageID repositoryTag;     // For asynchronous repository management

  MessageID rampUpHashTag;     // For synchronous repository management
  MessageID reposSyncTag;
  MessageID fetchSolTag;
  MessageID deliverSolTag;
  MessageID reposAuxTag;

  MessageID reconfigSPTag;      // Used when doing serial checkpoint restarts
  MessageID reconfigSolTag;
  MessageID reconfigResizeTag;
  MessageID reconfigDoneTag;

  MessageID llSyncOutTag;       // For writing load logs when no common
  MessageID llSyncBackTag;      // file system
  MessageID llDataTag;          // For sending load log data
  MessageID llTokenTag;         // Supports token ring for load log writing

  // To store information about general system workload.

  parLoadObject clusterLoad;
  parLoadObject globalLoad;
  
  // Master message balance counters for this processor.

  pbMessageBlock messages;

  // Other counters used to keep statistics.

  int hubHandledCount;
  int spReleaseCount;
  int spDeliverCount;
  int loadBalSPCount;
  int rebalanceSPCount;
  int rebalanceCount;
  int myHubsRebalCount;
  int broadcastMessageCount;

  // Message counters for final statistics printout

  int    solOutputMessages;
  double solOutputTime;      // CPU time
  double solOutputWCTime;    // wallclock time

  double messagesReceivedThisProcessor;
  double totalMessages;

  // Hub-worker clustering stuff.

  clusterObj cluster;

  bool iAmWorker()             { return cluster.iAmFollower; };
  bool iAmHub()                { return cluster.iAmLeader; };

  int numWorkers()             { return cluster.numFollowers; };
  int numRemoteWorkers()       { return numWorkers() - iAmHub(); };
  int clusterNumber()          { return cluster.clusterNumber; };
  int workerPosInCluster()     { return cluster.followerPosInCluster; };
  int myHub()                  { return cluster.leader; };
  int totalWorkers()           { return cluster.totalNumFollowers; };
  int numHubs()                { return cluster.numClusters; };
  int hubProc(int cNum)        { return cluster.leaderOfCluster(cNum); };
  int whichWorker(int src)     { return cluster.whichFollower(src); };
  int workerProc(int src)      { return cluster.follower(src); };
  int overallWorkerProc(int w) { return cluster.globalFollower(w); };  
  int whichCluster(int n)      { return cluster.whichCluster(n); };
  int firstHub()               { return cluster.leaderOfCluster(0); };
  int typicalClusterSize()     { return cluster.typicalSize; };
  int lastClusterSize()        { return cluster.lastClusterSize; };

  bool iAmFirstHub()           { return iAmHub() && (clusterNumber() == 0); };
  bool isHub(int n)            { return cluster.isLeader(n); };
  bool isWorker(int n)         { return cluster.isFollower(n); };
  bool lastCluster()           { return clusterNumber() + 1 == numHubs(); };
  bool pureHubThisCluster()    { return cluster.separateFunctions; };
  bool typicalHubsPure()       { return cluster.typicallySeparated; };
  bool lastHubPure()           { return cluster.lastSeparated; };

  // Data for tracking what fraction of the time hubs are busy with
  // hub-related work, and what time is free to be a worker

  double typicalHubBusyFraction;
  double lastHubBusyFraction;
  double myWorkerTimeFrac;
  double adjustedWorkerCount;

  // Functions related to same

  double adjustedWorkersInCluster(int cNum)
  {
    if (cNum + 1 == numHubs())
      return lastClusterSize() - lastHubBusyFraction;
    return typicalClusterSize() - typicalHubBusyFraction;
  };

  double adjustedWorkersInCluster()
  {
    return adjustedWorkersInCluster(clusterNumber());
  };

  double hubBusyFractionCalc(bool separated, int n, pbTimeTrackBlock& track);
  void   setHubBusyFractions();
  
  void setHubTracking(parLoadObject& load);
  void setHubTimeBlock(pbTimeTrackBlock& tt);

  double workerTimeFrac(int workerIndex);  // worker index within cluster

  double setMyWorkerTimeFrac()
  {
    myWorkerTimeFrac = workerTimeFrac(workerPosInCluster());
    DEBUGPR(100,ucout << "myWorkerTimeFrac=" << myWorkerTimeFrac << '\n');
    return myWorkerTimeFrac;
  };

  // This routine is used by both the worker and the hub to figure out
  // if a worker is low on work

  int lowWorkerCount();

  // Various data used by worker functions

  int forceWorkerToRun;

  int    workerLastPrint;
  double workerLastPrintTime;

  double workLeft;
  double workUsed;

  int                   spAckCount;
  BasicArray<spToken*>  spAckAddress;
  DoubleVector          spAckBound;

  parLoadObject lastWorkerReport;

  int releaseProbCount;
  int releaseTestCount;

  PackBuffer*    workerOutBuffer;
  outBufferQueue workerOutQ;

  scatterObj releaseDecision;
  scatterObj otherHubRelease;

  int incumbentThreadExists;

  double boundKnownToHub;

#ifdef ACRO_VALIDATING
  int hubMessageSeqNum;
#endif

  // Methods used by the worker thread to actually do the work

  loadObject load();

  void workerExecute(double* controlParam);
  void processSubproblem();

  void pruneIfNeeded();

  void computeBound(parallelBranchSub* p);
  void statusLine(loadObject& l, const char* tag="");

  void workerDisposeSP(parallelBranchSub* p);
  bool workerShouldRelease();
  int  scatterHub();
  bool shouldScatterToRandom();
  bool shouldCommunicateWithHub(int triggerCount = 1);
  void workerCommunicateWithHub(bool rebalanceFlag = false);
  void addToWorkerPool(parallelBranchSub* p,double bound,spToken* hubAddress);
  void getLoadInfoFromHub(UnPackBuffer& inBuf);
  int  rebalanceIfNeeded();

  void workerRelease(parallelBranchSub* p,
		     int whichChild,
		     int multiplicity,
		     int whichHub,
		     bool rebalanceFlag = false);

  // Stuff needed by the spServer/spReceiver threads.

  // Stores data for subproblems controlled by hub.

  heapPool< parallelBranchSub,
            loadObject,
            DynamicSPCompare<parallelBranchSub> > 
    serverPool;

  // Buffers used for deliver these subproblems to other processors.

  multiOutBufferQueue deliverSPBuffers;

  // Used to send "enlarge your buffer" warnings to other workers

  outBufferQueue auxDeliverSPQ;

  // Lower bounds on sizes of other workers buffers

  IntVector knownBufferSize;

  // Method to send subproblems from one worker to the next

  void deliverSP(branchSubId& id,
		 int whichChild,
		 double bound,
		 parallelBranchSub* p,
		 int destProcessor,
		 spToken* hubAddress);

  // Use instead for pruned problems.

  void abortDeliverSP(int hubProc);

  // Subroutines of deliverSP

  PackBuffer* startPackingSP(double bound,int dest,spToken* hubAddress);
  void        finishDeliverSP(PackBuffer* outBuf,int dest);

  // Data needed by hub.

  branchPool<spToken,parLoadObject>* hubPool; 

  multiOutBufferQueue dispatchSPBuffers;
  outBufferQueue      hubAuxBufferQ;

  int       wantLoadInformCount;
  IntVector wantLoadInform;
  IntVector workerToInform;
  int       sumWorkerReportCount;
  IntVector workerRebalanceCount;
  int       hubDispatchCount;
  bool      needHubPruning;
  IntVector workerReportCount;

  BasicArray<parLoadObject> workerLoadReport;
  BasicArray<parLoadObject> workerLoadEstimate;

  BasicArray< heapPool<spToken,parLoadObject> > workerTransitPool;

  // Keep track of workers by number of subproblems owned

  BasicArray<workerInHeap> workerHeapObj; 
  LocatedGenericHeap<workerInHeap> heapOfWorkers;

  // Keep track of workers by quality of their best bound

  BasicArray<workerInQHeap> workerQHeapObj; 
  LocatedGenericHeap<workerInQHeap> qHeapOfWorkers;

  branchPool<parallelBranchSub,parLoadObject> *workerPool;

  int    hubLastPrint;
  double hubLastPrintTime;

  // These are required to prevent spurious activation of hub logic,
  // and to prevent race conditions between the hub and termination check
  // (when there is only one hub).

  int searchInProgress;
  int termCheckInProgress;
  int wantAnotherTermCheck;
  
  // Methods needed by hub.

  void     handleHubMessage(UnPackBuffer& inBuf,int src);
  void     activateHub();
  void     activateHub(parBranchingThreadObj* callingThread);
  void     hubDistributeWork();
  void     hubPrune();
  void     hubCalculateLoads();
  void     tellWorkersLoads();
  spToken* tokenToSend(branchPool<spToken,parLoadObject>* thePool);
  int      lowerLoadWorker(int w1,int w2);
  int      worseQualityWorker(int w1,int w2);
  void     hubSendWorkTo(int w);
  void     repositionWorker(int w);
  void     alertWorkers(int code);
  void     clusterTerminate();
  void     setToInform(int w);
  void     setToInformAll();
  // bool     needClusterCheck();   May not need any more

  parLoadObject updatedPLoad();

  int workerCount(int w)
    {
      return workerLoadEstimate[w].count();
    };

  double workerBound(int w)
    {
      return workerLoadEstimate[w].aggregateBound;
    };

  // Stuff needed for multiple hubs and load balancing

  void fillLoadBalBuffer(PackBuffer& buffer,
			 loadObject  receiverLoad,
			 int         receiver);

  void unloadLoadBalBuffer(UnPackBuffer& buffer);

  void decideLoadBalAvailability(loadBalPair& eligible);

 // Remember if MPI was running when readAndBroadCast was called.

  int mpiWasRunning;

  // Remember which processor has the incumbent solution stored.

  int incumbentSource;

  // To remember size of packed subproblems and tokens.

  int rememberPackSize;

  // To support early output of solutions

  int outputInProgress;

  bool parallelNeedEarlyOutput()
    {
      if (!iAmFirstHub())
	return false;
      if (suspending())
	return false;
      if (outputInProgress)
	return false;
      return serialNeedEarlyOutput();
    }


  // Aborting the run do to excessive time or computation
  // Unlike the other aborts, this one tries to unwind out to the
  // calling program gracefully, rather than just calling exit().
  // That makes things trickier.

  bool aborting;                // Set if we are trying to abort

  void setupAbort();            // Set above variable and tell workers
                                // to abort too if we are a hub

  void clearAllSPsForAbort();  // Get rid of all subproblems so that
                               // destructors will get called in the
                               // right sequence

  // Methods to support parallel enumeration

  int owningProcessor(solution* sol)
    {
      size_type hash = sol->computeHashValue();
      return (hash % (uMPI::size*enumHashSize)) / enumHashSize;
    };

  void assignId(solution* sol);

  bool offerToRepository(solution* sol, syncType sync=notSynchronous);

  void pruneRepository();

  bool processOwnedSolution(solution* sol);
  bool disposeSolution(solution* sol);
  void routePackedSolution(packedSolution* psol);
  void sendPackedSolution(packedSolution* psol,int link);

  void solForwardAction(UnPackBuffer& inBuf,int sender);
  void solAckAction(int sender);

  void reposArraySend();
  void reposArrayMerge();
  void reposTreeRootAction();
  void lastSolRelay();
  bool mergeNow();

  bool incorporateSolution(UnPackBuffer& inBuf);
  void incorporateReposArray(UnPackBuffer& inBuf, int sender);
  void incorporateLastSol(UnPackBuffer& inBuf);
  void recordArrayArrival(int sender);

  void syncLastSol();
  void syncReposArrays(int maxLength, int& messageCounter);
  void rampUpRepositorySync();
  void finalReposSync();

  void adjustReposStats(loadObject& l);

  void printRepository(std::ostream* outStreamP);
  void printReposStatistics(std::ostream& stream = std::cout);

  // Data needed to support parallel enumeration.  After
  // reposChildren, everything else is for managing sorted arrays of
  // solution identifiers that are progressively merged up the
  // reposTree.

  int parallelSolCounter;         // Counts solutions owned by this processor

  int    totalReposSize;          // Used in ramp up and final printout
  double worstInRepos;
  bool   finalReposSyncDone;
  int    solsSent;

  BasicArray<solution*> rampUpSolQ;  // Buffers solutions during ramp-up
  size_type rampUpSolsPending;       // Number of solutions in above queue

  outBufferQueue solHashQ;           // Buffer queues used in asynchronous
  outBufferQueue reposArrayQ;        // repository management
  outBufferQueue newLastSolQ;
  outBufferQueue solAckQ;

  treeTopology* reposTree;           // Shape of the communication tree
  int           reposChildren;       // Cache number of children of this node

  BasicArray<solutionIdentifier> treeReposArray;
  BasicArray<solutionIdentifier> localReposArray;
  BasicArray< BasicArray<solutionIdentifier> > childReposArray;

  int reposArrayBufSize;

  BasicArray<reposArrayInHeap> childMergeObject;
  reposArrayInHeap             localMergeObject;

  GenericHeap<reposArrayInHeap> reposArrayHeap;

  double reposSkewSeconds;
  bool   needReposMerge;
  int    childArraysReceived;
  double lastMergeTime;

  BasicArray<bool> gotChildArray;

  size_type    queuedSolutions;
  unsigned int nProcessorBits;

  BasicArray<packedSolution*> solJustSent;
  BasicArray< GenericHeap<packedSolution> > solSendQ;

  int reposArrayCursor;
                             
  // Checkpointing

  bool restarted;               // Was run was restarted from a checkpoint?

  bool checkpointsEnabled;      // Checkpoints being used on this run
  bool checkpointing;           // Want to write a checkpoint now
  bool writingCheckpoint;       // Actually writing a checkpoint right now
  bool readingCheckpoint;       // Reading a checkpoing now

  int checkpointNumber;         // Sequence number of this checkpoint
  int restartCPNum;             // Sequence number of checkpoint restarted from

  double baseWallTime;          // Wall clock time at start of run
  double checkpointTriggerTime; // When to start next checkpoint
  double checkpointStartTime;   // When current checkpoint began
  double checkpointTotalTime;   // Total time spent writing checkpoints

  int cpAbortNum;               // Number of the checkpoint we will
                                // abort at (0 if no planned abort)

  // Figures out what checkpoint we should abort at, if any

  void prepareCPAbort();

  // Should we start a new checkpoint now?

  bool checkpointDue()
    {
      if (!checkpointsEnabled)
	return false;
      if (checkpointing)
	return false;
      if (outputInProgress)
	return false;
      return (WallClockSeconds() >= checkpointTriggerTime);
    };

  // Initiate a checkpoint

  void setupCheckpoint();

  // Actually write the checkpoint, assuming it is OK to do so

  void writeCheckpoint();

  // Restart from a checkpoint -- if false returned, no files were found

  bool restartFromCheckpoint();    // General restart, which is either...
  bool parallelRestart();          // ... one file per processor, or
  bool reconfigureRestart();       // ... serial read and redistribute work.

  // Subroutines of reconfigureRestart...

  void reconfigureRestartRoot(int filesFound);
  void reconfigureRestartLeaf(int filesFound);

  void reconfigureBufferCheck(UnPackBuffer& buf,int p,IntVector& rBufSize);

  // Figure out the global load situation after reading a checkpoint

  void restartSetLoads();

  void addPackedLoads(char* inbuf,char* inoutbuf);
  
  // Checkpoint filename manipulations

  std::string checkpointFilename(int k, int p);
  void   deleteCheckpointFile(int k, int p);

  int  scanForCheckpointFiles(int processor=MPI_ANY_SOURCE);
  bool checkpointFileMatch(std::string& filename,int& k, int& p);

  static bool stringMatch(std::string& str,int& cursor,const char* pattern);
  static bool intFromString(std::string& s,int& cursor,int& result);


  // This is just the 'or' of "checkpointing" and "aborting"
  // It's used in many places, so we have a special shorthand.

  bool suspending() { return checkpointing || aborting; };


  // Validation file stuff.

  std::ostream* valLogFile();

  // If ramping up, suppress validation log output on processors other than 0.

  bool valLogOutput()
    {
      return validateLog && !(rampingUp() && (uMPI::rank > 0));
    };

  // Load logging stuff; overrides serial versions (but may call on them)

  void startLoadLogIfNeeded();
  void recordLoadLogData(double time);

  void writeLoadLog();

  bool logTransition()
  {
    if (logTransitions)
      {
	recordLoadLogData(WallClockSeconds());
	return true;
      }
    return false;
  }

  void loadLogSMPWrite();
  void writeLoadLogPassToken();
  void receiveLLToken();

  parLoadLogRecord* pLastLog;
  bool              haveLLToken;
  MPI_Request       llSendRequest;

  // To help in printing out configuration and timing information.

  void configLine(std::ostream& stream,int pWidth,int number, const char* kind);

  void timingPrintText(std::ostream& stream,
		       char  percentTotalFill = '-',
		       char  restFill = '-',
		       const char* text1 = "",
		       const char* text2 = "",
		       const char* text3 = "",
		       const char* text4 = "",
		       const char* text5 = "",
		       const char* text6 = "",
		       const char* text7 = "",
		       const char* text8 = "");
		       
  void timingPrintData(std::ostream& 
		       stream,
		       const char* name,
		       int present,
		       double time,
		       double messageCount);

  virtual const char* preprocessPhaseName() { return "Preprocessing"; };

  double baseTime;
  
  double totalCPU;          // CPU time
  double maxCPU;            // CPU time
  double totalWC;           // wallclock time
  double maxWC;             // wallclock time

// for computing wall clock time in parallel.  There may be a serial
// parameter we could use here instead.

  double searchWCTime;

  double broadcastTime;     // CPU time
  double broadcastWCTime;   // wallclock time
  double preprocessTime;    // CPU time
  double preprocessWCTime;  // wallclock time

  int timingPrintNameWidth;
  int timingPrintPNWidth;
  int timingPrintTimeWidth;
  int timingPrintWCTimeWidth;  // another for wall clock time if doing both
  int timingPrintMessageWidth;

  int spTotal;


  // Things related to event logging 

#ifdef EVENT_LOGGING_PRESENT

  // Get the event log started

  void initEventLog();

  // Finish up the event log

  void finishEventLog()
  {
    if (uMPI::iDoIO)
     ucout << "(Writing event log.)\n";
    logEvent::finish();
  }

  logStateObject preprocLogState;
  logStateObject workerLogState;
  logStateObject boundLogState;
  logStateObject foundIncLogState;
  logStateObject pruneLogState;
  logStateObject hubPruneLogState;
  logStateObject hubLogState;
  logStateObject statusPrintLogState;
  logStateObject rebalLogState;
  logStateObject pCreateLogState;
  logStateObject pBoundedLogState;
  logStateObject pDeadLogState;
  logStateObject pSplitLogState;
  logStateObject rampUpLogState;
  logStateObject checkpointLogState;

#endif



  // This feature hasn't been used for a long while; commented out by JE

// #if NO_STRING_STREAM
//   char* outputCacheBuf;
//   std::ostrstream* outputCache;
// #else
//   std::string outputCacheBuf;
//   std::ostringstream* outputCache;
// #endif

//   virtual bool outputCacheNeeded();
//   void writeCachedOutput();


};


// This class fulfills a similar function, but for subproblems.

class parallelBranchSub : 
virtual public branchSub, 
virtual public parallelPebblBase
{
public:

  friend class parallelBranching;

  // This function should be set up to return a pointer back
  // to the instance of parallelBranching that "owns" the subproblem

  virtual parallelBranching* pGlobal() const = 0;

  // Again, the main thing the user must do is define how to pack and
  // and unpack application-dependent data.

  virtual void pack(PackBuffer& outBuffer) = 0;
  virtual void unpack(UnPackBuffer& inBuffer) = 0;

  // The following function derives a child from a subproblem and
  // places in a PackBuffer.  An obvious implementation is provided.
  // The user is free to substitute a more efficient implementation.

  virtual void packChild(PackBuffer& outBuffer,int whichChild)
    {
      parallelBranchSub* p = parallelChild(whichChild);
      p->packProblem(outBuffer);
      p->recycle();
    }

  // This function says whether a token can be made from a subproblem,
  // which might depend on the subproblems state or internal state in
  // some applications.  The default is that a token can be made.

  virtual bool canTokenize() { return true; };

  // Incumbent stuff.  quickIncumbentHeuristic is run directly in the
  // worker thread.  feedToIncumbentThread feeds the subproblem data
  // to the separate incumbent thread.  

  virtual void quickIncumbentHeuristic() { };
  virtual void feedToIncumbentThread()   { };

  // The above are not used during ramp-up.  Instead, we call 
  // rampUpIncumbentHeuristic().  The default implementation calls
  // the *serial* incumbent heuristic and then makes sure the results
  // synch up.

  virtual void rampUpIncumbentHeuristic()
    {
      if (bGlobal()->haveIncumbentHeuristic())
	{
	  incumbentHeuristic();
	  pGlobal()->rampUpIncumbentSync();
	}
    };

  // This allows you to check whether we are ramping up

  bool rampingUp() { return pGlobal()->rampUpFlag; };

  // Local information.

  int tokenCount;         // Number of subproblems pointed to by tokens.

  // Construction/destruction

  parallelBranchSub() :
    tokenCount(0)
  { };

  virtual parallelBranchSub* parallelChild(int whichChild);
  virtual parallelBranchSub* makeParallelChild(int whichChild) = 0;

#ifdef EVENT_LOGGING_PRESENT
  virtual int splitProblem()
    {
      LOG_EVENTX(pGlobal(),4,start,pGlobal()->pSplitLogState);
      int returnValue = branchSub::splitProblem();
      LOG_EVENTX(pGlobal(),4,end,  pGlobal()->pSplitLogState);
      return returnValue;
    };
#endif

  int loadXFactor() const
    {
      if (state == separated)
	return childrenLeft - tokenCount;
      else
	return state != dead;
    };

  virtual ~parallelBranchSub() { };

  // These pack and unpack an entire subproblem, including generic
  // information.
  
  void packProblem(PackBuffer& outBuffer)
    {
      DEBUGPRX(160,bGlobal(),"packProblem called.\n");
      packGeneric(outBuffer);
      pack(outBuffer);
    };

  void unpackProblem(UnPackBuffer& inBuffer)
    {
      DEBUGPRX(160,bGlobal(),"unpackProblem called.\n");
      unpackGeneric(inBuffer);
      unpack(inBuffer);
      if (valLogOutput() && !pGlobal()->readingCheckpoint)
	valLogUnpackPrint();
    };

  // Used to determine whether whether a problem still needs to
  // be in the worker pool.

  bool stillNeededByWorker() { return childrenLeft > tokenCount; };

protected:

  void packGeneric(PackBuffer& outBuffer);
  void unpackGeneric(UnPackBuffer& inBuffer);
  void packChildGeneric(PackBuffer& outBuffer);

  int valLogProc() { return id.creatingProcessor; };

  void valLogPackChildPrint();
  void valLogPackPrint();
  virtual void valLogUnpackPrint();

  virtual void valLogPackChildExtra() { };
  virtual void valLogPackExtra()      { };
  virtual void valLogUnpackExtra()    { };

  void valLogDestroyPrint()
    {
      if (pGlobal()->crossingOver())
	return;
      branchSub::valLogDestroyPrint();
    };
  
};



//  Class used to store subproblem tokens.

class spToken : public coreSPInfo, public parallelPebblBase
{
public:

  int spProcessor;
  int whichChild;
  int childrenRepresented;
  parallelBranchSub* memAddress;

  spToken(){};

  spToken(parallelBranchSub* sp,int childNum,int childCount);

  spToken(branching* global_,UnPackBuffer& inBuffer) : 
  	globalPtr(global_) 
    { unpack(inBuffer); };

  spToken(branching* global_) :
  	globalPtr(global_)
      { };

  branching* bGlobal() const { return globalPtr; };

  void recycle() { delete this; };

  int loadXFactor() const { return childrenRepresented; };
  
  ~spToken() { };

  void pack(PackBuffer& outBuffer);
  void unpack(UnPackBuffer& inBuffer);
  static void packEmpty(PackBuffer& buff);

  static int packSize()
  { 
    if (packedSize < 0)
      computePackSize();
    return packedSize; 
  };

  static void computePackSize()
  {
    PackBuffer trashBuf(128);
    packEmpty(trashBuf);
    packedSize = trashBuf.size();
  };

  void debugPrint(std::ostream& s) const;

  // Use custom allocation

  INSERT_CHUNKALLOC
  
private:
  
  static int packedSize;

  branching* globalPtr;

};


//  Inline definitions not possible before...
//  These are part of the hub logic.

inline void parallelBranching::workerDisposeSP(parallelBranchSub* p)
{
  if (p->tokenCount > 0)
    {
      DEBUGPR(100,ucout << "Moving " << p << " to server pool.\n");
      serverPool.insert(p);
    }
  else
    p->recycle();
};


// Parallel handler stuff

// Generic parallel handler.

class parSPHandler : public parallelPebblBase, virtual public spHandler
{
 public:

  virtual ~parSPHandler() { };

  virtual void setPGlobal(parallelBranching* pGlobal_)
    {
      pGlobal = pGlobal_;
      global  = pGlobal_;
    };

 protected:

  parallelBranching* pGlobal;   // Pointer back to parallel class
  parallelBranchSub* pp;        // Parallel version of current problem
  parallelBranchSub* pc;        // Parallel version of current child

  // Quick way to check ramp-up status

  inline bool rampingUp() { return pGlobal->rampUpFlag; };

  // The following operations are different in parallel handlers than 
  // in serial ones.

  void setProblem()    { pp = pGlobal->currentParSP;  p = pp;       };
  void erase()         { pGlobal->eraseCurrentSP();                 };
  void computeBound()  { boundOperation(pp);                        };
  void heuristic();    // Defined in pbWorker.cpp
  bool shouldRelease() { return pGlobal->workerShouldRelease();     };
  void releaseChild()  { releaseSP(pp,anyChild);                    };
  void getChild()      { pc = pp->parallelChild(anyChild);  c = pc; };
  void insertChild()   { pGlobal->workerPool->insert(pc);           };
  void eraseChild()    { pGlobal->workerDisposeSP(pc);              };

  void boundOperation(parallelBranchSub* sp)
    {
      pGlobal->computeBound(sp);
    };

  void releaseSP(parallelBranchSub* sp,int whichChild)
    {
      pGlobal->workerRelease(sp,whichChild,1,pGlobal->scatterHub());
    };
};


// Parallel version of lazy handler.  Do everything according to the
// generic parallel handler, except execute(), which comes from serial lazy.

class parLazyHandler : 
virtual public parSPHandler,
virtual public lazyHandler
{
 public:

  virtual ~parLazyHandler() { };

  void execute() { lazyHandler::execute(); };

 protected:

  void computeBound()  { parSPHandler::computeBound();         };
  void heuristic()     { parSPHandler::heuristic();            };
  bool shouldRelease() { return parSPHandler::shouldRelease(); };
  void releaseChild()  { parSPHandler::releaseChild();         };
  void insertChild()   { parSPHandler::insertChild();          };
  void eraseChild()    { parSPHandler::eraseChild();           };
  
};

  
// Parallel version of hybrid handler.  Do everything according to the
// generic parallel handler, except execute(), which comes from serial 
// hybrid, and stillNeedSP.

class parHybridHandler : 
virtual public parSPHandler,
virtual public hybridHandler
{
 public:

  virtual ~parHybridHandler() { };

  void execute() { hybridHandler::execute(); };

 protected:

  void computeBound()  { parSPHandler::computeBound();         };
  void heuristic()     { parSPHandler::heuristic();            };
  bool shouldRelease() { return parSPHandler::shouldRelease(); };
  void releaseChild()  { parSPHandler::releaseChild();         };
  void insertChild()   { parSPHandler::insertChild();          };
  void eraseChild()    { parSPHandler::eraseChild();           };

  bool  stillNeedSP()   { return pp->stillNeededByWorker() && !canFathom(); };
  
};


// Parallel version of eager handler.  The execute method comes from
// the serial method, and most other stuff comes from generic parallel.

class parEagerHandler : 
virtual public parSPHandler,
virtual public eagerHandler
{
 public:

  virtual ~parEagerHandler() { };

  void execute() { eagerHandler::execute(); };

 protected:

  void computeBound()  { parSPHandler::computeBound();         };
  void heuristic()     { parSPHandler::heuristic();            };
  bool shouldRelease() { return parSPHandler::shouldRelease(); };
  void releaseChild()  { parSPHandler::releaseChild();         };
  void insertChild()   { parSPHandler::insertChild();          };
  void eraseChild()    { parSPHandler::eraseChild();           };

  parallelBranchSub* pbp;

  void setToBoundCurrent() { pbp = pp;  bp = p; };
  void setToBoundChild()   { pbp = pc;  bp = c; };

  bool boundSubHandler();
  
};

} // namespace pebbl



utilib::PackBuffer& operator<<(utilib::PackBuffer& buf, 
			       const pebbl::branchSubId& id);

utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& buf, 
				 pebbl::branchSubId& id);

utilib::PackBuffer& operator<<(utilib::PackBuffer& buf, 
			       const pebbl::solutionIdentifier& solId);

utilib::UnPackBuffer& operator>>(utilib::UnPackBuffer& buf, 
				 pebbl::solutionIdentifier& solId);



namespace pebbl {

template <class ParBranchingType>
inline bool parallel_exec_test(int argc, char** argv, int nproc)
{
if (nproc > 1) return true;
for (int i=1; i<argc; i++) {
  if (strncmp(argv[i],"--help",6) == 0) return true;
  if (strncmp(argv[i],"--forceParallel",15) == 0) return true;
  }
return false;
}


/// Prepackaged parallel/serial main program

template <class B,class PB> int driver(int argc, char** argv)
{
  bool flag = true;

  try 
    {

      uMPI::init(&argc,&argv,MPI_COMM_WORLD);
      int nprocessors = uMPI::size;

      if (parallel_exec_test<parallelBranching>(argc,argv,nprocessors)) 
	{
	  CommonIO::begin();
	  CommonIO::setIOFlush(1);

	  PB instance;
	  flag = instance.setup(argc,argv);
	  if (flag)
	    {
	      instance.reset();
	      instance.printConfiguration();
	      instance.solve();
	    }

	  CommonIO::end();
	}
      else 
	flag = runSerial<B>(argc,argv);

      uMPI::done();
    }

  STD_CATCH(;)

  return !flag;
}

}


#endif


#endif
