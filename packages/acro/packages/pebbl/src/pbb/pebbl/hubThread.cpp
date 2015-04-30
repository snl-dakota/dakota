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
// hubThread.cpp
//
// Hub thread for PEBBL, source file.  Most of the intelligence is in
// the parBranching module, which can more easily access "global" data.
//
// Jonathan Eckstein
//


#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>
#include <pebbl/packPointer.h>
#include <utilib/logEvent.h>

using namespace std;

namespace pebbl {


//  Constructor.  The global pointer is set by the base class constructor.
//  Size and allocated buffer, then post a request to receive.

hubObj::hubObj(parallelBranching* global_) :
messageTriggeredPBThread(global_,
			 "Hub",
			 "Hub Thread",
			 "tomato",
			 2,100,
			 computeBufferSize(global_),
			 global_->hubTag)
{ }


// Sizing the input buffer is a little complicated here.

int hubObj::computeBufferSize(parallelBranching* global_)
{
  return global_->hubMessageSize(global_->maxTokensInHubMsg,
				 global_->maxSPAcks);
}



//  Run method.  Basically just passes the message into the parallelBranching
//  class.

ThreadObj::RunStatus hubObj::handleMessage(double* controlParam)
{
  global->handleHubMessage(inBuf,status.MPI_SOURCE);
  return RunOK;
}


void hubObj::preExitAction() 
{ 
  global->activateHub(); 
};

} // namespace pebbl

#endif
