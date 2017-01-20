/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataModel
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataModel.hpp 6879 2010-07-30 01:05:11Z mseldre $

#ifndef DATA_MODEL_H
#define DATA_MODEL_H

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"

namespace Dakota {

/// define special values for pointsManagement
enum { DEFAULT_POINTS, MINIMUM_POINTS, RECOMMENDED_POINTS, TOTAL_POINTS };

/// define special values for SurrogateModel::responseMode
enum { NO_SURROGATE=0,  UNCORRECTED_SURROGATE, AUTO_CORRECTED_SURROGATE,
       BYPASS_SURROGATE, MODEL_DISCREPANCY, AGGREGATED_MODELS };

/// define special values for approxCorrectionType
enum { NO_CORRECTION=0,  ADDITIVE_CORRECTION, MULTIPLICATIVE_CORRECTION,
       COMBINED_CORRECTION };

/// define types of random field approximations
enum { RF_KARHUNEN_LOEVE=0, RF_PCA_GP, RF_ICA };

/// define types of analytic covariance functions
enum { NOCOVAR=0, EXP_L2, EXP_L1 };

/// define special values for active subspace normalizations
enum { SUBSPACE_NORM_DEFAULT=0, SUBSPACE_NORM_MEAN_VALUE,
       SUBSPACE_NORM_MEAN_GRAD, SUBSPACE_NORM_LOCAL_GRAD }; 

/// define special values for componentParallelMode
/// (active model for parallel scheduling)
enum { SURROGATE_MODEL=1, TRUTH_MODEL };


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
  /// whether this model and its children will add hierarchy-based
  /// tags to eval ids
  bool hierarchicalTags;
  /// pointer to a sub-iterator used for global approximations (from the
  /// \c dace_method_pointer specification in \ref ModelSurrG) or by
  /// nested models (from the \c sub_method_pointer specification in
  /// \ref ModelNested)
  String subMethodPointer;

  // single/simulation models

  /// (state) variable identifier that defines a set or range of solution
  /// level controls (space/time discretization levels, iterative convergence
  /// tolerances, etc.) for defining a secondary hierarchy of fidelity within
  /// the scope of a single model form (from \c solution_level_control
  /// specification; see also \c ordered_model_fidelities)
  String solutionLevelControl;
  /// array of relative simulation costs corresponding to each of the
  /// solution levels (from \c solution_level_cost specification; see also
  /// \c solution_level_control); a scalar input is interpreted as a constant
  /// cost multiplier to be applied recursively
  RealVector solutionLevelCost;

  // surrogate models

  /// array specifying the response function set that is approximated
  IntSet surrogateFnIndices;
  /// the selected surrogate type: local_taylor, multipoint_tana,
  /// global_(neural_network,mars,orthogonal_polynomial,gaussian,
  /// polynomial,kriging), or hierarchical
  String surrogateType;
  /// pointer to the model specification for constructing the truth model
  /// used in constructing surrogates (from the \c actual_model_pointer
  /// specification in \ref ModelSurrL and \ref ModelSurrMP)
  String actualModelPointer;
  /// an ordered list of model pointers (low to high) corresponding to a
  /// hierarchy of modeling fidelity (from the \c ordered_model_fidelities
  /// specification in \ref ModelSurrH)
  StringArray orderedModelPointers;

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

  /// the file name from the \c import_build_points_file specification in
  /// \ref ModelSurrG
  String importBuildPtsFile;
  /// tabular format for the build point import file
  unsigned short importBuildFormat;
  /// whether to import active variables only
  bool importBuildActive;

  // the file name from the \c import_approx_points_file specification in
  // \ref ModelSurrG
  //String importApproxPtsFile;
  // tabular format for the approx point import file
  //unsigned short importApproxFormat;
  // whether to import active variables only
  //bool importApproxActive;

  /// the file name from the \c export_approx_points_file specification in
  /// \ref ModelSurrG
  String exportApproxPtsFile;
  /// tabular format for the approx point export file
  unsigned short exportApproxFormat;

  /// Option to turn on surrogate model export (export_model)
  bool exportSurrogate;

  /// the filename prefix for export_model
  String modelExportPrefix;

  /// Format selection for export_model
  unsigned short modelExportFormat;

  /// correction type for global and hierarchical approximations:
  /// NO_CORRECTION, ADDITIVE_CORRECTION, MULTIPLICATIVE_CORRECTION,
  /// or COMBINED_CORRECTION (from the \c correction specification in
  /// \ref ModelSurrG and \ref ModelSurrH)
  short approxCorrectionType;
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
  /// nugget value for kriging
  Real krigingNugget;
  /// option to have Kriging find the best nugget value to use
  short krigingFindNugget;
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

