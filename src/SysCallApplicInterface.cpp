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
#include <sys/types.h> // MAY REQUIRE ifndef(HPUX)
#include <sys/stat.h>

// eventually just use _WIN32 here
#if defined(_WIN32) || defined(_MSC_VER) || defined(__MINGW32__)
#define NOMINMAX
#include <windows.h> // for Sleep()
#elif defined(HAVE_UNISTD_H)
#include <unistd.h> // for usleep()
#endif

#include <boost/lexical_cast.hpp>


namespace Dakota {

SysCallApplicInterface::
SysCallApplicInterface(const ProblemDescDB& problem_db):
  ApplicationInterface(problem_db), sysCallSimulator(problem_db)
{ }


void SysCallApplicInterface::
derived_map(const Variables& vars, const ActiveSet& set, Response& response,
	    int fn_eval_id)
{
  // This function may be executed by a multiprocessor evalComm.

  sysCallSimulator.define_filenames(final_eval_id_tag(fn_eval_id)); // all of evalComm
  if (evalCommRank == 0)
    sysCallSimulator.write_parameters_files(vars, set, response, fn_eval_id);

  spawn_application(BLOCK);

  try { 
    if (evalCommRank == 0)
      sysCallSimulator.read_results_files(response, fn_eval_id,
					  final_eval_id_tag(fn_eval_id));
  }

  catch(std::string& err_msg) {
    // a std::string exception involves detection of an incomplete file/data
    // set.  In the synchronous case, there is no potential for an incomplete 
    // file resulting from a race condition -> echo the error and abort.
    Cerr << err_msg << std::endl;
    abort_handler(-1);
  }

  // Do not call manage_failure() from the following catch since the recursion 
  // of calling derived_map again would be confusing at best.  The approach 
  // here is to have catch(int) rethrow the exception to an outer catch (either
  // the catch within manage_failure or a catch that calls manage_failure).  An
  // alternative solution would be to eliminate the try block above and move 
  // all catches to the higher level of try { derived_map() } - this would be
  // simpler to understand but would replicate catch(std::string) in map, 
  // manage_failure, and serve.  By having catch(std::string) here and having
  // catch(int) rethrow, we eliminate unnecessary proliferation of 
  // catch(std::string).
  catch(int fail_code) { // failure capture exception thrown by response.read()
    //Cout << "Rethrowing int." << std::endl;
    throw; // from this catch to the outer one in manage_failure
  }
}


void SysCallApplicInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  // This function may not be executed by a multiprocessor evalComm.

  int fn_eval_id = pair.eval_id();
  sysCallSimulator.define_filenames(final_eval_id_tag(fn_eval_id));
  sysCallSimulator.write_parameters_files(pair.prp_parameters(),
					  pair.active_set(),
					  pair.prp_response(), fn_eval_id);

  spawn_application(FALL_THROUGH);

  sysCallSet.insert(fn_eval_id);
}


void SysCallApplicInterface::spawn_application(bool block_flag)
{
  // if a slave processor or if output verbosity is "silent", suppress output
  // from the SysCallAnalysisCode instance.  Must be done at eval time rather
  // than in the constructor since dependent on Model::init_communicators().
  if (suppressOutput || outputLevel == SILENT_OUTPUT)
    sysCallSimulator.suppress_output_flag(true); // suppress system call echoes

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
  // ForkApplicInterface/ForkAnalysisCode already provide the required
  // capabilities in a straightforward fashion) and local asynch analysis
  // concurrency will not be attempted in the system call case (although 
  // message passing concurrency will be supported).

  if (evalCommSize > 1) { // run a blocking schedule of single-proc. analyses
                          // over analysis servers
    if (!block_flag) {
      Cerr << "Error: multiprocessor evalComm does not support nonblocking "
	   << "SysCallApplicInterface::spawn_application." << std::endl;
      abort_handler(-1);
    }

    const std::string& ifilter_name = sysCallSimulator.input_filter_name();
    if (!ifilter_name.empty() && evalCommRank == 0)
      sysCallSimulator.spawn_input_filter(BLOCK);

    int i;
    if (evalCommRank == 0 && !suppressOutput) {
      if (eaDedMasterFlag)
        Cout << "System call: self-scheduling { ";
      else
        Cout << "System call: static scheduling { ";
      const std::vector<String>& prog_names = sysCallSimulator.program_names();
      for (i=0; i<numAnalysisDrivers; i++)
        Cout << prog_names[i] << ' ';
      Cout << "} among " << numAnalysisServers << " analysis servers"
           << std::endl;
    }

    // Schedule analyses using either master-slave/dynamic or peer/static
    bool output_setting = sysCallSimulator.suppress_output_flag();
    sysCallSimulator.suppress_output_flag(true); // turn off sys call output
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
        sysCallSimulator.spawn_analysis(i, BLOCK);

      // avoid peer 1 reading all the results files before peers 2-n have
      // completed writing them
      parallelLib.barrier_e();
    }
    sysCallSimulator.suppress_output_flag(output_setting); // rtn to prev. value

    const std::string& ofilter_name = sysCallSimulator.output_filter_name();
    if (!ofilter_name.empty() && evalCommRank == 0)
      sysCallSimulator.spawn_output_filter(BLOCK);
  }
  else // launch entire fn eval in a single system call on the local processor
    // An asynchronous_local_analyses scheduler is not supported because of the
    // difficulty in detecting analysis completion (there is not a user
    // specified results file in all cases).
    sysCallSimulator.spawn_evaluation(block_flag);
}


