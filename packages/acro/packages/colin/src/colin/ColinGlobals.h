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
 * \file ColinGlobals.h
 *
 * Defines the colin::ColinGlobals class.
 */

#ifndef colin_ColinGlobals_h
#define colin_ColinGlobals_h

#include <acro_config.h>
#include <string>

namespace colin
{


/// This class defines static data that is "global" for all of the COLIN
/// data structures.  For example, the processor ID (for parallel applications)
/// and precision used to write real values.
class ColinGlobals
{
public:

   ///
   static void reset();

   /// The precision used to write real values.
   static int write_precision;

   /// A string indicating the verbosity of output.
   static std::string output_level;

   /// The processor ID within MPI_COMM_WORLD.
   static int processor_id();
};

}

#endif
