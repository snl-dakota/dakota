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
// pbEnumerate.cpp
//
// Parallel branching class code for PEBBL -- code associated with
// parallel enumeration of multiple solutions
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>
#include <utilib/nicePrint.h>


using namespace std;
using namespace utilib;


namespace pebbl {


  // This routine stamps a solution as belonging to this processor,
  // and gives it a serial number.

  void parallelBranching::assignId(solution* sol)
  {
    DEBUGPRP(50,ucout << "Restamping id for " << sol << " -- now ");
    sol->owningProcessor = uMPI::rank;
    sol->serial          = ++parallelSolCounter;
    DEBUGPR(50,ucout << sol << endl);
  }


  // This overrides the corresponding routine in the branching::
  // class.  The "sync" argument now has some meaning: if we are in
  // ramp up, sync=synchronous means that every processor is
  // guaranteed to be executing the same call with an identical
  // solution.  After ramp-up, it has no meaning and is ignored.

  bool parallelBranching::offerToRepository(solution* sol,syncType sync)
  {
    solsOffered++;

    int  owner    = owningProcessor(sol);
    bool iAmOwner = (owner == uMPI::rank);

    DEBUGPR(50,ucout << "Parallel offerToRepository for "
	    << sol << " owner=" << owner << " -- "
	    << (iAmOwner ? "" : "not ") << "mine\n");

    // During ramp-up, we just queue new solutions in an array to be
    // processed by rampUpIncumbentSync.

    if (rampingUp())
      {
	// If the operation is synchronous, only bother doing anything
	// on the owning processor.

	if ((sync == synchronous) && !iAmOwner)
	  {
	    if (sol == incumbent)
	      incumbentSource = owner;
	    solsOffered--;              // Compensate for overcounting
	    deleteSolution(sol);
	    return false;
	  }

	// Make space in the queue for the solution, if necessary

	if (rampUpSolsPending == rampUpSolQ.size())
	  rampUpSolQ.resize(rampUpSolsPending + rampUpSolQuantum);

	// Put it in the queue

	DEBUGPR(100,ucout << "Element " << rampUpSolsPending
		<< " in ramp-up solution queue\n");

	rampUpSolQ[rampUpSolsPending++] = sol;

	return true;
      }

    // OK, we're not ramping up.  If we own the solution, handle it
    // locally.

    if (iAmOwner)
      return processOwnedSolution(sol);

    // We don't own the solution.  If flow control is enabled, make a
    // packed solution out of it and try to send it on its way.
    // Packed solutions assume minimization, so possibly negate the
    // objective.

    solsSent++;

    if (enumFlowControl)
      {
	packedSolution* psol 
	  = new packedSolution(owner,sense*(sol->value),sol);
	routePackedSolution(psol);
	return disposeSolution(sol);
      }

    // Flow control is disabled.  Send the solution directly to the
    // processor that owns it.

    DEBUGPR(100,ucout << "Sending directly to " << owner << endl);

    PackBuffer* outBuf = solHashQ.getFree();
    *outBuf << (int) hashSolSignal;
    sol->pack(*outBuf);
    solHashQ.send(outBuf,owner,repositoryTag);
    recordMessageSent(currentThread);

    return disposeSolution(sol);
  }


  // Process a new locally owned solution -- either put it in the
  // repository, or throw it away if it's fathomable.

  bool parallelBranching::processOwnedSolution(solution* sol)
  {
    DEBUGPR(100,ucout << "Locally processing solution " << sol << endl);

    assignId(sol);
    bool accepted = localReposOffer(sol);
    DEBUGPR(50,ucout << "Processed locally, accepted=" 
	    << accepted << endl);
    if (accepted && (enumCount > 1))
      needReposMerge = true;
    return accepted;
  }


  // Standard code for getting rid of a solution that might also be
  // the incumbent

  bool parallelBranching::disposeSolution(solution* sol)
  {
    DEBUGPR(100,ucout << "Disposing of solution " << sol << endl);

    if (sol == incumbent)
      return true;

    DEBUGPR(100,ucout << "Deleting\n");

    deleteSolution(sol);
    return false;
  }


  // Flow control for hashed solutions.  Given a packed solution, find
  // the hypercube-wise neighboring processor to send it to that has
  // the shortest flow control queue.  If no solutions are waiting,
  // just send it there.  Otherwise, queue it up.

