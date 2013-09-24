/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#elif defined(_WIN32)
#define NOMINMAX
#include <process.h>
#include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h> // for usleep()
#endif
#include <algorithm>


namespace Dakota {

ForkApplicInterface::
ForkApplicInterface(const ProblemDescDB& problem_db):
  ApplicationInterface(problem_db), forkSimulator(problem_db)
{ }


// -------------------------------------------------------
// Begin derived functions for evaluation level schedulers
// -------------------------------------------------------
void ForkApplicInterface::
derived_map(const Variables& vars, const ActiveSet& set, Response& response,
	    int fn_eval_id)
{
  // This function may be executed by a multiprocessor evalComm.

  forkSimulator.define_filenames(final_eval_id_tag(fn_eval_id)); // all of evalComm
  if (evalCommRank == 0)
    forkSimulator.write_parameters_files(vars, set, response, fn_eval_id);

  // execute the simulator application -- blocking call
  fork_application(BLOCK);

  try { 
    if (evalCommRank == 0)
      forkSimulator.read_results_files(response, fn_eval_id, 
				       final_eval_id_tag(fn_eval_id));
  }

  catch(std::string& err_msg) {
    // a std::string exception involves detection of an incomplete file/data
    // set. For the ForkApplicInterface, there is no potential for an 
    // incomplete file resulting from a race condition -> therefore, echo 
    // the error and abort.
    Cerr << err_msg << std::endl;
    abort_handler(-1);
  }

  catch(int fail_code) {
    // see explanantion in SysCallApplicInterface::derived_map()
    throw; // rethrow to outer catch
  }
}


void ForkApplicInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  // This function may not be executed by a multiprocessor evalComm.

  int fn_eval_id = pair.eval_id();
  forkSimulator.define_filenames(final_eval_id_tag(fn_eval_id)); // all of evalComm
  forkSimulator.write_parameters_files(pair.prp_parameters(), pair.active_set(),
				       pair.prp_response(),   fn_eval_id);
 
  // execute the simulator application -- nonblocking call
  pid_t pid = fork_application(FALL_THROUGH);

  // store the process & eval ids in a map.  The correspondence in completed
  // process id and fn. eval. id is then mapped to the appropriate index of
  // prp_queue in derived_synch_kernel.  Correspondence between processIdMap
  // order and beforeSynchCorePRPQueue order can not be assumed due to the
  // existence of hybrid parallelism, i.e. ApplicationInterface::serve_asynch().
  processIdMap[pid] = fn_eval_id;
}

#ifdef _WIN32
 static HANDLE*
wait_setup(std::map<pid_t, int> *M, size_t *pn)
{
	HANDLE *h;
	size_t i, n;
	std::map<pid_t, int>::iterator p, e;
	h = 0;
	if ((n = M->size()) > 0) {
		if (n > MAXIMUM_WAIT_OBJECTS) {
			Cerr << "Too many processes (" << n << ") in wait_setup\n";
			Cerr << "Current limit on processes = " << MAXIMUM_WAIT_OBJECTS << "\n";
			abort_handler(-1);
			}
		h = (HANDLE*)malloc(n*sizeof(HANDLE));
		if (!h) {
			Cerr << "malloc failure in wait_setup\n";
			abort_handler(-1);
			}
		e = M->end();
		for(i = 0, p = M->begin(); p != e; ++p)
			h[i++] = (HANDLE)p->first;
		}
	*pn = n;
	return h;
	}

 static int
wait_for_one(size_t n, HANDLE *h, int req1, size_t *pi)
{
	size_t i, j, k;

	for(j = 0; j < n; j += k) {
		if ((k = n - j) > MAXIMUM_WAIT_OBJECTS)
			k = MAXIMUM_WAIT_OBJECTS;
		i = WaitForMultipleObjects(k, h+j, 0, 0);
		if (i != WAIT_TIMEOUT)
			goto ret;
		}
	if (!req1)
		return 0;
	if (n > MAXIMUM_WAIT_OBJECTS) {
		// For simplicity, bail out.
		// Could instead create threads to wait for at most
		// MAXIMUM_WAIT_OBJECTS processes each, wait for one
		// to find a completed process, and tell the other threads
		// to quit.
		Cerr << "Too many processes (" << n << ") in wait_for_one\n";
		Cerr << "Current limit on processes = " << MAXIMUM_WAIT_OBJECTS << "\n";
		abort_handler(-1);
		}
	i = WaitForMultipleObjects(n, h, 0, INFINITE);
 ret:
	
	if (i >= WAIT_OBJECT_0 && i < WAIT_OBJECT_0 + n) {
		*pi = i - WAIT_OBJECT_0;
		return 1;
		}
	Cerr << "Surprise return " << i << " from WaitForMultipleObjects\n";
	abort_handler(-1);
	return 0;
	}
