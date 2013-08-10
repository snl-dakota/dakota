/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:        ExperimentData
//- Description:
//-
//-
//- Owner:        Laura Swiler
//- Version: $Id$

#ifndef EXPERIMENT_DATA_H 
#define EXPERIMENT_DATA_H 

#include "dakota_system_defs.hpp"
#include "dakota_data_types.hpp"
#include "dakota_tabular_io.hpp"

namespace Dakota {

// free function for now; to be encapsulated
/// Read data in historical format into x, y, sigma matrices
void read_historical_data(const std::string& expDataFileName,
			  const std::string& context_message,
			  size_t numExperiments,
			  size_t numExpConfigVars,
			  size_t numFunctions,
			  size_t numExpStdDeviationsRead,
			  bool expDataFileAnnotated,
			  bool calc_sigma_from_data,
			  RealMatrix& xObsData,
			  RealMatrix& yObsData,
			  RealMatrix& yStdData);
  
// special values for sigma_type 
enum sigtype { NO_SIGMA, SCALAR_SIGMA, COVARIANCE_MATRIX };
// special values for experimental data type 
enum edtype { SCALAR_DATA, FUNCTIONAL_DATA } ;


class SingleExperiment 
{

public:

  //
  //- Heading: Constructors, destructor, operators
  //

  SingleExperiment()
  { /* empty ctor */ }                                ///< constructor

  /// construct a single experiment from passed data
  SingleExperiment(int num_repl, const RealVector& real_config_vars,
		   int sigma_type, const RealVector& sigma_scalar,
		   const RealVector& scalar_data);


  //SingleExperiment(const SingleExperiment&);         ///< copy constructor
  //~SingleExperiment();                               ///< destructor
 
  //SingleExperiment& operator=(const SingleExperiment&);   ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// return number of replicates per experiment.  To be consistent 
  /// with the previous notation, we need this per experiment, per response
  int numReplicates;

  /// return int configuration variables.  There will be just one set of 
  /// configuration variables per experiment.
  IntVector intConfigVars;

  /// return real configuration variables
  RealVector realConfigVars;
   
  /// return sigma_type. I think this should be per experiment, though it 
  /// could be per response function. 
  int sigmaType;
 
  /// return sigma scalar if sigma_type = SCALAR
  RealVector sigmaScalar;

  /// return sigma covariance if sigma_type = COVARIANCE_MATRIX
  RealMatrix sigmaCovariance;
 
  /// return the scalar data.  I am assuming we need a data structure 
  /// at least with num_replicates per experiment. Each replicate within 
  /// this experiment returns one scalar value, all of the replicates are 
  /// in the vector
  RealVector experimentScalarData;

  /// return functional data.  Each element of the array is one vector of f(x) 
  /// values, the array represents the aggregation of replicates
  /// experimentFunctionalData[replicate][data element f(x_i)]
  RealVectorArray experimentFunctionalData;

  /// return coordinates corresponding to functional data. 
  //  I have set this up to be a RealVector2DArray for now.  
  //  This should allow for something like
  //  coordinates[replicate][data element x_i] which returns a vector [x_i]
  RealVector2DArray coordinates;
  // BMA NOTE: the coordinate array should probably allow for slicing all the x points instead of segregating by data element first... Multiarray?


  /// Interpolation method for interpolating between experimental and model data. 
  /// I need to work on inputs/outputs to this method.  For now, this assumes
  /// interpolation of functional data. 
  void interpolate(RealVector2DArray& functionalCoordinates, RealVectorArray& 
    experimentFunctionalData, RealVector2DArray& modelCoordinates, RealVectorArray&
    modelFunctionalData, RealVectorArray& interpolatedResults); 
  /// As Brian suggested, thsi class has the experimental data (coordinates and 
  // functional data) and can store the interpolated results.  So, we may want a
  // simpler interpolate definition given in the line below: 
  void interpolate(RealVector2DArray& modelCoordinates, RealVectorArray&
    modelFunctionalData); 

  RealVectorArray interpolatedResults; 
  
};



class ExpDataPerResponse{ 

public:

  //
  //- Heading: Constructors, destructor, operators
  //

  ExpDataPerResponse()
  { /* empty ctor */ } ;                                ///< constructor

  /// Constructor for legacy data, where each experiment has the same
  /// number of replicates = 1, same sigma type, etc.
  ExpDataPerResponse(int num_exp, int exp_type);


  //ExpDataPerResponse(const ExpDataPerResponse&);         ///< copy constructor
  //~ExpDataPerResponse();                               ///< destructor
 
  //ExpDataPerResponse& operator=(const ExpDataPerResponse&);   ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// return the number of experiments for this response 
  int numExperiments;
 
  /// return the experimental type for this response. 
  /// Only one type (scalar or functional, enumerated type) is allowed per response 
  int experimentType;

  /// vector containing all of the single experiments for this response
  std::vector<SingleExperiment> dataThisResponse;

  //dataThisResponse.resize(numExperiments);
};






/** The ExperimentData class is used to read and populate data 
    (currently from user-specified files and/or the input spec)
    relating to experimental (physical observations) data for 
    the purposes of calibration.  Such data may include (for example): 
    number of experiments, number of replicates, configuration variables, 
    type of data (scalar vs. functional), treatment of sigma (experimental
    uncertainties).  This class also provides an interpolation capability to 
    interpolate between simulation or experimental data so that the 
    differencing between simulation and experimental data may 
    be performed properly. */

class ExperimentData
{

public:

  //
  //- Heading: Constructors, destructor, operators
  //

  ExperimentData()
  { /* empty ctor */ } ;                                ///< constructor

  /// Constructor from legacy file format
  void load_scalar(const std::string& expDataFilename,
		   const std::string& context_message,
		   size_t numExperiments,
		   size_t numExpConfigVars,
		   size_t numFunctions,
		   size_t numExpStdDeviationsRead,
		   bool expDataFileAnnotated,
		   bool calc_sigma_from_data,
		   short verbosity);

  //ExperimentData(const ExperimentData&);            ///< copy constructor
  //~ExperimentData();                               ///< destructor
 
  //ExperimentData& operator=(const ExperimentData&);   ///< assignment operator

  //
  //- Heading: Member methods
  //

  const RealVector& config_vars(size_t response, size_t experiment);

  Real scalar_data(size_t response, size_t experiment, size_t replicate);

  Real scalar_sigma(size_t response, size_t experiment, size_t replicate);

  /// At the outer level, ExperimentData will just be a vector of ExpDataPerResponse;
  std::vector<ExpDataPerResponse> allExperiments;

private:


  //
  //- Heading: Data
  //
  
  /// allExperiments.resize(numResponseFunctions);


};

} // namespace Dakota

#endif
