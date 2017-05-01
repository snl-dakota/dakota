/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataMethod
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataMethod.hpp"
#include "dakota_data_io.hpp"
#include "pecos_global_defs.hpp"
#ifdef HAVE_OPTPP
#include "globals.h"
#endif


namespace Dakota {

DataMethodRep::DataMethodRep():
  methodOutput(NORMAL_OUTPUT), maxIterations(-1), maxRefineIterations(-1),
  maxSolverIterations(-1), maxFunctionEvaluations(1000), speculativeFlag(false),
  methodUseDerivsFlag(false), convergenceTolerance(1.e-4), 
  constraintTolerance(0.), methodScaling(false), numFinalSolutions(0),
  methodName(DEFAULT_METHOD), subMethod(SUBMETHOD_DEFAULT),
  // Meta-iterators
  iteratorServers(0), procsPerIterator(0), // 0 defaults to detect user spec
  iteratorScheduling(DEFAULT_SCHEDULING), hybridLSProb(0.1),
  //hybridProgThresh(0.5),
  concurrentRandomJobs(0),
  // Local surrogate-based opt/NLS
  softConvLimit(0), // dummy value -> method-specific default
  surrBasedLocalLayerBypass(false),  //trustRegionInitSize(0.4),
  trustRegionMinSize(1.0e-6),        trustRegionContractTrigger(0.25),
  trustRegionExpandTrigger(0.75),    trustRegionContract(0.25),
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
  numSymbols(0),mainEffectsFlag(false),
  // FSUDace
  latinizeFlag(false), volQualityFlag(false), numTrials(10000),
  //initializationType("grid"), trialType("random"),
  // COLINY, JEGA, NonD, & DACE
  randomSeed(0),
  // NOMAD
  initMeshSize(1.0), minMeshSize(1.e-6),
  historyFile("mads_history"), displayFormat("bbe obj"),
  vns(0.0), neighborOrder(1), showAllEval(false),
  useSurrogate("none"),
  // NonD & DACE
  numSamples(0), fixedSeedFlag(false),
  fixedSequenceFlag(false), //default is variable sampling patterns
  vbdFlag(false),
  vbdDropTolerance(-1.),backfillFlag(false), pcaFlag(false),
  percentVarianceExplained(0.95), wilksFlag(false), wilksOrder(1),
  wilksConfidenceLevel(0.95), wilksSidedInterval(ONE_SIDED_UPPER),
  // NonD
  vbdOrder(0), covarianceControl(DEFAULT_COVARIANCE), rngName("mt19937"),
  refinementType(Pecos::NO_REFINEMENT), refinementControl(Pecos::NO_CONTROL),
  nestingOverride(Pecos::NO_NESTING_OVERRIDE),
  growthOverride(Pecos::NO_GROWTH_OVERRIDE), expansionType(EXTENDED_U),
  piecewiseBasis(false), expansionBasisType(Pecos::DEFAULT_BASIS),
  cubIntOrder(USHRT_MAX), collocationRatio(0.), collocRatioTermsOrder(1.),
  regressionType(Pecos::DEFAULT_REGRESSION), lsRegressionType(DEFAULT_LS),
  regressionL2Penalty(0.), crossValidation(false), crossValidNoiseOnly(false),
  //adaptedBasisInitLevel(0),
  adaptedBasisAdvancements(3), normalizedCoeffs(false), tensorGridFlag(false),
  //expansionSampleType("lhs"),
  sampleType(SUBMETHOD_DEFAULT), dOptimal(false), numCandidateDesigns(0),
  reliabilitySearchType(MV), integrationRefine(NO_INT_REFINE),
  finalMomentsType(STANDARD_MOMENTS), distributionType(CUMULATIVE),
  responseLevelTarget(PROBABILITIES), responseLevelTargetReduce(COMPONENT),
  chainSamples(0), buildSamples(0), samplesOnEmulator(0), emulatorOrder(0),
  emulatorType(NO_EMULATOR), mcmcType("dram"), standardizedSpace(false),
  adaptPosteriorRefine(false), logitTransform(false), gpmsaNormalize(false),
  posteriorStatsKL(false),
  posteriorStatsMutual(false),  preSolveMethod(SUBMETHOD_DEFAULT),
  proposalCovUpdates(0), fitnessMetricType("predicted_variance"), 
  batchSelectionType("naive"), lipschitzType("local"), 
  calibrateErrorMode(CALIBRATE_NONE), burnInSamples(0), subSamplingPeriod(1),
  calModelDiscrepancy(false),
  // numPredConfigs (BMA TODO this is not initialized...)
  importPredConfigFormat(TABULAR_ANNOTATED), discrepancyType("global_kriging"),
  approxCorrectionOrder(2), exportCorrModelFormat(TABULAR_ANNOTATED),
  exportCorrVarFormat(TABULAR_ANNOTATED),
  exportDiscrepFormat(TABULAR_ANNOTATED), adaptExpDesign(false),
  importCandFormat(TABULAR_ANNOTATED), numCandidates(0), maxHifiEvals(-1.),  
  // DREAM
  numChains(3), numCR(3), crossoverChainPairs(3), grThreshold(1.2),
  jumpStep(5),

