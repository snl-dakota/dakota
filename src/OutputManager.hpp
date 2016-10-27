/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        OutputManager
//- Description:  Class to manage conditional output and redirection of streams
//- Owner:        Brian Adams
//- Version: $Id$

#ifndef DAKOTA_OUTPUT_MANAGER_H
#define DAKOTA_OUTPUT_MANAGER_H

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "dakota_data_types.hpp"
#include "dakota_global_defs.hpp"
#include "DakotaGraphics.hpp"


namespace Dakota {

class ProgramOptions;
class ProblemDescDB;
class ParamResponsePair;


/** Component to manage a redirected output or error stream */
class OutputWriter {

public:
  
  /// ostream constructor; used to construct a writer to existing
  /// stream, e.g., std::cout
  OutputWriter(std::ostream* output_stream);

  /// file redirect constructor; opens an overwriting file stream to given name
  OutputWriter(const String& output_filename);

  /// the (possibly empty) file name for this stream
  const String& filename() const;

  /// a pointer to the stream, either cout/cerr or a file
  std::ostream* output_stream();

protected:

  /// the name of the output file (empty when constructed from pointer)
  String outputFilename;

  /// file output stream for console text; only open if string non-empty
  std::ofstream outputFS;

  /// pointer to the stream for this writer
  std::ostream* outputStream;

};


/** Component to manage a set of output or error redirections.  Push
    operations may present a new filename, or none in order to
    preserve current binding to cout/cerr or file, but place an entry
    on the stack.  Cout/Cerr are rebound as needed when a stream is
    destroyed on pop. */
class ConsoleRedirector {

public:

  /// Constructor taking a reference to the Dakota Cout/Cerr handle
  /// and a default destination to use when no redirection (or destruct)
  ConsoleRedirector(std::ostream* & dakota_stream, std::ostream* default_dest);

  /// when the redirector stack is destroyed, it will rebind the
  /// output handle to the default ostream, then destroy open files
  ~ConsoleRedirector();

  /// push back the default or repeat the last pushed file stream
  void push_back();

  /// push back a new output filestream, or repeat the last one if no
  /// filename change
  void push_back(const String& filename);

  /// pop the last redirection
  void pop_back();

protected:
  /// The handle (target ostream) through which output is sent;
  /// typically dakota_cout or dakota_cerr.  Will be rebound to
  /// specific streams as they are pushed or popped.
  std::ostream*& ostreamHandle;
  
  /// initial stream to reset to when redirections are done (typically
  /// std::cout or std::cerr)
  std::ostream* defaultOStream;

  /// stack of redirections to OutputWriters; shared pointers are used
  /// to potentially share the same ostream at multiple levels
  std::vector<boost::shared_ptr<OutputWriter> > ostreamDestinations;

private:
  // private ctors since current implementation with streams isn't
  // easily copied.

  /// default constructor is disallowed
  ConsoleRedirector();
  /// copy constructor is disallowed due
  ConsoleRedirector(const ConsoleRedirector&);
  /// assignment is disallowed
  const ConsoleRedirector& operator=(const ConsoleRedirector&);
};


/** Component for writing restart files.  Creation and destruction of
    archive and associated stream are managed here. */
class RestartWriter {

public:
  /// optional default ctor allowing a non-outputting RestartWriter
  RestartWriter();

  /// typical ctor taking a filename
  RestartWriter(const String& write_restart_filename);
  
  /// output filename for this writer
  const String& filename();

  // TODO: operator &
  /// add the passed pair to the restart file
  void append_prp(const ParamResponsePair& prp_in);

  /// flush the restart stream so we have a complete restart record
  /// should Dakota abort
  void flush();

private:
  /// copy constructor is disallowed due to file stream
  RestartWriter(const RestartWriter&);
  /// assignment is disallowed due to file stream
  const RestartWriter& operator=(const RestartWriter&);

  /// the name of the restart output file
  String restartOutputFilename;

  /// Binary stream to which restart data is written
  std::ofstream restartOutputFS;

