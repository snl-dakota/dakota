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
// llChainer.cpp
//
// Orchestrates writing of load log file when it's written periodically
// * and not all that end of the run
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranching.h>


using namespace std;

namespace pebbl {


//  Constructor.  Note that the last line is the calculation of the
//  previous processor around the ring

llChainObj::llChainObj(parallelBranching* global_) :
messageTriggeredPBThread(global_,
			 "Load log chainer",
			 "LL Chain",
			 "LemonChiffon",
			 4,100,                    // log/debug levels
			 4,                        // Buffer size
			 global_->llTokenTag,
			 uMPI::rank == 0 ? uMPI::size - 1 : uMPI::rank - 1)
{ }


ThreadObj::RunStatus llChainObj::handleMessage(double* controlParam)
{
  global->receiveLLToken();
  return RunOK;
}

} // namespace pebbl

#endif
