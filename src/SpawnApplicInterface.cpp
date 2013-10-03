/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SpawnApplicInterface
//- Description:  Class implementation
//- Owner:        Dave Gay / Bill Bohnhoff

#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "SpawnApplicInterface.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#define NOMINMAX
#include <process.h>
#include <windows.h>
#include <algorithm>


namespace Dakota {

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


void SpawnApplicInterface::
derived_synch(PRPQueue& prp_queue)
{
  // Check for return of process id's corresponding to those stored in PRPairs.
  // Wait for at least one completion and complete all jobs that have returned.
  // This satisifies a "fairness" principle, in the sense that a completed job
  // will _always_ be processed (whereas accepting only a single completion 
  // could always accept the same completion - the case of very inexpensive fn.
  // evals. - and starve some servers).

	DWORD dw;
	HANDLE *h;
	int req;
	size_t i, n;

	if ((h = wait_setup(&evalProcessIdMap, &n))) {
		req = 1;
		while (wait_for_one(n,h,req,&i)) {
			GetExitCodeProcess(h[i], &dw);
			check_wait((pid_t)h[i], (int)dw);
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
}


void SpawnApplicInterface::
derived_synch_nowait(PRPQueue& prp_queue)
{
  // Check for return of process id's corresponding to those stored in PRPairs.
  // Do not wait - complete all jobs that are immediately available.

	DWORD dw;
	HANDLE *h;
	size_t i, n;

	if ((h = wait_setup(&evalProcessIdMap, &n))) {
		while (wait_for_one(n,h,0,&i)) {
			GetExitCodeProcess(h[i], &dw);
			check_wait((pid_t)h[i], (int)dw);
			derived_synch_kernel(prp_queue, (pid_t)h[i]);
			CloseHandle(h[i]);
			if (i < --n)
				h[i] = h[n];
			else if (!n)
				break;
			}
		free(h);
		}


  // reduce processor load from DAKOTA testing if jobs are not finishing
  if (completionSet.empty())
    Sleep(2);
}


pid_t ProcessHandleApplicInterface::
create_analysis_process(bool block_flag, bool new_group)
{
  const char *arg_list[4], **av;
  int status;
  pid_t pid = 0;
  static std::string no_workdir;

  av = WorkdirHelper::arg_adjust(commandLineArgs, argList, arg_list,
				 useWorkdir ? curWorkdir : no_workdir);
  if (block_flag) status = _spawnvp(  _P_WAIT, av[0], av);
  else               pid = _spawnvp(_P_NOWAIT, av[0], av);

  if (curWorkdir.c_str())
    WorkdirHelper::reset();

  return(pid);
}


size_t SpawnApplicInterface::
wait_local_analyses(std::map<pid_t, int>& proc_analysis_id_map)
{
	DWORD dw;
	HANDLE *h;
	int req;
	size_t i, j, n, completed = 0;

	if ((h = wait_setup(&evalProcessIdMap, &n))) { // *** BUG?
		req = 1;
		while (wait_for_one(n,h,req,&i)) {
			GetExitCodeProcess(h[i], &dw);
			check_wait((pid_t)h[i], (int)dw);
			std::map<pid_t, int>::iterator an_it = proc_analysis_id_map.find((pid_t)h[i]);
			if (an_it == proc_analysis_id_map.end()) {
			  Cerr << "Error: analysis completion does not match local process ids "
			       << "within SpawnApplicInterface::wait_local_analyses()."
			       << std::endl;
			  abort_handler(-1);
			}
#ifdef MPI_DEBUG
			Cout << "Analysis " << an_it->second << " has completed"
			     << std::endl;
#endif // MPI_DEBUG
			proc_analysis_id_map.erase(an_it); ++completed;
			CloseHandle(h[i]);
			if (i < --n)
				h[i] = h[n];
			else if (!n)
				break;
			req = 0;
			}
		free(h);
		}

	return completed;
}


/** This code runs multiple asynch analyses on each server.  It is modeled
    after ApplicationInterface::serve_evaluations_asynch().  NOTE: This fn
    should be elevated to ApplicationInterface if and when another derived
    interface class supports hybrid analysis parallelism. */
size_t SpawnApplicInterface::
wait_local_analyses_send(std::map<pid_t, int>& proc_analysis_id_map,
			 int analysis_id)
{
	DWORD dw;
	HANDLE *h;
	int rtn_code = 0;
	size_t i, j, n, completed = 0;

	if ((h = wait_setup(&evalProcessIdMap, &n))) { // *** BUG?
		while (wait_for_one(n,h,0,&i)) {
			GetExitCodeProcess(h[i], &dw);
			check_wait((pid_t)h[i], (int)dw);

			std::map<pid_t, int>::iterator an_it = proc_analysis_id_map.find((pid_t)h[i]);
			if (an_it == proc_analysis_id_map.end()) {
			  Cerr << "Error: analysis completion does not match local process ids "
			       << "within SpawnApplicInterface::wait_local_analyses_send()."
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
			proc_analysis_id_map.erase(an_it); ++completed;
			CloseHandle(h[i]);
			if (i < --n) h[i] = h[n];
			else if (!n) break;
		}
		free(h);
	}

	return completed;
}

} // namespace Dakota
