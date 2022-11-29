/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2022 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       EnsembleSurrModel
//- Description: Implementation code for the EnsembleSurrModel class
//- Owner:       Mike Eldred
//- Checked by:

#include "EnsembleSurrModel.hpp"
#include "ProblemDescDB.hpp"

static const char rcsId[]=
  "@(#) $Id: EnsembleSurrModel.cpp 6656 2010-02-26 05:20:48Z mseldre $";

namespace Dakota {

extern Model dummy_model; // defined in DakotaModel.cpp


EnsembleSurrModel::EnsembleSurrModel(ProblemDescDB& problem_db):
  SurrogateModel(problem_db), sameModelInstance(false),
  sameInterfaceInstance(false), mfPrecedence(true), modeKeyBufferSize(0),
  correctionMode(SINGLE_CORRECTION)
{
  const String& truth_model_ptr
    = problem_db.get_string("model.surrogate.truth_model_pointer");
  const StringArray& ensemble_model_ptrs
    = problem_db.get_sa("model.surrogate.ensemble_model_pointers");

  size_t i, num_models = ensemble_model_ptrs.size(),
           model_index = problem_db.get_db_model_node(); // for restoration
  bool truth_model_spec = !truth_model_ptr.empty();
  if (truth_model_spec) ++num_models;
  size_t num_approx = num_models - 1;

  approxModels.resize(num_approx);
  for (i=0; i<num_approx; ++i) {
    problem_db.set_db_model_nodes(ensemble_model_ptrs[i]);
    approxModels[i] = problem_db.get_model();
    check_submodel_compatibility(approxModels[i]);
  }
  if (truth_model_spec) problem_db.set_db_model_nodes(truth_model_ptr);
  else problem_db.set_db_model_nodes(ensemble_model_ptrs[num_approx]);
  truthModel = problem_db.get_model();
  check_submodel_compatibility(truthModel);

  problem_db.set_db_model_nodes(model_index); // restore

  assign_default_keys();

  // Correction is required for some responseModes.  Enforcement of a
  // correction type for these modes occurs in surrogate_response_mode().
  switch (responseMode) {
  case MODEL_DISCREPANCY: case AUTO_CORRECTED_SURROGATE:
    if (corrType) // initialize DiscrepancyCorrection using initial keys
      deltaCorr[activeKey].initialize(active_surrogate_model(0),
				      surrogateFnIndices, corrType, corrOrder);
    break;
  }
  //truthResponseRef[truthModelKey] = currentResponse.copy();
  
  // Ensemble surrogate models pass through numerical derivatives
  supportsEstimDerivs = false;
  // initialize ignoreBounds even though it's irrelevant for pass through
  ignoreBounds = problem_db.get_bool("responses.ignore_bounds");
  // initialize centralHess even though it's irrelevant for pass through
  centralHess = problem_db.get_bool("responses.central_hess");
}


void EnsembleSurrModel::assign_default_keys()
{
  // default key data values, to be overridden at run time

  // For now, use design of all models are active and specific fn sets are
  // requested via ASV.
  unsigned short id = 0, num_approx = approxModels.size();
  if (multifidelity()) { // first and last model form (no soln levels)
    truthModelKey = Pecos::ActiveKey(id, Pecos::RAW_DATA, num_approx,
				     truthModel.solution_level_cost_index());
    //if (responseMode == AGGREGATED_MODELS) {
      surrModelKeys.resize(num_approx);
      for (unsigned short i=0; i<num_approx; ++i)
	surrModelKeys[i] = Pecos::ActiveKey(id, Pecos::RAW_DATA, i,
	  approxModels[i].solution_level_cost_index());
    //}
  }
  else if (multilevel()) { // first and last solution level (last model)
    size_t truth_soln_lev = truthModel.solution_levels(),
      truth_index = truth_soln_lev - 1;
    truthModelKey
      = Pecos::ActiveKey(id, Pecos::RAW_DATA, num_approx, truth_index);
    //if (responseMode == AGGREGATED_MODELS) {
      surrModelKeys.resize(truth_index);
      for (size_t i=0; i<truth_index; ++i)
	surrModelKeys[i] = Pecos::ActiveKey(id, Pecos::RAW_DATA, num_approx, i);
    //}
  }
  // raw data only (no data reduction)
  activeKey.aggregate_keys(surrModelKeys, truthModelKey, Pecos::RAW_DATA);

  /* Old approach for paired models:
  unsigned short id = 0, last_m = approxModels.size();
  short r_type = Pecos::RAW_DATA;
  if (multilevel_multifidelity()) { // first and last model form / soln levels
    //size_t last_soln_lev = std::min(truthModel.solution_levels(),
    // 				      approxModels[0].solution_levels());
    //truthModelKey = Pecos::ActiveKey(id, r_type, last_m, last_soln_lev);
    //surrModelKey  = Pecos::ActiveKey(id, r_type,      0, last_soln_lev);

    // span both hierarchical dimensions by default
    size_t truth_soln_lev = truthModel.solution_levels();
    truthModelKey = Pecos::ActiveKey(id, r_type, last_m, truth_soln_lev - 1);
    surrModelKey  = Pecos::ActiveKey(id, r_type,      0, 0);
  }
  else if (multifidelity()) { // first and last model form (no soln levels)
    // Note: for proper modeKeyBufferSize estimation, must maintain consistency
    // with NonDExpansion::configure_{sequence,indices}() and key definition
    // for NonDMultilevelSampling::control_variate_mc() in terms of SZ_MAX
    // usage, since this suppresses allocation of a solution level array.
    truthModelKey = Pecos::ActiveKey(id, r_type, last_m,
      truthModel.solution_level_cost_index());
    surrModelKey  = Pecos::ActiveKey(id, r_type,      0,
      approxModels[0].solution_level_cost_index());
  }
  else if (multilevel()) { // first and last solution level (last model)
    size_t truth_soln_lev = truthModel.solution_levels();
    truthModelKey = Pecos::ActiveKey(id, r_type, last_m, truth_soln_lev - 1);
    surrModelKey  = Pecos::ActiveKey(id, r_type, last_m, 0);
  }
  activeKey.aggregate_keys(surrModelKey, truthModelKey,
			   Pecos::SINGLE_REDUCTION); // default: reduction only
  */

  if (parallelLib.mpirun_flag()) {
    MPIPackBuffer send_buff;  short mode(0);
    send_buff << mode << activeKey; // serve_run() recvs single | aggregate key
    modeKeyBufferSize = send_buff.size();
  }

  check_model_interface_instance();
}


void EnsembleSurrModel::
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
              num_models  = approxModels.size();
    if (!truthModel.is_null()) ++num_models;

    // For now, use the DB method name to construct a list of methods that
    // might perform gradient-based minimization.  Note: EnsembleSurrModel
    // has no construct on the fly option at this time.
    unsigned short method_name = probDescDB.get_ushort("method.algorithm");
    bool extra_deriv_config = (method_name  & MINIMIZER_BIT      ||
			       method_name == BAYES_CALIBRATION  ||
			       method_name == LOCAL_RELIABILITY  ||
			       method_name == LOCAL_INTERVAL_EST ||
			       method_name == LOCAL_EVIDENCE);

    for (i=0; i<num_models; ++i) {
      Model& model_i = model_from_index(i);
      probDescDB.set_db_model_nodes(model_i.model_id());
      model_i.init_communicators(pl_iter, max_eval_concurrency);
      if (extra_deriv_config)
        model_i.init_communicators(pl_iter, model_i.derivative_concurrency());
    }

    // OLD LOGIC:
    // init and free must cover possible subset of active responseModes and
    // active model subsets.  In order to reduce the number of parallel configs,
    // we group the responseModes into two sets: (1) the correction-based set
    // commonly used in surrogate-based optimization et al., and (2) the
    // aggregation-based set commonly used in multilevel/multifidelity UQ.

    // TO DO: would like a better detection option, but passing the mode from
    // the Iterator does not work in parallel w/o an additional bcast (Iterator
    // only instantiated on iteratorComm rank 0).  For now, we will infer it
    // from an associated method spec at init time.
    // Note: responseMode gets bcast at run time in component_parallel_mode()

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
    case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
      // LF and HF are used in iterator evals
      lf_model.init_communicators(pl_iter, max_eval_concurrency);
      hf_model.init_communicators(pl_iter, max_eval_concurrency);
      break;
    }
    */

    probDescDB.set_db_model_nodes(model_index); // restore all model nodes
  }
}


