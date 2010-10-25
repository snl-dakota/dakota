/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ForkAnalysisCode
//- Description:  Specialization of AnalysisCode base class for forks
//- Owner:        Mike Eldred
//- Version: $Id: ForkAnalysisCode.H 7021 2010-10-12 22:19:01Z wjbohnh $

#ifndef FORK_ANALYSIS_CODE_H
#define FORK_ANALYSIS_CODE_H

#include "AnalysisCode.H"
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
  //- Heading: Methods
  //

  /// spawn a child process using fork()/vfork()/execvp() and wait
  /// for completion using waitpid() if block_flag is true
  pid_t fork_program(const bool block_flag);

  /// check the exit status of a forked process and abort if an
  /// error code was returned
  void check_status(const int status);

  /// set argList for execution of the input filter
  void ifilter_argument_list();
  /// set argList for execution of the output filter
  void ofilter_argument_list();
  /// set argList for execution of the specified analysis driver
  void driver_argument_list(const int analysis_id);

private:

  //
  //- Heading: Data
  //

  /// an array of strings for use with execvp(const char *, char * const *).
  /// These are converted to an array of const char*'s in fork_program().
  std::vector<std::string> argList;
};

inline ForkAnalysisCode::ForkAnalysisCode(const ProblemDescDB& problem_db) : 
  AnalysisCode(problem_db), argList(3)
{ }

inline ForkAnalysisCode::~ForkAnalysisCode()
{ }


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
