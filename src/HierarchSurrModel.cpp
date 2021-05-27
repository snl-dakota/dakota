/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrModel
//- Description: Implementation code for the HierarchSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "HierarchSurrModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]=
  "@(#) $Id: HierarchSurrModel.cpp 6656 2010-02-26 05:20:48Z mseldre $";

namespace Dakota {

extern Model dummy_model; // defined in DakotaModel.cpp


HierarchSurrModel::HierarchSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db),
  corrOrder(problem_db.get_short("model.surrogate.correction_order")),
  correctionMode(SINGLE_CORRECTION), mfPrecedence(true), modeKeyBufferSize(0)
{
  // Hierarchical surrogate models pass through numerical derivatives
  supportsEstimDerivs = false;
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");

  const StringArray& ordered_model_ptrs
    = problem_db.get_sa("model.surrogate.ensemble_model_pointers");

  size_t i, num_models = ordered_model_ptrs.size(),
           model_index = problem_db.get_db_model_node(); // for restoration

  //const std::pair<short,short>& cv_view = currentVariables.view();
  orderedModels.resize(num_models);
  for (i=0; i<num_models; ++i) {
    problem_db.set_db_model_nodes(ordered_model_ptrs[i]);
    orderedModels[i] = problem_db.get_model();
    check_submodel_compatibility(orderedModels[i]);
    //if (cv_view != orderedModels[i].current_variables().view()) {
    //  Cerr << "Error: variable views in hierarchical models must be "
    //       << "identical." << std::endl;
    //  abort_handler(-1);
    //}
  }

  problem_db.set_db_model_nodes(model_index); // restore

  assign_default_keys();

  // Correction is required in HierarchSurrModel for some responseModes.
  // Enforcement of a correction type for these modes occurs in
  // surrogate_response_mode(short).
  switch (responseMode) {
  case MODEL_DISCREPANCY: case AUTO_CORRECTED_SURROGATE:
    if (corrType) // initialize DiscrepancyCorrection using initial keys
      deltaCorr[activeKey].initialize(surrogate_model(), surrogateFnIndices,
				      corrType, corrOrder);
    break;
  }
  //truthResponseRef[truthModelKey] = currentResponse.copy();
}


void HierarchSurrModel::assign_default_keys()
{
  // default key data values, to be overridden at run time
  unsigned short id = 0, last_m = orderedModels.size() - 1;
  short r_type = Pecos::RAW_DATA;
  if (multilevel_multifidelity()) { // first and last model form / soln levels
    //size_t last_soln_lev = std::min(orderedModels[last_m].solution_levels(),
    // 				      orderedModels[0].solution_levels());
    //truthModelKey = Pecos::ActiveKey(id, r_type, last_m, last_soln_lev);
    //surrModelKey  = Pecos::ActiveKey(id, r_type,      0, last_soln_lev);

    // span both hierarchical dimensions by default
    size_t truth_soln_lev = orderedModels[last_m].solution_levels();
    truthModelKey = Pecos::ActiveKey(id, r_type, last_m, truth_soln_lev - 1);
    surrModelKey  = Pecos::ActiveKey(id, r_type,      0, 0);
  }
  else if (multifidelity()) { // first and last model form (no soln levels)
    // Note: for proper modeKeyBufferSize estimation, must maintain consistency
    // with NonDExpansion::configure_{sequence,indices}() and key definition
    // for NonDMultilevelSampling::control_variate_mc() in terms of SZ_MAX
    // usage, since this suppresses allocation of a solution level array.
    truthModelKey = Pecos::ActiveKey(id, r_type, last_m, SZ_MAX);
    surrModelKey  = Pecos::ActiveKey(id, r_type,      0, SZ_MAX);
  }
  else if (multilevel()) { // first and last solution level (last model)
    size_t truth_soln_lev = orderedModels[last_m].solution_levels();
    truthModelKey = Pecos::ActiveKey(id, r_type, last_m, truth_soln_lev - 1);
    surrModelKey  = Pecos::ActiveKey(id, r_type, last_m, 0);
  }
  activeKey.aggregate_keys(truthModelKey, surrModelKey,
			   Pecos::SINGLE_REDUCTION); // default: reduction only

  if (parallelLib.mpirun_flag()) {
    MPIPackBuffer send_buff;  short mode;
    send_buff << mode << activeKey; // serve_run() recvs single | aggregate key
    modeKeyBufferSize = send_buff.size();
  }

  check_model_interface_instance();
}


void HierarchSurrModel::check_submodel_compatibility(const Model& sub_model)
{
  bool err1 = check_active_variables(sub_model),
       err2 = check_inactive_variables(sub_model),
       err3 = check_response_qoi(sub_model);

  if (err1 || err2 || err3)
    abort_handler(MODEL_ERROR);
}


void HierarchSurrModel::
derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  // responseMode is a run-time setting (in SBLMinimizer, it is switched among
  // AUTO_CORRECTED_SURROGATE, BYPASS_SURROGATE, and UNCORRECTED_SURROGATE;
  // in NonDExpansion, it is switching between MODEL_DISCREPANCY and
  // UNCORRECTED_SURROGATE).  Since it is neither static nor generally
  // available at construct/init time, take a conservative approach with init
  // and free and a more aggressive approach with set.

  if (recurse_flag) {
    size_t i, model_index = probDescDB.get_db_model_node(), // for restoration
              num_models = orderedModels.size();

    // init and free must cover possible subset of active responseModes and
    // ordered model fidelities, but only 2 models at mpst will be active at
    // runtime.  In order to reduce the number of parallel configurations, we
    // group the responseModes into two sets: (1) the correction-based set
    // commonly used in surrogate-based optimization et al., and (2) the
    // aggregation-based set commonly used in multilevel/multifidelity UQ.

    // TO DO: would like a better detection option, but passing the mode from
    // the Iterator does not work in parallel w/o an additional bcast (Iterator
    // only instantiated on iteratorComm rank 0).  For now, we will infer it
    // from an associated method spec at init time.
    // Note: responseMode gets bcast at run time in component_parallel_mode()
    bool extra_deriv_config
      = (probDescDB.get_ushort("method.algorithm") & MINIMIZER_BIT);
    //(responseMode == UNCORRECTED_SURROGATE ||
    // responseMode == BYPASS_SURROGATE ||
    // responseMode == AUTO_CORRECTED_SURROGATE);

    for (i=0; i<num_models; ++i) {
      Model& model_i = orderedModels[i];
      // superset of possible init calls (two configurations for i > 0)
      probDescDB.set_db_model_nodes(model_i.model_id());
      model_i.init_communicators(pl_iter, max_eval_concurrency);
      if (extra_deriv_config) // && i) // mid and high fidelity only?
        model_i.init_communicators(pl_iter, model_i.derivative_concurrency());
    }


    /* This version inits only two models
    Model& lf_model = surrogate_model();
    Model& hf_model = truth_model();

    // superset of possible init calls (two configurations for HF)
    probDescDB.set_db_model_nodes(lf_model.model_id());
    lf_model.init_communicators(pl_iter, max_eval_concurrency);

    probDescDB.set_db_model_nodes(hf_model.model_id());
    hf_model.init_communicators(pl_iter, hf_model.derivative_concurrency());
    hf_model.init_communicators(pl_iter, max_eval_concurrency);
    */


    /* This version does not support runtime updating of responseMode
    switch (responseMode) {
    case UNCORRECTED_SURROGATE:
      // LF are used in iterator evals
      lf_model.init_communicators(pl_iter, max_eval_concurrency);
      break;
    case AUTO_CORRECTED_SURROGATE:
      // LF are used in iterator evals
      lf_model.init_communicators(pl_iter, max_eval_concurrency);
      // HF evals are for correction and validation:
      // concurrency = one eval at a time * derivative concurrency per eval
      hf_model.init_communicators(pl_iter, hf_model.derivative_concurrency());
      break;
    case BYPASS_SURROGATE:
      // HF are used in iterator evals
      hf_model.init_communicators(pl_iter, max_eval_concurrency);
      break;
    case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
      // LF and HF are used in iterator evals
      lf_model.init_communicators(pl_iter, max_eval_concurrency);
      hf_model.init_communicators(pl_iter, max_eval_concurrency);
      break;
    }
    */

    probDescDB.set_db_model_nodes(model_index); // restore all model nodes
  }
}


void HierarchSurrModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                          bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  // HierarchSurrModels do not utilize default set_ie_asynchronous_mode() as
  // they do not define the ie_parallel_level

  // This aggressive logic is appropriate for invocations of the Model via
  // Iterator::run(), but is fragile w.r.t. invocations of the Model outside
  // this scope (e.g., Model::evaluate() within SBLMinimizer).  The default
  // responseMode value is {AUTO_,UN}CORRECTED_SURROGATE, which mitigates
  // the specific case of SBLMinimizer, but the general fragility remains.
  if (recurse_flag) {

    // bcast not needed for recurse_flag=false in serve_run call to set_comms
    //if (pl_iter->server_communicator_size() > 1)
    //  parallelLib.bcast(responseMode, *pl_iter);

    switch (responseMode) {

    // CASES WITH A SINGLE ACTIVE MODEL:

    case UNCORRECTED_SURROGATE: {
      Model& lf_model = surrogate_model();
      lf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = lf_model.asynch_flag();
      evaluationCapacity = lf_model.evaluation_capacity();
      break;
    }
    case BYPASS_SURROGATE: {
      Model& hf_model = truth_model();
      hf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = hf_model.asynch_flag();
      evaluationCapacity = hf_model.evaluation_capacity();
      break;
    }

    // CASES WHERE ANY/ALL MODELS COULD BE ACTIVE:

    case AUTO_CORRECTED_SURROGATE: {
      // Lowest fidelity model is interfaced with minimizer:
      Model& model_0 = orderedModels[0];
      model_0.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = model_0.asynch_flag();
      evaluationCapacity = model_0.evaluation_capacity();

      // TO DO: this will not be true for multigrid optimization:
      bool use_deriv_conc = true; // only verifications/corrections
      // Either need detection logic, a passed option, or to abandon the
      // specialization and just generalize init/set/free to use the max
      // of the two values...

      // Loop over all higher fidelity models:
      size_t i, num_models = orderedModels.size(); int cap_i;
      for (i=1; i<num_models; ++i) {
	Model& model_i = orderedModels[i];
	if (use_deriv_conc) {
	  int deriv_conc_i = model_i.derivative_concurrency();
	  model_i.set_communicators(pl_iter, deriv_conc_i);
	  if (deriv_conc_i > 1 && model_i.asynch_flag()) asynchEvalFlag = true;
	}
	else {
	  model_i.set_communicators(pl_iter, max_eval_concurrency);
	  if (model_i.asynch_flag()) asynchEvalFlag = true;
	}
	cap_i = model_i.evaluation_capacity();
	if (cap_i > evaluationCapacity) evaluationCapacity = cap_i;
      }
      break;
    }
    case MODEL_DISCREPANCY: case AGGREGATED_MODELS: {
      size_t i, num_models = orderedModels.size(); int cap_i;
      asynchEvalFlag = false; evaluationCapacity = 1;
      for (i=0; i<num_models; ++i) {
	Model& model_i = orderedModels[i];
	model_i.set_communicators(pl_iter, max_eval_concurrency);
	if (model_i.asynch_flag()) asynchEvalFlag = true;
	cap_i = model_i.evaluation_capacity();
	if (cap_i > evaluationCapacity) evaluationCapacity = cap_i;
      }
      break;
    }
    }
  }
}


void HierarchSurrModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  if (recurse_flag) {

    size_t i, num_models = orderedModels.size();
    bool extra_deriv_config = true;//(responseMode == UNCORRECTED_SURROGATE ||
    // responseMode == BYPASS_SURROGATE ||
    // responseMode == AUTO_CORRECTED_SURROGATE);
    for (i=0; i<num_models; ++i) {
      Model& model_i = orderedModels[i];
      // superset of possible init calls (two configurations for i > 0)
      model_i.free_communicators(pl_iter, max_eval_concurrency);
      if (extra_deriv_config) // && i) // mid and high fidelity only?
        model_i.free_communicators(pl_iter, model_i.derivative_concurrency());
    }


    /* This version frees only two models:
    // superset of possible free calls (two configurations for HF)
    surrogate_model().free_communicators(pl_iter, max_eval_concurrency);
    Model& hf_model = truth_model();
    hf_model.free_communicators(pl_iter, hf_model.derivative_concurrency());
    hf_model.free_communicators(pl_iter, max_eval_concurrency);
    */


    /* This version does not support runtime updating of responseMode:
    switch (responseMode) {
    case UNCORRECTED_SURROGATE:
      lf_model.free_communicators(pl_iter, max_eval_concurrency);
      break;
    case AUTO_CORRECTED_SURROGATE:
      lf_model.free_communicators(pl_iter, max_eval_concurrency);
      hf_model.free_communicators(pl_iter, hf_model.derivative_concurrency());
      break;
    case BYPASS_SURROGATE:
      hf_model.free_communicators(pl_iter, max_eval_concurrency);
      break;
    case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
      lf_model.free_communicators(pl_iter, max_eval_concurrency);
      hf_model.free_communicators(pl_iter, max_eval_concurrency);
      break;
    }
    */
  }
}