  void parallelBranching::routePackedSolution(packedSolution* psol)
  {
    DEBUGPR(100,ucout << "Routing packed solution with value " 
	    << psol->value << " in the direction of " << psol->owner << endl);

    // Find the bits where the owner differs from the current rank

    unsigned int bitDiff = psol->owner ^ uMPI::rank;

#ifdef ACRO_VALIDATING
    if (bitDiff == 0)
      EXCEPTION_MNGR(runtime_error,"Trying to route solution to myself");
#endif

    // Find least congested outgoing link matching one of those bits

    size_type bestLink = nProcessorBits;   // Start with invalid value
    size_type bestLen  = MAXINT;

    unsigned int mask = 1;

    for(size_type b=0; b<nProcessorBits; b++)
      {
	if (((uMPI::rank ^ mask) < uMPI::size) &&
	    ((bitDiff & mask) != 0))
	  {
	    size_type qlen = solSendQ[b].size();
	    if (solJustSent[b] != NULL)
	      qlen++;
	    DEBUGPR(150,ucout << (1 << b) << " link has length " << qlen
		    << " with justSent=" << solJustSent[b] << endl);
	    if (qlen < bestLen)
	      {
		bestLink = b;
		bestLen  = qlen;
	      }
	  }
	mask <<= 1;
      }

#ifdef ACRO_VALIDATING
    if (bestLink == nProcessorBits)
      EXCEPTION_MNGR(runtime_error,"Could not find an outgoing link");
#endif

    DEBUGPR(100,ucout << "Best link is " << bestLink 
	    << " / " << (1 << bestLink) << endl);

    // If there is no congestion on the link, send the packed
    // solution; otherwise, queue it up.

    if (bestLen == 0)
      sendPackedSolution(psol,bestLink);
    else
      {
	solSendQ[bestLink].add(*psol);
	queuedSolutions++;
	DEBUGPR(100,ucout << "Solution queued.  queuedSolutions="
		<< queuedSolutions << ", this queue has " 
		<< solSendQ[bestLink].size() << endl);
      }
  }


  // Send a packed solution out on link 'link'

  void parallelBranching::sendPackedSolution(packedSolution* psol,int link)
  {
    int dest = uMPI::rank ^ (1 << link);
    DEBUGPR(100,ucout << "Sending packed solution with value " 
	    << psol->value << " to " << dest << endl);
    psol->isend(dest,repositoryTag);
    solJustSent[link] = psol;
    recordMessageSent(currentThread);
  }


  // This gets called when a hashed solution arrives under flow control

  void parallelBranching::solForwardAction(UnPackBuffer& inBuf,int sender)
  {
    // Unpack the owner and value (the "signal" has already been unpacked)

    int    owner;
    double value;

    inBuf >> owner >> value;

    DEBUGPR(100,ucout << "solForwardAction for solution with owner "
	    << owner << " and value " << value << endl);

    // If local, process locally.  Otherwise, copy the incoming buffer
    // and try to send it on its way.

    if (owner == uMPI::rank)
      {
	solution* sol = unpackSolution(inBuf);
	processOwnedSolution(sol);
      }
    else
      {
	packedSolution* psol = new packedSolution(owner,value,inBuf);
	routePackedSolution(psol);
      }

    // Send an ack back to the sender

    PackBuffer* ackBuf = solAckQ.getFree();
    *ackBuf << (int) ackSolSignal;
    solAckQ.send(ackBuf,sender,repositoryTag);
    recordMessageSent(currentThread);

  }


  // Gets called when a solution receipt is acknowledged under flow
  // control.  If there are more solutions waiting to go to the
  // processor that sent the ack, dequeue one and send it.  Also
  // effectively prunes the queues of solutions that can now be
  // fathomed.

  void parallelBranching::solAckAction(int sender)
  {
    // Which link was this sent on?

    unsigned int bitDiff = sender ^ uMPI::rank;
    size_type    link    = bitWidth(bitDiff) - 1;

    DEBUGPR(100,ucout << "solAckAction for ack from processor "
	    << sender << ", link " << link << endl);

#ifdef ACRO_VALIDATING
    if (bitDiff != (1 << link))
      EXCEPTION_MNGR(runtime_error,"Got sol-ack message from " << sender
		     << " bitDiff=" << bitDiff);
    if (solJustSent[link] == NULL)
      EXCEPTION_MNGR(runtime_error,"Processor " << sender 
		     << "acked nonexistent solution");
#endif

    DEBUGPR(100,ucout << "Getting rid of packed solution with owner "
	    << solJustSent[link]->owner << " and value " 
	    << solJustSent[link]->value << endl);

    // Clean up the solution that was just acked

    delete solJustSent[link];
    solJustSent[link] = NULL;

    // See if anything else is waiting to go out on that link.  In
    // practice, because the solutions are queued by objective value,
    // the following loop will either exit after one iteration, return
    // immediately because the queue is now empty, or will delete all
    // solutions in the queue because they were all fathomable, and
    // return.  Note that packed solutions have their values negated
    // for max problems.

    packedSolution* psol;

    do
      {
	if (solSendQ[link].size() == 0)
	  return;
	GenericHeapItem<packedSolution>* qItem = solSendQ[link].top();
	psol = &(qItem->key());
	bool status;
	solSendQ[link].remove(qItem,status);
	delete qItem;
	queuedSolutions--;
	if (canFathom(sense*(psol->value)))
	  {
	    DEBUGPR(100,ucout << "Deleting fathomable queued solution value="
		    << psol->value << ", owner=" << psol->owner << endl);
	    delete psol;
	    psol = NULL;
	  }
      } while(psol == NULL);

    // If we get here, we have something to send, so send it.

    sendPackedSolution(psol,link);

  }


