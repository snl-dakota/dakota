/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: The main DAKOTA program.
//-              An ExecutableEnvironment is instantiated and executed.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: main.cpp 6882 2010-07-30 20:56:49Z wjbohnh $

/** \file main.cpp
    \brief file containing the main program for DAKOTA */

#include "dakota_system_defs.hpp"

// eventually use only _WIN32 here
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#define NOMINMAX
#include <windows.h>
#endif
#include "ExecutableEnvironment.hpp"

//#define MPI_DEBUG
#if defined(MPI_DEBUG) && defined(MPICH2)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef HAVE_AMPL
/** Floating-point initialization from AMPL: switch to 53-bit rounding
    if appropriate, to eliminate some cross-platform differences. */
  extern "C" void fpinit_ASL();
#endif

extern "C" int nidr_save_exedir(const char*, int);


/// The main DAKOTA program.

/** Manage command line inputs, input files, restart file(s), output
    streams, and top level parallel iterator communicators.  Instantiate
    the ExecutableEnvironment and invoke its execute() virtual function. */

int main(int argc, char* argv[])
{
  // 3 ==> add both the directory containing this binary and . to the end
  // of $PATH if not already on $PATH.
  nidr_save_exedir(argv[0], 3);

#ifdef HAVE_AMPL
  fpinit_ASL();	// Switch to 53-bit rounding if appropriate, to
		// eliminate some cross-platform differences.
#endif
#if defined(__MINGW32__) || defined(_MSC_VER)
  std::signal(SIGBREAK, Dakota::abort_handler);
#else
  std::signal(SIGKILL, Dakota::abort_handler);
#endif
  std::signal(SIGTERM, Dakota::abort_handler);
  std::signal(SIGINT,  Dakota::abort_handler);

#ifdef MPI_DEBUG
  // hold parallel job prior to MPI_Init() in order to attach debugger to
  // master process.  Then step past ParallelLibrary instantiation and attach
  // debugger to other processes.
#ifdef MPICH2
  // To use this approach, set $DAKOTA_DEBUGPIPE to a suitable name,
  // and create $DAKOTA_DEBUGPIPE by executing "mkfifo $DAKOTA_DEBUGPIPE".
  // After invoking "mpirun ... dakota ...", find the processes, invoke
  // a debugger on them, set breakpoints, and execute "echo >$DAKOTA_DEBUGPIPE"
  // to write something to $DAKOTA_DEBUGPIPE, thus releasing dakota from
  // a wait at the open invocation below.
  char *pname; int dfd;
  if ( ( pname = getenv("DAKOTA_DEBUGPIPE") ) &&
       ( dfd = open(pname,O_RDONLY) ) > 0 ) {
    char buf[80];
    read(dfd,buf,sizeof(buf));
    close(dfd);
  }
#else
  // This simple scheme has been observed to fail with MPICH2
  int test;
  std::cin >> test;
#endif // MPICH2
#endif // MPI_DEBUG

  // Instantiate the Environment (which instantiates all other Dakota objects).
  Dakota::ExecutableEnvironment env(argc, argv);

  // Check or execute the environment
  if (!env.check())
    env.execute();

  return 0;
}
