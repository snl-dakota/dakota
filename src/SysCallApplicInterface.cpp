/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SysCallApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "SysCallApplicInterface.hpp"
#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "CommandShell.hpp"
#include <sys/types.h> // MAY REQUIRE ifndef(HPUX)
#include <sys/stat.h>

// eventually just use _WIN32 here
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#include <dakota_windows.h> // for Sleep()
#elif defined(HAVE_UNISTD_H)
#include <unistd.h> // for usleep()
#endif

#include <boost/lexical_cast.hpp>


namespace Dakota {

SysCallApplicInterface::
SysCallApplicInterface(const ProblemDescDB& problem_db):
  ProcessApplicInterface(problem_db)
{ }


void SysCallApplicInterface::map_bookkeeping(pid_t pid, int fn_eval_id)
{ sysCallSet.insert(fn_eval_id); } // ignores pid


pid_t SysCallApplicInterface::create_evaluation_process(bool block_flag)
{
  // Check for erroneous concurrent analysis specification:
  if (asynchLocalAnalysisFlag && evalCommRank == 0 && evalServerId == 1)
    Cerr << "Warning: asynchronous analysis_drivers not supported in system "
	 << "call interfaces.\n         Concurrency request will be ignored.\n";
  // Analysis concurrency is a problem for SysCalls if there are no specified 
  // file names for detecting analysis completion (e.g., an oFilter maps
  // unspecified data files to results.out after all analyses have completed --
  // forks can handle this since they have process id's, but system calls need
  // files to test).  Also, even if all files are known (e.g., oFilter maps
  // results.out.[eval#].[1->numPrograms] to results.out.[eval#]), a fork would
  // be required to allow the (blocking) scheduling of concurrent analyses while
  // maintaining asynchrony at the fn eval level.  To accomplish the same thing
  // with a system call, a separate scheduler executable would be needed (or 
  // a nonblocking analysis scheduler, or a blended eval/analysis scheduler, or
  // if none of these, then the fn eval level would have to be blocking).  For
  // now, none of these options appear attractive (especially since
  // ForkApplicInterface already provides the required capabilities in a
  // straightforward fashion) and local asynch analysis concurrency will not be
  // attempted in the system call case (although message passing concurrency
  // will be supported).

  if (evalCommSize > 1) { // run a blocking schedule of single-proc. analyses
                          // over analysis servers
    if (!block_flag) {
      Cerr << "Error: multiprocessor evalComm does not support nonblocking "
	   << "SysCallApplicInterface::spawn_evaluation." << std::endl;
      abort_handler(-1);
    }

    if (!iFilterName.empty() && evalCommRank == 0)
      spawn_input_filter_to_shell(BLOCK);

    int i;
    if (evalCommRank == 0 && !suppressOutput) {
      if (eaDedMasterFlag)
        Cout << "System call: dynamic scheduling { ";
      else
        Cout << "System call: static scheduling { ";
      for (i=0; i<numAnalysisDrivers; i++)
        Cout << programNames[i] << ' ';
      Cout << "} among " << numAnalysisServers << " analysis servers"
           << std::endl;
    }

    // Schedule analyses using either master-slave/dynamic or peer/static
    bool output_setting = suppressOutput; // for restore below
    suppressOutput = true; // turn off sys call output
    if (eaDedMasterFlag) { // master-slave dynamic scheduling requires a
      // central pt of control & therefore needs separate schedule & serve fns.
      if (evalCommRank == 0)
        master_dynamic_schedule_analyses();
      else
        serve_analyses_synch();
    }
    else { // static scheduling does not require special schedule/serve fns
      // since it can distribute analyses using staggered spawn_analysis.
      // Barriers are required since there's no scheduler to enforce
      // synchronization.

      // avoid peers 2-n initiating analyses prior to completion of 
      // write_parameters_files() by peer 1
      parallelLib.barrier_e();

      for (i=analysisServerId; i<=numAnalysisDrivers; i+=numAnalysisServers)
        spawn_analysis_to_shell(i, BLOCK);

      // avoid peer 1 reading all the results files before peers 2-n have
      // completed writing them
      parallelLib.barrier_e();
    }
    suppressOutput = output_setting; // restore

    if (!oFilterName.empty() && evalCommRank == 0)
      spawn_output_filter_to_shell(BLOCK);
  }
  else // launch entire fn eval in a single system call on the local processor
    // An asynchronous_local_analyses scheduler is not supported because of the
    // difficulty in detecting analysis completion (there is not a user
    // specified results file in all cases).
    spawn_evaluation_to_shell(block_flag);

  return 0; // pid's not available for system calls
}


/** Check for completion of active asynch jobs (tracked with sysCallSet).
    Make one pass through sysCallSet & complete all jobs that have returned. */
void SysCallApplicInterface::test_local_evaluations(PRPQueue& prp_queue)
{
  // Convenience function for common code between wait and nowait case.

  for (ISIter it=sysCallSet.begin(); it!=sysCallSet.end(); ++it) {

    // Identify the corresponding PRPair
    int fn_eval_id = *it;
    bool err_msg_caught = false;

    // Test for existence of the results file(s) corresponding to this PRPair
    const std::string& file_to_test = fileNameMap[fn_eval_id].second;
    if (system_call_file_test(file_to_test)) {
      // File exists; test for complete/valid set of results (an incomplete 
      // set can result from a race condition in which Dakota is reading a 
      // file that a simulator has not finished writing).  Response::read
      // throws a std::string exception if data is missing/misformatted.
      PRPQueueIter queue_it = lookup_by_eval_id(prp_queue, fn_eval_id);
      if (queue_it == prp_queue.end()) {
	Cerr << "Error: failure in queue lookup within SysCallApplic"
	     << "Interface::test_local_evaluations()." << std::endl;
	abort_handler(-1);
      }
      Response response = queue_it->prp_response(); // shallow copy

      try {
	read_results_files(response, fn_eval_id, final_eval_id_tag(fn_eval_id));
      }

      // If a std::string exception (incomplete file) is caught, set 
      // err_msg_caught to true so that processing is not performed below.  
      // The for loop will then cycle through the other active asynch. evals.
      // before coming back to the one with the exception.  This should allow
      // file writing by a simulator to complete.  100 failures are currently
      // allowed for any fn_eval_id before it is assumed that the error is 
      // real (not race condition related) and aborting.
      catch(std::string& err_msg) {
        err_msg_caught = true;
	IntShMIter map_iter = failCountMap.find(fn_eval_id);
	if (map_iter != failCountMap.end()) {
          if (++map_iter->second > 100) {
            Cerr << "Error: too many failed reads for file " << file_to_test 
                 << "\n       check data format and completeness" << std::endl;
            abort_handler(-1);
          }
        }
        else
          failCountMap[fn_eval_id] = 1;
	// Test for MinGW first, since there we have usleep as well
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
	Sleep(1);     // 1 millisecond
#elif defined(HAVE_USLEEP)
        usleep(1000); // 1000 microseconds = 1 millisec
#endif // SLEEP
#ifdef ASYNCH_DEBUG
        Cerr << "Warning: exception caught in reading response file "
             << file_to_test << "\nException = \"" << err_msg
             << "\"\nException recovery: returning " << file_to_test 
             << " to processing queue.\n";
#endif
      }

      // If an int exception ("fail" detected in results file) is caught, 
      // call manage_failure which will either (1) repair the failure and 
      // populate response, or (2) abort the run.  NOTE: this destroys load 
      // balancing but trying to load balance failure recovery would be more
      // difficult than it is worth.
      catch(int fail_code) { // implemented at the derived class level since 
                             // DirectApplicInterface can do this w/o exceptions
        //Cout << "Caught int in test_local_evaluations()." << std::endl;
        manage_failure(queue_it->prp_parameters(), response.active_set(),
		       response, fn_eval_id);
      }

      if (!err_msg_caught) {
        // Successful processing of results for this asynchronous eval.  Set
        // the response within the PRPair, remove entry in failCountMap, and
        // add evaluation id to completion set.
        //Cout << "Evaluation " << fn_eval_id << " captured.\n";
	//queue_it->prp_response(response); // not needed for shallow copy
	//replace_by_eval_id(prp_queue, fn_eval_id, *queue_it); // not needed
        completionSet.insert(fn_eval_id);
	failCountMap.erase(fn_eval_id); // if present
      }
    }
  }

  // reduce processor load from DAKOTA testing if jobs are not finishing
  if (completionSet.empty()) { // no jobs completed in pass through entire set
    // Test for MinGW first, since there we have usleep as well
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
    Sleep(1);     // 1 millisecond
#elif defined(HAVE_USLEEP)
    usleep(1000); // 1000 microseconds = 1 millisec
#endif // SLEEP
  }
  // remove completed jobs from sysCallSet
  for (ISCIter it = completionSet.begin(); it != completionSet.end(); ++it)
    sysCallSet.erase(*it);
}


bool SysCallApplicInterface::
system_call_file_test(const std::string& root_file)
{
  // Unix stat utility returns 0 if successful in gathering file statistics,
  // -1 if there's an error (e.g., the file does not exist).
  struct stat buf; // see man pages for info available from buf (not used here)
  size_t num_programs = programNames.size();
  if ( num_programs > 1 && oFilterName.empty() ) {
#ifdef __SUNPRO_CC
    // Sun Solaris has been observed to have problems with the final results
    // file existing before previous results files exist (I/O threading?)
    for (size_t i=0; i<num_programs; ++i) {
      std::string tagged_file =   root_file + "."
                                + boost::lexical_cast<std::string>(i+1);
      if ( stat((char*)tagged_file.data(), &buf) == -1 )
        return false;
    }
    return true;
#else
    // Testing all files is usually overkill for sequential analyses.  It's only
    // really necessary to check the last tagged_file: root_file.[num_programs]
    std::string tagged_file =   root_file + "."
                              + boost::lexical_cast<std::string>(num_programs);
    return ( stat((char*)tagged_file.data(), &buf) == -1 ) ? false : true;
#endif // __SUNPRO_CC
  }
  else
    return ( stat((char*)root_file.data(), &buf) == -1 ) ? false : true;
}


/** Put the SysCallApplicInterface to the shell.  This function is
    used when all portions of the function evaluation (i.e., all analysis
    drivers) are executed on the local processor. */
void SysCallApplicInterface::spawn_evaluation_to_shell(bool block_flag)
{
  // MSE, 11/17/99: system call file passing changed to pass both files to all
  // 3 executables since: (1) in many cases, the OFilter will need to know the
  // asv, parameters, etc., and (2) in asynch usage, each of the 3 pieces must
  // be able to manage tagged files and/or working subdirectories.

  static std::string no_workdir;
  CommandShell shell(useWorkdir ? curWorkdir : no_workdir);
  const char* s = useWorkdir ? curWorkdir.c_str() : 0;
  size_t num_programs = programNames.size(),
    wd_strlen = useWorkdir ? curWorkdir.size() : 0;
  bool needparen;

  // Input filter portion
  if ((needparen = !block_flag &&
       (num_programs > 1 || !iFilterName.empty() || !oFilterName.empty())))
  	shell << "(";
  if (!iFilterName.empty()) {
    shell << iFilterName;
    if (commandLineArgs)
      shell << " " << paramsFileName << " " << resultsFileName;
    shell << "; ";
  }
  
  // Analysis code portion (function evaluation may be asynchronous, but
  // analyses must be sequential within each function evaluation)
  for (size_t i=0; i<num_programs; ++i) {
    shell << programNames[i];
    if (commandLineArgs) {
       const char* s1 = paramsFileName.c_str();
       if (s && !std::strncmp(s,s1,wd_strlen) && s1[wd_strlen] == '/')
		s1 += wd_strlen + 1;
      shell << " " << s1;
      std::string prog_num( (multipleParamsFiles || num_programs > 1) ?
                            "." + boost::lexical_cast<std::string>(i+1) : "" );
      if (multipleParamsFiles) // append program cntr to paramsFileName
	shell << prog_num;

      s1 = resultsFileName.c_str();
      if (s && !std::strncmp(s,s1,wd_strlen) && s1[wd_strlen] == '/')
		s1 += wd_strlen + 1;
      shell << " " << s1;
      if (num_programs > 1)     // append program cntr to resultsFileName
	shell << prog_num;
    }
    if (i != num_programs-1)
      shell << "; ";
  }

  // Output filter portion
  if (!oFilterName.empty()) {
    shell << "; " << oFilterName;
    if (commandLineArgs)
      shell << " " << paramsFileName << " " << resultsFileName;
  }
  if (needparen)
  	shell << ")"; // wasteful: needless extra shell layer

  // Process definition complete; now set the shell's asynchFlag and
  // suppressOutputFlag from the incoming block_flag & the interface's
  // suppressOutput and spawn the process.
  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutput);
  shell << flush;
}


