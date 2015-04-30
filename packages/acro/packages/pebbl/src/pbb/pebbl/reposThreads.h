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
 * \file reposThreads.h
 * \author Jonathan Eckstein
 *
 * Thread used to manage the repository during the asynchronous search phase
 * when enumerating in parallel.
 */

#ifndef pebbl_reposThreads_h
#define pebbl_reposThreads_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {


  class reposRecvObj : public messageTriggeredPBThread
    {
    public:

      reposRecvObj(parallelBranching* global_);

      RunStatus handleMessage(double* controlParam);

      static int computeBufferSize(parallelBranching* global_);

      void setDebugLevel(int level);

    };


  class reposMergeObj : public parBranchingThreadObj
    {
    public:

      reposMergeObj(parallelBranching* global_);

      RunStatus runWithinLogging(double* controlParam);

      ThreadState state();

      void setDebugLevel(int level);

    };


} // namespace pebbl

#endif

#endif
