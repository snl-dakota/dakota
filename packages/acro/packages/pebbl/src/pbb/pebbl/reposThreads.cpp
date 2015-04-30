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
// reposThreads.cpp
//
// Threads that manage enumCount in parallel.  The 'repository
// receiver' receives asynchronous messages managing the solution
// repository during parallel enumeration.  The 'repository merger'
// merges information about the repository and sends it up the
// repository tree.  This second function is managed by a separate
// thread so that the total message volume may be controlled.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {



// Constructor for receiver thread.

reposRecvObj::reposRecvObj(parallelBranching* global_) :
  messageTriggeredPBThread(global_,
			   "Repository Receiver",
			   "ReposRecv",
			   "thistle",
			   3,10,
			   computeBufferSize(global_),
			   global_->repositoryTag)
{ }


//  Routine to size input buffer.

int reposRecvObj::computeBufferSize(parallelBranching* global_)
{
  int size      = global_->solBufSize;
  int enumCount = global_->enumCount;

  if (global_->enumFlowControl)
    size += sizeof(int) + sizeof(double);

  if (enumCount > 1)
    size = max(size,global_->reposArrayBufSize);

  size += sizeof(int);

  return size;
}


//  Set debug level, possibly to a special value

void reposRecvObj::setDebugLevel(int level)
{
  if (global->parameter_initialized("reposDebug"))
    setDebug(global->reposDebug);
  else
    setDebug(level);
}


//  Main message-handling method.  Basically, sense each type of
//  message and hand it off to the corresponding 'incorporateXXX'
//  method in parallelBranching.  'SendUpFlag' controls whether,
//  after all messages have been processed, we forward any information
//  up the tree.

ThreadObj::RunStatus reposRecvObj::handleMessage(double* controlParam)
{
  int signal;
  inBuf >> signal;

  DEBUGPR(150,ucout << "Repository receiver got message containing code "
	             << signal << endl);

#ifdef ACRO_VALIDATING              // Really should be DEBUGGING
  int globalDebug = global->debug;
  if (globalDebug < debug)          // Temporarily increase main debug
    global->debug = debug;          // level to the same as this thread
#endif

  int sender = status.MPI_SOURCE;

  switch (signal)
    {
    case hashSolSignal:

      DEBUGPR(150,ucout << "Hashed solution signal\n");
      {
	bool accepted = global->incorporateSolution(inBuf);
	if (accepted && (global->enumCount > 1))
	  global->needReposMerge = true;
      }
      break;

    case reposArraySignal:

      DEBUGPR(150,ucout << "Repository array signal\n");
      global->incorporateReposArray(inBuf,sender);
      global->recordArrayArrival(sender);
      break;

    case newLastSolSignal:

      DEBUGPR(150,ucout << "New cutoff solution signal\n");
      global->incorporateLastSol(inBuf);
      break;

    case forwardSolSignal:

      global->solForwardAction(inBuf,sender);
      break;

    case ackSolSignal:

      global->solAckAction(sender);
      break;
      
    default:

    EXCEPTION_MNGR(runtime_error, 
		   "Repository manager thread received unknown signal " 
		   << signal);
    }
  
  global->recordMessageReceived(this);

#ifdef ACRO_VALIDATING
  global->debug = globalDebug;
#endif

  return RunOK;

};


// Constructor for merger thread

reposMergeObj::reposMergeObj(parallelBranching* global_) :
  parBranchingThreadObj(global_,
			"Repository Merger",
			"ReposMerge",
			"HotPink",
			3,10) 
{ }


//  Set debug level of merger thread, possibly to a special value

void reposMergeObj::setDebugLevel(int level)
{
  if (global->parameter_initialized("reposDebug"))
    setDebug(global->reposDebug);
  else
    setDebug(level);
}


// Decides whether the merger thread should run

ThreadObj::ThreadState reposMergeObj::state()
{
  if (global->mergeNow())
    return ThreadObj::ThreadReady;
  return ThreadObj::ThreadBlocked;
}


// This runs when all currently pending messages for the thread have
// been processed.  If there is any information to forward up the
// repository tree, do so.

ThreadObj::RunStatus reposMergeObj::runWithinLogging(double* controlParam)
{
  int globalDebug = 0;
  DEBUGPR(1,
	  globalDebug = global->debug;
	  if (globalDebug < debug)         // Temporarily increase main debug
	    global->debug = debug);        // level to the same as this thread

  global->reposArraySend();

  DEBUGPR(1,global->debug = globalDebug);

  return RunOK;
}


} // namespace pebbl

#endif
