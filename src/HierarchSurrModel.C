/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright (c) 2010, Sandia National Laboratories.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrModel
//- Description: Implementation code for the HierarchSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "HierarchSurrModel.H"
#include "ProblemDescDB.H"

static const char rcsId[]="@(#) $Id: HierarchSurrModel.C 6656 2010-02-26 05:20:48Z mseldre $";


using namespace std;

namespace Dakota {

HierarchSurrModel::HierarchSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db), hierModelEvalCntr(0),
  highFidRefResponse(currentResponse.copy())
{
  // Correction is required in the hierarchical case (since without
  // correction, all highFidelityModel evaluations are wasted).  Omission
  // of a correction type should be prevented by the input specification.
  if (correctionType.empty()) {
    Cerr << "Error: correction is required with model hierarchies." << endl;
    abort_handler(-1);
  }

  // Hierarchical surrogate models pass through numerical derivatives
  supports_derivative_estimation(false);
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");

  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");

  const String& low_fid_model_ptr
    = problem_db.get_string("model.surrogate.low_fidelity_model_pointer");
  const String& high_fid_model_ptr
    = problem_db.get_string("model.surrogate.high_fidelity_model_pointer");

  size_t model_index = problem_db.get_db_model_node(); // for restoration

  problem_db.set_db_model_nodes(low_fid_model_ptr);
  lowFidelityModel = problem_db.get_model();
  check_submodel_compatibility(lowFidelityModel);

  problem_db.set_db_model_nodes(high_fid_model_ptr);
  highFidelityModel = problem_db.get_model();
  check_submodel_compatibility(highFidelityModel);

  // Enforce additional sub-model compatibility: views must be identical
  const pair<short,short>& cv_view = currentVariables.view();
  if (cv_view !=  lowFidelityModel.current_variables().view() ||
      cv_view != highFidelityModel.current_variables().view()) {
    Cerr << "Error: variable views in hierarchical models must be identical."
	 << endl;
    abort_handler(-1);
  }

  problem_db.set_db_model_nodes(model_index); // restore
}


void HierarchSurrModel::build_approximation()
{
  Cout << "\n>>>>> Building hierarchical approximation.\n";

  // perform the eval for the low fidelity model
  // NOTE: For SBO, the low fidelity eval is performed externally and its
  // response is passed into compute_correction.
  // -->> move lowFidelityModel out and restructure if(!approxBuilds)
  //ActiveSet temp_set = lowFidelityModel.current_response().active_set();
  //temp_set.request_values(1);
  //lowFidelityModel.compute_response(temp_set);
  //const Response& lo_fi_response = lowFidelityModel.current_response();

  // set HierarchSurrModel parallelism mode to highFidelityModel
  component_parallel_mode(HF_MODEL);

  // update highFidelityModel with current variable values/bounds/labels
  update_model(highFidelityModel);

  // store inactive variable values for use in determining whether an
  // automatic rebuild of an approximation is required
  // (reference{C,D}{L,U}Bnds are not needed in the hierarchical case)
  const Variables& hf_vars = highFidelityModel.current_variables();
  copy_data(hf_vars.inactive_continuous_variables(),    referenceICVars);
  copy_data(hf_vars.inactive_discrete_int_variables(),  referenceIDIVars);
  copy_data(hf_vars.inactive_discrete_real_variables(), referenceIDRVars);

  // compute the response for the high fidelity model
  short asv_val = 1; // correctionOrder == 0
  if (correctionOrder == 1)      // >= 1 && !hess_flag
    asv_val = 3;
  else if (correctionOrder == 2) // == 2 && hess_flag
    asv_val = 7;
  ShortArray total_asv(numFns, asv_val), hf_asv, lf_asv;
  asv_mapping(total_asv, hf_asv, lf_asv, true);
  ActiveSet hf_set = highFidRefResponse.active_set(); // copy
  hf_set.request_vector(hf_asv);
  highFidelityModel.compute_response(hf_set);
  highFidRefResponse.update(highFidelityModel.current_response());

  // could compute the correction to lowFidelityModel here, but rely on an
  // external call for consistency with DataFitSurr and to facilitate SBO logic.
  //compute_correction(highFidRefResponse, lo_fi_response, ...);

  Cout << "\n<<<<< Hierarchical approximation build completed.\n";
  approxBuilds++;
}


