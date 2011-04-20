/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataMethod
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataMethod.H"
#include "data_io.h"
#include "pecos_global_defs.hpp"


namespace Dakota {

DataMethodRep::DataMethodRep():
  methodOutput(NORMAL_OUTPUT), maxIterations(-1), maxFunctionEvaluations(1000),
  speculativeFlag(false), methodUseDerivsFlag(false),
  convergenceTolerance(1.e-4), constraintTolerance(0.), methodScaling(false),
  numFinalSolutions(0),
  // Local surrogate-based opt/NLS
  surrBasedLocalSoftConvLimit(5),        surrBasedLocalLayerBypass(false),
  surrBasedLocalTRInitSize(0.4),         surrBasedLocalTRMinSize(1.0e-6),
  surrBasedLocalTRContractTrigger(0.25), surrBasedLocalTRExpandTrigger(0.75),
  surrBasedLocalTRContract(0.25),        surrBasedLocalTRExpand(2.0),
  surrBasedLocalSubProbObj(ORIGINAL_PRIMARY),
  surrBasedLocalSubProbCon(ORIGINAL_CONSTRAINTS),
  surrBasedLocalMeritFn(AUGMENTED_LAGRANGIAN_MERIT),
  surrBasedLocalAcceptLogic(FILTER),     surrBasedLocalConstrRelax(NO_RELAX),
  // Global surrogate-based opt/NLS
  surrBasedGlobalReplacePts(false),
  // Branch and bound
  //branchBndNumSamplesRoot(0), branchBndNumSamplesNode(0),
  // DOT and CONMIN
  minMaxType("minimize"),
  // DL_SOLVER
  dlDetails(""), dlLib(0),
  // NPSOL
  verifyLevel(-1), functionPrecision(1.e-10), lineSearchTolerance(0.9),
  // NL2SOL: Real values of -1. ==> use NL2SOL default
  absConvTol(-1.), xConvTol(-1.), singConvTol(-1.), singRadius(-1.),
  falseConvTol(-1.), initTRRadius(-1.), covarianceType(0), regressDiag(false),
  // OPT++
  // searchMethod default is null since "trust_region" is preferred for 
  // unconstrained opt., whereas "line_search" is preferred for bc opt.
  gradientTolerance(0.0001),
  maxStep(1.e+3),             // only used in trust_region case
  meritFn("argaez_tapia"), centralPath("argaez_tapia"),
  stepLenToBoundary(0.99995), // see IDRProblemDescDB.C for meritFn dependence
  centeringParam(0.2),        // see IDRProblemDescDB.C for meritFn dependence
  searchSchemeSize(32),
  // APPSPACK
  initStepLength(1.0), contractStepLength(0.5), threshStepLength(0.01),
  evalSynchronize("nonblocking"),
  meritFunction("merit2_squared"), constrPenalty(1.0), smoothFactor(0.0),
  // COLINY
  //evalSynchronization("nonblocking"), // leave empty string as default
  constraintPenalty(-1.), constantPenalty(false), globalBalanceParam(-1.),
  localBalanceParam(-1.), maxBoxSize(-1.), minBoxSize(-1.),
  //boxDivision("major_dimension"), // leave empty string as default
  showMiscOptions(false), mutationAdaptive(true),
  // These attributes must replicate the Coliny defaults due to Coliny 
  // member fn. structure:
  crossoverRate(0.8), mutationRate(1.0),
  mutationScale(0.1), contractFactor(0.5),
  // These attributes replicate COLINY defaults due to convenience:
  totalPatternSize(0), // simplifies maxConcurrency calculation
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
  randomizeOrderFlag(false),
  // JEGA
  numCrossPoints(2), numParents(2), numOffspring(2),
  //replacementType("domination_count"),
  convergenceType("max_evals_or_gens"), fitnessLimit(6.0),
  shrinkagePercent(0.9), percentChange(0.1), numGenerations(15),
  nichingType("null_niching"),
  postProcessorType("null_postprocessor"),
  logFile("JEGAGlobal.log"),
  printPopFlag(false),
  // JEGA/COLINY
  initializationType("unique_random"), populationSize(50),
  flatFile(),
  // NCSU 
  volBoxSize(-1.),
  // DDACE
  numSymbols(0),mainEffectsFlag(false),
  // FSUDace
  numTrials(10000), latinizeFlag(false), volQualityFlag(false),
  fixedSequenceFlag(false), //default is variable sampling patterns
  //initializationType("grid"), trialType("random"),
  // COLINY, NonD, & DACE
  randomSeed(0),
  // NonD & DACE
  numSamples(0), numEmulatorSamples(0), fixedSeedFlag(false), 
  previousSamples(0), vbdFlag(false),
  // NonD
  vbdControl(Pecos::ALL_VBD), vbdDropTolerance(-1.), rngName("mt19937"),
  refinementType(Pecos::NO_REFINEMENT), refinementControl(Pecos::NO_CONTROL),
  nestingOverride(Pecos::NO_OVERRIDE), expansionType(EXTENDED_U),
  expansionTerms(0), expansionSamples(0), sparseGridLevel(USHRT_MAX),
  cubIntOrder(USHRT_MAX), collocationPoints(0), collocationRatio(0.),
  probCollocFlag(false), allVarsFlag(false),
  //expansionSampleType("lhs"), sampleType("lhs"),
  distributionType("cumulative"), responseLevelMappingType("probabilities"),
  rejectionType("delayed"),metropolisType("hastings"),
  // Parameter Study
  numSteps(0),
  // Verification
  refinementRate(2.),
  referenceCount(1)
{ }


void DataMethodRep::write(MPIPackBuffer& s) const
{
  s << idMethod << modelPointer << methodOutput << maxIterations
    << maxFunctionEvaluations << speculativeFlag << methodUseDerivsFlag
    << convergenceTolerance << constraintTolerance << methodScaling
    << linearIneqConstraintCoeffs << linearIneqLowerBnds << linearIneqUpperBnds
    << linearIneqScaleTypes << linearIneqScales << linearEqConstraintCoeffs
    << linearEqTargets << linearEqScaleTypes << linearEqScales << methodName
    << subMethodName << subMethodPointer << numFinalSolutions;

  s << surrBasedLocalSoftConvLimit << surrBasedLocalLayerBypass
    << surrBasedLocalTRInitSize << surrBasedLocalTRMinSize
    << surrBasedLocalTRContractTrigger << surrBasedLocalTRExpandTrigger
    << surrBasedLocalTRContract << surrBasedLocalTRExpand
    << surrBasedLocalSubProbObj << surrBasedLocalSubProbCon
    << surrBasedLocalMeritFn << surrBasedLocalAcceptLogic
    << surrBasedLocalConstrRelax << surrBasedGlobalReplacePts;
  //<< branchBndNumSamplesRoot << branchBndNumSamplesNode

  // DOT and CONMIN
  s << minMaxType;

  // DL_SOLVER
  s << dlDetails;

  // NPSOL
  s << verifyLevel << functionPrecision << lineSearchTolerance;

  // NL2SOL
  s << absConvTol << xConvTol << singConvTol << singRadius << falseConvTol
    << initTRRadius << covarianceType << regressDiag;

  // OPT++
  s << searchMethod << gradientTolerance << maxStep << meritFn << centralPath
    << stepLenToBoundary << centeringParam << searchSchemeSize;

  // APPSPACK
  s << initStepLength << contractStepLength << threshStepLength
    << evalSynchronize << meritFunction << constrPenalty << smoothFactor;

  // COLINY
  s << evalSynchronization << constraintPenalty << constantPenalty
    << globalBalanceParam << localBalanceParam << maxBoxSize << minBoxSize
    << boxDivision << mutationAdaptive << showMiscOptions << miscOptions
    << solnTarget << crossoverRate << mutationRate << mutationScale
    << mutationMinScale << initDelta << threshDelta << contractFactor
    << newSolnsGenerated << numberRetained << expansionFlag
    << expandAfterSuccess << contractAfterFail << mutationRange
    << totalPatternSize << randomizeOrderFlag << selectionPressure
    << replacementType << crossoverType << mutationType << exploratoryMoves
    << patternBasis;

  // JEGA
  s << numCrossPoints << numParents << numOffspring << fitnessType
    << convergenceType << percentChange << numGenerations << fitnessLimit
    << shrinkagePercent << nichingType << nicheVector << postProcessorType
    << distanceVector;

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

  // NonD & DACE
  s << numSamples << numEmulatorSamples << fixedSeedFlag 
    << fixedSequenceFlag << previousSamples << vbdFlag << vbdDropTolerance;

  // NonD
  s << vbdControl << rngName << refinementType << refinementControl
    << nestingOverride << expansionType << expansionTerms << expansionOrder
    << expansionSamples << expansionSampleType << quadratureOrder
    << sparseGridLevel << cubIntOrder << collocationPoints << collocationRatio
    << collocPtReuse << probCollocFlag << expansionImportFile << sampleType
    << reliabilitySearchType << reliabilityIntegration << integrationRefine
    << nondOptAlgorithm << distributionType << responseLevelMappingType
    << responseLevels << probabilityLevels << reliabilityLevels
    << genReliabilityLevels << xObsDataFile << yObsDataFile << yStdDataFile
    << allVarsFlag << emulatorType << rejectionType << metropolisType;

  // Parameter Study
  s << finalPoint << stepVector << numSteps << stepsPerVariable << listOfPoints
    << varPartitions;

  // Verification
  s << refinementRate;
}


void DataMethodRep::read(MPIUnpackBuffer& s)
{
  s >> idMethod >> modelPointer >> methodOutput >> maxIterations
    >> maxFunctionEvaluations >> speculativeFlag >> methodUseDerivsFlag
    >> convergenceTolerance >> constraintTolerance >> methodScaling
    >> linearIneqConstraintCoeffs >> linearIneqLowerBnds >> linearIneqUpperBnds
    >> linearIneqScaleTypes >> linearIneqScales >> linearEqConstraintCoeffs
    >> linearEqTargets >> linearEqScaleTypes >> linearEqScales >> methodName
    >> subMethodName >> subMethodPointer >> numFinalSolutions;

  s >> surrBasedLocalSoftConvLimit >> surrBasedLocalLayerBypass
    >> surrBasedLocalTRInitSize >> surrBasedLocalTRMinSize
    >> surrBasedLocalTRContractTrigger >> surrBasedLocalTRExpandTrigger
    >> surrBasedLocalTRContract >> surrBasedLocalTRExpand
    >> surrBasedLocalSubProbObj >> surrBasedLocalSubProbCon
    >> surrBasedLocalMeritFn >> surrBasedLocalAcceptLogic
    >> surrBasedLocalConstrRelax >> surrBasedGlobalReplacePts;
  //>> branchBndNumSamplesRoot >> branchBndNumSamplesNode

  // DOT and CONMIN
  s >> minMaxType;

  // DL_SOLVER
  s >> dlDetails;

  // NPSOL
  s >> verifyLevel >> functionPrecision >> lineSearchTolerance;

  // NL2SOL
  s >> absConvTol >> xConvTol >> singConvTol >> singRadius >> falseConvTol
    >> initTRRadius >> covarianceType >> regressDiag;

  // OPT++
  s >> searchMethod >> gradientTolerance >> maxStep >> meritFn >> centralPath
    >> stepLenToBoundary >> centeringParam >> searchSchemeSize;

  // APPSPACK
  s >> initStepLength >> contractStepLength >> threshStepLength
    >> evalSynchronize >> meritFunction >> constrPenalty >> smoothFactor;

  // COLINY
  s >> evalSynchronization >> constraintPenalty >> constantPenalty
    >> globalBalanceParam >> localBalanceParam >> maxBoxSize >> minBoxSize
    >> boxDivision >> mutationAdaptive >> showMiscOptions >> miscOptions
    >> solnTarget >> crossoverRate >> mutationRate >> mutationScale
    >> mutationMinScale >> initDelta >> threshDelta >> contractFactor
    >> newSolnsGenerated >> numberRetained >> expansionFlag
    >> expandAfterSuccess >> contractAfterFail >> mutationRange
    >> totalPatternSize >> randomizeOrderFlag >> selectionPressure
    >> replacementType >> crossoverType >> mutationType >> exploratoryMoves
    >> patternBasis;

  // JEGA
  s >> numCrossPoints >> numParents >> numOffspring >> fitnessType
    >> convergenceType >> percentChange >> numGenerations >> fitnessLimit
    >> shrinkagePercent >> nichingType >> nicheVector >> postProcessorType
    >> distanceVector;

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

  // NonD & DACE
  s >> numSamples >> numEmulatorSamples >> fixedSeedFlag 
    >> fixedSequenceFlag >> previousSamples >> vbdFlag >> vbdDropTolerance;

  // NonD
  s >> vbdControl >> rngName >> refinementType >> refinementControl
    >> nestingOverride >> expansionType >> expansionTerms >> expansionOrder
    >> expansionSamples >> expansionSampleType >> quadratureOrder
    >> sparseGridLevel >> cubIntOrder >> collocationPoints >> collocationRatio
    >> collocPtReuse >> probCollocFlag >> expansionImportFile >> sampleType
    >> reliabilitySearchType >> reliabilityIntegration >> integrationRefine
    >> nondOptAlgorithm >> distributionType >> responseLevelMappingType
    >> responseLevels >> probabilityLevels >> reliabilityLevels
    >> genReliabilityLevels >> xObsDataFile >> yObsDataFile >> yStdDataFile
    >> allVarsFlag >> emulatorType >> rejectionType >> metropolisType;

  // Parameter Study
  s >> finalPoint >> stepVector >> numSteps >> stepsPerVariable >> listOfPoints
    >> varPartitions;

  // Verification
  s >> refinementRate;
}


void DataMethodRep::write(std::ostream& s) const
{
  s << idMethod << modelPointer << methodOutput << maxIterations
    << maxFunctionEvaluations << speculativeFlag << methodUseDerivsFlag
    << convergenceTolerance << constraintTolerance << methodScaling
    << linearIneqConstraintCoeffs << linearIneqLowerBnds << linearIneqUpperBnds
    << linearIneqScaleTypes << linearIneqScales << linearEqConstraintCoeffs
    << linearEqTargets << linearEqScaleTypes << linearEqScales << methodName
    << subMethodName << subMethodPointer << numFinalSolutions;

  s << surrBasedLocalSoftConvLimit << surrBasedLocalLayerBypass
    << surrBasedLocalTRInitSize << surrBasedLocalTRMinSize
    << surrBasedLocalTRContractTrigger << surrBasedLocalTRExpandTrigger
    << surrBasedLocalTRContract << surrBasedLocalTRExpand
    << surrBasedLocalSubProbObj << surrBasedLocalSubProbCon
    << surrBasedLocalMeritFn << surrBasedLocalAcceptLogic
    << surrBasedLocalConstrRelax << surrBasedGlobalReplacePts;
  //<< branchBndNumSamplesRoot << branchBndNumSamplesNode

  // DOT and CONMIN
  s << minMaxType;

  // DL_SOLVER
  s << dlDetails;

  // NPSOL
  s << verifyLevel << functionPrecision << lineSearchTolerance;

  // NL2SOL
  s << absConvTol << xConvTol << singConvTol << singRadius << falseConvTol
    << initTRRadius << covarianceType << regressDiag;

  // OPT++
  s << searchMethod << gradientTolerance << maxStep << meritFn << centralPath
    << stepLenToBoundary << centeringParam << searchSchemeSize;

  // APPSPACK
  s << initStepLength << contractStepLength << threshStepLength
    << evalSynchronize << meritFunction << constrPenalty << smoothFactor;

  // COLINY
  s << evalSynchronization << constraintPenalty << constantPenalty
    << globalBalanceParam << localBalanceParam << maxBoxSize << minBoxSize
    << boxDivision << mutationAdaptive << showMiscOptions << miscOptions
    << solnTarget << crossoverRate << mutationRate << mutationScale
    << mutationMinScale << initDelta << threshDelta << contractFactor
    << newSolnsGenerated << numberRetained << expansionFlag
    << expandAfterSuccess << contractAfterFail << mutationRange
    << totalPatternSize << randomizeOrderFlag << selectionPressure
    << replacementType << crossoverType << mutationType << exploratoryMoves
    << patternBasis;

  // JEGA
  s << numCrossPoints << numParents << numOffspring << fitnessType
    << convergenceType << percentChange << numGenerations << fitnessLimit
    << shrinkagePercent << nichingType << nicheVector << postProcessorType
    << distanceVector;

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

  // NonD & DACE
  s << numSamples << numEmulatorSamples << fixedSeedFlag 
    << fixedSequenceFlag << previousSamples << vbdFlag << vbdDropTolerance;

  // NonD
  s << vbdControl << rngName << refinementType << refinementControl
    << nestingOverride << expansionType << expansionTerms << expansionOrder
    << expansionSamples << expansionSampleType << quadratureOrder
    << sparseGridLevel << cubIntOrder << collocationPoints << collocationRatio
    << collocPtReuse << probCollocFlag << expansionImportFile << sampleType
    << reliabilitySearchType << reliabilityIntegration << integrationRefine
    << nondOptAlgorithm << distributionType << responseLevelMappingType
    << responseLevels << probabilityLevels << reliabilityLevels
    << genReliabilityLevels << xObsDataFile << yObsDataFile << yStdDataFile
    << allVarsFlag << emulatorType << rejectionType << metropolisType;

  // Parameter Study
  s << finalPoint << stepVector << numSteps << stepsPerVariable << listOfPoints
    << varPartitions;

  // Verification
  s << refinementRate;
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
    dataMethodRep->referenceCount++;

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
      dataMethodRep->referenceCount++;
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
