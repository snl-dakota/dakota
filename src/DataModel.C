/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2006, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataModel
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataModel.H"
#include "data_io.h"


namespace Dakota {

DataModelRep::DataModelRep():
  modelType("single"), //approxPointReuse("none"),
  pointsTotal(0), pointsMinimum(false), pointsRecommended(false),
  approxCorrectionOrder(0), approxDerivUsageFlag(false), polynomialOrder(2),
  krigingMaxTrials(0), mlsPolyOrder(0), mlsWeightFunction(0), rbfBases(0),
  rbfMaxPts(0), rbfMaxSubsets(0), rbfMinPartition(0), marsMaxBases(0),
  annRandomWeight(0), annNodes(0), annRange(0.0), trendOrder(2),
  pointSelection(false), referenceCount(1)
{ }


void DataModelRep::write(MPIPackBuffer& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << subMethodPointer << surrogateFnIndices
    << surrogateType << truthModelPointer << lowFidelityModelPointer
    << pointsTotal << pointsMinimum << pointsRecommended
    << approxPointReuse << approxPointReuseFile << approxCorrectionType
    << approxCorrectionOrder << approxDerivUsageFlag << polynomialOrder
    << krigingCorrelations << krigingConminSeed << krigingMaxTrials
    << krigingMaxCorrelations << krigingMinCorrelations << mlsPolyOrder 
    << mlsWeightFunction << rbfBases << rbfMaxPts << rbfMaxSubsets
    << rbfMinPartition << marsMaxBases << marsInterpolation << annRandomWeight
    << annNodes << annRange << trendOrder << pointSelection << diagMetrics
    << optionalInterfRespPointer << primaryVarMaps << secondaryVarMaps
    << primaryRespCoeffs << secondaryRespCoeffs;
}


void DataModelRep::read(MPIUnpackBuffer& s)
{
  s >> idModel >> modelType >> variablesPointer >> interfacePointer
    >> responsesPointer >> subMethodPointer >> surrogateFnIndices
    >> surrogateType >> truthModelPointer >> lowFidelityModelPointer
    >> pointsTotal >> pointsMinimum >> pointsRecommended
    >> approxPointReuse >> approxPointReuseFile >> approxCorrectionType
    >> approxCorrectionOrder >> approxDerivUsageFlag >> polynomialOrder
    >> krigingCorrelations >> krigingConminSeed >> krigingMaxTrials
    >> krigingMaxCorrelations >> krigingMinCorrelations >> mlsPolyOrder 
    >> mlsWeightFunction >> rbfBases >> rbfMaxPts >> rbfMaxSubsets
    >> rbfMinPartition >> marsMaxBases >> marsInterpolation >> annRandomWeight
    >> annNodes >> annRange >> trendOrder >> pointSelection >> diagMetrics
    >> optionalInterfRespPointer >> primaryVarMaps >> secondaryVarMaps
    >> primaryRespCoeffs >> secondaryRespCoeffs;
}


void DataModelRep::write(std::ostream& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << subMethodPointer << surrogateFnIndices
    << surrogateType << truthModelPointer << lowFidelityModelPointer
    << pointsTotal << pointsMinimum << pointsRecommended
    << approxPointReuse << approxPointReuseFile << approxCorrectionType
    << approxCorrectionOrder << approxDerivUsageFlag << polynomialOrder
    << krigingCorrelations << krigingConminSeed << krigingMaxTrials
    << krigingMaxCorrelations << krigingMinCorrelations << mlsPolyOrder 
    << mlsWeightFunction << rbfBases << rbfMaxPts << rbfMaxSubsets
    << rbfMinPartition << marsMaxBases << marsInterpolation << annRandomWeight
    << annNodes << annRange << trendOrder << pointSelection << diagMetrics
    << optionalInterfRespPointer << primaryVarMaps << secondaryVarMaps
    << primaryRespCoeffs << secondaryRespCoeffs;
}


DataModel::DataModel(): dataModelRep(new DataModelRep())
{
#ifdef REFCOUNT_DEBUG
  Cout << "DataModel::DataModel(), dataModelRep referenceCount = "
       << dataModelRep->referenceCount << std::endl;
#endif
}


DataModel::DataModel(const DataModel& data_model)
{
  // Increment new (no old to decrement)
  dataModelRep = data_model.dataModelRep;
  if (dataModelRep) // Check for an assignment of NULL
    dataModelRep->referenceCount++;

#ifdef REFCOUNT_DEBUG
  Cout << "DataModel::DataModel(DataModel&)" << std::endl;
  if (dataModelRep)
    Cout << "dataModelRep referenceCount = " << dataModelRep->referenceCount
	 << std::endl;
#endif
}


DataModel& DataModel::operator=(const DataModel& data_model)
{
  if (dataModelRep != data_model.dataModelRep) { // normal case: old != new
    // Decrement old
    if (dataModelRep) // Check for NULL
      if ( --dataModelRep->referenceCount == 0 ) 
	delete dataModelRep;
    // Assign and increment new
    dataModelRep = data_model.dataModelRep;
    if (dataModelRep) // Check for NULL
      dataModelRep->referenceCount++;
  }
  // else if assigning same rep, then do nothing since referenceCount
  // should already be correct

#ifdef REFCOUNT_DEBUG
  Cout << "DataModel::operator=(DataModel&)" << std::endl;
  if (dataModelRep)
    Cout << "dataModelRep referenceCount = " << dataModelRep->referenceCount
	 << std::endl;
#endif

  return *this;
}


DataModel::~DataModel()
{
  if (dataModelRep) { // Check for NULL
    --dataModelRep->referenceCount; // decrement
#ifdef REFCOUNT_DEBUG
    Cout << "dataModelRep referenceCount decremented to "
         << dataModelRep->referenceCount << std::endl;
#endif
    if (dataModelRep->referenceCount == 0) {
#ifdef REFCOUNT_DEBUG
      Cout << "deleting dataModelRep" << std::endl;
#endif
      delete dataModelRep;
    }
  }
}

} // namespace Dakota
