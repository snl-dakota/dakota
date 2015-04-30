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
// earlyOutputThread.cpp
//
// Code to implement early output of incumbents from parallel layer
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {


// Constructor.

earlyOutputObj::earlyOutputObj(parallelBranching* global_) :
messageTriggeredPBThread(global_,
			 "Early Output",
			 "Early Output",
			 "orchid",
			 2,150,
			 computeBufferSize(global_),
			 global_->earlyOutputTag)
{ }


//  Routine to size input buffer.

int earlyOutputObj::computeBufferSize(parallelBranching* global_)
{
  int s = sizeof(int) + sizeof(double);
  if (uMPI::iDoIO)
    s += global_->solBufSize;
  return s;
}


//  Main message-handling method.

ThreadObj::RunStatus earlyOutputObj::handleMessage(double* controlParam)
{
  DEBUGPRP(100,ucout << "Early output thread received message containing ");

  global->recordMessageReceived(this);

  int signal;         // Read code that says what kind of message this is.
  inBuf >> signal;

  // Handle the "request" signal.  This just calls activateEarlyOutput,
  // which in turn will write the solution if it's local, or request
  // it from another processor if it's not local.  The request signal
  // will keep being passed along until it reaches a processor that
  // thinks it owns the incumbent.

  if (signal == outputRequestSignal)
    {
      DEBUGPR(100,ucout << "request signal.\n");
      activateEarlyOutput();
      return RunOK;
    }

  // Check for a "deliver" signal.  This will be followed by the solution.  
  // We unpack it and write it.

  if (signal == outputDeliverSignal)
    {
      DEBUGPR(100,ucout << "solution.\n");
      ostream* outStreamP = global->openSolutionFile();
      solution* receivedSol = global->unpackSolution(inBuf);
      receivedSol->print(*outStreamP);
      global->closeSolutionFile(outStreamP);
      confirmEarlyOutput(receivedSol->value);
      delete receivedSol;
      return RunOK;
    }

  // Check for a "confirm" signal.  This should only be received at 
  // the first hub.  It calls confirmEarlyOutput, which -- since
  // we ought to be on the first hub -- will set the necessary flags.

  if (signal == outputConfirmSignal)
    {
      DEBUGPR(100,ucout << "output confirmation.");
#ifdef ACRO_VALIDATING
      if (uMPI::rank != global->firstHub())
	EXCEPTION_MNGR(runtime_error,"Received output confirm signal "
		       " on processor " << uMPI::rank);
#endif
      double receivedValue;
      inBuf >> receivedValue;
      confirmEarlyOutput(receivedValue);
      return RunOK;
    }

  DEBUGPR(100,ucout << "invalid signal.\n");
  EXCEPTION_MNGR(runtime_error, "Early output thread received unknown signal "
		 << signal);
  
  return RunSTOP;   // This should never execute; keeps compiler happy.

};


//  Routine for triggering early output.  Check if we own the incumbent.
//  If not, pass the request on a processor that seems to (which may
//  in turn pass it on again if a new incumbent is currently being
//  propagated).

void earlyOutputObj::activateEarlyOutput()
{
  DEBUGPR(1,ucout << "Early output activation\n");
  int incumbentProcessor = global->incumbentSource;
  if (uMPI::rank == incumbentProcessor)
    writeEarlyOutput();
  else
    {
      DEBUGPR(100,ucout << "Referring request to processor " 
	      << incumbentProcessor << endl);
      outBuf.reset();
      outBuf << (int) outputRequestSignal;
      sendMessage(incumbentProcessor);
    }
}


//  Routine to call to write a solution known to reside on the local 
//  processor.  If we are allowed to write it ourselves, do so.
//  Otherwise, pack it and send it over to the IO processor.

void earlyOutputObj::writeEarlyOutput()
{
  DEBUGPR(100,ucout << "Early output write.\n");
  if (uMPI::iDoIO || !global->printSolutionSynch) // Can just print ourselves.
    {
      DEBUGPR(100,ucout << "Writing locally.\n");
      global->directSolutionToFile();
      confirmEarlyOutput(global->incumbentValue);
    }
  else                                      // Have to send it.
    {
      DEBUGPR(100,ucout << "Sending solution to processor "
	      << uMPI::ioProc << endl);
      outBuf.reset();
      outBuf << (int) outputDeliverSignal;  // Indicate this is a solution
      global->incumbent->pack(outBuf);      // Pack in the solution
      sendMessage(uMPI::ioProc);            // Send!
    }
}


//  This routine is called to indicate that the output of the solution
//  has completed.  If we are on the first hub, this just sets some flags
//  and records the value of the solution.  Otherwise, it sends a 
//  message to the first hub.

void earlyOutputObj::confirmEarlyOutput(double outputVal)
{
  DEBUGPR(100,ucout << "Early output confirmation, value=" 
	  << outputVal << endl);
  if (global->iAmFirstHub())
    {
      global->outputInProgress = false;
      global->recordEarlyOutput(outputVal);
      DEBUGPR(100,ucout << "Confirmed locally.\n");
    }
  else
    {
      DEBUGPR(100,ucout << "Passing confirmation signal to processor "
	      << global->firstHub() << endl);
      outBuf.reset();
      outBuf << (int) outputConfirmSignal;
      outBuf << outputVal;
      sendMessage(global->firstHub());
    }
}


//  Utility routine that does the message sending.  Saves a bit
//  of repeated code.

void earlyOutputObj::sendMessage(int dest)
{
  uMPI::isend((void *) outBuf.buf(),
	      outBuf.size(),
	      MPI_PACKED,
	      dest,
	      global->earlyOutputTag);
  global->recordMessageSent(this);
}

} // namespace pebbl

#endif
