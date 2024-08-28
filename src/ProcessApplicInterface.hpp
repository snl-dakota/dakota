/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef PROCESS_APPLIC_INTERFACE_H
#define PROCESS_APPLIC_INTERFACE_H

#include "ApplicationInterface.hpp"
#include <memory>

#ifdef _WIN32
typedef intptr_t pid_t;
#endif

#include <boost/tuple/tuple.hpp>
#include <boost/filesystem/path.hpp>
namespace bfs = boost::filesystem;

namespace Dakota {


class ParametersFileWriter;
class ResultsFileReader;

/// Substitute parameters and results file names into driver strings
String substitute_params_and_results(const String &driver, const String &params, const String &results);


/// Triplet of filesystem paths: e.g., params, results, workdir 
typedef boost::tuple<bfs::path, bfs::path, bfs::path> PathTriple;


/// Derived application interface class that spawns a simulation code
/// using a separate process and communicates with it through files.

/** ProcessApplicInterface is subclassed for process handles or file
    completion testing. */

class ProcessApplicInterface: public ApplicationInterface
{
public:

  //
  //- Heading: Constructors and destructor
  //

  /// constructor
  ProcessApplicInterface(const ProblemDescDB& problem_db);
  /// destructor
  ~ProcessApplicInterface();

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void derived_map(const Variables& vars, const ActiveSet& set,
		   Response& response, int fn_eval_id);
  void derived_map_asynch(const ParamResponsePair& pair);

  void wait_local_evaluations(PRPQueue& prp_queue);
  void test_local_evaluations(PRPQueue& prp_queue);

  const StringArray& analysis_drivers() const;

  void file_cleanup() const;

  void file_and_workdir_cleanup(const bfs::path &params_path,
      const bfs::path &results_path,
      const bfs::path &workdir_path,
      const String &tag) const;

  /// Remove (potentially autotagged for multiple programs) parameters
  /// and results files with passed root names
  void remove_params_results_files(const bfs::path& params_path, 
				   const bfs::path& results_path) const;


  /// Utility to automatically tag parameters and results files with
  /// passed root names (the files may already need per-program
  /// tagging)
  void autotag_files(const bfs::path& params_path, 
		     const bfs::path& results_path,
		     const String& eval_id_tag
		     //, const bfs::path dest_dir = bfs::path()
		     ) const;


  //
  //- Heading: New virtual functions
  //

  /// version of wait_local_evaluations() managing of set of individual
  /// asynchronous evaluations
  virtual void wait_local_evaluation_sequence(PRPQueue& prp_queue) = 0;
  /// version of test_local_evaluations() managing of set of individual
  /// asynchronous evaluations
  virtual void test_local_evaluation_sequence(PRPQueue& prp_queue) = 0;

  /// bookkeeping of process and evaluation ids for asynchronous maps
  virtual void map_bookkeeping(pid_t pid, int fn_eval_id) = 0;

  /// Spawn the evaluation by managing the input filter, analysis drivers,
  /// and output filter.  Called from derived_map() & derived_map_asynch().
  virtual pid_t create_evaluation_process(bool block_flag) = 0;

  //
  //- Heading: Methods
  //

  /// batch version of wait_local_evaluations()
  void wait_local_evaluation_batch(PRPQueue& prp_queue);
  /// batch version of test_local_evaluations()
  void test_local_evaluation_batch(PRPQueue& prp_queue);

/// execute analyses synchronously on the local processor
  void synchronous_local_analyses(int start, int end, int step);

  //void clear_bookkeeping(); // virtual fn redefinition: clear processIdMap

  /// define modified filenames from user input by handling Unix temp
  /// file and optionally tagging with given eval_id_tag
  void define_filenames(const String& eval_id_tag);

  /// write the parameters data and response request data to one or
  /// more parameters files
  void write_parameters_files(const Variables& vars,     const ActiveSet& set,
			      const Response&  response, const int id);