/** Inactive variables must be propagated when a HierarchSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool HierarchSurrModel::initialize_mapping(ParLevLIter pl_iter)
{
  Model::initialize_mapping(pl_iter);

  // push inactive variable values/bounds from currentVariables and
  // userDefinedConstraints into orderedModels
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i) {
    orderedModels[i].initialize_mapping(pl_iter);
    init_model(orderedModels[i]);
  }

  return false; // no change to problem size
}


/** Inactive variables must be propagated when a HierarchSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool HierarchSurrModel::finalize_mapping()
{
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].finalize_mapping();

  Model::finalize_mapping();

  return false; // no change to problem size
}


void HierarchSurrModel::build_approximation()
{
  Cout << "\n>>>>> Building hierarchical approximation.\n";

  // perform the eval for the low fidelity model
  // NOTE: For SBO, the low fidelity eval is performed externally and its
  // response is passed into compute_correction.
  // -->> move LF model out and restructure if(!approxBuilds)
  //ActiveSet temp_set = lf_model.current_response().active_set();
  //temp_set.request_values(1);
  //if (sameModelInstance) assign_surrogate_key();
  //lf_model.evaluate(temp_set);
  //const Response& lo_fi_response = lf_model.current_response();

  Model& hf_model = truth_model();
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    hf_model.eval_tag_prefix(eval_tag);
  }

  // set HierarchSurrModel parallelism mode to HF model
  component_parallel_mode(TRUTH_MODEL_MODE);

  // update HF model with current variable values/bounds/labels
  update_model(hf_model);

  // store inactive variable values for use in determining whether an
  // automatic rebuild of an approximation is required
  // (reference{C,D}{L,U}Bnds are not needed in the hierarchical case)
  const Variables& hf_vars = hf_model.current_variables();
  copy_data(hf_vars.inactive_continuous_variables(),    referenceICVars);
  copy_data(hf_vars.inactive_discrete_int_variables(),  referenceIDIVars);
  referenceIDSVars = hf_vars.inactive_discrete_string_variables();
  copy_data(hf_vars.inactive_discrete_real_variables(), referenceIDRVars);

  // compute the response for the high fidelity model
  ShortArray total_asv, hf_asv, lf_asv;
  std::map<Pecos::ActiveKey, DiscrepancyCorrection>::iterator dc_it
    = deltaCorr.find(activeKey);
  if (dc_it!=deltaCorr.end() && dc_it->second.initialized())
    total_asv.assign(numFns, dc_it->second.data_order());
  else
    total_asv.assign(numFns, 1); // default: values only if no deriv correction
  asv_split(total_asv, hf_asv, lf_asv, true);

  if ( truthResponseRef.find(truthModelKey) == truthResponseRef.end() )
    truthResponseRef[truthModelKey] = currentResponse.copy();
  ActiveSet hf_set = currentResponse.active_set(); // copy
  hf_set.request_vector(hf_asv);
  if (sameModelInstance) assign_truth_key();
  hf_model.evaluate(hf_set);
  truthResponseRef[truthModelKey].update(hf_model.current_response());

  // could compute the correction to LF model here, but rely on an external
  // call for consistency with DataFitSurr and to facilitate SBO logic.  In
  // particular, lo_fi_response involves find_center(), hard conv check, etc.
  //deltaCorr[activeKey].compute(..., truthResponseRef, lo_fi_response);

  Cout << "\n<<<<< Hierarchical approximation build completed.\n";
  ++approxBuilds;
}


/*
bool HierarchSurrModel::
build_approximation(const RealVector& c_vars, const Response& response)
{
  // NOTE: this fn not currently used by SBO, but it could be.

  // Verify data content in incoming response
  const ShortArray& asrv = response.active_set_request_vector();
  bool data_complete = true; short corr_order = dataCorr.correction_order();
  for (size_t i=0; i<numFns; i++)
    if ( ( corr_order == 2 &&  (asrv[i] & 7) != 7 ) ||
	 ( corr_order == 1 &&  (asrv[i] & 3) != 3 ) ||
	 ( corr_order == 0 && !(asrv[i] & 1) ) )
      data_complete = false;
  if (data_complete) {
    Cout << "\n>>>>> Updating hierarchical approximation.\n";

    // are these updates necessary?
    Model& hf_model = truth_model();
    currentVariables.continuous_variables(c_vars);
    update_model(hf_model);
    const Variables& hf_vars = hf_model.current_variables();
    copy_data(hf_vars.inactive_continuous_variables(), referenceICVars);
    copy_data(hf_vars.inactive_discrete_variables(),   referenceIDVars);

    truthResponseRef.update(response);

    Cout << "\n<<<<< Hierarchical approximation update completed.\n";
  }
  else {
    Cerr << "Warning: cannot use anchor point in HierarchSurrModel::"
	 << "build_approximation(RealVector&, Response&).\n";
    currentVariables.continuous_variables(c_vars);
    build_approximation();
  }
  return false; // correction is not embedded and must be computed (by SBO)
}
*/


/** Compute the response synchronously using LF model, HF model, or
    both (mixed case).  For the LF model portion, compute the high
    fidelity response if needed with build_approximation(), and, if
    correction is active, correct the low fidelity results. */
void HierarchSurrModel::derived_evaluate(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  // define LF/HF evaluation requirements
  ShortArray hi_fi_asv, lo_fi_asv;
  bool hi_fi_eval, lo_fi_eval, mixed_eval;
  Response lo_fi_response, hi_fi_response; // don't use truthResponseRef
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
  case AGGREGATED_MODELS:
    asv_split(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
    hi_fi_eval = !hi_fi_asv.empty(); lo_fi_eval = !lo_fi_asv.empty();
    mixed_eval = (hi_fi_eval && lo_fi_eval);            break;
  case BYPASS_SURROGATE:
    hi_fi_eval = true; lo_fi_eval = mixed_eval = false; break;
  case MODEL_DISCREPANCY:
    hi_fi_eval = lo_fi_eval = mixed_eval = true;        break;
  }

  Model&   hf_model = (hi_fi_eval) ?     truth_model() : dummy_model;
  Model&   lf_model = (lo_fi_eval) ? surrogate_model() : dummy_model;
  Model& same_model = (hi_fi_eval) ? hf_model : lf_model;
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    if (sameModelInstance)
      same_model.eval_tag_prefix(eval_tag);
    else {
      if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
      if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
    }
  }

  if (sameModelInstance) update_model(same_model);

  // Notes on repetitive setting of model.solution_level_cost_index():
  // > when LF & HF are the same model, then setting the index for low or high
  //   invalidates the other fidelity definition.
  // > within a single derived_evaluate(), could protect these updates with
  //   "if (sameModelInstance && mixed_eval)", but this does not guard against
  //   changes in eval requirements from the previous evaluation.  Detecting
  //   the current solution index state is currently as expensive as resetting
  //   it, so just reset each time.

  // ------------------------------
  // Compute high fidelity response
  // ------------------------------
  if (hi_fi_eval) {
    component_parallel_mode(TRUTH_MODEL_MODE); // TO DO: sameModelInstance
    if (sameModelInstance) assign_truth_key();
    else                   update_model(hf_model);
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    case AGGREGATED_MODELS: {
      ActiveSet hi_fi_set(hi_fi_asv, set.derivative_vector());
      hf_model.evaluate(hi_fi_set);
      if (mixed_eval)
        hi_fi_response = (sameModelInstance) ? // deep copy or shared rep
	  hf_model.current_response().copy() : hf_model.current_response();
      else {
        currentResponse.active_set(hi_fi_set);
        currentResponse.update(hf_model.current_response());
      }
      break;
    }
    case BYPASS_SURROGATE:
      hf_model.evaluate(set);
      currentResponse.active_set(set);
      currentResponse.update(hf_model.current_response());
      break;
    case MODEL_DISCREPANCY:
      hf_model.evaluate(set);
      hi_fi_response = (sameModelInstance) ? hf_model.current_response().copy()
                       : hf_model.current_response(); // shared rep
      break;
    }
  }

  // -----------------------------
  // Compute low fidelity response
  // -----------------------------
  if (lo_fi_eval) {
    // pre-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE:
      // if build_approximation has not yet been called, call it now
      if (!approxBuilds || force_rebuild())
        build_approximation();
      break;
    }
    // compute the LF response
    component_parallel_mode(SURROGATE_MODEL_MODE); // TO DO: sameModelInstance
    if (sameModelInstance) assign_surrogate_key();
    else                   update_model(lf_model);
    ActiveSet lo_fi_set;
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    case AGGREGATED_MODELS:
      lo_fi_set.request_vector(lo_fi_asv);
      lo_fi_set.derivative_vector(set.derivative_vector());
      lf_model.evaluate(lo_fi_set);
      break;
    case MODEL_DISCREPANCY:
      lf_model.evaluate(set);
      break;
    }

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE: {
      // LF resp should not be corrected directly (see derived_synchronize())
      lo_fi_response = lf_model.current_response().copy();
      recursive_apply(currentVariables, lo_fi_response);
      if (!mixed_eval) {
        currentResponse.active_set(lo_fi_set);
        currentResponse.update(lo_fi_response);
      }
      break;
    }
    case UNCORRECTED_SURROGATE:
      if (mixed_eval)
        lo_fi_response = lf_model.current_response(); // shared rep
      else {
        currentResponse.active_set(lo_fi_set);
        currentResponse.update(lf_model.current_response());
      }
      break;
    }
  }

  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  switch (responseMode) {
  case MODEL_DISCREPANCY: {
    // don't update surrogate data within deltaCorr[key]'s Approximations;
    // just update currentResponse (managed as surrogate data at a higher level)
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    currentResponse.active_set(set);
    deltaCorr[activeKey].compute(hi_fi_response, lf_model.current_response(),
				 currentResponse, quiet_flag);
    break;
  }
  case AGGREGATED_MODELS:
    aggregate_response(hi_fi_response, lf_model.current_response(),
                       currentResponse);
    break;
  case UNCORRECTED_SURROGATE:   case AUTO_CORRECTED_SURROGATE:
    if (mixed_eval) {
      currentResponse.active_set(set);
      response_combine(hi_fi_response, lo_fi_response, currentResponse);
    }
    break;
  }
}


