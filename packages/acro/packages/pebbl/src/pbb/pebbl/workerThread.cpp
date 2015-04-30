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
// workerThread.cpp
//
// Thread object used by the worker in PEBBL -- non-inline code
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {


// Constructor.

workerThreadObj::workerThreadObj(parallelBranching* global_) :
parBranchSelfAdjThread(global_,
		       "Worker",
		       NULL,       // This thread not separately
		       NULL,       // logged. workerExecute is
		       1,1)        // logged instead.
{
  bias       = global->workerThreadBias;
  workerPool = global->workerPool;
  maxControl = global->maxWorkerControl;
};


//  Figures out if the thread is ready.

bool workerThreadObj::ready() 
{
  // The scheduler calls ready() when the thread is first inserted
  // In this case, nothing is set up, so return false immediately to
  // avoid seg faults
  if (!global->searchInProgress)
    return false;

  bool toReturn = false;
  int shouldComm = -1;
  if (!emptyPool() && !(global->suspending()))  // There is work.
    toReturn = true;
  else if (global->needPruning)     // A new incumbent came (maybe not needed)
    toReturn = true;
  else if (global->forceWorkerToRun)
    toReturn = true;
  else if (!global->iAmHub() && 
	   (shouldComm = global->shouldCommunicateWithHub()))
    toReturn = true;
  DEBUGPR(200,ucout << "workerThread::ready returning " << toReturn
	  << ", pool=" << (workerPool ? workerPool->size() : 0) 
	  << " needPruning=" << global->needPruning 
	  << " forceWorkerToRun=" << global->forceWorkerToRun
	  << " checkpointing=" << global->checkpointing 
	  << " aborting=" << global->aborting
	  << " shouldComm="  
	  << (shouldComm >=0 ? (char) ('0' + shouldComm) : '?') << endl);
  return toReturn;
}


//  The run method doesn't do much.  It just calls workerExecute in the
//  big-daddy class.

ThreadObj::RunStatus workerThreadObj::run(double* controlParam)
{
  global->workerExecute(controlParam);
  return RunOK;
};

} // namespace pebbl

#endif
