/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014 Sandia Corporation.
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrModel
//- Description: Implementation code for the HierarchSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "HierarchSurrModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]="@(#) $Id: HierarchSurrModel.cpp 6656 2010-02-26 05:20:48Z mseldre $";


namespace Dakota {

HierarchSurrModel::HierarchSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db), hierModelEvalCntr(0),
  truthResponseRef(currentResponse.copy())
{
  // Hierarchical surrogate models pass through numerical derivatives
  supports_derivative_estimation(false);
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");

  const StringArray& ordered_model_ptrs
    = problem_db.get_sa("model.surrogate.ordered_model_pointers");

  size_t i, num_models = ordered_model_ptrs.size(),
    model_index = problem_db.get_db_model_node(); // for restoration

  const std::pair<short,short>& cv_view = currentVariables.view();
  orderedModels.resize(num_models);
  for (i=0; i<num_models; ++i) {
    problem_db.set_db_model_nodes(ordered_model_ptrs[i]);
    orderedModels[i] = problem_db.get_model();
    check_submodel_compatibility(orderedModels[i]);
    if (cv_view != orderedModels[i].current_variables().view()) {
      Cerr << "Error: variable views in hierarchical models must be identical."
	   << std::endl;
      abort_handler(-1);
    }
  }

  problem_db.set_db_model_nodes(model_index); // restore

  // default index values, to be overridden at run time
  lowFidelityIndices.first = 0; highFidelityIndices.first = num_models - 1;
  if (num_models == 1) {
    sameModelForm = true;
    lowFidelityIndices.second = 0; highFidelityIndices.second = 1;
  }
  else {
    sameModelForm = false;
    lowFidelityIndices.second = highFidelityIndices.second = 0;
  }
  
  // Correction is required in the hierarchical case (since without correction,
  // all HF model evaluations are wasted).  Omission of a correction type
  // should be prevented by the input specification.
  short corr_type = problem_db.get_short("model.surrogate.correction_type");
  if (!corr_type) {
    Cerr << "Error: correction is required with model hierarchies."<< std::endl;
    abort_handler(-1);
  }
  else // initialize the DiscrepancyCorrection using lowest fidelity model;
       // this LF model gets updated at run time
    deltaCorr.initialize(orderedModels[0], surrogateFnIndices, corr_type,
      problem_db.get_short("model.surrogate.correction_order"));
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
    size_t model_index = probDescDB.get_db_model_node(); // for restoration

    Model& lf_model = orderedModels[lowFidelityIndices.first];
    Model& hf_model = orderedModels[highFidelityIndices.first];

    // superset of possible init calls (two configurations for HF)
    probDescDB.set_db_model_nodes(lf_model.model_id());
    lf_model.init_communicators(pl_iter, max_eval_concurrency);

    probDescDB.set_db_model_nodes(hf_model.model_id());
    hf_model.init_communicators(pl_iter, hf_model.derivative_concurrency());
    hf_model.init_communicators(pl_iter, max_eval_concurrency);

    /*
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
    case MODEL_DISCREPANCY:
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
  // this scope (e.g., Model::evaluate() within SBLMinimizer).  The
  // default responseMode value is AUTO_CORRECTED_SURROGATE, which mitigates
  // the specific case of SBLMinimizer, but the general fragility remains.
  if (recurse_flag) {

    // bcast not needed for recurse_flag=false in serve_run call to set_comms
    //if (pl_iter->server_communicator_size() > 1)
    //  parallelLib.bcast(responseMode, *pl_iter);

    switch (responseMode) {
    case UNCORRECTED_SURROGATE: {
      Model& lf_model = orderedModels[lowFidelityIndices.first];
      lf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = lf_model.asynch_flag();
      evaluationCapacity = lf_model.evaluation_capacity();
      break;
    }
    case AUTO_CORRECTED_SURROGATE: {
      Model& lf_model = orderedModels[lowFidelityIndices.first];
      Model& hf_model = orderedModels[highFidelityIndices.first];
      lf_model.set_communicators(pl_iter, max_eval_concurrency);
      int hf_deriv_conc = hf_model.derivative_concurrency();
      hf_model.set_communicators(pl_iter, hf_deriv_conc);
      asynchEvalFlag = ( lf_model.asynch_flag() ||
	( hf_deriv_conc > 1 && hf_model.asynch_flag() ) );
      evaluationCapacity = std::max( lf_model.evaluation_capacity(),
				     hf_model.evaluation_capacity() );
      break;
    }
    case BYPASS_SURROGATE: {
      Model& hf_model = orderedModels[highFidelityIndices.first];
      hf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = hf_model.asynch_flag();
      evaluationCapacity = hf_model.evaluation_capacity();
      break;
    }
    case MODEL_DISCREPANCY: {
      Model& lf_model = orderedModels[lowFidelityIndices.first];
      Model& hf_model = orderedModels[highFidelityIndices.first];
      lf_model.set_communicators(pl_iter, max_eval_concurrency);
      hf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag = ( lf_model.asynch_flag() || hf_model.asynch_flag() );
      evaluationCapacity = std::max( lf_model.evaluation_capacity(),
				     hf_model.evaluation_capacity() );
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
    // superset of possible free calls (two configurations for HF)
    orderedModels[lowFidelityIndices.first].free_communicators(pl_iter,
      max_eval_concurrency);
    Model& hf_model = orderedModels[highFidelityIndices.first];
    hf_model.free_communicators(pl_iter, hf_model.derivative_concurrency());
    hf_model.free_communicators(pl_iter, max_eval_concurrency);

    /*
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
    case MODEL_DISCREPANCY:
      lf_model.free_communicators(pl_iter, max_eval_concurrency);
      hf_model.free_communicators(pl_iter, max_eval_concurrency);
      break;
    }
    */
  }
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
  //if (sameModelForm) lf_model.solution_level_index(lowFidelityIndices.second);
  //lf_model.evaluate(temp_set);
  //const Response& lo_fi_response = lf_model.current_response();

  Model& hf_model = orderedModels[highFidelityIndices.first];
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + 
      boost::lexical_cast<String>(hierModelEvalCntr+1);
    hf_model.eval_tag_prefix(eval_tag);
  }

  // set HierarchSurrModel parallelism mode to HF model
  component_parallel_mode(HF_MODEL);

  // update HF model with current variable values/bounds/labels
  update_model(hf_model);

  // store inactive variable values for use in determining whether an
  // automatic rebuild of an approximation is required
  // (reference{C,D}{L,U}Bnds are not needed in the hierarchical case)
  const Variables& hf_vars = hf_model.current_variables();
  copy_data(hf_vars.inactive_continuous_variables(),      referenceICVars);
  copy_data(hf_vars.inactive_discrete_int_variables(),    referenceIDIVars);
  referenceIDSVars = hf_vars.inactive_discrete_string_variables();
  copy_data(hf_vars.inactive_discrete_real_variables(),   referenceIDRVars);

  // compute the response for the high fidelity model
  ShortArray total_asv(numFns, deltaCorr.data_order()), hf_asv, lf_asv;
  asv_mapping(total_asv, hf_asv, lf_asv, true);
  ActiveSet hf_set = truthResponseRef.active_set(); // copy
  hf_set.request_vector(hf_asv);
  if (sameModelForm) hf_model.solution_level_index(highFidelityIndices.second);
  hf_model.evaluate(hf_set);
  truthResponseRef.update(hf_model.current_response());

  // could compute the correction to LF model here, but rely on an
  // external call for consistency with DataFitSurr and to facilitate SBO logic.
  //deltaCorr.compute(..., truthResponseRef, lo_fi_response);

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
    Model& hf_model = orderedModels[highFidelityIndices.first];
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
  ++hierModelEvalCntr;

  // define LF/HF evaluation requirements
  ShortArray hi_fi_asv, lo_fi_asv; bool hi_fi_eval, lo_fi_eval, mixed_eval;
  Response lo_fi_response, hi_fi_response; // don't use truthResponseRef
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_mapping(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
    hi_fi_eval = !hi_fi_asv.empty(); lo_fi_eval = !lo_fi_asv.empty();
    mixed_eval = (hi_fi_eval && lo_fi_eval);            break;
  case BYPASS_SURROGATE:
    hi_fi_eval = true; lo_fi_eval = mixed_eval = false; break;
  case MODEL_DISCREPANCY:
    hi_fi_eval = lo_fi_eval = mixed_eval = true;        break;
  }

  Model& lf_model = orderedModels[lowFidelityIndices.first];
  Model& hf_model = orderedModels[highFidelityIndices.first];
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + 
      boost::lexical_cast<String>(hierModelEvalCntr+1);
    if (sameModelForm) lf_model.eval_tag_prefix(eval_tag);
    else {
      if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
      if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
    }
  }

  if (sameModelForm) update_model(lf_model);

  // Notes on repetitive setting of model.solution_level_index():
  // > when LF & HF are the same model, then setting the index for low or high
  //   invalidates the other fidelity definition.
  // > within a single derived_evaluate(), could protect these updates with
  //   "if (sameModelForm && mixed_eval)", but this does not guard against
  //   changes in eval requirements from the previous evaluation.  Detecting
  //   the current solution index state is currently as expensive as resetting
  //   it, so just reset each time.
  
  // ------------------------------
  // Compute high fidelity response
  // ------------------------------
  if (hi_fi_eval) {
    component_parallel_mode(HF_MODEL); // TO DO: sameModelForm
    if (sameModelForm)
      hf_model.solution_level_index(highFidelityIndices.second);
    else
      update_model(hf_model);      
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
      ActiveSet hi_fi_set = set; hi_fi_set.request_vector(hi_fi_asv);
      hf_model.evaluate(hi_fi_set);
      if (mixed_eval)
	hi_fi_response = (sameModelForm) ? hf_model.current_response().copy()
	               : hf_model.current_response(); // shared rep
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
      hi_fi_response = (sameModelForm) ? hf_model.current_response().copy()
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
    component_parallel_mode(LF_MODEL); // TO DO: sameModelForm
    if (sameModelForm) lf_model.solution_level_index(lowFidelityIndices.second);
    else               update_model(lf_model);
    ActiveSet lo_fi_set;
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      lo_fi_set = set; lo_fi_set.request_vector(lo_fi_asv);
      lf_model.evaluate(lo_fi_set); break;
    case MODEL_DISCREPANCY:
      lf_model.evaluate(set);       break;
    }

    // post-process
    switch (responseMode) {
    case AUTO_CORRECTED_SURROGATE: {
      // LF resp should not be corrected directly (see derived_synchronize())
      lo_fi_response = lf_model.current_response().copy();
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      if (!deltaCorr.computed())
	deltaCorr.compute(currentVariables, truthResponseRef, lo_fi_response,
			  quiet_flag);
      deltaCorr.apply(currentVariables, lo_fi_response, quiet_flag);
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
    // don't update surrogate data within deltaCorr's Approximations; just
    // update currentResponse (managed as surrogate data at a higher level)
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    currentResponse.active_set(set);
    deltaCorr.compute(hi_fi_response, lf_model.current_response(),
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


/** Compute the response asynchronously using LF model, HF model, or
    both (mixed case).  For the LF model portion, compute the high
    fidelity response with build_approximation() (for correcting the
    low fidelity results in derived_synchronize() and
    derived_synchronize_nowait()) if not performed previously. */
void HierarchSurrModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++hierModelEvalCntr;

  Model& lf_model = orderedModels[lowFidelityIndices.first];
  Model& hf_model = orderedModels[highFidelityIndices.first];

  ShortArray hi_fi_asv, lo_fi_asv;
  bool hi_fi_eval, lo_fi_eval, asynch_lo_fi = lf_model.asynch_flag(),
    asynch_hi_fi = hf_model.asynch_flag();
  switch (responseMode) {
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    asv_mapping(set.request_vector(), hi_fi_asv, lo_fi_asv, false);
    hi_fi_eval = !hi_fi_asv.empty(); lo_fi_eval = !lo_fi_asv.empty(); break;
  case BYPASS_SURROGATE:
    hi_fi_eval = true; lo_fi_eval = false;                            break;
  case MODEL_DISCREPANCY:
    hi_fi_eval = lo_fi_eval = true;                                   break;
  }

  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + 
      boost::lexical_cast<String>(hierModelEvalCntr+1);
    if (sameModelForm) lf_model.eval_tag_prefix(eval_tag);
    else {
      if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
      if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
    }
  }

  if (sameModelForm) update_model(lf_model);
  
  // perform Model updates and define active sets for LF and HF evaluations
  ActiveSet hi_fi_set, lo_fi_set;
  if (hi_fi_eval) {
    // update HF model
    if (!sameModelForm) update_model(hf_model);
    // update hi_fi_set
    hi_fi_set.derivative_vector(set.derivative_vector());
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      hi_fi_set.request_vector(hi_fi_asv);            break;
    case BYPASS_SURROGATE: case MODEL_DISCREPANCY:
      hi_fi_set.request_vector(set.request_vector()); break;
    }
  }
  if (lo_fi_eval) {
    // if build_approximation has not yet been called, call it now
    if ( responseMode == AUTO_CORRECTED_SURROGATE &&
	 ( !approxBuilds || force_rebuild() ) )
      build_approximation();
    // update LF model
    if (!sameModelForm) update_model(lf_model);
    // update lo_fi_set
    lo_fi_set.derivative_vector(set.derivative_vector());
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      lo_fi_set.request_vector(lo_fi_asv);            break;
    case MODEL_DISCREPANCY:
      lo_fi_set.request_vector(set.request_vector()); break;
    }
  }

  // HierarchSurrModel's asynchEvalFlag is set if _either_ LF or HF is
  // asynchronous, resulting in use of derived_evaluate_nowait().
  // To manage general case of mixed asynch, launch nonblocking evals first,
  // followed by blocking evals.

  // For notes on repetitive setting of model.solution_level_index(), see
  // derived_evaluate() above.
  
  // launch nonblocking evals before any blocking ones
  if (hi_fi_eval && asynch_hi_fi) { // HF model may be executed asynchronously
    // don't need to set component parallel mode since only queues the job
    if (sameModelForm)
      hf_model.solution_level_index(highFidelityIndices.second);
    hf_model.evaluate_nowait(hi_fi_set);
    // store map from HF eval id to HierarchSurrModel id
    truthIdMap[hf_model.evaluation_id()] = hierModelEvalCntr;
  }
  if (lo_fi_eval && asynch_lo_fi) { // LF model may be executed asynchronously
    // don't need to set component parallel mode since only queues the job
    if (sameModelForm) lf_model.solution_level_index(lowFidelityIndices.second);
    lf_model.evaluate_nowait(lo_fi_set);
    // store map from LF eval id to HierarchSurrModel id
    surrIdMap[lf_model.evaluation_id()] = hierModelEvalCntr;
    // store variables set needed for correction
    if (responseMode == AUTO_CORRECTED_SURROGATE)
      rawVarsMap[hierModelEvalCntr] = currentVariables.copy();
  }

  // now launch any blocking evals
  if (hi_fi_eval && !asynch_hi_fi) { // execute HF synchronously & cache resp
    component_parallel_mode(HF_MODEL);
    if (sameModelForm)
      hf_model.solution_level_index(highFidelityIndices.second);
    hf_model.evaluate(hi_fi_set);
    cachedTruthRespMap[hf_model.evaluation_id()]
      = hf_model.current_response().copy();
  }
  if (lo_fi_eval && !asynch_lo_fi) { // execute LF synchronously & cache resp
    component_parallel_mode(LF_MODEL);
    if (sameModelForm) lf_model.solution_level_index(lowFidelityIndices.second);
    lf_model.evaluate(lo_fi_set);
    Response lo_fi_response(lf_model.current_response().copy());
    // correct LF response prior to caching
    if (responseMode == AUTO_CORRECTED_SURROGATE) {
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      if (!deltaCorr.computed())
	deltaCorr.compute(currentVariables, truthResponseRef, lo_fi_response,
			  quiet_flag);
      deltaCorr.apply(currentVariables, lo_fi_response, quiet_flag);
    }
    // cache corrected LF response for retrieval during synchronization
    cachedApproxRespMap[lf_model.evaluation_id()] = lo_fi_response;
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
  bool asynch_hi_fi = !truthIdMap.empty(), asynch_lo_fi = !surrIdMap.empty(),
    cached_hi_fi = !cachedTruthRespMap.empty(),
    cached_lo_fi = !cachedApproxRespMap.empty(),
    hi_fi_evals  = (asynch_hi_fi || cached_hi_fi),
    lo_fi_evals  = (asynch_lo_fi || cached_lo_fi);

  if (asynch_hi_fi && asynch_lo_fi) {
    // in this case, we don't want to starve either LF or HF scheduling by
    // blocking on one or the other --> leverage derived_synchronize_nowait()
    IntResponseMap aggregated_map; // accumulate surrResponseMap returns
    while (!truthIdMap.empty() || !surrIdMap.empty()) {
      // partial_map is a reference to surrResponseMap, returned by _nowait()
      const IntResponseMap& partial_map	= derived_synchronize_nowait();
      if (!partial_map.empty())
	aggregated_map.insert(partial_map.begin(), partial_map.end());
    }
    surrResponseMap = aggregated_map; // now replace surrResponseMap for return
    return surrResponseMap;
  }

  Model& lf_model = orderedModels[lowFidelityIndices.first];
  Model& hf_model = orderedModels[highFidelityIndices.first];

  // --------------------------
  // synchronize HF model evals
  // --------------------------
  IntResponseMap hi_fi_resp_map_rekey;
  if (hi_fi_evals) {
    IntResponseMap& hi_fi_resp_map_proxy
      = (lo_fi_evals) ? hi_fi_resp_map_rekey : surrResponseMap;

    if (asynch_hi_fi) {
      // synchronize HF evals
      component_parallel_mode(HF_MODEL);
      const IntResponseMap& hi_fi_resp_map = hf_model.synchronize();
      // update map keys to use hierModelEvalCntr
      for (IntRespMCIter r_cit = hi_fi_resp_map.begin();
	   r_cit != hi_fi_resp_map.end(); ++r_cit)
	hi_fi_resp_map_proxy[truthIdMap[r_cit->first]] = r_cit->second;
      truthIdMap.clear();
    }

    if (cached_hi_fi) {
      // add cached truth evals for processing, where evals are cached from:
      // (a) recovered HF asynch evals that could not be returned since LF
      //     eval portions were still pending, or
      // (b) synchronous HF evals performed within evaluate_nowait()
      for (IntRespMCIter r_cit = cachedTruthRespMap.begin();
	   r_cit != cachedTruthRespMap.end(); ++r_cit)
	hi_fi_resp_map_proxy[r_cit->first] = r_cit->second;
      cachedTruthRespMap.clear();
    }

    // if no LF evals (BYPASS_SURROGATE mode or rare case of empty
    // surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE modes), return
    // all HF results.  MODEL_DISCREPANCY mode always has both lo & hi evals.
    if (!lo_fi_evals)
      return surrResponseMap; // rekeyed and augmented by proxy
  }

  // --------------------------
  // synchronize LF model evals
  // --------------------------
  IntResponseMap lo_fi_resp_map_rekey;
  if (lo_fi_evals) {
    IntResponseMap& lo_fi_resp_map_proxy
      = (hi_fi_evals) ? lo_fi_resp_map_rekey : surrResponseMap;

    if (asynch_lo_fi) {
      // synchronize LF evals
      component_parallel_mode(LF_MODEL);
      const IntResponseMap& lo_fi_resp_map = lf_model.synchronize();
      // update map keys to use hierModelEvalCntr
      for (IntRespMCIter r_cit = lo_fi_resp_map.begin();
	   r_cit != lo_fi_resp_map.end(); ++r_cit)
	lo_fi_resp_map_proxy[surrIdMap[r_cit->first]] =
	  // Interface::rawResponseMap should _not_ be corrected directly since
	  // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share
	  // a responseRep -->> modifying rawResponseMap affects data_pairs.
	  (responseMode == AUTO_CORRECTED_SURROGATE) ?
	  r_cit->second.copy() : r_cit->second;
      surrIdMap.clear();

      if (responseMode == AUTO_CORRECTED_SURROGATE) {
	// if a correction has not been computed, compute it now
	bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
	if (!deltaCorr.computed() && !lo_fi_resp_map_proxy.empty())
	  deltaCorr.compute(rawVarsMap.begin()->second, truthResponseRef,
			    lo_fi_resp_map_proxy.begin()->second, quiet_flag);
	// Apply the correction.  A rawVarsMap lookup is not needed since
	// rawVarsMap and lo_fi_resp_map are complete and consistently ordered.
	IntVarsMIter v_it; IntRespMIter r_it;
	for (r_it  = lo_fi_resp_map_proxy.begin(), v_it = rawVarsMap.begin();
	     r_it != lo_fi_resp_map_proxy.end(); ++r_it, ++v_it)
	  deltaCorr.apply(v_it->second, r_it->second,
			  quiet_flag); //rawVarsMap[r_it->first]
	rawVarsMap.clear();
      }
    }

    if (cached_lo_fi) {
      // add cached approx evals for processing, where evals are cached from:
      // (a) recovered LF asynch evals that could not be returned since HF
      //     eval portions were still pending, or
      // (b) synchronous LF evals performed within evaluate_nowait()
      // Do not correct them a second time.
      for (IntRespMCIter r_cit = cachedApproxRespMap.begin();
	   r_cit != cachedApproxRespMap.end(); ++r_cit)
	lo_fi_resp_map_proxy[r_cit->first] = r_cit->second;
      cachedApproxRespMap.clear();
    }

    // if no HF evals (full surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE
    // modes), return all LF results.  MODEL_DISCREPANCY mode always has both
    // lo & hi evals.
    if (!hi_fi_evals)
      return surrResponseMap; // rekeyed, corrected, and augmented by proxy
  }

  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  // Both HF model and LF model evals are present:
  // {hi,lo}_fi_resp_map_rekey may be partial sets (partial surrogateFnIndices
  // in {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY).
  Response empty_resp;
  IntRespMCIter hf_it = hi_fi_resp_map_rekey.begin(),
                lf_it = lo_fi_resp_map_rekey.begin();
  switch (responseMode) {
  case MODEL_DISCREPANCY: {
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


/** Nonblocking retrieval of asynchronous evaluations from LF model,
    HF model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the map.
    derived_synchronize_nowait() is designed for the general case
    where derived_evaluate_nowait() may be inconsistent in its use of
    actual evals, approx evals, or both. */
const IntResponseMap& HierarchSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();
  bool asynch_hi_fi = !truthIdMap.empty(), asynch_lo_fi = !surrIdMap.empty(),
    cached_hi_fi = !cachedTruthRespMap.empty(),
    cached_lo_fi = !cachedApproxRespMap.empty(),
    hi_fi_evals  = (asynch_hi_fi || cached_hi_fi),
    lo_fi_evals  = (asynch_lo_fi || cached_lo_fi);

  Model& lf_model = orderedModels[lowFidelityIndices.first];
  Model& hf_model = orderedModels[highFidelityIndices.first];

  // --------------------------
  // synchronize HF model evals
  // --------------------------
  IntResponseMap hi_fi_resp_map_rekey;
  if (hi_fi_evals) {
    IntResponseMap& hi_fi_resp_map_proxy
      = (lo_fi_evals) ? hi_fi_resp_map_rekey : surrResponseMap;

    if (asynch_hi_fi) {
      // synchronize HF evals
      component_parallel_mode(HF_MODEL);
      const IntResponseMap& hi_fi_resp_map = hf_model.synchronize_nowait();
      // update map keys to use hierModelEvalCntr
      for (IntRespMCIter r_cit = hi_fi_resp_map.begin();
	   r_cit != hi_fi_resp_map.end(); ++r_cit) {
	int hf_eval_id = r_cit->first;
	hi_fi_resp_map_proxy[truthIdMap[hf_eval_id]] = r_cit->second;
	if (!lo_fi_evals)
	  truthIdMap.erase(hf_eval_id); // erase now prior to return below
      }
    }

    if (cached_hi_fi) {
      // add cached truth evals for processing, where evals are cached from:
      // (a) recovered HF asynch evals that could not be returned since LF
      //     eval portions were still pending, or
      // (b) synchronous HF evals performed within evaluate_nowait()
      for (IntRespMCIter r_cit = cachedTruthRespMap.begin();
	   r_cit != cachedTruthRespMap.end(); ++r_cit)
	hi_fi_resp_map_proxy[r_cit->first] = r_cit->second;
      cachedTruthRespMap.clear();
    }

    // if no LF evals (BYPASS_SURROGATE mode or rare case of empty
    // surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE modes), return
    // all HF results.  MODEL_DISCREPANCY mode always has both lo & hi evals.
    if (!lo_fi_evals)
      return surrResponseMap; // rekeyed and augmented by proxy
  }

  // --------------------------
  // synchronize LF model evals
  // --------------------------
  IntResponseMap lo_fi_resp_map_rekey;
  if (lo_fi_evals) {
    IntResponseMap& lo_fi_resp_map_proxy
      = (hi_fi_evals) ? lo_fi_resp_map_rekey : surrResponseMap;

    if (asynch_lo_fi) {
      // synchronize LF evals
      component_parallel_mode(LF_MODEL);
      const IntResponseMap& lo_fi_resp_map = lf_model.synchronize_nowait();
      // update map keys to use hierModelEvalCntr
      for (IntRespMCIter r_cit = lo_fi_resp_map.begin();
	   r_cit != lo_fi_resp_map.end(); ++r_cit) {
	int lf_eval_id = r_cit->first;
	lo_fi_resp_map_proxy[surrIdMap[lf_eval_id]] =
	  // Interface::rawResponseMap should _not_ be corrected directly since
	  // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share
	  // a responseRep -->> modifying rawResponseMap affects data_pairs.
	  (responseMode == AUTO_CORRECTED_SURROGATE) ?
	  r_cit->second.copy() : r_cit->second;
	if (!hi_fi_evals)
	  surrIdMap.erase(lf_eval_id);
      }

      if (responseMode == AUTO_CORRECTED_SURROGATE) {
	// if a correction has not been computed, compute it now
	bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
	if (!deltaCorr.computed() && !lo_fi_resp_map_proxy.empty())
	  deltaCorr.compute(rawVarsMap.begin()->second, truthResponseRef,
			    lo_fi_resp_map_proxy.begin()->second, quiet_flag);
	// Apply the correction.  Must use rawVarsMap lookup in this case since
	// rawVarsMap is complete, but lo_fi_resp_map may not be.
	for (IntRespMIter r_it = lo_fi_resp_map_proxy.begin();
	     r_it != lo_fi_resp_map_proxy.end(); ++r_it) {
	  int hier_eval_id = r_it->first;
	  deltaCorr.apply(rawVarsMap[hier_eval_id], r_it->second, quiet_flag);
	  rawVarsMap.erase(hier_eval_id);
	}
      }
    }

    if (cached_lo_fi) {
      // add cached approx evals for processing, where evals are cached from:
      // (a) recovered LF asynch evals that could not be returned since HF
      //     eval portions were still pending, or
      // (b) synchronous LF evals performed within evaluate_nowait()
      // Do not correct them a second time.
      for (IntRespMCIter r_cit = cachedApproxRespMap.begin();
	   r_cit != cachedApproxRespMap.end(); ++r_cit)
	lo_fi_resp_map_proxy[r_cit->first] = r_cit->second;
      cachedApproxRespMap.clear();
    }

    // if no HF evals (full surrogateFnIndices in {UN,AUTO_}CORRECTED_SURROGATE
    // modes), return all LF results.  MODEL_DISCREPANCY mode always has both
    // lo & hi evals.
    if (!hi_fi_evals)
      return surrResponseMap; // rekeyed, corrected, and augmented by proxy
  }

  // ------------------------------
  // perform any LF/HF aggregations
  // ------------------------------
  // Both HF model and LF model evals are present:
  // {hi,lo}_fi_resp_map_rekey may be partial sets (partial surrogateFnIndices
  // in {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY).
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
      case MODEL_DISCREPANCY:
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
      case MODEL_DISCREPANCY:
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
      case MODEL_DISCREPANCY:
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
  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == mode)
  //  return; // already in correct parallel mode

  // terminate previous serve mode (if active)
  size_t index; int iter_comm_size;
  if (componentParallelMode != mode) {
    if (componentParallelMode == LF_MODEL) { // old mode
      Model& lf_model = orderedModels[lowFidelityIndices.first];
      ParConfigLIter pc_it = lf_model.parallel_configuration_iterator();
      size_t index = lf_model.mi_parallel_level_index();
      if (pc_it->mi_parallel_level_defined(index) && 
	  pc_it->mi_parallel_level(index).server_communicator_size() > 1)
	lf_model.stop_servers();
    }
    else if (componentParallelMode == HF_MODEL) { // old mode
      Model& hf_model = orderedModels[highFidelityIndices.first];
      ParConfigLIter pc_it = hf_model.parallel_configuration_iterator();
      size_t index = hf_model.mi_parallel_level_index();
      if (pc_it->mi_parallel_level_defined(index) && 
	  pc_it->mi_parallel_level(index).server_communicator_size() > 1)
	hf_model.stop_servers();
    }
  }

  // set ParallelConfiguration for new mode and retrieve new data
  if (mode == HF_MODEL) { // new mode
    // activation delegated to HF model
  }
  else if (mode == LF_MODEL) { // new mode
    // activation delegated to LF model
  }

  // activate new serve mode (matches HierarchSurrModel::serve_run(pl_iter)).
  // These bcasts match the outer parallel context (pl_iter).
  if (componentParallelMode != mode &&
      modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      parallelLib.bcast(mode, mi_pl);
      if (mode == HF_MODEL)
	parallelLib.bcast(responseMode, mi_pl);
    }
  }

  componentParallelMode = mode;
}


