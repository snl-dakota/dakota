/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ProgramOptions
//- Description:  Class to store run mode and command-line options
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef DAKOTA_PROGRAM_OPTIONS_H
#define DAKOTA_PROGRAM_OPTIONS_H

#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"

// ProgramOptions is currently default constructible and we pass it by
// value to the Environment constructors.  If it becomes larger or more 
// complex, we will consider using handle/body idiom to manage the memory.

// BMA TODO: define enum for run phases/modes and use <=, >, etc.
// BMA TODO: validate pre/run/post modes based on CLH and PL examples
// BMA TODO: transition to boost::program_options

namespace Dakota {

class CommandLineHandler;
class ProblemDescDB;

/// ProgramOptions stores options whether from the CLH or from library
/// user; initially valid only on worldRank = 0, but then broadcast in
/// ParallelLibrary::push_output_tag()
class ProgramOptions {

public:

  //- Constructors

  /// default constructor (needed for default environment ctors and
  /// could be used by library clients to late update data)
  ProgramOptions();

  /// constructor that accepts world rank to help with I/O control; allows 
  /// default constructed ProgramOptions to get rank in library mode
  ProgramOptions(int world_rank);

  /// standard constructor that uses a CommandLineHandler to parse
  /// user options
  ProgramOptions(int argc, char* argv[], int world_rank);


  //- Query functions

  /// Dakota input file base name (no tag)
  const String& input_file() const;
  /// alternate Dakota input string literal
  const String& input_string() const;
  /// is input echo specified?
  bool echo_input() const;
  /// (deprecated) NIDR parser options
  const String& parser_options() const;
  
  /// output (user-provided or default) file base name (no tag)
  String output_file() const;
  /// error file base name (no tag)
  const String& error_file() const;
  /// behavior of abort_handler (throw or exit)
  const String& exit_mode() const;
  
  /// restart file base name (no tag)
  const String& read_restart_file() const;
  /// eval ID at which to stop reading restart
  size_t stop_restart_evals() const;
  /// write retart (user-provided or default) file base name (no tag)
  String write_restart_file() const;

  /// is help mode active?
  bool help() const;
  /// is version mode active?
  bool version() const;
  /// is check mode active?
  bool check() const;
  /// is pre-run mode active?
  bool pre_run() const;
  /// is run mode active?
  bool run() const;
  /// is post-run mode active?
  bool post_run() const;
  /// are any non-default, user-specified run modes active?
  bool user_modes() const;

  /// filename for pre-run input
  const String& pre_run_input() const;
  /// filename for pre-run output
  const String& pre_run_output() const;
  /// filename for run input
  const String& run_input() const;
  /// filename for run output
  const String& run_output() const;
  /// filename for post-run input
  const String& post_run_input() const;
  /// filename for post-run output
  const String& post_run_output() const;

  /// tabular format for pre-run output
  unsigned int pre_run_output_format() const;
  /// tabular format for post-run input
  unsigned int post_run_input_format() const;

  /// whether steps beyond help/version are requested (instantiation required)
  bool proceed_to_instantiate() const;

  /// Whether steps beyond check are requested
  bool proceed_to_run() const;

  /// whether the user/client code requested a redirect of stdout
  bool user_stdout_redirect() const;

  /// whether the user/client code requested a redirect of stderr
  bool user_stderr_redirect() const;


  //- Set functions
  
  /// set the world rank to govern early conditional output
  void world_rank(int world_rank);

  /// set Dakota input file base name (no tag)
  void input_file(const String& in_file);
  /// set alternate Dakota input string literal
  void input_string(const String& in_string);
  /// set whether to echo input to output
  void echo_input(bool echo_flag);
  /// set behavior for abort_handler
  void exit_mode(const String& mode);
  /// set base file name for Dakota output
  void output_file(const String& out_file);
  /// set base file name for Dakota errors
  void error_file(const String& err_file);

  /// set base file name for restart file from which to read
  void read_restart_file(const String& read_rst);
  /// set eval ID at which to stop reading restart
  void stop_restart_evals(size_t stop_rst);
  /// set base file name for restart file to write
  void write_restart_file(const String& write_rst);