  // Prune repository -- same as in serial, only also prune the
  // in-transit subproblem queues -- this is not strictly necessary,
  // but should save memory and give a more accurate picture of actual
  // queue size.

  void parallelBranching::pruneRepository()
  {
    branching::pruneRepository();

    if (enumFlowControl)
      for(size_type b=0; b<nProcessorBits; b++)
	{
	  GenericHeap<packedSolution>& heap = solSendQ[b];
	  size_type n = heap.size();
	  while(n > (heap.size() >> 1))
	    {
	      GenericHeapItem<packedSolution>* item = heap.member(n);
	      packedSolution* psol = &(item->key());
	      if (canFathom(sense*(psol->value)))
		{
		  DEBUGPR(100,ucout << "Pruning queued solution value="
			  << psol->value << ", owner=" << psol->owner
			  << " from link " << b << endl);
		  bool status;
		  heap.remove(item,status);
		  delete item;
		  delete psol;
		  queuedSolutions--;
		  if (n > heap.size()) 
		    n = heap.size();
		}
	      else
		n--;
	    }
	}
  }


  // During main, asynchronous phase, decides whether it is OK to
  // merge repository arrays now

  bool parallelBranching::mergeNow()
  {
    if (!needReposMerge)
      return false;
    double timeSinceLastMerge = WallClockSeconds() - lastMergeTime;
    if (timeSinceLastMerge >= reposSkewSeconds)
      return true;
    if (childArraysReceived < reposChildren)
      return false;
    return (timeSinceLastMerge >= reposMergeSeconds);
  }


  // This is what happens when the local repository changes during the
  // regular asynchronous search phase, and enumCount is active.

  void parallelBranching::reposArraySend()
  {
    DEBUGPR(100,ucout << "In parallelBranching::reposArraySend\n");

    double startWall = WallClockSeconds();

    DEBUGPR(10,ucout << "Child reports=" << childArraysReceived
	    << ", time since last merge is " 
	    << startWall - lastMergeTime << " seconds\n");

    // Recompute the best enumCount solutions in this subtree of the
    // repository tree.

    reposArrayMerge();

    // Tell everybody else about what happened.  If we happen to be
    // the root of the tree, initiate a broadcast of the new
    // information.  Otherwise, forward it up the tree.

    if (reposTree->isRoot())
      reposTreeRootAction();
    else
      {
	DEBUGPR(100,ucout << "Sending array to " << reposTree->parent()
		<< endl);
	PackBuffer* outBuf = reposArrayQ.getFree();
	*outBuf << (int) reposArraySignal;
	*outBuf << treeReposArray;
	reposArrayQ.send(outBuf,reposTree->parent(),repositoryTag);
	recordMessageSent(currentThread);
      }

    // Reset everything so that we wait the proper interval before
    // calling this routine again.

    needReposMerge      = false;
    lastMergeTime       = startWall;
    childArraysReceived = 0;

    for(size_type child=0; child<reposChildren; child++)
      gotChildArray[child] = false;

    // We might now be able to terminate; prevent hangs by forcing the
    // hub to run.  Also possibly communicate changed state to the hub
    // if we are a pure worker.

    if (iAmHub())
      activateHub(currentThread);
    else if (iAmWorker() && shouldCommunicateWithHub(localScatterQuantum))
      workerCommunicateWithHub();

  }


  // This routine merges k+1 sorted arrays of solutionIdentifers,
  // where k is the number of children in the repository tree.  There
  // is one array for each subtree rooted here, and another for the
  // local processor.  This is accomplished using a special heap which
  // has already been mostly set up.  The effect is to compute a
  // sorted list of the solutions stored in this subtree of the
  // repository tree.  If enumCount is active, the output is truncated
  // at the first enumCount elements.

  void parallelBranching::reposArrayMerge()
  {
    DEBUGPR(100,ucout << "In parallelBranching::reposArrayMerge\n");

    // Figure out what's in the local repository

    sortReposIds(localReposArray);

    // Scan all arrays to be merged, and set their cursors back to the
    // beginning.  Figure out the total number of items to merge.

    int outputSize = localReposArray.size();

    localMergeObject.reset();

    for(int child=0; child<reposChildren; child++)
      {
	outputSize += childReposArray[child].size();
	childMergeObject[child].reset();
      }

    // If enumCount is active, cut off the result at enumCount
    // elements.  Then set up the size of the output array.

    if ((enumCount > 1) && (outputSize > enumCount))
      outputSize = enumCount;

    DEBUGPR(100,ucout << outputSize << " elements in treeReposArray\n");

    treeReposArray.resize(outputSize);

    // Get the heap back in order

    reposArrayHeap.reheapify();

    // Now just pull things out and slam them into the output. The
    // reposArrayInHeap objects in the heap take care of the details.

    for (int i=0; i<outputSize; i++)
      {
	treeReposArray[i].copy(reposArrayHeap.top()->key().advanceCursor());
	reposArrayHeap.sinkTop();
	DEBUGPR(200,ucout << &(treeReposArray[i]) << ' ');
      }

    DEBUGPR(150,ucout << endl);
  }


