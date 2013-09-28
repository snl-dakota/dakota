/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

#include "ApplicationInterface.hpp"
#include "SysCallAnalysisCode.hpp"


namespace Dakota {


/// Derived application interface class which spawns simulation codes
/// using system calls.

/** SysCallApplicInterface uses a SysCallAnalysisCode object for
    performing simulation invocations. */

class SysCallApplicInterface: public ApplicationInterface
{
public:

  //
  //- Heading: Constructor and destructor
  //

  SysCallApplicInterface(const ProblemDescDB& problem_db); ///< constructor
  ~SysCallApplicInterface();                               ///< destructor

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);

  void derived_map_asynch(const ParamResponsePair& pair);

  void derived_synch(PRPQueue& prp_queue);

  void derived_synch_nowait(PRPQueue& prp_queue);

  int  derived_synchronous_local_analysis(int analysis_id);

  const std::vector<String>& analysis_drivers() const;

  const AnalysisCode* analysis_code() const;

  void init_communicators_checks(int max_iterator_concurrency);

private:

  //
  //- Heading: Methods
  //

  /// Spawn the application by managing the input filter, analysis drivers,
  /// and output filter.  Called from derived_map() & derived_map_asynch().
  void spawn_application(bool block_flag);

  /// Convenience function for common code between derived_synch() &
  /// derived_synch_nowait()
  void derived_synch_kernel(PRPQueue& prp_queue);

  /// detect completion of a function evaluation through existence of
  /// the necessary results file(s)
  bool system_call_file_test(const std::string& root_file);

  //
  //- Heading: Data
  //

  /// SysCallAnalysisCode provides convenience functions for passing
  /// the input filter, the analysis drivers, and the output filter
  /// to a CommandShell in various combinations
  SysCallAnalysisCode sysCallSimulator;

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
inline void SysCallApplicInterface::derived_synch(PRPQueue& prp_queue)
{
  while (completionSet.empty()) // complete at least one job
    derived_synch_kernel(prp_queue);
}


/** Check for completion of active asynch jobs (tracked with sysCallSet).
    Make one pass through sysCallSet & complete all jobs that have returned. */
inline void SysCallApplicInterface::derived_synch_nowait(PRPQueue& prp_queue)
{ derived_synch_kernel(prp_queue); }


/** This code provides the derived function used by 
    ApplicationInterface::serve_analyses_synch(). */
inline int SysCallApplicInterface::
derived_synchronous_local_analysis(int analysis_id)
{
  sysCallSimulator.spawn_analysis(analysis_id, BLOCK);
  return 0; // used for failure codes in DirectFn case
}


inline const std::vector<String>&
SysCallApplicInterface::analysis_drivers() const
{ return sysCallSimulator.program_names(); }


inline const AnalysisCode* SysCallApplicInterface::analysis_code() const
{ return &sysCallSimulator; }


// define construct-time checks since no derived interface plug-ins
inline void SysCallApplicInterface::
init_communicators_checks(int max_iterator_concurrency)
{
  if (check_multiprocessor_analysis())
    abort_handler(-1);
}

} // namespace Dakota

#endif
