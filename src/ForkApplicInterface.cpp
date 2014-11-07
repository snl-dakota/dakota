/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ForkApplicInterface.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "WorkdirHelper.hpp"
#include <sys/wait.h> // for wait and waitpid
#include <unistd.h>   // for fork, execvp, setgpid, and usleep
#include <algorithm>


namespace Dakota {

ForkApplicInterface::
ForkApplicInterface(const ProblemDescDB& problem_db):
  ProcessHandleApplicInterface(problem_db)
{ }


void ForkApplicInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  // Check for return of process id's corresponding to those stored in PRPairs.
  // Wait for at least one completion and complete all jobs that have returned.
  // This satisifies a "fairness" principle, in the sense that a completed job
  // will _always_ be processed (whereas accepting only a single completion 
  // could always accept the same completion - the case of very inexpensive fn.
  // evals. - and starve some servers).

  // wait for any process within the process group to finish.  No need for
  // usleep in wait_local_evaluations() since blocking wait is already
  // system optimized.
  pid_t pid = wait_evaluation(true); // block for completion
  do { // Perform this loop at least once for the pid from wait.
    process_local_evaluation(prp_queue, pid);
  } while ( !evalProcessIdMap.empty() && (pid = wait_evaluation(false)) > 0 );
    // Check for any additional completions (scheduling fairness)
}


void ForkApplicInterface::test_local_evaluations(PRPQueue& prp_queue)
{
  // Check for return of process id's corresponding to those stored in PRPairs.
  // Do not wait - complete all jobs that are immediately available.

  pid_t pid;
  while ( !evalProcessIdMap.empty() && (pid=wait_evaluation(false)) > 0 )
    process_local_evaluation(prp_queue, pid);

#ifdef HAVE_USLEEP
  // HAVE_USLEEP should be defined due to dependence of fork on unistd.h
  // reduce processor load from DAKOTA testing if jobs are not finishing
  if (completionSet.empty())
    usleep(1000); // 1000 microseconds = 1 millisec
#endif // HAVE_UNISTD_H
}


size_t ForkApplicInterface::wait_local_analyses()
{
  // Enforce scheduling fairness with a Waitsome design
  size_t completed = 0, num_running = analysisProcessIdMap.size();
  pid_t pid = wait_analysis(true); // block for completion
  do { // perform this loop at least once for the pid from wait.
    std::map<pid_t, int>::iterator an_it = analysisProcessIdMap.find(pid);
    if (an_it == analysisProcessIdMap.end()) {
      Cerr << "Error: analysis completion does not match local process ids "
	   << "within ForkApplicInterface::wait_local_analyses()." << std::endl;
      abort_handler(-1);
    }
#ifdef MPI_DEBUG
    Cout << "Analysis " << an_it->second << " has completed" << std::endl;
#endif // MPI_DEBUG
    analysisProcessIdMap.erase(an_it); ++completed;
  } while ( completed < num_running && (pid=wait_analysis(false)) > 0 );
    // process any additional completions

  return completed;
}


size_t ForkApplicInterface::test_local_analyses_send(int analysis_id)
{
  int rtn_code = 0; pid_t pid;
  size_t num_running = analysisProcessIdMap.size(), completed = 0;
  while ( completed < num_running && (pid=wait_analysis(false)) > 0 ) {
    std::map<pid_t, int>::iterator an_it = analysisProcessIdMap.find(pid);
    if (an_it == analysisProcessIdMap.end()) {
      Cerr << "Error: analysis completion does not match local process ids "
	   << "within ForkApplicInterface::serve_analyses_asynch()."
	   << std::endl;
      abort_handler(-1);
    }
    analysis_id = an_it->second;
#ifdef MPI_DEBUG
    Cout << "Analysis " << analysis_id << " has completed" << std::endl;
#endif // MPI_DEBUG
    // In this case, use a blocking send to avoid having to manage waits on
    // multiple send buffers (which would be a pain since the number of
    // send_buffers would vary with num_completed).
    parallelLib.send_ea(rtn_code, 0, analysis_id);
    analysisProcessIdMap.erase(an_it); ++completed;
  }

  return completed;
}


