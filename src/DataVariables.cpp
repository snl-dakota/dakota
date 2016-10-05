/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataVariables
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataVariables.hpp"
#include "dakota_data_io.hpp"


namespace Dakota {

// Default constructor:
DataVariablesRep::DataVariablesRep():
  varsView(DEFAULT_VIEW), varsDomain(DEFAULT_DOMAIN),
  uncertainVarsInitPt(false), numContinuousDesVars(0),
  numDiscreteDesRangeVars(0), numDiscreteDesSetIntVars(0), numDiscreteDesSetStrVars(0),
  numDiscreteDesSetRealVars(0), numNormalUncVars(0), numLognormalUncVars(0),
  numUniformUncVars(0), numLoguniformUncVars(0), numTriangularUncVars(0),
  numExponentialUncVars(0), numBetaUncVars(0), numGammaUncVars(0),
  numGumbelUncVars(0), numFrechetUncVars(0), numWeibullUncVars(0),
  numHistogramBinUncVars(0), numPoissonUncVars(0), numBinomialUncVars(0),
  numNegBinomialUncVars(0), numGeometricUncVars(0), numHyperGeomUncVars(0),
  numHistogramPtIntUncVars(0), numHistogramPtStrUncVars(0), numHistogramPtRealUncVars(0), 
  numContinuousIntervalUncVars(0),
  numDiscreteIntervalUncVars(0), numDiscreteUncSetIntVars(0), numDiscreteUncSetStrVars(0),
  numDiscreteUncSetRealVars(0), numContinuousStateVars(0),
  numDiscreteStateRangeVars(0), numDiscreteStateSetIntVars(0), numDiscreteStateSetStrVars(0),
  numDiscreteStateSetRealVars(0), referenceCount(1)
{ }


void DataVariablesRep::write(MPIPackBuffer& s) const
{
  s << idVariables << varsView << varsDomain << uncertainVarsInitPt
    << numContinuousDesVars << numDiscreteDesRangeVars
    << numDiscreteDesSetIntVars << numDiscreteDesSetStrVars << numDiscreteDesSetRealVars << numNormalUncVars
    << numLognormalUncVars << numUniformUncVars << numLoguniformUncVars
    << numTriangularUncVars << numExponentialUncVars << numBetaUncVars
    << numGammaUncVars << numGumbelUncVars << numFrechetUncVars
    << numWeibullUncVars << numHistogramBinUncVars << numPoissonUncVars
    << numBinomialUncVars << numNegBinomialUncVars << numGeometricUncVars
    << numHyperGeomUncVars << numHistogramPtIntUncVars << numHistogramPtStrUncVars << numHistogramPtRealUncVars
    << numContinuousIntervalUncVars << numDiscreteIntervalUncVars
    << numDiscreteUncSetIntVars << numDiscreteUncSetStrVars << numDiscreteUncSetRealVars
    << numContinuousStateVars << numDiscreteStateRangeVars
    << numDiscreteStateSetIntVars << numDiscreteStateSetStrVars << numDiscreteStateSetRealVars;

  // Design arrays
  s << continuousDesignVars         << continuousDesignLowerBnds
    << continuousDesignUpperBnds    << continuousDesignScaleTypes
    << continuousDesignScales       << discreteDesignRangeVars
    << discreteDesignRangeLowerBnds << discreteDesignRangeUpperBnds << discreteDesignRangeCat
    << discreteDesignSetIntVars     << discreteDesignSetStrVars  << discreteDesignSetRealVars
    << discreteDesignSetInt         << discreteDesignSetStr     << discreteDesignSetReal
    << discreteDesignSetIntCat      << discreteDesignSetRealCat
    << discreteDesignSetIntAdj      << discreteDesignSetStrAdj << discreteDesignSetRealAdj
    << continuousDesignLabels       << discreteDesignRangeLabels
    << discreteDesignSetIntLabels   << discreteDesignSetStrLabels << discreteDesignSetRealLabels;

  // Aleatory uncertain arrays
  s << normalUncMeans << normalUncStdDevs << normalUncLowerBnds
    << normalUncUpperBnds << normalUncVars << lognormalUncLambdas
    << lognormalUncZetas << lognormalUncMeans << lognormalUncStdDevs
    << lognormalUncErrFacts << lognormalUncLowerBnds << lognormalUncUpperBnds
    << lognormalUncVars << uniformUncLowerBnds << uniformUncUpperBnds
    << uniformUncVars << loguniformUncLowerBnds << loguniformUncUpperBnds
    << loguniformUncVars << triangularUncModes << triangularUncLowerBnds
    << triangularUncUpperBnds << triangularUncVars << exponentialUncBetas
    << exponentialUncVars << betaUncAlphas << betaUncBetas << betaUncLowerBnds
    << betaUncUpperBnds << betaUncVars << gammaUncAlphas << gammaUncBetas
    << gammaUncVars << gumbelUncAlphas << gumbelUncBetas << gumbelUncVars
    << frechetUncAlphas << frechetUncBetas << frechetUncVars << weibullUncAlphas
    << weibullUncBetas << weibullUncVars << histogramUncBinPairs
    << histogramBinUncVars << poissonUncLambdas << poissonUncVars << poissonUncCat
    << binomialUncProbPerTrial << binomialUncNumTrials << binomialUncVars << binomialUncCat
    << negBinomialUncProbPerTrial << negBinomialUncNumTrials
    << negBinomialUncVars << negBinomialUncCat << geometricUncProbPerTrial << geometricUncVars << geometricUncCat
    << hyperGeomUncTotalPop << hyperGeomUncSelectedPop << hyperGeomUncNumDrawn
    << hyperGeomUncVars << hyperGeomUncCat 
    << histogramUncPointIntPairs << histogramPointIntUncVars << histogramUncPointIntCat 
    << histogramUncPointStrPairs << histogramPointStrUncVars
    << histogramUncPointRealPairs << histogramPointRealUncVars << histogramUncPointRealCat 
    << uncertainCorrelations;

  // Epistemic uncertain arrays
  s << continuousIntervalUncBasicProbs << continuousIntervalUncVars
    << discreteIntervalUncBasicProbs << discreteIntervalUncVars << discreteIntervalUncCat
    << discreteUncSetIntValuesProbs << discreteUncSetIntVars << discreteUncSetIntCat
    << discreteUncSetStrValuesProbs << discreteUncSetStrVars
    << discreteUncSetRealValuesProbs << discreteUncSetRealVars << discreteUncSetRealCat;

  // State arrays
  s << continuousStateVars         << continuousStateLowerBnds
    << continuousStateUpperBnds    << discreteStateRangeVars
    << discreteStateRangeLowerBnds << discreteStateRangeUpperBnds << discreteStateRangeCat
    << discreteStateSetIntVars     << discreteStateSetStrVars << discreteStateSetRealVars
    << discreteStateSetInt         << discreteStateSetStr     << discreteStateSetReal << discreteStateSetIntCat << discreteStateSetRealCat
    << continuousStateLabels       << discreteStateRangeLabels
    << discreteStateSetIntLabels   << discreteStateSetStrLabels << discreteStateSetRealLabels;

  // Inferred arrays
  s << discreteDesignSetIntLowerBnds << discreteDesignSetIntUpperBnds
    << discreteDesignSetStrLowerBnds << discreteDesignSetStrUpperBnds
    << discreteDesignSetRealLowerBnds << discreteDesignSetRealUpperBnds
    << continuousAleatoryUncVars << continuousAleatoryUncLowerBnds
    << continuousAleatoryUncUpperBnds << continuousAleatoryUncLabels
    << discreteIntAleatoryUncVars << discreteIntAleatoryUncLowerBnds
    << discreteIntAleatoryUncUpperBnds << discreteIntAleatoryUncLabels
    << discreteStrAleatoryUncVars << discreteStrAleatoryUncLowerBnds
    << discreteStrAleatoryUncUpperBnds << discreteStrAleatoryUncLabels
    << discreteRealAleatoryUncVars << discreteRealAleatoryUncLowerBnds
    << discreteRealAleatoryUncUpperBnds << discreteRealAleatoryUncLabels
    << continuousEpistemicUncVars << continuousEpistemicUncLowerBnds
    << continuousEpistemicUncUpperBnds << continuousEpistemicUncLabels
    << discreteIntEpistemicUncVars << discreteIntEpistemicUncLowerBnds
    << discreteIntEpistemicUncUpperBnds << discreteIntEpistemicUncLabels
    << discreteStrEpistemicUncVars << discreteStrEpistemicUncLowerBnds
    << discreteStrEpistemicUncUpperBnds << discreteStrEpistemicUncLabels
    << discreteRealEpistemicUncVars << discreteRealEpistemicUncLowerBnds
    << discreteRealEpistemicUncUpperBnds << discreteRealEpistemicUncLabels
    << discreteStateSetIntLowerBnds << discreteStateSetIntUpperBnds
    << discreteStateSetStrLowerBnds << discreteStateSetStrUpperBnds
    << discreteStateSetRealLowerBnds << discreteStateSetRealUpperBnds;

  // Linear constraints
  s << linearIneqConstraintCoeffs << linearIneqLowerBnds << linearIneqUpperBnds
    << linearIneqScaleTypes << linearIneqScales << linearEqConstraintCoeffs
    << linearEqTargets << linearEqScaleTypes << linearEqScales;
}


void DataVariablesRep::read(MPIUnpackBuffer& s)
{
  s >> idVariables >> varsView >> varsDomain >> uncertainVarsInitPt
    >> numContinuousDesVars >> numDiscreteDesRangeVars
    >> numDiscreteDesSetIntVars >> numDiscreteDesSetStrVars >> numDiscreteDesSetRealVars >> numNormalUncVars
    >> numLognormalUncVars >> numUniformUncVars >> numLoguniformUncVars
    >> numTriangularUncVars >> numExponentialUncVars >> numBetaUncVars
    >> numGammaUncVars >> numGumbelUncVars >> numFrechetUncVars
    >> numWeibullUncVars >> numHistogramBinUncVars >> numPoissonUncVars
    >> numBinomialUncVars >> numNegBinomialUncVars >> numGeometricUncVars
    >> numHyperGeomUncVars >> numHistogramPtIntUncVars >> numHistogramPtStrUncVars >> numHistogramPtRealUncVars
    >> numContinuousIntervalUncVars >> numDiscreteIntervalUncVars
    >> numDiscreteUncSetIntVars >> numDiscreteUncSetStrVars >> numDiscreteUncSetRealVars
    >> numContinuousStateVars >> numDiscreteStateRangeVars
    >> numDiscreteStateSetIntVars >> numDiscreteStateSetStrVars >> numDiscreteStateSetRealVars;

  // Design arrays
  s >> continuousDesignVars         >> continuousDesignLowerBnds
    >> continuousDesignUpperBnds    >> continuousDesignScaleTypes
    >> continuousDesignScales       >> discreteDesignRangeVars
    >> discreteDesignRangeLowerBnds >> discreteDesignRangeUpperBnds >> discreteDesignRangeCat
    >> discreteDesignSetIntVars     >> discreteDesignSetStrVars >> discreteDesignSetRealVars
    >> discreteDesignSetInt         >> discreteDesignSetStr >> discreteDesignSetReal
    >> discreteDesignSetIntCat      >> discreteDesignSetRealCat
    >> discreteDesignSetIntAdj      >> discreteDesignSetStrAdj >> discreteDesignSetRealAdj
    >> continuousDesignLabels       >> discreteDesignRangeLabels
    >> discreteDesignSetIntLabels   >> discreteDesignSetStrLabels >> discreteDesignSetRealLabels;

  // Aleatory uncertain arrays
  s >> normalUncMeans >> normalUncStdDevs >> normalUncLowerBnds
    >> normalUncUpperBnds >> normalUncVars >> lognormalUncLambdas
    >> lognormalUncZetas >> lognormalUncMeans >> lognormalUncStdDevs
    >> lognormalUncErrFacts >> lognormalUncLowerBnds >> lognormalUncUpperBnds
    >> lognormalUncVars >> uniformUncLowerBnds >> uniformUncUpperBnds
    >> uniformUncVars >> loguniformUncLowerBnds >> loguniformUncUpperBnds
    >> loguniformUncVars >> triangularUncModes >> triangularUncLowerBnds
    >> triangularUncUpperBnds >> triangularUncVars >> exponentialUncBetas
    >> exponentialUncVars >> betaUncAlphas >> betaUncBetas >> betaUncLowerBnds
    >> betaUncUpperBnds >> betaUncVars >> gammaUncAlphas >> gammaUncBetas
    >> gammaUncVars >> gumbelUncAlphas >> gumbelUncBetas >> gumbelUncVars
    >> frechetUncAlphas >> frechetUncBetas >> frechetUncVars >> weibullUncAlphas
    >> weibullUncBetas >> weibullUncVars >> histogramUncBinPairs
    >> histogramBinUncVars >> poissonUncLambdas >> poissonUncVars >> poissonUncCat
    >> binomialUncProbPerTrial >> binomialUncNumTrials >> binomialUncVars >> binomialUncCat
    >> negBinomialUncProbPerTrial >> negBinomialUncNumTrials
    >> negBinomialUncVars >> negBinomialUncCat >> geometricUncProbPerTrial >> geometricUncVars >> geometricUncCat
    >> hyperGeomUncTotalPop >> hyperGeomUncSelectedPop >> hyperGeomUncNumDrawn
    >> hyperGeomUncVars >>hyperGeomUncCat 
    >> histogramUncPointIntPairs >> histogramPointIntUncVars >> histogramUncPointIntCat 
    >> histogramUncPointStrPairs >> histogramPointStrUncVars
    >> histogramUncPointRealPairs >> histogramPointRealUncVars >> histogramUncPointRealCat
    >> uncertainCorrelations;

  // Epistemic uncertain arrays
  s >> continuousIntervalUncBasicProbs >> continuousIntervalUncVars
    >> discreteIntervalUncBasicProbs >> discreteIntervalUncVars >> discreteIntervalUncCat
    >> discreteUncSetIntValuesProbs >> discreteUncSetIntVars >> discreteUncSetIntCat
    >> discreteUncSetStrValuesProbs >> discreteUncSetStrVars
    >> discreteUncSetRealValuesProbs >> discreteUncSetRealVars >> discreteUncSetRealCat;

  // State arrays
  s >> continuousStateVars         >> continuousStateLowerBnds
    >> continuousStateUpperBnds    >> discreteStateRangeVars
    >> discreteStateRangeLowerBnds >> discreteStateRangeUpperBnds >> discreteStateRangeCat
    >> discreteStateSetIntVars     >> discreteStateSetStrVars >> discreteStateSetRealVars
    >> discreteStateSetInt         >> discreteStateSetStr     >> discreteStateSetReal >> discreteStateSetIntCat >> discreteStateSetRealCat
    >> continuousStateLabels       >> discreteStateRangeLabels
    >> discreteStateSetIntLabels   >> discreteStateSetStrLabels >> discreteStateSetRealLabels;

  // Inferred arrays
  s >> discreteDesignSetIntLowerBnds >> discreteDesignSetIntUpperBnds
    >> discreteDesignSetStrLowerBnds >> discreteDesignSetStrUpperBnds
    >> discreteDesignSetRealLowerBnds >> discreteDesignSetRealUpperBnds
    >> continuousAleatoryUncVars >> continuousAleatoryUncLowerBnds
    >> continuousAleatoryUncUpperBnds >> continuousAleatoryUncLabels
    >> discreteIntAleatoryUncVars >> discreteIntAleatoryUncLowerBnds
    >> discreteIntAleatoryUncUpperBnds >> discreteIntAleatoryUncLabels
    >> discreteStrAleatoryUncVars >> discreteStrAleatoryUncLowerBnds
    >> discreteStrAleatoryUncUpperBnds >> discreteStrAleatoryUncLabels
    >> discreteRealAleatoryUncVars >> discreteRealAleatoryUncLowerBnds
    >> discreteRealAleatoryUncUpperBnds >> discreteRealAleatoryUncLabels
    >> continuousEpistemicUncVars >> continuousEpistemicUncLowerBnds
    >> continuousEpistemicUncUpperBnds >> continuousEpistemicUncLabels
    >> discreteIntEpistemicUncVars >> discreteIntEpistemicUncLowerBnds
    >> discreteIntEpistemicUncUpperBnds >> discreteIntEpistemicUncLabels
    >> discreteStrEpistemicUncVars >> discreteStrEpistemicUncLowerBnds
    >> discreteStrEpistemicUncUpperBnds >> discreteStrEpistemicUncLabels
    >> discreteRealEpistemicUncVars >> discreteRealEpistemicUncLowerBnds
    >> discreteRealEpistemicUncUpperBnds >> discreteRealEpistemicUncLabels
    >> discreteStateSetIntLowerBnds >> discreteStateSetIntUpperBnds
    >> discreteStateSetStrLowerBnds >> discreteStateSetStrUpperBnds
    >> discreteStateSetRealLowerBnds >> discreteStateSetRealUpperBnds;

  // Linear constraints
  s >> linearIneqConstraintCoeffs >> linearIneqLowerBnds >> linearIneqUpperBnds
    >> linearIneqScaleTypes >> linearIneqScales >> linearEqConstraintCoeffs
    >> linearEqTargets >> linearEqScaleTypes >> linearEqScales;
}


void DataVariablesRep::write(std::ostream& s) const
{
  s << idVariables << varsView << varsDomain << uncertainVarsInitPt
    << numContinuousDesVars << numDiscreteDesRangeVars
    << numDiscreteDesSetIntVars << numDiscreteDesSetStrVars << numDiscreteDesSetRealVars << numNormalUncVars
    << numLognormalUncVars << numUniformUncVars << numLoguniformUncVars
    << numTriangularUncVars << numExponentialUncVars << numBetaUncVars
    << numGammaUncVars << numGumbelUncVars << numFrechetUncVars
    << numWeibullUncVars << numHistogramBinUncVars << numPoissonUncVars
    << numBinomialUncVars << numNegBinomialUncVars << numGeometricUncVars
    << numHyperGeomUncVars << numHistogramPtIntUncVars << numHistogramPtStrUncVars << numHistogramPtRealUncVars
    << numContinuousIntervalUncVars << numDiscreteIntervalUncVars
    << numDiscreteUncSetIntVars << numDiscreteUncSetStrVars << numDiscreteUncSetRealVars
    << numContinuousStateVars << numDiscreteStateRangeVars
    << numDiscreteStateSetIntVars << numDiscreteStateSetStrVars << numDiscreteStateSetRealVars;

  // Design arrays
  s << continuousDesignVars         << continuousDesignLowerBnds
    << continuousDesignUpperBnds    << continuousDesignScaleTypes
    << continuousDesignScales       << discreteDesignRangeVars
    << discreteDesignRangeLowerBnds << discreteDesignRangeUpperBnds << discreteDesignRangeCat
    << discreteDesignSetIntVars     << discreteDesignSetStrVars << discreteDesignSetRealVars
    << discreteDesignSetInt         << discreteDesignSetStr     << discreteDesignSetReal
    << discreteDesignSetIntCat      << discreteDesignSetRealCat
    << continuousDesignLabels       << discreteDesignRangeLabels
    << discreteDesignSetIntLabels   << discreteDesignSetStrLabels << discreteDesignSetRealLabels;

  // Aleatory uncertain arrays
  s << normalUncMeans << normalUncStdDevs << normalUncLowerBnds
    << normalUncUpperBnds << normalUncVars << lognormalUncLambdas
    << lognormalUncZetas << lognormalUncMeans << lognormalUncStdDevs
    << lognormalUncErrFacts << lognormalUncLowerBnds << lognormalUncUpperBnds
    << lognormalUncVars << uniformUncLowerBnds << uniformUncUpperBnds
    << uniformUncVars << loguniformUncLowerBnds << loguniformUncUpperBnds
    << loguniformUncVars << triangularUncModes << triangularUncLowerBnds
    << triangularUncUpperBnds << triangularUncVars << exponentialUncBetas
    << exponentialUncVars << betaUncAlphas << betaUncBetas << betaUncLowerBnds
    << betaUncUpperBnds << betaUncVars << gammaUncAlphas << gammaUncBetas
    << gammaUncVars << gumbelUncAlphas << gumbelUncBetas << gumbelUncVars
    << frechetUncAlphas << frechetUncBetas << frechetUncVars << weibullUncAlphas
    << weibullUncBetas << weibullUncVars << histogramUncBinPairs
    << histogramBinUncVars << poissonUncLambdas << poissonUncVars << poissonUncCat
    << binomialUncProbPerTrial << binomialUncNumTrials << binomialUncVars << binomialUncCat
    << negBinomialUncProbPerTrial << negBinomialUncNumTrials
    << negBinomialUncVars << negBinomialUncCat << geometricUncProbPerTrial << geometricUncVars << geometricUncCat
    << hyperGeomUncTotalPop << hyperGeomUncSelectedPop << hyperGeomUncNumDrawn
    << hyperGeomUncVars << hyperGeomUncCat 
    << histogramUncPointIntPairs << histogramPointIntUncVars << histogramUncPointIntCat 
    << histogramUncPointStrPairs << histogramPointStrUncVars
    << histogramUncPointRealPairs << histogramPointRealUncVars << histogramUncPointRealCat
    << uncertainCorrelations;

  // Epistemic uncertain arrays
  s << continuousIntervalUncBasicProbs << continuousIntervalUncVars
    << discreteIntervalUncBasicProbs << discreteIntervalUncVars << discreteIntervalUncCat
    << discreteUncSetIntValuesProbs << discreteUncSetIntVars << discreteUncSetIntCat
    << discreteUncSetStrValuesProbs << discreteUncSetStrVars
    << discreteUncSetRealValuesProbs << discreteUncSetRealVars << discreteUncSetRealCat;

  // State arrays
  s << continuousStateVars         << continuousStateLowerBnds
    << continuousStateUpperBnds    << discreteStateRangeVars << discreteStateRangeCat
    << discreteStateRangeLowerBnds << discreteStateRangeUpperBnds
    << discreteStateSetIntVars     << discreteStateSetStrVars << discreteStateSetRealVars
    << discreteStateSetInt         << discreteStateSetStr << discreteStateSetReal << discreteStateSetIntCat << discreteStateSetRealCat
    << continuousStateLabels       << discreteStateRangeLabels
    << discreteStateSetIntLabels   << discreteStateSetStrLabels << discreteStateSetRealLabels;

  // Inferred arrays
  s << discreteDesignSetIntLowerBnds << discreteDesignSetIntUpperBnds
    << discreteDesignSetStrLowerBnds << discreteDesignSetStrUpperBnds
    << discreteDesignSetRealLowerBnds << discreteDesignSetRealUpperBnds
    << continuousAleatoryUncVars << continuousAleatoryUncLowerBnds
    << continuousAleatoryUncUpperBnds << continuousAleatoryUncLabels
    << discreteIntAleatoryUncVars << discreteIntAleatoryUncLowerBnds
    << discreteIntAleatoryUncUpperBnds << discreteIntAleatoryUncLabels
    << discreteStrAleatoryUncVars << discreteStrAleatoryUncLowerBnds
    << discreteStrAleatoryUncUpperBnds << discreteStrAleatoryUncLabels
    << discreteRealAleatoryUncVars << discreteRealAleatoryUncLowerBnds
    << discreteRealAleatoryUncUpperBnds << discreteRealAleatoryUncLabels
    << continuousEpistemicUncVars << continuousEpistemicUncLowerBnds
    << continuousEpistemicUncUpperBnds << continuousEpistemicUncLabels
    << discreteIntEpistemicUncVars << discreteIntEpistemicUncLowerBnds
    << discreteIntEpistemicUncUpperBnds << discreteIntEpistemicUncLabels
    << discreteStrEpistemicUncVars << discreteStrEpistemicUncLowerBnds
    << discreteStrEpistemicUncUpperBnds << discreteStrEpistemicUncLabels
    << discreteRealEpistemicUncVars << discreteRealEpistemicUncLowerBnds
    << discreteRealEpistemicUncUpperBnds << discreteRealEpistemicUncLabels
    << discreteStateSetIntLowerBnds << discreteStateSetIntUpperBnds
    << discreteStateSetStrLowerBnds << discreteStateSetStrUpperBnds
    << discreteStateSetRealLowerBnds << discreteStateSetRealUpperBnds;

  // Linear constraints
  s << linearIneqConstraintCoeffs << linearIneqLowerBnds << linearIneqUpperBnds
    << linearIneqScaleTypes << linearIneqScales << linearEqConstraintCoeffs
    << linearEqTargets << linearEqScaleTypes << linearEqScales;
}


DataVariables::DataVariables(): dataVarsRep(new DataVariablesRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataVariables::DataVariables(), dataVarsRep referenceCount = "
       << dataVarsRep->referenceCount << std::endl;
#endif
}


DataVariables::DataVariables(const DataVariables& data_vars)
{
  // Increment new (no old to decrement)
  dataVarsRep = data_vars.dataVarsRep;
  if (dataVarsRep) // Check for an assignment of NULL
    ++dataVarsRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "DataVariables::DataVariables(DataVariables&)" << std::endl;
  if (dataVarsRep)
    Cout << "dataVarsRep referenceCount = " << dataVarsRep->referenceCount
	 << std::endl;
#endif
}


DataVariables DataVariables::operator=(const DataVariables& data_vars)
{
  if (dataVarsRep != data_vars.dataVarsRep) { // normal case: old != new
    // Decrement old
    if (dataVarsRep) // Check for NULL
      if ( --dataVarsRep->referenceCount == 0 ) 
	delete dataVarsRep;
    // Assign and increment new
    dataVarsRep = data_vars.dataVarsRep;
    if (dataVarsRep) // Check for NULL
      ++dataVarsRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataVariables::operator=(DataVariables&)" << std::endl;
  if (dataVarsRep)
    Cout << "dataVarsRep referenceCount = " << dataVarsRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


DataVariables::~DataVariables()
{
  if (dataVarsRep) { // Check for NULL
    --dataVarsRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataVarsRep referenceCount decremented to "
         << dataVarsRep->referenceCount << std::endl;
#endif
    if (dataVarsRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataVarsRep" << std::endl;
#endif
      delete dataVarsRep;
    }
  }
}

} // namespace Dakota
