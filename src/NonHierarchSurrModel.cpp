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
  SurrogateModel(problem_db),
  //corrOrder(problem_db.get_short("model.surrogate.correction_order")),
  //correctionMode(SINGLE_CORRECTION),
  sameModelInstance(false), sameInterfaceInstance(false),
  mfPrecedence(true), modeKeyBufferSize(0)
{
  // NonHierarchical surrogate models pass through numerical derivatives
  supportsEstimDerivs = false;
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");

  const String& truth_model_ptr
    = problem_db.get_string("model.surrogate.actual_model_pointer");
  const StringArray& unordered_model_ptrs
    = problem_db.get_sa("model.surrogate.ensemble_model_pointers");

  size_t i, num_approx = unordered_model_ptrs.size(),
    model_index = problem_db.get_db_model_node(); // for restoration

  unorderedModels.resize(num_approx);
  for (i=0; i<num_approx; ++i) {
    problem_db.set_db_model_nodes(unordered_model_ptrs[i]);
    unorderedModels[i] = problem_db.get_model();
    check_submodel_compatibility(unorderedModels[i]);
  }

  problem_db.set_db_model_nodes(truth_model_ptr);
  truthModel = problem_db.get_model();
  check_submodel_compatibility(truthModel);

  problem_db.set_db_model_nodes(model_index); // restore

  assign_default_keys();
}


void NonHierarchSurrModel::assign_default_keys()
{
  // default key data values, to be overridden at run time
  unsigned short id = 0, num_approx = unorderedModels.size();
  if (multifidelity()) { // first and last model form (no soln levels)
    truthModelKey = Pecos::ActiveKey(id, Pecos::RAW_DATA, num_approx, SZ_MAX);
    //if (responseMode == AGGREGATED_MODELS) {
      unorderedModelKeys.resize(num_approx);
      for (unsigned short i=0; i<num_approx; ++i)
	unorderedModelKeys[i]
	  = Pecos::ActiveKey(id, Pecos::RAW_DATA, i, SZ_MAX);
    //}
  }
  else if (multilevel()) { // first and last solution level (last model)
    size_t truth_soln_lev = truthModel.solution_levels(),
      truth_index = truth_soln_lev - 1;
    truthModelKey
      = Pecos::ActiveKey(id, Pecos::RAW_DATA, num_approx, truth_index);
    //if (responseMode == AGGREGATED_MODELS) {
      unorderedModelKeys.resize(truth_index);
      for (size_t i=0; i<truth_index; ++i)
	unorderedModelKeys[i]
	  = Pecos::ActiveKey(id, Pecos::RAW_DATA, num_approx, i);
    //}
  }
  activeKey.aggregate_keys(truthModelKey, unorderedModelKeys,
			   Pecos::RAW_DATA); // no data reduction

  if (parallelLib.mpirun_flag()) {
    MPIPackBuffer send_buff;  short mode;
    send_buff << mode << activeKey; // serve_run() recvs single | aggregate key
    modeKeyBufferSize = send_buff.size();
  }

  check_model_interface_instance();
}