/*
bool HierarchSurrModel::
build_approximation(const RealVector& c_vars, const Response& response)
{
  // NOTE: this fn not currently used by SBO, but it could be.

  // Verify data content in incoming response
  const ShortArray& asrv = response.active_set_request_vector();
  bool data_complete = true;
  for (size_t i=0; i<numFns; i++)
    if ( ( correctionOrder == 2 && (asrv[i] & 7) != 7 ) ||
	 ( correctionOrder == 1 && (asrv[i] & 3) != 3 ) ||
	 ( correctionOrder == 0 && !(asrv[i] & 1) ) )
      data_complete = false;
  if (data_complete) {
    Cout << "\n>>>>> Updating hierarchical approximation.\n";

    // are these updates necessary?
    currentVariables.continuous_variables(c_vars);
    update_model(highFidelityModel);
    const Variables& hf_vars = highFidelityModel.current_variables();
    copy_data(hf_vars.inactive_continuous_variables(), referenceICVars);
    copy_data(hf_vars.inactive_discrete_variables(),   referenceIDVars);

    highFidRefResponse.update(response);

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


/** Compute the response synchronously using lowFidelityModel,
    highFidelityModel, or both (mixed case).  For the lowFidelityModel
    portion, compute the high fidelity response if needed with
    build_approximation(), and, if correction is active, correct the
    low fidelity results. */
