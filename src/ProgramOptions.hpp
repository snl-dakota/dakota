/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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

// TODO: define enum for run phases/modes and use <=, >, etc.
// TODO: validate pre/run/post modes based on CLH and PL examples

namespace Dakota {

class CommandLineHandler;

/// ProgramOptions stores options whether from the CLH or from library
/// user; will eventually move to boost::program_options; initially
/// valid only oon worldRank = 0, but then broadcast in
/// ParallelLibrary::manage_outputs_restart
class ProgramOptions {

public:

  /// Default constructor (needed for default environment ctors and
  /// could be used by library clients to late update data)
  ProgramOptions();

  /// Standard constructor that uses a CommandLineHandler to parse
  /// user options
  ProgramOptions(int argc, char* argv[], int world_rank);


  /// whether steps beyond help/version are requested
  bool instantiate_flag() const;

  /// Whether steps beyond check are requested
  bool run_flag() const;

  /// whether the user/client code requested a redirect of stdout
  bool user_stdout_redirect() const;

  /// returns user-provided or default filename if empty
  String stdout_filename() const;

  /// returns user-provided or default filename if empty
  String write_restart_filename() const;

  /// helper function for reading some class data from MPI buffer
  void read(MPIUnpackBuffer& s);

  /// helper function for writing some class data to MPI buffer
  void write(MPIPackBuffer& s) const;
 

  // TODO: make data private

  bool help;
  bool version;
  String inputFile;       //< e.g., "dakota.in"
  String outputFile;      //< e.g., "dakota.out"
  String errorFile;       //< e.g., "dakota.err"
  String parserOptions;
  bool echoInput;

  // Run mode flags; intially only valid on rank 0.
  // Could condense flags into a bit-wise short, but using bool for
  // now for clarity; could use map or vector with enum for Strings
  bool checkFlag;       ///< flags invocation with command line option -check
  bool preRunFlag;      ///< flags invocation with command line option -pre_run
  bool runFlag;         ///< flags invocation with command line option -run
  bool postRunFlag;     ///< flags invocation with command line option -post_run
  bool userModesFlag;   ///< whether user run mdoes are active

  // pre/run/post I/O filenames; intially only valid on rank 0.
  String preRunInput;   ///< filename for pre_run input
  String preRunOutput;  ///< filename for pre_run output
  String runInput;      ///< filename for run input
  String runOutput;     ///< filename for run output
  String postRunInput;  ///< filename for post_run input
  String postRunOutput; ///< filename for post_run output

  String readRestartFile;    //< e.g., "dakota.old.rst"
  size_t stopRestartEvals;
  String writeRestartFile;   //< e.g., "dakota.new.rst"

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
