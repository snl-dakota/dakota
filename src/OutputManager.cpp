/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       OutputManager
//- Description: Implementation code for the OutputManager class
//- Owner:       Brian Adams
//- Checked by:

#include <boost/algorithm/string/predicate.hpp>
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

//#define OUTMGR_DEBUG 1

namespace Dakota {

// Note: MSVC requires these externs defined outside any function
extern PRPCache data_pairs;
extern ResultsManager iterator_results_db;

// BMA TODO: consider removing or reimplementing
/** Heartbeat function provided by dakota_filesystem_utils; pass
    output interval in seconds, or -1 to use $DAKOTA_HEARTBEAT */
void start_dakota_heartbeat(int);


OutputManager::OutputManager():
  graph2DFlag(false), tabularDataFlag(false), resultsOutputFlag(false), 
  worldRank(0), mpirunFlag(false), 
  redirCalled(false), 
  coutRedirector(dakota_cout, &std::cout), 
  cerrRedirector(dakota_cerr, &std::cerr),
  tabularFormat(TABULAR_ANNOTATED),
  graphicsCntr(1), tabularCntrLabel("eval_id"), outputLevel(NORMAL_OUTPUT)
{  /* empty ctor */  }


/// Only get minimal information off ProgramOptions as may be updated
/// later by broadcast
OutputManager::
OutputManager(const ProgramOptions& prog_opts, int dakota_world_rank,
	      bool dakota_mpirun_flag):
  graph2DFlag(false), tabularDataFlag(false), resultsOutputFlag(false),
  worldRank(dakota_world_rank), mpirunFlag(dakota_mpirun_flag), 
  redirCalled(false), 
  coutRedirector(dakota_cout, &std::cout), 
  cerrRedirector(dakota_cerr, &std::cerr),
  graphicsCntr(1), tabularCntrLabel("eval_id"), outputLevel(NORMAL_OUTPUT)
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
  tabularFormat = problem_db.get_ushort("environment.tabular_format");

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

  // for now protect results DB from more than one call
  if (!redirCalled) {
    if (resultsOutputFlag)
      iterator_results_db.initialize(resultsOutputFile + file_tag);
    redirCalled = true;
  }

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
  boost::shared_ptr<RestartWriter> rst_writer = restartDestinations.back();
  rst_writer->append_prp(prp);
  // flush is critical so we have a complete restart record should Dakota abort
  rst_writer->flush();
}


/** Opens the tabular data file stream and prints headings, one for
    each active continuous and discrete variable and one for each response
    function, using the variable and response function labels. This
    tabular data is used for post-processing of DAKOTA results in
    Matlab, Tecplot, etc. */
void OutputManager::
create_tabular_datastream(const Variables& vars, const Response& response)
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

  // tabular graphics data only supports annotated format, active AND inactive
  // TODO: only write header if newly opened?
  TabularIO::write_header_tabular(tabularDataFStream, vars, response, "eval_id",
				  tabularFormat);
}


/** Adds data to each 2d plot and each tabular data column (one for
    each active variable and for each response function).
    graphicsCntr is used for the x axis in the graphics and the first
    column in the tabular data.  */
void OutputManager::
add_datapoint(const Variables& vars, const String& iface,
	      const Response& response)
{
  // If the response data only contains derivative info, then there are no
  // response function values to record in either the graphics window or the
  // tabular data file.
  bool plot_data = false;
  const ShortArray& asv = response.active_set_request_vector();
  int i, num_fns = asv.size();
  for (i=0; i<num_fns; ++i) {
    if (asv[i] & 1) {
      plot_data = true;
      break;
    }
  }
  if (!plot_data)
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


void OutputManager::tabular_counter_label(const std::string& label)
{ tabularCntrLabel = label; }


void OutputManager::graphics_counter(int cntr)
{ graphicsCntr = cntr; }


int OutputManager::graphics_counter() const
{ return graphicsCntr; }


void OutputManager::read_write_restart(bool restart_requested,
				       bool read_restart_flag,
				       const String& read_restart_filename,
				       size_t stop_restart_evals,
				       const String& write_restart_filename)
{
  // If no restart requested, push back a level that doesn't open
  // files so we can later pop it
  if (!restart_requested) {
    boost::shared_ptr<RestartWriter> rst_writer(new RestartWriter());
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
    
    std::ifstream restart_input_fs(read_restart_filename.c_str(), 
				   std::ios::binary);
    if (!restart_input_fs.good()) {
      Cerr << "\nError: could not open restart file " << read_restart_filename
	   << std::endl;
      abort_handler(-1);
    }
    boost::archive::binary_iarchive restart_input_archive(restart_input_fs);

    // The -stop_restart input for restricting the number of evaluations read
    // in from the restart file is very useful when the last few evaluations in
    // a run were corrupted.  Note that the desired -stop_restart setting may
    // differ from the evaluation number in the previous run since detected 
    // duplicates are included in Interface::evalIdCntr, but are not written
    // to the restart file!
    if (stop_restart_evals)// cmd_line_handler rtns 0 if no setting
      Cout << "Stopping restart file processing at "
	   << stop_restart_evals << " evaluations." << std::endl;

    int cntr = 0;
    restart_input_fs.peek(); // peek to force EOF if the last record was read
    while ( restart_input_fs.good() && !restart_input_fs.eof() && 
            (!stop_restart_evals ||
	     cntr < stop_restart_evals) ) {
      // Use default constr. & rely on Variables::read(BiStream&)
      // & Response::read(BiStream&) to resize vars and response.
      ParamResponsePair current_pair;
      try { 
	restart_input_archive & current_pair;
      }
      catch(const boost::archive::archive_exception& e) {
	Cerr << "\nError reading restart file '" << read_restart_filename 
	     << "' (boost::archive exception):\n      " << e.what()<< std::endl;
	abort_handler(IO_ERROR);
      }
      // serialization functions no longer throw strings

      read_pairs.insert(current_pair);
      ++cntr;
      Cout << "\n------------------------------------------\nRestart record "
	   << std::setw(4) << cntr << "  (evaluation id " << std::setw(4)
	   << current_pair.eval_id() << "):"
	   << "\n------------------------------------------\n" << current_pair;
      // Note: interface id printed in ParamResponsePair::write(ostream&)

      restart_input_fs.peek(); // peek to force EOF if the last record was read
    }
    restart_input_fs.close();
    Cout << "Restart file processing completed: " << cntr
	 << " evaluations retrieved.\n";
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
    Cout << "Overwriting existing restart file " << write_restart_filename 
	 << std::endl;
  else
    Cout << "Writing new restart file " << write_restart_filename << std::endl;

  // create a new restart destination
  boost::shared_ptr<RestartWriter> 
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
    boost::shared_ptr<OutputWriter> 
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
    boost::shared_ptr<OutputWriter> 
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


RestartWriter::RestartWriter(const String& write_restart_filename):
  restartOutputFilename(write_restart_filename),
  restartOutputFS(restartOutputFilename.c_str(), std::ios::binary),
  restartOutputArchive(new boost::archive::binary_oarchive(restartOutputFS))
{  /* empty ctor */  }


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

