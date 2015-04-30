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
// pbWorker.cpp
//
// Parallel branching class code for PEBBL -- stuff associated with worker
// thread.
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/mpiUtil.h>
#include <utilib/seconds.h>
#include <pebbl/parBranching.h>
#include <pebbl/packPointer.h>
#include <pebbl/gRandom.h>

using namespace std;

namespace pebbl {


  // Simplification of worker debug output.

#define WORKERDEBUG(level,action)  if (workerDebug >= level) { action; }


// This overloads the signalIncumbent operation and makes it also
// initiate a broadcast of the new information.

void parallelBranching::signalIncumbent()
{
  LOG_EVENT(1,start,foundIncLogState);

  branching::signalIncumbent();
  incumbentSource = uMPI::rank;

  WORKERDEBUG(1,ucout << "New incumbent found: value=" << incumbentValue 
	      << ", source=" << incumbentSource 
	      << ", time=" << CPUSeconds() - baseTime << endl);

  if (iAmHub() && !rampingUp())
    {
      setToInformAll();
      needHubPruning = true;
    }

  if (!rampingUp())
    incumbentCaster->initiateBroadcast();

  LOG_EVENT(1,end,foundIncLogState);

  if (trackIncumbent)
    ucout << "New incumbent found: value=" << incumbentValue 
	  << ", source=" << incumbentSource 
	  << ", time=" << CPUSeconds() - baseTime 
	  << (rampingUp() ? " (ramp-up)" : "") << endl;

};


// Code for chomping through subproblem.  Called from workerExecute,
// but also from ramp-up.

void parallelBranching::processSubproblem()
{
 if (!haveCurrentSP())
   {
     if (workerPool->size() == 0)
       return;
     loadCurrentParSP(workerPool->remove());
   }

 WORKERDEBUG(10,ucout << "Looking at " << currentSP << '\n');
 if (!rampingUp())
   {
     WORKERDEBUG(20,ucout << "Load is " << load() << '\n');
   }

 if (currentParSP->canFathom())
   eraseCurrentSP();
 else 
   {
     handler->execute();

     if (haveCurrentSP() && !currentParSP->forceStayCurrent())
       unloadCurrentSPtoPool();

     pruneIfNeeded();
   }
} 


// Code for doing the search in the worker threads goes here...
// Note: this assumes that the granularity of the control parameter
// is on the order of 1.0.  Returns 0 normally, 1 if termination should occur.

void parallelBranching::workerExecute(double* controlParam)
{
#ifdef ACRO_VALIDATING
  double startTime = CPUSeconds();
#endif
  WORKERDEBUG(40,ucout << "Worker slice, control "
	      << *controlParam << ", pool size " 
	      << workerPool->size() << '\n');
  WORKERDEBUG(20,ucout << "Incumbent is " << incumbentValue << '\n');
  forceWorkerToRun = false;
  pruneIfNeeded();
  workLeft = *controlParam;
  workUsed = 0;

  LOG_EVENT(1,start,workerLogState);

  if (!suspending())
    {
      do
	{
	  processSubproblem();
	  workerPool->load().update();
	  if (workersPrintStatus)
	    {
	      statusPrint(workerLastPrint,
			  workerLastPrintTime,
			  workerPool->load(),
			  "w");
	    }
	  recordLoadLogIfNeeded();
	}
      while((spCount() > 0) && (workLeft >= 0.5));
    }
  
  *controlParam = workUsed;

  int didARebalance = rebalanceIfNeeded();
 
  if (iAmHub())
    activateHub(worker);
  else if (!didARebalance && shouldCommunicateWithHub(localScatterQuantum))
    workerCommunicateWithHub();

  LOG_EVENT(1,end,workerLogState);
  
  WORKERDEBUG(40,ucout << "Worker slice done at " << 
	      CPUSeconds() - startTime << " seconds, control " <<
	      *controlParam << ", pool size " << workerPool->size() << '\n');



  return;
}


// These guys load and unload problems

void parallelBranching::loadCurrentParSP(parallelBranchSub* p)
{
  loadCurrentSP(p);
  currentParSP = p;
}


void parallelBranching::unloadCurrentSP()
{
  branching::unloadCurrentSP();
  currentParSP = NULL;
}


void parallelBranching::eraseCurrentSP()
{
  parallelBranchSub* p = currentParSP;
  unloadCurrentSP();
  workerDisposeSP(p);  // This will either delete or keep in server pool.
}


// Parallel handler stuff

void parSPHandler::heuristic()
{
  if (rampingUp())
    pp->rampUpIncumbentHeuristic();
  else
    {
      pp->quickIncumbentHeuristic();
      if (pGlobal->incumbentThreadExists)
	pp->feedToIncumbentThread();
    }
}

// This is the bounding sub-handler for parallel eager search.
// It returns true if the subproblem being bounded is no longer needed.

bool parEagerHandler::boundSubHandler()
{
  if (pbp->canFathom())
    return true;
  boundOperation(pbp);
  if (pbp->canFathom())
    return true;
  if (pbp->state == beingBounded)
    return false;
  if (rampingUp())
    pbp->rampUpIncumbentHeuristic();
  else 
    {
      pbp->quickIncumbentHeuristic();
      if (pGlobal->haveIncumbentHeuristic())
	pbp->feedToIncumbentThread();
    }
  if((pbp->state == bounded) && shouldRelease())
    {
      releaseSP(pbp,self);
      return true;
    }
  return false;
}


// Compute bound procedure augments the serial one by keeping
// track of the amount of work done.

void parallelBranching::computeBound(parallelBranchSub* p)
{
  LOG_EVENT(1,start,boundLogState);
  double control = 1;
  if (!rampingUp())
    control = workLeft;
  p->computeBound(&control);
#ifdef ACRO_VALIDATING
  if (boundWasteTime > 0)
    {
      double targetTime = CPUSeconds() + boundWasteTime;
      while (CPUSeconds() < targetTime) { };
    }
#endif
  if (!rampingUp())
    {
      workLeft -= control;
      workUsed += control;
      WORKERDEBUG(100,ucout << "Work done = " << control << ", workLeft = "
		  << workLeft << ", workUsed = " << workUsed << ".\n");
    }
  LOG_EVENT(1,end,boundLogState);
}


//  To make parallel children.  With a little cleverness this could
//  probably be altered not to duplicate the code of the serial version.

parallelBranchSub* parallelBranchSub::parallelChild(int whichChild)
{
  if (state != separated)
    EXCEPTION_MNGR(runtime_error, "Attempt to extract child of " <<
		   stateString(state) << " subproblem");

  LOG_EVENTX(pGlobal(),4,start,pGlobal()->pCreateLogState);

  whichChild = chooseChild(whichChild);  // If whichChild==anyChild, fix it.
  if (--childrenLeft < 0)
    EXCEPTION_MNGR(runtime_error,
		   "Tried to make too many children of a subproblem");

  parallelBranchSub* c = makeParallelChild(whichChild);
  if (rampingUp())
    c->id.creatingProcessor = 0;

  LOG_EVENTX(pGlobal(),4,end,pGlobal()->pCreateLogState);

  DEBUGPR(10,ucout 
	  << "Extracted (parallel) child " << c << ", child "
	  << totalChildren - childrenLeft << " of " << this << '\n');

  if (valLogOutput())
    c->valLogCreatePrint(this);

  return c;
}


void parallelBranching::deliverSP(branchSubId& id,
				  int whichChild,
				  double bound,
				  parallelBranchSub* p,
				  int destProcessor,
				  spToken* hubAddress)
{
  WORKERDEBUG(100,ucout << "Delivery of " << p << " at " 
	      << (void *) hubAddress 
	      << " requested to " << destProcessor << '\n');
  if ((checkTokensMatch > 0) && !(id == p->id))
    EXCEPTION_MNGR(runtime_error,"Token mismatch error.");
  if (whichChild == self)
    {
      if (destProcessor == uMPI::rank)
	{                               // Token returns to creating processor.
	  serverPool.remove(p);
	  p->tokenCount = 0;            // The token is no longer on the loose.
	  WORKERDEBUG(100,ucout << "'Self' token delivered locally.\n");
	  addToWorkerPool(p,bound,hubAddress);
	}
      else
	{
	  PackBuffer* outBuf = startPackingSP(bound,destProcessor,hubAddress);
	  p->packProblem(*outBuf);
	  finishDeliverSP(outBuf,destProcessor);
	  WORKERDEBUG(100,ucout << "'Self' token sent to [" 
		      << destProcessor << "].\n");
	  serverPool.kill(p);
	}
    }
  else
    {
      if (destProcessor == uMPI::rank)
	{
	  parallelBranchSub* sp = p->parallelChild(whichChild);
	  WORKERDEBUG(100,ucout << "Child token " << whichChild 
		      << " created and delivered locally.\n");
	  addToWorkerPool(sp,bound,hubAddress);
	}
      else
	{
	  PackBuffer* outBuf = startPackingSP(bound,destProcessor,hubAddress);
	  p->packChild(*outBuf,p->chooseChild(whichChild));
	  finishDeliverSP(outBuf,destProcessor);
	  WORKERDEBUG(100,ucout << "Child " << whichChild 
		      << " sent to [" << destProcessor 
		      << "], " << outBuf->size() << " bytes.\n");
	}
      (p->tokenCount)--;
      WORKERDEBUG(100,ucout << "Token count = " << p->tokenCount 
		  << ", children left = " << p->childrenLeft << '\n');
      if ((p->childrenLeft <= 0) && (p->tokenCount <= 0)) 
	serverPool.kill(p);
    }
}


/// Subroutine of deliverSP -- get a buffer and put the initial
/// information in it.  

PackBuffer* parallelBranching::startPackingSP(double bound,
					      int dest,
					      spToken* hubAddress)
{
  // Get the buffer for processor 'dest'

  PackBuffer* outBuf = deliverSPBuffers.startSegmentBuffer(dest);

  // If it is a fresh buffer, stick in the signal saying that
  // it constains subproblems.

  if (outBuf->curr() == 0)
    *outBuf << (int) spDeliverSignal;

  // Throw in the hub address and the bound of the token

  packPointer(*outBuf,hubAddress);
  *outBuf << bound;

  // Return the buffer so the subproblem intself can be packed

  return outBuf;
}


// Subroutine of deliverSP -- fire of the actual message to the other
// worker (processor 'dest').  If the other worker doesn't seem to
// have enough buffer space, first send it a warning to expand.

void parallelBranching::finishDeliverSP(PackBuffer* outBuf,int dest)
{
  DEBUGPRP(100,ucout << "finishDeliverSP invoked: ");
  
  // How big will this buffer be after we add the segment separator
  // or message end marker?

  int bsize = outBuf->size() + sizeof(int);

  // Look at how many segments we've packed.  Estimate the size of the 
  // message if we were to pack one more segment of the present average
  // size.  If it looks like that would overflow the reciever's buffer,
  // send the message immediately without packing more subproblems.
  // If we are already over the receiver buffer size, send it a warning.

  float u = deliverSPBuffers.segCount(dest);
  DEBUGPRP(100,ucout << "bsize=" << bsize << " u=" << u);

  int destBufSize = knownBufferSize[dest];
  DEBUGPR(100,ucout << " destBufSize=" << destBufSize << endl);

  if (bsize*(u+1)/u > destBufSize)
    {
      if (bsize > destBufSize)
	{
	  DEBUGPR(100,ucout << "Buffer expansion to " << bsize 
		  << " requred\n");
	  PackBuffer* auxBuf = auxDeliverSPQ.getFree();
	  *auxBuf << (int) spBufferWarningSignal;
	  *auxBuf << bsize;
	  auxDeliverSPQ.send(auxBuf,dest,deliverSPTag);
	  knownBufferSize[dest] = bsize;
	}
      DEBUGPR(100,ucout << "Forcing spDeliver message now...\n");
      deliverSPBuffers.sendOnly(dest);
    }
  else
    deliverSPBuffers.segmentDone(dest);  // Send now if enough segs

  spDeliverCount++;
}


//  This method substitutes for the deliverSP when it turns out the problem
//  is already gone due to a better incumbent.  If the message came from
//  this cluster, just adjust the local message count.  Otherwise, tell
//  the foreign hub the message arrived.

void parallelBranching::abortDeliverSP(int hubProc)
{
  if (hubProc == myHub())
    {
      messages.hubDispatch.received++;
      WORKERDEBUG(100,ucout << "Abort delivery: hubDispatch.received "
		  << "incremented to " << messages.hubDispatch.received 
		  << ".\n");
    }
  else
    {
      PackBuffer* buf = workerOutQ.getFree();
      *buf << 0;                                   // No subproblems
      *buf << 0;                                   // No load information
      *buf << 0;                                   // No acknowledgements
      *buf << 1;                                   // One problem received
      *buf << 0;                                   // No rebalancing
      workerOutQ.send(buf,hubProc,hubTag);
      WORKERDEBUG(100,ucout << "Abort delivery: asked hub {" << hubProc 
		  << "} to adjust its count.\n");
    }
}


//  Decide whether worker should talk to hub now.

bool parallelBranching::shouldCommunicateWithHub(int triggerCount)
{
  WORKERDEBUG(500,ucout << "Should I talk to my hub?\n");

  if (releaseProbCount > triggerCount)
    {
      WORKERDEBUG(500,ucout << "Yes, releaseProbCount > triggerCount: "
		  << releaseProbCount << " > " << triggerCount << '\n');
      return true;
    }

  if (incumbentValue != lastWorkerReport.incumbentValue)
    {
      WORKERDEBUG(500,ucout << "Yes, I have a new incumbent.\n");
      return true;
    }

  int count     = spCount();
  int lowCount  = lowWorkerCount()*myWorkerTimeFrac;
  int lastCount = lastWorkerReport.count();

  if ((count <= lowCount) && (lastCount > lowCount))
    {
      WORKERDEBUG(500,ucout << "Yes, subproblem count of " 
		  << count << " below 'low' level of " << lowCount << endl);
      return true;
    }

  double denom = lastCount;
  if (denom == 0)
    denom = count;
  if ((denom > 0) && 
      (abs((double)(count - lastCount))/denom > workerLoadReportFac))
    {
      WORKERDEBUG(500,ucout << "Yes, subproblem count has changed "
		  "sufficiently: was "  << lastCount << ", now " 
		  << count << endl);
      return true;
    }

  if ((workerPool->size() == 0) || suspending())
    {
      WORKERDEBUG(500,ucout << "(workerPool=" << workerPool->size()
		  << " cp=" << checkpointing 
		  << " ab=" << aborting << ")\n");
      if (spAckCount > 0)
	{
	  WORKERDEBUG(500,ucout << "Yes, " << spAckCount << " acks.\n");
	  return true;
	}
      if (updatedPLoad().reportablyDifferent(lastWorkerReport))
	{
	  WORKERDEBUG(500,ucout << "Yes, reportably different load.\n");
	  return true;
	}
    }
  else     // If pool is non-empty and not in the middle of a checkpoint/abort
    {
      double bound = workerPool->select()->bound;
      if (incumbentValue == sense*MAXDOUBLE)
	{
	  denom = abs(bound);
	  if (denom == 0)
	    denom = abs(boundKnownToHub);
	}
      else
	denom = abs(incumbentValue - bound);
      if ((denom > 0) && 
	  (abs(bound - boundKnownToHub)/denom > workerBoundReportFac))
	{
	  WORKERDEBUG(500,ucout << "Yes, hub thinks my bound is " <<
		      boundKnownToHub << ", but it's really "
		      << bound << '\n');
	  return true;
	}
    }

  if (releaseTestCount >= workerLoadReportRate)
    {
      WORKERDEBUG(500,ucout << "Yes, releaseTestCount > workerLoadReportRate: "
		  << releaseTestCount << " > " 
		  << workerLoadReportRate << '\n');
      return true;
    }
  WORKERDEBUG(500,ucout << "No communication with hub called for.\n");
  return false;
}


//  Method for worker to talk to its (remote) hub.  Format:
//  [1,token ]*,0,{1,load,tranCount}{0},k,(k times)[spAckAddress,spAckBound]

void parallelBranching::workerCommunicateWithHub(bool rebalanceFlag)
{  
  lastWorkerReport = updatedPLoad();
#ifdef MEMUTIL_PRESENT
  lastWorkerReport.setMemory();
#endif
  boundKnownToHub  = lastWorkerReport.aggregateBound;

  // Took this out since I think it shouldn't be needed and it messes
  // up quality balancing.
  // lastWorkerReport.adjustBound(serverPool.globalBound());

  *workerOutBuffer << 0;                  // No more subproblems
  *workerOutBuffer << 1;                  // "Here's load information"
  *workerOutBuffer << lastWorkerReport;
  *workerOutBuffer << spAckCount;
  for(int i=0; i<spAckCount; i++)
    {
      packPointer(*workerOutBuffer,spAckAddress[i]);
      *workerOutBuffer << spAckBound[i];
    }
  *workerOutBuffer << 0;                 // No message count adjustment.
  *workerOutBuffer << rebalanceFlag;
  rebalanceCount += rebalanceFlag;

#ifdef ACRO_VALIDATING
  hubMessageSeqNum++;
  *workerOutBuffer << hubMessageSeqNum;
  WORKERDEBUG(100,ucout << "Sent sequence " << uMPI::rank
	      << '#' << hubMessageSeqNum << endl);
#endif

  WORKERDEBUG(100,ucout << "Worker communicating with hub, rebalanceCount="
	      << rebalanceCount << endl);

  WORKERDEBUG(100,ucout << "Load reported is " << lastWorkerReport
	      << ", load() returns " << load()
  	      << ", spAckCount = " << spAckCount << ".\n");

  WORKERDEBUG(110,lastWorkerReport.dump(ucout,"sent to hub"));

#ifdef ACRO_VALIDATING
  if (workerOutBuffer->size() > hub->sizeOfBuffer())
    EXCEPTION_MNGR(runtime_error,"Tried to send the hub a buffer of "
		<< workerOutBuffer->size()
		<< " > " 
		<< hub->sizeOfBuffer()
		<< " bytes.\nreleaseProbCount="
		<< releaseProbCount
		<< " spAckCount="
		<< spAckCount
	        << " rebalanceFlag="
		<< rebalanceFlag
		<< "\n"
		"maxTokensInHubMsg="
		<< maxTokensInHubMsg
	        << "	maxSPAcks="
		<< maxSPAcks);
#endif

  workerOutQ.send(workerOutBuffer,myHub(),hubTag);
  workerOutBuffer = workerOutQ.getFree();

  releaseProbCount = 0;
  releaseTestCount = 0;
  spAckCount       = 0;
}


//  Executed by worker when it wants to give a problem to its hub.

void parallelBranching::workerRelease(parallelBranchSub* p,
				      int whichChild,
				      int multiplicity,
				      int whichHub,
				      bool rebalanceFlag)
{
  WORKERDEBUG(100,ucout << "Releasing (child=" << whichChild 
	      << ",mult=" << multiplicity
	      << ") token for " << p << " to [" << whichHub << "].\n");
  spReleaseCount++;
  if (whichHub == myHub())
    {
      if (iAmHub())
	{
          MEMDEBUG_START_NEW("spToken")
          spToken* tmp = new spToken(p,whichChild,multiplicity);
          MEMDEBUG_END_NEW("spToken")
	  hubPool->insert(tmp);
	  hubHandledCount++;
	  WORKERDEBUG(100,ucout << "Completed locally.\n");
	}
      else
	{
	  spToken t(p,whichChild,multiplicity);
	  *workerOutBuffer << 1;
	  t.pack(*workerOutBuffer);
	  releaseProbCount++;
	  messages.localScatter.sent++;
	  WORKERDEBUG(100,ucout << "Packed into buffer for my hub, count=" 
		      << releaseProbCount << '\n');
	  if (releaseProbCount == maxTokensInHubMsg) {
	    WORKERDEBUG(100,ucout << "Sending to my hub, count=" 
			<< releaseProbCount << '\n');
	    workerCommunicateWithHub(rebalanceFlag);
	  }
	}
    }
  else
    {
      PackBuffer* scatterBuf = workerOutQ.getFree();
      *scatterBuf << 1;                                   // "Here's a subprob"
      spToken t(p,whichChild,multiplicity);
      t.pack(*scatterBuf);
      *scatterBuf << 0;                        // No more subproblems
      *scatterBuf << 0;                        // No load info
      *scatterBuf << 0;                        // No sp acknowledgements
      *scatterBuf << 0;                        // No message count adjustment
      *scatterBuf << 0;                        // Not rebalancing
      messages.nonLocalScatter.sent++;
      WORKERDEBUG(100,ucout << "Packed into buffer for another hub, hubTag=" 
		  << static_cast<int>(hubTag) << '\n');
      workerOutQ.send(scatterBuf,whichHub,hubTag);
    }
}


//  Code to decide which hub a scattered token goes to.

int parallelBranching::scatterHub()
{
  if (numHubs() > 1)
    {
      if ((spReleaseCount < initForceReleases) || shouldScatterToRandom())
	{
	  double x = gRandom()*adjustedWorkerCount;
	  double y = adjustedWorkersInCluster(0);
	  int cNum = (int) floor(x/y);
	  if (cNum >= numHubs())      // This can happen on rare occasions
	    cNum = numHubs() - 1;     // when the last cluster is large
	  int randomHub = hubProc(cNum);
	  WORKERDEBUG(140,ucout << "Random hub is " << randomHub << '\n');
	  return randomHub;
	}
    }
  return myHub();
}


// Decide whether to scatter to a random hub.

bool parallelBranching::shouldScatterToRandom()
{
  double gLoadMeasure = globalLoad.count();   // Try count instead of
  double cLoadMeasure = clusterLoad.count();  // loadMeasure here.
  double loadRatio;
  if (gLoadMeasure == 0)
    {
      if (cLoadMeasure == 0)
	loadRatio = 1/numHubs();
      else
	loadRatio = 1;
    }
  else
    loadRatio = cLoadMeasure/gLoadMeasure;
  WORKERDEBUG(150,ucout << "gLoad = " << gLoadMeasure << ", cLoad = "
	      << cLoadMeasure << ", ratio = " << loadRatio << '\n');
  double idealRatio = adjustedWorkersInCluster()/adjustedWorkerCount;
  double nonLocalChance = otherHubRelease.probability(loadRatio,idealRatio);
  double randomNumber = gRandom();
  WORKERDEBUG(150,ucout << "Nonlocal release probability is " 
	      << nonLocalChance << ", random number = " << randomNumber 
	      << '\n');
  return( randomNumber < nonLocalChance);
}



//  Decides whether the worker should give up control of a subproblem now.

bool parallelBranching::workerShouldRelease()
{
  if (rampingUp())
    return false;

  releaseTestCount++;

  WORKERDEBUG(110,ucout << "Worker release test.\n");
  if ((spReleaseCount < initForceReleases) && (spCount() > 0))
    {
      WORKERDEBUG(110,ucout << "Forcing initial release.\n");
      return true;
    }

  double actual = ((double) spCount())/globalLoad.count();
  double target = (1 - hubLoadFac)*myWorkerTimeFrac/adjustedWorkerCount;
  WORKERDEBUG(110,ucout << "actual fraction = " << actual 
	      << ", ideal fraction = " << target << '\n');
  double scatterChance = releaseDecision.probability(actual,target);
  double randomNumber  = gRandom();
  WORKERDEBUG(110,ucout << "scatterChance = " << scatterChance 
	      << ", random number = " << randomNumber << '\n');
  return randomNumber < scatterChance;
}


// Basic pruning code.

void parallelBranching::pruneIfNeeded()
{
  if (!needPruning)
    return;

  LOG_EVENT(2,start,pruneLogState);
  WORKERDEBUG(4 - iAmHub(),ucout << "Pruning invoked.\n");

  workerPool->prune();
  WORKERDEBUG(4 - iAmHub(),ucout << "Worker pruning complete.\n");
  needPruning = false;

  if (!rampingUp())
    {
      if (iAmWorker())
	{
	  serverPool.prune();
	  WORKERDEBUG(4 - iAmHub(),ucout << "Server pruning complete.\n");
	  if (!iAmHub())
	    workerCommunicateWithHub();
	}
      if (iAmHub() && needHubPruning)
	hubPrune();
    }
  
  if (enumerating)
    pruneRepository();

  WORKERDEBUG(4 - iAmHub(),ucout << "Pruning done.\n");
  LOG_EVENT(2,end,pruneLogState);
};


//  Deliver a problem into the worker pool.  Also adjusts boundKnownToHub and
//  marks the subproblem as delivered on a worker.  May cause immediate
//  communication with hub if the list of delivered problems is full.  
//  If the worker is also a hub, effect is immediate with no communication.

void parallelBranching::addToWorkerPool(parallelBranchSub* p,
					double bound,
					spToken* hubAddress)
{
  if (!iAmHub() && ((workerPool->size() == 0) ||
		    (sense*(bound - boundKnownToHub) < 0)))
    boundKnownToHub = bound;

  if (p->canFathom())
    {
      WORKERDEBUG(20,ucout << "Destroying fathomable arriving problem " 
		  << p << '\n');
      p->recycle();
    }
  else
    {
      WORKERDEBUG(20,ucout << "Inserting " << p << " into worker pool.\n");
      workerPool->insert(p);
    }

  messages.hubDispatch.received++;
  WORKERDEBUG(100,ucout << "Marking problem at " << (void *) hubAddress 
	       << " with bound " << bound << " as delivered ("
	       << messages.hubDispatch.received << "): ");
  if (iAmHub())
    {
      WORKERDEBUG(100,ucout << "local, ");
      if (!canFathom(bound))
	{
	  WORKERDEBUG(100,ucout << "completed.\n");
	  workerTransitPool[0].kill(hubAddress);
	  repositionWorker(0);
	}
      else
	WORKERDEBUG(100,ucout << "ignored.\n");
    }
  else
    {
      spAckAddress[spAckCount] = hubAddress;
      spAckBound[spAckCount]   = bound;
      WORKERDEBUG(100,ucout << "marked for transmission.\n");
      if (++spAckCount == maxTokenQueuing)
	workerCommunicateWithHub();
    }
  forceWorkerToRun = true;
}


//  Worker receives load information from its (remote) hub.

void parallelBranching::getLoadInfoFromHub(UnPackBuffer& inBuf)
{
  logTransition();
  inBuf >> clusterLoad;
  inBuf >> globalLoad;
  inBuf >> myHubsRebalCount;
  inBuf >> loadBalancer->roundNumber;
  logTransition();
  WORKERDEBUG(100,ucout << "Worker informed of loads, cluster = "
	      << clusterLoad << ", global = " << globalLoad << ".\n");
  WORKERDEBUG(100,ucout << "Hub's rebalance count is " 
	      << myHubsRebalCount << '\n');
  setHubBusyFractions();
  rebalanceIfNeeded();
}


//  Method to possibly return some tokens to hub control.  Returns the number
//  of tokens transferred.

int parallelBranching::rebalanceIfNeeded()
{
  if (!rebalancing)
    return 0;

  WORKERDEBUG(150,ucout << "\n\n\nrebalanceIfNeeded() activated.\n");

  if (myHubsRebalCount != rebalanceCount)
    return 0;  // No go -- hub didn't get last group of subproblems.
  WORKERDEBUG(150,ucout << "rebalanceCount test passed.\n");

  if ((clusterLoad.incumbentValue != incumbentValue) ||
      clusterLoad.mismatch())
    return 0;
  WORKERDEBUG(150,ucout << "Incumbent value matches in rebalanceIfNeeded.\n");

  int startSize = spCount();
  int lowCount  = lowWorkerCount();

  if (startSize <= lowCount)
    return 0;
  WORKERDEBUG(150,ucout << "Low work test passed\n");

  double target  = ((1 - hubLoadFac)*globalLoad.count())
                       * myWorkerTimeFrac/adjustedWorkerCount;
  if (target < lowCount)
    target = lowCount;
  double trigger = max(rebalLoadFac*target, target + rebalLoadDiff);
  if (startSize < trigger)
    return 0;
  WORKERDEBUG(150,ucout << "Final test passed: have " 
	      << startSize << " subproblems, above trigger of "
	      << trigger << endl);

  // OK, looks like we need to actually do something.  Set up.

  int startingPoolSize = workerPool->size();
  int sent             = 0;
  int scanned          = 0;
  parallelBranchSub* p = workerPool->firstToUnload();

  WORKERDEBUG(10,ucout << "Rebalance: startingPoolSize=" << startingPoolSize 
                      << ", target=" << target << ", lowCount="  << lowCount
	              << ", trigger=" << trigger << '\n' << Flush);

  // Loop over local pool of subproblems.

  while(p                                    &&  // We have a problem 
	(scanned < startingPoolSize)         &&  // We haven't seen them all
	(spCount() > target)                 &&  // We want to send more
	(workerPool->size() > 0))                // We still have something
    {
      WORKERDEBUG(10,ucout << "Considering rebalance of " << p << '\n');
 
      if (!p->canTokenize()) 
	{
	  WORKERDEBUG(10,ucout << "Cannot make token for " << p 
		      << ", skipping it.\n");
	  scanned++;
	  p = workerPool->nextToUnload();  // Can't send, go to next problem.
	  break;
	}

      workerPool->remove(p);

      int tokenType = self;
      if (p->state == separated) 
	tokenType = anyChild;  
      
      LOG_EVENT_CONDITIONAL(2,sent == 0,start,rebalLogState);

      int tokensToSend = 1;
      if (p->state == separated)
	tokensToSend=p->childrenLeft - p->tokenCount;
      for (int i=0;i<tokensToSend;i++)
	{
	  workerRelease(p,tokenType,1,myHub(),true); // Release a token.
	  sent++;
	}
         
      scanned++;
      workerDisposeSP(p);

      p = workerPool->nextToUnload();
	    
    }
  rebalanceSPCount += sent;

  WORKERDEBUG(10,ucout << "Done: scanned " << scanned << " problems, sent "
	      << sent << ", " << workerPool->size() << " left, loadMeasure="
	      << workerPool->loadMeasure() << '\n');
  WORKERDEBUG(10,ucout << "rebalanceSPCount=" << rebalanceSPCount << '\n');

  // If we are not a hub and there is something in the buffer to send,
  // initiate a communication.

  if (!iAmHub() && (sent > 0) && (releaseProbCount > 0))
    workerCommunicateWithHub(true);

  LOG_EVENT_CONDITIONAL(2,sent > 0,end,rebalLogState);

  return sent;
}


//  This sets the bias of the incumbent thread.  It's the default 
//  implementation and can be overriden.

double parallelBranching::incumbentThreadBias()
{
  double r = relGap();
  double bias = incThreadBiasFactor*pow(r,incThreadBiasPower);
  bias = max(bias,incThreadMinBias);
  if (incumbentValue == sense*MAXDOUBLE)
    bias = max(bias,noIncumbentMinBias);
  bias = min(bias,incThreadMaxBias);
  DEBUGPR(10,ucout << "incumbentThreadBias()=" << bias 
	  << ", relGap= " << r
	  << ", worker bias = " << worker->bias << endl);
  return bias;
}


} // namespace pebbl

#endif