void SysCallApplicInterface::derived_synch_kernel(PRPQueue& prp_queue)
{
  // Convenience function for common code between wait and nowait case.

  for (ISIter it=sysCallSet.begin(); it!=sysCallSet.end(); ++it) {

    // Identify the corresponding PRPair
    int fn_eval_id = *it;
    bool err_msg_caught = false;

    // Test for existence of the results file(s) corresponding to this PRPair
    const std::string& file_to_test
      = sysCallSimulator.results_filename(fn_eval_id);
    if (system_call_file_test(file_to_test)) {
      // File exists; test for complete/valid set of results (an incomplete 
      // set can result from a race condition in which Dakota is reading a 
      // file that a simulator has not finished writing).  Response::read
      // throws a std::string exception if data is missing/misformatted.
      ParamResponsePair pr_pair;
      bool found = lookup_by_eval_id(prp_queue, fn_eval_id, pr_pair);
      if (!found) {
	Cerr << "Error: failure in queue lookup within SysCallApplicInterface::"
	     << "derived_synch_kernel()." << std::endl;
	abort_handler(-1);
      }
      Response response = pr_pair.prp_response(); // shallow copy

      try {
	sysCallSimulator.read_results_files(response, fn_eval_id,
					    final_eval_id_tag(fn_eval_id));
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
        //Cout << "Caught int in derived_synch." << std::endl;
        manage_failure(pr_pair.prp_parameters(), response.active_set(),
		       response, fn_eval_id);
      }

      if (!err_msg_caught) {
        // Successful processing of results for this asynchronous eval.  Set
        // the response within the PRPair, remove entry in failCountMap, and
        // add evaluation id to completion set.
        //Cout << "Evaluation " << fn_eval_id << " captured.\n";
	//pr_pair.prp_response(response); // not needed for shallow copy
	//replace_by_eval_id(prp_queue, fn_eval_id, pr_pair); // not needed
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


bool SysCallApplicInterface::system_call_file_test(const std::string& root_file)
{
  // Unix stat utility returns 0 if successful in gathering file statistics,
  // -1 if there's an error (e.g., the file does not exist).
  struct stat buf; // see man pages for info available from buf (not used here)
  size_t num_progs = sysCallSimulator.program_names().size();
  if ( num_progs > 1 && sysCallSimulator.output_filter_name().empty() ) {
#ifdef __SUNPRO_CC
    // Sun Solaris has been observed to have problems with the final results
    // file existing before previous results files exist (I/O threading?)
    for (size_t i=0; i<num_progs; ++i) {
      std::string tagged_file =   root_file + "."
                                + boost::lexical_cast<std::string>(i+1);
      if ( stat((char*)tagged_file.data(), &buf) == -1 )
        return false;
    }
    return true;
#else
    // Testing all files is usually overkill for sequential analyses.  It's only
    // really necessary to check the last tagged_file: root_file.[num_progs]
    std::string tagged_file =   root_file + "."
                              + boost::lexical_cast<std::string>(num_progs);
    return ( stat((char*)tagged_file.data(), &buf) == -1 ) ? false : true;
#endif // __SUNPRO_CC
  }
  else
    return ( stat((char*)root_file.data(), &buf) == -1 ) ? false : true;
}

} // namespace Dakota
