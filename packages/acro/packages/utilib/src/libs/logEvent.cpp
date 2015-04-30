/*  _________________________________________________________________________
 *
 *  UTILIB: A utility library for developing portable C++ codes.
 *  Copyright (c) 2008 Sandia Corporation.
 *  This software is distributed under the BSD License.
 *  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
 *  the U.S. Government retains certain rights in this software.
 *  For more information, see the README file in the top UTILIB directory.
 *  _________________________________________________________________________
 */

/**
 * \file logEvent.cpp
 *
 * Routines for doing event logging/tracing of parallel codes using MPE.
 *
 * \author Jonathan Eckstein
 */

#include <utilib_config.h>
#include <utilib/CommonIO.h>
#include <utilib/logEvent.h>
#include <utilib/mpiUtil.h>

namespace utilib {


#if defined(UTILIB_HAVE_MPI) && defined(UTILIB_VALIDATING) && defined(UTILIB_HAVE_MPE)

  logEvent::logEvent() :
    eventLog(0)
  {
    create_categorized_parameter("eventLog",eventLog, 
				 "<int>","0",
				 "MPE event logging level",
				 "Debugging",
				 ParameterNonnegative<int>());

    // This parameter is is a bit weird since it's connected to a
    // static variable in the uMPI class.  I hope this does not
    // cause problems

    create_categorized_parameter("messageLog",uMPI::messageLog,
				 "<bool>","false",
				 "MPE message logging (requires event logging)",
				 "Debugging");
  };


#endif

} // namespace utilib
