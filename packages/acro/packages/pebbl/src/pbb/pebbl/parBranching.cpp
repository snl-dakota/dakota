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
// parBranching.cpp
//
// Parallel branching class code for PEBBL.  This module contains global
// class routines called in a synchronous fashion on all processors.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/mpiUtil.h>
#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {


int spToken::packedSize = -1;   // Negative forces token size computation 

CHUNK_ALLOCATOR_DEF(spToken,100);


  // The reset method now tries to do all the initialization it possibly can.

void parallelBranching::reset(bool /*VBflag*/)
{
  // Ramp-up control

  rampUpFlag     = false;
  rampUpMessages = 0;
  rampUpPool     = 0;
  rampUpBounds   = 0;
  rampUpTime     = 0;
  rampUpWCTime   = 0;
  crossoverFlag  = 0;

  // Current parallel subproblem

  if (currentParSP)
    {
      delete currentParSP;
      currentParSP = NULL;
    }

  // Various counters and flags
  
  searchInProgress = false;
  
  incumbentSource = MPI_ANY_SOURCE;

  needHubPruning = false;

  messages.reset();

  hubHandledCount = 0;
  spReleaseCount  = 0;
  spDeliverCount  = 0;
  loadBalSPCount  = 0;

  rebalanceSPCount = 0;
  rebalanceCount   = 0;
  myHubsRebalCount = 0;

  messagesReceivedThisProcessor = 0;
  totalMessages                 = 0;
  preprocessMessages            = 0;

  solOutputMessages = 0;
  solOutputTime     = 0;
  solOutputWCTime   = 0;

  solSerialCounter = 0;  // May have been incremented by reference solutions

  needReposMerge = false;

  // Parallel subproblem pools

  if (workerPool)
    {
      delete workerPool;
      workerPool = NULL;
    }

  if (hubPool)
    {
      delete hubPool;
      hubPool = NULL;
    }

  // Thread objects

  currentThread = NULL;

  if (worker)
    {
      delete worker;
      worker = NULL;
    }

  if (workerAux)
    {
      delete workerAux;
      workerAux = NULL;
    }

  if (incumbentCaster)
    {
      delete incumbentCaster;
      incumbentCaster = NULL;
    }

  if (incumbentSearcher)
    {
      delete incumbentSearcher;
      incumbentSearcher = NULL;
    }

  if (hub)
    {
      delete hub;
      hub = NULL;
    }

  if (loadBalancer)
    {
      delete loadBalancer;
      loadBalancer = NULL;
    }

  if (spServer)
    {
      delete spServer;
      spServer = NULL;
    }

  if (spReceiver)
    {
      delete spReceiver;
      spReceiver = NULL;
    }

  if (earlyOutputter)
    {
      delete earlyOutputter;
      earlyOutputter = NULL;
    }

  if (reposReceiver)
    {
      delete reposReceiver;
      reposReceiver = NULL;
    }

  if (reposMerger)
    {
      delete reposMerger;
      reposMerger = NULL;
    }

  if (llChainer)
    {
      delete llChainer;
      llChainer = NULL;
    }

  threadsList.clear();

  // Set up cluster tracking

  cluster.reset(uMPI::rank, uMPI::size, clusterSize, numClusters, 
		hubsDontWorkSize);

  // Initialize outgoing buffer objects
  // Hold two tokens and two acks
  // (but can expand).  Don't scavenge
  // until three are in use.

  workerOutQ.reset(3,hubMessageSize(2,2));  

  // Initialize stuff to do with scattering

  double workerLoadFac = 1 - hubLoadFac;
  
  DEBUGPR(20,ucout << "hubLoadFac=" << hubLoadFac 
	  << " workerLoadFac=" << workerLoadFac << endl);

  if (!parameter_initialized("targetScatterProb"))
    targetScatterProb = scatterFac*hubLoadFac;
  if (targetScatterProb < minScatterProb)
    targetScatterProb = minScatterProb;
  else if (targetScatterProb > maxScatterProb)
    targetScatterProb = maxScatterProb;

  releaseDecision.reset(lowLoadFac,
			highLoadFac,
			minScatterProb,
			targetScatterProb,  // Initialize scattering decision
			maxScatterProb);    // information.

  DEBUGPR(20,ucout << "releaseDecision: "; 
	  releaseDecision.write(ucout); ucout << endl);

  otherHubRelease.reset(clusterLowLoadRatio,
			clusterHighLoadRatio,
			minNonLocalScatterProb,
			targetNonLocalScatterProb,
			maxNonLocalScatterProb);

  DEBUGPR(20,ucout << "otherHubRelease: "; 
	  otherHubRelease.write(ucout); ucout << endl);

  // Initialize buffer objects

  deliverSPBuffers.reset(maxSPPacking,deliverSPTag,3);
  auxDeliverSPQ.reset(3,3*sizeof(int));

  // Don't scavenge Subproblem delivery buffers until
  // three are in use.  Starting size is set later.

  dispatchSPBuffers.reset(maxDispatchPacking,
			  forwardSPTag,
			  3*clusterSize);
  hubAuxBufferQ.reset(3*clusterSize,
		      workerAuxObj::computeBufferSize(this));

  // Initialize worker pool -- this must be done on all processors
  // now, because of the ramp-up phase.

  if ( depthFirst )
    workerPool =   // stack
      new doublyLinkedPool<parallelBranchSub,parLoadObject>(true);
  else if ( breadthFirst )
    workerPool =   // queue
      new doublyLinkedPool<parallelBranchSub,parLoadObject>(false);
  else             // heap
    workerPool = new heapPool<parallelBranchSub,parLoadObject,
      DynamicSPCompare<parallelBranchSub> >();

  workerPool->setGlobal(this);

  if (iAmWorker())
    {
      forceWorkerToRun = false;

      serverPool.setGlobal(this); 

      if (!iAmHub())
	{
	  spAckAddress.resize(maxSPAcks);
	  spAckBound.resize(maxSPAcks);
	  spAckCount = 0;
	  boundKnownToHub = sense*MAXDOUBLE;
	}

      releaseProbCount = 0;
      releaseTestCount = 0;
      workerOutBuffer  = workerOutQ.getFree();
#ifdef ACRO_VALIDATING
      hubMessageSeqNum = 0;
#endif
    }

  // This will set the busy fractions of all pure workers to 1, and, 
  // because the current time spent is zero, all combined worker-hubs 
  // to initialHubBusyFraction

  setHubBusyFractions();

  // Initialize hub structures

  if (iAmHub())
    {
      if ( depthFirst )
      	hubPool = 
	  new doublyLinkedPool<spToken,parLoadObject>(true);   //stack
      else if ( breadthFirst )
      	hubPool = 
	  new doublyLinkedPool<spToken,parLoadObject>(false);  //queue
      else
	hubPool = 
	  new heapPool<spToken,parLoadObject,DynamicSPCompare<spToken> >();

      hubPool->setGlobal(this); 
    
      wantLoadInformCount  = 0;
      sumWorkerReportCount = 0;
      hubDispatchCount     = 0;

      hubLastPrint = 0;

      wantLoadInform.resize(numWorkers());
      workerToInform.resize(numWorkers());
      workerLoadReport.resize(numWorkers());
      workerLoadEstimate.resize(numWorkers());
      workerReportCount.resize(numWorkers());
      workerRebalanceCount.resize(numWorkers());
      workerTransitPool.resize(numWorkers());
      workerHeapObj.resize(numWorkers());
      workerQHeapObj.resize(numWorkers());

      heapOfWorkers.clear();
      qHeapOfWorkers.clear();

      for (int w=0; w < numWorkers(); w++)
	{
	  wantLoadInform[w]       = false;
	  workerReportCount[w]    = 0;
	  workerRebalanceCount[w] = 0;

	  workerTransitPool[w].setGlobal(this);

	  workerHeapObj[w].global = this; 
	  workerHeapObj[w].w = w;

	  workerQHeapObj[w].global = this; 
	  workerQHeapObj[w].w = w;

	  // Don't form worker heaps here; do it after ramp-up
	}

      termCheckInProgress  = false;
      wantAnotherTermCheck = false;

    }

  // Parameter checks...

  // Quality balancing is not compatible with depth- or breadth-first search.

  if (qualityBalance && (depthFirst || breadthFirst))
    {
      qualityBalance = false;
      if (uMPI::iDoIO && parameter_initialized("qualityBalance"))
	{
	  CommonIO::end_tagging();
	  ucout << "\nQuality balancing inhibited because "
	    "search is not best-first.\n";
	  CommonIO::begin_tagging();
	}
    }

  if (rebalLoadFac < highLoadFac)
    if (uMPI::iDoIO)
      {
	CommonIO::end_tagging();
	ucout << "\n*** Warning *** rebalLoadFac=" << rebalLoadFac
	      << " is less than highLoadFac=" << highLoadFac << endl;
	CommonIO::begin_tagging();
      }

  if (minScatterProb > targetScatterProb)
    {
      if (uMPI::iDoIO)
	{
	  CommonIO::end_tagging();
	  ucout << "\nParameter minScatterProb=" << minScatterProb
		<< " is larger than targetScatterProb=" << targetScatterProb
		<< "; minScatterProb reset to " << targetScatterProb << endl;
	  CommonIO::begin_tagging();
	}
      minScatterProb = targetScatterProb;
    }

  if (maxScatterProb < targetScatterProb)
    {
      if (uMPI::iDoIO)
	{
	  CommonIO::end_tagging();
	  ucout << "\nParameter maxScatterProb=" << maxScatterProb
		<< " is less than targetScatterProb=" << targetScatterProb
		<< "; maxScatterProb reset to " << targetScatterProb << endl;
	  CommonIO::begin_tagging();
	}
      maxScatterProb = targetScatterProb;
    }

  if (logTransitions && (loadLogSeconds == 0))
    {
      if (uMPI::iDoIO)
	{
	  CommonIO::end_tagging();
	  ucout << "Parameter logTransitions meaningless if loadLogSeconds"
		<< " is zero; ignoring" << endl;
	  CommonIO::begin_tagging();
	}
      logTransitions = false;
    }


  // Prepare for checkpointing if need be

  checkpointsEnabled = (checkpointMinutes > 0);
  checkpointing      = false;
  checkpointNumber   = 0;
  restartCPNum       = 0;

  // Clear abort flag

  aborting    = false;
  abortReason = NULL;

  // Get rid of any incumbent

  resetIncumbent();

  // The code below hasn't been used for a long time; comment out -- JE

//   // If we are going to be dumping debug output to an internal buffer,
//   // create it.

//   if (outputCacheNeeded())
//     {
// #if !defined(ACRO_HAVE_SSTREAM)
//       outputCacheBuf = new char[outputCacheSize];
//       outputCache    = new ostrstream(outputCacheBuf,outputCacheSize,ios::out);
// #else
//       outputCache    = new ostringstream(outputCacheBuf,ios::out);
// #endif
//     }


  // This buffer size needs to be computed before setting up the threads

  solBufSize = computeSolBufferSize();

  // Enumeration stuff, if needed

  parallelSolCounter = 0;
  solsSent           = 0;

  if (enumerating)
    {
      rampUpSolQ.resize(rampUpSolQuantum);
      rampUpSolsPending = 0;

      reposArrayBufSize = 
	sizeof(int) + max(enumCount,0)*solutionIdentifier::packSize();

      DEBUGPR(100,ucout << "reposArrayBufSize=" << reposArrayBufSize << endl);

      solHashQ.reset(3,solBufSize);
      reposArrayQ.reset(1,reposArrayBufSize);
      newLastSolQ.reset(reposTreeRadix,
			sizeof(int)+solutionIdentifier::packSize());

      if (reposTree)
	delete reposTree;
      reposTree = new nAryTreeRememberParent(reposTreeRadix);
      reposChildren = reposTree->numChildren();

      childReposArray.resize(reposChildren);
      childMergeObject.resize(reposChildren);
      reposArrayHeap.clear();

      for(int child=0; child<reposChildren; child++)
	{
	  childMergeObject[child].setup(childReposArray[child]);
	  reposArrayHeap.add(childMergeObject[child]);
	}

      localMergeObject.setup(localReposArray);
      reposArrayHeap.add(localMergeObject);

      if (enumCount > 1)
	{
	  reposSkewSeconds    = (1 + reposMergeSkew)*reposMergeSeconds;
	  childArraysReceived = 0;
	  lastMergeTime       = WallClockSeconds();

	  gotChildArray.resize(reposChildren);
	  for(int child=0; child<reposChildren; child++)
	    gotChildArray[child] = false;
	}

      finalReposSyncDone = false;

      queuedSolutions = 0;

      if (enumFlowControl)
	{
	  nProcessorBits = bitWidth(uMPI::size - 1);
	  solJustSent.resize(nProcessorBits);
	  solSendQ.resize(nProcessorBits);
	  
	  for(size_type b=0; b<nProcessorBits; b++)
	    {
	      solJustSent[b] = NULL;
	      solSendQ[b].clear();
	    }

	  solAckQ.reset(nProcessorBits,sizeof(int));
	}
    }

  // Set up the scheduler

  IF_LOGGING_EVENTS(1,initEventLog(););

  DEBUGPR(300,ucout << "Setting up scheduler\n");
  initializeScheduler();

  // If a "pure" hub, debug everything at hubDebug level.

  if (iAmHub() && !iAmWorker() && (hubDebug > debug))
    setDebugLevelWithThreads(hubDebug);
  
  // Initialize load objects

  clusterLoad.init(this);
  globalLoad.init(this);
  lastWorkerReport.init(this);
  serverPool.load().init(this);
  workerPool->load().init(this,true,true);  // Pick up SP & message counts
  if (iAmHub())
    {
      // For hub pool, pick up message counts if not a worker
      hubPool->load().init(this,false,!iAmWorker()); 
      for (int w=0; w < numWorkers(); w++)
	{
	  workerLoadReport[w].init(this);
	  workerLoadEstimate[w].init(this);
	  workerTransitPool[w].load().init(this);
	}
    }

  // Set up handler -- may overwrite later if the user is supplying
  // his/her own handler, but so far I don't think anybody does that.

  if (handler)
    delete handler;
  if (lazyBounding)
    handler = new parLazyHandler;
  else if (eagerBounding)
    handler = new parEagerHandler;
  else
    handler = new parHybridHandler;

  handler->setPGlobal(this);

}


