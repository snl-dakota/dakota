/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ProcessApplicInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DakotaResponse.hpp"
#include "ParamResponsePair.hpp"
#include "ProcessApplicInterface.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "WorkdirHelper.hpp"
#include "dakota_filesystem_utils.hpp"
#include <algorithm>
#include <boost/filesystem/fstream.hpp>

/** 
    Work directory TODO
    
    * Remove legacy utilities (once concepts migrated)

    * Update search paths for driver check to work with lists of wildcards
 
    * Remove directories during file_cleanup()

    - In general review cases with race conditions such as single dir
      getting created / removed for each eval.

    - Verify creation/removal in parallel runs (eval_comm_rank 0?);
      are there scenarios where we should create once ahead of time?

    - Enforce tagging when asynch is possible

    - Challenge of shared vs. distinct filesystems

    - Verify template files exist at parse and that workdir parent exists

    - Verify behavior when directory exitsts
  
    - Allow recursive copy to descend to overwrite leaf nodes when
      directories already exist

    - Old code setting permissions mask 0700

    - Workdir with multiple analysis components (per analysis)

    - Workdirs shared for each unique concurrent eval (not per eval ID)

    - Evaluate environment variables

    - How to manage drivers that want just param names in the work
      dir?!?  I think arg_adjust is removing the directory args that
      aren't needed.

    - Consider making the class members for directories and files bfs::paths

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
      * Allow nested quotes in driver, at least one level: 
          analysis_driver = 'ad.sh "-opt foo -opt1 goo"' p.in.1 r.out.1

      * Env vars will be carried along for now, not expanded before
        eval; set some helpful env vars before the eval.

      * Allowed FOO=zorch and would set that in the environment; could
        allow separate env var specification; otherwise likely remove

    - TODO: pass environment to exec as separate pointer
*/


