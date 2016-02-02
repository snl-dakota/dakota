/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       DataTransformModel
//- Description: Implementation code for the DataTransformModel class
//- Owner:       Brian Adams
//- Checked by:

#include "DataTransformModel.hpp"
#include "ExperimentData.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

/// initialization of static needed by RecastModel
DataTransformModel* DataTransformModel::dtModelInstance(NULL);

// BMA TODO:
// * Construct with the Iterator's verbosity or the Model's?  Models
//   default to same as their Iterator...
// * Verify that the default variables, active set, and secondary
//   response mapping suffice Need test with data and constraints
//  * Don't want to output message during recast retrieve... (or do we?)

/** This constructor computes various indices and mappings, then
    updates the properties of the RecastModel.  Hyper-parameters are
    assumed to trail the active continuous variables when presented to
    this RecastModel */
DataTransformModel::
DataTransformModel(const Model& sub_model, const ExperimentData& exp_data,
                   size_t num_hyper, unsigned short mult_mode, 
                   short recast_resp_deriv_order):
  // BMA TODO: should the BitArrays be empty or same as submodel?
  // recast_secondary_offset is the index to the equality constraints within 
  // the secondary responses
  RecastModel(sub_model, variables_expand(sub_model, num_hyper),
	      BitArray(), BitArray(), 
	      exp_data.num_total_exppoints(), 
	      sub_model.num_functions() - sub_model.num_primary_fns(),
	      sub_model.num_nonlinear_ineq_constraints(),
              response_order(sub_model, recast_resp_deriv_order)), 
  expData(exp_data), numHyperparams(num_hyper), obsErrorMultiplierMode(mult_mode)
{
  dtModelInstance = this;

  size_t num_submodel_primary = sub_model.num_primary_fns();
  // the RecastModel will have one residual per experiment datum
  size_t num_recast_primary = expData.num_total_exppoints(),
    num_secondary = sub_model.num_functions() - sub_model.num_primary_fns(),
    num_recast_fns = num_recast_primary + num_secondary;

  // ---
  // Variables mapping (one-to-one), truncating trailing hyper-parameters
  // ---

  // BMA TODO: what variables subset and ordering convention do we
  // want to assume here? Do we need to iterate sub-types, or just the
  // containers...Assuming active for now.

  // For now, we assume that any hyper-parameters are appended to the
  // active continuous variables, and that active discrete int,
  // string, real follow in both the recast and sub-model
  size_t submodel_cv = sub_model.cv();
  size_t submodel_dv = sub_model.div() + sub_model.dsv() + sub_model.drv();
  Sizet2DArray vars_map_indices(submodel_cv + submodel_dv);
  for (size_t i=0; i<submodel_cv; ++i) {
    vars_map_indices[i].resize(1);
    vars_map_indices[i][0] = i;
  }
  // skip the trailing continuous hyper-parameters
  size_t recast_dv_start = submodel_cv + numHyperparams;
  for (size_t i=0; i<submodel_dv; ++i) {
    vars_map_indices[submodel_cv + i].resize(1);
    vars_map_indices[submodel_cv + i][0] = recast_dv_start + i;
  }
  bool nonlinear_vars_mapping = false;

  // mappings from the submodel Response to residual Response
  BoolDequeArray nonlinear_resp_mapping(num_recast_fns);

  // ---
  // Primary mapping
  // ---
  Sizet2DArray primary_resp_map_indices(num_recast_primary);
  const Response& curr_resp = sub_model.current_response();
  const SharedResponseData& srd = curr_resp.shared_data();
  gen_primary_resp_map(srd, primary_resp_map_indices, nonlinear_resp_mapping);

  // ---
  // Secondary mapping (one-to-one)
  // ---
  Sizet2DArray secondary_resp_map_indices(num_secondary);
  for (size_t i=0; i<num_secondary; i++) {
    secondary_resp_map_indices[i].resize(1);
    // the recast constraints just depend on the simulation
    // constraints, which start at num_submodel_primary
    secondary_resp_map_indices[i][0] = num_submodel_primary + i;
    nonlinear_resp_mapping[num_recast_primary + i].resize(1);
    nonlinear_resp_mapping[num_recast_primary + i][0] = false;
  }

  // callbacks for RecastModel transformations: default maps for all but primary
  void (*variables_map) (const Variables&, Variables&) = 
    (numHyperparams > 0) ? vars_mapping : NULL;
  void (*set_map)  (const Variables&, const ActiveSet&, ActiveSet&) = 
    (numHyperparams > 0) ? set_mapping : NULL;
  void (*primary_resp_map) (const Variables&, const Variables&, const Response&, 
			    Response&) = primary_resp_differencer;
  void (*secondary_resp_map) (const Variables&, const Variables&,
			      const Response&, Response&) = NULL;
  RecastModel::
    init_maps(vars_map_indices, nonlinear_vars_mapping, variables_map, set_map,
	      primary_resp_map_indices, secondary_resp_map_indices, 
	      nonlinear_resp_mapping, primary_resp_map, secondary_resp_map);


  // ---
  // Expand submodel Variables data to account for hyper-parameters
  // ---

  expand_var_labels(sub_model);


  // ---
  // Expand any submodel Response data to the expanded residual size
  // ---

  // The following expansions are conservative.  Could be skipped when
  // only scalar data present and no replicates.

  // Preserve weights through data transformations
  expand_array(srd, sub_model.primary_response_fn_weights(), num_recast_primary,
               primaryRespFnWts);
  // Preserve sense through data transformations
  expand_array(srd, sub_model.primary_response_fn_sense(), num_recast_primary,
               primaryRespFnSense);

  // CV scales don't change in this recasting; base RecastModel captures them

  // Adjust each scaling type to right size, leaving as length 1 if needed
  expand_scales_array(srd, sub_model.scaling_options().priScaleTypes, 
                      sub_model.scaling_options().priScaleTypes.size(),
                      num_recast_primary, scalingOpts.priScaleTypes);
  expand_scales_array(srd, sub_model.scaling_options().priScales, 
                      sub_model.scaling_options().priScales.length(),
                      num_recast_primary, scalingOpts.priScales);
  expand_scales_array(srd, sub_model.scaling_options().nlnIneqScaleTypes, 
                      sub_model.scaling_options().nlnIneqScaleTypes.size(),
                      num_recast_primary, scalingOpts.nlnIneqScaleTypes);
  expand_scales_array(srd, sub_model.scaling_options().nlnIneqScales, 
                      sub_model.scaling_options().nlnIneqScales.length(),
                      num_recast_primary, scalingOpts.nlnIneqScales);
  expand_scales_array(srd, sub_model.scaling_options().nlnEqScaleTypes, 
                      sub_model.scaling_options().nlnEqScaleTypes.size(),
                      num_recast_primary, scalingOpts.nlnEqScaleTypes);
  expand_scales_array(srd, sub_model.scaling_options().nlnEqScales, 
                      sub_model.scaling_options().nlnEqScales.length(),
                      num_recast_primary, scalingOpts.nlnEqScales);
  expand_scales_array(srd, sub_model.scaling_options().linIneqScaleTypes, 
                      sub_model.scaling_options().linIneqScaleTypes.size(),
                      num_recast_primary, scalingOpts.linIneqScaleTypes);
  expand_scales_array(srd, sub_model.scaling_options().linIneqScales, 
                      sub_model.scaling_options().linIneqScales.length(),
                      num_recast_primary, scalingOpts.linIneqScales);
  expand_scales_array(srd, sub_model.scaling_options().linEqScaleTypes, 
                      sub_model.scaling_options().linEqScaleTypes.size(),
                      num_recast_primary, scalingOpts.linEqScaleTypes);
  expand_scales_array(srd, sub_model.scaling_options().linEqScales, 
                      sub_model.scaling_options().linEqScales.length(),
                      num_recast_primary, scalingOpts.linEqScales);

  // For this derivation of RecastModel, all resizing can occur at construct
  // time --> Variables/Response are up to date for estimate_message_lengths()
  // within Model::init_communicators().
}


