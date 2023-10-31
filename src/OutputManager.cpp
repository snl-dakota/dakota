/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include <memory>
#include <utility>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include "dakota_global_defs.hpp"
#include "OutputManager.hpp"
#include "ProgramOptions.hpp"
#include "ProblemDescDB.hpp"
#include "ParamResponsePair.hpp"
#include "PRPMultiIndex.hpp"
#include "DakotaGraphics.hpp"
#include "ResultsManager.hpp"
#include "DakotaBuildInfo.hpp"
#include "dakota_tabular_io.hpp"
#include "ResultsDBAny.hpp"
#include "EvaluationStore.hpp"

#ifdef DAKOTA_HAVE_HDF5
#include "HDF5_IO.hpp"
#include "ResultsDBHDF5.hpp"
#endif
//#define OUTMGR_DEBUG 1

namespace Dakota {

// Note: MSVC requires these externs defined outside any function
extern PRPCache data_pairs;
extern ResultsManager iterator_results_db;
extern EvaluationStore evaluation_store_db;

// BMA TODO: consider removing or reimplementing
/** Heartbeat function provided by dakota_filesystem_utils; pass
    output interval in seconds, or -1 to use $DAKOTA_HEARTBEAT */
void start_dakota_heartbeat(int);


OutputManager::OutputManager():
  graph2DFlag(false), tabularDataFlag(false), resultsOutputFlag(false), 
  worldRank(0), mpirunFlag(false), 
  coutRedirector(dakota_cout, &std::cout), 
  cerrRedirector(dakota_cerr, &std::cerr),
  tabularFormat(TABULAR_ANNOTATED),
  graphicsCntr(1), tabularCntrLabel("eval_id"),
  tabularInterfLabel("interface"), outputLevel(NORMAL_OUTPUT)
{  /* empty ctor */  }


/// Only get minimal information off ProgramOptions as may be updated
/// later by broadcast
OutputManager::
OutputManager(const ProgramOptions& prog_opts, int dakota_world_rank,
	      bool dakota_mpirun_flag):
  graph2DFlag(false), tabularDataFlag(false), resultsOutputFlag(false),
  worldRank(dakota_world_rank), mpirunFlag(dakota_mpirun_flag), 
  coutRedirector(dakota_cout, &std::cout), 
  cerrRedirector(dakota_cerr, &std::cerr),
  graphicsCntr(1), tabularCntrLabel("eval_id"),
  tabularInterfLabel("interface"), outputLevel(NORMAL_OUTPUT)
{
  // This call will redirect based on command-line options
  initial_redirects(prog_opts);

  if (!mpirunFlag
      // || (mpirunFlag && worldRank == 0) 
      )
    start_dakota_heartbeat(-1); // -1 ==> take interval from $DAKOTA_HEARTBEAT
}


OutputManager::~OutputManager()
{
  close_streams();
}


void OutputManager::initial_redirects(const ProgramOptions& prog_opts)
{
  // This will duplicate a redirector for the case of command-line
  // option as that takes precedence over input file / environment and
  // the name won't have changed, but the user-requested redirect flag
  // will still be true.  This behavior is okay because the redirector
  // will see the same filename and not reopen the file.

  //  if output file specified, redirect immediately, possibly rebind later
  if (worldRank == 0 && prog_opts.user_stdout_redirect()) {
    if (outputLevel >= DEBUG_OUTPUT)
      std::cout << "\nRedirecting Cout on rank 0 to " << prog_opts.output_file()
                << std::endl;
    coutRedirector.push_back(prog_opts.output_file());
  }

  //  if error file specified, redirect immediately, possibly rebind later
  if (worldRank == 0 && prog_opts.user_stderr_redirect())
    cerrRedirector.push_back(prog_opts.error_file());
}


void OutputManager::check_input_redirs(const ProgramOptions& prog_opts,
				       const std::string& input_file,
				       const std::string& input_string)
{
  std::string input_specified_output, input_specified_error;
  if (!input_file.empty())
    check_inputfile_redirs(input_file, input_specified_output,
			   input_specified_error);
  else if (!input_string.empty())
    check_inputstring_redirs(input_string, input_specified_output,
			     input_specified_error);

  //  if output file specified, redirect immediately, possibly rebind later
  if (!prog_opts.user_stdout_redirect() && !input_specified_output.empty() &&
      worldRank == 0) {
    if (outputLevel >= DEBUG_OUTPUT)
      std::cout << "\nRedirecting Dakota standard output on rank 0 to "
		<< input_specified_output << std::endl;
    coutRedirector.push_back(input_specified_output);
  }

  // if error file specified, redirect immediately, possibly rebind later
  if (!prog_opts.user_stderr_redirect() && !input_specified_error.empty() &&
      worldRank == 0)
    cerrRedirector.push_back(input_specified_error);
}


void OutputManager::check_inputfile_redirs(const std::string& input_filename,
					   std::string& output_filename,
					   std::string& error_filename)
{
  // TODO: Check file operation exceptions
  std::ifstream infile(input_filename);
  OutputManager::check_input_redirs_impl(infile, output_filename, error_filename);
}


void OutputManager::check_inputstring_redirs(const std::string& input_string,
					     std::string& output_filename,
					     std::string& error_filename)
{
  std::istringstream infile(input_string);
  OutputManager::check_input_redirs_impl(infile, output_filename, error_filename);
}


/** This has a stream API to permit multiline matching. */
void OutputManager::check_input_redirs_impl(std::istream& input_stream,
					    std::string& output_filename,
					    std::string& error_filename)
{
  // RATIONALE: This doesn't allow abbreviations due to similar
  // keywords output_filter and error_factors. The regexs for
  // output/error_file require it to be anchored at word boundary (\b)
  // to avoid matching prefixed content or similar keywords.

  // symmetric-quoted filename with a match group for the contained filename;
  // the filename sub-group will always exist if the regex matches
  const std::string quoted_filename = "(['\"])(.+?)\\1";
  auto const quoted_filename_re = boost::regex(quoted_filename);
  auto const out_kw = boost::regex("\\boutput_file");
  // NIDR accepts keyword with adjoining token without whitespace (?!), but
  // we prohibit multiple =
  auto const out_kw_with_filename =
    boost::regex("\\boutput_file\\s*=?\\s*" + quoted_filename);
  auto const err_kw = boost::regex("\\berror_file");
  auto const err_kw_with_filename =
    boost::regex("\\berror_file\\s*=?\\s*" + quoted_filename);

  std::string line;
  while (std::getline(input_stream, line)) {

    boost::smatch matches;

    if(boost::regex_search(line, matches, out_kw_with_filename) && 
       !strcontains(matches.prefix(), "#") )
      output_filename = matches.str(2);
    // match only the keyword, then look ahead
    else if(boost::regex_search(line, matches, out_kw) &&
       !strcontains(matches.prefix(), "#") ) {
      // search until an uncommented quoted file name
      while (std::getline(input_stream, line)) {
	if (boost::regex_search(line, matches, quoted_filename_re) &&
	    !strcontains(matches.prefix(), "#")) {
	  output_filename = matches.str(2);
	  break;
	}
      }
    }

    if(boost::regex_search(line, matches, err_kw_with_filename) &&
       !strcontains(matches.prefix(), "#") )
      error_filename = matches.str(2);
    // match only the keyword, then look ahead
    else if(boost::regex_search(line, matches, err_kw) &&
       !strcontains(matches.prefix(), "#") ) {
      // search until an uncommented quoted file name
      while (std::getline(input_stream, line)) {
	if (boost::regex_search(line, matches, quoted_filename_re) &&
	    !strcontains(matches.prefix(), "#")) {
	  error_filename = matches.str(2);
	  break;
	}
      }
    }

  }
}


void OutputManager::close_streams()
{
  // cout/cerr will be restored to default when the redirector is destroyed

  // any remaining restart files will be closed at the destructor...
  //restartDestinations.clear();

  // After completion of timings in ParallelLibrary... 
  //
  // Don't need rank-based protection as will only be closed if they
  // were opened.  Do need user flag protection, otherwise dummy output
  // manager will close the global streams a second time wrongly.
  // BMA TODO: consider adding flag to track whether active...
  if (graph2DFlag || tabularDataFlag) {
    if (graph2DFlag)
      dakotaGraphics.close();
    // only close tabular stream if initialization was previously performed
    // not an error when not open so all ranks can call this
    if (tabularDataFlag && tabularDataFStream.is_open())
      tabularDataFStream.close();

    // could omit entirely or do this unconditionally...
    graphicsCntr = 1;
  }
}


void OutputManager::parse(const ProgramOptions& prog_opts, 
                          const ProblemDescDB& problem_db)
{
  initial_redirects(prog_opts);

  graph2DFlag = problem_db.get_bool("environment.graphics");
  tabularDataFlag = problem_db.get_bool("environment.tabular_graphics_data");
  tabularDataFile = problem_db.get_string("environment.tabular_graphics_file");
  resultsOutputFlag = problem_db.get_bool("environment.results_output");
  resultsOutputFile = problem_db.get_string("environment.results_output_file");
  modelEvalsSelection = problem_db.get_ushort("environment.model_evals_selection");
  interfEvalsSelection = problem_db.get_ushort("environment.interface_evals_selection");
  tabularFormat = problem_db.get_ushort("environment.tabular_format");
  resultsOutputFormat = problem_db.get_ushort("environment.results_output_format");
  if(resultsOutputFlag && resultsOutputFormat == 0)
    resultsOutputFormat = RESULTS_OUTPUT_TEXT;
  
  int db_write_precision = problem_db.get_int("environment.output_precision");
  if (db_write_precision > 0) {  // assign global write_precision
    if (db_write_precision > 16) {
      std::cout << "\nWarning: requested output_precision exceeds DAKOTA's "
		<< "internal precision;\n         resetting to 16."<< std::endl;
      write_precision = 16;
    }
    else
      write_precision = db_write_precision;
  }
}


void OutputManager::startup_message(const String& start_msg) 
{ startupMessage = start_msg; }


void OutputManager::
push_output_tag(const String& iterator_tag, const ProgramOptions& prog_opts,
		bool force_cout_redirect,
		bool force_rst_redirect)
{
  fileTags.push_back(iterator_tag); 

  String file_tag = build_output_tag();
  
  if (outputLevel >= DEBUG_OUTPUT)
    std::cout << "\nDEBUG: Rank " << worldRank 
	      << " pushing output tag; new tag '" << file_tag 
	      << "'; force_redirect = " << force_cout_redirect << std::endl;

  // Only redirect if parallel config requires it (otherwise may bind
  // multiple MPI ranks to same stream); any default user-requested
  // redirect would have happened in the constructor
  if (force_cout_redirect)
    coutRedirector.push_back(prog_opts.output_file() + file_tag);
  else  // output file remains same as before (possibly std::cout)
    coutRedirector.push_back();

  // Only redirect if parallel config requires it AND user requested
  // error redirection; otherwise errors remain to the console
  if (force_cout_redirect && !prog_opts.error_file().empty())
    cerrRedirector.push_back(prog_opts.error_file() + file_tag);
  else  // error file remains same as before (possibly std::cerr)
    cerrRedirector.push_back();

  // We always write a restart file; need to be careful to only read
  // if first time encountering this name
  // TODO: review whether all read/write should be tagged
  bool read_restart_flag = !prog_opts.read_restart_file().empty();
  read_write_restart(force_rst_redirect, read_restart_flag, 
		     prog_opts.read_restart_file() + file_tag,
		     prog_opts.stop_restart_evals(),
		     prog_opts.write_restart_file() + file_tag);
}


String OutputManager::build_output_tag() const
{  return std::accumulate(fileTags.begin(), fileTags.end(), String());  }


/** For now this assumes the tag is .<int> */
void OutputManager::pop_output_tag()
{
  if (fileTags.empty()) {
    Cout << "\nWarning: Rank " << worldRank 
	 << " attempting to pop non-existent output tag." << std::endl;
    return;
  }

  fileTags.pop_back();

  if (outputLevel >= DEBUG_OUTPUT) {
    String file_tag = build_output_tag();
    std::cout << "\nDEBUG: Rank " << worldRank << " popping output tag; new tag '"
	      << file_tag << "'" << std::endl;
  }

  // the redirector will rebind to previous stream when popped
  coutRedirector.pop_back();
  cerrRedirector.pop_back();

  // the restart file will get closed if this pops the last reference to it
  if (restartDestinations.empty())
    Cout << "\nWarning: Attempt to pop non-existent restart destination!"
	 << std::endl;
  else
    restartDestinations.pop_back();
}


void OutputManager::output_version(std::ostream& os) const
{
  // Version is always output to Cout

  if (worldRank == 0) {
    std::string version_info("Dakota version ");

    // release version, possibly stable with '+'
    version_info += DakotaBuildInfo::get_release_num(); 
    if (boost::ends_with(DakotaBuildInfo::get_release_num(), "+"))
      version_info += " (stable)";
    version_info += " released " + DakotaBuildInfo::get_release_date() + ".\n"; 

    // subversion revision
    version_info += "Repository revision " 
      + DakotaBuildInfo::get_rev_number()
      + " built " + DakotaBuildInfo::get_build_date()
      + " " + DakotaBuildInfo::get_build_time() + ".";

    os << version_info << std::endl;
  }
}


void OutputManager::output_startup_message(std::ostream& os) const 
{
  if (worldRank == 0) {
    output_version(os);
    // Generate the startup header, now that streams are potentially reassigned
    os << startupMessage << '\n'; 
    std::time_t curr_time = std::time(NULL);
    std::string pretty_time(std::asctime(std::localtime(&curr_time))); 
    // asctime appends a newline, but use the endl to force flush
    os << "Start time: " << pretty_time << std::endl;
  }
}



void OutputManager::
output_helper(const String& message, std::ostream &os) const
{
  if (worldRank == 0)
    os << message << std::endl;
}


void OutputManager::append_restart(const ParamResponsePair& prp)
{
  if (restartDestinations.empty()) {
    Cerr << "\nError: Attempt to append to restart file when not open."
	 << std::endl;
    abort_handler(-1);
  }
  std::shared_ptr<RestartWriter> rst_writer = restartDestinations.back();
  rst_writer->append_prp(prp);
  // flush is critical so we have a complete restart record should Dakota abort
  rst_writer->flush();
}


/** Opens the tabular data file stream and prints headings, one for
    each active continuous and discrete variable and one for each response
    function, using the variable and response function labels. This
    tabular data is used for post-processing of DAKOTA results in
    Matlab, Tecplot, etc. */
void OutputManager::open_tabular_datastream()
{
  // For output/restart/tabular data, all Iterator masters stream
  // output so tabular graphics files need to be tagged

  // TODO: with multiple tags, this will be wrong (last pushed tag)

  // tabular data file set up
  // prevent multiple opens of tabular_data_file
  if (!tabularDataFStream.is_open()) {
    String file_tag = build_output_tag();
    TabularIO::open_file(tabularDataFStream, tabularDataFile + file_tag, 
			 "DakotaGraphics");
  }
}


/** Opens the tabular data file stream and prints headings, one for
    each active continuous and discrete variable and one for each response
    function, using the variable and response function labels. This
    tabular data is used for post-processing of DAKOTA results in
    Matlab, Tecplot, etc. */
void OutputManager::
create_tabular_header(const Variables& vars, const Response& response)
{
  // tabular graphics data only supports annotated format, active AND inactive
  // TODO: only write header if newly opened?
  TabularIO::write_header_tabular(tabularDataFStream, vars, response,
				  tabularCntrLabel, tabularInterfLabel,
				  tabularFormat);
}


void OutputManager::
create_tabular_header(const StringArray& iface_ids)
{
  TabularIO::write_header_tabular(tabularDataFStream, tabularCntrLabel,
				  iface_ids, tabularFormat);
}


void OutputManager::
append_tabular_header(const Variables& vars)
{ TabularIO::append_header_tabular(tabularDataFStream, vars, tabularFormat); }


void OutputManager::
append_tabular_header(const Variables& vars, size_t start_index,
		      size_t num_items)
{
  TabularIO::append_header_tabular(tabularDataFStream, vars, start_index,
				   num_items, tabularFormat);
}


void OutputManager::
append_tabular_header(const StringArray& labels, bool rtn)
{
  TabularIO::append_header_tabular(tabularDataFStream, labels, tabularFormat);
  if (rtn) tabularDataFStream << std::endl;
}


void OutputManager::
append_tabular_header(const Response& response)
{
  TabularIO::append_header_tabular(tabularDataFStream, response, tabularFormat);
}


/** Adds data to each 2d plot and each tabular data column (one for
    each active variable and for each response function).
    graphicsCntr is used for the x axis in the graphics and the first
    column in the tabular data.  */
void OutputManager::
add_tabular_data(const Variables& vars, const String& iface,
		 const Response& response)
{
  // If the response data only contains derivative info, then there are no
  // response function values to record in either the graphics window or the
  // tabular data file.
  const ShortArray& asv = response.active_set_request_vector();
  bool active_fns = false;
  size_t i, num_fns = asv.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i] & 1) // require values for plots / tabulation
      { active_fns = true; break; }
  if (!active_fns)
    return;
  
  // post to the X graphics plots (active variables only)
  dakotaGraphics.add_datapoint(graphicsCntr, vars, response);
  
  // whether the file is open, not whether the user asked
  if (tabularDataFStream.is_open()) {

    // Historically, only active variables were tabulated, for top
    // level evaluations/iterations in the strategy.  Now, all
    // (active/inactive) are written out to improve usability of
    // re-import and to sync with the "to_tabular" option of
    // dakota_restart_util, which tabulates all variables for all
    // application interface evaluations.

    // Since this tabular data file is used for multiple top-level
    // Iterator outputs, the counter may not be that from an interface

    TabularIO::write_data_tabular(tabularDataFStream, vars, iface, response,
     				  graphicsCntr, tabularFormat);
  }

  // Only increment the graphics counter if posting data (incrementing on every
  // call regardless of data posting causes skipping in the response plots).
  ++graphicsCntr;
}


