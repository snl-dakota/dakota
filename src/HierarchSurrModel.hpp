/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020
    National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       HierarchSurrModel
//- Description: A model which manages a surrogate relationship between a low
//-              fidelity approximate model and a high fidelity truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: HierarchSurrModel.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef HIERARCH_SURR_MODEL_H
#define HIERARCH_SURR_MODEL_H

#include "EnsembleSurrModel.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"


namespace Dakota {

enum { DEFAULT_CORRECTION = 0, SINGLE_CORRECTION, FULL_MODEL_FORM_CORRECTION,
       FULL_SOLUTION_LEVEL_CORRECTION, SEQUENCE_CORRECTION };


/// Derived model class within the surrogate model branch for managing
/// hierarchical surrogates (models of varying fidelity).

/** The HierarchSurrModel class manages hierarchical models of varying
    fidelity.  The class contains an ordered array of model forms
    (fidelity ordered from low to high), where each model form may
    also contain a set of solution levels (space/time discretization,
    convergence tolerances, etc.).  At run time, one of these
    combinations is activated as the low fidelity model and used to
    perform approximate function evaluations, while another of these
    combinations is activated as the high fidelity model and used to
    provide truth evaluations for computing corrections to the low
    fidelity results. */

class HierarchSurrModel: public EnsembleSurrModel
{
public:

  //
  //- Heading: Constructors and destructor
  //

  HierarchSurrModel(ProblemDescDB& problem_db); ///< constructor
  ~HierarchSurrModel();                         ///< destructor

  //
  //- Heading: Member functions
  //

  /// return correctionMode
  const unsigned short correction_mode() const;
  /// set correctionMode
  void correction_mode(unsigned short corr_mode);

protected:

  //
  //- Heading: Virtual function redefinitions
  //

  bool initialize_mapping(ParLevLIter pl_iter);
  bool finalize_mapping();

  void derived_evaluate(const ActiveSet& set);
  void derived_evaluate_nowait(const ActiveSet& set);

  void derived_synchronize_sequential(
    IntResponseMapArray& model_resp_maps_rekey, bool block);
  void derived_synchronize_combine(IntResponseMapArray& model_resp_maps,
    IntResponseMap& combined_resp_map);
  void derived_synchronize_combine_nowait(IntResponseMapArray& model_resp_maps,
    IntResponseMap& combined_resp_map);

  size_t num_approximation_models() const;
  void assign_default_keys();
  void resize_maps();
  void resize_response(bool use_virtual_counts = true);

  size_t insert_response_start(size_t position);
  void insert_metadata(const RealArray& md, size_t position,
		       Response& agg_response);

  void nested_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2);

  DiscrepancyCorrection& discrepancy_correction();
  short correction_type();
  void  correction_type(short corr_type);
  short correction_order();

  void create_tabular_datastream();
  void derived_auto_graphics(const Variables& vars, const Response& resp);

  /// return the active low fidelity model
  Model& surrogate_model(size_t i = _NPOS);
  /// return the active low fidelity model
  const Model& surrogate_model(size_t i = _NPOS) const;

  /// return the active high fidelity model
  Model& truth_model();
  /// return the active high fidelity model
  const Model& truth_model() const;

  /// define the active model key and associated {truth,surr}ModelKey pairing
  void active_model_key(const Pecos::ActiveKey& key);
  /// remove keys for any approximations underlying orderedModels
  void clear_model_keys();

  /// return orderedModels and, optionally, their sub-model recursions
  void derived_subordinate_models(ModelList& ml, bool recurse_flag);

  /// resize currentResponse if needed when one of the subordinate
  /// models has been resized
  void resize_from_subordinate_model(size_t depth = SZ_MAX);
  /// update currentVariables using non-active data from the passed model
  /// (one of the ordered models)
  void update_from_subordinate_model(size_t depth = SZ_MAX);

  /// set the relative weightings for multiple objective functions or least
  /// squares terms and optionally recurses into LF/HF models
  void primary_response_fn_weights(const RealVector& wts,
                                   bool recurse_flag = true);

  /// set responseMode and pass any bypass request on to the high
  /// fidelity model for any lower-level surrogate recursions
  void surrogate_response_mode(short mode);

