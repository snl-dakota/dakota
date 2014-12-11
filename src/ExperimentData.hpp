/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
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
#include "ExperimentResponse.hpp"
#include "SharedResponseData.hpp"

namespace Dakota {

using boost::multi_array;
using boost::extents;
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
  
/// special values for sigma_type 
enum sigtype { NO_SIGMA, SCALAR_SIGMA, COVARIANCE_MATRIX };
/// special values for experimental data type 
enum edtype { SCALAR_DATA, FUNCTIONAL_DATA } ;


  /// Interpolation method for interpolating between experimental and model data. 
  /// I need to work on inputs/outputs to this method.  For now, this assumes
  /// interpolation of functional data. 
  /* void interpolate(RealVector2DArray& functionalCoordinates, RealVectorArray& 
     experimentFunctionalData, RealVector2DArray& modelCoordinates, RealVectorArray&
     modelFunctionalData, RealVectorArray& interpolatedResults);
  */ 
  /// As Brian suggested, thsi class has the experimental data (coordinates and 
  // functional data) and can store the interpolated results.  So, we may want a
  // simpler interpolate definition given in the line below: 
  /*void interpolate(RealVector2DArray& modelCoordinates, RealVectorArray&
    modelFunctionalData); 
  */
  /// RealVectorArray interpolatedResults; 
  

/** The ExperimentData class is used to read and populate data 
    (currently from user-specified files and/or the input spec)
    relating to experimental (physical observations) data for 
    the purposes of calibration.  Such data may include (for example): 
    number of experiments, configuration variables, 
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
  void load_data(const std::string& expDataFilename,
		   const std::string& context_message,
		   size_t numExperiments,
		   size_t numExpConfigVars,
		   size_t numFunctions,
		   size_t numExpStdDeviationsRead,
		   bool expDataFileAnnotated,
		   bool calc_sigma_from_data,
		   short verbosity, 
                   const SharedResponseData& shared_resp_data);

  //ExperimentData(const ExperimentData&);            ///< copy constructor
  //~ExperimentData();                               ///< destructor
 
  //ExperimentData& operator=(const ExperimentData&);   ///< assignment operator

  //
  //- Heading: Member methods
  //

  /// retrieve the vector of configuration variables for the given
  /// experiment number
  const RealVector& config_vars(size_t experiment);

  /// retrieve the data value for the given response, for the given
  /// experiment 
  Real scalar_data(size_t response, size_t experiment);

  /// retrieve the standard deviation value for the given response, for
  /// the given experiment
  Real scalar_sigma(size_t response, size_t experiment);

  /// ExperimentData will contain a vector of ExperimentResponses.  The size of this vector is numExperiments.
  std::vector<Response> allExperiments;

  /// ExperimentData will contain a vector of configVars.  The size of this vector is numExperiments.
  std::vector<RealVector> allConfigVars;

private:


  //
  //- Heading: Data
  //

  /// return the number of experiments for this response 
  /// int numExperiments;
 
  /// return the experimental type for this response. 
  /// Only one type (scalar or functional, enumerated type) is allowed per response 
  //int experimentType;
  RealMatrix sigmaScalarValues;

};

} // namespace Dakota

#endif