  // If we have a new list of the enumCount best elements at the root
  // of the repository tree.  Use it to update lastSolId, and
  // broadcast any changes.

  void parallelBranching::reposTreeRootAction()
  {
    DEBUGPR(100,ucout << "In parallelBranching::reposTreeRootAction\n");

    size_type arraySize = treeReposArray.size();
    bool changed = false;
    if (arraySize == enumCount)
      changed = updateLastSolId(&(treeReposArray[arraySize - 1]));
    DEBUGPR(10,ucout << "lastSolId now " << &lastSolId
	    << " changed=" << changed << endl);
    if (changed)
      {
	needPruning = true;
	if (iAmHub())
	  needHubPruning = true;
	lastSolRelay();
      }
  }


  // Broadcast the value of lastSolID to all children in the
  // repository tree.

  void parallelBranching::lastSolRelay()
  {
    DEBUGPR(100,ucout << "In parallelBranching::lastSolRelay\n");

    for (reposTree->resetChildren(); 
	 reposTree->childrenLeft(); 
	 reposTree->advanceChild())
      {
	DEBUGPR(50,ucout << "Sending lastSolId=" << &lastSolId << " to " 
		<< reposTree->currentChild() << endl);
	PackBuffer* outBuf = newLastSolQ.getFree();
	*outBuf << (int) newLastSolSignal;
	lastSolId.pack(*outBuf);
	newLastSolQ.send(outBuf,reposTree->currentChild(),repositoryTag);
	recordMessageSent(currentThread);
      }
  }


  // Extract a solution from a message reception buffer and see if it
  // can go in the local repository.

  bool parallelBranching::incorporateSolution(UnPackBuffer& inBuf)
  {
    solution* sol = unpackSolution(inBuf);
#ifdef ACRO_VALIDATING
    if (owningProcessor(sol) != uMPI::rank)
      EXCEPTION_MNGR(runtime_error,"Received a solution that should have "
		     "hashed to processor " << owningProcessor(sol));
#endif
    assignId(sol);
    return localReposOffer(sol);
  }


  // Read an array of solutionIds from an UnPackBuffer sent by
  // processor 'sender', and put it in the appropriate child
  // repository array.

  void parallelBranching::incorporateReposArray(UnPackBuffer& inBuf, 
						int sender)
  {
#ifdef ACRO_VALIDATING
    if (!reposTree->validChild(sender))
      EXCEPTION_MNGR(runtime_error,"IncorporateReposArray called for invalid "
		     "child " << sender);
#endif
    int whichChild = reposTree->whichChild(sender);
    DEBUGPR(50,ucout << "New repository array from " << sender
	    << " = child " << whichChild << endl);
    inBuf >> childReposArray[whichChild];
  }


  // Bookkeeping during asynchronous phase, when an array is received
  // from 'sender'.  Record how many different children we've received
  // arrays from.

  void parallelBranching::recordArrayArrival(int sender)
  {
    needReposMerge = true;
    int whichChild = reposTree->whichChild(sender);
    if (!gotChildArray[whichChild])
      {
	childArraysReceived++;
	gotChildArray[whichChild] = true;
      }
  }


  // Read a new worst global repository solution from a message
  // buffer, and possibly send it on down the tree.  Also make sure
  // the hub activates and critical information is passed from worker
  // to hub if necessary

  void parallelBranching::incorporateLastSol(UnPackBuffer& inBuf)
  {
    solutionIdentifier tempSolId;
    tempSolId.unpack(inBuf);
    DEBUGPR(50,ucout << "Incorporating new lastSolID = " 
	    << &tempSolId << endl);
    updateLastSolId(&tempSolId);
    needPruning = true;
    lastSolRelay();
    if (iAmHub())
      {
	needHubPruning = true;
	activateHub(currentThread);
      }
    else if (iAmWorker() && shouldCommunicateWithHub(localScatterQuantum))
      workerCommunicateWithHub();
  }


  // This routine is used during ramp-up and checkpoint restart, when
  // enumCount is active.  It globally sets lastSolId in a global,
  // synchronous manner.

  void parallelBranching::syncLastSol()
  {
    DEBUGPR(100,ucout << "In parallelBranching::syncLastSol\n");

    // First, synchronize the repository arrays, with a maximum length
    // of enumCount, charging the messages sent to the ramp-up phase.

    syncReposArrays(enumCount,rampUpMessages);

    // Just broadcast lastSolId from repository tree root

    if (reposTree->isRoot())
      {
	// Update lastSolId from the result of the syncReposArray

	if (treeReposArray.size() >= enumCount)
	  needPruning |= updateLastSolId(&treeReposArray[enumCount - 1]);

	DEBUGPR(100,ucout << "Broadcasting " << &lastSolId << endl);

	// Broadcast

	PackBuffer outBuf(solutionIdentifier::packSize());
	lastSolId.pack(outBuf);
	uMPI::broadcast((void*) outBuf.buf(),
			outBuf.curr(),
			MPI_PACKED,
			uMPI::rank);
      }
    else
      {
	UnPackBuffer inBuf(solutionIdentifier::packSize());
	uMPI::broadcast((void*) inBuf.buf(),
			inBuf.size(),
			MPI_PACKED,
			reposTree->rootNode());
	inBuf.reset(solutionIdentifier::packSize());
	solutionIdentifier tempId;
	tempId.unpack(inBuf);
	needPruning |= updateLastSolId(&tempId);
	DEBUGPR(100,ucout << "Got " << &lastSolId << endl);
	rampUpMessages++;
      }

    if (needPruning && iAmHub())
      needHubPruning = true;
  }


