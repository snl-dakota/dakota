/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SysCallApplicInterface
//- Description:  Derived class for the case when analysis code simulators use
//-               system calls to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: SysCallApplicInterface.hpp 7004 2010-10-04 17:55:00Z wjbohnh $

#ifndef SYS_CALL_APPLIC_INTERFACE_H
#define SYS_CALL_APPLIC_INTERFACE_H

#include "ProcessApplicInterface.hpp"


namespace Dakota {


/// Derived application interface class which spawns simulation codes
/// using system calls.

/** system() is part of the C API and can be used on both Windows and
    Unix systems. */

class SysCallApplicInterface: public ProcessApplicInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  SysCallApplicInterface(const ProblemDescDB& problem_db); ///< constructor
  ~SysCallApplicInterface();                               ///< destructor

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void wait_local_evaluations(PRPQueue& prp_queue);
  void test_local_evaluations(PRPQueue& prp_queue);

  int synchronous_local_analysis(int analysis_id);

  void init_communicators_checks(int max_eval_concurrency);
  void set_communicators_checks(int max_eval_concurrency);

  void map_bookkeeping(pid_t pid, int fn_eval_id);

  pid_t create_evaluation_process(bool block_flag);

private:

  //
  //- Heading: Methods
  //

  /// detect completion of a function evaluation through existence of
  /// the necessary results file(s)
  bool system_call_file_test(const bfs::path& root_file);

  /// spawn a complete function evaluation
  void spawn_evaluation_to_shell(bool block_flag);
  /// spawn the input filter portion of a function evaluation
  void spawn_input_filter_to_shell(bool block_flag);
  /// spawn a single analysis as part of a function evaluation
  void spawn_analysis_to_shell(int analysis_id, bool block_flag);
  /// spawn the output filter portion of a function evaluation
  void spawn_output_filter_to_shell(bool block_flag);

  //
  //- Heading: Data
  //

  /// set of function evaluation id's for active asynchronous
  /// system call evaluations
  IntSet sysCallSet;
    
  /// map linking function evaluation id's to number of response read failures
  IntShortMap failCountMap; 
};


inline SysCallApplicInterface::~SysCallApplicInterface() 
{ /* Virtual destructor handles referenceCount at Interface level. */ }


/** Check for completion of active asynch jobs (tracked with sysCallSet).
    Wait for at least one completion and complete all jobs that have returned.
    This satisifies a "fairness" principle, in the sense that a completed job
    will _always_ be processed (whereas accepting only a single completion 
    could always accept the same completion - the case of very inexpensive fn.
    evals. - and starve some servers). */
inline void SysCallApplicInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  while (completionSet.empty()) // complete at least one job
    test_local_evaluations(prp_queue);
}


/** This code provides the derived function used by 
    ApplicationInterface::serve_analyses_synch(). */
inline int SysCallApplicInterface::synchronous_local_analysis(int analysis_id)
{
  spawn_analysis_to_shell(analysis_id, BLOCK);
  return 0; // used for failure codes in DirectFn case
}


/** No derived interface plug-ins, so perform construct-time checks.
    However, process init issues as warnings since some contexts (e.g.,
    HierarchSurrModel) initialize more configurations than will be used. */
inline void SysCallApplicInterface::
init_communicators_checks(int max_eval_concurrency)
{
  bool warn = true;
  check_multiprocessor_analysis(warn);
}


/** Process run-time issues as hard errors. */
inline void SysCallApplicInterface::
set_communicators_checks(int max_eval_concurrency)
{
  bool warn = false;
  if (check_multiprocessor_analysis(warn))
    abort_handler(-1);
}

} // namespace Dakota

#endif
