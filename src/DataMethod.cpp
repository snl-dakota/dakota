/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataMethod.hpp"
#include "DataModel.hpp" // for C3 FT enumerations
#include "dakota_data_io.hpp"
#include "pecos_global_defs.hpp"
#ifdef HAVE_OPTPP
#include "globals.h"
#endif

namespace Dakota {

DataMethodRep::DataMethodRep():
  methodOutput(NORMAL_OUTPUT), maxIterations(SZ_MAX),
  maxRefineIterations(SZ_MAX), maxSolverIterations(SZ_MAX),
  maxFunctionEvals(SZ_MAX), speculativeFlag(false), methodUseDerivsFlag(false),
  constraintTolerance(0.), methodScaling(false), numFinalSolutions(0),
  convergenceTolerance(-std::numeric_limits<double>::max()),
  relativeConvMetric(true), statsMetricMode(Pecos::DEFAULT_EXPANSION_STATS),
  methodName(DEFAULT_METHOD), subMethod(SUBMETHOD_DEFAULT),
  // Meta-iterators
  iteratorServers(0), procsPerIterator(0), // 0 defaults to detect user spec
  iteratorScheduling(DEFAULT_SCHEDULING), hybridLSProb(0.1),
  //hybridProgThresh(0.5),
  concurrentRandomJobs(0),
  // Local surrogate-based opt/NLS
  softConvLimit(0), // dummy value -> method-specific default
  surrBasedLocalLayerBypass(false), //trustRegionInitSize(0.5),
  trustRegionMinSize(1.e-6),        trustRegionContractTrigger(0.25),
  trustRegionExpandTrigger(0.75),   trustRegionContract(0.25),
  trustRegionExpand(2.0), surrBasedLocalSubProbObj(ORIGINAL_PRIMARY),
  surrBasedLocalSubProbCon(ORIGINAL_CONSTRAINTS),
  surrBasedLocalMeritFn(AUGMENTED_LAGRANGIAN_MERIT),
  surrBasedLocalAcceptLogic(FILTER), surrBasedLocalConstrRelax(NO_RELAX),
  // Global surrogate-based opt/NLS
  surrBasedGlobalReplacePts(false),
  // Branch and bound
  //branchBndNumSamplesRoot(0), branchBndNumSamplesNode(0),
  // DL_SOLVER
  dlLib(NULL), //dlDetails(""),
  // NPSOL
  verifyLevel(-1), functionPrecision(1.e-10), lineSearchTolerance(0.9),
  // NL2SOL: Real values of -1. ==> use NL2SOL default
  absConvTol(-1.), xConvTol(-1.), singConvTol(-1.), singRadius(-1.),
  falseConvTol(-1.), initTRRadius(-1.), covarianceType(0), regressDiag(false),
  // OPT++
  // searchMethod default is null since "trust_region" is preferred for
  // unconstrained opt., whereas "line_search" is preferred for bc opt.
  gradientTolerance(0.0001), maxStep(1.e+3),
#if HAVE_OPTPP
  meritFn(OPTPP::ArgaezTapia),
#else
  meritFn(NULL),
#endif
  stepLenToBoundary(-1.), centeringParam(-1.), // dummy defaults (see SNLLBase)
  searchSchemeSize(32),
  // APPSPACK
  initStepLength(1.0), contractStepLength(0.5), threshStepLength(0.01),
  meritFunction("merit2_squared"), constrPenalty(1.0), smoothFactor(0.0),
  // COLINY
  constantPenalty(false), globalBalanceParam(-1.),
  localBalanceParam(-1.), maxBoxSize(-1.), minBoxSize(-1.),
  //boxDivision("major_dimension"), // leave empty string as default
  mutationAdaptive(true), showMiscOptions(false),
  // These attributes must replicate the Coliny defaults due to Coliny
  // member fn. structure:
  mutationRate(1.0),
  mutationScale(0.1), contractFactor(0.5),
  // These attributes replicate COLINY defaults due to convenience:
  totalPatternSize(0), // simplifies maxEvalConcurrency calculation
  // These attributes define DAKOTA defaults which may differ from Coliny
  // defaults:
  solnTarget(-DBL_MAX), // COLINY default of 1.e-5 can cause premature term.
  // These attributes use dummy defaults which are used to trigger conditional
  // option processing (since we don't want to get out of synch with SGOPT's
  // defaults which may change).  The dummy defaults should be values which
  // are _not_ reasonable user inputs.
  mutationMinScale(-1.), initDelta(-1.), threshDelta(-1.),
  newSolnsGenerated(-9999), numberRetained(-9999),
  expansionFlag(true), // default = on, no_expansion spec turns off
  expandAfterSuccess(0), contractAfterFail(0), mutationRange(-9999),
  randomizeOrderFlag(false), //betaSolverName(""),
  // COLINY + APPSPACK
  evalSynchronize(DEFAULT_SYNCHRONIZATION),
  // JEGA
  numCrossPoints(2), numParents(2), numOffspring(2), //convergenceType(""),
  percentChange(0.1), numGenerations(15), fitnessLimit(6.0),
  shrinkagePercent(0.9), nichingType("null_niching"), numDesigns(100),
  postProcessorType("null_postprocessor"), logFile("JEGAGlobal.log"),
  printPopFlag(false),
  // JEGA/COLINY
  constraintPenalty(-1.), crossoverRate(-1.), //crossoverType(""),
  initializationType("unique_random"),
  //mutationType(""), replacementType(""), fitnessType(""),
  populationSize(50), //flatFile(),
  // NCSU
  volBoxSize(-1.),
  // DDACE
  numSymbols(0), mainEffectsFlag(false),
  // FSUDace
  latinizeFlag(false), volQualityFlag(false), numTrials(10000),
  //initializationType("grid"), trialType("random"),
  // COLINY, JEGA, NonD, & DACE
  randomSeed(0),
  // NOMAD
  initMeshSize(1.0), minMeshSize(1.e-6), historyFile("mads_history"),
  displayFormat("bbe obj"), vns(0.0), neighborOrder(1), showAllEval(false),
  useSurrogate("none"),
  // C3 FT
  maxCrossIterations(1), solverTol(1.e-10), solverRoundingTol(1.e-10),
  statsRoundingTol(1.e-10), startOrder(2), kickOrder(1), maxOrder(USHRT_MAX),
  adaptOrder(false), startRank(2), kickRank(1), maxRank(SZ_MAX),
  adaptRank(false), maxCVRankCandidates(SZ_MAX),
  maxCVOrderCandidates(USHRT_MAX), c3AdvanceType(NO_C3_ADVANCEMENT),
  // NonD & DACE
  numSamples(0), fixedSeedFlag(false),
  fixedSequenceFlag(false), //default is variable sampling patterns
  vbdFlag(false),vbdDropTolerance(-1.),
  vbdViaSamplingMethod(VBD_PICK_AND_FREEZE),vbdViaSamplingNumBins(-1),
  backfillFlag(false), pcaFlag(false),
  percentVarianceExplained(0.95), wilksFlag(false), wilksOrder(1),
  wilksConfidenceLevel(0.95), wilksSidedInterval(ONE_SIDED_UPPER),
  // NonD
  toleranceIntervalsFlag(false), tiCoverage(0.95), tiConfidenceLevel(0.90),
  stdRegressionCoeffs(false),
  respScalingFlag(false), vbdOrder(0), covarianceControl(DEFAULT_COVARIANCE),
  rngName("mt19937"), refinementType(Pecos::NO_REFINEMENT),
  refinementControl(Pecos::NO_CONTROL),
  nestingOverride(Pecos::NO_NESTING_OVERRIDE),
  growthOverride(Pecos::NO_GROWTH_OVERRIDE), expansionType(EXTENDED_U),
  piecewiseBasis(false), expansionBasisType(Pecos::DEFAULT_BASIS),
  quadratureOrder(USHRT_MAX), sparseGridLevel(USHRT_MAX),
  expansionOrder(USHRT_MAX), collocationPoints(SZ_MAX),
  expansionSamples(SZ_MAX),  ensemblePilotSolnMode(ONLINE_PILOT),
  pilotGroupSampling(SHARED_PILOT), groupThrottleType(NO_GROUP_THROTTLE),
  groupSizeThrottle(USHRT_MAX), rCondBestThrottle(SZ_MAX),
  rCondTolThrottle(DBL_MAX), truthPilotConstraint(false),
  dagRecursionType(NO_GRAPH_RECURSION), dagDepthLimit(USHRT_MAX),
  modelSelectType(NO_MODEL_SELECTION), relaxFixedFactor(0.),
  relaxRecursiveFactor(0.), allocationTarget(TARGET_MEAN),
  useTargetVarianceOptimizationFlag(false), qoiAggregation(QOI_AGGREGATION_SUM),
  convergenceToleranceType(CONVERGENCE_TOLERANCE_TYPE_RELATIVE),
  convergenceToleranceTarget(CONVERGENCE_TOLERANCE_TARGET_VARIANCE_CONSTRAINT),
  //expansionSampleType("lhs"),
  cubIntOrder(USHRT_MAX), collocationRatio(0.), collocRatioTermsOrder(1.),
  regressionType(Pecos::DEFAULT_REGRESSION), lsRegressionType(DEFAULT_LS),
  regressionL2Penalty(0.), crossValidation(false), crossValidNoiseOnly(false),
  //adaptedBasisInitLevel(0),
  adaptedBasisAdvancements(3), normalizedCoeffs(false), tensorGridFlag(false),
  sampleType(SUBMETHOD_DEFAULT), rank1LatticeFlag(false),
  noRandomShiftFlag(false), log2MaxPoints(0), kuo(false),
  cools_kuo_nuyens(false), naturalOrdering(false),
  radicalInverseOrdering(false), digitalNetFlag(false),
  noDigitalShiftFlag(false), noScramblingFlag(false),
  mostSignificantBitFirst(false), leastSignificantBitFirst(false),
  numberOfBits(0), scrambleSize(64), joe_kuo(false), sobol_order_2(false), 
  grayCodeOrdering(false), dOptimal(false), numCandidateDesigns(0),
  //reliabilitySearchType(MV),
  integrationRefine(NO_INT_REFINE), optSubProbSolver(SUBMETHOD_DEFAULT),
  numericalSolveMode(NUMERICAL_FALLBACK),
  estVarMetricType(DEFAULT_ESTVAR_METRIC), estVarMetricNormOrder(2.),
  multilevAllocControl(DEFAULT_MLMF_CONTROL), multilevEstimatorRate(2.),
  multilevDiscrepEmulation(DEFAULT_EMULATION),
  finalStatsType(DEFAULT_FINAL_STATS),
  finalMomentsType(Pecos::STANDARD_MOMENTS),
  distributionType(CUMULATIVE), responseLevelTarget(PROBABILITIES),
  responseLevelTargetReduce(COMPONENT), chainSamples(0), buildSamples(0),
  samplesOnEmulator(0), emulatorOrder(0), emulatorType(NO_EMULATOR),
  mcmcType("dram"), standardizedSpace(false), adaptPosteriorRefine(false),
  logitTransform(false), gpmsaNormalize(false), posteriorStatsKL(false),
  posteriorStatsMutual(false), posteriorStatsKDE(false),
  chainDiagnostics(false), chainDiagnosticsCI(false), modelEvidence(false),
  modelEvidMC(false), modelEvidLaplace(false), priorPropCovMult(1.0),
  proposalCovUpdatePeriod(std::numeric_limits<int>::max()),
  fitnessMetricType("predicted_variance"), batchSelectionType("naive"),
  lipschitzType("local"), calibrateErrorMode(CALIBRATE_NONE),
  burnInSamples(0), subSamplingPeriod(1), calModelDiscrepancy(false),
  numPredConfigs(0), importPredConfigFormat(TABULAR_ANNOTATED),
  modelDiscrepancyType("global_kriging"), polynomialOrder(2),
  exportCorrModelFormat(TABULAR_ANNOTATED),
  exportCorrVarFormat(TABULAR_ANNOTATED),
  exportDiscrepFormat(TABULAR_ANNOTATED), adaptExpDesign(false),
  mutualInfoKSG2(false), importCandFormat(TABULAR_ANNOTATED),
  numCandidates(0), maxHifiEvals(-1), batchSize(1), batchSizeExplore(0),
  // DREAM
  numChains(3), numCR(3), crossoverChainPairs(3), grThreshold(1.2),
  jumpStep(5),
  generatePosteriorSamples(false), evaluatePosteriorDensity(false),
  // Wasabi
  numPushforwardSamples(10000),
  // MUQ
  drNumStages(3),
  drScaleType("Power"),
  drScale(2.0),
  amPeriodNumSteps(100),
  amStartingStep(100),
  amScale(1.0),
  malaStepSize(1.0),
  diliHessianType("GaussNewton"),
  diliAdaptInterval(-1),
  diliAdaptStart(1),
  diliAdaptEnd(-1),
  diliInitialWeight(100),
  diliHessTolerance(1.0e-4),
  diliLISTolerance(0.1),
  diliSesNumEigs(2),
  diliSesRelTol(0.001),
  diliSesAbsTol(0.0),
  diliSesExpRank(2),
  diliSesOversFactor(2),
  diliSesBlockSize(2),
  // Parameter Study
  numSteps(0), pstudyFileFormat(TABULAR_ANNOTATED), pstudyFileActive(false),
  // Verification
  refinementRate(2.),
  // Point import/export files
  importBuildFormat(TABULAR_ANNOTATED),   importBuildActive(false),
  importApproxFormat(TABULAR_ANNOTATED),  importApproxActive(false),
  exportApproxFormat(TABULAR_ANNOTATED),
  exportSampleSeqFlag(false), exportSamplesFormat(TABULAR_ANNOTATED),
  exportSurrogate(false), modelExportPrefix("exported_surrogate"),
  modelExportFormat(NO_MODEL_FORMAT)
{ }


void DataMethodRep::write(MPIPackBuffer& s) const
{
  s << idMethod << modelPointer << lowFidModelPointer << methodOutput
    << maxIterations << maxRefineIterations << maxSolverIterations
    << maxFunctionEvals << speculativeFlag << methodUseDerivsFlag
    << constraintTolerance << methodScaling << numFinalSolutions
    << convergenceTolerance << relativeConvMetric << statsMetricMode
    << methodName << subMethod << subMethodName << subModelPointer
    << subMethodPointer;

  // Meta-iterators
  s << iteratorServers << procsPerIterator << iteratorScheduling
    << hybridMethodNames << hybridModelPointers << hybridMethodPointers
  //<< hybridProgThresh
    << hybridGlobalMethodName << hybridGlobalModelPointer
    << hybridGlobalMethodPointer << hybridLocalMethodName
    << hybridLocalModelPointer << hybridLocalMethodPointer << hybridLSProb
  //<< branchBndNumSamplesRoot << branchBndNumSamplesNode
    << concurrentRandomJobs << concurrentParameterSets;

  // Surrogate-based
  s << softConvLimit << surrBasedLocalLayerBypass
    << trustRegionInitSize << trustRegionMinSize
    << trustRegionContractTrigger << trustRegionExpandTrigger
    << trustRegionContract << trustRegionExpand
    << surrBasedLocalSubProbObj << surrBasedLocalSubProbCon
    << surrBasedLocalMeritFn << surrBasedLocalAcceptLogic
    << surrBasedLocalConstrRelax << surrBasedGlobalReplacePts;

  // DL_SOLVER
  s << dlDetails;

  // NPSOL
  s << verifyLevel << functionPrecision << lineSearchTolerance;

  // NL2SOL
  s << absConvTol << xConvTol << singConvTol << singRadius << falseConvTol
    << initTRRadius << covarianceType << regressDiag;

  // OPT++
  s << searchMethod << gradientTolerance << maxStep << meritFn
    << stepLenToBoundary << centeringParam << searchSchemeSize;

  // APPSPACK
  s << initStepLength << contractStepLength << threshStepLength << meritFunction
    << constrPenalty << smoothFactor;

  // COLINY
  s << constraintPenalty << constantPenalty << globalBalanceParam
    << localBalanceParam << maxBoxSize << minBoxSize << boxDivision
    << mutationAdaptive << showMiscOptions << miscOptions << solnTarget
    << crossoverRate << mutationRate << mutationScale << mutationMinScale
    << initDelta << threshDelta << contractFactor << newSolnsGenerated
    << numberRetained << expansionFlag << expandAfterSuccess
    << contractAfterFail << mutationRange << totalPatternSize
    << randomizeOrderFlag << selectionPressure << replacementType
    << crossoverType << mutationType << exploratoryMoves << patternBasis
    << betaSolverName;

  // COLINY + APPSPACK
  s << evalSynchronize;

  // JEGA
  s << numCrossPoints << numParents << numOffspring << fitnessType
    << convergenceType << percentChange << numGenerations << fitnessLimit
    << shrinkagePercent << nichingType << nicheVector << numDesigns
    << postProcessorType << distanceVector;

  // JEGA/COLINY
  s << initializationType << flatFile << logFile << populationSize
    << printPopFlag;

  // NCSU
  s << volBoxSize;

  // DDACE
  s << numSymbols << mainEffectsFlag;

  // FSUDace
  s << latinizeFlag << volQualityFlag << sequenceStart << sequenceLeap
    << primeBase << numTrials << trialType;

  // COLINY, NonD, DACE, & JEGA
  s << randomSeed << randomSeedSeq;

  // MADS
  s << initMeshSize << minMeshSize << historyFile << displayFormat << vns
    << neighborOrder << showAllEval << useSurrogate;

  // C3 FT
  s << maxCrossIterations << solverTol << solverRoundingTol << statsRoundingTol
    << startOrder << kickOrder << maxOrder << adaptOrder
    << startRank  << kickRank  << maxRank  << adaptRank << maxCVRankCandidates
    << maxCVOrderCandidates << c3AdvanceType << startOrderSeq << startRankSeq;

  // NonD & DACE
  s << numSamples << fixedSeedFlag << fixedSequenceFlag
    << vbdFlag << vbdDropTolerance
    << vbdViaSamplingMethod << vbdViaSamplingNumBins
    << backfillFlag << pcaFlag
    << percentVarianceExplained << wilksFlag << wilksOrder
    << wilksConfidenceLevel << wilksSidedInterval;

  // NonD
  s << toleranceIntervalsFlag << tiCoverage << tiConfidenceLevel
    << stdRegressionCoeffs << respScalingFlag << vbdOrder << covarianceControl
    << rngName << refinementType << refinementControl << nestingOverride
    << growthOverride << expansionType << piecewiseBasis << expansionBasisType
    << quadratureOrderSeq << sparseGridLevelSeq << expansionOrderSeq
    << collocationPointsSeq << expansionSamplesSeq << quadratureOrder
    << sparseGridLevel << expansionOrder << collocationPoints
    << expansionSamples //<< expansionSampleType
    << anisoDimPref << cubIntOrder << collocationRatio
    << collocRatioTermsOrder << regressionType << lsRegressionType
    << regressionNoiseTol << regressionL2Penalty << crossValidation
    << crossValidNoiseOnly //<< adaptedBasisInitLevel
    << adaptedBasisAdvancements << normalizedCoeffs << pointReuse
    << tensorGridFlag << tensorGridOrder
    << importExpansionFile << exportExpansionFile << sampleType
    << rank1LatticeFlag <<  noRandomShiftFlag << log2MaxPoints << kuo 
    << cools_kuo_nuyens << naturalOrdering << radicalInverseOrdering 
    << digitalNetFlag <<  noDigitalShiftFlag <<  noScramblingFlag
    << mostSignificantBitFirst << leastSignificantBitFirst << numberOfBits
    << scrambleSize << joe_kuo << sobol_order_2 << grayCodeOrdering
    << dOptimal << numCandidateDesigns //<< reliabilitySearchType
    << reliabilityIntegration << integrationRefine << refineSamples
    << optSubProbSolver << numericalSolveMode << estVarMetricType
    << estVarMetricNormOrder << pilotSamples << ensemblePilotSolnMode
    << pilotGroupSampling << groupThrottleType << groupSizeThrottle
    << rCondBestThrottle << rCondTolThrottle
    << truthPilotConstraint << dagRecursionType << dagDepthLimit
    << modelSelectType << relaxFactorSequence << relaxFixedFactor
    << relaxRecursiveFactor << allocationTarget
    << useTargetVarianceOptimizationFlag
    << qoiAggregation << scalarizationRespCoeffs
    << convergenceToleranceType << convergenceToleranceTarget
    << multilevAllocControl << multilevEstimatorRate
    << multilevDiscrepEmulation << finalStatsType << finalMomentsType
    << distributionType << responseLevelTarget << responseLevelTargetReduce
    << responseLevels << probabilityLevels << reliabilityLevels
    << genReliabilityLevels << chainSamples << buildSamples
    << samplesOnEmulator << emulatorOrder << emulatorType << mcmcType
    << standardizedSpace << adaptPosteriorRefine << logitTransform
    << gpmsaNormalize << posteriorStatsKL << posteriorStatsMutual
    << posteriorStatsKDE << chainDiagnostics << chainDiagnosticsCI
    << modelEvidence << modelEvidLaplace << modelEvidMC
    << proposalCovType << priorPropCovMult << proposalCovUpdatePeriod
    << proposalCovInputType << proposalCovData << proposalCovFile
    << advancedOptionsFilename << quesoOptionsFilename << fitnessMetricType
    << batchSelectionType << lipschitzType << calibrateErrorMode
    << hyperPriorAlphas << hyperPriorBetas
    << burnInSamples << subSamplingPeriod << evidenceSamples
    << calModelDiscrepancy << numPredConfigs << predictionConfigList
    << importPredConfigs << importPredConfigFormat << modelDiscrepancyType
    << polynomialOrder << exportCorrModelFile << exportCorrModelFormat
    << exportCorrVarFile << exportCorrVarFormat << exportDiscrepFile
    << exportDiscrepFormat << adaptExpDesign << importCandPtsFile
    << importCandFormat << numCandidates << maxHifiEvals
    << batchSize << batchSizeExplore
    << mutualInfoKSG2 << numChains << numCR << crossoverChainPairs
    << grThreshold << jumpStep << numPushforwardSamples
    << drNumStages
    << drScaleType
    << drScale
    << amPeriodNumSteps
    << amStartingStep
    << amScale
    << malaStepSize
    << diliHessianType
    << diliAdaptInterval
    << diliAdaptStart
    << diliAdaptEnd
    << diliInitialWeight
    << diliHessTolerance
    << diliLISTolerance
    << diliSesNumEigs
    << diliSesRelTol
    << diliSesAbsTol
    << diliSesExpRank
    << diliSesOversFactor
    << diliSesBlockSize
    << dataDistType << dataDistCovInputType << dataDistMeans
    << dataDistCovariance << dataDistFile << posteriorDensityExportFilename
    << posteriorSamplesExportFilename << posteriorSamplesImportFilename
    << generatePosteriorSamples << evaluatePosteriorDensity;

  // Parameter Study
  s << finalPoint << stepVector << numSteps << stepsPerVariable << listOfPoints
    << pstudyFilename << pstudyFileFormat << pstudyFileActive
    << varPartitions;

  // Verification
  s << refinementRate;

  // Point import/export files
  s << importBuildPtsFile  << importBuildFormat  << importBuildActive
    << importApproxPtsFile << importApproxFormat << importApproxActive
    << exportApproxPtsFile << exportApproxFormat << exportMCMCPtsFile
    << exportSampleSeqFlag << exportSamplesFormat;
}


void DataMethodRep::read(MPIUnpackBuffer& s)
{
  s >> idMethod >> modelPointer >> lowFidModelPointer >> methodOutput
    >> maxIterations >> maxRefineIterations >> maxSolverIterations
    >> maxFunctionEvals >> speculativeFlag >> methodUseDerivsFlag
    >> constraintTolerance >> methodScaling >> numFinalSolutions
    >> convergenceTolerance >> relativeConvMetric >> statsMetricMode
    >> methodName >> subMethod >> subMethodName >> subModelPointer
    >> subMethodPointer;

  // Meta-iterators
  s >> iteratorServers >> procsPerIterator >> iteratorScheduling
    >> hybridMethodNames >> hybridModelPointers >> hybridMethodPointers
  //>> hybridProgThresh
    >> hybridGlobalMethodName >> hybridGlobalModelPointer
    >> hybridGlobalMethodPointer >> hybridLocalMethodName
    >> hybridLocalModelPointer >> hybridLocalMethodPointer >> hybridLSProb
  //>> branchBndNumSamplesRoot >> branchBndNumSamplesNode
    >> concurrentRandomJobs >> concurrentParameterSets;

  // Surrogate-based
  s >> softConvLimit >> surrBasedLocalLayerBypass
    >> trustRegionInitSize >> trustRegionMinSize
    >> trustRegionContractTrigger >> trustRegionExpandTrigger
    >> trustRegionContract >> trustRegionExpand
    >> surrBasedLocalSubProbObj >> surrBasedLocalSubProbCon
    >> surrBasedLocalMeritFn >> surrBasedLocalAcceptLogic
    >> surrBasedLocalConstrRelax >> surrBasedGlobalReplacePts;

  // DL_SOLVER
  s >> dlDetails;

  // NPSOL
  s >> verifyLevel >> functionPrecision >> lineSearchTolerance;

  // NL2SOL
  s >> absConvTol >> xConvTol >> singConvTol >> singRadius >> falseConvTol
    >> initTRRadius >> covarianceType >> regressDiag;

  // OPT++
  s >> searchMethod >> gradientTolerance >> maxStep >> meritFn
    >> stepLenToBoundary >> centeringParam >> searchSchemeSize;

  // APPSPACK
  s >> initStepLength >> contractStepLength >> threshStepLength >> meritFunction
    >> constrPenalty >> smoothFactor;

  // COLINY
  s >> constraintPenalty >> constantPenalty >> globalBalanceParam
    >> localBalanceParam >> maxBoxSize >> minBoxSize >> boxDivision
    >> mutationAdaptive >> showMiscOptions >> miscOptions >> solnTarget
    >> crossoverRate >> mutationRate >> mutationScale >> mutationMinScale
    >> initDelta >> threshDelta >> contractFactor >> newSolnsGenerated
    >> numberRetained >> expansionFlag >> expandAfterSuccess
    >> contractAfterFail >> mutationRange >> totalPatternSize
    >> randomizeOrderFlag >> selectionPressure >> replacementType
    >> crossoverType >> mutationType >> exploratoryMoves >> patternBasis
    >> betaSolverName;

  // COLINY + APPSPACK
  s >> evalSynchronize;

  // JEGA
  s >> numCrossPoints >> numParents >> numOffspring >> fitnessType
    >> convergenceType >> percentChange >> numGenerations >> fitnessLimit
    >> shrinkagePercent >> nichingType >> nicheVector >> numDesigns
    >> postProcessorType >> distanceVector;

  // JEGA/COLINY
  s >> initializationType >> flatFile >> logFile >> populationSize
    >> printPopFlag;

  // NCSU
  s >> volBoxSize;

  // DDACE
  s >> numSymbols >> mainEffectsFlag;

  // FSUDace
  s >> latinizeFlag >> volQualityFlag >> sequenceStart >> sequenceLeap
    >> primeBase >> numTrials >> trialType;

  // COLINY, NonD, DACE, & JEGA
  s >> randomSeed >> randomSeedSeq;

  // MADS
  s >> initMeshSize >> minMeshSize >> historyFile >> displayFormat >> vns
    >> neighborOrder >> showAllEval >> useSurrogate;

  // C3 FT
  s >> maxCrossIterations >> solverTol >> solverRoundingTol >> statsRoundingTol
    >> startOrder >> kickOrder >> maxOrder >> adaptOrder
    >> startRank  >> kickRank  >> maxRank  >> adaptRank >> maxCVRankCandidates
    >> maxCVOrderCandidates >> c3AdvanceType >> startOrderSeq >> startRankSeq;

  // NonD & DACE
  s >> numSamples >> fixedSeedFlag >> fixedSequenceFlag
    >> vbdFlag >> vbdDropTolerance
    >> vbdViaSamplingMethod >> vbdViaSamplingNumBins
    >> backfillFlag >> pcaFlag
    >> percentVarianceExplained >> wilksFlag >> wilksOrder
    >> wilksConfidenceLevel >> wilksSidedInterval;

  // NonD
  s >> toleranceIntervalsFlag >> tiCoverage >> tiConfidenceLevel
    >> stdRegressionCoeffs >> respScalingFlag >> vbdOrder >> covarianceControl
    >> rngName >> refinementType >> refinementControl >> nestingOverride
    >> growthOverride >> expansionType >> piecewiseBasis >> expansionBasisType
    >> quadratureOrderSeq >> sparseGridLevelSeq >> expansionOrderSeq
    >> collocationPointsSeq >> expansionSamplesSeq >> quadratureOrder
    >> sparseGridLevel >> expansionOrder >> collocationPoints
    >> expansionSamples //>> expansionSampleType
    >> anisoDimPref >> cubIntOrder >> collocationRatio
    >> collocRatioTermsOrder >> regressionType >> lsRegressionType
    >> regressionNoiseTol >> regressionL2Penalty >> crossValidation
    >> crossValidNoiseOnly //>> adaptedBasisInitLevel
    >> adaptedBasisAdvancements >> normalizedCoeffs >> pointReuse
    >> tensorGridFlag >> tensorGridOrder
    >> importExpansionFile >> exportExpansionFile >> sampleType
    >> rank1LatticeFlag >>  noRandomShiftFlag >> log2MaxPoints >> kuo 
    >> cools_kuo_nuyens >> naturalOrdering >> radicalInverseOrdering 
    >> digitalNetFlag >>  noDigitalShiftFlag >>  noScramblingFlag
    >> mostSignificantBitFirst >> leastSignificantBitFirst >> numberOfBits
    >> scrambleSize >> joe_kuo >> sobol_order_2 >> grayCodeOrdering
    >> dOptimal >> numCandidateDesigns //>> reliabilitySearchType
    >> reliabilityIntegration >> integrationRefine >> refineSamples
    >> optSubProbSolver >> numericalSolveMode >> estVarMetricType
    >> estVarMetricNormOrder >> pilotSamples >> ensemblePilotSolnMode
    >> pilotGroupSampling >> groupThrottleType >> groupSizeThrottle
    >> rCondBestThrottle >> rCondTolThrottle
    >> truthPilotConstraint >> dagRecursionType >> dagDepthLimit
    >> modelSelectType >> relaxFactorSequence >> relaxFixedFactor
    >> relaxRecursiveFactor >> allocationTarget
    >> useTargetVarianceOptimizationFlag
    >> qoiAggregation >> scalarizationRespCoeffs
    >> convergenceToleranceType >> convergenceToleranceTarget
    >> multilevAllocControl >> multilevEstimatorRate
    >> multilevDiscrepEmulation >> finalStatsType >> finalMomentsType
    >> distributionType >> responseLevelTarget >> responseLevelTargetReduce
    >> responseLevels >> probabilityLevels >> reliabilityLevels
    >> genReliabilityLevels >> chainSamples >> buildSamples
    >> samplesOnEmulator >> emulatorOrder >> emulatorType >> mcmcType
    >> standardizedSpace >> adaptPosteriorRefine >> logitTransform
    >> gpmsaNormalize >> posteriorStatsKL >> posteriorStatsMutual
    >> posteriorStatsKDE >> chainDiagnostics >> chainDiagnosticsCI
    >> modelEvidence >> modelEvidLaplace >> modelEvidMC
    >> proposalCovType >> priorPropCovMult >> proposalCovUpdatePeriod
    >> proposalCovInputType >> proposalCovData >> proposalCovFile
    >> advancedOptionsFilename >> quesoOptionsFilename >> fitnessMetricType
    >> batchSelectionType >> lipschitzType >> calibrateErrorMode
    >> hyperPriorAlphas >> hyperPriorBetas
    >> burnInSamples >> subSamplingPeriod >> evidenceSamples
    >> calModelDiscrepancy >> numPredConfigs >> predictionConfigList
    >> importPredConfigs >> importPredConfigFormat >> modelDiscrepancyType
    >> polynomialOrder >> exportCorrModelFile >> exportCorrModelFormat
    >> exportCorrVarFile >> exportCorrVarFormat >> exportDiscrepFile
    >> exportDiscrepFormat >> adaptExpDesign >> importCandPtsFile
    >> importCandFormat >> numCandidates >> maxHifiEvals
    >> batchSize >> batchSizeExplore
    >> mutualInfoKSG2 >> numChains >> numCR >> crossoverChainPairs
    >> grThreshold >> jumpStep >> numPushforwardSamples
    >> drNumStages
    >> drScaleType
    >> drScale
    >> amPeriodNumSteps
    >> amStartingStep
    >> amScale
    >> malaStepSize
    >> diliHessianType
    >> diliAdaptInterval
    >> diliAdaptStart
    >> diliAdaptEnd
    >> diliInitialWeight
    >> diliHessTolerance
    >> diliLISTolerance
    >> diliSesNumEigs
    >> diliSesRelTol
    >> diliSesAbsTol
    >> diliSesExpRank
    >> diliSesOversFactor
    >> diliSesBlockSize
    >> dataDistType >> dataDistCovInputType >> dataDistMeans
    >> dataDistCovariance >> dataDistFile >> posteriorDensityExportFilename
    >> posteriorSamplesExportFilename >> posteriorSamplesImportFilename
    >> generatePosteriorSamples >> evaluatePosteriorDensity;

  // Parameter Study
  s >> finalPoint >> stepVector >> numSteps >> stepsPerVariable >> listOfPoints
    >> pstudyFilename >> pstudyFileFormat >> pstudyFileActive
    >> varPartitions;

  // Verification
  s >> refinementRate;

  // Point import/export files
  s >> importBuildPtsFile  >> importBuildFormat  >> importBuildActive
    >> importApproxPtsFile >> importApproxFormat >> importApproxActive
    >> exportApproxPtsFile >> exportApproxFormat >> exportMCMCPtsFile
    >> exportSampleSeqFlag >> exportSamplesFormat;
}


void DataMethodRep::write(std::ostream& s) const
{
  s << idMethod << modelPointer << lowFidModelPointer << methodOutput
    << maxIterations << maxRefineIterations << maxSolverIterations
    << maxFunctionEvals << speculativeFlag << methodUseDerivsFlag
    << constraintTolerance << methodScaling << numFinalSolutions
    << convergenceTolerance << relativeConvMetric << statsMetricMode
    << methodName << subMethod << subMethodName << subModelPointer
    << subMethodPointer;

  // Meta-iterators
  s << iteratorServers << procsPerIterator << iteratorScheduling
    << hybridMethodNames << hybridModelPointers << hybridMethodPointers
  //<< hybridProgThresh
    << hybridGlobalMethodName << hybridGlobalModelPointer
    << hybridGlobalMethodPointer << hybridLocalMethodName
    << hybridLocalModelPointer << hybridLocalMethodPointer << hybridLSProb
  //<< branchBndNumSamplesRoot << branchBndNumSamplesNode
    << concurrentRandomJobs << concurrentParameterSets;

  // Surrogate-based
  s << softConvLimit << surrBasedLocalLayerBypass
    << trustRegionInitSize << trustRegionMinSize
    << trustRegionContractTrigger << trustRegionExpandTrigger
    << trustRegionContract << trustRegionExpand
    << surrBasedLocalSubProbObj << surrBasedLocalSubProbCon
    << surrBasedLocalMeritFn << surrBasedLocalAcceptLogic
    << surrBasedLocalConstrRelax << surrBasedGlobalReplacePts;

  // DL_SOLVER
  s << dlDetails;

  // NPSOL
  s << verifyLevel << functionPrecision << lineSearchTolerance;

  // NL2SOL
  s << absConvTol << xConvTol << singConvTol << singRadius << falseConvTol
    << initTRRadius << covarianceType << regressDiag;

  // OPT++
  s << searchMethod << gradientTolerance << maxStep << meritFn
    << stepLenToBoundary << centeringParam << searchSchemeSize;

  // APPSPACK
  s << initStepLength << contractStepLength << threshStepLength << meritFunction
    << constrPenalty << smoothFactor;

  // COLINY
  s << constraintPenalty << constantPenalty << globalBalanceParam
    << localBalanceParam << maxBoxSize << minBoxSize << boxDivision
    << mutationAdaptive << showMiscOptions << miscOptions << solnTarget
    << crossoverRate << mutationRate << mutationScale << mutationMinScale
    << initDelta << threshDelta << contractFactor << newSolnsGenerated
    << numberRetained << expansionFlag << expandAfterSuccess
    << contractAfterFail << mutationRange << totalPatternSize
    << randomizeOrderFlag << selectionPressure << replacementType
    << crossoverType << mutationType << exploratoryMoves << patternBasis
    << betaSolverName;

  // COLINY + APPSPACK
  s << evalSynchronize;

  // JEGA
  s << numCrossPoints << numParents << numOffspring << fitnessType
    << convergenceType << percentChange << numGenerations << fitnessLimit
    << shrinkagePercent << nichingType << nicheVector << numDesigns
    << postProcessorType << distanceVector;

  // JEGA/COLINY
  s << initializationType << flatFile << logFile << populationSize
    << printPopFlag;

  // NCSU
  s << volBoxSize;

  // DDACE
  s << numSymbols << mainEffectsFlag;

  // FSUDace
  s << latinizeFlag << volQualityFlag << sequenceStart << sequenceLeap
    << primeBase << numTrials << trialType;

  // COLINY, NonD, DACE, & JEGA
  s << randomSeed << randomSeedSeq;

  // MADS
  s << initMeshSize << minMeshSize << historyFile << displayFormat << vns
    << neighborOrder << showAllEval << useSurrogate;

  // C3 FT
  s << maxCrossIterations << solverTol << solverRoundingTol << statsRoundingTol
    << startOrder << kickOrder << maxOrder << adaptOrder
    << startRank  << kickRank  << maxRank  << adaptRank << maxCVRankCandidates
    << maxCVOrderCandidates << c3AdvanceType << startOrderSeq << startRankSeq;

  // NonD & DACE
  s << numSamples << fixedSeedFlag << fixedSequenceFlag
    << vbdFlag << vbdDropTolerance
    << vbdViaSamplingMethod << vbdViaSamplingNumBins
    << backfillFlag << pcaFlag
    << percentVarianceExplained << wilksFlag << wilksOrder
    << wilksConfidenceLevel << wilksSidedInterval;

  // NonD
  s << toleranceIntervalsFlag << tiCoverage << tiConfidenceLevel
    << stdRegressionCoeffs << respScalingFlag << vbdOrder << covarianceControl
    << rngName << refinementType << refinementControl << nestingOverride
    << growthOverride << expansionType << piecewiseBasis << expansionBasisType
    << quadratureOrderSeq << sparseGridLevelSeq << expansionOrderSeq
    << collocationPointsSeq << expansionSamplesSeq << quadratureOrder
    << sparseGridLevel << expansionOrder << collocationPoints
    << expansionSamples //<< expansionSampleType
    << anisoDimPref << cubIntOrder << collocationRatio
    << collocRatioTermsOrder << regressionType << lsRegressionType
    << regressionNoiseTol << regressionL2Penalty << crossValidation
    << crossValidNoiseOnly //<< adaptedBasisInitLevel
    << adaptedBasisAdvancements << normalizedCoeffs << pointReuse
    << tensorGridFlag << tensorGridOrder
    << importExpansionFile << exportExpansionFile << sampleType
    << rank1LatticeFlag <<  noRandomShiftFlag << log2MaxPoints << kuo 
    << cools_kuo_nuyens << naturalOrdering << radicalInverseOrdering 
    << digitalNetFlag <<  noDigitalShiftFlag <<  noScramblingFlag
    << mostSignificantBitFirst << leastSignificantBitFirst << numberOfBits
    << scrambleSize << joe_kuo << sobol_order_2 << grayCodeOrdering
    << dOptimal << numCandidateDesigns //<< reliabilitySearchType
    << reliabilityIntegration << integrationRefine << refineSamples
    << optSubProbSolver << numericalSolveMode << estVarMetricType
    << estVarMetricNormOrder << pilotSamples << ensemblePilotSolnMode
    << pilotGroupSampling << groupThrottleType << groupSizeThrottle
    << rCondBestThrottle << rCondTolThrottle
    << truthPilotConstraint << dagRecursionType << dagDepthLimit
    << modelSelectType << relaxFactorSequence << relaxFixedFactor
    << relaxRecursiveFactor << allocationTarget
    << useTargetVarianceOptimizationFlag
    << qoiAggregation << scalarizationRespCoeffs
    << convergenceToleranceType << convergenceToleranceTarget
    << multilevAllocControl << multilevEstimatorRate
    << multilevDiscrepEmulation << finalStatsType << finalMomentsType
    << distributionType << responseLevelTarget << responseLevelTargetReduce
    << responseLevels << probabilityLevels << reliabilityLevels
    << genReliabilityLevels << chainSamples << buildSamples
    << samplesOnEmulator << emulatorOrder << emulatorType << mcmcType
    << standardizedSpace << adaptPosteriorRefine << logitTransform
    << gpmsaNormalize << posteriorStatsKL << posteriorStatsMutual
    << posteriorStatsKDE << chainDiagnostics << chainDiagnosticsCI
    << modelEvidence << modelEvidLaplace << modelEvidMC
    << proposalCovType << priorPropCovMult << proposalCovUpdatePeriod
    << proposalCovInputType << proposalCovData << proposalCovFile
    << advancedOptionsFilename << quesoOptionsFilename << fitnessMetricType
    << batchSelectionType << lipschitzType << calibrateErrorMode
    << hyperPriorAlphas << hyperPriorBetas
    << burnInSamples << subSamplingPeriod << evidenceSamples
    << calModelDiscrepancy << numPredConfigs << predictionConfigList
    << importPredConfigs << importPredConfigFormat << modelDiscrepancyType
    << polynomialOrder << exportCorrModelFile << exportCorrModelFormat
    << exportCorrVarFile << exportCorrVarFormat << exportDiscrepFile
    << exportDiscrepFormat << adaptExpDesign << importCandPtsFile
    << importCandFormat << numCandidates << maxHifiEvals
    << batchSize << batchSizeExplore
    << mutualInfoKSG2 << numChains << numCR << crossoverChainPairs
    << grThreshold << jumpStep << numPushforwardSamples
    << drNumStages
    << drScaleType
    << drScale
    << amPeriodNumSteps
    << amStartingStep
    << amScale
    << malaStepSize
    << diliHessianType
    << diliAdaptInterval
    << diliAdaptStart
    << diliAdaptEnd
    << diliInitialWeight
    << diliHessTolerance
    << diliLISTolerance
    << diliSesNumEigs
    << diliSesRelTol
    << diliSesAbsTol
    << diliSesExpRank
    << diliSesOversFactor
    << diliSesBlockSize
    << dataDistType << dataDistCovInputType << dataDistMeans
    << dataDistCovariance << dataDistFile << posteriorDensityExportFilename
    << posteriorSamplesExportFilename << posteriorSamplesImportFilename
    << generatePosteriorSamples << evaluatePosteriorDensity;

  // Parameter Study
  s << finalPoint << stepVector << numSteps << stepsPerVariable << listOfPoints
    << pstudyFilename << pstudyFileFormat << pstudyFileActive
    << varPartitions;

  // Verification
  s << refinementRate;

  // Point import/export files
  s << importBuildPtsFile  << importBuildFormat  << importBuildActive
    << importApproxPtsFile << importApproxFormat << importApproxActive
    << exportApproxPtsFile << exportApproxFormat << exportMCMCPtsFile
    << exportSampleSeqFlag << exportSamplesFormat;
}


DataMethod::DataMethod(): dataMethodRep(new DataMethodRep())
{ /* empty ctor */ }


DataMethod::DataMethod(const DataMethod& data_method):
  dataMethodRep(data_method.dataMethodRep)
{ /* empty ctor */ }


DataMethod& DataMethod::operator=(const DataMethod& data_method)
{
  dataMethodRep = data_method.dataMethodRep;
  return *this;
}


DataMethod::~DataMethod()
{ /* empty dtor */ }

} // namespace Dakota
