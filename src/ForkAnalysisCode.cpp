/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkAnalysisCode
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "ForkAnalysisCode.hpp"
#include "ProblemDescDB.hpp"
#include "WorkdirHelper.hpp"
#include <boost/lexical_cast.hpp>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#ifdef _WIN32
#include <process.h>
#endif

static const char rcsId[]="@(#) $Id: ForkAnalysisCode.cpp 7021 2010-10-12 22:19:01Z wjbohnh $";


namespace Dakota {

void ForkAnalysisCode::driver_argument_list(int analysis_id)
{
  std::string tag_str = "." + boost::lexical_cast<std::string>(analysis_id);

  argList[0] = programNames[analysis_id-1];
  argList[1] = (multipleParamsFiles) ? paramsFileName+tag_str : paramsFileName;
  argList[2] = (numPrograms > 1) ?    resultsFileName+tag_str : resultsFileName;

#ifdef DEBUG
  Cout << "argList: " << argList[0] << ' ' << argList[1] << ' ' << argList[2]
       << std::endl;
#endif
}


pid_t ForkAnalysisCode::fork_analysis(bool block_flag, bool new_group)
{
  const char *arg_list[4], **av;
  int status;
  pid_t pid = 0;
  static std::string no_workdir;

  // vfork() should be used here since there is an immediate execvp(). This 
  // conserves memory over fork().  If some platforms have problems with a
  // hybrid fork/vfork approach, add #ifdef's but make vfork the default.
#if defined(_WIN32) //{{
	av = WorkdirHelper::arg_adjust(commandLineArgs, argList, arg_list,
                                       useWorkdir ? curWorkdir : no_workdir);
	if (block_flag)
		status = _spawnvp(_P_WAIT, av[0], av);
	else
		pid = _spawnvp(_P_NOWAIT, av[0], av);
	if (curWorkdir.c_str())
		WorkdirHelper::reset();
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
  if (pid == 0) { // this is the child: replace process & execute analysis

    if (!block_flag) // only child sets group id -> avoids race with execvp
      join_analysis_process_group(new_group);

    // Convert argList StringArray to an array of const char*'s.  An arg_list
    // entry is passed as the first argument, and the entire arg_list is cast
    // as the second argument.

    av = WorkdirHelper::arg_adjust(commandLineArgs, argList, arg_list,
                                   useWorkdir ? curWorkdir : no_workdir);

    // replace the child process with the fork target defined in arg_list
    status = execvp(av[0], (char*const*)av);
    // if execvp returns then it failed; exit gracefully. 
    _exit(status); // since this is a copy of the parent, use _exit
    // so that the parent i/o stream is not prematurely flushed and closed.
  }
  else { // parent

    if (block_flag) { // wait for completion for a particular pid
      // be explicit about waiting on the pid created above (use waitpid
      // instead of wait) so that this blocking fork works properly in
      // the possible presence of other nonblocking fork pid's (required
      // by failure capturing routine, but also good form in general).
      pid_t wpid = waitpid(pid, &status, 0);
      check_wait(wpid, status); // check the exit status
    }
    else if (new_group) {
      // store new process group id using child pid, for propagation to
      // subsequent children within this process group
      analysisProcGroupId = pid;
      //setpgid(pid, analysisProcGroupId); // race condition with execvp
    }
  }
#endif // HAVE_UNISTD_H && HAVE_SYS_WAIT_H
#endif //}} _WIN32

  return(pid);
}


pid_t ForkAnalysisCode::wait(pid_t proc_group_id, bool block_flag)
{
  int status;
  pid_t pid = (block_flag) ?
    waitpid(-proc_group_id, &status, 0) :     // block for completion w/i group
    waitpid(-proc_group_id, &status, WNOHANG);// don't block for completion

  check_wait(pid, status);

  return pid;
}


void ForkAnalysisCode::join_evaluation_process_group(bool new_group)
{
  if (new_group) // create new group id from the child process id
    evalProcGroupId = getpid(); // this child pid becomes group leader
  // else we rely on parent to propagate group id through fork() to here

  int err = setpgid(0, evalProcGroupId); // pid=0 -> use pid from this process
  check_group(err, evalProcGroupId);
}


void ForkAnalysisCode::join_analysis_process_group(bool new_group)
{
  if (new_group) // create new group id from the child process id
    analysisProcGroupId = getpid(); // this child pid becomes group leader
  // else we rely on parent to propagate group id through fork() to here

  int err = setpgid(0, analysisProcGroupId);//pid=0 -> use pid from this process
  check_group(err, analysisProcGroupId);
}


/** Check to see if the process terminated abnormally (WIFEXITED(status)==0)
    or if either execvp or the application returned a status code of -1
    (WIFEXITED(status)!=0 && (signed char)WEXITSTATUS(status)==-1).  If one
    of these conditions is detected, output a failure message and abort.
    Note: the application code should not return a status code of -1 unless an
    immediate abort of dakota is wanted.  If for instance, failure capturing
    is to be used, the application code should write the word "FAIL" to the
    appropriate results file and return a status code of 0 through exit(). */
void ForkAnalysisCode::check_wait(pid_t pid, int status)
{
  if (pid == -1) {
    Cerr << "\nFork error in parent retrieving child; error code " << errno
	 << " (" << std::strerror(errno) << ").\nConsider using system "
	 << "interface." << std::endl;
    abort_handler(-1);
  }
  else if (pid > 0) {
#ifdef HAVE_SYS_WAIT_H
    // checks for an abnormal exit status and, if present, abort 
    if ( WIFEXITED(status) == 0 || (signed char)WEXITSTATUS(status) == -1 ) {
      Cerr << "Fork application failure, aborting.\nSystem error message: "
	   << std::strerror(errno) << '\n';
      // std::strerror() returns the system error message associated with errno
      // (a system constant defined in errno.h containing the number of the last
      // observed error).  Note: If it becomes an issue, it would also be
      // possible to kill simulation drivers or intermediate processes for the
      // 3-piece interface here, based on pid's obtained from any asynch forks.
      // This would be beneficial on systems that do not perform a complete
      // cleanup when one (asynch) child process dies.
      abort_handler(-1);
    }
#endif // HAVE_SYS_WAIT_H
  }
}


void ForkAnalysisCode::check_group(int err, pid_t proc_group_id)
{
  if (err) {
    Cerr << "\nChild could not set process group id " << proc_group_id
	 << " for process id " << getpid() << "; error code " << errno
	 << ":\n  ";
    switch (errno) {
    case EACCES:
      Cerr << "An attempt was made to change the process group ID of one "
	   << "of the children of the calling process and the child had "
	   << "already performed an exec." << std::endl; break;
    case EINVAL:
      Cerr << "pgid is less than 0." << std::endl; break;
    case EPERM:
      Cerr << "An attempt was made to move a process into a process group "
	   << "in a different session, or to change the process group ID "
	   << "of one of the children of the calling process and the child "
	   << "was in a different session, or to change the process group "
	   << "ID of a session leader." << std::endl; break;
    case ESRCH:
      Cerr << "pid is not the calling process and not a child of the "
	   << "calling process." << std::endl; break;
    default:
      Cerr << "std::strerror(errno)" << std::endl; break;
    }
    abort_handler(-1);
  }
}

} // namespace Dakota