  /// Binary output archive to which data is written (pointer since no
  /// default ctor for oarchive and may not be initialized); 
  boost::scoped_ptr<boost::archive::binary_oarchive> restartOutputArchive;

};  // class RestartWriter



// TODO: tagging for pre/run/post I/O files
// TODO: consider a map of redirections with arbitrary rebinding
// TODO: better error checking in each function
// Consider a design with an array of output managers at
// ParallelLibrary instead of arrays of streams here...


/// Class to manage redirection of stdout/stderr, keep track of
/// current redir state, and manage rank 0 output.  Also manage
/// tabular data output for post-processing with Matlab, Tecplot,
/// etc. and delegate to Graphics for X Windows Graphics
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

  /// retrieve the graphics handler object
  Graphics& graphics() { return dakotaGraphics; }


  // -----
  // Modify output settings
  // -----

  /// Extract environment options from ProblemDescDB and update from
  /// late updates to ProgramOptions
  void parse(const ProgramOptions& prog_opts, const ProblemDescDB& problem_db);

  /// Set the Dakota startup message ("Running on...")
  void startup_message(const String& start_msg);

  /// Update the tag to use on files and rebind any streams as needed
  void push_output_tag(const String& iterator_tag, 
		       const ProgramOptions& prog_opts,
		       bool force_cout_redirect, bool force_rst_redirect);

  /// return the full output tag
  String build_output_tag() const;
  
  /// (Potentially) remove an output context and rebind streams
  void pop_output_tag();


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
  // Graphics and tabular output
  // -----

  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file based on the results of a model evaluation
  void add_datapoint(const Variables& vars, const String& iface, 
		     const Response& response);
  
  /// initialize the tabular datastream on iterator leaders
  void create_tabular_datastream(const Variables& vars, const Response& resp);

  /// close tabular datastream
  void close_tabular();

  /// set graphicsCntr equal to cntr
  void graphics_counter(int cntr);

  /// return graphicsCntr
  int graphics_counter() const;

  /// set tabularCntrLabel equal to label
  void tabular_counter_label(const std::string& label);


  // -----
  // Data to later be made private
  // -----

  bool graph2DFlag;       ///< whether user requested 2D graphics plots
  bool tabularDataFlag;   ///< whether user requested tabular data file
  bool resultsOutputFlag; ///< whether user requested results data output

   // // For items from the environment spec, can use DataEnvironment defaults
   //  tabular_filename       = outputManager.tabularDataFile;
   //  results_filename       = outputManager.resultsOutputFile;

  // Note: the following are raw untagged versions from parse():

  String tabularDataFile;   ///< filename for tabulation of graphics data
  String resultsOutputFile; ///< filename for results data

private:

  /// Perform initial output/error redirects from user requests
  void initial_redirects(const ProgramOptions& prog_opts);
  
  /// conditionally import evaluations from restart file, then always
  /// create or overwrite restart file
  void read_write_restart(bool restart_requested, bool read_restart_flag,
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

  /// set of tags for various input/output files (default none)
  StringArray fileTags;

  /// temporary variable to prevent recursive tagging initially
  bool redirCalled;

  /// set of redirections for Dakota::Cout; stores any tagged filename
  /// when there are concurrent Iterators
  ConsoleRedirector coutRedirector;

  /// set of redirections for Dakota::Cerr; stores any tagged filename
  /// when there are concurrent Iterators and error redirection is
  /// requested
  ConsoleRedirector cerrRedirector;

  /// Stack of active restart destinations; end is the last (active)
  /// redirection. All remain open until popped or destroyed.
  std::vector<boost::shared_ptr<RestartWriter> > restartDestinations;

  /// message to print at startup when proceeding to instantiate objects
  String startupMessage;


  /// graphics and tabular data output handler used by meta-iterators,
  /// models, and approximations; encapsulated here so destroyed with
  /// the OutputManager
  Graphics dakotaGraphics;     

  // For tabular output
  // -----

  /// tabular format options; see enum
  unsigned short tabularFormat;

  /// used for x axis values in 2D graphics and for 1st column in tabular data
  int graphicsCntr;

  /// file stream for tabulation of graphics data within compute_response
  std::ofstream tabularDataFStream;

  /// label for counter used in first line comment w/i the tabular data file
  std::string tabularCntrLabel;

  /// output level (for debugging only; not passed in)
  short outputLevel;

};  // class OutputManager


} //namespace Dakota

#endif  // DAKOTA_OUTPUT_MANAGER_H
