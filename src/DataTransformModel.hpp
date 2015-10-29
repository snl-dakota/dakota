/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataTransformModel
//- Description: Specialization of a RecastModel that manages the mapping from
//-              a simulation model to residuals based on data differencing.
//- Owner:       Brian Adams
//- Checked by:
//- Version: $Id$

#ifndef DATA_TRANSFORM_MODEL_H
#define DATA_TRANSFORM_MODEL_H

#include "RecastModel.hpp"

namespace Dakota {

/// forward declarations
class ExperimentData;

// BMA TODO: Consider using separate RecastModel to include hyper-parameters

/// Data transformation specialization of RecastModel

/** Specialization of RecastModel to create a residual model that maps
    (1) from an augmented set of calibration parameters (including
    hyper-parameters) to those needed by the underlying simulation
    model and (2) from the simulation model response to a set of
    residuals, whose overall size may differ from the simulation
    (sub-model) response.  The residuals may be scaled by experiment
    covariance information.  This class provides a simple constructor
    that forwards to the more complicated RecastModel API */
class DataTransformModel: public RecastModel
{
public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  DataTransformModel(const Model& sub_model, const ExperimentData& exp_data,
                     size_t num_hyper = 0,
                     unsigned short mult_mode = CALIBRATE_NONE, 
                     short recast_resp_deriv_order = 1);

  /// destructor
  ~DataTransformModel();

  /// Convenience function to help recover a residual response from the submodel
  void data_transform_response(const Variables& sub_model_vars, 
                               const Response& sub_model_resp,
                               Response& residual_resp);


protected:

  // ---
  // Construct time convenience functions
  // ---

  /// expand the variable counts to account for hyper-parameters
  static SizetArray variables_expand(const Model& sub_model, size_t num_hyper);

  /// helper to compute the recast response order during member
  /// initialization; recast_resp_order passed is the minimum request
  /// client needs
  static short response_order(const Model& sub_model, 
                              short recast_resp_order = 1);

  /// compute the primary response map for a data transform RecastModel
  void gen_primary_resp_map(const SharedResponseData& srd,
			    Sizet2DArray& primary_resp_map_indices,
			    BoolDequeArray& nonlinear_resp_map) const;
  
  // ---
  // Callback functions that perform data transform during the Recast operations
  // ---

  /// map the inbound expanded variables to the sub-model, discarding
  /// hyperparams
  static void vars_mapping(const Variables& recast_vars, 
			   Variables& submodel_vars);

  // BMA TODO: inverse isn't well-defined, but may need for active vars...

  /// map the inbound ActiveSet to the sub-model (map derivative variables)
  static void set_mapping(const Variables& recast_vars,
			  const ActiveSet& recast_set,
			  ActiveSet& sub_model_set);

  // BMA TODO: shouldn't need static in this context; find another way
  /// Recast callback function to difference residuals with observed data
  static void primary_resp_differencer(const Variables& submodel_vars, 
				       const Variables& recast_vars,
				       const Response& submodel_response, 
				       Response& recast_response);

  /// Core of data difference, which doesn't perform any output
  /// (silent version may not be needed if we manage verbosity more
  /// carefully)
  void data_difference_core(const Variables& submodel_vars, 
   			    const Variables& recast_vars,
   			    const Response& submodel_response, 
   			    Response& recast_response);

  // NOTE: Shouldn't need non-default active set or secondary response
  // recast; default based on indices should suffice.

  /// expand the variable labels to include the hyper parameters
  void expand_var_labels(const Model& sub_model);

  /// (if non-empty) expand submodel_array by replicates to populate a
  /// recast_array
  template<typename T>
  void expand_array(const SharedResponseData& srd, const T& submodel_array, 
                    size_t recast_size, T& recast_array) const;

  /// If size greater than 1, expand submodel_array by replicates to
  /// populate a pre-sized recast_array, otherwise copy
  template<typename T>
  void expand_scales_array(const SharedResponseData& srd, 
                           const T& submodel_array, 
                           size_t submodel_size, size_t recast_size, 
                           T& recast_array) const;

  /// Reference to the experiment data used to construct this Model
  const ExperimentData& expData;

  /// static pointer to this class for use in static callbacks
  static DataTransformModel* dtModelInstance;

  /// Number of calibrated variance multipliers
  size_t numHyperparams;

  /// Calibration mode for the hyper-parameters
  unsigned short obsErrorMultiplierMode;

};

} // namespace Dakota

#endif