  /// use the high fidelity model to compute the truth values needed for
  /// correction of the low fidelity model results
  void build_approximation();
  // Uses the c_vars/response anchor point to define highFidResponse
  //bool build_approximation(const RealVector& c_vars,const Response& response);

  /// update component parallel mode for supporting parallelism in
  /// the low ad high fidelity models
  void component_parallel_mode(short mode);

  IntIntPair estimate_partition_bounds(int max_eval_concurrency);

  /// set up parallel operations for the array of ordered model fidelities
  void derived_init_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                  bool recurse_flag = true);
  /// set up serial operations for the array of ordered model fidelities
  void derived_init_serial();
  /// set active parallel configuration within the current low and
  /// high fidelity models identified by {low,high}FidelityKey
  void derived_set_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                 bool recurse_flag = true);
  /// deallocate communicator partitions for the HierarchSurrModel
  /// (request forwarded to the the array of ordered model fidelities)
  void derived_free_communicators(ParLevLIter pl_iter, int max_eval_concurrency,
                                  bool recurse_flag = true);

  /// Service the low and high fidelity model job requests received from the
  /// master; completes when termination message received from stop_servers().
  void serve_run(ParLevLIter pl_iter, int max_eval_concurrency);

  /// update the Model's inactive view based on higher level (nested)
  /// context and optionally recurse into
  void inactive_view(short view, bool recurse_flag = true);

  /// if recurse_flag, return true if orderedModels evaluation cache usage
  bool evaluation_cache(bool recurse_flag = true) const;
  /// if recurse_flag, return true if orderedModels restart file usage
  bool restart_file(bool recurse_flag = true) const;

  /// request fine-grained evaluation reporting within the low and high
  /// fidelity models
  void fine_grained_evaluation_counters();
  /// print the evaluation summary for the HierarchSurrModel
  /// (request forwarded to the low and high fidelity models)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
                                bool relative_count = true) const;

  /// set the warm start flag, including the orderedModels
  void warm_start_flag(const bool flag);

private:

  //
  //- Heading: Convenience functions
  //

  /// synchronize the HF model's solution level control with truthModelKey
  void assign_truth_key();
  /// synchronize the LF model's solution level control with surrModelKey
  void assign_surrogate_key();

  /// define truth and surrogate keys from incoming active key.  In case of
  /// singleton, use responseMode to disambiguate.
  void extract_model_keys(const Pecos::ActiveKey& active_key,
			  Pecos::ActiveKey& truth_key,
			  Pecos::ActiveKey& surr_key);
  /// define truth and surrogate keys from incoming active key.  In case of
  /// singleton, use component parallel mode to disambiguate.
  void extract_model_keys(const Pecos::ActiveKey& active_key,
			  Pecos::ActiveKey& truth_key,
			  Pecos::ActiveKey& surr_key, short parallel_mode);

  /// check for matching interface ids among active truth/surrogate models
  /// (varies based on active keys)
  bool matching_truth_surrogate_interface_ids();
  /// check for matching interface ids across full set of models (invariant)
  bool matching_all_interface_ids();
  /// update sameInterfaceInstance based on interface ids for models
  /// identified by current {low,high}FidelityKey
  void check_model_interface_instance();

  /// helper function used in the AUTO_CORRECTED_SURROGATE responseMode
  /// for computing a correction and applying it to lf_resp_map
  void compute_apply_delta(IntResponseMap& lf_resp_map);

  /// helper function for applying a single response correction corresponding
  /// to deltaCorr[paired_key]
  void single_apply(const Variables& vars, Response& resp,
		    const Pecos::ActiveKey& paired_key);
  /// helper function for applying a correction across a sequence of
  /// model forms or discretization levels
  void recursive_apply(const Variables& vars, Response& resp);

  /// stop the servers for the orderedModels instance identified by
  /// the passed index
  void stop_model(size_t ordered_model_index);

  //
  //- Heading: Data members
  //

  /// manages construction and application of correction functions that
  /// are applied to a surrogate model (DataFitSurr or HierarchSurr) in
  /// order to reproduce high fidelity data.
  std::map<Pecos::ActiveKey, DiscrepancyCorrection> deltaCorr;
  /// order of correction: 0, 1, or 2
  short corrOrder;

  unsigned short correctionMode;

  // sequence of discrepancy corrections to apply in SEQUENCE_CORRECTION mode
  //std::vector<Pecos::ActiveKey> corrSequence;

  /// Ordered sequence (low to high) of model fidelities.  Models are of
  /// arbitrary type and supports recursions.
  ModelArray orderedModels;

  /// key defining model form / resolution level for the active LF approximation
  Pecos::ActiveKey surrModelKey;
  /// store {LF,HF} model key that is active in component_parallel_mode()
  Pecos::ActiveKey componentParallelKey;

  /// map of raw continuous variables used by apply_correction().
  /// Model::varsList cannot be used for this purpose since it does
  /// not contain lower level variables sets from finite differencing.
  IntVariablesMap rawVarsMap;

  /// map of reference truth (high fidelity) responses computed in
  /// build_approximation() and used for calculating corrections
  std::map<Pecos::ActiveKey, Response> truthResponseRef;
};


