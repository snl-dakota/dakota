/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ExecutableEnvironment
//- Description: Implementation code for the ExecutableEnvironment class
//- Owner:       Mike Eldred
//- Checked by:

#include "ExecutableEnvironment.hpp"
#include "ProblemDescDB.hpp"
#ifdef DAKOTA_USAGE_TRACKING
#include "TrackerHTTP.hpp"
#endif

static const char rcsId[]="@(#) $Id: ExecutableEnvironment.cpp 6492 2009-12-19 00:04:28Z briadam $";


namespace Dakota {

ExecutableEnvironment::ExecutableEnvironment():
#ifdef DAKOTA_USAGE_TRACKING
  Environment(BaseConstructor()), usageTracker()
#else
  Environment(BaseConstructor())
#endif
{ }


ExecutableEnvironment::ExecutableEnvironment(int argc, char* argv[]):
#ifdef DAKOTA_USAGE_TRACKING
  Environment(BaseConstructor(), argc, argv),
  usageTracker(parallelLib.world_rank())
#else
  Environment(BaseConstructor(), argc, argv)
#endif
{
  // could we wait to do redirection and any output here?
  // might get entangled with CL usage...
  if (programOptions.version)
    outputManager.output_version();
      
  // After command line parsing and before input DB parsing: if just
  // getting version or help, don't instantiate anything.
  if (programOptions.instantiate_flag()) {

    // parse input and instantiate the topLevelIterator
    construct();

#ifdef DAKOTA_USAGE_TRACKING
    // must wait for iterators to be instantiated; positive side effect is that 
    // we don't track dakota -version, -help, and errant usage
    usageTracker.post_start(probDescDB);
#endif
  }
}


ExecutableEnvironment::~ExecutableEnvironment()
{
#ifdef DAKOTA_USAGE_TRACKING
  usageTracker.post_finish();
#endif

  // destruction order of core objects:
  //  * ParallelLibrary closes things it opens, e.g., restart
  //  * OutputManager knows what it rebinds, so closes in dtor 
  //  * MPIManager calls MPI_Finalize only if needed
}


// BMA TODO: rename this function...
bool ExecutableEnvironment::check()
{
  // don't proceed to execute 
  if (programOptions.help || programOptions.version)
    return true;

  if (programOptions.checkFlag) {
    if (parallelLib.world_rank() == 0)
      Cout << "\nInput check completed successfully (input parsed and objects "
	   << "instantiated).\n" << std::endl;
    return true;
  }
  else
    return false;
}


void ExecutableEnvironment::execute()
{
#ifndef _WIN32
  // workaround: some callers of DAKOTA, e.g., mpirun might register a handler
  // for SIGCHLD, but never reset it, which interferes with our fork interface
  void (*sigchld_save)(int);
  sigchld_save = std::signal(SIGCHLD, SIG_DFL);
#endif

  Environment::execute();

#ifndef _WIN32
  std::signal(SIGCHLD, sigchld_save);
#endif
}

} // namespace Dakota