  generatePosteriorSamples(false), evaluatePosteriorDensity(false),

  // Parameter Study
  numSteps(0), pstudyFileFormat(TABULAR_ANNOTATED), pstudyFileActive(false),
  // Verification
  refinementRate(2.),
  // Point import/export files
  importBuildFormat(TABULAR_ANNOTATED),   importBuildActive(false),
  importApproxFormat(TABULAR_ANNOTATED),  importApproxActive(false),
  exportApproxFormat(TABULAR_ANNOTATED),
  exportSampleSeqFlag(false), exportSamplesFormat(TABULAR_ANNOTATED),
  referenceCount(1)
{ }


void DataMethodRep::write(MPIPackBuffer& s) const
{
  s << idMethod << modelPointer << lowFidModelPointer << methodOutput
    << maxIterations << maxRefineIterations << maxSolverIterations
    << maxFunctionEvaluations << speculativeFlag << methodUseDerivsFlag
    << convergenceTolerance << constraintTolerance << methodScaling
    << numFinalSolutions << methodName << subMethod << subMethodName
    << subModelPointer << subMethodPointer;

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
  s << randomSeed;

  // MADS
  s << initMeshSize << minMeshSize << historyFile << displayFormat << vns
    << neighborOrder << showAllEval << useSurrogate;

  // NonD & DACE
  s << numSamples << fixedSeedFlag << fixedSequenceFlag
    << vbdFlag << vbdDropTolerance << backfillFlag << pcaFlag
    << percentVarianceExplained << wilksFlag << wilksOrder
    << wilksConfidenceLevel << wilksSidedInterval;

  // NonD
  s << vbdOrder << covarianceControl << rngName << refinementType
    << refinementControl << nestingOverride << growthOverride << expansionType
    << piecewiseBasis << expansionBasisType << expansionOrder
    << expansionSamples << expansionSampleType << quadratureOrder
    << sparseGridLevel << anisoDimPref << cubIntOrder << collocationPoints
    << collocationRatio << collocRatioTermsOrder << regressionType
    << lsRegressionType << regressionNoiseTol << regressionL2Penalty
    << crossValidation << crossValidNoiseOnly //<< adaptedBasisInitLevel
    << adaptedBasisAdvancements << normalizedCoeffs << pointReuse
    << tensorGridFlag << tensorGridOrder << importExpansionFile
    << exportExpansionFile << sampleType << dOptimal << numCandidateDesigns
    << reliabilitySearchType
    << reliabilityIntegration << integrationRefine << refineSamples
    << pilotSamples << finalMomentsType << distributionType << responseLevelTarget
    << responseLevelTargetReduce << responseLevels << probabilityLevels
    << reliabilityLevels << genReliabilityLevels << chainSamples
    << buildSamples << samplesOnEmulator << emulatorOrder << emulatorType
    << mcmcType << standardizedSpace << adaptPosteriorRefine << logitTransform
    << gpmsaNormalize
    << posteriorStatsKL << posteriorStatsMutual << preSolveMethod
    << proposalCovType << proposalCovUpdates << proposalCovInputType
    << proposalCovData << proposalCovFile << quesoOptionsFilename
    << fitnessMetricType
    << batchSelectionType << lipschitzType << calibrateErrorMode
    << hyperPriorAlphas << hyperPriorBetas << burnInSamples << subSamplingPeriod
    << calModelDiscrepancy << numPredConfigs << predictionConfigList
    << importPredConfigs << importPredConfigFormat
    << discrepancyType << approxCorrectionOrder << exportCorrModelFile
    << exportCorrModelFormat << exportCorrVarFile << exportCorrVarFormat
    << exportDiscrepFile << exportDiscrepFormat << adaptExpDesign
    << importCandPtsFile << importCandFormat << numCandidates << maxHifiEvals
    << numChains << numCR << crossoverChainPairs << grThreshold << jumpStep
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
    >> maxFunctionEvaluations >> speculativeFlag >> methodUseDerivsFlag
    >> convergenceTolerance >> constraintTolerance >> methodScaling
    >> numFinalSolutions >> methodName >> subMethod >> subMethodName
    >> subModelPointer >> subMethodPointer;

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
  s >> randomSeed;

  // MADS
  s >> initMeshSize >> minMeshSize >> historyFile >> displayFormat >> vns
    >> neighborOrder >> showAllEval >> useSurrogate;

  // NonD & DACE
  s >> numSamples >> fixedSeedFlag >> fixedSequenceFlag
    >> vbdFlag >> vbdDropTolerance >> backfillFlag >> pcaFlag
    >> percentVarianceExplained >> wilksFlag >> wilksOrder
    >> wilksConfidenceLevel >> wilksSidedInterval;

  // NonD
  s >> vbdOrder >> covarianceControl >> rngName >> refinementType
    >> refinementControl >> nestingOverride >> growthOverride >> expansionType
    >> piecewiseBasis >> expansionBasisType >> expansionOrder
    >> expansionSamples >> expansionSampleType >> quadratureOrder
    >> sparseGridLevel >> anisoDimPref >> cubIntOrder >> collocationPoints
    >> collocationRatio >> collocRatioTermsOrder >> regressionType
    >> lsRegressionType >> regressionNoiseTol >> regressionL2Penalty
    >> crossValidation >> crossValidNoiseOnly //>> adaptedBasisInitLevel
    >> adaptedBasisAdvancements >> normalizedCoeffs >> pointReuse
    >> tensorGridFlag >> tensorGridOrder >> importExpansionFile
    >> exportExpansionFile >> sampleType >> dOptimal >> numCandidateDesigns
    >> reliabilitySearchType
    >> reliabilityIntegration >> integrationRefine >> refineSamples
    >> pilotSamples >> finalMomentsType >> distributionType >> responseLevelTarget
    >> responseLevelTargetReduce >> responseLevels >> probabilityLevels
    >> reliabilityLevels >> genReliabilityLevels >> chainSamples
    >> buildSamples >> samplesOnEmulator >> emulatorOrder >> emulatorType
    >> mcmcType >> standardizedSpace >> adaptPosteriorRefine >> logitTransform
    >> gpmsaNormalize
    >> posteriorStatsKL >> posteriorStatsMutual >> preSolveMethod
    >> proposalCovType >> proposalCovUpdates >> proposalCovInputType
    >> proposalCovData >> proposalCovFile >> quesoOptionsFilename
    >> fitnessMetricType
    >> batchSelectionType >> lipschitzType >> calibrateErrorMode
    >> hyperPriorAlphas >> hyperPriorBetas >> burnInSamples >> subSamplingPeriod
    >> calModelDiscrepancy >> numPredConfigs >> predictionConfigList
    >> importPredConfigs >> importPredConfigFormat
    >> discrepancyType >> approxCorrectionOrder >> exportCorrModelFile
    >> exportCorrModelFormat >> exportCorrVarFile >> exportCorrVarFormat
    >> exportDiscrepFile >> exportDiscrepFormat >> adaptExpDesign
    >> importCandPtsFile >> importCandFormat >> numCandidates >> maxHifiEvals
    >> numChains >> numCR >> crossoverChainPairs >> grThreshold >> jumpStep
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
    << maxFunctionEvaluations << speculativeFlag << methodUseDerivsFlag
    << convergenceTolerance << constraintTolerance << methodScaling
    << numFinalSolutions << methodName << subMethod << subMethodName
    << subModelPointer << subMethodPointer;

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
  s << randomSeed;

  // MADS
  s << initMeshSize << minMeshSize << historyFile << displayFormat << vns
    << neighborOrder << showAllEval << useSurrogate;

  // NonD & DACE
  s << numSamples << fixedSeedFlag << fixedSequenceFlag
    << vbdFlag << vbdDropTolerance << backfillFlag << pcaFlag
    << percentVarianceExplained << wilksFlag << wilksOrder
    << wilksConfidenceLevel << wilksSidedInterval;

  // NonD
  s << vbdOrder << covarianceControl << rngName << refinementType
    << refinementControl << nestingOverride << growthOverride << expansionType
    << piecewiseBasis << expansionBasisType << expansionOrder
    << expansionSamples << expansionSampleType << quadratureOrder
    << sparseGridLevel << anisoDimPref << cubIntOrder << collocationPoints
    << collocationRatio << collocRatioTermsOrder << regressionType
    << lsRegressionType << regressionNoiseTol << regressionL2Penalty
    << crossValidation << crossValidNoiseOnly //<< adaptedBasisInitLevel
    << adaptedBasisAdvancements << normalizedCoeffs << pointReuse
    << tensorGridFlag << tensorGridOrder << importExpansionFile
    << exportExpansionFile << sampleType << dOptimal << numCandidateDesigns
    << reliabilitySearchType
    << reliabilityIntegration << integrationRefine << refineSamples
    << pilotSamples << finalMomentsType << distributionType << responseLevelTarget
    << responseLevelTargetReduce << responseLevels << probabilityLevels
    << reliabilityLevels << genReliabilityLevels << chainSamples
    << buildSamples << samplesOnEmulator << emulatorOrder << emulatorType
    << mcmcType << standardizedSpace << adaptPosteriorRefine << logitTransform
    << gpmsaNormalize
    << posteriorStatsKL << posteriorStatsMutual << preSolveMethod
    << proposalCovType << proposalCovUpdates << proposalCovInputType
    << proposalCovData << proposalCovFile << quesoOptionsFilename
    << fitnessMetricType
    << batchSelectionType << lipschitzType << calibrateErrorMode
    << hyperPriorAlphas << hyperPriorBetas << burnInSamples << subSamplingPeriod
    << calModelDiscrepancy << numPredConfigs << predictionConfigList
    << importPredConfigs << importPredConfigFormat
    << discrepancyType << approxCorrectionOrder << exportCorrModelFile
    << exportCorrModelFormat << exportCorrVarFile << exportCorrVarFormat
    << exportDiscrepFile << exportDiscrepFormat << adaptExpDesign
    << importCandPtsFile << importCandFormat << numCandidates << maxHifiEvals
    << numChains << numCR << crossoverChainPairs << grThreshold << jumpStep
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
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataMethod::DataMethod(), dataMethodRep referenceCount = "
       << dataMethodRep->referenceCount << std::endl;
#endif
}


DataMethod::DataMethod(const DataMethod& data_method)
{
  // Increment new (no old to decrement)
  dataMethodRep = data_method.dataMethodRep;
  if (dataMethodRep) // Check for an assignment of NULL
    ++dataMethodRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "DataMethod::DataMethod(DataMethod&)" << std::endl;
  if (dataMethodRep)
    Cout << "dataMethodRep referenceCount = " << dataMethodRep->referenceCount
	 << std::endl;
#endif
}


DataMethod& DataMethod::operator=(const DataMethod& data_method)
{
  if (dataMethodRep != data_method.dataMethodRep) { // normal case: old != new
    // Decrement old
    if (dataMethodRep) // Check for NULL
      if ( --dataMethodRep->referenceCount == 0 ) 
	delete dataMethodRep;
    // Assign and increment new
    dataMethodRep = data_method.dataMethodRep;
    if (dataMethodRep) // Check for NULL
      ++dataMethodRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataMethod::operator=(DataMethod&)" << std::endl;
  if (dataMethodRep)
    Cout << "dataMethodRep referenceCount = " << dataMethodRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


DataMethod::~DataMethod()
{
  if (dataMethodRep) { // Check for NULL
    --dataMethodRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataMethodRep referenceCount decremented to "
         << dataMethodRep->referenceCount << std::endl;
#endif
    if (dataMethodRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataMethodRep" << std::endl;
#endif
      delete dataMethodRep;
    }
  }
}

} // namespace Dakota
