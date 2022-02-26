/*  _______________________________________________________________________

    DAKOTA: Design Analysis Kit for Optimization and Terascale Applications
    Copyright 2014-2020 National Technology & Engineering Solutions of Sandia, LLC (NTESS).
    This software is distributed under the GNU Lesser General Public License.
    For more information, see the README file in the top Dakota directory.
    _______________________________________________________________________ */

//- Class:       NonHierarchSurrModel
//- Description: A model which manages a surrogate relationship between a low
//-              fidelity approximate model and a high fidelity truth model.
//- Owner:       Mike Eldred
//- Checked by:
//- Version: $Id: NonHierarchSurrModel.hpp 7024 2010-10-16 01:24:42Z mseldre $

#ifndef NON_HIERARCH_SURR_MODEL_H
#define NON_HIERARCH_SURR_MODEL_H

#include "EnsembleSurrModel.hpp"
#include "ParallelLibrary.hpp"
#include "DataModel.hpp"

namespace Dakota {


/// Derived model class within the surrogate model branch for managing
/// unordered surrogate models of varying fidelity.

/** The NonHierarchSurrModel class manages a set of models of varying
    fidelity.  The class contains an unordered array of approximation 
    models, where each model form may also contain a set of solution 
    levels (space/time discretization, convergence tolerances, etc.). */

class NonHierarchSurrModel: public EnsembleSurrModel
{
public:

  //
  //- Heading: Constructors and destructor
  //

  NonHierarchSurrModel(ProblemDescDB& problem_db); ///< constructor
  ~NonHierarchSurrModel();                         ///< destructor

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

  void nested_variable_mappings(const SizetArray& c_index1,
				const SizetArray& di_index1,
				const SizetArray& ds_index1,
				const SizetArray& dr_index1,
				const ShortArray& c_target2,
				const ShortArray& di_target2,
				const ShortArray& ds_target2,
				const ShortArray& dr_target2);

  void create_tabular_datastream();
  void derived_auto_graphics(const Variables& vars, const Response& resp);

  size_t num_approximation_models() const;
  void assign_default_keys();
  void resize_maps();
  void resize_response(bool use_virtual_counts = true);

  /// return the indexed approximate model from unorderedModels
  Model& surrogate_model(size_t i = _NPOS);
  /// return the indexed approximate model from unorderedModels
  const Model& surrogate_model(size_t i = _NPOS) const;

  /// return the high fidelity model
  Model& truth_model();
  /// return the high fidelity model
  const Model& truth_model() const;

  /// define the active model key
  void active_model_key(const Pecos::ActiveKey& key);
  /// remove keys for any approximations underlying {truth,unordered}Models
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

  // use the high fidelity model to compute the truth values needed for
  // correction of approximate model results
  //void build_approximation();

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
  /// deallocate communicator partitions for the NonHierarchSurrModel
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
  /// print the evaluation summary for the NonHierarchSurrModel
  /// (request forwarded to the low and high fidelity models)
  void print_evaluation_summary(std::ostream& s, bool minimal_header = false,
                                bool relative_count = true) const;

  /// set the warm start flag, including the orderedModels
  void warm_start_flag(const bool flag);

private:

  //
  //- Heading: Convenience functions
  //

  /// assign the resolution level for the model form indicated by the key
  void assign_key(const Pecos::ActiveKey& key);
  /// assign the resolution level for the i-th model key
  void assign_key(size_t i);

  /// check for matching interface ids among active truth/surrogate models
  /// (varies based on active keys)
  bool matching_truth_surrogate_interface_ids();
  /// check for matching interface ids across full set of models (invariant)
  bool matching_all_interface_ids();
  /// update sameInterfaceInstance based on interface ids for models
  /// identified by current {low,high}FidelityKey
  void check_model_interface_instance();

  /// stop the servers for the model instance identified by the passed id
  void stop_model(short model_id);

  /// check whether incoming ASV has any active content
  bool test_asv(const ShortArray& asv);

  //
  //- Heading: Data members
  //

  /// the single truth reference model
  Model truthModel;
  /// unordered set of model approximations
  ModelArray unorderedModels;

