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

  SingleExperiment();                                ///< constructor

  /// construct a single experiment from passed data
  // BMA TODO: Ask Laura about sigma
  SingleExperiment(int num_repl, const RealVector& real_config_vars,
		   const RealVector& scalar_data);


  SingleExperiment(const SingleExperiment&);         ///< copy constructor
  ~SingleExperiment();                               ///< destructor
 
  SingleExperiment& operator=(const SingleExperiment&);   ///< assignment operator

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

  ExpDataPerResponse();                                ///< constructor
  ExpDataPerResponse(int num_exp, int exp_type);
  ExpDataPerResponse(const ExpDataPerResponse&);         ///< copy constructor
  ~ExpDataPerResponse();                               ///< destructor
 
  ExpDataPerResponse& operator=(const ExpDataPerResponse&);   ///< assignment operator

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

  ExperimentData();                                ///< constructor
  ExperimentData(const ExperimentData&);            ///< copy constructor
  ~ExperimentData();                               ///< destructor
 
  ExperimentData& operator=(const ExperimentData&);   ///< assignment operator

  //
  //- Heading: Member methods
  //


  /// At the outer level, ExperimentData will just be a vector of ExpDataPerResponse;
  std::vector<ExpDataPerResponse> allExperiments;

private:


  //
  //- Heading: Data
  //
  
  /// allExperiments.resize(numResponseFunctions);


};


// free function for now; to be encapsulated
/// Read data in historical format into x, y, sigma matrices
void read_historical_data(const std::string& expDataFilename,
			  const std::string& context_message,
			  size_t numExperiments,
			  size_t numExpConfigVars,
			  size_t numFunctions,
			  size_t numExpStdDeviationsRead,
			  bool expDataFileAnnotated,
			  const RealVector& expStdDeviations,
			  bool calc_sigma_from_data,
			  RealMatrix& xObsData,
			  RealMatrix& yObsData,
			  RealMatrix& yStdData);
  
inline void
read_historical_data(const std::string& expDataFileName,
		     const std::string& context_message,
		     size_t numExperiments,
		     size_t numExpConfigVars,
		     size_t numFunctions,
		     size_t numExpStdDeviationsRead,
		     bool expDataFileAnnotated,
		     const RealVector& expStdDeviations,
		     bool calc_sigma_from_data,
		     RealMatrix& xObsData,
		     RealMatrix& yObsData,
		     RealMatrix& yStdData)
{

  // Read from a matrix with numExperiments rows and a number of cols
  // columns:  numExpConfigVars X, numFunctions Y, [numFunctions Sigma]
  RealMatrix experimental_data;

  size_t num_cols = numExpConfigVars + numFunctions + numExpStdDeviationsRead;

  TabularIO::read_data_tabular(expDataFileName, context_message, 
			       experimental_data, numExperiments,  num_cols, 
			       expDataFileAnnotated);

  // Get views of the data in 3 matrices for convenience

  size_t start_row, start_col;
  if (numExpConfigVars > 0) {
    start_row = 0;
    start_col = 0;
    RealMatrix x_obs_data(Teuchos::View, experimental_data,
			  numExperiments, numExpConfigVars,
			  start_row, start_col);
    xObsData.reshape(numExperiments, numExpConfigVars);
    for (int i=0; i<numExperiments; i++)
      for (int j=0; j<numExpConfigVars; j++)
        xObsData(i,j) = x_obs_data(i,j);
  }
 
  start_row = 0;
  start_col = numExpConfigVars;
  RealMatrix y_obs_data(Teuchos::View, experimental_data,
			numExperiments, numFunctions,
			start_row, start_col);
  yObsData.reshape(numExperiments, numFunctions);
  for (int i=0; i<numExperiments; i++)
    for (int j=0; j<numFunctions; j++)
      yObsData(i,j) = y_obs_data(i,j);

  // BMA TODO: The number of experimental functions may not match the
  // user functions, so can't assume numFunctions
  yStdData.reshape(numExperiments, numFunctions);
  if (numExpStdDeviationsRead > 0) {
    start_row = 0;
    start_col = numExpConfigVars + numFunctions;
    RealMatrix y_std_data(Teuchos::View, experimental_data,
			  numExperiments, numExpStdDeviationsRead,
			  start_row, start_col);
    // We allow 1 or numFunctions sigmas
    for (int i=0; i<numExperiments; i++)
      for (int j=0; j<numFunctions; j++) {
	if (numExpStdDeviationsRead == 1)
	  yStdData(i,j) = y_std_data(i,0);
	else
	  yStdData(i,j) = y_std_data(i,j);
      }
    // BMA: This is odd to me -- we overwrite the read values with
    // user values?  Commenting out as we don't currently support
    // input file-specified errors.
    // if (expStdDeviations.length()==1) {
    //   for (int i=0; i<numExperiments; i++)
    //     for (int j=0; j<numFunctions; j++)
    //       yStdData(i,j) = expStdDeviations(0);
    // }
    // else if (expStdDeviations.length()==numFunctions) {
    //   for (int i=0; i<numExperiments; i++)
    //     for (int j=0; j<numFunctions; j++)
    //       yStdData(i,j) = expStdDeviations(j);
    // }
  }
  else if (calc_sigma_from_data) {
    // calculate sigma terms
    Real mean_est, var_est;
    for (int j=0; j<numFunctions; j++){
      mean_est = 0;
      for (int i=0; i<numExperiments; i++)
        mean_est += yObsData(i,j);
      mean_est = mean_est / ((Real)numExperiments);
      var_est = 0;
      for (int i=0; i<numExperiments; i++)
        var_est += (yObsData(i,j)-mean_est)*(yObsData(i,j)-mean_est); 
      // BMA TODO: Is this correct in the 1 experiment case (sigma <- 1.0)?
      for (int i=0; i<numExperiments; i++)
        yStdData(i,j) = (numExperiments > 1) ? 
	  std::sqrt(var_est/(Real)(numExperiments-1)) : 1.0;
    }
  }
  else {
    yStdData = 0.0;
  }

}

} // namespace Dakota

#endif