void OutputManager::add_tabular_data(const StringArray& iface_ids)
{
  // In the more finely-grained case, forego the check on ASV fns
  // --> always generate a row, even if no active response fns

  if (tabularDataFStream.is_open())
    TabularIO::write_leading_columns(tabularDataFStream, graphicsCntr,
				     iface_ids, tabularFormat);
}


void OutputManager::add_tabular_data(const Variables& vars)
{
  // In the more finely-grained case, forego the check on ASV fns
  // --> always generate a row, even if no active response fns

  // post to the X graphics plots (active variables only)
  //dakotaGraphics.add_datapoint(graphicsCntr, vars);
  
  // whether the file is open, not whether the user asked
  if (tabularDataFStream.is_open())
    TabularIO::write_data_tabular(tabularDataFStream, vars);
}


void OutputManager::
add_tabular_data(const Variables& vars, size_t start_index, size_t num_items)
{
  // In the more finely-grained case, forego the check on ASV fns
  // --> always generate a row, even if no active response fns

  // post to the X graphics plots
  //dakotaGraphics.add_datapoint(graphicsCntr, vars, start_index, num_items);

  // whether the file is open, not whether the user asked
  if (tabularDataFStream.is_open())
    TabularIO::write_data_tabular(tabularDataFStream, vars, start_index,
				  num_items);
}


