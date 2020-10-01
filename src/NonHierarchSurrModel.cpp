/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonHierarchSurrModel
//- Description: Implementation code for the NonHierarchSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "NonHierarchSurrModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]=
  "@(#) $Id: NonHierarchSurrModel.cpp 6656 2010-02-26 05:20:48Z mseldre $";

namespace Dakota {

extern Model dummy_model; // defined in DakotaModel.cpp


NonHierarchSurrModel::NonHierarchSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db)
  //corrOrder(problem_db.get_short("model.surrogate.correction_order")),
  //correctionMode(SINGLE_CORRECTION)
{
  // NonHierarchical surrogate models pass through numerical derivatives
  supportsEstimDerivs = false;
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");

  const String& truth_model_ptr
    = problem_db.get_sa("model.surrogate.truth_model_pointer");
  const StringArray& unordered_model_ptrs
    = problem_db.get_sa("model.surrogate.unordered_model_pointers");

  size_t i, num_approx_models = unordered_model_ptrs.size(),
           model_index = problem_db.get_db_model_node(); // for restoration

  problem_db.set_db_model_nodes(truth_model_ptr);
  truthModel = problem_db.get_model();
  check_submodel_compatibility(truthModel);

  unorderedModels.resize(num_approx_models);
  for (i=0; i<num_approx_models; ++i) {
    problem_db.set_db_model_nodes(unordered_model_ptrs[i]);
    unorderedModels[i] = problem_db.get_model();
    check_submodel_compatibility(unorderedModels[i]);
  }

  problem_db.set_db_model_nodes(model_index); // restore

  /*
  // default index values, to be overridden at run time
  surrModelKey.assign(3, 0); truthModelKey.assign(3, 0);
  if (num_models == 1) // first and last solution level (1 model)
    truthModelKey[2] = orderedModels[0].solution_levels() - 1;
  else { // first and last model form (solution levels ignored)
    truthModelKey[1] = num_models - 1;
    surrModelKey[2]  = truthModelKey[2] = USHRT_MAX;
  }
  Pecos::DiscrepancyCalculator::
    aggregate_keys(truthModelKey, surrModelKey, activeKey);
  check_model_interface_instance();

  // Correction is required in NonHierarchSurrModel for some responseModes.
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
  */
}


void NonHierarchSurrModel::check_submodel_compatibility(const Model& sub_model)
{
  SurrogateModel::check_submodel_compatibility(sub_model);
  
  bool error_flag = false;
  // Check for compatible array sizing between sub_model and currentResponse.
  // NonHierarchSurrModel creates aggregations (and a DataFitSurrModel will
  // consume them). Aggregations may span truthModel, unorderedModels, or both.
  // For now, allow any aggregation factor.
  size_t sm_qoi = sub_model.qoi();//, aggregation = numFns / sm_qoi;
  if ( numFns % sm_qoi ) {
    Cerr << "Error: incompatibility between subordinate and aggregate model "
	 << "response function sets\n       within NonHierarchSurrModel: "
	 << numFns << " aggregate and " << sm_qoi << " subordinate functions.\n"
	 << "       Check consistency of responses specifications."<< std::endl;
    error_flag = true;
  }

  // TO DO: Bayes exp design (hi2lo) introduces new requirements on a
  // hierarchical model, and MF active subspaces will as well.
  // > For (simulation-based) OED, one option is to enforce consistency in
  //   inactive state (config vars) and allow active parameterization to vary.
  // > For hi2lo, this implies that the active variable subset could be null
  //   for HF, as the active calibration variables only exist for LF.
  size_t sm_icv = sub_model.icv(),  sm_idiv = sub_model.idiv(),
    sm_idsv = sub_model.idsv(),     sm_idrv = sub_model.idrv(),
    icv  = currentVariables.icv(),  idiv = currentVariables.idiv(),
    idsv = currentVariables.idsv(), idrv = currentVariables.idrv();
  if (sm_icv != icv || sm_idiv != idiv || sm_idsv != idsv || sm_idrv != idrv) {
    Cerr << "Error: incompatibility between subordinate and aggregate model "
	 << "variable sets within\n       NonHierarchSurrModel: inactive "
	 << "subordinate = " << icv << " continuous, " << idiv
	 << " discrete int, " << idsv << " discrete string, and " << idrv
	 << " discrete real and\n       inactive aggregate = " << sm_icv
	 << " continuous, " << sm_idiv << " discrete int, " << sm_idsv
	 << " discrete string, and " << sm_idrv << " discrete real.  Check "
	 << "consistency of variables specifications." << std::endl;
    error_flag = true;
  }

  if (error_flag)
    abort_handler(-1);
}


