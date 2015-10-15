/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataInterface.hpp"
#include "DataMethod.hpp" // shared scheduling enums
#include "dakota_data_io.hpp"


namespace Dakota {

DataInterfaceRep::DataInterfaceRep():
  interfaceType(DEFAULT_INTERFACE),
  allowExistingResultsFlag(false), verbatimFlag(false), apreproFlag(false),
  resultsFileFormat(FLEXIBLE_RESULTS), fileTagFlag(false), fileSaveFlag(false),
  interfaceSynchronization(SYNCHRONOUS_INTERFACE),
  asynchLocalEvalConcurrency(0), asynchLocalEvalScheduling(DEFAULT_SCHEDULING),
  asynchLocalAnalysisConcurrency(0), evalServers(0),
  evalScheduling(DEFAULT_SCHEDULING), procsPerEval(0), analysisServers(0),
  analysisScheduling(DEFAULT_SCHEDULING), procsPerAnalysis(0),
  failAction("abort"), retryLimit(1), activeSetVectorFlag(true),
  evalCacheFlag(true), nearbyEvalCacheFlag(false),
  nearbyEvalCacheTol(DBL_EPSILON), // default relative tolerance is tight
  restartFileFlag(true), referenceCount(1), useWorkdir(false), dirTag(false),
  dirSave(false), templateReplace(false), numpyFlag(false)
  // asynchLocal{Eval,Analysis}Concurrency, procsPer{Eval,Analysis} and
  // {eval,analysis}Servers default to zero in order to allow detection of
  // user overrides > 0
{ }


void DataInterfaceRep::write(MPIPackBuffer& s) const
{
  s << idInterface << interfaceType << algebraicMappings << analysisDrivers
    << analysisComponents << inputFilter << outputFilter << parametersFile
    << resultsFile << allowExistingResultsFlag  << verbatimFlag << apreproFlag 
    << resultsFileFormat << fileTagFlag << fileSaveFlag //<< gridHostNames << gridProcsPerHost
    << interfaceSynchronization << asynchLocalEvalConcurrency
    << asynchLocalEvalScheduling << asynchLocalAnalysisConcurrency
    << evalServers << evalScheduling << procsPerEval << analysisServers
    << analysisScheduling << procsPerAnalysis << failAction << retryLimit
    << recoveryFnVals << activeSetVectorFlag << evalCacheFlag
    << nearbyEvalCacheFlag << nearbyEvalCacheTol << restartFileFlag
    << useWorkdir << workDir << dirTag << dirSave << linkFiles
    << copyFiles << templateReplace << numpyFlag;
}


void DataInterfaceRep::read(MPIUnpackBuffer& s)
{
  s >> idInterface >> interfaceType >> algebraicMappings >> analysisDrivers
    >> analysisComponents >> inputFilter >> outputFilter >> parametersFile
    >> resultsFile >> allowExistingResultsFlag  >> verbatimFlag >> apreproFlag 
    >> resultsFileFormat >> fileTagFlag >> fileSaveFlag //>> gridHostNames >> gridProcsPerHost
    >> interfaceSynchronization >> asynchLocalEvalConcurrency
    >> asynchLocalEvalScheduling >> asynchLocalAnalysisConcurrency
    >> evalServers >> evalScheduling >> procsPerEval >> analysisServers
    >> analysisScheduling >> procsPerAnalysis >> failAction >> retryLimit
    >> recoveryFnVals >> activeSetVectorFlag >> evalCacheFlag
    >> nearbyEvalCacheFlag >> nearbyEvalCacheTol >> restartFileFlag
    >> useWorkdir >> workDir >> dirTag >> dirSave >> linkFiles
    >> copyFiles >> templateReplace >> numpyFlag;
}


void DataInterfaceRep::write(std::ostream& s) const
{
  s << idInterface << interfaceType << algebraicMappings << analysisDrivers
    << analysisComponents << inputFilter << outputFilter << parametersFile
    << resultsFile << allowExistingResultsFlag  << verbatimFlag << apreproFlag 
    << resultsFileFormat << fileTagFlag << fileSaveFlag //<< gridHostNames << gridProcsPerHost
    << interfaceSynchronization << asynchLocalEvalConcurrency
    << asynchLocalEvalScheduling << asynchLocalAnalysisConcurrency
    << evalServers << evalScheduling << procsPerEval << analysisServers
    << analysisScheduling << procsPerAnalysis << failAction << retryLimit
    << recoveryFnVals << activeSetVectorFlag << evalCacheFlag
    << nearbyEvalCacheFlag << nearbyEvalCacheTol << restartFileFlag
    << useWorkdir << workDir << dirTag << dirSave << linkFiles
    << copyFiles << templateReplace << numpyFlag;
}


DataInterface::DataInterface(): dataIfaceRep(new DataInterfaceRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataInterface::DataInterface(), dataIfaceRep referenceCount = "
       << dataIfaceRep->referenceCount << std::endl;
#endif
}


DataInterface::DataInterface(const DataInterface& data_resp)
{
  // Increment new (no old to decrement)
  dataIfaceRep = data_resp.dataIfaceRep;
  if (dataIfaceRep) // Check for an assignment of NULL
    ++dataIfaceRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "DataInterface::DataInterface(DataInterface&)" << std::endl;
  if (dataIfaceRep)
    Cout << "dataIfaceRep referenceCount = " << dataIfaceRep->referenceCount
	 << std::endl;
#endif
}


DataInterface& DataInterface::operator=(const DataInterface& data_interface)
{
  if (dataIfaceRep != data_interface.dataIfaceRep) { // normal case: old != new
    // Decrement old
    if (dataIfaceRep) // Check for NULL
      if ( --dataIfaceRep->referenceCount == 0 ) 
	delete dataIfaceRep;
    // Assign and increment new
    dataIfaceRep = data_interface.dataIfaceRep;
    if (dataIfaceRep) // Check for NULL
      ++dataIfaceRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataInterface::operator=(DataInterface&)" << std::endl;
  if (dataIfaceRep)
    Cout << "dataIfaceRep referenceCount = " << dataIfaceRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


DataInterface::~DataInterface()
{
  if (dataIfaceRep) { // Check for NULL
    --dataIfaceRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataIfaceRep referenceCount decremented to "
         << dataIfaceRep->referenceCount << std::endl;
#endif
    if (dataIfaceRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataIfaceRep" << std::endl;
#endif
      delete dataIfaceRep;
    }
  }
}

} // namespace Dakota
