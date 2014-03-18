/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        OutputManager
//- Description:  Class to manage conditional output and redirection of streams
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef DAKOTA_OUTPUT_MANAGER_H
#define DAKOTA_OUTPUT_MANAGER_H

#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"

// forward declarations for restart I/O
namespace boost {
  namespace archive {
    class binary_oarchive;
    class binary_iarchive;
  }
}

namespace Dakota {

class ProgramOptions;
class ProblemDescDB;
class ParamResponsePair;

// TODO: tagging for pre/run/post I/O files
// TODO: consider maintaining a list of redirections and pop them off
// TODO: better error checking in each function

/// Class to manage redirection of stdout/stderr, and keep track of
/// current redir state, and manage rank 0 output
class OutputManager {

public:
  
  /// Default constructor (needed for default environment ctors)
  OutputManager();

  /// Standard constructor, taking user-specified program options and
  /// optionally taking the rank of this process in Dakota's MPI_Comm
  OutputManager(const ProgramOptions& prog_opts, int dakota_world_rank = 0,
		bool dakota_mpirun_flag = false);

  /// Destructor that closes streams and other outputs
  ~OutputManager();

  /// helper to close streams during destructor or abnormal abort
  void close_streams();


  // -----
  // Modify output settings
  // -----

  /// Extract environment options from ProblemDescDB
  void parse(const ProblemDescDB& problem_db);

  /// Set the Dakota startup message ("Running on...")
  void startup_message(const String& start_msg);

  /// Update the tag to use on files
  void file_tag(const String& iterator_tag);

  /// Redirect cout based on program options filenames and force flag
  void redirect_cout(const ProgramOptions& prog_opts, 
		     bool force_cout_redirect = false);

  /// Redirect cerr based on program options filenames only
  void redirect_cerr(const ProgramOptions& prog_opts);

  /// Initialize results DB based on problem DB 
  void init_resultsdb(ProgramOptions& prog_opts);

  /// Initialize restart DB based on program options filenames
  void init_restart(const ProgramOptions& prog_opts);


  // -----
  // Convenience functions to generate output
  // -----

  /// Output the current Dakota version
  void output_version(std::ostream& os = Cout) const;

  /// Output the startup header and time
  void output_startup_message(std::ostream& os = Cout) const;

  // BMA TODO: Review and possibly more cautious rank 0 I/O control
  /// Output only on Dakota world rank 0 (for version, help, etc.)
  void output_helper(const String& message, std::ostream& os) const;

  /// append a parameter/response set to the restart file
  void append_restart(const ParamResponsePair& prp);


  // -----
  // Data to later be made private
  // -----

  bool graph2DFlag;       ///< flag for using 2D graphics plots
  bool tabularDataFlag;   ///< flag for file tabulation of graphics data
  bool resultsOutputFlag; ///< whether to output results data

   // // For items from the environment spec, can use DataEnvironment defaults
   //  tabular_filename       = outputManager.tabularDataFile;
   //  results_filename       = outputManager.resultsOutputFile;

  // Note: the following are raw untagged versions from parse():

  String tabularDataFile;   ///< filename for tabulation of graphics data
  String resultsOutputFile; ///< filename for results data

private:
  
  /// implementation of cout redirection
  void redirect_cout(const String& new_filename);

  /// implementation of cerr redirection
  void redirect_cerr(const String& new_filename);

  /// conditionally import evaluations from restart file, then always
  /// create or overwrite restart file
  void read_write_restart(bool read_restart_flag,
			  const String& read_restart_filename,
			  size_t stop_restart_eval,
			  const String& write_restart_filename);

  // -----
  // Data
  // -----

  /// output manager handles rank 0 only output when needed
  int worldRank;

  /// some output is only for MPI runs
  bool mpirunFlag;

  /// tag for various input/output files (default none)
  String fileTag;

  /// message to print at startup when proceeding to instantiate objects
  String startupMessage;


  // For stdout/stderr management
  // -----

  /// current (potentially tagged) filename for output, ProgramOptions has root
  String coutFilename;
  /// current (potentially tagged) filename for error, ProgramOptions has root
  String cerrFilename;

  // Tagged output file streams used when there are concurrent iterators 
  std::ofstream output_ofstream; ///< tagged file redirection of stdout
  std::ofstream error_ofstream;  ///< tagged file redirection of stderr


  // For restart management
  // -----

  /// Binary stream to which restart data is written
  std::ofstream restartOutputFS;
  /// Binary output archive to which data is written (ptr as no default ctor)
  boost::archive::binary_oarchive *restartOutputArchive;

};  // class OutputManager

} //namespace Dakota

#endif  // DAKOTA_OUTPUT_MANAGER_H