DataTransformModel::~DataTransformModel()
{ /* empty dtor */}


void DataTransformModel::
data_transform_response(const Variables& sub_model_vars, 
                        const Response& sub_model_resp,
                        Response& residual_resp) 
{
  // A DataTransform doesn't map variables, but we map them here to
  // avoid introducing a latent bug
  Variables recast_vars(current_variables().copy());
  inverse_transform_variables(sub_model_vars, recast_vars);
  transform_response(recast_vars, sub_model_vars, sub_model_resp, residual_resp);
}


/** Incorporate the hyper parameters into Variables, assuming they are at the 
    end of the active continuous variables.  For example, append them to 
    continuous design or continuous aleatory uncertain. */ 
SizetArray DataTransformModel::
variables_expand(const Model& sub_model, size_t num_hyper)
{
  SizetArray vc_totals;  // default is no size change
  if (num_hyper) {
    const SharedVariablesData& svd = sub_model.current_variables().shared_data();
    vc_totals = svd.components_totals();

    short active_view = sub_model.current_variables().view().first;
    switch (active_view) {
      
    case MIXED_DESIGN: case RELAXED_DESIGN:
      // append to end of continuous design
      vc_totals[TOTAL_CDV] += num_hyper; 
      break;

    case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
      // append to end of continuous aleatory
      vc_totals[TOTAL_CAUV] += num_hyper;
      break;

    case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: 
    case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
      // append to end of continuous epistemic (note there may not actually be 
      // any epistemic variables in the *_UNCERTAIN cases)
      vc_totals[TOTAL_CEUV] += num_hyper;
      break;

    case MIXED_ALL: case RELAXED_ALL: case MIXED_STATE: case RELAXED_STATE:
      // append to end of continuous state
      vc_totals[TOTAL_CSV] += num_hyper;
      break;

    default:
      Cerr << "\nError: invalid active variables view " << active_view 
	   << " in DataTransformModel.\n";
      abort_handler(-1);
      break;

    }

  }
  return vc_totals;
}


