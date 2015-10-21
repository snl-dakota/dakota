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

// Don't want to output message during recast retrieve... (or do we?)


/** This constructor computes various indices and mappings, then
    updates the properties of the RecastModel */
DataTransformModel::
DataTransformModel(const Model& sub_model, const ExperimentData& exp_data):
  // BMA TODO: should the BitArrays be empty or same as submodel?
  // recast_secondary_offset is the index to the equality constraints within 
  // the secondary responses
  RecastModel(sub_model, SizetArray(), BitArray(), BitArray(), 
	      exp_data.num_total_exppoints(), 
	      sub_model.num_functions() - sub_model.num_primary_fns(),
	      sub_model.num_nonlinear_ineq_constraints(),
	      response_order(sub_model)), 
  expData(exp_data)
{
  dtModelInstance = this;

  size_t num_submodel_primary = sub_model.num_primary_fns();
  // the RecastModel will have one residual per experiment datum
  size_t num_recast_primary = expData.num_total_exppoints(),
    num_secondary = sub_model.num_functions() - sub_model.num_primary_fns(),
    num_recast_fns = num_recast_primary + num_secondary;

  // ---
  // Variables mapping (one-to-one)
  // ---
  Sizet2DArray vars_map_indices(sub_model.tv());
  // For now, we assume the mapping is for all active variables
  size_t total_active_vars = 
    sub_model.cv() + sub_model.div() + sub_model.dsv() + sub_model.drv();
  // TODO: all variables... active or all?
  for (size_t i=0; i<total_active_vars; ++i) {
    vars_map_indices[i].resize(1);
    vars_map_indices[i][0] = i;
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
  void (*variables_map) (const Variables&, Variables&) = NULL;
  void (*set_map)  (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
  void (*primary_resp_map) (const Variables&, const Variables&, const Response&, 
			    Response&) = primary_resp_differencer;
  void (*secondary_resp_map) (const Variables&, const Variables&,
			      const Response&, Response&) = NULL;
  RecastModel::
    initialize(vars_map_indices, nonlinear_vars_mapping, variables_map, set_map,
	       primary_resp_map_indices, secondary_resp_map_indices, 
	       nonlinear_resp_mapping, primary_resp_map, secondary_resp_map);


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
}


DataTransformModel::~DataTransformModel()
{ /* empty dtor */}


short DataTransformModel::response_order(const Model& sub_model)
{
  const Response& curr_resp = sub_model.current_response();

  short recast_resp_order = 1; // recast resp order to be same as original resp
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


void DataTransformModel::
primary_resp_differencer(const Variables& raw_vars,     
			 const Variables& residual_vars,
			 const Response& raw_response, 
			 Response& residual_response)
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

  dtModelInstance->data_difference_core(raw_response, residual_response);

  if (dtModelInstance->outputLevel >= VERBOSE_OUTPUT && 
      dtModelInstance->subordinate_model().num_primary_fns() > 0) {
    Cout << "Calibration data transformation:\n";
    write_data(Cout, residual_response.function_values(),
	       residual_response.function_labels());
    Cout << std::endl;
  }
  if (dtModelInstance->outputLevel >= DEBUG_OUTPUT && 
      dtModelInstance->subordinate_model().num_primary_fns() > 0) {
    size_t num_total_calib_terms = 
      dtModelInstance->expData.num_total_exppoints();
    const ShortArray& asv = residual_response.active_set_request_vector();
    for (size_t i=0; i < num_total_calib_terms; ++i) {
      if (asv[i] & 1) 
        Cout << " residual_response function " << i << ' ' 
	     << residual_response.function_value(i) << '\n';
      if (asv[i] & 2) 
        Cout << " residual_response gradient " << i << ' ' 
	     << residual_response.function_gradient_view(i) << '\n';
      if (asv[i] & 4) 
        Cout << " residual_response hessian " << i << ' ' 
	     << residual_response.function_hessian(i) << '\n';
    }
  }

}

/** quiet version of function used in recovery of function values */
void DataTransformModel::
data_difference_core(const Response& raw_response, Response& residual_response)
{
  ShortArray total_asv;

  bool apply_cov = expData.variance_active();
  // can't apply matrix-valued errors due to possibly incomplete
  // dataset when active set vector is in use (missing residuals)
  bool matrix_cov_active = expData.variance_type_active(MATRIX_SIGMA);

  bool interrogate_field_data = 
    ( matrix_cov_active || expData.interpolate_flag() );
  total_asv = expData.determine_active_request(residual_response, 
					       interrogate_field_data);
  expData.form_residuals(raw_response, total_asv, residual_response);
  if (apply_cov) 
    expData.scale_residuals(residual_response, total_asv);
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