void OutputManager::add_tabular_data(const Response& response, bool eol)
{
  // In the more finely-grained case, forego the check on ASV fns
  // --> always generate a row, even if no active response fns

  // post to the X graphics plots (active variables only)
  //dakotaGraphics.add_datapoint(graphicsCntr, response);
  
  // whether the file is open, not whether the user asked
  if (tabularDataFStream.is_open())
    TabularIO::write_data_tabular(tabularDataFStream, response, eol);

  ++graphicsCntr;
}


void OutputManager::
add_tabular_data(const Response& response, size_t start_index, size_t num_items)
{
  // In the more finely-grained case, forego the check on ASV fns
  // --> always generate a row, even if no active response fns

  // post to the X graphics plots (active variables only)
  //dakotaGraphics.add_datapoint(graphicsCntr, response);
  
  // whether the file is open, not whether the user asked
  if (tabularDataFStream.is_open())
    TabularIO::write_data_tabular(tabularDataFStream, response,
				  start_index, num_items);

  ++graphicsCntr;
}


void OutputManager::close_tabular_datastream()
{
  if (tabularDataFStream.is_open()) {
    tabularDataFStream.close();
    //TabularIO::close_file(tabularDataFStream, ...);
  }
}


void OutputManager::tabular_counter_label(const std::string& label)
{ tabularCntrLabel = label; }


