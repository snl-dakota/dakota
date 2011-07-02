/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataModel
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataModel.H 6879 2010-07-30 01:05:11Z mseldre $

#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "system_defs.h"
#include "data_types.h"
#include "DakotaString.H"

namespace Dakota {

// define special values for pointsManagement
enum { DEFAULT_POINTS, MINIMUM_POINTS, RECOMMENDED_POINTS, TOTAL_POINTS };

// define special values for SurrogateModel::responseMode
enum { NO_SURROGATE=0,  UNCORRECTED_SURROGATE, AUTO_CORRECTED_SURROGATE,
       BYPASS_SURROGATE, ADDITIVE_DISCREPANCY, MULTIPLICATIVE_DISCREPANCY };


/// Body class for model specification data.

/** The DataModelRep class is used to contain the data from a model
    keyword specification.  Default values are managed in the
    DataModelRep constructor.  Data is public to avoid maintaining
    set/get functions, but is still encapsulated within ProblemDescDB
    since ProblemDescDB::dataModelList is private. */

class DataModelRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataModel;

public:

  //
  //- Heading: Data
  //

  /// string identifier for the model specification data set (from
  /// the \c id_model specification in \ref ModelIndControl)
  String idModel;
  /// model type selection: single, surrogate, or nested (from the model type
  /// specification in \ref ModelIndControl)
  String modelType;
  /// string pointer to the variables specification to be used by this model
  /// (from the \c variables_pointer specification in \ref ModelIndControl)
  String variablesPointer;
  /// string pointer to the interface specification to be used by this model
  /// (from the \c interface_pointer specification in \ref ModelSingle and
  /// the \c optional_interface_pointer specification in \ref ModelNested)
  String interfacePointer;
  /// string pointer to the responses specification to be used by this model
  /// (from the \c responses_pointer specification in \ref ModelIndControl)
  String responsesPointer;
  /// pointer to a sub-iterator used for global approximations (from the
  /// \c dace_method_pointer specification in \ref ModelSurrG) or by
  /// nested models (from the \c sub_method_pointer specification in
  /// \ref ModelNested)
  String subMethodPointer;

  // surrogate models

  /// array specifying the response function set that is approximated
  IntSet surrogateFnIndices;
  /// the selected surrogate type: local_taylor, multipoint_tana,
  /// global_(neural_network,mars,orthogonal_polynomial,gaussian,
  /// polynomial,kriging), or hierarchical
  String surrogateType;
  /// pointer to the model specification for constructing the truth model
  /// used in building local, multipoint, and hierarchical approximations
  /// (from the \c actual_model_pointer specification in \ref ModelSurrL
  /// and \ref ModelSurrMP and the \c high_fidelity_model_pointer
  /// specification in \ref ModelSurrH)
  String truthModelPointer;
  /// pointer to the low fidelity model specification used in
  /// hierarchical approximations (from the \c low_fidelity_model_pointer
  /// specification in \ref ModelSurrH)
  String lowFidelityModelPointer;

  // controls for number of points with which to build the model

  /// user-specified lower bound on total points with which to build the model
  /// (if reuse_points < pointsTotal, new samples will make up the difference)
  int pointsTotal;
  /// points management configuration for DataFitSurrModel:
  /// DEFAULT_POINTS, MINIMUM_POINTS, or RECOMMENDED_POINTS
  short pointsManagement;

