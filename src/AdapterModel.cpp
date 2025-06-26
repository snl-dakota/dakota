/*  _______________________________________________________________________

    Dakota: Explore and predict with confidence.
    Copyright 2014-2025
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

// #include "dakota_system_defs.hpp"
#include "AdapterModel.hpp"

#include "DataMethod.hpp"
// #include "EvaluationStore.hpp"

static const char rcsId[] =
    "@(#) $Id: AdapterModel.cpp 7029 2010-10-22 00:17:02Z mseldre $";

namespace Dakota {

// #define DEBUG

/** Base portion of derived constructor chains. */
AdapterModel::AdapterModel(void (*resp_map)(const Variables& vars,
                                            const ActiveSet& set,
                                            Response& response))
    : Model(), adapterModelEvalCntr(0), respMapping(resp_map) {
  modelType = "adapter";

  // currentVariables = vars.copy(); // shared svd
  // numDerivVars     = vars.cv();
  // currentResponse  = resp.copy();
  // numFns           = resp.num_functions();
}

/** This constructor creates a generic stand-alone AdapterModel
    (not part of a derived constructor chain). */
AdapterModel::AdapterModel(const Variables& initial_vars,
                           const Constraints& cons, const Response& resp,
                           void (*resp_map)(const Variables& vars,
                                            const ActiveSet& set,
                                            Response& response))
    : Model(initial_vars.view(), initial_vars.shared_data(), true,
            resp.shared_data(), true, resp.active_set(), SILENT_OUTPUT),
      adapterModelEvalCntr(0),
      respMapping(resp_map) {
  modelType = "adapter";
  modelId = "ADAPTER";
  outputLevel = SILENT_OUTPUT;

  currentVariables.active_variables(initial_vars);  // {c,di,dr}_vars
  userDefinedConstraints.update(cons);  // update the Model ctor instantiation
}

/*
bool AdapterModel::
init_variables(const ShortShortPair& recast_vars_view,
               const SizetArray& vars_comps_totals,
               const BitArray& all_relax_di, const BitArray& all_relax_dr)
{
  const Variables& sub_model_vars = subModel.current_variables();
  const SharedVariablesData&  svd = sub_model_vars.shared_data();

  // BMA: We actually don't allow the case of a change in
  // vars_comp_totals, but no mapping, but have to allow it here in
  // case mapping not yet provided.

  // if any change in variable types, will need a new SharedVariablesData
  bool vars_char_same =
    ( vars_comps_totals.empty() ||
      svd.components_totals()         == vars_comps_totals ) &&
    ( all_relax_di.empty() ||
      svd.all_relaxed_discrete_int()  == all_relax_di )      &&
    ( all_relax_dr.empty() ||
      svd.all_relaxed_discrete_real() == all_relax_dr );

  bool new_vars_view = (recast_vars_view != sm_vars.view());

  // check change in character first as mapping may not yet be present...
  if (vars_char_same) {
    // variables are mapped but not resized: deep copy of vars and
    // same svd, since types may change in transformed space
    if (new_vars_view) { // avoid building + then updating views
      SharedVariablesData recast_svd(sm_svd.copy(recast_vars_view));
      currentVariables = sm_vars.copy(recast_svd);
    }
    else
      currentVariables = sub_model_vars.copy(true); // independent svd
  }
  else {
    // variables are resized; need new SVD regardless
    SharedVariablesData recast_svd(recast_vars_view, vars_comps_totals,
                                   all_relax_di, all_relax_dr);
    currentVariables = Variables(recast_svd);
  }

  // propagate number of active continuous vars to derivative vars
  numDerivVars = currentVariables.cv();

  return !vars_char_same; // return reshape_vars
}


void AdapterModel::
init_response(size_t num_recast_primary_fns, size_t num_recast_secondary_fns,
              short recast_resp_order, bool reshape_vars)
{
  numFns = num_recast_primary_fns + num_recast_secondary_fns;

  // recasting of response
  const Response& sub_model_resp = subModel.current_response();
  currentResponse = sub_model_resp.copy();

  bool grad_flag = (recast_resp_order & 2),
    hess_flag = (recast_resp_order & 4),
    sm_grad_flag = !sub_model_resp.function_gradients().empty(),
    sm_hess_flag = !sub_model_resp.function_hessians().empty();
  const Variables& sub_model_vars = subModel.current_variables();
  if ( sub_model_vars.cv()            != numDerivVars ||
       sub_model_resp.num_functions() != numFns       ||
       grad_flag != sm_grad_flag || hess_flag != sm_hess_flag )
    currentResponse.reshape(numFns, numDerivVars, grad_flag, hess_flag);
}


void AdapterModel::
init_constraints(size_t num_recast_secondary_fns,
                 size_t recast_secondary_offset, bool reshape_vars)
{
  // recasting of constraints
  SharedVariablesData recast_svd = currentVariables.shared_data();
  const Constraints& sub_model_cons = subModel.user_defined_constraints();
  userDefinedConstraints = (reshape_vars) ?
    Constraints(recast_svd) : sub_model_cons.copy();

  // the recast_secondary_offset cannot in general be inferred from the
  // contributing fns in secondaryRespMapIndices (recast constraints may be
  // defined, e.g., with no contributing fns), and must therefore be passed.
  size_t num_recast_nln_ineq = recast_secondary_offset,
    num_recast_nln_eq = num_recast_secondary_fns - num_recast_nln_ineq;
  if ( num_recast_nln_ineq != sub_model_cons.num_nonlinear_ineq_constraints()
       || num_recast_nln_eq   != sub_model_cons.num_nonlinear_eq_constraints() )
    userDefinedConstraints.reshape(num_recast_nln_ineq, num_recast_nln_eq,
      sub_model_cons.num_linear_ineq_constraints(),
      sub_model_cons.num_linear_eq_constraints());
}
*/