  /// set true to print help information and exit
  void help(bool help_flag);
  /// set true to print version information and exit
  void version(bool version_flag);
  /// set true to check input and instantiate objects, then exit
  void check(bool check_flag);
  /// set to enable/disable pre-run phase
  void pre_run(bool pre_run_flag);
  /// set to enable/disable run phase
  void run(bool run_flag);
  /// set to enable/disable post-run phase
  void post_run(bool post_run_flag);

  /// Specify the pre-run phase input filename
  void pre_run_input(const String& pre_run_in);
  /// Specify the pre-run phase output filename
  void pre_run_output(const String& pre_run_out);
  /// Specify the run phase input filename
  void run_input(const String& run_in);
  /// Specify the run phase output filename
  void run_output(const String& run_out);
  /// Specify the post-run phase input filename
  void post_run_input(const String& post_run_in);
  /// Specify the post-run phase output filename
  void post_run_output(const String& post_run_out);

  /// Extract environment options from ProblemDescDB
  void parse(const ProblemDescDB& problem_db);

  /// helper function for reading some class data from MPI buffer
  void read(MPIUnpackBuffer& s);

  /// helper function for writing some class data to MPI buffer
  void write(MPIPackBuffer& s) const;
 


private:

  /// any environment variables affecting global behavior get read here
  void parse_environment_options();

  /// retrieve run mode options from the CLH
  void manage_run_modes(const CommandLineHandler& clh);

  /// manage pre/run/post filenames
  void split_filenames(const char * filenames, std::string& input_filename,
		       std::string& output_filename);

  /// verify consistency of user settings (helpful for library mode especially)
  void validate();

  /// validate user run modes and set userModesFlag
  void validate_run_modes();

  /// retrieve environment.<db_name> from the problem db and update
  /// data_member, warning if needed
  void set_option(const ProblemDescDB& problem_db, const String& db_name, 
		  String& data_member);

  /// cache the world rank to help with conditional output
  int worldRank;

  // Any of this data coming from command line parsing is only valid
  // on rank 0 initially, then broadcast

  /// Dakota input file name, e.g., "dakota.in"
  String inputFile;
  /// alternate input means for library clients: input string
  /// (mutually exclusive with input file)
  String inputString;
  bool echoInput;         ///< whether to echo client's input file at parse 
  String parserOptions;   ///< Deprecated option for NIDR parser options
  String exitMode;        ///< Abort or throw on error

  String outputFile;      ///< Dakota output base file name, e.g., "dakota.out"
  String errorFile;       ///< Dakota error base file name, e.g., "dakota.err"

  String readRestartFile;    ///< e.g., "dakota.old.rst"
  size_t stopRestartEvals;   ///< eval number at which to stop restart read
  String writeRestartFile;   ///< e.g., "dakota.new.rst"

  // Run mode flags; intially only valid on rank 0.
  // Could condense flags into a bit-wise short, but using bool for
  // now for clarity; could use map or vector with enum for Strings
  bool helpFlag;        ///< whether to print help message and exit
  bool versionFlag;     ///< whether to print version message and exit
  bool checkFlag;       ///< flags invocation with command line option -check
  bool preRunFlag;      ///< flags invocation with command line option -pre_run
  bool runFlag;         ///< flags invocation with command line option -run
  bool postRunFlag;     ///< flags invocation with command line option -post_run
  bool userModesFlag;   ///< whether any user run modes are active

  // pre/run/post I/O filenames; intially only valid on rank 0.
  String preRunInput;   ///< filename for pre_run input
  String preRunOutput;  ///< filename for pre_run output
  String runInput;      ///< filename for run input
  String runOutput;     ///< filename for run output
  String postRunInput;  ///< filename for post_run input
  String postRunOutput; ///< filename for post_run output

  unsigned short preRunOutputFormat;  ///< tabular format for pre_run output
  unsigned short postRunInputFormat;  ///< tabular format for post_run input

};  // class ProgramOptions


/// MPIUnpackBuffer extraction operator
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, ProgramOptions& progopt)
{ 
  progopt.read(s);
  return s; 
}

/// MPIPackBuffer insertion operator
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const ProgramOptions& progopt)
{ 
  progopt.write(s);
  return s; 
}


}  // namespace Dakota

#endif  // DAKOTA_PROGRAM_OPTIONS_H
