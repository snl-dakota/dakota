/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ProgramOptions
//- Description: Implementation code for the ProgramOptions class
//- Owner:       Brian Adams
//- Checked by:

#include "ProgramOptions.hpp"
#include "CommandLineHandler.hpp"
#include "ProblemDescDB.hpp"

namespace Dakota {

// BMA TODO: review default settings from parallel library
// checkFlag(false),
// preRunFlag(true), runFlag(true), postRunFlag(true), userModesFlag(false),

// BMA TODO: some of this should be conditional on rank

// BMA TODO: How to call validate() in the case where a client default
// constructs and then makes additional sessings?  Could the copy ctor
// and assignment be used to validate when passed into Environment ctor?


ProgramOptions::ProgramOptions():
  worldRank(0),
  echoInput(true), stopRestartEvals(0),
  helpFlag(false), versionFlag(false), checkFlag(false), 
  preRunFlag(false), runFlag(false), postRunFlag(false), userModesFlag(false),
  preRunOutputFormat(TABULAR_ANNOTATED), postRunInputFormat(TABULAR_ANNOTATED)
{
  // environment settings are overridden by command line options
  parse_environment_options();
  validate();
}

ProgramOptions::ProgramOptions(int world_rank):
  worldRank(world_rank),
  echoInput(true), stopRestartEvals(0),
  helpFlag(false), versionFlag(false), checkFlag(false), 
  preRunFlag(false), runFlag(false), postRunFlag(false), userModesFlag(false),
  preRunOutputFormat(TABULAR_ANNOTATED), postRunInputFormat(TABULAR_ANNOTATED)
{
  // environment settings are overridden by command line options
  parse_environment_options();
  validate();
}

ProgramOptions::ProgramOptions(int argc, char* argv[], int world_rank):
  worldRank(world_rank),
  echoInput(true), stopRestartEvals(0),
  helpFlag(false), versionFlag(false), checkFlag(false), 
  preRunFlag(false), runFlag(false), postRunFlag(false), userModesFlag(false),
  preRunOutputFormat(TABULAR_ANNOTATED), postRunInputFormat(TABULAR_ANNOTATED)
{
  // environment settings are overridden by command line options
  parse_environment_options();

  // encapsulate CLH here; world_rank manages conditional output
  CommandLineHandler clh(argc, argv, world_rank);
  
  helpFlag = clh.retrieve("help");
  versionFlag = clh.retrieve("version");
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

  // CLH no longer sets the default value dakota.rst; see
  // ProgramOptions::write_restart_file() instead
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


const String& ProgramOptions::input_file() const
{ return inputFile; }

const String& ProgramOptions::input_string() const
{ return inputString; }

bool ProgramOptions::echo_input() const
{ return echoInput; }

const String& ProgramOptions::parser_options() const
{ return parserOptions; }


String ProgramOptions::output_file() const
{ return outputFile.empty() ? "dakota.out" : outputFile; }

const String& ProgramOptions::error_file() const
{ return errorFile; }

const String& ProgramOptions::exit_mode() const
{ return exitMode; }

const String& ProgramOptions::read_restart_file() const
{ return readRestartFile; }

size_t ProgramOptions::stop_restart_evals() const
{ return stopRestartEvals; }

String ProgramOptions::write_restart_file() const
{ return writeRestartFile.empty() ? "dakota.rst" : writeRestartFile; }


bool ProgramOptions::help() const
{ return helpFlag; }

bool ProgramOptions::version() const
{ return versionFlag; }

bool ProgramOptions::check() const
{ return checkFlag; }

bool ProgramOptions::pre_run() const
{ return preRunFlag; }

bool ProgramOptions::run() const
{ return runFlag; }

bool ProgramOptions::post_run() const
{ return postRunFlag; }

bool ProgramOptions::user_modes() const
{ return userModesFlag; }


const String& ProgramOptions::pre_run_input() const
{ return preRunInput; }

const String& ProgramOptions::pre_run_output() const
{ return preRunOutput; }

const String& ProgramOptions::run_input() const
{ return runInput; }

const String& ProgramOptions::run_output() const
{ return runOutput; }

const String& ProgramOptions::post_run_input() const
{ return postRunInput; }

const String& ProgramOptions::post_run_output() const
{ return postRunOutput; }


unsigned int ProgramOptions::pre_run_output_format() const
{ return preRunOutputFormat; }

unsigned int ProgramOptions::post_run_input_format() const
{ return postRunInputFormat; }


bool ProgramOptions::proceed_to_instantiate() const
{
  if (helpFlag || versionFlag)
    return false;
  return true;
}


bool ProgramOptions::proceed_to_run() const
{
  if (helpFlag || versionFlag || checkFlag)
    return false;
  return true;
}


bool ProgramOptions::user_stdout_redirect() const 
{ return !outputFile.empty(); }

bool ProgramOptions::user_stderr_redirect() const 
{ return !errorFile.empty(); }


void ProgramOptions::world_rank(int world_rank)
{
  worldRank = world_rank;
}


void ProgramOptions::input_file(const String& in_file)
{ 
  inputFile = in_file; 
  // not an error if client later resolves
  if ( !inputFile.empty() && !inputString.empty() && worldRank == 0)
    Cout << "Warning (ProgramOptions): both input file and string specified."
	 << std::endl;
}

void ProgramOptions::input_string(const String& in_string)
{  
  inputString = in_string; 
  // not an error if client later resolves
  if ( !inputFile.empty() && !inputString.empty() && worldRank == 0)
    Cout << "Warning (ProgramOptions): both input file and string specified."
	 << std::endl;
}

void ProgramOptions::echo_input(bool echo_flag)
{ echoInput = echo_flag; }


void ProgramOptions::output_file(const String& out_file)
{ outputFile = out_file; }

void ProgramOptions::error_file(const String& err_file)
{ errorFile = err_file; }

void ProgramOptions::exit_mode(const String& mode)
{ exitMode = mode; }

void ProgramOptions::read_restart_file(const String& read_rst)
{ readRestartFile = read_rst; }

void ProgramOptions::stop_restart_evals(size_t stop_rst)
{ stopRestartEvals = stop_rst; }

void ProgramOptions::write_restart_file(const String& write_rst)
{ writeRestartFile = write_rst; }


void ProgramOptions::help(bool help_flag)
{ helpFlag = help_flag; }

void ProgramOptions::version(bool version_flag)
{ versionFlag = version_flag; }

void ProgramOptions::check(bool check_flag)
{ checkFlag = check_flag; }

void ProgramOptions::pre_run(bool pre_run_flag)
{ preRunFlag = pre_run_flag; }

void ProgramOptions::run(bool run_flag)
{ runFlag = run_flag; }

void ProgramOptions::post_run(bool post_run_flag)
{ postRunFlag = post_run_flag; }


void ProgramOptions::pre_run_input(const String& pre_run_in)
{ preRunInput = pre_run_in; }

void ProgramOptions::pre_run_output(const String& pre_run_out)
{ preRunOutput = pre_run_out; }

void ProgramOptions::run_input(const String& run_in)
{ runInput = run_in; }

void ProgramOptions::run_output(const String& run_out)
{ runOutput = run_out; }

void ProgramOptions::post_run_input(const String& post_run_in)
{ postRunInput = post_run_in; }

void ProgramOptions::post_run_output(const String& post_run_out)
{ postRunOutput = post_run_out; }


void ProgramOptions::parse(const ProblemDescDB& problem_db)
{
  // command line options can override those in the input file, so
  // warn if both exist; DB should be valid on all ranks at this point
  
  // we don't offer an option to override input echo in the input file
  // since we want to echo the input prior to parsing to help with
  // debugging

  set_option(problem_db, "output_file", outputFile);
  set_option(problem_db, "error_file", errorFile);
  set_option(problem_db, "read_restart", readRestartFile);

  const int& stoprst = problem_db.get_int("environment.stop_restart");
  if (stoprst > 0) {
    if (stopRestartEvals == 0)
      stopRestartEvals = stoprst;
    else if (worldRank == 0)
      Cout << "Warning: stop restart evals specified in input file and passed "
	   << "options; option\n         specifying '" << stopRestartEvals
	   << "' takes precedence over input file value." << std::endl;
  }

  set_option(problem_db, "write_restart", writeRestartFile);

  // only override if non-default, no need to warn
  const bool& check_flag = problem_db.get_bool("environment.check");
  if (!checkFlag && check_flag) {
    checkFlag = check_flag;
  }


  const bool& pre_run = problem_db.get_bool("environment.pre_run");
  const bool& run = problem_db.get_bool("environment.run");
  const bool& post_run = problem_db.get_bool("environment.post_run");
  
  // if command line options already set, ignore all input file pre/run/post
  if (pre_run || run || post_run) {
    if (userModesFlag) {
      if (worldRank == 0)
	Cout << "Warning: run mode options already passed; input file run " 
	     << "modes will be ignored." << std::endl;
    }
    else {

      preRunFlag = pre_run;
      runFlag = run;
      postRunFlag = post_run;

      set_option(problem_db, "pre_run_input", preRunInput);
      set_option(problem_db, "pre_run_output", preRunOutput);
      set_option(problem_db, "run_input", runInput);
      set_option(problem_db, "run_output", runOutput);
      set_option(problem_db, "post_run_input", postRunInput);
      set_option(problem_db, "post_run_output", postRunOutput);

      preRunOutputFormat = 
	problem_db.get_ushort("environment.pre_run_output_format");
      postRunInputFormat = 
	problem_db.get_ushort("environment.post_run_input_format");

    }

    // only call this when user modes haven't already been parsed
    validate_run_modes();

  }

}


void ProgramOptions::read(MPIUnpackBuffer& s) 
{
  // core files and options
  s >> inputFile >> inputString >> echoInput >> parserOptions 
    >> outputFile >> errorFile 
    >> readRestartFile >> stopRestartEvals >> writeRestartFile;
  // run mode controls
  s >> helpFlag >> versionFlag >> checkFlag >> preRunFlag >> runFlag 
    >> postRunFlag >> userModesFlag;
  // run mode filenames
  s >> preRunInput >> preRunOutput >> runInput >> runOutput 
    >> postRunInput >> postRunOutput;
  // run mode formats
  s >> preRunOutputFormat >> postRunInputFormat;
}


void ProgramOptions::write(MPIPackBuffer& s) const
{
  // core files and options
  s << inputFile << inputString << echoInput << parserOptions 
    << outputFile << errorFile 
    << readRestartFile << stopRestartEvals << writeRestartFile;
  // run mode controls
  s << helpFlag << versionFlag << checkFlag << preRunFlag << runFlag 
    << postRunFlag << userModesFlag;
  // run mode filenames
  s << preRunInput << preRunOutput << runInput << runOutput 
    << postRunInput << postRunOutput;
  // run mode formats
  s << preRunOutputFormat << postRunInputFormat;
}


// any environment variables affecting global behavior go here
// TODO: decide precedence of these vs. CLOpts and input file
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
    if (worldRank == 0)
      Cerr << "\nError: both input file and string specified in ProgramOptions"
	   << std::endl;
    abort_handler(-1);
  }

  validate_run_modes();
}


void ProgramOptions::validate_run_modes() {

  if (preRunFlag && !runFlag && postRunFlag) {
    Cerr << "\nError: Run phase 'run' is required when specifying both " 
	 << "'pre_run' and 'post_run'.";
    abort_handler(-1);
  }

  // if no phases were given, assume default that all are active
  if ( !preRunFlag && !runFlag && !postRunFlag ) {
    preRunFlag = runFlag = postRunFlag = true;
    userModesFlag = false; // no active user-specified modes
  }
  else
    userModesFlag = true;  // one or more active user-specified modes

}


void ProgramOptions::
set_option(const ProblemDescDB& problem_db, const String& db_name,
	   String& data_member) {
    
  String lookup_prefix("environment.");
  const String& db_str = problem_db.get_string(lookup_prefix + db_name);
  
  if (!db_str.empty()) {
    if (data_member.empty())
      data_member = db_str;
    else if (worldRank == 0)
      Cout << "Warning: " << db_name << " specified in input file and passed "
	   << "options; option\n         specifying '" << data_member
	   << "' takes precedence over input file value." << std::endl;
  }
}


}  // namespace Dakota
