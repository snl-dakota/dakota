/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataEnvironment
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataEnvironment.hpp"
#include "dakota_data_io.hpp"


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
  resultsOutputFlag(false), resultsOutputFile("dakota_results.txt"), 
  referenceCount(1)
{ }


void DataEnvironmentRep::write(MPIPackBuffer& s) const
{
  s << checkFlag 
    << outputFile << errorFile << readRestart << stopRestart << writeRestart
    << preRunFlag << runFlag << postRunFlag << preRunInput << preRunOutput
    << runInput << runOutput << postRunInput << postRunOutput
    << preRunOutputFormat << postRunInputFormat
    << graphicsFlag << tabularDataFlag << tabularDataFile << tabularFormat 
    << outputPrecision
    << resultsOutputFlag << resultsOutputFile << topMethodPointer;
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
    >> resultsOutputFlag >> resultsOutputFile >> topMethodPointer;
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
    << resultsOutputFlag << resultsOutputFile << topMethodPointer;
}


DataEnvironment::DataEnvironment(): dataEnvRep(new DataEnvironmentRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataEnvironment::DataEnvironment(), dataEnvRep referenceCount = "
       << dataEnvRep->referenceCount << endl;
#endif
}


DataEnvironment::DataEnvironment(const DataEnvironment& data_env)
{
  // Increment new (no old to decrement)
  dataEnvRep = data_env.dataEnvRep;
  if (dataEnvRep) // Check for an assignment of NULL
    ++dataEnvRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "DataEnvironment::DataEnvironment(DataEnvironment&)" << endl;
  if (dataEnvRep)
    Cout << "dataEnvRep referenceCount = " << dataEnvRep->referenceCount
	 << endl;
#endif
}


DataEnvironment& DataEnvironment::operator=(const DataEnvironment& data_env)
{
  if (dataEnvRep != data_env.dataEnvRep) { // normal case: old != new
    // Decrement old
    if (dataEnvRep) // Check for NULL
      if ( --dataEnvRep->referenceCount == 0 ) 
	delete dataEnvRep;
    // Assign and increment new
    dataEnvRep = data_env.dataEnvRep;
    if (dataEnvRep) // Check for NULL
      ++dataEnvRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataEnvironment::operator=(DataEnvironment&)" << endl;
  if (dataEnvRep)
    Cout << "dataEnvRep referenceCount = " << dataEnvRep->referenceCount
	 << endl;
#endif

  return *this;
}


DataEnvironment::~DataEnvironment()
{
  if (dataEnvRep) { // Check for NULL
    --dataEnvRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataEnvRep referenceCount decremented to "
         << dataEnvRep->referenceCount << endl;
#endif
    if (dataEnvRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataEnvRep" << endl;
#endif
      delete dataEnvRep;
    }
  }
}

} // namespace Dakota