// Constructor for parallel branching class.  Sets threads to null.
// Most of the stuff that used to be here is now in reset().

parallelBranching::parallelBranching() :
  currentParSP(NULL),
  heapOfWorkers("Worker Tracking"),
  qHeapOfWorkers("Worker Quality Tracking"),
  reposArrayHeap("Repository Array")
{
  workerPool = NULL;
  hubPool    = NULL;

  worker            = NULL;
  workerAux         = NULL;
  incumbentCaster   = NULL;
  incumbentSearcher = NULL;
  hub               = NULL;
  loadBalancer      = NULL;
  spServer          = NULL;
  spReceiver        = NULL;
  earlyOutputter    = NULL;
  reposReceiver     = NULL;
  reposMerger       = NULL;
  llChainer         = NULL;

  workerOutBuffer   = NULL;

  reposTree = NULL;

  rememberPackSize = -1;

  handler = NULL;

  numRefSols = 0;

  broadcastTime         = 0;
  broadcastWCTime       = 0;
  broadcastMessageCount = 0;
}


// Helper to set debug levels

void parallelBranching::setDebugLevel(int level)
{
  setDebug(level);
  workerOutQ.setDebug(level);
  dispatchSPBuffers.setDebug(level);
  dispatchSPBuffers.bufferQ.setDebug(level);
  deliverSPBuffers.setDebug(level);
  deliverSPBuffers.bufferQ.setDebug(level);
  hubAuxBufferQ.setDebug(level);
}


