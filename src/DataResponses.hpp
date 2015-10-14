/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        DataResponses
//- Description:
//-
//-
//- Owner:        Mike Eldred
//- Version: $Id: DataResponses.hpp 6731 2010-04-16 03:42:16Z wjbohnh $

#ifndef DATA_RESPONSES_H 
#define DATA_RESPONSES_H 

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "MPIPackBuffer.hpp"

namespace Dakota {

/// special values for derived Response type
enum { BASE_RESPONSE=0, SIMULATION_RESPONSE, EXPERIMENT_RESPONSE };

/// values for primary response types
enum {GENERIC_FNS = 0, OBJECTIVE_FNS, CALIB_TERMS };

/// Body class for responses specification data.

/** The DataResponsesRep class is used to contain the data from a
    responses keyword specification.  Default values are managed in
    the DataResponsesRep constructor.  Data is public to avoid
    maintaining set/get functions, but is still encapsulated within
    ProblemDescDB since ProblemDescDB::dataResponsesList is private. */

class DataResponsesRep
{
  //
  //- Heading: Friends
  //

  /// the handle class can access attributes of the body class directly
  friend class DataResponses;

public:

  //
  //- Heading: Data
  //

  /// string identifier for the responses specification data set
  /// (from the \c id_responses specification in \ref RespSetId)
  String idResponses;
  /// the response labels array (from the \c response_descriptors
  /// specification in \ref RespLabels)
  StringArray responseLabels;

  // counts

  /// number of objective functions (from the \c
  /// num_objective_functions specification in \ref RespFnOpt)
  size_t numObjectiveFunctions;
  /// number of least squares terms (from the \c
  /// num_calibration_terms specification in \ref RespFnLS)
  size_t numLeastSqTerms;
  /// number of nonlinear inequality constraints (from the \c
  /// num_nonlinear_inequality_constraints specification in \ref RespFnOpt)
  size_t numNonlinearIneqConstraints;
  /// number of nonlinear equality constraints (from the \c
  /// num_nonlinear_equality_constraints specification in \ref RespFnOpt)
  size_t numNonlinearEqConstraints;
  /// number of generic response functions (from the \c
  /// num_response_functions specification in \ref RespFnGen)
  size_t numResponseFunctions;

  ///  scalar_objectives:  number of objective functions which are scalar
  size_t numScalarObjectiveFunctions;
  ///  scalar_calibration_terms:  number of calibration terms which are scalar
  size_t numScalarLeastSqTerms;
  /// number of scalar nonlinear inequality constraints (from the \c
  /// num_scalar_nonlinear_inequality_constraints specification in
  /// \ref RespFnOpt)
  size_t numScalarNonlinearIneqConstraints;
  /// number of scalar nonlinear equality constraints (from the \c
  /// num_scalar_nonlinear_equality_constraints specification in \ref RespFnOpt)
  size_t numScalarNonlinearEqConstraints;
  ///  scalar_responses:  number of response functions which are scalar
  size_t numScalarResponseFunctions;

  ///  field_objectives:  number of objective functions which are field-valued
  size_t numFieldObjectiveFunctions;
  ///  field_calibration_terms: number of calibration terms which are
  ///  field-valued
  size_t numFieldLeastSqTerms;
  /// number of field nonlinear inequality constraints (from the \c
  /// num_scalar_nonlinear_inequality_constraints specification in
  /// \ref RespFnOpt)
  size_t numFieldNonlinearIneqConstraints;
  /// number of field nonlinear equality constraints (from the \c
  /// num_scalar_nonlinear_equality_constraints specification in \ref RespFnOpt)
  size_t numFieldNonlinearEqConstraints;
  ///  field_responses:  number of response functions which are field-valued
  size_t numFieldResponseFunctions;

  // response set weights, bounds, targets

  /// optimization sense for each objective function: minimize or maximize
  StringArray primaryRespFnSense;
  /// vector of weightings for multiobjective optimization or weighted
  /// nonlinear least squares (from the \c multi_objective_weights
  /// specification in \ref RespFnOpt and the \c least_squares_weights
  /// specification in \ref RespFnLS)
  RealVector primaryRespFnWeights;
  /// vector of nonlinear inequality constraint lower bounds (from the \c
  /// nonlinear_inequality_lower_bounds specification in \ref RespFnOpt)
  RealVector nonlinearIneqLowerBnds;
  /// vector of nonlinear inequality constraint upper bounds (from the \c
  /// nonlinear_inequality_upper_bounds specification in \ref RespFnOpt)
  RealVector nonlinearIneqUpperBnds;
  /// vector of nonlinear equality constraint targets (from the \c
  /// nonlinear_equality_targets specification in \ref RespFnOpt)
  RealVector nonlinearEqTargets;
 
