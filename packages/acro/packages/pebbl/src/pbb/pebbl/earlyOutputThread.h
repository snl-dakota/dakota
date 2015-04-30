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
 * \file earlyOutputThread.h
 * \author Jonathan Eckstein
 *
 * Auxiliary thread used to orchetrate early output of incumbent solutions
 * from the parallel layer.
 */

#ifndef pebbl_earlyOutputThread_h
#define pebbl_earlyOutputThread_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parBranchThreads.h>

namespace pebbl {


class earlyOutputObj : public messageTriggeredPBThread
{
public:

  earlyOutputObj(parallelBranching* global_);

  RunStatus handleMessage(double* controlParam);

  void activateEarlyOutput();
  
private:

  int computeBufferSize(parallelBranching* global_);

  enum { outputRequestSignal, outputDeliverSignal, outputConfirmSignal };

  void writeEarlyOutput();

  void confirmEarlyOutput(double outputVal);

  void sendMessage(int dest);

  PackBuffer outBuf;

};

} // namespace pebbl

#endif

#endif