  // This routine is used in ramp-up, checkpoint restart, and solution
  // output.  It synchronously merges arrays of solution identifiers
  // up the repository tree, resulting in in the root having a sorted
  // list of all solutions in the repository.  The arguments are the
  // maximum length before truncation (usually enumCount, but
  // indefinite when used in solution output), and a counter to which
  // to "charge" all message activity.

  void parallelBranching::syncReposArrays(int maxLength,int& messageCounter)
  {
    DEBUGPR(100,ucout << "In parallelBranching::syncReposArrays\n");

    // Receive a message from each child in the repository tree, and
    // store the resulting arrays for merging

    if (reposChildren > 0)
      {
	int myBufSize = sizeof(int) + maxLength*solutionIdentifier::packSize();
	UnPackBuffer inBuf(uMPI::packSlop(myBufSize));
	MPI_Status status;

	for (reposTree->resetChildren(); 
	     reposTree->childrenLeft();
	     reposTree->advanceChild())
	  {
	    uMPI::recv((void*) inBuf.buf(),
		       inBuf.size(),
		       MPI_PACKED,
		       MPI_ANY_SOURCE,
		       reposSyncTag,
		       &status);
	    inBuf.reset(&status);
	    incorporateReposArray(inBuf,status.MPI_SOURCE);
	  }

	messageCounter += reposChildren;
      }

    // Merge everything to get a full picture of this subtree

    reposArrayMerge();

    // If we aren't the root, forward the information up the tree

    if (!reposTree->isRoot())
      {
	PackBuffer outBuf;
	outBuf << treeReposArray;
	uMPI::send((void*) outBuf.buf(),
		   outBuf.curr(),
		   MPI_PACKED,
		   reposTree->parent(),
		   reposSyncTag);
	DEBUGPR(100,ucout << "Sending to " << reposTree->parent() << endl);
      }
  }


  // This is used by rampUpIncumbentSync when enumeration is active.
  // Syncs up the whole repository.

