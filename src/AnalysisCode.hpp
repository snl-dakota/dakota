/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        AnalysisCode
//- Description:  Abstract base class for the analysis code simulators used
//-               by the interface classes to perform function evaluations.
//- Owner:        Mike Eldred
//- Version: $Id: AnalysisCode.hpp 7021 2010-10-12 22:19:01Z wjbohnh $

#ifndef ANALYSIS_CODE_H
#define ANALYSIS_CODE_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"


namespace Dakota {

class Variables;
class ActiveSet;
class Response;
class ProblemDescDB;
class ParallelLibrary;


/// Base class providing common functionality for derived classes
/// (SysCallAnalysisCode and ForkAnalysisCode) which spawn separate
/// processes for managing simulations.

/** The AnalysisCode class hierarchy provides simulation spawning
    services for ApplicationInterface derived classes and alleviates
    these classes of some of the specifics of simulation code
    management.  The hierarchy does not employ the letter-envelope
    technique since the ApplicationInterface derived classes
    instantiate the appropriate derived AnalysisCode class directly. */

class AnalysisCode
{
public:

  //
  //- Heading: Methods
  //

  /// define modified filenames from user input by handling Unix temp
  /// file and tagging options
  void define_filenames(const int id, const String& eval_tag_prefix);

  /// write the parameters data and response request data to one or
  /// more parameters files (using one or more invocations of
  /// write_parameters_file()) in either standard or aprepro format
  void write_parameters_files(const Variables& vars,     const ActiveSet& set,
			      const Response&  response, const int id);

  /// read the response object from one or more results files
  void read_results_files(Response& response, const int id,
			  const String& eval_tag_prefix);

  //
  //- Heading: Set and Inquire functions
  //

  //void io_filters(IOFilter& ifilter, IOFilter& ofilter);

  /// return programNames
  const std::vector<String>& program_names() const;
  /// return iFilterName
  const std::string& input_filter_name()    const;
  /// return oFilterName
  const std::string& output_filter_name()   const;

  /// return paramsFileName
  const std::string& parameters_filename()  const;
  /// return resultsFileName
  const std::string& results_filename()     const;
  /// return the results filename entry in fileNameMap corresponding to id
  const std::string& results_filename(const int id);

  /// set suppressOutputFlag
  void suppress_output_flag(const bool flag);
  /// return suppressOutputFlag
  bool suppress_output_flag() 	       const;

  /// return commandLineArgs
  bool command_line_arguments()        const;

  /// return multipleParamsFiles
  bool multiple_parameters_filenames() const;

  /// remove temporary files if not fileSaveFlag
  void file_cleanup() const;

protected:

  //
  //- Heading: Constructors and destructor (protected since only derived class
  //           objects should be instantiated)
  //

  AnalysisCode(const ProblemDescDB& problem_db); ///< constructor
  ~AnalysisCode();                               ///< destructor

  /// return Workdir if useWorkdir is true (only called by derived classes)
  const char* work_dir() const;

  //
  //- Heading: Data
  //

  /// flag set by master processor to suppress output from slave processors
  bool suppressOutputFlag;
  /// output verbosity level: {SILENT,QUIET,NORMAL,VERBOSE,DEBUG}_OUTPUT
  short outputLevel;
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
  /// the number of analysis code programs (length of programNames)
  size_t numPrograms;
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
  /// final eval id, but not program numbers
  std::string evalIdTag;
  /// by default analysis code interfaces delete results files if they
  /// exist; user may override with this flag and we'll try to gather
  /// and only fork if needed
  bool allowExistingResults;
  /// working directory when useWorkdir is true
  std::string curWorkdir;

  /// stores parameters and results file names used in spawning function
  /// evaluations.  Map key is the function evaluation identifier.
  std::map<int, std::pair<std::string, std::string> > fileNameMap;

  //IOFilter inputFilter;
  //IOFilter outputFilter;

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
  /// for dirTag, whether we have workDir
  bool haveWorkdir;
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

  /// reference to the ParallelLibrary object.  Used in define_filenames().
  ParallelLibrary& parallelLib;

  /// the set of optional analysis components used by the analysis drivers
  /// (from the analysis_components interface specification)
  String2DArray analysisComponents;
};


inline const std::vector<String>& AnalysisCode::program_names() const
{ return programNames; }

inline const std::string& AnalysisCode::input_filter_name() const
{ return iFilterName; }

inline const std::string& AnalysisCode::output_filter_name() const
{ return oFilterName; }

inline const std::string& AnalysisCode::parameters_filename() const
{ return paramsFileName; }

inline const std::string& AnalysisCode::results_filename() const
{ return resultsFileName; }

inline const std::string& AnalysisCode::results_filename(const int id)
{ return fileNameMap[id].second; }

inline void AnalysisCode::suppress_output_flag(const bool flag)
{ suppressOutputFlag = flag; }

inline bool AnalysisCode::suppress_output_flag() const
{ return suppressOutputFlag; }

inline bool AnalysisCode::command_line_arguments() const
{ return commandLineArgs; }

inline bool AnalysisCode::multiple_parameters_filenames() const
{ return multipleParamsFiles; }

inline const char* AnalysisCode::work_dir() const
{ return useWorkdir ? curWorkdir.c_str() : 0; }

//inline void AnalysisCode::io_filters(IOFilter& ifilter, IOFilter& ofilter)
//{ inputFilter = ifilter; outputFilter = ofilter; }

} // namespace Dakota

#endif
