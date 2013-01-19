/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataVariables
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataVariables.hpp"
#include "data_io.hpp"


namespace Dakota {

// Default constructor:
DataVariablesRep::DataVariablesRep():
  varsView(DEFAULT_VIEW), varsDomain(DEFAULT_DOMAIN),
  numContinuousDesVars(0), numDiscreteDesRangeVars(0),
  numDiscreteDesSetIntVars(0), numDiscreteDesSetRealVars(0),
  numNormalUncVars(0), numLognormalUncVars(0), numUniformUncVars(0),
  numLoguniformUncVars(0), numTriangularUncVars(0), numExponentialUncVars(0),
  numBetaUncVars(0), numGammaUncVars(0), numGumbelUncVars(0),
  numFrechetUncVars(0), numWeibullUncVars(0), numHistogramBinUncVars(0),
  numPoissonUncVars(0), numBinomialUncVars(0), numNegBinomialUncVars(0),
  numGeometricUncVars(0), numHyperGeomUncVars(0), numHistogramPtUncVars(0),
  numContinuousIntervalUncVars(0), numDiscreteIntervalUncVars(0),
  numDiscreteUncSetIntVars(0), numDiscreteUncSetRealVars(0),
  numContinuousStateVars(0), numDiscreteStateRangeVars(0),
  numDiscreteStateSetIntVars(0), numDiscreteStateSetRealVars(0),
  referenceCount(1)
{ }


void DataVariablesRep::write(MPIPackBuffer& s) const
{
  s << idVariables << varsView << varsDomain << numContinuousDesVars
    << numDiscreteDesRangeVars << numDiscreteDesSetIntVars
    << numDiscreteDesSetRealVars << numNormalUncVars << numLognormalUncVars
    << numUniformUncVars << numLoguniformUncVars << numTriangularUncVars
    << numExponentialUncVars << numBetaUncVars << numGammaUncVars
    << numGumbelUncVars << numFrechetUncVars << numWeibullUncVars
    << numHistogramBinUncVars << numPoissonUncVars << numBinomialUncVars
    << numNegBinomialUncVars << numGeometricUncVars << numHyperGeomUncVars 
    << numHistogramPtUncVars << numContinuousIntervalUncVars
    << numDiscreteIntervalUncVars << numDiscreteUncSetIntVars
    << numDiscreteUncSetRealVars << numContinuousStateVars
    << numDiscreteStateRangeVars << numDiscreteStateSetIntVars
    << numDiscreteStateSetRealVars;

  // Design arrays
  s << continuousDesignVars         << continuousDesignLowerBnds
    << continuousDesignUpperBnds    << continuousDesignScaleTypes
    << continuousDesignScales       << discreteDesignRangeVars
    << discreteDesignRangeLowerBnds << discreteDesignRangeUpperBnds
    << discreteDesignSetIntVars     << discreteDesignSetRealVars
    << discreteDesignSetInt         << discreteDesignSetReal
    << continuousDesignLabels       << discreteDesignRangeLabels
    << discreteDesignSetIntLabels   << discreteDesignSetRealLabels;

  // Aleatory uncertain arrays
  s << normalUncMeans << normalUncStdDevs << normalUncLowerBnds
    << normalUncUpperBnds << lognormalUncLambdas << lognormalUncZetas
    << lognormalUncMeans << lognormalUncStdDevs << lognormalUncErrFacts
    << lognormalUncLowerBnds << lognormalUncUpperBnds << uniformUncLowerBnds
    << uniformUncUpperBnds << loguniformUncLowerBnds << loguniformUncUpperBnds
    << triangularUncModes << triangularUncLowerBnds << triangularUncUpperBnds
    << exponentialUncBetas << betaUncAlphas << betaUncBetas << betaUncLowerBnds
    << betaUncUpperBnds << gammaUncAlphas << gammaUncBetas << gumbelUncAlphas
    << gumbelUncBetas << frechetUncAlphas << frechetUncBetas << weibullUncAlphas
    << weibullUncBetas << histogramUncBinPairs << poissonUncLambdas
    << binomialUncProbPerTrial << binomialUncNumTrials 
    << negBinomialUncProbPerTrial << negBinomialUncNumTrials 
    << geometricUncProbPerTrial << hyperGeomUncTotalPop
    << hyperGeomUncSelectedPop << hyperGeomUncNumDrawn << histogramUncPointPairs
    << uncertainCorrelations;

  // Epistemic uncertain arrays
  s << continuousIntervalUncBasicProbs << continuousIntervalUncLowerBounds
    << continuousIntervalUncUpperBounds << discreteIntervalUncBasicProbs
    << discreteIntervalUncLowerBounds << discreteIntervalUncUpperBounds
    << discreteUncSetIntValuesProbs << discreteUncSetRealValuesProbs;

  // State arrays
  s << continuousStateVars         << continuousStateLowerBnds
    << continuousStateUpperBnds    << discreteStateRangeVars
    << discreteStateRangeLowerBnds << discreteStateRangeUpperBnds
    << discreteStateSetIntVars     << discreteStateSetRealVars
    << discreteStateSetInt         << discreteStateSetReal
    << continuousStateLabels       << discreteStateRangeLabels
    << discreteStateSetIntLabels   << discreteStateSetRealLabels;

  // Inferred arrays
  s << discreteDesignSetIntLowerBnds << discreteDesignSetIntUpperBnds
    << discreteDesignSetRealLowerBnds << discreteDesignSetRealUpperBnds
    << continuousAleatoryUncVars << continuousAleatoryUncLowerBnds
    << continuousAleatoryUncUpperBnds << continuousAleatoryUncLabels
    << discreteIntAleatoryUncVars << discreteIntAleatoryUncLowerBnds
    << discreteIntAleatoryUncUpperBnds << discreteIntAleatoryUncLabels
    << discreteRealAleatoryUncVars << discreteRealAleatoryUncLowerBnds
    << discreteRealAleatoryUncUpperBnds << discreteRealAleatoryUncLabels
    << continuousEpistemicUncVars << continuousEpistemicUncLowerBnds
    << continuousEpistemicUncUpperBnds << continuousEpistemicUncLabels
    << discreteIntEpistemicUncVars << discreteIntEpistemicUncLowerBnds
    << discreteIntEpistemicUncUpperBnds << discreteIntEpistemicUncLabels
    << discreteRealEpistemicUncVars << discreteRealEpistemicUncLowerBnds
    << discreteRealEpistemicUncUpperBnds << discreteRealEpistemicUncLabels
    << discreteStateSetIntLowerBnds << discreteStateSetIntUpperBnds
    << discreteStateSetRealLowerBnds << discreteStateSetRealUpperBnds;
}


void DataVariablesRep::read(MPIUnpackBuffer& s)
{
  s >> idVariables >> varsView >> varsDomain >> numContinuousDesVars
    >> numDiscreteDesRangeVars >> numDiscreteDesSetIntVars
    >> numDiscreteDesSetRealVars >> numNormalUncVars >> numLognormalUncVars
    >> numUniformUncVars >> numLoguniformUncVars >> numTriangularUncVars
    >> numExponentialUncVars >> numBetaUncVars >> numGammaUncVars
    >> numGumbelUncVars >> numFrechetUncVars >> numWeibullUncVars
    >> numHistogramBinUncVars >> numPoissonUncVars >> numBinomialUncVars
    >> numNegBinomialUncVars >> numGeometricUncVars >> numHyperGeomUncVars 
    >> numHistogramPtUncVars >> numContinuousIntervalUncVars
    >> numDiscreteIntervalUncVars >> numDiscreteUncSetIntVars
    >> numDiscreteUncSetRealVars >> numContinuousStateVars
    >> numDiscreteStateRangeVars >> numDiscreteStateSetIntVars
    >> numDiscreteStateSetRealVars;

  // Design arrays
  s >> continuousDesignVars         >> continuousDesignLowerBnds
    >> continuousDesignUpperBnds    >> continuousDesignScaleTypes
    >> continuousDesignScales       >> discreteDesignRangeVars
    >> discreteDesignRangeLowerBnds >> discreteDesignRangeUpperBnds
    >> discreteDesignSetIntVars     >> discreteDesignSetRealVars
    >> discreteDesignSetInt         >> discreteDesignSetReal
    >> continuousDesignLabels       >> discreteDesignRangeLabels
    >> discreteDesignSetIntLabels   >> discreteDesignSetRealLabels;

  // Aleatory uncertain arrays
  s >> normalUncMeans >> normalUncStdDevs >> normalUncLowerBnds
    >> normalUncUpperBnds >> lognormalUncLambdas >> lognormalUncZetas
    >> lognormalUncMeans >> lognormalUncStdDevs >> lognormalUncErrFacts
    >> lognormalUncLowerBnds >> lognormalUncUpperBnds >> uniformUncLowerBnds
    >> uniformUncUpperBnds >> loguniformUncLowerBnds >> loguniformUncUpperBnds
    >> triangularUncModes >> triangularUncLowerBnds >> triangularUncUpperBnds
    >> exponentialUncBetas >> betaUncAlphas >> betaUncBetas >> betaUncLowerBnds
    >> betaUncUpperBnds >> gammaUncAlphas >> gammaUncBetas >> gumbelUncAlphas
    >> gumbelUncBetas >> frechetUncAlphas >> frechetUncBetas >> weibullUncAlphas
    >> weibullUncBetas >> histogramUncBinPairs >> poissonUncLambdas
    >> binomialUncProbPerTrial >> binomialUncNumTrials 
    >> negBinomialUncProbPerTrial >> negBinomialUncNumTrials 
    >> geometricUncProbPerTrial >> hyperGeomUncTotalPop
    >> hyperGeomUncSelectedPop >> hyperGeomUncNumDrawn >> histogramUncPointPairs
    >> uncertainCorrelations;

  // Epistemic uncertain arrays
  s >> continuousIntervalUncBasicProbs >> continuousIntervalUncLowerBounds
    >> continuousIntervalUncUpperBounds >> discreteIntervalUncBasicProbs
    >> discreteIntervalUncLowerBounds >> discreteIntervalUncUpperBounds
    >> discreteUncSetIntValuesProbs >> discreteUncSetRealValuesProbs;

  // State arrays
  s >> continuousStateVars         >> continuousStateLowerBnds
    >> continuousStateUpperBnds    >> discreteStateRangeVars
    >> discreteStateRangeLowerBnds >> discreteStateRangeUpperBnds
    >> discreteStateSetIntVars     >> discreteStateSetRealVars
    >> discreteStateSetInt         >> discreteStateSetReal
    >> continuousStateLabels       >> discreteStateRangeLabels
    >> discreteStateSetIntLabels   >> discreteStateSetRealLabels;

  // Inferred arrays
  s >> discreteDesignSetIntLowerBnds >> discreteDesignSetIntUpperBnds
    >> discreteDesignSetRealLowerBnds >> discreteDesignSetRealUpperBnds
    >> continuousAleatoryUncVars >> continuousAleatoryUncLowerBnds
    >> continuousAleatoryUncUpperBnds >> continuousAleatoryUncLabels
    >> discreteIntAleatoryUncVars >> discreteIntAleatoryUncLowerBnds
    >> discreteIntAleatoryUncUpperBnds >> discreteIntAleatoryUncLabels
    >> discreteRealAleatoryUncVars >> discreteRealAleatoryUncLowerBnds
    >> discreteRealAleatoryUncUpperBnds >> discreteRealAleatoryUncLabels
    >> continuousEpistemicUncVars >> continuousEpistemicUncLowerBnds
    >> continuousEpistemicUncUpperBnds >> continuousEpistemicUncLabels
    >> discreteIntEpistemicUncVars >> discreteIntEpistemicUncLowerBnds
    >> discreteIntEpistemicUncUpperBnds >> discreteIntEpistemicUncLabels
    >> discreteRealEpistemicUncVars >> discreteRealEpistemicUncLowerBnds
    >> discreteRealEpistemicUncUpperBnds >> discreteRealEpistemicUncLabels
    >> discreteStateSetIntLowerBnds >> discreteStateSetIntUpperBnds
    >> discreteStateSetRealLowerBnds >> discreteStateSetRealUpperBnds;
}


void DataVariablesRep::write(std::ostream& s) const
{
  s << idVariables << varsView << varsDomain << numContinuousDesVars
    << numDiscreteDesRangeVars << numDiscreteDesSetIntVars
    << numDiscreteDesSetRealVars << numNormalUncVars << numLognormalUncVars
    << numUniformUncVars << numLoguniformUncVars << numTriangularUncVars
    << numExponentialUncVars << numBetaUncVars << numGammaUncVars
    << numGumbelUncVars << numFrechetUncVars << numWeibullUncVars
    << numHistogramBinUncVars << numPoissonUncVars << numBinomialUncVars
    << numNegBinomialUncVars << numGeometricUncVars << numHyperGeomUncVars 
    << numHistogramPtUncVars << numContinuousIntervalUncVars
    << numDiscreteIntervalUncVars << numDiscreteUncSetIntVars
    << numDiscreteUncSetRealVars << numContinuousStateVars
    << numDiscreteStateRangeVars << numDiscreteStateSetIntVars
    << numDiscreteStateSetRealVars;

  // Design arrays
  s << continuousDesignVars         << continuousDesignLowerBnds
    << continuousDesignUpperBnds    << continuousDesignScaleTypes
    << continuousDesignScales       << discreteDesignRangeVars
    << discreteDesignRangeLowerBnds << discreteDesignRangeUpperBnds
    << discreteDesignSetIntVars     << discreteDesignSetRealVars
    << discreteDesignSetInt         << discreteDesignSetReal
    << continuousDesignLabels       << discreteDesignRangeLabels
    << discreteDesignSetIntLabels   << discreteDesignSetRealLabels;

  // Aleatory uncertain arrays
  s << normalUncMeans << normalUncStdDevs << normalUncLowerBnds
    << normalUncUpperBnds << lognormalUncLambdas << lognormalUncZetas
    << lognormalUncMeans << lognormalUncStdDevs << lognormalUncErrFacts
    << lognormalUncLowerBnds << lognormalUncUpperBnds << uniformUncLowerBnds
    << uniformUncUpperBnds << loguniformUncLowerBnds << loguniformUncUpperBnds
    << triangularUncModes << triangularUncLowerBnds << triangularUncUpperBnds
    << exponentialUncBetas << betaUncAlphas << betaUncBetas << betaUncLowerBnds
    << betaUncUpperBnds << gammaUncAlphas << gammaUncBetas << gumbelUncAlphas
    << gumbelUncBetas << frechetUncAlphas << frechetUncBetas << weibullUncAlphas
    << weibullUncBetas << histogramUncBinPairs << poissonUncLambdas
    << binomialUncProbPerTrial << binomialUncNumTrials 
    << negBinomialUncProbPerTrial << negBinomialUncNumTrials 
    << geometricUncProbPerTrial << hyperGeomUncTotalPop
    << hyperGeomUncSelectedPop << hyperGeomUncNumDrawn << histogramUncPointPairs
    << uncertainCorrelations;

  // Epistemic uncertain arrays
  s << continuousIntervalUncBasicProbs << continuousIntervalUncLowerBounds
    << continuousIntervalUncUpperBounds << discreteIntervalUncBasicProbs
    << discreteIntervalUncLowerBounds << discreteIntervalUncUpperBounds
    << discreteUncSetIntValuesProbs << discreteUncSetRealValuesProbs;

  // State arrays
  s << continuousStateVars         << continuousStateLowerBnds
    << continuousStateUpperBnds    << discreteStateRangeVars
    << discreteStateRangeLowerBnds << discreteStateRangeUpperBnds
    << discreteStateSetIntVars     << discreteStateSetRealVars
    << discreteStateSetInt         << discreteStateSetReal
    << continuousStateLabels       << discreteStateRangeLabels
    << discreteStateSetIntLabels   << discreteStateSetRealLabels;

  // Inferred arrays
  s << discreteDesignSetIntLowerBnds << discreteDesignSetIntUpperBnds
    << discreteDesignSetRealLowerBnds << discreteDesignSetRealUpperBnds
    << continuousAleatoryUncVars << continuousAleatoryUncLowerBnds
    << continuousAleatoryUncUpperBnds << continuousAleatoryUncLabels
    << discreteIntAleatoryUncVars << discreteIntAleatoryUncLowerBnds
    << discreteIntAleatoryUncUpperBnds << discreteIntAleatoryUncLabels
    << discreteRealAleatoryUncVars << discreteRealAleatoryUncLowerBnds
    << discreteRealAleatoryUncUpperBnds << discreteRealAleatoryUncLabels
    << continuousEpistemicUncVars << continuousEpistemicUncLowerBnds
    << continuousEpistemicUncUpperBnds << continuousEpistemicUncLabels
    << discreteIntEpistemicUncVars << discreteIntEpistemicUncLowerBnds
    << discreteIntEpistemicUncUpperBnds << discreteIntEpistemicUncLabels
    << discreteRealEpistemicUncVars << discreteRealEpistemicUncLowerBnds
    << discreteRealEpistemicUncUpperBnds << discreteRealEpistemicUncLabels
    << discreteStateSetIntLowerBnds << discreteStateSetIntUpperBnds
    << discreteStateSetRealLowerBnds << discreteStateSetRealUpperBnds;
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
    dataVarsRep->referenceCount++;

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
      dataVarsRep->referenceCount++;
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