namespace Dakota {

ProcessApplicInterface::
ProcessApplicInterface(const ProblemDescDB& problem_db):
  ApplicationInterface(problem_db), 
  fileTagFlag(problem_db.get_bool("interface.application.file_tag")),
  fileSaveFlag(problem_db.get_bool("interface.application.file_save")),
  commandLineArgs(!problem_db.get_bool("interface.application.verbatim")),
  apreproFlag(problem_db.get_bool("interface.application.aprepro")),
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
  templateReplace(problem_db.get_bool("interface.templateReplace")),
  analysisComponents(
    problem_db.get_s2a("interface.application.analysis_components"))
{
  size_t num_programs = programNames.size();
  if (num_programs > 0 && !programNames[0].empty()) {
    // Ignore anything beyond the first whitespace in programNames[0]
    std::vector<std::string> driver_and_args;

    boost::split( driver_and_args, programNames[0], boost::is_any_of("\t ") );

    std::string driver_path = WorkdirHelper::which(driver_and_args[0]);
    if ( driver_path.empty() ) {
      Cout << "\nWarning (analysis_driver): '" << driver_and_args[0] 
	   << "': command not found.\n" << std::endl;
      // we decided not to make this a fatal error as it's too fragile
      //abort_handler(INTERFACE_ERROR);
    }
    else if (driver_path.substr(0, 4) == DAK_PATH_ENV_NAME) {
      // Allow legacy logic to proceed normally
#ifdef DEBUG
      Cout << driver_and_args[0] << " - FOUND in:\n" << driver_path <<std::endl;
#endif
    }
    /* Uncomment for filename debugging
    else
      Cout << driver_path << std::endl;
    // End filename debugging */
  }

  if (num_programs > 1 && !analysisComponents.empty())
    multipleParamsFiles = true;

  // BMA: This is not quite correct as final
  // asynchLocalEvalConcurrency isn't known until set_communicators.
  // Might need to manage this there.

  // TODO: change synchronization to enum

  // For asynch local we require tagged directories (if active) or files
  // bool require_unique = (interfaceSynchronization == "asynchronous") && 
  //   asynchLocalEvalConcSpec != 1;

  // For MPI parallelism we allow non-tagged dirs / files since
  // nodes may not share a common filesystem (could consider enforcing
  // tagged even in this case), but can't check until comms are set...
  //  bool suggest_unique = (if MPI concurrency)

  /*
  if (require_unique) {
    if (useWorkdir && !dirTag && !workDirName.empty()) {
      Cout << "\nWarning: Concurrent local evaluations with named " 
	   << "work_directory require"
	   << "\n         directory_tag; enabling tagging." << std::endl;
      dirTag = true;
    }
    else if (!fileTagFlag && 
	     (!specifiedParamsFileName.empty() || 
	      !specifiedResultsFileName.empty()) ) {
      Cout << "\nWarning: Concurrent local evaluations with named " 
	   << "parameters_file or results_file"
	   << "\n require file_tag; enabling tagging." << std::endl;
      fileTag = true;
    }
  }
  */

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

  catch(std::string& err_msg) {
    // a std::string exception involves detection of an incomplete file/data
    // set.  In the synchronous case, there is no potential for an incomplete 
    // file resulting from a race condition -> echo the error and abort.
    Cerr << err_msg << std::endl;
    abort_handler(INTERFACE_ERROR);
  }

  // Do not call manage_failure() from the following catch since the recursion 
  // of calling derived_map again would be confusing at best.  The approach 
  // here is to have catch(int) rethrow the exception to an outer catch (either
  // the catch within manage_failure or a catch that calls manage_failure).  An
  // alternative solution would be to eliminate the try block above and move 
  // all catches to the higher level of try { derived_map() } - this would be
  // simpler to understand but would replicate catch(std::string) in map, 
  // manage_failure, and serve.  By having catch(std::string) here and having
  // catch(int) rethrow, we eliminate unnecessary proliferation of 
  // catch(std::string).
  catch(int fail_code) { // failure capture exception thrown by response.read()
    //Cout << "Rethrowing int." << std::endl;
    throw; // from this catch to the outer one in manage_failure
  }
}


void ProcessApplicInterface::derived_map_asynch(const ParamResponsePair& pair)
{
  // This function may not be executed by a multiprocessor evalComm.

  int fn_eval_id = pair.eval_id();
  define_filenames(final_eval_id_tag(fn_eval_id)); // all evalComm
  write_parameters_files(pair.prp_parameters(), pair.active_set(),
			 pair.prp_response(),   fn_eval_id);
 
  // execute the simulator application -- nonblocking call
  pid_t pid = create_evaluation_process(FALL_THROUGH);

  // store process & eval ids for use in synchronization
  map_bookkeeping(pid, fn_eval_id);
}


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
      curWorkdir = get_workdir_name();
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
    
    bfs::path params_path(specifiedParamsFileName);

    // no user spec -> use temp files, possibly in workdir; generate relative
    if (specifiedParamsFileName.empty())
      params_path = bfs::unique_path("dakota_params_%%%%%%%%");

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
	paramsFileName = (bfs::temp_directory_path() / params_path).string();
	paramsFileWritten = paramsFileName;
      }
      else
	paramsFileName = params_path.string();
    }
    else
      paramsFileName = params_path.string();
    

    // define resultsFileName for use in write_parameters_files

    bfs::path results_path(specifiedResultsFileName);

    // no user spec -> use temp files, possibly in workdir; generate relative
    if (specifiedResultsFileName.empty())
      results_path = bfs::unique_path("dakota_results_%%%%%%%%");

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
	resultsFileName = (bfs::temp_directory_path() / results_path).string();
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
    bfs::remove((map_iter->second).get<0>());  // parameters file name
    bfs::remove((map_iter->second).get<1>());  // results file name
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
    write_parameters_file(vars, set, response, prog, all_an_comps,
			  paramsFileWritten);
  }
  // If analysis components are used for multiple analysis drivers, then the
  // parameters filename is tagged with program number, e.g. params.in.20.2
  // provides the parameters for the 2nd analysis for the 20th fn eval.
  if (multipleParamsFiles) { // append prog counter
    for (size_t i=0; i<num_programs; ++i) {
      std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
      std::string tag_results_fname = resultsFileWritten + prog_num;
      std::string tag_params_fname  = paramsFileWritten  + prog_num;
      if (!allowExistingResults)
	std::remove(tag_results_fname.c_str());
      write_parameters_file(vars, set, response, programNames[i],
			    analysisComponents[i], tag_params_fname);
    }
  }
}


