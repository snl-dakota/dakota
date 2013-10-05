/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ProcessApplicInterface
//- Description:  Derived class for the case when analysis code simulators use
//-               vfork\exec\wait to provide the function evaluations
//- Owner:        Mike Eldred
//- Version: $Id: ProcessApplicInterface.hpp 6492 2009-12-19 00:04:28Z briadam $

#ifndef PROCESS_APPLIC_INTERFACE_H
#define PROCESS_APPLIC_INTERFACE_H

#include "ApplicationInterface.hpp"
#ifdef _WIN32
typedef int pid_t;
#endif


namespace Dakota {

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

  const StringArray& analysis_drivers() const;

  void file_cleanup() const;

  //
  //- Heading: New virtual functions
  //

  /// bookkeeping of process and evaluation ids for asynchronous maps
  virtual void map_bookkeeping(pid_t pid, int fn_eval_id) = 0;

  /// Spawn the evaluation by managing the input filter, analysis drivers,
  /// and output filter.  Called from derived_map() & derived_map_asynch().
  virtual pid_t create_evaluation_process(bool block_flag) = 0;

  //
  //- Heading: Methods
  //

  /// execute analyses synchronously on the local processor
  void synchronous_local_analyses(int start, int end, int step);

  //void clear_bookkeeping(); // virtual fn redefinition: clear processIdMap

  /// define modified filenames from user input by handling Unix temp
  /// file and optionally tagging with given eval_id_tag
  void define_filenames(const String& eval_id_tag);

  /// write the parameters data and response request data to one or
  /// more parameters files (using one or more invocations of
  /// write_parameters_file()) in either standard or aprepro format
  void write_parameters_files(const Variables& vars,     const ActiveSet& set,
			      const Response&  response, const int id);

  /// read the response object from one or more results files using
  /// full eval_id_tag passed
  void read_results_files(Response& response, const int id,
			  const String& eval_id_tag);

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
  /// flag indicating use of the APREPRO (the Sandia "A PRE PROcessor" utility)
  /// format for parameter files
  bool apreproFlag;
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
  /// temp files)
  std::string paramsFileName;
  /// the name of the results file from user specification
  std::string specifiedResultsFileName;
  /// the results file name actually used (modified with tagging or temp files)
  std::string resultsFileName;
  /// complete evalIdTag, possibly including hierarchical tagging and
  /// final eval id, but not program numbers, for passing to
  /// write_parameters_files
  std::string fullEvalId;

  /// by default analysis code interfaces delete results files if they
  /// exist; user may override with this flag and we'll try to gather
  /// and only fork if needed
  bool allowExistingResults;
  /// working directory when useWorkdir is true
  std::string curWorkdir;

  /// stores parameters and results file names used in spawning function
  /// evaluations.  Map key is the function evaluation identifier.
  std::map<int, std::pair<std::string, std::string> > fileNameMap;

  /// whether to use a new or specified work_directory
  bool useWorkdir;
  /// its name, if specified...
  std::string workDir;
  /// whether to tag the working directory
  bool dirTag;
  /// whether dir_save was specified
  bool dirSave;
  /// whether to delete the directory when Dakota terminates
  bool dirDel;
  /// for dirTag, whether we have workDir
  bool haveWorkdir;

  /// template directory (if specified)
  std::string templateDir;
  /// template files (if specified)
  StringArray templateFiles;
  /// whether to force a copy (versus link) every time
  bool templateCopy;
  /// whether to replace existing files
  bool templateReplace;
  /// state variable for template directory
  bool haveTemplateDir;

  /// Dakota directory (if needed)
  std::string dakDir;

private:

  //
  //- Heading: Convenience functions
  //

  /// write the variables, active set vector, derivative variables vector,
  /// and analysis components to the specified parameters file in either
  /// standard or aprepro format
  void write_parameters_file(const Variables& vars, const ActiveSet& set,
			     const Response& response, const std::string& prog,
			     const std::vector<String>& an_comps,
			     const std::string& params_fname);

  //
  //- Heading: Data
  //

  /// the set of optional analysis components used by the analysis drivers
  /// (from the analysis_components interface specification)
  String2DArray analysisComponents;
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