void HierarchSurrModel::derived_compute_response(const ActiveSet& set)
{
  ++hierModelEvalCntr;

  ShortArray hi_fi_asv, lo_fi_asv;
  asv_mapping(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
  bool hi_fi_eval = !hi_fi_asv.empty(), lo_fi_eval = !lo_fi_asv.empty(),
    mixed_eval = (hi_fi_eval && lo_fi_eval);
  Response lo_fi_response, hi_fi_response; // don't use highFidRefResponse

  if (hi_fi_eval) { // rare case: use hiFi Model instead of loFi Model
    component_parallel_mode(HF_MODEL);
    update_model(highFidelityModel);
    ActiveSet hi_fi_set = set;
    hi_fi_set.request_vector(hi_fi_asv);
    highFidelityModel.compute_response(hi_fi_set);
    hi_fi_response = highFidelityModel.current_response(); // shared rep
    if (!mixed_eval) {
      currentResponse.active_set(hi_fi_set);
      currentResponse.update(hi_fi_response);
    }
  }

  if (lo_fi_eval) { // normal case: evaluation of lowFidelityModel

    // if build_approximation has not yet been called, call it now
    if (!approxBuilds)
      autoCorrection = true; // default if stand-alone use
    if (!approxBuilds || force_rebuild())
      build_approximation();

    component_parallel_mode(LF_MODEL);
    update_model(lowFidelityModel);
    ActiveSet lo_fi_set = set;
    lo_fi_set.request_vector(lo_fi_asv);
    lowFidelityModel.compute_response(lo_fi_set);
    const Response& lf_resp = lowFidelityModel.current_response();
    // LF response should not be corrected directly (see derived_synchronize())
    lo_fi_response = (autoCorrection) ? lf_resp.copy() : lf_resp;

    if (autoCorrection) {
      if (!correctionComputed)
	compute_correction(highFidRefResponse, lo_fi_response,
			   currentVariables.continuous_variables());
      apply_correction(lo_fi_response, currentVariables.continuous_variables());
    }

    if (!mixed_eval) {
      currentResponse.active_set(lo_fi_set);
      currentResponse.update(lo_fi_response);
    }
  }

  if (mixed_eval) {
    currentResponse.active_set(set);
    response_mapping(hi_fi_response, lo_fi_response, currentResponse);
  }
}


/** Compute the response asynchronously using lowFidelityModel,
    highFidelityModel, or both (mixed case).  For the lowFidelityModel
    portion, compute the high fidelity response with build_approximation()
    (for correcting the low fidelity results in derived_synchronize() and
    derived_synchronize_nowait()) if not performed previously. */
void HierarchSurrModel::derived_asynch_compute_response(const ActiveSet& set)
{
  ++hierModelEvalCntr;

  ShortArray hi_fi_asv, lo_fi_asv;
  asv_mapping(set.request_vector(), hi_fi_asv, lo_fi_asv, false);

  if (!hi_fi_asv.empty()) { // rare case: use hiFi Model instead of loFi Model
    // don't need to set component parallel mode since this only queues the job
    update_model(highFidelityModel);
    ActiveSet hi_fi_set = set;
    hi_fi_set.request_vector(hi_fi_asv);
    highFidelityModel.asynch_compute_response(hi_fi_set);
    // store map from HF eval id to HierarchSurrModel id
    truthIdMap[highFidelityModel.evaluation_id()] = hierModelEvalCntr;
  }

  if (!lo_fi_asv.empty()) { // normal case: evaluation of lowFidelityModel

    // if build_approximation has not yet been called, call it now
    if (!approxBuilds)
      autoCorrection = true; // default if stand-alone use
    if (!approxBuilds || force_rebuild())
      build_approximation();

    // don't need to set component parallel mode since this only queues the job
    update_model(lowFidelityModel);
    ActiveSet lo_fi_set = set;
    lo_fi_set.request_vector(lo_fi_asv);
    lowFidelityModel.asynch_compute_response(lo_fi_set);
    if (autoCorrection)
      copy_data(currentVariables.continuous_variables(),
		rawCVarsMap[hierModelEvalCntr]);
    // store map from LF eval id to HierarchSurrModel id
    surrIdMap[lowFidelityModel.evaluation_id()] = hierModelEvalCntr;
  }
}


/** Blocking retrieval of asynchronous evaluations from lowFidelityModel,
    highFidelityModel, or both (mixed case).  For the lowFidelityModel
    portion, apply correction (if active) to each response in the array.
    derived_synchronize() is designed for the general case where
    derived_asynch_compute_response() may be inconsistent in its use
    of low fidelity evaluations, high fidelity evaluations, or both. */
const IntResponseMap& HierarchSurrModel::derived_synchronize()
{
  surrResponseMap.clear();
  bool hi_fi_evals = !truthIdMap.empty(), lo_fi_evals = !surrIdMap.empty();

  // synchronize highFidelityModel evals
  IntResponseMap hi_fi_resp_map_rekey;
  if (hi_fi_evals) {
    component_parallel_mode(HF_MODEL);
    const IntResponseMap& hi_fi_resp_map = highFidelityModel.synchronize();

    // update map keys to use hierModelEvalCntr
    IntResponseMap& hi_fi_resp_map_proxy
      = (lo_fi_evals) ? hi_fi_resp_map_rekey : surrResponseMap;
    for (IntRespMCIter r_cit = hi_fi_resp_map.begin();
	 r_cit != hi_fi_resp_map.end(); ++r_cit)
      hi_fi_resp_map_proxy[truthIdMap[r_cit->first]] = r_cit->second;
    truthIdMap.clear();

    // add cached truth evals (synchronized evals that could not be returned
    // since approx eval portions were still pending) for processing.
    for (IntRespMCIter r_cit = cachedTruthRespMap.begin();
	 r_cit != cachedTruthRespMap.end(); ++r_cit)
      hi_fi_resp_map_proxy[r_cit->first] = r_cit->second;
    cachedTruthRespMap.clear();

    if (!lo_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed and augmented by proxy
  }
  // synchronize lowFidelityModel evals
  IntResponseMap lo_fi_resp_map_rekey;
  if (lo_fi_evals) {
    component_parallel_mode(LF_MODEL);
    const IntResponseMap& lo_fi_resp_map = lowFidelityModel.synchronize();

    // update map keys to use hierModelEvalCntr
    IntResponseMap& lo_fi_resp_map_proxy
      = (hi_fi_evals) ? lo_fi_resp_map_rekey : surrResponseMap;
    for (IntRespMCIter r_cit = lo_fi_resp_map.begin();
	 r_cit != lo_fi_resp_map.end(); ++r_cit)
      lo_fi_resp_map_proxy[surrIdMap[r_cit->first]]
	= (autoCorrection) ? r_cit->second.copy() : r_cit->second;
    surrIdMap.clear();

    if (autoCorrection) {
      // Interface::rawResponseMap should _not_ be corrected directly since
      // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share
      // a responseRep -->> modifying rawResponseMap affects data_pairs.

      // if a correction has not been computed, compute it now
      if (!correctionComputed && !lo_fi_resp_map_proxy.empty())
	compute_correction(highFidRefResponse,
	  lo_fi_resp_map_proxy.begin()->second, rawCVarsMap.begin()->second);

      // Apply the correction.  A rawCVarsMap lookup is not needed since
      // rawCVarsMap and lo_fi_resp_map are complete and consistently ordered.
      IntRDVMIter v_it; IntRespMIter r_it;
      for (r_it  = lo_fi_resp_map_proxy.begin(), v_it = rawCVarsMap.begin();
	   r_it != lo_fi_resp_map_proxy.end(); ++r_it, ++v_it)
        apply_correction(r_it->second, v_it->second);//rawCVarsMap[r_it->first]
      rawCVarsMap.clear();
    }

    // add cached approx evals (synchronized evals that could not be returned
    // since truth eval portions were still pending) for processing.  Do not
    // correct them a second time.
    for (IntRespMCIter r_cit = cachedApproxRespMap.begin();
	 r_cit != cachedApproxRespMap.end(); ++r_cit)
      lo_fi_resp_map_proxy[r_cit->first] = r_cit->second;
    cachedApproxRespMap.clear();

    if (!hi_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed, corrected, and augmented by proxy
  }

  // mixed highFidelityModel and lowFidelityModel evals: both
  // hi_fi_resp_map_rekey and lo_fi_resp_map_rekey may be partial sets of evals
  Response empty_resp;
  IntRespMCIter hi_fi_it = hi_fi_resp_map_rekey.begin(),
                lo_fi_it = lo_fi_resp_map_rekey.begin();
  bool hi_fi_complete = false, lo_fi_complete = false;
  // process any combination of HF and LF completions
  while (!hi_fi_complete || !lo_fi_complete) {
    if (hi_fi_it == hi_fi_resp_map_rekey.end())
      hi_fi_complete = true;
    if (lo_fi_it == lo_fi_resp_map_rekey.end())
      lo_fi_complete = true;

    int hf_hier_model_eval_id = (hi_fi_complete) ? INT_MAX : hi_fi_it->first;
    int lf_hier_model_eval_id = (lo_fi_complete) ? INT_MAX : lo_fi_it->first;

    if (hf_hier_model_eval_id < lf_hier_model_eval_id) { // only HF available
      response_mapping(hi_fi_it->second, empty_resp,
		       surrResponseMap[hf_hier_model_eval_id]);
      ++hi_fi_it;
    }
    else if (lf_hier_model_eval_id < hf_hier_model_eval_id) {//only LF available
      response_mapping(empty_resp, lo_fi_it->second, 
		       surrResponseMap[lf_hier_model_eval_id]);
      ++lo_fi_it;
    }
    else if (!hi_fi_complete && !lo_fi_complete) { // both LF and HF available
      response_mapping(hi_fi_it->second, lo_fi_it->second,
		       surrResponseMap[hf_hier_model_eval_id]);
      ++hi_fi_it;
      ++lo_fi_it;
    }
  }

  return surrResponseMap;
}


/** Nonblocking retrieval of asynchronous evaluations from
    lowFidelityModel, highFidelityModel, or both (mixed case).  For
    the lowFidelityModel portion, apply correction (if active) to each
    response in the map.  derived_synchronize_nowait() is designed for
    the general case where derived_asynch_compute_response() may be
    inconsistent in its use of actual evals, approx evals, or both. */
const IntResponseMap& HierarchSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();
  bool hi_fi_evals = !truthIdMap.empty(), lo_fi_evals = !surrIdMap.empty();

  // synchronize highFidelityModel evals
  IntResponseMap hi_fi_resp_map_rekey;
  if (hi_fi_evals) {
    component_parallel_mode(HF_MODEL);
    const IntResponseMap& hi_fi_resp_map
      = highFidelityModel.synchronize_nowait();

    // update map keys to use hierModelEvalCntr
    IntResponseMap& hi_fi_resp_map_proxy
      = (lo_fi_evals) ? hi_fi_resp_map_rekey : surrResponseMap;
    for (IntRespMCIter r_cit = hi_fi_resp_map.begin();
	 r_cit != hi_fi_resp_map.end(); ++r_cit) {
      int hf_eval_id = r_cit->first;
      hi_fi_resp_map_proxy[truthIdMap[hf_eval_id]] = r_cit->second;
      if (!lo_fi_evals)
	truthIdMap.erase(hf_eval_id); // erase now prior to return below
    }

    // add cached truth evals (synchronized evals that could not be returned
    // since approx eval portions were still pending) for processing.
    for (IntRespMCIter r_cit = cachedTruthRespMap.begin();
	 r_cit != cachedTruthRespMap.end(); ++r_cit)
      hi_fi_resp_map_proxy[r_cit->first] = r_cit->second;
    cachedTruthRespMap.clear();

    if (!lo_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed and augmented by proxy
  }
  // synchronize lowFidelityModel evals
  IntResponseMap lo_fi_resp_map_rekey;
  if (lo_fi_evals) {
    component_parallel_mode(LF_MODEL);
    const IntResponseMap& lo_fi_resp_map
      = lowFidelityModel.synchronize_nowait();

    // update map keys to use hierModelEvalCntr
    IntResponseMap& lo_fi_resp_map_proxy
      = (hi_fi_evals) ? lo_fi_resp_map_rekey : surrResponseMap;
    for (IntRespMCIter r_cit = lo_fi_resp_map.begin();
	 r_cit != lo_fi_resp_map.end(); ++r_cit) {
      int lf_eval_id = r_cit->first;
      lo_fi_resp_map_proxy[surrIdMap[lf_eval_id]]
	= (autoCorrection) ? r_cit->second.copy() : r_cit->second;
      if (!hi_fi_evals)
	surrIdMap.erase(lf_eval_id);
    }

    if (autoCorrection) {
      // Interface::rawResponseMap should _not_ be corrected directly since
      // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share
      // a responseRep -->> modifying rawResponseMap affects data_pairs.

      // if a correction has not been computed, compute it now
      if (!correctionComputed && !lo_fi_resp_map_proxy.empty())
	compute_correction(highFidRefResponse,
	  lo_fi_resp_map_proxy.begin()->second, rawCVarsMap.begin()->second);

      // Apply the correction.  Must use rawCVarsMap lookup in this case since
      // rawCVarsMap is complete, but lo_fi_resp_map may not be.
      for (IntRespMIter r_it = lo_fi_resp_map_proxy.begin();
	   r_it != lo_fi_resp_map_proxy.end(); ++r_it) {
	int hier_eval_id = r_it->first;
        apply_correction(r_it->second, rawCVarsMap[hier_eval_id]);
        rawCVarsMap.erase(hier_eval_id);
      }
    }

    // add cached approx evals (synchronized evals that could not be returned
    // since truth eval portions were still pending) for processing.  Do not
    // correct them a second time.
    for (IntRespMCIter r_cit = cachedApproxRespMap.begin();
	 r_cit != cachedApproxRespMap.end(); ++r_cit)
      lo_fi_resp_map_proxy[r_cit->first] = r_cit->second;
    cachedApproxRespMap.clear();

    if (!hi_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed, corrected, and augmented by proxy
  }

  // mixed highFidelityModel and lowFidelityModel evals: both
  // hi_fi_resp_map_rekey and lo_fi_resp_map_rekey may be partial sets of evals
  Response empty_resp;
  IntRespMCIter hi_fi_it = hi_fi_resp_map_rekey.begin(),
                lo_fi_it = lo_fi_resp_map_rekey.begin();
  bool hi_fi_complete = false, lo_fi_complete = false;
  // invert truthIdMap and surrIdMap
  IntIntMap inverse_truth_id_map, inverse_surr_id_map;
  for (IntIntMCIter tim_it=truthIdMap.begin();
       tim_it!=truthIdMap.end(); ++tim_it)
    inverse_truth_id_map[tim_it->second] = tim_it->first;
  for (IntIntMCIter sim_it=surrIdMap.begin();
       sim_it!=surrIdMap.end(); ++sim_it)
    inverse_surr_id_map[sim_it->second] = sim_it->first;
  // process any combination of HF and LF completions
  while (!hi_fi_complete || !lo_fi_complete) {
    if (hi_fi_it == hi_fi_resp_map_rekey.end())
      hi_fi_complete = true;
    if (lo_fi_it == lo_fi_resp_map_rekey.end())
      lo_fi_complete = true;

    int hf_hier_model_eval_id = (hi_fi_complete) ? INT_MAX : hi_fi_it->first;
    int lf_hier_model_eval_id = (lo_fi_complete) ? INT_MAX : lo_fi_it->first;

    if (hf_hier_model_eval_id < lf_hier_model_eval_id) { // only HF available
      if (inverse_surr_id_map.count(hf_hier_model_eval_id))
	// response not complete: LF contribution not yet available
	cachedTruthRespMap[hf_hier_model_eval_id] = hi_fi_it->second;
      else { // there is no LF component to this response
	response_mapping(hi_fi_it->second, empty_resp,
			 surrResponseMap[hf_hier_model_eval_id]);
	truthIdMap.erase(inverse_truth_id_map[hf_hier_model_eval_id]);
      }
      ++hi_fi_it;
    }
    else if (lf_hier_model_eval_id < hf_hier_model_eval_id) {//only LF available
      if (inverse_truth_id_map.count(lf_hier_model_eval_id))
	// response not complete: HF contribution not yet available
	cachedApproxRespMap[lf_hier_model_eval_id] = lo_fi_it->second;
      else { // response complete: there is no HF contribution
	response_mapping(empty_resp, lo_fi_it->second, 
			 surrResponseMap[lf_hier_model_eval_id]);
	surrIdMap.erase(inverse_surr_id_map[lf_hier_model_eval_id]);
      }
      ++lo_fi_it;
    }
    else if (!hi_fi_complete && !lo_fi_complete) { // both LF and HF available
      response_mapping(hi_fi_it->second, lo_fi_it->second,
		       surrResponseMap[hf_hier_model_eval_id]);
      truthIdMap.erase(inverse_truth_id_map[hf_hier_model_eval_id]);
      surrIdMap.erase(inverse_surr_id_map[lf_hier_model_eval_id]);
      ++hi_fi_it;
      ++lo_fi_it;
    }
  }
  return surrResponseMap;
}


void HierarchSurrModel::component_parallel_mode(short mode)
{
  // mode may be correct, but can't guarantee active parallel configuration is
  // in synch
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // terminate previous serve mode (if active)
  if (componentParallelMode != mode) {
    if (componentParallelMode == LF_MODEL) {
      parallelLib.parallel_configuration_iterator(
        lowFidelityModel.parallel_configuration_iterator());
      const ParallelConfiguration& pc = parallelLib.parallel_configuration();
      if (parallelLib.si_parallel_level_defined() && 
	  pc.si_parallel_level().server_communicator_size() > 1)
	lowFidelityModel.stop_servers();
    }
    else if (componentParallelMode == HF_MODEL) {
      parallelLib.parallel_configuration_iterator(
        highFidelityModel.parallel_configuration_iterator());
      const ParallelConfiguration& pc = parallelLib.parallel_configuration();
      if (parallelLib.si_parallel_level_defined() && 
	  pc.si_parallel_level().server_communicator_size() > 1)
	highFidelityModel.stop_servers();
    }
  }

  // set ParallelConfiguration for new mode
  if (mode == HF_MODEL)
    parallelLib.parallel_configuration_iterator(
      highFidelityModel.parallel_configuration_iterator());
  else if (mode == LF_MODEL)
    parallelLib.parallel_configuration_iterator(
      lowFidelityModel.parallel_configuration_iterator());

  // retrieve new ParallelConfiguration data
  int new_iter_comm_size = 1;
  if (parallelLib.si_parallel_level_defined()) {
    const ParallelConfiguration& pc = parallelLib.parallel_configuration();
    new_iter_comm_size = pc.si_parallel_level().server_communicator_size();
  }

  // activate the new serve mode
  if (new_iter_comm_size > 1 && componentParallelMode != mode)
    parallelLib.bcast_i(mode);
  componentParallelMode = mode;
}


void HierarchSurrModel::update_model(Model& model)
{
  // update model with currentVariables/userDefinedConstraints data.  In the
  // hierarchical case, the variables view in low/highFidelityModel corresponds
  // to the currentVariables view.  Note: updating the bounds is not strictly
  // necessary in common usage for highFidelityModel (when a single model
  // evaluated only at the TR center), but is needed for lowFidelityModel and
  // could be relevant in cases where highFidelityModel involves additional
  // surrogates/nestings.

  // vars
  model.continuous_variables(currentVariables.continuous_variables());
  model.discrete_int_variables(currentVariables.discrete_int_variables());
  model.discrete_real_variables(currentVariables.discrete_real_variables());
  // bound constraints
  model.continuous_lower_bounds(
    userDefinedConstraints.continuous_lower_bounds());
  model.continuous_upper_bounds(
    userDefinedConstraints.continuous_upper_bounds());
  model.discrete_int_lower_bounds(
    userDefinedConstraints.discrete_int_lower_bounds());
  model.discrete_int_upper_bounds(
    userDefinedConstraints.discrete_int_upper_bounds());
  model.discrete_real_lower_bounds(
    userDefinedConstraints.discrete_real_lower_bounds());
  model.discrete_real_upper_bounds(
    userDefinedConstraints.discrete_real_upper_bounds());
  // linear constraints
  if (userDefinedConstraints.num_linear_ineq_constraints()) {
    model.linear_ineq_constraint_coeffs(
      userDefinedConstraints.linear_ineq_constraint_coeffs());
    model.linear_ineq_constraint_lower_bounds(
      userDefinedConstraints.linear_ineq_constraint_lower_bounds());
    model.linear_ineq_constraint_upper_bounds(
      userDefinedConstraints.linear_ineq_constraint_upper_bounds());
  }
  if (userDefinedConstraints.num_linear_eq_constraints()) {
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

  // Set the low/highFidelityModel variable descriptors with the variable
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
    // active not currently necessary, but included for completeness and 
    // consistency with global approximation case
    model.continuous_variable_labels(
      currentVariables.continuous_variable_labels());
    model.discrete_int_variable_labels(
      currentVariables.discrete_int_variable_labels());
    model.discrete_real_variable_labels(
      currentVariables.discrete_real_variable_labels());
    short active_view = currentVariables.view().first;
    if (active_view != MERGED_ALL && active_view != MIXED_ALL) {
      // inactive needed for Nested/Surrogate propagation
      model.inactive_continuous_variable_labels(
        currentVariables.inactive_continuous_variable_labels());
      model.inactive_discrete_int_variable_labels(
        currentVariables.inactive_discrete_int_variable_labels());
      model.inactive_discrete_real_variable_labels(
        currentVariables.inactive_discrete_real_variable_labels());
    }
  }
}

} // namespace Dakota