void OutputManager::graphics_counter(int cntr)
{ graphicsCntr = cntr; }


int OutputManager::graphics_counter() const
{ return graphicsCntr; }


void OutputManager::init_results_db()
{
  String file_tag;
  if (mpirunFlag)
    file_tag = "." + std::to_string(worldRank + 1);

  String filename = resultsOutputFile+file_tag;

  iterator_results_db.clear_databases();
  if(resultsOutputFormat & RESULTS_OUTPUT_TEXT) {
    std::unique_ptr<ResultsDBAny> db_ptr(new ResultsDBAny(filename + ".txt"));
    iterator_results_db.add_database(std::move(db_ptr));
  }
  if(resultsOutputFormat & RESULTS_OUTPUT_HDF5) {
  #ifdef DAKOTA_HAVE_HDF5
    // HDF5IOHelper object shared by ResultsManager and EvaluationStore
    std::shared_ptr<HDF5IOHelper> hdf5_helper_ptr(new HDF5IOHelper(filename + ".h5", true /* overwrite */));
    // 
    std::unique_ptr<ResultsDBHDF5> db_ptr(new ResultsDBHDF5(false /* in_core = false */, hdf5_helper_ptr));
    iterator_results_db.add_database(std::move(db_ptr));
    // initialize EvaluationStore
    evaluation_store_db.set_database(hdf5_helper_ptr);
    evaluation_store_db.model_selection(modelEvalsSelection);
    evaluation_store_db.interface_selection(interfEvalsSelection);
  #else
    Cerr << "WARNING: HDF5 results output was requested, but is not available in this build.\n";
  #endif
  }
}