void NonHierarchSurrModel::
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
              num_approx_models = unorderedModels.size();

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

    for (i=0; i<num_approx_models; ++i) {
      Model& model_i = unorderedModels[i];
      // superset of possible init calls (two configurations for i > 0)
      probDescDB.set_db_model_nodes(model_i.model_id());
      model_i.init_communicators(pl_iter, max_eval_concurrency);
      if (extra_deriv_config) // && i) // mid and high fidelity only?
        model_i.init_communicators(pl_iter, model_i.derivative_concurrency());
    }

    probDescDB.set_db_model_nodes(truthModel.model_id());
    truthModel.init_communicators(pl_iter, max_eval_concurrency);
    if (extra_deriv_config) // && i) // mid and high fidelity only?
      truthModel.init_communicators(pl_iter,
				    truthModel.derivative_concurrency());

    probDescDB.set_db_model_nodes(model_index); // restore all model nodes
  }
}


void NonHierarchSurrModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                          bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  // NonHierarchSurrModels do not utilize default set_ie_asynchronous_mode() as
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

    /*
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
    */
      size_t i, num_approx_models = unorderedModels.size(); int cap_i;
      asynchEvalFlag = false; evaluationCapacity = 1;

      for (i=0; i<num_approx_models; ++i) {
	Model& model_i = unorderedModels[i];
	model_i.set_communicators(pl_iter, max_eval_concurrency);
	if (model_i.asynch_flag()) asynchEvalFlag = true;
	cap_i = model_i.evaluation_capacity();
	if (cap_i > evaluationCapacity) evaluationCapacity = cap_i;
      }

      truthModel.set_communicators(pl_iter, max_eval_concurrency);
      if (truthModel.asynch_flag()) asynchEvalFlag = true;
      cap_i = truthModel.evaluation_capacity();
      if (cap_i > evaluationCapacity) evaluationCapacity = cap_i;

      //break;
    //}
    //}
  }
}


void NonHierarchSurrModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  if (recurse_flag) {

    size_t i, num_approx_models = unorderedModels.size();
    bool extra_deriv_config = true;//(responseMode == UNCORRECTED_SURROGATE ||
                                   // responseMode == BYPASS_SURROGATE ||
                                   // responseMode == AUTO_CORRECTED_SURROGATE);
    for (i=0; i<num_approx_models; ++i) {
      Model& model_i = unorderedModels[i];
      // superset of possible init calls (two configurations for i > 0)
      model_i.free_communicators(pl_iter, max_eval_concurrency);
      if (extra_deriv_config) // && i) // mid and high fidelity only?
        model_i.free_communicators(pl_iter, model_i.derivative_concurrency());
    }

    truthModel.free_communicators(pl_iter, max_eval_concurrency);
    if (extra_deriv_config) // && i) // mid and high fidelity only?
      truthModel.free_communicators(pl_iter,
				    truthModel.derivative_concurrency());
  }
}