void ProcessApplicInterface::
write_parameters_file(const Variables& vars, const ActiveSet& set,
		      const Response& response, const std::string& prog,
		      const std::vector<String>& an_comps,
                      const std::string& params_fname)
{
  // Write the parameters file
  std::ofstream parameter_stream(params_fname.c_str());
  using std::setw;
  if (!parameter_stream) {
    Cerr << "\nError: cannot create parameters file " << params_fname
         << std::endl;
    abort_handler(IO_ERROR);
  }
  StringMultiArrayConstView acv_labels  = vars.all_continuous_variable_labels();
  SizetMultiArrayConstView  acv_ids     = vars.all_continuous_variable_ids();
  const ShortArray&         asv         = set.request_vector();
  const SizetArray&         dvv         = set.derivative_vector();
  const StringArray&        resp_labels = response.function_labels();
  size_t i, asv_len = asv.size(), dvv_len = dvv.size(),
    ac_len = an_comps.size();
  StringArray asv_labels(asv_len), dvv_labels(dvv_len), ac_labels(ac_len);
  build_labels(asv_labels, "ASV_");
  build_labels(dvv_labels, "DVV_");
  build_labels(ac_labels,  "AC_");
  for (i=0; i<asv_len; ++i)
    asv_labels[i] += ":" + resp_labels[i];
  for (i=0; i<dvv_len; ++i) {
    size_t acv_index = find_index(acv_ids, dvv[i]);
    if (acv_index != _NPOS)
      dvv_labels[i] += ":" + acv_labels[acv_index];
  }
  if (!prog.empty()) // empty string passed if multiple attributions possible
    for (i=0; i<ac_len; ++i)
      ac_labels[i] += ":" + prog; // attribution to particular program
  int prec = write_precision; // for restoration
  //write_precision = 16; // 17 total digits: full double precision
  write_precision = 15;   // 16 total digits: preserves desirable roundoff in
                          //                  last digit
  int w = write_precision+7;
  if (apreproFlag) {
    std::string sp20("                    ");
    parameter_stream << sp20 << "{ DAKOTA_VARS     = " << setw(w) << vars.tv()
		     << " }\n";
    vars.write_aprepro(parameter_stream);
    parameter_stream << sp20 << "{ DAKOTA_FNS      = " << setw(w) << asv_len
		     << " }\n"; //<< setiosflags(ios::left);
    array_write_aprepro(parameter_stream, asv, asv_labels);
    parameter_stream << sp20 << "{ DAKOTA_DER_VARS = " << setw(w) << dvv_len
		     << " }\n";
    array_write_aprepro(parameter_stream, dvv, dvv_labels);
    parameter_stream << sp20 << "{ DAKOTA_AN_COMPS = " << setw(w) << ac_len
		     << " }\n";
    array_write_aprepro(parameter_stream, an_comps, ac_labels);
    // write full eval ID tag, without leading period, converting . to :
    String full_eval_id(fullEvalId);
    full_eval_id.erase(0,1); 
    boost::algorithm::replace_all(full_eval_id, String("."), String(":"));
    parameter_stream << sp20 << "{ DAKOTA_EVAL_ID  = " << setw(w) 
		     << full_eval_id << " }\n";
    //parameter_stream << resetiosflags(ios::adjustfield);
  }
  else {
    std::string sp21("                     ");
    parameter_stream << sp21 << setw(w) << vars.tv() << " variables\n" << vars
		     << sp21 << setw(w) << asv_len   << " functions\n";
		   //<< setiosflags(ios::left);
    array_write(parameter_stream, asv, asv_labels);
    parameter_stream << sp21 << setw(w) << dvv_len << " derivative_variables\n";
    array_write(parameter_stream, dvv, dvv_labels);
    parameter_stream << sp21 << setw(w) << ac_len  << " analysis_components\n";
    array_write(parameter_stream, an_comps, ac_labels);
    // write full eval ID tag, without leading period
    String full_eval_id(fullEvalId);
    full_eval_id.erase(0,1); 
    boost::algorithm::replace_all(full_eval_id, String("."), String(":"));
    parameter_stream << sp21 << setw(w) << full_eval_id << " eval_id\n";
    //parameter_stream << resetiosflags(ios::adjustfield);
  }
  write_precision = prec; // restore

  // Explicit flush and close added 3/96 to prevent Solaris problem of input
  // filter reading file before the write was completed.
  parameter_stream.flush();
  parameter_stream.close();
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
  const bfs::path& params_path  = (map_iter->second).get<0>();
  const bfs::path& results_path = (map_iter->second).get<1>();
  const bfs::path& workdir_path = (map_iter->second).get<2>();

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
      const std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
      bfs::path prog_tagged_results
	= WorkdirHelper::concat_path(results_path, prog_num);
      bfs::ifstream recovery_stream(prog_tagged_results);
      if (!recovery_stream) {
	Cerr << "\nError: cannot open results file " << prog_tagged_results
	     << std::endl;
	abort_handler(INTERFACE_ERROR); // will clean up files unless file_save was specified
      }
      recovery_stream >> partial_response;
      response.overlay(partial_response);
    }
  }
  else {
    bfs::ifstream recovery_stream(results_path);
    if (!recovery_stream) {
      Cerr << "\nError: cannot open results file " << results_path
	   << std::endl;
      abort_handler(INTERFACE_ERROR); // will clean up files unless file_save was specified
    }
    recovery_stream >> response;
  }

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
	  autotag_files(params_path, results_path, eval_id_tag);
      }
      else {
	// work_directory getting removed; unique tag the files into the rundir
	// take the filename off the path and use with rundir
	// TODO: do we even need to support this?
	// TODO: distinguish between params in vs. not in (absolute
	// path) the workdir
	// autotag_files(params_path, results_path, eval_id_tag,
	// 	      bfs::current_path());
	;
      }
    }
    else {
      // simple case; no workdir --> old behavior, tagging if needed
      // in place (whether relative or absolute)
      if (!fileTagFlag) {
	autotag_files(params_path, results_path, eval_id_tag);
      }
    }
  }
  else
    remove_params_results_files(params_path, results_path);
 
  // Now that files are handled, conditionally remove the work directory
  if (removing_workdir) {
    if (outputLevel > NORMAL_OUTPUT)
      Cout << "Removing work_directory " << workdir_path << std::endl;
    WorkdirHelper::recursive_remove(workdir_path);
  }
  // Remove the evaluation which has been processed from the bookkeeping
  fileNameMap.erase(map_iter);
}


