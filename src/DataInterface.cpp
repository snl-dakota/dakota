/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataInterface.hpp"
#include "DataMethod.hpp" // shared scheduling enums
#include "dakota_data_io.hpp"


namespace Dakota {

DataInterfaceRep::DataInterfaceRep():
  interfaceType(DEFAULT_INTERFACE),
  allowExistingResultsFlag(false), verbatimFlag(false),
  parametersFileFormat(PARAMETERS_FILE_STANDARD), 
  resultsFileFormat(RESULTS_FILE_STANDARD), dakotaResultsFileLabeled(false),
  fileTagFlag(false), fileSaveFlag(false),
  batchEvalFlag(false), asynchFlag(false),
  asynchLocalEvalConcurrency(0), asynchLocalEvalScheduling(DEFAULT_SCHEDULING),
  asynchLocalAnalysisConcurrency(0), evalServers(0),
  evalScheduling(DEFAULT_SCHEDULING), procsPerEval(0), analysisServers(0),
  analysisScheduling(DEFAULT_SCHEDULING), procsPerAnalysis(0),
  failAction("abort"), retryLimit(1), activeSetVectorFlag(true),
  evalCacheFlag(true), nearbyEvalCacheFlag(false),
  nearbyEvalCacheTol(DBL_EPSILON), // default relative tolerance is tight
  restartFileFlag(true), useWorkdir(false), dirTag(false),
  dirSave(false), templateReplace(false), numpyFlag(false)
  // asynchLocal{Eval,Analysis}Concurrency, procsPer{Eval,Analysis} and
  // {eval,analysis}Servers default to zero in order to allow detection of
  // user overrides > 0
{ }


void DataInterfaceRep::write(MPIPackBuffer& s) const
{
  s << idInterface << interfaceType << algebraicMappings << analysisDrivers
    << analysisComponents << inputFilter << outputFilter << parametersFile
    << resultsFile << allowExistingResultsFlag  << verbatimFlag  
    << parametersFileFormat << resultsFileFormat 
    << dakotaResultsFileLabeled << fileTagFlag << fileSaveFlag //<< gridHostNames << gridProcsPerHost
    << batchEvalFlag << asynchFlag << asynchLocalEvalConcurrency
    << asynchLocalEvalScheduling << asynchLocalAnalysisConcurrency
    << evalServers << evalScheduling << procsPerEval << analysisServers
    << analysisScheduling << procsPerAnalysis << failAction << retryLimit
    << recoveryFnVals << activeSetVectorFlag << evalCacheFlag
    << nearbyEvalCacheFlag << nearbyEvalCacheTol << restartFileFlag
    << useWorkdir << workDir << dirTag << dirSave << linkFiles
    << copyFiles << templateReplace << pluginLibraryPath << numpyFlag;
}


void DataInterfaceRep::read(MPIUnpackBuffer& s)
{
  s >> idInterface >> interfaceType >> algebraicMappings >> analysisDrivers
    >> analysisComponents >> inputFilter >> outputFilter >> parametersFile
    >> resultsFile >> allowExistingResultsFlag  >> verbatimFlag  
    >> parametersFileFormat >> resultsFileFormat 
    >> dakotaResultsFileLabeled >> fileTagFlag >> fileSaveFlag //>> gridHostNames >> gridProcsPerHost
    >> batchEvalFlag >> asynchFlag >> asynchLocalEvalConcurrency
    >> asynchLocalEvalScheduling >> asynchLocalAnalysisConcurrency
    >> evalServers >> evalScheduling >> procsPerEval >> analysisServers
    >> analysisScheduling >> procsPerAnalysis >> failAction >> retryLimit
    >> recoveryFnVals >> activeSetVectorFlag >> evalCacheFlag
    >> nearbyEvalCacheFlag >> nearbyEvalCacheTol >> restartFileFlag
    >> useWorkdir >> workDir >> dirTag >> dirSave >> linkFiles
    >> copyFiles >> templateReplace >> pluginLibraryPath >> numpyFlag;
}


void DataInterfaceRep::write(std::ostream& s) const
{
  s << idInterface << interfaceType << algebraicMappings << analysisDrivers
    << analysisComponents << inputFilter << outputFilter << parametersFile
    << resultsFile << allowExistingResultsFlag  << verbatimFlag  
    << parametersFileFormat << resultsFileFormat 
    << dakotaResultsFileLabeled << fileTagFlag << fileSaveFlag //<< gridHostNames << gridProcsPerHost
    << batchEvalFlag << asynchFlag << asynchLocalEvalConcurrency
    << asynchLocalEvalScheduling << asynchLocalAnalysisConcurrency
    << evalServers << evalScheduling << procsPerEval << analysisServers
    << analysisScheduling << procsPerAnalysis << failAction << retryLimit
    << recoveryFnVals << activeSetVectorFlag << evalCacheFlag
    << nearbyEvalCacheFlag << nearbyEvalCacheTol << restartFileFlag
    << useWorkdir << workDir << dirTag << dirSave << linkFiles
    << copyFiles << templateReplace << pluginLibraryPath << numpyFlag;
}


DataInterface::DataInterface(): dataIfaceRep(new DataInterfaceRep())
{ /* empty ctor */ }


DataInterface::DataInterface(const DataInterface& data_resp):
  dataIfaceRep(data_resp.dataIfaceRep)
{ /* empty ctor */ }


DataInterface& DataInterface::operator=(const DataInterface& data_interface)
{
  dataIfaceRep = data_interface.dataIfaceRep;
  return *this;
}


DataInterface::~DataInterface()
{ /* empty dtor */ }

} // namespace Dakota