void parallelBranching::setDebugLevelWithThreads(int level)
{
  setDebugLevel(level);
  if (worker)
    worker->setDebug(level);
  if (level > workerDebug)
    workerDebug = level;
  if (workerAux && (level > workerAuxDebug))
    workerAux->setDebug(level);
  if (incumbentCaster)
    incumbentCaster->setDebug(level);
  if (incumbentSearcher)
    incumbentSearcher->setDebug(level);
  if (hub)
    hub->setDebug(level);
  if (loadBalancer && (level > loadBalDebug))
    loadBalancer->setDebug(level);
  if (spServer)
    spServer->setDebug(level);
  if (spReceiver)
    spReceiver->setDebug(level);
  if (earlyOutputter)
    spReceiver->setDebug(level);
  if (reposReceiver)
    reposReceiver->setDebug(level);
  if (reposMerger)
    reposMerger->setDebug(level);
  if (llChainer)
    reposMerger->setDebug(level);
}


// Destructor for class.  Kill any threads still alive.  Zap auxiliary pools.

parallelBranching::~parallelBranching()
{
  DEBUGPR(200,ucout << "Destruction of parallelBranching object.\n");

  if (enumerating)
    delete reposTree;

  if (workerPool)
     delete workerPool;
  if (worker)
     delete worker;
  if (workerAux)
     delete workerAux;
  if (incumbentCaster)
     delete incumbentCaster; 
  if (incumbentSearcher)
     delete incumbentSearcher;
  if (hub)
     delete hub;
  if (loadBalancer)
     delete loadBalancer;
  if (spServer)
     delete spServer;
  if (spReceiver)
     delete spReceiver;
  if (earlyOutputter) 
    delete earlyOutputter;
  if (reposReceiver)
    delete reposReceiver;
  if (reposMerger)
    delete reposMerger;
  if (llChainer)
    delete llChainer;

  if (iAmWorker() && workerOutBuffer)
      delete workerOutBuffer;

  clearRegisteredSolutions();
  resetIncumbent();

  if (iAmHub() && hubPool)
    {
      DEBUGPR(200,ucout << "Hub parBranching destruction.\n");

      delete hubPool;

      hubAuxBufferQ.completeAll(workerTag); // Make sure all workers
                                            // get the message to shut down.
    }

//   if (outputCacheNeeded())
//     writeCachedOutput();

  DEBUGPR(200,ucout << "parallelBranching destruction complete.\n");
}


// To disambiguate the solve procedure when running in parallel

