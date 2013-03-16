/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Description: The main DAKOTA program.
//-              The ProblemDescDB and Strategy are instantiated and
//-              the virtual function run_strategy is invoked.
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
#include "ParallelLibrary.hpp"
#include "CommandLineHandler.hpp"
#include "ProblemDescDB.hpp"
#include "DakotaStrategy.hpp"
#ifdef DAKOTA_USAGE_TRACKING
#include "TrackerHTTP.hpp"
#endif

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

using namespace Dakota;

/// The main DAKOTA program.

/** Manage command line inputs, input files, restart file(s), output
    streams, and top level parallel iterator communicators.  Instantiate
    the Strategy and invoke its run_strategy() virtual function. */

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
  std::signal(SIGBREAK, abort_handler);
#else
  std::signal(SIGKILL, abort_handler);
#endif
  std::signal(SIGTERM, abort_handler);
  std::signal(SIGINT,  abort_handler);

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

  // Instantiate/initialize the parallel library, command line handler, and
  // problem description database objects.  The ParallelLibrary constructor
  // calls MPI_Init() if a parallel launch is detected.  This must precede
  // CommandLineHandler initialization/parsing so that MPI may extract its
  // command line arguments first, prior to DAKOTA command line extractions.
  ParallelLibrary parallel_lib(argc, argv);
  CommandLineHandler cmd_line_handler(argc, argv);
  
  // if just getting version or help, exit now
  if (!cmd_line_handler.instantiate_flag())
    return 0;

  ProblemDescDB problem_db(parallel_lib);

  // Manage input file parsing, output redirection, and restart processing.
  // Since all processors need the database, manage_inputs() does not require
  // iterator partitions and it can precede init_iterator_communicators()
  // (a simple world bcast is sufficient).  Output/restart management does
  // utilize iterator partitions, so manage_outputs_restart() must follow
  // init_iterator_communicators() within the Strategy constructor
  // (output/restart options may only be specified at this time).
  parallel_lib.specify_outputs_restart(cmd_line_handler);
  // ProblemDescDB requires run mode information from the parallel
  // library, so must follow the specify outputs_restart
  problem_db.manage_inputs(cmd_line_handler);

  // Instantiate the Strategy object (which instantiates all Model and Iterator
  // objects) using the parsed information in problem_db.  All MPI communicator
  // partitions are created during strategy construction.
  Strategy selected_strategy(problem_db);

#ifdef DAKOTA_USAGE_TRACKING
  // must wait for iterators to be instantiated; positive side effect is that 
  // we don't track dakota -version, -help, and errant usage
  TrackerHTTP usage_tracker(problem_db, parallel_lib.world_rank());
  usage_tracker.post_start();
#endif

  // Optionally run the strategy
  if (cmd_line_handler.retrieve("check")) {
    std::string check_msg("\nInput check completed successfully (input ");
    check_msg += "parsed and objects instantiated).\n";
    parallel_lib.output_helper(check_msg);  // rank 0 only
  }
  else {
    problem_db.lock(); // prevent run-time DB queries
#ifdef _WIN32
    selected_strategy.run_strategy();
#else
    // workaround: some callers of DAKOTA, e.g., mpirun might register a handler
    // for SIGCHLD, but never reset it, which interferes with our fork interface
    void (*sigchld_save)(int);
    sigchld_save = std::signal(SIGCHLD, SIG_DFL);
    selected_strategy.run_strategy();
    std::signal(SIGCHLD, sigchld_save);
#endif
  }

#ifdef DAKOTA_USAGE_TRACKING
  usage_tracker.post_finish();
#endif

  return 0;
}
