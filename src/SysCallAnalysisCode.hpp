/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SysCallAnalysisCode
//- Description:  Specialization of AnalysisCode base class for system calls
//- Owner:        Mike Eldred
//- Version: $Id: SysCallAnalysisCode.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef SYS_CALL_ANALYSIS_CODE_H
#define SYS_CALL_ANALYSIS_CODE_H

#include "AnalysisCode.hpp"

namespace Dakota {

class CommandShell;


/// Derived class in the AnalysisCode class hierarchy which spawns
/// simulations using system calls.

/** SysCallAnalysisCode creates separate simulation processes using
    the C system() command.  It utilizes CommandShell to manage shell
    syntax and asynchronous invocations. */

class SysCallAnalysisCode: public AnalysisCode
{
  //friend CommandShell& operator<<(CommandShell& shell, 
  //                                SysCallAnalysisCode& program);

public:

  //
  //- Heading: Constructors and destructor
  //

  SysCallAnalysisCode(const ProblemDescDB& problem_db); ///< constructor
  ~SysCallAnalysisCode();                               ///< destructor

  //
  //- Heading: Methods
  //

  /// spawn a complete function evaluation
  void spawn_evaluation(const bool block_flag);
  /// spawn the input filter portion of a function evaluation
  void spawn_input_filter(const bool block_flag);
  /// spawn a single analysis as part of a function evaluation
  void spawn_analysis(const int& analysis_id, const bool block_flag);
  /// spawn the output filter portion of a function evaluation
  void spawn_output_filter(const bool block_flag);

private:

  //
  //- Heading: Data
  //

  // a class attribute of SysCall analysis codes only.  Needed since flags
  // cannot be passed directly through to a CommandShell(operator<<).
  //bool asynchEvalFlag;
};


inline SysCallAnalysisCode::~SysCallAnalysisCode()
{ }

} // namespace Dakota

#endif