void parallelBranching::solve()
{
  double run_start = CPUSeconds();
  double run_WC_start = WallClockSeconds();

  parallelBranching::search();

  searchTime = CPUSeconds() - run_start;
  searchWCTime = WallClockSeconds() - run_WC_start;

  //
  // Final I/O
  //

  parallelBranching::printSolValue(ucout);
  if (printFullSolution)
    parallelBranching::printSolution("","\n",ucout);

  parallelBranching::solutionToFile();

  parallelBranching::printAllStatistics();
  ucout << endl;

  // Further use of this object might not require a broadcast
  broadcastTime         = 0;
  broadcastWCTime       = 0;
  broadcastMessageCount = 0;

  if (abortReason)
    ucout << "RUN ABORTED: " << abortReason << endl << endl;
}


// Does a parallel search.
  
double parallelBranching::parallelSearchFramework(parSPHandler* handler_)
{
  DEBUGPR(300,ucout << "parallelSearchFramework invoked\n");

  baseWallTime = WallClockSeconds();

  // Validation log file used to be opened here because preprocess
  // did some logging.  With the new ramp-up, we don't need that any more.
  // Now it should be done after we can tell if we restarted or not.

  outputInProgress = false;
  nextOutputTime   = baseWallTime + earlyOutputMinutes*60;
  lastSolValOutput = sense*MAXDOUBLE;

  checkpointTriggerTime = baseWallTime + checkpointMinutes*60;
  checkpointTotalTime   = 0;

  DEBUGPR(300,ucout << "Preprocessing\n");
  double startPreprocessTime = CPUSeconds();
  double startPreprocessWCTime = WallClockSeconds();
  LOG_EVENT(1,start,preprocLogState);
  preprocess();
  LOG_EVENT(1,end,preprocLogState);
  
  double endPreprocessTime = CPUSeconds();
  double endPreprocessWCTime = WallClockSeconds();
  preprocessTime = endPreprocessTime - startPreprocessTime;
  preprocessWCTime = endPreprocessWCTime - startPreprocessWCTime;

  // Replace handler if the user has their own
  
  if (handler_)
    {
      if (handler)
	delete handler;
      handler = handler_;
      handler->setPGlobal(this);
    }

  workerLastPrint     = 0;
  workerLastPrintTime = WallClockSeconds();

  baseTime = CPUSeconds();

  hubLastPrintTime    = WallClockSeconds();
  workerLastPrintTime = hubLastPrintTime;
  loadBalancer->surveyObject.lastPrintTime = hubLastPrintTime;

  DEBUGPR(150,ucout << "Starting search at baseTime=" << baseTime << endl);

  searchInProgress = true;

  if (reconfigure)
    restart = true;

  // Deal with checkpoints

  restarted         = false;
  writingCheckpoint = false;
  readingCheckpoint = false;

  if (restart)
    {
      readingCheckpoint = true;

      if (uMPI::rank == uMPI::ioProc)
	ucout << "Trying to restart from checkpoint\n\n";
      ucout.flush();

      restarted = restartFromCheckpoint();

      if (uMPI::rank == uMPI::ioProc)
	{
	  if (restarted)
	    ucout << "Checkpoint loaded successfully.\n";
	  else
	    ucout << "Warning: unable to read checkpoint files.  "
		  << "Reverting to normal ramp-up.\n\n";
	  ucout.flush();
	}

      readingCheckpoint = false;
    }

  // Now we know if we managed to restart from the checkpoint.
  // That will tell us to open the validation log files in either
  // out or append mode.

  vout = valLogFile();
  valLogInit();

  // If we didn't load from a checkpoint, do ramp-up.  Start load logs
  // if necessary.

  startLoadLogIfNeeded();

  if (!restarted)
    rampUpSearch();

  // If there were any problems left in the pool, do the real search.

  if (rampUpPool > 0)
    {
      prepareCPAbort();
      sched.execute();
    }

  // Clean up

  spToken::memClear();
  if (!handler_)
    {
      delete handler;
      handler = NULL;
    }

  searchInProgress = false;

  finishLoadLogIfNeeded();

  IF_LOGGING_EVENTS(1,finishEventLog(););

  if (checkpointNumber > 0)
    deleteCheckpointFile(checkpointNumber,uMPI::rank);

  if (validateLog)
    {
      if (uMPI::rank == 0)
	valLogFathomPrint();
      delete vout;
    }

  uMPI::barrier();   // Safety check to make sure that all processors think
                     // they're done before trying to call the destructor
                     // on any processor.

  return incumbentValue;
};


// Register a solution representation (giving up ownership)

size_type parallelBranching::registerFirstSolution(solution* referenceSolution)
{
  clearRegisteredSolutions();
  return registerSolution(referenceSolution);
}


size_type parallelBranching::registerSolution(solution* referenceSolution)
{
  size_type id          = numRefSols++;
  size_type currentSize = refSolArray.size();

  if (numRefSols > currentSize)
    refSolArray.resize(currentSize + refSolArrayQuantum);

  refSolArray[id] = referenceSolution;
  referenceSolution->typeId = id;
  DEBUGPR(5,ucout << "Registered " 
	  << referenceSolution->typeDescription()
	  << " as having type " << id 
	  << ", with buffer size " << referenceSolution->maxBufferSize()
	  << endl);
  return id;
}


// Clear the array of registered solutions

void parallelBranching::clearRegisteredSolutions()
{
  DEBUGPR(5,ucout << "Clearing registered solutions\n");
  for (size_type i=0; i<numRefSols; i++)
    delete refSolArray[i];
  numRefSols = 0;
}


// To compute the maximum buffer size we need to receive a solution.
// It's the maximum buffer size needed among all registed reference
// solution.

int parallelBranching::computeSolBufferSize()
{
  int toReturn = 0;
  for (size_type i=0; i<numRefSols; i++)
    {
      int s = refSolArray[i]->maxBufferSize();
      if (s > toReturn)
	toReturn = s;
    }
  DEBUGPR(5,ucout << "computeSolBufferSize yields " << toReturn 
	  << " numRefSols=" << numRefSols << endl);
  return toReturn;
}


// Unpack a solution by using typeId to index into the reference
// solution array.

solution* parallelBranching::unpackSolution(UnPackBuffer& inBuf)
{
  int typeId = -1;
  inBuf >> typeId;
  if ((typeId < 0) || (typeId >= numRefSols))
    EXCEPTION_MNGR(runtime_error,"Unpacked solution type id "
		   << typeId << " is out of range 0.." << numRefSols-1);
  solution* sol = refSolArray[typeId]->blankClone();
  sol->unpack(inBuf);
  return sol;
}