  /// keys defining model forms / resolution levels for the active set of
  /// approximations
  std::vector<Pecos::ActiveKey> surrModelKeys;
};


inline NonHierarchSurrModel::~NonHierarchSurrModel()
{ }


inline void NonHierarchSurrModel::
nested_variable_mappings(const SizetArray& c_index1,
			 const SizetArray& di_index1,
			 const SizetArray& ds_index1,
			 const SizetArray& dr_index1,
			 const ShortArray& c_target2,
			 const ShortArray& di_target2,
			 const ShortArray& ds_target2,
			 const ShortArray& dr_target2)
{
  // forward along to subordinate models:
  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i)
    unorderedModels[i].nested_variable_mappings(c_index1, di_index1, ds_index1,
					      dr_index1, c_target2, di_target2,
					      ds_target2, dr_target2);
  truthModel.nested_variable_mappings(c_index1, di_index1, ds_index1,
				      dr_index1, c_target2, di_target2,
				      ds_target2, dr_target2);
}


inline bool NonHierarchSurrModel::matching_all_interface_ids()
{
  size_t i, num_approx = unorderedModels.size();
  const String& hf_id  = truthModel.interface_id();
  for (i=0; i<num_approx; ++i)
    if (unorderedModels[i].interface_id() != hf_id)
      return false;
  return true;
}


inline bool NonHierarchSurrModel::matching_truth_surrogate_interface_ids()
{
  size_t i, num_approx = surrModelKeys.size();  unsigned short lf_form;
  const String& hf_id  = truthModel.interface_id();
  for (i=0; i<num_approx; ++i) {
    lf_form = surrModelKeys[i].retrieve_model_form();
    if (unorderedModels[lf_form].interface_id() != hf_id)
      return false;
  }
  return true;
}


inline void NonHierarchSurrModel::check_model_interface_instance()
{
  unsigned short hf_form = truthModelKey.retrieve_model_form();

  size_t i, num_approx = surrModelKeys.size();
  if (hf_form == USHRT_MAX || num_approx == 0)
    sameModelInstance = sameInterfaceInstance = false;
  else {
    sameModelInstance = true;
    for (i=0; i<num_approx; ++i)
      if (surrModelKeys[i].retrieve_model_form() != hf_form)
	{ sameModelInstance = false; break; }
    sameInterfaceInstance = (sameModelInstance) ? true :
      matching_truth_surrogate_interface_ids();
  }
}


inline Model& NonHierarchSurrModel::surrogate_model(size_t i)
{
  if (i == _NPOS) {
    //unsigned short lf_form = surrModelKeys[0].retrieve_model_form();
    //i = (lf_form == USHRT_MAX) // empty key or undefined model form
    //  ? 0 : lf_form;
    Cerr << "Error: model form must be specified in NonHierarchSurrModel::"
	 << "surrogate_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  else if (i >= unorderedModels.size()) {
    Cerr << "Error: model form (" << i << ") out of range in "
	 << "NonHierarchSurrModel::surrogate_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return unorderedModels[i];
}


inline const Model& NonHierarchSurrModel::surrogate_model(size_t i) const
{
  if (i == _NPOS) {
    //unsigned short lf_form = surrModelKeys[0].retrieve_model_form();
    //i = (lf_form == USHRT_MAX) // empty key or undefined model form
    //  ? 0 : lf_form;
    Cerr << "Error: model index must be specified in NonHierarchSurrModel::"
	 << "surrogate_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  if (i >= unorderedModels.size()) {
    Cerr << "Error: model index (" << i << ") out of range in "
	 << "NonHierarchSurrModel::surrogate_model()" << std::endl;
    abort_handler(MODEL_ERROR);
  }
  return unorderedModels[i];
}


inline size_t NonHierarchSurrModel::num_approximation_models() const
{ return unorderedModels.size(); }


inline Model& NonHierarchSurrModel::truth_model()
{ return truthModel; }


inline const Model& NonHierarchSurrModel::truth_model() const
{ return truthModel; }


inline void NonHierarchSurrModel::assign_key(const Pecos::ActiveKey& key)
{
  unsigned short form = key.retrieve_model_form();
  if (form != USHRT_MAX) {
    Model& model = (form < unorderedModels.size()) ?
      unorderedModels[form] : truthModel;
    model.solution_level_cost_index(key.retrieve_resolution_level());
  }
}


inline void NonHierarchSurrModel::assign_key(size_t i)
{
  if      (i  < surrModelKeys.size()) assign_key(surrModelKeys[i]);
  else if (i != _NPOS)                assign_key(truthModelKey);
}


inline void NonHierarchSurrModel::active_model_key(const Pecos::ActiveKey& key)
{
  // assign activeKey
  SurrogateModel::active_model_key(key);

  // update truthModelKey and surrModelKeys
  // TO DO: support group comprised only of approximations (no truth);
  //        implement extra logic as in HierarchSurrModel::extract_model_keys()
  key.extract_keys(truthModelKey, surrModelKeys);

  // assign same{Model,Interface}Instance
  check_model_interface_instance();

  // assign extracted keys
  if (sameModelInstance) {
    switch (responseMode) {
    case BYPASS_SURROGATE:  assign_key(truthModelKey);  break;
  //case AGGREGATED_MODELS: break; // defer setting active solution levels
    }
  }
  else { // approximations are separate models
    size_t i, num_approx = surrModelKeys.size();
    for (i=0; i<num_approx; ++i)
      assign_key(surrModelKeys[i]);
    assign_key(truthModelKey);
  }

  // Pull inactive variable change up into top-level currentVariables,
  // so that data flows correctly within Model recursions?  No, current
  // design is that forward pushes are automated, but inverse pulls are 
  // generally special case invocations from Iterator code (e.g., with
  // locally-managed Model recursions).
  //update_from_model(truthModel);
}


inline void NonHierarchSurrModel::clear_model_keys()
{
  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i)
    unorderedModels[i].clear_model_keys();
  truthModel.clear_model_keys();
}