  // scaling data

  /// vector of primary response function scaling types (from the \c
  /// objective_function_scale_types specification in \ref RespFnOpt and
  /// the \c least_squares_term_scale_types specification in \ref RespFnLS)
  StringArray primaryRespFnScaleTypes;
  /// vector of primary response function scaling factors (from the \c
  /// objective_function_scales specification in \ref RespFnOpt and
  /// the \c least_squares_term_scales specification in \ref RespFnLS)
  RealVector primaryRespFnScales;
  /// vector of nonlinear inequality constraint scaling types (from the \c
  /// nonlinear_inequality_scale_types specification in \ref RespFnOpt)
  StringArray nonlinearIneqScaleTypes;
  /// vector of nonlinear inequality constraint scaling factors (from the \c
  /// nonlinear_inequality_scales specification in \ref RespFnOpt)
  RealVector nonlinearIneqScales;
  /// vector of nonlinear equality constraint scaling types (from the \c
  /// nonlinear_equality_scale_types specification in \ref RespFnOpt)
  StringArray nonlinearEqScaleTypes;
  /// vector of nonlinear equality constraint scaling factors (from the \c
  /// nonlinear_equality_scales specification in \ref RespFnOpt)
  RealVector nonlinearEqScales;

  // experimental data (for least squares and Bayesian algorithms)

  /// whether calibration data was specified
  bool calibrationDataFlag;
  /// number of distinct experiments in experimental data
  size_t numExperiments;
  /// number of experimental configuration vars (state variables) in
  /// each row of data
  size_t numExpConfigVars;
  /// list of num_experiments x num_config_vars configuration variable values
  RealVector expConfigVars;
  /// whether one should interpolate between the experiment and simulation field data 
  bool interpolateFlag;

  // next two can be retired?
  /// list of num_calibration_terms observation data
  RealVector expObservations;
  /// list of 1 or num_calibration_terms observation standard deviations
  RealVector expStdDeviations;

  /// name of experimental data file containing response data (with
  /// optional state variable and sigma data) to read
  String scalarDataFileName;
  /// tabular format of the scalar data file
  unsigned short scalarDataFormat;

  // derivative settings

  /// gradient type: none, numerical, analytic, or mixed (from the \c
  /// no_gradients, \c numerical_gradients, \c analytic_gradients, and
  /// \c mixed_gradients specifications in \ref RespGrad)
  String gradientType;
  /// Hessian type: none, numerical, quasi, analytic, or mixed (from
  /// the \c no_hessians, \c numerical_hessians, \c quasi_hessians,
  /// \c analytic_hessians, and \c mixed_hessians specifications in
  /// \ref RespHess)
  String hessianType;
  /// option to ignore bounds when doing finite differences (default
  /// is to honor bounds)
  bool ignoreBounds;
  /// Temporary(?) option to use old 2nd-order diffs when computing
  /// finite-difference Hessians; default is forward differences.
  bool centralHess;
  /// quasi-Hessian type: bfgs, damped_bfgs, or sr1 (from the \c bfgs 
  /// and \c sr1 specifications in \ref RespHess)
  String quasiHessianType;
  /// numerical gradient method source: dakota or vendor (from the \c
  /// method_source specification in \ref RespGradNum and \ref RespGradMixed)
  String methodSource;
  /// numerical gradient interval type: forward or central (from the \c
  /// interval_type specification in \ref RespGradNum and \ref RespGradMixed)
  String intervalType;
  /// vector of finite difference step sizes for numerical gradients, one step
  /// size per active continuous variable, used in computing 1st-order forward
  /// or central differences (from the \c fd_gradient_step_size
  /// specification in \ref RespGradNum and \ref RespGradMixed)
  RealVector fdGradStepSize;
  /// type of finite difference step to use for numerical gradient:
  /// relative - step length is relative to x
  /// absolute - step length is what is specified
  /// bounds - step length is relative to range of x
  String fdGradStepType;
  /// vector of finite difference step sizes for numerical Hessians, one step
  /// size per active continuous variable, used in computing 1st-order
  /// gradient-based differences and 2nd-order function-based differences
  /// (from the \c fd_hessian_step_size specification in \ref RespHessNum and
  /// \ref RespHessMixed)
  RealVector fdHessStepSize;
  /// type of finite difference step to use for numerical Hessian:
  /// relative - step length is relative to x
  /// absolute - step length is what is specified
  /// bounds - step length is relative to range of x
  String fdHessStepType;
  /// mixed gradient numerical identifiers (from the \c id_numerical_gradients
  /// specification in \ref RespGradMixed)
  IntSet idNumericalGrads;
  /// mixed gradient analytic identifiers (from the \c id_analytic_gradients
  /// specification in \ref RespGradMixed)
  IntSet idAnalyticGrads;
  /// mixed Hessian numerical identifiers (from the \c id_numerical_hessians
  /// specification in \ref RespHessMixed)
  IntSet idNumericalHessians;
  /// mixed Hessian quasi identifiers (from the \c id_quasi_hessians
  /// specification in \ref RespHessMixed)
  IntSet idQuasiHessians;
  /// mixed Hessian analytic identifiers (from the \c id_analytic_hessians
  /// specification in \ref RespHessMixed)
  IntSet idAnalyticHessians;