inline HierarchSurrModel::~HierarchSurrModel()
{ }


inline size_t HierarchSurrModel::insert_response_start(size_t position)
{
  bool err_flag = false;
  switch (position) {
  case 0: return 0; break; // no offset
  case 1: // offset by HF size
    if (responseMode == AGGREGATED_MODELS)
      return
	truth_model().current_response().active_set_request_vector().size();
    else err_flag = true;
    break;
  default: err_flag = true; break;
  }
  if (err_flag) {
    Cerr << "Error: invalid position (" << position << ") in HierarchSurrModel"
	 << "::insert_response_start()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return SZ_MAX;
}


inline void HierarchSurrModel::
insert_metadata(const RealArray& md, size_t position, Response& agg_response)
{
  bool err_flag = false;
  switch (position) {
  case 0: agg_response.metadata(md, 0); break; // no offset
  case 1: // offset by HF size
    if (responseMode == AGGREGATED_MODELS)
      agg_response.metadata(md,
	truth_model().current_response().metadata().size());
    else err_flag = true;
    break;
  default: err_flag = true; break;
  }
  if (err_flag) {
    Cerr << "Error: invalid position (" << position << ") in HierarchSurrModel"
	 << "::insert_metadata()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
}


inline void HierarchSurrModel::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  EnsembleSurrModel::nested_variable_mappings(c_index1, di_index1, ds_index1,
					      dr_index1, c_target2, di_target2,
					      ds_target2, dr_target2);
  // forward along to subordinate models
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].nested_variable_mappings(c_index1, di_index1, ds_index1,
					      dr_index1, c_target2, di_target2,
					      ds_target2, dr_target2);
}


inline DiscrepancyCorrection& HierarchSurrModel::discrepancy_correction()
{ return deltaCorr[activeKey]; }


inline short HierarchSurrModel::correction_type()
{ return discrepancy_correction().correction_type(); }


inline void HierarchSurrModel::correction_type(short corr_type)
{
  std::map<Pecos::ActiveKey, DiscrepancyCorrection>::iterator it;
  for (it=deltaCorr.begin(); it!=deltaCorr.end(); ++it)
    it->second.correction_type(corr_type);
}


inline short HierarchSurrModel::correction_order()
{ return discrepancy_correction().correction_order(); }