#endif

#ifdef HAVE_SYS_WAIT_H
static void pid_botch()
{
  Cerr << "\nError in fork retrieving child; error code " << errno << " (" 
       << std::strerror(errno) << ")\n   Consider using system interface"
       << std::endl;
  abort_handler(1);
}
#endif

void ForkApplicInterface::
derived_synch(PRPQueue& prp_queue)
{
  // Check for return of process id's corresponding to those stored in PRPairs.
  // Wait for at least one completion and complete all jobs that have returned.
  // This satisifies a "fairness" principle, in the sense that a completed job
  // will _always_ be processed (whereas accepting only a single completion 
  // could always accept the same completion - the case of very inexpensive fn.
  // evals. - and starve some servers).

#ifdef HAVE_SYS_WAIT_H
  int status;
  // wait for any (1st level) child process to finish.  No need for
  // usleep in derived_synch since wait provides a system optimized
  // test facility.
  pid_t pid = wait(&status); 
  if (pid == -1)
    pid_botch();
  do { // Perform this loop at least once for the pid from wait.
    forkSimulator.check_status(status); // check the exit status
    derived_synch_kernel(prp_queue, pid);
  } while( (pid=waitpid((pid_t)-1, &status, WNOHANG)) > 0 ); // Check for any
  // additional completed pid's.  This satisfies the fairness requirement & is 
  // particularly useful in the case of inexpensive fn. evals.
#elif defined(_WIN32)
	DWORD dw;
	HANDLE *h;
	int req;
	size_t i, n;

	if ((h = wait_setup(&processIdMap, &n))) {
		req = 1;
		while(wait_for_one(n,h,req,&i)) {
			GetExitCodeProcess(h[i], &dw);
			forkSimulator.check_status((int)dw);
			derived_synch_kernel(prp_queue, (pid_t)h[i]);
			CloseHandle(h[i]);
			if (i < --n)
				h[i] = h[n];
			else if (!n)
				break;
			req = 0;
			}
		free(h);
		}
#endif // HAVE_SYS_WAIT_H
}


void ForkApplicInterface::
derived_synch_nowait(PRPQueue& prp_queue)
{
  // Check for return of process id's corresponding to those stored in PRPairs.
  // Do not wait - complete all jobs that are immediately available.

#ifdef HAVE_SYS_WAIT_H
  int status;
  pid_t pid;
  while( (pid=waitpid((pid_t)-1, &status, WNOHANG)) > 0 ) {
    forkSimulator.check_status(status); // check the exit status
    derived_synch_kernel(prp_queue, pid);
  }
#elif defined(_WIN32)
	DWORD dw;
	HANDLE *h;
	size_t i, n;

	if ((h = wait_setup(&processIdMap, &n))) {
		while(wait_for_one(n,h,0,&i)) {
			GetExitCodeProcess(h[i], &dw);
			forkSimulator.check_status((int)dw);
			derived_synch_kernel(prp_queue, (pid_t)h[i]);
			CloseHandle(h[i]);
			if (i < --n)
				h[i] = h[n];
			else if (!n)
				break;
			}
		free(h);
		}
#endif // HAVE_SYS_WAIT_H


  // reduce processor load from DAKOTA testing if jobs are not finishing
  if (completionSet.empty()) {
#ifdef HAVE_USLEEP
    usleep(1000); // 1000 microseconds = 1 millisec
#elif defined(_WIN32)
    Sleep(2);
#else
    Cerr << "Error: usleep is not supported under this interface with this OS."
	 << std::endl;
    abort_handler(-1);
#endif // HAVE_UNISTD_H
  }
}