/*
inline bool NonHierarchSurrModel::multilevel_from_keys() const
{
  bool ml = true;
  unsigned short hf_form = truthModelKey.retrieve_model_form();
  //if (hf_lev == SZ_MAX) return false;

  size_t         hf_lev  = truthModelKey.retrieve_resolution_level(),
    i, num_approx = surrModelKeys.size();
  for (i=0; i<num_approx; ++i) {
    const Pecos::ActiveKey& surr_key = surrModelKeys[i];
    if (surr_key.retrieve_model_form()       == hf_form &&
	surr_key.retrieve_resolution_level() != hf_lev)
      return true;
  }
  return false;
}
*/


inline void NonHierarchSurrModel::resize_maps()
{
  size_t num_steps = 1;
  if (responseMode == AGGREGATED_MODELS)
    num_steps += surrModelKeys.size();
  if (modelIdMaps.size() != num_steps)
    modelIdMaps.resize(num_steps);
  if (cachedRespMaps.size() != num_steps)
    cachedRespMaps.resize(num_steps);
}


inline bool NonHierarchSurrModel::test_asv(const ShortArray& asv)
{
  size_t i, num_fns = asv.size();
  for (i=0; i<num_fns; ++i)
    if (asv[i])
      return true;
  return false;
}


inline void NonHierarchSurrModel::
derived_subordinate_models(ModelList& ml, bool recurse_flag)
{
  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i) {
    ml.push_back(unorderedModels[i]);
    if (recurse_flag)
      unorderedModels[i].derived_subordinate_models(ml, true);
  }
  // ordered_models is low to high for HierarchSurr, so append truth last
  ml.push_back(truthModel);
  if (recurse_flag)
    truthModel.derived_subordinate_models(ml, true);
}


inline void NonHierarchSurrModel::resize_from_subordinate_model(size_t depth)
{
  bool approx_resize = false, truth_resize = false;
  switch (responseMode) {
  case AGGREGATED_MODELS:  approx_resize = truth_resize = true;  break;
  case BYPASS_SURROGATE:                   truth_resize = true;  break;
  }

  // bottom-up data flow, so recurse first
  if (approx_resize) {
    size_t i, num_unord = unorderedModels.size();
    for (i=0; i<num_unord; ++i) {
      Model& model_i = unorderedModels[i];
      if (depth == SZ_MAX)
	model_i.resize_from_subordinate_model(depth);// retain special val (inf)
      else if (depth)
	model_i.resize_from_subordinate_model(depth - 1);
    }
  }
  if (truth_resize) {
    if (depth == SZ_MAX)
      truthModel.resize_from_subordinate_model(depth);// retain special value
    else if (depth)
      truthModel.resize_from_subordinate_model(depth - 1);
  }
  // now resize this Models' response
  if (approx_resize || truth_resize)
    resize_response();
}


inline void NonHierarchSurrModel::update_from_subordinate_model(size_t depth)
{
  // bottom-up data flow, so recurse first
  if (depth == SZ_MAX)
    truthModel.update_from_subordinate_model(depth);//retain special value
  else if (depth)
    truthModel.update_from_subordinate_model(depth - 1);
  // now pull updates from HF
  update_from_model(truthModel);
}