/** Move specified params and results files to unique tagged versions
    when needed */
void ProcessApplicInterface::autotag_files(const bfs::path& params_path, 
					   const bfs::path& results_path,
					   const String& eval_id_tag
					   //, const bfs::path dest_dir
					   ) const

{
  size_t num_programs = programNames.size();

  if ((!specifiedParamsFileName.empty() || !specifiedParamsFileName.empty()) && 
      !suppressOutput && outputLevel > NORMAL_OUTPUT)
    Cout << "Files with nonunique names will be tagged for file_save:\n";

  if (!specifiedParamsFileName.empty()) {
    bfs::path eval_tagged_params = 
      WorkdirHelper::concat_path(params_path, eval_id_tag);

    if (!multipleParamsFiles || !iFilterName.empty()) {
      if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	Cout << "Moving " << params_path << " to " << eval_tagged_params << '\n';
      bfs::rename(params_path, eval_tagged_params);
    }
    if (multipleParamsFiles) { // append program counters to old/new strings
      for (size_t i=0; i<num_programs; ++i) {
	const std::string prog_num("."+boost::lexical_cast<std::string>(i+1));
	const bfs::path prog_tagged_old = 
	  WorkdirHelper::concat_path(params_path, prog_num);
	const bfs::path eval_prog_tagged_new = 
	  WorkdirHelper::concat_path(eval_tagged_params, prog_num);
	if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	  Cout << "Moving " << prog_tagged_old << " to " << eval_prog_tagged_new
	       << '\n';
	bfs::rename(prog_tagged_old, eval_prog_tagged_new);
      }
    }
  }

  if (!specifiedResultsFileName.empty()) {
    bfs::path eval_tagged_results = 
      WorkdirHelper::concat_path(results_path, eval_id_tag);

    if (num_programs == 1 || !oFilterName.empty()) {
      if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	Cout << "Moving " << results_path << " to "
	     << eval_tagged_results << '\n';
      bfs::rename(results_path, eval_tagged_results);
    }
    if (num_programs > 1) { // append program counters to old/new strings
      for (size_t i=0; i<num_programs; ++i) {
	const std::string prog_num("."+boost::lexical_cast<std::string>(i+1));
	const bfs::path prog_tagged_old = 
	  WorkdirHelper::concat_path(results_path, prog_num);
	const bfs::path eval_prog_tagged_new = 
	  WorkdirHelper::concat_path(eval_tagged_results, prog_num);
	if (!suppressOutput && outputLevel > NORMAL_OUTPUT)
	  Cout << "Moving " << prog_tagged_old << " to " << eval_prog_tagged_new
	       << '\n';
	bfs::rename(prog_tagged_old, eval_prog_tagged_new);
      }
    }
  }

}

