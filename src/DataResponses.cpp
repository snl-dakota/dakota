/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
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
DataResponsesRep::DataResponsesRep(): numObjectiveFunctions(0),
  numNonlinearIneqConstraints(0), numNonlinearEqConstraints(0),
  numLeastSqTerms(0), numResponseFunctions(0), numExperiments(1),
  numExpConfigVars(0), numExpStdDeviations(0), expDataFileAnnotated(true),
  ignoreBounds(false), centralHess(false), methodSource("dakota"),
  intervalType("forward"), fdGradStepType("relative"), fdHessStepType("relative"), 
  referenceCount(1)
{ }


void DataResponsesRep::write(MPIPackBuffer& s) const
{
  s << idResponses << responseLabels
    // counts
    << numObjectiveFunctions << numNonlinearIneqConstraints
    << numNonlinearEqConstraints << numLeastSqTerms << numResponseFunctions
    // weights, bounds, targets
    << primaryRespFnSense << primaryRespFnWeights << nonlinearIneqLowerBnds
    << nonlinearIneqUpperBnds << nonlinearEqTargets
    // scaling data
    << primaryRespFnScaleTypes << primaryRespFnScales << nonlinearIneqScaleTypes
    << nonlinearIneqScales << nonlinearEqScaleTypes << nonlinearEqScales 
    // experimental data
    << numExperiments << numExpConfigVars << numExpStdDeviations
    << expConfigVars << expObservations << expStdDeviations << expDataFileName
    << expDataFileAnnotated
    // derivative settings
    << gradientType << hessianType << ignoreBounds << centralHess
    << quasiHessianType << methodSource << intervalType << fdGradStepSize
    << fdGradStepType << fdHessStepSize << fdHessStepType
    << idNumericalGrads << idAnalyticGrads
    << idNumericalHessians << idQuasiHessians << idAnalyticHessians;
}


void DataResponsesRep::read(MPIUnpackBuffer& s)
{
  s >> idResponses >> responseLabels
    // counts
    >> numObjectiveFunctions >> numNonlinearIneqConstraints
    >> numNonlinearEqConstraints >> numLeastSqTerms >> numResponseFunctions
    // weights, bounds, targets
    >> primaryRespFnSense >> primaryRespFnWeights >> nonlinearIneqLowerBnds
    >> nonlinearIneqUpperBnds >> nonlinearEqTargets
    // scaling data
    >> primaryRespFnScaleTypes >> primaryRespFnScales >> nonlinearIneqScaleTypes
    >> nonlinearIneqScales >> nonlinearEqScaleTypes >> nonlinearEqScales 
    // experimental data
    >> numExperiments >> numExpConfigVars >> numExpStdDeviations
    >> expConfigVars >> expObservations >> expStdDeviations >> expDataFileName
    >> expDataFileAnnotated
    // derivative settings
    >> gradientType >> hessianType >> ignoreBounds >> centralHess
    >> quasiHessianType >> methodSource >> intervalType >> fdGradStepSize
    >> fdGradStepType >> fdHessStepSize >> fdHessStepType
    >> idNumericalGrads >> idAnalyticGrads
    >> idNumericalHessians >> idQuasiHessians >> idAnalyticHessians;
}


void DataResponsesRep::write(std::ostream& s) const
{
  s << idResponses << responseLabels
    // counts
    << numObjectiveFunctions << numNonlinearIneqConstraints
    << numNonlinearEqConstraints << numLeastSqTerms << numResponseFunctions
    // weights, bounds, targets
    << primaryRespFnSense << primaryRespFnWeights << nonlinearIneqLowerBnds
    << nonlinearIneqUpperBnds << nonlinearEqTargets
    // scaling data
    << primaryRespFnScaleTypes << primaryRespFnScales << nonlinearIneqScaleTypes
    << nonlinearIneqScales << nonlinearEqScaleTypes << nonlinearEqScales 
    // experimental data
    << numExperiments << numExpConfigVars << numExpStdDeviations
    << expConfigVars << expObservations << expStdDeviations << expDataFileName
    << expDataFileAnnotated
    // derivative settings
    << gradientType << hessianType << ignoreBounds << centralHess
    << quasiHessianType << methodSource << intervalType << fdGradStepSize
    << fdGradStepType << fdHessStepSize << fdHessStepType
    << idNumericalGrads << idAnalyticGrads
    << idNumericalHessians << idQuasiHessians << idAnalyticHessians;
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
    dataRespRep->referenceCount++;

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
      dataRespRep->referenceCount++;
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
