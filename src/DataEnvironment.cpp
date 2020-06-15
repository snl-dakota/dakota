/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataEnvironment
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataEnvironment.hpp"
#include "dakota_data_io.hpp"
#include "dakota_global_defs.hpp"

using namespace std;

namespace Dakota {

// Default constructor:
DataEnvironmentRep::DataEnvironmentRep():
  checkFlag(false), stopRestart(0), 
  preRunFlag(false), runFlag(false), postRunFlag(false),
  preRunOutputFormat(TABULAR_ANNOTATED), postRunInputFormat(TABULAR_ANNOTATED),
  graphicsFlag(false), tabularDataFlag(false), 
  tabularDataFile("dakota_tabular.dat"), tabularFormat(TABULAR_ANNOTATED), 
  outputPrecision(0), 
  resultsOutputFlag(false), resultsOutputFile("dakota_results"),
  resultsOutputFormat(0), modelEvalsSelection(MODEL_EVAL_STORE_TOP_METHOD),
  interfEvalsSelection(INTERF_EVAL_STORE_SIMULATION)
{ }


void DataEnvironmentRep::write(MPIPackBuffer& s) const
{
  s << checkFlag 
    << outputFile << errorFile << readRestart << stopRestart << writeRestart
    << preRunFlag << runFlag << postRunFlag << preRunInput << preRunOutput
    << runInput << runOutput << postRunInput << postRunOutput
    << preRunOutputFormat << postRunInputFormat
    << graphicsFlag << tabularDataFlag << tabularDataFile << tabularFormat 
    << outputPrecision << resultsOutputFlag << resultsOutputFile 
    << resultsOutputFormat << modelEvalsSelection << interfEvalsSelection
    << topMethodPointer;
}


void DataEnvironmentRep::read(MPIUnpackBuffer& s)
{
  s >> checkFlag 
    >> outputFile >> errorFile >> readRestart >> stopRestart >> writeRestart
    >> preRunFlag >> runFlag >> postRunFlag >> preRunInput >> preRunOutput
    >> runInput >> runOutput >> postRunInput >> postRunOutput
    >> preRunOutputFormat >> postRunInputFormat
    >> graphicsFlag >> tabularDataFlag >> tabularDataFile >> tabularFormat 
    >> outputPrecision
    >> resultsOutputFlag >> resultsOutputFile >> resultsOutputFormat 
    >> modelEvalsSelection >> interfEvalsSelection >> topMethodPointer;
}


void DataEnvironmentRep::write(std::ostream& s) const
{
  s << checkFlag 
    << outputFile << errorFile << readRestart << stopRestart << writeRestart
    << preRunFlag << runFlag << postRunFlag << preRunInput << preRunOutput
    << runInput << runOutput << postRunInput << postRunOutput
    << preRunOutputFormat << postRunInputFormat
    << graphicsFlag << tabularDataFlag << tabularDataFile << tabularFormat 
    << outputPrecision
    << resultsOutputFlag << resultsOutputFile << resultsOutputFormat 
    << modelEvalsSelection << interfEvalsSelection << topMethodPointer;
}


DataEnvironment::DataEnvironment(): dataEnvRep(new DataEnvironmentRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataEnvironment::DataEnvironment(), dataEnvRep referenceCount = "
       << dataEnvRep.use_count() << endl;
#endif
}


DataEnvironment::DataEnvironment(const DataEnvironment& data_env):
  dataEnvRep(data_env.dataEnvRep)
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataEnvironment::DataEnvironment(DataEnvironment&)" << endl;
  if (dataEnvRep)
    Cout << "dataEnvRep referenceCount = " << dataEnvRep.use_count()
	 << endl;
#endif
}


DataEnvironment& DataEnvironment::operator=(const DataEnvironment& data_env)
{
  dataEnvRep = data_env.dataEnvRep;

#ifdef REFCOUNT_DEBUG
  Cout << "DataEnvironment::operator=(DataEnvironment&)" << endl;
  if (dataEnvRep)
    Cout << "dataEnvRep referenceCount = " << dataEnvRep.use_count()
	 << endl;
#endif

  return *this;
}


DataEnvironment::~DataEnvironment()
{
#ifdef REFCOUNT_DEBUG
  Cout << "~DataEnvironment() dataEnvRep referenceCount "
       << dataEnvRep.use_count() << endl;
#endif
}

} // namespace Dakota