void HierarchSurrModel::serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false); // don't recurse

  // manage LF model and HF model servers, matching communication from
  // HierarchSurrModel::component_parallel_mode()
  componentParallelMode = 1;
  while (componentParallelMode) {
    parallelLib.bcast(componentParallelMode, *pl_iter); // outer context
    if (componentParallelMode == LF_MODEL) {
      orderedModels[lowFidelityIndices.first].serve_run(pl_iter,
	max_eval_concurrency);
      // Note: ignores erroneous BYPASS_SURROGATE to avoid responseMode bcast
    }
    else if (componentParallelMode == HF_MODEL) {
      // receive responseMode from HierarchSurrModel::component_parallel_mode()
      parallelLib.bcast(responseMode, *pl_iter);
      // employ correct iterator concurrency for HF model
      switch (responseMode) {
      case UNCORRECTED_SURROGATE:
	Cerr << "Error: setting parallel mode to HF_MODEL is erroneous for a "
	     << "response mode of UNCORRECTED_SURROGATE." << std::endl;
	abort_handler(-1);
	break;
      case AUTO_CORRECTED_SURROGATE: {
	Model& hf_model = orderedModels[highFidelityIndices.first];
	hf_model.serve_run(pl_iter, hf_model.derivative_concurrency());
	break;
      }
      case BYPASS_SURROGATE: case MODEL_DISCREPANCY:
	orderedModels[highFidelityIndices.first].serve_run(pl_iter,
	  max_eval_concurrency);
	break;
      }
    }
  }
}