pid_t ForkApplicInterface::
create_analysis_process(bool block_flag, bool new_group)
{
  // Guidance: Do as little between fork/exec as possible to avoid
  // memory issues and overhead from copy-on-write.  Guidance from
  // various sources leads to:
  //  * chdir, getpid, setpgid should be safe (async-signal-safe in man signal)
  //  * setenv is likely not (allocates memory); for now do before
  //    fork, consider execve in future
  //  * create_command_arguments allocates memory, so do before fork

  // Convert argList StringArray to an array of const char*'s.  av
  // will point to tokens in driver_and_args, so both get passed in.
  // Perform this operation before fork as it allocates memory
  boost::shared_array<const char*> av;  // delete[] called when av out of scope
  StringArray driver_and_args;
  create_command_arguments(av, driver_and_args);

  // Set PATH, environment, and change directory; perform this
  // operation before fork as setenv allocates memory
  prepare_process_environment();

  int status = 0;
  pid_t pid = 0;

  // Ideally fork() should always be used since it is considered best
  // practice on modern computers/OS's.  However, we have observed (1)
  // unreliable fork on HPC platforms with infiniband, and (2)
  // unreliable vfork on Mac

  // The code below is written so that operations between vfork() and
  // exec() should be safe (no memory allocated/manipulated and system
  // calls are asynch-signal-safe)

  // Default to vfork unless a user override to fork; fall-through to fork 
  // if needed
#if !defined(DAKOTA_PREFER_FORK) && defined(HAVE_WORKING_VFORK)
  pid = vfork();  // replicate this process
#elif defined(HAVE_WORKING_FORK)
  pid = fork();   // replicate this process
#else
  Cerr << "Error: fork not supported under this OS." << std::endl;
  abort_handler(-1);
#endif


  if (pid == -1) {
    Cerr << "\nCould not fork; error code " << errno << " (" 
	 << std::strerror(errno) << ")" << std::endl;
    abort_handler(-1);
  }

  if (pid == 0) { // this is the child: replace process & execute analysis

    if (!block_flag) // only child sets group id -> avoids race with execvp
      join_analysis_process_group(new_group);

    // replace the child process with the fork target defined in arg_list
    status = execvp(av[0], (char*const*)av.get());

    // if execvp returns then it failed; exit gracefully. 
    // since this is a copy of the parent, use _exit
    // so that the parent i/o stream is not prematurely flushed and closed.
    _exit(status); 

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

  reset_process_environment();

  return(pid);
}


pid_t ForkApplicInterface::
wait(pid_t process_group_id, std::map<pid_t, int>& process_id_map,
     bool block_flag)
{
  int status;

  // wait/test for any completion within the process group.  We prefer this
  // approach for the blocking wait case since it can utilize a system-optimized
  // wait facility that avoids a "busy wait."  But if the last child in the
  // group has exited, then the process group no longer exists and an error
  // will be returned (pid = -1).
  pid_t pid = (block_flag) ?
    waitpid(-process_group_id, &status, 0) : // block for completion w/i group
    waitpid(-process_group_id, &status, WNOHANG);// don't block for completion

  if (pid == -1 && errno == ECHILD) { // special case: mitigate w/ fallback
    // This fallback is consistent with Approach 3 below: abandon
    // group id and manually test each pid within the process_id_map
    std::map<pid_t, int>::iterator gp_it;
    bool done = false;
    while (!done) {
      for (gp_it=process_id_map.begin(); gp_it!=process_id_map.end(); ++gp_it) {
	pid = waitpid(gp_it->first, &status, WNOHANG);
	check_wait(pid, status);
	if (pid > 0)
	  { done = true; break; }
      }
      if (block_flag) {
#ifdef HAVE_USLEEP
	if (!done)
	  usleep(1000); // 1000 microseconds = 1 millisec
#endif // HAVE_UNISTD_H
      }
      else done = true;
    }
  }
  else // default error handling
    check_wait(pid, status);

  return pid;
}


void ForkApplicInterface::
join_process_group(pid_t& process_group_id, bool new_group)
{
  if (new_group) // create new group id from the child process id
    process_group_id = getpid(); // this child pid becomes group leader

  // else we rely on parent to propagate previous process_group_id
  // through fork() to here

  // race conditions have been observed with loss of group process ids,
  // presumably when the last child exits.  waitpid(-group_id) can fail
  // in this case, which is distinctly different from waitpid(proc_id) on
  // a process that has exited.

  // Approach 1: hard error if setpgid failure
  //int err = setpgid(0, process_group_id);
  //check_group(err, process_group_id);

  // Approach 2: as a fallback, try creating a new process group.
  // NOTE: use of this fallback cannot be communicated back to the parent.
  // Implementing consistent fall back logic appears to be more complicated
  // than the simpler approach 3.
  //int err1 = setpgid(0, process_group_id);//pid=0 -> use pid from this process
  //if (err1 && !new_group) { // && errno == E****)
  //  process_group_id = getpid(); // this child pid becomes group leader
  //  int err2 = setpgid(0, process_group_id);
  //  check_group(err2, process_group_id);
  //}
  //else
  //  check_group(err1, process_group_id);

  // Approach 3: rely on parent to mitigate with fallback not based on groups
  int err = setpgid(0, process_group_id); // pid=0 -> use pid from this process
  if (err && outputLevel == DEBUG_OUTPUT) // non-fatal
    Cerr << "Warning: setpgid failure for assigning fork process group on "
	 << "child.\n         Parent will mitigate with fallback approach."
	 << std::endl;
}


void ForkApplicInterface::check_group(int err, pid_t proc_group_id)
{
  if (err) {
    Cerr << "\nChild could not set process group id " << proc_group_id
	 << " for process id " << getpid() << "; error code " << errno
	 << ":\n  ";
    switch (errno) {
    case EACCES:
      Cerr << "An attempt was made to change the process group ID of one\n  "
	   << "of the children of the calling process and the child had\n  "
	   << "already performed an exec." << std::endl; break;
    case EINVAL:
      Cerr << "pgid is less than 0." << std::endl; break;
    case EPERM:
      Cerr << "An attempt was made to move a process into a process group\n  "
	   << "in a different session, or to change the process group ID\n  "
	   << "of one of the children of the calling process and the child\n  "
	   << "was in a different session, or to change the process group\n  "
	   << "ID of a session leader." << std::endl; break;
    case ESRCH:
      Cerr << "pid is not the calling process and not a child of the\n  "
	   << "calling process." << std::endl; break;
    default:
      Cerr << std::strerror(errno) << std::endl; break;
    }
    abort_handler(-1);
  }
}

} // namespace Dakota