/** Compute the response asynchronously using LF model, HF model, or
    both (mixed case).  For the LF model portion, compute the high
    fidelity response with build_approximation() (for correcting the
    low fidelity results in derived_synchronize() and
    derived_synchronize_nowait()) if not performed previously. */
void HierarchSurrModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  ShortArray hi_fi_asv, lo_fi_asv;  bool hi_fi_eval, lo_fi_eval;
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
  case AGGREGATED_MODELS:
    asv_split(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
    hi_fi_eval = !hi_fi_asv.empty();  lo_fi_eval = !lo_fi_asv.empty();  break;
  case BYPASS_SURROGATE:
    hi_fi_eval = true;  lo_fi_eval = false;                             break;
  case MODEL_DISCREPANCY:
    hi_fi_eval = lo_fi_eval = true;                                     break;
  }

  Model&   hf_model = (hi_fi_eval) ?     truth_model() : dummy_model;
  Model&   lf_model = (lo_fi_eval) ? surrogate_model() : dummy_model;
  Model& same_model = (hi_fi_eval) ? hf_model : lf_model;
  bool asynch_hi_fi = (hi_fi_eval) ? hf_model.asynch_flag() : false,
       asynch_lo_fi = (lo_fi_eval) ? lf_model.asynch_flag() : false;

  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    if (sameModelInstance)
      same_model.eval_tag_prefix(eval_tag);
    else {
      if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
      if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
    }
  }

  if (sameModelInstance) update_model(same_model);

  // perform Model updates and define active sets for LF and HF evaluations
  ActiveSet hi_fi_set, lo_fi_set;
  if (hi_fi_eval) {
    // update HF model
    if (!sameModelInstance) update_model(hf_model);
    // update hi_fi_set
    hi_fi_set.derivative_vector(set.derivative_vector());
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    case AGGREGATED_MODELS:
      hi_fi_set.request_vector(hi_fi_asv);             break;
    case BYPASS_SURROGATE: case MODEL_DISCREPANCY:
      hi_fi_set.request_vector(set.request_vector());  break;
    }
  }
  if (lo_fi_eval) {
    // if build_approximation has not yet been called, call it now
    if ( responseMode == AUTO_CORRECTED_SURROGATE &&
         ( !approxBuilds || force_rebuild() ) )
      build_approximation();
    // update LF model
    if (!sameModelInstance) update_model(lf_model);
    // update lo_fi_set
    lo_fi_set.derivative_vector(set.derivative_vector());
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    case AGGREGATED_MODELS:
      lo_fi_set.request_vector(lo_fi_asv);             break;
    case MODEL_DISCREPANCY:
      lo_fi_set.request_vector(set.request_vector());  break;
    }
  }

  // HierarchSurrModel's asynchEvalFlag is set if _either_ LF or HF is
  // asynchronous, resulting in use of derived_evaluate_nowait().
  // To manage general case of mixed asynch, launch nonblocking evals first,
  // followed by blocking evals.

  // For notes on repetitive setting of model.solution_level_cost_index(), see
  // derived_evaluate() above.

  // launch nonblocking evals before any blocking ones
  if (hi_fi_eval && asynch_hi_fi) { // HF model may be executed asynchronously
    // don't need to set component parallel mode since only queues the job
    if (sameModelInstance) assign_truth_key();
    hf_model.evaluate_nowait(hi_fi_set);
    // store map from HF eval id to HierarchSurrModel id
    truthIdMap[hf_model.evaluation_id()] = surrModelEvalCntr;
  }
  if (lo_fi_eval && asynch_lo_fi) { // LF model may be executed asynchronously
    // don't need to set component parallel mode since only queues the job
    if (sameModelInstance) assign_surrogate_key();
    lf_model.evaluate_nowait(lo_fi_set);
    // store map from LF eval id to HierarchSurrModel id
    surrIdMap[lf_model.evaluation_id()] = surrModelEvalCntr;
    // store variables set needed for correction
    if (responseMode == AUTO_CORRECTED_SURROGATE)
      rawVarsMap[surrModelEvalCntr] = currentVariables.copy();
  }

  // now launch any blocking evals
  if (hi_fi_eval && !asynch_hi_fi) { // execute HF synchronously & cache resp
    component_parallel_mode(TRUTH_MODEL_MODE);
    if (sameModelInstance) assign_truth_key();
    hf_model.evaluate(hi_fi_set);
    // not part of rekey_synch(); can rekey to surrModelEvalCntr immediately
    cachedTruthRespMap[surrModelEvalCntr] = hf_model.current_response().copy();
  }
  if (lo_fi_eval && !asynch_lo_fi) { // execute LF synchronously & cache resp
    component_parallel_mode(SURROGATE_MODEL_MODE);
    if (sameModelInstance) assign_surrogate_key();
    lf_model.evaluate(lo_fi_set);
    Response lo_fi_response(lf_model.current_response().copy());
    // correct LF response prior to caching
    if (responseMode == AUTO_CORRECTED_SURROGATE)
      // correct synch cases now (asynch cases get corrected in
      // derived_synchronize_aggregate*)
      recursive_apply(currentVariables, lo_fi_response);
    // cache corrected LF response for retrieval during synchronization.
    // not part of rekey_synch(); can rekey to surrModelEvalCntr immediately.
    cachedApproxRespMap[surrModelEvalCntr] = lo_fi_response;// deep copied above
  }
}


/** Blocking retrieval of asynchronous evaluations from LF model, HF
    model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the array.
    derived_synchronize() is designed for the general case where
    derived_evaluate_nowait() may be inconsistent in its use of low
    fidelity evaluations, high fidelity evaluations, or both. */