  /// whether domain decomposition is enabled
  bool domainDecomp;
  /// type of local cell of domain decomp
  String decompCellType;
  /// number of support layers for each local basis function
  int decompSupportLayers;
  /// whether discontinuity detection is enabled
  bool decompDiscontDetect;
  /// function value (jump) threshold for discontinuity detection in
  /// domain decomp
  Real discontJumpThresh;
  /// gradient threshold for discontinuity detection in domain decomp
  Real discontGradThresh;

  /// scalar integer indicating the order of the Gaussian process mean
  /// (0= constant, 1=linear, 2=quadratic, 3=cubic); from the
  /// \c gaussian_process specification  in \ref ModelSurrG)
  String trendOrder;
  /// flag indicating the use of point selection in the Gaussian process
  bool pointSelection;
  /// List of diagnostic metrics the user requests to assess the 
  /// goodness of fit for a surrogate model.
  StringArray diagMetrics;
  /// flag indicating the use of cross validation on the metrics specified
  bool crossValidateFlag;
  /// number of folds to perform in cross validation
  int numFolds;
  /// percentage of data to withhold for cross validation process
  Real percentFold;
  /// flag indicating the use of PRESS on the metrics specified
  bool pressFlag;

  /// the file name from the \c challenge_points_file specification in
  /// \ref ModelSurrG
  String importChallengePtsFile;
  /// tabular format of the challenge data file
  unsigned short importChallengeFormat;
  /// whether to import active variables only
  bool importChallengeActive;

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
  /// number of servers for concurrent sub-iterator parallelism
  int subMethodServers;
  /// number of processors for each concurrent sub-iterator partition
  int subMethodProcs;
  /// scheduling approach for concurrent sub-iterator parallelism:
  /// {DEFAULT,MASTER,PEER}_SCHEDULING
  short subMethodScheduling;

  // subspace models

  /// initial samples to build the subspace model
  int initialSamples;

  /// sampling method for building the subspace model
  unsigned short subspaceSampleType;

  /// refinement samples to add in each batch
  IntVector refineSamples;
  /// maximum number of subspace build iterations
  int maxIterations;
  /// convergence tolerance on build process
  Real convergenceTolerance;

  /// Flag to use Bing Li method to identify active subspace dimension
  bool subspaceIdBingLi;

  /// Flag to use Constantine method to identify active subspace dimension
  bool subspaceIdConstantine;

  /// Flag to use eigenvalue energy method to identify active subspace dimension
  bool subspaceIdEnergy;

  /// Flag to build surrogate over active subspace
  bool subspaceBuildSurrogate;

  /// Size of subspace
  int subspaceDimension;

  /// Normalization to use when forming a subspace with multiple response
  /// functions
  unsigned short subspaceNormalization;

  /// Number of bootstrap samples for subspace identification
  int numReplicates;

  /// Flag to use cross validation to identify active subspace dimension
  bool subspaceIdCV;

  /// relative tolerance used by cross validation subspace dimension id method
  Real relTolerance;

  /// decrease tolerance used by cross validation subspace dimension id method
  Real decreaseTolerance;

  /// maximum rank considered by cross validation subspace dimension id method
  int subspaceCVMaxRank;

  /// flag to use incremental dimension estimation in the cross validation metric
  bool subspaceCVIncremental;

  /// Contains which cutoff method to use in the cross validation metric
  unsigned short subspaceIdCVMethod;


  /// whether automatic surrogate refinement is enabled
  bool autoRefine;
  /// maximum evals in refinement
  int maxFunctionEvals;
  /// metric to use in cross-validation guided refinement
  String refineCVMetric;
  /// max number of iterations in refinement without improvement
  int softConvergenceLimit;
  /// number of cross-validation folds in guided refinement
  int refineCVFolds;

  /// sparse grid level for low-order PCE used to compute rotation
  /// matrix within adapted basis approach to dimension reduction
  unsigned short adaptedBasisSparseGridLev;
  /// expansion order for low-order PCE used to compute rotation
  /// matrix within adapted basis approach to dimension reduction
  unsigned short adaptedBasisExpOrder;
  /// collocation ratio for low-order PCE used to compute rotation
  /// matrix within adapted basis approach to dimension reduction
  Real adaptedBasisCollocRatio;

  // random field models

  /// Contains which type of random field model 
  unsigned short randomFieldIdForm;

  /// Contains which type of analytic covariance function 
  unsigned short analyticCovIdForm;

  /// truncation tolerance on build process: percent variance explained
  Real truncationTolerance;

  /// pointer to the model through which to propagate the random field
  String propagationModelPointer;
  /// File from which to build the random field
  String rfDataFileName;

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

  /// compares the idModel attribute of DataModel objects
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

  /// return dataModelRep
  DataModelRep* data_rep();

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataModelRep* dataModelRep;
};


inline DataModelRep* DataModel::data_rep()
{return dataModelRep; }


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
