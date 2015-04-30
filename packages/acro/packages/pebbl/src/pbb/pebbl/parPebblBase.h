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
 * \file parPebblBase.h
 * \author Jonathan Eckstein
 *
 * A base class for the parallel part of PEBBL.  Like the pebblBase class, 
 * it contains only static members (parameter definitions) and enum 
 * declarations.  The parallelBranching and parallelBranchSub classes both 
 * derive from this class, so they effectively share these declarations.
 *
 * Augmented to contain log event types.
 */

#ifndef pebbl_parPebblBase_h
#define pebbl_parPebblBase_h

#include <acro_config.h>
#include <utilib/logEvent.h>

#ifdef ACRO_HAVE_MPI

#include <utilib/std_headers.h>
#include <utilib/logEvent.h>
#include <pebbl/pebblBase.h>

namespace pebbl {


class parallelPebblBase
{
public:

  /// Needed when you start having tokens.
  enum { self = -2 };

  /// These signals specify subtypes of messages passed from
  /// hubs to workers
  enum 
    { 
      loadInfoSignal, 
      quiescencePollSignal,
      terminateCheckSignal, 
      terminateSignal,
      startCheckpointSignal,
      writeCheckpointSignal,
      startAbortSignal
    };

  /// These signals specify subtypes of messages used to managed the
  /// solution repository when enumerating
  enum
    {
      hashSolSignal,
      reposArraySignal,
      newLastSolSignal,
      forwardSolSignal,
      ackSolSignal
    };

  /// Codes that go at the beginning of subproblem delivery messages
  enum
    {
      spDeliverSignal = 2513,       // Subproblems follow
      spBufferWarningSignal = 18202 // Just a warning to enlarge receive buffer
    };
  
};


//  This class is a shorthand for parallelPebblBase combined with pebblBase

class fullPebblBase : virtual public pebblBase, public parallelPebblBase
{
  // Nothing new outside the two base classes.
};


} // namespace pebbl

#endif

#endif