inline Model& HierarchSurrModel::surrogate_model(size_t i)
{
  if (i == _NPOS) { // retrieve active surrogate, else indexed one
    unsigned short lf_form = surrModelKey.retrieve_model_form();
    i = (lf_form == USHRT_MAX) // empty key or undefined model form
      ? 0 : lf_form;
  }
  if (i >= orderedModels.size()) {
    Cerr << "Error: model index (" << i << ") out of range in "
	 << "HierarchSurrModel::surrogate_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return orderedModels[i];
}


inline const Model& HierarchSurrModel::surrogate_model(size_t i) const
{
  if (i == _NPOS) { // retrieve active surrogate, else indexed one
    unsigned short lf_form = surrModelKey.retrieve_model_form();
    i = (lf_form == USHRT_MAX) // empty key or undefined model form
      ? 0 : lf_form;
  }
  if (i >= orderedModels.size()) {
    Cerr << "Error: model index (" << i << ") out of range in "
	 << "HierarchSurrModel::surrogate_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return orderedModels[i];
}


inline Model& HierarchSurrModel::truth_model()
{
  unsigned short hf_form = truthModelKey.retrieve_model_form();
  if (hf_form == USHRT_MAX) // either empty key or undefined model form
    return orderedModels.back();
  else {
    if (hf_form >= orderedModels.size()) {
      Cerr << "Error: model index (" << hf_form << ") out of range in "
	   << "HierarchSurrModel::truth_model()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    return orderedModels[hf_form];
  }
}


inline const Model& HierarchSurrModel::truth_model() const
{
  unsigned short hf_form = truthModelKey.retrieve_model_form();
  if (hf_form == USHRT_MAX) // either empty key or undefined model form
    return orderedModels.back();
  else {
    if (hf_form >= orderedModels.size()) {
      Cerr << "Error: model form (" << hf_form << ") out of range in "
	   << "HierarchSurrModel::truth_model()" << std::endl;
      abort_handler(MODEL_ERROR);
    }
    return orderedModels[hf_form];
  }
}


inline bool HierarchSurrModel::matching_truth_surrogate_interface_ids()
{ return (surrogate_model().interface_id() == truth_model().interface_id()); }


inline bool HierarchSurrModel::matching_all_interface_ids()
{
  size_t m, num_m = orderedModels.size();
  const String& iface_id0 = orderedModels[0].interface_id();
  for (m=1; m<num_m; ++m)
    if (orderedModels[m].interface_id() != iface_id0)
      return false;
  return true;
}


inline void HierarchSurrModel::check_model_interface_instance()
{
  unsigned short lf_form =  surrModelKey.retrieve_model_form(),
                 hf_form = truthModelKey.retrieve_model_form();

  if (hf_form == USHRT_MAX || lf_form == USHRT_MAX)
    sameModelInstance = sameInterfaceInstance = false; // including both undef
  else {
    sameModelInstance = (lf_form == hf_form);
    sameInterfaceInstance = (sameModelInstance) ? true :
      matching_truth_surrogate_interface_ids();
  }
}


inline void HierarchSurrModel::assign_truth_key()
{
  unsigned short hf_form = truthModelKey.retrieve_model_form();
  if (hf_form != USHRT_MAX)
    orderedModels[hf_form].solution_level_cost_index(
      truthModelKey.retrieve_resolution_level());
}


inline void HierarchSurrModel::assign_surrogate_key()
{
  unsigned short lf_form = surrModelKey.retrieve_model_form();
  if (lf_form != USHRT_MAX)
    orderedModels[lf_form].solution_level_cost_index(
      surrModelKey.retrieve_resolution_level());
}


inline void HierarchSurrModel::
extract_model_keys(const Pecos::ActiveKey& active_key,
		   Pecos::ActiveKey& truth_key, Pecos::ActiveKey& surr_key)
{
  if (active_key.aggregated()) // AGGREGATED_MODELS, MODEL_DISCREPANCY
    active_key.extract_keys(truth_key, surr_key);
  else// single key: this version assigns to truth | surr based on responseMode
    switch (responseMode) {
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      surr_key  = active_key;  truth_key.clear();  break;
    default: // {BYPASS,NO}_SURROGATE
      truth_key = active_key;   surr_key.clear();  break;
    }
}


inline void HierarchSurrModel::
extract_model_keys(const Pecos::ActiveKey& active_key,
		   Pecos::ActiveKey& truth_key, Pecos::ActiveKey& surr_key,
		   short parallel_mode)
{
  if (active_key.aggregated())
    active_key.extract_keys(truth_key, surr_key);
  else// single key: this version assigns to truth | surr based on parallel mode
    switch (parallel_mode) {
    case SURROGATE_MODEL_MODE:
      surr_key  = active_key;  truth_key.clear();  break;
    case TRUTH_MODEL_MODE:
      truth_key = active_key;   surr_key.clear();  break;
    }
}


inline void HierarchSurrModel::active_model_key(const Pecos::ActiveKey& key)
{
  // assign activeKey
  SurrogateModel::active_model_key(key);
  // update {truth,surr}ModelKey
  extract_model_keys(key, truthModelKey, surrModelKey);
  // assign same{Model,Interface}Instance
  check_model_interface_instance();

  // If model forms are distinct (multifidelity case), can activate soln level
  // indices now and will persist; else (multilevel case) soln level is managed
  // for LF & HF contributions in derived_evaluate().
  if (sameModelInstance) {
    // Special case: multilevel data import in DataFitSurrModel::consistent()
    // requires correct state prior to evaluations in order to find level data.
    switch (responseMode) {
    case BYPASS_SURROGATE:      case NO_SURROGATE:
      assign_truth_key();     break;
    case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
      assign_surrogate_key(); break;
  //case AGGREGATED_MODELS: break; // defer setting active solution levels
    }
  }
  else {
    assign_truth_key();
    assign_surrogate_key();

    // Pull inactive variable change up into top-level currentVariables,
    // so that data flows correctly within Model recursions?  No, current
    // design is that forward pushes are automated, but inverse pulls are 
    // generally special case invocations from Iterator code (e.g., with
    // locally-managed Model recursions).
    //update_from_model(orderedModels[hf_form]);
  }

  switch (responseMode) {
  case MODEL_DISCREPANCY: case AUTO_CORRECTED_SURROGATE: {
    unsigned short lf_form = surrModelKey.retrieve_model_form();
    if (lf_form != USHRT_MAX) {// LF form def'd
      DiscrepancyCorrection& delta_corr = deltaCorr[key]; // per data group
      if (!delta_corr.initialized())
	delta_corr.initialize(surrogate_model(), surrogateFnIndices,
			      corrType, corrOrder);
    }
    break;
  }
  }
}


inline void HierarchSurrModel::clear_model_keys()
{
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].clear_model_keys();
}


inline size_t HierarchSurrModel::num_approximation_models() const
{ return orderedModels.size() - 1; }


inline void HierarchSurrModel::resize_maps()
{
  size_t num_steps = 2;
  if (modelIdMaps.size() != num_steps)
    modelIdMaps.resize(num_steps);
  if (cachedRespMaps.size() != num_steps)
    cachedRespMaps.resize(num_steps);
}


inline const unsigned short HierarchSurrModel::correction_mode() const
{ return correctionMode; }


inline void HierarchSurrModel::correction_mode(unsigned short corr_mode)
{ correctionMode = corr_mode; }


inline void HierarchSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i) {
    ml.push_back(orderedModels[i]);
    if (recurse_flag)
      orderedModels[i].derived_subordinate_models(ml, true);
  }
}