inline void NonHierarchSurrModel::
primary_response_fn_weights(const RealVector& wts, bool recurse_flag)
{
  primaryRespFnWts = wts;
  if (recurse_flag) {
    size_t i, num_unord = unorderedModels.size();
    for (i=0; i<num_unord; ++i)
      unorderedModels[i].primary_response_fn_weights(wts, recurse_flag);
    truthModel.primary_response_fn_weights(wts, recurse_flag);
  }
}


inline IntIntPair NonHierarchSurrModel::
estimate_partition_bounds(int max_eval_concurrency)
{
  // responseMode is a run-time setting, so we are conservative on usage of
  // max_eval_concurrency as in derived_init_communicators()

  probDescDB.set_db_model_nodes(truthModel.model_id());
  IntIntPair min_max_i,
    min_max = truthModel.estimate_partition_bounds(max_eval_concurrency);

  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i) {
    Model& model_i = unorderedModels[i];
    probDescDB.set_db_model_nodes(model_i.model_id());
    min_max_i = model_i.estimate_partition_bounds(max_eval_concurrency);
    if (min_max_i.first  < min_max.first)  min_max.first  = min_max_i.first;
    if (min_max_i.second > min_max.second) min_max.second = min_max_i.second;
  }

  return min_max;

  // list nodes are reset at the calling level after completion of recursion
}


inline void NonHierarchSurrModel::derived_init_serial()
{
  size_t i, num_unord_models = unorderedModels.size();
  for (i=0; i<num_unord_models; ++i)
    unorderedModels[i].init_serial();
  truthModel.init_serial();
}


inline void NonHierarchSurrModel::stop_model(short model_id)
{
  if (model_id) {
    short  model_index = model_id - 1; // id to index
    Model& model = (model_index < unorderedModels.size()) ?
      unorderedModels[model_index] : truthModel;
    ParConfigLIter pc_it = model.parallel_configuration_iterator();
    size_t pl_index = model.mi_parallel_level_index();
    if (pc_it->mi_parallel_level_defined(pl_index) &&
	pc_it->mi_parallel_level(pl_index).server_communicator_size() > 1)
      model.stop_servers();
  }
}


inline void NonHierarchSurrModel::inactive_view(short view, bool recurse_flag)
{
  currentVariables.inactive_view(view);
  userDefinedConstraints.inactive_view(view);
  if (recurse_flag) {
    size_t i, num_unord = unorderedModels.size();
    for (i=0; i<num_unord; ++i)
      unorderedModels[i].inactive_view(view, recurse_flag);
    truthModel.inactive_view(view, recurse_flag);
  }
}


inline bool NonHierarchSurrModel::evaluation_cache(bool recurse_flag) const
{
  if (recurse_flag) {
    if (truthModel.evaluation_cache(recurse_flag))
      return true;
    size_t i, num_unord = unorderedModels.size();
    for (i=0; i<num_unord; ++i)
      if (unorderedModels[i].evaluation_cache(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline bool NonHierarchSurrModel::restart_file(bool recurse_flag) const
{
  if (recurse_flag) {
    if (truthModel.restart_file(recurse_flag))
      return true;
    size_t i, num_unord = unorderedModels.size();
    for (i=0; i<num_unord; ++i)
      if (unorderedModels[i].restart_file(recurse_flag))
	return true;
    return false;
  }
  else
    return false;
}


inline void NonHierarchSurrModel::fine_grained_evaluation_counters()
{
  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i)
    unorderedModels[i].fine_grained_evaluation_counters();
  truthModel.fine_grained_evaluation_counters();
}


inline void NonHierarchSurrModel::
print_evaluation_summary(std::ostream& s, bool minimal_header,
                         bool relative_count) const
{
  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i)
    unorderedModels[i].print_evaluation_summary(s, minimal_header,
						relative_count);
  // emulate low to high ordering as in HierarchSurrModel
  truthModel.print_evaluation_summary(s, minimal_header, relative_count);
}


inline void NonHierarchSurrModel::warm_start_flag(const bool flag)
{
  // Note: supportsEstimDerivs prevents quasi-Newton Hessian accumulations
  warmStartFlag = flag; // for completeness

  size_t i, num_unord = unorderedModels.size();
  for (i=0; i<num_unord; ++i)
    unorderedModels[i].warm_start_flag(flag);
  truthModel.warm_start_flag(flag);
}

} // namespace Dakota

#endif
