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
 * \file spReceiver.h
 * \author Jonathan Eckstein
 *
 * This thread receives subproblems from other processors.  
 */

#ifndef pebbl_spReceiver_h
#define pebbl_spReceiver_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {


class spReceiverObj : public messageTriggeredPBThread
{
public:

  spReceiverObj(parallelBranching* global_);

  RunStatus handleMessage(double* controlParam);

  int computeBufferSize(parallelBranching* global_);
};

} // namespace pebbl

#endif

#endif