void HierarchSurrModel::update_model(Model& model)
{
  // update model with currentVariables/userDefinedConstraints data.  In the
  // hierarchical case, the variables view in LF/HF models correspond to the
  // currentVariables view.  Note: updating the bounds is not strictly necessary
  // in common usage for the HF model (when a single model evaluated only at the
  // TR center), but is needed for the LF model and could be relevant in cases
  // where the HF model involves additional surrogates/nestings.

  // vars
  model.continuous_variables(currentVariables.continuous_variables());
  model.discrete_int_variables(currentVariables.discrete_int_variables());
  model.discrete_string_variables(currentVariables.discrete_string_variables());
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
    // active not currently necessary, but included for completeness and 
    // consistency with global approximation case
    model.continuous_variable_labels(
      currentVariables.continuous_variable_labels());
    model.discrete_int_variable_labels(
      currentVariables.discrete_int_variable_labels());
    model.discrete_string_variable_labels(
      currentVariables.discrete_string_variable_labels());
    model.discrete_real_variable_labels(
      currentVariables.discrete_real_variable_labels());
    short active_view = currentVariables.view().first;
    if (active_view != RELAXED_ALL && active_view != MIXED_ALL) {
      // inactive needed for Nested/Surrogate propagation
      model.inactive_continuous_variable_labels(
        currentVariables.inactive_continuous_variable_labels());
      model.inactive_discrete_int_variable_labels(
        currentVariables.inactive_discrete_int_variable_labels());
      model.inactive_discrete_string_variable_labels(
        currentVariables.inactive_discrete_string_variable_labels());
      model.inactive_discrete_real_variable_labels(
        currentVariables.inactive_discrete_real_variable_labels());
    }
  }
}

} // namespace Dakota
