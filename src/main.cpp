/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

/** \file main.cpp
    \brief file containing the main program for DAKOTA */

#include "ExecutableEnvironment.hpp"

#ifdef HAVE_AMPL
/** Floating-point initialization from AMPL: switch to 53-bit rounding
    if appropriate, to eliminate some cross-platform differences. */
  extern "C" void fpinit_ASL();
#endif



/// The main DAKOTA program.

/** Manage command line inputs, input files, restart file(s), output
    streams, and top level parallel iterator communicators.  Instantiate
    the ExecutableEnvironment and invoke its execute() virtual function. */

int main(int argc, char* argv[])
{
#ifdef HAVE_AMPL
  // Switch to 53-bit rounding if appropriate, to eliminate some
  // cross-platform differences.
  fpinit_ASL();	
#endif

  // Tie signals to Dakota's abort_handler
  Dakota::register_signal_handlers();

  // Define MPI_DEBUG in dakota_global_defs.cpp to cause a hold here
  Dakota::mpi_debug_hold();

  // Instantiate the Environment (which instantiates all other Dakota objects).
  Dakota::ExecutableEnvironment env(argc, argv);

  // Check or execute the environment
  if (!env.check())
    env.execute();

  return 0;
}
