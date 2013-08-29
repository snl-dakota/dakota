/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        AnalysisCode
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "AnalysisCode.hpp"
#include "DakotaVariables.hpp"
#include "DakotaResponse.hpp"
#include "ProblemDescDB.hpp"
#include "ParallelLibrary.hpp"
#include "WorkdirHelper.hpp"
#include "dakota_filesystem_utils.hpp"
#include <boost/lexical_cast.hpp>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

static const char rcsId[]="@(#) $Id: AnalysisCode.cpp 7024 2010-10-16 01:24:42Z mseldre $";


namespace Dakota {

AnalysisCode::AnalysisCode(const ProblemDescDB& problem_db):
  suppressOutputFlag(false),
  // See base constructor in DakotaIterator.cpp for full discussion of output
  // verbosity.  Quiet is the default for AnalysisCode file operations.
  outputLevel(problem_db.get_short("method.output")),
  fileTagFlag(problem_db.get_bool("interface.application.file_tag")),
  fileSaveFlag(problem_db.get_bool("interface.application.file_save")),
  commandLineArgs(!problem_db.get_bool("interface.application.verbatim")),
  apreproFlag(problem_db.get_bool("interface.application.aprepro")),
  multipleParamsFiles(false),
  iFilterName(problem_db.get_string("interface.application.input_filter")),
  oFilterName(problem_db.get_string("interface.application.output_filter")),
  programNames(problem_db.get_sa("interface.application.analysis_drivers")),
  numPrograms(programNames.size()),
  specifiedParamsFileName(
    problem_db.get_string("interface.application.parameters_file")),
  specifiedResultsFileName(
    problem_db.get_string("interface.application.results_file")),
  allowExistingResults(problem_db.get_bool("interface.allow_existing_results")),
  analysisComponents(
    problem_db.get_s2a("interface.application.analysis_components")),
  parallelLib(problem_db.parallel_library()),
  useWorkdir(problem_db.get_bool("interface.useWorkdir")),
  workDir(problem_db.get_string("interface.workDir")),
  dirTag(problem_db.get_bool("interface.dirTag")),
  dirSave(problem_db.get_bool("interface.dirSave")), dirDel(false),
  templateDir(problem_db.get_string("interface.templateDir")),
  templateFiles(problem_db.get_sa("interface.templateFiles")),
  templateCopy(problem_db.get_bool("interface.templateCopy")),
  templateReplace(problem_db.get_bool("interface.templateReplace")),
  haveTemplateDir(false), haveWorkdir(false)
{
  if (numPrograms > 0 && !programNames[0].empty()) {
    // Ignore anything beyond the first whitespace in programNames[0]
    std::vector<std::string> driver_and_args;

    boost::split( driver_and_args, programNames[0], boost::is_any_of("\t ") );

    std::string driver_path = WorkdirHelper::which(driver_and_args[0]);
    if ( driver_path.empty() ) {
      Cout << driver_and_args[0] << ": Command not found.\n" << std::endl;
      abort_handler(-1);
    }
    else if (driver_path.substr(0, 4) == DAK_PATH_ENV_NAME) {
      ; // Allow legacy logic to proceed normally
#ifdef DEBUG
      Cout << driver_and_args[0] << " - FOUND in:\n" << driver_path <<std::endl;
#endif
    }
    /* Uncomment for filename debugging
    else
      Cout << driver_path << std::endl;
    // End filename debugging */
  }

  if (numPrograms > 1 && !analysisComponents.empty())
    multipleParamsFiles = true;
}


void AnalysisCode::define_filenames(const int id, const String& eval_tag_prefix)
{
  // Define modified file names by handling Unix temp file and tagging options.
  // These names are used in writing the parameters file, in defining the
  // Command Shell overloaded operator behavior in SysCallAnalysisCode.cpp, in
  // reading the results file, and in file cleanup.

  // Different analysis servers _must_ share the same parameters and results
  // file root names.  If temporary file names are not used, then each evalComm
  // proc can define the same names without need for interproc communication.
  // However, if temporary file names are used, then evalCommRank 0 must define
  // the names and broadcast them to other evalComm procs.

  // BMA NOTE: the file and/or directory names might also be adjusted
  // by workdir, so may need to broadcast in that case too!

  // BMA TODO: Current behavior is tagged parameters OR tagged
  // directory.  The input spec should enforce this!  Or support what
  // the spec expresses...

  // BMA NOTE: We allow run without tag, but then tag at cleanup when
  // saving

  // Classic behavior with no hierarchical tagging:
  //   directory_tag: tag dirs .2  no tag files
  //   both:          tag dirs .2  no tag files
  //   file_tag:      no tag dirs  tag files .2

  // Behavior for hierarchical tagging with eval_tag_prefix:
  //   directory_tag: tag dirs .4.9.2  no tag files
  //   both:          tag dirs .4.9    tag files .2
  //   file_tag:      no tag dirs      tag files .4.9.2

  const ParallelConfiguration& pc = parallelLib.parallel_configuration();
  int eval_comm_rank   = parallelLib.ie_parallel_level_defined()
	? pc.ie_parallel_level().server_communicator_rank() : 0;
  int analysis_servers = parallelLib.ea_parallel_level_defined()
	? pc.ea_parallel_level().num_servers() : 1;
  bool bcast_flag = ( analysis_servers > 1 && ( specifiedParamsFileName.empty()
		   || specifiedResultsFileName.empty() ) ) ? true : false;
  int k;
  size_t i, n;

  if (eval_comm_rank == 0 || !bcast_flag) {

    // always define fullEvalId string for use in parameters files
    String eval_id_tag = "." + boost::lexical_cast<std::string>(id);
    fullEvalId = eval_tag_prefix + eval_id_tag;

    // process options for classic or hierarchical tagging
    String file_suffix, dir_suffix;  // endings for file and directory names
    bool file_tag = false;
    if (eval_tag_prefix.empty()) {
      // classic behavior
      if (dirTag) {
	dir_suffix = eval_id_tag;
      }
      else if (fileTagFlag) {
	file_tag = true;
	file_suffix = eval_id_tag;
      }
    }
    else {
      // hierarchical tagging
      if (dirTag && fileTagFlag) {
	dir_suffix = eval_tag_prefix;
	file_tag = true;
	file_suffix = eval_id_tag;
      }
      else if (dirTag) {
	dir_suffix = fullEvalId;
      }
      else if (fileTagFlag) {
	file_tag = true;
	file_suffix = fullEvalId;
      }
    }

    if (specifiedParamsFileName.empty()) { // no user spec -> use temp files
//#ifdef LINUX
      /*
      // mkstemp generates a unique filename using tmp_str _and_ creates the
      // file.  This prevents potential issues with race conditions, but has
      // the undesirable feature of requiring additional file deletions in the
      // case of secondary tagging for multiple analysis drivers.
      char tmp_str[20] = "/tmp/dakvars_XXXXXX";
      mkstemp(tmp_str); // replaces XXXXXX with unique id
      paramsFileName = tmp_str;
      */
//#else
      // tmpnam generates a unique filename that does not exist at the time of
      // invocation, but does not create the file.  It is more dangerous than
      // mkstemp, since multiple applications could potentially generate the
      // same temp filename prior to creating the file.
      paramsFileName = std::tmpnam(NULL);

#if defined(_MSC_VER) && !(BOOST_FILESYSTEM_VERSION < 3)
      // Add the temporary directory in front. Windows always puts a backslash
      // in front of the filename, which would place the file in the current
      // working directory.
      paramsFileName = (bfs::temp_directory_path()
                        /= paramsFileName.substr(1)).string();
#endif
//#endif
    }
    else {
      paramsFileName = specifiedParamsFileName;
      if (file_tag)
	paramsFileName += file_suffix;
    }
    if (specifiedResultsFileName.empty()) { // no user spec -> use temp files
//#ifdef LINUX
      /*
      // mkstemp generates a unique filename using tmp_str _and_ creates the
      // file.  This prevents potential issues with race conditions, but has
      // the undesirable feature of requiring additional file deletions in the
      // case of secondary tagging for multiple analysis drivers.
      char tmp_str[20] = "/tmp/dakresp_XXXXXX";
      mkstemp(tmp_str); // replaces XXXXXX with unique id
      resultsFileName = tmp_str;
      */
//#else
      // tmpnam generates a unique filename that does not exist at the time of
      // invocation, but does not create the file.  It is more dangerous than
      // mkstemp, since multiple applications could potentially generate the
      // same temp filename prior to creating the file.
      resultsFileName = std::tmpnam(NULL);

#if defined(_MSC_VER) && !(BOOST_FILESYSTEM_VERSION < 3)
      // Add the temporary directory in front. Windows always puts a backslash
      // in front of the filename, which would place the file in the current
      // working directory.
      resultsFileName = (bfs::temp_directory_path()
                         /= resultsFileName.substr(1)).string();
#endif
//#endif
    }
    else {
      resultsFileName = specifiedResultsFileName;
      if (file_tag)
	resultsFileName += file_suffix;
    }
    if (useWorkdir) {
	if (!workDir.length()) {
          workDir = std::tmpnam(NULL);

#if defined(_MSC_VER) && !(BOOST_FILESYSTEM_VERSION < 3)
          // Add the temp directory in front. Windows always puts a backslash
          // in front of the filename, which would place the file in the current
          // working directory.
          workDir = (bfs::temp_directory_path() /= workDir.substr(1)).string();
#endif
          dirDel = true;
          dirSave = false;
	}

	std::string wd = workDir;
	struct stat sb;
	if (dirTag) {
	  if (!haveWorkdir) {
	    //if (stat(wd.c_str(), &sb)) {  // WJB: slight chance some team
            //                                      members prefer "extra" dir
	    if (stat(".", &sb)) {
                        /*
			if (DAK_MKDIR(wd.c_str(),0700)) {
			  Cerr << "\nError: cannot create directory \""
			       << wd << "\"." << std::endl;
			  abort_handler(-1);
			} */
			if (!dirSave && !fileSaveFlag)
			  dirDel = true;
            }
            haveWorkdir = true;
	  }
          wd += dir_suffix;
	}

	if (!haveTemplateDir) {
		if ((k = stat(wd.c_str(), &sb)) || dirTag) {
			if (!k && templateReplace) {
				rec_rmdir(wd.c_str());
				k = 1;
			}
			if (k) {
				if (DAK_MKDIR(wd.c_str(),0700)) {
			          Cerr << "\nError: cannot create directory \""
			               << wd << "\"." << std::endl;
			          abort_handler(-1);
			        }
				if (!dirSave && !dirTag)
					dirDel = true;
			}
		}
#if !defined(S_ISDIR)
# define S_ISDIR(mode) ((mode) & _S_IFDIR)
#endif
		else if (!S_ISDIR(sb.st_mode)) {
			Cerr << "\nError: \"" << wd << "\" exists but is not a directory."
				<< std::endl;
			abort_handler(-1);
		}
		else if ((sb.st_mode & 0700) != 0700) {
			Cerr << "\nError: directory \"" << wd
				<< "\" exists but has wrong permissions." << std::endl;
			abort_handler(-1);
		}
		if (!dirTag)
			haveTemplateDir = true;
		if (templateDir.length()) {
			if ((k = rec_cp(templateDir.c_str(), wd.c_str(), templateCopy, 1, templateReplace))) {
				Cerr << "Error in rec_cp(\""
					<< templateDir << "\", \""
					<< wd << "\")." << std::endl;
				abort_handler(-1);
			}
		}
		else for(n = templateFiles.size(), i = 0; i < n; ++i) {
		        if ((k = rec_cp(templateFiles[i].c_str(), wd.c_str(), templateCopy, 0, templateReplace))) {
				Cerr << "Error in rec_cp(\""
					<< templateFiles[i] << "\", \""
					<< wd << "\")." << std::endl;
				abort_handler(-1);
			}
		}
	}

	curWorkdir = wd;

	boost::filesystem::path wd_path(wd);
	boost::filesystem::path pfile_path(paramsFileName);
	boost::filesystem::path rfile_path(resultsFileName);
	// want the platform-dependent (native) string format here...
#if BOOST_FILESYSTEM_VERSION < 3
	paramsFileName  = (wd_path / pfile_path.filename()).file_string();
	resultsFileName  = (wd_path / rfile_path.filename()).file_string();
#else
	// consider native_string? Table in BFS V3 doc is confusing or wrong.
	paramsFileName  = (wd_path / pfile_path.filename()).string();
	resultsFileName  = (wd_path / rfile_path.filename()).string();
#endif

	if (outputLevel >= DEBUG_OUTPUT)
	  Cout << "\nAdjusting parameters_file to " << paramsFileName 
	       << " due to work_directory usage.\nAdjusting results_file to "
	       << resultsFileName << " due to work_directory usage." 
	       << std::endl;
    }
  }

  if (bcast_flag) {
    // Note that evalComm and evalAnalysisIntraComm are equivalent in this case
    // since multiprocessor analyses are forbidden when using system calls/forks
    // (enforced by ApplicationInterface::init_communicators()).
    if (eval_comm_rank == 0) {
      // pack the buffer with root file names for the evaluation
      MPIPackBuffer send_buffer;
      send_buffer << paramsFileName << resultsFileName << fullEvalId;
      // bcast buffer length so that other procs can allocate MPIUnpackBuffer
      //int buffer_len = send_buffer.len();
      //parallelLib.bcast_e(buffer_len);
      // broadcast actual buffer
      parallelLib.bcast_e(send_buffer);
    }
    else {
      // receive incoming buffer length and allocate space for MPIUnpackBuffer
      //int buffer_len;
      //parallelLib.bcast_e(buffer_len);
      // receive incoming buffer
      //MPIUnpackBuffer recv_buffer(buffer_len);
      MPIUnpackBuffer recv_buffer(512); // 512 should be plenty for 2 filenames
      parallelLib.bcast_e(recv_buffer);
      recv_buffer >> paramsFileName >> resultsFileName >> fullEvalId;
    }
  }
}

/// output error message about results file and call abort
static void
cleanup_and_abort(const std::string &resname)
{
  Cerr << "\nError: cannot open results file " << resname << std::endl;
  abort_handler(-1);	// will clean up files unless file_save was specified
}


void AnalysisCode::
write_parameters_files(const Variables& vars,    const ActiveSet& set,
		       const Response& response, const int id)
{
  std::pair<std::string,std::string> file_names(paramsFileName,resultsFileName);

  // If a new evaluation, insert the modified file names into map for use in
  // identifying the proper file names within read_results_files for the case
  // of asynch fn evaluations.  If a replacement eval (e.g., failure capturing
  // with retry or continuation), then overwrite old file names with new ones.
  std::map<int, std::pair<std::string, std::string> >::iterator map_iter
    = fileNameMap.find(id);
  if (map_iter != fileNameMap.end()) {
    // remove old files.  This is only necessary for the tmp file case, since
    // all other cases (including tagged) will overwrite the old files.  Since
    // we don't want to save tmp files, don't bother to check fileSaveFlag.
    // Also don't check allow existing results since they may be bogus.
    std::remove((map_iter->second).first.c_str());  // parameters file name
    std::remove((map_iter->second).second.c_str()); // results file name
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
  if (!multipleParamsFiles || !iFilterName.empty()) {
    std::string prog; // empty default indicates generic attribution of AC_i
    // populate prog string in cases where attribution is clear
    if (numPrograms == 1 && iFilterName.empty())
      prog = programNames[0]; // this file corresponds to sole analysis driver
    else if (!iFilterName.empty() && multipleParamsFiles)
      prog = iFilterName;     // this file corresponds to ifilter
    std::vector<String> all_an_comps;
    if (!analysisComponents.empty())
      copy_data(analysisComponents, all_an_comps);
    if (!allowExistingResults)
      std::remove(resultsFileName.c_str());
    write_parameters_file(vars, set, response, prog, all_an_comps,
			  paramsFileName);
  }
  // If analysis components are used for multiple analysis drivers, then the
  // parameters filename is tagged with program number, e.g. params.in.20.2
  // provides the parameters for the 2nd analysis for the 20th fn eval.
  if (multipleParamsFiles) { // append prog counter
    for (size_t i=0; i<numPrograms; ++i) {
      std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
      std::string tag_results_fname = resultsFileName + prog_num;
      std::string tag_params_fname  = paramsFileName  + prog_num;
      if (!allowExistingResults)
	std::remove(tag_results_fname.c_str());
      write_parameters_file(vars, set, response, programNames[i],
			    analysisComponents[i], tag_params_fname);
    }
  }
}


void AnalysisCode::
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
    abort_handler(-1);
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


void AnalysisCode::read_results_files(Response& response, const int id,
				      const String& eval_tag_prefix)
{
  // Retrieve parameters & results file names using fn. eval. id.  A map of
  // filenames is used because the names of tmp files must be available here
  // and asynch_recv operations can perform output filtering out of order
  // (which rules out making the file names into attributes of AnalysisCode or
  // rebuilding the file name from the root name plus the counter).
  std::map<int, std::pair<std::string, std::string> >::iterator map_iter =
    fileNameMap.find(id);
  const std::string& params_filename  = (map_iter->second).first;
  const std::string& results_filename = (map_iter->second).second;

  // If multiple analysis programs are used, then results_filename is tagged
  // with program number (to match ForkAnalysisCode::fork_application), e.g.
  // results.out.20.2 is the 2nd analysis results from the 20th fn eval.
  size_t i;
  StringArray results_filenames;
  if (numPrograms > 1) { // append program counter to results file
    results_filenames.resize(numPrograms);
    for (i=0; i<numPrograms; ++i) {
      const std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
      results_filenames[i] = results_filename + prog_num;
    }
  }

  // Read the results file(s).  If there's more than one program, then partial
  // responses must be overlaid to create the total response.  If an output
  // filter is used, then it has the responsibility to perform the overlay and
  // map results.out.[eval#].[1->numPrograms] to results.out.[eval#].  If no
  // output filter is used, then perform the overlay here.
  if (numPrograms > 1 && oFilterName.empty()) {
    response.reset();
    Response partial_response = response.copy();
    for (i=0; i<numPrograms; ++i) {
      std::ifstream recovery_stream(results_filenames[i].c_str());
      if (!recovery_stream)
	cleanup_and_abort(results_filenames[i]);
      recovery_stream >> partial_response;
      response.overlay(partial_response);
    }
  }
  else {
    std::ifstream recovery_stream(results_filename.c_str());
    if (!recovery_stream)
      cleanup_and_abort(results_filename);
    recovery_stream >> response;
  }

  // Clean up files based on fileSaveFlag.
  if (!fileSaveFlag) {
    if (!suppressOutputFlag && outputLevel > NORMAL_OUTPUT) {
      Cout << "Removing " << params_filename.c_str();
      if (multipleParamsFiles) {
        if (!iFilterName.empty())
          Cout << " and " << params_filename.c_str();
        Cout << ".[1-" << numPrograms << ']';
      }
      Cout << " and " << results_filename;
      if (numPrograms > 1) {
        if (!oFilterName.empty())
          Cout << " and " << results_filename;
        Cout << ".[1-" << numPrograms << ']';
      }
      Cout << '\n';
    }
    if (!multipleParamsFiles || !iFilterName.empty())
      std::remove(params_filename.c_str());
    if (multipleParamsFiles) {
      for (i=0; i<numPrograms; ++i) {
        const std::string prog_num("." + boost::lexical_cast<std::string>(i+1));
	const std::string tag_params_filename = params_filename + prog_num;
	std::remove(tag_params_filename.c_str());
      }
    }
    if (numPrograms == 1 || !oFilterName.empty())
      std::remove(results_filename.c_str());

    if (numPrograms > 1)
      for (i=0; i<numPrograms; ++i)
	std::remove(results_filenames[i].c_str());

    // optionally remove the work_directory 
    // BMA: appears we only want to remove if tagged, since otherwise
    // needs to be reused.  Later fix this.
    //    if (useWorkdir && !dirSave) {
    if (useWorkdir && !dirSave && dirTag) {
      std::string wd = workDir;
      //if (dirTag) {
	// can't rely on class member due to split call to write/read 
	String eval_id_tag = "." + boost::lexical_cast<std::string>(id);
	if (eval_tag_prefix.empty()) {
	  // classic behavior
	  wd += eval_id_tag;   
	}
	else {
	  // hierarchical tagging
	  wd += eval_tag_prefix;
	  if (!fileTagFlag)
	    wd += eval_id_tag;
	}

      //}
      rec_rmdir(wd.c_str());
    }
  }

  // Prevent overwriting of files with reused names for which a file_save
  // request has been given.
  if ( fileSaveFlag && !fileTagFlag && !dirTag &&
       ( !specifiedParamsFileName.empty() ||
	 !specifiedResultsFileName.empty() ) ) {
    // specifiedParamsFileName/specifiedResultsFileName are original user input
    if (!suppressOutputFlag && outputLevel > NORMAL_OUTPUT)
      Cout << "Files with nonunique names will be tagged for file_save:\n";

    // can't rely on class member due to split call to write/read 
    String file_suffix;
    if (eval_tag_prefix.empty()) {
      // classic behavior
      file_suffix = "." + boost::lexical_cast<std::string>(id);
    }
    else {
      // hierarchical tagging (in this block dirTag is false, so all
      // the tags are on the file name)
      file_suffix = eval_tag_prefix + "." + boost::lexical_cast<std::string>(id);
    }

    if (!specifiedParamsFileName.empty()) {
      const std::string new_str = specifiedParamsFileName + file_suffix;
      if (!multipleParamsFiles || !iFilterName.empty()) {
	if (!suppressOutputFlag && outputLevel > NORMAL_OUTPUT)
	  Cout << "Moving " << specifiedParamsFileName << " to " << new_str
               << '\n';
	rename(specifiedParamsFileName.c_str(), new_str.c_str());
      }
      if (multipleParamsFiles) { // append program counters to old/new strings
        for (i=0; i<numPrograms; ++i) {
          const std::string prog_num("."+boost::lexical_cast<std::string>(i+1));
          const std::string old_tagged_str = specifiedParamsFileName + prog_num;
          const std::string new_tagged_str = new_str + prog_num;
          if (!suppressOutputFlag && outputLevel > NORMAL_OUTPUT)
            Cout << "Moving " << old_tagged_str << " to " << new_tagged_str
                 << '\n';
          rename(old_tagged_str.c_str(), new_tagged_str.c_str());
	}
      }
    }
    if (!specifiedResultsFileName.empty()) {
      const std::string new_str = specifiedResultsFileName + file_suffix;
      if (numPrograms == 1 || !oFilterName.empty()) {
        if (!suppressOutputFlag && outputLevel > NORMAL_OUTPUT)
          Cout << "Moving " << specifiedResultsFileName << " to "
               << new_str << '\n';
        rename(specifiedResultsFileName.c_str(), new_str.c_str());
      }
      if (numPrograms > 1) { // append program counters to old/new strings
        for (i=0; i<numPrograms; ++i) {
          const std::string prog_num("."+boost::lexical_cast<std::string>(i+1));
          const std::string old_tagged_str = specifiedResultsFileName+prog_num;
          const std::string new_tagged_str = new_str + prog_num;
          if (!suppressOutputFlag && outputLevel > NORMAL_OUTPUT)
            Cout << "Moving " << old_tagged_str << " to " << new_tagged_str
                 << '\n';
          rename(old_tagged_str.c_str(), new_tagged_str.c_str());
	}
      }
    }
  }

  // Remove the evaluation which has been processed from the bookkeeping
  fileNameMap.erase(map_iter);
}


AnalysisCode::~AnalysisCode()
{
  if (dirDel && 
      (templateDir.length() || templateFiles.size() || rmdir(workDir.c_str())))
    rec_rmdir(workDir.c_str());
}


void AnalysisCode::file_cleanup() const
{
  if (fileSaveFlag)
    return;

  bool rrf = !multipleParamsFiles || !iFilterName.empty();
  std::string tname;

  std::map<int, std::pair<std::string, std::string> >::const_iterator
    file_name_map_it = fileNameMap.begin(),
    file_name_map_end = fileNameMap.end();
  for(; file_name_map_it != file_name_map_end; ++file_name_map_it) {
    const std::string& parfile = (file_name_map_it->second).first;
    const std::string& resfile = (file_name_map_it->second).second;
    if (rrf) {
      std::remove(parfile.c_str());
      std::remove(resfile.c_str());
    }
    if (multipleParamsFiles) {
      for(size_t i = 1; i <= numPrograms; ++i) {
        std::string prog_num("." + boost::lexical_cast<std::string>(i));
	tname = parfile + prog_num;
	std::remove(tname.c_str());
	tname = resfile + prog_num;
	std::remove(tname.c_str());
      }
    }
  }
}

} // namespace Dakota
