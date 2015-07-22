/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataModel
//- Description:  Class implementation
//- Owner:        Mike Eldred

#include "DataModel.hpp"
#include "DataMethod.hpp" // shared scheduling enums
#include "dakota_data_io.hpp"


namespace Dakota {

DataModelRep::DataModelRep():
  modelType("single"), //approxPointReuse("none"),
  hierarchicalTags(false),
  pointsTotal(0), pointsManagement(DEFAULT_POINTS), 
  approxImportFormat(TABULAR_ANNOTATED), approxImportActive(false), approxExportFormat(TABULAR_ANNOTATED),
  approxCorrectionType(NO_CORRECTION), approxCorrectionOrder(0),
  modelUseDerivsFlag(false), polynomialOrder(2), krigingMaxTrials(0),
  krigingNugget(0.0), krigingFindNugget(0), mlsPolyOrder(0), mlsWeightFunction(0),
  rbfBases(0), rbfMaxPts(0), rbfMaxSubsets(0), rbfMinPartition(0), marsMaxBases(0),
  annRandomWeight(0), annNodes(0), annRange(0.0), 
  domainDecomp(false), decompCellType("voronoi"), decompSupportLayers(0),
  decompDiscontDetect(false), discontJumpThresh(0.0), discontGradThresh(0.0),
  trendOrder("reduced_quadratic"),
  pointSelection(false), crossValidateFlag(false), numFolds(0), percentFold(0.0),
  pressFlag(false), approxChallengeFormat(TABULAR_ANNOTATED), approxChallengeActive(false),
  subMethodServers(0), subMethodProcs(0), // 0 defaults to detect user spec
  subMethodScheduling(DEFAULT_SCHEDULING), referenceCount(1)
{ }


void DataModelRep::write(MPIPackBuffer& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << hierarchicalTags << subMethodPointer 
    << surrogateFnIndices
    << surrogateType << truthModelPointer << lowFidelityModelPointer
    << pointsTotal << pointsManagement << approxPointReuse << approxImportFile
    << approxImportFormat << approxImportActive << approxExportFile << approxExportFormat
    << approxExportModelFile 
    << approxCorrectionType << approxCorrectionOrder << modelUseDerivsFlag
    << polynomialOrder << krigingCorrelations << krigingOptMethod
    << krigingMaxTrials << krigingMaxCorrelations << krigingMinCorrelations
    << krigingNugget << krigingFindNugget << mlsPolyOrder << mlsWeightFunction 
    << rbfBases << rbfMaxPts << rbfMaxSubsets << rbfMinPartition << marsMaxBases 
    << marsInterpolation << annRandomWeight << annNodes << annRange 
    << domainDecomp << decompCellType << decompSupportLayers
    << decompDiscontDetect << discontJumpThresh << discontGradThresh
    << trendOrder 
    << pointSelection << diagMetrics << crossValidateFlag << numFolds  
    << percentFold << pressFlag  << approxChallengeFile << approxChallengeFormat 
    << approxChallengeActive << optionalInterfRespPointer << primaryVarMaps
    << secondaryVarMaps << primaryRespCoeffs << secondaryRespCoeffs
    << subMethodServers << subMethodProcs << subMethodScheduling;
}


void DataModelRep::read(MPIUnpackBuffer& s)
{
  s >> idModel >> modelType >> variablesPointer >> interfacePointer
    >> responsesPointer >> hierarchicalTags >> subMethodPointer 
    >> surrogateFnIndices
    >> surrogateType >> truthModelPointer >> lowFidelityModelPointer
    >> pointsTotal >> pointsManagement >> approxPointReuse >> approxImportFile
    >> approxImportFormat >> approxImportActive >> approxExportFile >> approxExportFormat
    >> approxExportModelFile 
    >> approxCorrectionType >> approxCorrectionOrder >> modelUseDerivsFlag
    >> polynomialOrder >> krigingCorrelations >> krigingOptMethod
    >> krigingMaxTrials >> krigingMaxCorrelations >> krigingMinCorrelations
    >> krigingNugget >> krigingFindNugget >> mlsPolyOrder >> mlsWeightFunction
    >> rbfBases >> rbfMaxPts >> rbfMaxSubsets >> rbfMinPartition >> marsMaxBases
    >> marsInterpolation >> annRandomWeight >> annNodes >> annRange 
    >> domainDecomp >> decompCellType >> decompSupportLayers
    >> decompDiscontDetect >> discontJumpThresh >> discontGradThresh
    >> trendOrder
    >> pointSelection >> diagMetrics >> crossValidateFlag >> numFolds
    >> percentFold >> pressFlag  >> approxChallengeFile >> approxChallengeFormat 
    >> approxChallengeActive >> optionalInterfRespPointer >> primaryVarMaps
    >> secondaryVarMaps >> primaryRespCoeffs >> secondaryRespCoeffs
    >> subMethodServers >> subMethodProcs >> subMethodScheduling;
}


void DataModelRep::write(std::ostream& s) const
{
  s << idModel << modelType << variablesPointer << interfacePointer
    << responsesPointer << hierarchicalTags << subMethodPointer 
    << surrogateFnIndices
    << surrogateType << truthModelPointer << lowFidelityModelPointer
    << pointsTotal << pointsManagement << approxPointReuse << approxImportFile
    << approxImportFormat << approxImportActive << approxExportFile << approxExportFormat
    << approxExportModelFile
    << approxCorrectionType << approxCorrectionOrder << modelUseDerivsFlag
    << polynomialOrder << krigingCorrelations << krigingOptMethod
    << krigingMaxTrials << krigingMaxCorrelations << krigingMinCorrelations
    << krigingNugget << krigingFindNugget << mlsPolyOrder << mlsWeightFunction 
    << rbfBases << rbfMaxPts << rbfMaxSubsets << rbfMinPartition << marsMaxBases
    << marsInterpolation << annRandomWeight << annNodes << annRange 
    << domainDecomp << decompCellType << decompSupportLayers
    << decompDiscontDetect << discontJumpThresh << discontGradThresh
    << trendOrder
    << pointSelection << diagMetrics << crossValidateFlag << numFolds
    << percentFold << pressFlag  << approxChallengeFile << approxChallengeFormat 
    << approxChallengeActive << optionalInterfRespPointer << primaryVarMaps
    << secondaryVarMaps << primaryRespCoeffs << secondaryRespCoeffs
    << subMethodServers << subMethodProcs << subMethodScheduling;
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
    ++dataModelRep->referenceCount;

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
      ++dataModelRep->referenceCount;
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