void OutputManager::archive_input(const ProgramOptions &prog_opts) const {
  // Not strictly necessary to check, but it avoids potentially reading the
  // input file into memory needlessly.
  if(!iterator_results_db.active()) return;
  const String& dakota_input_file = prog_opts.input_file();
  const String& dakota_input_string = prog_opts.input_string();
  AttributeArray input_attr;

  if(!dakota_input_string.empty()) {
    input_attr.push_back(ResultAttribute<String>("input", dakota_input_string));
    iterator_results_db.add_metadata_to_study(input_attr);
  } else if(!dakota_input_file.empty()) {
      std::ifstream inputstream(dakota_input_file.c_str());
      if (!inputstream.good()) {
	Cerr << "\nError: Could not open input file '" << dakota_input_file 
	     << "' for reading." << std::endl;
	abort_handler(IO_ERROR);
      }
      std::stringstream input_sstr;
      input_sstr << inputstream.rdbuf();
      input_attr.push_back(ResultAttribute<String>("input", input_sstr.str()));
      iterator_results_db.add_metadata_to_study(input_attr);
  } 
}


void OutputManager::read_write_restart(bool restart_requested,
				       bool read_restart_flag,
				       const String& read_restart_filename,
				       size_t stop_restart_evals,
				       const String& write_restart_filename)
{
  // If no restart requested, push back a level that doesn't open
  // files so we can later pop it
  if (!restart_requested) {
    std::shared_ptr<RestartWriter> rst_writer(new RestartWriter());
    restartDestinations.push_back(rst_writer);
    return;
  }

  // True if this filename has already been read and opened for
  // writing (this restart name has already been processed), so push
  // it again; for now this assumes that if called twice with the same
  // filename, it is in immediate succession, i.e. no new tag added
  if ( !restartDestinations.empty() &&
       write_restart_filename == restartDestinations.back()->filename() ) {
    restartDestinations.push_back(restartDestinations.back());
    return;
  }

  // Conditionally process the evaluations from the restart file
  PRPCache read_pairs;
  if (read_restart_flag) {
    
    // catch errors with opening files and reading headers
    try {

      // warn on old restart file
      RestartVersion rst_ver =
	RestartVersion::check_restart_version(read_restart_filename);

      std::ifstream restart_input_fs(read_restart_filename.c_str(),
				     std::ios::binary);
      if (!restart_input_fs.good()) {
	Cerr << "\nError: could not open restart file '"
	     << read_restart_filename << "' for reading."<< std::endl;
	abort_handler(IO_ERROR);
      }
      boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

      Cout << "Reading restart file '" << read_restart_filename << "'.\n"
	   << "  Any unexpected errors may indicate a corrupt restart file; "
	   << "using -stop_restart\n  to truncate the read may help."
	   << std::endl;

      // re-read the full, correct version info from the new stream
      if (RestartVersion::restartFirstVersionNumber <= rst_ver.restartVersion)
	restart_input_archive & rst_ver;

      // The -stop_restart input for restricting the number of
      // evaluations read in from the restart file is very useful when
      // the last few evaluations in a run were corrupted.  Note that
      // the desired -stop_restart setting may differ from the
      // evaluation number in the previous run since detected
      // duplicates are included in Interface::evalIdCntr, but are not
      // written to the restart file!
      if (stop_restart_evals)// cmd_line_handler rtns 0 if no setting
	Cout << "Stopping restart file processing at "
	     << stop_restart_evals << " evaluations." << std::endl;

      int cntr = 0;
      restart_input_fs.peek(); // peek to force EOF if the last record was read
      while ( restart_input_fs.good() && !restart_input_fs.eof() &&
	      (!stop_restart_evals ||
	       cntr < stop_restart_evals) ) {
	// Use default ctor; relies on Variables and Response reads to size them
	ParamResponsePair current_pair;
	try {
	  // this reads vars (svd, vars), iface, resp, eval_id
	  // Would like to catch bad reads before bad allocs / segfaults...
	  restart_input_archive & current_pair;
	}
	// TODO: should it be the default to truncate and warn when bad?!?
	catch(const boost::archive::archive_exception& e) {
	  Cerr << "\nError reading restart file '" << read_restart_filename
	       << "'.\nYou may be able to recover the first " << cntr
	       << " evaluations with -stop_restart " << cntr
	       << ".\nDetails (boost::archive exception): "
	       << e.what() << std::endl;
	  abort_handler(IO_ERROR);
	}

	read_pairs.insert(current_pair);
	++cntr;
	Cout << "\n------------------------------------------\nRestart record "
	     << std::setw(4) << cntr << "  (evaluation id " << std::setw(4)
	     << current_pair.eval_id() << "):"
	     << "\n------------------------------------------\n"
	     << current_pair;
	// Note: interface id printed in ParamResponsePair::write(ostream&)

	restart_input_fs.peek(); // peek to force EOF if last record was read
      }
      restart_input_fs.close();
      Cout << "Restart file processing completed: " << cntr
	   << " evaluations retrieved.\n";
    }
    catch (const boost::archive::archive_exception& e) {
      // primarily to catch invalid_signature error or an immediately bum stream
      Cerr << "\nError reading restart file '" << read_restart_filename
	   << "' (empty or corrupt file).\nDetails (Boost archive exception): "
	   << e.what() << std::endl;
      abort_handler(IO_ERROR);
    }
    catch (const std::exception& e) {
      Cerr << "Unknown error reading restart file '" << read_restart_filename
	   << "'.\nIf some evaluations were read, using -stop_restart to "
	   << "truncate may help.\nDetails: " << e.what() << '\n';
      abort_handler(IO_ERROR);
    }

  }

  // Always write a restart log file.  Assign the write_restart stream to
  // the filename specified by the user on the dakota command line.  If a
  // write_restart file is not specified, "dakota.rst" is the default.

  // It would be desirable to suppress the creation of the restart file
  // altogether if the user has explicitly deactivated this feature; however
  // this is problematic for 2 reasons: (1) problem_db is not readily available
  // (except in init_iterator_communicators()), and (2) the "deactivate
  // restart_file" specification is linked to the interface and therefore should
  // be enforced per interface, whereas there is only one parallel lib instance.
  //if (!deactivateRestartFlag) {

  // Previously we supported append to an existing restart file.  With
  // Boost serialization, there's no easy way to append to a file (all
  // writes must occur with a single output archive instance).  This
  // also improves behavior with stop_restart, as now only the desired
  // evals are rewritten, omitting any corrupt data at the end of file.

  if (write_restart_filename == read_restart_filename)
    Cout << "Overwriting existing restart file '" << write_restart_filename 
	 << "'." << std::endl;
  else
    Cout << "Writing new restart file '" << write_restart_filename << "'."
	 << std::endl;

  try {

    // create a new restart destination
    std::shared_ptr<RestartWriter>
      rst_writer(new RestartWriter(write_restart_filename));
    restartDestinations.push_back(rst_writer);

    // Write any processed records from the old restart file to the new file.
    // This prevents the situation where good data from an initial run and a
    // restart run are in separate files.  By keeping all of the saved data in
    // 1 file, restarts can be chained together indefinitely.
    //
    // "View" the read_pairs cache as a collection ordered by eval_id

    if (!read_pairs.empty()) {
      // don't use a const iterator as PRP does not have a const serialize fn
      PRPCacheIter it, it_end = read_pairs.end();
      for (it = read_pairs.begin(); it != it_end; ++it) {
	// insert read records into new restart DB as is (no negation of id's)
	rst_writer->append_prp(*it);

	// Distinguish restart evals in memory by negating their eval ids;
	// positive ids could be misleading if inconsistent with the progression
	// of a restarted run (resulting in different evaluations that share the
	// same evalInterfaceIds).  Moreover, they may not be unique within the
	// restart DB in the presence of overlay/concatenation of multiple runs.
	//   id > 0 for unique evals from current execution (in data_pairs)
	//   id = 0 for evals from file import (not in data_pairs)
	//   id < 0 for non-unique evals from restart (in data_pairs)
	int restart_eval_id = it->eval_id();
	if (restart_eval_id > 0) {
	  ParamResponsePair pair(*it); // shallow vars/resp copy, deep ids copy
	  pair.eval_id(-restart_eval_id);
	  data_pairs.insert(pair);
	}
	else // should not be negative (see rst append above), but can be zero
	  data_pairs.insert(*it);
      }

      // flush is critical so we have a complete restart record in case of abort
      rst_writer->flush();
    }

  }
  catch (const boost::archive::archive_exception& e) {
    if (outputLevel > SILENT_OUTPUT)
      Cout << "\nWarning: Could not write restart file '" 
	   << write_restart_filename
	   << "'.\nDetails (Boost archive exception): "
	   << e.what() << std::endl;
  }

  //}
}


