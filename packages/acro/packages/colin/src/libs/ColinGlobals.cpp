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
// Colin.cpp
//

#include <acro_config.h>
#ifdef ACRO_HAVE_MPI
#include <mpi.h>
#endif
#include <colin/ColinGlobals.h>

#if defined(USING_PROBE)
void probe() {}
#endif

namespace colin
{

void ColinGlobals::reset()
{
   write_precision = 10;
   output_level = "normal";
}

int ColinGlobals::write_precision = 10;

std::string ColinGlobals::output_level = "normal";

//
// TODO - a more general model for processor id's?  (PVM/MPI/??)
//
int ColinGlobals::processor_id()
{
#ifdef ACRO_HAVE_MPI

   int local_mpi;
   MPI_Initialized(&local_mpi);
   if (!local_mpi)
      return 0;

   int rank;
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   return rank;

#else

   return 0;

#endif
}

};