void ForkApplicInterface::
derived_synch_kernel(PRPQueue& prp_queue, const pid_t pid)
{
  // Convenience function for common code between derived_synch() &
  // derived_synch_nowait() cases

  // Map pid to map_iter to fn_eval_id (using processIdMap[pid] does
  // the wrong thing if the pid key is not found)
  std::map<pid_t, int>::iterator map_iter = processIdMap.find(pid);
  if (map_iter == processIdMap.end()) {
    // should not happen so long as wait ignores any 2nd level child processes
    Cerr << "Error: pid returned from wait does not match any 1st level child "
	 << "process." << std::endl;
    abort_handler(-1);
  }
  int fn_eval_id = map_iter->second;

  // now populate the corresponding response by reading the results file 
  ParamResponsePair pr_pair;
  bool found = lookup_by_eval_id(prp_queue, fn_eval_id, pr_pair);
  if (!found) {
    Cerr << "Error: failure in queue lookup within ForkApplicInterface::"
	 << "derived_synch_kernel()." << std::endl;
    abort_handler(-1);
  }
  Response response = pr_pair.prp_response(); // shallow copy
  try { 
    forkSimulator.read_results_files(response, fn_eval_id, 
				     final_eval_id_tag(fn_eval_id));
  }
  catch(std::string& err_msg) { // For forks, there is no potential for an 
    // file write race condition since the process has completed -> an 
    // exception involving an incomplete file/data set is a true error.
    Cerr << err_msg << std::endl;
    abort_handler(-1);
  }

  catch(int fail_code) { // If an int exception ("fail" detected in results 
    // file) is caught, call manage_failure which will either (1) repair the 
    // failure and populate response, or (2) abort the run.  NOTE: this 
    // destroys load balancing but trying to load balance failure recovery 
    // would be more difficult than it is worth.
    manage_failure(pr_pair.prp_parameters(), response.active_set(), response,
		   fn_eval_id);
  }

  // bookkeep the completed job
  //pr_pair.prp_response(response);                    // not needed for shallow
  //replace_by_eval_id(prp_queue, fn_eval_id, pr_pair);// not needed for shallow
  completionSet.insert(fn_eval_id);
  processIdMap.erase(pid);
}


/** Manage the input filter, 1 or more analysis programs, and the
    output filter in blocking or nonblocking mode as governed by
    block_flag.  In the case of a single analysis and no filters, a
    single fork is performed, while in other cases, an initial fork is
    reforked multiple times.  Called from derived_map() with
    block_flag == BLOCK and from derived_map_asynch() with block_flag
    == FALL_THROUGH.  Uses ForkAnalysisCode::fork_program() to spawn
    individual program components within the function evaluation. */