void ProcessApplicInterface::
remove_params_results_files(const bfs::path& params_path, 
			    const bfs::path& results_path) const
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
    bfs::remove(params_path);

  if (multipleParamsFiles) {
    for (size_t i=0; i<num_programs; ++i) {
      const std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
      const bfs::path tagged_params = 
	WorkdirHelper::concat_path(params_path, prog_num);
      bfs::remove(tagged_params);
    }
  }

  if (num_programs == 1 || !oFilterName.empty())
    bfs::remove(results_path);

  if (num_programs > 1)
    for (size_t i=0; i<num_programs; ++i) {
      const std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
      const bfs::path tagged_results = 
	WorkdirHelper::concat_path(results_path, prog_num);
      bfs::remove(tagged_results);
    }
}


// TODO: remove directories as well
/** Remove any files still referenced in the fileNameMap */
void ProcessApplicInterface::file_cleanup() const
{
  if (fileSaveFlag)
    return;

  std::map<int, PathTriple>::const_iterator
    file_name_map_it  = fileNameMap.begin(),
    file_name_map_end = fileNameMap.end();
  for(; file_name_map_it != file_name_map_end; ++file_name_map_it) {
    const bfs::path& parfile = (file_name_map_it->second).get<0>();
    const bfs::path& resfile = (file_name_map_it->second).get<1>();
    if (!multipleParamsFiles || !iFilterName.empty()) {
      bfs::remove(parfile);
      bfs::remove(resfile);
    }
    if (multipleParamsFiles) {
      size_t i, num_programs = programNames.size();
      for(i=1; i<=num_programs; ++i) {
        std::string prog_num("." + boost::lexical_cast<std::string>(i));
	bfs::path pname = WorkdirHelper::concat_path(parfile, prog_num);
	bfs::remove(pname);
	bfs::path rname = WorkdirHelper::concat_path(resfile, prog_num);
	bfs::remove(rname);
      }
    }
  }
}


// get the current work directory name
bfs::path ProcessApplicInterface::get_workdir_name()
{
  // PDH suggets making in rundir instead of tmp area...
  bfs::path wd_name = workDirName.empty() ? 
    WorkdirHelper::system_tmp_name("dakota_work") :
    workDirName;

  // we allow tagging of tmp dirs in case the user's script needs the tag
  if (dirTag)
    return WorkdirHelper::concat_path(wd_name, fullEvalId);

  return wd_name;
}


void ProcessApplicInterface::prepare_process_environment()
{
  if (useWorkdir)
    WorkdirHelper::change_directory(curWorkdir);

  // setenv()/putenv() of DAKOTA_PARAMETERS_FILE/DAKOTA_RESULTS_FILE
  WorkdirHelper::set_environment("DAKOTA_PARAMETERS_FILE", paramsFileName);
  WorkdirHelper::set_environment("DAKOTA_RESULTS_FILE", resultsFileName);

  // prepend env path with run dir (already on path) and startup dir
  if (useWorkdir && curWorkdir.is_absolute())
    WorkdirHelper::prepend_path_item(curWorkdir, bfs::path(), false);
}


} // namespace Dakota