OutputWriter::OutputWriter(std::ostream* output_stream)
{ outputStream = output_stream; }

OutputWriter::OutputWriter(const String& output_filename):
  outputFilename(output_filename)
{
  outputFS.open(output_filename.c_str(), std::ios::out);
  if (!outputFS.good()) {
    Cerr << "\nError opening output file '" << output_filename << "'" 
	 << std::endl;
    abort_handler(-1);
  }
  outputStream = &outputFS;
}

const String& OutputWriter::filename() const
{ return outputFilename; }

 
std::ostream* OutputWriter::output_stream()
{ return outputStream; }


ConsoleRedirector::
ConsoleRedirector(std::ostream* & dakota_stream, std::ostream* default_dest):
  ostreamHandle(dakota_stream), defaultOStream(default_dest)
{ 
#ifdef OUTMGR_DEBUG
  std::cerr << "Constructing ConsoleRedirector with handle " << ostreamHandle
	    << " and default dest " << defaultOStream << std::endl;
#endif  
  /* empty ctor */ 
}


ConsoleRedirector::~ConsoleRedirector()
{
#ifdef OUTMGR_DEBUG
  std::cerr << "Destructing ConsoleRedirector setting handle " << ostreamHandle
	    << " to default dest " << defaultOStream << std::endl;
#endif 
  ostreamHandle = defaultOStream;
}


