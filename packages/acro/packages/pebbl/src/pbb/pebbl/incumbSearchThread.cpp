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
// incumbSearchThread.cpp
//
// Thread object to orchestrate asynchronous incumbent search
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <utilib/seconds.h>
#include <pebbl/parBranching.h>

using namespace std;

namespace pebbl {



incumbSearchObj::incumbSearchObj(parallelBranching* global_) :
parBranchSelfAdjThread(global_,
		       "Incumbent Heuristic",
		       "Inc Heuristic",
		       "gray",
		       2,40),
nextRunTime(-MAXDOUBLE)
{
  bias       = global_->incThreadMaxBias;
  maxControl = global_->incSearchMaxControl; // Set max control from param
};


//  Run method doesn't do much.  It just calls the incumbent heuristic
//  from the parallel branching class.

//  In addition, we now set the thread's bias from a virtual function in
//  parallelBranching.  The idea is that the fraction of time devoted to the 
//  incumbent search may fall as the gap narrows.

ThreadObj::RunStatus incumbSearchObj::runWithinLogging(double* controlParam)
{
  if (global->suspending())
    return RunOK;
  global->parallelIncumbentHeuristic(controlParam);
  bias = global->incumbentThreadBias();
  nextRunTime = CPUSeconds() + 
    (global->incThreadGapSlices)*(global->timeSlice);
  return RunOK;
};


//  Similarly for figuring out what state we're in.

ThreadObj::ThreadState incumbSearchObj::state()
{
  if (global->suspending() || (CPUSeconds() < nextRunTime))
    return ThreadObj::ThreadBlocked;
  return global->incumbentHeuristicState();
};

} // namespace pebbl 

#endif
