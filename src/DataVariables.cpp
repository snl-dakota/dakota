/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

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
  numDiscreteStateSetRealVars(0)
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
    << normalUncUpperBnds << normalUncVars
    << normalUncInferredLowerBnds << normalUncInferredUpperBnds
    << lognormalUncLambdas
    << lognormalUncZetas << lognormalUncMeans << lognormalUncStdDevs
    << lognormalUncErrFacts << lognormalUncLowerBnds << lognormalUncUpperBnds
    << lognormalUncVars << lognormalUncInferredLowerBnds
    << lognormalUncInferredUpperBnds << uniformUncLowerBnds << uniformUncUpperBnds
    << uniformUncVars << loguniformUncLowerBnds << loguniformUncUpperBnds
    << loguniformUncVars << triangularUncModes << triangularUncLowerBnds
    << triangularUncUpperBnds << triangularUncVars << exponentialUncBetas
    << exponentialUncVars << exponentialUncLowerBnds << exponentialUncUpperBnds << betaUncAlphas << betaUncBetas << betaUncLowerBnds
    << betaUncUpperBnds << betaUncVars << gammaUncAlphas << gammaUncBetas
    << gammaUncVars << gammaUncLowerBnds << gammaUncUpperBnds << gumbelUncAlphas << gumbelUncBetas << gumbelUncVars << gumbelUncLowerBnds << gumbelUncUpperBnds
    << frechetUncAlphas << frechetUncBetas << frechetUncVars << frechetUncLowerBnds << frechetUncUpperBnds << weibullUncAlphas
    << weibullUncBetas << weibullUncVars << weibullUncLowerBnds << weibullUncUpperBnds << histogramUncBinPairs
    << histogramBinUncVars << histogramBinUncLowerBnds << histogramBinUncUpperBnds << poissonUncLambdas << poissonUncVars << poissonUncLowerBnds << poissonUncUpperBnds << poissonUncCat
    << binomialUncProbPerTrial << binomialUncNumTrials << binomialUncVars << binomialUncLowerBnds << binomialUncUpperBnds << binomialUncCat
    << negBinomialUncProbPerTrial << negBinomialUncNumTrials
    << negBinomialUncVars << negBinomialUncLowerBnds << negBinomialUncUpperBnds << negBinomialUncCat << geometricUncProbPerTrial << geometricUncVars << geometricUncLowerBnds << geometricUncUpperBnds << geometricUncCat
    << hyperGeomUncTotalPop << hyperGeomUncSelectedPop << hyperGeomUncNumDrawn
    << hyperGeomUncVars << hyperGeomUncLowerBnds << hyperGeomUncUpperBnds << hyperGeomUncCat 
    << histogramUncPointIntPairs << histogramPointIntUncVars << histogramPointIntUncLowerBnds << histogramPointIntUncUpperBnds << histogramUncPointIntCat 
    << histogramUncPointStrPairs << histogramPointStrUncVars << histogramPointStrUncLowerBnds << histogramPointStrUncUpperBnds
    << histogramUncPointRealPairs << histogramPointRealUncVars << histogramPointRealUncLowerBnds << histogramPointRealUncUpperBnds << histogramUncPointRealCat 
    << uncertainCorrelations;

  // Epistemic uncertain arrays
  s << continuousIntervalUncBasicProbs << continuousIntervalUncVars << continuousIntervalUncLowerBnds << continuousIntervalUncUpperBnds
    << discreteIntervalUncBasicProbs << discreteIntervalUncVars << discreteIntervalUncLowerBnds << discreteIntervalUncUpperBnds << discreteIntervalUncCat
    << discreteUncSetIntValuesProbs << discreteUncSetIntVars << discreteUncSetIntLowerBnds << discreteUncSetIntUpperBnds << discreteUncSetIntCat
    << discreteUncSetStrValuesProbs << discreteUncSetStrVars << discreteUncSetStrLowerBnds << discreteUncSetStrUpperBnds
    << discreteUncSetRealValuesProbs << discreteUncSetRealVars << discreteUncSetRealLowerBnds << discreteUncSetRealUpperBnds << discreteUncSetRealCat;

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

  // Per-type labels for uncertain variables
  s << normalUncLabels << lognormalUncLabels << uniformUncLabels
    << loguniformUncLabels << triangularUncLabels << exponentialUncLabels
    << betaUncLabels << gammaUncLabels << gumbelUncLabels
    << frechetUncLabels << weibullUncLabels << histogramBinUncLabels
    << poissonUncLabels << binomialUncLabels << negBinomialUncLabels
    << geometricUncLabels << hyperGeomUncLabels << histogramPointIntUncLabels
    << histogramPointStrUncLabels << histogramPointRealUncLabels
    << continuousIntervalUncLabels << discreteIntervalUncLabels
    << discreteUncSetIntLabels << discreteUncSetStrLabels << discreteUncSetRealLabels;

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
    >> normalUncUpperBnds >> normalUncVars
    >> normalUncInferredLowerBnds >> normalUncInferredUpperBnds
    >> lognormalUncLambdas
    >> lognormalUncZetas >> lognormalUncMeans >> lognormalUncStdDevs
    >> lognormalUncErrFacts >> lognormalUncLowerBnds >> lognormalUncUpperBnds
    >> lognormalUncVars >> lognormalUncInferredLowerBnds
    >> lognormalUncInferredUpperBnds >> uniformUncLowerBnds >> uniformUncUpperBnds
    >> uniformUncVars >> loguniformUncLowerBnds >> loguniformUncUpperBnds
    >> loguniformUncVars >> triangularUncModes >> triangularUncLowerBnds
    >> triangularUncUpperBnds >> triangularUncVars >> exponentialUncBetas
    >> exponentialUncVars >> exponentialUncLowerBnds >> exponentialUncUpperBnds >> betaUncAlphas >> betaUncBetas >> betaUncLowerBnds
    >> betaUncUpperBnds >> betaUncVars >> gammaUncAlphas >> gammaUncBetas
    >> gammaUncVars >> gammaUncLowerBnds >> gammaUncUpperBnds >> gumbelUncAlphas >> gumbelUncBetas >> gumbelUncVars >> gumbelUncLowerBnds >> gumbelUncUpperBnds
    >> frechetUncAlphas >> frechetUncBetas >> frechetUncVars >> frechetUncLowerBnds >> frechetUncUpperBnds >> weibullUncAlphas
    >> weibullUncBetas >> weibullUncVars >> weibullUncLowerBnds >> weibullUncUpperBnds >> histogramUncBinPairs
    >> histogramBinUncVars >> histogramBinUncLowerBnds >> histogramBinUncUpperBnds >> poissonUncLambdas >> poissonUncVars >> poissonUncLowerBnds >> poissonUncUpperBnds >> poissonUncCat
    >> binomialUncProbPerTrial >> binomialUncNumTrials >> binomialUncVars >> binomialUncLowerBnds >> binomialUncUpperBnds >> binomialUncCat
    >> negBinomialUncProbPerTrial >> negBinomialUncNumTrials
    >> negBinomialUncVars >> negBinomialUncLowerBnds >> negBinomialUncUpperBnds >> negBinomialUncCat >> geometricUncProbPerTrial >> geometricUncVars >> geometricUncLowerBnds >> geometricUncUpperBnds >> geometricUncCat
    >> hyperGeomUncTotalPop >> hyperGeomUncSelectedPop >> hyperGeomUncNumDrawn
    >> hyperGeomUncVars >> hyperGeomUncLowerBnds >> hyperGeomUncUpperBnds >>hyperGeomUncCat 
    >> histogramUncPointIntPairs >> histogramPointIntUncVars >> histogramPointIntUncLowerBnds >> histogramPointIntUncUpperBnds >> histogramUncPointIntCat 
    >> histogramUncPointStrPairs >> histogramPointStrUncVars >> histogramPointStrUncLowerBnds >> histogramPointStrUncUpperBnds
    >> histogramUncPointRealPairs >> histogramPointRealUncVars >> histogramPointRealUncLowerBnds >> histogramPointRealUncUpperBnds >> histogramUncPointRealCat
    >> uncertainCorrelations;

  // Epistemic uncertain arrays
  s >> continuousIntervalUncBasicProbs >> continuousIntervalUncVars >> continuousIntervalUncLowerBnds >> continuousIntervalUncUpperBnds
    >> discreteIntervalUncBasicProbs >> discreteIntervalUncVars >> discreteIntervalUncLowerBnds >> discreteIntervalUncUpperBnds >> discreteIntervalUncCat
    >> discreteUncSetIntValuesProbs >> discreteUncSetIntVars >> discreteUncSetIntLowerBnds >> discreteUncSetIntUpperBnds >> discreteUncSetIntCat
    >> discreteUncSetStrValuesProbs >> discreteUncSetStrVars >> discreteUncSetStrLowerBnds >> discreteUncSetStrUpperBnds
    >> discreteUncSetRealValuesProbs >> discreteUncSetRealVars >> discreteUncSetRealLowerBnds >> discreteUncSetRealUpperBnds >> discreteUncSetRealCat;

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

  // Per-type labels for uncertain variables
  s >> normalUncLabels >> lognormalUncLabels >> uniformUncLabels
    >> loguniformUncLabels >> triangularUncLabels >> exponentialUncLabels
    >> betaUncLabels >> gammaUncLabels >> gumbelUncLabels
    >> frechetUncLabels >> weibullUncLabels >> histogramBinUncLabels
    >> poissonUncLabels >> binomialUncLabels >> negBinomialUncLabels
    >> geometricUncLabels >> hyperGeomUncLabels >> histogramPointIntUncLabels
    >> histogramPointStrUncLabels >> histogramPointRealUncLabels
    >> continuousIntervalUncLabels >> discreteIntervalUncLabels
    >> discreteUncSetIntLabels >> discreteUncSetStrLabels >> discreteUncSetRealLabels;
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
    << normalUncUpperBnds << normalUncVars
    << normalUncInferredLowerBnds << normalUncInferredUpperBnds
    << lognormalUncLambdas
    << lognormalUncZetas << lognormalUncMeans << lognormalUncStdDevs
    << lognormalUncErrFacts << lognormalUncLowerBnds << lognormalUncUpperBnds
    << lognormalUncVars << lognormalUncInferredLowerBnds
    << lognormalUncInferredUpperBnds << uniformUncLowerBnds << uniformUncUpperBnds
    << uniformUncVars << loguniformUncLowerBnds << loguniformUncUpperBnds
    << loguniformUncVars << triangularUncModes << triangularUncLowerBnds
    << triangularUncUpperBnds << triangularUncVars << exponentialUncBetas
    << exponentialUncVars << exponentialUncLowerBnds << exponentialUncUpperBnds << betaUncAlphas << betaUncBetas << betaUncLowerBnds
    << betaUncUpperBnds << betaUncVars << gammaUncAlphas << gammaUncBetas
    << gammaUncVars << gammaUncLowerBnds << gammaUncUpperBnds << gumbelUncAlphas << gumbelUncBetas << gumbelUncVars << gumbelUncLowerBnds << gumbelUncUpperBnds
    << frechetUncAlphas << frechetUncBetas << frechetUncVars << frechetUncLowerBnds << frechetUncUpperBnds << weibullUncAlphas
    << weibullUncBetas << weibullUncVars << weibullUncLowerBnds << weibullUncUpperBnds << histogramUncBinPairs
    << histogramBinUncVars << histogramBinUncLowerBnds << histogramBinUncUpperBnds << poissonUncLambdas << poissonUncVars << poissonUncLowerBnds << poissonUncUpperBnds << poissonUncCat
    << binomialUncProbPerTrial << binomialUncNumTrials << binomialUncVars << binomialUncLowerBnds << binomialUncUpperBnds << binomialUncCat
    << negBinomialUncProbPerTrial << negBinomialUncNumTrials
    << negBinomialUncVars << negBinomialUncLowerBnds << negBinomialUncUpperBnds << negBinomialUncCat << geometricUncProbPerTrial << geometricUncVars << geometricUncLowerBnds << geometricUncUpperBnds << geometricUncCat
    << hyperGeomUncTotalPop << hyperGeomUncSelectedPop << hyperGeomUncNumDrawn
    << hyperGeomUncVars << hyperGeomUncLowerBnds << hyperGeomUncUpperBnds << hyperGeomUncCat 
    << histogramUncPointIntPairs << histogramPointIntUncVars << histogramPointIntUncLowerBnds << histogramPointIntUncUpperBnds << histogramUncPointIntCat 
    << histogramUncPointStrPairs << histogramPointStrUncVars << histogramPointStrUncLowerBnds << histogramPointStrUncUpperBnds
    << histogramUncPointRealPairs << histogramPointRealUncVars << histogramPointRealUncLowerBnds << histogramPointRealUncUpperBnds << histogramUncPointRealCat
    << uncertainCorrelations;

  // Epistemic uncertain arrays
  s << continuousIntervalUncBasicProbs << continuousIntervalUncVars << continuousIntervalUncLowerBnds << continuousIntervalUncUpperBnds
    << discreteIntervalUncBasicProbs << discreteIntervalUncVars << discreteIntervalUncLowerBnds << discreteIntervalUncUpperBnds << discreteIntervalUncCat
    << discreteUncSetIntValuesProbs << discreteUncSetIntVars << discreteUncSetIntLowerBnds << discreteUncSetIntUpperBnds << discreteUncSetIntCat
    << discreteUncSetStrValuesProbs << discreteUncSetStrVars << discreteUncSetStrLowerBnds << discreteUncSetStrUpperBnds
    << discreteUncSetRealValuesProbs << discreteUncSetRealVars << discreteUncSetRealLowerBnds << discreteUncSetRealUpperBnds << discreteUncSetRealCat;

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

  // Per-type labels for uncertain variables
  s << normalUncLabels << lognormalUncLabels << uniformUncLabels
    << loguniformUncLabels << triangularUncLabels << exponentialUncLabels
    << betaUncLabels << gammaUncLabels << gumbelUncLabels
    << frechetUncLabels << weibullUncLabels << histogramBinUncLabels
    << poissonUncLabels << binomialUncLabels << negBinomialUncLabels
    << geometricUncLabels << hyperGeomUncLabels << histogramPointIntUncLabels
    << histogramPointStrUncLabels << histogramPointRealUncLabels
    << continuousIntervalUncLabels << discreteIntervalUncLabels
    << discreteUncSetIntLabels << discreteUncSetStrLabels << discreteUncSetRealLabels;
  // Linear constraints
  s << linearIneqConstraintCoeffs << linearIneqLowerBnds << linearIneqUpperBnds
    << linearIneqScaleTypes << linearIneqScales << linearEqConstraintCoeffs
    << linearEqTargets << linearEqScaleTypes << linearEqScales;
}


DataVariables::DataVariables(): dataVarsRep(new DataVariablesRep())
{ /* empty ctor */ }


DataVariables::DataVariables(const DataVariables& data_vars):
  dataVarsRep(data_vars.dataVarsRep)
{ /* empty ctor */ }


DataVariables DataVariables::operator=(const DataVariables& data_vars)
{
  dataVarsRep = data_vars.dataVarsRep;
  return *this;
}


DataVariables::~DataVariables()
{ /* empty dtor */ }

} // namespace Dakota
