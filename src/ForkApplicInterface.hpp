/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  ~ForkApplicInterface() override;

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void wait_local_evaluation_sequence(PRPQueue& prp_queue) override;
  void test_local_evaluation_sequence(PRPQueue& prp_queue) override;

  /// spawn a child process for an analysis component within an
  /// evaluation using fork()/execvp() and wait for completion
  /// using waitpid() if block_flag is true
  pid_t create_analysis_process(bool block_flag, bool new_group) override;

  size_t wait_local_analyses() override;
  size_t test_local_analyses_send(int analysis_id) override;

  void join_evaluation_process_group(bool new_group) override;
  void join_analysis_process_group(bool new_group) override;

  void  evaluation_process_group_id(pid_t pgid) override;
  pid_t evaluation_process_group_id() const override;
  void  analysis_process_group_id(pid_t pgid) override;
  pid_t analysis_process_group_id() const override;

  //
  //- Heading: Member functions
  //

  /// process all available completions within the evaluation process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait_evaluation(bool block_flag);
  /// process all available completions within the analysis process group;
  /// if block_flag = true, wait for at least one completion
  pid_t wait_analysis(bool block_flag);

  /// check the exit status of setpgid and abort if an error code was returned
  void check_group(int err, pid_t proc_group_id);

private:

  //
  //- Heading: Methods
  //

  /// core code used by wait_{evaluation,analysis}()
  pid_t wait(pid_t proc_group_id, std::map<pid_t, int>& process_id_map,
	     bool block_flag);

  /// core code used by join_{evaluation,analysis}_process_group()
  void join_process_group(pid_t& process_group_id, bool new_group);

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
{ }


inline pid_t ForkApplicInterface::wait_evaluation(bool block_flag)
{ return wait(evalProcGroupId, evalProcessIdMap, block_flag); }


inline pid_t ForkApplicInterface::wait_analysis(bool block_flag)
{ return wait(analysisProcGroupId, analysisProcessIdMap, block_flag); }


inline void ForkApplicInterface::join_evaluation_process_group(bool new_group)
{ join_process_group(evalProcGroupId, new_group); }


inline void ForkApplicInterface::join_analysis_process_group(bool new_group)
{ join_process_group(analysisProcGroupId, new_group); }


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