void ConsoleRedirector::push_back()
{
#ifdef OUTMGR_DEBUG
  std::cerr << "ConsoleRedirector appending additional redirect to current " 
	    << "destination " << std::endl;
#endif 
  if (ostreamDestinations.empty()) {
    std::shared_ptr<OutputWriter> 
      out_writer_ptr(new OutputWriter(defaultOStream));
    ostreamDestinations.push_back(out_writer_ptr);
  }
  else 
    ostreamDestinations.push_back(ostreamDestinations.back());
}


void ConsoleRedirector::push_back(const String& output_filename)
{
  if (output_filename.empty()) {
    push_back();
    return;
  }

  // if this is a new redirection, or a redirection with a new
  // filename, make a new stream, overwriting file contents
  if (ostreamDestinations.empty() || 
      ostreamDestinations.back()->filename() != output_filename) {
#ifdef OUTMGR_DEBUG
    if (ostreamDestinations.empty())
      std::cerr << "ConsoleRedirector appending new OutputWriter to filename "
		<< output_filename << std::endl;
    else
      std::cerr << "ConsoleRedirector appending new OutputWriter. Old filename "
		<< ostreamDestinations.back()->filename() << ". New filename "
		<< output_filename << std::endl;
#endif 
    std::shared_ptr<OutputWriter> 
      out_writer_ptr(new OutputWriter(output_filename));
    ostreamDestinations.push_back(out_writer_ptr);
  }
  // otherwise keep using the same stream
  else {
#ifdef OUTMGR_DEBUG
    std::cerr << "ConsoleRedirector appending same OutputWriter " << std::endl;
#endif 
    ostreamDestinations.push_back(ostreamDestinations.back());
  }

  ostreamHandle = ostreamDestinations.back()->output_stream();
#ifdef OUTMGR_DEBUG
  std::cerr << "ConsoleRedirector appending redirect to file " 
	    << output_filename << ". ostreamHandle now " 
	    << ostreamHandle << std::endl;
#endif 
}