void EnsembleSurrModel::
derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                          bool recurse_flag)
{
  miPLIndex = modelPCIter->mi_parallel_level_index(pl_iter);// run time setting

  // EnsembleSurrModels do not utilize default set_ie_asynchronous_mode() as
  // they do not define the ie_parallel_level

  if (recurse_flag) {

    // bcast not needed for recurse_flag=false in serve_run call to set_comms
    //if (pl_iter->server_communicator_size() > 1)
    //  parallelLib.bcast(responseMode, *pl_iter);

    /* Consolidated approach not good for asynchEvalFlag, evaluationCapacity

    // use_deriv_conc consistent with case logic in other code block below
    bool use_deriv_conc = (responseMode == AUTO_CORRECTED_SURROGATE);
    // Loop over all models:
    size_t i, num_models = approxModels.size();  int cap_i;
    if (!truthModel.is_null()) ++num_models;
    asynchEvalFlag = false; evaluationCapacity = 1;
    for (i=0; i<num_models; ++i) {
      Model& model_i = model_from_index(i);
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
    */

    // This aggressive logic is appropriate for invocations of the Model via
    // Iterator::run(), but is fragile w.r.t. invocations of the Model outside
    // this scope (e.g., Model::evaluate() within SBLMinimizer).  The default
    // responseMode value is {AUTO_,UN}CORRECTED_SURROGATE, which mitigates
    // the specific case of SBLMinimizer, but the general fragility remains.
    switch (responseMode) {

    // CASES WITH A SINGLE ACTIVE MODEL:
 
    case UNCORRECTED_SURROGATE: {
      Model& lf_model = active_surrogate_model(0);
      lf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = lf_model.asynch_flag();
      evaluationCapacity = lf_model.evaluation_capacity();
      break;
    }
    case BYPASS_SURROGATE: {
      Model& hf_model = active_truth_model();
      hf_model.set_communicators(pl_iter, max_eval_concurrency);
      asynchEvalFlag     = hf_model.asynch_flag();
      evaluationCapacity = hf_model.evaluation_capacity();
      break;
    }

    // CASES WHERE ANY/ALL MODELS COULD BE ACTIVE:

    default: { // AUTO_CORRECTED_SURROGATE, MODEL_DISCREPANCY,
               // AGGREGATED_MODEL_PAIR, AGGREGATED_MODELS

      // TO DO: this will not be true for multigrid optimization:
      bool use_deriv_conc = (responseMode == AUTO_CORRECTED_SURROGATE &&
			     corrType && corrOrder);
      // Either need detection logic, a passed option, or to abandon the
      // specialization and just generalize init/set/free to use the max
      // of the two values...

      size_t i, num_models = approxModels.size();  int cap_i;
      if (!truthModel.is_null()) ++num_models;
      asynchEvalFlag = false;  evaluationCapacity = 1;
      for (i=0; i<num_models; ++i) {
	Model& model_i = model_from_index(i);
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
    }
  }
}


void EnsembleSurrModel::
derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                           bool recurse_flag)
{
  if (recurse_flag) {

    size_t i, num_models = approxModels.size();
    if (!truthModel.is_null()) ++num_models;
    bool extra_deriv_config = true; // falls through if not defined
    for (i=0; i<num_models; ++i) {
      Model& model_i = model_from_index(i);
      model_i.free_communicators(pl_iter, max_eval_concurrency);
      if (extra_deriv_config)
        model_i.free_communicators(pl_iter, model_i.derivative_concurrency());
    }

    // OLD LOGIC:
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
    case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
      lf_model.free_communicators(pl_iter, max_eval_concurrency);
      hf_model.free_communicators(pl_iter, max_eval_concurrency);
      break;
    }
    */
  }
}


/** Inactive variables must be propagated when an EnsembleSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool EnsembleSurrModel::initialize_mapping(ParLevLIter pl_iter)
{
  Model::initialize_mapping(pl_iter);

  // push inactive variable values/bounds from currentVariables and
  // userDefinedConstraints into {truthModel,approxModels}
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i) {
    approxModels[i].initialize_mapping(pl_iter);
    init_model(approxModels[i]);
  }

  truthModel.initialize_mapping(pl_iter);
  init_model(truthModel);

  return false; // no change to problem size
}


/** Inactive variables must be propagated when an EnsembleSurrModel
    is employed by a sub-iterator (e.g., OUU with MLMC or MLPCE).
    In current use cases, this can occur once per sub-iterator
    execution within Model::initialize_mapping(). */
bool EnsembleSurrModel::finalize_mapping()
{
  size_t i, num_approx = approxModels.size();
  for (i=0; i<num_approx; ++i)
    approxModels[i].finalize_mapping();

  truthModel.finalize_mapping();

  Model::finalize_mapping();

  return false; // no change to problem size
}


void EnsembleSurrModel::init_model(Model& model)
{
  SurrogateModel::init_model(model);

  // Rather than map all inactive variables, propagate nested mappings from
  // currentVariables into the target model
  init_model_mapped_variables(model);
  //init_model_mapped_labels(model);
}


