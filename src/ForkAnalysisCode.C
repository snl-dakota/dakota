/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkAnalysisCode
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "ForkAnalysisCode.H"
#include "ProblemDescDB.H"
#include "filesystem_utils.h"
#include <boost/lexical_cast.hpp>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef _WIN32
#include <process.h>
#endif

static const char rcsId[]="@(#) $Id: ForkAnalysisCode.C 7021 2010-10-12 22:19:01Z wjbohnh $";


namespace Dakota {

const char** arg_list_adjust(const char **, void **);

#if (defined(HAVE_UNISTD_H) && defined(HAVE_SYS_WAIT_H)) || defined(_WIN32) //{
#define Need_Local_Decls

// WJB - ToDo: totally rewrite in C++ (perhaps leverage Boost.Filesystem)
static const char**
arg_adjust(bool cmd_line_args, std::vector<std::string> &args,
           const char **av, const char *s)
{
  const char *argL, *t;
  int i;
  size_t L;

  av[0] = args[0].c_str();
  i = 1;
  if (cmd_line_args)
    for(; i < 3; ++i)
      av[i] = args[i].c_str();

  av[i] = 0;
  av = arg_list_adjust(av, 0);

  if (s) {
    workdir_adjust(s);
    L = std::strlen(s);
    for(i = 1; (t = av[i]); ++i)
      if (!std::strncmp(s, t, L) && t[L] == '/')
        av[i] = t + L + 1;
  }

  return av;
}
#endif //}


pid_t ForkAnalysisCode::fork_program(const bool block_flag)
{
#ifdef Need_Local_Decls
  const char *arg_list[4], **av, *wd = work_dir();
  int status;
#endif
  pid_t pid = 0;

  // vfork() should be used here since there is an immediate execvp(). This 
  // conserves memory over fork().  If some platforms have problems with a
  // hybrid fork/vfork approach, add #ifdef's but make vfork the default.
#if defined(_WIN32) //{{
	av = arg_adjust(commandLineArgs, argList, arg_list, wd);
	if (block_flag)
		status = _spawnvp(_P_WAIT, av[0], av);
	else
		pid = _spawnvp(_P_NOWAIT, av[0], av);
	if (wd)
		workdir_reset();
#else //}{
#if defined(HAVE_WORKING_VFORK)
  pid = vfork(); // replicate this process
#elif defined(HAVE_WORKING_FORK)
  pid = fork(); // replicate this process
#else
  Cerr << "Error: fork/vfork are not supported under this OS." << std::endl;
  abort_handler(-1);
#endif
  if (pid == -1) {
     Cerr << "\nCould not fork; error code " << errno << " (" 
	  << std::strerror(errno) << ")" << std::endl;
     abort_handler(-1);
  }

#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_WAIT_H)
  if (pid == 0) { // this is the child: replace process & execute application

    // Convert argList StringArray to an array of const char*'s.  An arg_list
    // entry is passed as the first argument, and the entire arg_list is cast
    // as the second argument.

    av = arg_adjust(commandLineArgs, argList, arg_list, wd);

    // replace the child process with the fork target defined in arg_list
    status = execvp(av[0], (char*const*)av);
    // if execvp returns then it failed; exit gracefully. 
    _exit(status); // since this is a copy of the parent, use _exit
    // so that the parent i/o stream is not prematurely flushed and closed.
  }
  else if (block_flag) { // parent: wait for completion
    // be explicit about waiting on the pid created above (use waitpid
    // instead of wait) so that this blocking fork works properly in
    // the possible presence of other nonblocking fork pid's (required
    // by failure capturing routine, but also good form in general).
    pid_t wpid = waitpid(pid, &status, 0);
    if (wpid == -1) {
      Cerr << "\nError in fork retrieving pid " << pid << "; error code " 
	   << errno << " (" << std::strerror(errno) << ")" << std::endl;
      abort_handler(-1);
    }
    check_status(status); // check the exit status
  }
#endif // HAVE_UNISTD_H && HAVE_SYS_WAIT_H
#endif //}} _WIN32

  return(pid);
}


/** Check to see if the process terminated abnormally (WIFEXITED(status)==0)
    or if either execvp or the application returned a status code of -1
    (WIFEXITED(status)!=0 && (signed char)WEXITSTATUS(status)==-1).  If one
    of these conditions is detected, output a failure message and abort.
    Note: the application code should not return a status code of -1 unless an
    immediate abort of dakota is wanted.  If for instance, failure capturing
    is to be used, the application code should write the word "FAIL" to the
    appropriate results file and return a status code of 0 through exit(). */
void ForkAnalysisCode::check_status(const int status)
{
#ifdef HAVE_SYS_WAIT_H
  // checks for an abnormal exit status and, if present, abort 
  if ( (WIFEXITED(status) != 0 && (signed char)WEXITSTATUS(status) == -1)
     || WIFEXITED(status) == 0 ) {
    Cerr << "Fork application failure, aborting.\nSystem error message: "
         << std::strerror(errno) << '\n';
    // std::strerror() returns the system error message associated with errno (a
    // system constant defined in errno.h containing the number of the last
    // observed error).  Note: If it becomes an issue, it would also be possible
    // to kill simulation drivers or intermediate processes for the three-piece
    // interface here, based on pid's obtained from any asynch forks. This would
    // be beneficial on systems that do not perform a complete cleanup when one
    // (asynch) child process dies.
    abort_handler(-1);
  }
#endif // HAVE_SYS_WAIT_H
}


void ForkAnalysisCode::driver_argument_list(const int analysis_id)
{
  std::string tag_str = "." + boost::lexical_cast<std::string>(analysis_id);

  argList[0] = programNames[analysis_id-1];
  argList[1] = (multipleParamsFiles) ? paramsFileName+tag_str: paramsFileName;
  argList[2] = (numPrograms > 1) ?     resultsFileName+tag_str: resultsFileName;

#ifdef DEBUG
  Cout << "argList: " << argList[0] << ' ' << argList[1] << ' ' << argList[2]
       << std::endl;
#endif
}

} // namespace Dakota