/** Inactive variables must be propagated when a NonHierarchSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool NonHierarchSurrModel::initialize_mapping(ParLevLIter pl_iter)
{
  Model::initialize_mapping(pl_iter);

  // push inactive variable values/bounds from currentVariables and
  // userDefinedConstraints into orderedModels
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i) {
    unorderedModels[i].initialize_mapping(pl_iter);
    init_model(unorderedModels[i]);
  }

  truthModel.initialize_mapping(pl_iter);
  init_model(truthModel);

  return false; // no change to problem size
}


/** Inactive variables must be propagated when a NonHierarchSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool NonHierarchSurrModel::finalize_mapping()
{
  size_t i, num_approx_models = unorderedModels.size();
  for (i=0; i<num_approx_models; ++i)
    unorderedModels[i].finalize_mapping();

  truthModel.finalize_mapping();

  Model::finalize_mapping();

  return false; // no change to problem size
}


/*
void NonHierarchSurrModel::build_approximation()
{
  Cout << "\n>>>>> Building hierarchical approximation.\n";

  // perform the eval for the low fidelity model
  // NOTE: For SBO, the low fidelity eval is performed externally and its
  // response is passed into compute_correction.
  // -->> move LF model out and restructure if(!approxBuilds)
  //ActiveSet temp_set = lf_model.current_response().active_set();
  //temp_set.request_values(1);
  //if (sameModelInstance)
  //  lf_model.solution_level_index(surrogate_level_index());
  //lf_model.evaluate(temp_set);
  //const Response& lo_fi_response = lf_model.current_response();

  Model& hf_model = truth_model();
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    hf_model.eval_tag_prefix(eval_tag);
  }

  // set NonHierarchSurrModel parallelism mode to HF model
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
  std::map<UShortArray, DiscrepancyCorrection>::iterator dc_it
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
  if (sameModelInstance)
    hf_model.solution_level_index(truth_level_index());
  hf_model.evaluate(hf_set);
  truthResponseRef[truthModelKey].update(hf_model.current_response());

  // could compute the correction to LF model here, but rely on an external
  // call for consistency with DataFitSurr and to facilitate SBO logic.  In
  // particular, lo_fi_response involves find_center(), hard conv check, etc.
  //deltaCorr[activeKey].compute(..., truthResponseRef, lo_fi_response);

  Cout << "\n<<<<< NonHierarchical approximation build completed.\n";
  ++approxBuilds;
}
*/


/** Compute the response synchronously using LF model, HF model, or
    both (mixed case).  For the LF model portion, compute the high
    fidelity response if needed with build_approximation(), and, if
    correction is active, correct the low fidelity results. */