inline void HierarchSurrModel::resize_from_subordinate_model(size_t depth)
{
  bool lf_resize = false, hf_resize = false;
  switch (responseMode) {
  case AGGREGATED_MODELS:     case MODEL_DISCREPANCY:
    lf_resize = hf_resize = true; break;
  case UNCORRECTED_SURROGATE: case AUTO_CORRECTED_SURROGATE:
    lf_resize = true;             break;
  case BYPASS_SURROGATE:      case NO_SURROGATE:
    hf_resize = true;             break;
  }

  // bottom-up data flow, so recurse first
  if (lf_resize) {
    Model& lf_model = surrogate_model();
    if (depth == SZ_MAX)
      lf_model.resize_from_subordinate_model(depth);//retain special value (inf)
    else if (depth)
      lf_model.resize_from_subordinate_model(depth - 1);
  }
  if (hf_resize) {
    Model& hf_model = truth_model();
    if (depth == SZ_MAX)
      hf_model.resize_from_subordinate_model(depth);//retain special value (inf)
    else if (depth)
      hf_model.resize_from_subordinate_model(depth - 1);
  }

  if (lf_resize || hf_resize)
    resize_response();
}


inline void HierarchSurrModel::update_from_subordinate_model(size_t depth)
{
  switch (responseMode) {
  case UNCORRECTED_SURROGATE:      // LF only
  case AUTO_CORRECTED_SURROGATE: { // LF is active
    Model& lf_model = surrogate_model();
    // bottom-up data flow, so recurse first
    if (depth == SZ_MAX)
      lf_model.update_from_subordinate_model(depth);//retain special value (inf)
    else if (depth)
      lf_model.update_from_subordinate_model(depth - 1);
    // now pull updates from LF
    update_from_model(lf_model);
    break;
  }
  case BYPASS_SURROGATE:   case NO_SURROGATE:        // HF only
  case AGGREGATED_MODELS:  case MODEL_DISCREPANCY: { // prefer truth model
    Model& hf_model = truth_model();
    // bottom-up data flow, so recurse first
    if (depth == SZ_MAX)
      hf_model.update_from_subordinate_model(depth);//retain special value (inf)
    else if (depth)
      hf_model.update_from_subordinate_model(depth - 1);
    // now pull updates from HF
    update_from_model(hf_model);
    break;
  }
  }
}