pid_t ForkApplicInterface::fork_application(const bool block_flag)
{
  // if a slave processor or if output verbosity is "silent", suppress output
  // from the ForkAnalysisCode instance.  Must be done at eval time rather
  // than in the constructor since dependent on Model::init_communicators().
  if (suppressOutput || outputLevel == SILENT_OUTPUT)
    forkSimulator.suppress_output_flag(true); // suppress fork echoes

  pid_t pid = 0;
  const std::string& ifilter_name = forkSimulator.input_filter_name();
  const std::string& ofilter_name = forkSimulator.output_filter_name();
  const char *s;
  size_t i;
  if (evalCommRank == 0 && !suppressOutput) {
    if (block_flag) {
      if (eaDedMasterFlag)
        Cout << "blocking fork self-schedule: ";
      else if (numAnalysisServers > 1)
        Cout << "blocking fork static schedule: ";
      else
        Cout << "blocking fork: ";
    }
    else
      Cout << "nonblocking fork: ";

    const std::vector<String>& prog_names = forkSimulator.program_names();
    const std::string& params_filename = forkSimulator.parameters_filename();
    const std::string& res_filename = forkSimulator.results_filename();
    bool cmd_line_args = forkSimulator.command_line_arguments();
    if (!ifilter_name.empty()) {
      Cout << ifilter_name;
      if (cmd_line_args)
	Cout << ' ' << params_filename << ' ' << res_filename;
      Cout << "; ";
    }
    for (i=0; i<numAnalysisDrivers; i++) {
      Cout << prog_names[i];
      if (cmd_line_args) {
	Cout << ' ' << params_filename;
	if (forkSimulator.multiple_parameters_filenames())
	  Cout << '.' << i+1;
	Cout << ' ' << res_filename;
	if (numAnalysisDrivers > 1)
	  Cout << '.' << i+1;
      }
      if (i != numAnalysisDrivers-1)
        Cout << "; ";
    }
    if (!ofilter_name.empty()) {
      Cout << "; " << ofilter_name;
      if (cmd_line_args)
	Cout << ' ' << params_filename << ' ' << res_filename;
    }
    Cout << '\n';
  }
  // Cout must be flushed prior to the fork to clear the stdout buffer.
  // Otherwise, the intermediate process receives a copy of the contents of
  // this buffer and outputs the contents on the next buffer flush.
  Cout << std::flush;

  if (ifilter_name.empty() && ofilter_name.empty() && numAnalysisDrivers == 1) {
    // fork the one-piece interface directly (no intermediate process required)
    forkSimulator.driver_argument_list(1);
    pid = forkSimulator.fork_program(block_flag);
  }
  else if (evalCommSize > 1) {
    // run a blocking schedule of single-proc. analyses over analysis servers.
    // The schedules are executed by the parent processes.  Forks are not used
    // at this level since the message passing provides the needed asynchrony
    // at the evaluation level (unlike the final case below where 2 levels of
    // forks must be used to provide asynchrony at the eval level).

    if (!block_flag) {
      Cerr << "Error: multiprocessor evalComm does not support nonblocking "
	   << "ForkApplicInterface::fork_application." << std::endl;
      abort_handler(-1);
    }

    if (!ifilter_name.empty() && evalCommRank == 0) {
      forkSimulator.ifilter_argument_list();
      forkSimulator.fork_program(BLOCK);
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

    if (!ofilter_name.empty() && evalCommRank == 0) {
      forkSimulator.ofilter_argument_list();
      forkSimulator.fork_program(BLOCK);
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

    // vfork should only be used when followed immediately by an exec since 
    // vfork borrows the parent process and only returns control to the parent
    // when one of the functions from the exec() or exit() family is 
    // encountered.  Therefore, since we want this intermediate process to be 
    // able to execute concurrently with the parent dakota and other asynch
    // processes, fork should be used here since there is no matching exec().
#ifdef HAVE_WORKING_FORK
    // Note: working fork is necessary but not sufficient.
    // Need a better autoconf test for reforking of a forked process.
    if (!block_flag)
      pid = fork();
#else
    Cerr << "Error: fork is not supported under this OS." << std::endl;
    abort_handler(-1);
#endif

    if (pid == 0) { // if nonblocking, then this is the intermediate (1st level
      // child) process.  If blocking, then no fork has yet been performed, and
      // this is the parent (default pid == 0).

      // run the input filter by reforking the child process (2nd level child).
      // This refork is always blocking.  The ifilter is used just once per
      // evaluation since it is responsible for non-replicated pre-processing.
      // Any replicated pre-processing must be part of the analysis drivers
      // (see DirectApplicInterface::derived_map for additional info).
      if (!ifilter_name.empty()) {
        forkSimulator.ifilter_argument_list();
        forkSimulator.fork_program(BLOCK);
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
      if (!ofilter_name.empty()) {
        forkSimulator.ofilter_argument_list();
        forkSimulator.fork_program(BLOCK);
      }

      // If nonblocking, then this is the 1st level child process.  Quit this
      // process now.
      if (!block_flag)
        _exit(1);
    }
  }

  return(pid);
}


// --------------------------------------------------------------
// Begin analysis level schedulers (eventually derived
// fns for ApplicationInterface analysis schedulers)
//
// A simple list utility function, removeAt(index) is useful here
// --------------------------------------------------------------

template <class ListT>
void removeAt(ListT& l, typename ListT::size_type index)
{
  if (index >= l.size()) {
    Cerr << "Error: index passed to removeAt(std::list,index) is out of bounds."
         << std::endl;
    abort_handler(-1);
  }
  typename ListT::iterator it = l.begin();
  std::advance(it, index);
  l.erase(it);
}

/** Schedule analyses asynchronously on the local processor using a
    self-scheduling approach (start to end in step increments).
    Concurrency is limited by asynchLocalAnalysisConcurrency.  Modeled
    after ApplicationInterface::asynchronous_local_evaluations().
    NOTE: This function should be elevated to ApplicationInterface if
    and when another derived interface class supports asynchronous
    local analyses. */
void ForkApplicInterface::
asynchronous_local_analyses(const int& start, const int& end, const int& step)
{
  std::list<pid_t> pid_list;         // list of process id's for asynch jobs
  std::list<int>   analysis_id_list; // list of analysis id's for asynch jobs
  size_t i, num_sends;
  if (numAnalysisDrivers <= 1) {
    Cerr << "Error: ForkApplicInterface::asynchronous_local_analyses() should "
	 << "only be called for multiple analysis_drivers." << std::endl;
    abort_handler(-1);
  }
  int analysis_id, num_jobs = 1 + (int)((end-start)/step);

  if (asynchLocalAnalysisConcurrency)  // concurrency limited by user
    num_sends = std::min(asynchLocalAnalysisConcurrency, num_jobs);
  else // default: no limit, launch all jobs in first pass
    num_sends = num_jobs; // don't need to limit num_sends to 1 in the message
    // passing case since this fn is only called by the message passing
    // schedulers if there is asynchLocalAnalysisConcurrency

#ifdef MPI_DEBUG
  Cout << "First pass: initiating " << num_sends << " asynchronous analyses\n";
#endif // MPI_DEBUG
  for (i=0; i<num_sends; ++i) {
    analysis_id = start + i*step;
#ifdef MPI_DEBUG
    Cout << "Initiating analysis " << analysis_id << std::endl; // flush buffer
#endif // MPI_DEBUG
    forkSimulator.driver_argument_list(analysis_id);
    pid_t pid = forkSimulator.fork_program(FALL_THROUGH);
    pid_list.push_back(pid);
    analysis_id_list.push_back(analysis_id);
  }

#ifdef MPI_DEBUG
  if (num_sends < num_jobs)
    Cout << "Second pass: self-scheduling " << num_jobs-num_sends 
         << " remaining analyses\n";
#endif // MPI_DEBUG
  size_t send_cntr = num_sends, recv_cntr = 0;
  while (recv_cntr < num_jobs) {
#ifdef MPI_DEBUG
    Cout << "Waiting on completed analyses" << std::endl;
#endif // MPI_DEBUG

#ifdef HAVE_SYS_WAIT_H
    // Enforce scheduling fairness with a Waitsome design
    int status, completed = 0;
    pid_t pid = wait(&status); // wait for any analysis to finish
    if (pid == -1)
      pid_botch();
    do { // perform this loop at least once for the pid from wait.
      forkSimulator.check_status(status); // check the exit status
      completed++;
      size_t index = find_index(pid_list, pid);
#ifdef MPI_DEBUG
      Cout << "Analysis " << analysis_id_list[index] << " has completed"
	   << std::endl;
#endif // MPI_DEBUG
      removeAt(pid_list, index);
      removeAt(analysis_id_list, index);
    } while( (pid=waitpid((pid_t)-1, &status, WNOHANG)) > 0 ); // any additional
#elif defined(_WIN32)
	DWORD dw;
	HANDLE *h;
	int completed, req;
	size_t i, j, n;

	if ((h = wait_setup(&processIdMap, &n))) {
		req = 1;
		completed = 0;
		while(wait_for_one(n,h,req,&i)) {
			GetExitCodeProcess(h[i], &dw);
			forkSimulator.check_status((int)dw);
			++completed;
			j = find_index(pid_list, (pid_t)h[i]);
#ifdef MPI_DEBUG
			Cout << "Analysis " << analysis_id_list[j] << " has completed"
			<< std::endl;
#endif // MPI_DEBUG
			removeAt(pid_list, j);
			removeAt(analysis_id_list, j);
			CloseHandle(h[i]);
			if (i < --n)
				h[i] = h[n];
			else if (!n)
				break;
			req = 0;
			}
		free(h);
		}
#endif
#if defined(HAVE_SYS_WAIT_H) || defined(_WIN32)
    recv_cntr += completed;
    for (i=0; i<completed; ++i) {
      if (send_cntr < num_jobs) {
        analysis_id = start + send_cntr*step;
#ifdef MPI_DEBUG
        Cout << "Initiating analysis " << analysis_id << std::endl; // flush buf
#endif // MPI_DEBUG
	forkSimulator.driver_argument_list(analysis_id);
        pid_t pid = forkSimulator.fork_program(FALL_THROUGH);
        pid_list.push_back(pid);
        analysis_id_list.push_back(analysis_id);
        send_cntr++;
      }
    }
#endif // HAVE_SYS_WAIT_H
  }
}


/** This code runs multiple asynch analyses on each server.  It is
    modeled after ApplicationInterface::serve_evaluations_asynch().
    NOTE: This fn should be elevated to ApplicationInterface if and
    when another derived interface class supports hybrid analysis
    parallelism. */
void ForkApplicInterface::serve_analyses_asynch()
{
  if (numAnalysisDrivers <= 1) {
    Cerr << "Error: ForkApplicInterface::serve_analyses_asynch should only be "
	 << "called for multiple analysis_drivers." << std::endl;
    abort_handler(-1);
  }
  pid_t pid;
  int analysis_id;
  std::list<pid_t> pid_list; // list of process id's for asynch jobs
  std::list<int>   analysis_id_list; // corresponding list of analysis id's
  MPI_Status  status; // holds MPI_SOURCE, MPI_TAG, & MPI_ERROR
  MPI_Request recv_request = MPI_REQUEST_NULL;

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
           pid_list.size() < asynchLocalAnalysisConcurrency) {
      // test for completion
      if (recv_request)
        parallelLib.test(recv_request, mpi_test_flag, status);

      // if test indicates a completion: unpack, execute, & repost
      if (mpi_test_flag) {
        analysis_id = status.MPI_TAG;

        if (analysis_id) {
	  // execute
	  forkSimulator.driver_argument_list(analysis_id);
          pid = forkSimulator.fork_program(FALL_THROUGH);
          pid_list.push_back(pid);
          analysis_id_list.push_back(analysis_id);
	  // repost
          parallelLib.irecv_ea(analysis_id, 0, MPI_ANY_TAG, recv_request);
	}
      }
    }

    // -----------------------------------------------------------------
    // Step 3: check for any completed jobs and return results to master
    // -----------------------------------------------------------------
#ifdef HAVE_SYS_WAIT_H
    if (pid_list.size()) {
      int wait_status, rtn_code = 0;
      while( (pid=waitpid((pid_t)-1, &wait_status, WNOHANG)) > 0 ) {
        forkSimulator.check_status(wait_status); // check the exit status
        size_t index = find_index(pid_list, pid);
        ILIter anal_iter = analysis_id_list.begin();
        std::advance(anal_iter, index);
        analysis_id = *anal_iter;
#ifdef MPI_DEBUG
        Cout << "Analysis " << analysis_id << " has completed"
             << std::endl; // flush buffer
#endif // MPI_DEBUG
	// In this case, use a blocking send to avoid having to manage waits on
	// multiple send buffers (which would be a pain since the number of
	// send_buffers would vary with num_completed).
        parallelLib.send_ea(rtn_code, 0, analysis_id);
        removeAt(pid_list, index);
        removeAt(analysis_id_list, index);
      }
    }
#elif defined(_WIN32)
    if (pid_list.size()) {
	DWORD dw;
	HANDLE *h;
	int rtn_code = 0;
	size_t i, j, n;

	if ((h = wait_setup(&processIdMap, &n))) {
		while(wait_for_one(n,h,0,&i)) {
			GetExitCodeProcess(h[i], &dw);
			forkSimulator.check_status((int)dw);
			j = find_index(pid_list, (int) dw);
			ILIter anal_iter = analysis_id_list.begin();
			std::advance(anal_iter, j);
			analysis_id = *anal_iter;
#ifdef MPI_DEBUG
			Cout << "Analysis " << analysis_id << " has completed" << std::endl;
#endif // MPI_DEBUG
			// In this case, use a blocking send to avoid having to manage waits on
			// multiple send buffers (which would be a pain since the number of
			// send_buffers would vary with num_completed).
			parallelLib.send_ea(rtn_code, 0, analysis_id);
			removeAt(pid_list, j);
			removeAt(analysis_id_list, j);
			CloseHandle(h[i]);
			if (i < --n)
				h[i] = h[n];
			else if (!n)
				break;
			}
		free(h);
		}
      }
#endif // HAVE_SYS_WAIT_H

  } while (analysis_id || pid_list.size());
}

} // namespace Dakota