const IntResponseMap& HierarchSurrModel::derived_synchronize()
{
  surrResponseMap.clear();

  if (sameModelInstance  || sameInterfaceInstance ||
      truthIdMap.empty() || surrIdMap.empty()) { // 1 queue: blocking synch
    IntResponseMap hf_resp_map_rekey, lf_resp_map_rekey;
    derived_synchronize_sequential(hf_resp_map_rekey, lf_resp_map_rekey, true);
    derived_synchronize_combine(hf_resp_map_rekey, lf_resp_map_rekey,
                                surrResponseMap);
  }
  else                               // competing queues: nonblocking synch
    derived_synchronize_competing();

  return surrResponseMap;
}


/** Nonblocking retrieval of asynchronous evaluations from LF model,
    HF model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the map.
    derived_synchronize_nowait() is designed for the general case
    where derived_evaluate_nowait() may be inconsistent in its use of
    actual evals, approx evals, or both. */
const IntResponseMap& HierarchSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();

  IntResponseMap hf_resp_map_rekey, lf_resp_map_rekey;
  derived_synchronize_sequential(hf_resp_map_rekey, lf_resp_map_rekey, false);
  derived_synchronize_combine_nowait(hf_resp_map_rekey, lf_resp_map_rekey,
                                     surrResponseMap);

  return surrResponseMap;
}


void HierarchSurrModel::
derived_synchronize_sequential(IntResponseMap& hf_resp_map_rekey,
                               IntResponseMap& lf_resp_map_rekey, bool block)
{
  // --------------------------
  // synchronize HF model evals
  // --------------------------
  if (!truthIdMap.empty()) { // synchronize HF evals
    component_parallel_mode(TRUTH_MODEL_MODE);
    rekey_synch(truth_model(), block, truthIdMap, hf_resp_map_rekey);
  }
  // add cached truth evals from:
  // (a) recovered HF asynch evals that could not be returned since LF
  //     eval portions were still pending, or
  // (b) synchronous HF evals performed within evaluate_nowait()
  hf_resp_map_rekey.insert(cachedTruthRespMap.begin(),
                           cachedTruthRespMap.end());
  cachedTruthRespMap.clear();

  // --------------------------
  // synchronize LF model evals
  // --------------------------
  if (!surrIdMap.empty()) { // synchronize LF evals
    component_parallel_mode(SURROGATE_MODEL_MODE);
    // Interface::rawResponseMap should _not_ be corrected directly since
    // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share a
    // responseRep -> modifying rawResponseMap affects data_pairs.
    bool deep_copy = (responseMode == AUTO_CORRECTED_SURROGATE);
    rekey_synch(surrogate_model(), block, surrIdMap, lf_resp_map_rekey,
		deep_copy);
  }
  // add cached approx evals from:
  // (a) recovered LF asynch evals that could not be returned since HF
  //     eval portions were still pending, or
  // (b) synchronous LF evals performed within evaluate_nowait()
  lf_resp_map_rekey.insert(cachedApproxRespMap.begin(),
                           cachedApproxRespMap.end());
  cachedApproxRespMap.clear();
}


void HierarchSurrModel::derived_synchronize_competing()
{
  // in this case, we don't want to starve either LF or HF scheduling by
  // blocking on one or the other --> leverage derived_synchronize_nowait()
  IntResponseMap aggregated_map; // accumulate surrResponseMap returns
  while (!truthIdMap.empty() || !surrIdMap.empty()) {
    // partial_map is a reference to surrResponseMap, returned by _nowait()
    const IntResponseMap& partial_map = derived_synchronize_nowait();
    if (!partial_map.empty())
      aggregated_map.insert(partial_map.begin(), partial_map.end());
  }

  // Note: cached response maps and any LF/HF aggregations are managed
  // within derived_synchronize_nowait()

  std::swap(surrResponseMap, aggregated_map);
}


void HierarchSurrModel::
derived_synchronize_combine(const IntResponseMap& hf_resp_map,
                            IntResponseMap& lf_resp_map,
                            IntResponseMap& combined_resp_map)
{
  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  // {hf,lf}_resp_map may be partial sets (partial surrogateFnIndices
  // in {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY,
  // AGGREGATED_MODELS).

  IntRespMCIter hf_cit = hf_resp_map.begin(), lf_cit = lf_resp_map.begin();
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  switch (responseMode) {
  case MODEL_DISCREPANCY: {
    DiscrepancyCorrection& delta_corr = deltaCorr[activeKey];
    for (; hf_cit != hf_resp_map.end() && lf_cit != lf_resp_map.end();
	 ++hf_cit, ++lf_cit) {
      check_key(hf_cit->first, lf_cit->first);
      delta_corr.compute(hf_cit->second, lf_cit->second,
	combined_resp_map[hf_cit->first], quiet_flag);
    }
    break;
  }
  case AGGREGATED_MODELS:
    for (; hf_cit != hf_resp_map.end() && lf_cit != lf_resp_map.end();
	 ++hf_cit, ++lf_cit) {
      check_key(hf_cit->first, lf_cit->first);
      aggregate_response(hf_cit->second, lf_cit->second,
                         combined_resp_map[hf_cit->first]);
    }
    break;
  default: { // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
    if (lf_resp_map.empty()) {
      combined_resp_map = hf_resp_map;  // can't swap w/ const
      return;
    }
    if (responseMode == AUTO_CORRECTED_SURROGATE)
      compute_apply_delta(lf_resp_map);
    if (hf_resp_map.empty()) {
      std::swap(combined_resp_map, lf_resp_map);
      return;
    }
    // process combinations of HF and LF completions
    Response empty_resp;
    while (hf_cit != hf_resp_map.end() || lf_cit != lf_resp_map.end()) {
      // these have been rekeyed already to top-level surrModelEvalCntr:
      int hf_eval_id = (hf_cit == hf_resp_map.end()) ?
                       INT_MAX : hf_cit->first;
      int lf_eval_id = (lf_cit == lf_resp_map.end()) ?
                       INT_MAX : lf_cit->first;

      if (hf_eval_id < lf_eval_id) { // only HF available
        response_combine(hf_cit->second, empty_resp,
                         combined_resp_map[hf_eval_id]);
        ++hf_cit;
      }
      else if (lf_eval_id < hf_eval_id) { // only LF available
        response_combine(empty_resp, lf_cit->second,
                         combined_resp_map[lf_eval_id]);
        ++lf_cit;
      }
      else { // both LF and HF available
        response_combine(hf_cit->second, lf_cit->second,
                         combined_resp_map[hf_eval_id]);
        ++hf_cit;
        ++lf_cit;
      }
    }
    break;
  }
  }
}


