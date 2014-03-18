/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ProgramOptions
//- Description: Implementation code for the ProgramOptions class
//- Owner:       Brian Adams
//- Checked by:

#include "ProgramOptions.hpp"
#include "CommandLineHandler.hpp"

namespace Dakota {

// TODO: some of this should be conditional on rank

ProgramOptions::ProgramOptions():
  help(false), version(false), echoInput(true),
  checkFlag(false), preRunFlag(false), runFlag(false), postRunFlag(false),
  userModesFlag(false), stopRestartEvals(0)
{
  // environment settings are overridden by command line options
  parse_environment_options();
  validate();
}

  // TODO: review default settings from parallel library
  // checkFlag(false),
  // preRunFlag(true), runFlag(true), postRunFlag(true), userModesFlag(false),

ProgramOptions::ProgramOptions(int argc, char* argv[], int world_rank):
  help(false), version(false), echoInput(true),
  checkFlag(false), preRunFlag(false), runFlag(false), postRunFlag(false),
  userModesFlag(false), stopRestartEvals(0)
{
  // environment settings are overridden by command line options
  parse_environment_options();

  // encapsulate CLH here; world_rank manages conditional output
  CommandLineHandler clh(argc, argv, world_rank);
  
  help = clh.retrieve("help");
  version = clh.retrieve("version");
  checkFlag = clh.retrieve("check");

  // Need to catch the NULL case; this will clean up when GetLongOpt
  // gets refactored

  if (clh.retrieve("input"))
    inputFile = clh.retrieve("input");
  if (clh.retrieve("output"))
    outputFile = clh.retrieve("output");
  if (clh.retrieve("error"))
    errorFile = clh.retrieve("error");

  // only specify this file if the user passed the option
  if (clh.retrieve("read_restart")) {
    readRestartFile = clh.retrieve("read_restart");
      // check for specified, but without arg; use default restart filename
    if (readRestartFile.empty())
      readRestartFile = "dakota.rst";
  }

  // don't actually need this protection as CLH has the default value dakota.rst
  if (clh.retrieve("write_restart"))
    writeRestartFile = clh.retrieve("write_restart");
  stopRestartEvals = clh.read_restart_evals();

  manage_run_modes(clh);

  if (clh.retrieve("parser"))
    parserOptions = clh.retrieve("parser");

  if (clh.retrieve("no_input_echo"))
    echoInput = false;

  validate();
}


/// Whether command line args dictate instantiation of objects for run
bool ProgramOptions::instantiate_flag() const
{
  if (help || version)
    return false;
  return true;
}

/// Whether steps beyond check are requested
bool ProgramOptions::run_flag() const
{
  if (help || version || checkFlag)
    return false;
  return true;
}


bool ProgramOptions::user_stdout_redirect() const 
{ return !outputFile.empty(); }


String ProgramOptions::stdout_filename() const
{ return outputFile.empty() ? "dakota.out" : outputFile; }


String ProgramOptions::write_restart_filename() const
{ return writeRestartFile.empty() ? "dakota.rst" : writeRestartFile; }


// TODO: extend read/write to all data; currently only those exposed
// in ParallelLibrary

void ProgramOptions::read(MPIUnpackBuffer& s) 
{
  // run modes
  s >> checkFlag >> preRunFlag >> runFlag >> postRunFlag >> userModesFlag;
  // filenames
  s >> preRunInput >> preRunOutput >> runInput >> runOutput 
    >> postRunInput >> postRunOutput;

  // BMA TODO: all relevant data...

}


void ProgramOptions::write(MPIPackBuffer& s) const
{
  // run modes
  s << checkFlag << preRunFlag << runFlag << postRunFlag << userModesFlag;
  // filenames
  s << preRunInput << preRunOutput << runInput << runOutput 
    << postRunInput << postRunOutput;
}


// any environment variables affecting global behavior go here
void ProgramOptions::parse_environment_options() {

  if (parserOptions.empty()) {
    const char* parser_options = std::getenv("DAKOTA_PARSER");
    if (parser_options)
      parserOptions = parser_options;
  }
  
}


void ProgramOptions::manage_run_modes(const CommandLineHandler& clh)
{
  // If filenames empty, do not define defaults; user might not want.
  
  // populate the filenames as necessary
  if ( preRunFlag = (clh.retrieve("pre_run") != NULL))
    split_filenames(clh.retrieve("pre_run"), preRunInput, 
		    preRunOutput);      
  if ( runFlag = (clh.retrieve("run") != NULL))
    split_filenames(clh.retrieve("run"), runInput, runOutput);
  if ( postRunFlag = (clh.retrieve("post_run") != NULL))
    split_filenames(clh.retrieve("post_run"), postRunInput, 
		    postRunOutput);

}


/// Tokenize colon-delimited input and output filenames, returns
/// unchanged strings if tokens not found
void ProgramOptions::split_filenames(const char * filenames, 
				     std::string& input_filename,
				     std::string& output_filename)
{
  // Initial cut doesn't warn/error on invalid input, since pre and post 
  // are special cases in terms of permitted files.  For now admit all specs
  // and silently ignore.
  // NOTE: zero-length (omitted) strings are allowed, but the double colon is 
  // required if the output argument is specified, so, for instance.
  // -pre ::pre.out
  // -post post.in
  if (filenames != NULL) {
    std::string runarg(filenames);
    std::string::size_type pos = runarg.find("::");
    if (pos != std::string::npos) {
      input_filename = runarg.substr(0, pos);
      pos = runarg.find_first_not_of("::", pos);
      if (pos != std::string::npos)
	output_filename = runarg.substr(pos, std::string::npos);
    }
    else
      input_filename = runarg;
  }
}


void ProgramOptions::validate() {

  // BMA: This was a stub of an idea, but isn't true in library mode
  // if (run_flag() && inputFile.empty()) {
  //   Cerr << "when running input is needed";
  // }

  if ( !inputFile.empty() && !inputString.empty() ) {
    Cerr << "\nError: both input file and string specified in ProgramOptions"
	 << std::endl;
    abort_handler(-1);
  }

  if ( !preRunFlag && !runFlag && !postRunFlag ) {
    preRunFlag = runFlag = postRunFlag = true;
    userModesFlag = false; // no active user modes
  }
  else
    userModesFlag = true;  // one or more active user modes

  //  manage_run_modes();

}


}  // namespace Dakota