void NonHierarchSurrModel::derived_evaluate(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  // define eval reqmts, with unorderedModels followed by truthModel at end
  Short2DArray indiv_asv;
  asv_split(set.request_vector(), indiv_asv);

  size_t  num_models = indiv_asv.size(),
    num_unord_models = unorderedModels.size();
  ResponseArray indiv_response(num_models);

  /*
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    if (sameModelInstance)
      same_model.eval_tag_prefix(eval_tag);
    else {
      if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
      if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
    }
  }
  */

  switch (responseMode) {
  case AGGREGATED_MODELS: {
    ActiveSet set_i(set); // copy DVV
    for (size_t i=0; i<num_models; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      if (test_asv(asv_i)) {
	Model& model_i = (i < num_unord_models) ? unorderedModels[i]
	               : truthModel;
	/*
	component_parallel_mode(i); // TO DO: size_t instead of enum
	if (sameModelInstance)
	  model_i.solution_level_index(model_level_index(i)); // TO DO
	else
	  update_model(model_i);
	*/
	set_i.request_vector(asv_i);
	model_i.evaluate(set_i);
	indiv_response[i] = (sameModelInstance) ? // deep copy or shared rep
	  model_i.current_response().copy() : model_i.current_response();
      }
    }
    aggregate_response(indiv_response, currentResponse);
    break;
  }
  case BYPASS_SURROGATE:
    if (num_models > 1) {
      Cerr << "Error: wrong aggregate ASV size for BYPASS_SURROGATE mode in "
	   << "NonHierarchSurrModel::derived_evaluate()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    /*
    component_parallel_mode(num_unord_models); // TO DO: size_t instead of enum
    if (sameModelInstance)
      truthModel.solution_level_index(truth_level_index(i)); // TO DO
    else
      update_model(truthModel);
    */
    truthModel.evaluate(set);
    currentResponse.active_set(set);
    currentResponse.update(truthModel.current_response());
    break;
  }
}


/** Compute the response asynchronously using LF model, HF model, or
    both (mixed case).  For the LF model portion, compute the high
    fidelity response with build_approximation() (for correcting the
    low fidelity results in derived_synchronize() and
    derived_synchronize_nowait()) if not performed previously. */
void NonHierarchSurrModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  // define eval reqmts, with unorderedModels followed by truthModel at end
  Short2DArray indiv_asv;
  asv_split(set.request_vector(), indiv_asv);

  size_t  num_models = indiv_asv.size();

  // NonHierarchSurrModel's asynchEvalFlag is set if any model supports
  // asynchronous, resulting in use of derived_evaluate_nowait().
  // To manage general case of mixed asynch, launch nonblocking evals first,
  // followed by blocking evals.

  // For notes on repetitive setting of model.solution_level_index(), see
  // derived_evaluate() above.

  switch (responseMode) {
  case AGGREGATED_MODELS: {
    ActiveSet set_i(set); // copy DVV
    size_t i, num_unord_models = unorderedModels.size();
    // first pass for nonblocking models
    for (i=0; i<num_models; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      Model& model_i = (i < num_unord_models) ? unorderedModels[i]
	             : truthModel;
      if (model_i.asynch_flag() && test_asv(asv_i)) {
	/*
	if (sameModelInstance)
	  model_i.solution_level_index(model_level_index(i)); // TO DO
	else
	  update_model(model_i);
	*/
	set_i.request_vector(asv_i);
	model_i.evaluate_nowait(set_i);
	modelIdMap[i][model_i.evaluation_id()] = surrModelEvalCntr;
      }
    }
    // second pass for blocking models
    for (i=0; i<num_models; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      Model& model_i = (i < num_unord_models) ? unorderedModels[i]
	             : truthModel;
      if (!model_i.asynch_flag() && test_asv(asv_i)) {
	/*
	component_parallel_mode(i); // TO DO: size_t instead of enum
	if (sameModelInstance)
	  model_i.solution_level_index(model_level_index(i)); // TO DO
	else
	  update_model(model_i);
	*/
	set_i.request_vector(asv_i);
	model_i.evaluate(set_i);
	cachedRespMap[i][surrModelEvalCntr] = model_i.current_response().copy();
      }
    }
    break;
  }
  case BYPASS_SURROGATE:
    if (num_models > 1) {
      Cerr << "Error: wrong aggregate ASV size for BYPASS_SURROGATE mode in "
	   << "NonHierarchSurrModel::derived_evaluate()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    /*
    if (sameModelInstance)
      truthModel.solution_level_index(truth_level_index(i)); // TO DO
    else
      update_model(truthModel);
    */
    truthModel.evaluate_nowait(set); // no need to test for blocking eval
    modelIdMap[0][truthModel.evaluation_id()] = surrModelEvalCntr;
    break;
  }
}


/** Blocking retrieval of asynchronous evaluations from LF model, HF
    model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the array.
    derived_synchronize() is designed for the general case where
    derived_evaluate_nowait() may be inconsistent in its use of low
    fidelity evaluations, high fidelity evaluations, or both. */
const IntResponseMap& NonHierarchSurrModel::derived_synchronize()
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
const IntResponseMap& NonHierarchSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();

  IntResponseMap hf_resp_map_rekey, lf_resp_map_rekey;
  derived_synchronize_sequential(hf_resp_map_rekey, lf_resp_map_rekey, false);
  derived_synchronize_combine_nowait(hf_resp_map_rekey, lf_resp_map_rekey,
                                     surrResponseMap);

  return surrResponseMap;
}


