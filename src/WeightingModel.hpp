/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#ifndef DAKOTA_WEIGHTING_MODEL_H
#define DAKOTA_WEIGHTING_MODEL_H

#include "RecastModel.hpp"

namespace Dakota {

/// Weighting specialization of RecastModel

/** Specialization of a RecastModel that manages Response weighting
    (could be implemented as special case of ScalingModel, but kept
    separate for simplicity for now). This class provides a simple
    constructor that forwards to the more complicated RecastModel
    API */
class WeightingModel: public RecastModel
{
public:
  
  //
  //- Heading: Constructor and destructor
  //

  /// standard constructor
  WeightingModel(Model& sub_model);

  /// destructor
  ~WeightingModel();


protected:

  //
  //- Heading: Virtual function redefinitions
  //

  void assign_instance();

  void init_metadata() override { /* no-op to leave metadata intact */}

  //
  //- Heading: Member functions
  //

  static void primary_resp_weighter(const Variables& sub_model_vars,
				    const Variables& recast_vars,
				    const Response& sub_model_response,
				    Response& weighted_response);

  static void primary_resp_unweighter(const Variables& recast_vars,
				      const Variables& sub_model_vars,
				      const Response& weighted_resp,
				      Response& unweighted_resp) = delete;

private:

  /// static pointer to this class for use in static callbacks
  static WeightingModel* weightModelInstance;
};


inline void WeightingModel::assign_instance()
{ weightModelInstance = this; }

}  // namespace Dakota

#endif
