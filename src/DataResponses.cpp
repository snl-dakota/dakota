/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataResponses.hpp"

#include "dakota_data_io.hpp"

namespace Dakota {

// Default constructor:
DataResponsesRep::DataResponsesRep()
    : numObjectiveFunctions(0),
      numLeastSqTerms(0),
      numNonlinearIneqConstraints(0),
      numNonlinearEqConstraints(0),
      numResponseFunctions(0),
      numScalarObjectiveFunctions(0),
      numScalarLeastSqTerms(0),
      numScalarNonlinearIneqConstraints(0),
      numScalarNonlinearEqConstraints(0),
      numScalarResponseFunctions(0),
      numFieldObjectiveFunctions(0),
      numFieldLeastSqTerms(0),
      numFieldNonlinearIneqConstraints(0),
      numFieldNonlinearEqConstraints(0),
      numFieldResponseFunctions(0),
      calibrationDataFlag(false),
      numExperiments(1),
      numExpConfigVars(0),
      scalarDataFormat(TABULAR_EXPER_ANNOT),
      ignoreBounds(false),
      centralHess(false),
      methodSource("dakota"),
      intervalType("forward"),
      interpolateFlag(false),
      fdGradStepType("relative"),
      fdHessStepType("relative"),
      readFieldCoords(false) {}

void DataResponsesRep::write(MPIPackBuffer& s) const {
  s << idResponses
    << responseLabels
    // counts
    << numObjectiveFunctions << numLeastSqTerms << numNonlinearIneqConstraints
    << numNonlinearEqConstraints << numResponseFunctions
    << numScalarObjectiveFunctions << numScalarLeastSqTerms
    << numScalarNonlinearIneqConstraints << numScalarNonlinearEqConstraints
    << numScalarResponseFunctions << numFieldObjectiveFunctions
    << numFieldLeastSqTerms << numFieldNonlinearIneqConstraints
    << numFieldNonlinearEqConstraints
    << numFieldResponseFunctions
    // weights, bounds, targets
    << primaryRespFnSense << primaryRespFnWeights << nonlinearIneqLowerBnds
    << nonlinearIneqUpperBnds
    << nonlinearEqTargets
    // scaling data
    << primaryRespFnScaleTypes << primaryRespFnScales << nonlinearIneqScaleTypes
    << nonlinearIneqScales << nonlinearEqScaleTypes
    << nonlinearEqScales
    // experimental data
    << calibrationDataFlag << numExperiments << numExpConfigVars
    << expConfigVars << simVariance << expObservations << expStdDeviations
    << scalarDataFileName
    << scalarDataFormat
    // derivative settings
    << gradientType << hessianType << ignoreBounds << centralHess
    << quasiHessianType << methodSource << intervalType << interpolateFlag
    << fdGradStepSize << fdGradStepType << fdHessStepSize << fdHessStepType
    << idNumericalGrads << idAnalyticGrads << idNumericalHessians
    << idQuasiHessians
    << idAnalyticHessians
    // field data
    << fieldLengths << numCoordsPerField << readFieldCoords << varianceType
    << metadataLabels;
}

void DataResponsesRep::read(MPIUnpackBuffer& s) {
  s >> idResponses >> responseLabels
      // counts
      >> numObjectiveFunctions >> numLeastSqTerms >>
      numNonlinearIneqConstraints >> numNonlinearEqConstraints >>
      numResponseFunctions >> numScalarObjectiveFunctions >>
      numScalarLeastSqTerms >> numScalarNonlinearIneqConstraints >>
      numScalarNonlinearEqConstraints >> numScalarResponseFunctions >>
      numFieldObjectiveFunctions >> numFieldLeastSqTerms >>
      numFieldNonlinearIneqConstraints >> numFieldNonlinearEqConstraints >>
      numFieldResponseFunctions
      // weights, bounds, targets
      >> primaryRespFnSense >> primaryRespFnWeights >> nonlinearIneqLowerBnds >>
      nonlinearIneqUpperBnds >> nonlinearEqTargets
      // scaling data
      >> primaryRespFnScaleTypes >> primaryRespFnScales >>
      nonlinearIneqScaleTypes >> nonlinearIneqScales >> nonlinearEqScaleTypes >>
      nonlinearEqScales
      // experimental data
      >> calibrationDataFlag >> numExperiments >> numExpConfigVars >>
      expConfigVars >> simVariance >> expObservations >> expStdDeviations >>
      scalarDataFileName >> scalarDataFormat
      // derivative settings
      >> gradientType >> hessianType >> ignoreBounds >> centralHess >>
      quasiHessianType >> methodSource >> intervalType >> interpolateFlag >>
      fdGradStepSize >> fdGradStepType >> fdHessStepSize >> fdHessStepType >>
      idNumericalGrads >> idAnalyticGrads >> idNumericalHessians >>
      idQuasiHessians >> idAnalyticHessians
      // field data
      >> fieldLengths >> numCoordsPerField >> readFieldCoords >> varianceType >>
      metadataLabels;
}

void DataResponsesRep::write(std::ostream& s) const {
  s << idResponses
    << responseLabels
    // counts
    << numObjectiveFunctions << numLeastSqTerms << numNonlinearIneqConstraints
    << numNonlinearEqConstraints << numResponseFunctions
    << numScalarObjectiveFunctions << numScalarLeastSqTerms
    << numScalarNonlinearIneqConstraints << numScalarNonlinearEqConstraints
    << numScalarResponseFunctions << numFieldObjectiveFunctions
    << numFieldLeastSqTerms << numFieldNonlinearIneqConstraints
    << numFieldNonlinearEqConstraints
    << numFieldResponseFunctions
    // weights, bounds, targets
    << primaryRespFnSense << primaryRespFnWeights << nonlinearIneqLowerBnds
    << nonlinearIneqUpperBnds
    << nonlinearEqTargets
    // scaling data
    << primaryRespFnScaleTypes << primaryRespFnScales << nonlinearIneqScaleTypes
    << nonlinearIneqScales << nonlinearEqScaleTypes
    << nonlinearEqScales
    // experimental data
    << calibrationDataFlag << numExperiments << numExpConfigVars
    << expConfigVars << simVariance << expObservations << expStdDeviations
    << scalarDataFileName
    << scalarDataFormat
    // derivative settings
    << gradientType << hessianType << ignoreBounds << centralHess
    << quasiHessianType << methodSource << intervalType << interpolateFlag
    << fdGradStepSize << fdGradStepType << fdHessStepSize << fdHessStepType
    << idNumericalGrads << idAnalyticGrads << idNumericalHessians
    << idQuasiHessians
    << idAnalyticHessians
    // field data
    << fieldLengths << numCoordsPerField << readFieldCoords << varianceType
    << metadataLabels;
}

DataResponses::DataResponses()
    : dataRespRep(new DataResponsesRep()) { /* empty ctor */ }

DataResponses::DataResponses(const DataResponses& data_resp)
    : dataRespRep(data_resp.dataRespRep) { /* empty ctor */ }

DataResponses& DataResponses::operator=(const DataResponses& data_resp) {
  dataRespRep = data_resp.dataRespRep;
  return *this;
}

DataResponses::~DataResponses() { /* empty dtor */ }

}  // namespace Dakota