inline void HierarchSurrModel::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  primaryRespFnWts = wts;
  if (recurse_flag) {
    size_t i, num_models = orderedModels.size();
    for (i=0; i<num_models; ++i)
      orderedModels[i].primary_response_fn_weights(wts, recurse_flag);
  }
}


inline void HierarchSurrModel::surrogate_response_mode(short mode)
{
  // Trap the combination of no user correction specification with either
  // AUTO_CORRECTED_SURROGATE (NO_CORRECTION defeats the point for HSModel) or
  // MODEL_DISCREPANCY (which formulation for computing discrepancy?) modes.
  if ( !corrType && ( mode == AUTO_CORRECTED_SURROGATE ||
		      mode == MODEL_DISCREPANCY ) ) {
    Cerr << "Error: activation of mode ";
    if (mode == AUTO_CORRECTED_SURROGATE) Cerr << "AUTO_CORRECTED_SURROGATE";
    else                                  Cerr << "MODEL_DISCREPANCY";
    Cerr << " requires specification of a correction type." << std::endl;
    abort_handler(MODEL_ERROR);
  }

  EnsembleSurrModel::surrogate_response_mode(mode);
}


inline IntIntPair HierarchSurrModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // responseMode is a run-time setting, so we are conservative on usage of
  // max_eval_concurrency as in derived_init_communicators()

  size_t i, num_models = orderedModels.size();
  IntIntPair min_max(INT_MAX, INT_MIN), min_max_i;
  for (i=0; i<num_models; ++i) {
    Model& model_i = orderedModels[i];
    probDescDB.set_db_model_nodes(model_i.model_id());
    min_max_i = model_i.estimate_partition_bounds(max_eval_concurrency);
    if (min_max_i.first  < min_max.first)  min_max.first  = min_max_i.first;
    if (min_max_i.second > min_max.second) min_max.second = min_max_i.second;
  }
  return min_max;

  // list nodes are reset at the calling level after completion of recursion
}


inline void HierarchSurrModel::derived_init_serial()
{
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].init_serial();
}


inline void HierarchSurrModel::stop_model(size_t ordered_model_index)
{
  Model& model = orderedModels[ordered_model_index];
  ParConfigLIter pc_it = model.parallel_configuration_iterator();
  size_t index = model.mi_parallel_level_index();
  if (pc_it->mi_parallel_level_defined(index) &&
      pc_it->mi_parallel_level(index).server_communicator_size() > 1)
    model.stop_servers();
}


inline void HierarchSurrModel::inactive_view(short view, bool recurse_flag)
{
  currentVariables.inactive_view(view);
  userDefinedConstraints.inactive_view(view);
  if (recurse_flag) {
    size_t i, num_models = orderedModels.size();
    for (i=0; i<num_models; ++i)
      orderedModels[i].inactive_view(view, recurse_flag);
  }
}


//inline const String& HierarchSurrModel::interface_id() const
//{ return orderedModels[]->interface_id(); }


inline bool HierarchSurrModel::evaluation_cache(bool recurse_flag) const
{
  if (recurse_flag) {
    size_t i, num_models = orderedModels.size();
    for (i=0; i<num_models; ++i)
      if (orderedModels[i].evaluation_cache(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline bool HierarchSurrModel::restart_file(bool recurse_flag) const
{
  if (recurse_flag) {
    size_t i, num_models = orderedModels.size();
    for (i=0; i<num_models; ++i)
      if (orderedModels[i].restart_file(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline void HierarchSurrModel::fine_grained_evaluation_counters()
{
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].fine_grained_evaluation_counters();
}


inline void HierarchSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
                         bool relative_count) const
{
  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].print_evaluation_summary(s,minimal_header,relative_count);
}


inline void HierarchSurrModel::warm_start_flag(const bool flag)
{
  // Note: supportsEstimDerivs prevents quasi-Newton Hessian accumulations
  warmStartFlag = flag; // for completeness

  size_t i, num_models = orderedModels.size();
  for (i=0; i<num_models; ++i)
    orderedModels[i].warm_start_flag(flag);
}

} // namespace Dakota

#endif