void HierarchSurrModel::
derived_synchronize_combine_nowait(const IntResponseMap& hf_resp_map,
                                   IntResponseMap& lf_resp_map,
                                   IntResponseMap& combined_resp_map)
{
  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  // {hf,lf}_resp_map may be partial sets (partial surrogateFnIndices
  // in {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY).

  // Early return options avoid some overhead:
  if (lf_resp_map.empty() && surrIdMap.empty()) {// none completed, none pending
    combined_resp_map = hf_resp_map;  // can't swap w/ const
    return;
  }
  if (responseMode == AUTO_CORRECTED_SURROGATE)
    compute_apply_delta(lf_resp_map);
  if (hf_resp_map.empty() && truthIdMap.empty()) {//none completed, none pending
    std::swap(combined_resp_map, lf_resp_map);
    return;
  }

  // invert remaining entries (pending jobs) in truthIdMap and surrIdMap
  IntIntMap remain_truth_ids, remain_surr_ids;
  IntIntMCIter id_it;
  for (id_it=truthIdMap.begin(); id_it!=truthIdMap.end(); ++id_it)
    remain_truth_ids[id_it->second] = id_it->first;
  for (id_it=surrIdMap.begin();  id_it!=surrIdMap.end();  ++id_it)
    remain_surr_ids[id_it->second]  = id_it->first;

  // process any combination of HF and LF completions
  IntRespMCIter hf_cit = hf_resp_map.begin();
  IntRespMIter  lf_it  = lf_resp_map.begin();
  Response empty_resp;
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  std::map<Pecos::ActiveKey, DiscrepancyCorrection>::iterator dc_it;
  if (responseMode == MODEL_DISCREPANCY)
    dc_it = deltaCorr.find(activeKey);
  while (hf_cit != hf_resp_map.end() || lf_it != lf_resp_map.end()) {
    // these have been rekeyed already to top-level surrModelEvalCntr:
    int hf_eval_id = (hf_cit == hf_resp_map.end()) ? INT_MAX : hf_cit->first;
    int lf_eval_id = (lf_it  == lf_resp_map.end()) ? INT_MAX : lf_it->first;
    // process LF/HF results or cache them for next pass
    if (hf_eval_id < lf_eval_id) { // only HF available
      switch (responseMode) {
      case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
        // LF contribution is pending -> cache HF response
        cachedTruthRespMap[hf_eval_id] = hf_cit->second;
        break;
      default: // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
        if (remain_surr_ids.find(hf_eval_id) != remain_surr_ids.end())
          // LF contribution is pending -> cache HF response
          cachedTruthRespMap[hf_eval_id] = hf_cit->second;
        else // no LF component is pending -> HF contribution is sufficient
          response_combine(hf_cit->second, empty_resp,
                           surrResponseMap[hf_eval_id]);
        break;
      }
      ++hf_cit;
    }
    else if (lf_eval_id < hf_eval_id) { // only LF available
      switch (responseMode) {
      case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
        // HF contribution is pending -> cache LF response
        cachedApproxRespMap[lf_eval_id] = lf_it->second;
        break;
      default: // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
        if (remain_truth_ids.find(lf_eval_id) != remain_truth_ids.end())
          // HF contribution is pending -> cache LF response
          cachedApproxRespMap[lf_eval_id] = lf_it->second;
        else // no HF component is pending -> LF contribution is sufficient
          response_combine(empty_resp, lf_it->second,
                           surrResponseMap[lf_eval_id]);
        break;
      }
      ++lf_it;
    }
    else { // both LF and HF available
      bool cache_for_pending_corr = false;
      switch (responseMode) {
      case MODEL_DISCREPANCY: {
        dc_it->second.compute(hf_cit->second, lf_it->second,
			      surrResponseMap[hf_eval_id], quiet_flag);
        break;
      }
      case AGGREGATED_MODELS:
        aggregate_response(hf_cit->second, lf_it->second,
                           surrResponseMap[hf_eval_id]);
        break;
      default: // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
        response_combine(hf_cit->second, lf_it->second,
                         surrResponseMap[hf_eval_id]);
        break;
      }
      ++hf_cit;
      ++lf_it;
    }
  }
}


void HierarchSurrModel::create_tabular_datastream()
{
  OutputManager& mgr = parallelLib.output_manager();
  mgr.open_tabular_datastream();

  switch (responseMode) {
  case AGGREGATED_MODELS: case MODEL_DISCREPANCY: // two models/resolutions
  case BYPASS_SURROGATE: { // use same row len since commonly alternated

    bool mf = multifidelity();  StringArray iface_ids;
    if (mf) { // sameInterfaceInstance can't be used since it varies at run time
      iface_ids.push_back("HF_interf");
      iface_ids.push_back("LF_interf");
    }
    else
      iface_ids.push_back("interface");
    mgr.create_tabular_header(iface_ids); // includes graphics cntr

    // identify solution level control variable
    size_t av_index = truth_model().solution_control_variable_index();
    if (av_index == _NPOS)
      mgr.append_tabular_header(currentVariables);
    else {
      mgr.append_tabular_header(currentVariables, 0, av_index); // leading set

      // output paired solution control values
      const String& soln_cntl_label = solution_control_label();
      StringArray tab_labels(2);
      tab_labels[0] = "HF_" + soln_cntl_label;
      tab_labels[1] = "LF_" + soln_cntl_label;
      mgr.append_tabular_header(tab_labels);

      ++av_index; // output completed for the active soln control
      mgr.append_tabular_header(currentVariables, av_index,
				currentVariables.tv() - av_index);
    }

    //mgr.append_tabular_header(currentResponse);
    // Add HF/LF/Del prepends
    StringArray labels = currentResponse.function_labels(); // copy
    size_t q, num_qoi = qoi(), num_labels = labels.size();
    if (responseMode == MODEL_DISCREPANCY)
      for (q=0; q<num_qoi; ++q)
	labels[q].insert(0, "Del_");
    else {
      for (q=0; q<num_qoi; ++q)
	labels[q].insert(0, "HF_");
      for (q=num_qoi; q<num_labels; ++q)
	labels[q].insert(0, "LF_");
    }
    mgr.append_tabular_header(labels, true); // with endl
    break;
  }
  case NO_SURROGATE:
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    mgr.create_tabular_header(currentVariables, currentResponse);
    break;
  }
}


const String& HierarchSurrModel::solution_control_label()
{
  Model&  hf_model = truth_model();
  size_t adv_index = hf_model.solution_control_discrete_variable_index();
  switch (hf_model.solution_control_variable_type()) {
  case DISCRETE_DESIGN_RANGE:       case DISCRETE_DESIGN_SET_INT:
  case DISCRETE_INTERVAL_UNCERTAIN: case DISCRETE_UNCERTAIN_SET_INT:
  case DISCRETE_STATE_RANGE:        case DISCRETE_STATE_SET_INT:
    return currentVariables.all_discrete_int_variable_labels()[adv_index];
    break;
  case DISCRETE_DESIGN_SET_STRING:  case DISCRETE_UNCERTAIN_SET_STRING:
  case DISCRETE_STATE_SET_STRING:
    return currentVariables.all_discrete_string_variable_labels()[adv_index];
    break;
  case DISCRETE_DESIGN_SET_REAL:  case DISCRETE_UNCERTAIN_SET_REAL:
  case DISCRETE_STATE_SET_REAL:
    return currentVariables.all_discrete_real_variable_labels()[adv_index];
    break;
  }
}


