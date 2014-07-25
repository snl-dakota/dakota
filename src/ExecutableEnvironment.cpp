/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ExecutableEnvironment
//- Description: Implementation code for the ExecutableEnvironment class
//- Owner:       Mike Eldred
//- Checked by:

#include "ExecutableEnvironment.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: ExecutableEnvironment.cpp 6492 2009-12-19 00:04:28Z briadam $";


namespace Dakota {

ExecutableEnvironment::ExecutableEnvironment():
  Environment(BaseConstructor())
{ }


ExecutableEnvironment::ExecutableEnvironment(int argc, char* argv[]):
  Environment(BaseConstructor(), argc, argv)
{
  // could we wait to do redirection and any output here?
  // might get entangled with CL usage...
  if (programOptions.version())
    outputManager.output_version();
      
  // After command line parsing and before input DB parsing: if just
  // getting version or help, don't instantiate anything.
  if (programOptions.proceed_to_instantiate()) {

    outputManager.output_startup_message();

    // parse input, check, and sync across ranks
    bool check_and_bcast_db = true;  // explicit default
    parse(check_and_bcast_db);

    // for executable env, always instantiate the topLevelIterator
    construct();
  }
}


ExecutableEnvironment::~ExecutableEnvironment()
{
  // destruction order of core objects:
  //  * ParallelLibrary closes things it opens, e.g., restart
  //  * OutputManager knows what it rebinds, so closes in dtor 
  //  * MPIManager calls MPI_Finalize only if needed
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
