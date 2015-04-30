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
 * \file spServer.h
 * \author Jonathan Eckstein
 *
 * This thread receives tokens from other processors, and forwards the
 * corresponding subproblem data.  
 */

#ifndef pebbl_spServer_h
#define pebbl_spServer_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {


class spServerObj : public messageTriggeredPBThread
{
public:

  spServerObj(parallelBranching* global_);

  RunStatus handleMessage(double* controlParam);

  static int computeBufferSize();

protected: 

  void preExitAction();

};

} // namespace pebbl

#endif

#endif