  void parallelBranching::rampUpRepositorySync()
  {
    DEBUGPR(100,ucout << "In parallelBranching::rampUpRepositorySync\n");

    // We must now hash all the solutions in the queue to their proper
    // processors. Start by allocating a buffer to each possible
    // recipient processor, and a 0/1 flag saying whether it is used
    // (this is an int rather than a bool, because we are going to
    // sum-reduce it).

    BasicArray<PackBuffer> hashBuffer(uMPI::size);
    IntVector              usingBuffer(uMPI::size);
    size_type              myMaxBufSize = 0;

    for(size_type p=0; p<uMPI::size; p++)
      usingBuffer[p] = 0;

    int messagesToSend = 0;

    // Scan the queue of pending solutions and either put them in the
    // local repository or pack them into the outgoing buffer for the
    // processor that owns them.

    for(size_type s=0; s<rampUpSolsPending; s++)
      {
	solution* sol = rampUpSolQ[s];
	int owner = owningProcessor(sol);
	DEBUGPR(100,ucout << sol << " owned by " << owner << endl);
	if (owner == uMPI::rank)
	  {
	    assignId(sol);
	    localReposOffer(sol);
	  }
	else
	  {
	    if (usingBuffer[owner] == 0)
	      {
		messagesToSend++;
		usingBuffer[owner] = 1;
		DEBUGPR(100,ucout << "Sending to " << owner << endl);
	      }
	    sol->pack(hashBuffer[owner]);
	    myMaxBufSize = max(myMaxBufSize,hashBuffer[owner].curr());
	    DEBUGPR(150,ucout << "Buffer for " << owner << " now "
		    << hashBuffer[owner].curr() << " bytes, my max size now "
		    << myMaxBufSize << endl);
	    if (sol != incumbent)
	      deleteSolution(sol);
	  }
      }

    rampUpSolsPending = 0;   // Reset the queue

    // Figure out how many messages each processor will receive, and
    // the maximum buffer size needed.

    int messagesToReceive = 0;

    {
      IntVector totalUses(uMPI::size);
      uMPI::reduceCast((void*) usingBuffer.data(),
		       (void*) totalUses.data(),
		       uMPI::size,
		       MPI_INT,
		       MPI_SUM);
      messagesToReceive = totalUses[uMPI::rank];
    }

    int maxBufSize = 0;
    uMPI::reduceCast(&myMaxBufSize,&maxBufSize,1,MPI_INT,MPI_MAX);

    DEBUGPR(100,ucout << "Sending " << messagesToSend << " buffers "
	    << "and receiving " << messagesToReceive << " buffers, "
	    << "maximum size " << maxBufSize << endl);

    if (uMPI::rank > 0)
      rampUpMessages += 4;

    // Enter a weird MPI_Waitsome loop where we get all the buffers
    // moved to their destinations.  Once there, get all the solutions
    // out and store them.  Try to send to processors progressively to
    // your right around the "ring" of processors.

    UnPackBuffer inBuf(maxBufSize);

    bool sending   = false;
    bool receiving = false;

    int numSent     = 0;
    int numReceived = 0;

    size_type sendCursor = uMPI::rank;

    BasicArray<MPI_Request> reqArray(2);

    MPI_Request sendRequest = MPI_REQUEST_NULL;
    MPI_Request recvRequest = MPI_REQUEST_NULL;

    int completionCount = 0;
    IntVector completionIndices(2);
    BasicArray<MPI_Status> statusArray(2);

    while ((numReceived < messagesToReceive) ||
	   (numSent < messagesToSend))
      {
	if (!receiving && (numReceived < messagesToReceive))
	  {
	    receiving = true;
	    uMPI::irecv((void*) inBuf.buf(),
			maxBufSize,
			MPI_PACKED,
			MPI_ANY_SOURCE,
			rampUpHashTag,
			&recvRequest);
	    DEBUGPR(150,ucout << "Issued receive\n");
	  }
	if (!sending && (numSent < messagesToSend))
	  {
	    sending = true;
	    do
	      sendCursor = (sendCursor + 1) % uMPI::size;
	    while (usingBuffer[sendCursor] == 0);
	    uMPI::isend((void*) hashBuffer[sendCursor].buf(),
			hashBuffer[sendCursor].curr(),
			MPI_PACKED,
			sendCursor,
			rampUpHashTag,
		       &sendRequest);
	    DEBUGPR(100,ucout << "Sent to " << sendCursor << endl);
	  }

	// Get ready to call waitsome.  The receiving request goes in
	// position 0 if active.  The send request goes in the next
	// position, which (a bit confusingly) is position "receiving"
	// = 0 or 1.

	if (receiving)
	  reqArray[0] = recvRequest;
	if (sending)
	  reqArray[receiving] = sendRequest;

	MPI_Waitsome((int) sending + (int) receiving,
		     reqArray.data(),
		     &completionCount,
		     completionIndices.data(),
		     statusArray.data());

	// Take action for each completed request

	for (int c=0; c<completionCount; c++)
	  {
	    DEBUGPR(150,ucout << "Completion " << c << " = " 
		    << completionIndices[c] << endl);
	    if (receiving &&(completionIndices[c] == 0))
	      {
		// Must be the receive

		DEBUGPR(100,ucout << "Received from " 
			<< statusArray[c].MPI_SOURCE << endl);
		inBuf.reset(&statusArray[c]);
		while(inBuf.data_remaining())
		  incorporateSolution(inBuf);
		receiving = false;
		numReceived++;
	      }
	    else
	      {
		// Otherwise it must be the send (not much to do)

		DEBUGPR(100,ucout << "Send completed\n");
		sending = false;
		numSent++;
	      }
	  }
      }

    DEBUGPR(100,"rampUpRepositorySync loop done.\n");

    rampUpMessages += numReceived;
	
    // Phew!  Now the repositories are all hashed correctly.  Now make
    // sure we have cut-off correct everywhere if enumCount is active,
    // and correct statistics in processor 0 in all cases.  

    if (enumCount > 1)
      {
	// EnumCount is active, so we need to get the arrays straight.
	// Sync up all the arrays and lastSolId

	syncLastSol();

	pruneIfNeeded();

	// Make sure the statistics are right in processor 0, which is
	// the root of the repository tree (if that ever changes, this
	// code will have to be rewritten).

	if (uMPI::rank == 0)
	  {
	    totalReposSize = treeReposArray.size();
	    if (totalReposSize > 0)
	      worstInRepos = treeReposArray[totalReposSize - 1].value;
	  }
      }
    else
      {
	// EnumCount is off, so do some simple reductions instead.

	totalReposSize = repositorySize();             // The local size
	totalReposSize = uMPI::sum(totalReposSize,0);  // On proc 0, the total

	double localWorst = worstReposValue();
	MPI_Op reduceOp = MPI_MAX;
	if (sense == maximization)
	  reduceOp = MPI_MIN;
	uMPI::reduce(&localWorst,&worstInRepos,1,MPI_DOUBLE,reduceOp,0);
	if (uMPI::rank > 0)
	  rampUpMessages += 2;
      }

    DEBUGPR(10,if(uMPI::rank == 0) ucout << "Repository size is "
	    << totalReposSize << ", with worst solution value "
	    << worstInRepos << endl);
	    
  }


