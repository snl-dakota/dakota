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
#include "ExperimentDataUtils.hpp"
#include "SharedResponseData.hpp"
#include "SimulationResponse.hpp"
#include <boost/filesystem/operations.hpp>
#include "boost/filesystem/path.hpp"

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
  ExperimentData();

  /// typical DB-based constructor
  ExperimentData(const ProblemDescDB& prob_desc_db, 
                 const SharedResponseData& srd, short output_level);

  /// temporary? constructor for testing
  ExperimentData(size_t num_experiments, size_t num_config_vars, 
                 const boost::filesystem::path& data_prefix,
                 const SharedResponseData& srd,
                 const StringArray& variance_types,
                 short output_level,
                 std::string scalarDataFilename = "");
 
  ExperimentData(size_t num_experiments, const SharedResponseData& srd,
                 const RealMatrix& configVars, 
                 const IntResponseMap& all_responses, short output_level); 

  //ExperimentData(const ExperimentData&);            ///< copy constructor
  //~ExperimentData();                               ///< destructor
  //ExperimentData& operator=(const ExperimentData&);  ///< assignment operator

  //
  //- Heading: Member methods
  //
 
  /// Load experiments from data files (simple scalar or field)
  void load_data(const std::string& context_message);
  /// Add one data point to the experimental data set
  void add_data(const RealVector& one_configvars, const Response& one_response);

  /// retrieve the number of experiments
  size_t num_experiments() const
    { return allExperiments.size(); }

  /// retrieve the total number of experimental data points 
  /// over all experiments
  size_t num_total_exppoints() const;

  /// retrieve the number of scalars (applies to all experiments)
  size_t num_scalars() const;

  /// retrieve the number of fields (applies to all experiments)
  size_t num_fields() const;

  /// number of onfiguration variables
  size_t num_config_vars() const;

  /// values of the configuration variables, 1 RealVector per experiment
  const std::vector<RealVector>& config_vars() const;

  /// return contiguous vector of all data (scalar, followed by field)
  /// for the specified experiment
  const RealVector& all_data(size_t experiment);
  
  /// return response for the specified experiment
  const Response& response(size_t experiment);

  /// return the individual sizes of the experimental data lengths
  /// (all function values, scalar and field)
  void per_exp_length(IntVector& per_length) const;

  /// return the field lengths for specified experiment index
  const IntVector& field_lengths(size_t experiment) const;

  /// retrieve the data value for the given response, for the given
  /// experiment 
  Real scalar_data(size_t response, size_t experiment);

  /// retrieve a view of the field data for the given response, for the given
  /// experiment 
  RealVector field_data_view(size_t response, size_t experiment) const;

  /// retrieve a view of the field data coordinates for the given response, for the given
  /// experiment 
  RealMatrix field_coords_view(size_t response, size_t experiment) const;

  /// whether the specified variance type (enum value) is present and active
  bool variance_type_active(short variance_type) const;

  /// whether any variance type is active
  bool variance_active() const;

  /// apply the covariance responses to compute the triple product
  /// v'*inv(C)*v for the given experiment
  Real apply_covariance(const RealVector& residuals, size_t experiment) const;
  /// apply inverse sqrt of the covariance to compute weighted residuals
  void apply_covariance_inv_sqrt(const RealVector& residuals, 
				 size_t experiment, 
				 RealVector& weighted_residuals) const;
  /// apply inverse sqrt of the covariance to compute weighted gradients
  void apply_covariance_inv_sqrt(const RealMatrix& gradients, 
				 size_t experiment, 
				 RealMatrix& weighted_gradients) const;
  /// apply inverse sqrt of the covariance to compute weighted Hessians
  void apply_covariance_inv_sqrt(const RealSymMatrixArray& hessians, 
				 size_t experiment, 
				 RealSymMatrixArray& weighted_hessians) const;

  /// return a (copy) vector containing the main diagonal entries of a specified
  /// experimental covariance matrix
  void get_main_diagonal( RealVector &diagonal, size_t experiment ) const;

  /// get the standard deviation of the observation error process, one
  /// vector per experiment
  void cov_std_deviation(RealVectorArray& std_deviation) const;

  /// get the observation error covariance as a correlation matrix, one
  /// vector per experiment
  void cov_as_correlation(RealSymMatrixArray& corr_matrix) const;

  /// retrieve an individual covariance entry as a dense matrix
  void covariance(int exp_ind, RealSymMatrix& cov_mat) const;

  /// form residuals for all experiments, interpolating if necessary;
  /// one simulation response maps to all experiments
  void form_residuals(const Response& sim_resp, Response& residual_resp) const;
    
  /// Populate the portion of residual_resp corresponding to
  /// experiment curr_exp; the passed simulation response maps only to
  /// the specified experiment
  void form_residuals(const Response& sim_resp, const size_t curr_exp,
		      Response& residual_resp) const;

  /// form residuals for an individual experiment, interpolating if necessary 
  void form_residuals(const Response& sim_resp, size_t exp_num, 
		      const ShortArray &total_asv, size_t residual_resp_offset,
		      Response &residual_resp) const; 

  /// recover original model from the first experiment block in a full
  /// set of residuals; works in no interpolation case only (sizes same)
  void recover_model(size_t num_pri_fns, RealVector& model_fns) const;

  /// flag for interpolation.  If 0, no interpolation. 
  /// If 1, interpolate. 
  bool interpolate_flag() const;

  /// Interpolate simulation data (values, gradients and hessians) onto
  /// the coordinates of the experimental data
  void interpolate_simulation_data( const Response &sim_resp, 
				    size_t exp_num,
				    const ShortArray &total_asv, 
				    size_t exp_offset,
				    Response &interp_resp ) const; 

  /// Apply the experiment data covariance to the residual data (scale
  /// functions by Gamma_d^{-1/2}), returning in scaled_residuals
  void scale_residuals(const Response& residual_response, 
		       RealVector& scaled_residuals) const;

  /// Apply the experiment data covariance to the residual data in-place 
  /// (scale functions, gradients, and Hessians by Gamma_d^{-1/2})
  void scale_residuals(Response& residual_response) const;

  // All the following now assume any covariance scaling is already applied

  /// Build the gradient of the ssr from residuals and function gradients
  /// based on the response's active set request vector
  void build_gradient_of_sum_square_residuals(const Response& resp,
					      RealVector &ssr_gradient);
  /// Build the gradient of the ssr from residuals and function gradients
  /// using the passed active set request vector (overrides the response's
  /// request vector)
  void build_gradient_of_sum_square_residuals(const Response& resp,
					      const ShortArray& asrv,
					      RealVector &ssr_gradient);

  /// Update the gradient of ssr with the values from the gradient associated
  /// with a single experiment
  void build_gradient_of_sum_square_residuals_from_response( 
						    const Response& resp, 
						    const ShortArray& asrv,
						    int exp_ind,
						    RealVector &ssr_gradient);

  /** \brief Construct the gradient of the sum of squares of residuals
   *  
   * \param func_gradients A matrix containing the gradients of the
   * residual vector
   * \param residuals A vector of residuals (mismatch between experimental data
   * and the corresponding function values
   * \param asrv The active set request vector
   */  
  void build_gradient_of_sum_square_residuals_from_function_data(
    const RealMatrix &func_gradients, const RealVector &residuals,
    RealVector &ssr_gradient, const ShortArray& asrv);

  /// Build the hessian of the ssr from residuals, function gradients and
  /// function hessians based on the response's active set request vector
  void build_hessian_of_sum_square_residuals(const Response& resp,
					     RealSymMatrix &ssr_hessian);
  /// Build the hessian of the ssr from residuals, function gradients and
  /// function hessians using the passed active set request vector (overrides
  /// the response's request vector)
  void build_hessian_of_sum_square_residuals(const Response& resp,
					     const ShortArray& asrv,
					     RealSymMatrix &ssr_hessian);

  /// Update the hessian of ssr with the values from the hessian associated
  /// with a single experiment
  void build_hessian_of_sum_square_residuals_from_response( 
						    const Response& resp, 
						    const ShortArray& asrv,
						    int exp_ind,
						    RealSymMatrix &ssr_hessian);

  /** \brief Construct the hessian of the sum of squares of residuals
   *  
   * \param func_hessians A list of matrices containing the Hessians of the 
   * function elements in the residual vector
   * \param func_gradients A matrix containing the gradients of the
   * residual vector
   * \param residuals A vector of residuals (mismatch between experimental data
   * and the corresponding function values
   * \param asrv The active set request vector
   */  
  void build_hessian_of_sum_square_residuals_from_function_data(
    const RealSymMatrixArray &func_hessians, 
    const RealMatrix &func_gradients, const RealVector &residuals,
    RealSymMatrix &ssr_hessian, const ShortArray& asrv);

  /// in-place scale the residual response (functions, gradients,
  /// Hessians) by sqrt(multipliers), according to blocks indicated by
  /// multiplier mode
  void scale_residuals(const RealVector& multipliers, 
		       unsigned short multiplier_mode, size_t num_calib_params,
		       Response& residual_response) const;


  /// returns the determinant of (covariance block-scaled by the
  /// passed multipliers)
  Real cov_determinant(const RealVector& multipliers, 
                       unsigned short multiplier_mode) const;

  /// returns the log of the determinant of (covariance block-scaled
  /// by the passed multipliers)
  Real half_log_cov_determinant(const RealVector& multipliers, 
				unsigned short multiplier_mode) const;
  
  /// populated the passed gradient with derivatives w.r.t. the
  /// hyper-parameter multipliers, starting at hyper_offset (must be
  /// sized)
  void half_log_cov_det_gradient(const RealVector& multipliers, 
				 unsigned short multiplier_mode,
				 size_t hyper_offset,
				 RealVector& gradient) const;

  /// populated the passed Hessian with derivatives w.r.t. the
  /// hyper-parameter multipliers, starting at hyper_offset (must be
  /// sized)
  void half_log_cov_det_hessian(const RealVector& multipliers, 
				unsigned short multiplier_mode, 
				size_t hyper_offset,
				RealSymMatrix& hessian) const;

  /// generate variable labels for the covariance (error) multiplier hyperparams
  StringArray hyperparam_labels(unsigned short multiplier_mode) const;