short DataTransformModel::
response_order(const Model& sub_model, short recast_resp_order)
{
  const Response& curr_resp = sub_model.current_response();

  // recast resp order at least as much as original response
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  return recast_resp_order;
} 

void DataTransformModel::
gen_primary_resp_map(const SharedResponseData& srd,
		     Sizet2DArray& primary_resp_map_indices,
		     BoolDequeArray& nonlinear_resp_map) const
{
  size_t num_scalar = srd.num_scalar_responses(),
    num_field_groups = srd.num_field_response_groups();
  const IntVector& sim_field_lens = srd.field_lengths();
  size_t num_experiments = expData.num_experiments(), calib_term_ind = 0; 

  for (size_t exp_ind = 0; exp_ind < num_experiments; ++exp_ind) {
    // field lengths can be different per experiment
    const IntVector& exp_field_lens = expData.field_lengths(exp_ind);
    for (size_t scalar_ind = 0; scalar_ind < num_scalar; ++scalar_ind) {
      // simulation scalars inform calibration terms 1 to 1 
      // (no correlation or interpolation allowed)
      primary_resp_map_indices[calib_term_ind].resize(1);
      primary_resp_map_indices[calib_term_ind][0] = scalar_ind; //=calib_term_ind
      nonlinear_resp_map[calib_term_ind].resize(1);
      nonlinear_resp_map[calib_term_ind][0] = false;
      ++calib_term_ind;
    }
    // base index for simulation in current field group
    size_t sim_ind_offset = num_scalar; 
    for (size_t fg_ind = 0; fg_ind < num_field_groups; ++fg_ind) {
      // each field calibration term depends on all simulation field
      // entries for this field, due to correlation or interpolation
      // if no matrix correlation, no interpolation, could skip
      for (size_t z=0; z<exp_field_lens[fg_ind]; ++z) {
	primary_resp_map_indices[calib_term_ind].resize(sim_field_lens[fg_ind]);
	nonlinear_resp_map[calib_term_ind].resize(sim_field_lens[fg_ind]);
	// this residual depends on all other simulation data for this field
	for (size_t sim_ind = 0; sim_ind<sim_field_lens[fg_ind]; ++sim_ind) {
	  primary_resp_map_indices[calib_term_ind][sim_ind] = 
	    sim_ind_offset + sim_ind;
	  nonlinear_resp_map[calib_term_ind][sim_ind] = false;
	}
	++calib_term_ind;
      }
      sim_ind_offset += sim_field_lens[fg_ind];
    }
  }
}