void EnsembleSurrModel::init_model_mapped_variables(Model& model)
{
  /*
  // can implement this once a use-case exists
  if (secondaryACVarMapTargets.size()  || secondaryADIVarMapTargets.size() ||
      secondaryADSVarMapTargets.size() || secondaryADRVarMapTargets.size()) {
    Cerr << "Error: secondary mappings not yet supported in SurrogateModel::"
	 << "init_model_mapped_variables()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  */

  size_t i, num_map = primaryACVarMapIndices.size(), // all sizes are the same
    ac_index1, adi_index1, ads_index1, adr_index1, m_index;
  for (i=0; i<num_map; ++i) {
    ac_index1  = primaryACVarMapIndices[i];
    adi_index1 = primaryADIVarMapIndices[i];
    ads_index1 = primaryADSVarMapIndices[i];
    adr_index1 = primaryADRVarMapIndices[i];
    if (ac_index1 != _NPOS) {
      // retrieve the label of the mapped variable from currentVariables
      const String& surr_label
	= currentVariables.all_continuous_variable_labels()[ac_index1];
      // map this to sub-ordinate label variables
      m_index = find_index(model.all_continuous_variable_labels(), surr_label);
      // push value from currentVariables to sub-ordinate variables
      if (m_index != _NPOS)
	model.all_continuous_variable(
	  currentVariables.all_continuous_variables()[ac_index1], m_index);
    }
    else if (adi_index1 != _NPOS) {
      const String& surr_label
	= currentVariables.all_discrete_int_variable_labels()[adi_index1];
      m_index = find_index(model.all_discrete_int_variable_labels(),surr_label);
      if (m_index != _NPOS)
	model.all_discrete_int_variable(
	  currentVariables.all_discrete_int_variables()[adi_index1], m_index);
    }
    else if (ads_index1 != _NPOS) {
      const String& surr_label
	= currentVariables.all_discrete_string_variable_labels()[ads_index1];
      m_index = find_index(model.all_discrete_string_variable_labels(),
			   surr_label);
      if (m_index != _NPOS)
	model.all_discrete_string_variable(
	  currentVariables.all_discrete_string_variables()[ads_index1],m_index);
    }
    else if (adr_index1 != _NPOS) {
      const String& surr_label
	= currentVariables.all_discrete_real_variable_labels()[adr_index1];
      m_index = find_index(model.all_discrete_real_variable_labels(),
			   surr_label);
      if (m_index != _NPOS)
	model.all_discrete_real_variable(
	  currentVariables.all_discrete_real_variables()[adr_index1], m_index);
    }
    else {
      Cerr << "Error: undefined mapping in SurrogateModel::"
	   << "init_model_mapped_variables()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
  }
}


/** Compute the response synchronously using truthModel and approxModels. */
void EnsembleSurrModel::derived_evaluate(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  currentResponse.active_set(set);

  unsigned short m_index;
  switch (responseMode) {

  case AGGREGATED_MODELS: {
    // extract eval requirements from composite ASV
    Short2DArray indiv_asv;  asv_split(set.request_vector(), indiv_asv);
    ActiveSet set_i(set); // copy DVV
    size_t i, num_steps = indiv_asv.size();
    if (sameModelInstance) update_model(active_truth_model());
    for (i=0; i<num_steps; ++i) {
      ShortArray& asv_i = indiv_asv[i];
      if (test_asv(asv_i)) {
	assign_key(i);
	m_index = key_from_index(i).retrieve_model_form();
	component_parallel_mode(m_index+1); // index to id (0 is reserved)
	Model& model_i = model_from_index(m_index);
	if (!sameModelInstance) update_model(model_i);
	set_i.request_vector(asv_i);
	model_i.evaluate(set_i);
	// insert i-th contribution to currentResponse asrv/fns/grads/hessians
	insert_response(model_i.current_response(), i, currentResponse);
      }
    }
    break;
  }

  case BYPASS_SURROGATE: {
    if (set.request_vector().size() != qoi()) {
      Cerr << "Error: wrong ASV size for BYPASS_SURROGATE mode in "
	   << "EnsembleSurrModel::derived_evaluate()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    assign_truth_key();
    m_index = truthModelKey.retrieve_model_form();
    component_parallel_mode(m_index + 1); // index to id
    Model& hf_model = model_from_index(m_index);
    update_model(hf_model);
    hf_model.evaluate(set);
    currentResponse.update(hf_model.current_response(), true);// pull metadata
    break;
  }

  // NOTE: UNCORRECTED_SURROGATE case does not simplify to a single model as
  // for BYPASS_SURROGATE above due to possibility of mixed surrogate/truth
  // data from an id_surrogates spec

  default: { // paired cases: manage LF/HF evaluation requirements
    ShortArray lo_fi_asv, hi_fi_asv;  bool lo_fi_eval, hi_fi_eval, mixed_eval;
    Response lo_fi_response, hi_fi_response; // don't use truthResponseRef
    switch (responseMode) {
    // Note: incoming ASV is expanded size only for AGGREGATED_MODEL_PAIR
    case MODEL_DISCREPANCY:
      lo_fi_eval = hi_fi_eval = mixed_eval = true;        break;
    default: // {UN,AUTO_}CORRECTED_SURROGATE, AGGREGATED_MODEL_PAIR
      asv_split(set.request_vector(), lo_fi_asv, hi_fi_asv);
      lo_fi_eval = !lo_fi_asv.empty();  hi_fi_eval = !hi_fi_asv.empty();
      mixed_eval = (lo_fi_eval && hi_fi_eval);            break;
    }
    // Extract models corresponding to truthModelKey and surrModelKeys[0]
    Model&   lf_model = (lo_fi_eval) ? active_surrogate_model(0) : dummy_model;
    Model&   hf_model = (hi_fi_eval) ? active_truth_model()      : dummy_model;
    Model& same_model = (hi_fi_eval) ? hf_model : lf_model;
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.'+std::to_string(surrModelEvalCntr+1);
      if (sameModelInstance)
	same_model.eval_tag_prefix(eval_tag);
      else {
	if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
	if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
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
      m_index = truthModelKey.retrieve_model_form();
      component_parallel_mode(m_index + 1); // index to id (0 is reserved)
      assign_truth_key();
      if (!sameModelInstance) update_model(hf_model);
      switch (responseMode) {
      case MODEL_DISCREPANCY:
	hf_model.evaluate(set);
	hi_fi_response = (sameModelInstance) ? // shared rep
	  hf_model.current_response().copy() : hf_model.current_response();
	break;
      default: { // {UN,AUTO_}CORRECTED_SURROGATE, AGGREGATED_MODEL_PAIR
	ActiveSet hi_fi_set(hi_fi_asv, set.derivative_vector());
	hf_model.evaluate(hi_fi_set);
	if (mixed_eval)
	  hi_fi_response = (sameModelInstance) ? // deep copy or shared rep
	    hf_model.current_response().copy() : hf_model.current_response();
	else {
	  currentResponse.active_set(hi_fi_set);
	  currentResponse.update(hf_model.current_response(), true);// pull meta
	}
	break;
      }
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
      m_index = surrModelKeys[0].retrieve_model_form();
      component_parallel_mode(m_index + 1); // index to id (0 is reserved)
      assign_surrogate_key(0); // only 1 surrogate key when paired
      if (!sameModelInstance) update_model(lf_model);
      ActiveSet lo_fi_set;
      switch (responseMode) {
      case MODEL_DISCREPANCY:
	lf_model.evaluate(set);        break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE, AGGREGATED_MODEL_PAIR
	lo_fi_set.request_vector(lo_fi_asv);
	lo_fi_set.derivative_vector(set.derivative_vector());
	lf_model.evaluate(lo_fi_set);  break;
      }
      // post-process
      switch (responseMode) {
      case AUTO_CORRECTED_SURROGATE: {
	// LF resp should not be corrected directly (see derived_synchronize())
	lo_fi_response = lf_model.current_response().copy();
	recursive_apply(currentVariables, lo_fi_response);
	if (!mixed_eval) {
	  currentResponse.active_set(lo_fi_set);
	  currentResponse.update(lo_fi_response, true); // pull metadata
	}
	break;
      }
      case UNCORRECTED_SURROGATE:
	if (mixed_eval)
	  lo_fi_response = lf_model.current_response(); // shared rep
	else {
	  currentResponse.active_set(lo_fi_set);
	  currentResponse.update(lf_model.current_response(), true);// pull meta
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
      // just update currentResponse (managed as surrogate data at higher level)
      bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
      currentResponse.active_set(set);
      deltaCorr[activeKey].compute(hi_fi_response, lf_model.current_response(),
				   currentResponse, quiet_flag);
      break;
    }
    case AGGREGATED_MODEL_PAIR:
      aggregate_response(lf_model.current_response(), hi_fi_response,
			 currentResponse);
      break;
    case UNCORRECTED_SURROGATE:   case AUTO_CORRECTED_SURROGATE:
      if (mixed_eval) {
	currentResponse.active_set(set);
	response_combine(hi_fi_response, lo_fi_response, currentResponse);
      }
      break;
    }
    break;
  }
  }
}


/** Compute the response asynchronously using truthModel and approxModels.
    Results are then retrieved using blocking derived_synchronize() or
    nonblocking derived_synchronize_nowait()). */
void EnsembleSurrModel::derived_evaluate_nowait(const ActiveSet& set)
{
  ++surrModelEvalCntr;

  // asynchEvalFlag is set if any model supports asynchronous, resulting in
  // use of derived_evaluate_nowait().  To manage general case of mixed asynch,
  // launch nonblocking evals first, followed by blocking evals.

  // For notes on repetitive use of assign_key(), see derived_evaluate() above

  unsigned short m_index;
  switch (responseMode) {
  case AGGREGATED_MODELS: {
    // extract eval requirements from composite ASV
    Short2DArray indiv_asv;  asv_split(set.request_vector(), indiv_asv);
    size_t i, num_steps = indiv_asv.size();
    ActiveSet set_i(set); // copy DVV
    if (sameModelInstance) update_model(active_truth_model());

    // first pass for nonblocking models
    for (i=0; i<num_steps; ++i) {
      m_index = key_from_index(i).retrieve_model_form();
      Model& model_i = model_from_index(m_index);
      ShortArray& asv_i = indiv_asv[i];
      if (model_i.asynch_flag() && test_asv(asv_i)) {
	assign_key(i);
	if (!sameModelInstance) update_model(model_i);
	set_i.request_vector(asv_i);
	model_i.evaluate_nowait(set_i);
	modelIdMaps[i][model_i.evaluation_id()] = surrModelEvalCntr;
      }
    }
    // second pass for blocking models
    for (i=0; i<num_steps; ++i) {
      m_index = key_from_index(i).retrieve_model_form();
      Model& model_i = model_from_index(m_index);
      ShortArray& asv_i = indiv_asv[i];
      if (!model_i.asynch_flag() && test_asv(asv_i)) {
	assign_key(i);
	component_parallel_mode(m_index+1); // index to id (0 is reserved)
	if (!sameModelInstance) update_model(model_i);
	set_i.request_vector(asv_i);
	model_i.evaluate(set_i);
	cachedRespMaps[i][surrModelEvalCntr]
	  = model_i.current_response().copy();
      }
    }
    break;
  }

  case BYPASS_SURROGATE: {
    if (set.request_vector().size() != qoi()) {
      Cerr << "Error: wrong ASV size for BYPASS_SURROGATE mode in "
	   << "EnsembleSurrModel::derived_evaluate()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    assign_truth_key();
    m_index = truthModelKey.retrieve_model_form();
    component_parallel_mode(m_index + 1); // index to id
    Model& hf_model = model_from_index(m_index);
    update_model(hf_model);
    hf_model.evaluate_nowait(set); // no need to test for blocking eval
    modelIdMaps[0][hf_model.evaluation_id()] = surrModelEvalCntr;
    break;
  }

  default: { // paired cases: manage LF/HF evaluation requirements
    ShortArray lo_fi_asv, hi_fi_asv;  bool lo_fi_eval, hi_fi_eval;
    switch (responseMode) {
    case MODEL_DISCREPANCY:
      lo_fi_eval = hi_fi_eval = true;                                     break;
    default: // {UN,AUTO_}CORRECTED_SURROGATE, AGGREGATED_MODEL_PAIR
      asv_split(set.request_vector(), lo_fi_asv, hi_fi_asv);
      lo_fi_eval = !lo_fi_asv.empty();  hi_fi_eval = !hi_fi_asv.empty();  break;
    }
    // Extract models corresponding to truthModelKey and surrModelKeys[0]
    Model&   lf_model = (lo_fi_eval) ? active_surrogate_model(0) : dummy_model;
    Model&   hf_model = (hi_fi_eval) ? active_truth_model()      : dummy_model;
    Model& same_model = (hi_fi_eval) ? hf_model : lf_model;
    bool asynch_lo_fi = (lo_fi_eval) ? lf_model.asynch_flag() : false,
         asynch_hi_fi = (hi_fi_eval) ? hf_model.asynch_flag() : false;
    if (hierarchicalTagging) {
      String eval_tag = evalTagPrefix + '.'+std::to_string(surrModelEvalCntr+1);
      if (sameModelInstance)
	same_model.eval_tag_prefix(eval_tag);
      else {
	if (lo_fi_eval) lf_model.eval_tag_prefix(eval_tag);
	if (hi_fi_eval) hf_model.eval_tag_prefix(eval_tag);
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
      case MODEL_DISCREPANCY:
	hi_fi_set.request_vector(set.request_vector());  break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE, AGGREGATED_MODEL_PAIR
	hi_fi_set.request_vector(hi_fi_asv);             break;
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
      case MODEL_DISCREPANCY:
	lo_fi_set.request_vector(set.request_vector());  break;
      default: // {UN,AUTO_}CORRECTED_SURROGATE, AGGREGATED_MODEL_PAIR
	lo_fi_set.request_vector(lo_fi_asv);             break;
      }
    }

    // asynchEvalFlag is set if _either_ LF or HF is asynchronous, resulting in
    // use of derived_evaluate_nowait().  To manage general case of mixed
    // asynch, launch nonblocking evals first, followed by blocking evals.

    // For notes on repetitive setting of model.solution_level_cost_index(), see
    // derived_evaluate() above.

    // launch nonblocking evals before any blocking ones
    if (hi_fi_eval && asynch_hi_fi) { // HF model may be executed asynchronously
      // don't need to set component parallel mode since only queues the job
      assign_truth_key();
      hf_model.evaluate_nowait(hi_fi_set);
      // store map from HF eval id to EnsembleSurrModel id
      modelIdMaps[1][hf_model.evaluation_id()] = surrModelEvalCntr;
    }
    if (lo_fi_eval && asynch_lo_fi) { // LF model may be executed asynchronously
      // don't need to set component parallel mode since only queues the job
      assign_surrogate_key(0); // only 1 surrogate key when paired
      lf_model.evaluate_nowait(lo_fi_set);
      // store map from LF eval id to EnsembleSurrModel id
      modelIdMaps[0][lf_model.evaluation_id()] = surrModelEvalCntr;
      // store variables set needed for correction
      if (responseMode == AUTO_CORRECTED_SURROGATE)
	rawVarsMap[surrModelEvalCntr] = currentVariables.copy();
    }

    // now launch any blocking evals
    if (hi_fi_eval && !asynch_hi_fi) { // execute HF synchronously & cache resp
      m_index = truthModelKey.retrieve_model_form();
      component_parallel_mode(m_index + 1); // index to id (0 is reserved)
      assign_truth_key();
      hf_model.evaluate(hi_fi_set);
      // not part of rekey_synch(); can rekey to surrModelEvalCntr immediately
      cachedRespMaps[1][surrModelEvalCntr] = hf_model.current_response().copy();
    }
    if (lo_fi_eval && !asynch_lo_fi) { // execute LF synchronously & cache resp
      m_index = surrModelKeys[0].retrieve_model_form();
      component_parallel_mode(m_index + 1); // index to id (0 is reserved)
      assign_surrogate_key(0);
      lf_model.evaluate(lo_fi_set);
      Response lo_fi_response(lf_model.current_response().copy());
      // correct LF response prior to caching
      if (responseMode == AUTO_CORRECTED_SURROGATE)
	// correct synch cases now (asynch cases get corrected in
	// derived_synchronize_aggregate*)
	recursive_apply(currentVariables, lo_fi_response);
      // cache corrected LF response for retrieval during synchronization.
      // not part of rekey_synch(); can rekey to surrModelEvalCntr immediately.
      cachedRespMaps[0][surrModelEvalCntr] = lo_fi_response;// deep copied above
    }
    break;
  }
  }
}


/** Blocking retrieval of asynchronous evaluations from LF model, HF
    model, or both (mixed case).  For the LF model portion, apply
    correction (if active) to each response in the array.
    derived_synchronize() is designed for the general case where
    derived_evaluate_nowait() may be inconsistent in its use of low
    fidelity evaluations, high fidelity evaluations, or both. */
const IntResponseMap& EnsembleSurrModel::derived_synchronize()
{
  surrResponseMap.clear();

  if (sameModelInstance || sameInterfaceInstance ||
      count_id_maps(modelIdMaps) <= 1) { // 1 queue: blocking synch
    IntResponseMapArray model_resp_maps_rekey(modelIdMaps.size()); // num_steps
    derived_synchronize_sequential(model_resp_maps_rekey, true);
    derived_synchronize_combine(model_resp_maps_rekey, surrResponseMap);
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
const IntResponseMap& EnsembleSurrModel::derived_synchronize_nowait()
{
  surrResponseMap.clear();

  IntResponseMapArray model_resp_maps_rekey(modelIdMaps.size());
  derived_synchronize_sequential(model_resp_maps_rekey, false);
  derived_synchronize_combine_nowait(model_resp_maps_rekey, surrResponseMap);

  return surrResponseMap;
}


void EnsembleSurrModel::
derived_synchronize_sequential(IntResponseMapArray& model_resp_maps_rekey,
			       bool block)
{
  size_t i, num_steps = modelIdMaps.size();  unsigned short m_index;
  if (sameModelInstance) {

    // Seems sufficient to do this once and not reassign the i-th key on the
    // servers in order to communicate the resolution level
    // (ApplicationInterface::send_evaluation() includes full variables object
    // from beforeSynchCorePRPQueue, which synchronizes inactive state).
    // Otherwise need to move it inside loop and split synchronize apart again.
    m_index = truthModelKey.retrieve_model_form();
    component_parallel_mode(m_index+1); // index to id (0 is reserved)

    rekey_synch(model_from_index(m_index), block, modelIdMaps,
		model_resp_maps_rekey);
  }
  else {
    bool deep_copy, auto_corr = (responseMode == AUTO_CORRECTED_SURROGATE);
    size_t num_surr_keys = surrModelKeys.size();
    for (i=0; i<num_steps; ++i) {
      m_index = key_from_index(i).retrieve_model_form();
      IntIntMap& model_id_map_i = modelIdMaps[i];
      if (!model_id_map_i.empty()) { // synchronize evals for i-th Model
	component_parallel_mode(m_index+1); // index to id (0 is reserved)
	Model& model_i = model_from_index(m_index);
	// Note: unmatched Model::responseMap are moved to Model::
	//       cachedResponseMap for return on next synchronize()
	deep_copy = (auto_corr && i < num_surr_keys);
	rekey_synch(model_i, block, model_id_map_i,
		    model_resp_maps_rekey[i], deep_copy);
      }
    }

    /* Old code for paired models:
    model_resp_maps_rekey.resize(2);
    IntIntMap &lf_id_map = modelIdMaps[0], &hf_id_map = modelIdMaps[1];
    // --------------------------
    // synchronize HF model evals
    // --------------------------
    if (!hf_id_map.empty()) { // synchronize HF evals
      component_parallel_mode(TRUTH_MODEL_MODE);
      rekey_synch(truthModel, block, hf_id_map, model_resp_maps_rekey[1]);
      // Note: for sameModelInstance, unmatched Model::responseMap are moved to
      //       Model::cachedResponseMap for return on next synchronize()
    }
    // --------------------------
    // synchronize LF model evals
    // --------------------------
    if (!lf_id_map.empty()) { // synchronize LF evals
      component_parallel_mode(SURROGATE_MODEL_MODE);
      // Interface::rawResponseMap should _not_ be corrected directly since
      // rawResponseMap, beforeSynchCorePRPQueue, and data_pairs all share a
      // responseRep -> modifying rawResponseMap affects data_pairs.
      bool deep_copy = (responseMode == AUTO_CORRECTED_SURROGATE);
      rekey_synch(active_surrogate_model(0), block, lf_id_map,
		  model_resp_maps_rekey[0], deep_copy);
    }
    */
  }

  for (i=0; i<num_steps; ++i) {
    // add cached evals from:
    // (a) recovered asynch evals that could not be returned since other model
    //     eval portions were still pending, or
    // (b) synchronous model evals performed within evaluate_nowait()
    IntResponseMap& cached_map_i = cachedRespMaps[i];
    model_resp_maps_rekey[i].insert(cached_map_i.begin(), cached_map_i.end());
    cached_map_i.clear(); // clear map
  }
}


void EnsembleSurrModel::
derived_synchronize_combine(IntResponseMapArray& model_resp_maps,
                            IntResponseMap& combined_resp_map)
{
  // --------------------------------------------
  // perform blocking IntResponseMap aggregations
  // --------------------------------------------

  switch (responseMode) {

  case AGGREGATED_MODELS: {
    // loop over model_resp_maps and insert() into offset position.  Notes:
    // > cachedRespMaps have been inserted into model_resp_maps
    // > rekey_synch() has migrated from indiv model ids to surrModelEvalCntr
    size_t i, num_steps = model_resp_maps.size();  IntRespMCIter r_cit;
    for (i=0; i<num_steps; ++i) {
      const IntResponseMap& resp_map = model_resp_maps[i];
      for (r_cit=resp_map.begin(); r_cit!=resp_map.end(); ++r_cit)
	insert_response(r_cit->second, i,
			combined_resp_map[r_cit->first]); // already rekeyed
    }
    break;
  }

  case BYPASS_SURROGATE:
    combined_resp_map = model_resp_maps[0]; // one truth model
    //std::swap(combined_resp_map, model_resp_maps[0]); // can't swap w/ const
    break;

  default: { // paired cases
    // {hf,lf}_resp_map may be partial sets (partial surrogateFnIndices
    // in {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY,
    // AGGREGATED_MODEL_PAIR).
    IntResponseMap&       lf_resp_map = model_resp_maps[0];
    const IntResponseMap& hf_resp_map = model_resp_maps[1];
    IntRespMCIter hf_cit = hf_resp_map.begin();
    IntRespMIter  lf_it  = lf_resp_map.begin();
    bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
    switch (responseMode) {
    case MODEL_DISCREPANCY: {
      DiscrepancyCorrection& delta_corr = deltaCorr[activeKey];
      for (; hf_cit != hf_resp_map.end() && lf_it != lf_resp_map.end();
	   ++hf_cit, ++lf_it) {
	check_key(hf_cit->first, lf_it->first);
	delta_corr.compute(hf_cit->second, lf_it->second,
			   combined_resp_map[hf_cit->first], quiet_flag);
      }
      break;
    }
    case AGGREGATED_MODEL_PAIR:
      for (; hf_cit != hf_resp_map.end() && lf_it != lf_resp_map.end();
	   ++hf_cit, ++lf_it) {
	check_key(hf_cit->first, lf_it->first);
	aggregate_response(lf_it->second, hf_cit->second,
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
      while (hf_cit != hf_resp_map.end() || lf_it != lf_resp_map.end()) {
	// these have been rekeyed already to top-level surrModelEvalCntr:
	int hf_eval_id = (hf_cit == hf_resp_map.end()) ? INT_MAX: hf_cit->first;
	int lf_eval_id = (lf_it == lf_resp_map.end())  ? INT_MAX : lf_it->first;

	if (hf_eval_id < lf_eval_id) { // only HF available
	  response_combine(hf_cit->second, empty_resp,
			   combined_resp_map[hf_eval_id]);
	  ++hf_cit;
	}
	else if (lf_eval_id < hf_eval_id) { // only LF available
	  response_combine(empty_resp, lf_it->second,
			   combined_resp_map[lf_eval_id]);
	  ++lf_it;
	}
	else { // both LF and HF available
	  response_combine(hf_cit->second, lf_it->second,
			   combined_resp_map[hf_eval_id]);
	  ++hf_cit;
	  ++lf_it;
	}
      }
      break;
    }
    }
    break;
  }
  }
}


void EnsembleSurrModel::derived_synchronize_competing()
{
  // in this case, we don't want to starve either LF or HF scheduling by
  // blocking on one or the other --> leverage derived_synchronize_nowait()
  IntResponseMap aggregated_map; // accumulate surrResponseMap returns
  while (test_id_maps(modelIdMaps)) {
    // partial_map is a reference to surrResponseMap, returned by _nowait()
    const IntResponseMap& partial_map = derived_synchronize_nowait();
    if (!partial_map.empty())
      aggregated_map.insert(partial_map.begin(), partial_map.end());
  }

  // Note: cached response maps and any LF/HF aggregations are managed
  // within derived_synchronize_nowait()

  std::swap(surrResponseMap, aggregated_map);
}


void EnsembleSurrModel::
derived_synchronize_combine_nowait(IntResponseMapArray& model_resp_maps,
				   IntResponseMap& combined_resp_map)
{
  // ------------------------------------------------
  // perform non-blocking IntResponseMap aggregations
  // ------------------------------------------------

  switch (responseMode) {

  case AGGREGATED_MODELS: {
    // loop over model_resp_maps and insert() into offset position.  Notes:
    // > cachedRespMaps have been inserted into model_resp_maps
    // > rekey_synch() has migrated from indiv model ids to surrModelEvalCntr

    size_t i, num_steps = model_resp_maps.size();  IntRespMCIter r_cit;
    // assemble set of aggregate ids which still have pending contributions
    // (only pending jobs remain in modelIdMaps after nonblocking synch)
    IntSet pending_ids;  IntIntMCIter id_it;
    for (i=0; i<num_steps; ++i) {
      const IntIntMap& id_map_i = modelIdMaps[i];
      for (id_it=id_map_i.begin(); id_it!=id_map_i.end(); ++id_it)
	pending_ids.insert(id_it->second); // duplicates ignored
    }

    // process completed job sets or reinsert partial results into cache
    // Approach 1: repeated pending id lookups on innermost loop
    //for (i=0; i<num_steps; ++i) {
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
    for (i=0; i<num_steps; ++i) {
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
    break;
  }

  case BYPASS_SURROGATE:
    combined_resp_map = model_resp_maps[0]; // one truth model
    //std::swap(combined_resp_map, model_resp_maps[0]); // can't swap w/ const
    break;

  default: { // paired cases
    IntResponseMap&       lf_resp_map = model_resp_maps[0];
    const IntResponseMap& hf_resp_map = model_resp_maps[1];
    IntIntMap &lf_id_map = modelIdMaps[0], &hf_id_map = modelIdMaps[1];
    IntResponseMap &lf_cached_map = cachedRespMaps[0],
                   &hf_cached_map = cachedRespMaps[1];
    // {hf,lf}_resp_map may be partial sets (partial surrogateFnIndices
    // in {UN,AUTO_}CORRECTED_SURROGATE) or full sets (MODEL_DISCREPANCY).

    // Early return options avoid some overhead:
    if (lf_resp_map.empty() && lf_id_map.empty()) { // none completed or pending
      combined_resp_map = hf_resp_map;  // can't swap w/ const
      return;
    }
    if (responseMode == AUTO_CORRECTED_SURROGATE)
      compute_apply_delta(lf_resp_map);
    if (hf_resp_map.empty() && hf_id_map.empty()) { // none completed or pending
      std::swap(combined_resp_map, lf_resp_map);
      return;
    }

    // invert remaining entries (pending jobs) in hf_id_map and lf_id_map
    IntIntMap remain_truth_ids, remain_surr_ids;
    IntIntMIter id_it;
    for (id_it=hf_id_map.begin(); id_it!=hf_id_map.end(); ++id_it)
      remain_truth_ids[id_it->second] = id_it->first;
    for (id_it=lf_id_map.begin();  id_it!=lf_id_map.end();  ++id_it)
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
	case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
	  // LF contribution is pending -> cache HF response
	  hf_cached_map[hf_eval_id] = hf_cit->second;
	  break;
	default: // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
	  if (remain_surr_ids.find(hf_eval_id) != remain_surr_ids.end())
	    // LF contribution is pending -> cache HF response
	    hf_cached_map[hf_eval_id] = hf_cit->second;
	  else // no LF component is pending -> HF contribution is sufficient
	    response_combine(hf_cit->second, empty_resp,
			     surrResponseMap[hf_eval_id]);
	  break;
	}
	++hf_cit;
      }
      else if (lf_eval_id < hf_eval_id) { // only LF available
	switch (responseMode) {
	case MODEL_DISCREPANCY: case AGGREGATED_MODEL_PAIR:
	  // HF contribution is pending -> cache LF response
	  lf_cached_map[lf_eval_id] = lf_it->second;  break;
	default: // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
	  if (remain_truth_ids.find(lf_eval_id) != remain_truth_ids.end())
	    // HF contribution is pending -> cache LF response
	    lf_cached_map[lf_eval_id] = lf_it->second;
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
	case MODEL_DISCREPANCY:
	  dc_it->second.compute(hf_cit->second, lf_it->second,
				surrResponseMap[hf_eval_id], quiet_flag); break;
	case AGGREGATED_MODEL_PAIR:
	  aggregate_response(lf_it->second, hf_cit->second,
			     surrResponseMap[hf_eval_id]);                break;
	default: // {UNCORRECTED,AUTO_CORRECTED,BYPASS}_SURROGATE modes
	  response_combine(hf_cit->second, lf_it->second,
			   surrResponseMap[hf_eval_id]);                  break;
	}
	++hf_cit;  ++lf_it;
      }
    }
    break;
  }
  }
}


void EnsembleSurrModel::
asv_split(const ShortArray& orig_asv, ShortArray& approx_asv,
	  ShortArray& actual_asv, bool build_flag)
{
  size_t i, num_qoi = qoi();
  switch (responseMode) {
  case AGGREGATED_MODEL_PAIR: {
    // split actual & approx asv (can ignore build_flag)
    if (orig_asv.size() != 2*num_qoi) {
      Cerr << "Error: ASV not aggregated for AGGREGATED_MODEL_PAIR mode in "
	   << "SurrogateModel::asv_split()." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    approx_asv.resize(num_qoi); actual_asv.resize(num_qoi);
    // aggregated response uses {HF,LF} order:
    for (i=0; i<num_qoi; ++i)
      approx_asv[i] = orig_asv[i];
    for (i=0; i<num_qoi; ++i)
      actual_asv[i] = orig_asv[i+num_qoi];
    break;
  }
  default: // non-aggregated modes have consistent ASV request vector lengths
    if (surrogateFnIndices.size() == num_qoi) {
      if (build_flag) actual_asv = orig_asv;
      else            approx_asv = orig_asv;
    }
    // else response set is mixed:
    else if (build_flag) { // construct mode: define actual_asv
      actual_asv.assign(num_qoi, 0);
      for (StSIter it=surrogateFnIndices.begin();
	   it!=surrogateFnIndices.end(); ++it)
	actual_asv[*it] = orig_asv[*it];
    }
    else { // eval mode: define actual_asv & approx_asv contributions
      for (i=0; i<num_qoi; ++i) {
	short orig_asv_val = orig_asv[i];
	if (orig_asv_val) {
	  if (surrogateFnIndices.count(i)) {
	    if (approx_asv.empty()) // keep empty if no active requests
	      approx_asv.assign(num_qoi, 0);
	    approx_asv[i] = orig_asv_val;
	  }
	  else {
	    if (actual_asv.empty()) // keep empty if no active requests
	      actual_asv.assign(num_qoi, 0);
	    actual_asv[i] = orig_asv_val;
	  }
	}
      }
    }
    break;
  }
}


void EnsembleSurrModel::
asv_split(const ShortArray& aggregate_asv, Short2DArray& indiv_asv)
{
  // This API only used for AGGREGATED_MODELS mode

  size_t i, j, num_qoi = qoi();
  if (aggregate_asv.size() % num_qoi) {
    Cerr << "Error: size remainder for aggregated ASV in SurrogateModel::"
	 << "asv_split()." << std::endl;
    abort_handler(MODEL_ERROR);
  }
  size_t num_indiv = aggregate_asv.size() / num_qoi, cntr = 0;
  indiv_asv.resize(num_indiv);
  for (i=0; i<num_indiv; ++i) {
    ShortArray& asv_i = indiv_asv[i];
    asv_i.resize(num_qoi);
    for (j=0; j<num_qoi; ++j, ++cntr)
      asv_i[j] = aggregate_asv[cntr];
  }
}


void EnsembleSurrModel::active_model_key(const Pecos::ActiveKey& key)
{
  // assign activeKey
  SurrogateModel::active_model_key(key);
  // update truthModelKey and surrModelKeys
  extract_subordinate_keys(key, surrModelKeys, truthModelKey);
  // assign same{Model,Interface}Instance
  check_model_interface_instance();

  // assign extracted keys
  // If model forms are distinct (multifidelity case), can activate soln level
  // indices now and will persist; else (multilevel case) soln level is managed
  // for LF & HF contributions in derived_evaluate().
  // > Special case: multilevel data import in DataFitSurrModel::consistent()
  //   requires correct state prior to evaluations in order to find level data
  if (sameModelInstance) {
    switch (responseMode) {
    case BYPASS_SURROGATE:      case NO_SURROGATE:
      assign_truth_key();       break;
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      assign_surrogate_key(0);  break;
  //case AGGREGATED_MODELS: break; // defer setting active solution levels
    }
  }
  else { // approximations are separate models
    size_t i, num_approx = surrModelKeys.size();
    for (i=0; i<num_approx; ++i)
      assign_surrogate_key(i);
    assign_truth_key();
  }

  // if necessary, resize the response for new responseMode (performed by a
  // separate preceding update) and new active keys (above).  Since parallel
  // job scheduling only involves only 1 model at any given time, this call
  // does not need to be matched on serve_run() procs.
  resize_response();
  /// allocate modelIdMaps and cachedRespMaps arrays based on active keys
  resize_maps();

  // Pull inactive variable change up into top-level currentVariables,
  // so that data flows correctly within Model recursions?  No, current
  // design is that forward pushes are automated, but inverse pulls are 
  // generally special case invocations from Iterator code (e.g., with
  // locally-managed Model recursions).
  //update_from_model(truthModel);

  // Initialize deltaCorr for paired models
  switch (responseMode) {
  case MODEL_DISCREPANCY: case AUTO_CORRECTED_SURROGATE: {
    unsigned short lf_form = surrModelKeys[0].retrieve_model_form();
    if (lf_form != USHRT_MAX) {// LF form def'd
      DiscrepancyCorrection& delta_corr = deltaCorr[key]; // per data group
      if (!delta_corr.initialized())
	delta_corr.initialize(active_surrogate_model(0), surrogateFnIndices,
			      corrType, corrOrder);
    }
    break;
  }
  }
}


void EnsembleSurrModel::create_tabular_datastream()
{
  // This function is invoked early at run time, for which the results of
  // assign_default_keys() are available (defined from basic multifidelity()
  // and multilevel() logic) --> create_tabular_datastream() has to be fairly
  // inclusive of possible ensembles and derived_auto_graphics() can then
  // be more specialized to specific active keys.

  OutputManager& mgr = parallelLib.output_manager();
  mgr.open_tabular_datastream();

  switch (responseMode) {

  case AGGREGATED_MODELS: {
    // --------------------
    // {eval,interface} ids
    // --------------------
    // To flatten into one composite tabular format, we must rely on invariant
    // quantities rather than run-time flags like same{Model,Interface}Instance
    StringArray iface_ids;
    bool one_iface_id = matching_all_interface_ids();
    size_t l, num_l, m, num_m = approxModels.size();
    if (!truthModel.is_null()) ++num_m;
    if (one_iface_id) // invariant (sameInterfaceInstance can vary at run time)
      iface_ids.push_back("interface");
    else
      for (m=1; m<=num_m; ++m)
	iface_ids.push_back("interf_M" + std::to_string(m));
    mgr.create_tabular_header(iface_ids); // includes graphics cntr
    // ---------
    // Variables
    // ---------
    // FUTURE: manage solution level control variables
    // For now, enumerate model instances
    Variables& hf_vars = truthModel.current_variables();
    solnCntlAVIndex = (multifidelity()) ? _NPOS :
      truthModel.solution_control_variable_index();
    if (solnCntlAVIndex == _NPOS)
      mgr.append_tabular_header(hf_vars);
    else {
      num_l = truthModel.solution_levels();
      mgr.append_tabular_header(hf_vars, 0, solnCntlAVIndex); // leading set

      // output paired solution control values
      const String& soln_cntl_label = solution_control_label();
      StringArray tab_labels(num_l);
      for (l=0; l<num_l; ++l)
	tab_labels[l] = soln_cntl_label + "_L" + std::to_string(l+1);
      mgr.append_tabular_header(tab_labels);

      size_t start = solnCntlAVIndex + 1;
      mgr.append_tabular_header(hf_vars, start, hf_vars.tv() - start);
    }
    // --------
    // Response
    // --------
    //mgr.append_tabular_header(currentResponse);
    // Add HF/LF/Del prepends
    StringArray labels = currentResponse.function_labels(); // copy
    size_t q, num_qoi = qoi(), num_labels = labels.size(), cntr;
    if (solnCntlAVIndex == _NPOS)
      for (m=1, cntr=0; m<=num_m; ++m) {
	String postpend = "_M" + std::to_string(m);
	for (q=0; q<num_qoi; ++q, ++cntr)
	  labels[cntr].append(postpend);
      }
    else
      for (l=1, cntr=0; l<=num_l; ++l) {
	String postpend = "_L" + std::to_string(l);
	for (q=0; q<num_qoi; ++q, ++cntr)
	  labels[cntr].append(postpend);
      }
    mgr.append_tabular_header(labels, true); // with endl
    break;
  }

  case NO_SURROGATE: //case BYPASS_SURROGATE:
    mgr.create_tabular_header(active_truth_model().current_variables(),
			      currentResponse);
    break;

  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    mgr.create_tabular_header(active_surrogate_model(0).current_variables(),
			      currentResponse);
    break;

  case AGGREGATED_MODEL_PAIR: case MODEL_DISCREPANCY: // two models/resolutions
  case BYPASS_SURROGATE: { // use same row len since commonly alternated
    // --------------------
    // {eval,interface} ids
    // --------------------
    // To flatten into one composite tabular format, we must rely on invariant
    // quantities rather than run-time flags like same{Model,Interface}Instance
    bool one_iface_id = matching_all_interface_ids();
    StringArray iface_ids;
    if (one_iface_id) // invariant (sameInterfaceInstance can vary at run time)
      iface_ids.push_back("interface");
    else {
      iface_ids.push_back("interf_M");  //("HF_interf");
      iface_ids.push_back("interf_Mm1");//("LF_interf");
    }
    mgr.create_tabular_header(iface_ids); // includes graphics cntr
    // ---------
    // Variables
    // ---------
    // identify solution level control variable
    Model&     hf_model = active_truth_model();
    Variables& hf_vars  = hf_model.current_variables();
    // must detect ML versus MF since solution level index can exist for MF
    // and be one value per model instance
    solnCntlAVIndex = (multilevel()) ? // either ML or MLCV
      hf_model.solution_control_variable_index() : _NPOS;
    if (solnCntlAVIndex == _NPOS)
      mgr.append_tabular_header(hf_vars);
    else {
      mgr.append_tabular_header(hf_vars, 0, solnCntlAVIndex); // leading set

      // output paired solution control values
      const String& soln_cntl_label = solution_control_label();
      StringArray tab_labels(2);
      tab_labels[0] = soln_cntl_label + "_L";  // = "HF_" + soln_cntl_label;
      tab_labels[1] = soln_cntl_label + "_Lm1";// = "LF_" + soln_cntl_label;
      mgr.append_tabular_header(tab_labels);

      size_t start = solnCntlAVIndex + 1;
      mgr.append_tabular_header(hf_vars, start, hf_vars.tv() - start);
    }
    // --------
    // Response
    // --------
    //mgr.append_tabular_header(currentResponse);
    // Add Del_ pre-pend or model/resolution post-pends
    StringArray labels = currentResponse.function_labels(); // copy
    size_t q, num_qoi = qoi(), num_labels = labels.size();
    if (responseMode == MODEL_DISCREPANCY)
      for (q=0; q<num_qoi; ++q)
	labels[q].insert(0, "Del_");
    // Detection of the correct response label annotation is imperfect.  Basing
    // label alternation below on active solution level control seems the best
    // option -- improving it would require either knowledge of methodName
    // (violates encapsulation) or detection of the changing models/resolutions
    // (not known until run time)
    else if (solnCntlAVIndex == _NPOS) {
      for (q=0; q<num_qoi; ++q)
	labels[q].append("_M");  //labels[q].insert(0, "HF_");
      for (q=num_qoi; q<num_labels; ++q)
	labels[q].append("_Mm1");//labels[q].insert(0, "LF_");
    }
    else { // solution levels are present, but they might not be active
      for (q=0; q<num_qoi; ++q)
	labels[q].append("_L");  //labels[q].insert(0, "HF_");
      for (q=num_qoi; q<num_labels; ++q)
	labels[q].append("_Lm1");//labels[q].insert(0, "LF_");
    }
    mgr.append_tabular_header(labels, true); // include EOL
    break;
  }
  }
}


void EnsembleSurrModel::
derived_auto_graphics(const Variables& vars, const Response& resp)
{
  //parallelLib.output_manager().add_tabular_data(vars, interface_id(), resp);

  // As called from Model::evaluate() et al., passed data are top-level Model::
  // currentVariables (neglecting inactive specializations among {HF,LF} vars)
  // and final reduced/aggregated Model::currentResponse.  Active input/output
  // components are shared among the subordinate models, but inactive components
  // must be managed to provide sensible composite tabular output.
  // > Differences in solution control are handled via specialized handling for
  //   a solution control index.
  // > Other uncontrolled inactive variables must rely on the correct
  //   subordinate model Variables instance.

  OutputManager& output_mgr = parallelLib.output_manager();
  switch (responseMode) {

  case AGGREGATED_MODELS: { // use same #Cols since commonly alternated
    // Output interface id(s)
    bool one_iface_id = matching_all_interface_ids();
    StringArray iface_ids;  size_t i, num_m = approxModels.size();
    if (!truthModel.is_null()) ++num_m;
    if (one_iface_id) // invariant (sameInterfaceInstance can vary at run time)
      iface_ids.push_back(truthModel.interface_id());
    else
      for (i=0; i<num_m; ++i) {
	if (find_model_in_keys(i))
	  iface_ids.push_back(model_from_index(i).interface_id());
	else iface_ids.push_back("N/A");
      }
    output_mgr.add_tabular_data(iface_ids); // includes graphics cntr

    // Output Variables data
    // capture correct inactive by using lower-level variables
    Variables& export_vars = truthModel.current_variables();
    if (asynchEvalFlag) export_vars.active_variables(vars); // reqd for parallel
    if (solnCntlAVIndex == _NPOS)
      output_mgr.add_tabular_data(export_vars);
    else {
      // output leading set of variables in spec order
      output_mgr.add_tabular_data(export_vars, 0, solnCntlAVIndex);

      // output solution control values (flags are not invariant, but data
      // count is). If sameModelInstance, desired soln cntl was overwritten
      // by last model's value and must be temporarily restored.
      for (i=0; i<num_m; ++i) {
	if (find_model_in_keys(i)) {
	  if (sameModelInstance) assign_key(i);
	  add_tabular_solution_level_value(model_from_index(i));
	}
	else output_mgr.add_tabular_scalar("N/A");
      }

      // output trailing variables in spec order
      size_t start = solnCntlAVIndex + 1;
      output_mgr.add_tabular_data(export_vars, start, export_vars.tv() - start);
    }

    // Output Response data
    output_mgr.add_tabular_data(resp);
    /* This block no longer necessary with Response tabular change to
       output N/A for inactive functions
    size_t q, num_qoi = truthModel.qoi(), cntr = 0;
    for (i=0; i<num_approx; ++i) {
      if (surr_keys && !surrModelKeys[i].empty())
	output_mgr.add_tabular_data(resp, cntr, num_qoi);
      else
	for (q=0; q<num_qoi; ++q)
	  output_mgr.add_tabular_scalar("N/A");
      cntr += num_qoi;
    }
    if (truth_key)
      output_mgr.add_tabular_data(resp, cntr, num_qoi);
    else
      for (q=0; q<num_qoi; ++q)
	output_mgr.add_tabular_scalar("N/A");
    output_mgr.add_eol();
    */
    break;
  }

  case NO_SURROGATE: { //case BYPASS_SURROGATE:
    Model& hf_model = active_truth_model();
    output_mgr.add_tabular_data(hf_model.current_variables(),
				hf_model.interface_id(), resp);
    break;
  }

  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE: {
    Model& lf_model = active_surrogate_model(0);
    output_mgr.add_tabular_data(lf_model.current_variables(),
				lf_model.interface_id(), resp);
    break;
  }

  case AGGREGATED_MODEL_PAIR: case MODEL_DISCREPANCY: // two models/resolutions
  case BYPASS_SURROGATE: { // use same #Cols since commonly alternated
    Model& lf_model = active_surrogate_model(0); // rtns dummy_model for BYPASS
    Model& hf_model = active_truth_model();
    // Output interface ids, potentially paired
    bool one_iface_id = matching_all_interface_ids(),
      truth_key = !truthModelKey.empty(),
      surr_key  = (!surrModelKeys.empty() && !surrModelKeys[0].empty());
    StringArray iface_ids;
    if (one_iface_id) // invariant (sameInterfaceInstance can vary at run time)
      iface_ids.push_back(hf_model.interface_id());
    else {
      if (truth_key) iface_ids.push_back(hf_model.interface_id());
      else           iface_ids.push_back("N/A");//preserve row len
      if (surr_key)  iface_ids.push_back(lf_model.interface_id());
      else           iface_ids.push_back("N/A");//preserve row len
    }
    output_mgr.add_tabular_data(iface_ids); // includes graphics cntr

    // Output Variables data
    // capture correct inactive by using lower-level variables
    Variables& export_vars = hf_model.current_variables();
    if (asynchEvalFlag) export_vars.active_variables(vars); // reqd for parallel
    if (solnCntlAVIndex == _NPOS)
      output_mgr.add_tabular_data(export_vars);
    else {
      // output leading set of variables in spec order
      output_mgr.add_tabular_data(export_vars, 0, solnCntlAVIndex);

      // output paired solution control values (flags are not invariant,
      // but data count is)
      if (sameModelInstance && truth_key && surr_key) {//data count is invariant
	// HF soln cntl was overwritten by LF and must be temporarily restored
	assign_truth_key();      add_tabular_solution_level_value(hf_model);
	assign_surrogate_key(0); add_tabular_solution_level_value(lf_model);
      }
      else { // HF and LF soln levels are not overlapping
	if (truth_key)  add_tabular_solution_level_value(hf_model);
	else output_mgr.add_tabular_scalar("N/A");// preserve consistent row len
	if ( surr_key)  add_tabular_solution_level_value(lf_model);
	else output_mgr.add_tabular_scalar("N/A");// preserve consistent row len
      }

      // output trailing variables in spec order
      size_t start = solnCntlAVIndex + 1;
      output_mgr.add_tabular_data(export_vars, start, export_vars.tv() - start);
    }

    // Output response data
    if (surr_key)
      output_mgr.add_tabular_data(resp);        // include EOL
    else { // inactive: match header by padding empty cols with "N/A"
      output_mgr.add_tabular_data(resp, false); // defer EOL
      size_t q, num_qoi = qoi();
      for (q=0; q<num_qoi; ++q) // pad response data
	output_mgr.add_tabular_scalar("N/A");
      output_mgr.add_eol(); // now return the row
    }
    break;
  }
  }
}


void EnsembleSurrModel::resize_response(bool use_virtual_counts)
{
  size_t num_meta,
    num_truth_md = truthModel.current_response().metadata().size(),
    num_truth_fns = (use_virtual_counts) ?
    truthModel.qoi() : // allow models to consume lower-level aggregations
    truthModel.response_size(); // raw counts align w/ currentResponse raw count

  switch (responseMode) {
  case AGGREGATED_MODELS: {
    size_t i, num_surr = surrModelKeys.size();
    numFns = num_truth_fns;  num_meta = num_truth_md;
    for (i=0; i<num_surr; ++i) {
      unsigned short form = surrModelKeys[i].retrieve_model_form();
      Model& model_i = model_from_index(form);
      numFns += (use_virtual_counts) ? model_i.qoi() : model_i.response_size();
      num_meta += model_i.current_response().metadata().size();
    }
    //size_t multiplier = num_surr + 1;
    //numFns   = multiplier * num_truth_fns;
    //num_meta = multiplier * num_truth_md;
    break;
  }
  case BYPASS_SURROGATE:       case NO_SURROGATE:
    numFns = num_truth_fns;  num_meta = num_truth_md;  break;
  default:
    Model& lf_model = active_surrogate_model(0);
    size_t num_lf_meta = lf_model.current_response().metadata().size(),
      num_lf_fns = (use_virtual_counts) ?
        lf_model.qoi() : lf_model.response_size();
    switch (responseMode) {
    case AGGREGATED_MODEL_PAIR:
      numFns   = num_truth_fns + num_lf_fns;
      num_meta = num_truth_md  + num_lf_meta;            break;
    case MODEL_DISCREPANCY:
      if (num_lf_fns != num_truth_fns) {
	Cerr << "Error: mismatch in response sizes for MODEL_DISCREPANCY mode "
	     << "in EnsembleSurrModel::resize_response()." << std::endl;
	abort_handler(MODEL_ERROR);
      }
      numFns = num_truth_fns;  num_meta = num_truth_md;  break;
    case UNCORRECTED_SURROGATE:  case AUTO_CORRECTED_SURROGATE:  default:
      numFns = num_lf_fns;     num_meta = num_lf_meta;   break;
    }
    break;
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
    // --> NOT NECESSARY: Model::synchronize() and Model::serve_run() delegate
    //     to EnsembleSurrModel::{derived_synchronize,serve_run}() which
    //     delegate to synchronize() and serve_run() by the LF or HF model.
    // --> Jobs are never returned using messages containing the expanded
    //     Response object.  Expansion by combination only happens on
    //     iteratorCommRank 0 within derived_synchronize_combine{,_nowait}().
  }
  if (currentResponse.metadata().size() != num_meta)
    currentResponse.reshape_metadata(num_meta);
}


void EnsembleSurrModel::component_parallel_mode(short model_id)
{
  // This implementation differs from others that accept a par_mode enum.  Here
  // we still support the virtual API of passing a short, but we reinterpret as
  // model id (not model_index since 0 is reserved for use by stop_servers()).
  // This essentially extends from the pair of {SURROGATE,TRUTH}_MODEL_MODE to
  // an open-ended number of models.

  // mode may be correct, but can't guarantee active parallel config is in sync
  //if (componentParallelMode == model_id)
  //  return; // already in correct parallel mode

  // -----------------------------
  // terminate previous serve mode (if active)
  // -----------------------------
  // TO DO: restarting servers for a change in soln control index w/o change
  // in model may be overkill (send of state vars in vars buffer sufficient?)
  if (componentParallelMode != model_id) {//||componentParallelKey != activeKey)

    // for either model form or resolution level update (sameModelInstance or
    // not), serve_run() for the subordinate truth/approx model must be ended
    // to process the key assignment below in EnsembleSurrModel::serve_run()
    //if (!sameModelInstance)
    stop_model(componentParallelMode); // Note: must be model id, not step id

    // -----------------------
    // activate new serve mode: matches EnsembleSurrModel::serve_run(pl_iter)
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
 
    // componentParallelKey is not necessary for case where either all models
    // are active for a set of samples (AGGREGATED_MODELS mode for which any
    // drop outs are managed by ASV) or only one model is active
    // (BYPASS_SURROGATE mode)
    componentParallelMode = model_id;  //componentParallelKey = activeKey;
  }


  /* Old code supporting paired model cases:
  // -----------------------------
  // terminate previous serve mode (if active)
  // -----------------------------
  // TO DO: restarting servers for a change in soln control index w/o change
  // in model may be overkill (send of state vars in vars buffer sufficient?)
  bool restart = false;
  if (componentParallelMode != par_mode || componentParallelKey != activeKey) {
    Pecos::ActiveKey old_hf_key; std::vector<Pecos::ActiveKey> old_lf_keys;
    extract_subordinate_keys(componentParallelKey, old_lf_keys, old_hf_key,
		             componentParallelMode);
    switch (componentParallelMode) {
    case SURROGATE_MODEL_MODE:
      stop_model(old_lf_key[0].retrieve_model_form());  break;
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
  // activate new serve mode (matches EnsembleSurrModel::serve_run(pl_iter)).
  // -----------------------
  // These bcasts match the outer parallel context (pl_iter).
  if (restart && modelPCIter->mi_parallel_level_defined(miPLIndex)) {
    const ParallelLevel& mi_pl = modelPCIter->mi_parallel_level(miPLIndex);
    if (mi_pl.server_communicator_size() > 1) {
      parallelLib.bcast(par_mode, mi_pl);
      if (par_mode) { // send model index state corresponding to active mode
	<same>
      }
    }
  }
  componentParallelMode = par_mode;  componentParallelKey = activeKey;
  */
}


void EnsembleSurrModel::serve_run(ParLevLIter pl_iter, int max_eval_concurrency)
{
  set_communicators(pl_iter, max_eval_concurrency, false); // don't recurse

  // match communication from EnsembleSurrModel::component_parallel_mode()
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

      // recv model state from EnsembleSurrModel::component_parallel_mode()
      MPIUnpackBuffer recv_buffer(modeKeyBufferSize);
      parallelLib.bcast(recv_buffer, *pl_iter);
      recv_buffer >> responseMode >> activeKey; // replace previous/initial key
      // extract {truth,surr}ModelKeys, assign same{Model,Interface}Instance:
      active_model_key(activeKey);

      unsigned short m_index = componentParallelMode - 1; // id to index
      // propagate resolution level to server (redundant since send_evaluation()
      // sends all of variables object, including inactive state)
      //assign_key(m_index);
      Model& model = model_from_index(m_index);
      model.serve_run(pl_iter, max_eval_concurrency);

      /* Old code supporting paired model cases:
      if (componentParallelMode == SURROGATE_MODEL_MODE) {
	assign_surrogate_key(0); // may have been deferred

	// serve active LF model:
	active_surrogate_model(0).serve_run(pl_iter, max_eval_concurrency);
	// Note: ignores erroneous BYPASS_SURROGATE
      }
      else if (componentParallelMode == TRUTH_MODEL_MODE) {
	assign_truth_key(); // may have been deferred

	// serve active HF model, employing correct iterator concurrency:
	switch (responseMode) {
	case UNCORRECTED_SURROGATE:
	  Cerr << "Error: cannot set parallel mode to TRUTH_MODEL_MODE for a "
	       << "response mode of UNCORRECTED_SURROGATE." << std::endl;
	  abort_handler(-1);                                              break;
	case AUTO_CORRECTED_SURROGATE:
	  truthModel.serve_run(pl_iter, truthModel.derivative_concurrency());
	  break;
	case BYPASS_SURROGATE: case MODEL_DISCREPANCY:
	case AGGREGATED_MODEL_PAIR:
	  truthModel.serve_run(pl_iter, max_eval_concurrency); break;
	}
      }
      */
    }
  }
}


const String& EnsembleSurrModel::solution_control_label()
{
  size_t adv_index = truthModel.solution_control_discrete_variable_index();
  switch (truthModel.solution_control_variable_type()) {
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


void EnsembleSurrModel::add_tabular_solution_level_value(Model& model)
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


void EnsembleSurrModel::build_approximation()
{
  Cout << "\n>>>>> Building hierarchical approximation.\n";

  // perform the eval for the low fidelity model
  // NOTE: For SBO, the low fidelity eval is performed externally and its
  // response is passed into compute_correction.
  // -->> move LF model out and restructure if(!approxBuilds)
  //ActiveSet temp_set = lf_model.current_response().active_set();
  //temp_set.request_values(1);
  //if (sameModelInstance) assign_surrogate_key(0);
  //lf_model.evaluate(temp_set);
  //const Response& lo_fi_response = lf_model.current_response();

  assign_truth_key();
  Model& hf_model = active_truth_model();
  if (hierarchicalTagging) {
    String eval_tag = evalTagPrefix + '.' + std::to_string(surrModelEvalCntr+1);
    hf_model.eval_tag_prefix(eval_tag);
  }

  // set EnsembleSurrModel parallelism mode to HF model
  unsigned short m_index = truthModelKey.retrieve_model_form();
  component_parallel_mode(m_index + 1); // index to id (0 is reserved)
  //component_parallel_mode(TRUTH_MODEL_MODE);

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
  ShortArray total_asv, lf_asv, hf_asv;
  std::map<Pecos::ActiveKey, DiscrepancyCorrection>::iterator dc_it
    = deltaCorr.find(activeKey);
  if (dc_it!=deltaCorr.end() && dc_it->second.initialized())
    total_asv.assign(numFns, dc_it->second.data_order());
  else
    total_asv.assign(numFns, 1); // default: values only if no deriv correction
  asv_split(total_asv, lf_asv, hf_asv, true);

  std::map<Pecos::ActiveKey, Response>::iterator it
    = truthResponseRef.find(truthModelKey);
  if (it == truthResponseRef.end())
    it = truthResponseRef.insert(std::pair<Pecos::ActiveKey, Response>(
				 truthModelKey, currentResponse.copy())).first;
  ActiveSet hf_set = currentResponse.active_set(); // copy
  hf_set.request_vector(hf_asv);
  hf_model.evaluate(hf_set);
  it->second.update(hf_model.current_response());

  // could compute the correction to LF model here, but rely on an external
  // call for consistency with DataFitSurr and to facilitate SBO logic.  In
  // particular, lo_fi_response involves find_center(), hard conv check, etc.
  //deltaCorr[activeKey].compute(..., truthResponseRef, lo_fi_response);

  Cout << "\n<<<<< Hierarchical approximation build completed.\n";
  ++approxBuilds;
}


/*
bool EnsembleSurrModel::
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
    update_model(hf_model);
    const Variables& hf_vars = hf_model.current_variables();
    copy_data(hf_vars.inactive_continuous_variables(), referenceICVars);
    copy_data(hf_vars.inactive_discrete_variables(),   referenceIDVars);

    truthResponseRef.update(response);

    Cout << "\n<<<<< Hierarchical approximation update completed.\n";
  }
  else {
    Cerr << "Warning: cannot use anchor point in EnsembleSurrModel::"
	 << "build_approximation(RealVector&, Response&).\n";
    currentVariables.continuous_variables(c_vars);
    build_approximation();
  }
  return false; // correction is not embedded and must be computed (by SBO)
}
*/


void EnsembleSurrModel::compute_apply_delta(IntResponseMap& lf_resp_map)
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
  IntResponseMap& lf_cached_map = cachedRespMaps[0];
  for (lf_it=lf_resp_map.begin(); lf_it!=lf_resp_map.end(); ++lf_it) {
    lf_eval_id = lf_it->first;
    v_it = rawVarsMap.find(lf_eval_id);
    if (v_it != rawVarsMap.end()) {
      if (corr_comp) { // apply the correction to the LF response
	recursive_apply(v_it->second, lf_it->second);
        rawVarsMap.erase(v_it);
      }
      else // no new corrections can be applied -> cache uncorrected
        lf_cached_map.insert(*lf_it);
    }
    // else correction already applied
  }
  // remove cached responses from lf_resp_map
  if (!corr_comp)
    for (lf_it=lf_cached_map.begin(); lf_it!=lf_cached_map.end(); ++lf_it)
      lf_resp_map.erase(lf_it->first);
}


void EnsembleSurrModel::
single_apply(const Variables& vars, Response& resp,
	     const Pecos::ActiveKey& paired_key)
{
  bool quiet_flag = (outputLevel < NORMAL_OUTPUT);
  bool apply_corr = true;
  DiscrepancyCorrection& delta_corr = deltaCorr[paired_key];
  if (!delta_corr.computed()) {
    Pecos::ActiveKey truth_key;  extract_truth_key(paired_key, truth_key);
    std::map<Pecos::ActiveKey, Response>::iterator it
      = truthResponseRef.find(truth_key);
    if (it == truthResponseRef.end()) apply_corr = false; // not found
    else delta_corr.compute(vars, it->second, resp, quiet_flag);
  }
  if (apply_corr)
    delta_corr.apply(vars, resp, quiet_flag);
}


void EnsembleSurrModel::recursive_apply(const Variables& vars, Response& resp)
{
  switch (correctionMode) {
  case SINGLE_CORRECTION: case DEFAULT_CORRECTION:
    single_apply(vars, resp, activeKey);
    break;
  case FULL_MODEL_FORM_CORRECTION: {
    size_t num_models = approxModels.size() + 1;
    Pecos::ActiveKey& surr_key0 = surrModelKeys[0];
    unsigned short i, lf_form = surr_key0.retrieve_model_form();
    // perform a 1D sweep starting from current surrModelKey; this could be
    // part of a multidimensional sweep, so don't target truthModelKey at end
    Pecos::ActiveKey paired_key;// = activeKey.copy();
    paired_key.aggregate_keys(surr_key0.copy(), surr_key0.copy(),
			      Pecos::SINGLE_REDUCTION);
    for (i = lf_form; i < num_models - 1; ++i) {
      paired_key.id(i); // consistent with current pair sequences
      paired_key.assign_model_form(i,   0); // LF model form in KeyData[0]
      paired_key.assign_model_form(i+1, 1); // HF model form in KeyData[1]
      single_apply(vars, resp, paired_key);
    }
    break;
  }
  case FULL_SOLUTION_LEVEL_CORRECTION: {
    // assume a consistent model index from surrModelKey
    Pecos::ActiveKey& surr_key0 = surrModelKeys[0];
    size_t lf_lev = surr_key0.retrieve_resolution_level();
    if (lf_lev == SZ_MAX) {
      Cerr << "Error: FULL_SOLUTION_LEVEL_CORRECTION requires solution level "
	   << "within model key." << std::endl;
      abort_handler(MODEL_ERROR);
    }
    size_t i, num_levels = active_surrogate_model(0).solution_levels();
    // perform a 1D sweep starting from current surrModelKey; this could be
    // part of a multidimensional sweep, so don't target truthModelKey at end
    Pecos::ActiveKey paired_key;// = activeKey.copy();
    paired_key.aggregate_keys(surr_key0.copy(), surr_key0.copy(),
			      Pecos::SINGLE_REDUCTION);
    for (i = lf_lev; i < num_levels - 1; ++i) {
      paired_key.id(i); // consistent with current pair sequences
      paired_key.assign_resolution_level(i,   0); // coarse res in KeyData[0]
      paired_key.assign_resolution_level(i+1, 1); //   fine res in KeyData[1]
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

} // namespace Dakota
