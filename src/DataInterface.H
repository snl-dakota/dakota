/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataInterface
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataInterface.H 6959 2010-09-09 19:10:51Z briadam $

#ifndef DATA_INTERFACE_H
#define DATA_INTERFACE_H

#include "system_defs.h"
#include "data_types.h"
#include "DakotaString.H"

namespace Dakota {

/// define algebraic function types
enum { OBJECTIVE, INEQUALITY_CONSTRAINT, EQUALITY_CONSTRAINT };


/// Body class for interface specification data.

/** The DataInterfaceRep class is used to contain the data from a
    interface keyword specification.  Default values are managed in
    the DataInterfaceRep constructor.  Data is public to avoid
    maintaining set/get functions, but is still encapsulated within
    ProblemDescDB since ProblemDescDB::dataInterfaceList is private (a
    similar approach is used with SurrogateDataPoint objects contained
    in Dakota::Approximation). */

class DataInterface;

class DataInterfaceRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataInterface;

public:

  //
  //- Heading: Data
  //

  /// string identifier for an interface specification data set
  /// (from the id_interface specification in \ref InterfIndControl)
  String idInterface;
  /// the interface selection: system, fork, direct, or grid
  String interfaceType;
  /// defines the subset of the parameter to response mapping that is
  /// explicit and algebraic.  This is typically a stub.nl filename
  /// (AMPL format) from JAGUAR.
  String algebraicMappings;
  /// the set of analysis drivers for a simulation-based interface
  /// (from the \c analysis_drivers specification in \ref InterfIndControl)
  StringArray analysisDrivers;
  /// the set of analysis components for a simulation-based interface
  /// (from the \c analysis_components specification in \ref InterfIndControl)
  String2DArray analysisComponents;
  /// the input filter for a simulation-based interface (from the \c
  /// input_filter specification in \ref InterfIndControl)
  String inputFilter;
  /// the output filter for a simulation-based interface (from the \c
  /// output_filter specification in \ref InterfIndControl)
  String outputFilter;
  /// the parameters filename for system call and fork interfaces
  /// (from the \c parameters_file specification in \ref InterfApplicSC
  /// and \ref InterfApplicF)
  String parametersFile;
  /// the results filename for system call and fork interfaces (from
  /// the \c results_file specification in \ref InterfApplicSC and
  /// \ref InterfApplicF)
  String resultsFile;
  /// currently: when true, don't delete existing results files
  /// later: when true, also check existing files before doing fork/system call
  bool allowExistingResultsFlag;
  /// flag for suppressing command line augmentation of the specified
  /// analysis_drivers/input_filter/output_filter syntax (from the \c
  /// verbatim specification in \ref InterfApplicSC and \ref InterfApplicF)
  bool verbatimFlag;
  /// flag for aprepro format usage in the parameters file for
  /// system call and fork interfaces (from the \c aprepro
  /// specification in \ref InterfApplicSC and \ref InterfApplicF)
  bool apreproFlag;
  /// flag for file tagging of parameters and results files for
  /// system call and fork interfaces (from the \c file_tag
  /// specification in \ref InterfApplicSC and \ref InterfApplicF)
  bool fileTagFlag;
  /// flag for saving of parameters and results files for
  /// system call and fork interfaces (from the \c file_save
  /// specification in \ref InterfApplicSC and \ref InterfApplicF)
  bool fileSaveFlag;
  /// processors per parallel analysis for a direct interface (from the
  /// \c processors_per_analysis specification in \ref InterfApplicDF)
  int procsPerAnalysis;
  // names of host machines for a grid interface (from the
  // \c hostnames specification in \ref InterfApplicG)
  //StringArray gridHostNames;
  // processors per host machine for a grid interface (from the \c
  // processors_per_host specification in \ref InterfApplicG)
  //IntArray gridProcsPerHost;
  /// parallel mode for a simulation-based interface: synchronous or
  /// asynchronous (from the \c asynchronous specification in \ref
  /// InterfIndControl)
  String interfaceSynchronization;
  /// evaluation concurrency for asynchronous simulation-based interfaces (from
  /// the \c evaluation_concurrency specification in \ref InterfIndControl)
  int asynchLocalEvalConcurrency;
  /// scheduling approach for asynchronous local evaluations (from the 
  /// \c local_evaluation_self_scheduling and 
  /// \c local_evaluation_static_scheduling specifications in
  /// \ref InterfIndControl)
  String asynchLocalEvalScheduling;
  /// analysis concurrency for asynchronous simulation-based interfaces
  /// (from the \c analysis_concurrency specification in \ref InterfIndControl)
  int asynchLocalAnalysisConcurrency;
  /// number of evaluation servers to be used in the parallel configuration
  /// (from the \c evaluation_servers specification in \ref InterfIndControl)
  int evalServers;
  /// the scheduling approach to be used for concurrent evaluations
  /// within an iterator (from the \c evaluation_self_scheduling and
  /// evaluation_static_scheduling specifications in \ref InterfIndControl)
  String evalScheduling;
  /// number of analysis servers to be used in the parallel configuration
  /// (from the \c analysis_servers specification in \ref InterfIndControl)
  int analysisServers;
  /// the scheduling approach to be used for concurrent analyses within
  /// a function evaluation (from the \c analysis_self_scheduling and
  /// \c analysis_static_scheduling specifications in \ref InterfIndControl)
  String analysisScheduling;
  /// the selected action upon capture of a simulation failure:
  /// abort, retry, recover, or continuation (from the \c failure_capture
  /// specification in \ref InterfIndControl)
  String failAction;
  /// the limit on retries for captured simulation failures (from the
  /// \c retry specification in \ref InterfIndControl)
  int retryLimit;
  /// the function values to be returned in a recovery operation for
  /// captured simulation failures (from the \c recover specification
  /// in \ref InterfIndControl)
  RealVector recoveryFnVals;
  /// active set vector: 1=active (ASV control on), 0=inactive (ASV control
  /// off) (from the \c deactivate \c active_set_vector specification in
  /// \ref InterfIndControl)
  bool activeSetVectorFlag;
  /// function evaluation cache: 1=active (all new evaluations checked against
  /// existing cache and then added to cache), 0=inactive (cache neither
  /// queried nor augmented) (from the \c deactivate \c evaluation_cache
  /// specification in \ref InterfIndControl)
  bool evalCacheFlag;
  /// function evaluation cache: 1=active (all new evaluations written to
  /// restart), 0=inactive (no records written to restart) (from the
  /// \c deactivate \c restart_file specification in \ref InterfIndControl)
  bool restartFileFlag;
  /// whether to use a new or specified work_directory
  bool useWorkdir;
  /// its name, if specified...
  String workDir;
  /// whether to tag the working_directory
  bool dirTag;
  /// whether dir_save was specified
  bool dirSave;
  /// template directory (if specified)
  String templateDir;
  /// template files (if specified)
  StringArray templateFiles;
  /// whether to force a copy (versus link) every time
  bool templateCopy;
  /// whether to replace existing files
  bool templateReplace;

private:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataInterfaceRep();  ///< constructor
  ~DataInterfaceRep(); ///< destructor