// To create the scheduler object we need.

void parallelBranching::initializeScheduler()
{
  sched.reset();
  sched.setDebug(schedulerDebug);
  highPriorityGroup = sched.add(new ThreadQueue(round_robin));
  baseGroup         = sched.add(new ThreadQueue(time_weighted_priority));
  placeTasks();

  ListItem<parBranchingThreadObj*> *l1;
  for(l1 = threadsList.head(); l1;  l1=threadsList.next(l1))
      if (l1->data()->active) l1->data()->setDebugLevel(debug);
  
}


//  Places tasks into the scheduler object.  NOTE: on every processor,
//  we create all threads that might be used on any processor, but
//  don't place them all in the scheduler.  If processors created
//  different subsets of the possible threads, there could be message tag
//  mismatches.

void parallelBranching::placeTasks()
{
  worker = new workerThreadObj(this);
  placeTask(worker,iAmWorker(),baseGroup);

  incumbentThreadExists = hasParallelIncumbentHeuristic() &&
                          useIncumbentThread;
  incumbentSearcher = createIncumbentSearchThread();
  placeTask(incumbentSearcher,
	    iAmWorker() && incumbentThreadExists,
	    baseGroup);

  hub = new hubObj(this);
  placeTask(hub,iAmHub(),highPriorityGroup);

  incumbentCaster = new incumbCastObj(this);
  placeTask(incumbentCaster,true,highPriorityGroup);

  spReceiver = new spReceiverObj(this);
  placeTask(spReceiver,iAmWorker(),highPriorityGroup);

  // Now that we know the spReceiver buffer size, set up the buffer size 
  // tracking for other processors (it will be the same on all workers)

  if (iAmWorker())
    {
      knownBufferSize.resize(uMPI::size);
      for (int i=0; i<uMPI::rank; i++)
	knownBufferSize[i] = spReceiver->sizeOfBuffer();
      DEBUGPR(100,ucout <<"Initialized knownBufferSize to " 
	      << spReceiveBuf << '/' << knownBufferSize[0] << endl);
    }

  // Back to creating threads...

  spServer = new spServerObj(this);
  placeTask(spServer,iAmWorker(),highPriorityGroup);

  loadBalancer = new loadBalObj(this);
  placeTask(loadBalancer,iAmHub(),highPriorityGroup);
  
  workerAux = new workerAuxObj(this);
  placeTask(workerAux,iAmWorker() && !iAmHub(),highPriorityGroup);

  if (earlyOutputMinutes > 0)
    {
      earlyOutputter = new earlyOutputObj(this);
      placeTask(earlyOutputter,true,highPriorityGroup);
    }

  if (enumerating)
    {
      reposReceiver = new reposRecvObj(this);
      placeTask(reposReceiver,true,highPriorityGroup);
      if (enumCount > 1)
	{
	  reposMerger = new reposMergeObj(this);
	  placeTask(reposMerger,true,highPriorityGroup);
	}
    }

  if ((loadLogSeconds > 0) && (loadLogWriteSeconds > 0) && (uMPI::size > 1))
    {
      llChainer = new llChainObj(this);
      placeTask(llChainer,true,highPriorityGroup);
    }
}


void parallelBranching::placeTask(parBranchingThreadObj* thread,
				  int cond,
				  int group)
{
  DEBUGPR(200,ucout << "parallelBranching::placeTask" << endl);
  DEBUGPR(300,thread->dump(ucout));
  thread->active = cond;
  if (cond)
    {
      DEBUGPRP(100,ucout << "Placing task " << thread->name);
      thread->startup();
      DEBUGPRP(100,ucout << " (started)");
      sched.insert(thread,group,0,thread->name);
      DEBUGPR(100,ucout << " (inserted)" << endl);
    }
  threadsList.add(thread); 
}


parallelBranchSub* parallelBranching::makeParRoot()
{
  DEBUGPR(75,ucout << "parallelBranching::makeRoot invoked.\n");
  parallelBranchSub* root = blankParallelSub();
  root->setRoot();
  root->id.creatingProcessor = 0;
  DEBUGPR(10,ucout << "Created root problem " << root << '\n');
  if (valLogOutput())
    root->valLogCreatePrint(0);
  return root;
}


// Standard setup pattern for parallel branching

bool parallelBranching::setup(int& argc, char**& argv)
{
  int flag = false;

  // The I/O processor reads the problem as in serial.
  // The others just initialize timers and set up parameters.

  if (uMPI::iDoIO)
    flag = (int)branching::setup(argc,argv);
  else
    {
      startTime = CPUSeconds();
      startWall = WallClockSeconds();
      // Don't check for errors here; the I/O processor will do that.
      processParameters(argc,argv,min_num_required_args);
      set_parameters(plist,false);
    }

  if (stallForDebug) 
    {
      ucout << "MPI Rank " << uMPI::rank << " = " << getpid() << std::endl 
	    << utilib::Flush;
      uMPI::barrier();
      if (uMPI::iDoIO) 
	{
	  ucout << "Type <Enter> to continue: " << Flush;
	  char staller;
	  cin.get(staller);
	}
      uMPI::barrier();
    }

  // The debugSeqDigits parameter needs extra processing 

  CommonIO::numDigits = debugSeqDigits; 

  // Broadcast the read-in success flag to everbody

  uMPI::broadcast(&flag,1,MPI_INT,uMPI::ioProc);

  // If things worked, broadcast the problem to everybody

  if (flag)
    broadcastProblem();

  return (bool)flag;
}


// To broadcast a problem to all processors

