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

/// Data transformation specialization of RecastModel

/** Specialization of RecastModel to create a residual model that maps
    from a simulation model response to a set of residuals, whose
    overall size may differ from the simulation (sub-model) response.
    This class provides a simple constructor that forwards to the more
    complicated RecastModel API */
class DataTransformModel: public RecastModel
{
public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  DataTransformModel(const Model& sub_model, const ExperimentData& exp_data);

  /// destructor
  ~DataTransformModel();

protected:

  /// helper to compute the recast response order during member initialization
  static short response_order(const Model& sub_model);

  /// compute the primary response map for a data transform RecastModel
  void gen_primary_resp_map(const SharedResponseData& srd,
			    Sizet2DArray& primary_resp_map_indices,
			    BoolDequeArray& nonlinear_resp_map) const;
  
  // BMA TODO: shouldn't need static in this context; find another way
  /// Recast callback function to difference residuals with observed data
  static void primary_resp_differencer(const Variables& raw_vars, 
				       const Variables& residual_vars,
				       const Response& raw_response, 
				       Response& residual_response);

  /// Core of data difference, which doesn't perform any output
  /// (silent version may not be needed if we manage verbosity more
  /// carefully)
  void data_difference_core(const Response& raw_response, 
			    Response& residual_response);

  // NOTE: Shouldn't need non-default active set or secondary response
  // recast; default based on indices should suffice.

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
  //  const String contextMessage;

  /// static pointer to this class for use in static callbacks
  static DataTransformModel* dtModelInstance;

};

} // namespace Dakota

#endif