/** Put the input filter to the shell.  This function is used when multiple
    analysis drivers are spread between processors.  No need to check for a
    Null input filter, as this is checked externally.  Use of nonblocking
    shells is supported in this fn, although its use is currently prevented
    externally. */
void SysCallApplicInterface::spawn_input_filter_to_shell(bool block_flag)
{
  static std::string no_workdir;
  CommandShell shell(useWorkdir ? curWorkdir : no_workdir);

  shell << iFilterName;
  if (commandLineArgs)
    shell << " " << paramsFileName << " " << resultsFileName;

  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutput);
  shell << flush;
}


/** Put a single analysis to the shell.  This function is used when
    multiple analysis drivers are spread between processors.  Use of
    nonblocking shells is supported in this fn, although its use is
    currently prevented externally. */
void SysCallApplicInterface::
spawn_analysis_to_shell(int analysis_id, bool block_flag)
{
  static std::string no_workdir;
  CommandShell shell(useWorkdir ? curWorkdir : no_workdir);

  shell << programNames[analysis_id-1];
  if (commandLineArgs) {
    using std::string;
    size_t num_programs = programNames.size();
    shell << " " << paramsFileName;
    string prog_num( (multipleParamsFiles || num_programs > 1) ?
                     "." + boost::lexical_cast<string>(analysis_id) : "" );
    if (multipleParamsFiles) // append program cntr to paramsFileName
      shell << prog_num;
    shell << " " << resultsFileName;
    if (num_programs > 1)     // append program cntr to resultsFileName
      shell << prog_num;
  }

  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutput);
  shell << flush;
}


/** Put the output filter to the shell.  This function is used when multiple
    analysis drivers are spread between processors.  No need to check for a
    Null output filter, as this is checked externally.  Use of nonblocking
    shells is supported in this fn, although its use is currently prevented
    externally. */
void SysCallApplicInterface::spawn_output_filter_to_shell(bool block_flag)
{
  static std::string no_workdir;
  CommandShell shell(useWorkdir ? curWorkdir : no_workdir);

  shell << oFilterName;
  if (commandLineArgs)
    shell << " " << paramsFileName << " " << resultsFileName;

  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutput);
  shell << flush;
}

} // namespace Dakota
