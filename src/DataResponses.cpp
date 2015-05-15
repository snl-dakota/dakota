/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataResponses
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataResponses.hpp"
#include "dakota_data_io.hpp"


namespace Dakota {

// Default constructor:
DataResponsesRep::DataResponsesRep():
  numObjectiveFunctions(0), numLeastSqTerms(0), numNonlinearIneqConstraints(0),
  numNonlinearEqConstraints(0), numResponseFunctions(0),
  numScalarObjectiveFunctions(0), numScalarLeastSqTerms(0),
  numScalarNonlinearIneqConstraints(0), numScalarNonlinearEqConstraints(0),
  numScalarResponseFunctions(0), numFieldObjectiveFunctions(0),
  numFieldLeastSqTerms(0), numFieldNonlinearIneqConstraints(0),
  numFieldNonlinearEqConstraints(0), numFieldResponseFunctions(0),
  calibrationDataFlag(false), numExperiments(1), numExpConfigVars(0),
  scalarDataFormat(TABULAR_EXPER_ANNOT), ignoreBounds(false), centralHess(false), 
  methodSource("dakota"), intervalType("forward"), interpolateFlag(false),
  fdGradStepType("relative"), fdHessStepType("relative"), readFieldCoords(false),
  referenceCount(1)
{ }


void DataResponsesRep::write(MPIPackBuffer& s) const
{
  s << idResponses << responseLabels
    // counts
    << numObjectiveFunctions << numLeastSqTerms << numNonlinearIneqConstraints
    << numNonlinearEqConstraints << numResponseFunctions
    << numScalarObjectiveFunctions << numScalarLeastSqTerms
    << numScalarNonlinearIneqConstraints << numScalarNonlinearEqConstraints
    << numScalarResponseFunctions << numFieldObjectiveFunctions
    << numFieldLeastSqTerms << numFieldNonlinearIneqConstraints
    << numFieldNonlinearEqConstraints << numFieldResponseFunctions
    // weights, bounds, targets
    << primaryRespFnSense << primaryRespFnWeights << nonlinearIneqLowerBnds
    << nonlinearIneqUpperBnds << nonlinearEqTargets
    // scaling data
    << primaryRespFnScaleTypes << primaryRespFnScales << nonlinearIneqScaleTypes
    << nonlinearIneqScales << nonlinearEqScaleTypes << nonlinearEqScales 
    // experimental data
    << calibrationDataFlag << numExperiments << numExpConfigVars
    << expConfigVars << expObservations << expStdDeviations << scalarDataFileName
    << scalarDataFormat
    // derivative settings
    << gradientType << hessianType << ignoreBounds << centralHess
    << quasiHessianType << methodSource << intervalType << interpolateFlag 
    << fdGradStepSize << fdGradStepType << fdHessStepSize << fdHessStepType
    << idNumericalGrads << idAnalyticGrads
    << idNumericalHessians << idQuasiHessians << idAnalyticHessians
    // field data
    << fieldLengths << numCoordsPerField 
    << readFieldCoords << varianceType;
}


void DataResponsesRep::read(MPIUnpackBuffer& s)
{
  s >> idResponses >> responseLabels
    // counts
    >> numObjectiveFunctions >> numLeastSqTerms >> numNonlinearIneqConstraints
    >> numNonlinearEqConstraints >> numResponseFunctions
    >> numScalarObjectiveFunctions >> numScalarLeastSqTerms
    >> numScalarNonlinearIneqConstraints >> numScalarNonlinearEqConstraints
    >> numScalarResponseFunctions >> numFieldObjectiveFunctions
    >> numFieldLeastSqTerms >> numFieldNonlinearIneqConstraints
    >> numFieldNonlinearEqConstraints >> numFieldResponseFunctions
    // weights, bounds, targets
    >> primaryRespFnSense >> primaryRespFnWeights >> nonlinearIneqLowerBnds
    >> nonlinearIneqUpperBnds >> nonlinearEqTargets
    // scaling data
    >> primaryRespFnScaleTypes >> primaryRespFnScales >> nonlinearIneqScaleTypes
    >> nonlinearIneqScales >> nonlinearEqScaleTypes >> nonlinearEqScales 
    // experimental data
    >> calibrationDataFlag >> numExperiments  >> numExpConfigVars
    >> expConfigVars >> expObservations >> expStdDeviations >> scalarDataFileName
    >> scalarDataFormat
    // derivative settings
    >> gradientType >> hessianType >> ignoreBounds >> centralHess
    >> quasiHessianType >> methodSource >> intervalType >> interpolateFlag 
    >> fdGradStepSize >> fdGradStepType >> fdHessStepSize >> fdHessStepType
    >> idNumericalGrads >> idAnalyticGrads
    >> idNumericalHessians >> idQuasiHessians >> idAnalyticHessians
    // field data
    >> fieldLengths >> numCoordsPerField
    >> readFieldCoords >> varianceType;

}


void DataResponsesRep::write(std::ostream& s) const
{
  s << idResponses << responseLabels
    // counts
    << numObjectiveFunctions << numLeastSqTerms << numNonlinearIneqConstraints
    << numNonlinearEqConstraints << numResponseFunctions
    << numScalarObjectiveFunctions << numScalarLeastSqTerms
    << numScalarNonlinearIneqConstraints << numScalarNonlinearEqConstraints
    << numScalarResponseFunctions << numFieldObjectiveFunctions
    << numFieldLeastSqTerms << numFieldNonlinearIneqConstraints
    << numFieldNonlinearEqConstraints << numFieldResponseFunctions
    // weights, bounds, targets
    << primaryRespFnSense << primaryRespFnWeights << nonlinearIneqLowerBnds
    << nonlinearIneqUpperBnds << nonlinearEqTargets
    // scaling data
    << primaryRespFnScaleTypes << primaryRespFnScales << nonlinearIneqScaleTypes
    << nonlinearIneqScales << nonlinearEqScaleTypes << nonlinearEqScales 
    // experimental data
    << calibrationDataFlag << numExperiments << numExpConfigVars
    << expConfigVars << expObservations << expStdDeviations << scalarDataFileName
    << scalarDataFormat
    // derivative settings
    << gradientType << hessianType << ignoreBounds << centralHess
    << quasiHessianType << methodSource << intervalType << interpolateFlag 
    << fdGradStepSize << fdGradStepType << fdHessStepSize << fdHessStepType
    << idNumericalGrads << idAnalyticGrads
    << idNumericalHessians << idQuasiHessians << idAnalyticHessians
    // field data
    << fieldLengths << numCoordsPerField  
    << readFieldCoords << varianceType;
}


DataResponses::DataResponses(): dataRespRep(new DataResponsesRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataResponses::DataResponses(), dataRespRep referenceCount = "
       << dataRespRep->referenceCount << std::endl;
#endif
}


DataResponses::DataResponses(const DataResponses& data_resp)
{
  // Increment new (no old to decrement)
  dataRespRep = data_resp.dataRespRep;
  if (dataRespRep) // Check for an assignment of NULL
    ++dataRespRep->referenceCount;

#ifdef REFCOUNT_DEBUG
  Cout << "DataResponses::DataResponses(DataResponses&)" << std::endl;
  if (dataRespRep)
    Cout << "dataRespRep referenceCount = " << dataRespRep->referenceCount
	 << std::endl;
#endif
}


DataResponses& DataResponses::operator=(const DataResponses& data_resp)
{
  if (dataRespRep != data_resp.dataRespRep) { // normal case: old != new
    // Decrement old
    if (dataRespRep) // Check for NULL
      if ( --dataRespRep->referenceCount == 0 ) 
	delete dataRespRep;
    // Assign and increment new
    dataRespRep = data_resp.dataRespRep;
    if (dataRespRep) // Check for NULL
      ++dataRespRep->referenceCount;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataResponses::operator=(DataResponses&)" << std::endl;
  if (dataRespRep)
    Cout << "dataRespRep referenceCount = " << dataRespRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


DataResponses::~DataResponses()
{
  if (dataRespRep) { // Check for NULL
    --dataRespRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataRespRep referenceCount decremented to "
         << dataRespRep->referenceCount << std::endl;
#endif
    if (dataRespRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataRespRep" << std::endl;
#endif
      delete dataRespRep;
    }
  }
}

} // namespace Dakota
