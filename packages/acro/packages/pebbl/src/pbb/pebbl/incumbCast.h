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
 * \file incumbCast.h
 * \author Jonathan Eckstein
 *
 * This thread receives and distributes incumbent information from/to other
 * processors.  
 */

#ifndef pebbl_incumbCast_h
#define pebbl_incumbCast_h

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

namespace pebbl {

class parallelBranching;

class incumbCastObj : public broadcastPBThread
{
public:

  incumbCastObj(parallelBranching* );

  bool unloadBuffer();
  void initialLoadBuffer(PackBuffer* buf) { relayLoadBuffer(buf); };
  void relayLoadBuffer(PackBuffer* buf);

  void preExitAction();  // This thread has an exit action to activate the hub.
  
private:

  optimType minOrMax;
};

}  // namespace pebbl

#endif

#endif