protected:

  /// Perform check on the active request vector to make sure
  /// it is amenable to interpolation of simulation data and application
  /// of apply covariance
  ShortArray determine_active_request(const Response& resid_resp) const;

  /// count the number of residuals influenced by each multiplier
  SizetArray residuals_per_multiplier(unsigned short multiplier_mode) const;

  /// Generate a set of multipliers commensurate with the residual
  /// size for the total experiment data set.  Instead of repeating
  /// the loops all over the place, generate an expanded set of
  /// multipliers; the conditionals get too complicated otherwise
  void generate_multipliers(const RealVector& multipliers,
                            unsigned short multiplier_mode,
                            RealVector& expanded_multipliers) const;

  void resid2mult_map(unsigned short multiplier_mode,
		      IntVector& resid2mult_indices) const;

private:

  // initialization helpers

  /// shared body of constructor initialization
  void initialize(const StringArray& variance_types, 
		  const SharedResponseData& srd);

  /// parse user-provided sigma type strings and populate enums
  void parse_sigma_types(const StringArray& sigma_types);

  // data loading helpers

  /// Load a single experiment exp_index into exp_resp
  void load_experiment(size_t exp_index, std::ifstream& scalar_data_stream, 
		       size_t num_field_sigma_matrices, 
		       size_t num_field_sigma_diagonals, 
		       size_t num_field_sigma_scalars, 
		       size_t num_field_sigma_none, Response& exp_resp);

  /// read or default populate the scalar sigma
  void read_scalar_sigma(std::ifstream& scalar_data_stream, 
			 RealVector& sigma_scalars, 
			 IntVector& scalar_map_indices);


  /// Return a view (to allowing updaing in place) of the residuals associated
  /// with a given experiment, from a vector contaning residuals from
  /// all experiments
  RealVector residuals_view(const RealVector& residuals, size_t experiment) const;
  
  /// Return a view (to allowing updaing in place) of the gradients associated
  /// with a given experiment, from a matrix contaning gradients from
  /// all experiments
  RealMatrix gradients_view( const RealMatrix &gradients, size_t experiment) const;
  
  /// Return a view (to allowing updaing in place) of the hessians associated
  /// with a given experiment, from an array contaning the hessians from 
  /// all experiments
  RealSymMatrixArray hessians_view(const RealSymMatrixArray &hessians, 
				   size_t experiment) const;

  //
  //- Heading: Data
  //


  // configuration and sizing information

  /// whether the user specified a calibration data block
  bool calibrationDataFlag;

  /// the total number of experiments
  size_t numExperiments;
 
  /// number of configuration (state) variables to read for each experiment
  size_t numConfigVars;

  /// type of variance specified for each variable, one per response
  /// group; empty varianceType indicates none specified by user
  UShortArray varianceTypes;

  /// cached product of each experiment covariance's determinant 
  Real covarianceDeterminant;

  /// cached sum of each experiment covariance's log determinant 
  Real logCovarianceDeterminant;

  /// path to prepend to any data file names
  boost::filesystem::path dataPathPrefix;

  // scalar data config information

  /// the user-specied scalar data filename
  String scalarDataFilename;
  
  /// tabular format of the simple scalar data file; supports
  /// TABULAR_NONE, TABULAR_HEADER, TABULAR_EVAL_ID, TABULAR_EXPER_ANNOT
  unsigned short scalarDataFormat;

  /// number of sigma values to read from each row in simple data file
  /// format (calculated from variance types strings
  size_t scalarSigmaPerRow;

  /// whether to read coordinate data files for simulation fields
  bool readSimFieldCoords;

  /// archived shared data for use in sizing fields, total functions
  /// (historically we read all functions, including constraints,
  /// which might not be correct)
  SharedResponseData simulationSRD;

  /// flag for interpolation.  
  bool interpolateFlag;
  /// output verbosity level
  short outputLevel;

  // core data storage

  /// Vector of numExperiments ExperimentResponses, holding the
  /// observed data and error (sigma/covariance) for each experiment.
  std::vector<Response> allExperiments;
  
  // TODO: migrate this to a vector of Variables?
  /// Vector of numExperiments configurations at which data were
  /// gathered; empty if no configurations specified.
  std::vector<RealVector> allConfigVars;

  /// Length of each experiment
  IntVector experimentLengths;
  /// function index offsets for individual experiment data sets
  IntVector expOffsets;
};


inline void ExperimentData::
build_gradient_of_sum_square_residuals( const Response& resp, 
					RealVector &ssr_gradient )
{
  // default to asrv in resp (no override)
  build_gradient_of_sum_square_residuals( resp,
					  resp.active_set_request_vector(),
					  ssr_gradient);
}

inline void ExperimentData::
build_hessian_of_sum_square_residuals( const Response& resp, 
				       RealSymMatrix &ssr_hessian )
{
  // default to asrv in resp (no override)
  build_hessian_of_sum_square_residuals( resp, resp.active_set_request_vector(),
					 ssr_hessian);
}

} // namespace Dakota

#endif