void NonHierarchSurrModel::
derived_synchronize_sequential(IntResponseMap& hf_resp_map_rekey,
                               IntResponseMap& lf_resp_map_rekey, bool block)
{
  // --------------------------
  // synchronize HF model evals
  // --------------------------
  IntRespMCIter r_cit;
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


void NonHierarchSurrModel::derived_synchronize_competing()
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


void NonHierarchSurrModel::
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


void NonHierarchSurrModel::
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
  std::map<UShortArray, DiscrepancyCorrection>::iterator dc_it;
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


void NonHierarchSurrModel::compute_apply_delta(IntResponseMap& lf_resp_map)
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


void NonHierarchSurrModel::
single_apply(const Variables& vars, Response& resp,
	     const UShortArray& paired_key)
{
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  bool apply_corr = true;
  DiscrepancyCorrection& delta_corr = deltaCorr[paired_key];
  if (!delta_corr.computed()) {
    UShortArray truth_key;
    Pecos::DiscrepancyCalculator::extract_key(paired_key, truth_key, 0);
    std::map<UShortArray, Response>::iterator it
      = truthResponseRef.find(truth_key);
    if (it == truthResponseRef.end()) apply_corr = false; // not found
    else delta_corr.compute(vars, it->second, resp, quiet_flag);
  }
  if (apply_corr)
    delta_corr.apply(vars, resp, quiet_flag);
}


void NonHierarchSurrModel::
recursive_apply(const Variables& vars, Response& resp)
{
  switch (correctionMode) {
  case SINGLE_CORRECTION: case DEFAULT_CORRECTION:
    single_apply(vars, resp, activeKey);
    break;
  case FULL_MODEL_FORM_CORRECTION: {
    // assume a consistent level index from surrModelKey
    size_t i, num_approx_models = orderedModels.size();  UShortArray paired_key;
    unsigned short lf_form = surrModelKey[1];
    Pecos::DiscrepancyCalculator::
      aggregate_keys(surrModelKey, surrModelKey, paired_key); // initialize
    for (i = lf_form; i < num_approx_models - 1; ++i) {
      paired_key[1] = i+1;  // update HF model form
      paired_key[3] = i;    // update LF model form
      single_apply(vars, resp, paired_key);
    }
    break;
  }
  case FULL_SOLUTION_LEVEL_CORRECTION: {
    // assume a consistent model index from surrModelKey
    unsigned short lf_lev = surrModelKey[2];
    if (lf_lev == USHRT_MAX) {
      Cerr << "Error: FULL_SOLUTION_LEVEL_CORRECTION requires solution level "
	   << "within model key." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    size_t i, num_levels = surrogate_model().solution_levels();
    UShortArray paired_key;
    Pecos::DiscrepancyCalculator::
      aggregate_keys(surrModelKey, surrModelKey, paired_key); // initialize
    for (i = lf_lev; i < num_levels - 1; ++i) {
      paired_key[2] = i+1;  // update   fine solution level
      paired_key[4] = i;    // update coarse solution level
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


void NonHierarchSurrModel::resize_response(bool use_virtual_counts)
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
	   << "in NonHierarchSurrModel::resize_response()." << std::endl;
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
    // NonHierarchSurrModel::{derived_synchronize,serve_run}() which delegate to
    // synchronize() and serve_run() by the LF or HF model.
    // --> Jobs are never returned using messages containing the expanded
    //     Response object.  Expansion by combination only happens on
    //     iteratorCommRank 0 within derived_synchronize_combine{,_nowait}().
  }
}


void NonHierarchSurrModel::component_parallel_mode(short par_mode)
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
    UShortArray old_hf_key, old_lf_key;
    extract_model_keys(componentParallelKey, old_hf_key, old_lf_key);
    switch (componentParallelMode) {
    case SURROGATE_MODEL_MODE:  stop_model(old_lf_key[1]);  break;
    case     TRUTH_MODEL_MODE:  stop_model(old_hf_key[1]);  break;
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
  // activate new serve mode (matches NonHierarchSurrModel::serve_run(pl_iter)).
  // -----------------------
  // These bcasts match the outer parallel context (pl_iter).
  if (restart && modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      parallelLib.bcast(par_mode, mi_pl);
      if (par_mode) { // send model index state corresponding to active mode
	MPIPackBuffer send_buff;
	send_buff << responseMode << activeKey;
 	parallelLib.bcast(send_buff, mi_pl);
      }
    }
  }

  componentParallelMode = par_mode;  componentParallelKey = activeKey;
}


void NonHierarchSurrModel::
serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false); // don't recurse

  // manage LF model and HF model servers, matching communication from
  // NonHierarchSurrModel::component_parallel_mode()
  // Note: could consolidate logic by bcasting componentParallelKey,
  //       except for special handling of responseMode for TRUTH_MODEL_MODE.
  componentParallelMode = 1; // dummy value to be replaced inside loop
  while (componentParallelMode) {
    parallelLib.bcast(componentParallelMode, *pl_iter); // outer context
    if (componentParallelMode) {
      // use a quick size estimation for recv buffer i/o size bcast
      UShortArray dummy_key(5, 0); // for size estimation (worst 2-model case)
      MPIPackBuffer send_buff;  send_buff << responseMode << dummy_key;
      int buffer_len = send_buff.size();
      // receive model state from NonHierarchSurrModel::component_parallel_mode()
      MPIUnpackBuffer recv_buffer(buffer_len);
      parallelLib.bcast(recv_buffer, *pl_iter);
      recv_buffer >> responseMode >> activeKey;

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


void NonHierarchSurrModel::init_model(Model& model)
{
  // Set the low/high fidelity model variable descriptors with the variable
  // descriptors from currentVariables (eliminates the need to replicate
  // variable descriptors in the input file).  This only needs to be performed
  // once (as opposed to the other updates above).  However, performing this set
  // in the constructor does not propagate properly for multiple surrogates/
  // nestings since the sub-model construction (and therefore any sub-sub-model
  // constructions) must finish before calling any set functions on it.  That
  // is, after-the-fact updating in constructors only propagates one level,
  // whereas before-the-fact updating in compute/build functions propagates
  // multiple levels.
  if (!approxBuilds) {
    size_t num_cv  = currentVariables.cv(),  num_div = currentVariables.div(),
           num_drv = currentVariables.drv(), num_dsv = currentVariables.dsv();
    if (num_cv && num_cv == model.cv())
      model.continuous_variable_labels(
	currentVariables.continuous_variable_labels());
    if (num_div && num_div == model.div())
      model.discrete_int_variable_labels(
        currentVariables.discrete_int_variable_labels());
    if (num_drv && num_drv == model.drv())
      model.discrete_real_variable_labels(
        currentVariables.discrete_real_variable_labels());
    if (num_dsv && num_dsv == model.dsv())
      model.discrete_string_variable_labels(
        currentVariables.discrete_string_variable_labels());
  }

  // linear constraints
  if ( ( userDefinedConstraints.num_linear_ineq_constraints() || 
	 userDefinedConstraints.num_linear_eq_constraints() ) &&
       currentVariables.cv()  == model.cv()  &&
       currentVariables.div() == model.div() &&
       currentVariables.drv() == model.drv() ) {
    model.linear_ineq_constraint_coeffs(
      userDefinedConstraints.linear_ineq_constraint_coeffs());
    model.linear_ineq_constraint_lower_bounds(
      userDefinedConstraints.linear_ineq_constraint_lower_bounds());
    model.linear_ineq_constraint_upper_bounds(
      userDefinedConstraints.linear_ineq_constraint_upper_bounds());

    model.linear_eq_constraint_coeffs(
      userDefinedConstraints.linear_eq_constraint_coeffs());
    model.linear_eq_constraint_targets(
      userDefinedConstraints.linear_eq_constraint_targets());
  }

  // nonlinear constraints
  if (userDefinedConstraints.num_nonlinear_ineq_constraints()) {
    model.nonlinear_ineq_constraint_lower_bounds(
      userDefinedConstraints.nonlinear_ineq_constraint_lower_bounds());
    model.nonlinear_ineq_constraint_upper_bounds(
      userDefinedConstraints.nonlinear_ineq_constraint_upper_bounds());
  }
  if (userDefinedConstraints.num_nonlinear_eq_constraints())
    model.nonlinear_eq_constraint_targets(
      userDefinedConstraints.nonlinear_eq_constraint_targets());

  short active_view = currentVariables.view().first;
  if (active_view == RELAXED_ALL || active_view == MIXED_ALL)
    return;

  // update model with inactive currentVariables/userDefinedConstraints data.
  // For efficiency, we avoid doing this on every evaluation, instead calling
  // it from a pre-execution initialization context (initialize_mapping()).
  size_t num_icv  = currentVariables.icv(),  num_idiv = currentVariables.idiv(),
         num_idrv = currentVariables.idrv(), num_idsv = currentVariables.idsv();
  if (num_icv && num_icv == model.icv()) {
    model.inactive_continuous_variables(
      currentVariables.inactive_continuous_variables());
    model.inactive_continuous_lower_bounds(
      userDefinedConstraints.inactive_continuous_lower_bounds());
    model.inactive_continuous_upper_bounds(
      userDefinedConstraints.inactive_continuous_upper_bounds());
    if (!approxBuilds)
      model.inactive_continuous_variable_labels(
        currentVariables.inactive_continuous_variable_labels());
  }
  if (num_idiv && num_idiv == model.idiv()) {
    model.inactive_discrete_int_variables(
      currentVariables.inactive_discrete_int_variables());
    model.inactive_discrete_int_lower_bounds(
      userDefinedConstraints.inactive_discrete_int_lower_bounds());
    model.inactive_discrete_int_upper_bounds(
      userDefinedConstraints.inactive_discrete_int_upper_bounds());
    if (!approxBuilds)
      model.inactive_discrete_int_variable_labels(
        currentVariables.inactive_discrete_int_variable_labels());
  }
  if (num_idrv && num_idrv == model.idrv()) {
    model.inactive_discrete_real_variables(
      currentVariables.inactive_discrete_real_variables());
    model.inactive_discrete_real_lower_bounds(
      userDefinedConstraints.inactive_discrete_real_lower_bounds());
    model.inactive_discrete_real_upper_bounds(
      userDefinedConstraints.inactive_discrete_real_upper_bounds());
    if (!approxBuilds)
      model.inactive_discrete_real_variable_labels(
        currentVariables.inactive_discrete_real_variable_labels());
  }
  if (num_idsv && num_idsv == model.idsv()) {
    model.inactive_discrete_string_variables(
      currentVariables.inactive_discrete_string_variables());
    if (!approxBuilds)
      model.inactive_discrete_string_variable_labels(
        currentVariables.inactive_discrete_string_variable_labels());
  }
}


void NonHierarchSurrModel::update_model(Model& model)
{
  // update model with currentVariables/userDefinedConstraints data.  In the
  // hierarchical case, the variables view in LF/HF models correspond to the
  // currentVariables view.  Note: updating the bounds is not strictly necessary
  // in common usage for the HF model (when a single model evaluated only at the
  // TR center), but is needed for the LF model and could be relevant in cases
  // where the HF model involves additional surrogates/nestings.

  // active variable vals/bnds (active labels, inactive vals/bnds/labels, and
  // linear/nonlinear constraint coeffs/bnds updated in init_model())
  if (currentVariables.cv()) {
    model.continuous_variables(currentVariables.continuous_variables());
    model.continuous_lower_bounds(
      userDefinedConstraints.continuous_lower_bounds());
    model.continuous_upper_bounds(
      userDefinedConstraints.continuous_upper_bounds());
  }
  if (currentVariables.div()) {
    model.discrete_int_variables(currentVariables.discrete_int_variables());
    model.discrete_int_lower_bounds(
      userDefinedConstraints.discrete_int_lower_bounds());
    model.discrete_int_upper_bounds(
      userDefinedConstraints.discrete_int_upper_bounds());
  }
  if (currentVariables.drv()) {
    model.discrete_real_variables(currentVariables.discrete_real_variables());
    model.discrete_real_lower_bounds(
      userDefinedConstraints.discrete_real_lower_bounds());
    model.discrete_real_upper_bounds(
      userDefinedConstraints.discrete_real_upper_bounds());
  }
  if (currentVariables.dsv())
    model.discrete_string_variables(
      currentVariables.discrete_string_variables());
}


void NonHierarchSurrModel::update_from_model(Model& model)
{
  // update complement of active currentVariables using model data.

  // Note: this approach makes a strong assumption about non-active variable
  // consistency, which is limiting.  Better to perform an individual variable
  // mapping (e.g., solution control) when needed and allow for a different
  // ADV position.

  // active variable vals/bnds (active labels, inactive vals/bnds/labels, and
  // linear/nonlinear constraint coeffs/bnds updated in init_model())

  // *** TO DO: make this robust to differing inactive parameterizations using 
  // tag lookups.  Omit mappings for failed lookups.

  const Variables&   vars = model.current_variables();
  const Constraints& cons = model.user_defined_constraints();

  const RealVector& acv = vars.all_continuous_variables();
  StringMultiArrayConstView acv_labels = vars.all_continuous_variable_labels();
  const RealVector& acv_l_bnds = cons.all_continuous_lower_bounds();
  const RealVector& acv_u_bnds = cons.all_continuous_upper_bounds();
  StringMultiArrayConstView cv_acv_labels
    = currentVariables.all_continuous_variable_labels();
  size_t i, index, cv_begin = vars.cv_start(), num_cv = vars.cv(),
    cv_end = cv_begin + num_cv, num_acv = vars.acv();
  for (i=0; i<cv_begin; ++i) {
    index = find_index(cv_acv_labels, acv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_continuous_variable(acv[i], index);
      userDefinedConstraints.all_continuous_lower_bound(acv_l_bnds[i], index);
      userDefinedConstraints.all_continuous_upper_bound(acv_u_bnds[i], index);
    }
  }
  for (i=cv_end; i<num_acv; ++i) {
    index = find_index(cv_acv_labels, acv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_continuous_variable(acv[i], index);
      userDefinedConstraints.all_continuous_lower_bound(acv_l_bnds[i], index);
      userDefinedConstraints.all_continuous_upper_bound(acv_u_bnds[i], index);
    }
  }

  const IntVector& adiv = vars.all_discrete_int_variables();
  StringMultiArrayConstView adiv_labels
    = vars.all_discrete_int_variable_labels();
  const IntVector& adiv_l_bnds = cons.all_discrete_int_lower_bounds();
  const IntVector& adiv_u_bnds = cons.all_discrete_int_upper_bounds();
  StringMultiArrayConstView cv_adiv_labels
    = currentVariables.all_discrete_int_variable_labels();
  size_t div_begin = vars.div_start(), num_div = vars.div(),
    div_end = div_begin + num_div, num_adiv = vars.adiv();
  for (i=0; i<div_begin; ++i) {
    index = find_index(cv_adiv_labels, adiv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_int_variable(adiv[i], index);
      userDefinedConstraints.all_discrete_int_lower_bound(adiv_l_bnds[i],index);
      userDefinedConstraints.all_discrete_int_upper_bound(adiv_u_bnds[i],index);
    }
  }
  for (i=div_end; i<num_adiv; ++i) {
    index = find_index(cv_adiv_labels, adiv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_int_variable(adiv[i], index);
      userDefinedConstraints.all_discrete_int_lower_bound(adiv_l_bnds[i],index);
      userDefinedConstraints.all_discrete_int_upper_bound(adiv_u_bnds[i],index);
    }
  }

  size_t dsv_begin = vars.dsv_start(), num_dsv = vars.dsv(),
    dsv_end = dsv_begin + num_dsv, num_adsv = vars.adsv();
  StringMultiArrayConstView adsv = vars.all_discrete_string_variables();
  StringMultiArrayConstView adsv_labels
    = vars.all_discrete_string_variable_labels();
  StringMultiArrayConstView cv_adsv_labels
    = currentVariables.all_discrete_string_variable_labels();
  for (i=0; i<dsv_begin; ++i) {
    index = find_index(cv_adsv_labels, adsv_labels[i]);
    if (index != _NPOS)
      currentVariables.all_discrete_string_variable(adsv[i], index);
  }
  for (i=dsv_end; i<num_adsv; ++i) {
    index = find_index(cv_adsv_labels, adsv_labels[i]);
    if (index != _NPOS)
      currentVariables.all_discrete_string_variable(adsv[i], index);
  }

  const RealVector& adrv = vars.all_discrete_real_variables();
  StringMultiArrayConstView adrv_labels
    = vars.all_discrete_real_variable_labels();
  const RealVector& adrv_l_bnds = cons.all_discrete_real_lower_bounds();
  const RealVector& adrv_u_bnds = cons.all_discrete_real_upper_bounds();
  StringMultiArrayConstView cv_adrv_labels
    = currentVariables.all_discrete_real_variable_labels();
  size_t drv_begin = vars.drv_start(), num_drv = vars.drv(),
    drv_end = drv_begin + num_drv, num_adrv = vars.adrv();
  for (i=0; i<drv_begin; ++i) {
    index = find_index(cv_adrv_labels, adrv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_real_variable(adrv[i], index);
      userDefinedConstraints.all_discrete_real_lower_bound(adrv_l_bnds[i],
							   index);
      userDefinedConstraints.all_discrete_real_upper_bound(adrv_u_bnds[i],
							   index);
    }
  }
  for (i=drv_end; i<num_adrv; ++i) {
    index = find_index(cv_adrv_labels, adrv_labels[i]);
    if (index != _NPOS) {
      currentVariables.all_discrete_real_variable(adrv[i], index);
      userDefinedConstraints.all_discrete_real_lower_bound(adrv_l_bnds[i],
							   index);
      userDefinedConstraints.all_discrete_real_upper_bound(adrv_u_bnds[i],
							   index);
    }
  }
}

} // namespace Dakota
