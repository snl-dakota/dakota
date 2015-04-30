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

/**
 * \file hubThread.h
 * \author Jonathan Eckstein
 *
 * Hub thread for PEBBL, header file.  Most of the intelligence is in
 * the parBranching module, which can more easily access "global" data.
 */

#ifndef pebbl_hubThread_h
#define pebbl_hubThread_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranchThreads.h>

namespace pebbl {


class hubObj : public messageTriggeredPBThread
{
public:

  hubObj(parallelBranching* global_);

  RunStatus handleMessage(double* controlParam);

protected:

  void preExitAction();

private:
  
  int computeBufferSize(parallelBranching* global_);
  
};

} // namespace pebbl

#endif 

#endif
