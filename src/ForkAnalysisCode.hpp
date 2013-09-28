/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkAnalysisCode
//- Description:  Specialization of AnalysisCode base class for forks
//- Owner:        Mike Eldred
//- Version: $Id: ForkAnalysisCode.hpp 7021 2010-10-12 22:19:01Z wjbohnh $

#ifndef FORK_ANALYSIS_CODE_H
#define FORK_ANALYSIS_CODE_H

#include "AnalysisCode.hpp"
#include <sys/types.h>


namespace Dakota {

/// Derived class in the AnalysisCode class hierarchy which spawns
/// simulations using forks.

/** ForkAnalysisCode creates a copy of the parent DAKOTA process using
    fork()/vfork() and then replaces the copy with a simulation
    process using execvp().  The parent process can then use waitpid()
    to wait on completion of the simulation process. */

class ForkAnalysisCode: public AnalysisCode
{
public:

  //
  //- Heading: Constructors and destructor
  //

  ForkAnalysisCode(const ProblemDescDB& problem_db); ///< constructor
  ~ForkAnalysisCode();                               ///< destructor

  //
  //- Heading: Member functions
  //

  /// spawn a child process using fork()/vfork()/execvp() and wait
  /// for completion using waitpid() if block_flag is true
  pid_t fork_analysis(bool block_flag, bool new_group);

  /// process all available completions within the nonblocking process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait_evaluation(bool block_flag);
  /// process all available completions within the nonblocking process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait_analysis(bool block_flag);

  /// create (if new_group) and join the process group for asynch evaluations
  void join_evaluation_process_group(bool new_group);
  /// create (if new_group) and join the process group for asynch analyses
  void join_analysis_process_group(bool new_group);

  /// set evalProcGroupId
  void evaluation_process_group_id(pid_t pgid);
  /// return evalProcGroupId
  pid_t evaluation_process_group_id() const;
  /// set analysisProcGroupId
  void analysis_process_group_id(pid_t pgid);
  /// return analysisProcGroupId
  pid_t analysis_process_group_id() const;

  /// check the exit status of a forked process and abort if an error code
  /// was returned
  void check_wait(pid_t pid, int status);
  /// check the exit status of setpgid and abort if an error code was returned
  void check_group(int err, pid_t proc_group_id);

  /// set argList for execution of the input filter
  void ifilter_argument_list();
  /// set argList for execution of the output filter
  void ofilter_argument_list();
  /// set argList for execution of the specified analysis driver
  void driver_argument_list(int analysis_id);

private:

  //
  //- Heading: Member functions
  //

  /// process all available completions within the nonblocking process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait(pid_t proc_group_id, bool block_flag);

  //
  //- Heading: Data
  //

  /// an array of strings for use with execvp(const char *, char * const *).
  /// These are converted to an array of const char*'s in fork_program().
  std::vector<std::string> argList;

  /// the process group id used to identify a set of child evaluation processes
  /// used by this interface instance (to distinguish from other interface
  /// instances that could be running at the same time)
  pid_t evalProcGroupId;
  /// the process group id used to identify a set of child analysis processes
  /// used by this interface instance (to distinguish from other interface
  /// instances that could be running at the same time)
  pid_t analysisProcGroupId;
};

inline ForkAnalysisCode::ForkAnalysisCode(const ProblemDescDB& problem_db) : 
  AnalysisCode(problem_db), argList(3)
{ }

inline ForkAnalysisCode::~ForkAnalysisCode()
{ }


inline pid_t ForkAnalysisCode::wait_evaluation(bool block_flag)
{ return wait(evalProcGroupId, block_flag); }


inline pid_t ForkAnalysisCode::wait_analysis(bool block_flag)
{ return wait(analysisProcGroupId, block_flag); }


inline void ForkAnalysisCode::evaluation_process_group_id(pid_t pgid)
{ evalProcGroupId = pgid; }


inline pid_t ForkAnalysisCode::evaluation_process_group_id() const
{ return evalProcGroupId; }


inline void ForkAnalysisCode::analysis_process_group_id(pid_t pgid)
{ analysisProcGroupId = pgid; }


inline pid_t ForkAnalysisCode::analysis_process_group_id() const
{ return analysisProcGroupId; }


inline void ForkAnalysisCode::ifilter_argument_list()
{
  argList[0] = iFilterName;
  argList[1] = paramsFileName;
  argList[2] = resultsFileName;
}


inline void ForkAnalysisCode::ofilter_argument_list()
{
  argList[0] = oFilterName;
  argList[1] = paramsFileName;
  argList[2] = resultsFileName;
}

} // namespace Dakota

#endif
