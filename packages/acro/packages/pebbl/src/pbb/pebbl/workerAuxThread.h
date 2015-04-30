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
 * \file workerAuxThread.h
 * \author Jonathan Eckstein
 *
 * Auxiliary thread object used by the worker in PEBBL when there is
 * no hub present on the same processor.
 */

#ifndef pebbl_workerAuxThread_h
#define pebbl_workerAuxThread_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {


class workerAuxObj : public messageTriggeredPBThread
{
public:

  workerAuxObj(parallelBranching* global_);

  RunStatus handleMessage(double* controlParam);

  static int computeBufferSize(parallelBranching* global_);

  void setDebugLevel(int level);

private:

  int terminateCheckValue;

};

} // namespace pebbl

#endif

#endif