  /// read the response object from one or more results files using
  /// full eval_id_tag passed
  void read_results_files(Response& response, const int id,
			  const String& eval_id_tag);

  /// construct a work directory name (tmp or named), with optional tag
  bfs::path get_workdir_name();
  
  /// set PATH, environment variables, and change directory prior to
  /// fork/system/spawn
  void prepare_process_environment();

  /// reset PATH and current directory after system/spawn (workdir case)
  void reset_process_environment();

  //
  //- Heading: Data
  //

  /// flags tagging of parameter/results files
  bool fileTagFlag;
  /// flags retention of parameter/results files
  bool fileSaveFlag;
  /// flag indicating use of passing of filenames as command line arguments to
  /// the analysis drivers and input/output filters
  bool commandLineArgs;
  /// parameters file writer
  std::unique_ptr<ParametersFileWriter> paramsFileWriter;
  /// results file reader
  std::unique_ptr<ResultsFileReader> resultsFileReader;
  /// flag indicating the need for separate parameters files for multiple
  /// analysis drivers
  bool multipleParamsFiles;

  /// the name of the input filter (input_filter user specification)
  std::string iFilterName;
  /// the name of the output filter (output_filter user specification)
  std::string oFilterName;
  /// the names of the analysis code programs (analysis_drivers user
  /// specification)
  std::vector<String> programNames;
  /// the name of the parameters file from user specification
  std::string specifiedParamsFileName;
  /// the parameters file name actually used (modified with tagging or
  /// temp files); only valid from define_filenames to write_parameters_files
  std::string paramsFileName;

  /// actual, qualified name of the params file written, possibly with workdir
  std::string paramsFileWritten;

  /// the name of the results file from user specification
  std::string specifiedResultsFileName;
  /// the results file name actually used (modified with tagging or
  /// temp files); only valid from define_filenames to write_parameters_files
  std::string resultsFileName;

  /// actual, qualified name of the results file written, possibly with workdir
  std::string resultsFileWritten;

  /// complete evalIdTag, possibly including hierarchical tagging and
  /// final eval id, but not program numbers, for passing to
  /// write_parameters_files
  std::string fullEvalId;

  /// by default analysis code interfaces delete results files if they
  /// exist; user may override with this flag and we'll try to gather
  /// and only fork if needed
  bool allowExistingResults;

  /// Maps function evaluation ID to triples (parameters, results, and
  /// workdir) paths used in spawning function evaluations.  Workdir
  /// will be empty if not created specifically for this eval.
  std::map<int, PathTriple> fileNameMap;

  // work_directory creation/removal controls

  /// whether to use a work_directory
  bool useWorkdir;
  /// work_directory name, if specified...
  std::string workDirName;
  /// whether to tag the working directory
  bool dirTag;
  /// whether dir_save was specified
  bool dirSave;
  /// active working directory for this evaluation; valid only from
  /// define_filenames to create_evaluation_process
  bfs::path curWorkdir;

  /// non-empty if created for this eval; valid only from
  /// define_filenames to write_parameters_files
  bfs::path createdDir;

  // work directory population controls

  /// template directory (if specified)
  StringArray linkFiles;
  /// template files (if specified)
  StringArray copyFiles;
  /// whether to replace existing files
  bool templateReplace;

private:

  //
  //- Heading: Convenience functions
  //

  //
  //- Heading: Data
  //

};


/** Execute analyses synchronously in succession on the local
    processor (start to end in step increments).  Modeled after
    ApplicationInterface::synchronous_local_evaluations(). */
inline void ProcessApplicInterface::
synchronous_local_analyses(int start, int end, int step)
{
  for (int analysis_id=start; analysis_id<=end; analysis_id+=step)
    synchronous_local_analysis(analysis_id);
}


inline const StringArray& ProcessApplicInterface::analysis_drivers() const
{ return programNames; }

} // namespace Dakota

#endif
