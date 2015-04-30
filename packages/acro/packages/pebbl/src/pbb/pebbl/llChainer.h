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
 * \file llChainer.h
 * \author Jonathan Eckstein
 *
 * Orchestrates writing of load log file when it's written periodically
 * and not all that end of the run
 */

#ifndef pebbl_llChainer_h
#define pebbl_llChainer_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {


class llChainObj : public messageTriggeredPBThread
{
public:

  llChainObj(parallelBranching* global_);

  RunStatus handleMessage(double* controlParam);
};


} // namespace pebbl

#endif

#endif