void HierarchSurrModel::
derived_auto_graphics(const Variables& vars, const Response& resp)
{
  //parallelLib.output_manager().add_tabular_data(vars, interface_id(), resp);

  // manage interface id(s) and resolution control
  Model &lf_model = surrogate_model(), &hf_model = truth_model();
  OutputManager& output_mgr = parallelLib.output_manager();

  switch (responseMode) {
  case AGGREGATED_MODELS: case MODEL_DISCREPANCY: // two models/resolutions
  case BYPASS_SURROGATE: { // use same row len since commonly alternated

    // output interface ids, potentially paired
    bool mf = multifidelity();  StringArray iface_ids;
    if (mf) { // sameInterfaceInstance can't be used since it varies at run time
      if (truthModelKey.empty()) iface_ids.push_back("N/A");//preserve row len
      else                       iface_ids.push_back(hf_model.interface_id());
      if ( surrModelKey.empty()) iface_ids.push_back("N/A");//preserve row len
      else                       iface_ids.push_back(lf_model.interface_id());
    }
    else
      iface_ids.push_back(hf_model.interface_id());
    output_mgr.add_tabular_data(iface_ids); // includes graphics cntr

    // identify solution level control variable
    size_t av_index = hf_model.solution_control_variable_index();
    if (av_index == _NPOS)
      output_mgr.add_tabular_data(vars);
    else {
      output_mgr.add_tabular_data(vars, 0, av_index);//leading set in spec order

      // output paired solution control values
      bool truth_key = !truthModelKey.empty(), surr_key = !surrModelKey.empty();
      if (sameModelInstance && truth_key && surr_key) {
	// HF soln cntl was overwritten by LF and must be temporarily restored
	assign_truth_key();      add_tabular_solution_level_value(hf_model);
	assign_surrogate_key();  add_tabular_solution_level_value(lf_model);
      }
      else { // HF and LF soln levels are not overlapping
	if (truth_key)  add_tabular_solution_level_value(hf_model);
	else output_mgr.add_tabular_scalar("N/A");// preserve consistent row len
	if ( surr_key)  add_tabular_solution_level_value(lf_model);
	else output_mgr.add_tabular_scalar("N/A");// preserve consistent row len
      }

      ++av_index; // output completed for the active soln control
      output_mgr.add_tabular_data(vars, av_index, vars.tv()-av_index);//trailing
    }
     
    output_mgr.add_tabular_data(resp);
    break;
  }
  case NO_SURROGATE:
    output_mgr.add_tabular_data(hf_model.current_variables(),
				hf_model.interface_id(), resp);
    break;
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    output_mgr.add_tabular_data(lf_model.current_variables(),
				lf_model.interface_id(), resp);
    break;
  }
}


void HierarchSurrModel::add_tabular_solution_level_value(Model& model)
{
  OutputManager& output_mgr = parallelLib.output_manager();
  switch (model.solution_control_variable_type()) {
  case DISCRETE_DESIGN_RANGE:       case DISCRETE_DESIGN_SET_INT:
  case DISCRETE_INTERVAL_UNCERTAIN: case DISCRETE_UNCERTAIN_SET_INT:
  case DISCRETE_STATE_RANGE:        case DISCRETE_STATE_SET_INT:
    output_mgr.add_tabular_scalar(model.solution_level_int_value());    break;
  case DISCRETE_DESIGN_SET_STRING:  case DISCRETE_UNCERTAIN_SET_STRING:
  case DISCRETE_STATE_SET_STRING:
    output_mgr.add_tabular_scalar(model.solution_level_string_value()); break;
  case DISCRETE_DESIGN_SET_REAL:  case DISCRETE_UNCERTAIN_SET_REAL:
  case DISCRETE_STATE_SET_REAL:
    output_mgr.add_tabular_scalar(model.solution_level_real_value());   break;
  }
}


void HierarchSurrModel::compute_apply_delta(IntResponseMap& lf_resp_map)
{
  // Incoming we have a completed LF evaluation that may be used to compute a
  // correction and may be the target of application of a correction.

  // First, test if a correction is previously available or can now be computed
  DiscrepancyCorrection& delta_corr = deltaCorr[activeKey];
  bool corr_comp = delta_corr.computed(), cache_for_pending_corr = false,
      quiet_flag = (outputLevel < NORMAL_OUTPUT);
  if (!corr_comp) {
    // compute a correction corresponding to the first entry in rawVarsMap
    IntVarsMCIter v_corr_cit = rawVarsMap.begin();
    if (v_corr_cit != rawVarsMap.end()) {
      // if corresponding LF response is complete, compute the delta
      IntRespMCIter lf_corr_cit = lf_resp_map.find(v_corr_cit->first);
      if (lf_corr_cit != lf_resp_map.end()) {
        delta_corr.compute(v_corr_cit->second, truthResponseRef[truthModelKey],
			   lf_corr_cit->second, quiet_flag);
        corr_comp = true;
      }
    }
  }

  // Next, apply the correction.  We cache an uncorrected eval when the
  // components necessary for correction are still pending (returning
  // corrected evals with the first available LF response would lead to
  // nondeterministic results).
  IntVarsMIter v_it; IntRespMIter lf_it; int lf_eval_id;
  for (lf_it=lf_resp_map.begin(); lf_it!=lf_resp_map.end(); ++lf_it) {
    lf_eval_id = lf_it->first;
    v_it = rawVarsMap.find(lf_eval_id);
    if (v_it != rawVarsMap.end()) {
      if (corr_comp) { // apply the correction to the LF response
	recursive_apply(v_it->second, lf_it->second);
        rawVarsMap.erase(v_it);
      }
      else // no new corrections can be applied -> cache uncorrected
        cachedApproxRespMap.insert(*lf_it);
    }
    // else correction already applied
  }
  // remove cached responses from lf_resp_map
  if (!corr_comp)
    for (lf_it =cachedApproxRespMap.begin();
         lf_it!=cachedApproxRespMap.end(); ++lf_it)
      lf_resp_map.erase(lf_it->first);
}


void HierarchSurrModel::
single_apply(const Variables& vars, Response& resp,
	     const Pecos::ActiveKey& paired_key)
{
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  bool apply_corr = true;
  DiscrepancyCorrection& delta_corr = deltaCorr[paired_key];
  if (!delta_corr.computed()) {
    Pecos::ActiveKey truth_key, surr_key;
    extract_model_keys(paired_key, truth_key, surr_key);
    std::map<Pecos::ActiveKey, Response>::iterator it
      = truthResponseRef.find(truth_key);
    if (it == truthResponseRef.end()) apply_corr = false; // not found
    else delta_corr.compute(vars, it->second, resp, quiet_flag);
  }
  if (apply_corr)
    delta_corr.apply(vars, resp, quiet_flag);
}


