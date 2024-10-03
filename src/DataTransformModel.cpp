/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2024
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

#include "DataTransformModel.hpp"
#include "ExperimentData.hpp"
#include "DakotaMinimizer.hpp"
#include "PRPMultiIndex.hpp"
#include "ResultsManager.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

extern PRPCache data_pairs; // global container

/// initialization of static needed by RecastModel
DataTransformModel* DataTransformModel::dtModelInstance(NULL);

// BMA TODO:
// * Construct with the Iterator's verbosity or the Model's?  Models
//   default to same as their Iterator...
// * Verify that the default variables, active set, and secondary
//   response mapping suffice Need test with data and constraints
//  * Don't want to output message during recast retrieve... (or do we?)

/** This constructor computes various indices and mappings, then updates the
    properties of the RecastModel.  Hyper-parameters are assumed to trail the
    active continuous variables when presented to this RecastModel. */
DataTransformModel::
DataTransformModel(const Model& sub_model, ExperimentData& exp_data,
		   const ShortShortPair& recast_vars_view,
                   size_t num_hyper, unsigned short mult_mode, 
                   short recast_resp_deriv_order):
  // BMA TODO: should the BitArrays be empty or same as submodel?
  // recast_secondary_offset is the index to the equality constraints within 
  // the secondary responses
  RecastModel(sub_model, variables_expand(sub_model, num_hyper),
	      BitArray(), BitArray(), recast_vars_view,
	      exp_data.num_total_exppoints(), sub_model.num_secondary_fns(),
	      sub_model.num_nonlinear_ineq_constraints(),
              response_order(sub_model, recast_resp_deriv_order)), 
  expData(exp_data), numHyperparams(num_hyper),
  obsErrorMultiplierMode(mult_mode)
{
  modelId = RecastModel::recast_model_id(root_model_id(), "DATA_TRANSFORM");
  // register state variables as inactive vars if config vars are present
  // BMA TODO: correctly manage the view if relaxed, also review recursion

  // BMA NOTE: This will change the inactive view of any Variables object
  // sharing the same SharedVariables data as the subModel's Variables
  size_t num_config_vars = expData.num_config_vars();
  short  active_sm_view  = subModel.current_variables().view().first;
  if ( num_config_vars &&
       ( active_sm_view != RELAXED_ALL && active_sm_view != MIXED_ALL ) ) {
    subModel.inactive_view(MIXED_STATE);
    int num_state_vars =
      subModel.current_variables().icv() + subModel.current_variables().idiv() + subModel.current_variables().idsv() + subModel.current_variables().idrv();
    if (num_state_vars != num_config_vars) {
      Cerr << "\nError: (DataTransformModel) Number of state "
	   << "variables = " << num_state_vars << " must match\n       number "
	   << "of configuration variables = " << num_config_vars << "\n";
      abort_handler(MODEL_ERROR);
    }
  }

  size_t num_submodel_primary = sub_model.num_primary_fns();
  // the RecastModel will have one residual per experiment datum
  size_t num_recast_primary = expData.num_total_exppoints(),
    num_secondary = sub_model.num_secondary_fns(),
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
  size_t submodel_cv = sub_model.current_variables().cv();
  size_t submodel_dv = sub_model.current_variables().div() + sub_model.current_variables().dsv() + sub_model.current_variables().drv();
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

  // transform configuration variables in expData from the original
  // "user space" to the space used by this Model
  if (manage_data_recastings()) {
    VariablesArray& exp_vars_array = expData.configuration_variables();
    size_t i, num_exp_vars = exp_vars_array.size();
    for (i=0; i<num_exp_vars; ++i) {
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "User-space configuration vars:\n" << exp_vars_array[i];
      user_space_to_iterator_space(exp_vars_array[i]);
      if (outputLevel >= DEBUG_OUTPUT)
	Cout << "Iterator-space configuration vars:\n" << exp_vars_array[i];
    }
  }

  // ---
  // Expand currentVariables values/labels to account for hyper-parameters
  // ---
  init_continuous_vars();

  // TODO: mvDist likely needs size change for hyper-parameters. Its
  // bounds need updating too; above variable updates bypass mvDist
  // as sets on constraints object only instead of Model's
  // setters...
  //mvDist = subModel.multivariate_distribution(); // shared rep
  init_distribution(true);
  // copy_values is false in RecastModel::init_sizes(), which is correct in
  // general since the variables config changes.  In the derived class, we can
  // revisit this with special knowledge that subModel mvDist can be adapted.

  // ---
  // Expand any submodel Response data to the expanded residual size
  // ---

  // Need to update when multiple experiments and/or interpolation
  update_expanded_response(subModel);


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


void DataTransformModel::data_resize()
{
  // Actions from ctor chain to check:
  // RecastModel ctor
  // nonlinear_resp_mapping
  // update primary resp mapping
  // update secondary resp mapping, nonlinear resp mapping

  // RecastModel init_maps

  // expand arrays
  if (numHyperparams > 0 || obsErrorMultiplierMode > CALIBRATE_NONE) {
    // TODO: We could support update without size change, or even with
    // size change in the case of only one multiplier.  Or later could
    // allow updates including the whole parameter domain change.
    Cerr << "\nError (DataTransformModel): data updates not supported when "
	 << "calibrating\nhyper-parameters.";
    abort_handler(MODEL_ERROR);
  }

  // there is no change in variables or derivatives for now
  reshape_response(expData.num_total_exppoints(),
		   subModel.num_secondary_fns());

}


void DataTransformModel::init_metadata()
{
  // only clear if there are multiple configs, else leave intact
  if (expData.configuration_variables().size() > 1)
    currentResponse.reshape_metadata(0);
}


void DataTransformModel::update_from_subordinate_model(size_t depth)
{
  // data flows from the bottom-up, so recurse first
  if (depth == SZ_MAX)
    subModel.update_from_subordinate_model(depth); // retain special value (inf)
  else if (depth)
    subModel.update_from_subordinate_model(depth - 1); // decrement
  //else depth exhausted --> update this level only

  if (numHyperparams > 0) {

    update_cv_skip_hyperparams(subModel);

    // for discrete, update all
    update_all_discrete_variables(subModel);
    //update_discrete_variable_values(subModel);
    //update_discrete_variable_bounds(subModel);
    //update_discrete_variable_labels(subModel);

    // TODO: mvDist likely needs size change for hyper-parameters. Its
    // bounds need updating too; above variable updates bypass mvDist
    // as sets on constraints object only instead of Model's
    // setters...
    mvDist = subModel.multivariate_distribution(); // shared rep

    // Add column of zeroes corresponding to the hyper-parameters
    expand_linear_constraints(subModel);

  }
  else { // base class implementation should suffice for variables
    bool update_active_complement = update_variables_from_model(subModel);
    if (update_active_complement)
      update_variables_active_complement_from_model(subModel);
  }

  // Need to update when multiple experiments and/or interpolation
  update_expanded_response(subModel);
}


void DataTransformModel::update_cv_skip_hyperparams(const Model& model)
{
  // update active cv from submodel, leave hyper params as-is,
  // update active complement
  const Variables& sm_vars = model.current_variables();
  size_t i,  // indexes the model's all continuous variables
    cv_begin = sm_vars.cv_start(),
    num_cv  = sm_vars.cv(), // omits any hyper-parameters
    cv_end = cv_begin + num_cv,
    num_acv = sm_vars.acv();
  const RealVector& acv = model.current_variables().all_continuous_variables();
  const RealVector& acv_l_bnds = model.all_continuous_lower_bounds();
  const RealVector& acv_u_bnds = model.all_continuous_upper_bounds();
  StringMultiArrayConstView acv_labels
    = model.current_variables().all_continuous_variable_labels();

  // active complement [0, cv_begin), followed by active [cv_begin, cv_end)
  for (i=0; i<cv_end; ++i) {
    currentVariables.all_continuous_variable(acv[i], i);
    userDefinedConstraints.all_continuous_lower_bound(acv_l_bnds[i], i);
    userDefinedConstraints.all_continuous_upper_bound(acv_u_bnds[i], i);
    currentVariables.all_continuous_variable_label(acv_labels[i], i);
  }
  // skip hyper-parameters on *this
  // active complement [cv_end, num_acv)
  for (i=cv_end; i<num_acv; ++i) {
    currentVariables.all_continuous_variable(acv[i], numHyperparams + i);
    userDefinedConstraints.all_continuous_lower_bound(acv_l_bnds[i],
						      numHyperparams + i);
    userDefinedConstraints.all_continuous_upper_bound(acv_u_bnds[i],
						      numHyperparams + i);
    currentVariables.all_continuous_variable_label(acv_labels[i],
						   numHyperparams + i);
  }
}

void DataTransformModel::expand_linear_constraints(const Model& model)
{
  if (model.num_linear_ineq_constraints()) {

    const RealMatrix& sm_coeffs = model.linear_ineq_constraint_coeffs();
    RealMatrix dt_coeffs(sm_coeffs.numRows(),
			 sm_coeffs.numCols() + numHyperparams);
    RealMatrix sm_subset(Teuchos::View, dt_coeffs, sm_coeffs.numRows(),
			 sm_coeffs.numCols(), 0, 0);
    sm_subset.assign(sm_coeffs);
    userDefinedConstraints.linear_ineq_constraint_coeffs(dt_coeffs);

    userDefinedConstraints.linear_ineq_constraint_lower_bounds
      (model.linear_ineq_constraint_lower_bounds());
    userDefinedConstraints.linear_ineq_constraint_upper_bounds
      (model.linear_ineq_constraint_upper_bounds());
  }

  if (model.num_linear_eq_constraints()) {

    const RealMatrix& sm_coeffs = model.linear_eq_constraint_coeffs();
    RealMatrix dt_coeffs(sm_coeffs.numRows(),
			 sm_coeffs.numCols() + numHyperparams);
    RealMatrix sm_subset(Teuchos::View, dt_coeffs, sm_coeffs.numRows(),
			 sm_coeffs.numCols(), 0, 0);
    sm_subset.assign(sm_coeffs);
    userDefinedConstraints.linear_eq_constraint_coeffs(dt_coeffs);

    userDefinedConstraints.linear_eq_constraint_targets
      (model.linear_eq_constraint_targets());
  }
}


/** Expand response-related arrays, accounting for multiple
    experiments and/or interpolation. */
void DataTransformModel::update_expanded_response(const Model& model)
{
  size_t num_recast_primary = expData.num_total_exppoints();

  // TODO: verify that clients of this Model will properly work with
  // expanded weights, sense, scales.

  // Preserve weights through data transformations
  // Weights are always by group, expanded in DakotaModel; just replicate
  expand_primary_array(model.primary_response_fn_weights().length(),
		       model.primary_response_fn_weights(),
		       num_recast_primary, primaryRespFnWts);

  // TODO: Sense is 1 or group, and NOT currently properly expanded
  // for fields in DakotaModel
  // TODO: Moreover sense does not apply to calibration_terms?!?
  // Preserve sense through data transformations
  expand_primary_array(model.primary_response_fn_sense().size(),
		       model.primary_response_fn_sense(),
		       num_recast_primary, primaryRespFnSense);

  // Scaling-related notes:
  //  * RecastModel pulls ScalingOpts from subModel
  //  * CV scales don't change in this recasting; base RecastModel captures them
  //  * TODO: What if there are hyper-parameters active?

  // Ideally, disallow per-element response scaling when interpolation
  // is active, however user might have toggled "scaling" off in
  // method, so don't make fatal. TODO: needs tighter check
  if (scalingOpts.priScales.length() == model.num_primary_fns() &&
      expData.interpolate_flag()) {
    Cout << "\nWarning: When interpolating simulation to calibration data, "
	 << "primary\nresponse scales should not be specified per field element,"
	 << "rather\nper response group, or a single value." << std::endl;
  }

  // Adjust each scaling type to right size, leaving as length 1 if needed
  expand_primary_array(model.scaling_options().priScaleTypes.size(),
		       model.scaling_options().priScaleTypes,
		       num_recast_primary, scalingOpts.priScaleTypes);
  expand_primary_array(model.scaling_options().priScales.length(),
		       model.scaling_options().priScales,
		       num_recast_primary, scalingOpts.priScales);

  // update primary labels in-place
  expData.fill_primary_function_labels
    (currentResponse.shared_data().function_labels());

  // nonlinear constraints aren't affected by data transforms
  update_secondary_response(model);
}


/** Incorporate the hyper parameters into Variables, assuming they are at the 
    end of the active continuous variables.  For example, append them to 
    continuous design or continuous aleatory uncertain. */ 
SizetArray DataTransformModel::
variables_expand(const Model& sub_model, size_t num_hyper)
{
  SizetArray vc_totals;  // default is no size change
  if (num_hyper > 0) {
    const SharedVariablesData& svd = sub_model.current_variables().shared_data();
    vc_totals = svd.components_totals();
    vc_totals[get_hyperparam_vc_index(sub_model)] += num_hyper;
  }
  return vc_totals;
}


int DataTransformModel::get_hyperparam_vc_index(const Model& sub_model)
{
  int vc_index = TOTAL_CDV;

  const SharedVariablesData& svd = sub_model.current_variables().shared_data();
  const SizetArray& vc_totals = svd.components_totals();
  short active_view = sub_model.current_variables().view().first;
  switch (active_view) {
      
  case MIXED_DESIGN: case RELAXED_DESIGN:
    // append to end of continuous design
    vc_index = TOTAL_CDV;

  case MIXED_ALEATORY_UNCERTAIN: case RELAXED_ALEATORY_UNCERTAIN:
    // append to end of continuous aleatory
    vc_index = TOTAL_CAUV;
    break;

  case MIXED_UNCERTAIN: case RELAXED_UNCERTAIN: 
  case MIXED_EPISTEMIC_UNCERTAIN: case RELAXED_EPISTEMIC_UNCERTAIN:
    // append to end of continuous epistemic (note there may not actually be 
    // any epistemic variables in the *_UNCERTAIN cases)
    vc_index = TOTAL_CEUV;
    break;

  case MIXED_ALL: case RELAXED_ALL: case MIXED_STATE: case RELAXED_STATE:
    // append to end of continuous state
    vc_index = TOTAL_CSV;
    break;

  default:
    Cerr << "\nError: invalid active variables view " << active_view 
	 << " in DataTransformModel.\n";
    abort_handler(MODEL_ERROR);
    break;

  }
  
  return vc_index;
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
  size_t num_scalar = srd.num_scalar_primary(),
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
transform_inactive_variables(const Variables& exp_config_vars,
			     Variables& sub_model_vars)
{
  // Note: experiment configuration vars are imported in "user space" (i.e.,
  // the original vars spec) and are transformed to this model's "iterator
  // space" in the constructor by user_space_to_iterator_space().

  // experimental configurations are always stored as inactive vars (refer
  // to ExperimentData ctor).  Thus we alternate only on the subModel view.
  short sm_active_view = sub_model_vars.view().first; 
  if (sm_active_view == RELAXED_ALL || sm_active_view == MIXED_ALL)
    sub_model_vars.inactive_into_all_variables(exp_config_vars);
  else //if (sm_active_view >= RELAXED_DESIGN)
    sub_model_vars.inactive_variables(exp_config_vars);
}


/** Blocking evaluation over all experiment configurations to compute
    a single set of expanded residuals.  If the subModel supports
    asynchronous evaluate_nowait(), do the configuration evals
    concurrently and then synchronize. */
void DataTransformModel::derived_evaluate(const ActiveSet& set)
{
  // If no configuration variables, use base class implementation
  if (expData.num_config_vars() == 0)
    RecastModel::derived_evaluate(set);
  else {
    ++recastModelEvalCntr;

    // transform from recast (Iterator) to sub-model (user) variables;
    // NOTE: these are the same for all configurations
    transform_variables(currentVariables, subModel.current_variables());

    // the incoming set is for the recast problem, which must be converted
    // back to the underlying response set for evaluation by the subModel.
    ActiveSet sub_model_set;
    transform_set(currentVariables, set, sub_model_set);
    // update currentResponse early as it's used in form_residuals
    currentResponse.active_set(set);

    if (outputLevel >= VERBOSE_OUTPUT) {
      Cout << "\n------------------------------------";
      Cout << "\nEvaluating model for each experiment";
      Cout << "\n------------------------------------" << std::endl;
    }

    size_t num_exp = expData.num_experiments();
    const VariablesArray& config_vars = expData.configuration_variables();
    Variables& sm_vars = subModel.current_variables();
    for (size_t i=0; i<num_exp; ++i) {
      // update the subModel variables with the experiment configuration vars
      transform_inactive_variables(config_vars[i], sm_vars);

      if (subModel.asynch_flag()) {
        subModel.evaluate_nowait(sub_model_set);
        // be able to map the subModel's evalID back to the right
        // recastModel eval and omit evals we didn't schedule
        // Don't need to cache ActiveSet or Variables
        recastIdMap[subModel.evaluation_id()] = recastModelEvalCntr;
      }
      else {
        // No need to cache when the subModel is synchronous; populate
        // a subset of residuals for each subModel eval
        subModel.evaluate(sub_model_set);
        // recast the subModel response ("user space") into the currentResponse
        // ("iterator space"); populate one experiment's residuals
        expData.form_residuals(subModel.current_response(), i, currentResponse);
      }
    }

    if (subModel.asynch_flag()) {
      // Synchronize and map all configurations to the single eval's residuals
      // BMA TODO ask MSE: Will these always be in the right order (seems so);
      // it's important to map the responses in the right order (not
      // checking yet)

      // In this case we don't need a mapping to a recast ID, just an
      // IntResponseMap with the subset of evals we own; filter in-place
      const IntResponseMap& submodel_resp = filter_submodel_responses();
      transform_response_map(submodel_resp, currentVariables, currentResponse);
    }
    else {
      // BMA TODO: doesn't need submodel vars...
      scale_response(subModel.current_variables(), currentVariables, 
                     currentResponse);
    }

    print_residual_response(currentResponse);

    // BMA TODO:
    // We know that DataTransformModel didn't register a secondary
    // transformation, but what if the submodel constraints differ per
    // configuration?  Need to aggregate/expand the constraints!
    //RecastModel::transform_secondary_response();
  }
}


/** Non-blocking evaluation (scheduling) over all experiment
    configurations.  Assumes that if this model supports nowait, its
    subModel does too and schedules them all. */
void DataTransformModel::derived_evaluate_nowait(const ActiveSet& set)
{
  // BMA ask MSE: Is it possible RecastModel is asynch, but not subModel?

  // If no configuration variables, use base class implementation
  if (expData.num_config_vars() == 0)
    RecastModel::derived_evaluate_nowait(set);
  else {
    ++recastModelEvalCntr;

    // transform from recast (Iterator) to sub-model (user) variables
    // NOTE: these are the same for all configurations
    transform_variables(currentVariables, subModel.current_variables());

    // the incoming set is for the recast problem, which must be converted
    // back to the underlying response set for evaluation by the subModel.
    ActiveSet sub_model_set;
    transform_set(currentVariables, set, sub_model_set);

    if (outputLevel >= VERBOSE_OUTPUT) {
      Cout << "\n------------------------------------";
      Cout << "\nEvaluating model for each experiment";
      Cout << "\n------------------------------------" << std::endl;
    }

    size_t num_exp = expData.num_experiments();
    const VariablesArray& config_vars = expData.configuration_variables();
    Variables& sm_vars = subModel.current_variables();
    for (size_t i=0; i<num_exp; ++i) {
      // update the subModel variables with the experiment configuration vars
      transform_inactive_variables(config_vars[i], sm_vars);

      subModel.evaluate_nowait(sub_model_set);

      // be able to map the subModel's evalID back to the right
      // recastModel eval
      recastIdMap[subModel.evaluation_id()] = recastModelEvalCntr;
    }

    // bookkeep variables for use in primaryRespMapping/secondaryRespMapping
    //if (primaryRespMapping || secondaryRespMapping) {
    recastSetMap[recastModelEvalCntr]  = set;
    recastVarsMap[recastModelEvalCntr] = currentVariables.copy();
    // This RecastModel doens't map variables in a way that needs these
    // if (variablesMapping)
    // 	subModelVarsMap[recastModelEvalCntr] =
    // 	  subModel.current_variables().copy();
    //}
  }
}


/** Collect all the subModel evals and build the residual sets for all
    evaluations.  Like rekey functions in DakotaModel, but many
    sub-model to one recast-model.  For the blocking synchronize case,
    we force the subModel to synch and have all needed data. */
const IntResponseMap& DataTransformModel::derived_synchronize()
{
  if (expData.num_config_vars() == 0)
    return RecastModel::derived_synchronize();
  else {
    // We don't even really need the recast ID lookup nor the cached
    // data structure, but want to be tolerant of evals that this
    // Model didn't schedule (and it simplifies the indexing logic).
    bool deep_copy = false;
    cache_submodel_responses(subModel.synchronize(), deep_copy);

    // populate recastResponseMap with all evals
    bool collect_all = true;
    collect_residuals(collect_all);
  }

  return recastResponseMap;
}


/** Collect any completed subModel evals and build the residual sets
    for any fully completed evaluations.  Like rekey functions in
    DakotaModel, but many sub-model to one recast-model.  We do not
    force the subModel to synch. */
const IntResponseMap& DataTransformModel::derived_synchronize_nowait()
{
  if (expData.num_config_vars() == 0)
    return RecastModel::derived_synchronize_nowait();
  else {
    // need deep copy in case all the configurations aren't yet complete
    bool deep_copy = true;
    cache_submodel_responses(subModel.synchronize_nowait(), deep_copy);

    // populate recastResponseMap with any fully completed evals
    bool collect_all = false;
    collect_residuals(collect_all);
  }

  return recastResponseMap;
}


void DataTransformModel::vars_mapping(const Variables& recast_vars, 
				      Variables& submodel_vars)
{
  // Forward the calibration parameters, omitting the hyper-parameters, which 
  // are assumed to trail the active continuous variables.
  RealVector sm_cv = submodel_vars.continuous_variables_view();
  copy_data_partial(recast_vars.continuous_variables(), 0, 
		    (int)submodel_vars.cv(), sm_cv);

  // this map only supports continuous variables, but rest need to come along
  submodel_vars.discrete_int_variables(recast_vars.discrete_int_variables());
  submodel_vars.discrete_string_variables(
    recast_vars.discrete_string_variables());
  submodel_vars.discrete_real_variables(recast_vars.discrete_real_variables());
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
  size_t max_sm_id = dtModelInstance->subordinate_model().current_variables().cv();
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

  // form residuals (and gradients/Hessians) from the simulation
  // response this call has to be careful not to resize gradients and
  // Hessians in a way that tramples hyper-parameters: only update
  // submodel cv entries, leaving objects sized
  dtModelInstance->expData.form_residuals(submodel_response, recast_response);

  // scale by covariance, including hyper-parameter multipliers
  dtModelInstance->scale_response(submodel_vars, recast_vars, recast_response);

  // no sensible way to transform metadata in multi-config case
  if (dtModelInstance->expData.configuration_variables().size() > 1)
    recast_response.metadata(submodel_response.metadata());

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


/** (We don't quite want the rekey behavior since multiple subModel
    evals map to one recast eval.) */
const IntResponseMap& DataTransformModel::filter_submodel_responses()
{
  const IntResponseMap& sm_resp_map = subModel.synchronize();
  // Not using BOOST_FOREACH due to potential for iterator invalidation in 
  // erase in cache_unmatched_response (shouldn't be a concern with map?)
  IntRespMCIter sm_r_it = sm_resp_map.begin(), sm_r_end = sm_resp_map.end();
  while (sm_r_it != sm_r_end) {
    int sm_id = sm_r_it->first;
    IntIntMIter id_it = recastIdMap.find(sm_id);
    if (id_it != recastIdMap.end()) {
      // no need to cache, just leave in the subModel's IntResponseMap
      recastIdMap.erase(sm_id);
      ++sm_r_it;
    }
    else {
      ++sm_r_it; // prior to invalidation from erase()
      subModel.cache_unmatched_response(sm_id);
    }
  }
  return sm_resp_map;
}


void DataTransformModel::
cache_submodel_responses(const IntResponseMap& sm_resp_map, bool deep_copy)
{
  // Not using BOOST_FOREACH due to potential for iterator invalidation in 
  // erase in cache_unmatched_response (shouldn't be a concern with map?)
  IntRespMCIter sm_r_it = sm_resp_map.begin(), sm_r_end = sm_resp_map.end();
  while (sm_r_it != sm_r_end) {
    int sm_id = sm_r_it->first;
    IntIntMIter id_it = recastIdMap.find(sm_id);
    if (id_it != recastIdMap.end()) {
      int recast_id = id_it->second;

      // this is our eval, cache it as <recast_id, <sm_id, Response> >
      if (cachedResp.find(recast_id) == cachedResp.end()) {
        // insert a new recast_id instance
        cachedResp[recast_id] = IntResponseMap();
        cachedResp[recast_id][sm_id] = deep_copy ? sm_r_it->second.copy() :
          sm_r_it->second;
      }
      else
        cachedResp[recast_id][sm_id] = deep_copy ? sm_r_it->second.copy() :
          sm_r_it->second;
      recastIdMap.erase(sm_id);
      ++sm_r_it;
    }
    else {
      ++sm_r_it;
      subModel.cache_unmatched_response(sm_id);
    }
  }
}


void DataTransformModel::collect_residuals(bool collect_all)
{
  recastResponseMap.clear();

  //BOOST_FOREACH(IntIntResponseMapMap::value_type& cr_pair, cachedResp) 
  IntIntResponseMapMap::iterator cr_pair = cachedResp.begin();
  while(cr_pair != cachedResp.end()) {
    int recast_id = cr_pair->first;  // (.second is a subModel IntResponseMap)
    size_t num_exp = expData.num_experiments();

    // the blocking synch case requires all data present
    if (collect_all && cr_pair->second.size() != num_exp) {
      Cerr << "\nError (DataTransformModel): Sub-model returned " 
           << cr_pair->second.size() << "evaluations,\n  but have " << num_exp 
           << " experiment configurations.\n";
      abort_handler(MODEL_ERROR);
    }

    // populate recastResponseMap with any recast evals that have all
    // their configs complete (only complete/clear those with finished
    // experiment configs)
    if (cr_pair->second.size() == num_exp) {

      IntASMIter s_it = recastSetMap.find(recast_id);
      IntVarsMIter v_it = recastVarsMap.find(recast_id);

      recastResponseMap[recast_id] = currentResponse.copy();
      recastResponseMap[recast_id].active_set(s_it->second);

      transform_response_map(cr_pair->second, v_it->second,
                             recastResponseMap[recast_id]);

      // cleanup (could do clear() at end)
      recastVarsMap.erase(v_it);
      recastSetMap.erase(s_it);
      // BMA TODO:
      //RecastModel::transform_secondary_response();
      // BMA TODO: consider iterator here instead of value?
      cr_pair++;      
      cachedResp.erase(recast_id);
      print_residual_response(recastResponseMap[recast_id]);
    } else {
      cr_pair++;
    }
  }
}


/** This transformation assumes the residuals are in submodel eval_id
    order. */
void DataTransformModel::
transform_response_map(const IntResponseMap& submodel_resp,
                       const Variables& recast_vars,
                       Response& residual_resp)
{
  size_t num_exp = expData.num_experiments();
  if (submodel_resp.size() != num_exp) {
    // unsupported case: could (shouldn't) happen in complex MF workflows
    Cerr << "\nError (DataTransformModel): sub model evals wrong size.\n";
    abort_handler(MODEL_ERROR);
  }
  IntRespMCIter sm_eval_it = submodel_resp.begin();
  for (size_t i=0; i<num_exp; ++i, ++sm_eval_it)
    expData.form_residuals(sm_eval_it->second, i, residual_resp);

  // scale by covariance, including hyper-parameter multipliers
  // BMA TODO: doesn't need submodel vars...
  scale_response(subModel.current_variables(), recast_vars, residual_resp);
}

void DataTransformModel::
scale_response(const Variables& submodel_vars, 
	       const Variables& recast_vars,
	       Response& recast_response)
{
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


/** Pull up the continuous variable values and labels into the
    RecastModel, inserting the hyper-parameter values/labels  */
void DataTransformModel::init_continuous_vars()
{
  const SharedVariablesData& svd = subModel.current_variables().shared_data();
  const SizetArray& sm_vc_totals = svd.components_totals();
  const RealVector& sm_acv = subModel.current_variables().all_continuous_variables();
  StringMultiArrayConstView sm_acvl = subModel.current_variables().all_continuous_variable_labels();
  const RealVector & sm_aclb = subModel.all_continuous_lower_bounds();
  const RealVector & sm_acub = subModel.all_continuous_upper_bounds();

  int continuous_vc_inds[4] = {TOTAL_CDV, TOTAL_CAUV, TOTAL_CEUV, TOTAL_CSV};
  int hyperparam_vc_ind = get_hyperparam_vc_index(subModel);

  size_t sm_offset = 0;
  size_t dtm_offset = 0;

  for(const int& vci : continuous_vc_inds) {
    
    size_t num_cvars = sm_vc_totals[vci];
    for (size_t i=0; i<num_cvars; ++i) {
      current_variables().all_continuous_variable(sm_acv[sm_offset], dtm_offset);
      current_variables().all_continuous_variable_label(sm_acvl[sm_offset], dtm_offset);
      all_continuous_lower_bound(sm_aclb[sm_offset], dtm_offset);
      all_continuous_upper_bound(sm_acub[sm_offset], dtm_offset);
      ++sm_offset;
      ++dtm_offset;
    }
    if (vci == hyperparam_vc_ind) {
      // insert the hyper-parameter values/labels
      const StringArray& hyper_labels = 
	expData.hyperparam_labels(obsErrorMultiplierMode);
      for (size_t i=0; i<numHyperparams; ++i) {
	current_variables().all_continuous_variable(1.0, dtm_offset);
	current_variables().all_continuous_variable_label(hyper_labels[i], dtm_offset);
	all_continuous_lower_bound(0.0, dtm_offset);
	all_continuous_upper_bound(std::numeric_limits<double>::infinity(),
				   dtm_offset);
	++dtm_offset;
      }
    }

  }

}


/** Passing the inbound array size so we can use one function for
    Teuchos and std containers (size vs. length) */
template<typename T>
void DataTransformModel::
expand_primary_array(size_t submodel_size, const T& submodel_array,
		     size_t recast_size, T& recast_array) const 
{
  // Assume that coefficients have been expanded for fields (weights, scales, sense)
  // So each is either len1 or num_elements
  if (submodel_size == 1)
    // this copy may not be needed, depends on ctor behavior
    recast_array = submodel_array;
  else if (submodel_size > 1) {

    // TODO: convenience function to do this
    // For num_elements case, just fill the recast_array with copies
    size_t num_exp = expData.num_experiments();

    // TODO: this will fail in general for interpolation-active cases
    assert(submodel_size * num_exp == recast_size);

    recast_array.resize(recast_size);
    size_t calib_term_ind = 0;
    for (size_t exp_ind = 0; exp_ind < num_exp; ++exp_ind)
      for (size_t sma_ind = 0; sma_ind < submodel_size; ++sma_ind)
	recast_array[calib_term_ind++] = submodel_array[sma_ind];
  }
  // else leave recast_array empty
}


void DataTransformModel::print_residual_response(const Response& resid_resp)
{
  if (outputLevel >= VERBOSE_OUTPUT) {
    Cout << "\n-----------------------------------------------------------";
    Cout << "\nPost-processing Function Evaluation: Data Transformation";
    Cout << "\n-----------------------------------------------------------"
	 << std::endl;
  }

  if (outputLevel >= VERBOSE_OUTPUT &&
      subordinate_model().num_primary_fns() > 0) {
    Cout << "Calibration data transformation; residuals:\n";
    write_data(Cout, resid_resp.function_values(),
	       resid_resp.function_labels());
    Cout << std::endl;
  }
  if (outputLevel >= DEBUG_OUTPUT &&
      subordinate_model().num_primary_fns() > 0) {
    Cout << "Calibration data transformation; full response:\n"
	 << resid_resp << std::endl;
  }
}


void DataTransformModel::
print_best_responses(std::ostream& s, 
                     const Variables& best_submodel_vars,
                     const Response& best_submodel_resp,
                     size_t num_best, size_t best_ind)
{
  // BMA TODO: Why copying the response, why not just update dataTransformModel?
  Response residual_resp(current_response().copy());
  // only transform residuals, not derivatives
  ActiveSet fn_only_as = residual_resp.active_set();
  fn_only_as.request_values(1);
  residual_resp.active_set(fn_only_as);

  s << "Original (as-posed) response:\n";

  // print the original userModel Responses
  if (expData.num_config_vars() == 0) {
    const RealVector& best_fns = best_submodel_resp.function_values();
    Minimizer::print_model_resp(subModel.num_primary_fns(), best_fns, num_best,
                                best_ind, s);
    // form residuals from model responses and apply covariance
    short dt_verbosity = output_level();
    output_level(SILENT_OUTPUT);
    data_transform_response(best_submodel_vars, best_submodel_resp,
                            residual_resp);
    output_level(dt_verbosity);
  }
  else {
    recover_submodel_responses(s, best_submodel_vars, 
                               num_best, best_ind, residual_resp);
  }

  const RealVector& resid_fns = residual_resp.function_values();
  // The data transformation may be weighting by variance; remind the user:
  if (expData.variance_active())
    s << "Variance-weighted original (as-posed) residuals:\n";
  else
    s << "Original (as-posed) residuals:\n";
  Minimizer::print_residuals(num_primary_fns(), resid_fns, RealVector(),
			     num_best, best_ind, s);

  // must use the expanded weight set from the data difference model
  const RealVector& lsq_weights = Model::primary_response_fn_weights();
  Minimizer::print_residuals(num_primary_fns(), resid_fns, lsq_weights,
                             num_best, best_ind, s);
}


// The core of this can be modular on the Model (and static)
void DataTransformModel::
recover_submodel_responses(std::ostream& s, 
                           const Variables& best_submodel_vars,
                           size_t num_best, size_t best_ind,
                           Response& residual_resp)
{
  if (subModel.num_primary_fns() > 1 || expData.num_config_vars() > 1)
    s << "<<<<< Best model responses ";
  else
    s << "<<<<< Best model response ";
  if (num_best > 1) s << "(set " << best_ind+1 << ") ";
  s << "\n";

  // first try cache lookup

  // Have to make sure the Variables object is in the right mode for
  // inactive operations. Make a deep copy of the SharedVariablesData
  // to avoid corrupting any inbound Variables information... (const
  // doesn't protect the SVD)
  Variables lookup_vars = best_submodel_vars.copy(true);
  lookup_vars.inactive_view(MIXED_STATE);

  String interface_id = subModel.interface_id();
  Response lookup_resp = subModel.current_response().copy();
  ActiveSet lookup_as = lookup_resp.active_set();
  // TODO: don't need to lookup constraints here...
  lookup_as.request_values(1);
  lookup_resp.active_set(lookup_as);
  ParamResponsePair lookup_pr(lookup_vars, interface_id, lookup_resp);

  // use model_resp to populate residuals as we go
  Response model_resp;
  size_t num_exp = expData.num_experiments();
  for (size_t i=0; i<num_exp; ++i) {

    lookup_vars.inactive_variables(expData.configuration_variables()[i]);

    // TODO: use user-provided experiment numbers if given
    s << "<<<<< Best configuration variables (experiment " << i+1 << ") =\n";
    lookup_vars.write(s, INACTIVE_VARS);  // vars object writes labels

    bool lookup_failure = false;
    // BMA: why is this necessary?  Should have a reference to same object as PRP
    lookup_pr.variables(lookup_vars);
    PRPCacheHIter cache_it = lookup_by_val(data_pairs, lookup_pr);

    // TODO: allow exact or partial match...
    if (cache_it == data_pairs.get<hashed>().end()) {

      // If model is a data fit surrogate, re-evaluate it if needed.
      // Didn't use != "ensemble" in case other surrogate types are added.
      if ( subModel.model_type() == "surrogate" &&
           (strbegins(subModel.surrogate_type(), "global_") ||
            strbegins(subModel.surrogate_type(), "local_") ||
            strbegins(subModel.surrogate_type(), "multipoint_")) ) {
        // TODO: Want to make this a quiet evaluation, but not easy to
        // propagate to the interface?!?
        //subModel.ouput_level(SILENT_OUTPUT); // and need restore
        subModel.current_variables() = lookup_vars;
        subModel.evaluate(lookup_resp.active_set());
        model_resp = subModel.current_response();

        // TODO: There are other cases where re-evaluate would be
        // safe, like a recast of a simulation model that has caching
        // enabled, but don't treat that for now.
      }
      else {
        // BMA TODO: Consider NaN so downstream output isn't misleading
        lookup_failure = true;
        s << "<<<<< Best model responses (experiment " << i+1
          << ") not available\n";
      }
    }
    else {
      model_resp = cache_it->response();
    }

    if (lookup_failure) {
      // print eval IDs for any partial matches
      Minimizer::print_best_eval_ids(interface_id, lookup_vars, lookup_as, s);
    }
    else {
      expData.form_residuals(model_resp, i, residual_resp);

      // By including labels, this deviates from other summary function output
      if (subModel.num_primary_fns() > 1)
        s << "<<<<< Best model responses (experiment " << i+1 << ") =\n";
      else
        s << "<<<<< Best model response (experiment " << i+1 << ") =\n";
      write_data_partial(s, (size_t)0, subModel.num_primary_fns(),
                         model_resp.function_values(),
                         model_resp.function_labels());
      Minimizer::print_best_eval_ids(interface_id, lookup_vars, lookup_as, s);
    }
  }

  // TODO: this won't scale properly if hyper-parameters are
  // calibrated; would need the error multipliers, which are in the
  // recast variables space
  scale_response(subModel.current_variables(), currentVariables, residual_resp);
}

void DataTransformModel::
archive_best_residuals(const ResultsManager &results_db, 
                              const StrStrSizet &iterator_id,
                              const int num_terms,
                              const RealVector &best_terms, 
                              const Real wssr, const int num_points,
                              const int point_index) {
  if(!results_db.active()) return;
  
  StringArray residuals_location;
  StringArray norm_location;
  if(num_points > 1) {
    String set_string = String("set:") + std::to_string(point_index+1);
    residuals_location.push_back(set_string);
    norm_location.push_back(set_string);
  }
  residuals_location.push_back("best_residuals");
  norm_location.push_back("best_norm");
  Teuchos::SerialDenseVector<int, Real> residuals(Teuchos::View, 
                              const_cast<Real*>(&best_terms[0]), 
                              num_terms);
  results_db.insert(iterator_id, residuals_location, residuals);
  results_db.insert(iterator_id, norm_location, wssr);
}

void DataTransformModel::archive_best_original(const ResultsManager &results_db, 
                                               const StrStrSizet &iterator_id, 
                                               const RealVector &function_values, 
                                               const int &exp_index,
                                               const int &num_best, 
                                               const int &best_index) {
  if(!results_db.active())  return;
  
  DimScaleMap scales;
  scales.emplace(0, StringScale("responses", subModel.response_labels(), 
                                ScaleScope::SHARED)
                );

  StringArray location;
  if(num_best > 1)
    location.push_back(String("set:") + std::to_string(best_index+1));
  location.push_back("best_model_responses");
  if(expData.num_config_vars()) {
    location.push_back(String("experiment:") + std::to_string(exp_index+1));
    location.push_back("responses");
  }

  results_db.insert(iterator_id, location, function_values, scales);
}


void DataTransformModel::
archive_submodel_responses(const ResultsManager &results_db, 
                           const StrStrSizet &iterator_id,
                           const Variables& best_submodel_vars,
                           size_t num_best, size_t best_ind,
                           Response& residual_resp)
{
  // first try cache lookup

  // Have to make sure the Variables object is in the right mode for
  // inactive operations. Make a deep copy of the SharedVariablesData
  // to avoid corrupting any inbound Variables information... (const
  // doesn't protect the SVD)
  

  Variables lookup_vars = best_submodel_vars.copy(true);
  lookup_vars.inactive_view(MIXED_STATE);

  String interface_id = subModel.interface_id();
  Response lookup_resp = subModel.current_response().copy();
  ActiveSet lookup_as = lookup_resp.active_set();
  lookup_as.request_values(1);
  lookup_resp.active_set(lookup_as);
  ParamResponsePair lookup_pr(lookup_vars, interface_id, lookup_resp);

  // use model_resp to populate residuals as we go
  Response model_resp;
  size_t num_exp = expData.num_experiments();
  for (size_t i=0; i<num_exp; ++i) {

    lookup_vars.inactive_variables(expData.configuration_variables()[i]);

    bool lookup_failure = false;
    // BMA: why is this necessary?  Should have a reference to same object as PRP
    lookup_pr.variables(lookup_vars);
    PRPCacheHIter cache_it = lookup_by_val(data_pairs, lookup_pr);
    if (cache_it == data_pairs.get<hashed>().end()) {

      // If model is a data fit surrogate, re-evaluate it if needed.
      // Didn't use != "ensemble" in case other surrogate types are added.
      if ( subModel.model_type() == "surrogate" &&
           (strbegins(subModel.surrogate_type(), "global_") ||
            strbegins(subModel.surrogate_type(), "local_") ||
            strbegins(subModel.surrogate_type(), "multipoint_")) ) {
        // TODO: Want to make this a quiet evaluation, but not easy to
        // propagate to the interface?!?
        //subModel.ouput_level(SILENT_OUTPUT); // and need restore
        subModel.current_variables() = lookup_vars;
        subModel.evaluate(lookup_resp.active_set());
        model_resp = subModel.current_response();

        // TODO: There are other cases where re-evaluate would be
        // safe, like a recast of a simulation model that has caching
        // enabled, but don't treat that for now.
      }
    }
    else {
      model_resp = cache_it->response();
    }

    if (!lookup_failure) {
      expData.form_residuals(model_resp, i, residual_resp);
      archive_best_original(results_db, iterator_id, 
                             model_resp.function_values(), 
                             i, num_best, best_ind);
      if(expData.num_config_vars() != 0)
        archive_best_config_variables(results_db, iterator_id, 
                                     lookup_vars, i, 
                                     num_best, best_ind);

    }
  }

  // TODO: this won't scale properly if hyper-parameters are
  // calibrated; would need the error multipliers, which are in the
  // recast variables space
  scale_response(subModel.current_variables(), currentVariables, residual_resp);
}

/// archive best responses 
void DataTransformModel::
archive_best_responses(const ResultsManager &results_db,
                       const StrStrSizet iterator_id,
                       const Variables& best_submodel_vars,
                       const Response& best_submodel_resp,
                       size_t num_best, size_t best_ind) {

  // BMA TODO: Why copying the response, why not just update dataTransformModel?
  Response residual_resp(current_response().copy());
  // only transform residuals, not derivatives
  ActiveSet fn_only_as = residual_resp.active_set();
  fn_only_as.request_values(1);
  residual_resp.active_set(fn_only_as);

  // print the original userModel Responses
  if (expData.num_config_vars() == 0) {
    const RealVector& best_fns = best_submodel_resp.function_values();
    archive_best_original(results_db, iterator_id, best_fns, 0, num_best, best_ind);
    // form residuals from model responses and apply covariance
    short dt_verbosity = output_level();
    output_level(SILENT_OUTPUT);
    data_transform_response(best_submodel_vars, best_submodel_resp,
                            residual_resp);
    output_level(dt_verbosity);
  }
  else {
    archive_submodel_responses(results_db, iterator_id, best_submodel_vars, 
                               num_best, best_ind, residual_resp);
  }

  const RealVector& resid_fns = residual_resp.function_values();
  // print_residuals will archive the residuals. for now, skip archiving the un-weighted
  // ones by passing in an uninitialized ResultsManager.
  const RealVector& lsq_weights = Model::primary_response_fn_weights();
  Real wssr = std::sqrt(Minimizer::sum_squared_residuals(num_primary_fns(), 
                                                         resid_fns, lsq_weights)
                       );
  // Currently only the weighted residuals are archived. This differs from what's printed
  // to the console (see print_best_responses() above).
  archive_best_residuals(results_db, iterator_id, num_primary_fns(), 
      resid_fns, wssr, num_best, best_ind);

}

/// Archive best configuration variables
void DataTransformModel::
archive_best_config_variables(const ResultsManager &results_db, 
                              const StrStrSizet &iterator_id,
                              const Variables &vars,
                              const int &exp_index,
                              const int &num_best, 
                              const int &best_index) {

  if(!results_db.active())  return;
  // When using configuration variables, all inactive variables are
  // expected to be config vars.

  const auto & cv_labels = vars.inactive_continuous_variable_labels();
  const auto & div_labels = vars.inactive_discrete_int_variable_labels();
  const auto & dsv_labels = vars.inactive_discrete_string_variable_labels();
  const auto & drv_labels = vars.inactive_discrete_real_variable_labels();

  StringArray location;
  size_t d_index = 2;
  if(num_best > 1) {
    location.push_back(String("set:") + std::to_string(best_index+1));
    d_index = 3;
    
  }
  location.push_back("best_model_responses");
  location.push_back(String("experiment:") + std::to_string(exp_index+1));
  location.push_back(""); // placeholder for variable domain
  if(cv_labels.size()) {
    location[d_index] = "continuous_config_variables";
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", cv_labels));
    results_db.insert(iterator_id, location, 
        vars.inactive_continuous_variables(), scales);
  }
  if(div_labels.size()) {
    location[d_index] = "discrete_integer_config_variables";
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", div_labels));
    results_db.insert(iterator_id, location, 
        vars.inactive_discrete_int_variables(), scales);
  }
  if(dsv_labels.size()) {
    location[d_index] = "discrete_string_config_variables";
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", dsv_labels));
    results_db.insert(iterator_id, location, 
        vars.inactive_discrete_string_variables(), scales);
  }
  if(drv_labels.size()) {
    location[d_index] = "discrete_real_config_variables";
    DimScaleMap scales;
    scales.emplace(0, StringScale("variables", drv_labels));
    results_db.insert(iterator_id, location, 
        vars.inactive_discrete_real_variables(), scales);
  }
}

int DataTransformModel::num_config_vars() const {
  return expData.num_config_vars();
}

}  // namespace Dakota