  // A minor method that tweaks a loadObjects repository information.
  // This is needed during ramp up, because the information at
  // processor 0 will only be local; everything else during ramp up is
  // globally valid.  This only works on processor 0.

  void parallelBranching::adjustReposStats(loadObject& l)
  {
    if (enumerating && reposTree->isRoot())
      {
	if (rampingUp())
	  {
	    l.repositorySize = totalReposSize;
	    l.worstInRepos   = worstInRepos;
	  }
	else if (enumCount > 1)
	  {
	    size_type rsize = treeReposArray.size();
	    if (rsize > 0)
	      {
		l.repositorySize = rsize;
		l.worstInRepos   = treeReposArray[rsize - 1].value;
	      }
	  }
      }
  }


  // Used for final either the final repository or relevant statistics

  void parallelBranching::finalReposSync()
  {
    DEBUGPR(100,ucout << "In parallelBranching::finalReposSync\n");

    // Don't do this twice if it can be avoided!

    if (finalReposSyncDone)
      return;

    // To make sure buffers are sized right, the first step is to
    // determine the total size of the repository.  Normally, that
    // should just be sitting in the globalLoad object, but in certain
    // weird situations, that might not be the case in all processors.
    // For example, the problem could have been completely solved in
    // ramp up, and the usual asynchronous search not run at all.  So,
    // to be safe, do a global reduction to get the repository size.

    DEBUGPR(100,ucout << "globalLoad.repositorySize=" 
	    << globalLoad.repositorySize
	    << " globalLoad.worstInRepos="
	    << globalLoad.worstInRepos << endl);
    int myReposSize = repositorySize();
    DEBUGPR(100,ucout << "Local repository size is " << myReposSize << endl);
    DEBUGPR(150,
	    BasicArray<solutionIdentifier> temp;
	    sortReposIds(temp);
	    for (size_type i=0; i<temp.size(); i++)
	      ucout << &(temp[i]) << endl);
    DEBUGPR(100,if(myReposSize > 0) ucout << "Worst member is " 
	    << worstReposSol() << endl);
    DEBUGPR(100,ucout << "lastSolId looks like " << &lastSolId << endl);
    uMPI::reduceCast(&myReposSize,&totalReposSize,1,MPI_INT,MPI_SUM);
    if (uMPI::rank > 0)
      solOutputMessages += 2;

    DEBUGPR(100,ucout << "totalReposSize=" << totalReposSize << endl);

    if ((enumCount > 1) && (totalReposSize > enumCount))
      {
	ucout << "Warning -- Repository is too large by "
	      << (totalReposSize - enumCount)
	      << " elements.  Resetting.\n";
	totalReposSize = enumCount;
      }

    // Next, create a sorted list of all the solutions in the
    // repository.

    syncReposArrays(totalReposSize,solOutputMessages);

    // Broadcast to all processors

    int bufSize = 0;
    if (reposTree->isRoot())
      {
	MPI_Status status;
	DEBUGPR(100,ucout << "Sending treeReposArray\n");
	PackBuffer outBuf(bufSize);
	outBuf << treeReposArray;
	bufSize = outBuf.size();
	uMPI::broadcast(&bufSize,1,MPI_INT,reposTree->rootNode());
	uMPI::broadcast((void*) outBuf.buf(),
			bufSize,
			MPI_PACKED,
			reposTree->rootNode());
      }
    else 
      {
	DEBUGPR(100,ucout << "Trying to receive treeReposArray\n");
	uMPI::broadcast(&bufSize,1,MPI_INT,reposTree->rootNode());
	solOutputMessages++;
	UnPackBuffer inBuf(bufSize);
	uMPI::broadcast((void*) inBuf.buf(),
			bufSize,
			MPI_PACKED,
			reposTree->rootNode());
	inBuf.reset(bufSize);
	inBuf >> treeReposArray;
	solOutputMessages++;
      }

    DEBUGPR(150,for(int i=0; i<totalReposSize; i++)
		  ucout << &(treeReposArray[i]) << endl;);

    finalReposSyncDone = true;

  }


  void parallelBranching::getAllSolutions(BasicArray<solution*>& solArray,
					  int whichProcessor)
  {
    int nSols = startRepositoryScan();
    bool returningAnything = 
      (whichProcessor == allProcessors) ||
      (whichProcessor == uMPI::rank);
    if (returningAnything)
      solArray.resize(nSols);
    for (int i=0; i<nSols; i++)
      {
	solution* solPtr = nextRepositoryMember(whichProcessor);
	if (returningAnything)
	  solArray[i] = solPtr;
      }
  }

 
  int parallelBranching::startRepositoryScan()
  {
    DEBUGPR(100,ucout << "In parallelBranching::startRepositoryScan:\n");

    finalReposSync();
    sortRepository(solPtrArray);
    reposArrayCursor = 0;
    solPtrCursor     = 0;
    return totalReposSize;
  }


