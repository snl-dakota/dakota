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
// incumbCast.cpp
//
// This thread receives and distributes incumbent information from/to other
// processors.  
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI               // If no MPI, this module is a stub.

#include <pebbl/parBranching.h>
#include <pebbl/treeTopology.h>

using namespace std;

namespace pebbl {


// Construction.

incumbCastObj::incumbCastObj(parallelBranching* global_) :
  broadcastPBThread(global_,
		    "Incumbent Broadcast",
		    "Inc Bcast",
		    "cyan",
		    1,30,
		    sizeof(double) + sizeof(int),
		    global_->incumbCastTag,
		    global_->incumbTreeRadix),
  minOrMax(global->sense)     // (for efficiency later).
{ }


// Run method.  This is only invoked if a message arrives.  Check that
// it improves on the current incumbent, and if so, relay it down
// the tree.

bool incumbCastObj::unloadBuffer()
{
  double value;
  inBuf >> value;
  inBuf >> originator;
  DEBUGPR(20,ucout << "Incumbent message received from "
	  << status.MPI_SOURCE 
	  << ", value " 
	  << value
	  << ", originator " 
	  << originator 
	  << ", time="
	  << MPI_Wtime() - global->baseTime
	  << '\n');
  if (((value - global->incumbentValue)*minOrMax < 0) ||
      ((value == global->incumbentValue) &&
       (originator < global->incumbentSource)))
    {
      // The incumbent is better than one we already have; replace it.
      global->resetIncumbent();
      global->incumbentValue  = value;
      global->newIncumbentEffect(value);
      global->incumbentSource = originator;
      global->needPruning     = true;
      if (global->iAmHub())
	      global->needHubPruning = true;
      DEBUGPR(10,ucout << "Using new incumbent (" 
	      << global->incumbentValue 
	      << ',' 
	      << global->incumbentSource
	      << ")\n");
       return true;
    }
  else
    DEBUGPR(25,ucout << "Does not improve current incumbent.  Discarding.\n");
  return false;
}


// Logic to relay information to other nodes.  This can be called from
// other threads via "global->incumbentCaster->relay()";

void incumbCastObj::relayLoadBuffer(PackBuffer* buf)
{
  *buf << global->incumbentValue;
  *buf << originator;
}


// This gets called after the incumbent is relayed.  If we are a hub,
// make sure the hub gets activated.  Otherwise, there is a chance we'll
// never terminate because all the workers are already idle and the hub 
// data stuctures might never get updated to reflect this incumbent.

void incumbCastObj::preExitAction()
{ 
  if (global->iAmHub())
    global->activateHub(this); 
}

} // namespace pebbl

#endif
