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

/// special values for sigmaType 
enum sigtype { NO_SIGMA, SCALAR_SIGMA, DIAGONAL_SIGMA, MATRIX_SIGMA };
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

  /// default constructor
  ExperimentData()
  { /* empty ctor */ }                                

  /// typical constructor
  ExperimentData(short output_level);

  //ExperimentData(const ExperimentData&);            ///< copy constructor
  //~ExperimentData();                               ///< destructor
   //ExperimentData& operator=(const ExperimentData&);   ///< assignment operator

  //
  //- Heading: Member methods
  //


  // sizing methods

  /// set the shared response data
  void shared_data(const SharedResponseData& srd);

  /// set the number of experiments
  void num_experiments(size_t num_experiments_in);

  /// set the number of configuration (state) variables to be read
  void num_config_vars(size_t num_config_vars_in);

  /// set the array of sigmas, one per scalar response / field response group
  void sigma_type(const StringArray& sigma_type_in);

  /// Load experiments from legacy file format
  void load_data(const std::string& expDataFilename,
		 const std::string& context_message,
		 bool expDataFileAnnotated,
		 bool calc_sigma_from_data);

  /// retrieve the vector of configuration variables for the given
  /// experiment number
  const RealVector& config_vars(size_t experiment);

  /// retrieve the data value for the given response, for the given
  /// experiment 
  Real scalar_data(size_t response, size_t experiment);

  /// retrieve the standard deviation value for the given response, for
  /// the given experiment
  Real scalar_sigma(size_t response, size_t experiment);

private:

  // initialization helpers

  /// parse user-provided sigma type strings and populate enums
  void parse_sigma_types(const StringArray& sigma_types);

  // data loading helpers

  /// Load a single experiment exp_index into exp_resp
  void load_experiment(size_t exp_index, std::ifstream& scalar_data_stream, 
		       size_t num_sigma_matrices, size_t num_sigma_diagonals, 
		       size_t num_sigma_scalars, Response& exp_resp);

  /// read or default populate the scalar sigma
  void read_scalar_sigma(std::ifstream& scalar_data_stream, 
			 RealVector& sigma_scalars, 
			 IntVector& scalar_map_indices);

  //
  //- Heading: Data
  //

  /// archived shared data for use in sizing fields, total functions
  /// (historically we read all functions, including constraints,
  /// which might not be correct)
  SharedResponseData simulationSRD;

  /// the total number of experiments
  size_t numExperiments;
 
  /// number of configuration (state) variables to read for each experiment
  size_t numConfigVars;

  // empty sigmaType indicates none specified?!?
  /// type of sigma specified for each variable, one per response group
  UShortArray sigmaTypes;

  /// number of sigma values to read from each row in simple data file format
  size_t scalarSigmaPerRow;

  /// Vector of numExperiments ExperimentResponses, holding the
  /// observed data and error (sigma/covariance) for each experiment.
  std::vector<Response> allExperiments;
  
  // TODO: migrate this to a vector of Variables?
  /// Vector of numExperiments configurations at which data were
  /// gathered; empty if no configurations specified.
  std::vector<RealVector> allConfigVars;

  // TO REMOVE
  /// Temporary container to hold sigma data while migrating to
  /// ExperimentCovariance
  RealMatrix sigmaScalarValues;

  /// output verbosity level
  short outputLevel;

};

} // namespace Dakota

#endif
