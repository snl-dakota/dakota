/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataInterface
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataInterface.H"
#include "data_io.h"


namespace Dakota {

DataInterfaceRep::DataInterfaceRep():
  allowExistingResultsFlag(false), verbatimFlag(false), apreproFlag(false),
  fileTagFlag(false), fileSaveFlag(false), procsPerAnalysis(0),
  interfaceSynchronization("synchronous"), asynchLocalEvalConcurrency(0),
  asynchLocalAnalysisConcurrency(0), evalServers(0), analysisServers(0),
  failAction("abort"), retryLimit(1), activeSetVectorFlag(true),
  evalCacheFlag(true), restartFileFlag(true), referenceCount(1),
  useWorkdir(false), dirTag(false), dirSave(false), templateCopy(false),
  templateReplace(false)
{ }


void DataInterfaceRep::write(MPIPackBuffer& s) const
{
  s << idInterface << interfaceType << algebraicMappings << analysisDrivers
    << analysisComponents << inputFilter << outputFilter << parametersFile
    << resultsFile << allowExistingResultsFlag  << verbatimFlag << apreproFlag 
    << fileTagFlag << fileSaveFlag
    << procsPerAnalysis //<< gridHostNames << gridProcsPerHost
    << interfaceSynchronization << asynchLocalEvalConcurrency
    << asynchLocalEvalScheduling
    << asynchLocalAnalysisConcurrency << evalServers << evalScheduling
    << analysisServers << analysisScheduling << failAction << retryLimit
    << recoveryFnVals << activeSetVectorFlag << evalCacheFlag
    << restartFileFlag << useWorkdir << workDir << dirTag << dirSave
    << templateDir << templateFiles << templateCopy << templateReplace;
}


void DataInterfaceRep::read(MPIUnpackBuffer& s)
{
  s >> idInterface >> interfaceType >> algebraicMappings >> analysisDrivers
    >> analysisComponents >> inputFilter >> outputFilter >> parametersFile
    >> resultsFile >> allowExistingResultsFlag >> verbatimFlag >> apreproFlag 
    >> fileTagFlag >> fileSaveFlag
    >> procsPerAnalysis //>> gridHostNames >> gridProcsPerHost
    >> interfaceSynchronization >> asynchLocalEvalConcurrency
    >> asynchLocalEvalScheduling
    >> asynchLocalAnalysisConcurrency >> evalServers >> evalScheduling
    >> analysisServers >> analysisScheduling >> failAction >> retryLimit
    >> recoveryFnVals >> activeSetVectorFlag >> evalCacheFlag
    >> restartFileFlag >> useWorkdir >> workDir >> dirTag >> dirSave
    >> templateDir >> templateFiles >> templateCopy >> templateReplace;
}


void DataInterfaceRep::write(std::ostream& s) const
{
  s << idInterface << interfaceType << algebraicMappings << analysisDrivers
    << analysisComponents << inputFilter << outputFilter << parametersFile
    << resultsFile << allowExistingResultsFlag << verbatimFlag << apreproFlag 
    << fileTagFlag << fileSaveFlag
    << procsPerAnalysis //<< gridHostNames << gridProcsPerHost
    << interfaceSynchronization << asynchLocalEvalConcurrency
    << asynchLocalEvalScheduling
    << asynchLocalAnalysisConcurrency << evalServers << evalScheduling
    << analysisServers << analysisScheduling << failAction << retryLimit
    << recoveryFnVals << activeSetVectorFlag << evalCacheFlag
    << restartFileFlag << useWorkdir << workDir << dirTag << dirSave
    << templateDir << templateFiles << templateCopy << templateReplace;
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
    dataIfaceRep->referenceCount++;

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
      dataIfaceRep->referenceCount++;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataInterface::operator=(DataInterface&)" << std::endl;
  if (dataIFaceRep)
    Cout << "dataIFaceRep referenceCount = " << dataIFaceRep->referenceCount
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