  // Field Data specification

  /// number of entries in each field
  IntVector fieldLengths;
  /// number of coordinates per field
  IntVector numCoordsPerField;
  /// Field data related storage:  whether to read simulation field coordinates
  bool readFieldCoords;
   /// Array which specifies the sigma type per response (none, one 
  /// constant value, one per response (vector) or a full covariance matrix
  StringArray varianceType; 

private:

  //
  //- Heading: Constructors, destructor, operators
  //

  DataResponsesRep();                                ///< constructor
  ~DataResponsesRep();                               ///< destructor

  //
  //- Heading: Member methods
  //

  /// write a DataResponsesRep object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataResponsesRep object from a packed MPI buffer
  void read(MPIUnpackBuffer& s); 
  /// write a DataResponsesRep object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  //
  //- Heading: Private data members
  //

  /// number of handle objects sharing this dataResponsesRep
  int referenceCount;
};   


inline DataResponsesRep::~DataResponsesRep()
{ }


/// Handle class for responses specification data.

/** The DataResponses class is used to provide a memory management
    handle for the data in DataResponsesRep.  It is populated by
    IDRProblemDescDB::responses_kwhandler() and is queried by the
    ProblemDescDB::get_<datatype>() functions.  A list of
    DataResponses objects is maintained in
    ProblemDescDB::dataResponsesList, one for each responses
    specification in an input file. */

class DataResponses
{
  //
  //- Heading: Friends
  //

  // the problem description database
  friend class ProblemDescDB;
  // the NIDR derived problem description database
  friend class NIDRProblemDescDB;

public:

  /// compares the idResponses attribute of DataResponses objects
  static bool id_compare(const DataResponses& dr, const std::string& id)
  { return id == dr.dataRespRep->idResponses; }

  //
  //- Heading: Constructors, destructor, operators
  //

  DataResponses();                                ///< constructor
  DataResponses(const DataResponses&);            ///< copy constructor
  ~DataResponses();                               ///< destructor

  DataResponses& operator=(const DataResponses&);   ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// write a DataResponses object to an std::ostream
  void write(std::ostream& s) const;

  /// read a DataResponses object from a packed MPI buffer
  void read(MPIUnpackBuffer& s);
  /// write a DataResponses object to a packed MPI buffer
  void write(MPIPackBuffer& s) const;

  /// return dataRespRep
  DataResponsesRep* data_rep();

private:

  //
  //- Heading: Data
  //

  /// pointer to the body (handle-body idiom)
  DataResponsesRep* dataRespRep;
};


inline DataResponsesRep* DataResponses::data_rep()
{return dataRespRep; }


/// MPIPackBuffer insertion operator for DataResponses
inline MPIPackBuffer& operator<<(MPIPackBuffer& s, const DataResponses& data)
{ data.write(s); return s;}


/// MPIUnpackBuffer extraction operator for DataResponses
inline MPIUnpackBuffer& operator>>(MPIUnpackBuffer& s, DataResponses& data)
{ data.read(s); return s;}


/// std::ostream insertion operator for DataResponses
inline std::ostream& operator<<(std::ostream& s, const DataResponses& data)
{ data.write(s); return s;}

inline void DataResponses::write(std::ostream& s) const
{ dataRespRep->write(s); }


inline void DataResponses::read(MPIUnpackBuffer& s)
{ dataRespRep->read(s); }


inline void DataResponses::write(MPIPackBuffer& s) const
{ dataRespRep->write(s); }

} // namespace Dakota

#endif
