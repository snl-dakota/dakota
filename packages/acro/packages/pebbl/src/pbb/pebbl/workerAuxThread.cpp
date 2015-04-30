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
// workerAuxThread.cpp
//
// Auxiliary thread object used by the worker in PEBBL, when there is
// no hub present on the same processor.  
//
// If this thread is activated, it is the only way to terminate the
// parallel search on this processor.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {



// Constructor.

workerAuxObj::workerAuxObj(parallelBranching* global_) :
messageTriggeredPBThread(global_,
			 "Worker Auxiliary",
			 "Worker Aux",
			 "aquamarine",
			 3,210,
			 computeBufferSize(global_),
			 global_->workerTag,
			 global_->myHub())
{ }


//  Routine to size input buffer.

int workerAuxObj::computeBufferSize(parallelBranching* global_)
{
  return 3*sizeof(int) + 2*parLoadObject::packSize(global_);
}


// Setting the debug level.

void workerAuxObj::setDebugLevel(int level)
{
  if (global->workerAuxDebug == -1)
    setDebug(level);
  else
    setDebug(global->workerAuxDebug);
}


//  Main message-handling method.

ThreadObj::RunStatus workerAuxObj::handleMessage(double* controlParam)
{
  int signal;
  inBuf >> signal;

  // If a higher level of debuging was requested for termination,
  // increase the debug level.
  if ((signal == quiescencePollSignal  || 
       signal == terminateSignal       ||
       signal == terminateCheckSignal) &&
      (global->termDebug > debug))
       setDebug(global->termDebug);

  DEBUGPRP(150,ucout << "Worker Auxiliary received code "
	             << signal << ": ");

  switch (signal)
    {
 
   case quiescencePollSignal:

     DEBUGPR(150,ucout << "quiescence check signal.\n");
     DEBUGPR(150,ucout << "Local general message block "
	     << global->messages.general << endl);
      {
	PackBuffer* pollBuffer = global->workerOutQ.getFree();
	global->lastWorkerReport = global->updatedPLoad();
	DEBUGPR(160,global->lastWorkerReport.dump(ucout,"lastWorkerReport"));
	*pollBuffer << global->lastWorkerReport;
	global->workerOutQ.send(pollBuffer,
				global->myHub(),
				global->quiescencePollTag);
      }
      break;

    case terminateSignal:

      DEBUGPR(150,ucout << "termination signal.\n");
      Scheduler::termination_flag = true;
      if (global->aborting)
	global->clearAllSPsForAbort();
      break;
      
    case loadInfoSignal:

      DEBUGPR(150,ucout << "load information.\n");
      global->getLoadInfoFromHub(inBuf);
      break;
      
    case terminateCheckSignal:

      terminateCheckValue = global->messages.nonLocalScatter.sent +
	global->messages.general.sent;
      DEBUGPR(150,ucout << "termination check query.\n");
      DEBUGPR(150,ucout << terminateCheckValue << " messages.\n");
      uMPI::isend(&terminateCheckValue,
		  1,
		  MPI_INT,
		  global->myHub(),
		  global->termCheckTag);
      break;
      
    case startCheckpointSignal:

      DEBUGPR(150,ucout << "start checkpoint signal.\n");
      global->setupCheckpoint();
      global->workerCommunicateWithHub();
      break;
      
    case writeCheckpointSignal:

      DEBUGPR(150,ucout << "write checkpoint signal.\n");
      global->writeCheckpoint();
      break;
      
    case startAbortSignal:

      DEBUGPR(150,ucout << "start abort signal.\n");
      if (global->abortDebug > 0)
	{
	  ucout << "Abort signalled!\n";
	  global->setDebugLevelWithThreads(global->abortDebug);
	}
      global->setupAbort();
      global->workerCommunicateWithHub();
      break;
      
    default:

    EXCEPTION_MNGR(runtime_error, 
		   "Worker auxiliary thread received unknown signal " 
		   << signal);
    }
  
  return RunOK;

};

} // namespace pebbl

#endif
