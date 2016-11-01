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

  /// The size of the ExperimentData changed; update the residualModel size
  void data_resize();


  /// manage best responses including residuals and model responses per config
  void print_best_responses(std::ostream& s, const Variables& best_submodel_vars,
                            const Response& best_submodel_resp,
                            size_t num_best, size_t best_ind);

protected:

  // ---
  // Construct time convenience functions
  // ---

  /// expand the variable counts to account for hyper-parameters
  static SizetArray variables_expand(const Model& sub_model, size_t num_hyper);

  /// determine the index into vc_totals corresponding to where the
  /// hyper-parameters go
  static int get_hyperparam_vc_index(const Model& sub_model);

  /// helper to compute the recast response order during member
  /// initialization; recast_resp_order passed is the minimum request
  /// client needs
  static short response_order(const Model& sub_model, 
                              short recast_resp_order = 1);

  /// compute the primary response map for a data transform RecastModel
  void gen_primary_resp_map(const SharedResponseData& srd,
			    Sizet2DArray& primary_resp_map_indices,
			    BoolDequeArray& nonlinear_resp_map) const;
  
  /// specialization of evaluate that iterates over configuration variables
  void derived_evaluate(const ActiveSet& set);
  /// specialization of evaluate that iterates over configuration variables
  void derived_evaluate_nowait(const ActiveSet& set);

  /// synchronize all evaluations (all residuals for all experiment
  /// configurations)
  const IntResponseMap& derived_synchronize();

  /// return any evaluations for which all experiment configurations
  /// have completed
  const IntResponseMap& derived_synchronize_nowait();

  // Synchronize the subModel and filter the IntResponseMap in-place,
  // caching any that we didn't schedule.
  const IntResponseMap& filter_submodel_responses();

  /// cache the subModel responses into a per-RecastModel eval ID map
  void cache_submodel_responses(const IntResponseMap& sm_resp_map,
                                bool deep_copy);

  /// collect any (or force all) completed subModel evals and populate
  /// recastResponseMap with residuals for those that are fully completed
  void collect_residuals(bool collect_all);

  /// transform a set of per-configuration subModel Responses to a
  /// single evaluation's residuals
  void transform_response_map(const IntResponseMap& submodel_resp,
			      const Variables& recast_vars,
			      Response& residual_resp) ;

  // ---
  // Callback functions that perform data transform during the Recast operations
  // ---

  /// map the inbound expanded variables to the sub-model, discarding
  /// hyperparams (assumes hyper-parameters are at end of active
  /// continuous variables)
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

  /// scale the populated residual response by any covariance
  /// information, including hyper-parameter multipliers
  void scale_response(const Variables& submodel_vars, 
		      const Variables& recast_vars,
		      Response& recast_response);

  // NOTE: Shouldn't need non-default active set or secondary response
  // recast; default based on indices should suffice.

  /// Initialize continuous variable values/labels
  void init_continuous_vars();

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

  void print_residual_response(const Response& resid_resp);

  void recover_submodel_responses(std::ostream& s,
                                  const Variables& best_submodel_vars,
                                  size_t num_best, size_t best_ind,
                                  Response& residual_resp);

  /// Reference to the experiment data used to construct this Model
  const ExperimentData& expData;

  /// static pointer to this class for use in static callbacks
  static DataTransformModel* dtModelInstance;

  /// Number of calibrated variance multipliers
  size_t numHyperparams;

  /// Calibration mode for the hyper-parameters
  unsigned short obsErrorMultiplierMode;

  // BMA TODO: fix this terrible typedef
  typedef std::map<int, IntResponseMap> IntIntResponseMapMap;
  IntIntResponseMapMap cachedResp;

};

} // namespace Dakota

#endif
