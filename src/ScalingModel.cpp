/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       ScalingModel
//- Description: Implementation code for the ScalingModel class
//- Owner:       Brian Adams
//- Checked by:

#include "ScalingModel.hpp"

static const char rcsId[]="@(#) $Id$";

namespace Dakota {

/// initialization of static needed by RecastModel
ScalingModel* ScalingModel::scaleModelInstance(NULL);


/** This constructor computes various indices and mappings, then
    updates the properties of the RecastModel */
ScalingModel::
ScalingModel(Model& sub_model):
  // BMA TODO: should the BitArrays be empty or same as submodel?
  // recast_secondary_offset is the index to the equality constraints within 
  // the secondary responses
  RecastModel(sub_model, SizetArray(), BitArray(), BitArray(), 
	      sub_model.num_primary_fns(), 
	      sub_model.num_functions() - sub_model.num_primary_fns(),
	      sub_model.num_nonlinear_ineq_constraints(),
	      response_order(sub_model))
{
  if (outputLevel >= DEBUG_OUTPUT)
    Cout << "Initializing scaling transformation" << std::endl;

  scaleModelInstance = this;

  // RecastModel is constructed, then later initialized because scaled
  // properties need to be set on the RecastModel, like bounds, but
  // the nonlinearity of the mapping is determined by the scales
  // themselves.

  // initialize_scaling function needs to modify the iteratedModel
  // compute needed class data...
  initialize_scaling(sub_model);


  // No change in sizes for scaling
  size_t num_primary = sub_model.num_primary_fns(),
    num_secondary = sub_model.num_functions() - sub_model.num_primary_fns(),
    num_recast_fns = num_primary + num_secondary;

  // the scaling transformation doesn't change any counts of variables
  // or responses, but may require a nonlinear transformation of
  // responses (grad, Hess) when variables are transformed

  // ---
  // Variables mapping (one-to-one)
  // ---

  // BMA TODO: Scaling only works with continuous variables, but
  // verify this is the correct way to default map the other
  // variables.

  // We assume the mapping is for all active variables
  size_t total_active_vars = 
    sub_model.cv() + sub_model.div() + sub_model.dsv() + sub_model.drv();
  Sizet2DArray vars_map_indices(total_active_vars);
  bool nonlinear_vars_mapping = false;
  for (size_t i=0; i<total_active_vars; ++i) {
    vars_map_indices[i].resize(1);
    vars_map_indices[i][0] = i;
    if (varsScaleFlag && cvScaleTypes[i] & SCALE_LOG)
      nonlinear_vars_mapping = true;
  }


  // mappings from the submodel Response to residual Response
  BoolDequeArray nonlinear_resp_mapping(num_recast_fns);

  // ---
  // Primary mapping
  // ---
  Sizet2DArray primary_resp_map_indices(num_primary);
  for (size_t i=0; i<num_primary; i++) {
    primary_resp_map_indices[i].resize(1);
    primary_resp_map_indices[i][0] = i;
    nonlinear_resp_mapping[i].resize(1);
    nonlinear_resp_mapping[i][0] = 
      (primaryRespScaleFlag && responseScaleTypes[i] & SCALE_LOG);
  }

  // ---
  // Secondary mapping (one-to-one)
  // ---
  Sizet2DArray secondary_resp_map_indices(num_secondary);
  for (size_t i=0; i<num_secondary; i++) {
    secondary_resp_map_indices[i].resize(1);
    secondary_resp_map_indices[i][0] = num_primary + i;
    nonlinear_resp_mapping[num_primary+i].resize(1);
    nonlinear_resp_mapping[num_primary+i][0] = secondaryRespScaleFlag &&
      responseScaleTypes[num_primary + i] & SCALE_LOG;
  }

  // callbacks for RecastModel transformations: default maps when not needed
  void (*variables_map) (const Variables&, Variables&) = variables_scaler;
  void (*set_map)  (const Variables&, const ActiveSet&, ActiveSet&) = NULL;
  // register primary response scaler if requested, or variables scaled
  void (*primary_resp_map) 
    (const Variables&, const Variables&, const Response&, Response&) = 
    (primaryRespScaleFlag || varsScaleFlag) ? primary_resp_scaler : NULL;
  // scale secondary response if requested, or variables scaled
  void (*secondary_resp_map) (const Variables&, const Variables&,
                              const Response&, Response&) = 
    (secondaryRespScaleFlag || varsScaleFlag) ? secondary_resp_scaler : NULL;
  
  RecastModel::
    init_maps(vars_map_indices, nonlinear_vars_mapping, variables_map, set_map,
	      primary_resp_map_indices, secondary_resp_map_indices, 
	      nonlinear_resp_mapping, primary_resp_map, secondary_resp_map);

  // need inverse vars mapping for use with late updates from sub-model
  inverse_mappings(variables_unscaler, NULL, NULL, NULL);

  // Preserve weights through scaling transformation
  primary_response_fn_weights(sub_model.primary_response_fn_weights());

  // Preserve sense through scaling transformation
  // Note: for a specification of negative scaling, we will assume that
  // the user's intent is to overlay the scaling and sense as specified,
  // such that we will not enforce a flip in sense for negative scaling. 
  primary_response_fn_sense(sub_model.primary_response_fn_sense());

  // BMA TODO: consume scales so they aren't here anymore?
}


ScalingModel::~ScalingModel()
{ /* empty dtor */}


/** Since this convenience function is public, it must have a
    fall-through to return a copy for when this scaling type isn't
    active. */
RealVector ScalingModel::cv_scaled2native(const RealVector& scaled_cv) const
{
  return (varsScaleFlag) ?
    modify_s2n(scaled_cv, cvScaleTypes, cvScaleMultipliers, cvScaleOffsets) :
    scaled_cv;
}


/** Since this convenience function is public, it must behave
    correctly when this scale type isn't active.  It does, because it
    modifies in-place */
void ScalingModel::resp_scaled2native(const Variables& native_vars, 
                                      Response& updated_resp) const
{
  if (primaryRespScaleFlag || secondaryRespScaleFlag ||
      // NOTE: formerly DakotaLeastSq::post_run didn't have this check:
      need_resp_trans_byvars(updated_resp.active_set_request_vector(), 0,
                             num_primary_fns())){
    size_t num_nln_cons = 
      num_nonlinear_ineq_constraints() + num_nonlinear_eq_constraints();
    Response tmp_response = updated_resp.copy();
    if (primaryRespScaleFlag || 
        need_resp_trans_byvars(tmp_response.active_set_request_vector(), 0,
                               num_primary_fns())) {
      response_modify_s2n(native_vars, updated_resp, tmp_response, 0, 
                          num_primary_fns());
      updated_resp.update_partial(0, num_primary_fns(), tmp_response, 0 );
    }
    if (secondaryRespScaleFlag || 
        need_resp_trans_byvars(tmp_response.active_set_request_vector(),
                               num_primary_fns(), num_nln_cons)) {
      response_modify_s2n(native_vars, updated_resp, tmp_response,
                          num_primary_fns(), num_nln_cons);
      updated_resp.update_partial(num_primary_fns(), num_nln_cons,
                                  tmp_response, num_primary_fns());
    }
  }
}


/** Since this convenience function is public, it must have a
    fall-through to return a copy for when this scaling type isn't
    active. */
void ScalingModel::
secondary_resp_scaled2native(const RealVector& scaled_nln_cons,
                             const ShortArray& asv,
                             RealVector& native_fns) const
{
  size_t num_nln_cons = 
    num_nonlinear_ineq_constraints() + num_nonlinear_eq_constraints();
  if (secondaryRespScaleFlag || 
      need_resp_trans_byvars(asv, num_primary_fns(), num_nln_cons)) {
    // scale all functions, but only copy constraints
    copy_data_partial
      (modify_s2n(scaled_nln_cons, responseScaleTypes, responseScaleMultipliers,
                  responseScaleOffsets),
       num_primary_fns(), num_nln_cons, native_fns, num_primary_fns());
  }
  else 
    copy_data_partial(scaled_nln_cons, num_primary_fns(), num_nln_cons, 
                      native_fns, num_primary_fns());
}

short ScalingModel::response_order(const Model& sub_model)
{
  const Response& curr_resp = sub_model.current_response();

  short recast_resp_order = 1; // recast resp order to be same as original resp
  if (!curr_resp.function_gradients().empty()) recast_resp_order |= 2;
  if (!curr_resp.function_hessians().empty())  recast_resp_order |= 4;

  return recast_resp_order;
} 


/** Initialize scaling types, multipliers, and offsets.  Update the
    iteratedModel appropriately */
void ScalingModel::initialize_scaling(Model& sub_model)
{
  if (outputLevel > NORMAL_OUTPUT)
    Cout << "\nScalingModel: Scaling enabled ('auto' scaling is reported as "
         << "derived values)" << std::endl;
  else if (outputLevel > SILENT_OUTPUT)
    Cout << "\nScalingModel: Scaling enabled" << std::endl;

  // in the scaled case, perform numerical derivatives at the RecastModel level
  // (override the RecastModel default and the subModel default)
  supportsEstimDerivs = true;
  sub_model.supports_derivative_estimation(false);

  size_t num_cv = cv(), num_primary = num_primary_fns(),
    num_nln_ineq = num_nonlinear_ineq_constraints(),
    num_nln_eq = num_nonlinear_eq_constraints(),
    num_lin_ineq = num_linear_ineq_constraints(),
    num_lin_eq = num_linear_eq_constraints();

  // temporary arrays
  IntArray    tmp_types;
  RealVector  tmp_multipliers, tmp_offsets;
  RealVector lbs, ubs, targets;
  //RealMatrix linear_constraint_coeffs;

  // NOTE: When retrieving scaling vectors from database, excepting linear 
  //       constraints, they've already been checked at input to have length 0, 
  //       1, or number of vars, constraints, etc.

  // -----------------
  // CONTINUOUS DESIGN
  // -----------------
  const StringArray& cdv_scale_strings = scalingOpts.cvScaleTypes;
  const RealVector& cdv_scales = scalingOpts.cvScales;
  varsScaleFlag = scaling_active(cdv_scale_strings);

  copy_data(sub_model.continuous_lower_bounds(), lbs); // view->copy
  copy_data(sub_model.continuous_upper_bounds(), ubs); // view->copy

  
  compute_scaling(CDV, BOUNDS, num_cv, lbs, ubs, targets,
                  cdv_scale_strings, cdv_scales, cvScaleTypes,
                  cvScaleMultipliers, cvScaleOffsets);

  continuous_lower_bounds(lbs);
  continuous_upper_bounds(ubs);
  continuous_variables(
                       modify_n2s(sub_model.continuous_variables(), cvScaleTypes,
                                  cvScaleMultipliers, cvScaleOffsets) );

  if (outputLevel > NORMAL_OUTPUT && varsScaleFlag) {
    StringArray cv_labels;
    copy_data(continuous_variable_labels(), cv_labels);
    print_scaling("Continuous design variable scales", cvScaleTypes,
                  cvScaleMultipliers, cvScaleOffsets, cv_labels);
  }

  // each responseScale* = [fnScale*, nonlinearIneqScale*, nonlinearEqScale*]
  // to make transformations faster at run time 
  // numFunctions should reflect size of user-space model
  responseScaleTypes.resize(num_functions());
  responseScaleMultipliers.resize(num_functions());
  responseScaleOffsets.resize(num_functions());

  // -------------------------
  // OBJECTIVE FNS / LSQ TERMS
  // -------------------------
  const StringArray& primary_scale_strings = scalingOpts.priScaleTypes;
  const RealVector& primary_scales = scalingOpts.priScales;
  primaryRespScaleFlag = scaling_active(primary_scale_strings);

  lbs.size(0); ubs.size(0);
  compute_scaling(FN_LSQ, DISALLOW, num_primary, lbs, ubs, targets,
                  primary_scale_strings, primary_scales, tmp_types,
                  tmp_multipliers, tmp_offsets);

  for (int i=0; i<num_primary; ++i) {
    responseScaleTypes[i]       = tmp_types[i];
    responseScaleMultipliers[i] = tmp_multipliers[i];
    responseScaleOffsets[i]     = 0;
  }

  // --------------------
  // NONLINEAR INEQUALITY
  // --------------------
  const StringArray& nln_ineq_scale_strings = scalingOpts.nlnIneqScaleTypes;
  const RealVector& nln_ineq_scales = scalingOpts.nlnIneqScales;
  secondaryRespScaleFlag = scaling_active(nln_ineq_scale_strings);

  lbs = sub_model.nonlinear_ineq_constraint_lower_bounds();
  ubs = sub_model.nonlinear_ineq_constraint_upper_bounds();

  compute_scaling(NONLIN, BOUNDS, num_nln_ineq, lbs, ubs,
                  targets, nln_ineq_scale_strings, nln_ineq_scales, tmp_types,
                  tmp_multipliers, tmp_offsets);

  for (int i=0; i<num_nln_ineq; ++i) {
    responseScaleTypes[num_primary+i]       = tmp_types[i];
    responseScaleMultipliers[num_primary+i] = tmp_multipliers[i];
    responseScaleOffsets[num_primary+i]     = tmp_offsets[i];
  }

  nonlinear_ineq_constraint_lower_bounds(lbs);
  nonlinear_ineq_constraint_upper_bounds(ubs);

  // --------------------
  // NONLINEAR EQUALITY
  // --------------------
  const StringArray& nln_eq_scale_strings = scalingOpts.nlnEqScaleTypes;
  const RealVector& nln_eq_scales = scalingOpts.nlnEqScales;
  secondaryRespScaleFlag
    = (secondaryRespScaleFlag || scaling_active(nln_eq_scale_strings));

  lbs.size(0); ubs.size(0);
  targets = sub_model.nonlinear_eq_constraint_targets();
  compute_scaling(NONLIN, TARGET, num_nln_eq,
                  lbs, ubs, targets, nln_eq_scale_strings, nln_eq_scales,
                  tmp_types, tmp_multipliers, tmp_offsets);

  // BMA TODO: use copy_data?
  for (int i=0; i<num_nln_eq; ++i) {
    responseScaleTypes[num_primary+num_nln_ineq+i] 
      = tmp_types[i];
    responseScaleMultipliers[num_primary+num_nln_ineq+i] 
      = tmp_multipliers[i];
    responseScaleOffsets[num_primary+num_nln_ineq+i] 
      = tmp_offsets[i];
  }

  nonlinear_eq_constraint_targets(targets);

  if (outputLevel > NORMAL_OUTPUT && 
      (primaryRespScaleFlag || secondaryRespScaleFlag) )
    print_scaling("Response scales", responseScaleTypes,
                  responseScaleMultipliers, responseScaleOffsets,
                  sub_model.response_labels());

  // LINEAR CONSTRAINT SCALING:
  // computed scales account for scaling in CVs x
  // updating constraint coefficient matrices now handled in derived classes
  // NOTE: cannot use offsets since would be an affine scale
  // ScaleOffsets in this case are only for applying to bounds

  // -----------------
  // LINEAR INEQUALITY
  // -----------------
  const StringArray& lin_ineq_scale_strings = scalingOpts.linIneqScaleTypes;
  const RealVector& lin_ineq_scales = scalingOpts.linIneqScales;

  if ( ( lin_ineq_scale_strings.size() != 0 &&
         lin_ineq_scale_strings.size() != 1 && 
         lin_ineq_scale_strings.size() != num_lin_ineq  ) ||
       ( lin_ineq_scales.length() != 0 && lin_ineq_scales.length() != 1 && 
         lin_ineq_scales.length() != num_lin_ineq ) ) {
    Cerr << "Error: linear_inequality_scale specifications must have length 0, "
         << "1, or " << num_lin_ineq << ".\n";
    abort_handler(-1);
  }

  linearIneqScaleOffsets.resize(num_lin_ineq);

  lbs = sub_model.linear_ineq_constraint_lower_bounds();
  ubs = sub_model.linear_ineq_constraint_upper_bounds();
  targets.size(0);

  const RealMatrix& lin_ineq_coeffs
    = sub_model.linear_ineq_constraint_coeffs();
  for (int i=0; i<num_lin_ineq; ++i) {

    // compute A_i*cvScaleOffset for current constraint -- discrete variables
    // aren't scaled so don't contribute
    linearIneqScaleOffsets[i] = 0.0;
    for (int j=0; j<num_cv; ++j)
      linearIneqScaleOffsets[i] += lin_ineq_coeffs(i,j)*cvScaleOffsets[j];
    
    lbs[i] -= linearIneqScaleOffsets[i];
    ubs[i] -= linearIneqScaleOffsets[i];

  }
  compute_scaling(LINEAR, BOUNDS, num_lin_ineq,
                  lbs, ubs, targets, lin_ineq_scale_strings, lin_ineq_scales,
                  linearIneqScaleTypes, linearIneqScaleMultipliers, 
                  tmp_offsets);

  linear_ineq_constraint_lower_bounds(lbs);
  linear_ineq_constraint_upper_bounds(ubs);
  linear_ineq_constraint_coeffs(
                                lin_coeffs_modify_n2s(lin_ineq_coeffs, cvScaleMultipliers, 
                                                      linearIneqScaleMultipliers) );

  if (outputLevel > NORMAL_OUTPUT && num_lin_ineq > 0)
    print_scaling("Linear inequality scales (incl. any variable scaling)",
                  linearIneqScaleTypes, linearIneqScaleMultipliers,
                  linearIneqScaleOffsets, StringArray());

  // ---------------
  // LINEAR EQUALITY
  // ---------------
  const StringArray& lin_eq_scale_strings = scalingOpts.linEqScaleTypes;
  const RealVector& lin_eq_scales = scalingOpts.linEqScales;

  if ( ( lin_eq_scale_strings.size() != 0 && lin_eq_scale_strings.size() != 1 &&
         lin_eq_scale_strings.size() != num_lin_eq ) ||
       ( lin_eq_scales.length() != 0 && lin_eq_scales.length() != 1 && 
         lin_eq_scales.length() != num_lin_eq ) ) {
    Cerr << "Error: linear_equality_scale specifications must have length 0, "
         << "1, or " << num_lin_eq << ".\n";
    abort_handler(-1);
  }

  linearEqScaleOffsets.resize(num_lin_eq);

  lbs.size(0); ubs.size(0);
  targets = sub_model.linear_eq_constraint_targets();

  const RealMatrix& lin_eq_coeffs
    = sub_model.linear_eq_constraint_coeffs();
  for (int i=0; i<num_lin_eq; ++i) {
    // compute A_i*cvScaleOffset for current constraint
    linearEqScaleOffsets[i] = 0.0;
    for (int j=0; j<num_cv; ++j)
      linearEqScaleOffsets[i] += lin_eq_coeffs(i,j)*cvScaleOffsets[j];
   
    targets[i] -= linearEqScaleOffsets[i];
  }
  compute_scaling(LINEAR, TARGET, num_lin_eq,
                  lbs, ubs, targets, lin_eq_scale_strings, lin_eq_scales,
                  linearEqScaleTypes, linearEqScaleMultipliers, 
                  tmp_offsets);

  linear_eq_constraint_targets(targets);
  linear_eq_constraint_coeffs(
                              lin_coeffs_modify_n2s(lin_eq_coeffs, cvScaleMultipliers, 
                                                    linearEqScaleMultipliers) );

  if (outputLevel > NORMAL_OUTPUT && num_lin_eq > 0)
    print_scaling("Linear equality scales (incl. any variable scaling)",
                  linearEqScaleTypes, linearEqScaleMultipliers,
                  linearEqScaleOffsets, StringArray());

  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;
}


bool ScalingModel::scaling_active(const StringArray& scale_types)
{
  BOOST_FOREACH(const String& sc_type, scale_types) {
    if (sc_type != "none")
      return true;
  }
  return false;  // false if array empty or all types == "none"
}


// compute_scaling will potentially modify lbs, ubs, and targets; will resize
// and set class data referenced by scale_types, scale_mults, and scale_offsets
void ScalingModel::
compute_scaling(int object_type, // type of object being scaled 
                int auto_type,   // option for auto scaling type
                int num_vars,    // length of object being scaled
                RealVector& lbs,     RealVector& ubs,
                RealVector& targets, const StringArray& scale_strings,
                const RealVector& scales, IntArray& scale_types,
                RealVector& scale_mults,  RealVector& scale_offsets)
{
  // temporary arrays
  String tmp_scl_str; 
  Real tmp_bound, tmp_mult, tmp_offset;
  //RealMatrix linear_constraint_coeffs;

  const int num_scale_strings = scale_strings.size();
  const int num_scales        = scales.length();
  
  scale_types.resize(num_vars);
  scale_mults.resize(num_vars);
  scale_offsets.resize(num_vars);

  for (int i=0; i<num_vars; ++i) {

    //set defaults
    scale_types[i]   = SCALE_NONE;
    scale_mults[i]   = 1.0;
    scale_offsets[i] = 0.0;

    // set the string for scale_type, depending on whether user sent
    // 0, 1, or N scale_strings
    tmp_scl_str = "none";
    if (num_scale_strings == 1)
      tmp_scl_str = scale_strings[0];
    else if (num_scale_strings > 1)
      tmp_scl_str = scale_strings[i];

    if (tmp_scl_str != "none") {
      size_t num_lin_cons =
        num_linear_ineq_constraints() + num_linear_eq_constraints();
      if (  object_type == CDV && num_lin_cons > 0 && tmp_scl_str == "log" ) {
        Cerr << "Error: Continuous design variables cannot be logarithmically "
             << "scaled when linear\nconstraints are present.\n";
        abort_handler(-1);
      } 
      else if ( num_scales > 0 ) {
	
        // process scale_values for all types of scaling 
        // indicate that scale values are active, update bounds, poss. negating
        scale_types[i] |= SCALE_VALUE;
        scale_mults[i] = (num_scales == 1) ? scales[0] : scales[i];
        if (std::fabs(scale_mults[i]) < SCALING_MIN_SCALE)
          Cout << "Warning: abs(scale) < " << SCALING_MIN_SCALE
               << " provided; carefully verify results.\n";
        // adjust bounds or targets
        if (!lbs.empty()) {
          // don't scale bounds if the user intended no bound
          if (-BIG_REAL_BOUND < lbs[i])
            lbs[i] /= scale_mults[i];
          if (ubs[i] < BIG_REAL_BOUND)
            ubs[i] /= scale_mults[i];
          if (scale_mults[i] < 0) {
            tmp_bound = lbs[i];
            lbs[i] = ubs[i];
            ubs[i] = tmp_bound;
          }
        } 
        else if (!targets.empty())
          targets[i] /= scale_mults[i];
      }

    } // endif for generic scaling preprocessing

      // At this point bounds/targets are scaled with user-provided values and
      // scale_mults are set to user-provided values.
      // Now auto or log scale as relevant and allowed:
    if ( tmp_scl_str == "auto" && auto_type > DISALLOW ) {
      bool scale_flag = false; // will be true for valid auto-scaling
      if ( auto_type == TARGET ) {
        scale_flag = compute_scale_factor(targets[i], &tmp_mult);
        tmp_offset = 0.0;
      }
      else if (auto_type == BOUNDS )
        scale_flag = compute_scale_factor(lbs[i], ubs[i], 
                                          &tmp_mult, &tmp_offset);
      if (scale_flag) {

        scale_types[i] |= SCALE_VALUE;
        // tmp_offset was calculated based on scaled bounds, so
        // includes the effect of user scale values, so in computing
        // the offset, need to include the effect of any user-supplied
        // characteristic value scaling, then update multipliers
        scale_offsets[i] += tmp_offset*scale_mults[i];
        scale_mults[i] *= tmp_mult;  
      
        // necessary since the initial values may have already been value scaled
        if (auto_type == BOUNDS) {
          // don't scale bounds if the user intended no bound
          if (-BIG_REAL_BOUND < lbs[i])
            lbs[i] = (lbs[i] - tmp_offset)/tmp_mult;
          if (ubs[i] < BIG_REAL_BOUND)
            ubs[i] = (ubs[i] - tmp_offset)/tmp_mult;
        }
        else if (auto_type == TARGET)
          targets[i] /= tmp_mult;

      }
    }
    else if ( tmp_scl_str == "log" ) {

      scale_types[i] |= SCALE_LOG;
      if (auto_type == BOUNDS) {
        if (-BIG_REAL_BOUND < lbs[i]) {
          if ( lbs[i] < SCALING_MIN_LOG )
            Cout << "Warning: scale_type 'log' used without positive lower "
                 << "bound.\n";
          lbs[i] = std::log(lbs[i])/SCALING_LN_LOGBASE;
        }
        if (ubs[i] < BIG_REAL_BOUND) {
          if ( ubs[i] < SCALING_MIN_LOG )
            Cout << "Warning: scale_type 'log' used without positive upper "
                 << "bound.\n";
          ubs[i] = std::log(ubs[i])/SCALING_LN_LOGBASE;
        }
      }
      else if (auto_type == TARGET) {
        targets[i] = std::log(targets[i])/SCALING_LN_LOGBASE;
        if ( targets[i] < SCALING_MIN_LOG )
          Cout << "Warning: scale_type 'log' used without positive target.\n";
      }
    }

  } // end for each variable
}


// automatically compute scaling factor
// bounds case allows for negative multipliers
// returns true if a valid scaling factor was computed
bool ScalingModel::compute_scale_factor(const Real lower_bound,
                                        const Real upper_bound,
                                        Real *multiplier, Real *offset)
{
  /*  Compute scaleMultipliers for each design var, fn, constr, etc.
      1. user-specified scaling was already detected at higher level
      2. check for two-sided bounds
      3. then check for one-sided bounds
      4. else resort to no scaling

      Auto-scaling is to [0,1] (affine scaling) in two sided case 
      or value of bound in one-sided case
  */

  bool lb_flag = false;
  bool ub_flag = false;

  if (-BIG_REAL_BOUND < lower_bound)
    lb_flag = true;
  if (upper_bound < BIG_REAL_BOUND)
    ub_flag = true;

  // process two-sided, then single-sided bounds
  if ( lb_flag && ub_flag ) {
    *multiplier = upper_bound - lower_bound;
    *offset = lower_bound;
  } 
  else if (lb_flag) {
    *multiplier = lower_bound;
    *offset = 0.0;
  } 
  else if (ub_flag) {
    *multiplier = upper_bound;
    *offset = 0.0;
  } 
  else {
    Cout << "Warning: abs(bounds) > BIG_REAL_BOUND. Not auto-scaling "
         << "component." << std::endl;
    *multiplier = 1.0;
    *offset = 0.0;
    return(false);
  }

  if (std::fabs(*multiplier) < SCALING_MIN_SCALE) {
    *multiplier = (*multiplier >= 0.0) ? SCALING_MIN_SCALE :
      -(SCALING_MIN_SCALE); 
    Cout << "Warning: in auto-scaling abs(computed scale) < " 
         << SCALING_MIN_SCALE << "; resetting scale = " 
         << *multiplier << ".\n";
  }

  return(true);
}


// automatically compute scaling factor
// target case allows for negative multipliers
// returns true if a valid scaling factor was computed
bool ScalingModel::compute_scale_factor(const Real target, Real *multiplier)
{
  if ( std::fabs(target) < BIG_REAL_BOUND ) 
    *multiplier = target;
  else {
    Cout << "Automatic Scaling Warning: abs(target) > BIG_REAL_BOUND. "
         << "Not scaling this component." << std::endl;
    *multiplier = 1.0;
    return(false);
  }

  if (std::fabs(*multiplier) < SCALING_MIN_SCALE) {
    *multiplier = (*multiplier >= 0.0) ? SCALING_MIN_SCALE :
      -(SCALING_MIN_SCALE); 
    Cout << "Warning: in auto-scaling abs(computed scale) < " 
         << SCALING_MIN_SCALE << "; resetting scale = " 
         << *multiplier << ".\n";
  }

  return(true);
}


/** compute scaled linear constraint matrix given design variable 
    multipliers and linear scaling multipliers.  Only scales components 
    corresponding to continuous variables so for src_coeffs of size MxN, 
    lin_multipliers.size() <= M, cv_multipliers.size() <= N */
RealMatrix ScalingModel::
lin_coeffs_modify_n2s(const RealMatrix& src_coeffs,
                      const RealVector& cv_multipliers,
                      const RealVector& lin_multipliers) const
{
  RealMatrix dest_coeffs(src_coeffs);
  for (int i=0; i<lin_multipliers.length(); ++i)
    for (int j=0; j<cv_multipliers.length(); ++j)
      dest_coeffs(i,j) *= cv_multipliers[j] / lin_multipliers[i];

  return(dest_coeffs);
}


void ScalingModel::print_scaling(const String& info, const IntArray& scale_types,
                                 const RealVector& scale_mults,
                                 const RealVector& scale_offsets,
                                 const StringArray& labels)
{
  // labels will be empty for linear constraints
  Cout << "\n" << info << ":\n";
  Cout << "scale type " << std::setw(write_precision+7) << "multiplier" << " "
       << std::setw(write_precision+7) << "offset"
       << (labels.empty() ? " constraint number" : " label") << std::endl; 
  for (size_t i=0; i<scale_types.size(); ++i) {
    switch (scale_types[i]) {
    case SCALE_NONE: 
      Cout << "none       ";
      break;
    case SCALE_VALUE: 
      Cout << "value      ";
      break;
    case SCALE_LOG:
      Cout << "log        ";
      break;
    case (SCALE_VALUE | SCALE_LOG): 
      Cout << "value+log  ";
      break;
    }
    Cout << std::setw(write_precision+7) << scale_mults[i]   << " " 
         << std::setw(write_precision+7) << scale_offsets[i] << " ";
    if (labels.empty())
      Cout << i << std::endl;
    else
      Cout << labels[i] << std::endl;
  }
}


/** Variables map from iterator/scaled space to user/native space
    using a RecastModel. */
void ScalingModel::
variables_scaler(const Variables& scaled_vars, Variables& native_vars)
{
  if (scaleModelInstance->outputLevel > NORMAL_OUTPUT) {
    Cout << "\n----------------------------------";
    Cout << "\nPre-processing Function Evaluation";
    Cout << "\n----------------------------------";
    Cout << "\nVariables before unscaling transformation:\n";
    write_data(Cout, scaled_vars.continuous_variables(),
               scaled_vars.continuous_variable_labels());
    Cout << std::endl;
  }
  native_vars.continuous_variables
    (scaleModelInstance->modify_s2n(scaled_vars.continuous_variables(), 
                                    scaleModelInstance->cvScaleTypes,
                                    scaleModelInstance->cvScaleMultipliers, 
                                    scaleModelInstance->cvScaleOffsets));
}

void ScalingModel::
variables_unscaler(const Variables& native_vars, Variables& scaled_vars)
{
  scaled_vars.continuous_variables
    (scaleModelInstance->modify_n2s(native_vars.continuous_variables(),
                                    scaleModelInstance->cvScaleTypes,
                                    scaleModelInstance->cvScaleMultipliers,
                                    scaleModelInstance->cvScaleOffsets));
}


void ScalingModel::
primary_resp_scaler(const Variables& native_vars, const Variables& scaled_vars,
                    const Response& native_response, Response& iterator_response)
{
  // scaling is always applied on a model with user's original size,
  // so can query this object or the underlying model for sizes

  // need to scale if primary responses are scaled or (variables are
  // scaled and grad or hess requested)
  size_t start_offset = 0;
  size_t num_responses = scaleModelInstance->num_primary_fns();
  bool scale_transform_needed = 
    scaleModelInstance->primaryRespScaleFlag ||
    scaleModelInstance->need_resp_trans_byvars
    (native_response.active_set_request_vector(), start_offset, num_responses);

  if (scale_transform_needed) {
    if (scaleModelInstance->outputLevel > NORMAL_OUTPUT) {
      Cout << "\n--------------------------------------------";
      Cout << "\nPost-processing Function Evaluation: Primary";
      Cout << "\n--------------------------------------------" << std::endl; 
    }
    scaleModelInstance->
      response_modify_n2s(native_vars, native_response,
                          iterator_response, start_offset, num_responses);

  }
  else
    // could reach this if variables are scaled and only functions are requested
    iterator_response.update_partial(start_offset, num_responses,
                                     native_response, start_offset);
}


/** Constraint function map from user/native space to iterator/scaled/combined
    space using a RecastModel. */
void ScalingModel::
secondary_resp_scaler(const Variables& native_vars,
                      const Variables& scaled_vars,
                      const Response& native_response,
                      Response& iterator_response)
{
  // scaling is always applied on a model with user's original size,
  // so can query this object or the underlying model for sizes

  // need to scale if secondary responses are scaled or (variables are
  // scaled and grad or hess requested)
  size_t start_offset = scaleModelInstance->num_primary_fns();
  size_t num_nln_cons = scaleModelInstance->num_nonlinear_ineq_constraints() +
    scaleModelInstance->num_nonlinear_eq_constraints();
  bool scale_transform_needed = 
    scaleModelInstance->secondaryRespScaleFlag ||
    scaleModelInstance->need_resp_trans_byvars
    (native_response.active_set_request_vector(), start_offset, num_nln_cons);

  if (scale_transform_needed) {
    if (scaleModelInstance->outputLevel > NORMAL_OUTPUT) {
      Cout << "\n----------------------------------------------";
      Cout << "\nPost-processing Function Evaluation: Secondary";
      Cout << "\n----------------------------------------------" << std::endl; 
    }
    scaleModelInstance->
      response_modify_n2s(native_vars, native_response,
                          iterator_response, start_offset, num_nln_cons);
  }
  else
    // could reach this if variables are scaled and only functions are requested
    iterator_response.update_partial(start_offset, num_nln_cons,
                                     native_response, start_offset);
}


/** Determine if variable transformations present and derivatives
    requested, which implies a response transformation is necessay */ 
bool ScalingModel::
need_resp_trans_byvars(const ShortArray& asv, int start_index, 
                       int num_resp) const
{
  if (varsScaleFlag)
    for (size_t i=start_index; i<start_index+num_resp; ++i)
      if (asv[i] & 2 || asv[i] & 4)
        return(true);
  
  return(false);
}

/** general RealVector mapping from native to scaled variables; loosely,
    in greatest generality:
    scaled_var = log( (native_var - offset) / multiplier ) */
RealVector ScalingModel::
modify_n2s(const RealVector& native_vars, const IntArray& scale_types,
           const RealVector& multipliers, const RealVector& offsets) const
{
  RealVector scaled_vars(native_vars.length(), false);
  for (int i=0; i<native_vars.length(); ++i) {

    if (scale_types[i] & SCALE_VALUE)
      scaled_vars[i] = (native_vars[i] - offsets[i]) / multipliers[i];
    else 
      scaled_vars[i] = native_vars[i];

    if (scale_types[i] & SCALE_LOG)
      scaled_vars[i] = std::log(scaled_vars[i])/SCALING_LN_LOGBASE;

  }

  return(scaled_vars);
}

/** general RealVector mapping from scaled to native variables and/or vals;
    loosely, in greatest generality:
    scaled_var = (LOG_BASE^scaled_var) * multiplier + offset */
RealVector ScalingModel::
modify_s2n(const RealVector& scaled_vars, const IntArray& scale_types,
           const RealVector& multipliers, const RealVector& offsets) const
{
  RealVector native_vars(scaled_vars.length(), false);
  for (RealVector::ordinalType i=0; i<scaled_vars.length(); ++i) {

    if (scale_types[i] & SCALE_LOG)
      native_vars[i] = std::pow(SCALING_LOGBASE, scaled_vars[i]);
    else 
      native_vars[i] = scaled_vars[i];

    if (scale_types[i] & SCALE_VALUE)
      native_vars[i] = native_vars[i]*multipliers[i] + offsets[i];
    
  }

  return(native_vars);
}


/** Scaling response mapping: modifies response from a model
    (user/native) for use in iterators (scaled). Maps num_responses
    starting at response_offset */
void ScalingModel::response_modify_n2s(const Variables& native_vars,
                                       const Response& native_response,
                                       Response& recast_response,
                                       int start_offset,
                                       int num_responses) const
{
  int i, j, k;
  int end_offset = start_offset + num_responses;
  SizetMultiArray var_ids;
  RealVector cdv;

  // unroll the unscaled (native/user-space) response
  const ActiveSet&  set = native_response.active_set();
  const ShortArray& asv = set.request_vector();
  const SizetArray& dvv = set.derivative_vector();
  size_t num_deriv_vars = dvv.size(); 
 
  if (dvv == native_vars.continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.cv()]);
    var_ids = native_vars.continuous_variable_ids();
    cdv = native_vars.continuous_variables(); // view OK
  }
  else if (dvv == native_vars.inactive_continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.icv()]);
    var_ids = native_vars.inactive_continuous_variable_ids();
    cdv = native_vars.inactive_continuous_variables(); // view OK
  }
  else { // general derivatives
    var_ids.resize(boost::extents[native_vars.acv()]);
    var_ids = native_vars.all_continuous_variable_ids();
    cdv = native_vars.all_continuous_variables(); // view OK
  }

  if (outputLevel > NORMAL_OUTPUT)
    if (start_offset < scaleModelInstance->num_primary_fns())
      Cout << "Primary response after scaling transformation:\n";
    else
      Cout << "Secondary response after scaling transformation:\n";

  // scale functions and constraints
  // assumes Multipliers and Offsets are 1 and 0 when not in use
  // there's a tradeoff here between flops and logic simplicity
  // (responseScaleOffsets may be nonzero for constraints)
  // iterate over components of ASV-requested functions and scale when necessary
  Real recast_val;
  const RealVector&  native_vals   = native_response.function_values();
  const StringArray& recast_labels = recast_response.function_labels();
  for (i = start_offset; i < end_offset; ++i)
    if (asv[i] & 1) {
      // SCALE_LOG case here includes case of SCALE_LOG && SCALE_VALUE
      if (responseScaleTypes[i] & SCALE_LOG)
        recast_val = std::log( (native_vals[i] - responseScaleOffsets[i]) / 
                               responseScaleMultipliers[i] )/SCALING_LN_LOGBASE; 
      else if (responseScaleTypes[i] & SCALE_VALUE)
        recast_val = (native_vals[i] - responseScaleOffsets[i]) / 
          responseScaleMultipliers[i]; 
      else
        recast_val = native_vals[i];
      recast_response.function_value(recast_val, i);
      if (outputLevel > NORMAL_OUTPUT)
        Cout << "                     " << std::setw(write_precision+7) 
             << recast_val << ' ' << recast_labels[i] << '\n';
    }

  // scale gradients
  const RealMatrix& native_grads = native_response.function_gradients();
  for (i = start_offset; i < end_offset; ++i)
    if (asv[i] & 2) {

      Real fn_divisor;
      if (responseScaleTypes[i] & SCALE_LOG)
        fn_divisor = (native_vals[i] - responseScaleOffsets[i]) *
          SCALING_LN_LOGBASE;
      else if (responseScaleTypes[i] & SCALE_VALUE)
        fn_divisor = responseScaleMultipliers[i];
      else
        fn_divisor = 1.;

      RealVector recast_grad
        = recast_response.function_gradient_view(i);
      copy_data(native_grads[i], (int)num_deriv_vars, recast_grad);
      for (j=0; j<num_deriv_vars; ++j) {
        size_t xj_index = find_index(var_ids, dvv[j]);

        // first multiply by d(f_scaled)/d(f) based on scaling type
        recast_grad[xj_index] /= fn_divisor;

        // now multiply by d(x)/d(x_scaled)
        if (cvScaleTypes[xj_index] & SCALE_LOG)
          recast_grad[xj_index] *= 
            (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;
        else if (cvScaleTypes[xj_index] & SCALE_VALUE)
          recast_grad[xj_index] *= cvScaleMultipliers[xj_index];
      }
      if (outputLevel > NORMAL_OUTPUT) {
        write_col_vector_trans(Cout, i, recast_response.function_gradients(),
                               true, true, false);
        Cout << recast_labels[i] << " gradient\n";
      }
    }
  
  // scale hessians
  const RealSymMatrixArray& native_hessians
    = native_response.function_hessians();
  for (i = start_offset; i < end_offset; ++i)
    if (asv[i] & 4) {
      RealSymMatrix recast_hess
        = recast_response.function_hessian_view(i);
      recast_hess.assign(native_hessians[i]);

      Real offset_fn = 1.;
      if (responseScaleTypes[i] & SCALE_LOG)
        offset_fn = native_vals[i] - responseScaleOffsets[i];
      for (j=0; j<num_deriv_vars; ++j) {
        size_t xj_index = find_index(var_ids, dvv[j]);
        for (k=0; k<=j; ++k) {
          size_t xk_index = find_index(var_ids, dvv[k]);

          // first multiply by d(f_scaled)/d(f) based on scaling type
          if (responseScaleTypes[i] & SCALE_LOG) {

            recast_hess(xj_index,xk_index) -= 
              native_grads(xj_index,i)*native_grads(xk_index,i) / offset_fn;

            recast_hess(xj_index,xk_index) /= offset_fn*SCALING_LN_LOGBASE;

          }
          else if (responseScaleTypes[i] & SCALE_VALUE)
            recast_hess(xj_index,xk_index) /= responseScaleMultipliers[i];

          // now multiply by d(x)/d(x_scaled) for j,k
          if (cvScaleTypes[xj_index] & SCALE_LOG)
            recast_hess(xj_index,xk_index) *= 
              (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;
          else if (cvScaleTypes[xj_index] & SCALE_VALUE)
            recast_hess(xj_index,xk_index) *= cvScaleMultipliers[xj_index];

          if (cvScaleTypes[xk_index] & SCALE_LOG)
            recast_hess(xj_index,xk_index) *= 
              (cdv[xk_index] - cvScaleOffsets[xk_index]) * SCALING_LN_LOGBASE;
          else if (cvScaleTypes[xk_index] & SCALE_VALUE)
            recast_hess(xj_index,xk_index) *= cvScaleMultipliers[xk_index];

          // need gradient term only for diagonal entries
          if (xj_index == xk_index && cvScaleTypes[xj_index] & SCALE_LOG)
            if (responseScaleTypes[i] & SCALE_LOG)
              recast_hess(xj_index,xk_index) += native_grads(xj_index,i)*
                (cdv[xj_index] - cvScaleOffsets[xj_index]) *
                SCALING_LN_LOGBASE / offset_fn;
            else
              recast_hess(xj_index,xk_index) += native_grads(xj_index,i)*
                (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE
                * SCALING_LN_LOGBASE / responseScaleMultipliers[i];

        }
      }
      if (outputLevel > NORMAL_OUTPUT) {
        write_data(Cout, recast_hess, true, true, false);
        Cout << recast_labels[i] << " Hessian\n";
      }
    }
  
  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;
}

/** Unscaling response mapping: modifies response from scaled
    (iterator) to native (user) space.  Maps num_responses starting at
    response_offset */
void ScalingModel::response_modify_s2n(const Variables& native_vars,
                                       const Response& scaled_response,
                                       Response& native_response,
                                       int start_offset,
                                       int num_responses) const
{
  using std::pow;

  int i, j, k;
  int end_offset = start_offset + num_responses;
  SizetMultiArray var_ids;
  RealVector cdv;

  // unroll the unscaled (native/user-space) response
  const ActiveSet&  set = scaled_response.active_set();
  const ShortArray& asv = set.request_vector();
  const SizetArray& dvv = set.derivative_vector();
  size_t num_deriv_vars = dvv.size(); 
 
  if (dvv == native_vars.continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.cv()]);
    var_ids = native_vars.continuous_variable_ids();
    cdv = native_vars.continuous_variables(); // view OK
  }
  else if (dvv == native_vars.inactive_continuous_variable_ids()) {
    var_ids.resize(boost::extents[native_vars.icv()]);
    var_ids = native_vars.inactive_continuous_variable_ids();
    cdv = native_vars.inactive_continuous_variables(); // view OK
  }
  else {    // general derivatives
    var_ids.resize(boost::extents[native_vars.acv()]);
    var_ids = native_vars.all_continuous_variable_ids();
    cdv = native_vars.all_continuous_variables(); // view OK
  }

  if (outputLevel > NORMAL_OUTPUT)
    if (start_offset < scaleModelInstance->num_primary_fns())
      Cout << "Primary response after unscaling transformation:\n";
    else
      Cout << "Secondary response after unscaling transformation:\n";

  // scale functions and constraints
  // assumes Multipliers and Offsets are 1 and 0 when not in use
  // there's a tradeoff here between flops and logic simplicity
  // (responseScaleOffsets may be nonzero for constraints)
  // iterate over components of ASV-requested functions and scale when necessary
  Real native_val;
  const RealVector&  scaled_vals   = scaled_response.function_values();
  const StringArray& native_labels = native_response.function_labels();
  for (i = start_offset; i < end_offset; ++i)
    if (asv[i] & 1) {
      // SCALE_LOG case here includes case of SCALE_LOG && SCALE_VALUE
      if (responseScaleTypes[i] & SCALE_LOG)
        native_val = pow(SCALING_LOGBASE, scaled_vals[i]) *
          responseScaleMultipliers[i] + responseScaleOffsets[i];
 
      else if (responseScaleTypes[i] & SCALE_VALUE)
        native_val = scaled_vals[i]*responseScaleMultipliers[i] + 
          responseScaleOffsets[i];
      else
        native_val = scaled_vals[i];
      native_response.function_value(native_val, i);
      if (outputLevel > NORMAL_OUTPUT)
        Cout << "                     " << std::setw(write_precision+7) 
             << native_val << ' ' << native_labels[i] << '\n';
    }

  // scale gradients
  Real df_dfscl;
  const RealMatrix& scaled_grads = scaled_response.function_gradients();
  for (i = start_offset; i < end_offset; ++i)
    if (asv[i] & 2) {

      if (responseScaleTypes[i] & SCALE_LOG)
        df_dfscl = pow(SCALING_LOGBASE, scaled_vals[i]) * SCALING_LN_LOGBASE *
          responseScaleMultipliers[i];	 
      else if (responseScaleTypes[i] & SCALE_VALUE)
        df_dfscl = responseScaleMultipliers[i];
      else
        df_dfscl = 1.;

      RealVector native_grad
        = native_response.function_gradient_view(i);
      copy_data(scaled_grads[i], (int)num_deriv_vars, native_grad);
      for (j=0; j<num_deriv_vars; ++j) {
        size_t xj_index = find_index(var_ids, dvv[j]);

        // first multiply by d(f)/d(f_scaled) based on scaling type
        native_grad[xj_index] *= df_dfscl; 

        // now multiply by d(x_scaled)/d(x)
        if (cvScaleTypes[xj_index] & SCALE_LOG)
          native_grad[xj_index] /= (cdv[xj_index] - cvScaleOffsets[xj_index]) *
            SCALING_LN_LOGBASE;
        else if (cvScaleTypes[xj_index] & SCALE_VALUE)
          native_grad[xj_index] /= cvScaleMultipliers[xj_index];
      }
      if (outputLevel > NORMAL_OUTPUT) {
        write_col_vector_trans(Cout, i, native_response.function_gradients(),
                               true, true, false);
        Cout << native_labels[i] << " gradient\n";
      }
    }
  
  // scale hessians
  const RealSymMatrixArray& scaled_hessians
    = scaled_response.function_hessians();
  for (i = start_offset; i < end_offset; ++i)
    if (asv[i] & 4) {
 
      if (responseScaleTypes[i] & SCALE_LOG)
        df_dfscl = pow(SCALING_LOGBASE, scaled_vals[i]) * SCALING_LN_LOGBASE *
          responseScaleMultipliers[i];
      else if (responseScaleTypes[i] & SCALE_VALUE)
        df_dfscl = responseScaleMultipliers[i];
      else
        df_dfscl = 1.;

      RealSymMatrix native_hess
        = native_response.function_hessian_view(i);
      native_hess.assign(scaled_hessians[i]);
      for (j=0; j<num_deriv_vars; ++j) {
        size_t xj_index = find_index(var_ids, dvv[j]);
        for (k=0; k<=j; ++k) {
          size_t xk_index = find_index(var_ids, dvv[k]);
	  
          // first multiply by d(f_scaled)/d(f) based on scaling type

          if (responseScaleTypes[i] & SCALE_LOG) {

            native_hess(xj_index,xk_index) += scaled_grads(xj_index,i) *
              scaled_grads(xk_index,i) * SCALING_LN_LOGBASE; 

            native_hess(xj_index,xk_index) *= df_dfscl; 

          }
          else if (responseScaleTypes[i] & SCALE_VALUE)
            native_hess(xj_index,xk_index) *= df_dfscl;

          // now multiply by d(x_scaled)/d(x) for j,k
          if (cvScaleTypes[xj_index] & SCALE_LOG)
            native_hess(xj_index,xk_index) /= 
              (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;
          else if (cvScaleTypes[xj_index] & SCALE_VALUE)
            native_hess(xj_index,xk_index) /= cvScaleMultipliers[xj_index];

          if (cvScaleTypes[xk_index] & SCALE_LOG)
            native_hess(xj_index,xk_index) /= 
              (cdv[xk_index] - cvScaleOffsets[xk_index]) * SCALING_LN_LOGBASE;
          else if (cvScaleTypes[xk_index] & SCALE_VALUE)
            native_hess(xj_index,xk_index) /= cvScaleMultipliers[xk_index];

          // need gradient term only for diagonal entries
          if (xj_index == xk_index && cvScaleTypes[xj_index] & SCALE_LOG)
            native_hess(xj_index,xk_index) -=
              df_dfscl * scaled_grads(xj_index,i) /
              (cdv[xj_index] - cvScaleOffsets[xj_index]) * 
              (cdv[xj_index] - cvScaleOffsets[xj_index]) * SCALING_LN_LOGBASE;

        }
      }
      if (outputLevel > NORMAL_OUTPUT) {
        write_data(Cout, native_hess, true, true, false);
        Cout << native_labels[i] << " Hessian\n";
      }
    }
  
  if (outputLevel > NORMAL_OUTPUT)
    Cout << std::endl;
}




}  // namespace Dakota