void NonHierarchSurrModel::check_submodel_compatibility(const Model& sub_model)
{
  bool err1 = check_active_variables(sub_model),
       err2 = check_inactive_variables(sub_model),
       err3 = check_response_qoi(sub_model);

  if (err1 || err2 || err3)
    abort_handler(MODEL_ERROR);
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
      num_approx = unorderedModels.size();

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

    for (i=0; i<num_approx; ++i) {
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

    switch (responseMode) {

    case BYPASS_SURROGATE: {
      truthModel.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = truthModel.asynch_flag();
      evaluationCapacity = truthModel.evaluation_capacity();
      break;
    }

    case AGGREGATED_MODELS: {
      size_t i, num_approx = unorderedModels.size(); int cap_i;
      asynchEvalFlag = false; evaluationCapacity = 1;

      for (i=0; i<num_approx; ++i) {
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

      break;
    }
    }
  }
}


void NonHierarchSurrModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  if (recurse_flag) {

    size_t i, num_approx = unorderedModels.size();
    bool extra_deriv_config = true;//(responseMode == UNCORRECTED_SURROGATE ||
                                   // responseMode == BYPASS_SURROGATE ||
                                   // responseMode == AUTO_CORRECTED_SURROGATE);
    for (i=0; i<num_approx; ++i) {
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
  size_t i, num_approx = unorderedModels.size();
  for (i=0; i<num_approx; ++i) {
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
  size_t i, num_approx = unorderedModels.size();
  for (i=0; i<num_approx; ++i)
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
  //if (sameModelInstance) assign_surrogate_key();
  //lf_model.evaluate(temp_set);
  //const Response& lo_fi_response = lf_model.current_response();

  Model& hf_model = truth_model();
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    hf_model.eval_tag_prefix(eval_tag);
  }

  // set NonHierarchSurrModel parallelism mode to HF model
  component_parallel_mode(num_models);

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

  currentResponse.active_set(set);
  size_t num_approx = unorderedModels.size();
  switch (responseMode) {
  case AGGREGATED_MODELS: {
    // define eval reqmts, with unorderedModels followed by truthModel at end
    Short2DArray indiv_asv;
    asv_split(set.request_vector(), indiv_asv);
    size_t i, num_models = indiv_asv.size();

    ActiveSet set_i(set); // copy DVV
    for (i=0; i<num_models; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      if (test_asv(asv_i)) {
	Model& model_i = (i<num_approx) ? unorderedModels[i] : truthModel;
	component_parallel_mode(i+1); // index to id (0 is reserved)
	if (sameModelInstance) assign_key(i);
	else                   update_model(model_i);
	set_i.request_vector(asv_i);
	model_i.evaluate(set_i);
	// insert i-th contribution to currentResponse asrv/fns/grads/hessians
	insert_response(model_i.current_response(), i, currentResponse);
      }
    }
    break;
  }
  case BYPASS_SURROGATE:
    if (set.request_vector().size() != qoi()) {
      Cerr << "Error: wrong ASV size for BYPASS_SURROGATE mode in "
	   << "NonHierarchSurrModel::derived_evaluate()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    component_parallel_mode(num_approx+1); // truth model id
    if (sameModelInstance) assign_key(truthModelKey);
    else                   update_model(truthModel);
    truthModel.evaluate(set);
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

  // NonHierarchSurrModel's asynchEvalFlag is set if any model supports
  // asynchronous, resulting in use of derived_evaluate_nowait().
  // To manage general case of mixed asynch, launch nonblocking evals first,
  // followed by blocking evals.

  // For notes on repetitive use of assign_key(), see derived_evaluate() above

  switch (responseMode) {
  case AGGREGATED_MODELS: {
    // define eval reqmts, with unorderedModels followed by truthModel at end
    Short2DArray indiv_asv;
    asv_split(set.request_vector(), indiv_asv);
    size_t i, num_models = indiv_asv.size(),
      num_approx = unorderedModels.size();
    ActiveSet set_i(set); // copy DVV

    // first pass for nonblocking models
    for (i=0; i<num_models; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      Model& model_i = (i<num_approx) ? unorderedModels[i] : truthModel;
      if (model_i.asynch_flag() && test_asv(asv_i)) {
	if (sameModelInstance) assign_key(i);
	else                   update_model(model_i);
	set_i.request_vector(asv_i);
	model_i.evaluate_nowait(set_i);
	modelIdMap[i][model_i.evaluation_id()] = surrModelEvalCntr;
      }
    }
    // second pass for blocking models
    for (i=0; i<num_models; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      Model& model_i = (i<num_approx) ? unorderedModels[i] : truthModel;
      if (!model_i.asynch_flag() && test_asv(asv_i)) {
	component_parallel_mode(i+1); // model id (0 is reserved)
	if (sameModelInstance) assign_key(i);
	else                   update_model(model_i);
	set_i.request_vector(asv_i);
	model_i.evaluate(set_i);
	cachedRespMaps[i][surrModelEvalCntr]
	  = model_i.current_response().copy();
      }
    }
    break;
  }
  case BYPASS_SURROGATE:
    if (set.request_vector().size() != qoi()) {
      Cerr << "Error: wrong ASV size for BYPASS_SURROGATE mode in "
	   << "NonHierarchSurrModel::derived_evaluate()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    if (sameModelInstance) assign_key(truthModelKey);
    else                   update_model(truthModel);
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

  if (sameModelInstance || sameInterfaceInstance ||
      count_id_maps(modelIdMap) <= 1) { // 1 queue: blocking synch
    IntResponseMapArray model_resp_map_rekey(modelIdMap.size());
    derived_synchronize_sequential(model_resp_map_rekey, true);
    derived_synchronize_combine(model_resp_map_rekey, surrResponseMap, true);
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

  IntResponseMapArray model_resp_map_rekey(modelIdMap.size());
  derived_synchronize_sequential(model_resp_map_rekey, false);
  derived_synchronize_combine(model_resp_map_rekey, surrResponseMap, false);

  return surrResponseMap;
}


void NonHierarchSurrModel::
derived_synchronize_sequential(IntResponseMapArray& model_resp_maps_rekey,
			       bool block)
{
  size_t i, num_models = model_resp_maps_rekey.size(),
     num_approx_models = unorderedModels.size();
  for (i=0; i<num_models; ++i) {
    Model& model = (i < unorderedModels.size()) ?
      unorderedModels[i] : truthModel;
    IntIntMap& model_id_map = modelIdMap[i];
    IntResponseMap& model_resp_map = model_resp_maps_rekey[i];
    if (!model_id_map.empty()) { // synchronize evals for i-th Model
      component_parallel_mode(i+1); // index to id (0 is reserved)
      rekey_synch(model, block, model_id_map, model_resp_map);
    }
    // add cached evals from:
    // (a) recovered asynch evals that could not be returned since other model
    //     eval portions were still pending, or
    // (b) synchronous model evals performed within evaluate_nowait()
    IntResponseMap& cached_resp_map = cachedRespMaps[i];
    model_resp_map.insert(cached_resp_map.begin(), cached_resp_map.end());
    cached_resp_map.clear(); // clear map but leave outer array
  }
}


void NonHierarchSurrModel::derived_synchronize_competing()
{
  // in this case, we don't want to starve either LF or HF scheduling by
  // blocking on one or the other --> leverage derived_synchronize_nowait()
  IntResponseMap aggregated_map; // accumulate surrResponseMap returns
  while (test_id_maps(modelIdMap)) {
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
derived_synchronize_combine(const IntResponseMapArray& model_resp_maps,
                            IntResponseMap& combined_resp_map, bool block)
{
  // -----------------------------------
  // perform IntResponseMap aggregations
  // -----------------------------------

  switch (responseMode) {
  case AGGREGATED_MODELS: {
    // loop over model_resp_maps and insert() into offset position.  Notes:
    // > cachedRespMaps have been inserted into model_resp_maps
    // > rekey_synch() has migrated from indiv model ids to surrModelEvalCntr

    size_t i, num_models = model_resp_maps.size();  IntRespMCIter r_cit;
    if (block) { // all model evaluation contributions are available
      for (i=0; i<num_models; ++i) {
	const IntResponseMap& resp_map = model_resp_maps[i];
	for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit)
	  insert_response(r_cit->second, i,
			  combined_resp_map[r_cit->first]); // already rekeyed
      }
    }
    else { // manage partial results across set of models

      // assemble set of aggregate ids which still have pending contributions
      // (only pending jobs remain in modelIdMap after nonblocking synch)
      IntSet pending_ids;  IntIntMCIter id_it;
      for (i=0; i<num_models; ++i) {
	const IntIntMap& id_map_i = modelIdMap[i];
	for (id_it=id_map_i.begin(); id_it!=id_map_i.end(); ++id_it)
	  pending_ids.insert(id_it->second); // duplicates ignored
      }

      // process completed job sets or reinsert partial results into cache
      // Approach 1: repeated pending id lookups on innermost loop
      //for (i=0; i<num_models; ++i) {
      //  const IntResponseMap&        resp_map = model_resp_maps[i];
      //  const IntResponseMap& cached_resp_map =  cachedRespMaps[i];
      //  for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit) {
      //    eval_id = r_cit->first; // already rekeyed to NonHier response id
      //    // test for completion of each aggregate key
      //    if (pending_ids.find(eval_id) == pending_ids.end()) // no pending
      //      insert_response(r_cit->second, i, combined_resp_map[eval_id]);
      //    else // return to i-th model cache
      //      cached_map_i[eval_id] = r_cit->second;
      //  }
      //}
      // Approach 2: one pending id traversal for each resp map traversal
      int eval_id, pending_id;  ISIter p_it;
      for (i=0; i<num_models; ++i) {
	const IntResponseMap&  resp_map = model_resp_maps[i];
        IntResponseMap& cached_resp_map =  cachedRespMaps[i];
	p_it = pending_ids.begin();
	pending_id = (p_it == pending_ids.end()) ? INT_MAX : *p_it;
	for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit) {
	  eval_id = r_cit->first; // already rekeyed to NonHier response id
	  while (eval_id > pending_id) {
	    ++p_it;
	    pending_id = (p_it == pending_ids.end()) ? INT_MAX : *p_it;
	  }
	  if (eval_id < pending_id)
	    insert_response(r_cit->second, i, combined_resp_map[eval_id]);
	  else // eval_id has pending contributions; return to i-th model cache
	    cached_resp_map[eval_id] = r_cit->second;
	}
      }
      // Approach 3: use array of r_cit's to advance each resp map in
      // coordination with single pending id advancement (overkill)
    }
    break;
  }
  case BYPASS_SURROGATE:
    combined_resp_map = model_resp_maps[0]; // one truth model
    //std::swap(combined_resp_map, model_resp_maps[0]); // can't swap w/ const
    break;
  }
}


void NonHierarchSurrModel::resize_response(bool use_virtual_counts)
{
  size_t num_truth = (use_virtual_counts) ?
    truthModel.qoi() : // allow models to consume lower-level aggregations
    truthModel.response_size(); // raw counts align w/ currentResponse raw count

  switch (responseMode) {
  case AGGREGATED_MODELS:
    numFns = (unorderedModels.size() + 1) * num_truth;  break;
  case BYPASS_SURROGATE:
    numFns = num_truth;  break;
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


void NonHierarchSurrModel::component_parallel_mode(short model_id)
{
  // This implementation differs from others that accept a par_mode enum.  Here
  // we still support the virtual API of passing a short, but we reinterpret as
  // model id (not model_index since 0 is reserved for use by stop_servers()).
  // This essentially extends from the pair of {SURROGATE,TRUTH}_MODEL_MODE to
  // an open-ended number of models.

  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // componentParallelKey may not be necessary in the case where either all
  // models are active for a set of samples (AGGREGATED_MODELS mode for which
  // any drop outs are managed by ASV's) or only one model is active
  // (BYPASS_SURROGATE mode)

  // -----------------------------
  // terminate previous serve mode (if active)
  // -----------------------------
  // TO DO: restarting servers for a change in soln control index w/o change
  // in model may be overkill (send of state vars in vars buffer sufficient?)
  if (componentParallelMode != model_id || componentParallelKey != activeKey) {
    //Pecos::ActiveKey old_truth;  std::vector<Pecos::ActiveKey> old_surr;
    //componentParallelKey.extract_keys(old_truth, old_surr);
    //switch (componentParallelMode) {
    //case SURROGATE_MODEL_MODE:  stop_model(old_surr[model_id][1]);  break;
    //case     TRUTH_MODEL_MODE:  stop_model(old_truth[1]);  break;
    //}
    stop_model(componentParallelMode);

    // -----------------------
    // activate new serve mode: matches NonHierarchSurrModel::serve_run(pl_iter)
    // -----------------------
    // These bcasts match the outer parallel context (pl_iter).
    if (modelPCIter->mi_parallel_level_defined(miPLIndex)) {
      const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
      if (mi_pl.server_communicator_size() > 1) {
	parallelLib.bcast(model_id, mi_pl);
	if (model_id) { // send model index state corresponding to active mode
	  MPIPackBuffer send_buff;
	  send_buff << responseMode << activeKey;
	  //int buffer_len = send_buff.size();
	  //parallelLib.bcast(buffer_len, mi_pl); // avoid this overhead
	  parallelLib.bcast(send_buff, mi_pl);
	}
      }
    }
  }

  componentParallelMode = model_id;  componentParallelKey = activeKey;
}


void NonHierarchSurrModel::
serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false); // don't recurse

  // match communication from NonHierarchSurrModel::component_parallel_mode()
  componentParallelMode = 1; // dummy value for entering loop
  while (componentParallelMode) { // model id is bcast, so 0 is exit code
    parallelLib.bcast(componentParallelMode, *pl_iter); // outer context
    if (componentParallelMode) {
      // Local size estimation for recv buffer can't simply use activeKey
      // since previous key may be a singleton from bypass_surrogate_mode():
      //MPIPackBuffer send_buff;  send_buff << responseMode << activeKey;
      //int buffer_len = send_buff.size();
      //
      // This approach works, but avoid the additional bcast overhead:
      //parallelLib.bcast(buffer_len, *pl_iter);

      // recv model state from NonHierarchSurrModel::component_parallel_mode()
      MPIUnpackBuffer recv_buffer(modeKeyBufferSize);
      parallelLib.bcast(recv_buffer, *pl_iter);
      recv_buffer >> responseMode >> activeKey; // replace previous/initial key

      active_model_key(activeKey);
      size_t m_index = componentParallelMode - 1; // id to index
      Model& model = (m_index < unorderedModels.size()) ?
	unorderedModels[m_index] : truthModel;
      model.serve_run(pl_iter, max_eval_concurrency);
    }
  }
}

} // namespace Dakota
