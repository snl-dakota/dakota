/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ProcessApplicInterface.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "WorkdirHelper.hpp"
#include "ParametersFileWriter.hpp"
#include "ResultsFileReader.hpp"
#include <algorithm>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
/* 
    Work directory TODO
    
    * Doc: we will search for drivers in PATH, workdir (.), RUNDIR

    * Remove legacy utilities (once concepts migrated)

    - In general review cases with race conditions such as single dir
      getting created / removed for each eval.

    - Verify creation/removal in parallel runs (eval_comm_rank 0?);
      are there scenarios where we should create once ahead of time?

    - Enforce tagging when asynch is possible

    - Challenge of shared vs. distinct filesystems

    - Verify template files exist at parse and that workdir parent exists

    - Verify behavior when directory exists
  
    - Allow recursive copy to descend to overwrite leaf nodes when
      directories already exist

    - Old code setting permissions mask 0700

    - Workdir with multiple analysis components (per analysis)

    - Workdirs shared for each unique concurrent eval (not per eval ID)

    - Evaluate environment variables

    - How to manage drivers that want just param names in the work
      dir?!?  I think arg_adjust is removing the directory args that
      aren't needed.

    - Consider making the class members for directories and files std::filesystem::paths

    - Behavior of file_save when directory not saved

    - Error checking:
      * directory was created
      * directory path is a directory
      * directory has rwx for user
      * population worked
      * try/catch around all fs operations

    - Verify correct handling of relative vs. absolute files/dirs

    - Enforce that first argument must be an executable program for
      all drivers; at least for fork

    - Historical behaviors / features to consider
      * Template dirs on PATH: likely no longer

      * Allowed FOO=zorch and would set that in the environment; could
        allow separate env var specification; otherwise likely remove

    - TODO: pass environment to exec as separate pointer

   TESTING NEEDS
    - Allow nested quotes in driver, at least one level: 
      analysis_driver = 'ad.sh "-opt foo -opt1 goo"' p.in.1 r.out.1

    - Env vars will be carried along for now, not expanded before
      eval; set some helpful env vars before the eval.
*/


