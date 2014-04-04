/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: The main DAKOTA program.
//-              An ExecutableEnvironment is instantiated and executed.
//- Owner:       Mike Eldred
//- Checked by:  Brian Adams
//- Version: $Id: main.cpp 6882 2010-07-30 20:56:49Z wjbohnh $

/** \file main.cpp
    \brief file containing the main program for DAKOTA */

#include "ExecutableEnvironment.hpp"

#ifdef HAVE_AMPL
/** Floating-point initialization from AMPL: switch to 53-bit rounding
    if appropriate, to eliminate some cross-platform differences. */
  extern "C" void fpinit_ASL();
#endif

/// Add exedir and . to $PATH
extern "C" int nidr_save_exedir(const char*, int);


/// The main DAKOTA program.

/** Manage command line inputs, input files, restart file(s), output
    streams, and top level parallel iterator communicators.  Instantiate
    the ExecutableEnvironment and invoke its execute() virtual function. */

int main(int argc, char* argv[])
{
  // Add both the directory containing this binary and . to the end of
  // $PATH if not already on $PATH.
  unsigned short exedir2path = 1;
  unsigned short dot2path = 2;
  nidr_save_exedir(argv[0], exedir2path | dot2path);

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
