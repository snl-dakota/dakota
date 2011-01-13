/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        SysCallAnalysisCode
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "SysCallAnalysisCode.H"
#include "CommandShell.H"
#include "ProblemDescDB.H"
#include <boost/lexical_cast.hpp>

static const char rcsId[]="@(#) $Id: SysCallAnalysisCode.C 7004 2010-10-04 17:55:00Z wjbohnh $";


namespace Dakota {

SysCallAnalysisCode::SysCallAnalysisCode(const ProblemDescDB& problem_db): 
  AnalysisCode(problem_db)
{ }


/** Put the SysCallAnalysisCode to the shell.  This function is used
    when all portions of the function evaluation (i.e., all analysis
    drivers) are executed on the local processor. */
void SysCallAnalysisCode::spawn_evaluation(const bool block_flag)
{
  // MSE, 11/17/99: system call file passing changed to pass both files to all
  // 3 executables since: (1) in many cases, the OFilter will need to know the
  // asv, parameters, etc., and (2) in asynch usage, each of the 3 pieces must
  // be able to manage tagged files and/or working subdirectories.

  CommandShell shell;
  bool needparen;
  const char *s, *s1;
  size_t L;

  if ((s = shell.workDir = work_dir()))
	L = std::strlen(s);

  // Input filter portion
  if ((needparen = !block_flag && (numPrograms > 1
				|| !iFilterName.empty()
				|| !oFilterName.empty())))
  	shell << "(";
  if (!iFilterName.empty()) {
    shell << iFilterName;
    if (commandLineArgs)
      shell << " " << paramsFileName << " " << resultsFileName;
    shell << "; ";
  }
  
  // Analysis code portion (function evaluation may be asynchronous, but
  // analyses must be sequential within each function evaluation)
  for (size_t i=0; i<numPrograms; ++i) {
    shell << programNames[i];
    if (commandLineArgs) {
       s1 = paramsFileName.c_str();
       if (s && !std::strncmp(s,s1,L) && s1[L] == '/')
		s1 += L + 1;
      shell << " " << s1;
      std::string prog_num( (multipleParamsFiles || numPrograms > 1) ?
                            "." + boost::lexical_cast<std::string>(i+1) : "" );
      if (multipleParamsFiles) // append program cntr to paramsFileName
	shell << prog_num;

      s1 = resultsFileName.c_str();
      if (s && !std::strncmp(s,s1,L) && s1[L] == '/')
		s1 += L + 1;
      shell << " " << s1;
      if (numPrograms > 1)     // append program cntr to resultsFileName
	shell << prog_num;
    }
    if (i != numPrograms-1)
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
  // suppressOutputFlag from the incoming block_flag & the program's
  // suppressOutputFlag and spawn the process.
  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutputFlag);
  shell << flush;
}


/** Put the input filter to the shell.  This function is used when multiple
    analysis drivers are spread between processors.  No need to check for a
    Null input filter, as this is checked externally.  Use of nonblocking
    shells is supported in this fn, although its use is currently prevented
    externally. */
void SysCallAnalysisCode::spawn_input_filter(const bool block_flag)
{
  CommandShell shell;

  shell.workDir = work_dir();
  shell << iFilterName;
  if (commandLineArgs)
    shell << " " << paramsFileName << " " << resultsFileName;

  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutputFlag);
  shell << flush;
}


/** Put a single analysis to the shell.  This function is used when
    multiple analysis drivers are spread between processors.  Use of
    nonblocking shells is supported in this fn, although its use is
    currently prevented externally. */
void SysCallAnalysisCode::
spawn_analysis(const int& analysis_id, const bool block_flag)
{
  CommandShell shell;

  shell.workDir = work_dir();
  shell << programNames[analysis_id-1];
  if (commandLineArgs) {
    using std::string;
    shell << " " << paramsFileName;
    string prog_num( (multipleParamsFiles || numPrograms > 1) ?
                     "." + boost::lexical_cast<string>(analysis_id) : "" );
    if (multipleParamsFiles) // append program cntr to paramsFileName
      shell << prog_num;
    shell << " " << resultsFileName;
    if (numPrograms > 1)     // append program cntr to resultsFileName
      shell << prog_num;
  }

  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutputFlag);
  shell << flush;
}


/** Put the output filter to the shell.  This function is used when multiple
    analysis drivers are spread between processors.  No need to check for a
    Null output filter, as this is checked externally.  Use of nonblocking
    shells is supported in this fn, although its use is currently prevented
    externally. */
void SysCallAnalysisCode::spawn_output_filter(const bool block_flag)
{
  CommandShell shell;

  shell.workDir = work_dir();
  shell << oFilterName;
  if (commandLineArgs)
    shell << " " << paramsFileName << " " << resultsFileName;

  shell.asynch_flag(!block_flag);
  shell.suppress_output_flag(suppressOutputFlag);
  shell << flush;
}


/* OLD overloaded operator approach:
CommandShell& operator<<(CommandShell& shell, SysCallAnalysisCode& program)
{
  // Put the SysCallAnalysisCode to the shell using either the default syntax
  // or specified commandUsage syntax

  // modified file names handle differences between what was specified in
  // dakota.in and what is actually used in the system call (due to file 
  // tagging or Unix tmp file usage).

  // MSE, 11/17/99: system call file passing changed to pass both files to all
  // 3 executables since: (1) in many cases, the OFilter will need to know the
  // asv, parameters, etc., and (2) in asynch usage, each of the 3 pieces must
  // be able to manage tagged files and/or working subdirectories.

  // Input filter portion
  shell << "(";  
  if (!program.iFilterName.empty())
    shell << program.iFilterName << " " << program.paramsFileName << " "
          << program.resultsFileName << "; ";
  
  // Analysis code portion (function evaluation may be asynchronous, but
  // analyses must be sequential within each function evaluation)
  if (program.commandUsage.empty()) {
    for (size_t i=0; i<program.numPrograms; i++) {
      shell << program.programNames[i] << " " << program.paramsFileName << " "
            << program.resultsFileName;
      if (program.numPrograms > 1) { // append cntr to resultsFileName
        // NOTE: std::string and boost::lexical_cast prevents hardwired strlen
        char prog_num[16];
        std::sprintf(prog_num,".%d",i+1);
        shell << prog_num;
      }
      if (i != program.numPrograms-1)
        shell << "; ";
    }
  }
  else                             // if numPrograms>1, then commandUsage must
    shell << program.commandUsage; // include the syntax for all programs.

  // Output filter portion
  if (!program.oFilterName.empty())
    shell << "; " << program.oFilterName << " " << program.paramsFileName << " "
          << program.resultsFileName;
  shell << ")";

  // Process definition complete; now set the shell's asynchFlag &
  // suppressOutputFlag from the program's asynchEvalFlag & suppressOutputFlag
  // and spawn the process.
  shell.asynch_flag(program.asynch_eval_flag());
  shell.suppress_output_flag(program.suppress_output_flag());
  shell << flush;

  return shell;
}
*/

} // namespace Dakota
