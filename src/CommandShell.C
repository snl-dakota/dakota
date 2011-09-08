/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        CommandShell
//- Description:  Class implementation
//- Owner:        Bill Bohnhoff

#include "CommandShell.H"
#include "WorkdirHelper.H"
#include "global_defs.h"

static const char rcsId[]="@(#) $Id: CommandShell.C 7021 2010-10-12 22:19:01Z wjbohnh $";


namespace Dakota {

/** Executes the sysCommand by passing it to system().  Appends an
    "&" if asynchFlag is set (background system call) and echos the
    sysCommand to Cout if suppressOutputFlag is not set. */
CommandShell& CommandShell::flush()
{
  if (asynchFlag)
    sysCommand += " &";

  if (!suppressOutputFlag)
    Cout << sysCommand << std::endl;  // output the cmd string for verification

  if ( !workDir.empty() )
    WorkdirHelper::change_cwd(workDir);

#ifdef HAVE_SYSTEM
  std::system(sysCommand.c_str());
#else
  Cout << "ERROR: attempting to use a system call on a system that does"
       << " NOT support system calls" << std::endl;
  abort_handler(-1);
#endif

  if ( !workDir.empty() )
    WorkdirHelper::reset();

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
