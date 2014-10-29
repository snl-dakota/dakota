/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ProcessHandleApplicInterface
//- Description:  Derived class for the case when analysis code simulators use
//-               fork\exec\wait to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ProcessHandleApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef PROCESS_HANDLE_APPLIC_INTERFACE_H
#define PROCESS_HANDLE_APPLIC_INTERFACE_H

#include "ProcessApplicInterface.hpp"
#include <boost/shared_array.hpp>

namespace Dakota {

/// Derived application interface class that spawns a simulation code
/// using a separate process, receives a process identifier, and
/// communicates with the spawned process through files.

/** ProcessHandleApplicInterface is subclassed for fork/execvp/waitpid
    (Unix) and spawnvp (Windows). */

class ProcessHandleApplicInterface: public ProcessApplicInterface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  ProcessHandleApplicInterface(const ProblemDescDB& problem_db);
  /// destructor
  ~ProcessHandleApplicInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  int synchronous_local_analysis(int analysis_id);

  void init_communicators_checks(int max_eval_concurrency);
  void set_communicators_checks(int max_eval_concurrency);

  void map_bookkeeping(pid_t pid, int fn_eval_id);

  pid_t create_evaluation_process(bool block_flag);

  //
  //- Heading: New virtual functions
  //

  /// spawn a child process for an analysis component within an evaluation
  virtual pid_t create_analysis_process(bool block_flag, bool new_group) = 0;

  /// wait for asynchronous analyses on the local processor, completing
  /// at least one job
  virtual size_t wait_local_analyses() = 0;

  /// test for asynchronous analysis completions on the local processor
  /// and return results for any completions by sending messages
  virtual size_t test_local_analyses_send(int analysis_id) = 0;

  /// create (if new_group) and join the process group for asynch evaluations
  virtual void join_evaluation_process_group(bool new_group);
  /// create (if new_group) and join the process group for asynch analyses
  virtual void join_analysis_process_group(bool new_group);

  /// set evalProcGroupId
  virtual void evaluation_process_group_id(pid_t pgid);
  /// return evalProcGroupId
  virtual pid_t evaluation_process_group_id() const;
  /// set analysisProcGroupId
  virtual void analysis_process_group_id(pid_t pgid);
  /// return analysisProcGroupId
  virtual pid_t analysis_process_group_id() const;

  //
  //- Heading: Methods
  //

  /// Common processing code used by {wait,test}_local_evaluations
  void process_local_evaluation(PRPQueue& prp_queue, const pid_t pid);

  //void clear_bookkeeping(); // virtual fn redefinition: clear processIdMap

  /// check the exit status of a forked process and abort if an error code
  /// was returned
  void check_wait(pid_t pid, int status);

  /// execute analyses asynchronously on the local processor
  void asynchronous_local_analyses(int start, int end, int step);

  /// serve the analysis scheduler and execute analysis jobs asynchronously
  void serve_analyses_asynch();

  /// set argList for execution of the input filter
  void ifilter_argument_list();
  /// set argList for execution of the output filter
  void ofilter_argument_list();
  /// set argList for execution of the specified analysis driver
  void driver_argument_list(int analysis_id);

  /// parse argList into argument array av suitable for passing to
  /// execvp, appending parameters and results filenames if requested
  /// by commandLineArgs
  void create_command_arguments(boost::shared_array<const char*>& av, 
				StringArray& driver_and_args); 


  //
  //- Heading: Data
  //

  /// map of fork process id's to function evaluation id's for asynchronous
  /// evaluations
  std::map<pid_t, int> evalProcessIdMap;
  /// map of fork process id's to analysis job id's for asynchronous analyses
  std::map<pid_t, int> analysisProcessIdMap;

  /// an array of strings for use with execvp(const char *, char * const *).
  /// These are converted to an array of const char*'s in fork_program().
  std::vector<std::string> argList;

private:

  //
  //- Heading: Data
  //

};


/** argList sized 3 for [driver name, input file, output file] */
inline ProcessHandleApplicInterface::
ProcessHandleApplicInterface(const ProblemDescDB& problem_db):
  ProcessApplicInterface(problem_db), argList(3)
{ }


inline ProcessHandleApplicInterface::~ProcessHandleApplicInterface() 
{ }


/** This code provides the derived function used by ApplicationInterface::
    serve_analyses_synch() as well as a convenience function for
    ProcessHandleApplicInterface::synchronous_local_analyses() below. */
inline int ProcessHandleApplicInterface::
synchronous_local_analysis(int analysis_id)
{
#ifdef MPI_DEBUG
  Cout << "Blocking fork to analysis " << analysis_id << std::endl; // flush buf
#endif // MPI_DEBUG
  driver_argument_list(analysis_id);
  create_analysis_process(BLOCK, false);
  return 0; // used for failure codes in DirectFn case
}


/** No derived interface plug-ins, so perform construct-time checks.
    However, process init issues as warnings since some contexts (e.g.,
    HierarchSurrModel) initialize more configurations than will be used. */
inline void ProcessHandleApplicInterface::
init_communicators_checks(int max_eval_concurrency)
{
  bool warn = true;
  check_multiprocessor_analysis(warn);
  check_multiprocessor_asynchronous(warn, max_eval_concurrency);
}


/** Process run-time issues as hard errors. */
inline void ProcessHandleApplicInterface::
set_communicators_checks(int max_eval_concurrency)
{
  bool warn = false, mp1 = check_multiprocessor_analysis(warn),
    mp2 = check_multiprocessor_asynchronous(warn, max_eval_concurrency);
  if (mp1 || mp2)
    abort_handler(-1);
}


//inline void ProcessHandleApplicInterface::clear_bookkeeping()
//{ evalProcessIdMap.clear(); }


inline void ProcessHandleApplicInterface::ifilter_argument_list()
{
  argList[0] = iFilterName;
  argList[1] = paramsFileName;
  argList[2] = resultsFileName;
}


inline void ProcessHandleApplicInterface::ofilter_argument_list()
{
  argList[0] = oFilterName;
  argList[1] = paramsFileName;
  argList[2] = resultsFileName;
}


inline void ProcessHandleApplicInterface::driver_argument_list(int analysis_id)
{
  std::string tag_str = "." + boost::lexical_cast<std::string>(analysis_id);
  argList[0] = programNames[analysis_id-1];
  argList[1] = (multipleParamsFiles) ? paramsFileName+tag_str : paramsFileName;
  argList[2] = (programNames.size() > 1) ? resultsFileName+tag_str :
    resultsFileName;

#ifdef DEBUG
  Cout << "argList: " << argList[0] << ' ' << argList[1] << ' ' << argList[2]
       << std::endl;
#endif
}

} // namespace Dakota

#endif