namespace Dakota {

// Regular expressions used to substitute the names of parameters and resutls
// files into driver strings
boost::regex PARAMS_TOKEN("\\{PARAMETERS\\}");
boost::regex RESULTS_TOKEN("\\{RESULTS\\}");

/// Substitute parameters and results file names into driver strings
String substitute_params_and_results(const String &driver, const String &params, const String &results) {
  String params_subbed  = boost::regex_replace(driver, PARAMS_TOKEN,  params);
  String results_subbed = boost::regex_replace(params_subbed, RESULTS_TOKEN,  results);
  return results_subbed;  
}


ProcessApplicInterface::
ProcessApplicInterface(const ProblemDescDB& problem_db):
  ApplicationInterface(problem_db), 
  fileTagFlag(problem_db.get_bool("interface.application.file_tag")),
  fileSaveFlag(problem_db.get_bool("interface.application.file_save")),
  commandLineArgs(!problem_db.get_bool("interface.application.verbatim")),
  paramsFileWriter(ParametersFileWriter::get_writer(problem_db.get_ushort("interface.application.parameters_file_format"))),
  resultsFileReader(ResultsFileReader::get_reader(
    problem_db.get_ushort("interface.application.results_file_format"),
    problem_db.get_bool("interface.labeled_results")
  )),
  multipleParamsFiles(false),
  iFilterName(problem_db.get_string("interface.application.input_filter")),
  oFilterName(problem_db.get_string("interface.application.output_filter")),
  programNames(problem_db.get_sa("interface.application.analysis_drivers")),
  specifiedParamsFileName(
    problem_db.get_string("interface.application.parameters_file")),
  specifiedResultsFileName(
    problem_db.get_string("interface.application.results_file")),
  allowExistingResults(problem_db.get_bool("interface.allow_existing_results")),
  useWorkdir(problem_db.get_bool("interface.useWorkdir")),
  workDirName(problem_db.get_string("interface.workDir")),
  dirTag(problem_db.get_bool("interface.dirTag")),
  dirSave(problem_db.get_bool("interface.dirSave")),
  linkFiles(problem_db.get_sa("interface.linkFiles")),
  copyFiles(problem_db.get_sa("interface.copyFiles")),
  templateReplace(problem_db.get_bool("interface.templateReplace"))
{
  // When using work directory, relative analysis drivers starting
  // with . or .. may need to be converted to absolute so they work
  // from the work directory.  While inefficient, we convert them in
  // place as strings (could leave tokenized for Fork/Spawn, but
  // assemble the string for SysCall Interfaces
  if (useWorkdir) {
    StringArray::iterator pn_it = programNames.begin();
    StringArray::iterator pn_end = programNames.end();
    for ( ; pn_it != pn_end; ++pn_it)
      if (WorkdirHelper::resolve_driver_path(*pn_it) && 
	  outputLevel >= DEBUG_OUTPUT)
	Cout << "Adjusted relative analysis_driver to absolute path:\n  " 
	     << *pn_it << std::endl;
  }

  size_t num_programs = programNames.size();
  if (num_programs > 1 && !analysisComponents.empty())
    multipleParamsFiles = true;

  // RATIONALE: While a user might truly want concurrent evaluations
  // with non-unique work_directory and/or parameters/results files,
  // it could too easily lead to errors or surprising results.
  // Moreover, with asynch it's not clear which eval would make the
  // directory.  We therefore force tagging on either the directory or
  // the parameters/results files.

  // EDGE CASE: For MPI parallelism we could allow non-tagged dirs /
  // files since nodes may not share a common filesystem.  However, we
  // consider this an edge case and prefer to be robust, requiring
  // unique files/dirs.

  // BMA TODO: extend this to the message-passing concurrent
  // evaluation case in init_communicators.  Can't rely on
  // asynchLocalEvalConcurrency because this is set per parallel
  // configuration in set_communicators.

  bool require_unique =
    (interface_synchronization() == ASYNCHRONOUS_INTERFACE) &&
    (asynchLocalEvalConcSpec != 1) && 
    !batchEval;

  if (require_unique) {
    if (useWorkdir) {
      if (!dirTag && !workDirName.empty()) {
	Cout << "\nWarning: Concurrent local evaluations with named " 
	     << "work_directory require\n         directory_tag; "
	     << "enabling directory_tag." << std::endl;
	dirTag = true;
      }
      // even with a work_directory, there might be absolute
      // params/results paths...
      std::filesystem::path spf_path(specifiedParamsFileName);
      std::filesystem::path srf_path(specifiedResultsFileName);
      // an empty path is not absolute, so no need to check empty
      bool exist_abs_filenames = 
	spf_path.is_absolute() || srf_path.is_absolute(); 
      if (!fileTagFlag && exist_abs_filenames) {
	Cout << "\nWarning: Concurrent local evaluations with absolute named " 
	     << "parameters_file or\n         results_file require file_tag; "
	     << "enabling file_tag." << std::endl;
	fileTagFlag = true;
      }
    }
    else if (!fileTagFlag && 
	     (!specifiedParamsFileName.empty() || 
	      !specifiedResultsFileName.empty()) ) {
      Cout << "\nWarning: Concurrent local evaluations with named " 
	   << "parameters_file or\n         results_file require file_tag; "
	   << "enabling file_tag." << std::endl;
      fileTagFlag = true;
    }
  }

}


ProcessApplicInterface::~ProcessApplicInterface() 
{  /* empty dtor */  }


// -------------------------------------------------------
// Begin derived functions for evaluation level schedulers
// -------------------------------------------------------
void ProcessApplicInterface::
derived_map(const Variables& vars, const ActiveSet& set, Response& response,
	    int fn_eval_id)
{
  // This function may be executed by a multiprocessor evalComm.

  define_filenames(final_eval_id_tag(fn_eval_id)); // all evalComm
  if (evalCommRank == 0)
    write_parameters_files(vars, set, response, fn_eval_id);

  // execute the simulator application -- blocking call
  create_evaluation_process(BLOCK);

  try { 
    if (evalCommRank == 0)
      read_results_files(response, fn_eval_id, final_eval_id_tag(fn_eval_id));
  }

  // catch exception for ResultsFileError or TabularDataTruncated;
  // TODO: need to verify that tabular failure can happen
  catch(const FileReadException& fr_except) {
    // a FileReadException means detection of an incomplete file/data
    // set.  In the synchronous case, there is no potential for an incomplete 
    // file resulting from a race condition -> echo the error and abort.
    Cerr << fr_except.what() << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Do not call manage_failure() from the following catch since the
  // recursion of calling derived_map again would be confusing at
  // best.  The approach here is to have catch(FunctionEvalFailure)
  // rethrow the exception to an outer catch (either the catch within
  // manage_failure or a catch that calls manage_failure).  An
  // alternative solution would be to eliminate the try block above
  // and move all catches to the higher level of try { derived_map() }
  // - this would be simpler to understand but would replicate
  // catch(FileReadException) in map, manage_failure, and serve.  By
  // having catch(FileReadException) here and having
  // catch(FunctionEvalFailure) rethrow, we eliminate unnecessary
  // proliferation of catch(FileReadException).
  catch(const FunctionEvalFailure& fneval_except) { 
    // failure capture exception thrown by response.read()
    //Cout << "Rethrowing FunctionEvalFailure; message: " 
    //     << fneval_except.what() << std::endl;
    throw; // from this catch to the outer one in manage_failure
  }
}


void ProcessApplicInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  // This function may not be executed by a multiprocessor evalComm.
  if(!batchEval) {
    int fn_eval_id = pair.eval_id();
    define_filenames(final_eval_id_tag(fn_eval_id)); // all evalComm
    write_parameters_files(pair.variables(), pair.active_set(),
			 pair.response(),  fn_eval_id);
    // execute the simulator application -- nonblocking call
    pid_t pid = create_evaluation_process(FALL_THROUGH);
    // bind process id with eval id for use in synchronization
    map_bookkeeping(pid, fn_eval_id);
  }
}


void ProcessApplicInterface::wait_local_evaluations(PRPQueue& prp_queue)
{
  if (batchEval) wait_local_evaluation_batch(prp_queue);
  else           wait_local_evaluation_sequence(prp_queue);
}


void ProcessApplicInterface::test_local_evaluations(PRPQueue& prp_queue)
{
  if (batchEval) test_local_evaluation_batch(prp_queue);
  else           test_local_evaluation_sequence(prp_queue);
}


void ProcessApplicInterface::wait_local_evaluation_batch(PRPQueue& prp_queue)
{

  batchIdCntr++;
  // define_filenames sets paramsFileWritten and resultsFileWritten, taking
  // into consideration all the myriad settings the user could have provided,
  // hierarchical tagging, etc.
  String batch_id_tag = final_batch_id_tag();
  define_filenames(batch_id_tag);
  if(!allowExistingResults)
    std::remove(resultsFileWritten.c_str());
  std::vector<String> an_comps;
  if(!analysisComponents.empty())
    copy_data(analysisComponents, an_comps);
  std::remove(paramsFileWritten.c_str());
  paramsFileWriter->write_parameters_file(prp_queue, programNames[0], an_comps, evalTagPrefix, batchIdCntr, paramsFileWritten);

  // In this case, individual jobs have not been launched and we launch the
  // user's analysis driver once for the complete batch:
  create_evaluation_process(BLOCK);

  // Response.read() expects results for a single evaluation to be present in a
  // stream, but the results file for a batch evaluation will contain multiple.
  // The strategy here is to consume the results file one evaluation at a time,
  // placing the content for one evaluation into a stringstream, which will be
  // passed to Response.read(). The # character at the beginning of a line is
  // presumed to separate evaluations (content that follows on that line will 
  // be dropped), and the evaluations are presumed to be in the same order as 
  // prp_queue. Probably not a very robust way of doing things, but for purposes
  // of prototyping, it'll do.
  try {
    resultsFileReader->read_results_file(prp_queue, resultsFileWritten, batchIdCntr, completionSet);
  } catch(const FunctionBatchEvalFailure& e) {
    manage_failure(e.prp.variables(), e.response.active_set(), e.response, e.prp.eval_id());
  }
 
  file_and_workdir_cleanup(paramsFileWritten, resultsFileWritten, createdDir, batch_id_tag);

}

void ProcessApplicInterface::test_local_evaluation_batch(PRPQueue& prp_queue)
{ wait_local_evaluation_batch(prp_queue); }


// ------------------------
// Begin file I/O utilities
// ------------------------
void ProcessApplicInterface::define_filenames(const String& eval_id_tag)
{
  // Define modified file names by handling Unix temp file and tagging options.
  // These names are used in writing the parameters file, in defining the
  // Command Shell behavior in SysCallProcessApplicInterface.cpp, in
  // reading the results file, and in file cleanup.

  // Different analysis servers _must_ share the same parameters and results
  // file root names.  If temporary file names are not used, then each evalComm
  // proc can define the same names without need for interproc communication.
  // However, if temporary file names are used, then evalCommRank 0 must define
  // the names and broadcast them to other evalComm procs.

  // Behavior has been simplified to tag with either hierarchical tag,
  // e.g. 4.9.2, or just eval id tag, .2.  If directory_tag, workdirs
  // are tagged, if file_tag, directories are tagged, or both if both
  // specified.

  const ParallelConfiguration& pc = parallelLib.parallel_configuration();
  int eval_comm_rank   = parallelLib.ie_parallel_level_defined()
	? pc.ie_parallel_level().server_communicator_rank() : 0;
  int analysis_servers = parallelLib.ea_parallel_level_defined()
	? pc.ea_parallel_level().num_servers() : 1;

  // technically don't need to broadcast in some useWorkdir subcases
  // like absolute params/results filenames
  bool dynamic_filenames = specifiedParamsFileName.empty() || 
    specifiedResultsFileName.empty() || useWorkdir;

  bool bcast_flag = ( analysis_servers > 1 &&  dynamic_filenames );

  if (eval_comm_rank == 0 || !bcast_flag) {

    // eval_tag_prefix will be empty if no hierarchical tagging
    fullEvalId = eval_id_tag;

    // establish the workdir name first, since parameters files might go in it
    bool wd_created = false;
    if (useWorkdir) {
      // curWorkdir is used by Fork/SysCall arg_adjust
      curWorkdir = std::filesystem::path(get_workdir_name().string());
      // TODO: Create with 0700 mask?
      wd_created = WorkdirHelper::create_directory(curWorkdir, DIR_PERSIST);
      // copy/link tolerate empty items
      WorkdirHelper::copy_items(copyFiles, curWorkdir, templateReplace);
      WorkdirHelper::link_items(linkFiles, curWorkdir, templateReplace);
    }

    // non-empty createdDir communicates to write_parameters_files that
    // the directory was created specifically for this eval and should
    // be removed when results collected
    if (wd_created)
      createdDir = curWorkdir;
    else
      createdDir.clear();

    // define paramsFleName for use in write_parameters_files
    
    std::filesystem::path params_path(specifiedParamsFileName);

    // no user spec -> use temp files, possibly in workdir; generate relative
    if (specifiedParamsFileName.empty())
      params_path = WorkdirHelper::system_tmp_file("dakota_params");

    // append tag for all cases (previously temp files weren't tagged)
    if (fileTagFlag)
      params_path = WorkdirHelper::concat_path(params_path, fullEvalId);

    // TODO: do we want to modify the parameters file here or later?
    // Need to differentiate where the file is written, removed,
    // etc. from the name of it as passed to the analysis driver

    // could we consider the tmpdir to be a workdir...?

    // if a relative path, prepend with workdir or tmp, else use absolute name
    // BMA TODO: don't use native string here...
    paramsFileName = paramsFileWritten = params_path.string();
    if (params_path.is_relative()) {
      if (useWorkdir) {
	paramsFileWritten = (curWorkdir / params_path).string();
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "\nAdjusting parameters_file to " << paramsFileName 
	       << " due to work_directory usage." << std::endl;
      }
      else if (specifiedParamsFileName.empty()) {
	// historically temp files go in /tmp or C:\temp, not rundir
	paramsFileName = (WorkdirHelper::system_tmp_path()/params_path).string();
	paramsFileWritten = paramsFileName;
      }
      else
	paramsFileName = params_path.string();
    }
    else
      paramsFileName = params_path.string();
    

    // define resultsFileName for use in write_parameters_files

    std::filesystem::path results_path(specifiedResultsFileName);

    // no user spec -> use temp files, possibly in workdir; generate relative
    if (specifiedResultsFileName.empty())
      results_path = WorkdirHelper::system_tmp_file("dakota_results");

    // append tag for all cases (previously temp files weren't tagged)
    if (fileTagFlag)
      results_path = WorkdirHelper::concat_path(results_path, fullEvalId);

    // if a relative path, prepend with workdir or tmp, else use absolute name
    // BMA TODO: don't use native string here...
    resultsFileName = resultsFileWritten = results_path.string();
    if (results_path.is_relative()) {
      if (useWorkdir) {
	resultsFileWritten = (curWorkdir / results_path).string();
	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "\nAdjusting results_file to " << resultsFileName 
	       << " due to work_directory usage." <<std::endl;
      }
      else if (specifiedResultsFileName.empty()) {
	// historically temp files go in /tmp or C:\temp, not rundir
	resultsFileName = (WorkdirHelper::system_tmp_path()/results_path).string();
	resultsFileWritten = resultsFileName;
      }
      else
	resultsFileName = results_path.string();
    }
    else
      resultsFileName = results_path.string();

  }

  // Not broadcasting curWorkdir as we don't want other ranks to
  // remove it later.

  if (bcast_flag) {
    // Note that evalComm and evalAnalysisIntraComm are equivalent in this case
    // since multiprocessor analyses are forbidden when using system calls/forks
    // (enforced by ApplicationInterface::init_communicators()).
    if (eval_comm_rank == 0) {
      // pack the buffer with root file names for the evaluation
      MPIPackBuffer send_buffer;
      send_buffer << paramsFileName << resultsFileName << fullEvalId;
      // bcast buffer length so that other procs can allocate MPIUnpackBuffer
      int buffer_len = send_buffer.size();
      parallelLib.bcast_e(buffer_len);
      // broadcast actual buffer
      parallelLib.bcast_e(send_buffer);
    }
    else {
      // receive incoming buffer length and allocate space for MPIUnpackBuffer
      int buffer_len;
      parallelLib.bcast_e(buffer_len);
      // receive incoming buffer
      MPIUnpackBuffer recv_buffer(buffer_len);
      parallelLib.bcast_e(recv_buffer);
      recv_buffer >> paramsFileName >> resultsFileName >> fullEvalId;
    }
  }
}


void ProcessApplicInterface::
write_parameters_files(const Variables& vars,    const ActiveSet& set,
		       const Response& response, const int id)
{
  PathTriple file_names(paramsFileWritten, resultsFileWritten, createdDir);

  // If a new evaluation, insert the modified file names into map for use in
  // identifying the proper file names within read_results_files for the case
  // of asynch fn evaluations.  If a replacement eval (e.g., failure capturing
  // with retry or continuation), then overwrite old file names with new ones.
  std::map<int, PathTriple>::iterator map_iter = fileNameMap.find(id);
  if (map_iter != fileNameMap.end()) {
    // remove old files.  This is only necessary for the tmp file case, since
    // all other cases (including tagged) will overwrite the old files.  Since
    // we don't want to save tmp files, don't bother to check fileSaveFlag.
    // Also don't check allow existing results since they may be bogus.
    const std::filesystem::path& pfile_path = (map_iter->second).get<0>();
    WorkdirHelper::recursive_remove(pfile_path, FILEOP_WARN);
    const std::filesystem::path& rfile_path = (map_iter->second).get<1>();
    WorkdirHelper::recursive_remove(rfile_path, FILEOP_WARN);
    // replace file names in map, avoiding 2nd lookup
    map_iter->second = file_names;
  }
  else // insert new filenames.
    fileNameMap[id] = file_names;

  /* Uncomment for filename debugging
  Cout << "\nFile name entries at fn. eval. " << id << '\n';
  for (map_iter = fileNameMap.begin(); map_iter != fileNameMap.end();map_iter++)
    Cout << map_iter->first << " " << (map_iter->second).first << " "
         << (map_iter->second).second << '\n';
  Cout << std::endl;
  */

  // Write paramsFileName without prog_num tag if there's an input filter or if
  // multiple sets of analysisComponents are not used.
  size_t num_programs = programNames.size();
  if (!multipleParamsFiles || !iFilterName.empty()) {
    std::string prog; // empty default indicates generic attribution of AC_i
    // populate prog string in cases where attribution is clear
    if (num_programs == 1 && iFilterName.empty())
      prog = programNames[0]; // this file corresponds to sole analysis driver
    else if (!iFilterName.empty() && multipleParamsFiles)
      prog = iFilterName;     // this file corresponds to ifilter
    std::vector<String> all_an_comps;
    if (!analysisComponents.empty())
      copy_data(analysisComponents, all_an_comps);
    if (!allowExistingResults)
      std::remove(resultsFileWritten.c_str());
      paramsFileWriter->write_parameters_file(vars, set, response, prog, all_an_comps, fullEvalId,
			  paramsFileWritten);
  }
  // If analysis components are used for multiple analysis drivers, then the
  // parameters filename is tagged with program number, e.g. params.in.20.2
  // provides the parameters for the 2nd analysis for the 20th fn eval.
  if (multipleParamsFiles) { // append prog counter
    for (size_t i=0; i<num_programs; ++i) {
      std::string prog_num("." + std::to_string(i+1));
      std::string tag_results_fname = resultsFileWritten + prog_num;
      std::string tag_params_fname  = paramsFileWritten  + prog_num;
      if (!allowExistingResults)
	std::remove(tag_results_fname.c_str());
      paramsFileWriter->write_parameters_file(vars, set, response, programNames[i],
			    analysisComponents[i], fullEvalId, tag_params_fname);
    }
  }
}


void ProcessApplicInterface::
read_results_files(Response& response, const int id, const String& eval_id_tag)
{
  // Retrieve parameters & results file names using fn. eval. id.  A map of
  // filenames is used because the names of tmp files must be available here
  // and asynch_recv operations can perform output filtering out of order
  // (which rules out making the file names into attributes of
  // ProcessApplicInterface or rebuilding the file name from the root name
  // plus the counter).
  std::map<int, PathTriple>::iterator map_iter = fileNameMap.find(id);
  const std::filesystem::path& params_path  = (map_iter->second).get<0>();
  const std::filesystem::path& results_path = (map_iter->second).get<1>();
  const std::filesystem::path& workdir_path = (map_iter->second).get<2>();

  // If multiple analysis programs are used, then results_filename is tagged
  // with program number (to match ProcessApplicInterface::spawn_evaluation)
  // e.g. results.out.20.2 is the 2nd analysis results from the 20th fn eval.

  // Read the results file(s).  If there's more than one program, then partial
  // responses must be overlaid to create the total response.  If an output
  // filter is used, then it has the responsibility to perform the overlay and
  // map results.out.[eval#].[1->num_programs] to results.out.[eval#].  If no
  // output filter is used, then perform the overlay here.
  size_t num_programs = programNames.size();
  if (num_programs > 1 && oFilterName.empty()) {
    response.reset();
    Response partial_response = response.copy();
    for (size_t i=0; i<num_programs; ++i) {
      const std::string prog_num("." + std::to_string(i+1));
      std::filesystem::path prog_tagged_results
	= WorkdirHelper::concat_path(results_path, prog_num);
      resultsFileReader->read_results_file(partial_response, prog_tagged_results, id);
      response.overlay(partial_response);
    }
  }
  else 
    resultsFileReader->read_results_file(response,results_path,id);

  file_and_workdir_cleanup(params_path, results_path, workdir_path, eval_id_tag);
  // Remove the evaluation which has been processed from the bookkeeping
  fileNameMap.erase(map_iter);
}


/** Move specified params and results files to unique tagged versions
    when needed */
void ProcessApplicInterface::autotag_files(const std::filesystem::path& params_path, 
					   const std::filesystem::path& results_path,
					   const String& eval_id_tag
					   //, const std::filesystem::path dest_dir
					   ) const

{
  size_t num_programs = programNames.size();

  if ((!specifiedParamsFileName.empty() || !specifiedParamsFileName.empty()) && 
      !suppressOutput && outputLevel > NORMAL_OUTPUT)
    Cout << "Files with nonunique names will be tagged for file_save:\n";

  if (!specifiedParamsFileName.empty()) {
    std::filesystem::path eval_tagged_params = 
      WorkdirHelper::concat_path(params_path, eval_id_tag);

    if (!multipleParamsFiles || !iFilterName.empty()) {
      if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	Cout << "Moving " << params_path << " to " << eval_tagged_params << '\n';
      WorkdirHelper::rename(params_path, eval_tagged_params, FILEOP_WARN);
    }
    if (multipleParamsFiles) { // append program counters to old/new strings
      for (size_t i=0; i<num_programs; ++i) {
	const std::string prog_num("." + std::to_string(i+1));
	const std::filesystem::path prog_tagged_old = 
	  WorkdirHelper::concat_path(params_path, prog_num);
	const std::filesystem::path eval_prog_tagged_new = 
	  WorkdirHelper::concat_path(eval_tagged_params, prog_num);
	if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	  Cout << "Moving " << prog_tagged_old << " to " << eval_prog_tagged_new
	       << '\n';
	WorkdirHelper::
	  rename(prog_tagged_old, eval_prog_tagged_new, FILEOP_WARN);
      }
    }
  }

  if (!specifiedResultsFileName.empty()) {
    std::filesystem::path eval_tagged_results = 
      WorkdirHelper::concat_path(results_path, eval_id_tag);

    if (num_programs == 1 || !oFilterName.empty()) {
      if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	Cout << "Moving " << results_path << " to "
	     << eval_tagged_results << '\n';
      WorkdirHelper::rename(results_path, eval_tagged_results, FILEOP_WARN);
    }
    if (num_programs > 1) { // append program counters to old/new strings
      for (size_t i=0; i<num_programs; ++i) {
	const std::string prog_num("." + std::to_string(i+1));
	const std::filesystem::path prog_tagged_old = 
	  WorkdirHelper::concat_path(results_path, prog_num);
	const std::filesystem::path eval_prog_tagged_new = 
	  WorkdirHelper::concat_path(eval_tagged_results, prog_num);
	if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	  Cout << "Moving " << prog_tagged_old << " to " << eval_prog_tagged_new
	       << '\n';
	WorkdirHelper::rename(prog_tagged_old, eval_prog_tagged_new, FILEOP_WARN);
      }
    }
  }

}

void ProcessApplicInterface::
remove_params_results_files(const std::filesystem::path& params_path, 
			    const std::filesystem::path& results_path) const
{
  size_t num_programs = programNames.size();

  if (!suppressOutput && outputLevel > NORMAL_OUTPUT) {
    Cout << "Removing " << params_path;
    if (multipleParamsFiles) {
      if (!iFilterName.empty())
	Cout << " and " << params_path;
      Cout << ".[1-" << num_programs << ']';
    }
    Cout << " and " << results_path;
    if (num_programs > 1) {
      if (!oFilterName.empty())
	Cout << " and " << results_path;
      Cout << ".[1-" << num_programs << ']';
    }
    Cout << '\n';
  }

  if (!multipleParamsFiles || !iFilterName.empty())
    WorkdirHelper::recursive_remove(params_path, FILEOP_WARN);

  if (multipleParamsFiles) {
    for (size_t i=0; i<num_programs; ++i) {
      const std::string prog_num("." + std::to_string(i+1));
      const std::filesystem::path tagged_params = 
	WorkdirHelper::concat_path(params_path, prog_num);
      WorkdirHelper::recursive_remove(tagged_params, FILEOP_WARN);
    }
  }

  if (num_programs == 1 || !oFilterName.empty())
    WorkdirHelper::recursive_remove(results_path, FILEOP_WARN);

  if (num_programs > 1)
    for (size_t i=0; i<num_programs; ++i) {
      const std::string prog_num("." + std::to_string(i+1));
      const std::filesystem::path tagged_results = 
	WorkdirHelper::concat_path(results_path, prog_num);
      WorkdirHelper::recursive_remove(tagged_results, FILEOP_WARN);
    }
}


/** Remove any files and directories still referenced in the fileNameMap */
void ProcessApplicInterface::file_cleanup() const
{
  if (fileSaveFlag && dirSave)
    return;

  std::map<int, PathTriple>::const_iterator
    file_name_map_it  = fileNameMap.begin(),
    file_name_map_end = fileNameMap.end();
  for(; file_name_map_it != file_name_map_end; ++file_name_map_it) {
    const std::filesystem::path& parfile = (file_name_map_it->second).get<0>();
    const std::filesystem::path& resfile = (file_name_map_it->second).get<1>();
    const std::filesystem::path& wd_path = (file_name_map_it->second).get<2>();
    if (!fileSaveFlag) {
      if (!multipleParamsFiles || !iFilterName.empty()) {
	WorkdirHelper::recursive_remove(parfile, FILEOP_SILENT);
	WorkdirHelper::recursive_remove(resfile, FILEOP_SILENT);
      }
      if (multipleParamsFiles) {
	size_t i, num_programs = programNames.size();
	for(i=1; i<=num_programs; ++i) {
	  std::string prog_num("." + std::to_string(i));
          std::filesystem::path pname = WorkdirHelper::concat_path(parfile, prog_num);
	  WorkdirHelper::recursive_remove(pname, FILEOP_SILENT);
          std::filesystem::path rname = WorkdirHelper::concat_path(resfile, prog_num);
	  WorkdirHelper::recursive_remove(rname, FILEOP_SILENT);
	}
      }
    }
    // a non-empty entry here indicates the directory was created for this eval
    if (!dirSave && !wd_path.empty())
      WorkdirHelper::recursive_remove(wd_path, FILEOP_SILENT);
  }
}


// get the current work directory name
std::filesystem::path ProcessApplicInterface::get_workdir_name()
{
  // PDH suggets making in rundir instead of tmp area...
  std::filesystem::path wd_name = workDirName.empty() ? 
    ( WorkdirHelper::system_tmp_path() / 
      WorkdirHelper::system_tmp_file("dakota_work") ) :
    std::filesystem::path(workDirName);

  // we allow tagging of tmp dirs in case the user's script needs the tag
  if (dirTag)
    return WorkdirHelper::concat_path(wd_name, fullEvalId);

  return wd_name;
}


/** Guidance: environment (PATH, current directory) should be set
    immediately before Dakota spawns a process and reset immediately
    afterwards (except fork which never returns) */
void ProcessApplicInterface::prepare_process_environment()
{
  // If not using workdir, just put . and startupPWD on PATH.  If
  // using workdir, also put the absolute path to the workdir on the
  // PATH (. should suffice; this is conservative).  It doesn't help
  // to prepend a relative workdir path, since we will change
  // directory into it, so the helper makes the path absolute.
  if (useWorkdir) {
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Prepending environment PATH with work_directory " 
	   << curWorkdir << "." << std::endl;
    WorkdirHelper::set_preferred_path(curWorkdir);
    if (outputLevel >= VERBOSE_OUTPUT)
      Cout << "Changing directory to " << curWorkdir << std::endl;
    WorkdirHelper::change_directory(curWorkdir);
  }
  else
    WorkdirHelper::set_preferred_path();

  WorkdirHelper::set_environment("DAKOTA_PARAMETERS_FILE", paramsFileName);
  WorkdirHelper::set_environment("DAKOTA_RESULTS_FILE", resultsFileName);
}


/** Undo anything done prior to spawn */
void ProcessApplicInterface::reset_process_environment()
{
  // BMA TODO: consider unsetting environment variables previously set

  // No need to reset in non-workdir case, as long as PATH doesn't get
  // multiply appended to
  if (useWorkdir) {
    if (outputLevel >= VERBOSE_OUTPUT)
      Cout << "Changing directory back to " << WorkdirHelper::startup_pwd()
	   << std::endl;
    if (outputLevel >= DEBUG_OUTPUT)
      Cout << "Resetting environment PATH." << std::endl;
    WorkdirHelper::reset();
  }
}

void ProcessApplicInterface::
file_and_workdir_cleanup(const std::filesystem::path &params_path,
    const std::filesystem::path &results_path,
    const std::filesystem::path &workdir_path, 
    const String &tag) const
{
  
  // remove the workdir if in the map and we're not saving
  bool removing_workdir = (!workdir_path.empty() && !dirSave);
  if (fileSaveFlag) {

    // Prevent overwriting of files with reused names for which a file_save 
    // request has been given.  Assume tmp files always unique.

    // Cases (work in progress):
    //  * no workdir --> tag if needed (old behavior)
    //  * workdir, shared, saved --> tag if needed
    //  * a workdir can be unique via dir_tag or tmp files
    //    - workdir, unique, saved --> done no tag
    //    - workdir, not saved --> tag and move to rundir
    //  * when workdir and absolute path to files tag in abs path

    if ( useWorkdir ) {
      if ( dirSave ) {
	// if saving the directory, just need to make sure filenames are unique
	// use legacy tagging mechanism within the workdir
	// TODO: don't need to tag if workdir is unique per eval...
	if (!fileTagFlag && !dirTag && !workDirName.empty())
	  autotag_files(params_path, results_path, tag);
      }
      else {
	// work_directory getting removed; unique tag the files into the rundir
	// take the filename off the path and use with rundir
	// TODO: do we even need to support this?
	// TODO: distinguish between params in vs. not in (absolute
	// path) the workdir
	// autotag_files(params_path, results_path, eval_id_tag,
	// 	      std::filesystem::current_path());
      }
    }
    else {
      // simple case; no workdir --> old behavior, tagging if needed
      // in place (whether relative or absolute)
      if (!fileTagFlag)
	autotag_files(params_path, results_path, tag);
    }
  }
  else
    remove_params_results_files(params_path, results_path);

  // Now that files are handled, conditionally remove the work directory
  if (removing_workdir) {
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "Removing work_directory " << workdir_path << std::endl;
    WorkdirHelper::recursive_remove(workdir_path, FILEOP_ERROR);
  }

}

} // namespace Dakota