void HierarchSurrModel::recursive_apply(const Variables& vars, Response& resp)
{
  switch (correctionMode) {
  case SINGLE_CORRECTION: case DEFAULT_CORRECTION:
    single_apply(vars, resp, activeKey);
    break;
  case FULL_MODEL_FORM_CORRECTION: {
    size_t num_models = orderedModels.size();
    unsigned short i, lf_form = surrModelKey.retrieve_model_form();
    // perform a 1D sweep starting from current surrModelKey; this could be
    // part of a multidimensional sweep, so don't target truthModelKey at end
    Pecos::ActiveKey paired_key;// = activeKey.copy();
    paired_key.aggregate_keys(surrModelKey.copy(), surrModelKey.copy(),
			      Pecos::SINGLE_REDUCTION);
    for (i = lf_form; i < num_models - 1; ++i) {
      paired_key.id(i); // consistent with current pair sequences
      paired_key.assign_model_form(i+1, 0); // HF model form in KeyData[0]
      paired_key.assign_model_form(i,   1); // LF model form in KeyData[1]
      single_apply(vars, resp, paired_key);
    }
    break;
  }
  case FULL_SOLUTION_LEVEL_CORRECTION: {
    // assume a consistent model index from surrModelKey
    size_t lf_lev = surrModelKey.retrieve_resolution_level();
    if (lf_lev == SZ_MAX) {
      Cerr << "Error: FULL_SOLUTION_LEVEL_CORRECTION requires solution level "
	   << "within model key." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    size_t i, num_levels = surrogate_model().solution_levels();
    // perform a 1D sweep starting from current surrModelKey; this could be
    // part of a multidimensional sweep, so don't target truthModelKey at end
    Pecos::ActiveKey paired_key;// = activeKey.copy();
    paired_key.aggregate_keys(surrModelKey.copy(), surrModelKey.copy(),
			      Pecos::SINGLE_REDUCTION);
    for (i = lf_lev; i < num_levels - 1; ++i) {
      paired_key.id(i); // consistent with current pair sequences
      paired_key.assign_resolution_level(i+1, 0); //   fine res in KeyData[0]
      paired_key.assign_resolution_level(i,   1); // coarse res in KeyData[1]
      single_apply(vars, resp, paired_key);
    }
    break;
  }
  //case SEQUENCE_CORRECTION: // apply sequence of discrepancy corrections
  //  for (size_t i = 0; i < corrSequence.size(); ++i)
  //    single_apply(vars, resp, corrSequence[i]);
  //  break;
  }
}


void HierarchSurrModel::resize_response(bool use_virtual_counts)
{
  size_t num_surr, num_truth;
  if (use_virtual_counts) { // allow models to consume lower-level aggregations
    num_surr  = surrogate_model().qoi();
    num_truth =     truth_model().qoi();
  }
  else { // raw counts align with currentResponse raw count
    num_surr  = surrogate_model().response_size();
    num_truth =     truth_model().response_size();
  }

  switch (responseMode) {
  case AGGREGATED_MODELS:
    numFns = num_surr + num_truth;  break;
  case MODEL_DISCREPANCY:
    if (num_surr != num_truth) {
      Cerr << "Error: mismatch in response sizes for MODEL_DISCREPANCY mode "
	   << "in HierarchSurrModel::resize_response()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    numFns = num_truth;  break;
  case BYPASS_SURROGATE:       case NO_SURROGATE:
    numFns = num_truth;  break;
  case UNCORRECTED_SURROGATE:  case AUTO_CORRECTED_SURROGATE:  default:
    numFns = num_surr;   break;
  }

  // gradient and Hessian settings are based on independent spec (not LF, HF)
  // --> preserve previous settings
  if (currentResponse.num_functions() != numFns) {
    currentResponse.reshape(numFns, currentVariables.cv(),
                            !currentResponse.function_gradients().empty(),
                            !currentResponse.function_hessians().empty());

    // update message lengths for send/receive of parallel jobs (normally
    // performed once in Model::init_communicators() just after construct time)
    //estimate_message_lengths();
    //
    // NOT NECESSARY: Model::synchronize() and Model::serve_run() delegate to
    // HierarchSurrModel::{derived_synchronize,serve_run}() which delegate to
    // synchronize() and serve_run() by the LF or HF model.
    // --> Jobs are never returned using messages containing the expanded
    //     Response object.  Expansion by combination only happens on
    //     iteratorCommRank 0 within derived_synchronize_combine{,_nowait}().
  }
}


void HierarchSurrModel::component_parallel_mode(short par_mode)
{
  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // -----------------------------
  // terminate previous serve mode (if active)
  // -----------------------------
  // TO DO: restarting servers for a change in soln control index w/o change
  // in model may be overkill (send of state vars in vars buffer sufficient?)
  bool restart = false;
  if (componentParallelMode != par_mode || componentParallelKey != activeKey) {
    Pecos::ActiveKey old_hf_key, old_lf_key;
    extract_model_keys(componentParallelKey, old_hf_key, old_lf_key,
		       componentParallelMode);
    switch (componentParallelMode) {
    case SURROGATE_MODEL_MODE:
      stop_model(old_lf_key.retrieve_model_form());  break;
    case     TRUTH_MODEL_MODE:
      stop_model(old_hf_key.retrieve_model_form());  break;
    }
    restart = true;
  }

  // ------------------------------------------------------------
  // set ParallelConfiguration for new mode and retrieve new data
  // ------------------------------------------------------------
  if (par_mode == TRUTH_MODEL_MODE) { // new mode
    // activation delegated to HF model
  }
  else if (par_mode == SURROGATE_MODEL_MODE) { // new mode
    // activation delegated to LF model
  }

  // -----------------------
  // activate new serve mode (matches HierarchSurrModel::serve_run(pl_iter)).
  // -----------------------
  // These bcasts match the outer parallel context (pl_iter).
  if (restart && modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      parallelLib.bcast(par_mode, mi_pl);
      if (par_mode) { // send model index state corresponding to active mode
	MPIPackBuffer send_buff;
	send_buff << responseMode << activeKey;
	//int buffer_len = send_buff.size();
	//parallelLib.bcast(buffer_len, mi_pl); // avoid this overhead
 	parallelLib.bcast(send_buff, mi_pl);
      }
    }
  }

  componentParallelMode = par_mode;  componentParallelKey = activeKey;
}


void HierarchSurrModel::serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false); // don't recurse

  // manage LF model and HF model servers, matching communication from
  // HierarchSurrModel::component_parallel_mode()
  // Note: could consolidate logic by bcasting componentParallelKey,
  //       except for special handling of responseMode for TRUTH_MODEL_MODE.
  componentParallelMode = 1; // dummy value to enter loop
  while (componentParallelMode) {
    parallelLib.bcast(componentParallelMode, *pl_iter); // outer context
    if (componentParallelMode) {
      // Local size estimation for recv buffer can't simply use activeKey
      // since previous key may be a singleton from bypass_surrogate_mode():
      //MPIPackBuffer send_buff;  send_buff << responseMode << activeKey;
      //int buffer_len = send_buff.size();
      //
      // This approach works, but avoid the additional bcast overhead:
      //parallelLib.bcast(buffer_len, *pl_iter);

      // receive model state from HierarchSurrModel::component_parallel_mode()
      MPIUnpackBuffer recv_buffer(modeKeyBufferSize);
      parallelLib.bcast(recv_buffer, *pl_iter);
      recv_buffer >> responseMode >> activeKey; // replace previous/initial key

      active_model_key(activeKey); // updates {truth,surr}ModelKey
      if (componentParallelMode == SURROGATE_MODEL_MODE) {
	// serve active LF model:
	surrogate_model().serve_run(pl_iter, max_eval_concurrency);
	// Note: ignores erroneous BYPASS_SURROGATE
      }
      else if (componentParallelMode == TRUTH_MODEL_MODE) {
	// serve active HF model, employing correct iterator concurrency:
	Model& hf_model = truth_model();
	switch (responseMode) {
	case UNCORRECTED_SURROGATE:
	  Cerr << "Error: cannot set parallel mode to TRUTH_MODEL_MODE for a "
	       << "response mode of UNCORRECTED_SURROGATE." << std::endl;
	  abort_handler(-1);                                              break;
	case AUTO_CORRECTED_SURROGATE:
	  hf_model.serve_run(pl_iter, hf_model.derivative_concurrency()); break;
	case BYPASS_SURROGATE: case MODEL_DISCREPANCY: case AGGREGATED_MODELS:
	  hf_model.serve_run(pl_iter, max_eval_concurrency);              break;
	}
      }
    }
  }
}

} // namespace Dakota