void DataTransformModel::vars_mapping(const Variables& recast_vars, 
				      Variables& submodel_vars)
{
  // Forward the calibration parameters, omitting the hyper-parameters, which 
  // are assumed to trail the active continuous variables.
  RealVector sm_cv = submodel_vars.continuous_variables_view();
  copy_data_partial(recast_vars.continuous_variables(), 0, 
		    (int)submodel_vars.cv(), sm_cv);
}


/** RecastModel sets up a default set mapping before calling this
    update, so focus on updating the derivative variables vector */
void DataTransformModel::set_mapping(const Variables& recast_vars,
 				     const ActiveSet& recast_set,
 				     ActiveSet& sub_model_set)
{
  // This transformation should forward DVV requests for derivatives
  // w.r.t. calibration parameters and discard requests for
  // derivatives w.r.t. hyper-parameters.
  
  // The sub-model should be working with variable IDs from 1 to
  // number of active continuous variables
  SizetArray sub_model_dvv;
  const SizetArray& recast_dvv = recast_set.derivative_vector();
  size_t max_sm_id = dtModelInstance->subordinate_model().cv();
  for (size_t i=0; i<recast_dvv.size(); ++i)
    if (1 <= recast_dvv[i] && recast_dvv[i] <= max_sm_id)
      sub_model_dvv.push_back(recast_dvv[i]);
  sub_model_set.derivative_vector(sub_model_dvv);

  // When calibrating hyper-parameters in a MAP solve, requests for
  // gradients and Hessians require lower-order data to be present.  This
  // could be relaxed depending on which derivative vars are requested.
  if (dtModelInstance->numHyperparams > 0) {
    ShortArray sub_model_asv = sub_model_set.request_vector();
    for (size_t i=0; i<sub_model_asv.size(); ++i) {
      if (sub_model_asv[i] & 4)
	sub_model_asv[i] |= 2;
      if (sub_model_asv[i] & 2)
	sub_model_asv[i] |= 1;
    }
    sub_model_set.request_vector(sub_model_asv);
  }
}


void DataTransformModel::
primary_resp_differencer(const Variables& submodel_vars,     
			 const Variables& recast_vars,
			 const Response& submodel_response, 
			 Response& recast_response)
{
  // BMA REVIEW: data differencing doesn't affect gradients and
  // Hessians, as long as they use the updated residual in their
  // computation.  They probably don't!

  if (dtModelInstance->outputLevel >= VERBOSE_OUTPUT) {
    Cout << "\n-----------------------------------------------------------";
    Cout << "\nPost-processing Function Evaluation: Data Transformation";
    Cout << "\n-----------------------------------------------------------" 
	 << std::endl;
  }

  dtModelInstance->data_difference_core(submodel_vars, recast_vars, 
					submodel_response, recast_response);

  if (dtModelInstance->outputLevel >= VERBOSE_OUTPUT && 
      dtModelInstance->subordinate_model().num_primary_fns() > 0) {
    Cout << "Calibration data transformation; residuals:\n";
    write_data(Cout, recast_response.function_values(),
	       recast_response.function_labels());
    Cout << std::endl;
  }
  if (dtModelInstance->outputLevel >= DEBUG_OUTPUT && 
      dtModelInstance->subordinate_model().num_primary_fns() > 0) {
    Cout << "Calibration data transformation; full response:\n"
	 << recast_response << std::endl;
  }

}

