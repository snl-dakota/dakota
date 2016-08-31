/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataModel
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataModel.hpp"
#include "DataMethod.hpp" // shared scheduling enums
#include "dakota_data_io.hpp"


namespace Dakota {

DataModelRep::DataModelRep():
  modelType("simulation"),
//approxPointReuse("none"), // default depends on point import
  hierarchicalTags(false),
  pointsTotal(0), pointsManagement(DEFAULT_POINTS), exportSurrogate(false),
  modelExportPrefix("exported_surrogate"), modelExportFormat(NO_MODEL_FORMAT),
  importBuildFormat(TABULAR_ANNOTATED),  importBuildActive(false),
//importApproxFormat(TABULAR_ANNOTATED), importApproxActive(false),
  exportApproxFormat(TABULAR_ANNOTATED),
  approxCorrectionType(NO_CORRECTION), approxCorrectionOrder(0),
  modelUseDerivsFlag(false), polynomialOrder(2), krigingMaxTrials(0),
  krigingNugget(0.0), krigingFindNugget(0), mlsWeightFunction(0),
  rbfBases(0), rbfMaxPts(0), rbfMaxSubsets(0), rbfMinPartition(0),
  marsMaxBases(0), annRandomWeight(0), annNodes(0), annRange(0.0), 
  domainDecomp(false), decompCellType("voronoi"), decompSupportLayers(0),
  decompDiscontDetect(false), discontJumpThresh(0.0), discontGradThresh(0.0),
  trendOrder("reduced_quadratic"), pointSelection(false),
  crossValidateFlag(false), numFolds(0), percentFold(0.0), pressFlag(false),
  importChallengeFormat(TABULAR_ANNOTATED), importChallengeActive(false),
  subMethodServers(0), subMethodProcs(0), // 0 defaults to detect user spec
  subMethodScheduling(DEFAULT_SCHEDULING), initialSamples(0),
  maxIterations(100), convergenceTolerance(1.0e-4), softConvergenceLimit(0),
  subspaceIdBingLi(false), subspaceIdConstantine(false),
  subspaceIdEnergy(false), subspaceIdCV(false), subspaceBuildSurrogate(false),
  subspaceSampleType(SUBMETHOD_DEFAULT), subspaceDimension(0),
  subspaceNormalization(SUBSPACE_NORM_DEFAULT),
  numReplicates(100), autoRefine(false), maxFunctionEvals(1000),
  refineCVMetric("root_mean_squared"), refineCVFolds(10),
  adaptedBasisSparseGridLev(0), adaptedBasisExpOrder(0),
  adaptedBasisCollocRatio(1.), truncationTolerance(1.0e-6),
  analyticCovIdForm(NOCOVAR), referenceCount(1), relTolerance(1.0e-6),
  decreaseTolerance(1.0e-6), subspaceCVMaxRank(-1), subspaceCVIncremental(true),
  subspaceIdCVMethod(CV_ID_DEFAULT)
{ }


void DataModelRep::write(MPIPackBuffer& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << hierarchicalTags << subMethodPointer
    << solutionLevelControl << solutionLevelCost << surrogateFnIndices
    << surrogateType << actualModelPointer << orderedModelPointers
    << pointsTotal << pointsManagement << approxPointReuse
    << importBuildPtsFile << importBuildFormat << exportSurrogate
    << modelExportPrefix << modelExportFormat << importBuildActive
  //<< importApproxPtsFile << importApproxFormat << importApproxActive
    << exportApproxPtsFile << exportApproxFormat 
    << approxCorrectionType << approxCorrectionOrder << modelUseDerivsFlag
    << polynomialOrder << krigingCorrelations << krigingOptMethod
    << krigingMaxTrials << krigingMaxCorrelations << krigingMinCorrelations
    << krigingNugget << krigingFindNugget << mlsWeightFunction
    << rbfBases << rbfMaxPts << rbfMaxSubsets << rbfMinPartition
    << marsMaxBases << marsInterpolation << annRandomWeight << annNodes
    << annRange << domainDecomp << decompCellType << decompSupportLayers
    << decompDiscontDetect << discontJumpThresh << discontGradThresh
    << trendOrder << pointSelection << diagMetrics << crossValidateFlag
    << numFolds << percentFold << pressFlag << importChallengePtsFile
    << importChallengeFormat << importChallengeActive
    << optionalInterfRespPointer << primaryVarMaps << secondaryVarMaps
    << primaryRespCoeffs << secondaryRespCoeffs << subMethodServers
    << subMethodProcs << subMethodScheduling 
    << initialSamples << refineSamples << maxIterations 
    << convergenceTolerance << softConvergenceLimit << subspaceIdBingLi 
    << subspaceIdConstantine << subspaceIdEnergy << subspaceBuildSurrogate
    << subspaceDimension << subspaceNormalization << numReplicates << autoRefine
    << maxFunctionEvals << refineCVMetric << refineCVFolds
    << adaptedBasisSparseGridLev << adaptedBasisExpOrder
    << adaptedBasisCollocRatio << propagationModelPointer << truncationTolerance
    << rfDataFileName << randomFieldIdForm << analyticCovIdForm
    << subspaceSampleType << subspaceIdCV << relTolerance
    << decreaseTolerance << subspaceCVMaxRank << subspaceCVIncremental
    << subspaceIdCVMethod;
}


void DataModelRep::read(MPIUnpackBuffer& s)
{
  s >> idModel >> modelType >> variablesPointer >> interfacePointer
    >> responsesPointer >> hierarchicalTags >> subMethodPointer 
    >> solutionLevelControl >> solutionLevelCost >> surrogateFnIndices
    >> surrogateType >> actualModelPointer >> orderedModelPointers
    >> pointsTotal >> pointsManagement >> approxPointReuse
    >> importBuildPtsFile >> importBuildFormat >> exportSurrogate
    >> modelExportPrefix >> modelExportFormat >> importBuildActive
  //>> importApproxPtsFile >> importApproxFormat >> importApproxActive
    >> exportApproxPtsFile >> exportApproxFormat 
    >> approxCorrectionType >> approxCorrectionOrder >> modelUseDerivsFlag
    >> polynomialOrder >> krigingCorrelations >> krigingOptMethod
    >> krigingMaxTrials >> krigingMaxCorrelations >> krigingMinCorrelations
    >> krigingNugget >> krigingFindNugget >> mlsWeightFunction
    >> rbfBases >> rbfMaxPts >> rbfMaxSubsets >> rbfMinPartition
    >> marsMaxBases >> marsInterpolation >> annRandomWeight >> annNodes
    >> annRange >> domainDecomp >> decompCellType >> decompSupportLayers
    >> decompDiscontDetect >> discontJumpThresh >> discontGradThresh
    >> trendOrder >> pointSelection >> diagMetrics >> crossValidateFlag
    >> numFolds >> percentFold >> pressFlag >> importChallengePtsFile
    >> importChallengeFormat >> importChallengeActive
    >> optionalInterfRespPointer >> primaryVarMaps >> secondaryVarMaps
    >> primaryRespCoeffs >> secondaryRespCoeffs >> subMethodServers
    >> subMethodProcs >> subMethodScheduling     
    >> initialSamples >> refineSamples >> maxIterations 
    >> convergenceTolerance >> softConvergenceLimit >> subspaceIdBingLi 
    >> subspaceIdConstantine >> subspaceIdEnergy >> subspaceBuildSurrogate
    >> subspaceDimension >> subspaceNormalization >> numReplicates >> autoRefine
    >> maxFunctionEvals >> refineCVMetric >> refineCVFolds
    >> adaptedBasisSparseGridLev >> adaptedBasisExpOrder
    >> adaptedBasisCollocRatio >> propagationModelPointer >> truncationTolerance
    >> rfDataFileName >> randomFieldIdForm >> analyticCovIdForm
    >> subspaceSampleType >> subspaceIdCV >> relTolerance
    >> decreaseTolerance >> subspaceCVMaxRank >> subspaceCVIncremental
    >> subspaceIdCVMethod;
}


void DataModelRep::write(std::ostream& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << hierarchicalTags << subMethodPointer 
    << solutionLevelControl << solutionLevelCost << surrogateFnIndices
    << surrogateType << actualModelPointer << orderedModelPointers
    << pointsTotal << pointsManagement << approxPointReuse
    << importBuildPtsFile << importBuildFormat << exportSurrogate
    << modelExportPrefix << modelExportFormat << importBuildActive
  //<< importApproxPtsFile << importApproxFormat << importApproxActive
    << exportApproxPtsFile << exportApproxFormat 
    << approxCorrectionType << approxCorrectionOrder << modelUseDerivsFlag
    << polynomialOrder << krigingCorrelations << krigingOptMethod
    << krigingMaxTrials << krigingMaxCorrelations << krigingMinCorrelations
    << krigingNugget << krigingFindNugget << mlsWeightFunction
    << rbfBases << rbfMaxPts << rbfMaxSubsets << rbfMinPartition
    << marsMaxBases << marsInterpolation << annRandomWeight << annNodes
    << annRange << domainDecomp << decompCellType << decompSupportLayers
    << decompDiscontDetect << discontJumpThresh << discontGradThresh
    << trendOrder << pointSelection << diagMetrics << crossValidateFlag
    << numFolds << percentFold << pressFlag << importChallengePtsFile
    << importChallengeFormat << importChallengeActive
    << optionalInterfRespPointer << primaryVarMaps << secondaryVarMaps
    << primaryRespCoeffs << secondaryRespCoeffs << subMethodServers
    << subMethodProcs << subMethodScheduling 
    << initialSamples << refineSamples << maxIterations 
    << convergenceTolerance << subspaceIdBingLi << subspaceIdConstantine
    << subspaceIdEnergy << subspaceBuildSurrogate
    << subspaceDimension << subspaceNormalization << numReplicates << autoRefine
    << maxFunctionEvals << refineCVMetric << refineCVFolds
    << adaptedBasisSparseGridLev << adaptedBasisExpOrder
    << adaptedBasisCollocRatio << propagationModelPointer << truncationTolerance
    << rfDataFileName << randomFieldIdForm << analyticCovIdForm
    << subspaceSampleType << subspaceIdCV << relTolerance
    << decreaseTolerance << subspaceCVMaxRank << subspaceCVIncremental
    << subspaceIdCVMethod;
}


DataModel::DataModel(): dataModelRep(new DataModelRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataModel::DataModel(), dataModelRep referenceCount = "
       << dataModelRep->referenceCount << std::endl;
#endif
}


DataModel::DataModel(const DataModel& data_model)
{
  // Increment new (no old to decrement)
  dataModelRep = data_model.dataModelRep;
  if (dataModelRep) // Check for an assignment of NULL
    ++dataModelRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "DataModel::DataModel(DataModel&)" << std::endl;
  if (dataModelRep)
    Cout << "dataModelRep referenceCount = " << dataModelRep->referenceCount
	 << std::endl;
#endif
}


DataModel& DataModel::operator=(const DataModel& data_model)
{
  if (dataModelRep != data_model.dataModelRep) { // normal case: old != new
    // Decrement old
    if (dataModelRep) // Check for NULL
      if ( --dataModelRep->referenceCount == 0 ) 
	delete dataModelRep;
    // Assign and increment new
    dataModelRep = data_model.dataModelRep;
    if (dataModelRep) // Check for NULL
      ++dataModelRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataModel::operator=(DataModel&)" << std::endl;
  if (dataModelRep)
    Cout << "dataModelRep referenceCount = " << dataModelRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


DataModel::~DataModel()
{
  if (dataModelRep) { // Check for NULL
    --dataModelRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataModelRep referenceCount decremented to "
         << dataModelRep->referenceCount << std::endl;
#endif
    if (dataModelRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataModelRep" << std::endl;
#endif
      delete dataModelRep;
    }
  }
}

} // namespace Dakota
