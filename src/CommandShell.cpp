/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        CommandShell
//- Description:  Class implementation
//- Owner:        Bill Bohnhoff

#include "CommandShell.hpp"
#include "WorkdirHelper.hpp"
#include "dakota_global_defs.hpp"

static const char rcsId[]="@(#) $Id: CommandShell.cpp 7021 2010-10-12 22:19:01Z wjbohnh $";


namespace Dakota {

/** Executes the sysCommand by passing it to system().  Appends an
    "&" if asynchFlag is set (background system call) and echos the
    sysCommand to Cout if suppressOutputFlag is not set. */
CommandShell& CommandShell::flush()
{
  if (asynchFlag) {
#if !defined(_MSC_VER)
    sysCommand += " &";
#else
    // using /B to keep the started command in the same Command Prompt
    sysCommand = "start \"SystemInterface-Evaluation\" /B " + sysCommand;
#endif
  }

  if (!suppressOutputFlag)
    Cout << sysCommand << std::endl;  // output the cmd string for verification

#ifdef HAVE_SYSTEM
  std::system(sysCommand.c_str());
#else
  Cout << "ERROR: attempting to use a system call on a system that does"
       << " NOT support system calls" << std::endl;
  abort_handler(-1);
#endif

  sysCommand.clear();
  return *this;
}


/** global convenience function for manipulating the shell; invokes
    the class member flush function. */
CommandShell& flush(CommandShell& shell)
{
  shell.flush();
  return shell;
}

} // namespace Dakota
