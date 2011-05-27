/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       RecastModel
//- Description: Implementation code for the RecastModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "system_defs.h"
#include "RecastModel.H"

static const char rcsId[]="@(#) $Id: RecastModel.C 7029 2010-10-22 00:17:02Z mseldre $";


using namespace std;

namespace Dakota {

// define special values for componentParallelMode
#define SUB_MODEL 2
//#define DEBUG


RecastModel::
RecastModel(const Model& sub_model, const Sizet2DArray& vars_map_indices,
	    bool nonlinear_vars_mapping,
	    void (*variables_map)      (const Variables& recast_vars,
					Variables& sub_model_vars),
	    void (*set_map)            (const Variables& recast_vars,
					const ActiveSet& recast_set,
					ActiveSet& sub_model_set),
	    const Sizet2DArray& primary_resp_map_indices,
	    const Sizet2DArray& secondary_resp_map_indices,
	    size_t recast_secondary_offset,
	    const BoolDequeArray& nonlinear_resp_mapping,
	    void (*primary_resp_map)   (const Variables& sub_model_vars,
					const Variables& recast_vars,
					const Response& sub_model_response,
					Response& recast_response),
	    void (*secondary_resp_map) (const Variables& sub_model_vars,
					const Variables& recast_vars,
					const Response& sub_model_response,
					Response& recast_response)):
  Model(RecastBaseConstructor(), sub_model.problem_description_db(),
	sub_model.parallel_library()),
  subModel(sub_model), varsMapIndices(vars_map_indices),
  nonlinearVarsMapping(nonlinear_vars_mapping), variablesMapping(variables_map),
  setMapping(set_map), primaryRespMapIndices(primary_resp_map_indices),
  secondaryRespMapIndices(secondary_resp_map_indices),
  nonlinearRespMapping(nonlinear_resp_mapping),
  primaryRespMapping(primary_resp_map), secondaryRespMapping(secondary_resp_map)
{
  componentParallelMode = SUB_MODEL;
  outputLevel     = sub_model.output_level();

  gradType        = sub_model.gradient_type();
  methodSrc       = sub_model.method_source();
  ignoreBounds    = sub_model.ignore_bounds();
  centralHess	  = sub_model.central_hess();
  intervalType    = sub_model.interval_type();
  fdGradSS        = sub_model.fd_gradient_step_size();
  gradIdAnalytic  = sub_model.gradient_id_analytic();
  gradIdNumerical = sub_model.gradient_id_numerical();

  hessType        = sub_model.hessian_type();
  quasiHessType   = sub_model.quasi_hessian_type();
  fdHessByFnSS    = sub_model.fd_hessian_by_fn_step_size();
  fdHessByGradSS  = sub_model.fd_hessian_by_grad_step_size();
  hessIdAnalytic  = sub_model.hessian_id_analytic();
  hessIdNumerical = sub_model.hessian_id_numerical();
  hessIdQuasi     = sub_model.hessian_id_quasi();
 
  respMapping = (primaryRespMapping || secondaryRespMapping) ? true : false;
  size_t num_recast_primary_fns = primaryRespMapIndices.size(),
    num_recast_secondary_fns = secondaryRespMapIndices.size(),
    num_recast_fns = num_recast_primary_fns + num_recast_secondary_fns;
  if (nonlinearRespMapping.size() != num_recast_fns) {
    Cerr << "Error: size mismatch in response mapping configuration." << endl;
    abort_handler(-1);
  }

  // recasting of variables
  const Variables& sub_model_vars = subModel.current_variables();
  currentVariables = sub_model_vars.copy();
  // all current variables mappings are one-to-one -> no reshape needed
  //if (variablesMapping) {
    // varsMapIndices maps from currentVariables to subModel variables using
    // varsMapIndices[subModel vars id][currentVariables id] indexing
    //numDerivVars = ...; // unroll varsMapIndices arrays and count unique
    //if (currentVariables.cv() != numDerivVars)
    //  currentVariables.reshape(vc_totals);
  //}
  //else
    numDerivVars = currentVariables.cv(); 

  // recasting of response
  const Response& sub_model_resp = subModel.current_response();
  currentResponse = sub_model_resp.copy();
  if (respMapping) {
    numFns = num_recast_fns;
    if (subModel.num_functions() != numFns)
      currentResponse.reshape(numFns, numDerivVars,
			      !sub_model_resp.function_gradients().empty(),
			      !sub_model_resp.function_hessians().empty());
  }
  else
    numFns = currentResponse.num_functions();

  // recasting of constraints
  const Constraints& sub_model_cons = subModel.user_defined_constraints();
  userDefinedConstraints = sub_model_cons.copy();
  if (secondaryRespMapping) {
    // the recast_secondary_offset cannot in general be inferred from the
    // contributing fns in secondaryRespMapIndices (recast constraints may be
    // defined, e.g., with no contributing fns), and must therefore be passed.
    const size_t& num_recast_nln_ineq = recast_secondary_offset;
    size_t num_recast_nln_eq = num_recast_secondary_fns - num_recast_nln_ineq;
    if ( num_recast_nln_ineq != sub_model_cons.num_nonlinear_ineq_constraints()
      || num_recast_nln_eq   != sub_model_cons.num_nonlinear_eq_constraints() )
      userDefinedConstraints.reshape(num_recast_nln_ineq, num_recast_nln_eq,
        sub_model_cons.num_linear_ineq_constraints(),
        sub_model_cons.num_linear_eq_constraints());
  }
  // could be separate reshapes or could be combined
  //if (variablesMapping && currentVariables.cv() != numDerivVars)
  //  userDefinedConstraints.reshape(vc_totals);
}


/** This alternate constructor defers initialization of the function
    pointers until a separate call to initialize(), and accepts the
    minimum information needed to construct currentVariables,
    currentResponse, and userDefinedConstraints.  The resulting model
    is sufficiently complete for passing to an Iterator. */
RecastModel::
RecastModel(const Model& sub_model, //size_t num_deriv_vars,
	    size_t num_recast_primary_fns, size_t num_recast_secondary_fns,
	    size_t recast_secondary_offset):
  Model(RecastBaseConstructor(), sub_model.problem_description_db(),
	sub_model.parallel_library()),
  subModel(sub_model), nonlinearVarsMapping(false), respMapping(false),
  variablesMapping(NULL), setMapping(NULL), primaryRespMapping(NULL),
  secondaryRespMapping(NULL)
{
  componentParallelMode = SUB_MODEL;
  outputLevel     = sub_model.output_level();

  gradType        = sub_model.gradient_type();
  methodSrc       = sub_model.method_source();
  ignoreBounds    = sub_model.ignore_bounds();
  centralHess	  = sub_model.central_hess();
  intervalType    = sub_model.interval_type();
  fdGradSS        = sub_model.fd_gradient_step_size();
  gradIdAnalytic  = sub_model.gradient_id_analytic();
  gradIdNumerical = sub_model.gradient_id_numerical();

  hessType        = sub_model.hessian_type();
  quasiHessType   = sub_model.quasi_hessian_type();
  fdHessByFnSS    = sub_model.fd_hessian_by_fn_step_size();
  fdHessByGradSS  = sub_model.fd_hessian_by_grad_step_size();
  hessIdAnalytic  = sub_model.hessian_id_analytic();
  hessIdNumerical = sub_model.hessian_id_numerical();
  hessIdQuasi     = sub_model.hessian_id_quasi();

  // recasting of variables
  const Variables& sub_model_vars = subModel.current_variables();
  currentVariables = sub_model_vars.copy();
  // all current variables mappings are one-to-one -> no reshape needed
  //if (variablesMapping) {
    // varsMapIndices maps from currentVariables to subModel variables using
    // varsMapIndices[subModel vars id][currentVariables id] indexing
    //numDerivVars = ...; // unroll varsMapIndices arrays and count unique
    //if (currentVariables.cv() != numDerivVars)
    //  currentVariables.reshape(vc_totals);
  //}
  //else
    numDerivVars = currentVariables.cv(); 

  // recasting of response
  const Response& sub_model_resp = subModel.current_response();
  currentResponse = sub_model_resp.copy();
  numFns = num_recast_primary_fns + num_recast_secondary_fns;
  if (subModel.num_functions() != numFns)
    currentResponse.reshape(numFns, numDerivVars,
			    !sub_model_resp.function_gradients().empty(),
			    !sub_model_resp.function_hessians().empty());

  // recasting of constraints
  const Constraints& sub_model_cons = subModel.user_defined_constraints();
  userDefinedConstraints = sub_model_cons.copy();
  // the recast_secondary_offset cannot in general be inferred from the
  // contributing fns in secondaryRespMapIndices (recast constraints may be
  // defined, e.g., with no contributing fns), and must therefore be passed.
  const size_t& num_recast_nln_ineq = recast_secondary_offset;
  size_t num_recast_nln_eq = num_recast_secondary_fns - num_recast_nln_ineq;
  if ( num_recast_nln_ineq != sub_model_cons.num_nonlinear_ineq_constraints()
    || num_recast_nln_eq   != sub_model_cons.num_nonlinear_eq_constraints() )
    userDefinedConstraints.reshape(num_recast_nln_ineq, num_recast_nln_eq,
      sub_model_cons.num_linear_ineq_constraints(),
      sub_model_cons.num_linear_eq_constraints());

  // could be separate reshapes or could be combined
  //if (variablesMapping && currentVariables.cv() != numDerivVars)
  //  userDefinedConstraints.reshape(vc_totals);
}


/** This function is used for late initialization of the recasting
    functions.  It is used in concert with the alternate constructor. */
void RecastModel::
initialize(const Sizet2DArray& vars_map_indices,
	   bool nonlinear_vars_mapping,
	   void (*variables_map)      (const Variables& recast_vars,
				       Variables& sub_model_vars),
	   void (*set_map)            (const Variables& recast_vars,
				       const ActiveSet& recast_set,
				       ActiveSet& sub_model_set),
	   const Sizet2DArray& primary_resp_map_indices,
	   const Sizet2DArray& secondary_resp_map_indices,
	   const BoolDequeArray& nonlinear_resp_mapping,
	   void (*primary_resp_map)   (const Variables& sub_model_vars,
				       const Variables& recast_vars,
				       const Response& sub_model_response,
				       Response& recast_response),
	   void (*secondary_resp_map) (const Variables& sub_model_vars,
				       const Variables& recast_vars,
				       const Response& sub_model_response,
				       Response& recast_response))
{
  varsMapIndices          = vars_map_indices;
  nonlinearVarsMapping    = nonlinear_vars_mapping;
  variablesMapping        = variables_map;
  setMapping              = set_map;
  primaryRespMapIndices   = primary_resp_map_indices;
  secondaryRespMapIndices = secondary_resp_map_indices;
  nonlinearRespMapping    = nonlinear_resp_mapping;
  primaryRespMapping      = primary_resp_map;
  secondaryRespMapping    = secondary_resp_map;

  respMapping = (primaryRespMapping || secondaryRespMapping) ? true : false;
  if (nonlinearRespMapping.size() != primaryRespMapIndices.size() +
      secondaryRespMapIndices.size()) {
    Cerr << "Error: size mismatch in response mapping configuration." << endl;
    abort_handler(-1);
  }
}


void RecastModel::derived_compute_response(const ActiveSet& set)
{
  // The RecastModel is evaluated by an Iterator for a recast problem
  // formulation.  Therefore, the currentVariables, incoming active
  // set, and output currentResponse all correspond to the recast
  // inputs/outputs.

  // map from recast variables ("iterator space") into the model variables
  // ("user space") and set these vars within the subModel
  if (variablesMapping) {
    Variables sub_model_vars = subModel.current_variables(); // shared rep
    variablesMapping(currentVariables, sub_model_vars);
    //subModel.active_variables(sub_model_vars); // not really needed
  }
  else
    subModel.active_variables(currentVariables);

  // the incoming set is for the recast problem, which must be converted
  // back to the underlying response set for evaluation by the subModel.
  ActiveSet sub_model_set;
  set_mapping(currentVariables, set, sub_model_set);

  parallelLib.parallel_configuration_iterator(
    subModel.parallel_configuration_iterator());
  // evaluate the subModel in the original fn set definition.  Doing this here 
  // eliminates the need for eval tracking logic within the separate eval fns.
  subModel.compute_response(sub_model_set);

  // recast the subModel response ("user space") into the currentResponse
  // ("iterator space")
  const Variables& sub_model_vars = subModel.current_variables();
  const Response&  sub_model_resp = subModel.current_response();
  currentResponse.active_set(set);
  if (respMapping) {
    if (primaryRespMapping)
      primaryRespMapping(sub_model_vars, currentVariables, sub_model_resp,
        currentResponse);
    else
      currentResponse.update_partial(0, primaryRespMapIndices.size(),
				     sub_model_resp, 0);
    if (secondaryRespMapping)
      secondaryRespMapping(sub_model_vars, currentVariables, sub_model_resp,
	currentResponse);
    else {
      size_t num_recast_secondary_fns = secondaryRespMapIndices.size(),
	num_orig_primary_fns
	  = sub_model_resp.num_functions() - num_recast_secondary_fns;
      currentResponse.update_partial(primaryRespMapIndices.size(),
				     num_recast_secondary_fns, sub_model_resp,
				     num_orig_primary_fns);
    }
  }
  else
    currentResponse.update(sub_model_resp);

#ifdef DEBUG
  Cout << "Recast variables:\n"   << currentVariables
       << "subModel variables:\n" << subModel.current_variables()
       << "subModel response:\n"  << sub_model_resp
       << "Recast response:\n"    << currentResponse;
#endif
}


void RecastModel::derived_asynch_compute_response(const ActiveSet& set)
{
  // set the vars within the subModel
  if (variablesMapping) {
    Variables sub_model_vars = subModel.current_variables(); // shared rep
    variablesMapping(currentVariables, sub_model_vars);
    //subModel.active_variables(sub_model_vars); // not really needed
  }
  else
    subModel.active_variables(currentVariables);

  // the incoming set is for the recast problem, which must be converted
  // back to the underlying response set for evaluation by the subModel.
  ActiveSet sub_model_set;
  set_mapping(currentVariables, set, sub_model_set);

  // evaluate the subModel in the original fn set definition.  Doing this here 
  // eliminates the need for eval tracking logic within the separate eval fns.
  subModel.asynch_compute_response(sub_model_set);

  // bookkeep variables for use in primaryRespMapping/secondaryRespMapping
  if (respMapping) {
    int eval_id = subModel.evaluation_id();
    recastSetMap[eval_id]  = set;
    recastVarsMap[eval_id] = currentVariables.copy();
    if (variablesMapping)
      subModelVarsMap[eval_id] = subModel.current_variables().copy();
  }
}


const IntResponseMap& RecastModel::derived_synchronize()
{
  parallelLib.parallel_configuration_iterator(
    subModel.parallel_configuration_iterator());
  const IntResponseMap& orig_resp_map = subModel.synchronize();
  if (respMapping) {
    recastResponseMap.clear();
    size_t num_recast_primary_fns = primaryRespMapIndices.size(),
      num_recast_secondary_fns  = secondaryRespMapIndices.size(),
      num_orig_primary_fns
      = subModel.num_functions() - num_recast_secondary_fns;
    IntASMIter    rsm_it = recastSetMap.begin();
    IntVarsMIter  rvm_it = recastVarsMap.begin();
    IntVarsMIter smvm_it = (variablesMapping) ? subModelVarsMap.begin()
                                              : recastVarsMap.begin();
    for (IntRespMCIter map_cit = orig_resp_map.begin();
	 map_cit != orig_resp_map.end();
	 ++map_cit, ++rsm_it, ++rvm_it, ++smvm_it) {
      int eval_id = map_cit->first;
      const ActiveSet& recast_set     =  rsm_it->second;
      const Variables& recast_vars    =  rvm_it->second;
      const Variables& sub_model_vars = smvm_it->second;
      const Response&  sub_model_resp = map_cit->second;
      currentResponse.active_set(recast_set);
      if (primaryRespMapping)
	primaryRespMapping(sub_model_vars, recast_vars, sub_model_resp,
			   currentResponse);
      else
	currentResponse.update_partial(0, num_recast_primary_fns,
				       sub_model_resp, 0);
      if (secondaryRespMapping)
	secondaryRespMapping(sub_model_vars, recast_vars, sub_model_resp,
			     currentResponse);
      else
	currentResponse.update_partial(num_recast_primary_fns,
				       num_recast_secondary_fns, sub_model_resp,
				       num_orig_primary_fns);
      recastResponseMap[eval_id] = currentResponse.copy();
    }
    recastSetMap.clear();
    recastVarsMap.clear();
    if (variablesMapping)
      subModelVarsMap.clear();
    return recastResponseMap;
  }
  else
    return orig_resp_map;
}


const IntResponseMap& RecastModel::derived_synchronize_nowait()
{
  parallelLib.parallel_configuration_iterator(
    subModel.parallel_configuration_iterator());
  const IntResponseMap& orig_resp_map = subModel.synchronize_nowait();
  if (respMapping) {
    recastResponseMap.clear();
    size_t num_recast_primary_fns = primaryRespMapIndices.size(),
      num_recast_secondary_fns  = secondaryRespMapIndices.size(),
      num_orig_primary_fns
        = subModel.num_functions() - num_recast_secondary_fns;
    for (IntRespMCIter map_cit = orig_resp_map.begin();
	 map_cit != orig_resp_map.end(); ++map_cit) {
      int eval_id = map_cit->first;
      // IntResponseMap from subModel.synchronize_nowait() must be
      // consistent with subModel.evaluation_id() used above
      const ActiveSet& recast_set  = recastSetMap[eval_id];
      const Variables& recast_vars = recastVarsMap[eval_id];
      const Variables& sub_model_vars = (variablesMapping) ?
	subModelVarsMap[eval_id] : recastVarsMap[eval_id];
      const Response&  sub_model_resp = map_cit->second;
      currentResponse.active_set(recast_set);
      if (primaryRespMapping)
	primaryRespMapping(sub_model_vars, recast_vars, sub_model_resp,
			   currentResponse);
      else
	currentResponse.update_partial(0, num_recast_primary_fns,
				       sub_model_resp, 0);
      if (secondaryRespMapping)
	secondaryRespMapping(sub_model_vars, recast_vars, sub_model_resp,
			     currentResponse);
      else
	currentResponse.update_partial(num_recast_primary_fns,
				       num_recast_secondary_fns, sub_model_resp,
				       num_orig_primary_fns);
      recastResponseMap[eval_id] = currentResponse.copy();
      recastSetMap.erase(eval_id);
      recastVarsMap.erase(eval_id);
      if (variablesMapping)
	subModelVarsMap.erase(eval_id);
    }
    return recastResponseMap;
  }
  else
    return orig_resp_map;
}


void RecastModel::
set_mapping(const Variables& recast_vars, const ActiveSet& recast_set,
	    ActiveSet& sub_model_set)
{
  const size_t& num_recast_primary_fns   = primaryRespMapIndices.size();
  const size_t& num_recast_secondary_fns = secondaryRespMapIndices.size();
  size_t i, j, num_recast_fns
    = num_recast_primary_fns + num_recast_secondary_fns;
  const ShortArray& recast_asv = recast_set.request_vector();
  if (recast_asv.size() != num_recast_fns) {
    Cerr << "Error: inconsistent asv sizing in RecastModel::set_mapping()."
	 << endl;
    abort_handler(-1);
  }
  if (sub_model_set.request_vector().empty()) {
    ShortArray sub_model_asv(subModel.num_functions(), 0);
    sub_model_set.request_vector(sub_model_asv);
  }
  else
    sub_model_set.request_values(0);
  sub_model_set.derivative_vector(recast_set.derivative_vector());//default=copy

  // perform any supplied ASV modifications that augment the standard
  // mapping rules
  if (setMapping)
    setMapping(recast_vars, recast_set, sub_model_set);

  // project each recast_asv request onto the contributing set of functions
  // within the sub_model_asv.  In the case of nonlinear input/output mappings,
  // the recast_asv request is augmented with additional data requirements
  // derived from chain rule differentiation.
  ShortArray sub_model_asv = sub_model_set.request_vector();
  for (i=0; i<num_recast_fns; i++) {
    short asv_val = recast_asv[i];
    // For nonlinear variable mappings, gradient required to transform Hessian.
    // A single nonlinear variable mapping affects all function derivatives.
    if (nonlinearVarsMapping && (asv_val & 4))
      asv_val |= 2;
    // assign the asv_val to each contributing sub-model function
    const SizetArray& recast_fn_contributors = (i<num_recast_primary_fns) ?
      primaryRespMapIndices[i] :
      secondaryRespMapIndices[i-num_recast_primary_fns];
    size_t num_contributors = recast_fn_contributors.size();
    for (j=0; j<num_contributors; j++) {
      short sub_model_asv_val = asv_val;
      // For nonlinear resp mappings, derivatives require all lower order data.
      // The nonlinearity of each function contribution is employed.
      if (nonlinearRespMapping[i][j]) {
	if (asv_val & 4)
	  sub_model_asv_val |= 3;
	else if (asv_val & 2)
	  sub_model_asv_val |= 1;
      }
      sub_model_asv[recast_fn_contributors[j]] |= sub_model_asv_val;
    }
  }
  sub_model_set.request_vector(sub_model_asv);
}


/** Update inactive values and labels in currentVariables and inactive
    bound constraints in userDefinedConstraints from variables and
    constraints data within subModel. */
void RecastModel::update_from_sub_model()
{
  currentVariables.inactive_continuous_variables(
    subModel.inactive_continuous_variables());
  currentVariables.inactive_discrete_int_variables(
    subModel.inactive_discrete_int_variables());
  currentVariables.inactive_discrete_real_variables(
    subModel.inactive_discrete_real_variables());

  userDefinedConstraints.inactive_continuous_lower_bounds(
    subModel.inactive_continuous_lower_bounds());
  userDefinedConstraints.inactive_continuous_upper_bounds(
    subModel.inactive_continuous_upper_bounds());
  userDefinedConstraints.inactive_discrete_int_lower_bounds(
    subModel.inactive_discrete_int_lower_bounds());
  userDefinedConstraints.inactive_discrete_int_upper_bounds(
    subModel.inactive_discrete_int_upper_bounds());
  userDefinedConstraints.inactive_discrete_real_lower_bounds(
    subModel.inactive_discrete_real_lower_bounds());
  userDefinedConstraints.inactive_discrete_real_upper_bounds(
    subModel.inactive_discrete_real_upper_bounds());

  currentVariables.inactive_continuous_variable_labels(
    subModel.inactive_continuous_variable_labels());
  currentVariables.inactive_discrete_int_variable_labels(
    subModel.inactive_discrete_int_variable_labels());
  currentVariables.inactive_discrete_real_variable_labels(
    subModel.inactive_discrete_real_variable_labels());

  if (variablesMapping) {
    // can't just apply variables mapping to values/bounds, since need inverse
    // of variablesMapping to go from subModel vars to currentVariables

    // any label, uncertain variable distributions, and linear
    // constraint mappings must be performed explicitly
  }
  else {
    // variable values
    currentVariables.continuous_variables(subModel.continuous_variables());
    currentVariables.discrete_int_variables(subModel.discrete_int_variables());
    currentVariables.discrete_real_variables(
      subModel.discrete_real_variables());
    // variable bounds
    userDefinedConstraints.continuous_lower_bounds(
      subModel.continuous_lower_bounds());
    userDefinedConstraints.continuous_upper_bounds(
      subModel.continuous_upper_bounds());
    userDefinedConstraints.discrete_int_lower_bounds(
      subModel.discrete_int_lower_bounds());
    userDefinedConstraints.discrete_int_upper_bounds(
      subModel.discrete_int_upper_bounds());
    userDefinedConstraints.discrete_real_lower_bounds(
      subModel.discrete_real_lower_bounds());
    userDefinedConstraints.discrete_real_upper_bounds(
      subModel.discrete_real_upper_bounds());
    // variable labels
    currentVariables.continuous_variable_labels(
      subModel.continuous_variable_labels());
    currentVariables.discrete_int_variable_labels(
      subModel.discrete_int_variable_labels());
    currentVariables.discrete_real_variable_labels(
      subModel.discrete_real_variable_labels());

    if (!subModel.discrete_design_set_int_values().empty())
      discreteDesignSetIntValues = subModel.discrete_design_set_int_values();
    if (!subModel.discrete_design_set_real_values().empty())
      discreteDesignSetRealValues = subModel.discrete_design_set_real_values();

    // uncertain variable distribution data
    distParams.update(subModel.distribution_parameters());

    if (!subModel.discrete_state_set_int_values().empty())
      discreteStateSetIntValues = subModel.discrete_state_set_int_values();
    if (!subModel.discrete_state_set_real_values().empty())
      discreteStateSetRealValues = subModel.discrete_state_set_real_values();

    // linear constraints
    if (subModel.num_linear_ineq_constraints()) {
      userDefinedConstraints.linear_ineq_constraint_coeffs(
        subModel.linear_ineq_constraint_coeffs());
      userDefinedConstraints.linear_ineq_constraint_lower_bounds(
        subModel.linear_ineq_constraint_lower_bounds());
      userDefinedConstraints.linear_ineq_constraint_upper_bounds(
        subModel.linear_ineq_constraint_upper_bounds());
    }
    if (subModel.num_linear_eq_constraints()) {
      userDefinedConstraints.linear_eq_constraint_coeffs(
        subModel.linear_eq_constraint_coeffs());
      userDefinedConstraints.linear_eq_constraint_targets(
        subModel.linear_eq_constraint_targets());
    }
  }

  if (primaryRespMapping) {
    // response mappings are in opposite direction from variables
    // mappings, so primaryRespMapping could potentially be used to
    // update currentResponse from subModel primary fns
  }
  else {
    // primary response function weights
    primaryRespFnWts = subModel.primary_response_fn_weights();

    // primary response function labels
    const StringArray& sm_resp_labels = subModel.response_labels();
    size_t i, num_primary = numFns 
      - userDefinedConstraints.num_nonlinear_eq_constraints()
      - userDefinedConstraints.num_nonlinear_ineq_constraints();
    for (i=0; i<num_primary; i++)
      currentResponse.function_label(sm_resp_labels[i], i);
  }

  if (secondaryRespMapping) {
    // response mappings are in opposite direction from variables
    // mappings, so secondaryRespMapping could potentially be used to
    // update currentResponse from subModel secondary fns
  }
  else {
    // secondary response function labels
    const StringArray& sm_resp_labels = subModel.response_labels();
    size_t i,
      num_nln_con = userDefinedConstraints.num_nonlinear_eq_constraints() +
        userDefinedConstraints.num_nonlinear_ineq_constraints(),
      num_primary    = numFns - num_nln_con,
      num_sm_primary = subModel.num_functions() - num_nln_con;
    for (i=0; i<num_nln_con; i++)
      currentResponse.function_label(sm_resp_labels[num_sm_primary+i],
				     num_primary+i);

    // nonlinear constraint bounds/targets
    if (subModel.num_nonlinear_ineq_constraints()) {
      userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds(
        subModel.nonlinear_ineq_constraint_lower_bounds());
      userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds(
        subModel.nonlinear_ineq_constraint_upper_bounds());
    }
    if (subModel.num_nonlinear_eq_constraints())
      userDefinedConstraints.nonlinear_eq_constraint_targets(
        subModel.nonlinear_eq_constraint_targets());
  }
}

} // namespace Dakota