void parallelBranching::broadcastProblem()
{
  if (uMPI::size == 1)
    return;

  double startBroadcastTime = CPUSeconds();
  double startBroadcastWCTime = WallClockSeconds();

  if(uMPI::iDoIO)                   // If we are the special IO processor
    {
      PackBuffer outBuf(8192);             // Pack everything into a buffer.
      packAll(outBuf);
      int probSize = outBuf.size();        // Figure out length.
      DEBUGPR(70,ucout << "Broadcast size is " << probSize << " bytes.\n");
      uMPI::broadcast(&probSize,          // Broadcast length.
		      1,
		      MPI_INT,
		      uMPI::ioProc);
      uMPI::broadcast((void*) outBuf.buf(), // Now broadcast buffer itself.
		      probSize,
		      MPI_PACKED,
		      uMPI::ioProc);
    }

  else   // On the other processors, we receive the same information...

    {
      int probSize;                          // Get length of buffer
      uMPI::broadcast(&probSize,             // we're going to get.
		      1,
		      MPI_INT,
		      uMPI::ioProc);
      DEBUGPR(70,ucout << "Received broadcast size is " << probSize << 
	      " bytes.\n");
      UnPackBuffer inBuf(probSize);          // Create a big enough
      inBuf.reset(probSize);                 // temporary buffer.
      uMPI::broadcast((void *) inBuf.buf(),  // Get the data...
		      probSize,
		      MPI_PACKED,
		      uMPI::ioProc);
      DEBUGPR(100,ucout << "Broadcast received.\n");
      unpackAll(inBuf);                      // ... and unpack it.
      DEBUGPR(100,ucout << "Unpack seems successful.\n");
    }

  broadcastTime   += CPUSeconds() - startBroadcastTime;
  broadcastWCTime += WallClockSeconds() - startBroadcastWCTime;

  broadcastMessageCount += 2*(uMPI::rank > 0);

  DEBUGPR(70,ucout << "Problem broadcast done.\n");

  // Compute and remember size of packed subproblems.
  rememberPackSize = spAllPackSize();
  DEBUGPR(50,ucout << "Packed subproblem size is " 
	  << rememberPackSize << ".\n");
  deliverSPBuffers.bufferQ.setStartingBufferSize(rememberPackSize);
}


void parallelBranching::rampUpIncumbentSync()
{
  DEBUGPR(100,ucout << "Entered rampUpIncumbentSync(): value=" 
	  << incumbentValue << ", source=" << incumbentSource << endl);
  MPI_Op reduceOp = MPI_MIN;    // For minimization
  if (sense == maximization)    // Change if maximization
    reduceOp = MPI_MAX;
  double bestIncumbent = sense*MAXDOUBLE;
  uMPI::reduceCast(&incumbentValue,&bestIncumbent,1,MPI_DOUBLE,reduceOp);
  DEBUGPR(100,ucout << "Got value " << bestIncumbent << endl);

  int sourceRank = uMPI::size;
  if (incumbentValue == bestIncumbent)
    sourceRank = incumbentSource;
  int lowestRank = 0;
  uMPI::reduceCast(&sourceRank,&lowestRank,1,MPI_INT,MPI_MIN);
  DEBUGPR(100,ucout << "Got source " << lowestRank << endl);

  if (bestIncumbent != incumbentValue)
    {
      needPruning = true;
      newIncumbentEffect(bestIncumbent);
      if ((uMPI::rank == 0) && trackIncumbent)
	ucout << "New incumbent found: value=" << bestIncumbent
	      << ", source=" << lowestRank
	      << ", time=" << CPUSeconds() - baseTime 
	      << (rampingUp() ? " (ramp-up)" : "") << endl;
    }
  
  incumbentValue  = bestIncumbent;
  incumbentSource = lowestRank;

  DEBUGPR(100,ucout << "Leaving rampUpIncumbentSync(): value=" 
	  << incumbentValue << ", source=" << incumbentSource << endl);

  if (uMPI::rank > 0)
    rampUpMessages += 4;

  // If enumerating, then things are much hairier

  if (enumerating)
    rampUpRepositorySync();
}


