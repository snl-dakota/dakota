/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkApplicInterface
//- Description:  Derived class for the case when analysis code simulators use
//-               vfork\exec\wait to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ForkApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef FORK_APPLIC_INTERFACE_H
#define FORK_APPLIC_INTERFACE_H

#include "ProcessHandleApplicInterface.hpp"


namespace Dakota {

/// Derived application interface class which spawns simulation codes
/// using fork/execvp/waitpid.

/** ForkApplicInterface is used on Unix systems and is a peer to
    SpawnApplicInterface for Windows systems. */

class ForkApplicInterface: public ProcessHandleApplicInterface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  ForkApplicInterface(const ProblemDescDB& problem_db);
  /// destructor
  ~ForkApplicInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_synch(PRPQueue& prp_queue);

  void derived_synch_nowait(PRPQueue& prp_queue);

  /// spawn a child process for an analysis component within an
  /// evaluation using fork()/vfork()/execvp() and wait for completion
  /// using waitpid() if block_flag is true
  pid_t create_analysis_process(bool block_flag, bool new_group);

  size_t wait_local_analyses(std::map<pid_t, int>& proc_analysis_id_map);
  size_t wait_local_analyses_send(std::map<pid_t, int>& proc_analysis_id_map,
				  int analysis_id);

  void join_evaluation_process_group(bool new_group);
  void join_analysis_process_group(bool new_group);

  void evaluation_process_group_id(pid_t pgid);
  pid_t evaluation_process_group_id() const;
  void analysis_process_group_id(pid_t pgid);
  pid_t analysis_process_group_id() const;

  //
  //- Heading: Member functions
  //

  /// process all available completions within the nonblocking process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait_evaluation(bool block_flag);
  /// process all available completions within the nonblocking process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait_analysis(bool block_flag);

  /// check the exit status of setpgid and abort if an error code was returned
  void check_group(int err, pid_t proc_group_id);

private:

  //
  //- Heading: Methods
  //

  /// process all available completions within the nonblocking process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait(pid_t proc_group_id, bool block_flag);

  //
  //- Heading: Data
  //

  /// the process group id used to identify a set of child evaluation processes
  /// used by this interface instance (to distinguish from other interface
  /// instances that could be running at the same time)
  pid_t evalProcGroupId;
  /// the process group id used to identify a set of child analysis processes
  /// used by this interface instance (to distinguish from other interface
  /// instances that could be running at the same time)
  pid_t analysisProcGroupId;
};


inline ForkApplicInterface::~ForkApplicInterface() 
{ /* Virtual destructor handles referenceCount at Interface level. */ }


inline pid_t ForkApplicInterface::wait_evaluation(bool block_flag)
{ return wait(evalProcGroupId, block_flag); }


inline pid_t ForkApplicInterface::wait_analysis(bool block_flag)
{ return wait(analysisProcGroupId, block_flag); }


inline void ForkApplicInterface::evaluation_process_group_id(pid_t pgid)
{ evalProcGroupId = pgid; }


inline pid_t ForkApplicInterface::evaluation_process_group_id() const
{ return evalProcGroupId; }


inline void ForkApplicInterface::analysis_process_group_id(pid_t pgid)
{ analysisProcGroupId = pgid; }


inline pid_t ForkApplicInterface::analysis_process_group_id() const
{ return analysisProcGroupId; }

} // namespace Dakota

#endif
