/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataEnvironment
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataEnvironment.hpp 6740 2010-04-30 16:12:00Z briadam $

#ifndef DATA_ENVIRONMENT_H
#define DATA_ENVIRONMENT_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"

namespace Dakota {


/// Body class for environment specification data.

/** The DataEnvironmentRep class is used to contain the data from the
    environment keyword specification.  Default values are managed in the
    DataEnvironmentRep constructor.  Data is public to avoid maintaining
    set/get functions, but is still encapsulated within ProblemDescDB
    since ProblemDescDB::environmentSpec is private. */

class DataEnvironmentRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataEnvironment;

public:

  //
  //- Heading: Data
  //

  /// flag for whether to run in check only mode (default false)
  bool checkFlag;

  /// file name for output redirection (overrides command-line)
  String outputFile;
  /// file name for error redirection (overrides command-line)
  String errorFile;
  /// file name for restart read (overrides command-line)
  String readRestart;
  /// record at which to stop reading restart
  int stopRestart;
  /// file name for restart write (overrides command-line)
  String writeRestart;

  bool preRunFlag;      ///< flags invocation with command line option -pre_run
  bool runFlag;         ///< flags invocation with command line option -run
  bool postRunFlag;     ///< flags invocation with command line option -post_run

  String preRunInput;   ///< filename for pre_run input
  String preRunOutput;  ///< filename for pre_run output
  String runInput;      ///< filename for run input
  String runOutput;     ///< filename for run output
  String postRunInput;  ///< filename for post_run input
  String postRunOutput; ///< filename for post_run output

  unsigned short preRunOutputFormat;  ///< tabular format for pre_run output
  unsigned short postRunInputFormat;  ///< tabular format for post_run input

  /// flags use of graphics by the environment (from the \c graphics
  /// specification in \ref EnvIndControl)
  bool graphicsFlag;
  /// flags tabular data collection by the environment (from
  /// the tabular_graphics_data specification in \ref EnvIndControl)
  bool tabularDataFlag;
  /// the filename used for tabular data collection by the environment (from
  /// the tabular_graphics_file specification in \ref EnvIndControl)
  String tabularDataFile;
  /// format for tabular data files (see enum)
  unsigned short tabularFormat;

  /// output precision for tabular and screen output
  int outputPrecision;

  /// flags use of results output to default file
  bool resultsOutputFlag;
  /// named file for results output
  String resultsOutputFile;

  /// method identifier for the environment (from the \c top_method_pointer
  /// specification
  String topMethodPointer;

private:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataEnvironmentRep();  ///< constructor
  ~DataEnvironmentRep(); ///< destructor

  //
  //- Heading: Member methods
  //

  /// write a DataEnvironmentRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataEnvironmentRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataEnvironmentRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing this dataEnvironmentRep
  int referenceCount;
};


inline DataEnvironmentRep::~DataEnvironmentRep() { }


/// Handle class for environment specification data.

/** The DataEnvironment class is used to provide a memory management
    handle for the data in DataEnvironmentRep.  It is populated by
    IDRProblemDescDB::environment_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A single DataEnvironment
    object is maintained in ProblemDescDB::environmentSpec. */

class DataEnvironment
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;

public:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataEnvironment();                                ///< constructor
  DataEnvironment(const DataEnvironment&);             ///< copy constructor
  ~DataEnvironment();                               ///< destructor

  DataEnvironment& operator=(const DataEnvironment&); ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// write a DataEnvironment object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataEnvironment object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataEnvironment object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  /// return dataEnvRep
  DataEnvironmentRep* data_rep();

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataEnvironmentRep* dataEnvRep;
};


inline DataEnvironmentRep* DataEnvironment::data_rep()
{return dataEnvRep; }


/// MPIPackBuffer insertion operator for DataEnvironment
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataEnvironment& data)
{ data.write(s); return s;}


/// MPIUnpackBuffer extraction operator for DataEnvironment
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataEnvironment& data)
{ data.read(s); return s;}


/// std::ostream insertion operator for DataEnvironment
inline std::ostream& operator<<(std::ostream& s, const DataEnvironment& data)
{ data.write(s); return s;}


inline void DataEnvironment::write(std::ostream& s) const
{ dataEnvRep->write(s); }


inline void DataEnvironment::read(MPIUnpackBuffer& s)
{ dataEnvRep->read(s); }


inline void DataEnvironment::write(MPIPackBuffer& s) const
{ dataEnvRep->write(s); }

} // namespace Dakota

#endif
