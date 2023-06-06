/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
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
#include "dakota_tabular_io.hpp"
#include "DakotaGraphics.hpp"
#include "RestartVersion.hpp"
#include <memory>


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
  std::vector<std::shared_ptr<OutputWriter> > ostreamDestinations;

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

  /// typical ctor taking a filename; this class encapsulates the output stream
  RestartWriter(const String& write_restart_filename,
		bool write_version = true);

  /// alternate ctor taking non-default version info, helpful for testing
  RestartWriter(const String& write_restart_filename,
		const RestartVersion& rst_version);

  /// alternate ctor taking a stream, helpful for testing; assumes
  /// client manages the output stream
  RestartWriter(std::ostream& write_restart_stream);
  
  /// output filename for this writer
  const String& filename();

  /// serialize the passed data_out to the restart file
  template<typename T>
  void operator&(const T& data_out)
  { restartOutputArchive->operator&(data_out); }

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
  std::unique_ptr<boost::archive::binary_oarchive> restartOutputArchive;

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

  /// open the tabular datastream on iterator leaders
  void open_tabular_datastream();
  /// output a complete header to the tabular datastream
  void create_tabular_header(const Variables& vars, const Response& resp);
  /// initiate the header for the tabular datastream with the leading fields
  void create_tabular_header(const StringArray& iface_ids);

  /// append variables labels to the tabular header
  void append_tabular_header(const Variables& vars);
  /// append a range of variables labels to the tabular header
  void append_tabular_header(const Variables& vars, size_t start_index,
			     size_t num_items);
  /// append an array of labels to the tabular header
  void append_tabular_header(const StringArray& labels, bool rtn = false);
  /// append response labels to the tabular header
  void append_tabular_header(const Response& response);

  // all tabular data at once:

  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file for the evaluation variables/response
  void add_tabular_data(const Variables& vars, const String& iface, 
			const Response& response);

  // fine-grained options:

  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file for the evaluation variables
  void add_tabular_data(const Variables& vars);
  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file for a portion of the evaluation variables
  void add_tabular_data(const Variables& vars, size_t start_index,
			size_t num_items);
  /// adds data to a row of the tabular data file for the interface id
  void add_tabular_data(const StringArray& iface_ids);
  // adds data to each window in the 2d graphics and adds a row to
  // the tabular data file for the evaluation interface id
  //void add_tabular_data(const String& iface);
  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file for the response functions
  void add_tabular_data(const Response& response, bool eol = true);
  /// adds data to each window in the 2d graphics and adds a row to
  /// the tabular data file for a portion of the response functions
  void add_tabular_data(const Response& response, size_t start_index,
			size_t num_items);
  /// augments the data set for a row in the tabular data file
  template<class T> 
  void add_tabular_scalar(T val);
  /// complete tabular row with EOL
  void add_eol();

  /// close tabular datastream
  void close_tabular_datastream();

  /// set graphicsCntr equal to cntr
  void graphics_counter(int cntr);

  /// return graphicsCntr
  int graphics_counter() const;

  /// set tabularCntrLabel equal to label
  void tabular_counter_label(const std::string& label);


  // -----
  // Results DB outputs
  // -----

  /// At runtime, initialize the global ResultsManager, tagging
  /// filename with MPI worldRank + 1 if needed
  void init_results_db();

  /// Archive the input file to the results database
  void archive_input(const ProgramOptions &prog_opts) const;

  /// Checked the passed input file or string for output/error_file
  /// and redirect accordingly. Command line options take precedence
  /// over input file options.
  void check_input_redirs(const ProgramOptions& prog_opts,
			  const std::string& input_file,
			  const std::string& input_string);

  /// check the specified input file contents for output/error redirection
  static void check_inputfile_redirs(const std::string& input_string,
				     std::string& output_filename,
				     std::string& error_filename);

  /// check the specified input string contents for output/error redirection
  static void check_inputstring_redirs(const std::string& input_string,
				       std::string& output_filename,
				       std::string& error_filename);

  /// check the passed input file stream for output/error redirection
  static void check_input_redirs_impl(std::istream& input_stream,
				      std::string& output_filename,
				      std::string& error_filename);

  // -----
  // Data to later be made private
  // -----

  bool graph2DFlag;       ///< whether user requested 2D graphics plots
  bool tabularDataFlag;   ///< whether user requested tabular data file
  bool resultsOutputFlag; ///< whether user requested results data output

   // For items from the environment spec, can use DataEnvironment defaults
   //  tabular_filename       = outputManager.tabularDataFile;
   //  results_filename       = outputManager.resultsOutputFile;

  // Note: the following are raw untagged versions from parse():

  String tabularDataFile;   ///< filename for tabulation of graphics data
  String resultsOutputFile; ///< filename for results data

  /// Models selected to store their evaluations
  unsigned short modelEvalsSelection;
  /// Interfaces selected to store their evaluations
  unsigned short interfEvalsSelection;

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

  /// set of redirections for Dakota::Cout; stores any tagged filename
  /// when there are concurrent Iterators
  ConsoleRedirector coutRedirector;

  /// set of redirections for Dakota::Cerr; stores any tagged filename
  /// when there are concurrent Iterators and error redirection is
  /// requested
  ConsoleRedirector cerrRedirector;

  /// Stack of active restart destinations; end is the last (active)
  /// redirection. All remain open until popped or destroyed.
  std::vector<std::shared_ptr<RestartWriter> > restartDestinations;

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
  /// label for interface used in first line comment w/i the tabular data file
  std::string tabularInterfLabel;

  /// output level (for debugging only; not passed in)
  short outputLevel;

  /// Output results  format
  unsigned short resultsOutputFormat;
};


template<class T> 
void OutputManager::add_tabular_scalar(T val)
{
  // post to the X graphics plots (active variables only)
  //dakotaGraphics.add_datapoint(graphicsCntr, val);
  
  // whether the file is open, not whether the user asked
  if (tabularDataFStream.is_open())
    TabularIO::write_scalar_tabular(tabularDataFStream, val);
}


inline void OutputManager::add_eol()
{
  if (tabularDataFStream.is_open())
    TabularIO::write_eol(tabularDataFStream);
}

} //namespace Dakota

#endif  // DAKOTA_OUTPUT_MANAGER_H