  solution* parallelBranching::nextRepositoryMember(int whichProcessor)
  {
    DEBUGPR(100,ucout << "In parallelBranching::nextRepositoryMember, dest="
	    << whichProcessor << endl);

    int sourceProcessor = treeReposArray[reposArrayCursor++].owningProcessor;

    DEBUGPR(100,ucout << "Owner is " << sourceProcessor << endl);

    // If we own the next solution, then either
    //   1. Return it directly
    //   2. Broadcast and return it
    //   3. Send it where needed and return NULL

    if (sourceProcessor == uMPI::rank)
      {
	solution* solPtr = solPtrArray[solPtrCursor++];
	DEBUGPR(100,ucout << "Solution is " << solPtr << endl);
	if (whichProcessor == uMPI::rank)
	  {
	    DEBUGPR(100,ucout << "Local delivery\n");
	    return solPtr;
	  }
	PackBuffer outBuf;
	solPtr->pack(outBuf);
	int bufSize = outBuf.size();
	if (whichProcessor == allProcessors)
	  {
	    DEBUGPR(100,ucout << "Sending broadcast, size " << bufSize << endl);
	    uMPI::broadcast(&bufSize,1,MPI_INT,sourceProcessor);
	    uMPI::broadcast((void*) outBuf.buf(),
			    bufSize,
			    MPI_PACKED,
			    sourceProcessor);
	    return solPtr;
	  }
	else
	  {
	    // Wait for receiver to want the solution
	    MPI_Status status;
	    DEBUGPR(100,ucout << "Waiting to send...\n");
	    uMPI::recv(&status,0,MPI_INT,whichProcessor,fetchSolTag,&status);
	    solOutputMessages++;
	    DEBUGPR(100,ucout << "Sending...\n");
	    uMPI::send((void*) outBuf.buf(),
		       bufSize,
		       MPI_PACKED,
		       whichProcessor,
		       deliverSolTag);
	    return NULL;
	  }
      }

    // We don't own the next solution.  If we are not getting it, just bail

    if ((whichProcessor != allProcessors) && (whichProcessor != uMPI::rank))
      return NULL;

    // Receive the solution, either by broadcast or by message

    UnPackBuffer inBuf(solBufSize);

    if (whichProcessor == allProcessors)
      {
	int bufSize = 0;
	DEBUGPR(100,ucout << "Receiving broadcast ...");
	uMPI::broadcast(&bufSize,1,MPI_INT,sourceProcessor);
	solOutputMessages++;
	DEBUGPR(100,ucout << " size " << bufSize << endl);
	uMPI::broadcast((void*) inBuf.buf(),
			bufSize,
			MPI_PACKED,
			sourceProcessor);
	solOutputMessages++;
	inBuf.reset(bufSize);
      }
    else
      {
	MPI_Status status;
	// Tell the owner we are ready
	DEBUGPR(100,ucout << "Fetching...\n");
	uMPI::send(&sourceProcessor,0,MPI_INT,sourceProcessor,fetchSolTag);
	uMPI::recv((void*) inBuf.buf(),
		   solBufSize,
		   MPI_PACKED,
		   sourceProcessor,
		   deliverSolTag,
		   &status);
	inBuf.reset(&status);
	solOutputMessages++;
     }

    solution* solPtr = unpackSolution(inBuf);
    DEBUGPR(100,ucout << "Got " << solPtr << endl);
    return solPtr;
  }


  // This code replaces the usual solution output method in the case
  // of enumeration.  It coordinates the sorted printing of all
  // solutions.  The stream argument is used only on the I/O
  // processor; elsewhere it is ignored.

  void parallelBranching::printRepository(ostream* outStreamP)
  {
    DEBUGPR(100,ucout << "In parallelBranching::printRepository\n");

    int sols = startRepositoryScan();

    if (uMPI::iDoIO)
      {
	outStreamP->precision(statusLinePrecision);
	*outStreamP << "******** " << sols << " solution" 
		    << plural(sols) << " found ********\n";
      }

    for (int i=0; i<sols; i++)
      {
	DEBUGPR(100,ucout << "Printing solution " << i+1 << endl);
	solution* solPtr = nextRepositoryMember(uMPI::ioProc);
	if (uMPI::iDoIO)
	  {
	    *outStreamP << "\n\n******** Solution " << i+1 << " ********\n";
	    solPtr->print(*outStreamP);
	    solPtr->deleteIfNotLocal();
	  }
      }
  }


  // Print repository statistics

  void parallelBranching::printReposStatistics(std::ostream& stream)
  {
    finalReposSync();
    int totalOffer = uMPI::sum(solsOffered);
    int totalAdmit = uMPI::sum(solsAdmitted);
    int totalSent  = uMPI::sum(solsSent);
    if (uMPI::iDoIO)
      {
	int wasTagging = tagging_active();
	end_tagging();
	printReposCommon(stream,
			 totalReposSize,
			 totalOffer,
			 totalAdmit,
			 treeReposArray[totalReposSize - 1].value,
			 totalSent);
	if (wasTagging)
	  begin_tagging();
      }
  }



} // namespace pebbl

#endif