/** The AdapterModel is evaluated by an Iterator for a recast problem
    formulation.  Therefore, the currentVariables, incoming active set,
    and output currentResponse all correspond to the recast inputs/outputs. */
void AdapterModel::derived_evaluate(const ActiveSet& set) {
  ++adapterModelEvalCntr;
  respMapping(currentVariables, set, currentResponse);
}

void AdapterModel::derived_evaluate_nowait(const ActiveSet& set) {
  ++adapterModelEvalCntr;

  // Bookkeep Variables/ActiveSet instances for use in synchronize, emulating
  // (for now) the potential for future parallel execution of call-backs
  // > Note: Model::parallelLib is currently initialized to dummy_lib

  // A reduced overhead alternative would be to just short-circuit with a
  // blocking execution of respMapping() that writes directly to adapterRespMap

  adapterVarsMap[adapterModelEvalCntr] = currentVariables.copy();
  adapterSetMap[adapterModelEvalCntr] = set;
}

const IntResponseMap& AdapterModel::derived_synchronize() {
  adapterRespMap.clear();

  IntVarsMIter v_it;
  IntASMIter s_it;
  std::pair<IntRespMIter, bool> r_it_pr;
  const SharedResponseData& srd = currentResponse.shared_data();
  for (v_it = adapterVarsMap.begin(), s_it = adapterSetMap.begin();
       v_it != adapterVarsMap.end() && s_it != adapterSetMap.end();
       ++v_it, ++s_it) {
    ActiveSet& set = s_it->second;
    r_it_pr = adapterRespMap.insert(
        std::pair<int, Response>(v_it->first, Response(srd, set)));
    respMapping(v_it->second, set, r_it_pr.first->second);
  }

  adapterVarsMap.clear();
  adapterSetMap.clear();
  return adapterRespMap;
}

const IntResponseMap& AdapterModel::derived_synchronize_nowait() {
  return derived_synchronize();
}  // redirect to blocking

/*
void AdapterModel::
transform_response(const Variables& vars, Response& resp)
{
  if (respMapping) {
    assign_instance();
    respMapping(vars, resp);
  }
}


void AdapterModel::
transform_response_map(const IntResponseMap& old_resp_map,
                       IntResponseMap& new_resp_map)
{
  IntRespMCIter r_cit; IntASMIter s_it; IntVarsMIter v_it;
  for (r_cit=old_resp_map.begin(); r_cit!=old_resp_map.end(); ++r_cit) {
    int native_id = r_cit->first;
    s_it =  recastSetMap.find(native_id);
    v_it = recastVarsMap.find(native_id);

    Response new_resp(currentResponse.copy()); // correct size, labels, etc.
    new_resp.active_set(s_it->second);
    transform_response(v_it->second, r_cit->second, new_resp);
    new_resp_map[native_id] = new_resp;

    // cleanup
    recastSetMap.erase(s_it);  recastVarsMap.erase(v_it);
  }
}


void AdapterModel::initialize_data_from_submodel()
{
  componentParallelMode = SUB_MODEL_MODE;
  outputLevel           = subModel.output_level();

  gradientType          = subModel.gradient_type();
  methodSource          = subModel.method_source();
  ignoreBounds          = subModel.ignore_bounds();
  centralHess	        = subModel.central_hess();
  intervalType          = subModel.interval_type();
  fdGradStepSize        = subModel.fd_gradient_step_size();
  fdGradStepType        = subModel.fd_gradient_step_type();
  gradIdAnalytic        = subModel.gradient_id_analytic();
  gradIdNumerical       = subModel.gradient_id_numerical();

  hessianType           = subModel.hessian_type();
  quasiHessType         = subModel.quasi_hessian_type();
  fdHessByFnStepSize    = subModel.fd_hessian_by_fn_step_size();
  fdHessByGradStepSize  = subModel.fd_hessian_by_grad_step_size();
  fdHessStepType        = subModel.fd_hessian_step_type();
  hessIdAnalytic        = subModel.hessian_id_analytic();
  hessIdNumerical       = subModel.hessian_id_numerical();
  hessIdQuasi           = subModel.hessian_id_quasi();

  scalingOpts           = subModel.scaling_options();
}


void AdapterModel::assign_instance()
{ } // no static instance pointer to assign at base (default is no-op)


ActiveSet AdapterModel::default_active_set()
{
  // The "base class" implementation assumes that supportsEstimDerivs is false
  // and that gradients/hessians, if available, are computed by a submodel and
  // hence can be provided by this model.
  ActiveSet set;
  set.derivative_vector(currentVariables.all_continuous_variable_ids());
  bool has_deriv_vars = set.derivative_vector().size() != 0;
  ShortArray asv(numFns, 1);
  if(has_deriv_vars) {
    if (gradientType != "none")
        // && (gradientType == "analytic" || supportsEstimDerivs))
        for(auto &a : asv)
          a |=  2;

    if (hessianType != "none")
        // && (hessianType == "analytic" || supportsEstimDerivs))
        for(auto &a : asv)
          a |=  4;
  }
  set.request_vector(asv);
  return set;
}


void AdapterModel::declare_sources()
{
  evaluationsDB.declare_source(modelId, modelType, subModel.model_id(),
                               subModel.model_type());
}
*/

}  // namespace Dakota