void parallelBranching::rampUpSearch()
{
  // Decide that we're getting started

  double startRampUpTime = CPUSeconds();
  double startRampUpWCTime = WallClockSeconds();
  LOG_EVENT(1,start,rampUpLogState);
  rampUpFlag = true;
  DEBUGPR(1,ucout << "Starting ramp-up phase\n");

  foundSolution(initialGuess(),synchronous);
  rampUpIncumbentSync();

  // Insert the root on all processors, and synchronously chomp
  // through subproblems until we've got nothing to do (lucky!)
  // or the continueRampUp() method tells use to stop.

  workerPool->insert(makeParRoot());

  while((spCount() > 0) && keepRampingUp())
    {
      processSubproblem();
      if (uMPI::rank == 0)
	{
	  loadObject lo = updatedPLoad();
	  statusPrint(workerLastPrint,workerLastPrintTime,lo,"r");
	  recordLoadLogIfNeeded();
	}
    }

  DEBUGPR(1,ucout << "Synchronous ramp-up loop complete.\n");

  rampUpCleanUp();  // Derived codes can do any clean up they need to

  if (haveCurrentSP())
    unloadCurrentSPtoPool();

  rampUpIncumbentSync();

  double aggBoundAtCrossover = workerPool->updatedLoad().aggregateBound;
  
  // Now pick through the worker pools (all identical, WE HOPE)
  // and place all suproblems destined for this particular processor
  // in a temporary pool.  Also adjust all load tracking objects.
  // The temporary pool is a stack unless the breadthFirst() parameter
  // is set, in which case it's a queue.

  DEBUGPR(10,ucout << "Starting ramp-up crossover, bounded="
	  << subCount[bounded] << endl);
  DEBUGPR(10,statusLine(workerPool->updatedLoad(),"X"));
  
  crossoverFlag = true;
  rampUpPool = workerPool->size();

  doublyLinkedPool<parallelBranchSub,loadObject> tempPool(!breadthFirst);
  tempPool.load().init(this);

  globalLoad.reset();
  clusterLoad.reset();
  if (iAmHub())
    for (int w=0; w<numWorkers(); w++)
      workerLoadEstimate[w].reset();

  // Determine the "skip factor" -- the smallest integer >= the number
  // of workers per cluster which is relatively prime to the total
  // number of workers.  Compute this in parallel by trying all values
  // starting at the cluster size.  Values not relatively prime are
  // changed into totalWorkers+1, which will be equivalent to 1.  The
  // location of each suproblem is determined by adding the skipFactor
  // to the rank of the previous subproblem, modulo the total number
  // of workers.

  int trialSkip = typicalClusterSize() - typicalHubsPure() + uMPI::rank;
  if (gcd(trialSkip,totalWorkers()) != 1)  // If not relatively prime, 
    trialSkip = totalWorkers() + 1;        // use a big number
  int skipFactor = -1;
  uMPI::reduceCast(&trialSkip,&skipFactor,1,MPI_INT,MPI_MIN); // Find smallest
  rampUpMessages += (uMPI::rank != uMPI::ioProc);
  DEBUGPR(2,ucout << "Crossover skip factor is " << skipFactor << endl);

  // Rank of current worker within all workers.  Start with 0, then
  // increase by skipFactor mod the total number of workers

  int workerRank = 0;

  while(workerPool->size() > 0)
    {
      // Get a subproblem from the pool and count it in the global load

      parallelBranchSub* sp = workerPool->remove();
      globalLoad += *sp;
      DEBUGPR(100,ucout << "Ramp-up crossover: " << sp 
	      << ", global load " << globalLoad << endl);

      // Figure out which processor will be the worker for this SP.
      // workerRank is the rank in the set of workers, and 
      // procRank is this worker's rank in the set of processors.

      int procRank = cluster.globalFollower(workerRank);
      DEBUGPR(100,ucout << "For processor " << procRank << endl);
      
      // Figure out if the processor belongs to this cluster, and if so
      // count the subproblem in the clusterLoad and perhaps the hub
      // information

      if (whichCluster(procRank) == whichCluster(uMPI::rank))
	{
	  clusterLoad += *sp;
	  DEBUGPR(100,ucout << "My cluster, load " << clusterLoad << endl);
	  if (iAmHub())
	    {
	      int workerClusterRank = whichWorker(procRank);
	      DEBUGPR(100,ucout << "Worker rank in cluster = " 
		      << workerClusterRank << endl);
	      workerLoadReport[workerClusterRank]   += *sp;
	      workerLoadEstimate[workerClusterRank] += *sp;
	    }
	}

      // If this is the worker processor owning the subproblem,
      // save it in the temporary pool; otherwise, discard.

      if (procRank == uMPI::rank)
	{
	  DEBUGPR(100,ucout << "Keeping...\n");
	  tempPool.insert(sp);
	}
      else
	{
	  DEBUGPR(100,ucout << "Eliminating.\n");
	  sp->recycle();
	}

      // Update worker rank for next problem (if we have one...)
      workerRank = (workerRank + skipFactor) % totalWorkers();
    }

  // Move any saved problems back into the worker pool.

  DEBUGPR(100,ucout << "Transferring problems to worker pool...\n");

  while(tempPool.size() > 0)
    workerPool->insert(tempPool.remove());

  DEBUGPR(100,ucout << "Done moving problems to worker pool...\n");

  crossoverFlag = false;      // Crossover is complete

  // Adjust subproblem counters to avoid multiple counting, and
  // start serial numbers from 0 on processors other than [0].
  // Also, save subproblem counters

  rampUpBounds = subCount[bounded];

  if (uMPI::rank > 0)
    {
      probCounter = 0;
      for(int i=0; i<numStates; i++)
	subCount[i] = 0;
    }
  DEBUGPR(10,ucout << "bounded=" << subCount[bounded] << endl);

  // Set initial loads correctly, and possibly dump them.
  // Also, set up hub worker heaps correctly.

  clusterLoad.update();
  clusterLoad.senseBusy();
  globalLoad.update();
  globalLoad.senseBusy();
  adjustReposStats(globalLoad);
  DEBUGPR(20,clusterLoad.dump(ucout,"clusterLoad"));
  DEBUGPR(20,globalLoad.dump(ucout,"globalLoad"));
  globalLoad.boundedSPs = rampUpBounds;
  clusterLoad.boundedSPs = rampUpBounds;

  if (iAmHub())
    {
      for (int w=0; w<numWorkers(); w++)
	{
	  workerLoadEstimate[w].update();
	  heapOfWorkers.add(workerHeapObj[w]);
	  qHeapOfWorkers.add(workerQHeapObj[w]);
	}
      if (iAmFirstHub())
	DEBUGPR(1,statusLine(globalLoad,"x"));
    }

  // Set printing times.  These are only really meaningful on processor 0, 
  // and we might get some output immediately on other processors if
  // hubsPrintStatus or workersPrintStatus are set.

  hubLastPrintTime = workerLastPrintTime;
  hubLastPrint     = workerLastPrint;
  
  loadBalancer->surveyObject.lastPrintTime = workerLastPrintTime;
  loadBalancer->surveyObject.lastPrint     = workerLastPrint;

  // Declare ramp-up to be over and tidy up.

  DEBUGPR(1,ucout << "Ramp-up phase complete, bounded=" << rampUpBounds
	  << " global-pool=" << rampUpPool 
	  << " local-pool=" << workerPool->size() << endl);

  rampUpFlag = false;
  LOG_EVENT(1,end,rampUpLogState);
  rampUpTime = CPUSeconds() - startRampUpTime;
  rampUpWCTime = WallClockSeconds() - startRampUpWCTime;

  // If load logging, unconditionally record state at end of ramp-up.

  if (loadLogSeconds > 0)
    recordLoadLogData(WallClockSeconds());

  // Abort here if rampUpOnly option is set

  if (rampUpOnly)
    postRampUpAbort(aggBoundAtCrossover);

}


// JE -- this was looking identical to the code in branching::; try removing
// void parallelBranching::solve()
// {
//   double run_start = CPUSeconds();
//   search();
//   searchTime = CPUSeconds() - run_start;

//   //
//   // Final I/O
//   //

//   ucout << endl;
//   printAllStatistics();
//   ucout << endl;

//   solutionToFile();

//   if (abortReason)
//     ucout << "RUN ABORTED: " << abortReason << endl << endl;
// }




// Called when it's time to abort

void parallelBranching::setupAbort()
{
  // If we already know we're aborting, don't do anything
  if (aborting)
    return;

  aborting = true;

  if (iAmHub())
    {
      if (clusterNumber() == 0)
	ucout << "\nStarting to abort (" << abortReason << ")\n";
      alertWorkers(startAbortSignal);
    }
}


// This makes sure to delete all subproblems and tokens during an
// abort.  This way, all subproblems will be destructed before various
// other things (otherwise you can get seg faults on exit).

