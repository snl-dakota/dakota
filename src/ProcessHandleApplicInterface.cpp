/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ProcessHandleApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ProcessHandleApplicInterface.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "WorkdirHelper.hpp"
#include <algorithm>

namespace Dakota {

void ProcessHandleApplicInterface::map_bookkeeping(pid_t pid, int fn_eval_id)
{
  // store the process & eval ids in a map.  The correspondence in completed
  // process id and fn eval id is then mapped to the appropriate index of
  // prp_queue in process_local_evaluation().  Correspondence between
  // evalProcessIdMap and beforeSynchCorePRPQueue orders cannot be assumed
  // due to hybrid parallelism, i.e. ApplicationInterface::serve_asynch().
  evalProcessIdMap[pid] = fn_eval_id;
}


void ProcessHandleApplicInterface::
process_local_evaluation(PRPQueue& prp_queue, const pid_t pid)
{
  // Common processing code used by {wait,test}_local_evaluations()

  // Map pid to fn_eval_id (using evalProcessIdMap[pid] does the wrong thing
  // if the pid key is not found)
  std::map<pid_t, int>::iterator map_iter = evalProcessIdMap.find(pid);
  if (map_iter == evalProcessIdMap.end()) {
    // should not happen so long as wait ignores any 2nd level child processes
    Cerr << "Error: pid returned from wait does not match any 1st level child "
	 << "process for this interface." << std::endl;
    abort_handler(-1);
  }
  int fn_eval_id = map_iter->second;

  // now populate the corresponding response by reading the results file 
  PRPQueueIter queue_it = lookup_by_eval_id(prp_queue, fn_eval_id);
  if (queue_it == prp_queue.end()) {
    Cerr << "Error: failure in queue lookup within ProcessHandleApplicInterface"
	 << "::process_local_evaluation()." << std::endl;
    abort_handler(-1);
  }
  Response response = queue_it->response(); // shallow copy
  try { 
    read_results_files(response, fn_eval_id, final_eval_id_tag(fn_eval_id));
  }
  catch(const FileReadException& fr_except) { 
    // For forks, there is no potential for an file write race
    // condition since the process has completed -> an exception
    // involving an incomplete file/data set is a true error.
    Cerr << fr_except.what() << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  catch(const FunctionEvalFailure& fneval_except) { 
    // If a FunctionEvalFailure exception ("fail" detected in results 
    // file) is caught, call manage_failure which will either (1) repair the 
    // failure and populate response, or (2) abort the run.  NOTE: this 
    // destroys load balancing but trying to load balance failure recovery 
    // would be more difficult than it is worth.
    manage_failure(queue_it->variables(), response.active_set(), response,
		   fn_eval_id);
  }

  // bookkeep the completed job
  //queue_it->response(response);                        // not needed (shallow)
  //replace_by_eval_id(prp_queue, fn_eval_id, *queue_it);// not needed (shallow)
  completionSet.insert(fn_eval_id);
  evalProcessIdMap.erase(pid);
}


/** Manage the input filter, 1 or more analysis programs, and the
    output filter in blocking or nonblocking mode as governed by
    block_flag.  In the case of a single analysis and no filters, a
    single fork is performed, while in other cases, an initial fork is
    reforked multiple times.  Called from derived_map() with
    block_flag == BLOCK and from derived_map_asynch() with block_flag
    == FALL_THROUGH.  Uses create_analysis_process() to spawn
    individual program components within the function evaluation. */
pid_t ProcessHandleApplicInterface::create_evaluation_process(bool block_flag)
{
  size_t i;
  if (evalCommRank == 0 && !suppressOutput) {
    if (block_flag) {
      if (eaDedMasterFlag)
        Cout << "blocking fork dynamic schedule: ";
      else if (numAnalysisServers > 1)
        Cout << "blocking fork static schedule: ";
      else
        Cout << "blocking fork: ";
    }
    else
      Cout << "nonblocking fork: ";

    if (!iFilterName.empty()) {
      Cout << iFilterName;
      if (commandLineArgs)
	Cout << ' ' << paramsFileName << ' ' << resultsFileName;
      Cout << "; ";
    }
    for (i=0; i<numAnalysisDrivers; i++) {
      Cout << programNames[i];
      if (commandLineArgs) {
	Cout << ' ' << paramsFileName;
	if (multipleParamsFiles)
	  Cout << '.' << i+1;
	Cout << ' ' << resultsFileName;
	if (numAnalysisDrivers > 1)
	  Cout << '.' << i+1;
      }
      if (i != numAnalysisDrivers-1)
        Cout << "; ";
    }
    if (!oFilterName.empty()) {
      Cout << "; " << oFilterName;
      if (commandLineArgs)
	Cout << ' ' << paramsFileName << ' ' << resultsFileName;
    }
    Cout << '\n';
  }
  // Cout must be flushed prior to the fork to clear the stdout buffer.
  // Otherwise, the intermediate process receives a copy of the contents of
  // this buffer and outputs the contents on the next buffer flush.
  Cout << std::flush;

  pid_t pid = 0;
  if (iFilterName.empty() && oFilterName.empty() && numAnalysisDrivers == 1) {
    // fork the one-piece interface directly (no intermediate process required)
    driver_argument_list(1);
    bool new_group = (!block_flag && evalProcessIdMap.empty());
    pid = create_analysis_process(block_flag, new_group);
    if (new_group) // collapse the 2 groups on behalf of wait_evaluation()
      evaluation_process_group_id(analysis_process_group_id());
  }
  else if (evalCommSize > 1) {
    // run a blocking schedule of single-proc. analyses over analysis servers.
    // The schedules are executed by the parent processes.  Forks are not used
    // at this level since the message passing provides the needed asynchrony
    // at the evaluation level (unlike the final case below where 2 levels of
    // forks must be used to provide asynchrony at the eval level).

    if (!block_flag) {
      Cerr << "Error: multiprocessor evalComm does not support nonblocking "
	   << "ProcessHandleApplicInterface::create_evaluation_process()."
	   << std::endl;
      abort_handler(-1);
    }

    if (!iFilterName.empty() && evalCommRank == 0) {
      ifilter_argument_list();
      create_analysis_process(BLOCK, false);
    }

    // Schedule analyses using either master-slave/dynamic or peer/static
    if (eaDedMasterFlag) {
      // master-slave dynamic scheduling requires a central point of control 
      // and therefore needs separate schedule & serve functions.
      if (evalCommRank == 0)
        master_dynamic_schedule_analyses();
      else {
	// in message passing mode, the user must explicitly specify analysis
	// concurrency to get hybrid parallelism
        if (asynchLocalAnalysisConcurrency > 1)
          serve_analyses_asynch();
        else
          serve_analyses_synch();
      }
    }
    else {
      // static scheduling does not require special schedule/serve functions
      // since it can support message passing & hybrid mode directly using
      // synchronous_local & asynchronous_local with staggered starts.  However,
      // it does require barriers since there's no scheduler to enforce
      // synchronization.

      // avoid peers 2-n initiating analyses prior to completion of 
      // write_parameters_files() by peer 1
      parallelLib.barrier_e();

      if (asynchLocalAnalysisConcurrency > 1) // hybrid requires explicit spec
        asynchronous_local_analyses(analysisServerId, numAnalysisDrivers,
                                    numAnalysisServers); // hybrid mode
      else
        synchronous_local_analyses(analysisServerId, numAnalysisDrivers,
                                   numAnalysisServers);  // msg passing mode

      // avoid peer 1 reading all the results files before peers 2-n have
      // completed writing them
      parallelLib.barrier_e();
    }

    if (!oFilterName.empty() && evalCommRank == 0) {
      ofilter_argument_list();
      create_analysis_process(BLOCK, false);
    }
  }
  else { // schedule all analyses local to this processor

    // If the evaluation is nonblocking, then an intermediate process must be
    // forked to manage the 3-piece interface, multiple analysis drivers, or
    // both.  The intermediate process provides asynchrony at the evaluation
    // level, even though the input filter execution, analysis drivers
    // scheduling, and output filter execution are blocking.

    // In the 3-piece case, it would be desirable to utilize the same format as
    // is used in the SysCall case, i.e., grouping i_filter, simulator, and
    // o_filter with ()'s and ;'s, but this is not supported by the exec family
    // of functions (see exec man pages).

    // Since we want this intermediate process to be able to execute
    // concurrently with the parent dakota and other asynch processes,
    // fork() should be used here since there is no matching exec().
    // (NOTE: vfork should NOT be used since exec doesn't immediately follow!)
    if (!block_flag) {
#ifdef HAVE_WORKING_FORK
      // Note: working fork is necessary but not sufficient.
      // Need a better configure-time probe for reforking of a forked process.
      pid = fork();
#else
      // Note: Windows spawn does not currently support this asynch mode.
      Cerr << "Error: fork is not supported under this OS." << std::endl;
      abort_handler(-1);
#endif
    }

    bool new_group = evalProcessIdMap.empty();
    if (block_flag || pid == 0) {
      // if nonblocking, then this is the intermediate (1st level child)
      // process.  If blocking, then no fork has yet been performed, and
      // this is the parent.

      if (!block_flag) // child evaluation process from fork() above
	join_evaluation_process_group(new_group);

      // run the input filter by reforking the child process (2nd level child).
      // This refork is always blocking.  The ifilter is used just once per
      // evaluation since it is responsible for non-replicated pre-processing.
      // Any replicated pre-processing must be part of the analysis drivers
      // (see DirectApplicInterface::derived_map for additional info).
      if (!iFilterName.empty()) {
        ifilter_argument_list();
        create_analysis_process(BLOCK, false);
      }

      // run the simulator programs by reforking the child process again
      // (additional 2nd level children).  These reforks run a blocking schedule
      // (i.e., while jobs within the schedule may be nonblocking, the schedule
      // itself does not complete until all analyses are completed).  Need for a
      // nonblocking schedule is not currently anticipated, since the 1st level
      // fork provides the nonblocking evaluations needed for nonblocking
      // synchronization by certain iterators.
      if (asynchLocalAnalysisFlag) // asynch w/ concurrency limit>1 or unlimited
        asynchronous_local_analyses(1, numAnalysisDrivers, 1);
      else
        synchronous_local_analyses(1, numAnalysisDrivers, 1);

      // run the output filter by reforking the child process again (another 2nd
      // level child).  This refork is always blocking.  The ofilter is used
      // just once per evaluation since it is responsible for non-replicated
      // post-processing.  Any replicated post-processing must be part of the
      // analysis drivers (see DirectApplicInterface::derived_map for additional
      // info).
      if (!oFilterName.empty()) {
        ofilter_argument_list();
        create_analysis_process(BLOCK, false);
      }

      // If nonblocking, then this is the 1st level child process.  Quit this
      // process now.
      if (!block_flag)
        _exit(1);
    }
    else // parent process from fork() above
      if (new_group)
	evaluation_process_group_id(pid);
  }

  return(pid);
}


/** Schedule analyses asynchronously on the local processor using a dynamic
    scheduling approach (start to end in step increments).  Concurrency is
    limited by asynchLocalAnalysisConcurrency.  Modeled after
    ApplicationInterface::asynchronous_local_evaluations().  NOTE: This
    function should be elevated to ApplicationInterface if and when another
    derived interface class supports asynchronous local analyses. */
void ProcessHandleApplicInterface::
asynchronous_local_analyses(int start, int end, int step)
{
  if (numAnalysisDrivers <= 1) {
    Cerr << "Error: ForkApplicInterface::asynchronous_local_analyses() "
	 << "should only be called for multiple analysis_drivers." << std::endl;
    abort_handler(-1);
  }

  // link process id's to analysis id's for asynch jobs
  analysisProcessIdMap.clear();
  size_t i, num_sends;
  int analysis_id, num_jobs = 1 + (int)((end-start)/step);
  pid_t pid, proc_gp;

  if (asynchLocalAnalysisConcurrency)  // concurrency limited by user
    num_sends = std::min(asynchLocalAnalysisConcurrency, num_jobs);
  else // default: no limit, launch all jobs in first pass
    num_sends = num_jobs; // don't need to limit num_sends to 1 in the message
    // passing case since this fn is only called by the message passing
    // schedulers if there is asynchLocalAnalysisConcurrency

#ifdef MPI_DEBUG
  Cout << "First pass: initiating " << num_sends << " asynchronous analyses\n";
#endif // MPI_DEBUG
  bool new_group = true;
  for (i=0; i<num_sends; ++i) {
    analysis_id = start + i*step;
#ifdef MPI_DEBUG
    Cout << "Initiating analysis " << analysis_id << std::endl; // flush buffer
#endif // MPI_DEBUG
    driver_argument_list(analysis_id);
    pid = create_analysis_process(FALL_THROUGH, new_group);
    analysisProcessIdMap[pid] = analysis_id;
    new_group = false;
  }

#ifdef MPI_DEBUG
  if (num_sends < num_jobs)
    Cout << "Second pass: dynamic scheduling " << num_jobs-num_sends 
         << " remaining analyses\n";
#endif // MPI_DEBUG
  size_t send_cntr = num_sends, recv_cntr = 0, completed;
  while (recv_cntr < num_jobs) {
#ifdef MPI_DEBUG
    Cout << "Waiting on completed analyses" << std::endl;
#endif // MPI_DEBUG
    recv_cntr += completed = wait_local_analyses(); // virtual fn
    new_group = analysisProcessIdMap.empty();
    for (i=0; i<completed; ++i) {
      if (send_cntr < num_jobs) {
        analysis_id = start + send_cntr*step;
#ifdef MPI_DEBUG
        Cout << "Initiating analysis " << analysis_id << std::endl;
#endif // MPI_DEBUG
	driver_argument_list(analysis_id);
        pid = create_analysis_process(FALL_THROUGH, new_group);
        analysisProcessIdMap[pid] = analysis_id;
        ++send_cntr; new_group = false;
      }
      else break;
    }
  }
}


/** This code runs multiple asynch analyses on each server.  It is modeled
    after ApplicationInterface::serve_evaluations_asynch().  NOTE: This fn
    should be elevated to ApplicationInterface if and when another derived
    interface class supports hybrid analysis parallelism. */
void ProcessHandleApplicInterface::serve_analyses_asynch()
{
  if (numAnalysisDrivers <= 1) {
    Cerr << "Error: ForkApplicInterface::serve_analyses_asynch should "
	 << "only be called for multiple analysis_drivers." << std::endl;
    abort_handler(-1);
  }

  // link process id's to analysis id's for asynch jobs
  pid_t pid, proc_gp; int analysis_id;
  analysisProcessIdMap.clear();

  MPI_Status  status; // holds MPI_SOURCE, MPI_TAG, & MPI_ERROR
  MPI_Request recv_request = MPI_REQUEST_NULL;
  bool new_group = true; size_t num_running = 0;

  // ----------------------------------------------------------
  // Step 1: block on first message before entering while loops
  // ----------------------------------------------------------
  parallelLib.recv_ea(analysis_id, 0, MPI_ANY_TAG, status);

  do { // main loop

    // -----------------------------------------------------------------
    // Step 2: check for additional incoming messages & execute all jobs
    // -----------------------------------------------------------------
    int mpi_test_flag = 1;
    // check on asynchLocalAnalysisConcurrency limit below only required for
    // static scheduling (self scheduler handles this from the master side).
    // Leave it in for completeness even though static analysis scheduler
    // doesn't use serve fns.
    while (mpi_test_flag && analysis_id &&
	   num_running < asynchLocalAnalysisConcurrency) {
      // test for completion
      if (recv_request)
        parallelLib.test(recv_request, mpi_test_flag, status);

      // if test indicates a completion: unpack, execute, & repost
      if (mpi_test_flag) {
        analysis_id = status.MPI_TAG;

        if (analysis_id) {
	  // execute
	  driver_argument_list(analysis_id);
          pid = create_analysis_process(FALL_THROUGH, new_group);
	  analysisProcessIdMap[pid] = analysis_id; ++num_running;
	  new_group = false;
	  // repost
          parallelLib.irecv_ea(analysis_id, 0, MPI_ANY_TAG, recv_request);
	}
      }
    }

    // -----------------------------------------------------------------
    // Step 3: check for any completed jobs and return results to master
    // -----------------------------------------------------------------
    if (num_running)
      num_running -= test_local_analyses_send(analysis_id); // virtual fn

  } while (analysis_id || num_running);
}


/** Check to see if the process terminated abnormally (WIFEXITED(status)==0)
    or if either execvp or the application returned a status code of -1
    (WIFEXITED(status)!=0 && (signed char)WEXITSTATUS(status)==-1).  If one
    of these conditions is detected, output a failure message and abort.
    Note: the application code should not return a status code of -1 unless an
    immediate abort of dakota is wanted.  If for instance, failure capturing
    is to be used, the application code should write the word "FAIL" to the
    appropriate results file and return a status code of 0 through exit(). */
void ProcessHandleApplicInterface::check_wait(pid_t pid, int status)
{
  if (pid == -1) {
    Cerr << "\nFork error in parent retrieving child; error code " << errno
	 << ":\n  ";
    switch (errno) {
    case ECHILD:
      Cerr << "The process specified by pid does not exist or is not a\n  "
	   << "child of the calling process";     break;
    case EINTR:
      Cerr << "WNOHANG was not set and an unblocked signal or a SIGCHLD\n  "
	   << "was caught";                       break;
    case EINVAL:
      Cerr << "The options argument was invalid"; break;
    default:
      Cerr << std::strerror(errno);               break;
    }
    Cerr << ".\nConsider using system interface." << std::endl;
    abort_handler(-1);
  }
  else if (pid > 0) {
#ifdef HAVE_SYS_WAIT_H
    // checks for an abnormal exit status and, if present, abort.
    // From waitpid man pages: If &status is not NULL, waitpid() stores status
    // information which can be inspected with the following macros (which take
    // status as an argument):
    // > WIFEXITED(status): returns true if the child terminated normally, that
    //   is, by calling exit or _exit, or by returning from main(). 
    // > WEXITSTATUS(status): returns the exit status of the child. This
    //   consists of the status argument that the child specified in a call
    //   to exit or _exit or as the argument for a return statement in main().
    //   This macro should only be employed if WIFEXITED returned true.
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
      abort_handler(INTERFACE_ERROR);
    }
#endif // HAVE_SYS_WAIT_H
  }
}


void ProcessHandleApplicInterface::join_evaluation_process_group(bool new_group)
{ } // no-op


void ProcessHandleApplicInterface::join_analysis_process_group(bool new_group)
{ } // no-op


void ProcessHandleApplicInterface::evaluation_process_group_id(pid_t pgid)
{ } // no-op


pid_t ProcessHandleApplicInterface::evaluation_process_group_id() const
{ return 0; } // dummy default


void ProcessHandleApplicInterface::analysis_process_group_id(pid_t pgid)
{ } // no-op


pid_t ProcessHandleApplicInterface::analysis_process_group_id() const
{ return 0; } // dummy default


/** This function will split the analysis command in argList[0] based
    on whitespace, but preserve spaces within quoted strings, such
    that quoted strings can be passed as single command arguments.
    NOTE: This function allocates memory in av that might be
    implicitly freed when the child exits (control never returns to
    caller).  driver_and_args needs to be a return argument because av
    will contain pointers into its c_str()'s when done.
*/
void ProcessHandleApplicInterface::
create_command_arguments(boost::shared_array<const char*>& av, 
			 StringArray& driver_and_args)
{
  driver_and_args = WorkdirHelper::tokenize_driver(argList[0]);

  // if commandLineArgs, include params/results files at end
  size_t nargs = driver_and_args.size();
  if (commandLineArgs)
    nargs += 2;
  // ideally would use char *const argv[],
  av.reset(new const char*[nargs+1]);  // need extra NULL

  size_t i = 0;
  for ( ; i<driver_and_args.size(); ++i)
    av[i] = driver_and_args[i].c_str();
  // put params/results filenames if needed
  if (commandLineArgs) {
    av[i++] = argList[1].c_str();
    av[i++] = argList[2].c_str();
  }
    
  // last entry must be null-terminated
  av[i] = NULL;
}


} // namespace Dakota
