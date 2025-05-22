/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "WeightingModel.hpp"
#include "DataMethod.hpp"

namespace Dakota {

/// initialization of static needed by RecastModel
WeightingModel* WeightingModel::weightModelInstance(NULL);


WeightingModel::WeightingModel(std::shared_ptr<Model> sub_model
			       // TODO: weight_transformer = sqrt
			       ):
  // BMA TODO: should the BitArrays be empty or same as submodel?
  RecastModel(sub_model, 
	      SizetArray(), // no change in vars size
	      BitArray(),   // default discrete int relaxation
	      BitArray(),   // default discrete real relaxation
	      sub_model->current_variables().view(),
	      sub_model->num_primary_fns(), sub_model->num_secondary_fns(),
	      ModelUtils::num_nonlinear_ineq_constraints(*sub_model),
	      response_order(*sub_model) )
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Info: Constructing WeightingModel" << std::endl;

  // This model maps things through 1--1 in t erms of sizes
  // TODO: reasonable default code in Recast for the following

  // TODO: this could generalize to arbitrary variable types...
  // TODO: is this sufficiently general for Bayesian hyperparameters?
  size_t num_cv = ModelUtils::cv(*sub_model);
  Sizet2DArray vars_map_indices(num_cv, SizetArray(1));
  for (size_t i=0; i<num_cv; ++i)
    vars_map_indices[i][0] = i;
  bool nonlinear_vars_mapping = false;

  size_t num_pri_fns = sub_model->num_primary_fns();
  Sizet2DArray primary_resp_map_indices(num_pri_fns, SizetArray(1));
  for (size_t i=0; i<num_pri_fns; ++i)
    primary_resp_map_indices[i][0] = i;

  size_t num_sec_fns = sub_model->num_secondary_fns();
  Sizet2DArray secondary_resp_map_indices(num_sec_fns, SizetArray(1));
  for (size_t i=0; i<num_sec_fns; ++i)
    secondary_resp_map_indices[i][0] = num_pri_fns + i;

  BoolDequeArray nonlinear_resp_mapping(num_pri_fns + num_sec_fns, 
					BoolDeque(1, false));

  init_maps(vars_map_indices, nonlinear_vars_mapping,
	    NULL,  // no vars mapping
	    NULL,  // no set mapping
	    primary_resp_map_indices, secondary_resp_map_indices, 
	    nonlinear_resp_mapping, primary_resp_weighter,
	    NULL // no secondary mapping
	    );

  // This transformation consumes (uses) weights, so the resulting
  // RecastModel doesn't need them any longer; however don't want to
  // recurse and wipe out in sub-models.  Be explicit in case later
  // update_from_model() is used instead.
  bool recurse_flag = false;
  primary_response_fn_weights(RealVector(), recurse_flag);

  // Preserve optimization min/max sense through the weighting transformation
  const BoolDeque& submodel_sense = sub_model->primary_response_fn_sense();
  primary_response_fn_sense(submodel_sense);

  // TODO: handle relax DI, DR?
}


WeightingModel::~WeightingModel()
{  /* empty dtor */  }


void WeightingModel::
primary_resp_weighter(const Variables& sub_model_vars,
		      const Variables& recast_vars,
		      const Response& sub_model_response,
		      Response& weighted_response)
{

  if (weightModelInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n--------------------------------------------------------";
    Cout << "\nPost-processing Function Evaluation: Weighting Residuals";
    Cout << "\n--------------------------------------------------------" 
	 << std::endl;
  }

  // weights are available in the sub-model, but not in *this
  const RealVector& lsq_weights = weightModelInstance->
    subModel->primary_response_fn_weights();
  RealVector wt_fn_vals = weighted_response.function_values_view();
  const ShortArray& asv = weighted_response.active_set_request_vector();
  const RealVector& sm_fn_vals = sub_model_response.function_values();
  // only transform primary functions (same # on submodel or this)
  size_t num_pri_fns = weightModelInstance->num_primary_fns();
  if (lsq_weights.length() != num_pri_fns) {
    Cerr << "Error: mismatch in length of weighting vector (" << num_pri_fns
	 << " expected and " << lsq_weights.length()<< " provided)."<<std::endl;
    abort_handler(MODEL_ERROR);
  }

  for (size_t i=0; i<num_pri_fns; ++i) {

    // For LSQ: \Sum_i w_i T^2_i => residual scaling as \Sum_i [sqrt(w_i) T_i]^2
    Real wt_i = std::sqrt(lsq_weights[i]);

    // functions
    if (asv[i] & 1)
      wt_fn_vals[i] = wt_i * sm_fn_vals[i];

    // gradients
    if (asv[i] & 2) {
      RealVector wt_fn_grad = weighted_response.function_gradient_view(i);
      wt_fn_grad.assign(sub_model_response.function_gradient_view(i));
      wt_fn_grad *= wt_i;
    }

    // hessians
    if (asv[i] & 4) {
      RealSymMatrix wt_fn_hess = weighted_response.function_hessian_view(i);
      wt_fn_hess.assign(sub_model_response.function_hessian(i));
      wt_fn_hess *= wt_i;
    }

  }

  weighted_response.metadata(sub_model_response.metadata());

  if (weightModelInstance->outputLevel > NORMAL_OUTPUT)
    Cout << "Least squares weight-transformed response:\n" << weighted_response 
	 << std::endl;
}


}  // namespace Dakota