  /// sample reuse selection for building global approximations: none, all,
  /// region, or file (from the \c reuse_samples specification in
  /// \ref ModelSurrG)
  String approxPointReuse;
  /// the file name for the "file" setting for the \c reuse_samples
  /// specification in \ref ModelSurrG
  String approxPointReuseFile;
  /// correction type for global and hierarchical approximations: additive,
  /// multiplicative, or combined (from the \c correction specification in
  /// \ref ModelSurrG and \ref ModelSurrH)
  String approxCorrectionType;
  /// correction order for global and hierarchical approximations: 0,
  /// 1, or 2 (from the \c correction specification in \ref ModelSurrG
  /// and \ref ModelSurrH)
  short approxCorrectionOrder;
  /// flags the use of derivatives in building global approximations
  /// (from the \c use_derivatives specification in \ref ModelSurrG)
  bool modelUseDerivsFlag;
  /// scalar integer indicating the order of the polynomial approximation
  /// (1=linear, 2=quadratic, 3=cubic; from the \c polynomial specification
  /// in \ref ModelSurrG)
  short polynomialOrder;
  /// vector of correlations used in building a kriging approximation 
  /// (from the \c correlations specification in \ref ModelSurrG)
  RealVector krigingCorrelations;
  /// optimization method to use in finding optimal correlation parameters:
  /// none, sampling, local, global
  String krigingOptMethod;
  /// maximum number of trials in optimization of kriging correlations
  short krigingMaxTrials;
  /// upper bound on kriging correlation vector
  RealVector krigingMaxCorrelations;
  /// lower bound on kriging correlation vector
  RealVector krigingMinCorrelations;
  /// polynomial order for moving least squares approximation
  short mlsPolyOrder;
  /// weight function for moving least squares approximation
  short mlsWeightFunction;
  /// bases for radial basis function approximation
  short rbfBases;
  /// maximum number of points for radial basis function approximation
  short rbfMaxPts;
  /// maximum number of subsets for radial basis function approximation
  short rbfMaxSubsets;
  /// minimum partition for radial basis function approximation
  short rbfMinPartition;
  /// maximum number of bases for MARS approximation
  short marsMaxBases;
  /// interpolation type for MARS approximation
  String marsInterpolation;
  /// random weight for artificial neural network approximation
  short annRandomWeight;
  /// number of nodes for artificial neural network approximation
  short annNodes;
  /// range for artificial neural network approximation
  Real annRange;
  /// scalar integer indicating the order of the Gaussian process mean
  /// (0= constant, 1=linear, 2=quadratic, 3=cubic); from the
  /// \c gaussian_process specification  in \ref ModelSurrG)
  String trendOrder;
  /// flag indicating the use of point selection in the Gaussian process
  bool pointSelection;
  /// List of diagnostic metrics the user requests to assess the 
  /// goodness of fit for a surrogate model.
  StringArray diagMetrics;

  // nested models

  /// string pointer to the responses specification used by the optional
  /// interface in nested models (from the \c
  /// optional_interface_responses_pointer specification in \ref ModelNested)
  String optionalInterfRespPointer;
  /// the primary variable mappings used in nested models for identifying
  /// the lower level variable targets for inserting top level variable
  /// values (from the \c primary_variable_mapping specification in
  /// \ref ModelNested)
  StringArray primaryVarMaps;
  /// the secondary variable mappings used in nested models for identifying
  /// the (distribution) parameter targets within the lower level variables
  /// for inserting top level variable values (from the
  /// \c secondary_variable_mapping specification in \ref ModelNested)
  StringArray secondaryVarMaps;
  /// the primary response mapping matrix used in nested models for
  /// weighting contributions from the sub-iterator responses in the top
  /// level (objective) functions (from the \c primary_response_mapping
  /// specification in \ref ModelNested)
  RealVector primaryRespCoeffs;
  /// the secondary response mapping matrix used in nested models for
  /// weighting contributions from the sub-iterator responses in the top
  /// level (constraint) functions (from the \c secondary_response_mapping
  /// specification in \ref ModelNested)
  RealVector secondaryRespCoeffs;

private:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataModelRep();  ///< constructor
  ~DataModelRep(); ///< destructor

  //
  //- Heading: Member methods
  //

  /// write a DataModelRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataModelRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s); 
  /// write a DataModelRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private convenience functions
  //

  /// number of handle objects sharing this dataModelRep
  int referenceCount;
};


inline DataModelRep::~DataModelRep() { }


/// Handle class for model specification data.

/** The DataModel class is used to provide a memory management handle
    for the data in DataModelRep.  It is populated by
    IDRProblemDescDB::model_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A list of DataModel
    objects is maintained in ProblemDescDB::dataModelList, one for
    each model specification in an input file. */

class DataModel
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;

public:

  // comparison function
  static bool id_compare(const DataModel& dm, const std::string& id)
  { return id == dm.dataModelRep->idModel; }

  //
  //- Heading: Constructors, destructor, operators
  //

  DataModel();                                ///< constructor
  DataModel(const DataModel&);               ///< copy constructor
  ~DataModel();                               ///< destructor

  DataModel& operator=(const DataModel&); ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// write a DataModel object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataModel object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);

  /// write a DataModel object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataModelRep* dataModelRep;
};


/// MPIPackBuffer insertion operator for DataModel
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataModel& data)
{ data.write(s); return s; }


/// MPIUnpackBuffer extraction operator for DataModel
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataModel& data)
{ data.read(s); return s; }


/// std::ostream insertion operator for DataModel
inline std::ostream& operator<<(std::ostream& s, const DataModel& data)
{ data.write(s); return s; }

inline void DataModel::write(std::ostream& s) const
{ dataModelRep->write(s); }


inline void DataModel::read(MPIUnpackBuffer& s)
{ dataModelRep->read(s); }


inline void DataModel::write(MPIPackBuffer& s) const
{ dataModelRep->write(s); }

} // namespace Dakota

#endif