void ConsoleRedirector::pop_back()
{
  if (!ostreamDestinations.empty())
    ostreamDestinations.pop_back();
  else 
    Cerr << "\nWarning: Attempt to pop non-existent console output destination!"
	 << std::endl;

  // if stack is empty, rebind to the default
  if (ostreamDestinations.empty())
    ostreamHandle = defaultOStream;
  else
    ostreamHandle = ostreamDestinations.back()->output_stream();
#ifdef OUTMGR_DEBUG
  std::cerr << "ConsoleRedirector popping redirect. ostreamHandle now " 
	    << ostreamHandle << std::endl;
#endif 
}


RestartWriter::RestartWriter()
{  /* empty ctor */  }


RestartWriter::RestartWriter(const String& write_restart_filename,
			     bool write_version):
  restartOutputFilename(write_restart_filename),
  restartOutputFS(restartOutputFilename.c_str(), std::ios::binary)
{
  if (!restartOutputFS.good()) {
    Cerr << "\nError: could not open restart file '"
	 << write_restart_filename << "' for writing."<< std::endl;
    abort_handler(IO_ERROR);
  }

  restartOutputArchive.reset(new boost::archive::binary_oarchive(restartOutputFS));

  if (write_version) {
    RestartVersion rst_version(DakotaBuildInfo::get_release_num(),
			       DakotaBuildInfo::get_rev_number());
    restartOutputArchive->operator&(rst_version);
  }
}


RestartWriter::RestartWriter(const String& write_restart_filename,
			     const RestartVersion& rst_version):
  restartOutputFilename(write_restart_filename),
  restartOutputFS(restartOutputFilename.c_str(), std::ios::binary)
{
  if (!restartOutputFS.good()) {
    Cerr << "\nError: could not open restart file '"
	 << write_restart_filename << "' for writing."<< std::endl;
    abort_handler(IO_ERROR);
  }

  restartOutputArchive.reset(new boost::archive::binary_oarchive(restartOutputFS));

  restartOutputArchive->operator&(rst_version);
}


RestartWriter::RestartWriter(std::ostream& write_restart_ostream):
  restartOutputArchive(new boost::archive::binary_oarchive(write_restart_ostream))
{
  RestartVersion rst_version(DakotaBuildInfo::get_release_num(),
			     DakotaBuildInfo::get_rev_number());
  restartOutputArchive->operator&(rst_version);
}


const String& RestartWriter::filename()
{ return restartOutputFilename; }


void RestartWriter::append_prp(const ParamResponsePair& prp_in)
{ 
  if (restartOutputArchive)  // equivalent to NULL check
    restartOutputArchive->operator&(prp_in);
  else {
    Cerr << "\nError: attempt to write to invalid restart file." << std::endl;
    abort_handler(IO_ERROR);
  }
}

void RestartWriter::flush()
{ restartOutputFS.flush(); }


#ifdef Want_Heartbeat /*{*/
 static time_t start_time;

extern "C" void dak_sigcatch(int sig)
{
        struct rusage ru, ruc;
        unsigned long elapsed;

        if (getrusage(RUSAGE_SELF, &ru))
                std::memset(&ru, 0, sizeof(ru));
        if (getrusage(RUSAGE_SELF, &ruc))
                std::memset(&ruc, 0, sizeof(ru));
        elapsed = time(0) - start_time;
        std::printf("\n<<<<DAKOTA_HEARTBEAT seconds: elapsed %lu, cpu %.3g, child %.3g>>>>\n",
                elapsed, ru.ru_utime.tv_sec + ru.ru_stime.tv_sec
                        + 1e-6*(ru.ru_utime.tv_usec + ru.ru_stime.tv_usec),
                        ruc.ru_utime.tv_sec + ruc.ru_stime.tv_sec
                        + 1e-6*(ruc.ru_utime.tv_usec + ruc.ru_stime.tv_usec));
        std::fflush(stdout);
        signal(SIGALRM, dak_sigcatch);
}
#endif /*Want_Heartbeat }*/

void start_dakota_heartbeat(int seconds)
{
#ifdef Want_Heartbeat /*{*/
        char *s;
        struct itimerval itv;
        static void(*oldsig)(int);
        void (*oldsig1)(int);

        start_time = time(0);

        if (seconds <= 0 && (s = std::getenv("DAKOTA_HEARTBEAT")))
                seconds = (int)std::strtol(s,0,10);
        if (seconds > 0) {
                std::memset(&itv, 0, sizeof(itv));
                itv.it_interval.tv_sec = itv.it_value.tv_sec = seconds;
                setitimer(ITIMER_REAL, &itv, 0);
                oldsig1 = signal(SIGALRM, dak_sigcatch);
                if (!oldsig)
                        oldsig = oldsig1;
        }
        else if (oldsig) {
                signal(SIGALRM, oldsig);
                oldsig = 0;
        }
#endif /*Want_Heartbeat }*/
}

} //namespace Dakota

