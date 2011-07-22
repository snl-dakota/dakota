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


namespace Dakota {

HierarchSurrModel::HierarchSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db), hierModelEvalCntr(0),
  highFidRefResponse(currentResponse.copy())
{
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
  const std::pair<short,short>& cv_view = currentVariables.view();
  if (cv_view !=  lowFidelityModel.current_variables().view() ||
      cv_view != highFidelityModel.current_variables().view()) {
    Cerr << "Error: variable views in hierarchical models must be identical."
	 << std::endl;
    abort_handler(-1);
  }

  problem_db.set_db_model_nodes(model_index); // restore

  // Correction is required in the hierarchical case (since without
  // correction, all highFidelityModel evaluations are wasted).  Omission
  // of a correction type should be prevented by the input specification.
  const String& corr_type
    = problem_db.get_string("model.surrogate.correction_type");
  if (corr_type.empty()) {
    Cerr << "Error: correction is required with model hierarchies."<< std::endl;
    abort_handler(-1);
  }
  else // initialize the DiscrepancyCorrection instance
    deltaCorr.initialize(lowFidelityModel, surrogateFnIndices, corr_type,
      problem_db.get_short("model.surrogate.correction_order"));
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
  ShortArray total_asv(numFns, deltaCorr.data_order()), hf_asv, lf_asv;
  asv_mapping(total_asv, hf_asv, lf_asv, true);
  ActiveSet hf_set = highFidRefResponse.active_set(); // copy
  hf_set.request_vector(hf_asv);
  highFidelityModel.compute_response(hf_set);
  highFidRefResponse.update(highFidelityModel.current_response());

  // could compute the correction to lowFidelityModel here, but rely on an
  // external call for consistency with DataFitSurr and to facilitate SBO logic.
  //deltaCorr.compute(..., highFidRefResponse, lo_fi_response);

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
  bool data_complete = true; short corr_order = dataCorr.correction_order();
  for (size_t i=0; i<numFns; i++)
    if ( ( corr_order == 2 &&  (asrv[i] & 7) != 7 ) ||
	 ( corr_order == 1 &&  (asrv[i] & 3) != 3 ) ||
	 ( corr_order == 0 && !(asrv[i] & 1) ) )
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

  // define LF/HF evaluation requirements
  ShortArray hi_fi_asv, lo_fi_asv; bool hi_fi_eval, lo_fi_eval, mixed_eval;
  Response lo_fi_response, hi_fi_response; // don't use highFidRefResponse
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_mapping(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
    hi_fi_eval = !hi_fi_asv.empty(); lo_fi_eval = !lo_fi_asv.empty();
    mixed_eval = (hi_fi_eval && lo_fi_eval); break;
  case BYPASS_SURROGATE:
    hi_fi_eval = true; lo_fi_eval = false;   break;
  case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
    hi_fi_eval = lo_fi_eval = true;          break;
  }

  // ------------------------------
  // Compute high fidelity response
  // ------------------------------
  if (hi_fi_eval) {
    component_parallel_mode(HF_MODEL);
    update_model(highFidelityModel);
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet hi_fi_set = set; hi_fi_set.request_vector(hi_fi_asv);
      highFidelityModel.compute_response(hi_fi_set);
      if (mixed_eval)
	hi_fi_response = highFidelityModel.current_response(); // shared rep
      else {
	currentResponse.active_set(hi_fi_set);
	currentResponse.update(highFidelityModel.current_response());
      }
      break;
    }
    case BYPASS_SURROGATE:
      highFidelityModel.compute_response(set);
      currentResponse.active_set(set);
      currentResponse.update(highFidelityModel.current_response());
      break;
    case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
      highFidelityModel.compute_response(set);
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
    component_parallel_mode(LF_MODEL);
    update_model(lowFidelityModel);
    ActiveSet lo_fi_set;
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      lo_fi_set = set; lo_fi_set.request_vector(lo_fi_asv);
      lowFidelityModel.compute_response(lo_fi_set); break;
    case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
      lowFidelityModel.compute_response(set);       break;
    }

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE: {
      // LF resp should not be corrected directly (see derived_synchronize())
      lo_fi_response = lowFidelityModel.current_response().copy();
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      if (!deltaCorr.computed())
	deltaCorr.compute(currentVariables.continuous_variables(),
			  highFidRefResponse, lo_fi_response, quiet_flag);
      deltaCorr.apply(currentVariables.continuous_variables(), lo_fi_response,
		      quiet_flag);
      if (!mixed_eval) {
	currentResponse.active_set(lo_fi_set);
	currentResponse.update(lo_fi_response);
      }
      break;
    }
    case UNCORRECTED_SURROGATE:
      if (mixed_eval)
	lo_fi_response = lowFidelityModel.current_response(); // shared rep
      else {
	currentResponse.active_set(lo_fi_set);
	currentResponse.update(lowFidelityModel.current_response());
      }
      break;
    }
  }

  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  switch (responseMode) {
  case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY: {
    // don't update surrogate data within deltaCorr's Approximations; just
    // update currentResponse (managed as surrogate data at a higher level)
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    deltaCorr.compute(highFidelityModel.current_response(),
		      lowFidelityModel.current_response(),
		      currentResponse, quiet_flag);
    break;
  }
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    if (mixed_eval) {
      currentResponse.active_set(set);
      response_mapping(hi_fi_response, lo_fi_response, currentResponse);
    }
    break;
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

  ShortArray hi_fi_asv, lo_fi_asv; bool hi_fi_eval, lo_fi_eval;
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_mapping(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
    hi_fi_eval = !hi_fi_asv.empty(); lo_fi_eval = !lo_fi_asv.empty(); break;
  case BYPASS_SURROGATE:
    hi_fi_eval = true; lo_fi_eval = false;                            break;
  case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
    hi_fi_eval = lo_fi_eval = true;                                   break;
  }

  // ------------------------------
  // Compute high fidelity response
  // ------------------------------
  if (hi_fi_eval) {
    // don't need to set component parallel mode since this only queues the job
    update_model(highFidelityModel);
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet hi_fi_set = set; hi_fi_set.request_vector(hi_fi_asv);
      highFidelityModel.asynch_compute_response(hi_fi_set); break;
    }
    case BYPASS_SURROGATE:
    case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
      highFidelityModel.asynch_compute_response(set);       break;
    }
    // store map from HF eval id to HierarchSurrModel id
    truthIdMap[highFidelityModel.evaluation_id()] = hierModelEvalCntr;
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
    // don't need to set component parallel mode since this only queues the job
    update_model(lowFidelityModel);
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet lo_fi_set = set; lo_fi_set.request_vector(lo_fi_asv);
      lowFidelityModel.asynch_compute_response(lo_fi_set); break;
    }
    case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
      lowFidelityModel.asynch_compute_response(set);       break;
    }

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE:
      copy_data(currentVariables.continuous_variables(),
		rawCVarsMap[hierModelEvalCntr]);           break;
    }
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

  // -----------------------------------
  // synchronize highFidelityModel evals
  // -----------------------------------
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

    // if no LF evals (BYPASS_SURROGATE mode or rare case of empty
    // surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE modes), return
    // all HF results.  *_DISCREPANCY modes always have both lo & hi evals.
    if (!lo_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed and augmented by proxy
  }

  // ----------------------------------
  // synchronize lowFidelityModel evals
  // ----------------------------------
  IntResponseMap lo_fi_resp_map_rekey;
  if (lo_fi_evals) {
    component_parallel_mode(LF_MODEL);
    const IntResponseMap& lo_fi_resp_map = lowFidelityModel.synchronize();

    // update map keys to use hierModelEvalCntr
    IntResponseMap& lo_fi_resp_map_proxy
      = (hi_fi_evals) ? lo_fi_resp_map_rekey : surrResponseMap;
    for (IntRespMCIter r_cit = lo_fi_resp_map.begin();
	 r_cit != lo_fi_resp_map.end(); ++r_cit)
      lo_fi_resp_map_proxy[surrIdMap[r_cit->first]] =
	(responseMode == AUTO_CORRECTED_SURROGATE) ?
	r_cit->second.copy() : r_cit->second;
    surrIdMap.clear();

    if (responseMode == AUTO_CORRECTED_SURROGATE) {
      // Interface::rawResponseMap should _not_ be corrected directly since
      // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share
      // a responseRep -->> modifying rawResponseMap affects data_pairs.

      // if a correction has not been computed, compute it now
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      if (!deltaCorr.computed() && !lo_fi_resp_map_proxy.empty())
	deltaCorr.compute(rawCVarsMap.begin()->second, highFidRefResponse,
			  lo_fi_resp_map_proxy.begin()->second, quiet_flag);

      // Apply the correction.  A rawCVarsMap lookup is not needed since
      // rawCVarsMap and lo_fi_resp_map are complete and consistently ordered.
      IntRDVMIter v_it; IntRespMIter r_it;
      for (r_it  = lo_fi_resp_map_proxy.begin(), v_it = rawCVarsMap.begin();
	   r_it != lo_fi_resp_map_proxy.end(); ++r_it, ++v_it)
        deltaCorr.apply(v_it->second, r_it->second, quiet_flag); //rawCVarsMap
                                                                 //[r_it->first]
      rawCVarsMap.clear();
    }

    // add cached approx evals (synchronized evals that could not be returned
    // since truth eval portions were still pending) for processing.  Do not
    // correct them a second time.
    for (IntRespMCIter r_cit = cachedApproxRespMap.begin();
	 r_cit != cachedApproxRespMap.end(); ++r_cit)
      lo_fi_resp_map_proxy[r_cit->first] = r_cit->second;
    cachedApproxRespMap.clear();

    // if no HF evals (full surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE
    // modes), return all LF results.  *_DISCREPANCY modes always have both
    // lo & hi evals.
    if (!hi_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed, corrected, and augmented by proxy
  }

  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  // Both highFidelityModel and lowFidelityModel evals are present:
  // {hi,lo}_fi_resp_map_rekey may be partial sets (partial surrogateFnIndices
  // in {UN,AUTO_}CORRECTED_SURROGATE modes) or full sets (*_DISCREPANCY modes).
  Response empty_resp;
  IntRespMCIter hf_it = hi_fi_resp_map_rekey.begin(),
                lf_it = lo_fi_resp_map_rekey.begin();
  switch (responseMode) {
  case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY: {
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    for (; hf_it != hi_fi_resp_map_rekey.end() && 
	   lf_it != lo_fi_resp_map_rekey.end(); ++hf_it, ++lf_it)
      deltaCorr.compute(hf_it->second, lf_it->second,
			surrResponseMap[hf_it->first], quiet_flag);
    break;
  }
  default: // {UN,AUTO_}CORRECTED_SURROGATE modes
    // process any combination of HF and LF completions
    while (hf_it != hi_fi_resp_map_rekey.end() ||
	   lf_it != lo_fi_resp_map_rekey.end()) {
      int hf_eval_id = (hf_it == hi_fi_resp_map_rekey.end()) ?
	INT_MAX : hf_it->first;
      int lf_eval_id = (lf_it == lo_fi_resp_map_rekey.end()) ?
	INT_MAX : lf_it->first;

      if (hf_eval_id < lf_eval_id) // only HF available
	{ response_mapping(hf_it->second, empty_resp,
			   surrResponseMap[hf_eval_id]); ++hf_it; }
      else if (lf_eval_id < hf_eval_id) // only LF available
	{ response_mapping(empty_resp, lf_it->second,
			   surrResponseMap[lf_eval_id]); ++lf_it; }
      else // both LF and HF available
	{ response_mapping(hf_it->second, lf_it->second,
			   surrResponseMap[hf_eval_id]); ++hf_it; ++lf_it; }
    }
    break;
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

  // -----------------------------------
  // synchronize highFidelityModel evals
  // -----------------------------------
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

    // if no LF evals (BYPASS_SURROGATE mode or rare case of empty
    // surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE modes), return
    // all HF results.  *_DISCREPANCY modes always have both lo & hi evals.
    if (!lo_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed and augmented by proxy
  }

  // ----------------------------------
  // synchronize lowFidelityModel evals
  // ----------------------------------
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
      lo_fi_resp_map_proxy[surrIdMap[lf_eval_id]] =
	(responseMode == AUTO_CORRECTED_SURROGATE) ?
	r_cit->second.copy() : r_cit->second;
      if (!hi_fi_evals)
	surrIdMap.erase(lf_eval_id);
    }

    if (responseMode == AUTO_CORRECTED_SURROGATE) {
      // Interface::rawResponseMap should _not_ be corrected directly since
      // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share
      // a responseRep -->> modifying rawResponseMap affects data_pairs.

      // if a correction has not been computed, compute it now
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      if (!deltaCorr.computed() && !lo_fi_resp_map_proxy.empty())
	deltaCorr.compute(rawCVarsMap.begin()->second, highFidRefResponse,
			  lo_fi_resp_map_proxy.begin()->second, quiet_flag);

      // Apply the correction.  Must use rawCVarsMap lookup in this case since
      // rawCVarsMap is complete, but lo_fi_resp_map may not be.
      for (IntRespMIter r_it = lo_fi_resp_map_proxy.begin();
	   r_it != lo_fi_resp_map_proxy.end(); ++r_it) {
	int hier_eval_id = r_it->first;
        deltaCorr.apply(rawCVarsMap[hier_eval_id], r_it->second, quiet_flag);
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

    // if no HF evals (full surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE
    // modes), return all LF results.  *_DISCREPANCY modes always have both
    // lo & hi evals.
    if (!hi_fi_evals)         // return ref to non-temporary
      return surrResponseMap; // rekeyed, corrected, and augmented by proxy
  }

  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  // Both highFidelityModel and lowFidelityModel evals are present:
  // {hi,lo}_fi_resp_map_rekey may be partial sets (partial surrogateFnIndices
  // in {UN,AUTO_}CORRECTED_SURROGATE modes) or full sets (*_DISCREPANCY modes).
  Response empty_resp;
  IntRespMCIter hf_it = hi_fi_resp_map_rekey.begin(),
                lf_it = lo_fi_resp_map_rekey.begin();
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  // invert truthIdMap and surrIdMap
  IntIntMap inverse_truth_id_map, inverse_surr_id_map;
  for (IntIntMCIter tim_it=truthIdMap.begin();
       tim_it!=truthIdMap.end(); ++tim_it)
    inverse_truth_id_map[tim_it->second] = tim_it->first;
  for (IntIntMCIter sim_it=surrIdMap.begin();
       sim_it!=surrIdMap.end(); ++sim_it)
    inverse_surr_id_map[sim_it->second] = sim_it->first;
  // process any combination of HF and LF completions
  while (hf_it != hi_fi_resp_map_rekey.end() ||
	 lf_it != lo_fi_resp_map_rekey.end()) {
    int hf_eval_id = (hf_it == hi_fi_resp_map_rekey.end()) ?
      INT_MAX : hf_it->first;
    int lf_eval_id = (lf_it == lo_fi_resp_map_rekey.end()) ?
      INT_MAX : lf_it->first;
    // process LF/HF results or cache them for next pass
    if (hf_eval_id < lf_eval_id) { // only HF available
      switch (responseMode) {
      case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
	// cache HF response since LF contribution not yet available
	cachedTruthRespMap[hf_eval_id] = hf_it->second; break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	if (inverse_surr_id_map.count(hf_eval_id))
	  // cache HF response since LF contribution not yet available
	  cachedTruthRespMap[hf_eval_id] = hf_it->second;
	else { // there is no LF component to this response
	  response_mapping(hf_it->second, empty_resp,
			   surrResponseMap[hf_eval_id]);
	  truthIdMap.erase(inverse_truth_id_map[hf_eval_id]);
	}
	break;
      }
      ++hf_it;
    }
    else if (lf_eval_id < hf_eval_id) { // only LF available
      switch (responseMode) {
      case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
	// cache LF response since HF contribution not yet available
	cachedApproxRespMap[lf_eval_id] = lf_it->second; break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	if (inverse_truth_id_map.count(lf_eval_id))
	  // cache LF response since HF contribution not yet available
	  cachedApproxRespMap[lf_eval_id] = lf_it->second;
	else { // response complete: there is no HF contribution
	  response_mapping(empty_resp, lf_it->second, 
			   surrResponseMap[lf_eval_id]);
	  surrIdMap.erase(inverse_surr_id_map[lf_eval_id]);
	}
	break;
      }
      ++lf_it;
    }
    else { // both LF and HF available
      switch (responseMode) {
      case ADDITIVE_DISCREPANCY: case MULTIPLICATIVE_DISCREPANCY:
	deltaCorr.compute(hf_it->second, lf_it->second,
			  surrResponseMap[hf_eval_id], quiet_flag); break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE modes
	response_mapping(hf_it->second, lf_it->second,
			 surrResponseMap[hf_eval_id]);              break;
      }
      truthIdMap.erase(inverse_truth_id_map[hf_eval_id]);
      surrIdMap.erase(inverse_surr_id_map[lf_eval_id]);
      ++hf_it; ++lf_it;
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