// BMA TODO: decide how much output to have and where...

/** quiet version of function used in recovery of function values */
void DataTransformModel::
data_difference_core(const Variables& submodel_vars, 
                     const Variables& recast_vars,
                     const Response& submodel_response, 
                     Response& recast_response)
{
  // form residuals (and gradients/Hessians) from the simulation
  // response this call has to be careful not to resize gradients and
  // Hessians in a way that tramples hyper-parameters: only update
  // submodel cv entries, leaving objects sized
  expData.form_residuals(submodel_response, recast_response);

  // scale by (error covariance)^{-1/2}
  if (expData.variance_active())
    expData.scale_residuals(recast_response);

  // TODO: may need to scale by hyperparameters in Covariance as well
  if (obsErrorMultiplierMode > CALIBRATE_NONE) {  
    // r <- r ./ mult, where mult might be per-block
    // scale by mult, whether 1, per-experiment, per-response, or both
    // for now, the multiplier calibration mode is a method-related
    // parameter; could instead have a by-index interface here...

    // For now, assume only continuous variables, with hyper at end
    
    // extract hyperparams; now both raw and sub vars have size total params
    // hyper are the last numHyperParams
    size_t num_calib_params = submodel_vars.cv();
    RealVector hyper_params;
    hyper_params.sizeUninitialized(numHyperparams);
    copy_data_partial(recast_vars.continuous_variables(), num_calib_params, 
		      numHyperparams, hyper_params);
    
    // Apply hyperparameter multipliers to all fn, grad, Hess,
    // including derivative contributions from hyperparmeters
    // BMA TODO: Model after the above scale_residuals, operating block-wise 
    // and accounting for ASV
    expData.
      scale_residuals(hyper_params, obsErrorMultiplierMode, num_calib_params,
		      recast_response);
  }
}


void DataTransformModel::expand_var_labels(const Model& sub_model)
{
  // currentVariables should be sized by the RecastModel initialization
  size_t num_calib = sub_model.cv();

  StringMultiArrayConstView sm_labels(sub_model.continuous_variable_labels());
  for (size_t i=0; i<num_calib; ++i)
    currentVariables.continuous_variable_label(sm_labels[i], i);
  
  StringArray hyper_labels = expData.hyperparam_labels(obsErrorMultiplierMode);
  for (size_t i=0; i<numHyperparams; ++i)
    currentVariables.continuous_variable_label(hyper_labels[i], num_calib + i);
}


/** Passing the inbound array size so we can use one function for
    Teuchos and std containers (size vs. length) */
template<typename T>
void DataTransformModel::
expand_scales_array(const SharedResponseData& srd, const T& submodel_array,
                    size_t submodel_size, size_t recast_size, 
                    T& recast_array) const 
{
  if (submodel_size == 1)
    // this copy may not be needed, depends on ctor behavior
    recast_array = submodel_array;
  else if (submodel_size > 1) {
    expand_array(srd, submodel_array, recast_size, recast_array);
  }
  // else leave recast_array empty
}


template<typename T>
void DataTransformModel::
expand_array(const SharedResponseData& srd, const T& submodel_array, 
             size_t recast_size, T& recast_array) const 
{
  if (submodel_array.empty())
    return;  // leave recast_array empty

  recast_array.resize(recast_size);

  size_t num_scalar = srd.num_scalar_responses();
  size_t num_field_groups = srd.num_field_response_groups();
  size_t calib_term_ind = 0;
  for (size_t exp_ind=0; exp_ind<expData.num_experiments(); ++exp_ind) {
    const IntVector& exp_field_lens = expData.field_lengths(exp_ind);
    for (size_t sc_ind = 0; sc_ind < num_scalar; ++sc_ind)
      recast_array[calib_term_ind++] = submodel_array[sc_ind];
    for (size_t fg_ind = 0; fg_ind < num_field_groups; ++fg_ind)
      recast_array[calib_term_ind++] = submodel_array[num_scalar + fg_ind];
  }
}


}  // namespace Dakota
