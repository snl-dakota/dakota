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
//
// parPebblBase.cpp
//
// A base class for the parallel part of PEBBL.  Like the pebblBase class, 
// it contains only static members (parameter definitions) and enum 
// declarations.  The parallelBranching and parallelBranchSub classes both 
// derive from this class, so they effectively share these declarations.
//
// Jonathan Eckstein
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI

#include <pebbl/parPebblBase.h>
#include <utilib/logEvent.h>

#ifdef EVENT_LOGGING_PRESENT
using utilib::logEvent;
#endif

using namespace std;

namespace pebbl {


} // namespace pebbl

#endif
