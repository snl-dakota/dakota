/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2023
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataModel.hpp"
#include "DataMethod.hpp" // shared scheduling enums
#include "dakota_data_io.hpp"
#include "pecos_global_defs.hpp"

namespace Dakota {

DataModelRep::DataModelRep():
  modelType("simulation"),
//approxPointReuse("none"), // default depends on point import
  hierarchicalTags(false),
  pointsTotal(0), pointsManagement(DEFAULT_POINTS), exportSurrogate(false),
  modelExportPrefix("exported_surrogate"), modelExportFormat(NO_MODEL_FORMAT),
  importSurrogate(false),
  modelImportPrefix("exported_surrogate"), modelImportFormat(NO_MODEL_FORMAT),
  importBuildFormat(TABULAR_ANNOTATED),  importUseVariableLabels(false),
  importBuildActive(false),
//importApproxFormat(TABULAR_ANNOTATED), importApproxActive(false),
  exportApproxFormat(TABULAR_ANNOTATED),
  exportApproxVarianceFormat(TABULAR_ANNOTATED), numRestarts(10),
  approxCorrectionType(NO_CORRECTION), approxCorrectionOrder(0),
  modelUseDerivsFlag(false), respScalingFlag(false), polynomialOrder(2),
  krigingMaxTrials(0), krigingNugget(0.0), krigingFindNugget(0),
  mlsWeightFunction(0), rbfBases(0), rbfMaxPts(0), rbfMaxSubsets(0),
  rbfMinPartition(0), marsMaxBases(0), annRandomWeight(0), annNodes(0),
  annRange(0.), domainDecomp(false), decompCellType("voronoi"),
  decompSupportLayers(0), decompDiscontDetect(false), discontJumpThresh(0.0),
  discontGradThresh(0.0), trendOrder("reduced_quadratic"),
  pointSelection(false), crossValidateFlag(false), numFolds(0), percentFold(0.),
  pressFlag(false), importChallengeFormat(TABULAR_ANNOTATED),
  importChalUseVariableLabels(false), importChallengeActive(false),
  identityRespMap(false),
  subMethodServers(0), subMethodProcs(0), // 0 defaults to detect user spec
  subMethodScheduling(DEFAULT_SCHEDULING), initialSamples(0),
  maxIterations(SZ_MAX), convergenceTolerance(1.e-4), softConvergenceLimit(0),
  subspaceIdBingLi(false), subspaceIdConstantine(false),
  subspaceIdEnergy(false), subspaceIdCV(false), subspaceBuildSurrogate(false),
  subspaceSampleType(SUBMETHOD_DEFAULT), subspaceDimension(0),
  subspaceNormalization(SUBSPACE_NORM_DEFAULT),
  numReplicates(100), relTolerance(1.0e-6),
  decreaseTolerance(1.0e-6), subspaceCVMaxRank(-1), subspaceCVIncremental(true),
  subspaceIdCVMethod(CV_ID_DEFAULT), regressionType(FT_LS),
  regressionL2Penalty(0.), maxSolverIterations(SZ_MAX), maxCrossIterations(1),
  solverTol(1.e-10), solverRoundingTol(1.e-10), statsRoundingTol(1.e-10),
  tensorGridFlag(false), startOrder(2), kickOrder(1), maxOrder(USHRT_MAX),
  adaptOrder(false), startRank(2), kickRank(1),
  maxRank(SZ_MAX), adaptRank(false), maxCVRankCandidates(SZ_MAX),
  maxCVOrderCandidates(USHRT_MAX), c3AdvanceType(NO_C3_ADVANCEMENT),
  collocationPoints(SZ_MAX), collocationRatio(0.), autoRefine(false),
  maxFunctionEvals(SZ_MAX), refineCVMetric("root_mean_squared"),
  refineCVFolds(10), adaptedBasisSparseGridLev(0), adaptedBasisExpOrder(0),
  adaptedBasisCollocRatio(1.), truncationTolerance(1.0e-6),
  analyticCovIdForm(NOCOVAR),
  method_rotation(ROTATION_METHOD_RANKED),
  adaptedBasisTruncationTolerance(0.9)
{ }


void DataModelRep::write(MPIPackBuffer& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << hierarchicalTags << subMethodPointer
    << solutionLevelControl << solutionLevelCost << costRecoveryMetadata
    << surrogateFnIndices << surrogateType << truthModelPointer
    << ensembleModelPointers << pointsTotal << pointsManagement
    << approxPointReuse << importBuildPtsFile << importBuildFormat
    << exportSurrogate << modelExportPrefix << modelExportFormat
    << importSurrogate << modelImportPrefix << modelImportFormat
    << importUseVariableLabels << importBuildActive
  //<< importApproxPtsFile << importApproxFormat << importApproxActive
    << exportApproxPtsFile << exportApproxFormat
    << exportApproxVarianceFile << exportApproxVarianceFormat
    << numRestarts << approxCorrectionType << approxCorrectionOrder
    << modelUseDerivsFlag << respScalingFlag << polynomialOrder
    << krigingCorrelations << krigingOptMethod << krigingMaxTrials
    << krigingMaxCorrelations << krigingMinCorrelations
    << krigingNugget << krigingFindNugget << mlsWeightFunction
    << rbfBases << rbfMaxPts << rbfMaxSubsets << rbfMinPartition
    << marsMaxBases << marsInterpolation << annRandomWeight << annNodes
    << annRange << domainDecomp << decompCellType << decompSupportLayers
    << decompDiscontDetect << discontJumpThresh << discontGradThresh
    << trendOrder << pointSelection << diagMetrics << crossValidateFlag
    << numFolds << percentFold << pressFlag << importChallengePtsFile
    << importChallengeFormat << importChalUseVariableLabels
    << importChallengeActive << advancedOptionsFilename
    << optionalInterfRespPointer << primaryVarMaps << secondaryVarMaps
    << primaryRespCoeffs << secondaryRespCoeffs << identityRespMap
    << subMethodServers << subMethodProcs << subMethodScheduling 
    << initialSamples << refineSamples << maxIterations 
    << convergenceTolerance << softConvergenceLimit << subspaceIdBingLi 
    << subspaceIdConstantine << subspaceIdEnergy << subspaceBuildSurrogate
    << subspaceDimension << subspaceNormalization << numReplicates
    << regressionType << regressionL2Penalty << maxSolverIterations
    << maxCrossIterations << solverTol << solverRoundingTol << statsRoundingTol
    << tensorGridFlag << startOrder << kickOrder << maxOrder << adaptOrder
    << startRank << kickRank << maxRank << adaptRank
    << maxCVRankCandidates << maxCVOrderCandidates
    << c3AdvanceType << collocationPoints << collocationRatio
    << autoRefine << maxFunctionEvals << refineCVMetric << refineCVFolds
    << adaptedBasisSparseGridLev << adaptedBasisExpOrder
    << adaptedBasisCollocRatio << propagationModelPointer << truncationTolerance
    << rfDataFileName << randomFieldIdForm << analyticCovIdForm
    << subspaceSampleType << subspaceIdCV << relTolerance
    << decreaseTolerance << subspaceCVMaxRank << subspaceCVIncremental
    << subspaceIdCVMethod << method_rotation << adaptedBasisTruncationTolerance;
}


void DataModelRep::read(MPIUnpackBuffer& s)
{
  s >> idModel >> modelType >> variablesPointer >> interfacePointer
    >> responsesPointer >> hierarchicalTags >> subMethodPointer
    >> solutionLevelControl >> solutionLevelCost >> costRecoveryMetadata
    >> surrogateFnIndices >> surrogateType >> truthModelPointer
    >> ensembleModelPointers >> pointsTotal >> pointsManagement
    >> approxPointReuse >> importBuildPtsFile >> importBuildFormat
    >> exportSurrogate >> modelExportPrefix >> modelExportFormat
    >> importSurrogate >> modelImportPrefix >> modelImportFormat
    >> importUseVariableLabels >> importBuildActive
  //>> importApproxPtsFile >> importApproxFormat >> importApproxActive
    >> exportApproxPtsFile >> exportApproxFormat
    >> exportApproxVarianceFile >> exportApproxVarianceFormat
    >> numRestarts >> approxCorrectionType >> approxCorrectionOrder
    >> modelUseDerivsFlag >> respScalingFlag >> polynomialOrder
    >> krigingCorrelations >> krigingOptMethod >> krigingMaxTrials
    >> krigingMaxCorrelations >> krigingMinCorrelations
    >> krigingNugget >> krigingFindNugget >> mlsWeightFunction
    >> rbfBases >> rbfMaxPts >> rbfMaxSubsets >> rbfMinPartition
    >> marsMaxBases >> marsInterpolation >> annRandomWeight >> annNodes
    >> annRange >> domainDecomp >> decompCellType >> decompSupportLayers
    >> decompDiscontDetect >> discontJumpThresh >> discontGradThresh
    >> trendOrder >> pointSelection >> diagMetrics >> crossValidateFlag
    >> numFolds >> percentFold >> pressFlag >> importChallengePtsFile
    >> importChallengeFormat >> importChalUseVariableLabels 
    >> importChallengeActive >> advancedOptionsFilename
    >> optionalInterfRespPointer >> primaryVarMaps >> secondaryVarMaps
    >> primaryRespCoeffs >> secondaryRespCoeffs >> identityRespMap
    >> subMethodServers >> subMethodProcs >> subMethodScheduling 
    >> initialSamples >> refineSamples >> maxIterations 
    >> convergenceTolerance >> softConvergenceLimit >> subspaceIdBingLi 
    >> subspaceIdConstantine >> subspaceIdEnergy >> subspaceBuildSurrogate
    >> subspaceDimension >> subspaceNormalization >> numReplicates
    >> regressionType >> regressionL2Penalty >> maxSolverIterations
    >> maxCrossIterations >> solverTol >> solverRoundingTol >> statsRoundingTol
    >> tensorGridFlag >> startOrder >> kickOrder >> maxOrder >> adaptOrder
    >> startRank >> kickRank >> maxRank >> adaptRank
    >> maxCVRankCandidates >> maxCVOrderCandidates
    >> c3AdvanceType >> collocationPoints >> collocationRatio
    >> autoRefine >> maxFunctionEvals >> refineCVMetric >> refineCVFolds
    >> adaptedBasisSparseGridLev >> adaptedBasisExpOrder
    >> adaptedBasisCollocRatio >> propagationModelPointer >> truncationTolerance
    >> rfDataFileName >> randomFieldIdForm >> analyticCovIdForm
    >> subspaceSampleType >> subspaceIdCV >> relTolerance
    >> decreaseTolerance >> subspaceCVMaxRank >> subspaceCVIncremental
    >> subspaceIdCVMethod >> method_rotation >> adaptedBasisTruncationTolerance;
}


void DataModelRep::write(std::ostream& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << hierarchicalTags << subMethodPointer
    << solutionLevelControl << solutionLevelCost << costRecoveryMetadata
    << surrogateFnIndices << surrogateType << truthModelPointer
    << ensembleModelPointers << pointsTotal << pointsManagement
    << approxPointReuse << importBuildPtsFile << importBuildFormat
    << exportSurrogate << modelExportPrefix << modelExportFormat
    << importSurrogate << modelImportPrefix << modelImportFormat
    << importUseVariableLabels << importBuildActive
  //<< importApproxPtsFile << importApproxFormat << importApproxActive
    << exportApproxPtsFile << exportApproxFormat
    << exportApproxVarianceFile << exportApproxVarianceFormat
    << numRestarts << approxCorrectionType << approxCorrectionOrder
    << modelUseDerivsFlag << respScalingFlag << polynomialOrder
    << krigingCorrelations << krigingOptMethod << krigingMaxTrials
    << krigingMaxCorrelations << krigingMinCorrelations
    << krigingNugget << krigingFindNugget << mlsWeightFunction
    << rbfBases << rbfMaxPts << rbfMaxSubsets << rbfMinPartition
    << marsMaxBases << marsInterpolation << annRandomWeight << annNodes
    << annRange << domainDecomp << decompCellType << decompSupportLayers
    << decompDiscontDetect << discontJumpThresh << discontGradThresh
    << trendOrder << pointSelection << diagMetrics << crossValidateFlag
    << numFolds << percentFold << pressFlag << importChallengePtsFile
    << importChallengeFormat << importChalUseVariableLabels
    << importChallengeActive << advancedOptionsFilename
    << optionalInterfRespPointer << primaryVarMaps << secondaryVarMaps
    << primaryRespCoeffs << secondaryRespCoeffs << identityRespMap
    << subMethodServers << subMethodProcs << subMethodScheduling 
    << initialSamples << refineSamples << maxIterations 
    << convergenceTolerance << softConvergenceLimit << subspaceIdBingLi 
    << subspaceIdConstantine << subspaceIdEnergy << subspaceBuildSurrogate
    << subspaceDimension << subspaceNormalization << numReplicates
    << regressionType << regressionL2Penalty << maxSolverIterations
    << maxCrossIterations << solverTol << solverRoundingTol << statsRoundingTol
    << tensorGridFlag << startOrder << kickOrder << maxOrder << adaptOrder
    << startRank << kickRank << maxRank << adaptRank
    << maxCVRankCandidates << maxCVOrderCandidates
    << c3AdvanceType << collocationPoints << collocationRatio
    << autoRefine << maxFunctionEvals << refineCVMetric << refineCVFolds
    << adaptedBasisSparseGridLev << adaptedBasisExpOrder
    << adaptedBasisCollocRatio << propagationModelPointer << truncationTolerance
    << rfDataFileName << randomFieldIdForm << analyticCovIdForm
    << subspaceSampleType << subspaceIdCV << relTolerance
    << decreaseTolerance << subspaceCVMaxRank << subspaceCVIncremental
    << subspaceIdCVMethod << method_rotation << adaptedBasisTruncationTolerance;
}


DataModel::DataModel(): dataModelRep(new DataModelRep())
{ /* empty ctor */ }


DataModel::DataModel(const DataModel& data_model):
  dataModelRep(data_model.dataModelRep)
{ /* empty ctor */ }


DataModel& DataModel::operator=(const DataModel& data_model)
{
  dataModelRep = data_model.dataModelRep;
  return *this;
}


DataModel::~DataModel()
{ /* empty dtor*/ }

} // namespace Dakota
