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
 * \file incumbSearchThread.h
 * \author Jonathan Eckstein
 *
 * Thread object to orchestrate asynchronous incumbent search
 */

#ifndef pebbl_incumbSearchThread_h
#define pebbl_incumbSearchThread_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranchThreads.h>

namespace pebbl {


class incumbSearchObj :
public parBranchSelfAdjThread
{
public:

  incumbSearchObj(parallelBranching* global_);

  RunStatus runWithinLogging(double* controlParam);

  ThreadState state();

  double nextRunTime;
};

} // namespace pebbl

#endif

#endif