void parallelBranching::clearAllSPsForAbort()
{
  // Clear the hub pool.  If we are aborting, we should be in 
  // quiesence, and the worker transit pool should already be empty.

  if (iAmHub())
    hubPool->clear();

  // Clear the worker-related pools.

  if (iAmWorker())
    {
      if (haveCurrentSP())
	unloadCurrentSPtoPool();
      workerPool->clear();
      serverPool.clear();
    }
}


spToken::spToken(parallelBranchSub* sp,int childNum,int childCount) :
  coreSPInfo(*sp)
{
  if (!sp->canTokenize())
    EXCEPTION_MNGR(runtime_error, "Attempt to make a token from a subproblem "
		"for which canTokenize() == false");
  spProcessor         = uMPI::rank;
  whichChild          = childNum;
  memAddress          = sp;
  childrenRepresented = childCount;
  sp->tokenCount     += childCount;
  globalPtr           = sp->bGlobal();
  depth		      = -1;			// dummy initialization
};



parLoadObject parallelBranching::updatedPLoad()
{
  parLoadObject l = workerPool->updatedLoad();
  if (!rampingUp())
    l.serverBound = serverPool.updatedLoad().aggregateBound;
  else 
    l.serverBound = sense*MAXDOUBLE;

  // Try this 5/31/2013
  l.senseClusterBusy();

  DEBUGPR(200,ucout << "updatedPLoad: pool load is " << l << endl);
  if (haveCurrentSP())
    l.addLoad(currentParSP->bound,1);

  DEBUGPR(200,ucout << "updatedPLoad: calculated " << l << endl);

  return l;
};



void spToken::packEmpty(PackBuffer& buf)
{
spToken dummy(NULL);
#ifdef ACRO_VALIDATING
  //
  // WEH - initialize data to avoid error warnings in purify
  //
  dummy.bound = -999.0;
  dummy.integralityMeasure = -999.0;
  dummy.state = dead;
  dummy.id.setEmpty();
  dummy.depth = -999;
  dummy.childrenRepresented = -999;
  dummy.spProcessor = -999;
  dummy.whichChild = -999;
  dummy.memAddress = NULL;
#endif
  dummy.pack(buf);
}


void parallelBranching::setHubTracking(parLoadObject& load)
{
  if (lastCluster())
    {
      setHubTimeBlock(load.lastHubTrack);
      load.hubTrack.reset();
    }
  else
    {
      setHubTimeBlock(load.hubTrack);
      load.lastHubTrack.reset();
    }
}


void parallelBranching::setHubTimeBlock(pbTimeTrackBlock& tt)
{
  tt.numerator   = hub->usefulTime();
  tt.denominator = CPUSeconds() - baseTime;
}


double parallelBranching::hubBusyFractionCalc(bool separated, 
					      int n, 
					      pbTimeTrackBlock& track)
{
  if (separated)       // If hub is "pure"...
    return 1;          // it spends all its time being a hub

  double pad         = n*initialHubSeconds;
  double denominator = track.denominator + pad;

  if (denominator == 0)
    return initialHubBusyFraction;

  double numerator = track.numerator + initialHubBusyFraction*pad;
  double ratio     = numerator/denominator;

  return std::min(ratio*hubBusyInflate,
		  ratio + hubBusyAdjust*(1 - ratio));
}


void parallelBranching::setHubBusyFractions()
{
  int nminus1 = numHubs() - 1;

  typicalHubBusyFraction = hubBusyFractionCalc(typicalHubsPure(),
					       nminus1,
					       globalLoad.hubTrack);

  lastHubBusyFraction = hubBusyFractionCalc(lastHubPure(),
					    1,
					    globalLoad.lastHubTrack);

  adjustedWorkerCount = uMPI::size - nminus1*typicalHubBusyFraction
                                   - lastHubBusyFraction;

  DEBUGPR(100,ucout << "typicalHubBusyFraction=" << typicalHubBusyFraction
	  << " lastHubBusyFraction=" << lastHubBusyFraction
	  << " adjustedWorkerCount=" << adjustedWorkerCount << '\n');

  if (iAmWorker())
    setMyWorkerTimeFrac();
}


double parallelBranching::workerTimeFrac(int workerIndex)
{
  if ((workerIndex > 0) || pureHubThisCluster())
    return 1.0;
  if (lastCluster())
    return 1 - lastHubBusyFraction;
  return 1 - typicalHubBusyFraction;
}


int parallelBranching::lowWorkerCount()
{
  int toReturn = 0;
  double targetCount = floor((1-hubLoadFac)*globalLoad.count()) 
                          / adjustedWorkerCount;
  if (targetCount > toReturn)
    toReturn = targetCount;
  double timeElapsed = WallClockSeconds() - baseWallTime;
  double rate        = globalLoad.boundedSPs/(timeElapsed*adjustedWorkerCount);
  double starveCount = floor(starveSeconds*rate);
  if (starveCount > toReturn)
    toReturn = starveCount;
  if (toReturn < workerKeepCount)    // Workers should never give up
    toReturn = workerKeepCount;      // up problems below workerKeepCount
  return toReturn;
}


loadObject parallelBranching::load()
{
  parLoadObject l = workerPool->load();
  if (haveCurrentSP())
    l += *currentParSP;

  return l;
}


// Cancel all outstanding receive requests so we don't hang
// on exit.

void parallelBranching::cancelComm()
{
ListItem<parBranchingThreadObj*>* tptr;
for(tptr=threadsList.head(); tptr; tptr=threadsList.next(tptr) )
  tptr->data()->cancelComm();

workerOutQ.clear();
deliverSPBuffers.clear();
auxDeliverSPQ.clear();
dispatchSPBuffers.clear();
hubAuxBufferQ.clear();
}


// Call this routine if you want to terminate cleanly during the middle of 
// a parallel search.  Hopefully there will be no hangs or error 
// messages.

void parallelBranching::cleanAbort()
{
  // Clean up validation logs.

  if (validateLog)
    delete vout;

  // Make sure we don't get a memory leak message (the "false" does that)

  spToken::memClear(false);

  // Cancel all outstanding receive requests so we don't hang
  // on exit.

  cancelComm();

  // Synchronize, terminate MPI, and try for a clean exit.

  uMPI::barrier();
  uMPI::done();
  exit(0);
}



} // namespace pebbl

#endif