  //
  //- Heading: Member methods
  //

  /// write a DataInterfaceRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataInterfaceRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataInterfaceRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing this dataInterfaceRep
  int referenceCount;
};


inline DataInterfaceRep::~DataInterfaceRep()
{ }


/// Handle class for interface specification data.

/** The DataInterface class is used to provide a memory management
    handle for the data in DataInterfaceRep.  It is populated by
    IDRProblemDescDB::interface_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A list of
    DataInterface objects is maintained in
    ProblemDescDB::dataInterfaceList, one for each interface
    specification in an input file. */

class DataInterface
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;
  // library_mode default data initializer
  friend void run_dakota_data();

public:

  // comparison function
  static bool id_compare(const DataInterface& di, const std::string& id)
  { return id == di.dataIfaceRep->idInterface; }

  //
  //- Heading: Constructors, destructor, operators
  //

  DataInterface();                                ///< constructor
  DataInterface(const DataInterface&);            ///< copy constructor
  ~DataInterface();                               ///< destructor

  DataInterface& operator=(const DataInterface&); ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// write a DataInterface object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataInterface object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);

  /// write a DataInterface object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataInterfaceRep* dataIfaceRep;
};


/// MPIPackBuffer insertion operator for DataInterface
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataInterface& data)
{ data.write(s); return s;}


/// MPIUnpackBuffer extraction operator for DataInterface
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataInterface& data)
{ data.read(s); return s;}


/// std::ostream insertion operator for DataInterface
inline std::ostream& operator<<(std::ostream& s, const DataInterface& data)
{ data.write(s); return s;}


inline void DataInterface::write(std::ostream& s) const
{ dataIfaceRep->write(s); }


inline void DataInterface::read(MPIUnpackBuffer& s)
{ dataIfaceRep->read(s); }


inline void DataInterface::write(MPIPackBuffer& s) const
{ dataIfaceRep->write(s); }

} // namespace Dakota

#endif
